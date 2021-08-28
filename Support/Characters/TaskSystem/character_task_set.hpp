///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  character_task_set.hpp
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef character_task_set_hpp
#define character_task_set_hpp

///////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Obj_mgr\obj_mgr.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "character_task.hpp"
#include "..\Support\TriggerEx\Affecters\object_affecter.hpp"

#define MAX_TASK_ARRAY_SIZE     16

///////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS
///////////////////////////////////////////////////////////////////////////////////////////////////

class character_task_set : public object
{
public:

    enum char_states_to_transition_to
    {
        INVALID_CHAR_STATE_TRANS = -1,
 
        CHAR_STATE_TRANS_IDLE,                      //0
        CHAR_STATE_TRANS_PATROL,                    //1

        CHAR_STATE_TRANS_END
    };

    CREATE_RTTI( character_task_set, object, object )

                                character_task_set          ( void );
                                ~character_task_set         ( void );
                            
    virtual         bbox        GetLocalBBox            ( void ) const;
    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );
    virtual         s32         GetMaterial             ( void ) const { return MAT_TYPE_NULL; }

	virtual			void	    OnEnumProp		        ( prop_enum& rList );
	virtual			xbool	    OnProperty		        ( prop_query& rPropQuery );

#ifndef X_RETAIL
    virtual         void        OnDebugRender           ( void );
#endif // X_RETAIL
    
                    void        AddTasks                ( s32 nCount = 1 );
                    void        RemoveTask              ( character_task* pTask );           
                    void        MoveTaskUp              ( character_task* pTask );                 
                    void        MoveTaskDown            ( character_task* pTask );     
                    
                    void        ActivateTask            ( void );
    virtual         void        OnActivate              ( xbool Flag );    
                    xbool       AssignNextTask          ( f32 DeltaTime );

                    void        OnTaskItemComplete      ( void ); //called when ai has completed a subtask
                    void        OnTaskFailure           ( void );
                    xbool       OnTaskInterrupt         ( void ); //return true if we should continue task after interrupt

                    void        OnTaskListFinished      ( void );

                    void        ResetTaskListToTop      ( void );
                    void        ResetTaskListToCurrent  ( void );
                    void        ResetTaskListToNext     ( void );
                    void        ResetIndexPtrs          ( xbool bOnlySubTasks );

                    void        MarkForDeletion         ( void );
    virtual         void        OnAdvanceLogic          ( f32 DeltaTime );
                    object_affecter GetObjectAffecter   ( void )                { return m_TaskAffecter; }
    

//    virtual         void        PreloadNextSound        ( void );

public:
    static enum_table<character_task::handle_commands>  m_HandleOnSuccess;
    static enum_table<char_states_to_transition_to>     m_CharStatesToTransitionTo;

    object_affecter                     m_TaskAffecter;
    guid                                m_TaskApplyToGuid;

    character_task*                     m_Task[MAX_TASK_ARRAY_SIZE];
    s32                                 m_TaskCount;

    character_task::handle_commands     m_OnSuccess;
    guid                                m_ActivateOnSuccess;

    s32                                 m_CurrentTaskIndex;     //index into which task we are currently on
    char_states_to_transition_to        m_NextCharState;

    xbool                               m_bEndingTask;
    xbool                               m_bDelete;
    xbool                               m_bStartActive;
    xbool                               m_bActive;
    xbool                               m_bBlink;
    
protected:
                    
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////////////////////////////
#endif//character_task_set_hpp