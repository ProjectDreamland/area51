//==============================================================================
//
//  character_task.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "character_task.hpp"
#include "character_task_set.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

static const s32        MAX_STRING_TASK_DESC_LEN = 255;

//=========================================================================
// STATIC FUNCTIONS
//=========================================================================

character_task* character_task::CreateTask( guid SetGuid )
{
    character_task* pTask = new character_task();
    pTask->m_SetGuid = SetGuid;
    return pTask;
}

//=========================================================================
// ENUMS
//=========================================================================

typedef enum_pair<character_task::handle_commands> handle_commands_enum_pair;

static handle_commands_enum_pair s_HandleOnInterrupt[] = 
{
    handle_commands_enum_pair("Retry at current",         character_task::HANDLE_COMMAND_RETRY_AT_CURRENT),
    handle_commands_enum_pair("Retry at Top",             character_task::HANDLE_COMMAND_RETRY_AT_TOP),
    handle_commands_enum_pair("Next Task",                character_task::HANDLE_COMMAND_MOVE_TO_NEXT_TASK),
//    handle_commands_enum_pair("Reassign at current",      character_task::HANDLE_COMMAND_REASSIGN_AT_CURRENT),
//    handle_commands_enum_pair("Reassign at top",          character_task::HANDLE_COMMAND_REASSIGN_AT_TOP),
    handle_commands_enum_pair("Remove TaskList",          character_task::HANDLE_COMMAND_DELETE_TASK),
    handle_commands_enum_pair( k_EnumEndStringConst,      character_task::INVALID_HANDLE_COMMAND) //**MUST BE LAST**//
};

static handle_commands_enum_pair s_HandleOnFailure[] = 
{
    handle_commands_enum_pair("Reassign at current",      character_task::HANDLE_COMMAND_REASSIGN_AT_CURRENT),
    handle_commands_enum_pair("Reassign at top",          character_task::HANDLE_COMMAND_REASSIGN_AT_TOP),
    handle_commands_enum_pair("Remove TaskList",          character_task::HANDLE_COMMAND_DELETE_TASK),
    handle_commands_enum_pair( k_EnumEndStringConst,      character_task::INVALID_HANDLE_COMMAND) //**MUST BE LAST**//
};

enum_table<character_task::handle_commands>  character_task::m_HandleOnInterrupt( s_HandleOnInterrupt );              
enum_table<character_task::handle_commands>  character_task::m_HandleOnFailure( s_HandleOnFailure );              


//=========================================================================
// FUNCTIONS
//=========================================================================

