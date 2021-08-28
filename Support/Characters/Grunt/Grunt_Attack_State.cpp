#include "grunt_attack_state.hpp"
#include "grunt.hpp"
#include "Objects\WeaponSMP.hpp"
#include "Objects\WeaponShotgun.hpp"
#include "Objects\Player.hpp"

//=========================================================================
// grunt ATTACK STATE
//=========================================================================

const s32 k_ShotsPerBurst               = 4;
const f32 k_MinTimeBetweenBursts        = 2.0f;
const f32 k_MinTimeShooting             = 3.0f;
const f32 k_MinDistForShooting          = 300.0f;
const s32 k_StandAndShootPercent        = 30;
const f32 k_MinTimeBetweenEvades        = 2.0f;
const f32 k_MinTimeBetweenSameShuffles   = 0.0f;
const f32 k_MinTimeBetweenDiffShuffles   = 2.0f;
const f32 k_MinTimeBetweenLeaps         = 2.0f;
const f32 k_MinTimeBetweenMelee         = 1.0f;
const f32 k_EvadeFOV                    = R_30;
const f32 k_MinTimeBetweenFleeAttempts  = 4.0f;
const f32 k_TimeTillFleeIfCantReach     = 3.0f;
const f32 k_MinTimePacing               = 1.0f;
const f32 k_MinAngleChaseDist           = 500.0f;
const f32 k_OptimalFiringDistance       = 1000.0f;
const f32 k_MinHealthForCharge          = 0.3f;
const f32 k_MinTimeInPaceIdle           = 1.0f;
const f32 k_MaxTimeInPaceIdle           = 4.0f;
const f32 k_MinAngleToLongMelee         = R_15;
const f32 k_MinDistForIsMeleeingSqr     = 400.0f * 400.0f;
const f32 k_MinKungFuDist               = 600.0f;
const f32 k_MaxKungFuDist               = 1000.0f;
const f32 k_MinTimeBetweenStrafes       = 1.0f;
const f32 k_MinGruntJumpAttackDist      = 500.0f;
const f32 k_MaxGruntJumpAttackDist      = 1200.0f;

s32 grunt_attack_state::s_LastChasePhase = PHASE_NONE;

//=========================================================================

grunt_attack_state::grunt_attack_state( character& Mutant, states State ) :
    character_attack_state(Mutant, State)
{
    if( m_CharacterBase.WeaponReady() )
    {
        m_MoveStyle = loco::MOVE_STYLE_RUNAIM;
    }
    else
    {    
        m_MoveStyle = loco::MOVE_STYLE_RUN;
    }
    m_LeapTimer  = 0.0f;
    m_bWantToEvade = FALSE;
    m_TimeInPaceIdle = 0.0f;
}

//=========================================================================

grunt_attack_state::~grunt_attack_state ( void )
{
}

//=========================================================================

void grunt_attack_state::OnEnter( void )
{
    m_bLastShuffleLeft = x_irand(0,1);
    m_LastPaceLeft = x_irand(0,1);
    m_bSwitchOutOfAttack = FALSE;
    character_attack_state::OnEnter();
    m_CantReachTimer = 0.0f;
    m_SinceLastShuffle = k_MinTimeBetweenDiffShuffles;
    m_SinceLastStrafe = k_MinTimeBetweenStrafes;
}

//=========================================================================

xbool grunt_attack_state::CanDoLongMelee()
{
    if( !m_bDoLongMelee ) 
    {
        return FALSE;
    }
    f32 distToTargetSqrd = m_CharacterBase.GetToTarget().LengthSquared();;
    radian ourYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
    radian toTargetYaw = m_CharacterBase.GetToTarget().GetYaw();
    radian yawDiff = x_abs(x_MinAngleDiff(ourYaw,toTargetYaw));

    return ( distToTargetSqrd < ( m_CharacterBase.GetLongMeleeRange() * m_CharacterBase.GetLongMeleeRange() ) &&
             distToTargetSqrd > ( m_CharacterBase.GetLongMeleeRange() * m_CharacterBase.GetLongMeleeRange() * 0.9f * 0.9f ) &&
             yawDiff <= k_MinAngleToLongMelee );
}

//=========================================================================

xbool grunt_attack_state::CanDoShortMelee()
{
    f32 distToTargetSqrd = m_CharacterBase.GetToTarget().LengthSquared();;

    return ( distToTargetSqrd < (m_CharacterBase.GetShortMeleeRange() * m_CharacterBase.GetShortMeleeRange()) );
}

//=========================================================================

