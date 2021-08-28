#ifndef MUSIC_MGR_HPP
#define MUSIC_MGR_HPP

#include "e_audio.hpp"
#include "Parsing/textin.hpp"

class music_mgr 
{
public:

    #define MINIMUM_TRANSITION_TIME  (1.5f)
    #define MINIMUM_TRANSITION_DELTA (0.3f)

    enum VOLUMEDIRECTION
    {
        VOLUME_FADEOUT = 0,
        VOLUME_SILENT,
        VOLUME_FADEIN,
    };

    enum SWITCHTYPE             // Method Used to Switch to the Next Requested Track
    { 
        SWITCH_NONE = -1,
        SWITCH_END_OF_TRACK,    // Is used when there are no special conditions, and is the default Switch type Switch occurs when end of track is reached
        SWITCH_BREAKPOINT,      // Switch occurs at the next breakpoint. 
        SWITCH_VOLUME_FADE,     // Switches when Volume fades to zero.
        SWITCH_IMMEDIATE        // Switch happens next tick.  No transitions. It is abrupt and immediate.
    };


public:
    
                music_mgr       ( void );
               ~music_mgr       ( void );
    void        Init            ( void );
    void        Kill            ( void );
    void        Update          ( f32   DeltaTime );
    void        StartTransition ( void );
    void        UpdateTransition( f32   DeltaTime );
    void        UpdateNormal    ( void );
    void        UpdateVolume    ( f32   DeltaTime );
    xbool       CanTransition   ( s32   TransitionMode,
                                  f32&  FadeOutTime );
    xbool       Request         ( char* pTrackName,
                                  s32   TransitionMode,
                                  f32   FadeOutTime,
                                  f32   DelayTime,
                                  f32   FadeInTime,
                                  xbool bLoop );

    xbool       m_bIsWarming;
    xbool       m_bIsSilent;
    xbool       m_bInTransition;
    xbool       m_bIsLooping;
    xbool       m_bWarmFadeIn;
    xbool       m_bStartFadeIn;
    xbool       m_bImmediateSwitch;
    char        m_CurrentTrack[64];
    s32         m_CurrentTransition;
    voice_id    m_CurrentVoiceID;
    char        m_TransTrack[64];
    xbool       m_bStartTransition;
    s32         m_TransMode;
    f32         m_TransFadeOutTime;
    f32         m_TransDelayTime;
    f32         m_TransFadeInTime;
    f32         m_TransBreakPoint;
    xbool       m_TransIsLooped;
    voice_id    m_TransVoiceID;
    s32         m_EnvelopeState;
    f32         m_Volume;
    f32         m_EnvelopeTime;
};

extern music_mgr g_MusicMgr;

#endif // MUSIC_MGR_HPP
