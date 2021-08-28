///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  character_task.hpp
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef character_task_hpp
#define character_task_hpp

///////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Obj_mgr\obj_mgr.hpp"
//#include "character_sub_task.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "TriggerEx\Actions\action_ai_base.hpp"
#include "TriggerEx\TriggerEX_Actions.hpp"
#include "Characters\ResponseList.hpp"

#define MAX_SUB_TASK_ARRAY_SIZE     48

///////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS
///////////////////////////////////////////////////////////////////////////////////////////////////

class character_task : public prop_interface
{
public:

    enum handle_commands
    {
        INVALID_HANDLE_COMMAND = -1,
 
        HANDLE_COMMAND_RETRY_AT_CURRENT,            //0
        HANDLE_COMMAND_RETRY_AT_TOP,                //1
        HANDLE_COMMAND_REASSIGN_AT_CURRENT,         //2
        HANDLE_COMMAND_REASSIGN_AT_TOP,             //3
        HANDLE_COMMAND_DELETE_TASK,                 //4
        HANDLE_COMMAND_MOVE_TO_NEXT_TASK,
        HANDLE_COMMAND_DO_NOTHING,

        HANDLE_COMMAND_END
    };

    enum task_flags
    {
        TF_NULL                         =      0,
        TF_IGNORE_ATTACKS               = BIT( 1),  
        TF_IGNORE_SIGHT                 = BIT( 2),  
        TF_IGNORE_SOUND                 = BIT( 3),  
        TF_IGNORE_ALERTS                = BIT( 4),  
        TF_INVINCIBLE                   = BIT( 5)
    };
    

                                    character_task          ( void );
                                    ~character_task         ( void );
                            
	virtual			void	        OnEnumProp		        ( prop_enum& rList );
	virtual			xbool	        OnProperty		        ( prop_query& rPropQuery );  

                    void            AddSubtask              ( actions_ex_base::action_ex_types Type );
                    void            RemoveSubTask           ( action_ai_base* pSubTask );           
                    void            MoveSubTaskUp           ( action_ai_base* pSubTask );                 
                    void            MoveSubTaskDown         ( action_ai_base* pSubTask );  
                    s32             GetSubTaskIndex         ( action_ai_base* pSubTask );
                    void            SetSubTaskIndex         ( action_ai_base* pAction, s32 Index );
                    
                    u32             GetTaskFlags            ( void ) { return m_ResponseList.GetFlags(); }
                    guid            GetSetGuid              ( void ) { return m_SetGuid;   }
    
	virtual		    xbool           ExecuteTask		        ( f32 DeltaTime );
	virtual		    void            OnTaskItemCompleted     ( void );
                    void            OnTaskFailure           ( void );
                    xbool           OnTaskInterrupt         ( void ); //return true if we should continue task after interrupt
                    
    virtual         void            OnRender                ( s32 TaskId );
                    
public:

    static      character_task*     CreateTask              ( guid SetGuid );

    handle_commands                      m_OnInterrupt;
    handle_commands                      m_OnFailure;
    actions_ex_base::action_ex_types     m_SubTasksChooser;

    guid                                 m_ActivateOnSuccess;
    guid                                 m_ActivateOnInterrupt;
    guid                                 m_ActivateOnFailure;

    actions_ex_base*                     m_SubTask[MAX_SUB_TASK_ARRAY_SIZE];
    s32                                  m_SubTaskCount;
    s32                                  m_SubCurrentTaskIndex;

protected:

    guid        m_SetGuid;
//    u32         m_TaskFlags;
    response_list m_ResponseList;
    

    static      enum_table<handle_commands>     m_HandleOnInterrupt; 
    static      enum_table<handle_commands>     m_HandleOnFailure; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////////////////////////////
#endif//character_task_hpp