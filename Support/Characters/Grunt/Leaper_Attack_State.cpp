#include "leaper_attack_state.hpp"
#include "grunt.hpp"
#include "Objects\WeaponSMP.hpp"
#include "Objects\WeaponShotgun.hpp"
#include "Objects\Player.hpp"


//=========================================================================
// leaper ATTACK STATE
//=========================================================================

const s32 k_ShotsPerBurst               = 4;
const f32 k_MinTimeBetweenBursts        = 2.0f;
const f32 k_MinTimeShooting             = 3.0f;
const f32 k_MinDistForShooting          = 300.0f;
const s32 k_StandAndShootPercent        = 30;
const f32 k_MinTimeBetweenEvades        = 1.5f;
const f32 k_MinTimeBetweenLeaps         = 1.0f;
const f32 k_EvadeFOV                    = R_30;
const f32 k_MinRunAndCowerTime          = 3.0f;
const f32 k_MinTimeBetweenFleeAttempts  = 4.0f;
const f32 k_TimeTillFleeIfCantReach     = 3.0f;
const f32 k_MinTimePacing               = 1.0f;
const f32 k_MinAngleChaseDist           = 500.0f;
const f32 k_OptimalFiringDistance       = 1000.0f;
const f32 k_MinHealthForCharge          = 0.3f;
const f32 k_MinDistanceToFleeSqr        = 350.0f * 350.0f;
const f32 k_MinDistForIsMeleeingSqr     = 400.0f * 400.0f;
const f32 k_MinLeapErrorDist            = -200.0f;
const f32 k_MaxLeapErrorDist            = 300.0f;
const f32 k_MaxTimeRetreating           = 2.5f;

const f32 k_MinLeaperJumpAttackDist  = 500.0f;
const f32 k_MaxLeaperJumpAttackDist  = 900.0f;

const f32 k_MinLeaperEvadeJumpAttackDistSqr  = 600.0f * 600.0f;
const f32 k_MaxLeaperEvadeJumpAttackDistSqr  = 1200.0f * 1200.0f;

s32 leaper_attack_state::s_LastChasePhase = PHASE_NONE;

//=========================================================================

leaper_attack_state::leaper_attack_state( character& Mutant, states State ) :
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
}

//=========================================================================

leaper_attack_state::~leaper_attack_state ( void )
{
}

//=========================================================================

void leaper_attack_state::OnEnter( void )
{
    m_LastPaceLeft = x_irand(0,1);
    m_bSwitchOutOfAttack = FALSE;
    character_attack_state::OnEnter();
    m_CantReachTimer = 0.0f;
}

//=========================================================================

