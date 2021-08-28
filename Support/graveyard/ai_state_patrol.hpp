///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_patrol.cpp
//
//      - implements a simple state that makes the unit patrol
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ai_state_patrol_HPP
#define ai_state_patrol_HPP
#include "ai_state.hpp"
#include "navigation\base_node.hpp"

class ai_state_patrol : public ai_state
{
public:
                                ai_state_patrol(brain* myBrain = NULL);
    virtual                     ~ai_state_patrol();

    virtual     void            OnAdvanceLogic( f32 deltaTime );

    virtual     void            OnEnterState( void ); 
   
    virtual     ai_state_type   GetType(void)       { return ai_state::AI_STATE_TYPE_PATROL; }
    virtual     const char*     GetTypeName(void)   { return ("PATROL"); }


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

//    virtual     void            OnEnumProp( prop_enum&  List );
//    virtual     xbool           OnProperty( prop_query& I    );


protected:
    xbool               m_bForward;
    node_slot_id        m_CurrentPatrolNode;

};


#endif//ai_state_patrol_HPP