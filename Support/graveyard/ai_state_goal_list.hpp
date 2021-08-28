///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_goal_list.hpp
//
//      - goal list implements a sequence of actions in an ai state.
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AI_STATE_GOAL_LIST_HPP
#define AI_STATE_GOAL_LIST_HPP
#include "ai_state.hpp"

class ai_state_goal_list : public ai_state
{
public:
                                ai_state_goal_list(brain* myBrain = NULL);
    virtual                     ~ai_state_goal_list();

    virtual     void            OnAdvanceLogic( f32 deltaTime );

    virtual     void            OnEnterState( void ); 
    virtual     xbool           OnAttemptExit( void );
    virtual     void            OnExitState( void );
    
    virtual     void            OnInit( void );
    
    virtual     ai_state_type   GetType(void) { return ai_state::AI_STATE_TYPE_GOAL_LIST; }
    virtual     const char*     GetTypeName(void) { return ("GOAL_LIST"); }


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );

 

protected:
    
    xarray<action>          m_ActionList;


};


#endif//AI_STATE_GOAL_LIST_HPP