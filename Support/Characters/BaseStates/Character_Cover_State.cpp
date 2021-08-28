#include "Character_Cover_State.hpp"
#include "..\Character.hpp"
#include "navigation\coverNode.hpp"
#include "objects\NewWeapon.hpp"
#include "audiomgr\audiomgr.hpp"
#include "objects\Player.hpp"

//=========================================================================
// constants
//=========================================================================

const f32 k_MinTimeInIdle             = 0.0f;
const f32 k_MinTimeAligning           = 1.0f;
const f32 k_MinTimeBetweenCoverReloads  = 3.0f;
const f32 k_MinTimeBetweenMelee       = 2.0f;
const f32 k_MinTimeBetweenFullAuto    = 8.0f;
const f32 k_MinRollDistSqr            = 50.0f * 50.0f;
const f32 k_MinTimePlayerShootingToRollin = 3.0f;
const f32 k_MinTimeBlockingToRollIn   = 0.5f;
const f32 k_MinToBeAtCoverSqr         = 100.0f * 100.0f;
const f32 k_MinDistForGrenadeClusterSqr = 600.0f * 600.0f;
const f32 k_MinDistToBeAtSqr          = 50.0f * 50.0f;
const f32 k_MinDistToGotoRolledOutSqr = 100.0f * 100.0f;
const f32 k_MinScanTime               = 2.0f;
const f32 k_MaxScanTime               = 4.0f;
const f32 k_MinTimeBetweenEnterCover  = 3.0f;

//=========================================================================
// GRAY COVER STATE
//=========================================================================

character_cover_state::character_cover_state( character& ourCharacter, character_state::states State ) :
    character_state(ourCharacter, State)
{
    m_TimeTillBored = 10.0f;
    m_MoveStyle = loco::MOVE_STYLE_RUNAIM;
    
    m_ActionDelayMin = 0.0f;
    m_ActionDelayMax = 1.5f;

    m_ShootWeight = 70;
    m_GrenadeWeight = 10;
    m_LeaveCoverCondition = LEAVE_COVER_WHEN_BROKEN;

    m_CurrentCover = 0;
    m_TimeSinceLastFullAuto = 0.0f;
    m_bRolledOutBlocksLOFTime = 0.0f;
}

//=========================================================================

character_cover_state::~character_cover_state()
{
}

//=========================================================================

void character_cover_state::OnInit( void )
{
    character_state::OnInit();
}

//=========================================================================

void character_cover_state::OnEnter( void )
{
    m_HasClearGrenadeThrow  = FALSE;
    m_DesiredState          = STATE_NULL;
    m_InCover               = FALSE;
    m_RolledOut             = FALSE;
    m_AutofireCanceled      = FALSE;
    m_DoAutofireDialog      = FALSE;
    m_TimeInCover           = 0.0f;
    m_TimeRolledOut         = 0.0f;
    m_TimeSinceLastMelee    = k_MinTimeBetweenMelee;
    m_TimeSinceLastEnterCover = k_MinTimeBetweenEnterCover;
    UpdateUsedDelays();
    m_DelayTillNextAction   = 0.0f;
    m_AutofireRequested     = FALSE;
    m_TimeSinceLastFullAuto = k_MinTimeBetweenFullAuto;
    m_bRolledInBlocksLOF    = FALSE;
    m_bRolledOutBlocksLOF   = FALSE;
    m_ScanTime              = x_frand(k_MinScanTime,k_MaxScanTime);
    // Clear cover
    SetCurrentCover( 0 );
    
    character_state::OnEnter();
}

//=========================================================================

xbool character_cover_state::OnExit( void )
{
    // Clear cover
    SetCurrentCover( 0 );
    
    // Turn off cover relative mode
    loco* pLoco = m_CharacterBase.GetLocoPointer();
    if( pLoco )
    {
        pLoco->SetCoverRelativeMode( FALSE );
    }
    
    // Call base class
    return character_state::OnExit() ;
}

//=========================================================================

void character_cover_state::UpdateUsedDelays()
{
    m_UsedActionDelayMin = m_ActionDelayMin;
    m_UsedActionDelayMax = m_ActionDelayMax;
    // check for cover node overrides
    object *coverObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetCurrentCover() );
    if( coverObject && coverObject->IsKindOf(cover_node::GetRTTI()) )
    {
        cover_node& coverNode = cover_node::GetSafeType( *coverObject );
        if( coverNode.GetMinDelay() >= 0.0f )
        {
            m_UsedActionDelayMin = coverNode.GetMinDelay();
        }
        if( coverNode.GetMaxDelay() >= 0.0f )
        {
            m_UsedActionDelayMax = coverNode.GetMaxDelay();
        }
    }
}

//=========================================================================

xbool character_cover_state::GetAnimNameFromPhase( s32 nextPhase, char* animName )
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
    case PHASE_COVER_ENTER_COVER:    
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
    case PHASE_COVER_IDLE:    
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
    case PHASE_COVER_SHOOT:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_SHOOT");
        }
        else
        {
            x_strcpy(animName,"COVER_SHOOT");
        }
        break;
    case PHASE_COVER_ROLL_OUT:    
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
    case PHASE_COVER_FULL_AUTO:    
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
    case PHASE_COVER_OUT_IDLE:    
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
    case PHASE_COVER_OUT_GRENADE:    
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
    case PHASE_COVER_OUT_SCAN:    
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
    case PHASE_COVER_ROLL_IN:    
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
    case PHASE_COVER_THROW_GRENADE:    
        bFoundName = TRUE;
        if( weaponPrefix[0] != 0 )
        {    
            x_strcpy(animName,xfs("%s_",weaponPrefix));
            x_strcat(animName,"COVER_GRENADE");
        }
        else
        {
            x_strcpy(animName,"COVER_GRENADE");
        }
        break;
    case PHASE_COVER_EXIT_COVER:    
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

xbool character_cover_state::HasAnimForCoverPhase( s32 coverPhase )
{
    char AnimNameBuffer[64];

    GetAnimNameFromPhase(coverPhase,AnimNameBuffer);
    return ( m_CoverAnimGroupHandle.GetPointer() && 
             m_CoverAnimGroupHandle.GetPointer()->GetAnimIndex(AnimNameBuffer) >= 0 );
}

