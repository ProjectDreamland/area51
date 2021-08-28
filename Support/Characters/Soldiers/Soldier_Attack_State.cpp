#include "Soldier_Attack_State.hpp"
#include "Characters\Character.hpp"
#include "Characters\god.hpp"
#include "navigation\CoverNode.hpp"

//=========================================================================
// CONSTs
//=========================================================================

const f32 k_MinPhaseTime            = 2.0f;
const f32 k_MaxPhaseTime            = 4.0f;
const f32 k_MinTimeBetweenBursts    = 1.0f;
const f32 k_MinTimeBetweenRethinks  = 2.0f;
const f32 k_MinTimeBetweenRangeRethinks = 5.0f;
const f32 k_MinTimeBetweenGrenadeThrows = 10.0f;
const f32 k_MinTimeBetweenMoves   = 5.0f;
const f32 k_MinDistToCover          = 50.0f;


//=========================================================================
// Character ATTACK STATE
//=========================================================================

soldier_attack_state::soldier_attack_state( character& ourCharacter, character_state::states State ) :
    character_attack_state(ourCharacter, State)
{
}

//=========================================================================

soldier_attack_state::~soldier_attack_state()
{
}

//=========================================================================

void soldier_attack_state::OnInit( void )
{
    m_OurCoverDesire = SEEK_COVER_ALWAYS;
    m_OptimalFiringDistance     = 1600.0f;
    character_attack_state::OnInit();
}

//=========================================================================

void soldier_attack_state::OnEnter( void )
{
    character_attack_state::OnEnter();
    m_TimeSinceLastRethink = k_MinTimeBetweenRethinks;
    m_TimeSinceLastRangeRethink = k_MinTimeBetweenRangeRethinks;
    m_TimeSinceLastMove = k_MinTimeBetweenMoves;
    m_ForceRethink = FALSE;
    m_ForceRangeRethink = FALSE;
    m_CouldSeeTarget = TRUE;
    m_EvadePercent = 50;
    m_StandShootPercent = 20;
    m_PainStrafePercent = 50;
    m_TargetRangeBracket = GetTargetRangeBracket();
}

//=========================================================================
xbool soldier_attack_state::OnExit()
{
    return character_attack_state::OnExit();
}

//=========================================================================

