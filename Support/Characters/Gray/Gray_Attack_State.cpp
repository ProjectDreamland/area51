#include "gray_attack_state.hpp"
#include "gray.hpp"
#include "Objects\WeaponSMP.hpp"
#include "Objects\WeaponShotgun.hpp"

#ifdef GRAY_AI_LOGGING
#define DEBUG_LOG_MSG       LOG_MESSAGE
#else
#define DEBUG_LOG_MSG       debug_log_msg_fn  
#endif

inline 
void debug_log_msg_fn(...) {}

//=========================================================================
// gray ATTACK STATE
//=========================================================================

static       f32 k_MinTimeBetweenVolleys                = 1.75f;
static       f32 k_MinTimeBetweenRounds                 = 0.3f;

static const f32 k_MaxFollowTime                        = 30.0f;
static const f32 k_MinTimeBetweenEngageFightFallback    = 3.0f;
static const f32 k_MaxHoldTime                          = 3.0f;
static const f32 k_ProbabilityOfCheatDuringReacquire    = 0.5f;     //  if rand(0,1) > this = cheatmode!
//static       f32 k_TargetLookAtTriggerStrafe            = R_5;
//static       f32 k_TargetLookAtTriggerStrafe            = 0;
//static       f32 k_FailedStrafeRecoveryTime             = 1.5f;
//static       f32 k_TimeBetweenEvades                    = 1.5f;
//static       f32 k_ProbPlayEvade                        = 0.5f;
static       f32 k_TimeBetweenCrouchChecks              = 2.0f;
static       f32 k_CrouchProb                           = 0.10f;    //  if rand(0,1) > this then crouch
static       f32 k_MaxCrouchFightTime                   = 3.5f;
static       f32 k_MaxStrafeTime                        = 1.0f;
//static       f32 k_MinTimeBetweenGrenadeChecks          = 10.0f;

//=========================================================================

gray_attack_state::gray_attack_state( character& Mutant, states State ) :
    character_attack_state(Mutant, State)
{
    m_MoveStyle             = loco::MOVE_STYLE_RUN;

    m_MinComfortDistance    = 800;
    m_MaxComfortDistance    = 2000;

    m_CurMinComfortDistance    = m_MinComfortDistance;
    m_CurMaxComfortDistance    = m_MaxComfortDistance;

    m_bDebugRender          = FALSE;
    
    m_bGiveUpAndSearch      = FALSE;

    m_LastGrayPhase         = PHASE_NONE;

    m_rTargetViewYaw        = 0;

    m_iRoundsLeftToFire     = 0;
    m_fNextRoundTimer       = 0;
    m_fNextVolleyTimer      = k_MinTimeBetweenVolleys;
    m_FailedStrafeTimer     = 0;

    m_EvadeTimer            = 0;
    m_EvadeTimeBetween      = 1.25f;
    m_EvadeProb             = 50.0f;

    m_GrenadeTimer          = 0;

    m_CheckForCrouchTimer   = 0;
}

//=========================================================================

gray_attack_state::~gray_attack_state ( void )
{
}

//=========================================================================

void gray_attack_state::OnInit( void )
{
    m_OurCoverDesire = SEEK_COVER_ALWAYS;
    character_attack_state::OnInit();
}

//=========================================================================

void gray_attack_state::OnEnter( void )
{
    m_LastPaceLeft = x_irand(0,1);    

    character_attack_state::OnEnter();
}

//=========================================================================