s32 grunt_attack_state::UpdatePhase( f32 DeltaTime )
{
    m_SinceLastEvade += DeltaTime;
    m_SinceLastStrafe += DeltaTime;
    m_LeapTimer  += DeltaTime;
    m_CantReachTimer += DeltaTime;
    m_SinceLastShuffle += DeltaTime;
    m_SinceLastMeleeAttack += DeltaTime;

    s32 newPhase = PHASE_NONE;    
    f32 distToTargetSqrd = m_CharacterBase.GetToTarget().LengthSquared();;

    if( m_CharacterBase.CanPathToTarget() )
    {
        m_CantReachTimer = 0.0f;
    }

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        if( m_CharacterBase.CanPathToTarget() )
        {            
            newPhase = GetChasePhase();
        }
        else
        {
            newPhase = GetPacePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_SHUFFLE_LEFT:
        if( !m_CharacterBase.GetCanMoveLeft() ||
            !m_CharacterBase.GetCanMoveForward() || 
            m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_SHUFFLE_RIGHT:
        if( !m_CharacterBase.GetCanMoveRight() ||
            !m_CharacterBase.GetCanMoveForward() || 
            m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_SURPRISED:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( m_CharacterBase.CanPathToTarget() )
            {            
                newPhase = GetChasePhase();
            }
            else
            {
                newPhase = GetPacePhase();
            }
        }
        break;
    case PHASE_GRUNT_ATTACK_GROWL:   
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetPacePhase();
//            newPhase = GetChasePhase();   
        }
        break;
    case PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_LEFT:   
        m_CharacterBase.CheckShooting();
        if( !m_CharacterBase.WeaponReady() )
        {
            newPhase = GetChasePhase();
        }
        else if( m_CharacterBase.GetGoalCompleted() )
        {            
            if( m_CharacterBase.CanPathToTarget() )
            {            
                newPhase = PHASE_GRUNT_ATTACK_STAND_SHOOT;
            }
            else
            {
                newPhase = GetStandPacePhase();
            }
        }
        else if( !m_CharacterBase.CanSeeTarget() )
        {
            newPhase = GetChasePhase();
        }
        else if( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        else if( GetMeleePhase() )
        {
            newPhase = GetMeleePhase();
        }
        else if( !m_CharacterBase.GetCanMoveLeft() )
        {
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_RIGHT:   
        m_CharacterBase.CheckShooting();
        if( !m_CharacterBase.WeaponReady() )
        {
            newPhase = GetChasePhase();
        }
        else if( m_CharacterBase.GetGoalCompleted() )
        {            
            if( m_CharacterBase.CanPathToTarget() )
            {            
                newPhase = PHASE_GRUNT_ATTACK_STAND_SHOOT;
            }
            else
            {
                newPhase = GetStandPacePhase();
            }
        }
        else if( !m_CharacterBase.CanSeeTarget() )
        {
            newPhase = GetChasePhase();
        }
        else if( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        else if( GetMeleePhase() )
        {
            newPhase = GetMeleePhase();
        }
        else if( !m_CharacterBase.GetCanMoveRight() )
        {
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_STAND_SHOOT:   
        m_CharacterBase.CheckShooting();
        if( m_CharacterBase.GetParametricHealth() <= k_MinHealthForCharge &&
            m_TimeInPhase > 1.0f )
        {
            newPhase = GetChasePhase();
        }
        else if ( !m_CharacterBase.WeaponReady() )
        {
            newPhase = GetChasePhase();
        }
        else if (!m_CharacterBase.CanPathToTarget())
        {
            //we can no longer reach our target
            if( m_CharacterBase.WeaponReady() )
            {
                newPhase = GetStandPacePhase();        
            }
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        else if( GetMeleePhase() )
        {
            newPhase = GetMeleePhase();
        }
        else if( m_TimeInPhase > x_frand(2.0f,5.0f) )
        {
            newPhase = GetStandPacePhase();
        }

        break;
    case PHASE_GRUNT_ATTACK_RETREAT:   
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetChasePhase();
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        break;
    case PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE:   
        m_CharacterBase.CheckShooting();

        if (!m_CharacterBase.CanPathToTarget())
        {
            //we can no longer reach our target
            if( m_CharacterBase.WeaponReady() )
            {
                // sometimes growl angrily if can't reach.
                if( x_irand(0,3) )
                {                
                    newPhase = GetPacePhase();            
                }
                else
                {                
                    // if we are aren't facing player, close distance anyway.        
                    radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
                    radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
                    radian m_DiffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));
                    if( m_DiffToTargetYaw > R_45 )
                    {
                        newPhase = PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE;
                    }
                    else
                    {                    
                        newPhase = PHASE_GRUNT_ATTACK_GROWL;
                    }
                }
            }
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        else if( GetMeleePhase() )
        {
            newPhase = GetMeleePhase();
        }
        else if ( distToTargetSqrd > k_MinGruntJumpAttackDist * k_MinGruntJumpAttackDist && 
                  distToTargetSqrd < k_MinGruntJumpAttackDist * k_MaxGruntJumpAttackDist&& 
                  m_LeapTimer > k_MinTimeBetweenLeaps      &&
                  m_CharacterBase.CanSeeTarget()        && 
                  m_CharacterBase.CanPathToTarget()        &&
                  m_CharacterBase.GetHasClearJumpAttack() )
        {
            //should we leap
            if( x_irand(1,100) <= m_JumpAttackPercent )
            {            
                radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
                radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
                radian diffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));

                if( diffToTargetYaw <= R_10 )
                {            
                    newPhase = PHASE_GRUNT_ATTACK_LEAP;
                }
            }
            else
            {
                // don't check again for a while...
                m_LeapTimer = 0.0f;
            }
        }
        // guys with shotguns can't melee anymore. So we have to do this.
        else if( distToTargetSqrd <= k_OptimalFiringDistance*k_OptimalFiringDistance && 
//                 m_CharacterBase.GetParametricHealth() > 0.3f && 
                 m_CharacterBase.WeaponReady() &&
                 m_CharacterBase.CanSeeTarget() )
        {
            newPhase = PHASE_GRUNT_ATTACK_STAND_SHOOT;
        }
        else if( m_CharacterBase.GetGoalCompleted() && !m_CharacterBase.GetGoalSucceeded() )
        {
            // we completed our goal but aren't close enough to attack...
            newPhase = GetPacePhase();
        }
        else if ( m_CharacterBase.GetFriendlyBlocksTarget() &&
                  m_SinceLastShuffle > k_MinTimeBetweenSameShuffles )
        {
            newPhase = GetShufflePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_EVADE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_SinceLastEvade = 0.0f;
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_LEAP:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_LeapTimer = 0.0f;
            newPhase = GetMeleeStrafePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_SHORT_MELEE:
    case PHASE_GRUNT_ATTACK_LONG_MELEE:
    case PHASE_GRUNT_ATTACK_MELEE_BACK_LEFT:
    case PHASE_GRUNT_ATTACK_MELEE_BACK_RIGHT:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetMeleeStrafePhase();
            m_SinceLastMeleeAttack = 0.0f;
        }
        break;
    case PHASE_GRUNT_ATTACK_PACE_IDLE:
        m_CharacterBase.CheckShooting();
        if( m_TimeInPhase > m_TimeInPaceIdle )
        {            
            newPhase = GetPacePhase();
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        else if( GetMeleePhase() )
        {
            newPhase = GetMeleePhase();
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        else if( m_CharacterBase.CanPathToTarget() && m_TimeInPhase >= k_MinTimePacing )
        {
            newPhase = GetChasePhase();
        }
        else if( !m_CharacterBase.GetCanMoveRight() )
        {
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_MELEE_STRAFE_LEFT:
        {
            object *targetObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetTargetGuid() );
            if( targetObject )
            {            
                vector3 targetToUs = m_CharacterBase.GetPosition() - targetObject->GetPosition();
                targetToUs.GetY() = 0;
                targetToUs.NormalizeAndScale( m_CharacterBase.GetShortMeleeRange()*1.2f);
                targetToUs.RotateY( -R_25 );
                vector3 vDestination = targetObject->GetPosition() + targetToUs;
                m_CharacterBase.UpdateGoalLocation( vDestination );
            }
        }
        if( m_TimeInPhase >= k_MinTimePacing ||
            !m_CharacterBase.GetCanMoveForward() || 
            !m_CharacterBase.GetCanMoveLeft() )
        {
            if( GetMeleePhase() )
            {
                newPhase = GetMeleePhase();
            }
            else if( m_SinceLastMeleeAttack < k_MinTimeBetweenMelee &&
                     m_CharacterBase.HasAllies() )
            {
                // if we are aren't facing player, close distance anyway.        
                radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
                radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
                radian m_DiffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));
                if( m_DiffToTargetYaw > R_45 )
                {
                    newPhase = PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE;
                }
                else
                {                    
                    newPhase = PHASE_GRUNT_ATTACK_GROWL;
                }
            }
            else
            {            
                newPhase = GetChasePhase();
            }
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        break;
    case PHASE_GRUNT_ATTACK_MELEE_STRAFE_RIGHT:
        {
            object *targetObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetTargetGuid() );
            if( targetObject )
            {            
                vector3 targetToUs = m_CharacterBase.GetPosition() - targetObject->GetPosition();
                targetToUs.GetY() = 0;
                targetToUs.NormalizeAndScale( m_CharacterBase.GetShortMeleeRange()*1.2f);
                targetToUs.RotateY( R_25 );
                vector3 vDestination = targetObject->GetPosition() + targetToUs;
                m_CharacterBase.UpdateGoalLocation( vDestination );
            }
        }
        if( m_TimeInPhase >= k_MinTimePacing ||
            !m_CharacterBase.GetCanMoveForward() ||
            !m_CharacterBase.GetCanMoveLeft() )
        {
            if( GetMeleePhase() )
            {
                newPhase = GetMeleePhase();
            }
            else if( m_SinceLastMeleeAttack < k_MinTimeBetweenMelee &&
                    m_CharacterBase.HasAllies() )
            {
                // if we are aren't facing player, close distance anyway.        
                radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
                radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
                radian m_DiffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));
                if( m_DiffToTargetYaw > R_45 )
                {
                    newPhase = PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE;
                }
                else
                {                    
                    newPhase = PHASE_GRUNT_ATTACK_GROWL;
                }
            }
            else
            {            
                newPhase = GetChasePhase();
            }
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        break;
    case PHASE_GRUNT_ATTACK_PACE_LEFT:
        m_CharacterBase.CheckShooting();
        if( m_CharacterBase.GetGoalCompleted() )
        {            
            if( m_CharacterBase.CanPathToTarget() )
            {            
                newPhase = GetChasePhase();
            }
            else
            {
                newPhase = GetPacePhase();
            }
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        else if( GetMeleePhase() )
        {
            newPhase = GetMeleePhase();
        }
        else if( m_CharacterBase.CanPathToTarget() && m_TimeInPhase >= k_MinTimePacing )
        {
            newPhase = GetChasePhase();
        }
        else if( !m_CharacterBase.GetCanMoveLeft() || 
                 !m_CharacterBase.GetCanMoveForward() )
        {
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_PACE_RIGHT:
        m_CharacterBase.CheckShooting();
        if( m_CharacterBase.GetGoalCompleted() )
        {            
            if( m_CharacterBase.CanPathToTarget() )
            {            
                newPhase = GetChasePhase();
            }
            else
            {
                newPhase = GetPacePhase();
            }
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_GRUNT_ATTACK_EVADE;
        }
        else if( GetMeleePhase() )
        {
            newPhase = GetMeleePhase();
        }
        else if( m_CharacterBase.CanPathToTarget() && m_TimeInPhase >= k_MinTimePacing )
        {
            newPhase = GetChasePhase();
        }
        else if( !m_CharacterBase.GetCanMoveRight() || 
                 !m_CharacterBase.GetCanMoveForward() )
        {
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_GRUNT_ATTACK_NO_TARGET:
        if( m_CharacterBase.GetAwarenessLevel() > character::AWARENESS_SEARCHING )
        {
            //resume attack
            newPhase = GetChasePhase();
        }           
        else if( m_CharacterBase.GetGoalCompleted() )
        {
            //leave attack
            m_bSwitchOutOfAttack = TRUE;
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    }

    // if we leave the connections go to base state, unless we are leaping, then ignore.
    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE && m_CurrentPhase != PHASE_GRUNT_ATTACK_LEAP )
    {
        newPhase = basePhase;
    }

    // removed the no target phase, he was getting stuck in it at first. 
    // if this creates a new problem, fix the instance of the grunt not
    // moving after ripping mccan's head off.

/*    if( m_CurrentPhase != PHASE_GRUNT_ATTACK_NO_TARGET && 
        m_CharacterBase.GetAwarenessLevel() <= character::AWARENESS_SEARCHING )
    {
        newPhase = PHASE_GRUNT_ATTACK_NO_TARGET;
    }*/

    return newPhase;
}

//=========================================================================

void grunt_attack_state::ChangePhase( s32 newPhase )
{
    switch( newPhase ) 
    {
    case PHASE_GRUNT_ATTACK_PACE_IDLE:
        m_CharacterBase.SetIdleGoal();
        m_TimeInPaceIdle = x_frand( k_MinTimeInPaceIdle, k_MaxTimeInPaceIdle );
        break;
    case PHASE_GRUNT_ATTACK_SHUFFLE_RIGHT:
        m_bLastShuffleLeft = FALSE;
        m_SinceLastShuffle = 0.0f;
        m_CharacterBase.SetWantsToAim(TRUE);
        {
            object *targetObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetTargetGuid() );
            if( targetObject )
            {            
                vector3 usToTarget =  targetObject->GetPosition() - m_CharacterBase.GetPosition();
                usToTarget.GetY() = 0;
                usToTarget.NormalizeAndScale( x_frand(100.0f,150.0f) );
                usToTarget.RotateY( -(x_frand(R_10,R_15 )) );
                vector3 vDestination = m_CharacterBase.GetPosition() + usToTarget;
                m_CharacterBase.SetGotoLocationGoal( vDestination, loco::MOVE_STYLE_NULL, 50.0f );
            }
            else
            {
                m_CharacterBase.SetIdleGoal();
            }
        }
        break;
    case PHASE_GRUNT_ATTACK_RETREAT:
        m_CharacterBase.SetRetreatFromTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_NULL, k_MinKungFuDist, FALSE );
        break;
    case PHASE_GRUNT_ATTACK_SHUFFLE_LEFT:
        m_bLastShuffleLeft = TRUE;
        m_SinceLastShuffle = 0.0f;
        m_CharacterBase.SetWantsToAim(TRUE);
        {
            object *targetObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetTargetGuid() );
            if( targetObject )
            {            
                vector3 usToTarget = targetObject->GetPosition() - m_CharacterBase.GetPosition();
                usToTarget.GetY() = 0;
                usToTarget.NormalizeAndScale( x_frand(100.0f,150.0f) );
                usToTarget.RotateY( (x_frand(R_10,R_15 )) );
                vector3 vDestination = m_CharacterBase.GetPosition() + usToTarget;
                m_CharacterBase.SetGotoLocationGoal( vDestination, loco::MOVE_STYLE_NULL, 50.0f );
            }
            else
            {
                m_CharacterBase.SetIdleGoal();
            }
        }
        break;
    case PHASE_GRUNT_ATTACK_SURPRISED:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_SPOT_TARGET);
        break;
    case PHASE_GRUNT_ATTACK_NO_TARGET:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_LOST_TARGET);        
        break;
    case PHASE_GRUNT_ATTACK_GROWL:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_ADD_REACT_RAGE);
        break;
    case PHASE_GRUNT_ATTACK_STAND_SHOOT:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.AllowShootingNow();
        m_CharacterBase.SetIdleGoal();
        break;
    case PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetGotoTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_NULL, m_CharacterBase.GetShortMeleeRange() );
    	break;
    case PHASE_GRUNT_ATTACK_EVADE:
        //stop shooting
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
        if ( x_irand(0, 1) )
        {
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_EVADE_LEFT);
        }
        else
        {
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_EVADE_RIGHT);
        }
        break;
    case PHASE_GRUNT_ATTACK_LEAP:
        //stop shooting
        m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
        m_CharacterBase.SetScaledPlayAnimationGoal(loco::ANIM_MELEE_LEAP);
    	break;
    case PHASE_GRUNT_ATTACK_SHORT_MELEE:
        {
            m_bDoLongMelee = x_irand(0,1);
            radian toTargetYaw = m_CharacterBase.GetToTarget().GetYaw();
            radian ourYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            m_CharacterBase.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_SHORT);
        }
    	break;
    case PHASE_GRUNT_ATTACK_LONG_MELEE:
        {
            m_bDoLongMelee = x_irand(0,1);
            radian toTargetYaw = m_CharacterBase.GetToTarget().GetYaw();
            radian ourYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            m_CharacterBase.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_LONG);
        }
        break;
    case PHASE_GRUNT_ATTACK_MELEE_BACK_LEFT:
        //stop shooting
        m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_BACK_LEFT);
        break;
    case PHASE_GRUNT_ATTACK_MELEE_BACK_RIGHT:
        //stop shooting
        m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_BACK_RIGHT);
        break;
    case PHASE_GRUNT_ATTACK_MELEE_STRAFE_LEFT:

        m_CharacterBase.SetWantsToAim(TRUE);
        {
            object *targetObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetTargetGuid() );
            if( targetObject )
            {            
                vector3 targetToUs = m_CharacterBase.GetPosition() - targetObject->GetPosition();
                targetToUs.GetY() = 0;
                targetToUs.NormalizeAndScale( m_CharacterBase.GetLongMeleeRange()*1.2f);
                targetToUs.RotateY( -R_25 );
                vector3 vDestination = targetObject->GetPosition() + targetToUs;
                m_CharacterBase.SetGotoLocationGoal( vDestination );
            }
            else
            {
                m_CharacterBase.SetIdleGoal();
            }
            m_SinceLastStrafe = 0.0f;
        }
        break;
    case PHASE_GRUNT_ATTACK_MELEE_STRAFE_RIGHT:
        m_CharacterBase.SetWantsToAim(TRUE);
        {
            object *targetObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetTargetGuid() );
            if( targetObject )
            {            
                vector3 targetToUs = m_CharacterBase.GetPosition() - targetObject->GetPosition();
                targetToUs.GetY() = 0;
                targetToUs.NormalizeAndScale( m_CharacterBase.GetLongMeleeRange()*1.2f);
                targetToUs.RotateY( R_25 );
                vector3 vDestination = targetObject->GetPosition() + targetToUs;
                m_CharacterBase.SetGotoLocationGoal( vDestination );
            }
            else
            {
                m_CharacterBase.SetIdleGoal();
            }
            m_SinceLastStrafe = 0.0f;
        }
        break;
    case PHASE_GRUNT_ATTACK_PACE_LEFT:
    case PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_LEFT:
        m_CharacterBase.SetWantsToAim(TRUE);
        {
            vector3 locationToTarget = m_CharacterBase.GetToTarget();            
            if( m_CharacterBase.CanSeeTarget() )
            {
                // this one paces back and forth, because he can shoot from this basic position
                m_LastPaceLeft = TRUE;
                locationToTarget.GetY() = 0;
                locationToTarget.NormalizeAndScale(x_frand(300.0f,800.0f) );
                locationToTarget.RotateY(R_270+x_frand(-R_10,R_20));
            }
            else
            {
                //we can't reach our target, so try to get to a good shooting position
                //if target above try to move away, if target same level or below try to get closer
                if( locationToTarget.GetY() > 200.0f )
                {
                    locationToTarget.GetY() = 0;
                    locationToTarget.NormalizeAndScale(x_frand(500.0f,500.0f) );
                    locationToTarget.RotateY(x_frand(R_90,R_270));
                }
                else
                {                
                    locationToTarget.GetY() = 0;
                    locationToTarget.NormalizeAndScale(x_frand(500.0f,500.0f) );
                    locationToTarget.RotateY(x_frand(-R_90,R_90));
                }
            }
            vector3 vDestination = m_CharacterBase.GetPosition() + locationToTarget;
            nav_connection_slot_id iConn = g_NavMap.GetNearestConnectionInGrid( m_CharacterBase.GetCurrentConnection(), vDestination);
            m_CharacterBase.SetGotoLocationGoal( g_NavMap.GetNearestPointOnConnection( iConn, vDestination ) );
        }
        break;
    case PHASE_GRUNT_ATTACK_PACE_RIGHT:
    case PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_RIGHT:
        m_CharacterBase.SetWantsToAim(TRUE);
        {
            vector3 locationToTarget = m_CharacterBase.GetToTarget();            
            if( m_CharacterBase.CanSeeTarget() )
            {
                // this one paces back and forth, because he can shoot from this basic position
                m_LastPaceLeft = FALSE;
                locationToTarget.GetY() = 0;
                locationToTarget.NormalizeAndScale(x_frand(300.0f,800.0f) );
                locationToTarget.RotateY(R_90+x_frand(-R_20,R_10));
            }
            else
            {
                //we can't reach our target, so try to get to a good shooting position
                //if target above try to move away, if target same level or below try to get closer
                if( locationToTarget.GetY() > 200.0f )
                {
                    locationToTarget.GetY() = 0;
                    locationToTarget.NormalizeAndScale(x_frand(500.0f,500.0f) );
                    locationToTarget.RotateY(x_frand(R_90,R_270));
                }
                else
                {                
                    locationToTarget.GetY() = 0;
                    locationToTarget.NormalizeAndScale(x_frand(500.0f,500.0f) );
                    locationToTarget.RotateY(x_frand(-R_90,R_90));
                }
            }
            vector3 vDestination = m_CharacterBase.GetPosition() + locationToTarget;
            nav_connection_slot_id iConn = g_NavMap.GetNearestConnectionInGrid( m_CharacterBase.GetCurrentConnection(), vDestination);
            m_CharacterBase.SetGotoLocationGoal( g_NavMap.GetNearestPointOnConnection( iConn, vDestination ) );
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

s32 grunt_attack_state::GetKungFuPhase()
{
    f32 distToTargetSqrd = m_CharacterBase.GetToTarget().LengthSquared();;
    // if we are far then close.
    if( distToTargetSqrd > k_MaxKungFuDist * k_MaxKungFuDist )
    {
        return PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE;
    }
    // if we are very close melee strafe        
    else if ( distToTargetSqrd < k_MinKungFuDist * k_MinKungFuDist )
    {
        return GetMeleeStrafePhase();
    }
    // if we are aren't facing player, close distance anyway.        
    radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
    radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
    radian m_DiffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));
    if( m_DiffToTargetYaw > R_45 )
    {
        return PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE;
    }
    // otherwise growl.
    return PHASE_GRUNT_ATTACK_GROWL;
}