s32 soldier_attack_state::UpdatePhase( f32 DeltaTime )
{
    s32 newPhase = PHASE_NONE;  
    xbool completedCurrentPhase = FALSE;
    vector3 toTarget;
    m_TimeSinceLastRethink += DeltaTime;
    m_TimeSinceLastMove += DeltaTime;

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        completedCurrentPhase = TRUE;
        break;
    case PHASE_BO_ATTACK_SURPRISED:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            completedCurrentPhase = TRUE;
        }
        break;
    case PHASE_BO_ATTACK_STRAFE_LEFT:
        m_CharacterBase.CheckShooting();
        toTarget = m_CharacterBase.GetToTarget();
        toTarget.GetY() = 0.0f;
        toTarget.Normalize();
        toTarget = toTarget.Cross( vector3(0.0f,-1.0f,0.0f) );
        toTarget.Scale( 100.0f );
        toTarget += m_CharacterBase.GetPosition();
        m_CharacterBase.UpdateGoalLocation(toTarget);
        if( m_CharacterBase.GetGoalCompleted() || m_TimeInPhase > m_MinPhaseTime || !m_CharacterBase.GetCanMoveLeft() )
        {
            completedCurrentPhase = TRUE;
        }
        break;
    case PHASE_BO_ATTACK_STRAFE_RIGHT:
        m_CharacterBase.CheckShooting();

        toTarget = m_CharacterBase.GetToTarget();
        toTarget.GetY() = 0.0f;
        toTarget.Normalize();
        toTarget = toTarget.Cross( vector3(0.0f,1.0f,0.0f) );
        toTarget.Scale( 100.0f );
        toTarget += m_CharacterBase.GetPosition();
        m_CharacterBase.UpdateGoalLocation(toTarget);
        if( m_CharacterBase.GetGoalCompleted() || m_TimeInPhase > m_MinPhaseTime || !m_CharacterBase.GetCanMoveRight()    )
        {
            completedCurrentPhase = TRUE;
        }
        break;
    case PHASE_BO_ATTACK_ADVANCE:
    case PHASE_BO_ATTACK_BACKUP:
        m_CharacterBase.CheckShooting();
        if( m_CharacterBase.GetGoalCompleted() || m_TimeInPhase > m_MinPhaseTime )
        {
            m_TimeSinceLastMove = 0.0f;
            completedCurrentPhase = TRUE;
        }
        break;
    case PHASE_BO_ATTACK_ALIGN_FOR_GRENADE_THROW:
        m_ForceRethink = FALSE;
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_BO_ATTACK_THROW_GRENADE;
        }
        break;
    case PHASE_BO_ATTACK_THROW_GRENADE:
    case PHASE_BO_ATTACK_EVADE_LEFT:
    case PHASE_BO_ATTACK_EVADE_RIGHT:
        m_ForceRethink = FALSE;
        if( m_CharacterBase.GetGoalCompleted() )
        {
            completedCurrentPhase = TRUE;
        }
        break;
    case PHASE_BO_ATTACK_MELEE:
    case PHASE_BO_ATTACK_MELEE_BACK_RIGHT:
    case PHASE_BO_ATTACK_MELEE_BACK_LEFT:
        m_ForceRethink = FALSE;
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_CharacterBase.AllowShootingNow();
//            completedCurrentPhase = TRUE;
            newPhase = PHASE_BO_ATTACK_BACKUP;
        }
        break;
    case PHASE_BO_ATTACK_STAND_SHOOT:
        if( m_CharacterBase.IsReloading() )
        {
            newPhase = PHASE_BO_ATTACK_CROUCH_SHOOT;
        }
    case PHASE_BO_ATTACK_CROUCH_SHOOT:
        m_CharacterBase.CheckShooting();        
        if( m_CharacterBase.GetTargetGuid() )
        {        
            m_TimeSinceLastRangeRethink += DeltaTime;   
            // we will always consider a rethink if we are outside optimal range.
            if( m_TargetRangeBracket != TARGET_RANGE_OPTIMAL && m_TimeSinceLastRangeRethink > k_MinTimeBetweenRangeRethinks )   
            {
                m_ForceRangeRethink = TRUE;
            }
            // if I can't see or shoot at my target, then re-think.
            if( !m_CharacterBase.CanSeeTarget() )
            {
                m_ForceRethink = TRUE;        
            }
        }
        break;
    case PHASE_BO_ATTACK_CLOSE_FOR_MELEE:
        m_CharacterBase.CheckShooting();

        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( m_CharacterBase.GetGoalSucceeded() )
            {            
                radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
                radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
                radian m_DiffToTargetYawRelative = x_MinAngleDiff(locoYaw, TargetYaw);
                radian m_DiffToTargetYaw = x_abs(m_DiffToTargetYawRelative);

                if( m_DiffToTargetYawRelative > R_135 &&
                    m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_LEFT) )
                {
                    newPhase = PHASE_BO_ATTACK_MELEE_BACK_LEFT;
                }
                else if( m_DiffToTargetYawRelative < -R_135 &&
                    m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_RIGHT) )
                {
                    newPhase = PHASE_BO_ATTACK_MELEE_BACK_RIGHT;
                }
                else if( x_abs(m_DiffToTargetYaw) >= k_MinAngleToMeleeFace )
                {            
                    newPhase = PHASE_BO_ATTACK_ALIGN_FOR_MELEE;
                }
                else
                {
                    newPhase = PHASE_BO_ATTACK_MELEE;
                }
            }            
            else
            {
                completedCurrentPhase = TRUE;
            }
        }
        else if ( m_TimeInPhase > m_MinPhaseTime )
        {
            completedCurrentPhase = TRUE;
        }
        break;
    case PHASE_BO_ATTACK_ALIGN_FOR_MELEE:
        {        
            m_ForceRethink = FALSE;
            radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
            radian m_DiffToTargetYawRelative = x_MinAngleDiff(locoYaw, TargetYaw);

            if( m_DiffToTargetYawRelative > R_135 &&
                m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_LEFT) )
            {
                newPhase = PHASE_BO_ATTACK_MELEE_BACK_LEFT;
            }
            else if( m_DiffToTargetYawRelative < -R_135 &&
                m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_RIGHT) )
            {
                newPhase = PHASE_BO_ATTACK_MELEE_BACK_RIGHT;
            }
            else if( m_CharacterBase.GetGoalCompleted() )
            {
                newPhase = PHASE_BO_ATTACK_MELEE;
            }
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    };
    // base phase overrides...
    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE )
    {
        newPhase = basePhase;
    }
    else if( completedCurrentPhase && (m_CurrentPhase >= PHASE_BASE_COUNT || m_CurrentPhase == PHASE_NONE) )
    {
        newPhase = GetNextPhaseNormal();
    }
    else if( m_ForceRethink && (m_CurrentPhase >= PHASE_BASE_COUNT || m_CurrentPhase == PHASE_NONE) )
    {
        newPhase = GetForcedRethinkPhase();
    }
    else if( m_ForceRangeRethink && (m_CurrentPhase >= PHASE_BASE_COUNT || m_CurrentPhase == PHASE_NONE) )
    {
        m_TimeSinceLastRangeRethink = 0.0f;
        newPhase = GetNextPhaseNormal();
    }

    // clear all flags for this tick
    m_ForceRethink      = FALSE;
    m_ForceRangeRethink = FALSE;

    return newPhase;
}