s32 gray_attack_state::UpdatePhase( f32 DeltaTime )
{
    AdvanceTimers( DeltaTime );

    s32 newPhase = PHASE_NONE;    
    f32 distToTarget = m_CharacterBase.GetToTarget().Length();

    f32 fDistToTarget = m_CharacterBase.GetToTarget().Length();        


    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        if( m_LastPhase == PHASE_GOTO_NEAREST_CONNECTION )
        {
            newPhase = PHASE_GRAY_ENGAGE;
        }
        else
        {        
            newPhase = PHASE_GRAY_ATTACK_SURPRISE;    
        }
        break;
    case PHASE_GRAY_ATTACK_SURPRISE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_GRAY_ENGAGE;
        }
        break;
    case PHASE_GRAY_ENGAGE: 
        {
            // Shoot when possible
            HandleShooting();
            
            if( m_CharacterBase.GetGoalCompleted())
            {
                newPhase = PHASE_GRAY_FIGHT;
            }
            else            
            {
                newPhase = CheckForFallback();
            }               
        }
        break;
    case PHASE_GRAY_FIGHT_STRAFE:
        {
            HandleShooting();

            vector3 toTarget;
            toTarget = m_CharacterBase.GetToTarget();
            toTarget.GetY() = 0.0f;
            toTarget.Normalize();

            vector3 Side = toTarget;

            Side = Side.Cross( vector3(0.0f,1.0f,0.0f) );
            if (m_StrafingLeft)
                Side.Scale( -100.0f );
            else
                Side.Scale(  100.0f );

            Side += m_CharacterBase.GetPosition();

            toTarget.Scale( m_StrafeForwardBackModifier );
            Side += toTarget;

            m_CharacterBase.UpdateGoalLocation(Side);

            if( m_CharacterBase.GetGoalCompleted() || 
                (m_TimeInPhase > k_MaxStrafeTime) )                
            {
                if (m_CharacterBase.GetGoalCompleted())
                {
                    if (!m_CharacterBase.GetGoalSucceeded())                    
                    {
                        m_FailedStrafeLeft  = m_StrafingLeft;
                        m_FailedStrafeTimer = 0;
                    }
                }
                newPhase = PHASE_GRAY_FIGHT;
            }
            else
            {   
                newPhase = CheckForFallback();
            }
        }
        break;
    case PHASE_GRAY_FIGHT_CROUCHED:
        if (m_TimeInPhase > k_MaxCrouchFightTime)
        {
            newPhase = PHASE_GRAY_FIGHT;
        }
        // NO BREAK
    case PHASE_GRAY_FIGHT:        
        // Shoot when possible
        HandleShooting();

        // Reacquire if we lose the target
        if (!m_CharacterBase.CanSeeTarget())
        {
            // We have lost line of sight
            // Move to last place where we saw the target
            newPhase = ChoosePhase_TargetHidden();
        }        
        else // Check comfort range        
        if (distToTarget < m_CurMinComfortDistance)
        {
            newPhase = CheckForFallback();
        }
        else if (distToTarget > m_CurMaxComfortDistance)
        {
            newPhase = PHASE_GRAY_ENGAGE;
        }
        else if (m_HitByFriendlyThisTick)
        {
            newPhase = PHASE_GRAY_FIGHT_CROUCHED;
        }
        else if (m_HitFriendlyThisTick)
        {
            newPhase = PHASE_GRAY_FIGHT_STRAFE;
        }
        
        else if (m_ShotAtThisTick)
        {
            if ( m_CurrentPhase == PHASE_GRAY_FIGHT_CROUCHED )
            {
                // Get out of the way
                //SH: REMOVING STRAFE - newPhase = PHASE_GRAY_FIGHT_STRAFE;
                newPhase = PHASE_GRAY_FIGHT;
            }
            else
            {
                // Possibly evade
                if (m_EvadeTimer > m_EvadeTimeBetween)
                {
                    if (x_frand(0,100) < m_EvadeProb)
                        newPhase = PHASE_GRAY_FIGHT_EVADE;
                }
            }           
        }
        else if (m_CheckForCrouchTimer > k_TimeBetweenCrouchChecks)
        {
            m_CheckForCrouchTimer = 0;

            if ( m_CurrentPhase != PHASE_GRAY_FIGHT_CROUCHED )
            {
                if (x_frand(0,1) < k_CrouchProb)
                {
                    newPhase = PHASE_GRAY_FIGHT_CROUCHED;
                }
            }
        }
        break;
    case PHASE_GRAY_FIGHT_EVADE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            m_EvadeTimer = 0.0f;
            newPhase = PHASE_GRAY_FIGHT;
        }
        break;
        break;
    case PHASE_GRAY_FALLBACK_FACE_TARGET:

        // Shoot when possible
        HandleShooting();
