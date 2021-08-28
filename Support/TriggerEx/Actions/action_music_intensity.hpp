///////////////////////////////////////////////////////////////////////////////
//
//  Action_Music_Intensiy.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _ACTION_MUSIC_INTENSITY
#define _ACTION_MUSIC_INTENSITY

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"


//=========================================================================
// MUSIC_INTENSITY
//=========================================================================

class action_music_intensity : public actions_ex_base
{
public:
    
    enum music_states
    {
        ONESHOT_MUSIC,
        AMBIENT_MUSIC,
        ALERT_MUSIC,
        COMBAT_MUSIC,
        NUM_MUSIC_STATES,
    };

    struct transition
    {
        rhandle<char>   AudioPackage;
        char            TrackName[64];
        s32             TransitionMode;
        f32             FadeInTime;
        f32             FadeOutTime;
        f32             DelayTime;
    };

                    action_music_intensity                  ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_MUSIC_INTENSITY;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Change Music Intensity."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );
   
protected:
    
    transition      m_Transitions[NUM_MUSIC_STATES];
};

#endif
