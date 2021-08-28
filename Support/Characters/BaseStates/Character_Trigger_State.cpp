// character_trigger_state : implementation file
/////////////////////////////////////////////////////////////////////////////

#include "character_trigger_state.hpp"
#include "..\Support\TriggerEx\TriggerEx_Object.hpp"
#include "..\Support\TriggerEx\Actions\action_ai_base.hpp"
#include "..\Support\TriggerEx\Actions\action_ai_attack_guid.hpp"
#include "..\Support\TriggerEx\Actions\action_ai_pathto_guid.hpp"
#include "..\Support\TriggerEx\Actions\action_ai_lookat_guid.hpp"
#include "..\Support\TriggerEx\Actions\action_ai_play_anim.hpp"
#include "..\Support\TriggerEx\Actions\action_ai_dialog_line.hpp"
#include "..\Support\TriggerEx\Actions\action_ai_searchto_guid.hpp"
#include "..\Support\TriggerEx\Actions\action_ai_death.hpp"
#include "..\Support\Characters\Character.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "Characters\TaskSystem\character_task_set.hpp"

//=========================================================================
// character_trigger_state class
//=========================================================================

character_trigger_state::character_trigger_state( character& Character, states State )
    : character_state( Character, State )
{
    m_PostTriggerTarget = NULL_GUID;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

void character_trigger_state::OnEnter( void )
{
    m_TaskCompleted = FALSE;
    m_CharacterBase.ClearInitialState();
    m_TriggerData = m_CharacterBase.GetTriggerStateData();
    
    // clear our target so we don't immediately switch out to attack state.
    m_CharacterBase.SetTargetGuid(0);
    m_CharacterBase.ClearSoundHeard();
    m_CharacterBase.SetAwarenessLevel(character::AWARENESS_NONE);
    m_BaseAnimPlayRate = m_CharacterBase.GetLocoPointer()->GetStateAnimRate(loco::STATE_PLAY_ANIM);
    character_state::OnEnter();
}

//=========================================================================

s32 character_trigger_state::UpdatePhase( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 newPhase = PHASE_NONE;
    switch( m_CurrentPhase )
    {
    case PHASE_NONE:
        switch(m_TriggerData.m_ActionType)
        {
        case action_ai_base::AI_ATTACK_GUID :
            newPhase = PHASE_TRIGGER_ATTACK_TARGET;
            break;
        case action_ai_base::AI_PATHFIND_TO_GUID:
            newPhase = PHASE_TRIGGER_GOTO_TARGET;
            break;
        case action_ai_base::AI_LOOK_AT_GUID:
            newPhase = PHASE_TRIGGER_LOOKAT_TARGET;
            break;
        case action_ai_base::AI_DIALOG_LINE:
            newPhase = PHASE_TRIGGER_DIALOG;
            break;
        case action_ai_base::AI_PLAY_ANIMATION:
            newPhase = PHASE_TRIGGER_DO_ACTION;
            break;
        case action_ai_base::AI_SEARCHTO_GUID :
            newPhase = PHASE_TRIGGER_SEARCH_FOR_TARGET;
            break;
        case action_ai_base::AI_DEATH:
            newPhase = PHASE_TRIGGER_DEATH;
            break;
        default:
            ASSERT(FALSE);
            break;
        }
        break;
    case PHASE_TRIGGER_GOTO_TARGET:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( m_TriggerData.m_MustSucceed && !m_CharacterBase.GetGoalSucceeded() )
            {
                newPhase = m_CurrentPhase;
            }
            else if( m_TriggerData.m_UnionData.m_PathfindData.m_YawDifference < R_180 &&
                     m_CharacterBase.GetGoalSucceeded() )
            {                
                // check, are we within the yawdiff, if so no turn.
                object* actionObject = g_ObjMgr.GetObjectByGuid( m_TriggerData.m_ActionFocus );
                if( actionObject )
                {
                    // create the location
                    vector3 positionFacing(0.0f, 0.0f, 2000.0f);
                    positionFacing.Rotate( actionObject->GetL2W().GetRotation() );
                    radian markerYaw = positionFacing.GetYaw();
                    radian characterYaw = m_CharacterBase.GetYaw();
                    if( x_abs(x_MinAngleDiff(markerYaw,characterYaw)) <= m_TriggerData.m_UnionData.m_PathfindData.m_YawDifference )
                    {
                        m_TaskCompleted = TRUE;
                    }
                    else
                    {
                        newPhase = PHASE_TRIGGER_TURNTO_LOCATION;
                    }
                }
            }
            else
            {            
                m_TaskCompleted = TRUE;
            }
        }
        break;
    case PHASE_TRIGGER_LOOKAT_TARGET:
    case PHASE_TRIGGER_TURNTO_LOCATION:
    case PHASE_TRIGGER_DIALOG:
    case PHASE_TRIGGER_DO_ACTION:
        if( m_CharacterBase.GetGoalCompleted() )
        {
            if( m_TriggerData.m_MustSucceed && !m_CharacterBase.GetGoalSucceeded() )
            {            
                newPhase = m_CurrentPhase;
            }
            else
            {
                m_TaskCompleted = TRUE;
            }
        }
        break;
    case PHASE_TRIGGER_ATTACK_TARGET:
    case PHASE_TRIGGER_SEARCH_FOR_TARGET:
    case PHASE_TRIGGER_DEATH:
        m_TaskCompleted = TRUE;
        break;
    default:
        if( m_CurrentPhase >= PHASE_BASE_COUNT )
        {        
            ASSERTS(FALSE,"Invalid Current Phase" );
        }
    };
    
    // trigger state does not obey the base state.
    s32 basePhase = character_state::UpdatePhase(DeltaTime);
    if( basePhase != PHASE_NONE &&
        !m_CharacterBase.GetRootWhenIdle() )
    {
        newPhase = basePhase;
    }    
    return newPhase;
}