/*
        if (fDistToTarget > k_FallbackPanicDist * 1.1f)
        {
            newPhase = PHASE_GRAY_FALLBACK_FACE_AWAY;
        }
*/
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_GRAY_FIGHT;
        }
        else
        {
            newPhase = CheckForFallback();
        }
        /*
        else
        {
            if ( !m_CharacterBase.CanSeeTarget() )
                newPhase = PHASE_GRAY_HOLD_NO_TARGET;            
        }*/
        break;
    case PHASE_GRAY_FALLBACK_FACE_AWAY:

        if( m_CharacterBase.GetGoalCompleted() )
        {
            if (fDistToTarget < m_CurMinComfortDistance)
                newPhase = PHASE_GRAY_FALLBACK_FACE_AWAY;
            else
                newPhase = PHASE_GRAY_FALLBACK_STOP_AND_LOOKAT_TARGET;
        }
        /*
        else
        {
            if (fDistToTarget < k_FallbackPanicDist * 0.9f)
            {   
                newPhase = PHASE_GRAY_FALLBACK_FACE_TARGET;
            }
        }*/
        break;
    case PHASE_GRAY_FALLBACK_STOP_AND_LOOKAT_TARGET:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_GRAY_FIGHT;
        }
        break;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_VIEWABLE:
        if ( m_CharacterBase.CanSeeTarget() )
        {   
            DEBUG_LOG_MSG( "gray_attack_state::UpdatePhase", "REACQUIRE - Success.  Target is visible." );
            newPhase = PHASE_GRAY_FIGHT;
        }
        else if (m_CharacterBase.GetGoalCompleted())
        {
            newPhase = PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_KNOWN_POS;
        }
        break;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_KNOWN_POS:
        if ( m_CharacterBase.CanSeeTarget() )
        {   
            DEBUG_LOG_MSG( "gray_attack_state::UpdatePhase", "REACQUIRE - Success.  Target is visible." );
            newPhase = PHASE_GRAY_FIGHT;
        }
        else if (m_CharacterBase.GetGoalCompleted())
        {
            newPhase = PHASE_GRAY_HOLD_NO_TARGET;
        }
        break;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_TARGET:
        if ( m_CharacterBase.CanSeeTarget() )
        {   
            DEBUG_LOG_MSG( "gray_attack_state::UpdatePhase", "REACQUIRE - Success.  Target is visible." );
            newPhase = PHASE_GRAY_FIGHT;
        }

        if (m_TimeInPhase > k_MaxFollowTime)
        {
            newPhase = PHASE_GRAY_HOLD_NO_TARGET;
        }                   
        break;
    case PHASE_GRAY_ATTACK_PACE:
        HandleShooting();

        if( m_CharacterBase.GetGoalCompleted() )
        {        
            newPhase = PHASE_GRAY_ATTACK_PACE;
        }
        break;
    case PHASE_GRAY_HOLD_NO_TARGET:
        //  
        //  Normal loss of target behaviour will eventually kick us out
        //  to the search state
        //
        if (m_CharacterBase.CanSeeTarget())
        {
            // We have line of sight
            // Start fighting
            newPhase = PHASE_GRAY_FIGHT;
        }
        if (m_TimeInPhase > k_MaxHoldTime)
        {
            newPhase = PHASE_GRAY_REACQUIRE_TARGET_GOTO_TARGET;
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    };

    if (newPhase == PHASE_NONE)
    {
        // Handle switching to strafe under certain circumstances
        newPhase = CheckForRequiredStrafing();
    }         

    newPhase = ValidateNewPhase( newPhase );

    // if we leave the connections go to base state, unless we are leaping, then ignore.
    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE )
    {
        newPhase = basePhase;
    }

    //PostUpdateCleanup();

    return newPhase;
}

//=========================================================================