s32 leaper_attack_state::UpdatePhase( f32 DeltaTime )
{
    m_SinceLastEvade += DeltaTime;
    m_LeapTimer  += DeltaTime;
    m_CantReachTimer += DeltaTime;

    s32 newPhase = PHASE_NONE;    

    if( m_CharacterBase.CanPathToTarget() )
    {
        m_CantReachTimer = 0.0f;
    }

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = GetChasePhase();
    case PHASE_LEAPER_ATTACK_STRAFE_LEFT:                
        // update the position we are heading for.
        {        
            // we want to attempt to stay at a couple meters over the max jump distance away from the player.
            vector3 toTarget = m_CharacterBase.GetToTarget();
            toTarget.NormalizeAndScale( - k_MaxLeaperJumpAttackDist * 1.2f );
            // rotate.
            toTarget.RotateY( -R_10 );
            toTarget += m_CharacterBase.GetTargetPosWithOffset();
            m_CharacterBase.UpdateGoalLocation( toTarget );
        }

        // we want to close if we can't go forward anymore,
        // our target isn't facing us, or we've simply been 
        // circling enough.
        if( m_TimeInPhase >= 0.25f &&
            ( m_CharacterBase.GetIsStuck() ||
              !m_CharacterBase.IsTargetFacingUs(R_270) ||
              !m_CharacterBase.GetCanMoveLeft() ||
              m_TimeInPhase >= 3.0f) ) 
        {
            newPhase = PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE;
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_LEAPER_ATTACK_EVADE;
        }

        break;
    case PHASE_LEAPER_ATTACK_STRAFE_RIGHT:
        {        
            // we want to attempt to stay at a couple meters over the max jump distance away from the player.
            vector3 toTarget = m_CharacterBase.GetToTarget();
            toTarget.NormalizeAndScale( - k_MaxLeaperJumpAttackDist * 1.2f );
            // rotate.
            toTarget.RotateY( R_10 );
            toTarget += m_CharacterBase.GetTargetPosWithOffset();
            m_CharacterBase.UpdateGoalLocation( toTarget );
        }

        if( m_TimeInPhase >= 0.25f &&
            (m_CharacterBase.GetIsStuck() ||
             !m_CharacterBase.IsTargetFacingUs(R_270) ||
             !m_CharacterBase.GetCanMoveRight() ||
             m_TimeInPhase >= 3.0f) ) 
        {
            newPhase = PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE;
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_LEAPER_ATTACK_EVADE;
        }
        break;
    case PHASE_LEAPER_ATTACK_RETREAT:
        if( m_CharacterBase.GetGoalCompleted() ||
            m_TimeInPhase >= k_MaxTimeRetreating )
        {
            newPhase = GetStrafePhase();
        }
        else if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_LEAPER_ATTACK_RETREAT_EVADE;
        }
        break;
    case PHASE_LEAPER_ATTACK_SURPRISED:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( m_CharacterBase.CanPathToTarget() )
            {            
                newPhase = GetChasePhase();
            }
        }
        break;
    case PHASE_LEAPER_ATTACK_GROWL:   
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = GetChasePhase();   
        }
        break;
    case PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE:   
    case PHASE_LEAPER_ATTACK_CLOSE_TO_LEAP:   
        if ( m_bWantToEvade )
        {
            //should we evade
            m_bWantToEvade = FALSE;
            newPhase = PHASE_LEAPER_ATTACK_EVADE;
        }
        else
        {        
            newPhase = GetAttackPhase();
        }
        break;
    case PHASE_LEAPER_ATTACK_RETREAT_EVADE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_SinceLastEvade = 0.0f;
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_LEAPER_ATTACK_EVADE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_SinceLastEvade = 0.0f;
            newPhase = GetChasePhase();
        }
        break;
    case PHASE_LEAPER_ATTACK_LEAP:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            // if we are close then we can retreat, 
            // if we are still pretty far away then let's close and 
            // melee.
            m_LeapTimer = 0.0f;
            if( x_irand(0,3) )
            {            
                newPhase = PHASE_LEAPER_ATTACK_RETREAT;
            }
            else
            {
                newPhase = PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE;
            }
        }
        break;
    case PHASE_LEAPER_ATTACK_SHORT_MELEE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( x_irand(0,3) )
            {            
                newPhase = PHASE_LEAPER_ATTACK_RETREAT;
            }
            else
            {
                newPhase = PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE;
            }
        }
        break;
    case PHASE_LEAPER_ATTACK_LONG_MELEE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( x_irand(0,3) )
            {            
                newPhase = PHASE_LEAPER_ATTACK_RETREAT;
            }
            else
            {
                newPhase = PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE;
            }
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
    if( basePhase != PHASE_NONE && m_CurrentPhase != PHASE_LEAPER_ATTACK_LEAP )
    {
        newPhase = basePhase;
    }

    return newPhase;
}

//=========================================================================