//==============================================================================
character_task::character_task(void)
{
    m_OnInterrupt           = HANDLE_COMMAND_DELETE_TASK;
    m_OnFailure             = HANDLE_COMMAND_DELETE_TASK;

    m_SubTasksChooser       = actions_ex_base::TYPE_ACTION_AI_PATHTO_GUID;

    m_SubTaskCount          = 0;
    m_SubCurrentTaskIndex   = 0;

    for( s32 i=0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    { 
        m_SubTask[i]    = NULL;
    }
}

//==============================================================================
character_task::~character_task(void)
{
    for( s32 i=0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    {
        if (m_SubTask[i] != NULL)
        {
            delete m_SubTask[i];
            m_SubTask[i] = NULL;
        }
    }
}

//==============================================================================

void character_task::OnRender ( s32 TaskId )
{
    (void)TaskId;

#ifndef X_RETAIL
    //render all tasks
    for( s32 i=0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_SubTask[i])
        {
            m_SubTask[i]->OnDebugRender(i);
        }
    }
#endif // X_RETAIL
}

//=========================================================================

void character_task::OnEnumProp( prop_enum&  List )
{
    List.PropEnumButton  ( "MoveUp",      "Move this task up the list.",    PROP_TYPE_MUST_ENUM );
    List.PropEnumButton  ( "MoveDown",    "Move this task down the list.",    PROP_TYPE_MUST_ENUM );
    List.PropEnumButton  ( "DeleteTask",  "Remove this Task from the List.",    PROP_TYPE_MUST_ENUM );
/*
    List.PropEnumHeader  ( "ResponseList",                   "List Various criteria for ignoring external stimuli");
    List.PropEnumBool    ( "ResponseList\\IgnoreAttacks",    "Ignore all attacks and stick to tasks.");
    List.PropEnumBool    ( "ResponseList\\IgnoreSight",      "Ignore all spotted enemies and stick to tasks.");
    List.PropEnumBool    ( "ResponseList\\IgnoreSound",      "Ignore all threatening sounds and stick to tasks.");
    List.PropEnumBool    ( "ResponseList\\IgnoreAlerts",     "Ignore all alerts and stick to tasks.");
    List.PropEnumBool    ( "ResponseList\\Invincible",       "Ignore all pain and damage and stick to tasks. (use with caution)");
*/
    m_ResponseList.OnEnumProp(List);

    List.PropEnumHeader  ( "OnSuccess",                  "The task was completed successfully.", 0 );
    List.PropEnumGuid    ( "OnSuccess\\ActivateGuid",    "Optional activation of an object such as a trigger.", 0 );

    List.PropEnumHeader  ( "OnInterrupt",                "While performing this task, the character was interrupted (Attacked, Saw Enemy, Heard Enemy)", 0 );
    List.PropEnumEnum    ( "OnInterrupt\\Action",        m_HandleOnInterrupt.BuildString(), "What to do if we are interrupted during this task", 0 );
    List.PropEnumGuid    ( "OnInterrupt\\ActivateGuid",  "Optional activation of an object such as a trigger.", 0 );

    List.PropEnumHeader  ( "OnFailure",                  "The character failed to perform the task (Character Died, or impossibly stuck)", 0 );
    List.PropEnumEnum    ( "OnFailure\\Action",          m_HandleOnFailure.BuildString(), "What to do if we are interrupted during this task", 0 );
    List.PropEnumGuid    ( "OnFailure\\ActivateGuid",    "Optional activation of an object such as a trigger.", 0 );

//    List.PropEnumEnum    ( "Choose SubTask",             character_sub_task::m_AtomicSubTaskList.BuildString(), "Pick a sub task to add to this task." );
    List.PropEnumEnum    ( "Choose AI SubTask",          actions_ex_base::m_ActionsAIEnum.BuildString(),            "Types of AI actions available." ,PROP_TYPE_DONT_SAVE );
    List.PropEnumEnum    ( "Choose Meta SubTask",        actions_ex_base::m_ActionsTaskEnum.BuildString(),          "Types of Meta actions available." ,PROP_TYPE_DONT_SAVE );
    List.PropEnumButton  ( "Add SubTask",                "Add a new subtask",    PROP_TYPE_MUST_ENUM );

    s32 i = 0;
    for( i=0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_SubTask[i] != NULL)
        {
            List.PropEnumInt( xfs("SubTaskType[%d]", i), "", PROP_TYPE_DONT_SHOW );
        }
    }

    for( i=0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_SubTask[i])
        {
            List.PropEnumString( xfs("SubTask[%d]", i) , "An ordered sub-task.", PROP_TYPE_HEADER );        
            s32 iHeader = List.PushPath( xfs("SubTask[%d]\\", i) );        
            
            m_SubTask[i]->OnEnumProp(List);
            List.PopPath( iHeader );
        }
    }
}

//===========================================================================