void gray_attack_state::ChangePhase( s32 newPhase )
{
    vector3 locationToTarget;
    
    f32 fDistToTarget = m_CharacterBase.GetToTarget().Length();        
    f32 fBasis = (fDistToTarget - m_CurMinComfortDistance) / (m_CurMaxComfortDistance - m_CurMinComfortDistance);

    fBasis -= 0.5f;
    fBasis *= 2.0f;

    // How to read Basis:  -1 = player is at min comfort
    //                     +1 = player is at max comfort
    //                     Think of it as how much you desire to move forward


    f32 fRand         = x_frand(0.25f,0.75f);
    f32 fComfortRange = m_CurMaxComfortDistance - m_CurMinComfortDistance;
    f32 fComfortDist  = m_CurMinComfortDistance + ( fComfortRange * fRand );

    DEBUG_LOG_MSG( "gray_attack_state::ChangePhase", "**Phase was [%s]", GetPhaseName() );

    switch( newPhase ) 
    {
    case PHASE_GRAY_ATTACK_SURPRISE:
        DEBUG_LOG_MSG( "gray_attack_state::ChangePhase", "Setting action goal to logo::ANIM_SPOT_TARGET" );
        m_CharacterBase. SetPlayAnimationGoal(loco::ANIM_SPOT_TARGET);
    	break;
    case PHASE_GRAY_ENGAGE:
        {           
            m_CharacterBase.SetWantsToAim(TRUE);            
            DEBUG_LOG_MSG( "gray_attack_state::ChangePhase", "Setting a target goal for engage. Basis to target [%f]",fBasis );
            if (m_CharacterBase.CanReachTarget())
                m_CharacterBase.SetGotoTargetGoal(m_CharacterBase.GetTargetGuid(),vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_NULL, fComfortDist );
            else
            {
                nav_connection_slot_id iConn = g_NavMap.GetNearestConnectionInGrid( m_CharacterBase.GetCurrentConnection(), 
                                                                                    m_CharacterBase.GetTargetPosWithOffset());

                if (NULL_NAV_SLOT == iConn)
                {
                    // Are there no nav connections?
                    m_CharacterBase.SetIdleGoal();       
                    break;
                }
                vector3 Pos = g_NavMap.GetNearestPointOnConnection( iConn, m_CharacterBase.GetTargetPosWithOffset() );

                m_CharacterBase.SetGotoLocationGoal( Pos );
            }
        }                
    	break;
    case PHASE_GRAY_FIGHT_STRAFE:        
        {
            m_CharacterBase.SetWantsToAim( TRUE );
            m_CharacterBase.GetLocoPointer()->SetMoveStyle( loco::MOVE_STYLE_RUNAIM );

            vector3 toTarget;
            toTarget = m_CharacterBase.GetToTarget();
            toTarget.GetY() = 0.0f;
            toTarget.Normalize();
            toTarget = toTarget.Cross( vector3(0.0f,1.0f,0.0f) );
            if (m_StrafingLeft)
                toTarget.Scale( -100.0f );
            else
                toTarget.Scale(  100.0f );

            toTarget += m_CharacterBase.GetPosition();
            
            m_CharacterBase.SetGotoLocationGoal(toTarget);             
        }
        break;
    case PHASE_GRAY_FIGHT_CROUCHED:        
        m_CharacterBase.SetWantsToAim( TRUE );
        m_CharacterBase.GetLocoPointer()->SetMoveStyle( loco::MOVE_STYLE_CROUCHAIM );
        m_CharacterBase.SetIdleGoal();        
        break;
    case PHASE_GRAY_FIGHT:        
        m_CharacterBase.SetWantsToAim( TRUE );
        m_CharacterBase.SetIdleGoal();        
        break;
    case PHASE_GRAY_FIGHT_EVADE:
        //if (m_rTargetViewYaw >= R_0)
        if (m_StrafingLeft)
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_EVADE_LEFT);
        else
            m_CharacterBase.SetPlayAnimationGoal(loco::ANIM_EVADE_RIGHT);
        break;
    case PHASE_GRAY_FALLBACK_FACE_TARGET:
        {
            m_CharacterBase.SetOverrideLookatMode( character::LOOKAT_NONE );
            m_CharacterBase.SetWantsToAim( TRUE );
            
            DEBUG_LOG_MSG( "gray_attack_state::ChangePhase", "Setting a target goal for fallback.");
            m_CharacterBase.SetRetreatFromTargetGoal( m_CharacterBase.GetTargetGuid(), vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_RUN, fComfortDist );                            
        }
    	break;
    case PHASE_GRAY_FALLBACK_FACE_AWAY:
        {
            m_CharacterBase.SetOverrideLookatMode( character::LOOKAT_NAVIGATION );
            m_CharacterBase.SetWantsToAim( FALSE );
                        
            DEBUG_LOG_MSG( "gray_attack_state::ChangePhase", "Setting a target goal for fallback.");
            m_CharacterBase.SetRetreatFromTargetGoal( m_CharacterBase.GetTargetGuid(), vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_RUN, fComfortDist );                            
        }
    	break;
    case PHASE_GRAY_FALLBACK_STOP_AND_LOOKAT_TARGET:
        m_CharacterBase.SetWantsToAim( TRUE );
        m_CharacterBase.SetIdleGoal();        
        m_CharacterBase.SetTurnToTargetGoal( m_CharacterBase.GetTargetGuid(),
                                             vector3(0,0,0),
                                             R_25,
                                             FALSE );
        break;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_VIEWABLE:
        DEBUG_LOG_MSG( "gray_attack_state::ChangePhase", "REACQUIRE - heading to last position where I saw target" );
        m_CharacterBase.SetGotoLocationGoal( m_LastPlaceWhereTargetWasViewable );
        break;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_KNOWN_POS:
        DEBUG_LOG_MSG( "gray_attack_state::ChangePhase", "REACQUIRE - heading to target's last known position" );
        m_CharacterBase.SetGotoLocationGoal( g_NavMap.GetNearestPointInNavMap( m_CharacterBase.GetLastKnownLocationOfTarget() ));        
        break;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_TARGET:       
        {
            guid gTarget = m_CharacterBase.GetTargetGuid();
            if (gTarget==0)
            {
                DEBUG_LOG_MSG( "gray_attack_state::ChangePhase", "REACQUIRE - want to go to target (cheat) but target has been removed" );
                m_CharacterBase.SetIdleGoal();
            }
            else
            {
                //object* pTarget = g_ObjMgr.GetObjectByGuid( gTarget );
                //ASSERT( pTarget );
                DEBUG_LOG_MSG( "gray_attack_state::ChangePhase", "REACQUIRE - heading to target's position (cheating)" );
                m_CharacterBase.SetGotoTargetGoal( m_CharacterBase.GetTargetGuid(), vector3(0,0,0), loco::MOVE_STYLE_NULL, fComfortDist );                
            }
        }
        break;    
    case PHASE_GRAY_ATTACK_PACE:
        // this one paces back and forth
        m_CharacterBase.SetWantsToAim(TRUE);
        locationToTarget = m_CharacterBase.GetToTarget();            
        if( m_LastPaceLeft )
        {
            m_LastPaceLeft = FALSE;
            locationToTarget.GetY() = 0;
            locationToTarget.NormalizeAndScale(x_frand(400.0f,600.0f) );
            locationToTarget.RotateY(R_90);
            
        }
        else
        {
            m_LastPaceLeft = TRUE;
            locationToTarget.GetY() = 0;
            locationToTarget.NormalizeAndScale(x_frand(200.0f,500.0f) );
            locationToTarget.RotateY(R_270);
        }
        m_CharacterBase.SetGotoLocationGoal( g_NavMap.GetNearestPointInNavMap( m_CharacterBase.GetPosition() + locationToTarget ));
    	break;
    case PHASE_GRAY_HOLD_NO_TARGET:
        m_CharacterBase.SetWantsToAim( TRUE );
        m_CharacterBase.SetIdleGoal();
        break;
    default:        
        if( newPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid New Phase " );
        }
    }
    character_state::ChangePhase( newPhase );