//=========================================================================

s32 character_cover_state::GetNextPhaseRolledOut()
{
    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();   
    radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
    radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
    radian m_DiffToTargetYaw = x_abs(x_MinAngleDiff(locoYaw, TargetYaw));

    player *pPlayer = SMP_UTIL_GetActivePlayer();

    // if I desired to change states, or I'm not at my node, 
    // or my node is invalid and I have a target, then exit
    if( m_DesiredState )
    {
        // just pop us out of cover...
        SetInCover(FALSE);
        return PHASE_COVER_STAND_AND_SHOOT;                
    }
    else if( m_CurrentCover != m_CharacterBase.GetCurrentCover() )
    {
        // just pop us out of cover...
        SetInCover(FALSE);
        return PHASE_COVER_STAND_AND_SHOOT;                
    }
    else if( m_CharacterBase.GetTargetGuid() &&
        distToTargetSqr <= (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) &&
        m_TimeSinceLastMelee >= k_MinTimeBetweenMelee )
    {
        if( m_DiffToTargetYaw > R_10 )
        {            
            return PHASE_COVER_ALIGN_FOR_MELEE;
        }
        else
        {
            return PHASE_COVER_MELEE;
        }
    }
    else if( !m_CharacterBase.GetCoverIsValid() && 
             m_CharacterBase.GetAwarenessLevel() >= character::AWARENESS_TARGET_LOST )
    {
        // just pop us out of cover...
        SetInCover(FALSE);
        return PHASE_COVER_STAND_AND_SHOOT;
    }
    else if( m_CharacterBase.IsReloading() ||
             (!m_bRolledInBlocksLOF && 
              m_bRolledOutBlocksLOFTime >= k_MinTimeBlockingToRollIn &&
              pPlayer &&
              pPlayer->GetCurrentAnimState() != player::ANIM_STATE_RELOAD && 
              pPlayer->GetCurrentAnimState() != player::ANIM_STATE_RELOAD_IN &&
              pPlayer->GetCurrentAnimState() != player::ANIM_STATE_RELOAD_OUT) )
    {
        if( HasAnimForCoverPhase(PHASE_COVER_ROLL_IN) )
        {
            return PHASE_COVER_ROLL_IN;
        }
        else
        {
            return PHASE_COVER_IDLE;
        }
    }
    else if( m_AutofireRequested && 
             HasAnimForCoverPhase(PHASE_COVER_FULL_AUTO) )
    {
        m_AutofireRequested = FALSE;
        return PHASE_COVER_FULL_AUTO;
    }
    else if( m_TookPainThisTick &&
             m_TimeSinceLastEnterCover >= k_MinTimeBetweenEnterCover )
    {
        if( HasAnimForCoverPhase(PHASE_COVER_ROLL_IN) )
        {                
            return PHASE_COVER_ROLL_IN;
        }
        else
        {
            SetRolledOut(FALSE);
            return PHASE_COVER_IDLE;
        }
    }
    else if ( m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET ||
              ( m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_TARGET_LOST &&
                m_CharacterBase.GetTimeSinceLastSound() <= m_ScanTime ) )
    {
        // get a new scan time 
        if( m_CharacterBase.GetTimeSinceLastSound() < k_MinScanTime )
        {
            m_ScanTime = x_frand(k_MinScanTime,k_MaxScanTime);
        }

        if( HasAnimForCoverPhase(PHASE_COVER_OUT_SCAN) )
        {                
            return PHASE_COVER_OUT_SCAN;
        }
        else
        {
            return PHASE_COVER_OUT_IDLE;
        }
    }
    else if( m_DelayTillNextAction > 0.0f )
    {

        return PHASE_COVER_OUT_IDLE;
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
        if( !HasAnimForCoverPhase(PHASE_COVER_OUT_GRENADE) || 
            !m_HasClearGrenadeThrow || 
            m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET )
        {
            usedGrenadeWeight = 0;
        }

        s32 totalWeight = usedShootWeight + usedGrenadeWeight;
        if( totalWeight <= 0 )
        { 
            return PHASE_COVER_OUT_IDLE;
        }

        s32 chosenWeight = x_irand(1,totalWeight);
        chosenWeight -= usedGrenadeWeight;
        if( chosenWeight <= 0 )
        {
            m_DelayTillNextAction = x_frand(m_UsedActionDelayMin,m_UsedActionDelayMax);
            return PHASE_COVER_OUT_GRENADE;
        }
        else
        {
            m_DelayTillNextAction = x_frand(m_UsedActionDelayMin,m_UsedActionDelayMax);
            m_CharacterBase.SetWantsToFirePrimary(TRUE);
        }
    }
    return PHASE_COVER_OUT_IDLE;
}

//=========================================================================

