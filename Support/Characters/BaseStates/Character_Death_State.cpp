#include "Character_Death_State.hpp"
#include "Characters\Character.hpp"
#include "Characters\BaseStates\Character_Cover_State.hpp"
#include "Characters\God.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "Objects\Corpse.hpp"
#include "TriggerEx\Actions\action_ai_death.hpp"

#ifndef X_EDITOR
#include "NetworkMgr\NetworkMgr.hpp"
#endif


s32  g_ManDownIndex     = 1;
s32  g_DeathDownIndex   = 1;
const f32 k_MaxTimeExplodFalling = 4.0f;

//=========================================================================
// CHARACTER DEATH STATE
//=========================================================================

character_death_state::character_death_state( character& ourCharacter, states State ) :
    character_state(ourCharacter, State),
    m_bDeathFromTrigger( FALSE )
{

}

//=========================================================================

void character_death_state::OnEnter( void )
{
    LOG_MESSAGE("character_death_state::OnEnter","----------------");

    m_TimeInState = 0.0f;
    // Get a reference to the pain info 
    //const pain& Pain = m_CharacterBase.GetPainThatKilledUs();

    // Get loco pointer 
    loco* pLoco = m_CharacterBase.GetLocoPointer();
    ASSERT( pLoco );

    //
    // Do the misc housekeeping
    //
    {
        // Drop any inventory
        m_CharacterBase.DropInventory();

        // I guess loco need to know
        pLoco->SetDead(TRUE) ;

        // Stop any talking the character was doing
        if( g_ConverseMgr.IsActive( m_CharacterBase.GetVoiceID()) )
            g_ConverseMgr.Stop( m_CharacterBase.GetVoiceID());
    }

    // Death from a trigger action?  
    if( SetupTriggerDeath() )
        return;

    // 
    // if autoragdoll is true, let's do it! No anim or force just ragdoll.
    // 
    if( SetupAutoRagdollDeath() )
        return;

    //
    // If the character has a simple death animation then play the animation -
    // If a ragdoll rig is present, then at the end the anim, one will be created.
    // 
    //
    if( SetupSimpleDeath() )
        return;

    //
    // This death is for large explosive knockbacks.
    //
    if( SetupExplosiveDeath() )
        return;

    //
    // This death is for if the character is using a covernode
    //
    if( SetupCoverNodeDeath() )
        return;

    //
    // This death is picking a specific death animation if available
    //
    if( SetupAnimatedDeath() )
        return;

    //
    // This death handles general ragdoll tossing
    //
    if( SetupGeneralRagdollDeath() )
        return;

    ASSERT(FALSE);
}

//=========================================================================

void character_death_state::OnAdvance( f32 DeltaTime ) 
{
    // Check to see if animation has finished
    xbool bDeathAnimFinished = TRUE;
    loco* pLoco = m_CharacterBase.GetLocoPointer();
    if( pLoco )
    {
        // Let whole anim play if no ragdoll eg. dust mite
        const geom* pGeom = m_CharacterBase.GetGeomPtr();
        if( ( pGeom ) && ( pGeom->m_nRigidBodies == 0 ) )
        {
            // Wait for all anim to finish
            bDeathAnimFinished = pLoco->m_Player.IsAtEnd();
        }
        else
        {    
            // Lookup info (leaving room for inheriting velocities from animation)
            f32 LastFrame = pLoco->m_Player.GetNFrames() - 4.0f;
            f32 CurrFrame = pLoco->m_Player.GetCurrAnim().GetFrame();            
            
            // Done?
            bDeathAnimFinished = ( CurrFrame >= LastFrame );
        }
    }
    
    // Update state time
    m_TimeInState += DeltaTime;
	
	// Update our death based on its type
	switch ( m_CharacterBase.m_DeathType )
	{
    case character::DEATH_BY_RAGDOLL: 
        break;
        
    case character::DEATH_BY_EXPLOSION:
        {
            //if we collide or are no longer falling
            ASSERT( pLoco );
            if (pLoco->m_Physics.GetNavCollided() || 
                !(pLoco->m_Physics.GetFallMode() ||
                  pLoco->m_Physics.GetJumpMode()) ||
                m_TimeInState >= k_MaxTimeExplodFalling )
            {
                // Create corpse
                corpse* pCorpse = CreateAndInitCorpse(); 
                if( pCorpse )
                {
                    vector3 Distance = pLoco->m_Physics.GetVelocity();
                    Distance.Normalize();
                    pCorpse->GetPhysicsInst().ApplyVectorForce(Distance, 100.0f);                
                }
                                
                m_CharacterBase.KillMe();
                LOG_MESSAGE("character_death_state::OnAdvance","DEATH_BY_EXPLOSION");
		        return;
            }
        }
        break;
        
    case character::DEATH_BY_ANIM:
        {
            // Create ragdoll if the animation has finished
            if( bDeathAnimFinished )
            {
                LOG_MESSAGE("character_death_state::OnAdvance","DEATH_BY_ANIM");
                
                // Spawn ragdoll
                CreateAndInitCorpse();
                
                // Kill character                    
                m_CharacterBase.KillMe();
                return;
            }
        }
        break;
    case character::DEATH_SIMPLE:
    default:
    
        // Create ragdoll if the animation has finished
        if( bDeathAnimFinished )
        {
            LOG_MESSAGE("character_death_state::OnAdvance","DEATH_SIMPLE");
            
            // Create dead body if ragdoll rig exists so that the npc doesn't just vanish!
            geom* pGeom = m_CharacterBase.GetGeomPtr();
            if( ( pGeom ) && ( pGeom->m_nRigidBodies ) )
                CreateAndInitCorpse();
                
            // Kill character                    
            m_CharacterBase.KillMe();
            return;
        }
        break;
	}
}