#ifdef GRAY_AI_LOGGING
    LOG_MESSAGE( "gray_attack_state::ChangePhase", "Phase is now [%s]", GetPhaseName() );
#endif

    m_LastGrayPhase = newPhase;
}

//=========================================================================

character_state::states gray_attack_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    
    xbool bCanExitAttack = TRUE;

    character_state::states NewState = character_state::STATE_NULL;

    //
    //  Determine if we are ready to potentially exit the attack state
    //
    switch( m_CurrentPhase )
    {
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_VIEWABLE:
        bCanExitAttack = FALSE;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_KNOWN_POS:
        bCanExitAttack = FALSE;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_TARGET:
        bCanExitAttack = FALSE;
    }
    
    //
    //  Only update NewState in here
    //
    if ( bCanExitAttack )
    {
    
        if( !m_CharacterBase.GetTargetGuid() )
        {
            if( m_CharacterBase.HasState(STATE_ALERT) )
            {        
                return STATE_ALERT;
            }
            else
            {        
                return STATE_IDLE;
            }
        }
           
        if (m_bGiveUpAndSearch)
            NewState = character_state::STATE_ALERT;
    }

    //
    //  Allow the base character state to override
    //
    character_state::states BaseState = character_attack_state::UpdateState( DeltaTime );
/*
    if (BaseState == STATE_NULL)
        return NewState;
*/
    return BaseState;
}


//=========================================================================

xbool gray_attack_state::OnExit( void )
{
    return character_attack_state::OnExit();
}


//=========================================================================

const char* gray_attack_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_GRAY_ATTACK_SURPRISE:
        return "PHASE_GRAY_ATTACK_SURPRISE";
    	break;
    case PHASE_GRAY_ENGAGE:
        return "PHASE_GRAY_ENGAGE";
    	break;
    case PHASE_GRAY_FIGHT_CROUCHED:
        return "PHASE_GRAY_FIGHT_CROUCHED";
        break;
    case PHASE_GRAY_FIGHT:
        return "PHASE_GRAY_FIGHT";
        break;
    case PHASE_GRAY_FIGHT_STRAFE:
        return "PHASE_GRAY_FIGHT_STRAFE";
        break;
    case PHASE_GRAY_FIGHT_EVADE:
        return "PHASE_GRAY_FIGHT_EVADE";
        break;
    case PHASE_GRAY_FALLBACK_FACE_TARGET:
        return "PHASE_GRAY_FALLBACK_FACE_TARGET";
    	break;
    case PHASE_GRAY_FALLBACK_FACE_AWAY:
        return "PHASE_GRAY_FALLBACK_FACE_AWAY";
    	break;
    case PHASE_GRAY_FALLBACK_STOP_AND_LOOKAT_TARGET:
        return "PHASE_GRAY_FALLBACK_STOP_AND_LOOKAT_TARGET";
        break;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_VIEWABLE:
        return "PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_VIEWABLE";
        break;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_KNOWN_POS:
        return "PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_KNOWN_POS";
        break;
    case PHASE_GRAY_REACQUIRE_TARGET_GOTO_TARGET:
        return "PHASE_GRAY_REACQUIRE_TARGET_GOTO_TARGET";
        break;
    case PHASE_GRAY_ATTACK_PACE:
        return "PHASE_GRAY_ATTACK_PACE";
    	break;
    case PHASE_GRAY_HOLD_NO_TARGET:
        return "PHASE_GRAY_HOLD_NO_TARGET";
        break;
    }
    
    return character_state::GetPhaseName(thePhase);    
}

