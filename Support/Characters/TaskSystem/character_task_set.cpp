//==============================================================================
//
//  character_task_set.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "character_task_set.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Entropy.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Characters\Character.hpp"

#define MAX_TASK_STRING_LEN     255

//=========================================================================
// ENUMS
//=========================================================================

typedef enum_pair<character_task::handle_commands> handle_commands_enum_pair;
typedef enum_pair<character_task_set::char_states_to_transition_to> handle_next_char_state;

static handle_commands_enum_pair s_HandleOnSuccess[] = 
{
    handle_commands_enum_pair("Retry at Top",       character_task::HANDLE_COMMAND_RETRY_AT_TOP),
    handle_commands_enum_pair("Remove TaskList",    character_task::HANDLE_COMMAND_DELETE_TASK),
    handle_commands_enum_pair("Do Nothing",         character_task::HANDLE_COMMAND_DO_NOTHING),
    handle_commands_enum_pair( k_EnumEndStringConst,character_task::INVALID_HANDLE_COMMAND) //**MUST BE LAST**//
};

static handle_next_char_state s_NextCharState[] = 
{
    handle_next_char_state("Idle",                  character_task_set::CHAR_STATE_TRANS_IDLE),
//    handle_next_char_state("Patrol",                character_task_set::CHAR_STATE_TRANS_PATROL),
    handle_next_char_state( k_EnumEndStringConst,   character_task_set::INVALID_CHAR_STATE_TRANS) //**MUST BE LAST**//
};

enum_table<character_task::handle_commands>   character_task_set::m_HandleOnSuccess( s_HandleOnSuccess );              
enum_table<character_task_set::char_states_to_transition_to>  character_task_set::m_CharStatesToTransitionTo( s_NextCharState );              

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================
const f32 c_Sphere_Radius = 50.0f;

static struct character_task_set_desc : public object_desc
{
    character_task_set_desc( void ) : object_desc( object::TYPE_CHARACTER_TASK, 
                                        "Character TaskList",
                                        "SCRIPT",

                                        object::ATTR_NEEDS_LOGIC_TIME,

                                        FLAGS_GENERIC_EDITOR_CREATE | 
                                        FLAGS_TARGETS_OBJS          |
                                        FLAGS_IS_DYNAMIC ) {}

    virtual object* Create( void ) { return new character_task_set; }

#ifdef X_EDITOR

    virtual s32 OnEditorRender( object& Object ) const 
    { 
        object_desc::OnEditorRender(Object); 

        if( Object.IsKindOf( character_task_set::GetRTTI() ) )
        {
            character_task_set& Task = character_task_set::GetSafeType( Object );   
            if (Task.m_bActive)
            {
                //should we blink
                if (Task.m_bBlink)
                {
                    EditorIcon_Draw( EDITOR_ICON_LOOP, Task.GetL2W(), FALSE, XCOLOR_YELLOW);
                }
                Task.m_bBlink = !Task.m_bBlink;
            }
        }
        return EDITOR_ICON_CHARACTER_TASK; 
    }

#endif // X_EDITOR

} s_character_task_set_Desc;

//==============================================================================

const object_desc& character_task_set::GetTypeDesc( void ) const
{
    return s_character_task_set_Desc;
}

//==============================================================================

const object_desc& character_task_set::GetObjectType   ( void )
{
    return s_character_task_set_Desc;
}

//==============================================================================
// character_task_set
//==============================================================================

//==============================================================================
character_task_set::character_task_set(void)
{
    m_OnSuccess         = character_task::HANDLE_COMMAND_DELETE_TASK;

    m_TaskApplyToGuid   = 0;
    m_CurrentTaskIndex  = 0;
    m_TaskCount         = 0;

    m_NextCharState     = CHAR_STATE_TRANS_IDLE;
    m_bEndingTask       = FALSE;
    m_bDelete           = FALSE;
    m_bStartActive      = FALSE;
    m_bActive           = FALSE;
    
    for( s32 i=0; i< MAX_TASK_ARRAY_SIZE ; i++ )
    { 
        m_Task[i]    = NULL;
    }

    m_bBlink = FALSE;
}