//=========================================================================

s32 grunt_attack_state::GetShufflePhase()
{
    // should we do kung-fu theater?
    if( m_CharacterBase.GetIsKungFuTime() )
    {
        return GetKungFuPhase();
    }
    else if( m_SinceLastShuffle >= k_MinTimeBetweenDiffShuffles )
    {
        // we can shuffle either way
        if( m_bLastShuffleLeft )
        {        
            if( m_CharacterBase.GetCanMoveRight() )
            {
                return PHASE_GRUNT_ATTACK_SHUFFLE_RIGHT;
            }
            else if( m_CharacterBase.GetCanMoveLeft() )
            {        
                return PHASE_GRUNT_ATTACK_SHUFFLE_LEFT;
            }
        }
        else
        {        
            if( m_CharacterBase.GetCanMoveLeft() )
            {
                return PHASE_GRUNT_ATTACK_SHUFFLE_LEFT;
            }
            else if( m_CharacterBase.GetCanMoveRight() )
            {        
                return PHASE_GRUNT_ATTACK_SHUFFLE_RIGHT;
            }
        }
    }
    else if ( m_SinceLastShuffle >= k_MinTimeBetweenSameShuffles )
    {
        // must Shuffle same direction or not at all
        if( m_bLastShuffleLeft)
        {
            if( m_CharacterBase.GetCanMoveLeft() )
            {
                return PHASE_GRUNT_ATTACK_SHUFFLE_LEFT;
            }
            else if( m_CharacterBase.GetCanMoveRight() )
            {
                return PHASE_GRUNT_ATTACK_SHUFFLE_RIGHT;
            }
        }
        else if( !m_bLastShuffleLeft )
        {
            if( m_CharacterBase.GetCanMoveRight() )
            {
                return PHASE_GRUNT_ATTACK_SHUFFLE_RIGHT;
            }
            else if( m_CharacterBase.GetCanMoveLeft() )
            {
                return PHASE_GRUNT_ATTACK_SHUFFLE_LEFT;
            }
        }
    }
    return PHASE_NONE;
}

