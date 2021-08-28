///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  action_script.hpp
//
//      - represents a script actions.  Just runs a script
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ACTION_SCRIPT_HPP
#define ACTION_SCRIPT_HPP
#include "action.hpp"

class action_script : public action
{
public: 
    

                            action_script(void);
    virtual                 ~action_script();

    

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
    
    char*       m_ScriptName;


};


#endif//ACTION_SCRIPT_HPP