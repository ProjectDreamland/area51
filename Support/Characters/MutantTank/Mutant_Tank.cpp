//=========================================================================
//
//  Mutant_Tank.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "Mutant_Tank.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Gamelib\StatsMgr.hpp"
#include "Objects\Player.hpp"
#include "Objects\SuperDestructible.hpp"
#include "Objects\ProjectileMutantParasite2.hpp"
#include "Objects\ProjectileMutantContagion.hpp"
#include "Objects\ProjectileEnergy.hpp"
#include "Objects\ParticleEventEmitter.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "TemplateMgr\TemplateMgr.hpp"


//=========================================================================
// DEFINES
//=========================================================================


#ifndef X_RETAIL
// Use to test in debugger
static f32 MUTANT_TANK_MOVE_SPEED = -1;    
static f32 MUTANT_TANK_CHARGE_SPEED = -1;
#endif

// Parasite shield vars
static f32 MUTANT_TANK_PARASITE_FADE_OUT_TIME = 2.0f;
static f32 MUTANT_TANK_PARASITE_FADE_IN_TIME = 2.0f;

// Contagion defines
#define CONTAGION_ATTACK_PROJECTILE_TIME    ( 0.25f )
#define CONTAGION_REGEN_SHIELD_TIME         ( 1.00f )

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct mutant_tank_desc : public object_desc
{
        mutant_tank_desc( void ) : object_desc( 
            object::TYPE_MUTANT_TANK, 
            "NPC - Mutant_Tank", 
            "AI",
            object::ATTR_NEEDS_LOGIC_TIME       |
            object::ATTR_COLLIDABLE             | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_RAGDOLL         | 
            object::ATTR_BLOCKS_CHARACTER_LOS   | 
            object::ATTR_BLOCKS_PLAYER_LOS      | 
            object::ATTR_BLOCKS_SMALL_DEBRIS    | 
            object::ATTR_RENDERABLE             |
            object::ATTR_TRANSPARENT            |
            object::ATTR_SPACIAL_ENTRY          |
            object::ATTR_CHARACTER_OBJECT       |
            object::ATTR_DAMAGEABLE             |
            object::ATTR_LIVING                 |
            object::ATTR_CAST_SHADOWS,

            FLAGS_TARGETS_OBJS |
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON               )  {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new mutant_tank; } 

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "SkinGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR
    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

} s_Mutant_Tank_Desc;

//=============================================================================

const object_desc& mutant_tank::GetTypeDesc( void ) const
{
    return s_Mutant_Tank_Desc;
}

//=============================================================================

const object_desc& mutant_tank::GetObjectType( void )
{
    return s_Mutant_Tank_Desc;
}


//=========================================================================
// MUTANT_TANK CHARACTER
//=========================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

mutant_tank::mutant_tank():
    m_Idle              ( *this, character_state::STATE_IDLE    ),
    m_Alert             ( *this, character_state::STATE_ALERT   ),
    m_Search            ( *this, character_state::STATE_SEARCH  ),
    m_Attack            ( *this, mutanttank_attack_state::STATE_ATTACK  ),
    m_Death             ( *this, character_state::STATE_DEATH   )
{
    // Setup pointer to loco for base class to use
    m_pLoco             = &m_Loco;

    m_Faction = FACTION_MUTANTS_GREATER;
    m_FriendFlags |= FACTION_MUTANTS_GREATER;

    m_CombatReady           = TRUE;    
    
    m_ProjectileTemplateID = -1;
    m_ParasiteTemplateID   = -1;

    m_bRegenerateSpores = FALSE;
    
    m_ShieldFxBoneName  = -1;
    m_ShieldFxBoneIndex = -1;
    
    // Pain vars
    m_PainLightTimer = 0.0f;
    m_PainHardTimer  = 0.0f;
    m_PainOrigin     = 0;
    m_PainFrame      = -1;
    
    // Jump vars
    m_iJumpAnim     = -1;
    m_JumpStartPos.Zero();
    m_JumpStartYaw   = R_0;
    m_JumpEndPos.Zero();
    m_JumpEndYaw     = R_0;
    m_bJumpInterpYaw = FALSE;
    m_bJumpInterpVert = TRUE;

    // Parasite shield vars    
    m_nParasites     = 0;
    m_ParasiteShieldPercent = -1.0f;
    m_ParasiteRegenSite.Zero();

    // perch and grating location guids.
    m_PerchGuid = NULL_GUID;
    m_PerchHoppoint = NULL_GUID;
    m_PerchHoppointRadius = 0.0f;

    m_GrateGuid = NULL_GUID;
    m_GrateHoppoint = NULL_GUID;
    m_GrateHoppointRadius = 0.0f;

    m_PerchScriptGuid = NULL_GUID;
    m_GrateScriptGuid = NULL_GUID;
    rhandle<char> FxResource;
    FxResource.SetName( "Theta_shield_Flash.fxo" );
    if( FxResource.GetPointer() )
    {    
        m_FXHandle.InitInstance( FxResource.GetPointer() );
        m_FXHandle.SetSuspended( TRUE );
    }
    m_ShieldHitBoneIndex = 0;

    // Finally, load attack state tweaks now everything is constructed
    // (spore guids have been cleared!)
    m_Attack.LoadTweaks();
}

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( pop )
#endif

//=========================================================================

mutant_tank::~mutant_tank()
{
    KillFx();
}

//=========================================================================

void mutant_tank::OnAdvanceLogic( f32 DeltaTime )
{
    character::OnAdvanceLogic( DeltaTime );
}

//=========================================================================

void mutant_tank::KillMe( )
{
    KillFx();
    s32 i; 
    for(i=0;i<SPORE_COUNT;i++)
    {
        // Lookup spore object (super destructible)
        spore&  Spore  = m_Spores[i];
        object* pSpore = g_ObjMgr.GetObjectByGuid( Spore.m_Guid );
        if( pSpore )
        {
            g_ObjMgr.DestroyObject( Spore.m_Guid );
        }
    }
    character::KillMe();
}

//=========================================================================

void mutant_tank::InitFx( rhandle<char>& hFx, fx_handle& Fx )
{
    // Init?
    if( ( Fx.Validate() == FALSE ) && ( hFx.GetPointer() ) )
    {
        Fx.InitInstance( hFx.GetPointer() );
        Fx.SetSuspended( TRUE );
    }
}

//=========================================================================

void mutant_tank::KillFx( fx_handle& Fx )
{
    Fx.KillInstance();
    m_FXHandle.KillInstance();
}

//=========================================================================

void mutant_tank::SetFxActive ( fx_handle& Fx, xbool bActive )
{
    Fx.SetSuspended( !bActive );
}

//=========================================================================

void mutant_tank::RestartFx( fx_handle& Fx )
{
    Fx.Restart();
    Fx.SetSuspended( FALSE );
}

//=========================================================================

void mutant_tank::MoveFx( fx_handle& Fx, const matrix4& L2W )
{
    Fx.SetScale      ( L2W.GetScale() );
    Fx.SetRotation   ( L2W.GetRotation() );
    Fx.SetTranslation( L2W.GetTranslation() );
}

//=========================================================================

void mutant_tank::MoveFx( fx_handle& Fx, object* pParent )
{
    MoveFx( Fx, pParent->GetL2W() );
}

//=========================================================================

void mutant_tank::MoveFx( fx_handle& Fx, s32 iBone )
{
    // Lookup L2W
    matrix4 L2W;
    if( iBone != -1 )
    {
        // Get animation L2W and remove inverse bind position which is baked in
        L2W = GetLocoPointer()->m_Player.GetBoneL2W( iBone );
        L2W.PreTranslate( GetLocoPointer()->m_Player.GetBoneBindPosition( iBone ) );
    }
    else
    {
        // If no bone specified, use tanks L2W
        L2W = GetL2W();
    }
    
    MoveFx( Fx, L2W );
}

//=========================================================================

void mutant_tank::AdvanceFx( fx_handle& Fx, f32 DeltaTime )
{
    Fx.AdvanceLogic( DeltaTime );
}

//=========================================================================

void mutant_tank::RenderFx( fx_handle& Fx )
{
    Fx.Render();
}

//=========================================================================

void mutant_tank::InitFx( void )
{
    // Init main particle effects
    InitFx( m_hShieldFx, m_ShieldFx );

    // Init spore particle effects
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Lookup spore object (super destructible)
        spore&  Spore  = m_Spores[i];
        object* pSpore = g_ObjMgr.GetObjectByGuid( Spore.m_Guid );
        if( pSpore )
        {
            InitFx( m_hSporeDestroyedFx,  Spore.m_DestroyedFx );
            InitFx( m_hSporeRegenerateFx, Spore.m_RegenerateFx );
            InitFx( m_hSporeHitFx,        Spore.m_HitFx );
        }
    }
}

//=========================================================================

void mutant_tank::KillFx( void )
{
    // Kill main particle effects
    KillFx( m_ShieldFx );

    // Kill spore particle effects
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Lookup spore
        spore&  Spore  = m_Spores[i];
        KillFx( Spore.m_DestroyedFx );
        KillFx( Spore.m_RegenerateFx );
        KillFx( Spore.m_HitFx );
    }
}

//=========================================================================

void mutant_tank::MoveObject( guid Guid, s32 iBone )
{
    // Lookup object
    object* pObject = g_ObjMgr.GetObjectByGuid( Guid );
    if( !pObject )
        return;

    // Lookup L2W
    matrix4 L2W;
    if( iBone != -1 )
    {
        // Get animation L2W and remove inverse bind position which is baked in
        L2W = GetLocoPointer()->m_Player.GetBoneL2W( iBone );
        L2W.PreTranslate( GetLocoPointer()->m_Player.GetBoneBindPosition( iBone ) );
    }
    else
    {
        // If no bone specified, use tanks L2W
        L2W = GetL2W();
    }

    // Update object        
    pObject->OnTransform( L2W );
    pObject->SetZone1( GetZone1() );
    pObject->SetZone2( GetZone2() );
}