//=========================================================================

s32 grunt_attack_state::GetMeleeStrafePhase()
{
    if( m_CharacterBase.GetCanMoveForward() &&
        m_SinceLastStrafe >= k_MinTimeBetweenStrafes )
    {    
        // do the one that brings us to the back more.
        object *targetObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetTargetGuid() );
        if( targetObject && targetObject->IsKindOf(actor::GetRTTI()) )
        {
            actor &targetActor = actor::GetSafeType( *targetObject );
            radian actorFacing = targetActor.GetYaw();
            vector3 targetToMe = m_CharacterBase.GetPosition() - targetActor.GetPosition();
            radian targetToMeYaw = targetToMe.GetYaw();
            radian angleDiff = x_MinAngleDiff(actorFacing,targetToMeYaw);
            if( angleDiff > 0.0f && 
                m_CharacterBase.GetCanMoveLeft() )
            {
                return PHASE_GRUNT_ATTACK_MELEE_STRAFE_LEFT;
            }
            else if (m_CharacterBase.GetCanMoveRight() )
            {
                return PHASE_GRUNT_ATTACK_MELEE_STRAFE_RIGHT;
            }
        }
        else
        {
            if( x_irand(0,1) && 
                m_CharacterBase.GetCanMoveRight() )
            {        
                return PHASE_GRUNT_ATTACK_MELEE_STRAFE_RIGHT;
            }
            else if( m_CharacterBase.GetCanMoveLeft() )
            {        
                return PHASE_GRUNT_ATTACK_MELEE_STRAFE_LEFT;
            }
        }
    }
    // we want to strafe, if we can't let's just growl.
    return PHASE_GRUNT_ATTACK_RETREAT;
}