//==============================================================================
character_task_set::~character_task_set(void)
{
    for( s32 i=0; i< MAX_TASK_ARRAY_SIZE ; i++ )
    {
        if (m_Task[i] != NULL)
        {
            delete m_Task[i];
            m_Task[i] = NULL;
        }
    }
}

//==============================================================================

#ifndef X_RETAIL
void character_task_set::OnDebugRender ( void )
{
#if defined( ENABLE_DEBUG_MENU )
    if (g_GameLogicDebug) 
    {
        //who am I linked to?
        object* pObject = m_TaskAffecter.GetObjectPtr();
        if ( pObject )
        {
            xcolor ActivatorColor = XCOLOR_YELLOW;
            draw_Line( GetPosition(), pObject->GetBBox().GetCenter(), ActivatorColor );
            draw_BBox( pObject->GetBBox(), ActivatorColor );
#ifdef X_EDITOR
            draw_Label( pObject->GetBBox().GetCenter(), XCOLOR_WHITE, "Task Character" );
#endif // X_EDITOR
        }
    
        if (m_OnSuccess == character_task::HANDLE_COMMAND_RETRY_AT_TOP)
        {
//            draw_editor_icon( EDITOR_ICON_LOOP, GetPosition() + vector3(0.0f, - 15.0f, 0.0f));
        }

        if(GetAttrBits() & ATTR_EDITOR_SELECTED )
        {
//            draw_editor_icon( EDITOR_ICON_CHARACTER_TASK, Pos, xcolor(255,0,0,128) );

            if (m_ActivateOnSuccess != 0)
            {
                object_ptr<object> ObjectPtr(m_ActivateOnSuccess);
    
                if ( ObjectPtr.IsValid() )
                {
                    xcolor ActivatorColor = XCOLOR_BLUE;
                    draw_Line( GetPosition(), ObjectPtr.m_pObject->GetBBox().GetCenter(), ActivatorColor );
                    draw_BBox( ObjectPtr.m_pObject->GetBBox(), ActivatorColor );
#ifdef X_EDITOR
                    draw_Label( ObjectPtr.m_pObject->GetBBox().GetCenter(), XCOLOR_WHITE, "Activate On Success" );
#endif // X_EDITOR
                }
            }

            //render all tasks
#ifdef X_EDITOR
            for( s32 i=0; i< MAX_TASK_ARRAY_SIZE ; i++ )
            { 
                if (m_Task[i])
                {
                    m_Task[i]->OnRender(i);
                }
            }
#endif // X_EDITOR
        }

        if (m_bActive)
        {
            //what task am I on
            if (m_CurrentTaskIndex < m_TaskCount && m_Task[m_CurrentTaskIndex])
            {
                draw_Label( GetPosition(), XCOLOR_WHITE, xfs("[%d,%d]",
                    m_CurrentTaskIndex, m_Task[m_CurrentTaskIndex]->m_SubCurrentTaskIndex) );
            }
        }
    }
#endif
}
#endif // X_RETAIL
     
//==============================================================================

bbox character_task_set::GetLocalBBox( void ) const 
{ 
    return bbox(vector3(0,0,0), c_Sphere_Radius);
}

//=========================================================================