//=========================================================================

void soldier_attack_state::ChangePhase( s32 newPhase )
{
    vector3 toTarget;
    m_MinPhaseTime = x_frand(k_MinPhaseTime, k_MaxPhaseTime);

    switch( newPhase ) 
    {
    case PHASE_BO_ATTACK_SURPRISED:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_SPOT_TARGET);
        break;
    case PHASE_BO_ATTACK_STRAFE_LEFT:
        m_CharacterBase.SetWantsToAim(TRUE);
        toTarget = m_CharacterBase.GetToTarget();
        toTarget.GetY() = 0.0f;
        toTarget.Normalize();
        toTarget = toTarget.Cross( vector3(0.0f,-1.0f,0.0f) );
        toTarget.Scale( 100.0f );
        toTarget += m_CharacterBase.GetPosition();
        m_CharacterBase.SetGotoLocationGoal(toTarget);
    	break;
    case PHASE_BO_ATTACK_STRAFE_RIGHT:
        m_CharacterBase.SetWantsToAim(TRUE);
        toTarget = m_CharacterBase.GetToTarget();
        toTarget.GetY() = 0.0f;
        toTarget.Normalize();
        toTarget = toTarget.Cross( vector3(0.0f,1.0f,0.0f) );
        toTarget.Scale( 100.0f );
        toTarget += m_CharacterBase.GetPosition();
        m_CharacterBase.SetGotoLocationGoal(toTarget);
    	break;
    case PHASE_BO_ATTACK_STAND_SHOOT:
        m_TimeSinceLastRangeRethink = 0.0f;   
        m_MinPhaseTime = x_frand(k_MinPhaseTime*2.0f, k_MaxPhaseTime*2.0f);
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetIdleGoal();
    	break;
    case PHASE_BO_ATTACK_CROUCH_SHOOT:
        m_TimeSinceLastRangeRethink = 0.0f;   
        m_CharacterBase.SetWantsToAim(TRUE);
        m_MinPhaseTime = x_frand(k_MinPhaseTime*2.0f, k_MaxPhaseTime*2.0f);
        m_CharacterBase.GetLocoPointer()->SetMoveStyle( loco::MOVE_STYLE_CROUCHAIM );
        m_CharacterBase.SetIdleGoal();
    	break;
    case PHASE_BO_ATTACK_ADVANCE:
        if( m_CurrentPhase != PHASE_BO_ATTACK_ADVANCE )
        {        
            m_CharacterBase.PlayDialog( character::DIALOG_RUSH );        
        }
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetGotoTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,m_OptimalFiringDistance);
    	break;
    case PHASE_BO_ATTACK_BACKUP:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetRetreatFromTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,m_OptimalFiringDistance);
    	break;
    case PHASE_BO_ATTACK_ALIGN_FOR_GRENADE_THROW:
        m_CharacterBase.SetTurnToTargetGoal(m_CharacterBase.GetTargetGuid());
    	break;
    case PHASE_BO_ATTACK_THROW_GRENADE:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_GRENADE_THROW_LONG);
    	break;
    case PHASE_BO_ATTACK_EVADE_LEFT:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_EVADE_LEFT);
        break;
    case PHASE_BO_ATTACK_EVADE_RIGHT:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_EVADE_RIGHT);
        break;
    case PHASE_BO_ATTACK_CLOSE_FOR_MELEE:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetGotoTargetGoal( m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f),loco::MOVE_STYLE_NULL,m_CharacterBase.GetShortMeleeRange() );
    	break;
    case PHASE_BO_ATTACK_ALIGN_FOR_MELEE:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetTurnToTargetGoal(m_CharacterBase.GetTargetGuid());
    	break;
    case PHASE_BO_ATTACK_MELEE:
        {
            radian toTargetYaw = m_CharacterBase.GetToTarget().GetYaw();
            radian ourYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            m_CharacterBase.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            m_CharacterBase.SetWantsToAim(TRUE);
            m_TimeSinceLastMove = k_MinTimeBetweenMoves;
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_SHORT);
        }
    	break;
    case PHASE_BO_ATTACK_MELEE_BACK_LEFT:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_TimeSinceLastMove = k_MinTimeBetweenMoves;
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_BACK_LEFT);
        break;
    case PHASE_BO_ATTACK_MELEE_BACK_RIGHT:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_TimeSinceLastMove = k_MinTimeBetweenMoves;
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_BACK_RIGHT);
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
// return the range bracket to our target [10/12/2003]
soldier_attack_state::eTargetRangeBrackets soldier_attack_state::GetTargetRangeBracket()
{
    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();
    f32 optimalRangeSqr = ( m_OptimalFiringDistance * m_OptimalFiringDistance );
    f32 meleeDistSqr    = ( m_CharacterBase.GetShortMeleeRange() * m_CharacterBase.GetShortMeleeRange() );
    
    // optimal is within 20% either way of our optimal range.
    if( distToTargetSqr > optimalRangeSqr * 2.25f ) //1.5*1.5
    {
        return TARGET_RANGE_FURTHER_THAN_OPTIMAL;
    }
    else if( distToTargetSqr > optimalRangeSqr * 0.25f) // 0.5*0.5
    {
        return TARGET_RANGE_OPTIMAL;
    }
    else if( distToTargetSqr > meleeDistSqr )
    {
        return TARGET_RANGE_CLOSER_THAN_OPTIMAL;
    }
    else
    {
        return TARGET_RANGE_WITHIN_MELEE_POSSIBILITY;
    }
}