xbool character_task::OnProperty( prop_query& I )
{
/*    if ( I.IsVar( "ResponseList\\IgnoreAttacks") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(m_TaskFlags & TF_IGNORE_ATTACKS);
        }
        else
        {
            if (I.GetVarBool())
            {
                m_TaskFlags = m_TaskFlags | TF_IGNORE_ATTACKS;
            }
            else
            {
                m_TaskFlags = m_TaskFlags & ~TF_IGNORE_ATTACKS;
            }
        }
        return TRUE;
    }

    if ( I.IsVar( "ResponseList\\IgnoreSight") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(m_TaskFlags & TF_IGNORE_SIGHT);
        }
        else
        {
            if (I.GetVarBool())
            {
                m_TaskFlags = m_TaskFlags | TF_IGNORE_SIGHT;
            }
            else
            {
                m_TaskFlags = m_TaskFlags & ~TF_IGNORE_SIGHT;
            }
        }
        return TRUE;
    }

    if ( I.IsVar( "ResponseList\\IgnoreSound") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(m_TaskFlags & TF_IGNORE_SOUND);
        }
        else
        {
            if (I.GetVarBool())
            {
                m_TaskFlags = m_TaskFlags | TF_IGNORE_SOUND;
            }
            else
            {
                m_TaskFlags = m_TaskFlags & ~TF_IGNORE_SOUND;
            }
        }
        return TRUE;
    }

    if ( I.IsVar( "ResponseList\\IgnoreAlerts") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(m_TaskFlags & TF_IGNORE_ALERTS);
        }
        else
        {
            if (I.GetVarBool())
            {
                m_TaskFlags = m_TaskFlags | TF_IGNORE_ALERTS;
            }
            else
            {
                m_TaskFlags = m_TaskFlags & ~TF_IGNORE_ALERTS;
            }
        }
        return TRUE;
    }
    
    if ( I.IsVar( "ResponseList\\Invincible") )
    {
        if( I.IsRead() )
        {
            I.SetVarBool(m_TaskFlags & TF_INVINCIBLE);
        }
        else
        {
            if (I.GetVarBool())
            {
                m_TaskFlags = m_TaskFlags | TF_INVINCIBLE;
            }
            else
            {
                m_TaskFlags = m_TaskFlags & ~TF_INVINCIBLE;
            }
        }
        return TRUE;
    }*/
    if( m_ResponseList.OnProperty( I ) )
    {    
        return TRUE;
    }

    if ( I.VarGUID( "OnSuccess\\ActivateGuid", m_ActivateOnSuccess ))
        return TRUE;

    if ( I.IsVar( "OnInterrupt\\Action") )
    {
        if( I.IsRead() )
        {
            if ( character_task::m_HandleOnInterrupt.DoesValueExist( m_OnInterrupt ) )
            {
                I.SetVarEnum( character_task::m_HandleOnInterrupt.GetString( m_OnInterrupt ) );
            }
            else
            {
                I.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            character_task::handle_commands HandleCommand;

            if( character_task::m_HandleOnInterrupt.GetValue( I.GetVarEnum(), HandleCommand ) )
            {
                m_OnInterrupt = HandleCommand;
            }
        }
        
        return( TRUE );
    }
    
    if ( I.VarGUID( "OnInterrupt\\ActivateGuid", m_ActivateOnInterrupt ))
        return TRUE;

    if ( I.IsVar( "OnFailure\\Action") )
    {
        if( I.IsRead() )
        {
            if ( character_task::m_HandleOnFailure.DoesValueExist( m_OnFailure ) )
            {
                I.SetVarEnum( character_task::m_HandleOnFailure.GetString( m_OnFailure ) );
            }
            else
            {
                I.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            character_task::handle_commands HandleCommand;

            if( character_task::m_HandleOnFailure.GetValue( I.GetVarEnum(), HandleCommand ) )
            {
                m_OnFailure = HandleCommand;
            }
        }
        
        return( TRUE );
    }

    if ( I.VarGUID( "OnFailure\\ActivateGuid", m_ActivateOnFailure ))
        return TRUE;

    if ( I.IsVar( "Choose AI SubTask") )
    {
        if( I.IsRead() )
        {
            if ( actions_ex_base::m_ActionsAIEnum.DoesValueExist( m_SubTasksChooser ) )
            {
                I.SetVarEnum( actions_ex_base::m_ActionsAIEnum.GetString( m_SubTasksChooser ) );
            }
            else
            {
                I.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            actions_ex_base::action_ex_types SubTasks;

            if( actions_ex_base::m_ActionsAIEnum.GetValue( I.GetVarEnum(), SubTasks ) )
            {
                m_SubTasksChooser = SubTasks;
            }
        }
        
        return( TRUE );
    }

    if ( I.IsVar( "Choose Meta SubTask") )
    {
        if( I.IsRead() )
        {
            if ( actions_ex_base::m_ActionsTaskEnum.DoesValueExist( m_SubTasksChooser ) )
            {
                I.SetVarEnum( actions_ex_base::m_ActionsTaskEnum.GetString( m_SubTasksChooser ) );
            }
            else
            {
                I.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            actions_ex_base::action_ex_types SubTasks;

            if( actions_ex_base::m_ActionsTaskEnum.GetValue( I.GetVarEnum(), SubTasks ) )
            {
                m_SubTasksChooser = SubTasks;
            }
        }
        return( TRUE );
    }

    if( I.IsVar( "Add SubTask" ) )
    {
        if( I.IsRead() )
            I.SetVarButton( "Add" );
        else
            AddSubtask( m_SubTasksChooser ); 
        
        return TRUE;
    }

    if( I.IsSimilarPath( "SubTaskType[" ) )
    {
        s32 iIndex = I.GetIndex(1);
        
        ASSERT( iIndex < MAX_SUB_TASK_ARRAY_SIZE && iIndex >= 0 );

        if( I.IsRead() )
        {        
            if (m_SubTask[iIndex] != NULL)
            {
                I.SetVarInt( m_SubTask[iIndex]->GetType() );
            }
        }
        else
        { 
            s32 SubTaskType = -1;
            
            SubTaskType = I.GetVarInt();
            
            //for backwards compatibility
            AddSubtask( (actions_ex_base::action_ex_types) SubTaskType );
        }
        
        return( TRUE );
    }

    if( I.IsSimilarPath( "SubTask" ) )
    {
        s32 iIndex = I.GetIndex(1);
        
        ASSERT( iIndex < MAX_SUB_TASK_ARRAY_SIZE && iIndex >= 0 );
  
        if ( I.IsVar( "SubTask[]" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarString(m_SubTask[iIndex]->GetDescription(), MAX_STRING_TASK_DESC_LEN );
            }

            return TRUE;
        }

        if (m_SubTask[iIndex])
        {
            s32 iHeader = I.PushPath( "SubTask[]\\" );        
            
            if( m_SubTask[iIndex]->OnProperty(I) )
            {
                I.PopPath( iHeader );
                return TRUE;
            }  
            
            I.PopPath( iHeader );
        }
    }

    //base class calls follow
    object_ptr<character_task_set> SetPtr ( m_SetGuid );
    if (!SetPtr.IsValid())
    {
        return FALSE;
    }

    if( I.IsVar( "MoveUp" ) )
    {
        if( I.IsRead() )
            I.SetVarButton( "Up" );
        else
            SetPtr.m_pObject->MoveTaskUp( this ); 
        
        return TRUE;
    }

    if( I.IsVar( "MoveDown" ) )
    {
        if( I.IsRead() )
            I.SetVarButton( "Down" );
        else
            SetPtr.m_pObject->MoveTaskDown( this ); 
        
        return TRUE;
    }

    if( I.IsVar( "DeleteTask" ) )
    {
        if( I.IsRead() )
            I.SetVarButton( "Delete" );
        else
            SetPtr.m_pObject->RemoveTask( this ); 
        
        return TRUE;
    }

    return FALSE;
}

//===========================================================================

void character_task::AddSubtask( actions_ex_base::action_ex_types Type )
{
    if (m_SubTaskCount < MAX_SUB_TASK_ARRAY_SIZE)
    {
        m_SubTask[m_SubTaskCount] = (action_ai_base *)actions_ex_base::CreateAction(Type, 0);
        m_SubTask[m_SubTaskCount]->SetTaskOwner(this);
        if( m_SubTask[m_SubTaskCount]->IsKindOf(action_ai_base::GetRTTI()) )
        {
            action_ai_base &actionAIBase = action_ai_base::GetSafeType( *m_SubTask[m_SubTaskCount] );
            actionAIBase.SetResponseFlags( m_ResponseList.GetFlags() );
        }
        m_SubTaskCount++;
    }
    else
    {
        //we don't support that many subtasks
        ASSERT(FALSE);
        return;
    }
}

//=============================================================================

void character_task::RemoveSubTask ( action_ai_base* pSubTask )
{
    s32 i=0;
    
    for( i = 0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_SubTask[i] == pSubTask)
        {
            delete m_SubTask[i];
            m_SubTask[i] = NULL;
            break;
        }
    } 
    
    if ( i == MAX_SUB_TASK_ARRAY_SIZE )
    {
        x_DebugMsg("character_task::RemoveSubTask, Cannot find task in table.\n");
        ASSERT(FALSE);
        return;
    }

    //Shift the array to remove the empty slot...
    for ( i++; i < MAX_SUB_TASK_ARRAY_SIZE; i++)
    {
        m_SubTask[i-1] = m_SubTask[i];
    }

    m_SubTask[MAX_SUB_TASK_ARRAY_SIZE-1] = NULL;

    m_SubTaskCount--;
}

//=============================================================================

s32 character_task::GetSubTaskIndex ( action_ai_base* pSubTask )
{
    for( s32 i = 0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_SubTask[i] == pSubTask)
        {
            //found it
            return i;
        }
    } 
    
    return -1;
}

//=============================================================================

void character_task::SetSubTaskIndex ( action_ai_base* pSubTask, s32 Index )
{
    if ( Index < 0 )
    {
        return;
    }

    if (!pSubTask)
        return;

    //if the index is too high, set it to the highest used index
    if (Index >= m_SubTaskCount)
        Index = m_SubTaskCount - 1;

    for( s32 i = 0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_SubTask[i] == pSubTask)
        {
            //found the right action, first shift the array to fill in the empty spot
            for ( i++; i < MAX_SUB_TASK_ARRAY_SIZE; i++)
            {
                m_SubTask[i-1] = m_SubTask[i];
            }
            m_SubTask[MAX_SUB_TASK_ARRAY_SIZE-1] = NULL;

            //now clear out the desired index
            for ( i = MAX_SUB_TASK_ARRAY_SIZE-1; i > Index; i--)
            {
                m_SubTask[i] = m_SubTask[i-1];
            }
            m_SubTask[Index] = pSubTask;

            return;
        }
    }
    
    ASSERT(FALSE);
}

//=============================================================================

void character_task::MoveSubTaskUp ( action_ai_base* pSubTask )
{
    s32 i=0;
    
    for( i = 0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_SubTask[i] == pSubTask)
        {
            //found it
            break;
        }
    } 

    if ( i == MAX_SUB_TASK_ARRAY_SIZE )
    {
        x_DebugMsg("character_task::MoveSubTask, Cannot find task in table.\n");
        ASSERT(FALSE);
        return;
    }
    
    if ( i == 0 )
    { 
        //already at top of list
        return;
    }

    actions_ex_base* pOldSubTask = m_SubTask[i-1];

    m_SubTask[i-1] = pSubTask;
    m_SubTask[i]   = pOldSubTask;
}