//=========================================================================

void mutant_tank::UpdateSporesAndFx( f32 DeltaTime )
{
    if( !IsAlive() )
    {
        KillFx();
        return;
    }

    // Update shield fx
    MoveFx   ( m_ShieldFx, m_ShieldFxBoneIndex );
    AdvanceFx( m_ShieldFx, DeltaTime );

    // Update Shield Hit FX
    MoveFx   ( m_FXHandle, m_ShieldHitBoneIndex );
    AdvanceFx(m_FXHandle,DeltaTime);

    // Update spores
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Lookup spore object (super destructible)
        spore&                  Spore  = m_Spores[i];
        super_destructible_obj* pSpore = (super_destructible_obj*)g_ObjMgr.GetObjectByGuid( Spore.m_Guid );
        if( pSpore )
        {
            // Update flash timer
            Spore.m_FlashTimer = x_max( 0.0f, Spore.m_FlashTimer - DeltaTime );
        
            // Update spore position
            MoveObject( Spore.m_Guid, Spore.m_BoneIndex );
            
            // Update spore particle positions
            MoveFx( Spore.m_DestroyedFx,  pSpore );
            MoveFx( Spore.m_RegenerateFx, pSpore );
            MoveFx( Spore.m_HitFx,        pSpore );

            // Turn fx on/off
            SetFxActive( Spore.m_DestroyedFx, pSpore->IsDestroyed() );
            SetFxActive( Spore.m_RegenerateFx, m_bRegenerateSpores );

            // Advance spore particles
            AdvanceFx( Spore.m_DestroyedFx,  DeltaTime );
            AdvanceFx( Spore.m_RegenerateFx, DeltaTime );
            AdvanceFx( Spore.m_HitFx,        DeltaTime );

            // Regenerate?           
            if( m_bRegenerateSpores )
                pSpore->Regenerate( DeltaTime, 10.0f );
        }
    }
}

//=========================================================================

void mutant_tank::RenderFx( void )
{
    // Render shield fx
    RenderFx( m_ShieldFx );

    // render shield hits.
    RenderFx(m_FXHandle);
    
    // Render spore fx
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Lookup spore object (super destructible)
        spore&                  Spore  = m_Spores[i];
        super_destructible_obj* pSpore = (super_destructible_obj*)g_ObjMgr.GetObjectByGuid( Spore.m_Guid );
        if( pSpore )
        {
            // Render spore particles
            RenderFx( Spore.m_DestroyedFx );
            RenderFx( Spore.m_RegenerateFx );
            RenderFx( Spore.m_HitFx );
        }
    }
}

//=========================================================================

void mutant_tank::LaunchParasite( const vector3& Pos )
{
    // Create the parasite
    guid ParasiteGuid = g_TemplateMgr.CreateSingleTemplate( g_TemplateStringMgr.GetString( m_ParasiteTemplateID ), vector3(0.0f,0.0f,0.0f), radian3(0.0f,0.0f,0.0f), GetZone1(), GetZone2() ); 
    mutant_parasite_projectile* pParasite = ( mutant_parasite_projectile* )g_ObjMgr.GetObjectByGuid( ParasiteGuid );
    if( !pParasite )
        return;
    
    // Initialize
    pain_handle PainHandle("THETA_PARASITE");
    vector3 Dir = GetLastKnownLocationOfTarget() - Pos;
    radian3 Rot( Dir.GetPitch(), Dir.GetYaw(), 0.0f );
    vector3 Vel( 0.0f, 10.0f, 0.0f );
    f32     Speed = GetTweakF32( "THETA_ParasiteSpeed", 2000.0f );
    pParasite->Initialize( Pos, Rot, Vel, Speed, GetGuid(), PainHandle, net_GetSlot(), FALSE );
    pParasite->SetTarget( GetTargetGuid() );
}

//=========================================================================