//=========================================================================

s32 grunt_attack_state::GetPacePhase()
{

    // added go to pace when a friendly blocks our target. This was making him just
    // growl all the time. Why always growl first anyway?

//    if( m_CurrentPhase != PHASE_GRUNT_ATTACK_PACE_IDLE )
    if( m_CurrentPhase == PHASE_GRUNT_ATTACK_PACE_RIGHT ||
        m_CurrentPhase == PHASE_GRUNT_ATTACK_PACE_LEFT )
    {
        return PHASE_GRUNT_ATTACK_PACE_IDLE;
    }

    if( m_LastPaceLeft )
    {
        if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_GRUNT_ATTACK_PACE_RIGHT;
        }
        else if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_GRUNT_ATTACK_PACE_LEFT;
        }
        else
        {
            return GetChasePhase();
        }
    }
    else
    {
        if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_GRUNT_ATTACK_PACE_LEFT;
        }
        else if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_GRUNT_ATTACK_PACE_RIGHT;
        }
        else
        {
            return GetChasePhase();
        }
    }   
}
    
//=========================================================================

s32 grunt_attack_state::GetStandPacePhase()
{
    if( m_LastPaceLeft )
    {
        if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_RIGHT;
        }
        else if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_LEFT;
        }
        else if ( m_CharacterBase.CanPathToTarget() )
        {
            return GetChasePhase();
        }
        else
        {
            return PHASE_GRUNT_ATTACK_STAND_SHOOT;
        }
    }
    else
    {
        if( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_LEFT;
        }
        else if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_RIGHT;
        }
        else if ( m_CharacterBase.CanPathToTarget() )
        {
            return GetChasePhase();
        }
        else
        {
            return PHASE_GRUNT_ATTACK_STAND_SHOOT;
        }
    }   
}