//=============================================================================

void character_task::MoveSubTaskDown ( action_ai_base* pSubTask )
{
    s32 i=0;
    
    for( i = 0; i< MAX_SUB_TASK_ARRAY_SIZE ; i++ )
    { 
        if (m_SubTask[i] == pSubTask)
        {
            //found it
            break;
        }
    } 

    if ( i == MAX_SUB_TASK_ARRAY_SIZE )
    {
        x_DebugMsg("character_task::MoveSubTask, Cannot find task in table.\n");
        ASSERT(FALSE);
        return;
    }
    
    if ( i == (m_SubTaskCount-1) )
    { 
        //already at bottom of list
        return;
    }

    actions_ex_base* pOldSubTask = m_SubTask[i+1];

    m_SubTask[i+1] = pSubTask;
    m_SubTask[i]   = pOldSubTask;
}

//===========================================================================

//return TRUE if there is a good task
xbool character_task::ExecuteTask( f32 DeltaTime )
{
    if (m_SubCurrentTaskIndex < m_SubTaskCount )
    {
        if (m_SubTask[m_SubCurrentTaskIndex])
        {
            // make sure our affector and flags are properly setup.
            if( m_SubTask[m_SubCurrentTaskIndex]->IsKindOf(action_ai_base::GetRTTI()) )
            {
                action_ai_base &actionAIBase = action_ai_base::GetSafeType( *m_SubTask[m_SubCurrentTaskIndex] );
                actionAIBase.SetResponseFlags( m_ResponseList.GetFlags() );
                object_ptr<character_task_set> SetPtr ( m_SetGuid );
                if( SetPtr.IsValid() )
                {        
                    actionAIBase.SetCharacterAffecter( SetPtr.m_pObject->GetObjectAffecter() );
                }
            // then execute!
                m_SubTask[m_SubCurrentTaskIndex]->Execute(DeltaTime);
            }
            else
            {
                if( m_SubTask[m_SubCurrentTaskIndex]->Execute(DeltaTime) )
                {
                    OnTaskItemCompleted();
                }
            }
            return TRUE;
        }
    }

    //no sub tasks left in this task
    return FALSE;
}