s32 character_cover_state::GetNextPhaseRolledIn()
{
    player *pPlayer = SMP_UTIL_GetActivePlayer();

    // if we need to move, move!
    if ( m_CurrentCover != m_CharacterBase.GetCurrentCover() )
    {
        if( HasAnimForCoverPhase(PHASE_COVER_EXIT_COVER) )
        {
            return PHASE_COVER_EXIT_COVER;
        }
        else
        {                
            // just pop us out of cover...
            SetInCover(FALSE);
            return PHASE_COVER_STAND_AND_SHOOT;                
        }
    }
    // if our cover is not valid pop us out!
    // remove the sticky part, in alamo guys were standing and reloading... they should
    // stand and shoot reguardless of weather or not it's sticky shouldn't they?
    else if( !m_CharacterBase.GetCoverIsValid() && 
              m_CharacterBase.GetAwarenessLevel() >= character::AWARENESS_TARGET_LOST /*&&
             !m_CharacterBase.GetIsCoverSticky(m_CurrentCover)*/ )
    {
        if( HasAnimForCoverPhase(PHASE_COVER_EXIT_COVER) )
        {
            return PHASE_COVER_EXIT_COVER;
        }
        else
        {                
            // just pop us out of cover...
            SetInCover(FALSE);
            return PHASE_COVER_STAND_AND_SHOOT;                
        }
    }
    // if reloading stay down.
    else if( m_CharacterBase.IsReloading() )
    {
        return PHASE_COVER_IDLE;
    }
    // if reloading or not time yet,
    // then 
    // if I desired to change states, or I'm not at my node, 
    // or my node is invalid and I have a target, then exit
    else if( m_DesiredState )
    {
        if( HasAnimForCoverPhase(PHASE_COVER_EXIT_COVER) )
        {
            return PHASE_COVER_EXIT_COVER;
        }
        else
        {                
            // just pop us out of cover...
            SetInCover(FALSE);
            return PHASE_COVER_STAND_AND_SHOOT;                
        }
    }
    else if( m_AutofireRequested && 
        HasAnimForCoverPhase(PHASE_COVER_FULL_AUTO) &&
        HasAnimForCoverPhase(PHASE_COVER_OUT_IDLE) )
    {
        m_AutofireRequested = FALSE;
        if( HasAnimForCoverPhase(PHASE_COVER_ROLL_OUT) )
        {                    
            return PHASE_COVER_ROLL_OUT;
        }
        else
        {
            SetRolledOut(TRUE);
            return PHASE_COVER_FULL_AUTO;
        }
    }
    else if( HasAnimForCoverPhase(PHASE_COVER_OUT_IDLE) )
    {
        if( m_TimeInPhase >= k_MinTimeInIdle &&
            pPlayer &&
            (!m_bRolledOutBlocksLOF ||
             pPlayer->GetCurrentAnimState() == player::ANIM_STATE_RELOAD || 
             pPlayer->GetCurrentAnimState() == player::ANIM_STATE_RELOAD_IN ||
             pPlayer->GetCurrentAnimState() == player::ANIM_STATE_RELOAD_OUT ) )
        {                
            if( HasAnimForCoverPhase(PHASE_COVER_ROLL_OUT) )
            {                    
                return PHASE_COVER_ROLL_OUT;
            }
            else
            {
                SetRolledOut(TRUE);
                return PHASE_COVER_OUT_IDLE;
            }
        }
        else
        {
            return PHASE_COVER_IDLE;
        }
    }
    else 
    {   
        if( m_DelayTillNextAction > 0.0f )
        {
            return PHASE_COVER_IDLE;
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
            if( !HasAnimForCoverPhase(PHASE_COVER_SHOOT) || 
                m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET )
            {
                usedShootWeight = 0;
            }
            // can we do a grenade throw?
            if( !HasAnimForCoverPhase(PHASE_COVER_THROW_GRENADE) || 
                !m_HasClearGrenadeThrow || 
                m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET )
            {
                usedGrenadeWeight = 0;
            }

            s32 totalWeight = usedShootWeight + usedGrenadeWeight;
            if( totalWeight <= 0 )
            { 
                return PHASE_COVER_IDLE;
            }

            s32 chosenWeight = x_irand(1,totalWeight);
            chosenWeight -= usedGrenadeWeight;
            if( chosenWeight <= 0 )
            {
                m_DelayTillNextAction = x_frand(m_UsedActionDelayMin,m_UsedActionDelayMax);
                return PHASE_COVER_THROW_GRENADE;
            }
        }
    }
    m_DelayTillNextAction = x_frand(m_UsedActionDelayMin,m_UsedActionDelayMax);
    return PHASE_COVER_SHOOT;
}

//=========================================================================

s32 character_cover_state::GetNextPhaseOutOfCover()
{
    // I'not m currently in my cover, what to do...

    // first, should we just smackthe guy in face?
    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();   
    radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
    radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
    radian m_DiffToTargetYaw = x_abs(x_MinAngleDiff(locoYaw, TargetYaw));

    if( m_CharacterBase.GetTargetGuid() &&
        distToTargetSqr <= (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) &&
        m_TimeSinceLastMelee >= k_MinTimeBetweenMelee )
    {
        if( m_DiffToTargetYaw > R_10 )
        {            
            return PHASE_COVER_ALIGN_FOR_MELEE;
        }
        else
        {
            return PHASE_COVER_MELEE;
        }
    }
    // if my cover is valid || I'm not fighting, it's ok to get into cover...
    else if( m_CurrentCover != m_CharacterBase.GetCurrentCover() )
    {
        SetCurrentCover(0);
        return GetGotoCoverPhase();                
    }
    // setup a delay on entering cover so we don't do it over and over.
    else if( ( m_CharacterBase.GetCoverIsValid() || 
               m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET )
               && m_TimeSinceLastEnterCover >= k_MinTimeBetweenEnterCover )
    {
        return GetGotoCoverPhase();                
/*        // go to whichever is closer, cover or rolled out.
        object *coverObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetCurrentCover() );
        if( coverObject )
        {    
            vector3 toCover = coverObject->GetPosition() - m_CharacterBase.GetPosition();
            vector3 toRolledOut = m_RolledOutPosition - m_CharacterBase.GetPosition();
            if( toRolledOut.LengthSquared() <= toCover.LengthSquared() )
            {
                return PHASE_COVER_GOTO_ROLLED_OUT;
            }
        }
        return PHASE_COVER_GOTO_COVER;                */
    }
    return PHASE_COVER_STAND_AND_SHOOT;
}

//=========================================================================

s32 character_cover_state::GetGotoCoverPhase()
{
    // go to whichever is closer, cover or rolled out...

    // on second thought... only go to rolled out IF IT IS VERY CLOSE (causing too many issues)
    object *coverObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetCurrentCover() );
    if( coverObject )
    {    
        vector3 toCover = coverObject->GetPosition() - m_CharacterBase.GetPosition();
        vector3 rolledOutPos;
        xbool rolledOutValid = GetRolledOutPosition(m_CharacterBase.GetCurrentCover(), rolledOutPos );
        vector3 toRolledOut = rolledOutPos - m_CharacterBase.GetPosition();
        if( toRolledOut.LengthSquared() <= toCover.LengthSquared() &&
            rolledOutValid &&
            toRolledOut.LengthSquared() <= k_MinDistToGotoRolledOutSqr )
        {
            m_RolledOutPosition = rolledOutPos;
            if( toRolledOut.LengthSquared() > k_MinDistToBeAtSqr )
            {
                return PHASE_COVER_GOTO_ROLLED_OUT;    
            }
            else
            {
                SetCurrentCover( m_CharacterBase.GetCurrentCover() );
                return PHASE_COVER_FACE_FORWARD;
            }
        }        
        else
        {
            if( toCover.LengthSquared() > k_MinDistToBeAtSqr )
            {
                return PHASE_COVER_GOTO_COVER;    
            }
            else
            {
                SetCurrentCover( m_CharacterBase.GetCurrentCover() );
                return GetFacePhase();
            }
        }
    }
    return PHASE_NONE;                
}