//=========================================================================

void gray_attack_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader(  "AttackState",  "Different variables that effect the way that the character behaves when attacking.", 0 );

    List.PropEnumFloat( "AttackState\\Min Comfort Distance", "How close an enemy can be before the AI will fallback", 0 ) ;
    List.PropEnumFloat( "AttackState\\Max Comfort Distance", "How far away an enemy can be and still be attacked", 0 ) ;    
    List.PropEnumBool ( "AttackState\\Render", "Toggle rendering of debug information", PROP_TYPE_DONT_SAVE );

    List.PropEnumFloat( "AttackState\\Evade Delay", "How long the gray must wait between evades", 0 ) ;    
    List.PropEnumFloat( "AttackState\\Evade Probability", "0-100% chance of evading", 0 ) ;    

}

//=========================================================================

xbool gray_attack_state::OnProperty ( prop_query& rPropQuery )
{
    if (rPropQuery.VarFloat("AttackState\\Min Comfort Distance", m_MinComfortDistance))
        return TRUE;

    if (rPropQuery.VarFloat("AttackState\\Max Comfort Distance", m_MaxComfortDistance))
        return TRUE;

    if (rPropQuery.VarBool("AttackState\\Render", m_bDebugRender ))
        return TRUE;

    if (rPropQuery.VarFloat("AttackState\\Evade Delay", m_EvadeTimeBetween))
        return TRUE;

    if (rPropQuery.VarFloat("AttackState\\Evade Probability", m_EvadeProb))
    {
        m_EvadeProb = MIN(100,MAX(0,m_EvadeProb));
        return TRUE;
    }

    return FALSE ;
}

//=========================================================================

#ifndef X_RETAIL
void gray_attack_state::OnDebugRender()
{
    character_attack_state::OnDebugRender();

    if (!m_bDebugRender)
        return;    

#ifdef X_EDITOR
    vector3 Pos = m_CharacterBase.GetPosition();

    Pos += vector3(0,10,0);

    draw_3DCircle( Pos, m_CurMinComfortDistance, xcolor(180,0,0) );
    draw_3DCircle( Pos, m_CurMaxComfortDistance, xcolor(180,100,0) );    

    // Draw sight info
    radian ArcYaw = m_CharacterBase.GetSightYaw();

    vector3 Delta;
    Delta.Set(0,0,m_CurMinComfortDistance);
    Delta.RotateY( ArcYaw );
    draw_Label( Pos+Delta, XCOLOR_WHITE, "Min Comfort Dist");

    Delta.Set(0,0,m_CurMaxComfortDistance);
    Delta.RotateY( ArcYaw );
    draw_Label( Pos+Delta, XCOLOR_WHITE, "Max Comfort Dist");

    draw_Label( m_LastPlaceWhereTargetWasViewable, XCOLOR_YELLOW, "LastSawTarget\nFromHere" );    
#endif // X_EDITOR
 
}
#endif // X_RETAIL

//=========================================================================

void gray_attack_state::OnThink( void )
{
    // Determine if sight has been lost
    xbool bCanSee   = m_CharacterBase.CanSeeTarget();
    xbool bCanGetTo = m_CharacterBase.CanReachTarget();
    
    if (bCanSee)
        m_LastPlaceWhereTargetWasViewable = m_CharacterBase.GetPosition();
   
    // Get target
    guid gTarget = m_CharacterBase.GetTargetGuid();

    //
    //  Build the TargetLookingAt value
    //
    object* pTarget = g_ObjMgr.GetObjectByGuid( gTarget );

    if (NULL == pTarget)
        return;

    if ( pTarget->IsKindOf( actor::GetRTTI() ))
    {
        const actor& Target = actor::GetSafeType( *pTarget );
   
        /*
        // Get a player yaw, and delta from player to npc
        vector3 DeltaFromTarget = m_CharacterBase.GetPosition() - Target.GetPosition();
        radian  TargetFacing = Target.GetSightYaw();

        // Clean
        DeltaFromTarget.Normalize();
        vector3 Facing(0,0,1);
        Facing.RotateY( TargetFacing );

        // Get dot product normalized to [0,1]     
        //
        //  0 = target looking directly away from me
        //  1 = target looking directly at me
        //
        m_fTargetLookingAt = (Facing.Dot( DeltaFromTarget ) + 1.0f) / 2.0f;
    */
        vector3 Delta = m_CharacterBase.GetPosition() - Target.GetPosition();
        Delta.Normalize();

        radian  TargetYaw = Target.GetSightYaw();
        radian  DeltaYaw  = Delta.GetYaw();

        TargetYaw -= DeltaYaw;

        m_rTargetViewYaw = x_ModAngle2( TargetYaw );
    }
    else
    {
        m_rTargetViewYaw = 0;
    }

    if (gTarget != 0)
    {    
        if (bCanGetTo)
        {
            // If we can get to the target, adjust the min and max
            // comfort distance based on the direction the target is
            // facing, relative to the npc.
            //
            // directly towards = normal distances
            // directly away    = fully contracted distances.
            //            
            // Adjust comfort zones
            f32 T = 1.0f - x_abs(m_rTargetViewYaw) / R_180;

            T = (T * 0.6f) + 0.4f;

            m_CurMinComfortDistance = m_MinComfortDistance * T;
            m_CurMaxComfortDistance = m_MaxComfortDistance * T;
        }   
        else
        {
            m_CurMinComfortDistance = m_MinComfortDistance;
            m_CurMaxComfortDistance = m_MaxComfortDistance;
        }
    }
}