void mutant_tank::LaunchProjectile( const vector3& Pos )
{
    // Create the parasite
    guid ProjectileID = CREATE_NET_OBJECT( energy_projectile::GetObjectType(), netobj::TYPE_BBG_1ST );
    energy_projectile* pProjectile = (energy_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
    if( !pProjectile )
        return;

    // Initialize
    pain_handle PainHandle( "THETA_PROJECTILE" );
    vector3 Dir = GetLastKnownLocationOfTarget() - Pos;
    radian3 Rot( Dir.GetPitch(), Dir.GetYaw(), 0.0f );
    vector3 Vel( 0.0f, 0.0f, GetTweakF32( "THETA_ProjectileSpeed", 1000.0f ) );
    Vel.Rotate( Rot );
    pProjectile->Setup( GetGuid(), net_GetSlot(), Pos, Rot, Vel, GetZone1(), GetZone2(), 0.0f, PainHandle );
    g_AudioMgr.Play( "Theta_Projectile", GetPosition(), GetZone1(), TRUE );
}

//=========================================================================

// do we want to jump to a hop point? 
/*guid mutant_tank::GetHopPoint( void )
{
    // first see if we are too close to the center.
    guid bestMarker = NULL_GUID;
    object* hopCenter = g_ObjMgr.GetObjectByGuid( m_HopPointCenter );
    if( hopCenter )
    {
        // only proceed if we are outside the check radius.
        if( GetToTarget(m_HopPointCenter).LengthSquared() > m_HopCheckRadius*m_HopCheckRadius )
        {
            s32 i;
            f32 shortestDist = -1;
            for(i=0;i<HOP_POINT_COUNT;i++)
            {
                object* hopMarker = g_ObjMgr.GetObjectByGuid( m_HopPoints[i] );
                if( hopMarker )
                {
                    if( GetToTarget( m_HopPoints[i] ).LengthSquared() < shortestDist || shortestDist < 0.0f )
                    {
                        bestMarker = m_HopPoints[i];
                        shortestDist = GetToTarget( m_HopPoints[i] ).LengthSquared();
                    }
                }
            }
        }
    }
    return bestMarker;
}*/

//=========================================================================

// Lookup attack phase (if any)
s32 mutant_tank::GetCurrentAttackPhase( void )
{
    // Default to non
    s32 Phase = mutanttank_attack_state::PHASE_NONE;
    
    // In attack state?
    if(     ( m_pActiveState ) 
        &&  ( m_pActiveState->GetStateType() == character_state::STATE_ATTACK ) )
    {
        // Get current phase
        Phase = m_pActiveState->GetCurrentPhase();
    }
    
    return Phase;
}

//=========================================================================

void mutant_tank::OnEvent( const event& Event )
{
    // Is this a weapon fire event?
    if( Event.Type == event::EVENT_WEAPON )
    {   
        // Lookup attack phase (if any)
        s32 Phase = GetCurrentAttackPhase();
    
        // Cast to actual event
        const weapon_event& WeaponEvent = weapon_event::GetSafeType( Event );

        // Create parasites?
        if( Phase == mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_BUBBLE )
        {            
            // Lookup a spore that has health
            s32 iSpore = GetSporeWithHealth( -1 );
            if( iSpore == -1 )
                return;
            
            // Lookup spore
            super_destructible_obj* pSpore = GetSporeObject( iSpore );
            ASSERT( pSpore );

            // Create parasite
            LaunchParasite( pSpore->GetPosition() );
            
            // Apply pain directly to spore to destroy it
            pain Pain;
            pain_handle PainHandle = GetPainHandleForGenericPain( TYPE_LETHAL );
            Pain.Setup( PainHandle, GetGuid(), pSpore->GetPosition() );
            Pain.SetDirectHitGuid( pSpore->GetGuid() );
            pSpore->OnPain( Pain );
        }
        else if( Phase == mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_RANGED_ATTACK )
        {
            // Launch a nice meson charge
            LaunchProjectile( WeaponEvent.Pos );
        }
        
        return;
    }        
        
    // Is this a generic super event?
    if( Event.Type == event::EVENT_GENERIC )
    {    
        // Get event
        const generic_event& GenericEvent = generic_event::GetSafeType( Event );
    
        // Is this a "SmashCanister" event?
        if( x_strcmp( GenericEvent.GenericType, "SmashCanister" ) == 0 )
        {
            // Lookup canister
            canister& Canister = GetCanister( m_Attack.m_iDestCanister );
            
            // Activate canister trigger
            object* pTrigger = g_ObjMgr.GetObjectByGuid( Canister.m_TriggerGuid );
            if( pTrigger )            
                pTrigger->OnActivate( TRUE ); 
            
            // Only trigger once
            Canister.m_TriggerGuid = 0;            
            return;
        }
        
        // Is this an init parasite shield event?
        if( x_strcmp( GenericEvent.GenericType, "InishShield" ) == 0 )
        {
            InitParasiteShield();
            return;
        }         
        
        // Is this a regenerate shield event?
        if( x_strcmp( GenericEvent.GenericType, "ShieldRegenParasite" ) == 0 )
        {
            RegenParasiteShield( GetTweakF32( "THETA_ParasiteShieldRegenPercent", 5.0f ) );
            return;
        }
        
        // Is this a shield launch effect?
        if( x_strcmp( GenericEvent.GenericType, "ShieldLaunch" ) == 0 )
        {
            // Create contagion from shield
            CreateParasiteShieldContagion();
            return;
        }         
    }        
                    
    // Call base class
    character::OnEvent(Event);
}

//=========================================================================

void mutant_tank::OnPain( const pain& Pain )
{
    // If pain came from self (leap shockwave) then skip!
    if( Pain.GetOriginGuid() == GetGuid() )
        return;

    // Already received pain this frame?
    // (this fixes the spores receiving multiple pains from grenade explosions)
    if( ( m_PainOrigin == Pain.GetOriginGuid() ) && ( m_PainFrame == g_ObjMgr.GetNLogicLoops() ) )
        return;

    // Record pain info so it only happens once per frame
    m_PainOrigin = Pain.GetOriginGuid();
    m_PainFrame  = g_ObjMgr.GetNLogicLoops();

    // The tank bone bounding boxes get in the way of the spores so
    // if we are close to a spore, take damage from it also.
    if( GetAllSporesHealth() > 0.0f )
    {   
        // Find closest spore with health and within hit dist (if any)
        s32 iSpore = GetClosestSpore( Pain.GetPosition(), 
                                      x_sqr( GetTweakF32( "THETA_BubbleHitDist", 40.0f ) ),
                                      0.0f );
        
        // Found one?
        if( iSpore != -1 )
        {
            // Apply pain to spore
            ApplyPainToSpore( iSpore, Pain );
                
            // Scale up damage before passing onto the theta
            pain BiggerPain = Pain;
            BiggerPain.SetCustomScalar( GetTweakF32( "THETA_BubbleDamageMultiplier", 1.5f ) );
            
            // Process pain to character
            character::OnPain( BiggerPain );
            return;
        }
    }
    
    // If there is a parasite shield particle close by, then delete it and flag we hit the shield
    xbool bHitShield = FALSE;
    const vector3& PainPos = Pain.GetPosition();
    for( s32 i = 0; i < m_nParasites; i++ )
    {
        // Lookup parasite and skip if already not visible
        parasite& Parasite = m_Parasites[i];
        if( !Parasite.m_bVisible )
            continue;

        // Close to pain?     
        f32 DistSqr = ( PainPos - Parasite.m_WorldPos ).LengthSquared();       
        if( DistSqr < x_sqr( 50.0f ) )
        {
            // Fade out and do not take any damage
            Parasite.m_bVisible = FALSE;

            m_ShieldHitBoneIndex = Parasite.m_iBone;

            // Record that shield was hit
            bHitShield = TRUE;
            break;
        }
    }
    
    // Was the parasite shield hit?
    if( bHitShield )
    {                                    
        // Scale damage before passing onto the theta
        pain ScaledPain = Pain;
        ScaledPain.SetCustomScalar( GetTweakF32( "THETA_ParasiteShieldDamageMultiplier", 0.5f ) );

        // Setup shield hit effect
        m_FXHandle.Restart();  
        m_FXHandle.SetTranslation( Pain.GetPosition() );

        radian3 painDirection(Pain.GetDirection().GetPitch(),Pain.GetDirection().GetYaw(),0.0f);
        m_FXHandle.SetRotation( painDirection );

        // Process pain to character
        character::OnPain( ScaledPain );
        return;
    }
               
    // Process pain
    character::OnPain( Pain );
}

//=========================================================================

xbool mutant_tank::OnChildPain( guid ChildGuid, const pain& Pain )
{
    // Make invulnerable when in trigger state 
    // (return TRUE so child does not process pain)
    if( GetActiveState() == character_state::STATE_TRIGGER )
        return TRUE;

    // If pain came from self (leap shockwave) then skip!
    if( Pain.GetOriginGuid() == GetGuid() )
        return TRUE;

    // Already received pain this frame?
    // (this fixes the spores receiving multiple pains from grenade explosions)
    if( ( m_PainOrigin == Pain.GetOriginGuid() ) && ( m_PainFrame == g_ObjMgr.GetNLogicLoops() ) )
        return TRUE;

    // We also need to apply this pain to the theta
    pain ThetaPain = Pain;
    ThetaPain.SetDirectHitGuid( GetGuid() );

    // Get spore that was hit
    s32 iSpore = GetSporeIndex( ChildGuid );
    if( iSpore != -1 )
    {
        // Apply pain to spore if it still has health
        if( GetSporeHealth( iSpore ) > 0.0f )
        {
            // Scale up the pain we give to the Theta
            ThetaPain.SetCustomScalar( GetTweakF32( "THETA_BubbleDamageMultiplier", 1.5f ) );
        }
        
        // Apply pain to the spore
        ApplyPainToSpore( iSpore, Pain );
    
        // Apply pain to self
        OnPain( ThetaPain );
    }

    // Record pain info so it only happens once per frame
    m_PainOrigin = Pain.GetOriginGuid();
    m_PainFrame  = g_ObjMgr.GetNLogicLoops();
        
    // Do not let child process pain since we already have
    return TRUE;
}

//=========================================================================

void mutant_tank::PlayImpactAnim( const pain& Pain, eHitType HitType )
{
    // Make invulnerable when in trigger state
    if( GetActiveState() == character_state::STATE_TRIGGER )
        return;
        
    // Lookup attack phase (if any)
    s32 Phase = GetCurrentAttackPhase();

    // Skip impacts altogether in these phases
    switch( Phase )
    {
        case mutanttank_attack_state::PHASE_MUTANTTANK_STAGE_RAGE:
        case mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_BUBBLE_ALIGN:
        case mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_BUBBLE:
            return;    
    }

    // Playing a light type?
    if( HitType == HITTYPE_LIGHT )
    {
        // Only perform hard anim every so often
        if( m_PainLightTimer < GetTweakF32( "THETA_LightPainInterval", 0.5f ) )
            return;

        // Reset timer and allow anim            
        m_PainLightTimer = 0.0f;
    }

    // Never play hard impacts in these stages - switch to light impact
    if( HitType == HITTYPE_HARD )    
    {
        switch( Phase )
        {
        case mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_CHARGE_ALIGN:
        case mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_CHARGE:
        case mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_LEAP:
        case mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_RANGED_ATTACK:
            HitType = HITTYPE_LIGHT;
            break;
        }
    }

    // Playing a hard type?
    if( HitType == HITTYPE_HARD )
    {
        // Only perform hard anim every so often
        if( m_PainHardTimer < GetTweakF32( "THETA_HardPainInterval", 8.0f ) )
        {
            HitType = HITTYPE_LIGHT;
        }            
        else
        {
            m_PainHardTimer = 0.0f;
        }            
    }
            
    // Never perform idle pains
    if( HitType == HITTYPE_IDLE )
        HitType = HITTYPE_LIGHT;
        
        
    // If dead, always play light impacts, otherwise we get resurrected!
    // SB: 
    // NOTE: Cannot call "IsDead" function because the m_bDead flag is
    //       never TRUE for the Theta - he's just permanently in his death anim.
    //       This will be fixed when rigid body ragdoll is put in sometime in the future...         
    if( GetHealth() <= 0.0f )        
        HitType = HITTYPE_LIGHT;
    
    // Process impact anim
    character::PlayImpactAnim( Pain, HitType );
}

//=========================================================================

xbool mutant_tank::TakeDamage( const pain& Pain )
{
    // Nothing to do if already dead
    // SB: 
    // NOTE: Cannot call "IsDead" function because the m_bDead flag is
    //       never TRUE for the Theta - he's just permanently in his death anim.
    //       This will be fixed when rigid body ragdoll is put in sometime in the future...         
    if( GetHealth() <= 0.0f )        
        return TRUE;

    // Make invulnerable when in trigger state
    if( GetActiveState() == character_state::STATE_TRIGGER )
        return TRUE;
    
    // Lookup attack phase (if any)
    s32 Phase = GetCurrentAttackPhase();

    // Skip damage altogether on these phases
    switch( Phase )
    {
        case mutanttank_attack_state::PHASE_MUTANTTANK_STAGE_RAGE:
        case mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_BUBBLE_ALIGN:
        case mutanttank_attack_state::PHASE_MUTANTTANK_ATTACK_BUBBLE:
            return TRUE;    
    }

    // Process damage as normal and compute the damage taken
    f32   PrevHealth  = GetHealth();
    xbool bStatus     = character::TakeDamage( Pain );
    f32   CurrHealth  = GetHealth();
    f32   DamageTaken = PrevHealth - CurrHealth;

    // Lookup stage
    ASSERT( m_Attack.m_CurrentStage >= 0 );
    ASSERT( m_Attack.m_CurrentStage < m_Attack.m_nStages );
    mutanttank_attack_state::stage& Stage = m_Attack.m_Stages[ m_Attack.m_CurrentStage ];

    // If this is a bubble stage and bubble health has been taken, then flag we want to do a bubble attack
    if(     ( Stage.m_BubbleHealthInterval != -1 )
        &&  ( GetAllSporesHealth() > 0 ) )
    {
        // Update bubble health
        m_Attack.m_BubbleHealth -= DamageTaken;
    }    
    
    // If this is a canister stage and there are some remaining, then flag we want to do a canister attack
    if( ( Stage.m_CanisterHealthInterval != -1 ) && ( GetCanisterCount() ) )
    {
        // Update canister health
        m_Attack.m_CanisterHealth -= DamageTaken;
    }

    // If this is a contagion stage and there are some remaining, then flag we want to do a contagion attack
    if( Stage.m_ContagionHealthInterval != -1 )
    {
        // Update contagion health
        m_Attack.m_ContagionHealth -= DamageTaken;
    }

    return bStatus;
}

//=========================================================================

void mutant_tank::OnRender( void )
{
    // Call base class to render mutant
    character::OnRender();
    
    // Render any flashing spores?
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Lookup spore
        spore& Spore = m_Spores[ i ];
        
        // Flashing?
        if( Spore.m_FlashTimer > 0.0f )
        {
            // Get spore
            object_ptr<super_destructible_obj> pSpore( Spore.m_Guid );
            if( pSpore )
            {
                // Flash it by rendering it multiple times!
                pSpore->OnRender();
                pSpore->OnRender();
            }
        }
    }
    
// Render stage, health etc in sbroumley or jfranklin builds ONLY
#if !defined( X_RETAIL )
#if defined( sbroumley ) || defined( jfranklin )
    // TO DO: Render real boss bar?
    x_printfxy( 1,1, "Stage:%d Health:%.1f Spores:%.1f", 
        m_Attack.m_CurrentStage,
        GetHealth(), 
        GetAllSporesHealth() );
#endif  //#if defined( sbroumley ) || defined( jfranklin )
#endif  //#if !defined( X_RETAIL )
}

//=========================================================================