//=========================================================================

s32 character_cover_state::GetFacePhase()
{
    // if we have an enter cover, we will blend roll in instead of facing initially.
    if( HasAnimForCoverPhase(PHASE_COVER_ENTER_COVER) )
    {
        return PHASE_COVER_ENTER_COVER;
    }
    else
    {
        return PHASE_COVER_FACE_EXACT;
    }
}

//=========================================================================

xbool character_cover_state::ProvideAutofire()
{   
    if( HasAnimForCoverPhase(PHASE_COVER_FULL_AUTO) &&
        m_InCover &&
        m_CharacterBase.GetTargetGuid() &&
        m_TimeSinceLastFullAuto >= k_MinTimeBetweenFullAuto && 
        !m_CharacterBase.IsReloading() && 
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

void character_cover_state::CancelAutofire()
{   
    m_AutofireCanceled = TRUE;
}

//=========================================================================

xbool character_cover_state::IgnoreFullBodyFlinches()
{
    if( m_CurrentPhase == PHASE_COVER_ENTER_COVER || 
        m_CurrentPhase == PHASE_COVER_EXIT_COVER )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=========================================================================

xbool character_cover_state::UseRelativeMode( void )
{
    // let's see what removing this does for us.
    // it allows for bugs such as NPCs facing the wrong direction when at their cover nodes.
    // this will at least guarentee the guy is always facing correctly.
//    return FALSE;

    // Which phase?
    switch( m_CurrentPhase )
    {
        // If arriving at cover node, turn on relative mode and set the position
    case PHASE_COVER_IDLE:
    case PHASE_COVER_SHOOT:
    case PHASE_COVER_THROW_GRENADE:
    case PHASE_COVER_ROLL_OUT:
    case PHASE_COVER_ROLL_IN:
        return TRUE;

        // If npc is moving to cover or doing a base phase, turn off relative mode
    default:

        // These "OUT" anims are at the origin so they can't use relative mode
    case PHASE_COVER_OUT_IDLE:
    case PHASE_COVER_OUT_SCAN:
    case PHASE_COVER_OUT_GRENADE:
    case PHASE_COVER_FULL_AUTO:
    case PHASE_COVER_ALIGN_FOR_MELEE:
    case PHASE_COVER_MELEE:
    case PHASE_COVER_STAND_AND_SHOOT:

        // These phases are either going to or coming from cover so no relative mode
    case PHASE_COVER_FACE_EXACT:
    case PHASE_COVER_ENTER_COVER:
    case PHASE_COVER_GOTO_COVER:
    case PHASE_COVER_EXIT_COVER:

        // Turn off
        return FALSE;
    }
}

//=========================================================================

void character_cover_state::UpdateRelativeMode( void )
{
    // Get loco
    loco* pLoco = m_CharacterBase.GetLocoPointer();
    if( !pLoco )
        return;

    // Use relative mode?
    object_ptr<cover_node> pCoverNode( m_CurrentCover );
    xbool bRelativeMode = ( pCoverNode ) && ( UseRelativeMode() );

    // Update loco
    if( bRelativeMode )
    {
        // Turn on!
        ASSERT( pCoverNode );
        pLoco->SetCoverRelativeMode( TRUE );
        pLoco->SetCoverRelativeInfo( pCoverNode->GetPosition(), pCoverNode->GetNPCFacing() );
    }
    else
    {
        // Turn off
        pLoco->SetCoverRelativeMode( FALSE );
    }
}

//=========================================================================

s32 character_cover_state::UpdatePhase( f32 DeltaTime )
{
    if( m_bRolledOutBlocksLOF )
    {
        m_bRolledOutBlocksLOFTime += DeltaTime;
    }
    else
    {
        m_bRolledOutBlocksLOFTime = 0.0f;
    }

    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();   
    s32 newPhase = PHASE_NONE;
    m_TimeSinceLastMelee += DeltaTime;
    m_TimeSinceLastFullAuto += DeltaTime;
    m_TimeSinceLastEnterCover += DeltaTime;
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
        newPhase = GetGotoCoverPhase();
        break;
    case PHASE_COVER_GOTO_COVER:
        {       
            SetInCover(FALSE);
            m_CharacterBase.CheckShooting();
            // if sucessfully completed
            if( m_CharacterBase.GetGoalCompleted() &&
                m_CharacterBase.GetGoalSucceeded() )
            {
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
                newPhase = GetGotoCoverPhase();
            }
            else if( m_CharacterBase.GetTargetGuid() &&
                distToTargetSqr <= (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) &&
                m_TimeSinceLastMelee >= k_MinTimeBetweenMelee )
            {
                newPhase = GetNextPhaseOutOfCover();
            }
        }
        break;
    case PHASE_COVER_GOTO_ROLLED_OUT:
        {       
            SetInCover(FALSE);
            m_CharacterBase.CheckShooting();
            // if sucessfully completed
            if( m_CharacterBase.GetGoalCompleted() &&
                m_CharacterBase.GetGoalSucceeded() )
            {
                SetCurrentCover( m_CharacterBase.GetCurrentCover() );
            
                if( m_CharacterBase.GetCoverIsValid() || 
                    m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET ||
                    m_CharacterBase.GetIsCoverHopper() )
                {                
                    newPhase = PHASE_COVER_FACE_FORWARD;
                }
                else
                {
                    newPhase = GetNextPhaseRolledOut();
                }
            }
            // else if failed or target changes.
            else if( m_CharacterBase.GetCurrentGoalInfo().m_TargetGuid != m_CharacterBase.GetCurrentCover() ||
                     m_CharacterBase.GetGoalCompleted() )
            {
                newPhase = GetGotoCoverPhase();
            }
            else if( m_CharacterBase.GetTargetGuid() &&
                     distToTargetSqr <= (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) &&
                     m_TimeSinceLastMelee >= k_MinTimeBetweenMelee )
            {
                newPhase = GetNextPhaseRolledOut();
            }
        }
        break;
    case PHASE_COVER_FACE_EXACT:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {   
            // Lookup roll out anim
            xbool bHasRollOutAnim = HasAnimForCoverPhase( PHASE_COVER_ROLL_OUT );
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
                    GetAnimNameFromPhase( PHASE_COVER_ROLL_OUT, RollOutAnimName );

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
                HasAnimForCoverPhase(PHASE_COVER_OUT_IDLE) )
            {
                SetRolledOut(TRUE);
                newPhase = GetNextPhaseRolledOut();
            }
            else if( m_CharacterBase.GetCoverIsValid() ||
                     m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET )
            {                
                if( HasAnimForCoverPhase(PHASE_COVER_ENTER_COVER) )
                {                
                    newPhase = PHASE_COVER_ENTER_COVER;
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
    case PHASE_COVER_ENTER_COVER:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetInCover(TRUE);
            newPhase = GetNextPhaseRolledIn();
        }
        break;
    case PHASE_COVER_IDLE:
        {        
            SetInCover(TRUE);
            SetRolledOut(FALSE);
            m_DelayTillNextAction -= DeltaTime;

            newPhase = GetNextPhaseRolledIn();
            // if idle and we aren't done, then clear it
            if( newPhase == PHASE_COVER_IDLE && 
                !m_CharacterBase.GetGoalCompleted() )
            {
                newPhase = PHASE_NONE;
            }
        }
        break;
    case PHASE_COVER_SHOOT:
        SetInCover(TRUE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetNextPhaseRolledIn();
        }
        break;
    case PHASE_COVER_THROW_GRENADE:
        SetInCover(TRUE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetNextPhaseRolledIn();
            m_CharacterBase.CancelAutofire();
        }
        break;
    case PHASE_COVER_ROLL_OUT:
        SetInCover(TRUE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetRolledOut(TRUE);
            newPhase = GetNextPhaseRolledOut();
        }
        break;
    case PHASE_COVER_OUT_GRENADE:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetNextPhaseRolledOut();
        }
        break;
    case PHASE_COVER_OUT_IDLE:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        m_DelayTillNextAction -= DeltaTime;
        newPhase = GetNextPhaseRolledOut();
        if( newPhase == PHASE_COVER_OUT_IDLE && 
            !m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_NONE;
        }
        break;
    case PHASE_COVER_OUT_SCAN:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        newPhase = GetNextPhaseRolledOut();
        if( newPhase == PHASE_COVER_OUT_SCAN && 
            !m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_NONE;
        }
        break;
    case PHASE_COVER_FULL_AUTO:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        m_CharacterBase.SetCrazyFire(TRUE);
        if( m_CharacterBase.IsReloading() || m_AutofireCanceled )
        {
            m_AutofireCanceled = FALSE;
            newPhase = GetNextPhaseRolledOut();
            m_TimeSinceLastFullAuto = 0.0f;
        }
        else if( m_CharacterBase.GetTargetGuid() &&
                 distToTargetSqr <= (m_CharacterBase.GetShortMeleeRange()*m_CharacterBase.GetShortMeleeRange()) &&
                 m_TimeSinceLastMelee >= k_MinTimeBetweenMelee )
        {
            newPhase = GetNextPhaseRolledOut();
            m_TimeSinceLastFullAuto = 0.0f;
        }
        else if( m_CharacterBase.GetFriendlyBlocksTarget() )
        {
            newPhase = GetNextPhaseRolledOut();
            m_TimeSinceLastFullAuto = 0.0f;
        }
        else if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_COVER_FULL_AUTO;
            m_TimeSinceLastFullAuto = 0.0f;
        }
        break;
    case PHASE_COVER_ROLL_IN:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetRolledOut(FALSE);
            newPhase = GetNextPhaseRolledIn();
        }
        break;
    case PHASE_COVER_EXIT_COVER:
        SetInCover(TRUE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            SetInCover(FALSE);
            newPhase = GetNextPhaseOutOfCover();
        }
        break;
    case PHASE_COVER_ALIGN_FOR_MELEE:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() ||
            m_TimeInPhase >= k_MinTimeAligning )
        {
            newPhase = PHASE_COVER_MELEE;
        }
        break;
    case PHASE_COVER_MELEE:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        if( m_CharacterBase.GetGoalCompleted() )
        {            
            newPhase = GetNextPhaseOutOfCover();
            m_TimeSinceLastMelee = 0.0f;
        }
        break;
    case PHASE_COVER_FACE_FORWARD:
        SetInCover(TRUE);
        SetRolledOut(TRUE);
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetNextPhaseRolledOut();
        }
        break;
    case PHASE_COVER_STAND_AND_SHOOT:
        SetInCover(FALSE);
        SetRolledOut(FALSE);
        m_CharacterBase.CheckShooting();
        newPhase = GetNextPhaseOutOfCover();
        // if stand and shoot, then clear it we are already doing that.
        if( newPhase == PHASE_COVER_STAND_AND_SHOOT )
        {
            newPhase = PHASE_NONE;
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    };

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

