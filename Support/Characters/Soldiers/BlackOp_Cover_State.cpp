#include "BlackOp_Cover_State.hpp"
#include "..\Character.hpp"
#include "navigation\coverNode.hpp"
#include "objects\NewWeapon.hpp"
#include "audiomgr\audiomgr.hpp"
#include "soldier.hpp"

//=========================================================================
// constants
//=========================================================================

const f32 k_MinTimeInIdle               = 0.0f;
const f32 k_MinTimeAligning             = 1.0f;
const f32 k_MinTimeBetweenCoverReloads  = 3.0f;
const f32 k_MinTimeBetweenMelee         = 2.0f;
const f32 k_MinTimeBetweenFullAuto      = 8.0f;
const f32 k_MaxTimeFullAuto             = 3.0f;
const f32 k_MinTimeBetweenRollin        = 3.0f;

//=========================================================================
// GRAY COVER STATE
//=========================================================================

blackOp_cover_state::blackOp_cover_state( character& ourCharacter, character_state::states State ) :
    character_cover_state(ourCharacter, State)
{
    m_AutofireRequestSent = FALSE;
    m_WantsToRollin = FALSE;
    m_MinTimeRolledin = 0.0f;
}

//=========================================================================

blackOp_cover_state::~blackOp_cover_state()
{
}

//=========================================================================

void blackOp_cover_state::OnEnter()
{
    m_AutofireRequestSent = FALSE;
    m_WantsToRollin = FALSE;
    m_MinTimeRolledin = 0.0f;
    character_cover_state::OnEnter();
}

//=========================================================================

xbool blackOp_cover_state::GetAnimNameFromPhase( s32 nextPhase, char* animName )
{
    xbool bFoundName = FALSE;
    char weaponPrefix[4];

    weaponPrefix[0] = 0;
    
    new_weapon* pWeapon = m_CharacterBase.GetCurrentWeaponPtr();
    if( pWeapon )
    {
        x_strcpy( weaponPrefix,new_weapon::GetWeaponPrefixFromInvType2( pWeapon->GetInvenItem()) );
    }    

    switch( nextPhase )
    {
    case PHASE_BO_COVER_ENTER_COVER:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_ENTER");
        }
        else
        {
            x_strcpy(animName,"COVER_ENTER");
        }
        break;
    case PHASE_BO_COVER_IDLE:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_IDLE");
        }
        else
        {
            x_strcpy(animName,"COVER_IDLE");
        }
        break;
    case PHASE_BO_COVER_ROLL_OUT:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_ROLL_OUT");
        }
        else
        {
            x_strcpy(animName,"COVER_ROLL_OUT");
        }
        break;
    case PHASE_BO_COVER_FULL_AUTO:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_OUT_FULL_AUTO");
        }
        else
        {
            x_strcpy(animName,"COVER_OUT_FULL_AUTO");
        }
        break;
    case PHASE_BO_COVER_OUT_IDLE:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_OUT_IDLE");
        }
        else
        {
            x_strcpy(animName,"COVER_OUT_IDLE");
        }
        break;
    case PHASE_BO_COVER_OUT_GRENADE:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_OUT_GRENADE");
        }
        else
        {
            x_strcpy(animName,"COVER_OUT_GRENADE");
        }
        break;
    case PHASE_BO_COVER_OUT_SCAN:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_OUT_SCAN");
        }
        else
        {
            x_strcpy(animName,"COVER_OUT_SCAN");
        }
        break;
    case PHASE_BO_COVER_ROLL_IN:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_ROLL_IN");
        }
        else
        {
            x_strcpy(animName,"COVER_ROLL_IN");
        }
        break;
    case PHASE_BO_COVER_EXIT_COVER:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_EXIT");
        }
        else
        {
            x_strcpy(animName,"COVER_EXIT");
        }
        break;
    };  
    return bFoundName;
}

//=========================================================================