void character_task_set::OnEnumProp( prop_enum&  List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "TaskList",               "The overall task list associated with this object", 0 );

    s32 iTaskHeader = List.PushPath( "TaskList\\" );        
    m_TaskAffecter.OnEnumProp(List,"Target");
    List.PopPath( iTaskHeader );
    
    List.PropEnumButton  ( "TaskList\\AddTask",      "Adds a new task to the Task List.",    PROP_TYPE_MUST_ENUM );

    List.PropEnumInt     ( "TaskList\\TaskCount",    "HIDDEN",    PROP_TYPE_DONT_SHOW );

    for( s32 i=0; i< MAX_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_Task[i])
        {
            List.PropEnumString( xfs("TaskList\\Task[%d]", i) , "An ordered task.", PROP_TYPE_HEADER );        
            s32 iHeader = List.PushPath( xfs("TaskList\\Task[%d]\\", i) );        
            
            m_Task[i]->OnEnumProp(List);
            List.PopPath( iHeader );
        }
    }

    List.PropEnumBool    ( "TaskList\\Start Active",   "Does this task start active, if false a trigger will need to activate it.", 0 );

    List.PropEnumHeader  ( "TaskList\\OnSuccess",                  "The task was completed successfully.", 0 );
    List.PropEnumEnum    ( "TaskList\\OnSuccess\\Action",          m_HandleOnSuccess.BuildString(), "What to do once the task is completed.", 0 );
    List.PropEnumGuid    ( "TaskList\\OnSuccess\\ActivateGuid",    "Optional activation of an object such as a trigger.", 0 );

    List.PropEnumEnum    ( "TaskList\\OnSuccess\\NextState",       m_CharStatesToTransitionTo.BuildString(), "Which state to transition the character to, when done.", 0 );
}

//===========================================================================

xbool character_task_set::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
        return TRUE;
    
    if (I.VarBool("TaskList\\Start Active", m_bStartActive))
        return TRUE;

    s32 iTaskHeader = I.PushPath( "TaskList\\" );        
    if ( m_TaskAffecter.OnProperty( I, "Target" ))
    {    
        return TRUE;
    }

    I.PopPath( iTaskHeader );

    if( I.IsVar( "TaskList\\AddTask" ) )
    {
        if( I.IsRead() )
            I.SetVarButton( "Add Task" );
        else
            AddTasks( ); 
        
        return TRUE;
    }

    if( I.IsVar( "TaskList\\TaskCount" ) )
    {
        if( I.IsRead() )
            I.SetVarInt( m_TaskCount );
        else
            AddTasks( I.GetVarInt() ); 

        return( TRUE );
    }

    if( I.IsSimilarPath( "TaskList\\Task" ) )
    {
        s32 iIndex = I.GetIndex(0);
        
        if (m_Task[iIndex])
        {
            s32 iHeader = I.PushPath( "TaskList\\Task[]\\" );        
            
            if( m_Task[iIndex]->OnProperty(I) )
            {
                I.PopPath( iHeader );
                return TRUE;
            }  
            
            I.PopPath( iHeader );
        }
    }

    if ( I.VarGUID( "TaskList\\OnSuccess\\ActivateGuid", m_ActivateOnSuccess ))
        return TRUE;

    if ( I.IsVar( "TaskList\\OnSuccess\\Action") )
    {
        if( I.IsRead() )
        {
            if ( character_task_set::m_HandleOnSuccess.DoesValueExist( m_OnSuccess ) )
            {
                I.SetVarEnum( character_task_set::m_HandleOnSuccess.GetString( m_OnSuccess ) );
            }
            else
            {
                I.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            character_task::handle_commands HandleCommand;

            if( character_task_set::m_HandleOnSuccess.GetValue( I.GetVarEnum(), HandleCommand ) )
            {
                m_OnSuccess = HandleCommand;
            }
        }
        
        return( TRUE );
    }

    if ( I.IsVar( "TaskList\\OnSuccess\\NextState") )
    {
        if( I.IsRead() )
        {
            if ( character_task_set::m_CharStatesToTransitionTo.DoesValueExist( m_NextCharState ) )
            {
                I.SetVarEnum( character_task_set::m_CharStatesToTransitionTo.GetString( m_NextCharState ) );
            }
            else
            {
                I.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            character_task_set::char_states_to_transition_to NextState;

            if( character_task_set::m_CharStatesToTransitionTo.GetValue( I.GetVarEnum(), NextState ) )
            {
                m_NextCharState = NextState;
            }
        }
        
        return( TRUE );
    }


    return FALSE;
}

//===========================================================================

void character_task_set::AddTasks( s32 nCount )
{
    for (s32 i = 0; i < nCount; i++ )
    {
        if (m_TaskCount < MAX_TASK_ARRAY_SIZE)
        {
            m_Task[m_TaskCount] = character_task::CreateTask(GetGuid());
            m_TaskCount++;
        }
        else
        {
            //we don't support that many tasks
            ASSERT(FALSE);
            return;
        }
    }
}

//=============================================================================

void character_task_set::RemoveTask ( character_task* pTask )
{
    s32 i=0;
    
    for( i = 0; i< MAX_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_Task[i] == pTask)
        {
            delete m_Task[i];
            m_Task[i] = NULL;
            break;
        }
    } 
    
    if ( i == MAX_TASK_ARRAY_SIZE )
    {
        x_DebugMsg("character_task_set::RemoveTask, Cannot find task in table.\n");
        ASSERT(FALSE);
        return;
    }

    //Shift the array to remove the empty slot...
    for ( i++; i < MAX_TASK_ARRAY_SIZE; i++)
    {
        m_Task[i-1] = m_Task[i];
    }

    m_Task[MAX_TASK_ARRAY_SIZE-1] = NULL;

    m_TaskCount--;
}