//=========================================================================

corpse* character_death_state::CreateAndInitCorpse( void )
{
    // Create a dead body.
    corpse* pCorpse = m_CharacterBase.CreateCorpseObject( m_CharacterBase.GetBodyFades() );
    
    // If death came from a trigger, force the ragdoll to be active for at least this
    // time so that the rigid bodies can react to animated objects they maybe resting on
    // (just like in some of the death cinemas)
    if( ( pCorpse ) && ( m_bDeathFromTrigger ) )
        pCorpse->GetPhysicsInst().SetKeepActiveTime( 2.0f );
        
    return pCorpse;        
}

//=========================================================================

xbool character_death_state::SetupTriggerDeath( void )
{
    // Was previous state trigger?
    if( m_CharacterBase.GetLastState() != character_state::STATE_TRIGGER )
        return FALSE;

    // Lookup trigger state
    character_trigger_state* pTriggerState = (character_trigger_state*)m_CharacterBase.GetStateByType( character_state::STATE_TRIGGER );
    if( !pTriggerState )
        return FALSE;
        
    // Was last phase death?
    if( pTriggerState->GetCurrentPhase() != character_trigger_state::PHASE_TRIGGER_DEATH )
        return FALSE;
        
    // Okay, this definitely came from a trigger command - lookup the trigger data
    m_bDeathFromTrigger = TRUE;
    character_trigger_state::TriggerData TriggerData = pTriggerState->GetTriggerData();
    
    // Which type of death?
    switch( TriggerData.m_UnionData.m_DeathData.m_DeathType )
    {
    case action_ai_death::DEATH_TYPE_RAGDOLL:
    {
        // Create the ragdoll
        corpse* pCorpse = CreateAndInitCorpse();
        if( !pCorpse )
            return TRUE;
    
        // Setup force defaults
        vector3 ForcePos( m_CharacterBase.GetPosition() );
        vector3 ForceDir( 0.0f, 0.0f, 1.0f );
        f32     ForceAmount = TriggerData.m_UnionData.m_DeathData.m_RagdollForceAmount;
        f32     ForceRadius = TriggerData.m_UnionData.m_DeathData.m_RagdollForceRadius;
        
        // Lookup force position and direction from marker object if specified
        object* pMarker = g_ObjMgr.GetObjectByGuid( TriggerData.m_TriggerGuid );
        if( pMarker )
        {
            const matrix4& L2W = pMarker->GetL2W();
            ForcePos = L2W.GetTranslation();
            ForceDir = L2W.RotateVector( ForceDir );
            ForceDir.Normalize();
        }
        
        // Apply blast to corpse
        if( ForceAmount > 0.0f )
            pCorpse->GetPhysicsInst().ApplyBlast( ForcePos, ForceDir, ForceRadius, ForceAmount );
        
        // Play death audio        
        g_AudioMgr.PlayVolumeClipped( xfs( "%s_VOX_DEATH01", m_CharacterBase.GetDialogPrefix() ), 
            m_CharacterBase.GetPosition(), 
            m_CharacterBase.GetZone1(),
            TRUE );
        
        // Kill character        
        m_CharacterBase.m_DeathType = character::DEATH_BY_RAGDOLL;
        m_CharacterBase.KillMe();
        
        LOG_MESSAGE("character_death_state","TriggerDeath: ByRagdoll");
        
        return TRUE;
    }
    break;
    
    case action_ai_death::DEATH_TYPE_ANIM:
    {
        // Lookup loco
        loco* pLoco = m_CharacterBase.GetLocoPointer();
        ASSERT( pLoco );
        
        // Start death anim
        anim_group::handle hAnimGroup;
        hAnimGroup.SetName( TriggerData.m_UnionData.m_DeathData.m_AnimGroupName );
        pLoco->PlayDeathAnim( hAnimGroup, 
                              TriggerData.m_UnionData.m_DeathData.m_AnimName,
                              DEFAULT_BLEND_TIME );
        
        // Set type
        m_CharacterBase.m_DeathType = character::DEATH_BY_ANIM;
        
        LOG_MESSAGE("character_death_state","TriggerDeath: ByAnim");
        return TRUE;
    }    
    break;
    
    case action_ai_death::DEATH_TYPE_RAGDOLL_INACTIVE:
    {
        // Create the ragdoll
        corpse* pCorpse = CreateAndInitCorpse();
        if( !pCorpse )
            return TRUE;

        // Play death audio        
        g_AudioMgr.PlayVolumeClipped( xfs( "%s_VOX_DEATH01", m_CharacterBase.GetDialogPrefix() ), 
            m_CharacterBase.GetPosition(), 
            m_CharacterBase.GetZone1(),
            TRUE );

        // Kill character        
        m_CharacterBase.m_DeathType = character::DEATH_BY_RAGDOLL;
        m_CharacterBase.KillMe();

        // Freeze the ragdoll
        pCorpse->GetPhysicsInst().SetKeepActiveTime( 0.0f );
        pCorpse->GetPhysicsInst().Deactivate();
        
        LOG_MESSAGE("character_death_state","TriggerDeath: ByRagdollInactive");

        return TRUE;
    }
    break;
    
    }

    return FALSE;
}