void mutant_tank::OnRenderTransparent( void )
{
    character::OnRenderTransparent();

    // Render fx particles
    RenderFx();    
}

//=========================================================================
// EDITOR FUNCTIONS
//=========================================================================

void mutant_tank::OnEnumProp ( prop_enum&    List )
{
    s32 i;
    
    // Call base class
    character::OnEnumProp(List);

    // Header
    List.PropEnumHeader( "Mutant_Tank","Mutant_Tank NPC", 0 );
    List.PropEnumFileName( "Mutant_Tank\\Projectile Blueprint Path",
        "Area51 blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||",
        "Resource for this item",
        PROP_TYPE_MUST_ENUM );
    List.PropEnumFileName( "Mutant_Tank\\Parasite Blueprint Path",
        "Area51 blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||",
        "Resource for this item",
        PROP_TYPE_MUST_ENUM );
    
    // Create "bone\0bone\0\12345678:12345678\0\0"
    char BoneExternal[32] = {0};
    x_strcpy( BoneExternal, "bone" );
    x_strcpy( &BoneExternal[5], "bone\\" );
    x_strcpy( &BoneExternal[10], (const char*)guid_ToString( GetGuid() ) );

    // Particle effects    
    List.PropEnumExternal( "Mutant_Tank\\ShieldFx",          "Resource\0fxo\0",  "Shield particle effect.", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Mutant_Tank\\ShieldFxBone",      BoneExternal,       "Name of bone to attach shield to.", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Mutant_Tank\\SporeDestroyedFx",  "Resource\0fxo\0",  "Particle effect for when spores are destroyed.", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Mutant_Tank\\SporeRegenerateFx", "Resource\0fxo\0",  "Particle effect for when spores are regenerating.", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Mutant_Tank\\SporeHitFx",        "Resource\0fxo\0",  "Particle effect for when spores are hit.", PROP_TYPE_MUST_ENUM );
    
    // Spores
    for( i = 0; i < SPORE_COUNT; i++ )
    {
        // Add spore header
        List.PropEnumHeader  ( xfs( "Mutant_Tank\\Spore[%d]", i ), "Connected spore.", 0 );
        s32 iHeader = List.PushPath( xfs( "Mutant_Tank\\Spore[%d]\\", i ) );
        
        // Add spore properties
        List.PropEnumGuid    ( "Guid",            "Guid of spore to connect.", 0 );
#ifdef USE_OBJECT_NAMES
        List.PropEnumString  ( "Name",            "Name of spore object.", PROP_TYPE_READ_ONLY );
#endif        
        List.PropEnumExternal( "Bone",   BoneExternal, "Name of bone to attach to.", PROP_TYPE_MUST_ENUM );
        
        // Pop spore header
        List.PopPath( iHeader );
    }
    
    // Canisters
    for( i = 0; i < CANISTER_COUNT; i++ )
    {
        // Add canister header
        List.PropEnumHeader  ( xfs( "Mutant_Tank\\Canister[%d]", i ), "Canister that theta jumps up to.", 0 );
        s32 iHeader = List.PushPath( xfs( "Mutant_Tank\\Canister[%d]\\", i ) );

        // Center marker properties
        List.PropEnumGuid    ( "CenterGuid",            "Guid of marker placed at center of canister.", 0 );

#ifdef USE_OBJECT_NAMES
        List.PropEnumString  ( "CenterName",            "Name of marker placed at center of canister.", PROP_TYPE_READ_ONLY );
#endif        

        // Attach marker properties
        List.PropEnumGuid    ( "AttachGuid",            "Guid of marker placed at Theta attach point on canister.", 0 );

#ifdef USE_OBJECT_NAMES
        List.PropEnumString  ( "AttachName",            "Name of marker placed at Theta attach point on canister.", PROP_TYPE_READ_ONLY );
#endif        

        // Trigger properties
        List.PropEnumGuid    ( "TriggerGuid",            "Guid of trigger placed next to canister to activate alien blobs.", 0 );

#ifdef USE_OBJECT_NAMES
        List.PropEnumString  ( "TriggerName",            "Name of trigger placed next to canister to activate alien blobs.", PROP_TYPE_READ_ONLY );
#endif        

        // Pop canister header
        List.PopPath( iHeader );
    }
/*    List.PropEnumGuid    ( "Mutant_Tank\\No Hop Center", "Guid of marker placed at the center of the area the TANK will not hop if he is inside. Just ask Jason.", 0 );
    List.PropEnumFloat   ( "Mutant_Tank\\No Hop Radius", "radius around marker placed at the center of the area the TANK will not hop if he is inside. Just ask Jason.", 0 );
    List.PropEnumHeader  ( "Mutant_Tank\\Hop Points", "Point the theta will hop to.", 0 );
    s32 iHeader = List.PushPath( "Mutant_Tank\\Hop Points\\" );
    for(i=0;i<HOP_POINT_COUNT;i++)
    {
        List.PropEnumGuid    ( xfs("Hop Point Guid [%d]",i), "Guid of marker placed at the position of the theta will hop to before jumping up to stargate.", 0 );
    }
    List.PopPath( iHeader );    */

/*    List.PropEnumHeader  ( "Mutant_Tank\\Stargate Hooks", "Hooks that theta jumps up to.", 0 );
    iHeader = List.PushPath( "Mutant_Tank\\Stargate Hooks\\" );
    for( i = 0; i < STARGATE_HOOK_COUNT; i++ )
    {
        List.PropEnumGuid    ( xfs("Perch Hook Guid [%d]",i), "Guid of marker placed at the position of the theta when hanging from the hooks used to jump to the perch.", 0 );
    }
    for( i = 0; i < STARGATE_HOOK_COUNT; i++ )
    {
        List.PropEnumGuid    ( xfs("Grating Hook Guid [%d]",i), "Guid of marker placed at the position of the theta when hanging from the hooks used to jump to the grating.", 0 );
    }        
    List.PopPath( iHeader );        */
    List.PropEnumGuid   ( "Mutant_Tank\\Perch Guid", "Guid of marker placed at tanks landing spot on the perch facing his landing direction.", 0 );
    List.PropEnumGuid   ( "Mutant_Tank\\Perch Hoppoint Guid", "Guid of marker placed at tanks landing spot on the perch facing his landing direction.", 0 );
    List.PropEnumFloat  ( "Mutant_Tank\\Perch Hoppoint Radius", "Guid of marker placed at tanks landing spot on the perch facing his landing direction.", 0 );
//    List.PropEnumGuid    ( "Mutant_Tank\\Perch End Guid", "Guid of marker placed at tanks landing spot when jumping off the perch.", 0 );
    List.PropEnumGuid   ( "Mutant_Tank\\Grating Guid", "Guid of marker placed at tanks landing spot on the grating facing his landing direction.", 0 );
    List.PropEnumGuid   ( "Mutant_Tank\\Grating Hoppoint Guid", "Guid of marker placed at tanks landing spot on the grating facing his landing direction.", 0 );
    List.PropEnumFloat  ( "Mutant_Tank\\Grating Hoppoint Radius", "Guid of marker placed at tanks landing spot on the grating facing his landing direction.", 0 );

    List.PropEnumGuid   ( "Mutant_Tank\\Perch Script Guid", "Script to activate when Theta Roars at the perch.", 0 );
    List.PropEnumGuid   ( "Mutant_Tank\\Grate Script Guid", "Script to activate when the Theta starts the smash the grate animation.", 0 );
}

//=============================================================================