//=========================================================================

s32 soldier_attack_state::GetTargetVisiblePhase( )
{    
    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();   
    // our target is visible, but our shot is blocked.
    if( !m_CharacterBase.CanShootAtTarget() )
    {
        if( m_CurrentPhase == PHASE_BO_ATTACK_CROUCH_SHOOT )
        {
            return PHASE_BO_ATTACK_STAND_SHOOT;
        }
        else if( m_CurrentPhase == PHASE_BO_ATTACK_STAND_SHOOT )
        {
            return GetAnyMovementPhase();
        }
    }

    // if the target is too far we may close ( allow 20% buffer )
    if( m_TargetRangeBracket == TARGET_RANGE_FURTHER_THAN_OPTIMAL )
    {
        // at 2x optimal we definately close, otherwise it's a % chance.
        f32 closeCutoff = x_frand( m_OptimalFiringDistance, m_OptimalFiringDistance*2.0f );
        if( distToTargetSqr > closeCutoff*closeCutoff && m_TimeSinceLastMove > k_MinTimeBetweenMoves)
        {
            return PHASE_BO_ATTACK_ADVANCE;
        }
    }
    
    // if target very close we may melee
    if( m_TargetRangeBracket == TARGET_RANGE_WITHIN_MELEE_POSSIBILITY )
    {
        // first should we melee?
        f32 closeCutoff = x_frand( m_CharacterBase.GetShortMeleeRange(), m_CharacterBase.GetShortMeleeRange()*2.0f );
        if( distToTargetSqr < closeCutoff*closeCutoff )
        {            
            if( distToTargetSqr <= m_CharacterBase.GetShortMeleeRange() * m_CharacterBase.GetShortMeleeRange() )
            {            
                radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
                radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
                radian m_DiffToTargetYawRelative = x_MinAngleDiff(locoYaw, TargetYaw);
                radian m_DiffToTargetYaw = x_abs(m_DiffToTargetYawRelative);

                if( m_DiffToTargetYawRelative > R_135 &&
                    m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_LEFT) )
                {
                    return PHASE_BO_ATTACK_MELEE_BACK_LEFT;
                }
                else if( m_DiffToTargetYawRelative < -R_135 &&
                    m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_RIGHT) )
                {
                    return PHASE_BO_ATTACK_MELEE_BACK_RIGHT;
                }
                else if( m_DiffToTargetYaw > k_MinAngleToMeleeFace )
                {            
                    return PHASE_BO_ATTACK_ALIGN_FOR_MELEE;
                }
                else
                {
                    return PHASE_BO_ATTACK_MELEE;
                }
            }
            else
            {            
                return PHASE_BO_ATTACK_CLOSE_FOR_MELEE;
            }
        }
    }
    
    // if closer than optimal we may backup.
    if( m_TargetRangeBracket == TARGET_RANGE_CLOSER_THAN_OPTIMAL || m_TargetRangeBracket == TARGET_RANGE_WITHIN_MELEE_POSSIBILITY )
    {        
        // do we want to backup instead?
        f32 closeCutoff = x_frand( m_CharacterBase.GetShortMeleeRange(), m_OptimalFiringDistance );
        if( distToTargetSqr < closeCutoff*closeCutoff && 
            m_TimeSinceLastMove > k_MinTimeBetweenMoves &&
            m_CurrentPhase != PHASE_BO_ATTACK_CLOSE_FOR_MELEE &&
            m_CurrentPhase != PHASE_BO_ATTACK_ADVANCE )
        {
            return PHASE_BO_ATTACK_BACKUP;
        }       
    }

    // we can see him and decided not to move... let's shoot if we aren't already
    if( m_CurrentPhase != PHASE_BO_ATTACK_STAND_SHOOT && m_CurrentPhase != PHASE_BO_ATTACK_CROUCH_SHOOT)
    {    
        if( x_irand(0,100) <= m_StandShootPercent )
        {
            return PHASE_BO_ATTACK_STAND_SHOOT;   
        }
        else
        {
            return PHASE_BO_ATTACK_CROUCH_SHOOT;   
        }
    }
    return PHASE_NONE;
}

