#include "Character_Alert_State.hpp"
#include "..\Character.hpp"

//=========================================================================
// Alert STATE
//=========================================================================

const f32 k_MinTimeInIdle = 1.0f;

character_alert_state::character_alert_state( character& ourCharacter, character_state::states State ) :
    character_state(ourCharacter, State)
{
    m_TimeTillBored     = 5.0f;
    m_MoveStyle         = loco::MOVE_STYLE_PROWL;
    m_bDebugRender      = FALSE;
}

//=========================================================================

character_alert_state::~character_alert_state()
{
}

//=========================================================================

void character_alert_state::OnInit( void )
{
    character_state::OnInit();
}

//=========================================================================

void character_alert_state::OnEnter( void )
{
    character_state::OnEnter();
}

//=========================================================================

s32 character_alert_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;    

    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_ALERT_FACE_SOURCE;
        break;
    case PHASE_ALERT_FACE_SOURCE:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_ALERT_REACT_SOUND;
        }
        break;
    case PHASE_ALERT_REACT_SOUND:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_ALERT_IDLE;
        }
        break;
    case PHASE_ALERT_IDLE:
        if( !m_CharacterBase.IsTargetInSightCone( m_CharacterBase.GetLastLocationOfInterest(), -1.0f, m_CharacterBase.GetSightYaw()) && m_TimeInPhase >= k_MinTimeInIdle )
        {
            newPhase = PHASE_ALERT_FACE_SOURCE;
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

void character_alert_state::ChangePhase( s32 newPhase )
{
    switch( newPhase ) 
    {
    case PHASE_ALERT_FACE_SOURCE:
        m_CharacterBase.SetTurnToLocationGoal(m_CharacterBase.GetLastLocationOfInterest() );
    	break;
    case PHASE_ALERT_REACT_SOUND:
        m_CharacterBase.SetPlayAnimationGoal( loco::ANIM_HEAR_TARGET );
        break;
    case PHASE_ALERT_IDLE:
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

character_state::states character_alert_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    if( m_CharacterBase.GetCurrentCover() &&
        m_CharacterBase.HasState(STATE_COVER))
    {
        return STATE_COVER;
    }
    else if( m_CharacterBase.GetAwarenessLevel() != character::AWARENESS_ALERT )
    {
        return m_CharacterBase.GetStateFromAwareness();;
    }

    return STATE_NULL;
}

//=========================================================================

void character_alert_state::OnThink( void )
{
    character_state::OnThink();

    if( m_CharacterBase.GetSoundHeard() )
    {
        // if I've heard something send a message like everything else...
        m_CharacterBase.SendAlert( alert_package::ALERT_TYPE_SOUND, m_CharacterBase.GetLastSoundPosition(), m_CharacterBase.GetGuid() );
    }
}

//=========================================================================

void character_alert_state::OnBeingShotAt( object::type Type, guid ShooterID )
{
    character_state::OnBeingShotAt( Type, ShooterID );
}

//=========================================================================

void character_alert_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader(  "AlertState",  "Different variables that effect the way that the character behaves when standing still.", 0 );

    List.PropEnumFloat( "AlertState\\TimeTillBored", "Amount of Time to stay in state with no new sounds heard.", 0 );
    List.PropEnumBool ( "AlertState\\Render", "Toggle rendering of debug information", PROP_TYPE_DONT_SAVE );
}

//=========================================================================

xbool character_alert_state::OnProperty ( prop_query& rPropQuery )
{

    if (rPropQuery.VarFloat("AlertState\\TimeTillBored", m_TimeTillBored))
        return TRUE;

    if (rPropQuery.VarBool("AlertState\\Render", m_bDebugRender ))
        return TRUE;

    return FALSE ;
}

//=========================================================================

const char*character_alert_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_ALERT_FACE_SOURCE:
        return "PHASE_ALERT_FACE_SOURCE";
    	break;
    case PHASE_ALERT_REACT_SOUND:
        return "PHASE_ALERT_REACT_SOUND";
        break;
    case PHASE_ALERT_IDLE:
        return "PHASE_ALERT_IDLE";
    	break;
    }
    return character_state::GetPhaseName(thePhase);
}

//=========================================================================

#ifndef X_RETAIL
void character_alert_state::OnDebugRender( void )
{
    if (!m_bDebugRender)
        return;
    
    character_state::OnDebugRender();    
}
#endif // X_RETAIL