//=========================================================================

xbool character_death_state::SetupAutoRagdollDeath()
{
    // Skip if max active corpses reached
    if( corpse::ReachedMaxActiveLimit() )
        return FALSE;

    // Auto ragdoll?
    if( m_CharacterBase.GetAutoRagdoll() )
    {    
        // Create corpse and take the pain impact
        corpse* pCorpse = CreateAndInitCorpse();
        if( !pCorpse )
            return FALSE;

        // Fling ragdoll            
        pCorpse->OnPain( m_CharacterBase.GetPainThatKilledUs() );                    

        // Play death audio        
        g_AudioMgr.PlayVolumeClipped( xfs( "%s_VOX_DEATH01", m_CharacterBase.GetDialogPrefix() ), 
            m_CharacterBase.GetPosition(),
            m_CharacterBase.GetZone1(), 
            TRUE );

        // Kill character        
        m_CharacterBase.m_DeathType = character::DEATH_BY_RAGDOLL;
        m_CharacterBase.KillMe();
        return TRUE;
    }
    return FALSE;
}

//=========================================================================

xbool character_death_state::SetupCoverNodeDeath( void )
{
    if( m_CharacterBase.GetLastState() != character_state::STATE_COVER )
        return FALSE;

    const pain& Pain = m_CharacterBase.GetPainThatKilledUs();
    character::eHitType HitType = m_CharacterBase.GetHitType( Pain );
    //character::hit_location HitLocation = m_CharacterBase.GetHitLocation( Pain );
    loco* pLoco = m_CharacterBase.GetLocoPointer();
    ASSERT( pLoco );

    character_cover_state* coverState = (character_cover_state*)m_CharacterBase.GetStateByType(character_state::STATE_COVER);
    if( coverState && coverState->GetIsInCover() )
    {       
        radian FaceYaw  = pLoco->m_Player.GetFacingYaw();
        radian PainYaw;
        if( Pain.GetDirection().LengthSquared() < 1.0f )
        {
            vector3 NewDir =  Pain.GetPosition() - m_CharacterBase.GetPosition();
            PainYaw = NewDir.GetYaw();
        }
        else
        {
            PainYaw = Pain.GetDirection().GetYaw();
        }

        radian DeltaYaw                     = x_MinAngleDiff( FaceYaw, PainYaw );    

        // Determine which animations are available
        if( m_CharacterBase.GetCoverAnimGroupHandle().GetPointer() )
        {
            const anim_group::handle& CoverAnimGroupHandle = m_CharacterBase.GetCoverAnimGroupHandle();
            xbool bHasBigBackDeath       = (CoverAnimGroupHandle.GetPointer()->GetAnimIndex("DEATH_BIG_BACK")     >= 0);
            xbool bHasSmallBackDeath     = (CoverAnimGroupHandle.GetPointer()->GetAnimIndex("DEATH_SMALL_BACK")   >= 0);
            xbool bHasBigForwardDeath    = (CoverAnimGroupHandle.GetPointer()->GetAnimIndex("DEATH_BIG_FORWARD")  >= 0);
            xbool bHasSmallForwardDeath  = (CoverAnimGroupHandle.GetPointer()->GetAnimIndex("DEATH_SMALL_FORWARD")>= 0);

            // Find death anim
            const char* pDeathAnim = NULL;
            if( (x_abs(DeltaYaw) > R_90) && (bHasBigBackDeath || bHasSmallBackDeath) )
            {
                if( ((HitType == character::HITTYPE_LIGHT) || (HitType == character::HITTYPE_IDLE) || (!bHasBigBackDeath)) &&
                    bHasSmallBackDeath )                        
                {
                    pDeathAnim = "DEATH_SMALL_BACK";
                }
                else if( bHasBigBackDeath )
                {
                    pDeathAnim = "DEATH_BIG_BACK";
                }
            }
            else if( bHasBigForwardDeath || bHasSmallForwardDeath )
            {
                if( ((HitType == character::HITTYPE_LIGHT) || (HitType == character::HITTYPE_IDLE) || (!bHasBigForwardDeath)) &&
                    bHasSmallForwardDeath )                   
                {
                    pDeathAnim = "DEATH_SMALL_FORWARD";
                }
                else if( bHasBigForwardDeath )
                {
                    pDeathAnim = "DEATH_BIG_FORWARD";
                }
            }    

            // If we found a covernode anim then we're finished
            if( pDeathAnim )
            {
                // Play death
                pLoco->PlayDeathAnim( CoverAnimGroupHandle, pDeathAnim, DEFAULT_BLEND_TIME );
                m_CharacterBase.m_DeathType = character::DEATH_BY_ANIM;
                LOG_MESSAGE("character_death_state","SetupCoverNodeDeath");
                return TRUE;
            }
        }
    }

    return FALSE;
}

