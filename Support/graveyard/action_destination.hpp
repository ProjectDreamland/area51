///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  action_destination.hpp
//
//      - represents a destination action.  Just runs does a movement action
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ACTION_DESTINATION_HPP
#define ACTION_DESTINATION_HPP
#include "action.hpp"

class action_destination : public action
{
public:
    

                            action_destination(void);
    virtual                 ~action_destination();

    

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


#endif//ACTION_DESTINATION_HPP