//=============================================================================

void character_task_set::MoveTaskUp ( character_task* pTask )
{
    s32 i=0;
    
    for( i = 0; i< MAX_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_Task[i] == pTask)
        {
            //found it
            break;
        }
    } 

    if ( i == MAX_TASK_ARRAY_SIZE )
    {
        x_DebugMsg("character_task_set::MoveTask, Cannot find task in table.\n");
        ASSERT(FALSE);
        return;
    }
    
    if ( i == 0 )
    { 
        //already at top of list
        return;
    }

    character_task* pOldTask = m_Task[i-1];

    m_Task[i-1] = pTask;
    m_Task[i]   = pOldTask;
}

//=============================================================================

void character_task_set::MoveTaskDown ( character_task* pTask )
{
    s32 i=0;
    
    for( i = 0; i< MAX_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_Task[i] == pTask)
        {
            //found it
            break;
        }
    } 

    if ( i == MAX_TASK_ARRAY_SIZE )
    {
        x_DebugMsg("character_task_set::MoveTask, Cannot find task in table.\n");
        ASSERT(FALSE);
        return;
    }
    
    if ( i == (m_TaskCount-1) )
    { 
        //already at bottom of list
        return;
    }

    character_task* pOldTask = m_Task[i+1];

    m_Task[i+1] = pTask;
    m_Task[i]   = pOldTask;
}

//=============================================================================

void character_task_set::ActivateTask( void )
{
    if (m_bActive)
    {
#ifdef X_EDITOR
        x_DebugMsg("ERROR: Task System character_task_set::ActivateTask for set guid[%s], you attempted to reactivate an already active task!\n",
            guid_ToString(GetGuid()));
#else // X_EDITOR
        x_DebugMsg("ERROR: Task System character_task_set::ActivateTask, you attempted to reactivate an already active task!\n");
#endif // X_EDITOR
        return;
    }

    ResetIndexPtrs(FALSE);

    //do something
    object* pObject = m_TaskAffecter.GetObjectPtr();
    if (pObject)
    {
        m_TaskApplyToGuid = pObject->GetGuid();
    }
    else
    {
        m_TaskApplyToGuid = 0;
        x_DebugMsg("Failed to Activate Task due to invalid guid.\n");
        return;
    }

    object* pCharObject = g_ObjMgr.GetObjectByGuid(m_TaskApplyToGuid);
    if ( pCharObject && pCharObject->IsKindOf( character::GetRTTI() ) == TRUE )
    {
        ((character *)pCharObject)->SetPendingTaskListGuid(GetGuid());
    }        
//    AssignNextTask();
}

//=============================================================================

void character_task_set::OnActivate ( xbool Flag )
{
    object* pCharObject = g_ObjMgr.GetObjectByGuid(m_TaskApplyToGuid);
    if ( pCharObject && pCharObject->IsKindOf( character::GetRTTI() ) == TRUE )
    {
        ((character *)pCharObject)->SetupState(character_state::STATE_IDLE);
    }        
    
    if (Flag)
    {
        ActivateTask();
        m_bActive = TRUE;
    }
    else
    {
        //turn task off
        m_bActive = FALSE;
        if ( pCharObject && pCharObject->IsKindOf( character::GetRTTI() ) == TRUE )
        {
            ((character *)pCharObject)->SetPendingTaskListGuid(0);
        }        
    }
}

