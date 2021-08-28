///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_GAME_MUSIC_INTENSITY
#define _TRIGGER_ACTIONS_GAME_MUSIC_INTENSITY

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// MUSIC_INTENSITY
//=========================================================================

class  game_music_intensity : public actions_base
{
public:
    
    enum music_actions
    {
        SWITCH_TO_TRACK,
        INCREASE_INTENSITY,
        DECREASE_INTENSITY,
        SET_INTENSITY,
        SET_DEFAULT_STATE,
    };

                    game_music_intensity            ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Music Intensity"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Change the music's intensity."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_MUSIC_INTENSITY;}

protected:
    
    char    m_SuiteName[64];
    char    m_TrackName[64];
    s32     MusicTriggerAction;
    s32     m_IntensityLevel;
    s32     m_SwitchMode;
    f32     m_VolumeFade;
    f32     m_FadeInTime;
    xbool   m_RunLogic;
};

#endif
