///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  action.hpp
//
//      - represents a single action in the game.  This is just the base class that must be
//        implemented for any action
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ACTION_HPP
#define ACTION_HPP

class action
{
public:
    
    enum action_type
    {
        ACTION_SCRIPT =0,
        ACTION_SOUND,
        ACTION_ANIM,
        ACTION_DESTINATION,
        ACTION_AI_STATE,
        ACTION_LOOK_AT,

        ACTION_LAST,
        ACTION_FORCE32BIT = 0xFFFFFFFF
        
    
    };


                            action(void);
    virtual                 ~action();

    

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );

    virtual     action_type     GetType ( void ) = 0;

    virtual     void            Play ( void );
    virtual     void            OnAdvanceLogic( f32 deltaTime );

    
    virtual     void            Pause ( xbool pause = true );
    virtual     void            Unpause ( void );
    virtual     xbool           IsPaused ( void );

protected:
    
    xbool   m_Paused;
    xbool   m_WaitForCompletion;



};


#endif//ACTION_HPP