#ifndef X_RETAIL
void character_cover_state::OnDebugRender()
{
    draw_Sphere(m_RolledOutPosition,50.0f);

    vector3 straightAhead(0.0f,0.0f,1.0f);
    straightAhead.RotateY(m_RolledOutFacing);
    straightAhead.NormalizeAndScale(100.0f);
    draw_Sphere(m_RolledOutPosition+straightAhead,50.0f,XCOLOR_GREEN);
}
#endif

//=========================================================================

void character_cover_state::ChangePhase( s32 newPhase )
{
    object* nodeObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetCurrentCover());

    object* ourNodeObject = g_ObjMgr.GetObjectByGuid(m_CurrentCover);
    
    char AnimNameFromPhase[32];
    switch( newPhase)
    {    
    case PHASE_COVER_GOTO_COVER:                
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
    case PHASE_COVER_FACE_EXACT:              
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
    case PHASE_COVER_IDLE:                   
    case PHASE_COVER_OUT_SCAN:                   
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              /*loco::ANIM_FLAG_END_STATE_HOLD |*/ loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_RESTART_IF_SAME_ANIM );
        break;
    case PHASE_COVER_ENTER_COVER:                   
        {        
            m_TimeSinceLastEnterCover = 0.0f;
            UpdateUsedDelays();
            if ( ourNodeObject && ourNodeObject->IsKindOf( cover_node::GetRTTI() ))
            {   
                cover_node &coverNode = cover_node::GetSafeType( *ourNodeObject );           
                radian yawDiff = x_MinAngleDiff( m_CharacterBase.GetLocoPointer()->GetYaw(),coverNode.GetNPCFacing() );
                m_CharacterBase.SetAnimYawDelta( -yawDiff );
            }
        }
    case PHASE_COVER_ROLL_OUT:                   
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              loco::ANIM_FLAG_INTERRUPT_BLEND,  0 );
        break;
    case PHASE_COVER_ROLL_IN:                   
        m_TimeSinceLastEnterCover = 0.0f;
    case PHASE_COVER_EXIT_COVER:                   
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              /*loco::ANIM_FLAG_END_STATE_HOLD |*/ loco::ANIM_FLAG_TURN_OFF_AIMER );
        break;
    case PHASE_COVER_THROW_GRENADE:                
    case PHASE_COVER_OUT_GRENADE:                
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
    case PHASE_COVER_FULL_AUTO:                
        if( m_DoAutofireDialog )
        {        
            m_CharacterBase.PlayDialog( character::DIALOG_COVER );
            m_DoAutofireDialog = FALSE;
        }
    case PHASE_COVER_SHOOT:                
    case PHASE_COVER_OUT_IDLE:                
        m_CharacterBase.SetWantsToAim(TRUE);
        GetAnimNameFromPhase(newPhase,AnimNameFromPhase);
        m_CharacterBase.SetPlayAnimationGoal( AnimNameFromPhase,
                                              m_CoverAnimGroupHandle.GetName(), 
                                              DEFAULT_BLEND_TIME,
                                              /*loco::ANIM_FLAG_END_STATE_HOLD | */loco::ANIM_FLAG_RESTART_IF_SAME_ANIM );
        break;
    case PHASE_COVER_ALIGN_FOR_MELEE:                
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetTurnToTargetGoal( m_CharacterBase.GetTargetGuid() );
        break;
    case PHASE_COVER_MELEE:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_SHORT);
        break;
    case PHASE_COVER_GOTO_ROLLED_OUT:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetGotoLocationGoal(m_RolledOutPosition,loco::MOVE_STYLE_NULL,0.0f,TRUE);
        break;
    case PHASE_COVER_FACE_FORWARD:
        {        
            m_CharacterBase.SetWantsToAim(TRUE);
            vector3 facingLocation(0.0f,m_RolledOutFacing);
            facingLocation.NormalizeAndScale(100.0f);
            facingLocation += m_RolledOutPosition;
            m_CharacterBase.SetTurnToLocationGoal(facingLocation);
        }
        break;
    case PHASE_COVER_STAND_AND_SHOOT:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetIdleGoal();
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