//=========================================================================

gray_attack_state::gray_attack_phases gray_attack_state::ChoosePhase_TargetHidden( void )
{
    /*
    if (m_GrenadeTimer > k_MinTimeBetweenGrenadeChecks)
    {
        // Consider tossing a gren
        if( m_CharacterBase.HasClearGrenadeThrow() )
        {
            return PHASE_GRAY_ATTACK_GRENADE_PREP;
        }
    }
    */
    f32 Rand = x_frand(0,1);
    if (Rand > k_ProbabilityOfCheatDuringReacquire)
        return gray_attack_state::PHASE_GRAY_REACQUIRE_TARGET_GOTO_TARGET;
    else
        return gray_attack_state::PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_KNOWN_POS;

    // this is an unused return for now.
    return PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_VIEWABLE;
}

//=========================================================================

s32 gray_attack_state::ValidateNewPhase( s32 NewPhase )
{
    /*
    // restricted transitions
    //
    //  Fight->Engage
    //  Fight->Fallback

    switch( NewPhase )
    {
    case PHASE_GRAY_ENGAGE:
        if (m_LastGrayPhase == gray_attack_state::PHASE_GRAY_FALLBACK)
        {
            if (m_TimeInPhase < k_MinTimeBetweenEngageFightFallback)
                return PHASE_NONE;
        }
        break;                
    case PHASE_GRAY_FIGHT:
    case PHASE_GRAY_FALLBACK:
        if (NewPhase != m_LastGrayPhase)
        {
            if (m_TimeInPhase < k_MinTimeBetweenEngageFightFallback)
                return PHASE_NONE;
        }
        break;
    }
*/
    /*
    switch( NewPhase )
    {
    case PHASE_GRAY_ENGAGE:        
    case PHASE_GRAY_FIGHT:
    case PHASE_GRAY_FALLBACK:
        if (m_CharacterBase.GetWantsToFirePrimary())
            return PHASE_NONE;
    }
    */
    return NewPhase;
}

//=========================================================================

void gray_attack_state::AdvanceTimers( f32 DeltaTime )
{
    m_fNextRoundTimer += DeltaTime;
    m_fNextVolleyTimer += DeltaTime;
    m_FailedStrafeTimer += DeltaTime;
    m_EvadeTimer += DeltaTime;
    m_CheckForCrouchTimer += DeltaTime;
    m_GrenadeTimer += DeltaTime;
}

//=========================================================================