//=========================================================================
// we can't see our target, we will either throw a grenade or advance.

s32 soldier_attack_state::GetTargetHiddenPhase()
{
    // first should we throw a grenade?
    if( g_ObjMgr.GetGameDeltaTime(m_CharacterBase.GetLastGrenadeThrowTime()) > k_MinTimeBetweenGrenadeThrows )
    {
        if( m_CharacterBase.HasClearGrenadeThrow() )
        {
            return PHASE_BO_ATTACK_ALIGN_FOR_GRENADE_THROW;
        }
    }
//    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();
    if( !m_CharacterBase.CanPathToTarget() )
    {
        // see if the target is above us. If so backup.
        if( m_CharacterBase.GetToTarget().GetY() > 100.0f )
        {
            return PHASE_BO_ATTACK_BACKUP;
        }
        else
        {
            // move randomly trying to find the target
            switch( x_irand(0,1) )
            {
            case 0:                
                if( m_CharacterBase.GetCanMoveLeft() )
                {                
                    return PHASE_BO_ATTACK_STRAFE_LEFT;               
                }
            case 1:
                if( m_CharacterBase.GetCanMoveRight() )
                {                
                    return PHASE_BO_ATTACK_STRAFE_RIGHT;
                }
            default:
                return PHASE_BO_ATTACK_CLOSE_FOR_MELEE;
                break;
            }
        }
    }
    else
    {    
        return PHASE_BO_ATTACK_CLOSE_FOR_MELEE;
    }
}

//=========================================================================
//  [10/12/2003] we must try to move.