void leaper_attack_state::ChangePhase( s32 newPhase )
{
    switch( newPhase ) 
    {
    case PHASE_LEAPER_ATTACK_SURPRISED:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_SPOT_TARGET);
        break;
    case PHASE_LEAPER_ATTACK_RETREAT:
        m_CharacterBase.SetRetreatFromTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_NULL, k_MaxLeaperJumpAttackDist);
        break;
    case PHASE_LEAPER_ATTACK_GROWL:
        m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_ADD_REACT_RAGE);
        break;
    case PHASE_LEAPER_ATTACK_CLOSE_TO_LEAP:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetGotoTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_CHARGE, k_MaxLeaperJumpAttackDist);
        break;
    case PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE:
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.SetGotoTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_CHARGE, m_CharacterBase.GetShortMeleeRange() - 50.0f );
    	break;
    case PHASE_LEAPER_ATTACK_EVADE:
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
    case PHASE_LEAPER_ATTACK_RETREAT_EVADE:
        //stop shooting
        m_CharacterBase.SetWantsToAim(TRUE);
        m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
        if ( x_irand(0, 1) )
        {
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_GRENADE_EVADE_LEFT);
        }
        else
        {
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_GRENADE_EVADE_RIGHT);
        }
        break;
    case PHASE_LEAPER_ATTACK_STRAFE_RIGHT:
        {        
            // we want to attempt to stay at a couple meters over the max jump distance away from the player.
            vector3 toTarget = m_CharacterBase.GetToTarget();
            toTarget.NormalizeAndScale( - k_MaxLeaperJumpAttackDist * 1.2f );
            // rotate.
            toTarget.RotateY( -R_10 );
            toTarget += m_CharacterBase.GetTargetPosWithOffset();
            m_CharacterBase.SetGotoLocationGoal(toTarget);
        }
    case PHASE_LEAPER_ATTACK_STRAFE_LEFT:
        {        
            // we want to attempt to stay at a couple meters over the max jump distance away from the player.
            vector3 toTarget = m_CharacterBase.GetToTarget();
            toTarget.NormalizeAndScale( - k_MaxLeaperJumpAttackDist * 1.2f );
            // rotate.
            toTarget.RotateY( R_10 );
            toTarget += m_CharacterBase.GetTargetPosWithOffset();
            m_CharacterBase.SetGotoLocationGoal(toTarget);
        }
        break;
    case PHASE_LEAPER_ATTACK_LEAP:
        {
            vector3 leapError = m_CharacterBase.GetToTarget();
            leapError.NormalizeAndScale( x_frand(k_MinLeapErrorDist,k_MaxLeapErrorDist) );

            m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
            // there is a chance we will jump left/right or over our target.
            s32 randResult = x_irand(1,100);
            if( randResult <= 25 )
            {
                // jump overhead.
                m_CharacterBase.SetScaledPlayAnimationGoal(loco::ANIM_MELEE_LEAP,DEFAULT_BLEND_TIME,0,m_CharacterBase.GetTargetPosWithOffset()+leapError);
            }
            else 
            {            
                m_CharacterBase.SetScaledPlayAnimationGoal(loco::ANIM_MELEE_LEAP);
            }
        }
    	break;
    case PHASE_LEAPER_ATTACK_SHORT_MELEE:
        {
            radian toTargetYaw = m_CharacterBase.GetToTarget().GetYaw();
            radian ourYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            m_CharacterBase.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_SHORT);
        }
        break;
    case PHASE_LEAPER_ATTACK_LONG_MELEE:
        {
            radian toTargetYaw = m_CharacterBase.GetToTarget().GetYaw();
            radian ourYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
            m_CharacterBase.SetAnimYawDelta( x_MinAngleDiff(toTargetYaw,ourYaw) );
            m_CharacterBase.GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).Clear();
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_MELEE_LONG);
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

s32 leaper_attack_state::GetStrafePhase()
{
    if( !m_CharacterBase.IsTargetFacingUs(R_270) ||
        !m_CharacterBase.GetCanReachGoalTarget() )
    {
        return GetChasePhase();
    }

    if( x_irand(0,1) )
    {            

        if( m_CharacterBase.GetCanMoveLeft() )
        {            
            return PHASE_LEAPER_ATTACK_STRAFE_LEFT;
        }
        else if( m_CharacterBase.GetCanMoveRight() )
        {
            return PHASE_LEAPER_ATTACK_STRAFE_RIGHT;
        }
    }
    else
    {
        if( m_CharacterBase.GetCanMoveRight() )
        {            
            return PHASE_LEAPER_ATTACK_STRAFE_RIGHT;
        }
        else if ( m_CharacterBase.GetCanMoveLeft() )
        {
            return PHASE_LEAPER_ATTACK_STRAFE_LEFT;
        }
    }
    return GetChasePhase();
}
//=========================================================================