s32 blackOp_cover_state::GetNextPhaseRolledOut()
{
    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();   
    radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
    radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
    radian m_DiffToTargetYaw = x_abs(x_MinAngleDiff(locoYaw, TargetYaw));

    // if I desired to change states, or I'm not at my node, 
    // or my node is invalid and I have a target, then exit
//    soldier &soldierObject = soldier::GetSafeType( *(g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetGuid())) );
//    object* pOurCorpse = g_ObjMgr.GetObjectByGuid( soldierObject.GetAllyCorpseGuid() );

    if( m_DesiredState )
    {
        // just pop us out of cover...
        SetInCover(FALSE);
        return PHASE_BO_COVER_STAND_AND_SHOOT;                
    }
    else if( m_CurrentCover != m_CharacterBase.GetCurrentCover() )
    {
        // just pop us out of cover...
        SetInCover(FALSE);
        return PHASE_BO_COVER_STAND_AND_SHOOT;                
    }
    else if( m_CharacterBase.GetTargetGuid() &&
        distToTargetSqr <= (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) &&
        m_TimeSinceLastMelee >= k_MinTimeBetweenMelee )
    {
        if( m_DiffToTargetYaw > R_10 )
        {            
            return PHASE_BO_COVER_ALIGN_FOR_MELEE;
        }
        else
        {
            return PHASE_BO_COVER_MELEE;
        }
    }
    else if( !m_CharacterBase.GetCoverIsValid() && 
             m_CharacterBase.GetAwarenessLevel() >= character::AWARENESS_TARGET_LOST )
    {
        // just pop us out of cover...
        SetInCover(FALSE);
        return PHASE_BO_COVER_STAND_AND_SHOOT;
    }
    else if( m_CharacterBase.IsReloading() || 
             m_WantsToRollin )
    {
        m_WantsToRollin = FALSE;
        if( HasAnimForCoverPhase(PHASE_BO_COVER_ROLL_IN) )
        {
            return PHASE_BO_COVER_ROLL_IN;
        }
        else
        {
            return PHASE_BO_COVER_IDLE;
        }
    }
    else if( m_AutofireRequested && 
             HasAnimForCoverPhase(PHASE_BO_COVER_FULL_AUTO) )
    {
        return PHASE_BO_COVER_FULL_AUTO;
    }
    else if ( m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET )
    {
        if( HasAnimForCoverPhase(PHASE_BO_COVER_OUT_SCAN) )
        {                
            return PHASE_BO_COVER_OUT_SCAN;
        }
        else
        {
            return PHASE_BO_COVER_OUT_IDLE;
        }
    }
    else if( m_DelayTillNextAction > 0.0f )
    {
        return PHASE_BO_COVER_OUT_IDLE;
    }
    else
    {                
        s32 usedShootWeight = m_ShootWeight;
        s32 usedGrenadeWeight = m_GrenadeWeight;

        // check for cover node overrides
        object *coverObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetCurrentCover() );
        if( coverObject && coverObject->IsKindOf(cover_node::GetRTTI()) )
        {
            cover_node& coverNode = cover_node::GetSafeType( *coverObject );
            if( coverNode.GetShootWeight() >= 0.0f )
            {
                usedShootWeight = coverNode.GetShootWeight();
            }
            if( coverNode.GetGrenadeWeight() >= 0.0f )
            {
                usedGrenadeWeight = coverNode.GetGrenadeWeight();
            }
        }

        // can we shoot?
        if( m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET ||
            !m_CharacterBase.CanShootAtTarget() )
        {
            usedShootWeight = 0;
        }

        // can we throw a grenade?
        if( !HasAnimForCoverPhase(PHASE_BO_COVER_OUT_GRENADE) || 
            !m_HasClearGrenadeThrow || 
            m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET )
        {
            usedGrenadeWeight = 0;
        }

        s32 totalWeight = usedShootWeight + usedGrenadeWeight;
        if( totalWeight <= 0 )
        { 
            return PHASE_BO_COVER_OUT_IDLE;
        }

        s32 chosenWeight = x_irand(1,totalWeight);
        chosenWeight -= usedGrenadeWeight;
        if( chosenWeight <= 0 )
        {
            m_DelayTillNextAction = x_frand(m_UsedActionDelayMin,m_UsedActionDelayMax);
            return PHASE_BO_COVER_OUT_GRENADE;
        }
        else
        {
            m_DelayTillNextAction = x_frand(m_UsedActionDelayMin,m_UsedActionDelayMax);
            m_CharacterBase.SetWantsToFirePrimary(TRUE);
        }
    }
    return PHASE_BO_COVER_OUT_IDLE;
}

//=========================================================================

xbool blackOp_cover_state::OnPain( const pain& Pain )
{        
    if( ( m_CurrentPhase == PHASE_BO_COVER_OUT_IDLE || 
          m_CurrentPhase == PHASE_BO_COVER_OUT_SCAN ) &&
        m_TimeInPhase >= k_MinTimeBetweenRollin )
    {
        m_WantsToRollin = TRUE;
        m_MinTimeRolledin = x_frand(0.5f,2.0f);
    }
    // if we take damage while rolled in, then we might as well come out.
    else if ( m_CurrentPhase == PHASE_BO_COVER_IDLE )
    {
        m_MinTimeRolledin = 0.0f;
    }
    // every so often, on pain we will automatically rethink.
    return character_cover_state::OnPain( Pain );
}

//=========================================================================

