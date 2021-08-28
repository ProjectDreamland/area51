///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_move_to.cpp
//
//      - implements a simple state that moves the ai to a fixed point
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ai_state_move_to_HPP
#define ai_state_move_to_HPP
#include "ai_state.hpp"

class ai_state_move_to : public ai_state
{
public:
                                ai_state_move_to(brain* myBrain = NULL);
    virtual                     ~ai_state_move_to();

    virtual     void            OnAdvanceLogic( f32 deltaTime );

    virtual     void            OnEnterState( void ); 
   
    virtual     ai_state_type   GetType(void) { return ai_state::AI_STATE_TYPE_MOVE_TO; }
    virtual     const char*     GetTypeName(void) { return ("MOVE_TO"); }


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );


protected:
    vector3     m_TargetPoint;



};


#endif//ai_state_move_to_HPP