//=========================================================================

void character_trigger_state::ChangePhase( s32 newPhase )
{
    object *actionObject = NULL;
    vector3 positionFacing;
    switch( newPhase ) 
    {
    case PHASE_TRIGGER_ATTACK_TARGET:
        {        
            m_CharacterBase.SetTargetGuid( m_TriggerData.m_ActionFocus );
/*            
            object *targetObject = g_ObjMgr.GetObjectByGuid( m_TriggerData.m_ActionFocus );
            if( targetObject )
            {            
                m_CharacterBase.SetAwarenessLevel(character::AWARENESS_TARGET_SPOTTED);
                m_CharacterBase.ForceTargetSeen();
            }*/
        }
    	break;
    case PHASE_TRIGGER_GOTO_TARGET:
        if( m_TriggerData.m_UnionData.m_PathfindData.m_Retreating )
        {
            m_CharacterBase.SetRetreatFromTargetGoal( m_TriggerData.m_ActionFocus, vector3(0.0f,0.0f,0.0f), m_TriggerData.m_UnionData.m_PathfindData.m_MoveStyle, m_TriggerData.m_UnionData.m_PathfindData.m_Distance );
        }
        else
        {        
            m_CharacterBase.SetGotoTargetGoal( m_TriggerData.m_ActionFocus, vector3(0.0f,0.0f,0.0f), m_TriggerData.m_UnionData.m_PathfindData.m_MoveStyle, m_TriggerData.m_UnionData.m_PathfindData.m_Distance, (m_TriggerData.m_UnionData.m_PathfindData.m_Distance < 0.0f) );
        }
    	break;
    case PHASE_TRIGGER_TURNTO_LOCATION:
        // get the facing of our previous target and face that exact same direction.
        actionObject = g_ObjMgr.GetObjectByGuid( m_TriggerData.m_ActionFocus );
        if( actionObject )
        {
            // create the location
            positionFacing = vector3(0.0f, 0.0f, 2000.0f);
            positionFacing.Rotate( actionObject->GetL2W().GetRotation() );
            positionFacing += actionObject->GetPosition();

            m_CharacterBase.SetTurnToLocationGoal( positionFacing, m_TriggerData.m_UnionData.m_PathfindData.m_YawDifference, TRUE );
        }
    	break;
    case PHASE_TRIGGER_LOOKAT_TARGET:
        m_CharacterBase.SetLookAtTargetGoal( m_TriggerData.m_ActionFocus,vector3(0.0f,0.0f,0.0f), m_TriggerData.m_UnionData.m_LookatData.m_LookatDistance, m_TriggerData.m_UnionData.m_LookatData.m_LookatFOV,  m_TriggerData.m_UnionData.m_LookatData.m_LookatHead );
        m_CharacterBase.SetOverrideLookatInterest( m_TriggerData.m_ActionFocus );
    	break;
    case PHASE_TRIGGER_DIALOG:
        m_CharacterBase.SetSayDialogGoal( m_TriggerData.m_UnionData.m_DialogData.m_SoundName,
                                          m_TriggerData.m_UnionData.m_DialogData.m_AnimName,
                                          m_TriggerData.m_UnionData.m_DialogData.m_AnimGroupName, 
                                          DEFAULT_BLEND_TIME,
                                          m_TriggerData.m_UnionData.m_DialogData.m_AnimFlags,                                           
                                          m_TriggerData.m_UnionData.m_DialogData.m_SoundFlags, 
                                          m_TriggerData.m_UnionData.m_DialogData.m_BlockOnDialog, 
                                          m_TriggerData.m_UnionData.m_DialogData.m_KillAnim,
                                          m_TriggerData.m_UnionData.m_DialogData.m_AnimBlendTime );
    	break;
    case PHASE_TRIGGER_DO_ACTION:
        {   
            m_CharacterBase.GetLocoPointer()->SetStateAnimRate(loco::STATE_PLAY_ANIM,1.0f);
            object *scaleTarget = g_ObjMgr.GetObjectByGuid(m_TriggerData.m_ActionFocus );
            if( scaleTarget )
            {
                m_CharacterBase.SetScaledPlayAnimationGoal(m_TriggerData.m_UnionData.m_PlayAnimData.m_AnimName,
                                                           m_TriggerData.m_UnionData.m_PlayAnimData.m_AnimGroupName, 
                                                           m_TriggerData.m_UnionData.m_PlayAnimData.m_AnimBlendTime,
                                                           m_TriggerData.m_UnionData.m_PlayAnimData.m_AnimFlags, 
                                                           m_TriggerData.m_UnionData.m_PlayAnimData.m_AnimPlayTime, 
                                                           scaleTarget->GetPosition() );
            }
            else
            {            
                m_CharacterBase.SetPlayAnimationGoal( m_TriggerData.m_UnionData.m_PlayAnimData.m_AnimName,
                                                      m_TriggerData.m_UnionData.m_PlayAnimData.m_AnimGroupName, 
                                                      DEFAULT_BLEND_TIME,
                                                      m_TriggerData.m_UnionData.m_PlayAnimData.m_AnimFlags, 
                                                      m_TriggerData.m_UnionData.m_PlayAnimData.m_AnimPlayTime );
            }
        }
    	break;
    case PHASE_TRIGGER_SEARCH_FOR_TARGET:
        if ( m_CharacterBase.HasState(STATE_SEARCH) )
        {
            actionObject = g_ObjMgr.GetObjectByGuid( m_TriggerData.m_ActionFocus );
            if( actionObject )
            {
                m_CharacterBase.SetLastLocationOfInterest(actionObject->GetPosition());
                m_CharacterBase.SetLastKnownLocationOfTarget(actionObject->GetPosition());
                m_CharacterBase.SetupState(STATE_SEARCH);
            }
        }
        else
        {
            m_CharacterBase.SetupState(STATE_IDLE);
        }
        break;
        
    case PHASE_TRIGGER_DEATH:
        {        
            // Switch to death state
            m_CharacterBase.SetupState( STATE_DEATH );
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

character_state::states character_trigger_state::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;
    if( m_TaskCompleted )
    {
        return ((character_state::states)m_TriggerData.m_NextAiState);
    }
    else if( m_CharacterBase.GetAwarenessLevel() != character::AWARENESS_NONE && 
             m_CharacterBase.GetAwarenessLevel() != character::AWARENESS_COMBAT_READY )
    {
        LOG_MESSAGE( "character::Trigger Break", "Trigger %08X:%08X Character %08X:%08X due to awareness change to state %s",
            GetTriggerGuid().GetHigh(),
            GetTriggerGuid().GetLow(),
            m_CharacterBase.GetGuid().GetHigh(),
            m_CharacterBase.GetGuid().GetLow(),
            m_CharacterBase.GetStateName(m_CharacterBase.GetStateFromAwareness()) );
        return m_CharacterBase.GetStateFromAwareness();
    }
    return STATE_NULL;
}

//=========================================================================

#ifndef X_RETAIL
void character_trigger_state::OnDebugRender()
{
    object *triggerSource = NULL;
    if( m_TriggerData.m_TaskListGuid )
    {
        triggerSource = g_ObjMgr.GetObjectByGuid( m_TriggerData.m_TaskListGuid );
    }
    else
    {
        triggerSource = g_ObjMgr.GetObjectByGuid( m_TriggerData.m_TriggerGuid );
    }

    if( triggerSource )
    {
        draw_Line( triggerSource->GetPosition(), m_CharacterBase.GetPositionWithOffset(character::OFFSET_CENTER), XCOLOR_BLUE );
    }
}
#endif // X_RETAIL

//=========================================================================

void character_trigger_state::OnThink( void )
{
    character_state::OnThink();
}

//=========================================================================

xbool character_trigger_state::OnExit( void )
{   
    m_CharacterBase.GetLocoPointer()->SetStateAnimRate(loco::STATE_PLAY_ANIM,m_BaseAnimPlayRate);
    if( m_TriggerData.m_TriggerGuid && m_TriggerData.m_Blocking )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_TriggerData.m_TriggerGuid );
        if( pObject && pObject->IsKindOf(trigger_ex_object::GetRTTI()) )
        {
            trigger_ex_object &tempTrigger = trigger_ex_object::GetSafeType( *pObject );
            tempTrigger.ReleaseBlocking();
        }
    }
    else if( m_TriggerData.m_TaskListGuid )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_TriggerData.m_TaskListGuid );
        if( pObject && pObject->IsKindOf(character_task_set::GetRTTI()) )
        {
            character_task_set &taskSet = character_task_set::GetSafeType( *pObject );
            if( m_TaskCompleted )
            {
                taskSet.OnTaskItemComplete();
            }
            else
            {
                taskSet.OnTaskInterrupt();
            }
        }
    }

    // set up the target if we have one.
    if( m_PostTriggerTarget != NULL_GUID )
    {
        m_CharacterBase.SetTargetGuid( m_PostTriggerTarget );
    }

    return character_state::OnExit();
}

