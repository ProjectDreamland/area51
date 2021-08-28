///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_follow.cpp
//
//      - implements a simple state that moves the ai to a fixed point
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ai_state_follow_HPP
#define ai_state_follow_HPP
#include "ai_state.hpp"

class ai_state_follow : public ai_state
{
public:
                                ai_state_follow(brain* myBrain = NULL);
    virtual                     ~ai_state_follow();

    virtual     void            OnAdvanceLogic( f32 deltaTime );

    virtual     void            OnEnterState( void ); 
   
    virtual     ai_state_type   GetType(void) { return ai_state::AI_STATE_TYPE_FOLLOW; }
    virtual     const char*     GetTypeName(void) { return ("FOLLOW"); }


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );


protected:

    guid        m_FollowTarget;
    f32         m_FollowDistance;
    xbool       m_FollowFriend;


};


#endif//ai_state_follow_HPP