s32 blackOp_cover_state::GetNextPhaseRolledIn()
{
    // if we need to move to a new place, let's go.

    // if reloading stay down.
    if ( m_CurrentCover != m_CharacterBase.GetCurrentCover() )
    {
        if( HasAnimForCoverPhase(PHASE_BO_COVER_EXIT_COVER) )
        {
            return PHASE_BO_COVER_EXIT_COVER;
        }
        else
        {                
            // just pop us out of cover...
            SetInCover(FALSE);
            return PHASE_BO_COVER_STAND_AND_SHOOT;                
        }
    }
    else if( m_CharacterBase.IsReloading() )
    {
        return PHASE_BO_COVER_IDLE;
    }
    // if reloading or not time yet,
    // then 
    // if I desired to change states, or I'm not at my node, 
    // or my node is invalid and I have a target, then exit
    else if( m_DesiredState )
    {
        if( HasAnimForCoverPhase(PHASE_BO_COVER_EXIT_COVER) )
        {
            return PHASE_BO_COVER_EXIT_COVER;
        }
        else
        {                
            // just pop us out of cover...
            SetInCover(FALSE);
            return PHASE_BO_COVER_STAND_AND_SHOOT;                
        }
    }
    else if( !m_CharacterBase.GetCoverIsValid() && 
             m_CharacterBase.GetAwarenessLevel() >= character::AWARENESS_TARGET_LOST &&
             !m_CharacterBase.GetIsCoverSticky(m_CurrentCover) )
    {
        if( HasAnimForCoverPhase(PHASE_BO_COVER_EXIT_COVER) )
        {
            return PHASE_BO_COVER_EXIT_COVER;
        }
        else
        {                
            // just pop us out of cover...
            SetInCover(FALSE);
            return PHASE_BO_COVER_STAND_AND_SHOOT;                
        }
    }
    else if( m_AutofireRequested && 
            HasAnimForCoverPhase(PHASE_BO_COVER_FULL_AUTO) &&
            HasAnimForCoverPhase(PHASE_BO_COVER_OUT_IDLE) )
    {
        if( HasAnimForCoverPhase(PHASE_BO_COVER_ROLL_OUT) )
        {                    
            return PHASE_BO_COVER_ROLL_OUT;
        }
        else
        {
            SetRolledOut(TRUE);
            return PHASE_BO_COVER_FULL_AUTO;
        }
    }
    else if( HasAnimForCoverPhase(PHASE_BO_COVER_OUT_IDLE) &&
             m_MinTimeRolledin <= 0.0f )
    {
        if( HasAnimForCoverPhase(PHASE_BO_COVER_ROLL_OUT) )
        {                    
            return PHASE_BO_COVER_ROLL_OUT;
        }
        else
        {
            SetRolledOut(TRUE);
            return PHASE_BO_COVER_OUT_IDLE;
        }
    }
    return PHASE_BO_COVER_IDLE;
}

//=========================================================================

s32 blackOp_cover_state::GetNextPhaseOutOfCover()
{
    // I'not m currently in my cover, what to do...

    // first, should we just smackthe guy in face?
    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();   
    radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
    radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
    radian m_DiffToTargetYaw = x_abs(x_MinAngleDiff(locoYaw, TargetYaw));

    // if I desired to change states, or I'm not at my node, 
    // or my node is invalid and I have a target, then exit
    if( m_CharacterBase.GetTargetGuid() &&
        distToTargetSqr <= (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) &&
        m_TimeSinceLastMelee >= k_MinTimeBetweenMelee )
    {
        if( m_DiffToTargetYaw > R_10 )
        {            
            return PHASE_BO_COVER_ALIGN_FOR_MELEE;
        }
        else
        {
            return PHASE_BO_COVER_MELEE;
        }
    }
    // if my cover is valid || I'm not fighting, it's ok to get into cover...
    else if( m_CurrentCover != m_CharacterBase.GetCurrentCover() )
    {
        SetCurrentCover(0);
        return PHASE_BO_COVER_GOTO_COVER;
    }
    else if( m_CharacterBase.GetCoverIsValid() || 
             m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET )
    {
//        return GetFacePhase();                
        return PHASE_BO_COVER_GOTO_COVER;
    }
    return PHASE_BO_COVER_STAND_AND_SHOOT;
}

//=========================================================================

s32 blackOp_cover_state::GetFacePhase()
{
    // if we have an enter cover, we will blend roll in instead of facing initially.
    if( HasAnimForCoverPhase(PHASE_BO_COVER_ENTER_COVER) )
    {
        return PHASE_BO_COVER_ENTER_COVER;
    }
    else
    {
        return PHASE_BO_COVER_FACE_EXACT;
    }
}

//=========================================================================

