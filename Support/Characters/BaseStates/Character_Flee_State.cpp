#include "Character_Flee_State.hpp"
#include "..\Character.hpp"
#include "navigation\coverNode.hpp"

//=========================================================================
// constants
//=========================================================================

//=========================================================================
// GRAY FLEE STATE
//=========================================================================

character_flee_state::character_flee_state( character& ourCharacter, character_state::states State ) :
    character_state(ourCharacter, State)
{
    m_TimeTillBored = 10.0f;
    m_MoveStyle = loco::MOVE_STYLE_RUN;
}

//=========================================================================

character_flee_state::~character_flee_state()
{
}

//=========================================================================

void character_flee_state::OnInit( void )
{
    character_state::OnInit();
}

//=========================================================================

void character_flee_state::OnEnter( void )
{
    character_state::OnEnter();
    m_PauseInPlace = 0.0f;
    m_CharacterBase.PlayDialog( character::DIALOG_FLEE );
}

//=========================================================================

s32 character_flee_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;
    if( m_PauseInPlace > 0.0f )
        m_PauseInPlace -= DeltaTime;

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_FLEE_FLEE_FROM_TARGET;
        break;
    case PHASE_FLEE_FLEE_FROM_TARGET:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            // if we tried to run away but couldn't, we must cower in place for a time before we try to run away again.
            if( !m_CharacterBase.GetGoalSucceeded() )
            {            
                m_PauseInPlace = x_frand(1.0f,5.0f);
            }
            newPhase = PHASE_FLEE_TURN_TO_TARGET_RUNNING;
        }
        break;
    case PHASE_FLEE_RUNING_COWER:
        // if the target gets too close, run again!
        if( m_CharacterBase.GetToTarget().Length() < 600.0f && m_PauseInPlace <= 0.0f )
        {
            newPhase = PHASE_FLEE_FLEE_FROM_TARGET;
        }
        break;
    case PHASE_FLEE_TURN_TO_TARGET_RUNNING:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_FLEE_RUNING_COWER;
        }
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    };

    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE )
    {
        newPhase = basePhase;
    }

    return newPhase;
}

//=========================================================================

void character_flee_state::ChangePhase( s32 newPhase )
{
    switch( newPhase ) 
    {
    case PHASE_FLEE_FLEE_FROM_TARGET:
        // run 12 meters away
        if( m_CharacterBase.GetTargetGuid() != 0 )
        {
            m_CharacterBase.SetRetreatFromTargetGoal( m_CharacterBase.GetTargetGuid(), vector3(0.0f,0.0f,0.0f), loco::MOVE_STYLE_RUN, 3000.0f, FALSE );
        }
        else
        {
            m_CharacterBase.SetRetreatFromLocationGoal( m_CharacterBase.GetLastLocationOfInterest(), loco::MOVE_STYLE_RUN, 3000.0f, FALSE );
        }
        break;
    case PHASE_FLEE_TURN_TO_TARGET_RUNNING:
        m_CharacterBase.SetTurnToLocationGoal( m_CharacterBase.GetLastKnownLocationOfTarget(), 5.0f, TRUE );
    	break;
    case PHASE_FLEE_RUNING_COWER:
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

character_state::states character_flee_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    if( m_CharacterBase.GetCurrentCover() )
    {
        return STATE_COVER;   
    }
    else 
    {
        states newState = m_CharacterBase.GetStateFromAwareness();
        if( newState != STATE_FLEE )
        {
            return newState;
        }
    }
    return STATE_NULL;
}

//=========================================================================

void character_flee_state::OnThink( void )
{
    character_state::OnThink();
}

//=========================================================================

void character_flee_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader(  "FleeState",  "Different variables that effect the way that the character behaves when standing still.", 0 );
    List.PropEnumFloat( "FleeState\\TimeTillBored", "Amount of Time to stay in state with no new sounds heard.", 0 ) ;
}

//=========================================================================

xbool character_flee_state::OnProperty ( prop_query& rPropQuery )
{
    if (rPropQuery.VarFloat("FleeState\\TimeTillBored", m_TimeTillBored))
        return TRUE;

    return FALSE ;
}

//=========================================================================

const char*character_flee_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_FLEE_FLEE_FROM_TARGET:
        return "PHASE_FLEE_FLEE_FROM_TARGET";
    	break;
    case PHASE_FLEE_RUNING_COWER:
        return "PHASE_FLEE_RUNING_COWER";
    	break;
    case PHASE_FLEE_TURN_TO_TARGET_RUNNING:
        return "PHASE_FLEE_TURN_TO_TARGET_RUNNING";
    	break;
    }
    return character_state::GetPhaseName(thePhase);
}
