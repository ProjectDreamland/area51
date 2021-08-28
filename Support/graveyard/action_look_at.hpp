///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  action_look_at.hpp
//
//      - represents an animation action.  Just runs a animation
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ACTION_LOOK_AT_HPP
#define ACTION_LOOK_AT_HPP
#include "action.hpp"

class action_look_at : public action
{
public:
    

                            action_look_at(void);
    virtual                 ~action_look_at();

    

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

    guid        m_LookAtObject;


};


#endif//ACTION_LOOK_AT_HPP