xbool blackOp_cover_state::ProvideAutofire()
{   
    if( HasAnimForCoverPhase(PHASE_BO_COVER_FULL_AUTO) &&
        m_InCover &&
        m_CharacterBase.GetTargetGuid() &&
        m_TimeSinceLastFullAuto >= k_MinTimeBetweenFullAuto && 
        !m_CharacterBase.IsReloading() &&
        m_CharacterBase.GetWeaponItem() != INVEN_WEAPON_MESON_CANNON &&
        m_CharacterBase.GetWeaponItem() != INVEN_WEAPON_BBG &&
        m_CharacterBase.CanSeeTarget() )
    {
        m_AutofireRequested = TRUE;
        m_AutofireCanceled  = FALSE;
        m_DoAutofireDialog  = FALSE;
        return TRUE;
    }
    return FALSE;
}

//=========================================================================

xbool blackOp_cover_state::IgnoreFullBodyFlinches()
{
    if( m_CurrentPhase == PHASE_BO_COVER_ENTER_COVER || 
        m_CurrentPhase == PHASE_BO_COVER_EXIT_COVER )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=========================================================================

xbool blackOp_cover_state::UseRelativeMode( void )
{

    switch( m_CurrentPhase )
    {
        // If arriving at cover node, turn on relative mode and set the position
    case PHASE_BO_COVER_IDLE:
    case PHASE_BO_COVER_ROLL_OUT:
    case PHASE_BO_COVER_ROLL_IN:
        return TRUE;
        
        // If npc is moving to cover or doing a base phase, turn off relative mode
    default:

        // These "OUT" anims are at the origin so they can't use relative mode
    case PHASE_BO_COVER_OUT_IDLE:
    case PHASE_BO_COVER_OUT_SCAN:
    case PHASE_BO_COVER_OUT_GRENADE:
    case PHASE_BO_COVER_FULL_AUTO:
    case PHASE_BO_COVER_ALIGN_FOR_MELEE:
    case PHASE_BO_COVER_MELEE:            
    case PHASE_BO_COVER_GOTO_CORPSE:
    case PHASE_BO_COVER_DRAIN_CORPSE:
    case PHASE_BO_COVER_SUMMON_ALLIES:
    case PHASE_BO_COVER_STAND_AND_SHOOT:
    case PHASE_BO_COVER_REQUEST_ATTACK:
    case PHASE_BO_COVER_COVER_ME:

        // These phases are either going to or coming from cover so no relative mode
    case PHASE_BO_COVER_FACE_EXACT:
    case PHASE_BO_COVER_ENTER_COVER:
    case PHASE_BO_COVER_GOTO_COVER:
    case PHASE_BO_COVER_EXIT_COVER:
        return FALSE;
    }
}

//=========================================================================

s32 blackOp_cover_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();   
    s32 newPhase = PHASE_NONE;
    m_TimeSinceLastMelee += DeltaTime;
    m_TimeSinceLastFullAuto += DeltaTime;
    
    if( m_MinTimeRolledin > 0.0f )
    {    
        m_MinTimeRolledin -= DeltaTime;
    }
    else
    {
        m_MinTimeRolledin = 0.0f;
    }

    if( m_InCover )
    {
        m_TimeInCover += DeltaTime;
    }
    else
    {
        m_TimeInCover = 0.0f;
    }

    if( m_RolledOut )
    {
        m_TimeRolledOut += DeltaTime;
    }
    else
    {
        m_TimeRolledOut = 0.0f;
    }

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_BO_COVER_GOTO_COVER;
        break;
    case PHASE_BO_COVER_GOTO_COVER:
        {       
            SetInCover(FALSE);
            m_CharacterBase.CheckShooting();
            // if sucessfully completed
            if( m_CharacterBase.GetGoalCompleted() &&
                m_CharacterBase.GetGoalSucceeded() )
            {
                // Record that we are at this cover       
                SetCurrentCover( m_CharacterBase.GetCurrentCover() );
            
                if( m_CharacterBase.GetCoverIsValid() || 
                    m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET ||
                    m_CharacterBase.GetIsCoverHopper() )
                {                
                    newPhase = GetFacePhase();
                }
                else
                {
                    newPhase = GetNextPhaseOutOfCover();
                }
            }
            // else if failed or target changes.
            else if( m_CharacterBase.GetCurrentGoalInfo().m_TargetGuid != m_CharacterBase.GetCurrentCover() ||
                     m_CharacterBase.GetGoalCompleted() )
            {
                newPhase = PHASE_BO_COVER_GOTO_COVER;
            }
/*            else if( pOurCorpse && 
                     soldierObject.GetSubtype() == soldier::SUBTYPE_BLACKOPS )
            {
                return PHASE_BO_COVER_GOTO_CORPSE;
            }*/
            else if( m_CharacterBase.GetTargetGuid() &&
                     distToTargetSqr <= (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) &&
                     m_TimeSinceLastMelee >= k_MinTimeBetweenMelee )
            {
                newPhase = GetNextPhaseOutOfCover();
            }
        }
        break;
    case PHASE_BO_COVER_FACE_EXACT:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {   
            // Lookup roll out anim
            xbool bHasRollOutAnim = HasAnimForCoverPhase( PHASE_BO_COVER_ROLL_OUT );
            f32   RollOutDistSqr = 0.0f;
            
            // Is roll out animation present?
            if( bHasRollOutAnim )
            {
                // Lookup anim group
                const anim_group::handle& hAnimGroup = m_CoverAnimGroupHandle;
                const anim_group* pAnimGroup = hAnimGroup.GetPointer();
                if( pAnimGroup )
                {                    
                    // Lookup roll out animation
                    char  RollOutAnimName[64];
                    GetAnimNameFromPhase( PHASE_BO_COVER_ROLL_OUT, RollOutAnimName );

                    s32 iRollOutAnim = pAnimGroup->GetAnimIndex( RollOutAnimName );
                    if( iRollOutAnim != -1 )
                    {
                        // Lookup anim info
                        const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( iRollOutAnim );
                        
                        // Finally, get the distance squared!
                        RollOutDistSqr = AnimInfo.GetTotalTranslation().LengthSquared();
                    }
                }
            }
      
            // Can npc go straight to cover idle ready to shoot?
            if( m_CharacterBase.GetTargetGuid() &&
                m_CharacterBase.CanSeeTarget() && 
                m_CharacterBase.GetCoverIsValid() &&
                RollOutDistSqr < x_sqr( 0.25f ) &&
                HasAnimForCoverPhase(PHASE_BO_COVER_OUT_IDLE) )
            {
                SetRolledOut(TRUE);
                newPhase = GetNextPhaseRolledOut();
            }
            else if( m_CharacterBase.GetCoverIsValid() ||
                     m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET )
            {                
                if( HasAnimForCoverPhase(PHASE_BO_COVER_ENTER_COVER) )
                {                
                    newPhase = PHASE_BO_COVER_ENTER_COVER;
                }
                else
                {
                    SetInCover(TRUE);
                    newPhase = GetNextPhaseRolledIn();
                }
            }
            else
            {
                newPhase = GetNextPhaseOutOfCover();
            }
        }
        break;
    case PHASE_BO_COVER_ENTER_COVER:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetInCover(TRUE);
            newPhase = GetNextPhaseRolledIn();
        }
        break;
    case PHASE_BO_COVER_IDLE:
        {        
            SetInCover(TRUE);
            SetRolledOut(FALSE);
            m_DelayTillNextAction -= DeltaTime;

            newPhase = GetNextPhaseRolledIn();
            // if idle and we aren't done, then clear it
            if( newPhase == PHASE_BO_COVER_IDLE && 
                !m_CharacterBase.GetGoalCompleted() )
            {
                newPhase = PHASE_NONE;
            }
        }
        break;
    case PHASE_BO_COVER_ROLL_OUT:
        SetInCover(TRUE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetRolledOut(TRUE);
            newPhase = GetNextPhaseRolledOut();
        }
        break;
    case PHASE_BO_COVER_OUT_GRENADE:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetNextPhaseRolledOut();
        }
        break;
    case PHASE_BO_COVER_OUT_IDLE:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        m_DelayTillNextAction -= DeltaTime;
        newPhase = GetNextPhaseRolledOut();
        if( newPhase == PHASE_BO_COVER_OUT_IDLE && 
            !m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_NONE;
        }
        break;
    case PHASE_BO_COVER_OUT_SCAN:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        newPhase = GetNextPhaseRolledOut();
        if( newPhase == PHASE_BO_COVER_OUT_SCAN && 
            !m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_NONE;
        }
        break;
    case PHASE_BO_COVER_FULL_AUTO:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        m_CharacterBase.SetCrazyFire(TRUE);
        if( m_CharacterBase.IsReloading() || 
            m_TimeInPhase >= k_MaxTimeFullAuto )
        {
            m_AutofireRequested = FALSE;
            newPhase = GetNextPhaseRolledOut();
            m_TimeSinceLastFullAuto = 0.0f;
        }
        else if( m_CharacterBase.GetTargetGuid() &&
                 distToTargetSqr <= (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) &&
                 m_TimeSinceLastMelee >= k_MinTimeBetweenMelee )
        {
            m_AutofireRequested = FALSE;
            newPhase = GetNextPhaseRolledOut();
            m_TimeSinceLastFullAuto = 0.0f;
        }
        else if( m_CharacterBase.GetFriendlyBlocksTarget() )
        {
            m_AutofireRequested = FALSE;
            newPhase = GetNextPhaseRolledOut();
            m_TimeSinceLastFullAuto = 0.0f;
        }
        else if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_BO_COVER_FULL_AUTO;
            m_TimeSinceLastFullAuto = 0.0f;
        }
        break;
    case PHASE_BO_COVER_ROLL_IN:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetRolledOut(FALSE);
            newPhase = GetNextPhaseRolledIn();
        }
        break;
    case PHASE_BO_COVER_EXIT_COVER:
        SetInCover(TRUE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetInCover(FALSE);
            newPhase = GetNextPhaseOutOfCover();
        }
        break;
    case PHASE_BO_COVER_ALIGN_FOR_MELEE:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() ||
            m_TimeInPhase >= k_MinTimeAligning )
        {
            newPhase = PHASE_BO_COVER_MELEE;
        }
        break;
    case PHASE_BO_COVER_GOTO_CORPSE:
        {        
            // are we a soldier? we better be!
            if( m_CharacterBase.IsKindOf(soldier::GetRTTI()) )
            {
                soldier &soldierObject = soldier::GetSafeType( *(g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetGuid())) );
                object* pOurCorpse = g_ObjMgr.GetObjectByGuid( soldierObject.GetAllyCorpseGuid() );
                if( !pOurCorpse )
                {
                    newPhase = GetNextPhaseOutOfCover();
                }
                else if( m_CharacterBase.GetGoalCompleted() )
                {
                    if( m_CharacterBase.GetGoalSucceeded() )
                    {
                        newPhase = PHASE_BO_COVER_DRAIN_CORPSE;
                    }
                    else
                    {
                        newPhase = PHASE_BO_COVER_GOTO_CORPSE;
                    }
                }               
            }
            else
            {
                //wha? not a soldier? 
                ASSERT(FALSE);
                newPhase = GetNextPhaseOutOfCover();
            }
        }
        break;
    case PHASE_BO_COVER_DRAIN_CORPSE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            // we are done, change us!
            if( m_CharacterBase.IsKindOf(soldier::GetRTTI()) )
            {
                soldier &soldierObject = soldier::GetSafeType( *(g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetGuid())) );
                soldierObject.BecomeLeader();
                newPhase = GetNextPhaseOutOfCover();
            }
        }
        break;
    case PHASE_BO_COVER_SUMMON_ALLIES:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetNextPhaseOutOfCover();
        }
        break;
    case PHASE_BO_COVER_MELEE:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {            
            newPhase = GetNextPhaseOutOfCover();
            m_TimeSinceLastMelee = 0.0f;
        }
        break;
    case PHASE_BO_COVER_STAND_AND_SHOOT:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        m_CharacterBase.CheckShooting();
        newPhase = GetNextPhaseOutOfCover();
        // if stand and shoot, then clear it we are already doing that.
        if( newPhase == PHASE_BO_COVER_STAND_AND_SHOOT )
        {
            newPhase = PHASE_NONE;
        }
        break;
    case PHASE_BO_COVER_COVER_ME:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetNextPhaseOutOfCover();
        }
        break;
    case PHASE_BO_COVER_REQUEST_ATTACK:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetNextPhaseOutOfCover();
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    };

    // nice override of the overrides,
    if( m_CharacterBase.GetAllyAcquiredTarget() )
    {
        newPhase = PHASE_BO_COVER_REQUEST_ATTACK;
        m_CharacterBase.SetAllyAcquiredTarget( FALSE );
    }
    else     if( m_AutofireRequestSent )
    {
        newPhase = PHASE_BO_COVER_COVER_ME;
        m_AutofireRequestSent = FALSE;
    }

    // Setup relative mode
    UpdateRelativeMode();
    
    // ignore the base state.
    //??? why ignore the base state? Then they can't evade grenades or get back into 
    // the nav path when running and they get out.
    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE )
    {
        newPhase = basePhase;
    }
    return newPhase;
}

