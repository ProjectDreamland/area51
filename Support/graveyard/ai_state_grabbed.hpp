///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_grabbed.cpp
//
//      - implements a dead ai
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AI_STATE_GRABBED_HPP
#define AI_STATE_GRABBED_HPP
#include "ai_state.hpp"

class ai_state_grabbed : public ai_state
{
public:
                                ai_state_grabbed(brain* myBrain = NULL);
    virtual                     ~ai_state_grabbed();

    virtual     void            OnAdvanceLogic( f32 deltaTime );
 
    virtual     void            OnEnterState( void ); 
    virtual     xbool           OnAttemptExit( void );
    virtual     void            OnExitState( void );
    
    virtual     void            OnInit( void );
    
    virtual     ai_state_type   GetType(void) { return ai_state::AI_STATE_TYPE_GRABBED; }
    virtual     const char*     GetTypeName(void) { return ("GRABBED"); }


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////
 
    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );


protected:
    


};


#endif//AI_STATE_GRABBED_HPP