///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_flee.cpp
//
//      - implements a state that represent the AI running away
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AI_STATE_FLEE_HPP
#define AI_STATE_FLEE_HPP
#include "ai_state.hpp"

class ai_state_flee : public ai_state
{
public:
                                ai_state_flee(brain* myBrain = NULL);
    virtual                     ~ai_state_flee();

    virtual     void            OnAdvanceLogic( f32 deltaTime );

    virtual     void            OnEnterState( void ); 
    virtual     xbool           OnAttemptExit( void );
    virtual     void            OnExitState( void );
    
    virtual     void            OnInit( void );
    
    virtual     ai_state_type   GetType(void) { return ai_state::AI_STATE_TYPE_FLEE; }
    virtual     const char*     GetTypeName(void) { return ("FLEE"); }


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );


protected:
    
    char        m_ExitStateTargetLost[32];


};


#endif//AI_STATE_FLEE_HPP