//=========================================================================

s32 grunt_attack_state::GetMeleePhase()
{
    if( m_SinceLastMeleeAttack < k_MinTimeBetweenMelee )
    {
        return PHASE_NONE;
    }
    else if( CanDoShortMelee() )
    {
        return GetShortMeleePhase();
    }
    else if( CanDoLongMelee() )
    {
        return GetLongMeleePhase();
    }
    else
    {
        return PHASE_NONE;
    }
}

//=========================================================================

s32 grunt_attack_state::GetShortMeleePhase()
{
    // should we do kung-fu theater?
    if( m_CharacterBase.GetIsKungFuTime() )
    {
        return GetKungFuPhase();
    }

    radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
    radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
    radian diffToTargetYawRelative = x_MinAngleDiff(locoYaw, TargetYaw);
    s32 newPhase = PHASE_NONE;
    if( diffToTargetYawRelative > R_135 &&
        m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_LEFT) )
    {
        newPhase = PHASE_GRUNT_ATTACK_MELEE_BACK_LEFT;
    }
    else if( diffToTargetYawRelative < -R_135 &&
        m_CharacterBase.HasAnim(loco::ANIM_MELEE_BACK_RIGHT) )
    {
        newPhase = PHASE_GRUNT_ATTACK_MELEE_BACK_RIGHT;
    }
    else
    {
        return PHASE_GRUNT_ATTACK_SHORT_MELEE;
    }
    return newPhase;
}