s32 soldier_attack_state::GetAnyMovementPhase()
{
    f32 distToTargetSqr = m_CharacterBase.GetToTarget().LengthSquared();
    f32 optimalFiringDistanceSqr = m_OptimalFiringDistance * m_OptimalFiringDistance;

    // if the target is too far we may close ( allow 20% buffer )
    if( distToTargetSqr  > optimalFiringDistanceSqr * 1.2f )
    {
        // at 2x optimal we definately close, otherwise it's a % chance.
        f32 twiceOptimalSqr = optimalFiringDistanceSqr * 4.0f;
        f32 closeCutoff = x_frand( optimalFiringDistanceSqr, twiceOptimalSqr );
        if( closeCutoff > distToTargetSqr && m_TimeSinceLastMove > k_MinTimeBetweenMoves )
        {
            return PHASE_BO_ATTACK_ADVANCE;
        }
    }
    // if too close, may back up or close for melee.
    else if( distToTargetSqr < optimalFiringDistanceSqr * 0.8f )
    {
        f32 meleeAttackRadiusSqr = m_CharacterBase.GetShortMeleeRange()* m_CharacterBase.GetShortMeleeRange();
        f32 twiceMeleeAttackRadius = meleeAttackRadiusSqr * 4.0f;
        // first should we melee?
        f32 closeCutoff = x_frand( meleeAttackRadiusSqr, twiceMeleeAttackRadius );
        if( distToTargetSqr < closeCutoff )
        {            
            if( distToTargetSqr <= m_CharacterBase.GetShortMeleeRange() * m_CharacterBase.GetShortMeleeRange() )
            {            
                radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
                radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
                radian m_DiffToTargetYawRelative = x_MinAngleDiff(locoYaw, TargetYaw);
                radian m_DiffToTargetYaw = x_abs(m_DiffToTargetYawRelative);

                if( m_DiffToTargetYawRelative > R_135 &&
                    m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_LEFT) )
                {
                    return PHASE_BO_ATTACK_MELEE_BACK_LEFT;
                }
                else if( m_DiffToTargetYawRelative < -R_135 &&
                    m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_RIGHT) )
                {
                    return PHASE_BO_ATTACK_MELEE_BACK_RIGHT;
                }
                else if( m_DiffToTargetYaw > R_10 )
                {            
                    return PHASE_BO_ATTACK_ALIGN_FOR_MELEE;
                }
                else
                {
                    return PHASE_BO_ATTACK_MELEE;
                }
            }
            else
            {            
                return PHASE_BO_ATTACK_CLOSE_FOR_MELEE;
            }
        }
        
        // do we want to backup instead?
        closeCutoff = x_frand( m_CharacterBase.GetShortMeleeRange(), optimalFiringDistanceSqr );
        if( distToTargetSqr < closeCutoff && m_TimeSinceLastMove > k_MinTimeBetweenMoves )
        {
            return PHASE_BO_ATTACK_BACKUP;
        }       
    }
    // if all else fails, just close on him  
    return GetStrafePhase();
}

s32 soldier_attack_state::GetStrafePhase()
{
    if( x_irand(0,1) )
    {            
        if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_BO_ATTACK_STRAFE_RIGHT;
        }
        else if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_BO_ATTACK_STRAFE_LEFT;
        }
    }
    else
    {
        if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_BO_ATTACK_STRAFE_LEFT;
        }
        else if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_BO_ATTACK_STRAFE_RIGHT;
        }
    }            
    return PHASE_NONE;
}

//=========================================================================
// Return a valid evade if any [10/12/2003]

s32 soldier_attack_state::GetEvadePhase()
{
    if( x_irand(0,1) )
    {            
        if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_BO_ATTACK_EVADE_RIGHT;
        }
        else if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_BO_ATTACK_EVADE_LEFT;
        }
    }
    else
    {
        if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_BO_ATTACK_EVADE_LEFT;
        }
        else if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_BO_ATTACK_EVADE_RIGHT;
        }
    }
    return PHASE_NONE;
}

//=========================================================================
// We were forced to do a rethink [10/12/2003]

