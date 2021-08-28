///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_curious.cpp
//
//      - implements a curious state where the AI will search for something
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AI_STATE_CURIOUS_HPP
#define AI_STATE_CURIOUS_HPP
#include "ai_state.hpp"

class ai_state_curious : public ai_state
{
public:
                                ai_state_curious(brain* myBrain = NULL);
    virtual                     ~ai_state_curious();

    virtual     void            OnAdvanceLogic( f32 deltaTime );

    virtual     void            OnEnterState( void ); 
    virtual     xbool           OnAttemptExit( void );
    virtual     void            OnExitState( void );
    
    virtual     void            OnInit( void );
    
    virtual     ai_state_type   GetType(void) { return ai_state::AI_STATE_TYPE_CURIOUS; }
    virtual     const char*     GetTypeName(void) { return ("CURIOUS"); }


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );


protected:
    
    vector3     m_PointOfInterest;
    char        m_ExitStateTargetFound[32];


};


#endif//AI_STATE_CURIOUS_HPP