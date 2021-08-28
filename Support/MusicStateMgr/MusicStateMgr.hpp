#ifndef MUSIC_STATE_MGR_HPP
#define MUSIC_STATE_MGR_HPP

#include "x_types.hpp"
#include "TriggerEX\Actions\action_music_intensity.hpp"

class music_state_mgr
{
public:

    struct transition
    {
        char            TrackName[64];
        s32             TransitionMode;
        f32             FadeInTime;
        f32             FadeOutTime;
        f32             DelayTime;
    };

                music_state_mgr             ( void );
               ~music_state_mgr             ( void );
    void        Init                        ( void );
    void        Kill                        ( void );
    void        Update                      ( void );
    void        Reset                       ( void );
    void        SetOneShot                  ( void );
    void        ClearOneShot                ( void );
    void        SetAwarenessLevel           ( s32   AwarenessLevel,
                                              f32   DeltaTime );
    void        SetMusicEntry               ( s32   Index,
                                              char* TrackName,
                                              s32   TransitionMode,
                                              f32   FadeOutTime,
                                              f32   DelayTime,
                                              f32   FadeInTime );
    void        SetIncreaseStateDelay       ( f32   Delay );
    void        SetDecreaseStateDelay       ( f32   Delay );


    s32         m_CurrentState;
    s32         m_DesiredState;
    f32         m_IncreaseStateDelay;
    f32         m_DecreaseStateDelay;
    f32         m_DelayTimer;
    xbool       m_bDirty;
    xbool       m_bOneShot;
    transition  m_Transitions[action_music_intensity::NUM_MUSIC_STATES];
    char        m_CurrentTrack[64];
    char        m_DesiredTrack[64];
};

extern music_state_mgr g_MusicStateMgr;

#endif // MUSIC_STATE_MGR_HPP