character_state::states character_cover_state::UpdateState( f32 DeltaTime )
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
        cover_node &coverNode = cover_node::GetSafeType( *nodeObject );
        if( (m_LeaveCoverCondition == LEAVE_COVER_WHEN_DAMAGED && 
             m_TookPainThisTick)  ||
            (m_LeaveCoverCondition == LEAVE_COVER_WHEN_CAN_REACH_TARGET && 
             m_CharacterBase.CanPathToTarget())/* ||
            (m_LeaveCoverCondition == LEAVE_COVER_WHEN_BROKEN &&
             !m_CharacterBase.GetCoverIsValid()) */)
        {
            coverNode.InvalidateNode();
            m_DesiredState = m_CharacterBase.GetStateFromAwareness();
            if( m_DesiredState == STATE_COVER )
            {
                m_DesiredState = STATE_NULL;
            }
        }
    }
    
    if( !m_InCover && m_DesiredState != STATE_NULL )
    {
        return m_DesiredState;
    }
    return STATE_NULL;
}

//=========================================================================

xbool character_cover_state::GetRolledOutPosition( guid coverNode, vector3& rolledOutPos )
{
    object *coverObject = g_ObjMgr.GetObjectByGuid(coverNode);
    if( coverObject && coverObject->IsKindOf(cover_node::GetRTTI()) )
    {    
        cover_node &coverNode = cover_node::GetSafeType( *coverObject );
        const char* coverAnimGroupName = coverNode.GetAnimGroupName( m_CharacterBase.GetType(), m_CharacterBase.GetLogicalName() );
        if( coverAnimGroupName == NULL )
        {
            return FALSE;
        }

        rolledOutPos = coverNode.GetPosition();
        // let's get the animation for rolling out.
        const anim_group::handle& hAnimGroup = m_CoverAnimGroupHandle;
        const anim_group* pAnimGroup = hAnimGroup.GetPointer();
        if( pAnimGroup )
        {                    
            // Lookup roll out animation
            char  RollOutAnimName[64];
            char weaponPrefix[4];
            weaponPrefix[0] = 0;
            new_weapon* pWeapon = m_CharacterBase.GetCurrentWeaponPtr();
            if( pWeapon )
            {
                x_strcpy( weaponPrefix,new_weapon::GetWeaponPrefixFromInvType2( pWeapon->GetInvenItem()) );
            }    
            if( weaponPrefix[0] != 0 )
            {    
                x_strcpy(RollOutAnimName,xfs("%s_",weaponPrefix));
                x_strcat(RollOutAnimName,"COVER_ROLL_OUT");
            }
            else
            {
                x_strcpy(RollOutAnimName,"COVER_ROLL_OUT");
            }
            s32 iRollOutAnim = pAnimGroup->GetAnimIndex( RollOutAnimName );
            if( iRollOutAnim != -1 )
            {
                // Lookup anim info
                const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( iRollOutAnim );
                vector3 animTranslation = AnimInfo.GetTotalTranslation();
                animTranslation.RotateY(coverNode.GetL2W().GetRotation().Yaw );
                rolledOutPos -= animTranslation;
            }
            return TRUE;
        }
    }        
    return FALSE;
}