//=========================================================================

void blackOp_cover_state::ChangePhase( s32 newPhase )
{
    object *nodeObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetCurrentCover());
    object *ourNodeObject = g_ObjMgr.GetObjectByGuid(m_CurrentCover);

    char AnimNameFromPhase[32];
    switch( newPhase)
    {    
    case PHASE_BO_COVER_GOTO_COVER:                
        m_CharacterBase.SetWantsToAim(TRUE);
        if ( nodeObject && nodeObject->IsKindOf( cover_node::GetRTTI() ))
        {   
            cover_node &coverNode = cover_node::GetSafeType( *nodeObject );           
            m_CharacterBase.SetGotoTargetGoal(coverNode.GetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,0.0f,TRUE);
        }
        else
        {
            m_CharacterBase.SetIdleGoal();
        }
        break;
    case PHASE_BO_COVER_FACE_EXACT:              
        m_CharacterBase.SetWantsToAim(TRUE);
        if ( ourNodeObject && ourNodeObject->IsKindOf( cover_node::GetRTTI() ))
        {   
            cover_node &coverNode = cover_node::GetSafeType( *ourNodeObject );           
            vector3 coverNodeFacing(0.0f,coverNode.GetNPCFacing());
            coverNodeFacing.NormalizeAndScale(1000.0f);
            m_CharacterBase.SetTurnToLocationGoal(coverNode.GetPosition() + coverNodeFacing, 0.0f, TRUE);
        }
        else
        {
            m_CharacterBase.SetIdleGoal();
        }
        break;
    case PHASE_BO_COVER_IDLE:                   
    case PHASE_BO_COVER_OUT_SCAN:                   
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              /*loco::ANIM_FLAG_END_STATE_HOLD |*/ loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_RESTART_IF_SAME_ANIM );
        break;
    case PHASE_BO_COVER_ENTER_COVER:                   
        {        
            UpdateUsedDelays();
            if ( ourNodeObject && ourNodeObject->IsKindOf( cover_node::GetRTTI() ))
            {   
                cover_node &coverNode = cover_node::GetSafeType( *ourNodeObject );           
                radian yawDiff = x_MinAngleDiff( m_CharacterBase.GetLocoPointer()->GetYaw(),coverNode.GetNPCFacing() );
                m_CharacterBase.SetAnimYawDelta( -yawDiff );
            }
        }
    case PHASE_BO_COVER_ROLL_OUT:                   
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              /*loco::ANIM_FLAG_END_STATE_HOLD */ 0 );
        break;
    case PHASE_BO_COVER_ROLL_IN:                   
    case PHASE_BO_COVER_EXIT_COVER:                   
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              /*loco::ANIM_FLAG_END_STATE_HOLD |*/ loco::ANIM_FLAG_TURN_OFF_AIMER );
        break;
    case PHASE_BO_COVER_OUT_GRENADE:                
        // request full auto while grenading
        m_CharacterBase.SetWantsToAim(TRUE);
        if( m_CharacterBase.HasAllies() )
        {        
            if( m_CharacterBase.RequestAutofire() )
            {            
                m_CharacterBase.PlayDialog( character::DIALOG_GRENADE_THROW );
            }
        }

        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              // removing hold, this can make them never leave the play_anim state
                                              // in loco. Happened when went to goto goal but was staggering (so no force to move state).
                                              /*loco::ANIM_FLAG_END_STATE_HOLD*/ 0 );
        break;
    case PHASE_BO_COVER_FULL_AUTO:                
        if( m_DoAutofireDialog )
        {        
            m_CharacterBase.PlayDialog( character::DIALOG_COVER );
            m_DoAutofireDialog = FALSE;
        }
    case PHASE_BO_COVER_OUT_IDLE:                
        m_CharacterBase.SetWantsToAim(TRUE);
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              /*loco::ANIM_FLAG_END_STATE_HOLD | */loco::ANIM_FLAG_RESTART_IF_SAME_ANIM );
        break;
    case PHASE_BO_COVER_ALIGN_FOR_MELEE:                
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetTurnToTargetGoal( m_CharacterBase.GetTargetGuid() );
        break;
    case PHASE_BO_COVER_GOTO_CORPSE:
        if( m_CharacterBase.IsKindOf(soldier::GetRTTI()) )
        {
            soldier &soldierObject = soldier::GetSafeType( *(g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetGuid())) );
            object* pOurCorpse = g_ObjMgr.GetObjectByGuid( soldierObject.GetAllyCorpseGuid() );
            m_CharacterBase.SetGotoTargetGoal(pOurCorpse->GetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,100.0f);
        }
        else
        { 
            ASSERT(FALSE);
            m_CharacterBase.SetIdleGoal();
        }
        break;
    case PHASE_BO_COVER_DRAIN_CORPSE:
        m_CharacterBase.SetPlayAnimationGoal( loco::ANIM_DRAIN_LIFE );
        break;
    case PHASE_BO_COVER_SUMMON_ALLIES:
        m_CharacterBase.SetPlayAnimationGoal( loco::ANIM_EVADE_LEFT );
        break;
    case PHASE_BO_COVER_MELEE:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_SHORT);
        break;
    case PHASE_BO_COVER_STAND_AND_SHOOT:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetIdleGoal();
        break;
    case PHASE_BO_COVER_COVER_ME:
        if( m_CharacterBase.GetSubtype() == soldier::SUBTYPE_BLACKOP_LEADER )
        {        
            m_CharacterBase.SetPlayAnimationGoal( loco::ANIM_REQUEST_COVER );
        }
        else
        {
            m_CharacterBase.SetIdleGoal();
        }
        break;
    case PHASE_BO_COVER_REQUEST_ATTACK:
        if( m_CharacterBase.GetSubtype() == soldier::SUBTYPE_BLACKOP_LEADER )
        {        
            m_CharacterBase.SetPlayAnimationGoal( loco::ANIM_REQUEST_ATTACK );
        }
        else
        {
            m_CharacterBase.SetIdleGoal();
        }
        break;
    default:        
        if( newPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid New Phase " );
        }        
    }
    character_state::ChangePhase( newPhase );
}