//=========================================================================

s32 grunt_attack_state::GetLongMeleePhase()
{
    // should we do kung-fu theater?
    if( m_CharacterBase.GetIsKungFuTime() )
    {
        return GetKungFuPhase();
    }

    return PHASE_GRUNT_ATTACK_LONG_MELEE;
}

//=========================================================================

s32 grunt_attack_state::GetChasePhase()
{
    s32 newPhase = PHASE_NONE;
    // should we do kung-fu theater?
    if( m_CharacterBase.GetIsKungFuTime() )
    {
        newPhase = GetKungFuPhase();
    }
    // let's see if a friendly is in the way...   
    if ( !newPhase && 
         m_CharacterBase.GetFriendlyBlocksTarget() &&
         m_SinceLastShuffle > k_MinTimeBetweenSameShuffles )
    {
        newPhase = GetShufflePhase();
    }
    // this is odd... get melee strafe for this, get chase should return a chase yo.
    // this is called in melee strafe if strafing is bad... and was returning strafe again.
/*    if( !newPhase &&
        m_SinceLastMeleeAttack < k_MinTimeBetweenMelee )
    {
        newPhase = GetMeleeStrafePhase();
    }*/
    if( !newPhase )
    {    
        newPhase = PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE;
    }
    return newPhase;
}

//=========================================================================

character_state::states grunt_attack_state::UpdateState( f32 DeltaTime )
{
    // we need to update our cover status
    if( !m_CharacterBase.WeaponReady() && 
        !m_CharacterBase.CanPathToTarget() )
    {
        m_OurCoverDesire = SEEK_COVER_ALWAYS;        
    }
    else
    {
        m_OurCoverDesire = SEEK_COVER_SHOTAT;
    }

    (void)DeltaTime;
    if( m_bSwitchOutOfAttack )
    {
        return m_CharacterBase.GetStateFromAwareness();
    }
    return character_attack_state::UpdateState( DeltaTime );
}