//=========================================================================

void character_trigger_state::OnRender( void )
{
}

//=========================================================================

xbool character_trigger_state::IgnoreFlinches()
{
/*    if( m_TriggerData.m_ActionType == action_ai_base::AI_DIALOG_LINE || m_TriggerData.m_ActionType == action_ai_base::AI_PLAY_ANIMATION )
    {
        return TRUE;
    }
    else*/
    // decided we want to flinch, removed to fix bug (they do not flinch when doing dialogs.
    {    
        return FALSE;
    }
}

//=========================================================================

xbool character_trigger_state::OnPain( const pain& Pain )
{ 
    if( m_TriggerData.m_ResponseList.HasFlags(response_list::RF_INVINCIBLE) )
    {
        return FALSE;
    }
    return character_state::OnPain( Pain );
}

//=========================================================================

void character_trigger_state::OnEnumProp( prop_enum& List )
{
    (void)List;
}

//=========================================================================

xbool character_trigger_state::OnProperty ( prop_query& rPropQuery )
{
    (void)rPropQuery;
    return FALSE ;
}

//=========================================================================

const char*character_trigger_state::GetPhaseName ( s32 thePhase )
{
    s32 namedPhase = thePhase;
    if( namedPhase == PHASE_NONE )
    {
        namedPhase = m_CurrentPhase;
    }

    switch( namedPhase ) 
    {
    case PHASE_TRIGGER_ATTACK_TARGET:
        return "PHASE_TRIGGER_ATTACK_TARGET";
    	break;
    case PHASE_TRIGGER_GOTO_TARGET:
        return "PHASE_TRIGGER_GOTO_TARGET";
    	break;
    case PHASE_TRIGGER_LOOKAT_TARGET:
        return "PHASE_TRIGGER_LOOKAT_TARGET";
    	break;
    case PHASE_TRIGGER_TURNTO_LOCATION:
        return "PHASE_TRIGGER_TURNTO_LOCATION";
    	break;
    case PHASE_TRIGGER_DIALOG:
        return "PHASE_TRIGGER_DIALOG";
    	break;
    case PHASE_TRIGGER_DO_ACTION:
        return "PHASE_TRIGGER_DO_ACTION";
        break;
    case PHASE_TRIGGER_DEATH:
        return "PHASE_TRIGGER_DEATH";
        break;
    }
    return character_state::GetPhaseName(thePhase);
}