s32 gray_attack_state::CheckForRequiredStrafing( void )
{
    xbool bCanStrafe     = TRUE;
    xbool bNewStrafeLeft = (m_rTargetViewYaw >= R_0)?TRUE:FALSE;


    s32   ReturnPhase = PHASE_GRAY_FIGHT_EVADE;

    xbool bAllowStrafe = FALSE;

    switch( m_CurrentPhase)
    {
    case PHASE_GRAY_ENGAGE:
        m_StrafeForwardBackModifier = 100.0f;
        bAllowStrafe = TRUE;
        break;
    case PHASE_GRAY_FIGHT:
        m_StrafeForwardBackModifier = 0.0f;
        bAllowStrafe = TRUE;
        break;
    case PHASE_GRAY_FIGHT_CROUCHED:
        m_StrafeForwardBackModifier = 0.0f;
        bAllowStrafe = TRUE;
        break;        
    case PHASE_GRAY_FALLBACK_FACE_TARGET:
        m_StrafeForwardBackModifier = -100.0f;
        bAllowStrafe = TRUE;
        break;               
    }

    if (!bAllowStrafe)
        return PHASE_NONE;
    
    xbool bSwitchToStrafe = FALSE;

    if (bAllowStrafe)
    {
        if (m_ShotAtThisTick)
        {
            bSwitchToStrafe = TRUE;            
        }
        else if (m_TookPainThisTick)
        {
            bSwitchToStrafe = TRUE;            
        }
        else if (m_HitFriendlyThisTick)
        {
            // Just step sideways if we hit a friendly
            bSwitchToStrafe = TRUE;            
            ReturnPhase = PHASE_GRAY_FIGHT_STRAFE;
        }        
    }

    if (!bSwitchToStrafe)
        return PHASE_NONE;
    
    // Don't strafe if the target is beyond viewable range
    vector3 DeltaToTarget = m_CharacterBase.GetToTarget();
    f32     DistSq = DeltaToTarget.LengthSquared();
    f32     SightR = m_CharacterBase.GetSightRadius();

    if (DistSq > SightR*SightR)
        bSwitchToStrafe = FALSE;

    // Check for validity of strafe
    if (bNewStrafeLeft)
    {
        if ( !m_CharacterBase.GetCanMoveLeft() )
        {
            // Can't go left, can we go right?
            if (m_CharacterBase.GetCanMoveRight())
                bNewStrafeLeft = FALSE;
            else
                bCanStrafe = FALSE; 
        }
    }
    else
    {
        if ( !m_CharacterBase.GetCanMoveRight() )
        {
            // Can't go left, can we go right?
            if (m_CharacterBase.GetCanMoveLeft())
                bNewStrafeLeft = TRUE;
            else
                bCanStrafe = FALSE; 
        }
    }

    if (!bCanStrafe)
        return PHASE_NONE;


    // Set it up
    m_StrafingLeft = bNewStrafeLeft;
    return ReturnPhase;    
}

//=========================================================================

void gray_attack_state::HandleShooting( void )
{
    if( m_fNextVolleyTimer > k_MinTimeBetweenVolleys && m_CharacterBase.CanShootAtTarget() )
    {
        m_fNextVolleyTimer = 0.0f;
        //m_CharacterBase.SetWantsToFirePrimary(TRUE);
        m_iRoundsLeftToFire = x_irand( 4,4 );
    }

    xbool bWantsPrimary = m_CharacterBase.GetWantsToFirePrimary();
    xbool bPlayingFiringAnim = !(m_CharacterBase.GetLocoPointer()->GetAdditiveController( character::ANIM_FLAG_SHOOT_CONTROLLER ).IsAtEnd());

    if (m_iRoundsLeftToFire > 0 && !bWantsPrimary)
    {
        f32 T = (f32)m_iRoundsLeftToFire / 3.0f;

        T *= 1.5f;

        T = 0;

//        m_CharacterBase.SetAimDegradeMultiplier( T );

        if ((!bPlayingFiringAnim) && ( m_fNextRoundTimer >= k_MinTimeBetweenRounds ))
        {
            m_CharacterBase.SetWantsToFirePrimary(TRUE);
            m_iRoundsLeftToFire--;
        }
    }

    // Any request to fire invalidates the next volley timer.
    if ( bWantsPrimary )
        m_fNextVolleyTimer = 0.0f;
}

//=========================================================================

s32 gray_attack_state::CheckForFallback( void )
{
    f32 fDistToTarget = m_CharacterBase.GetToTarget().Length();        

    f32 Flee  = m_CurMinComfortDistance;
//    f32 Panic = m_CurMinComfortDistance * 0.75f;
    
    if (fDistToTarget > Flee)
        return PHASE_NONE;
  // HACKITY:
    // For now, skip the fallback_face_away phase
    if (m_CurrentPhase == PHASE_GRAY_FALLBACK_FACE_TARGET)
        return PHASE_NONE;
    else
        return PHASE_GRAY_FALLBACK_FACE_TARGET;
 
    /*
    // HACKITY:
    // For now, skip the fallback_face_away phase
    if (m_CurrentPhase == PHASE_GRAY_FALLBACK_FACE_TARGET)
        return PHASE_NONE;
    else
        return PHASE_GRAY_FALLBACK_FACE_TARGET;

    /*
    if (fDistToTarget > Panic)
    {
        // Target is too close, but not within panic distance
        //
        // If we are already in fallback face target mode,
        // don't bother changing anything, otherwise, switch
        // to the fallback phase.
        //
        if (m_CurrentPhase == PHASE_GRAY_FALLBACK_FACE_TARGET)
            return PHASE_NONE;
        else
            return PHASE_GRAY_FALLBACK_FACE_TARGET;
    }
 
    // Otherwise, the target is too close, and it's within
    // the freak-out-and-panic distance.
    //
    //
    //  Same as above, if we are already in the panic fallback,
    //  don't change anything.
    //
    if (m_CurrentPhase == PHASE_GRAY_FALLBACK_FACE_AWAY)
        return PHASE_NONE;
    
    return PHASE_GRAY_FALLBACK_FACE_AWAY;
    */

}