xbool mutant_tank::OnProperty ( prop_query& I )
{
    // Call base class
    if (character::OnProperty(I))
    {
        return TRUE;
    }
    else if( SMP_UTIL_IsTemplateVar( I, "Mutant_Tank\\Projectile Blueprint Path", m_ProjectileTemplateID ) )
    {
        return TRUE;
    }
    else if( SMP_UTIL_IsTemplateVar( I, "Mutant_Tank\\Parasite Blueprint Path", m_ParasiteTemplateID ) )
    {
        return TRUE;
    }
    else if( SMP_UTIL_IsParticleFxVar( I, "Mutant_Tank\\ShieldFx", m_hShieldFx ) )
    {
        return TRUE;
    }
    else if( SMP_UTIL_IsBoneVar( I, "Mutant_Tank\\ShieldFxBone", *this, m_ShieldFxBoneName, m_ShieldFxBoneIndex ) )
    {
        return TRUE;
    }        
    else if( SMP_UTIL_IsParticleFxVar( I, "Mutant_Tank\\SporeDestroyedFx", m_hSporeDestroyedFx ) )
    {
        return TRUE;
    }
    else if( SMP_UTIL_IsParticleFxVar( I, "Mutant_Tank\\SporeRegenerateFx", m_hSporeRegenerateFx ) )
    {
        return TRUE;
    }
    else if( SMP_UTIL_IsParticleFxVar( I, "Mutant_Tank\\SporeHitFx", m_hSporeHitFx ) )
    {
        return TRUE;
    }
    else if( I.IsSimilarPath( "Mutant_Tank\\Spore[" ) )
    {
        // Lookup spore
        s32     Index = I.GetIndex( 0 );
        spore&  Spore = m_Spores[ Index ];

        // Spore guid?
        if( I.VarGUID( "Mutant_Tank\\Spore[]\\Guid", Spore.m_Guid ) )
        {
            // Tell the spore its parent
            object_ptr<super_destructible_obj> pSpore( Spore.m_Guid );
            if( pSpore )
                pSpore->SetParentGuid( GetGuid() );
        
            return TRUE;
        }
        
#ifdef USE_OBJECT_NAMES
        // Name?
        if( SMP_UTIL_IsObjectNameVar( I, "Mutant_Tank\\Spore[]\\Name", Spore.m_Guid ) )
            return TRUE;
#endif
        
        // Bone?
        if( SMP_UTIL_IsBoneVar( I, "Mutant_Tank\\Spore[]\\Bone", *this, Spore.m_BoneName, Spore.m_BoneIndex ) )
            return TRUE;
    }
    else if( I.IsSimilarPath( "Mutant_Tank\\Canister[" ) )
    {
        // Lookup canister
        s32         Index    = I.GetIndex( 0 );
        canister&   Canister = m_Canisters[ Index ];

        // Center guid?
        if( I.VarGUID( "Mutant_Tank\\Canister[]\\CenterGuid", Canister.m_CenterGuid ) )
            return TRUE;

        // Attach guid?
        if( I.VarGUID( "Mutant_Tank\\Canister[]\\AttachGuid", Canister.m_AttachGuid ) )
            return TRUE;

        // Trigger guid?
        if( I.VarGUID( "Mutant_Tank\\Canister[]\\TriggerGuid", Canister.m_TriggerGuid ) )
            return TRUE;

#ifdef USE_OBJECT_NAMES

        if( SMP_UTIL_IsObjectNameVar( I, "Mutant_Tank\\Canister[]\\CenterName", Canister.m_CenterGuid ) )
            return TRUE;

        if( SMP_UTIL_IsObjectNameVar( I, "Mutant_Tank\\Canister[]\\AttachName", Canister.m_AttachGuid ) )
            return TRUE;

        if( SMP_UTIL_IsObjectNameVar( I, "Mutant_Tank\\Canister[]\\TriggerName", Canister.m_TriggerGuid ) )
            return TRUE;
#endif
    }
    else if( I.IsSimilarPath( "Mutant_Tank\\Canister[" ) )
    {
        // Lookup canister
        s32         Index    = I.GetIndex( 0 );
        canister&   Canister = m_Canisters[ Index ];

        // Center guid?
        if( I.VarGUID( "Mutant_Tank\\Canister[]\\CenterGuid", Canister.m_CenterGuid ) )
            return TRUE;

        // Attach guid?
        if( I.VarGUID( "Mutant_Tank\\Canister[]\\AttachGuid", Canister.m_AttachGuid ) )
            return TRUE;

        // Trigger guid?
        if( I.VarGUID( "Mutant_Tank\\Canister[]\\TriggerGuid", Canister.m_TriggerGuid ) )
            return TRUE;

#ifdef USE_OBJECT_NAMES

        if( SMP_UTIL_IsObjectNameVar( I, "Mutant_Tank\\Canister[]\\CenterName", Canister.m_CenterGuid ) )
            return TRUE;

        if( SMP_UTIL_IsObjectNameVar( I, "Mutant_Tank\\Canister[]\\AttachName", Canister.m_AttachGuid ) )
            return TRUE;

        if( SMP_UTIL_IsObjectNameVar( I, "Mutant_Tank\\Canister[]\\TriggerName", Canister.m_TriggerGuid ) )
            return TRUE;
#endif
    }
/*    else if( I.VarGUID( "Mutant_Tank\\No Hop Center", m_HopPointCenter ) )
        return TRUE;
    else if( I.VarFloat( "Mutant_Tank\\No Hop Radius", m_HopCheckRadius ) )
        return TRUE;
    else if( I.IsSimilarPath( "Mutant_Tank\\Hop Points\\Hop Point Guid [" ) )
    {
        s32         Index    = I.GetIndex( 0 );
        if( I.VarGUID( "Mutant_Tank\\Hop Points\\Hop Point Guid []", m_HopPoints[Index] ) )
            return TRUE;

    }*/ 
/*    else if( I.IsSimilarPath( "Mutant_Tank\\Stargate Hooks\\Perch Hook Guid [" ) )
    {
        // Lookup stargate perch hook
        s32         Index    = I.GetIndex( 0 );
        stargate_hooks&  StargateHook = m_PerchHooks[ Index ];

        // Center guid?
        if( I.VarGUID( "Mutant_Tank\\Stargate Hooks\\Perch Hook Guid []", StargateHook.m_AttachGuid ) )
            return TRUE;

    }
    else if( I.IsSimilarPath( "Mutant_Tank\\Stargate Hooks\\Grating Hook Guid [" ) )
    {
        // Lookup stargate perch hook
        s32         Index    = I.GetIndex( 0 );
        stargate_hooks&  StargateHook = m_GrateHooks[ Index ];

        // Center guid?
        if( I.VarGUID( "Mutant_Tank\\Stargate Hooks\\Grating Hook Guid []", StargateHook.m_AttachGuid ) )
            return TRUE;
    }
*/    
    if( I.VarGUID( "Mutant_Tank\\Perch Guid", m_PerchGuid ) )
        return TRUE;
    if( I.VarGUID( "Mutant_Tank\\Perch Hoppoint Guid", m_PerchHoppoint ) )
        return TRUE;
    if( I.VarFloat( "Mutant_Tank\\Perch Hoppoint Radius", m_PerchHoppointRadius ) )
        return TRUE;

    if( I.VarGUID( "Mutant_Tank\\Grating Guid", m_GrateGuid ) )
        return TRUE;
    if( I.VarGUID( "Mutant_Tank\\Grating Hoppoint Guid", m_GrateHoppoint ) )
        return TRUE;
    if( I.VarFloat( "Mutant_Tank\\Grating Hoppoint Radius", m_GrateHoppointRadius ) )
        return TRUE;
  
    // script to activate
    if( I.VarGUID( "Mutant_Tank\\Perch Script Guid", m_PerchScriptGuid ) )
        return TRUE;
    // script to activate
    if( I.VarGUID( "Mutant_Tank\\Grate Script Guid", m_GrateScriptGuid ) )
        return TRUE;
    
    // Not found
    return FALSE;
}

//=============================================================================

xbool mutant_tank::GetWithinGrateHoppointRadius( void )
{
    // are withing the radius of the hoppoint?
    object *grateHoppoint = g_ObjMgr.GetObjectByGuid(m_GrateHoppoint);
    if( grateHoppoint )
    {
        if( GetToTarget(m_GrateHoppoint).LengthSquared() <= m_GrateHoppointRadius * m_GrateHoppointRadius )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================

xbool mutant_tank::GetWithinPerchHoppointRadius( void )
{
    // are withing the radius of the hoppoint?
    object *perchHoppoint = g_ObjMgr.GetObjectByGuid(m_PerchHoppoint);
    if( perchHoppoint )
    {
        if( GetToTarget(m_PerchHoppoint).LengthSquared() <= m_PerchHoppointRadius * m_PerchHoppointRadius )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================

void mutant_tank::OnInit( void )
{
    // Call base class
    character::OnInit() ;
}

//=============================================================================

void mutant_tank::AdvanceLoco( f32 DeltaTime )
{
    STAT_LOGGER( temp, k_stats_AI_Advance );
   
    // Call base class
    character::AdvanceLoco( DeltaTime );
       
    // Override position/yaw if jumping
    UpdateJump();
           
    // Let the tank boss sounds to work!
    m_AllowDialog = TRUE;   
       
    // Make sure all particle effects are created
    InitFx();

    // Update pain timers
    m_PainLightTimer += DeltaTime;
    m_PainHardTimer  += DeltaTime;

    // Lookup stage
    ASSERT( m_Attack.m_CurrentStage >= 0 );
    ASSERT( m_Attack.m_CurrentStage < m_Attack.m_nStages );
    mutanttank_attack_state::stage& Stage = m_Attack.m_Stages[ m_Attack.m_CurrentStage ];

    // Update move style mixing
    switch( m_Loco.GetMoveStyle() )
    {
        // Turn off move style mixing
        default:
            m_Loco.SetBlendMoveStyle( loco::MOVE_STYLE_NULL );
            break;
            
        // Turn on move style mixing
        case loco::MOVE_STYLE_WALK:
        case loco::MOVE_STYLE_RUN:
        case loco::MOVE_STYLE_RUNAIM:
            m_Loco.SetMoveStyle     ( loco::MOVE_STYLE_WALK );
            m_Loco.SetBlendMoveStyle( loco::MOVE_STYLE_RUN );
            m_Loco.SetBlendMoveStyleAmount( Stage.m_MoveSpeed );

            // Debug override mix speed?    
            #ifndef X_RETAIL
                if( MUTANT_TANK_MOVE_SPEED != -1 )
                    m_Loco.SetBlendMoveStyleAmount( MUTANT_TANK_MOVE_SPEED );
            #endif
            break;
        
        // Turn on move style mixing
        case loco::MOVE_STYLE_CHARGE:
            m_Loco.SetMoveStyle     ( loco::MOVE_STYLE_CHARGE );
            m_Loco.SetBlendMoveStyle( loco::MOVE_STYLE_CHARGE_FAST );
            m_Loco.SetBlendMoveStyleAmount( m_Attack.m_ChargeSpeed );
            
            // Debug override mix speed?    
            #ifndef X_RETAIL
                if( MUTANT_TANK_CHARGE_SPEED != -1 )
                    m_Loco.SetBlendMoveStyleAmount( MUTANT_TANK_CHARGE_SPEED );
            #endif
            break;
    }
    
    // Update spores and effects
    UpdateSporesAndFx( DeltaTime );
    
    // Update parasite shield    
    UpdateParasiteShield( DeltaTime );
}

//=============================================================================
// SPORE FUNCTIONS
//=============================================================================

s32 mutant_tank::GetSporeCount( void ) const
{
    s32 nSpores = 0;
    
    // Loop over all spores
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Lookup spore
        if( g_ObjMgr.GetObjectByGuid( m_Spores[ i ].m_Guid ) )
            nSpores++;
    }
    
    return nSpores;
}

//=============================================================================

super_destructible_obj* mutant_tank::GetSporeObject( s32 iSpore )
{
    ASSERT( iSpore >= 0 );
    ASSERT( iSpore < SPORE_COUNT );

    object_ptr<super_destructible_obj> pSpore( m_Spores[ iSpore ].m_Guid );
    return pSpore.m_pObject;
}

//=============================================================================

super_destructible_obj* mutant_tank::GetRandomSporeObject( void )
{
    // Start at random index
    s32 Index = x_irand( 0, SPORE_COUNT - 1 );
    
    // Loop over all spores
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Lookup spore
        super_destructible_obj* pSpore = (super_destructible_obj*)g_ObjMgr.GetObjectByGuid( m_Spores[ Index ].m_Guid );
        if( pSpore )
            return pSpore;
            
        // Next
        if( ++Index == SPORE_COUNT )
            Index = 0;
    }

    // None found
    return NULL;
}

//=============================================================================

vector3 mutant_tank::GetRandomSporePosition( void )
{
    // Lookup random spore?
    object* pSpore = GetRandomSporeObject();
    if( pSpore )
        return pSpore->GetPosition();

    // Return top of theta
    vector3 Position = GetPosition();
    Position.GetY() += GetCollisionHeight();
    return Position;
}

//=============================================================================

f32 mutant_tank::GetSporeHealth( s32 iSpore ) const
{
    ASSERT( iSpore >= 0 );
    ASSERT( iSpore < SPORE_COUNT );

    object* pSpore = g_ObjMgr.GetObjectByGuid( m_Spores[ iSpore ].m_Guid );
    if( pSpore )
    {
        return pSpore->GetHealth();
    }
    else
    {
        return 0.0f;
    }
}

//=============================================================================

f32 mutant_tank::GetAllSporesHealth( void ) const
{
    // Loop through all spores and total up health
    f32 Health = 0.0f;
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        object* pSpore = g_ObjMgr.GetObjectByGuid( m_Spores[ i ].m_Guid );
        if( pSpore )
            Health += pSpore->GetHealth();
    }
            
    return Health;
}

//=============================================================================

s32 mutant_tank::GetSporeIndex( guid SporeGuid ) const
{
    // Loop through all spores
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Found?
        if( m_Spores[ i ].m_Guid == SporeGuid )
            return i;
    }
    
    // Not found
    return -1;
}