s32 soldier_attack_state::GetForcedRethinkPhase()
{
    if( !m_CharacterBase.CanPathToTarget() )
    {
        return PHASE_BO_ATTACK_STAND_SHOOT;
    }

    // change phase if we lose or gain sight of the target
    if( (m_CouldSeeTarget && !m_CharacterBase.CanSeeTarget()) || (!m_CouldSeeTarget && m_CharacterBase.CanSeeTarget()) )
    {       
        return GetNextPhaseNormal();
    }   

    if ( m_ShotAtThisTick )
    {
        // we might want to evade... 
        if( x_irand(1,100) < m_EvadePercent )
        {
            return GetEvadePhase();
        }
    }
    
    if( m_TookPainThisTick )
    {
        // we may want to strafe if we are being hit
        if( x_irand(1,100) < m_PainStrafePercent )
        {
            return GetStrafePhase();
        }
    }

    if( m_HitFriendlyThisTick )
    {
        // if I hit a friendly guy and I was crouching, I will stand.
        if( m_CurrentPhase == PHASE_BO_ATTACK_CROUCH_SHOOT )
        {
            return PHASE_BO_ATTACK_STAND_SHOOT;
        }
        // if I hit a friendly guy and I was standing, I must move.
        else if (m_CurrentPhase == PHASE_BO_ATTACK_STAND_SHOOT )
        {
            return GetAnyMovementPhase();
        }
    }
    
    if( m_HitByFriendlyThisTick )
    {
        // if I was hit by a friendly crouch so he can shoot over me.
        if( m_CurrentPhase == PHASE_BO_ATTACK_STAND_SHOOT )
        {
            return PHASE_BO_ATTACK_CROUCH_SHOOT;
        }
        // if I was hit by a friendly and I was crounching, evade
        else if (m_CurrentPhase == PHASE_BO_ATTACK_CROUCH_SHOOT )
        {
            return GetEvadePhase();
        }
    }

    // made it through, let's just do a normal check then.
    return GetNextPhaseNormal();
}

//=========================================================================

s32 soldier_attack_state::GetNextPhaseNormal( void )
{
    if( !m_CharacterBase.CanPathToTarget() )
    {
        if( m_CharacterBase.CanSeeTarget() )
        {
            return PHASE_BO_ATTACK_CROUCH_SHOOT;
        }
        else
        {
            // can't path and can't see, let's strafe some.
            return GetStrafePhase();
        }
    }
    else if( m_CharacterBase.CanSeeTarget() )
    {
        // I can see you! 
        return GetTargetVisiblePhase();
    }
    else if (m_CharacterBase.IsTargetInSightCone() )
    {
        // target is in our sight cone but not visible... the hidin bastard.
        return GetTargetHiddenPhase();
    }
    else
    {
        return PHASE_BO_ATTACK_CLOSE_FOR_MELEE;
    }
}

//=========================================================================

xbool soldier_attack_state::OnPain( const pain& Pain )
{    
    // every so often, on pain we will automatically rethink.
    if( m_TimeSinceLastRethink > k_MinTimeBetweenRethinks )
    {
        m_TimeSinceLastRethink = 0.0f;
        m_ForceRethink = TRUE;
    }
    return character_attack_state::OnPain( Pain );
}

//=========================================================================

void soldier_attack_state::OnBeingShotAt( object::type Type, guid ShooterID )
{
    if( m_TimeSinceLastRethink > k_MinTimeBetweenRethinks )
    {
        m_TimeSinceLastRethink = 0.0f;
        m_ForceRethink = TRUE;
    }
    character_attack_state::OnBeingShotAt( Type, ShooterID );
}

//=========================================================================

void soldier_attack_state::OnHitByFriendly( guid ShooterID )
{
    if( m_TimeSinceLastRethink > k_MinTimeBetweenRethinks )
    {
        m_TimeSinceLastRethink = 0.0f;
        m_ForceRethink = TRUE;
    }
    character_attack_state::OnHitByFriendly( ShooterID );
}

//=========================================================================

void soldier_attack_state::OnHitFriendly( guid FriendlyID )
{
    if( m_TimeSinceLastRethink > k_MinTimeBetweenRethinks )
    {
        m_TimeSinceLastRethink = 0.0f;
        m_ForceRethink = TRUE;
    }
    character_attack_state::OnHitFriendly( FriendlyID );
}

//=========================================================================

character_state::states soldier_attack_state::UpdateState( f32 DeltaTime )
{  
    return character_attack_state::UpdateState( DeltaTime );
}

//=========================================================================

void soldier_attack_state::OnThink( void )
{
    // force a rethink if I can't see my target
    if( m_CouldSeeTarget != m_CharacterBase.CanSeeTarget() )
    {
        m_ForceRethink = TRUE;
    }
    m_CouldSeeTarget = m_CharacterBase.CanSeeTarget();
    m_TargetRangeBracket = GetTargetRangeBracket();
    character_attack_state::OnThink();
}

