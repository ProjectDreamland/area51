///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_dead.cpp
//
//      - implements a dead ai
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AI_STATE_DEAD_HPP
#define AI_STATE_DEAD_HPP
#include "ai_state.hpp"

class ai_state_dead : public ai_state
{
public:
                                ai_state_dead(brain* myBrain = NULL);
    virtual                     ~ai_state_dead();

    virtual     void            OnAdvanceLogic( f32 deltaTime );

    virtual     void            OnEnterState( void ); 
    virtual     xbool           OnAttemptExit( void );
    virtual     void            OnExitState( void );
    
    virtual     void            OnInit( void );
    
    virtual     ai_state_type   GetType(void) { return ai_state::AI_STATE_TYPE_DEAD; }
    virtual     const char*     GetTypeName(void) { return ("DEAD"); }


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );


protected:
    


};


#endif//AI_STATE_DEAD_HPP