///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state.hpp
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AI_STATE_HPP
#define AI_STATE_HPP

#include "x_math.hpp"
#include "x_debug.hpp"
#include "MiscUtils\RTTI.hpp"
#include "obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "ai\emotion_controller.hpp"

class brain;


class ai_state
{
public:

    enum ai_state_type
    {
        AI_STATE_TYPE_IDLE = 0,
        AI_STATE_TYPE_GOAL_LIST, 
        AI_STATE_TYPE_CURIOUS,
        AI_STATE_TYPE_FLEE, 
        AI_STATE_TYPE_ATTACK_NORMAL,
        AI_STATE_TYPE_ATTACK_PASSIVE,
        AI_STATE_TYPE_ATTACK_AGGRESSIVE,
        AI_STATE_TYPE_HIDING,
        AI_STATE_TYPE_PATROL,
        AI_STATE_TYPE_DYING,
        AI_STATE_TYPE_DEAD,
        AI_STATE_TYPE_MOVE_TO,
        AI_STATE_TYPE_FOLLOW,
        AI_STATE_TYPE_GRABBED,
        AI_STATE_TYPE_LAST,
        
        
        
        AI_STATE_TYPE_FORCE32BIT = 0xFFFFFFFF
    
    };

    enum ai_exit_state
    {
        AI_EXIT_STATE_NORMAL    =0 ,
        AI_EXIT_STATE_ENRAGED   ,
        AI_EXIT_STATE_CURIOUS   ,
        AI_EXIT_STATE_BORED     ,
        AI_EXIT_STATE_SCARED    ,

        AI_EXIT_STATE_LAST      ,
        AI_EXIT_STATE_FORCE32BIT = 0xFFFFFFFF
        
    };

    enum eye_wander
    {
        AI_EYE_WANDER_NONE = 0,
        AI_EYE_WANDER_WANDER,
        AI_EYE_WANDER_RANDOM,

        AI_EYE_WANDER_FORCE32BIT = 0xFFFFFFFF

    };


                                ai_state(brain* myBrain = NULL);
    virtual                     ~ai_state();


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  OnAdvanceLogic - Normal advance logic call but only gets a call when this is the active state
//              in a brain.
//
//  OnEnterState - Called by brain when the state is entered.
//
//  OnAttemptExit - Called by brain to start a state exit.  Returns true if it is ready to exit, 
//              false if it needs more time.
//
//  OnExitState - Called by brain after OnAttemptExit returns true
//
//  OnInit - Called to reset the AI state to a base state.  Called by the constructor or to reset
//
//  GetType - returns the specific AI type  
//
//  GetName - returns the custom name for this AI
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnAdvanceLogic( f32 deltaTime );
    virtual     void            OnDamaged( s32 Damage, vector3* thisDirection );

    virtual     void            OnEnterState( void ); 

    virtual     xbool           OnAttemptExit( void );

    virtual     void            OnExitState( void );
    
    virtual     void            OnInit( void );
    
    virtual     ai_state_type   GetType(void) = 0;
    virtual     const char*     GetTypeName(void) = 0;

    virtual     const char*     GetName( void );
    virtual     void            SetName( const char* newName );

    virtual     f32             GetTimeInState( void );

    virtual     void            UpdateLookAtObject( f32 deltaTime );

    virtual     f32             GetWanderFrequency(void )  { return m_WanderFrequency; }

    virtual     guid            GetCurrentTarget( void );
    
    virtual     void            OnPain( const pain& Pain ) { (void)Pain; }
    
///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );

    virtual     s32             GetCyclesInState( void );

    virtual     void            SetMinTimeInState( f32 newMinTime );

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Performance
///////////////////////////////////////////////////////////////////////////////////////////////////

                void            AddToTicksUsed(xtick timeUsed );
                xtick           GetTicksUsed( void );

protected:
    
    f32         m_TimeInState;
    f32         m_MinTimeInState;
    s32         m_TotalNumberOfCyclesInState;
    f32         m_MaxTimeInState;


    xtick       m_TicksUsed;

    char        m_CustomName[32];     

    char        m_ExitStateNormal[32],
                m_ExitStateAgro[32],
                m_ExitStateBored[32];

    brain*      m_Brain;

    static s32  m_DummyNumber;      //  used to generate a unique dummy name for newly created states

    emotion_controller::emotion_level m_EmotionLevelToSwitchToBored;

    vector3     m_LookAtPoint;
    f32         m_LookAtPointWanderSpeed;
    eye_wander  m_WanderingEyes;    
    
    f32         m_WanderFrequency;
};



inline
void  ai_state::SetMinTimeInState( f32 newMinTime ) 
{ 
    m_MinTimeInState = newMinTime; 
}




#endif//AI_STATE_HPP