//=========================================================================

xbool character_death_state::SetupExplosiveDeath( void )
{
    // Skip if max active corpses reached
    if( corpse::ReachedMaxActiveLimit() )
        return FALSE;

    // Get pain
    const pain& Pain = m_CharacterBase.GetPainThatKilledUs();

    // Only for explosions
    s32 HitType = Pain.GetHitType();
    if( ( HitType == 2 ) || ( HitType == 3 ) )
    {
        // Create corpse and take the pain impact
        corpse* pCorpse = CreateAndInitCorpse();
        if( pCorpse )
            pCorpse->OnPain( Pain );

        // Play death audio        
        g_AudioMgr.PlayVolumeClipped( xfs( "%s_VOX_DEATH01", m_CharacterBase.GetDialogPrefix() ), 
            m_CharacterBase.GetPosition(), 
            m_CharacterBase.GetZone1(),
            TRUE );

        // Kill character
        m_CharacterBase.m_DeathType = character::DEATH_BY_RAGDOLL;
        m_CharacterBase.KillMe();
    
        LOG_MESSAGE("character_death_state","SetupExplosiveDeath");
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

xbool character_death_state::SetupSimpleDeath( void )
{
    // Does anim exist?        
    if( m_CharacterBase.HasAnim(loco::ANIM_DEATH_SIMPLE))
    {
        LOG_MESSAGE("character_death_state","SetupSimpleDeath");
    
        // Start anim
        loco* pLoco = m_CharacterBase.GetLocoPointer();
        ASSERT( pLoco );
        pLoco->PlayDeathAnim( loco::ANIM_DEATH_SIMPLE, DEFAULT_BLEND_TIME );

        // Set type
        m_CharacterBase.m_DeathType = character::DEATH_SIMPLE;
        
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

xbool character_death_state::SetupAnimatedDeath( void )
{
    // Only for lite-arms fire
    const pain& Pain = m_CharacterBase.GetPainThatKilledUs();

    // Chance of playing one of these
    s32 ChanceOfPlayingAnim = 25;

    // If it was headshot then reconsider animation chance
    geom::bone::hit_location HitLocation = m_CharacterBase.GetHitLocation( Pain );
    if( HitLocation == geom::bone::HIT_LOCATION_HEAD )
        ChanceOfPlayingAnim = 50;

    // If max active corpse limit has been reached, then always play an anim!
    if( corpse::ReachedMaxActiveLimit() )
        ChanceOfPlayingAnim = 200;
    
    // Decide whether we should play an anim or just do generic ragdoll
    if( x_irand( 0, 100 ) > ChanceOfPlayingAnim )
        return FALSE;

    // Log it
    LOG_MESSAGE("character_death_state","SetupAnimatedDeath");

    // Get loco    
    loco* pLoco = m_CharacterBase.GetLocoPointer();
    ASSERT( pLoco );

    // Setup default values
    f32             BlendTime = DEFAULT_BLEND_TIME;
    loco::anim_type DeathAnim = m_CharacterBase.GetDeathAnim( Pain );

    // If we have an animation then set it up
    if( (DeathAnim != loco::ANIM_NULL) && m_CharacterBase.HasAnim(DeathAnim) )
    {
        // Spin the character around to line up with the pain more closely
        if( Pain.GetOriginGuid() )
        {
            object* pOriginOfPain = g_ObjMgr.GetObjectByGuid(Pain.GetOriginGuid());
            if (pOriginOfPain)
            {
                vector3 vToTarget = pOriginOfPain->GetPosition() - m_CharacterBase.GetPosition();

                //rotate to face appropriate direction
                switch(DeathAnim)
                {
                case loco::ANIM_DEATH_HARD_SHOT_IN_BACK_HIGH:
                case loco::ANIM_DEATH_HARD_SHOT_IN_BACK_MED:
                case loco::ANIM_DEATH_HARD_SHOT_IN_BACK_LOW:
                    BlendTime = 0.0f;
                    pLoco->SetYawFacingTarget(R_180 + vToTarget.GetYaw(), R_360);
                    break;
                case loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_HIGH:
                case loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_MED:
                case loco::ANIM_DEATH_HARD_SHOT_IN_FRONT_LOW:
                    BlendTime = 0.0f;
                    pLoco->SetYawFacingTarget(vToTarget.GetYaw(), R_360);
                    break;
                }
            }
        }

        // Play death anim
        pLoco->PlayDeathAnim( DeathAnim, BlendTime );
        
        // Set type
        m_CharacterBase.m_DeathType = character::DEATH_BY_ANIM;
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

xbool character_death_state::SetupGeneralRagdollDeath( void )
{
    // Create corpse and take the pain impact
    corpse* pCorpse = CreateAndInitCorpse();
    if( pCorpse )
        pCorpse->OnPain( m_CharacterBase.GetPainThatKilledUs() );                    

    // Play death audio        
    g_AudioMgr.PlayVolumeClipped( xfs( "%s_VOX_DEATH01", m_CharacterBase.GetDialogPrefix() ),  
        m_CharacterBase.GetPosition(), 
        m_CharacterBase.GetZone1(),
        TRUE );

    // Kill character
    m_CharacterBase.m_DeathType = character::DEATH_BY_RAGDOLL;
    m_CharacterBase.KillMe();
    
    return TRUE;
}

//=========================================================================