//=============================================================================

xbool character_task_set::AssignNextTask( f32 DeltaTime )
{
    m_bActive = TRUE;

    xbool bTasksLeft = FALSE;
    while(!bTasksLeft)
    {
        if (m_CurrentTaskIndex < m_TaskCount && m_Task[m_CurrentTaskIndex])
        {
            bTasksLeft = m_Task[m_CurrentTaskIndex]->ExecuteTask( DeltaTime );
        }
        else
        {
            //out of tasks, must be done                
            OnTaskListFinished();
            if (m_OnSuccess == character_task::HANDLE_COMMAND_DELETE_TASK)
            {
                MarkForDeletion();
                break;
            }
        }

        if (!bTasksLeft)
        {
            //no good tasks left, so increment task pointer
            m_CurrentTaskIndex++;
        }
    }
    return bTasksLeft;
}

//=============================================================================

void character_task_set::OnTaskItemComplete ( void )
{
    if (m_CurrentTaskIndex < m_TaskCount && m_Task[m_CurrentTaskIndex])
    {
        m_Task[m_CurrentTaskIndex]->OnTaskItemCompleted();
    }
    else
    {
        //something went wrong somewhere... helpful huh
#ifdef X_EDITOR
        x_DebugMsg("ERROR character_task_set::OnTaskItemComplete, set guid[%s] has no tasks available!\n",
            guid_ToString(GetGuid()));
#endif // X_EDITOR
    }
//    AssignNextTask();
}

//=============================================================================

void character_task_set::OnTaskFailure ( void )
{
    if (m_CurrentTaskIndex < m_TaskCount && m_Task[m_CurrentTaskIndex])
    {
        m_Task[m_CurrentTaskIndex]->OnTaskFailure();
    }
    else
    {
        //something went wrong somewhere... helpful huh
#ifdef X_EDITOR
        x_DebugMsg("ERROR character_task_set::OnTaskFailure, set guid[%s] has no tasks available!\n",
            guid_ToString(GetGuid()));
#endif // X_EDITOR
    }
}

//=============================================================================

xbool character_task_set::OnTaskInterrupt ( void )
{
    if (m_CurrentTaskIndex < m_TaskCount && m_Task[m_CurrentTaskIndex])
    {
        return m_Task[m_CurrentTaskIndex]->OnTaskInterrupt();
    }
    else if (!m_bEndingTask)
    {
        //something went wrong somewhere... helpful huh
#ifdef X_EDITOR
        x_DebugMsg("ERROR character_task_set::OnTaskInterrupt, set guid[%s] has no tasks available!\n",
            guid_ToString(GetGuid()));
#endif // X_EDITOR
    }

    return FALSE;
}

//=============================================================================

void character_task_set::OnTaskListFinished( void )
{   
    if (m_ActivateOnSuccess != 0)
    {
        object_ptr<object>  ObjectPtr( m_ActivateOnSuccess );
    
        if ( ObjectPtr.IsValid() )
        {
            ObjectPtr.m_pObject->OnActivate(TRUE);
        }
    }    

    switch(m_OnSuccess)
    {
        case character_task::HANDLE_COMMAND_RETRY_AT_TOP:
            //restart tasks
            ResetIndexPtrs(FALSE);
            m_CurrentTaskIndex = -1; //will get incremented to 0 below
            break;
        case character_task::HANDLE_COMMAND_DO_NOTHING:
        case character_task::HANDLE_COMMAND_DELETE_TASK:
        default:
            {
                object* pCharObject = g_ObjMgr.GetObjectByGuid(m_TaskApplyToGuid);
                if ( pCharObject && pCharObject->IsKindOf( character::GetRTTI() ) == TRUE )
                {
                    m_bEndingTask = TRUE; //ignore the interrupt

                    character* pCharacter = (character*)pCharObject;
                    switch(m_NextCharState)
                    {
/*                        case CHAR_STATE_TRANS_PATROL:
                            pCharacter->SetupState(character_state::STATE_PATROL);
                            break;*/
                        case CHAR_STATE_TRANS_IDLE:
                        default:
                            pCharacter->SetupState(character_state::STATE_IDLE);
                            break;
                    }
                } 
            }
            break;
    }
}