//=========================================================================

character_state::states blackOp_cover_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    object *nodeObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetCurrentCover());
    if ( !nodeObject || !nodeObject->IsKindOf(cover_node::GetRTTI()) )
    {   
        m_DesiredState = m_CharacterBase.GetStateFromAwareness();
        if( m_DesiredState == STATE_COVER )
        {
            m_DesiredState = STATE_NULL;
        }
    }
    // we stay here as long as we have sticky cover.
    else if( !m_CharacterBase.GetStickyCoverNode() )
    {   
        // if leave cover conditions met, leave cover
        if( (m_LeaveCoverCondition == LEAVE_COVER_WHEN_DAMAGED && 
             m_TookPainThisTick)  ||
            (m_LeaveCoverCondition == LEAVE_COVER_WHEN_CAN_REACH_TARGET && 
             m_CharacterBase.CanPathToTarget()) )
        {
            m_DesiredState = m_CharacterBase.GetStateFromAwareness();
            if( m_DesiredState == STATE_COVER )
            {
                m_DesiredState = STATE_NULL;
            }
        }
    }
    
    if( !m_InCover && m_DesiredState != STATE_NULL )
    {
        if( nodeObject && nodeObject->IsKindOf(cover_node::GetRTTI()) )
        {        
            cover_node &coverNode = cover_node::GetSafeType( *nodeObject );
            coverNode.InvalidateNode();
        }
        return m_DesiredState;
    }
    return STATE_NULL;
}