//=========================================================================

void character_cover_state::SetCurrentCover( guid Cover )
{
    // Set cover
    m_CurrentCover = Cover;
    
    // Can't be in or rolled out
    SetInCover( FALSE );
    SetRolledOut( FALSE );

    // set the correct anim group.
    object *coverObject = g_ObjMgr.GetObjectByGuid(Cover);
    if( coverObject && coverObject->IsKindOf(cover_node::GetRTTI()) )
    {    
        cover_node &coverNode = cover_node::GetSafeType( *coverObject );
        const char* coverAnimGroupName = coverNode.GetAnimGroupName( m_CharacterBase.GetType(), m_CharacterBase.GetLogicalName() );
        if( coverAnimGroupName == NULL )
        {
            return;
        }
        m_CoverAnimGroupHandle.SetName( coverAnimGroupName );
        m_RolledOutPosition = coverNode.GetPosition();
        m_RolledOutFacing = coverNode.GetL2W().GetRotation().Yaw;

        // we want the y to be our Y.
        m_RolledOutPosition.Set( m_RolledOutPosition.GetX(),m_CharacterBase.GetPosition().GetY(),m_RolledOutPosition.GetZ() );
        // let's get the animation for rolling out.
        const anim_group::handle& hAnimGroup = m_CoverAnimGroupHandle;
        const anim_group* pAnimGroup = hAnimGroup.GetPointer();
        if( pAnimGroup )
        {                    
            // Lookup roll out animation
            char  RollOutAnimName[64];
            char weaponPrefix[4];
            weaponPrefix[0] = 0;
            new_weapon* pWeapon = m_CharacterBase.GetCurrentWeaponPtr();
            if( pWeapon )
            {
                x_strcpy( weaponPrefix,new_weapon::GetWeaponPrefixFromInvType2( pWeapon->GetInvenItem()) );
            }    
            if( weaponPrefix[0] != 0 )
            {    
                x_strcpy(RollOutAnimName,xfs("%s_",weaponPrefix));
                x_strcat(RollOutAnimName,"COVER_ROLL_OUT");
            }
            else
            {
                x_strcpy(RollOutAnimName,"COVER_ROLL_OUT");
            }
            s32 iRollOutAnim = pAnimGroup->GetAnimIndex( RollOutAnimName );
            if( iRollOutAnim != -1 )
            {
                // Lookup anim info
                const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( iRollOutAnim );
                vector3 animTranslation = AnimInfo.GetTotalTranslation();
                animTranslation.RotateY(coverNode.GetL2W().GetRotation().Yaw );
                m_RolledOutPosition -= animTranslation;
            }
        }
    }        
}
                
//=========================================================================

void character_cover_state::SetInCover( xbool inCover )
{
    if( ((xbool)m_InCover) != inCover )
    {    
        m_InCover = inCover;
        if( m_InCover == TRUE &&
            !m_AutofireRequested &&
            m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_ACQUIRING_TARGET &&
            !m_CharacterBase.HasFullClip() )
        {        
            m_CharacterBase.ReloadWeapon();
        }
    }
}

//=========================================================================

void character_cover_state::SetRolledOut( xbool rolledOut )
{
    if( ((xbool)m_RolledOut) != rolledOut )
    {
        m_RolledOut = rolledOut;
    }
}

//=========================================================================

void character_cover_state::CoverRequestDialogDone()
{
    if( m_AutofireRequested )
    {
        m_DoAutofireDialog = TRUE;
    }
}

//=========================================================================

void character_cover_state::OnThink( void )
{
    object *nodeObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetCurrentCover() );
    if( nodeObject && nodeObject->IsKindOf(cover_node::GetRTTI()) )
    {   
        cover_node &coverNode = cover_node::GetSafeType( *nodeObject );           
        object *checkObject = g_ObjMgr.GetObjectByGuid( coverNode.GetCheckPoint() );
        if( !checkObject )
        {
            m_HasClearGrenadeThrow = m_CharacterBase.HasClearGrenadeThrow();
        }
        else
        {
            m_HasClearGrenadeThrow = m_CharacterBase.HasClearGrenadeThrow( TRUE, checkObject->GetPosition() );
        }

        // we will only throw grenades if our target has an ally within 7 meters.
        object *targetObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetTargetGuid() );
        if( targetObject && targetObject->IsKindOf(character::GetRTTI()) )
        {
            character& characterObject = character::GetSafeType( *targetObject );
            m_HasClearGrenadeThrow &= characterObject.HasAlliesWithin( k_MinDistForGrenadeClusterSqr );
        }

//        UpdateBlocksLOF();
    }
    character_state::OnThink();
}

//=========================================================================

void character_cover_state::UpdateBlocksLOF()
{
    // rolled out block? 
    // how about rolled in?
    // are we in the players LOF or
    // are we moving into the players LOF.
    // we only care if we are allies.
    player* pPlayer = SMP_UTIL_GetActivePlayer();

    f32 yValue = m_CharacterBase.GetPosition().GetY();
    m_bRolledOutBlocksLOF = FALSE;
    m_bRolledInBlocksLOF = FALSE;
    if( pPlayer && m_CharacterBase.IsFriendlyFaction(pPlayer->GetFaction()) )
    {
        // do we care if the player is shooting? For now... no.
        // Let's start with a very simple player aim/bbox intersect test?
        vector3 playerFacing(pPlayer->GetPitch(),pPlayer->GetYaw());
        playerFacing.NormalizeAndScale(750.0f); // we will care out to 5 meters.
        bbox myBbox = m_CharacterBase.GetLocoPointer()->m_Physics.GetBBox();

        object *nodeObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetCurrentCover() );
        vector3 rolledInPos( nodeObject->GetPosition().GetX(), yValue, nodeObject->GetPosition().GetZ() );
        vector3 rolledOutPos( m_RolledOutPosition.GetX(), yValue, m_RolledOutPosition.GetZ() );

        f32 t; // t is how far along the path we impact.
        xbool intersect;
        myBbox.Translate( rolledOutPos );
        intersect = myBbox.Intersect( t ,pPlayer->GetPositionWithOffset(actor::OFFSET_EYES),pPlayer->GetPositionWithOffset(actor::OFFSET_EYES)+playerFacing );
        m_bRolledOutBlocksLOF = ( intersect && 
                                t > 0.0f &&
                                t <= 1.0f);

        myBbox = m_CharacterBase.GetLocoPointer()->m_Physics.GetBBox();
        // if we translate very little, assume we are sinking down instead.
        if( (rolledInPos - rolledOutPos).LengthSquared() < k_MinRollDistSqr )
        {        
            myBbox.Deflate( 0.0f, myBbox.GetSize().GetY()/4.0f, 0.0f );
            myBbox.Translate( vector3(0.0f, -myBbox.GetSize().GetY()/2.0f, 0.0f) );
        }
        myBbox.Translate( rolledInPos );
        intersect = myBbox.Intersect( t ,pPlayer->GetPositionWithOffset(actor::OFFSET_EYES),pPlayer->GetPositionWithOffset(actor::OFFSET_EYES)+playerFacing );
        m_bRolledInBlocksLOF = ( intersect && 
            t > 0.0f &&
            t <= 1.0f);
    }
}