s32 leaper_attack_state::GetAttackPhase()
{
    f32 distToTargetSqrd = m_CharacterBase.GetToTarget().LengthSquared();;

    if( m_CharacterBase.GetIsKungFuTime() )
    {
        // if we are far then close.
        if( distToTargetSqrd > k_MaxLeaperJumpAttackDist * k_MaxLeaperJumpAttackDist )
        {
            return PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE;
        }
        
        // if we are very close retreat        
        if ( distToTargetSqrd < m_CharacterBase.GetShortMeleeRange() * m_CharacterBase.GetShortMeleeRange() )
        {
            return PHASE_LEAPER_ATTACK_RETREAT;
        }
        // if we are aren't facing player, close distance anyway.        
        radian locoYaw = m_CharacterBase.GetLocoPointer()->GetYaw();
        radian TargetYaw = m_CharacterBase.GetToTarget().GetYaw();
        radian m_DiffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));
        if( m_DiffToTargetYaw > R_45 )
        {
            return PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE;
        }
        if( m_CurrentPhase == PHASE_LEAPER_ATTACK_GROWL )
        {
            return GetStrafePhase();
        }
        // otherwise growl.
        return PHASE_LEAPER_ATTACK_GROWL;
    }

    // do we want to leap? or do a melee?
    if( distToTargetSqrd < ( m_CharacterBase.GetShortMeleeRange() * m_CharacterBase.GetShortMeleeRange()) )
    {
        return PHASE_LEAPER_ATTACK_SHORT_MELEE;
    }
    else if( distToTargetSqrd < ( m_CharacterBase.GetLongMeleeRange() * m_CharacterBase.GetLongMeleeRange()) )
    {
        return PHASE_LEAPER_ATTACK_LONG_MELEE;
    }
    else if( distToTargetSqrd > k_MinLeaperJumpAttackDist * k_MinLeaperJumpAttackDist && 
            distToTargetSqrd < k_MaxLeaperJumpAttackDist * k_MaxLeaperJumpAttackDist && 
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
            radian m_DiffToTargetYaw = ABS(x_MinAngleDiff(locoYaw, TargetYaw));

            if( m_DiffToTargetYaw <= R_5 )
            {
                return PHASE_LEAPER_ATTACK_LEAP;
            }
        }
        else
        {
            // don't check again for a while...
            m_LeapTimer = 0.0f;
        }
    }
    return PHASE_NONE;
}

//=========================================================================

s32 leaper_attack_state::GetChasePhase()
{
    if( m_CharacterBase.GetToTarget().Length() < k_MaxLeaperJumpAttackDist * 1.2f )
    {
        return PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE;
    }
    else
    {    
        return PHASE_LEAPER_ATTACK_CLOSE_TO_LEAP;
    }
}

//=========================================================================

character_state::states leaper_attack_state::UpdateState( f32 DeltaTime )
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

xbool leaper_attack_state::OnExit( void )
{
    return character_attack_state::OnExit();
}


//=========================================================================

const char* leaper_attack_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_LEAPER_ATTACK_CLOSE_TO_LEAP:
        return "PHASE_LEAPER_ATTACK_CLOSE_TO_LEAP";
    	break;
    case PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE:
        return "PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE";
        break;
    case PHASE_LEAPER_ATTACK_GROWL:
        return "PHASE_LEAPER_ATTACK_GROWL";
    	break;
    case PHASE_LEAPER_ATTACK_EVADE:
        return "PHASE_LEAPER_ATTACK_EVADE";
    	break;
    case PHASE_LEAPER_ATTACK_LEAP:
        return "PHASE_LEAPER_ATTACK_LEAP";
    	break;
    case PHASE_LEAPER_ATTACK_SHORT_MELEE:
        return "PHASE_LEAPER_ATTACK_SHORT_MELEE";
    	break;
    case PHASE_LEAPER_ATTACK_LONG_MELEE:
        return "PHASE_LEAPER_ATTACK_LONG_MELEE";
        break;
    case PHASE_LEAPER_ATTACK_SURPRISED:
        return "PHASE_LEAPER_ATTACK_SURPRISED";
        break;
    case PHASE_LEAPER_ATTACK_RETREAT:
        return "PHASE_LEAPER_ATTACK_RETREAT";
        break;
    case PHASE_LEAPER_ATTACK_STRAFE_LEFT:
        return "PHASE_LEAPER_ATTACK_STRAFE_LEFT";
        break;
    case PHASE_LEAPER_ATTACK_STRAFE_RIGHT:
        return "PHASE_LEAPER_ATTACK_STRAFE_RIGHT";
        break;
    }
    return character_state::GetPhaseName(thePhase);
}

//=========================================================================

void leaper_attack_state::OnBeingShotAt( object::type Type , guid ShooterID )
{
    if( m_CurrentPhase == PHASE_LEAPER_ATTACK_CLOSE_TO_LEAP ||
        m_CurrentPhase == PHASE_LEAPER_ATTACK_STRAFE_LEFT ||
        m_CurrentPhase == PHASE_LEAPER_ATTACK_STRAFE_RIGHT ||
        m_CurrentPhase == PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE )
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

xbool leaper_attack_state::IsMeleeingPlayer( void )
{
    // only true if target is player.

    if( m_CurrentPhase == PHASE_LEAPER_ATTACK_SHORT_MELEE ||
        m_CurrentPhase == PHASE_LEAPER_ATTACK_LONG_MELEE ||
        m_CurrentPhase == PHASE_LEAPER_ATTACK_LEAP ||
        ( m_CurrentPhase == PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE &&
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

//=========================================================================