//=========================================================================

const char*blackOp_cover_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_BO_COVER_GOTO_COVER:
        return "PHASE_BO_COVER_GOTO_COVER";
    	break;
    case PHASE_BO_COVER_FACE_EXACT:
        return "PHASE_BO_COVER_FACE_EXACT";
    	break;
    case PHASE_BO_COVER_ENTER_COVER:
        return "PHASE_BO_COVER_ENTER_COVER";
        break;
    case PHASE_BO_COVER_IDLE:
        return "PHASE_BO_COVER_IDLE";
    	break;
    case PHASE_BO_COVER_ROLL_OUT:
        return "PHASE_BO_COVER_ROLL_OUT";
    	break;
    case PHASE_BO_COVER_OUT_IDLE:
        return "PHASE_BO_COVER_OUT_IDLE";
        break;
    case PHASE_BO_COVER_OUT_GRENADE:
        return "PHASE_BO_COVER_OUT_GRENADE";
        break;
    case PHASE_BO_COVER_OUT_SCAN:
        return "PHASE_BO_COVER_OUT_SCAN";
        break;
    case PHASE_BO_COVER_FULL_AUTO:
        return "PHASE_BO_COVER_FULL_AUTO";
        break;
    case PHASE_BO_COVER_ROLL_IN:
        return "PHASE_BO_COVER_ROLL_IN";
        break;
    case PHASE_BO_COVER_EXIT_COVER:
        return "PHASE_BO_COVER_EXIT_COVER";
    	break;
    case PHASE_BO_COVER_ALIGN_FOR_MELEE:
        return "PHASE_BO_COVER_ALIGN_FOR_MELEE";
        break;
    case PHASE_BO_COVER_MELEE:
        return "PHASE_BO_COVER_MELEE";
        break;
    case PHASE_BO_COVER_GOTO_CORPSE:
        return "PHASE_BO_COVER_GOTO_CORPSE";
        break;
    case PHASE_BO_COVER_DRAIN_CORPSE:
        return "PHASE_BO_COVER_DRAIN_CORPSE";
        break;
    case PHASE_BO_COVER_SUMMON_ALLIES:
        return "PHASE_BO_COVER_SUMMON_ALLIES";
        break;
    case PHASE_BO_COVER_STAND_AND_SHOOT:
        return "PHASE_BO_COVER_STAND_AND_SHOOT";
        break;
    case PHASE_BO_COVER_COVER_ME:
        return "PHASE_BO_COVER_COVER_ME";
        break;
    case PHASE_BO_COVER_REQUEST_ATTACK:
        return "PHASE_BO_COVER_REQUEST_ATTACK";
        break;
    }
    return character_state::GetPhaseName(thePhase);
}