//=============================================================================

s32 mutant_tank::GetClosestSpore( const vector3& Position, 
                                        f32      MaxDistSqr,
                                        f32      MinHealth )
{
    // Init closest dist
    s32 iSpore       = -1;
    f32 SporeDistSqr = F32_MAX;

    // Loop over all spores
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Lookup spore
        super_destructible_obj* pSpore = GetSporeObject( i ) ;
        if( pSpore )
        {
            // Lookup spore info
            vector3 Delta   = pSpore->GetPosition() - Position;
            f32     DistSqr = Delta.LengthSquared();
            f32     Health  = pSpore->GetHealth();
            
            // Too far away?
            if( DistSqr > MaxDistSqr )
                continue;
                
            // Not enough health?
            if( Health <= MinHealth )
                continue;
                
            // Further away than current closest?
            if( DistSqr >= SporeDistSqr )                
                continue;
                
            // Record
            SporeDistSqr  = DistSqr;
            iSpore        = i;
        }
    }
    
    return iSpore;
}

//=============================================================================

s32 mutant_tank::GetSporeWithHealth( s32 iHintSpore )
{
    // Use hint spore if it's got health
    if( iHintSpore != -1 )
    {
        // Does spore object have health - if so return it
        object* pSpore = GetSporeObject( iHintSpore );
        if( ( pSpore ) && ( pSpore->GetHealth() > 0.0f ) )
            return iHintSpore;
    }
        
    // Choose a random spore
    s32 iSpore = x_irand( 0, SPORE_COUNT-1 );

    // Starting with a random or the hit spore, apply pain to any spore that has health
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Does this spore have health?
        object* pSpore = GetSporeObject( iSpore );
        if( ( pSpore ) && ( pSpore->GetHealth() > 0.0f ) )
            return iSpore;

        // Check next spore
        if( ++iSpore == SPORE_COUNT )
            iSpore = 0;
    }
    
    // Not found
    return -1;
}

//=============================================================================

xbool mutant_tank::ApplyPainToSpore( s32 iSpore, const pain& Pain )
{
    // Invalid spore?
    if( ( iSpore <= -1 ) || ( iSpore >= SPORE_COUNT ) )
        return FALSE;
    
    // Lookup spore object
    spore&  Spore  = m_Spores[ iSpore ];
    object* pSpore = g_ObjMgr.GetObjectByGuid( Spore.m_Guid );
    if( !pSpore )
        return FALSE;
        
    // Create pain that will affect spore
    pain SporePain = Pain;
    SporePain.SetDirectHitGuid( pSpore->GetGuid() );
    
    // Apply pain
    pSpore->OnPain( SporePain );
    
    // Flash the spore
    Spore.m_FlashTimer = 0.5f;
    
    // Generate spore hit particle
    RestartFx( Spore.m_HitFx );
    
    return TRUE;
}

//=============================================================================

void mutant_tank::SetRegenerateSpores( xbool bRegenerate )
{
    m_bRegenerateSpores = bRegenerate;
}



//=============================================================================

/*mutant_tank::stargate_hooks& mutant_tank::GetGrateHook( s32 iHook )
{
    ASSERT( iHook >= 0 );
    ASSERT( iHook < STARGATE_HOOK_COUNT );
    return m_GrateHooks[ iHook ];
}

//=============================================================================

mutant_tank::stargate_hooks& mutant_tank::GetPerchHook( s32 iHook )
{
    ASSERT( iHook >= 0 );
    ASSERT( iHook < STARGATE_HOOK_COUNT );
    return m_PerchHooks[ iHook ];
}*/

//=============================================================================
// Canister functions
//=============================================================================

s32 mutant_tank::GetCanisterCount( void ) const
{
    s32 nCanisters = 0;

    // Loop over all canister triggers
    for( s32 i = 0; i < CANISTER_COUNT; i++ )
    {
        // Has canister not been smashed yet?
        if( g_ObjMgr.GetObjectByGuid( m_Canisters[ i ].m_TriggerGuid ) )
            nCanisters++;
    }

    return nCanisters;
}

//=============================================================================

mutant_tank::canister& mutant_tank::GetCanister( s32 iCanister )
{
    ASSERT( iCanister >= 0 );
    ASSERT( iCanister < CANISTER_COUNT );
    return m_Canisters[ iCanister ];
}

//=============================================================================

s32 mutant_tank::GetClosestCanister( const vector3& Position, 
                                           f32      MinDistSqr,
                                           f32      MaxDistSqr ) const
{
    // Clear best
    s32 iBestCanister = -1;
    f32 BestDistSqr   = F32_MAX; 
    
    // Loop over all canisters
    for( s32 i = 0; i < CANISTER_COUNT; i++ )
    {
        // Lookup canister
        const canister& Canister = m_Canisters[i];
        
        // Skip if canister has already been triggered
        if( Canister.m_TriggerGuid == 0 )
            continue;
            
        // Lookup canister center object
        object* pCenterMarker = g_ObjMgr.GetObjectByGuid( Canister.m_CenterGuid );
        if( !pCenterMarker )
            continue;
            
        // Lookup canister center position            
        vector3 Center = pCenterMarker->GetPosition();
            
        // Get XZ distance squared from canister
        f32 DistSqr = x_sqr( Center.GetX() - Position.GetX() ) + x_sqr( Center.GetZ() - Position.GetZ() );
        
        // Skip if outside of distance limits
        if( ( DistSqr < MinDistSqr ) || ( DistSqr > MaxDistSqr ) )
            continue;
            
        // Record if the closest so far
        if( DistSqr < BestDistSqr )            
        {
            BestDistSqr   = DistSqr;
            iBestCanister = i;
        }
    }
            
    return iBestCanister;
}

//=============================================================================

/*s32 mutant_tank::GetClosestPerchHook( const vector3& Position, 
                                            f32      MinDistSqr,
                                            f32      MaxDistSqr ) const
{
    // Clear best
    s32 iBestPerch = -1;
    f32 BestDistSqr   = F32_MAX; 
    
    // Loop over all perchs
    for( s32 i = 0; i < STARGATE_HOOK_COUNT; i++ )
    {
        // Lookup perch center object
        object* pPerchMarker = g_ObjMgr.GetObjectByGuid( m_PerchHooks[i].m_AttachGuid );
        if( !pPerchMarker )
            continue;

        // Lookup perch center position            
        vector3 Center = pPerchMarker->GetPosition();

        // Get XZ distance squared from perch
        f32 DistSqr = x_sqr( Center.GetX() - Position.GetX() ) + x_sqr( Center.GetZ() - Position.GetZ() );

        // Skip if outside of distance limits
        if( ( DistSqr < MinDistSqr ) || ( DistSqr > MaxDistSqr ) )
            continue;

        // Record if the closest so far
        if( DistSqr < BestDistSqr )            
        {
            BestDistSqr   = DistSqr;
            iBestPerch = i;
        }
    }

    return iBestPerch;
}

//=============================================================================

s32 mutant_tank::GetClosestGrateHook( const vector3& Position, 
                                     f32      MinDistSqr,
                                     f32      MaxDistSqr ) const
{
    // Clear best
    s32 iBestGrate = -1;
    f32 BestDistSqr   = F32_MAX; 

    // Loop over all grates
    for( s32 i = 0; i < STARGATE_HOOK_COUNT; i++ )
    {
        // Lookup grate center object
        object* pGrateMarker = g_ObjMgr.GetObjectByGuid( m_GrateHooks[i].m_AttachGuid );
        if( !pGrateMarker )
            continue;

        // Lookup grate center position            
        vector3 Center = pGrateMarker->GetPosition();

        // Get XZ distance squared from grate
        f32 DistSqr = x_sqr( Center.GetX() - Position.GetX() ) + x_sqr( Center.GetZ() - Position.GetZ() );

        // Skip if outside of distance limits
        if( ( DistSqr < MinDistSqr ) || ( DistSqr > MaxDistSqr ) )
            continue;

        // Record if the closest so far
        if( DistSqr < BestDistSqr )            
        {
            BestDistSqr   = DistSqr;
            iBestGrate = i;
        }
    }

    return iBestGrate;
}*/