//=========================================================================

xbool grunt_attack_state::OnExit( void )
{
    return character_attack_state::OnExit();
}


//=========================================================================

const char* grunt_attack_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE:
        return "PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE";
    	break;
    case PHASE_GRUNT_ATTACK_RETREAT:
        return "PHASE_GRUNT_ATTACK_RETREAT";
        break;    
    case PHASE_GRUNT_ATTACK_GROWL:
        return "PHASE_GRUNT_ATTACK_GROWL";
    	break;
    case PHASE_GRUNT_ATTACK_EVADE:
        return "PHASE_GRUNT_ATTACK_EVADE";
    	break;
    case PHASE_GRUNT_ATTACK_LEAP:
        return "PHASE_GRUNT_ATTACK_LEAP";
    	break;
    case PHASE_GRUNT_ATTACK_SHORT_MELEE:
        return "PHASE_GRUNT_ATTACK_SHORT_MELEE";
    	break;
    case PHASE_GRUNT_ATTACK_LONG_MELEE:
        return "PHASE_GRUNT_ATTACK_LONG_MELEE";
        break;
    case PHASE_GRUNT_ATTACK_MELEE_BACK_RIGHT:
        return "PHASE_GRUNT_ATTACK_MELEE_BACK_RIGHT";
        break;
    case PHASE_GRUNT_ATTACK_MELEE_BACK_LEFT:
        return "PHASE_GRUNT_ATTACK_MELEE_BACK_LEFT";
        break;
    case PHASE_GRUNT_ATTACK_STAND_SHOOT:
        return "PHASE_GRUNT_ATTACK_STAND_SHOOT";
    	break;
    case PHASE_GRUNT_ATTACK_PACE_LEFT:
        return "PHASE_GRUNT_ATTACK_PACE_LEFT";
    	break;
    case PHASE_GRUNT_ATTACK_PACE_RIGHT:
        return "PHASE_GRUNT_ATTACK_PACE_RIGHT";
        break;
    case PHASE_GRUNT_ATTACK_PACE_IDLE:
        return "PHASE_GRUNT_ATTACK_PACE_IDLE";
        break;
    case PHASE_GRUNT_ATTACK_MELEE_STRAFE_LEFT:
        return "PHASE_GRUNT_ATTACK_MELEE_STRAFE_LEFT";
        break;
    case PHASE_GRUNT_ATTACK_MELEE_STRAFE_RIGHT:
        return "PHASE_GRUNT_ATTACK_MELEE_STRAFE_RIGHT";
        break;
    case PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_LEFT:
        return "PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_LEFT";
    	break;
    case PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_RIGHT:
        return "PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_RIGHT";
        break;
    case PHASE_GRUNT_ATTACK_NO_TARGET:
        return "PHASE_GRUNT_ATTACK_NO_TARGET";
    	break;
    case PHASE_GRUNT_ATTACK_SURPRISED:
        return "PHASE_GRUNT_ATTACK_SURPRISED";
        break;
    case PHASE_GRUNT_ATTACK_SHUFFLE_LEFT:
        return "PHASE_GRUNT_ATTACK_SHUFFLE_LEFT";
        break;
    case PHASE_GRUNT_ATTACK_SHUFFLE_RIGHT:
        return "PHASE_GRUNT_ATTACK_SHUFFLE_RIGHT";
        break;
    }
    return character_state::GetPhaseName(thePhase);
}

//=========================================================================

void grunt_attack_state::OnBeingShotAt( object::type Type , guid ShooterID )
{
    if( m_CurrentPhase == PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE ||
        m_CurrentPhase == PHASE_GRUNT_ATTACK_STAND_SHOOT )
    {
        //we are chasing the target, and the target is shooting at us, so we might want to evade the fire
        if (m_SinceLastEvade > k_MinTimeBetweenEvades)
        {
            //we only evade if we have not evaded recently
            if ( m_CharacterBase.IsFacingTarget( m_CharacterBase.GetTargetPosWithOffset(), k_EvadeFOV ) )
            {
                //we only evade if our target is in front of us
                m_bWantToEvade = TRUE;
            }
        }
    }

    character_attack_state::OnBeingShotAt( Type, ShooterID );
}

//=========================================================================

xbool grunt_attack_state::IsMeleeingPlayer( void )
{
    if( m_CurrentPhase == PHASE_GRUNT_ATTACK_SHORT_MELEE ||
        m_CurrentPhase == PHASE_GRUNT_ATTACK_LONG_MELEE ||
        m_CurrentPhase == PHASE_GRUNT_ATTACK_MELEE_BACK_LEFT ||
        m_CurrentPhase == PHASE_GRUNT_ATTACK_MELEE_BACK_RIGHT ||
        m_CurrentPhase == PHASE_GRUNT_ATTACK_LEAP || 
        ( m_CurrentPhase == PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE &&
          m_CharacterBase.GetToTarget().LengthSquared() < k_MinDistForIsMeleeingSqr )          
      )
    {
        object *targetObject = g_ObjMgr.GetObjectByGuid( m_CharacterBase.GetTargetGuid() );
        if( targetObject && targetObject->IsKindOf(player::GetRTTI()) )
        {        
            return TRUE;
        }
    }

    return FALSE;
}