//===========================================================================

void character_task::OnTaskItemCompleted( void )
{
    m_SubCurrentTaskIndex++;

    if (m_SubCurrentTaskIndex >= m_SubTaskCount)
    {
        //must be last subtask
        if (m_ActivateOnSuccess != 0)
        {
            object_ptr<object>  ObjectPtr( m_ActivateOnSuccess );
    
            if ( ObjectPtr.IsValid() )
            {
                ObjectPtr.m_pObject->OnActivate(TRUE);
            }
        }
    }
}

//=============================================================================

void character_task::OnTaskFailure ( void )
{
    //handle failure cases
    switch(m_OnFailure)
    {
    case HANDLE_COMMAND_REASSIGN_AT_CURRENT:
//        ASSERT(FALSE);
        break;
    case HANDLE_COMMAND_REASSIGN_AT_TOP:
//        ASSERT(FALSE);
        break;
    case HANDLE_COMMAND_DELETE_TASK:
        {
            object_ptr<character_task_set> SetPtr ( m_SetGuid );
            if (SetPtr.IsValid())
            {
                SetPtr.m_pObject->MarkForDeletion();
            }            
        }
        break;
    default: 
        ASSERT(FALSE);
        break;
    }
    
    if (m_ActivateOnFailure != 0)
    {
        object_ptr<object>  ObjectPtr( m_ActivateOnFailure );
    
        if ( ObjectPtr.IsValid() )
        {
            ObjectPtr.m_pObject->OnActivate(TRUE);
        }
    }
}