//=============================================================================
// Jump functions
//=============================================================================

void mutant_tank::BeginJump(       loco::anim_type AnimType, 
                             const vector3&        EndPos,
                                   radian          EndYaw,
                                   xbool           bInterpYaw,
                                   xbool           bInterpVert )

{
    // Keep the info
    m_iJumpAnim       = m_Loco.GetAnimIndex( AnimType );
    m_JumpStartPos    = GetPosition();
    m_JumpStartYaw    = GetYaw();
    m_JumpEndPos      = EndPos;
    m_JumpEndYaw      = EndYaw;
    m_bJumpInterpYaw  = bInterpYaw;
    m_bJumpInterpVert = bInterpVert;
}

//=============================================================================

void mutant_tank::BeginJump( loco::anim_type AnimType, 
                             guid            DestMarkerGuid,
                             xbool           bInterpYaw,
                             xbool           bInterpVert )
{
    // Lookup dest info
    object* pDestMarker = g_ObjMgr.GetObjectByGuid( DestMarkerGuid );
    ASSERT( pDestMarker );
    vector3 JumpEndPos = pDestMarker->GetPosition();
    radian  JumpEndYaw = pDestMarker->GetL2W().GetRotation().Yaw;
    
    BeginJump( AnimType, JumpEndPos, JumpEndYaw, bInterpYaw, bInterpVert );
}

//=============================================================================

xbool mutant_tank::UpdateJump( void )
{
    // Nothing to do?
    if( m_iJumpAnim != m_Loco.m_Player.GetCurrAnim().GetAnimIndex() )
    {
        return TRUE;
    }
            
    // Get anim group
    const anim_group* pAnimGroup = m_Loco.GetAnimGroupHandle().GetPointer();
    ASSERT( pAnimGroup );

    // Get anim info
    const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( m_iJumpAnim );
    
    // Lookup motion prop
    s32 iMotionProp = AnimInfo.GetPropChannel( "MotionProp" );
    ASSERT( iMotionProp != -1 );
   
    // Lookup start and key key of motion prop
    anim_key StartKey, EndKey, CurrKey;
    AnimInfo.GetPropRawKey   ( iMotionProp, 0, StartKey );
    AnimInfo.GetPropRawKey   ( iMotionProp, AnimInfo.GetNFrames()-1, EndKey );
    AnimInfo.GetPropInterpKey( iMotionProp, m_Loco.m_Player.GetCurrAnim().GetFrame(), CurrKey );

    // Compute animation total translation and rotation
    vector3 DeltaPos = EndKey.Translation - StartKey.Translation;

    // Motion must move on the horizontally and vertically for ratio code to work
    ASSERT( ( x_abs( DeltaPos.GetX() ) > 1.0f ) || ( x_abs( DeltaPos.GetZ() ) > 1.0f ) );
    ASSERT( x_abs( DeltaPos.GetY() ) > 1.0f );
    
    // Compute motion ratio of current key
    f32 HorizT = 0.0f;
    if( x_abs( DeltaPos.GetX() ) > x_abs( DeltaPos.GetZ() ) )
        HorizT = x_parametric( CurrKey.Translation.GetX(), StartKey.Translation.GetX(), EndKey.Translation.GetX(), FALSE );
    else        
        HorizT = x_parametric( CurrKey.Translation.GetZ(), StartKey.Translation.GetZ(), EndKey.Translation.GetZ(), FALSE );
    
    f32 VertT  = x_parametric( CurrKey.Translation.GetY(), StartKey.Translation.GetY(), EndKey.Translation.GetY(), FALSE );
    
    // Interpolate horizontally
    vector3 Pos;
    Pos.GetX() = m_JumpStartPos.GetX() + ( HorizT * ( m_JumpEndPos.GetX() - m_JumpStartPos.GetX() ) );
    Pos.GetZ() = m_JumpStartPos.GetZ() + ( HorizT * ( m_JumpEndPos.GetZ() - m_JumpStartPos.GetZ() ) );
    
    // Interpolate or use exisiting vertical?
    if( m_bJumpInterpVert )
        Pos.GetY() = m_JumpStartPos.GetY() + ( VertT  * ( m_JumpEndPos.GetY() - m_JumpStartPos.GetY() ) );
    else
        Pos.GetY() = m_Loco.GetPosition().GetY();
    
    m_Loco.SetPosition( Pos );

    // Interpolate to compute current jump yaw (yaw ratio is taken from the vertical movement)?
    if( m_bJumpInterpYaw )
    {
        radian Yaw = m_JumpStartYaw + ( HorizT * x_MinAngleDiff( m_JumpEndYaw, m_JumpStartYaw ) );
        m_Loco.SetYaw( Yaw );
    }
        
    // Complete?
    return ( m_Loco.m_Player.IsAtEnd() );
}

//=============================================================================
// Parasite shield functions
//=============================================================================

void mutant_tank::InitParasiteShield( void )
{
    // Already initialized?
    if( m_nParasites )
        return;
    
    // Lookup anim info of current anim
    const anim_info& AnimInfo = m_Loco.m_Player.GetAnimInfo();
        
    // Loop through all the events in the anim searching for particle events
    for( s32 iEvent = 0; iEvent < AnimInfo.GetNEvents(); iEvent++ )
    {
        // Lookup event
        const anim_event& Event = AnimInfo.GetEvent( iEvent );

        // Skip if not a particle event
        const char* pEventType = Event.GetType();
        if( x_stricmp( pEventType, "Particle" ) != 0 )
            continue;
            
        // Skip if not attached to a bone
        s32 iBone = Event.GetInt( anim_event::INT_IDX_BONE );
        if( iBone == -1 )
            continue;

        // Lookup event info            
        const char* pFxName = Event.GetString( anim_event::STRING_IDX_HOTPOINT_TYPE );
        vector3     BonePos = Event.GetPoint ( anim_event::POINT_IDX_OFFSET );
        vector3     BoneRot = Event.GetPoint ( anim_event::POINT_IDX_ROTATION );
            
        // Allocate next available parasite
        parasite& Parasite = m_Parasites[ m_nParasites++ ];
        ASSERT( m_nParasites <= PARASITE_COUNT );

        // Setup parasite
        Parasite.m_iBone   = iBone;
        Parasite.m_BonePos = BonePos;
        Parasite.m_BoneRot.Set( BoneRot.GetX(), BoneRot.GetY(), BoneRot.GetZ() );

        // Search for particle event emitter that was created by the event
        for( slot_id Slot = g_ObjMgr.GetFirst( object::TYPE_PARTICLE_EVENT_EMITTER ); Slot != SLOT_NULL; Slot = g_ObjMgr.GetNext( Slot ) )
        {
            // Lookup emitter
            particle_event_emitter* pEmitter = (particle_event_emitter*)g_ObjMgr.GetObjectBySlot( Slot );
            ASSERT( pEmitter );
        
            // Skip if not attached to tank
            if( pEmitter->GetParentGuid() != GetGuid() )
                continue;
                
            // Skip if not this event ID
            if( pEmitter->GetEventID() != iEvent )
                continue;
                
            // Skip if not this particle
            if( x_stricmp( pEmitter->GetFxName(), pFxName ) != 0 )
                continue;
                
            // Emitter was found so record the info!
            Parasite.m_WorldPos     = pEmitter->GetPosition();
            Parasite.m_EmitterGuid  = pEmitter->GetGuid();
            Parasite.m_ParticleGuid = pEmitter->GetParticleGuid();
            Parasite.m_bVisible     = TRUE;
            Parasite.m_Alpha        = 1.0f;
            break;
        }  
        
        // Make sure the event was found!
        ASSERT( Parasite.m_EmitterGuid );              
    }                
}

//=============================================================================

void mutant_tank::UpdateParasiteShield( f32 DeltaTime )
{   
    // Reset regeneration site
    m_ParasiteRegenSite.Zero();
    m_ParasiteShieldPercent = -1.0f;

    // No parasites?
    if( m_nParasites == 0 )
        return;
        
    // Update all parasites and count the # of shield
    s32 nVisible = 0;
    s32 nRegenCount = 0;
    for( s32 i = 0; i < m_nParasites; i++ )
    {
        // Lookup parasite
        parasite& Parasite = m_Parasites[i];

        // Compute bone space transform of parasite
        matrix4 BoneTransform( vector3( 1.0f, 1.0f, 1.0f ),
                               Parasite.m_BoneRot,
                               Parasite.m_BonePos );
                               
        // Compute world space transform of parasite
        const matrix4& BoneL2W = m_Loco.m_Player.GetBoneL2W( Parasite.m_iBone );
        matrix4 L2W = BoneL2W * BoneTransform;

        // Update emitter
        object_ptr<particle_event_emitter> pEmitter( Parasite.m_EmitterGuid );
        if( pEmitter )
            pEmitter->OnTransform( L2W );
            
        // Update parasite
        Parasite.m_WorldPos = L2W.GetTranslation();

        // If dead, fade out the shield        
        if( GetHealth() <= 0.0f )        
            Parasite.m_bVisible = FALSE;
            
        // Update alpha
        if( Parasite.m_bVisible )
        {
            // Fade in
            Parasite.m_Alpha += MUTANT_TANK_PARASITE_FADE_IN_TIME * DeltaTime;
            if( Parasite.m_Alpha > 1.0f )
                Parasite.m_Alpha = 1.0f;
                
            // Update stats
            nVisible++;                    
        }
        else
        {
            // Fade out
            Parasite.m_Alpha -= MUTANT_TANK_PARASITE_FADE_OUT_TIME * DeltaTime;
            if( Parasite.m_Alpha < 0.0f )
                Parasite.m_Alpha = 0.0f;
                
            // Add to re-gen site
            m_ParasiteRegenSite += Parasite.m_WorldPos;
            nRegenCount++;
        }
        
        // Update scale based on alpha
        f32 Scale = x_max( 0.1f, Parasite.m_Alpha );
        
        // Update particle fx
        object_ptr<particle_emitter> pParticle( Parasite.m_ParticleGuid );
        if( pParticle )
        {
            pParticle->SetColor( xcolor( 255, 255, 255, (u8)( Parasite.m_Alpha * 255.0f ) ) );
            pParticle->SetScale( Scale );
        }            
    }
    
    // Compute shield percent
    m_ParasiteShieldPercent = (100.0f * nVisible ) / (f32)m_nParasites;
    
    // Compute regenerate site
    if( nRegenCount )
    {
        // Use average
        m_ParasiteRegenSite /= (f32)nRegenCount;
    }
    else
    {
        // Use theta center
        m_ParasiteRegenSite = GetBBox().GetCenter();
    }
    
/*    // Fire at player projectiles?
    if( m_ParasiteShieldPercent > 0.0f )
    {
        // Any player projectiles to hit?
        guid PlayerProjGuid = ScanForPlayerProjectile();
        if( PlayerProjGuid )
        {
            // Fire contagion at the players projectile
            CreateContagion( GetRandomSporePosition(),          // Position
                             radian3( R_0, R_0, R_0 ),          // Rotation
                             vector3( 0.0f , 100.0f, 0.0f ),    // Velocity
                             "THETA_PARASITE",                  // pPainHandle,
                             CONTAGION_ATTACK_PROJECTILE_TIME,  // AttackTime,
                             PlayerProjGuid );                  // TargetGuid )
        }
    }*/
}