//=========================================================================
//=========================================================================

void soldier_attack_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader  ( "BlackOppAttackState",                      "Different variables that effect the way that the character behaves in attack state.", 0 );
    List.PropEnumFloat   ( "BlackOppAttackState\\OptimalDistance",     "Distance to target we consider to be optimal.", 0 );
    List.PropEnumInt     ( "BlackOppAttackState\\Evade Percent",       "Percent chance we will play an evade when being shot at", 0 );
    List.PropEnumInt     ( "BlackOppAttackState\\Stand Percent",       "Percent chance we will stand an shoot instead of crounch and shoot.", 0 );
    List.PropEnumInt     ( "BlackOppAttackState\\Pain Strafe Percent", "Percent chance we will strafe a bit if taking pain.", 0 );
    character_attack_state::OnEnumProp( List );
}

//=========================================================================

xbool soldier_attack_state::OnProperty ( prop_query& rPropQuery )
{
    if( rPropQuery.VarFloat("BlackOppAttackState\\OptimalDistance", m_OptimalFiringDistance) )
        return TRUE;
    if( rPropQuery.VarInt("BlackOppAttackState\\Evade Percent", m_EvadePercent) )
        return TRUE;
    if( rPropQuery.VarInt("BlackOppAttackState\\Stand Percent", m_StandShootPercent) )
        return TRUE;
    if( rPropQuery.VarInt("BlackOppAttackState\\Pain Strafe Percent", m_PainStrafePercent) )
        return TRUE;
    return character_attack_state::OnProperty( rPropQuery );
}

//=========================================================================

const char*soldier_attack_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_BO_ATTACK_STRAFE_LEFT:
        return "PHASE_BO_ATTACK_STRAFE_LEFT";
    	break;
    case PHASE_BO_ATTACK_STRAFE_RIGHT:
        return "PHASE_BO_ATTACK_STRAFE_RIGHT";
    	break;
    case PHASE_BO_ATTACK_STAND_SHOOT:
        return "PHASE_BO_ATTACK_STAND_SHOOT";
    	break;
    case PHASE_BO_ATTACK_CROUCH_SHOOT:
        return "PHASE_BO_ATTACK_CROUCH_SHOOT";
    	break;
    case PHASE_BO_ATTACK_ADVANCE:
        return "PHASE_BO_ATTACK_ADVANCE";
    	break;
    case PHASE_BO_ATTACK_BACKUP:
        return "PHASE_BO_ATTACK_BACKUP";
    	break;
    case PHASE_BO_ATTACK_ALIGN_FOR_GRENADE_THROW:
        return "PHASE_BO_ATTACK_ALIGN_FOR_GRENADE_THROW";
    	break;
    case PHASE_BO_ATTACK_THROW_GRENADE:
        return "PHASE_BO_ATTACK_THROW_GRENADE";
    	break;
    case PHASE_BO_ATTACK_EVADE_LEFT:
        return "PHASE_BO_ATTACK_EVADE_LEFT";
    	break;
    case PHASE_BO_ATTACK_EVADE_RIGHT:
        return "PHASE_BO_ATTACK_EVADE_RIGHT";
    	break;
    case PHASE_BO_ATTACK_CLOSE_FOR_MELEE:
        return "PHASE_BO_ATTACK_CLOSE_FOR_MELEE";
    	break;
    case PHASE_BO_ATTACK_ALIGN_FOR_MELEE:
        return "PHASE_BO_ATTACK_ALIGN_FOR_MELEE";
    	break;
    case PHASE_BO_ATTACK_MELEE:
        return "PHASE_BO_ATTACK_MELEE";
    	break;
    case PHASE_BO_ATTACK_MELEE_BACK_LEFT:
        return "PHASE_BO_ATTACK_MELEE_BACK_LEFT";
        break;
    case PHASE_BO_ATTACK_MELEE_BACK_RIGHT:
        return "PHASE_BO_ATTACK_MELEE_BACK_RIGHT";
        break;
    case PHASE_BO_ATTACK_SURPRISED:
        return "PHASE_BO_ATTACK_SURPRISED";
        break;
    }
    return character_state::GetPhaseName(thePhase);
}