//=============================================================================

xbool character_task::OnTaskInterrupt ( void )
{
    xbool bReturn = FALSE;

    if (m_ActivateOnInterrupt != 0)
    {
        object_ptr<object>  ObjectPtr( m_ActivateOnInterrupt );
    
        if ( ObjectPtr.IsValid() )
        {
            ObjectPtr.m_pObject->OnActivate(TRUE);
        }
    }

    object_ptr<character_task_set> SetPtr ( m_SetGuid );
    if (!SetPtr.IsValid())
    {
        return FALSE;
    }

    // handle interrupt cases
    switch(m_OnInterrupt)
    {
    case HANDLE_COMMAND_RETRY_AT_CURRENT:
        m_SubCurrentTaskIndex = 0;
        SetPtr.m_pObject->ResetTaskListToCurrent();
        bReturn = TRUE;
        break;
    case HANDLE_COMMAND_RETRY_AT_TOP:
        m_SubCurrentTaskIndex = 0;
        SetPtr.m_pObject->ResetTaskListToTop();
        bReturn = TRUE;
        break;
    case HANDLE_COMMAND_REASSIGN_AT_CURRENT:
        ASSERT(FALSE); 
        break;
    case HANDLE_COMMAND_REASSIGN_AT_TOP:
        ASSERT(FALSE); 
        break;
    case HANDLE_COMMAND_MOVE_TO_NEXT_TASK:
        m_SubCurrentTaskIndex = 0;
        SetPtr.m_pObject->ResetTaskListToNext();
        bReturn = TRUE;
        break;
    case HANDLE_COMMAND_DELETE_TASK:
        SetPtr.m_pObject->MarkForDeletion();
        break;
    default: 
        ASSERT(FALSE); 
        break;
    }
    
    return bReturn;
}

//=============================================================================