//=========================================================================

xbool character_cover_state::OnPlayFullBodyImpactAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags )
{
    // Try play anim
    xbool bResult = character_state::OnPlayFullBodyImpactAnim( AnimType, BlendTime, Flags );
    
    // If anim was started, flag the we are no longer at cover
    if( bResult )
        SetCurrentCover( 0 );
        
    return bResult;        
}

//=========================================================================

void character_cover_state::OnCoverChanged( guid NewCover )
{
    (void)NewCover;
    // Clear cover if it's changed
//    if( m_CurrentCover != NewCover )
//        SetCurrentCover( 0 );
}

//=========================================================================

void character_cover_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader (  "CoverState",  "Different variables that effect the way that the character behaves when standing still.", 0 );
    List.PropEnumFloat  ( "CoverState\\TimeTillBored", "Amount of Time to stay in state with no new sounds heard.", 0 ) ;
    List.PropEnumInt    ( "CoverState\\Shoot Percent", "Chance we will shoot.", PROP_TYPE_EXPOSE ) ;
    List.PropEnumInt    ( "CoverState\\Grenade Percent", "Chance we will throw a grenade.", PROP_TYPE_EXPOSE ) ;
    List.PropEnumFloat  ( "CoverState\\MinTimeBetweenCombatPopup", "Min time we stay down while fighting.", PROP_TYPE_EXPOSE ) ;
    List.PropEnumFloat  ( "CoverState\\MaxTimeBetweenCombatPopup", "Max time we stay down while fighting.", PROP_TYPE_EXPOSE ) ;
    List.PropEnumEnum   ( "CoverState\\Move Style", loco::GetMoveStyleEnum(), "What style of movement to use to get to location.", PROP_TYPE_MUST_ENUM | PROP_TYPE_EXPOSE );    
}

//=========================================================================

xbool character_cover_state::OnProperty ( prop_query& rPropQuery )
{
    if (rPropQuery.VarFloat("CoverState\\TimeTillBored", m_TimeTillBored))
        return TRUE;
    else if (rPropQuery.VarInt("CoverState\\Shoot Percent", m_ShootWeight))
        return TRUE;
    else if (rPropQuery.VarInt("CoverState\\Grenade Percent", m_GrenadeWeight))
        return TRUE;
    else if (rPropQuery.VarFloat("CoverState\\MinTimeBetweenCombatPopup", m_ActionDelayMin))
        return TRUE;
    else if (rPropQuery.VarFloat("CoverState\\MaxTimeBetweenCombatPopup", m_ActionDelayMax))
        return TRUE;
    else if ( rPropQuery.IsVar( "CoverState\\Move Style") )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( loco::GetMoveStyleName( m_MoveStyle ) ); 

            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            m_MoveStyle = loco::GetMoveStyleByName  ( pString) ;
            return( TRUE );
        }
    }
    return FALSE ;
}

//=========================================================================

const char*character_cover_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_COVER_GOTO_COVER:
        return "PHASE_COVER_GOTO_COVER";
    	break;
    case PHASE_COVER_FACE_EXACT:
        return "PHASE_COVER_FACE_EXACT";
    	break;
    case PHASE_COVER_ENTER_COVER:
        return "PHASE_COVER_ENTER_COVER";
        break;
    case PHASE_COVER_IDLE:
        return "PHASE_COVER_IDLE";
    	break;
    case PHASE_COVER_SHOOT:
        return "PHASE_COVER_SHOOT";
    	break;
    case PHASE_COVER_THROW_GRENADE:
        return "PHASE_COVER_THROW_GRENADE";
        break;
    case PHASE_COVER_ROLL_OUT:
        return "PHASE_COVER_ROLL_OUT";
    	break;
    case PHASE_COVER_OUT_IDLE:
        return "PHASE_COVER_OUT_IDLE";
        break;
    case PHASE_COVER_OUT_GRENADE:
        return "PHASE_COVER_OUT_GRENADE";
        break;
    case PHASE_COVER_OUT_SCAN:
        return "PHASE_COVER_OUT_SCAN";
        break;
    case PHASE_COVER_FULL_AUTO:
        return "PHASE_COVER_FULL_AUTO";
        break;
    case PHASE_COVER_ROLL_IN:
        return "PHASE_COVER_ROLL_IN";
        break;
    case PHASE_COVER_EXIT_COVER:
        return "PHASE_COVER_EXIT_COVER";
    	break;
    case PHASE_COVER_ALIGN_FOR_MELEE:
        return "PHASE_COVER_ALIGN_FOR_MELEE";
        break;
    case PHASE_COVER_MELEE:
        return "PHASE_COVER_MELEE";
        break;
    case PHASE_COVER_STAND_AND_SHOOT:
        return "PHASE_COVER_STAND_AND_SHOOT";
        break;
    case PHASE_COVER_GOTO_ROLLED_OUT:
        return "PHASE_COVER_GOTO_ROLLED_OUT";
        break;
    case PHASE_COVER_FACE_FORWARD:
        return "PHASE_COVER_FACE_FORWARD";
        break;
    }
    return character_state::GetPhaseName(thePhase);
}