//=============================================================================

void character_task_set::ResetIndexPtrs( xbool bOnlySubTasks )
{
    if (!bOnlySubTasks)
    {
        //reset main task list
        m_CurrentTaskIndex = 0;
    }

    //now reset sub task lists
    for( s32 i=0; i< MAX_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_Task[i])
        {
            m_Task[i]->m_SubCurrentTaskIndex = 0;
        }
    }
}

//=============================================================================

void character_task_set::ResetTaskListToTop( void )
{
    ResetIndexPtrs(FALSE);
//    AssignNextTask(FALSE);
}

//=============================================================================

void character_task_set::ResetTaskListToCurrent( void )
{
    ResetIndexPtrs(TRUE);
//    AssignNextTask(FALSE);
}

//=============================================================================

void character_task_set::ResetTaskListToNext( void )
{
    ResetIndexPtrs(TRUE);
    m_CurrentTaskIndex++;

    //make sure we don't go past the end of the array
    if (m_CurrentTaskIndex >= m_TaskCount)
    {
        m_CurrentTaskIndex = m_TaskCount-1;
    }
//    AssignNextTask(FALSE);
}

//=============================================================================

void character_task_set::MarkForDeletion()
{
    //set up logic time
    m_bDelete = TRUE;
    
    //SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
}

//=============================================================================

void character_task_set::OnAdvanceLogic( f32 DeltaTime )
{
    (void)DeltaTime;

    //remove logic time immediately
    //only want 1 pass through unless reactivated
    //SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );

    if (m_bStartActive)
    {
        m_bStartActive = FALSE;
        ActivateTask();
    }

    if (m_bActive)
    {
        //do object polling to test for failure
        if (m_TaskApplyToGuid != 0)
        {
            if (!g_ObjMgr.GetObjectByGuid(m_TaskApplyToGuid))
            {
                //oh no object does not exist
                OnTaskFailure();
            }
        }
    }

    //do delete last
    if (m_bDelete)
    {
        //got logic time, must mean we need to delete ourselves
        g_ObjMgr.DestroyObject(GetGuid());
    }
}

//=============================================================================

/*void character_task_set::PreloadNextSound( void )
{
    //can only do this if we have a proper guid
    if (m_TaskApplyToGuid != 0)
    {
        //get the next task
        s32 nTaskToCheck = m_CurrentTaskIndex;
        while(TRUE)
        {
            if (nTaskToCheck < m_TaskCount && m_Task[nTaskToCheck])
            {
                //this is the task to check, get its next sub task
                s32 nSubTaskIndex = m_Task[nTaskToCheck]->m_SubCurrentTaskIndex+1;
                if (nSubTaskIndex < m_Task[nTaskToCheck]->m_SubTaskCount )
                {
                    if (m_Task[nTaskToCheck]->m_SubTask[nSubTaskIndex])
                    {
                        //if this next task is a dialog line, then play it!
                        if (m_Task[nTaskToCheck]->m_SubTask[nSubTaskIndex]->GetType() == character_sub_task::AST_SAY_DIALOG_LINE)
                        {
                            //warm up this sound
                            dialog_line* pSubTask = (dialog_line*)m_Task[nTaskToCheck]->m_SubTask[nSubTaskIndex];
                            pSubTask->WarmupSound(m_TaskApplyToGuid);
                        }
                    }
                    return;
                }
            }
            else
            {
                //no more tasks
                return;
            }

            //no good sub-tasks left, so increment task pointer
            nTaskToCheck++;
        }
    }
}*/