//=============================================================================

guid mutant_tank::ScanForPlayerProjectile( void )
{
    // List of player projectile types that can be intercepted
    static object::type s_ProjTypes[] =
    {
        object::TYPE_GRAV_CHARGE_PROJECTILE,        // net_proj
        object::TYPE_GRENADE_PROJECTILE,            // net_proj
        object::TYPE_JUMPING_BEAN_PROJECTILE,       // net_proj
        object::TYPE_ENERGY_PROJECTILE,             // net_proj
        object::TYPE_HOMING_PROJECTILE,             // base_projectile
        object::TYPE_MESONSEEKER_PROJECTILE,        // net_proj                                   
        object::TYPE_MUTANT_CONTAGION_PROJECTILE,   // net_proj 
        object::TYPE_MUTANT_PARASITE_PROJECTILE,    // base_projectile    
    };

    // Lookup the player
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if( !pPlayer )
        return 0;
    guid PlayerGuid = pPlayer->GetGuid();        
    guid ThetaGuid  = GetGuid();
    
    // Loop over all types
    for( s32 iType = 0; iType < ( sizeof(s_ProjTypes) / sizeof(s_ProjTypes[0]) ); iType++ )
    {                
        // Search for particle event emitter that was created by the event
        for( slot_id Slot = g_ObjMgr.GetFirst( s_ProjTypes[iType] ); Slot != SLOT_NULL; Slot = g_ObjMgr.GetNext( Slot ) )
        {
            // Lookup projectile info
            object* pProjectile = g_ObjMgr.GetObjectBySlot( Slot );
            ASSERT( pProjectile );
            guid ProjectileGuid = pProjectile->GetGuid();
            
            // Inherit from net_proj?
            if( pProjectile->IsKindOf( net_proj::GetRTTI() ) )
            {
                // Fired by the player?
                net_proj& NetProj = net_proj::GetSafeType( *pProjectile );
                if( NetProj.GetOriginGuid() == PlayerGuid )
                {
                    // Not yet targeted?
                    if( !mutant_contagion_projectile::IsTargeted( ThetaGuid, ProjectileGuid ) )
                        return ProjectileGuid;
                }                    
            }
            // Inherit from base_projectile?
            else if( pProjectile->IsKindOf( base_projectile::GetRTTI() ) )
            {
                // Fired by the player?
                base_projectile& BaseProj = base_projectile::GetSafeType( *pProjectile );
                if( BaseProj.GetOwnerID() == PlayerGuid )
                {
                    // Not yet targeted?
                    if( !mutant_contagion_projectile::IsTargeted( ThetaGuid, ProjectileGuid ) )
                        return ProjectileGuid;
                }                    
            }
            else
            {
                ASSERTS( 0, "Need to see SteveB and add new type here!" );
            }
        }
    }                    
    
    // None found
    return 0;
}

//=============================================================================

f32 mutant_tank::GetParasiteShieldPercent( void )
{
    return m_ParasiteShieldPercent;
}

//=============================================================================

struct parasite_sort
{
    mutant_tank::parasite*  m_pParasite;  // Pointer to parasite
    f32                     m_DistSqr;    // Sort value
};

//=============================================================================

static
s32 ParasiteSort( const void* pItem1, const void* pItem2 )
{
    parasite_sort* pParasiteSort1 = (parasite_sort*)pItem1;
    parasite_sort* pParasiteSort2 = (parasite_sort*)pItem2;

    if( pParasiteSort1->m_DistSqr > pParasiteSort2->m_DistSqr )
        return 1;
    else if( pParasiteSort1->m_DistSqr < pParasiteSort2->m_DistSqr )
        return -1;
    else
        return 0;
}

//=============================================================================

void mutant_tank::RegenParasiteShield( f32 Percent )
{
    s32 i;

    // No parasites?
    if( m_nParasites == 0 )
        return;

    // Setup parasite sort list
    parasite_sort SortList[ PARASITE_COUNT ];
    for( i = 0; i < m_nParasites; i++ )
    {
        parasite& Parasite = m_Parasites[i];
        SortList[i].m_pParasite = &Parasite;
        SortList[i].m_DistSqr   = ( Parasite.m_WorldPos - m_ParasiteRegenSite ).LengthSquared();
    }

    // Sort from closest to furthest away from regeneration site
    x_qsort( &SortList[0], m_nParasites, sizeof(parasite_sort), ParasiteSort );

    // Compute how many to regenerate
    s32 Count = 1 + (s32) ( ( Percent * m_nParasites ) / 100.0f );

    s32 regened = 0;
    // Now regenerate from closest to furthest away
    for( i = 0; ( i < m_nParasites ) && ( Count > 0 ) ; i++ )
    {
        // Bring back parasite if it's currently not present
        parasite& Parasite = *SortList[i].m_pParasite;
        if( !Parasite.m_bVisible )
        {
            // Bring parasite back
            Parasite.m_bVisible = TRUE;
            Count--; 
            regened++;
        }                             
    }    
    while( regened > 0 )
    {    
        parasite& Parasite = *SortList[0].m_pParasite;
        // Create contagion for every 16 parasites.
        CreateContagion( GetRandomSporePosition(),          // Position
            radian3( R_0, R_0, R_0 ),          // Rotation
            vector3( 0.0f , 100.0f, 0.0f ),    // Velocity
            "THETA_PARASITE",                  // pPainHandle,
            CONTAGION_REGEN_SHIELD_TIME,       // AttackTime,
            Parasite.m_EmitterGuid );          // TargetGuid )
        regened -= 16;
    }
    // Update spores
    for( s32 i = 0; i < SPORE_COUNT; i++ )
    {
        // Lookup spore object (super destructible)
        super_destructible_obj* pSpore = GetSporeObject( i );
        if( pSpore )
        {
            // Apply pain directly to spore to destroy it
            pain Pain;
            pain_handle PainHandle = GetPainHandleForGenericPain( TYPE_LETHAL );
            Pain.Setup( PainHandle, GetGuid(), pSpore->GetPosition() );
            Pain.SetDirectHitGuid( pSpore->GetGuid() );
            pSpore->OnPain( Pain );
        }
    }
}

//=============================================================================

void mutant_tank::CreateParasiteShieldContagion( void )
{
    // Lookup the shield percent to convert into contagion
    f32 Percent = GetTweakF32( "THETA_ContagionShieldPercent", 25.0f );

    // Compute how many to generate
    s32 Count = 1 + (s32)( ( Percent * m_nParasites ) / 100.0f );
    // Loop over all parasites
    for( s32 i = 0; ( i < m_nParasites ) && ( Count > 0 ) ; i++ )
    {
        // Lookup parasite
        parasite& Parasite = m_Parasites[i];
        
        // Skip if not visible
        if( !Parasite.m_bVisible )
            continue;
    
        // Fade out
        Parasite.m_bVisible = FALSE;
                         
        // Update count
        Count--;    
    }

    // Fire contagion at the target guid (the player)
    CreateContagion( GetRandomSporePosition(),          // Position
        radian3( R_0, R_0, R_0 ),          // Rotation
        vector3( 0.0f , 100.0f, 0.0f ),    // Velocity
        "THETA_PARASITE",                  // pPainHandle,
        CONTAGION_REGEN_SHIELD_TIME,       // AttackTime,
        GetTargetGuid() );                 // TargetGuid )
}

//=============================================================================

void mutant_tank::CreateContagion( const vector3& Position, 
                                   const radian3& Rotation,
                                   const vector3& Velocity,
                                   const char*    pPainHandle,
                                         f32      AttackTime,
                                         guid     TargetGuid )
{
    // Create the contagion projectile
    guid ProjectileID = CREATE_NET_OBJECT( mutant_contagion_projectile::GetObjectType(), netobj::TYPE_CONTAGION );
    mutant_contagion_projectile* pProjectile = (mutant_contagion_projectile*) g_ObjMgr.GetObjectByGuid( ProjectileID );
    ASSERT( pProjectile );
    if( !pProjectile )
        return;

    // Setup pain handle
    pain_handle PainHandle( pPainHandle );

    // Setup projectile
    pProjectile->Setup( GetGuid(),      // OriginGuid
                        net_GetSlot(),  // OriginNetSlot
                        Position,       // Position
                        Rotation,       // Orientation
                        Velocity,       // Velocity
                        GetZone1(),     // Zone1
                        GetZone2(),     // Zone2
                        PainHandle );   // hPain

    // Go for target
    pProjectile->TriggerFromTheta( AttackTime, TargetGuid );
}

//=============================================================================
