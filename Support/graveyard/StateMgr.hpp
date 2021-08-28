///////////////////////////////////////////////////////////////////////////////
//
//  StateMgr.hpp
//
//      -Despite Name
//
///////////////////////////////////////////////////////////////////////////////
#ifndef STATE_MGR_HPP
#define STATE_MGR_HPP

#include "Obj_mgr\obj_mgr.hpp"

class object_state;

//
//  object_state_container
//
//      -used to build a list of states for an object
//
struct object_state_container
{
    object_state*               m_ObjectState;
    object_state_container*     m_NextStateContainer;


};




class state_mgr
{
public:
    
    enum object_state_types
    {
        OBJECT_STATE_FIRST = 0,
        OBJECT_STATE_NONE, 
        OBJECT_STATE_MOVEMENT,
        OBJECT_STATE_CONVERSATION,
        OBJECT_STATE_COMBAT,
        OBJECT_STATE_HIT_RESPONSE,
        OBJECT_STATE_COLLISION_RESPONSE,
        OBJECT_STATE_LAST,
        FORCE_32BIT = 0xFFFFFFFF    
    };

public:
                                state_mgr   ( guid  Owner );
    virtual                    ~state_mgr   ( void );

    virtual void                AddState    ( object_state_types State, const char* name = NULL );
    virtual void                AddState    ( object_state*      State );
    virtual object_state*       FindState   ( object_state_types State, const char* name = NULL );
    virtual object_state*       FindState   ( object_state*      State );
    virtual void                RemoveState ( object_state*      State );

    virtual void                LoadTweakables( X_FILE* thisFile );
    virtual void                LoadTweakables( const char* customTweaks = NULL );  // If called with Null it loads tweaks based on name of object

    virtual void                OnInit          ( void );
    virtual void                OnKill          ( void );               

    virtual void                OnRender        ( void );
    virtual void                OnAdvanceLogic  ( f32 DeltaTime );      
    virtual void                OnColCheck      ( void );

    virtual xbool               RequestStateChange( object_state& NewState  );
    virtual void                SetState(           object_state& NewState  );
    virtual object_state*        GetState(           void                    );

    virtual void                SetGoalCompleted( void );
    virtual void                GetGoalCompleted( void );
    virtual const object_state* GetCurrentState ( void );

    virtual xbool               RequestStateChange( object_state*   newState );
    virtual xbool               ChangeState       ( object_state*   newState );
    virtual xbool               OnStateChange     ( object_state*   newState, object_state* oldState );



protected:
    

    
    object_state*       m_pFirstState;      //  pointer to the first member of the state list
    guid                m_ObjectGuid;       //  guid of associated object


};


#endif//STATE_MGR_HPP