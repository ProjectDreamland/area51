///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  action_ai_state.hpp
//
//      - represents a ai_state action.  Just runs does a movement action
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ACTION_AI_STATE_HPP
#define ACTION_AI_STATE_HPP
#include "action.hpp"

class action_ai_state : public action
{
public:
    

                            action_ai_state(void);
    virtual                 ~action_ai_state();

    

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );

    virtual     action_type     GetType ( void );

    virtual     void            Play ( void );
    virtual     void            OnAdvanceLogic( f32 deltaTime );

    
    virtual     void            Pause ( xbool pause = true );
    virtual     void            Unpause ( void );
    virtual     xbool           IsPaused ( void );

protected:

    vector3     m_Destination;


};


#endif//ACTION_AI_STATE_HPP