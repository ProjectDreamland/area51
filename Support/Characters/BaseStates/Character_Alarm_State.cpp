#include "Character_Alarm_State.hpp"
#include "..\Character.hpp"
#include "navigation\alarmNode.hpp"
#include "objects\NewWeapon.hpp"

//=========================================================================
// GRAY ALARM STATE
//=========================================================================

character_alarm_state::character_alarm_state( character& ourCharacter, character_state::states State ) :
    character_state(ourCharacter, State)
{
    m_MoveStyle = loco::MOVE_STYLE_RUNAIM;
}

//=========================================================================

character_alarm_state::~character_alarm_state()
{
}

//=========================================================================

void character_alarm_state::OnInit( void )
{
    character_state::OnInit();
}

//=========================================================================

void character_alarm_state::OnEnter( void )
{
    m_StateDone = FALSE;
    character_state::OnEnter();
}

//=========================================================================

s32 character_alarm_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;
    
    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        newPhase = PHASE_ALARM_GOTO_ALARM;
        break;
    case PHASE_ALARM_GOTO_ALARM:
        m_CharacterBase.CheckShooting();
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( m_CharacterBase.GetGoalSucceeded() )
            {            
                newPhase = PHASE_ALARM_FACE_EXACT;
            }
            else
            {
                newPhase = PHASE_ALARM_GOTO_ALARM;
            }
        }
        break;
    case PHASE_ALARM_FACE_EXACT:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            newPhase = PHASE_ALARM_USE_ALARM;
        }
        break;
    case PHASE_ALARM_USE_ALARM:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            object *nodeObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetCurrentAlarm());
            if( nodeObject && nodeObject->IsKindOf( alarm_node::GetRTTI() ) )
            {
                alarm_node &alarmNode = alarm_node::GetSafeType( *nodeObject );
                alarmNode.Activate(TRUE);
            }
            m_StateDone = TRUE;
        }
        break;
    }
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

void character_alarm_state::ChangePhase( s32 newPhase )
{
    object *alarmObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetCurrentAlarm());

    switch( newPhase)
    {    
    case PHASE_ALARM_GOTO_ALARM:                
        m_CharacterBase.SetWantsToAim(TRUE);
        if ( alarmObject && alarmObject->IsKindOf( alarm_node::GetRTTI() ))
        {   
            m_CharacterBase.SetGotoLocationGoal(alarmObject->GetPosition(),loco::MOVE_STYLE_NULL,0.0f,TRUE);
        }
        else
        {
            m_CharacterBase.SetIdleGoal();
        }
        break;
    case PHASE_ALARM_FACE_EXACT:              
        if ( alarmObject && alarmObject->IsKindOf( alarm_node::GetRTTI() ))
        {   
            alarm_node &alarmNode = alarm_node::GetSafeType( *alarmObject );           
            vector3 alarmNodeFacing(0.0f,alarmNode.GetL2W().GetRotation().Yaw);
            alarmNodeFacing.NormalizeAndScale(1000.0f);
            m_CharacterBase.SetTurnToLocationGoal(alarmNode.GetPosition() + alarmNodeFacing, 0.0f, TRUE);
        }
        else
        {
            m_CharacterBase.SetIdleGoal();
        }
        break;
    case PHASE_ALARM_USE_ALARM:                   
        if ( alarmObject && alarmObject->IsKindOf( alarm_node::GetRTTI() ))
        {   
            alarm_node &alarmNode = alarm_node::GetSafeType( *alarmObject );           
            m_CharacterBase.SetPlayAnimationGoal( alarmNode.GetAnimName(),
                                                  alarmNode.GetAnimGroup(), 
                                                  DEFAULT_BLEND_TIME,
                                                  alarmNode.GetAnimFlags() | loco::ANIM_FLAG_TURN_OFF_AIMER, alarmNode.GetAnimPlayTime() );
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

character_state::states character_alarm_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    states newState = m_CharacterBase.GetStateFromAwareness();
    object *targetObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetTargetGuid());
    m_HasValidTarget = ( targetObject != NULL && targetObject->IsAlive() );

    object *nodeObject = g_ObjMgr.GetObjectByGuid(m_CharacterBase.GetCurrentAlarm());
    if ( !nodeObject || !nodeObject->IsKindOf( alarm_node::GetRTTI() ))
    {   
        return newState;
    }
    else
    {   
        alarm_node &alarmNode = alarm_node::GetSafeType( *nodeObject );    
        if( alarmNode.IsReserved(m_CharacterBase.GetGuid()) )
        {
            alarmNode.InvalidateNode();
            return newState;
        }
    }
    
    if( m_StateDone )
    {
        return newState;
    }
    return STATE_NULL;
}

//=========================================================================

void character_alarm_state::OnThink( void )
{
    character_state::OnThink();
}

//=========================================================================

void character_alarm_state::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader(  "AlarmState",  "Different variables that effect the way that the character behaves when standing still.", 0 );
}

//=========================================================================

xbool character_alarm_state::OnProperty ( prop_query& rPropQuery )
{
    (void)rPropQuery;
    return FALSE ;
}

//=========================================================================

const char*character_alarm_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_ALARM_GOTO_ALARM:
        return "PHASE_ALARM_GOTO_ALARM";
    	break;
    case PHASE_ALARM_FACE_EXACT:
        return "PHASE_ALARM_FACE_EXACT";
    	break;
    case PHASE_ALARM_USE_ALARM:
        return "PHASE_ALARM_USE_ALARM";
    	break;
    }
    return character_state::GetPhaseName(thePhase);
}
