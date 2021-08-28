///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  action_anim.hpp
//
//      - represents an animation action.  Just runs a animation
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ACTION_ANIM_HPP
#define ACTION_ANIM_HPP
#include "action.hpp"

class action_anim : public action
{
public:
    

                            action_anim(void);
    virtual                 ~action_anim();

    

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

    


};


#endif//ACTION_ANIM_HPP