#include "MusicStateMgr.hpp"
#include "Characters\character.hpp"
#include "Music_Mgr\music_mgr.hpp"

music_state_mgr g_MusicStateMgr;

music_state_mgr::music_state_mgr( void )
{
    m_CurrentState       = -1;
    m_DesiredState       = -2;
    m_IncreaseStateDelay = 1.0f;
    m_DecreaseStateDelay = 5.0f;
    m_DelayTimer         = 0.0f;
    m_bDirty             = FALSE;
    m_bOneShot           = FALSE;

    for( s32 i=0 ; i<action_music_intensity::NUM_MUSIC_STATES ; i++ )
    {
        m_Transitions[i].TrackName[0] = 0;
    }
}

music_state_mgr::~music_state_mgr( void )
{
}

void music_state_mgr::Init( void )
{
    g_MusicMgr.Kill();
    g_MusicMgr.Init();
    m_CurrentState       = -1;
    m_DesiredState       = -2;
    m_IncreaseStateDelay = 1.0f;
    m_DecreaseStateDelay = 5.0f;
    m_DelayTimer         = 0.0f;
    m_bDirty             = FALSE;
    m_bOneShot           = FALSE;

    for( s32 i=0 ; i<action_music_intensity::NUM_MUSIC_STATES ; i++ )
    {
        m_Transitions[i].TrackName[0] = 0;
    }
}

void music_state_mgr::Kill( void )
{
}

void music_state_mgr::SetAwarenessLevel( s32   AwarenessLevel,
                                         f32   DeltaTime )
{
    s32 DesiredState;

    switch( AwarenessLevel )
    {
        default:
        case character::AWARENESS_NONE:
        case character::AWARENESS_COMBAT_READY:
            DesiredState = action_music_intensity::AMBIENT_MUSIC;
            break;

        case character::AWARENESS_ALERT:
        case character::AWARENESS_SEARCHING:
            DesiredState = action_music_intensity::ALERT_MUSIC;
            break;

        case character::AWARENESS_ACQUIRING_TARGET:
        case character::AWARENESS_TARGET_LOST:
        case character::AWARENESS_TARGET_SPOTTED:
            DesiredState = action_music_intensity::COMBAT_MUSIC;
            break;
    }

    if( DesiredState != m_CurrentState )
    {
        if( DesiredState != m_DesiredState )
        {
            m_DesiredState = DesiredState;
            m_DelayTimer   = 0.0f;
        }

        if( ((m_DesiredState > m_CurrentState) && (m_DelayTimer >= m_IncreaseStateDelay)) ||
            ((m_DesiredState < m_CurrentState) && (m_DelayTimer >= m_DecreaseStateDelay)) )
        {
            LOG_MESSAGE( "ChangeMusicState", "OldState: %d, NewState: %d", m_CurrentState, m_DesiredState );
            m_bDirty       = TRUE;
            m_CurrentState = m_DesiredState;
            m_DesiredState = -1;
            m_DelayTimer   = 0.0f;
        }
        else
        {
            // Bump the delay timer.
            m_DelayTimer += DeltaTime;
        }
    }
}

void music_state_mgr::SetMusicEntry( s32   i,
                                     char* TrackName,
                                     s32   TransitionMode,
                                     f32   FadeOutTime,
                                     f32   DelayTime,
                                     f32   FadeInTime )
{
    ASSERT( (i >= 0) && (i<action_music_intensity::NUM_MUSIC_STATES) );

    if( TrackName && *TrackName )
    {
        x_strncpy( m_Transitions[i].TrackName, TrackName, 64 );
        m_Transitions[i].TrackName[63]  = 0;
        m_Transitions[i].TransitionMode = TransitionMode;
        m_Transitions[i].FadeOutTime    = FadeOutTime;
        m_Transitions[i].DelayTime      = DelayTime;
        m_Transitions[i].FadeInTime     = FadeInTime;
        
        // Mark it dirty
        m_bDirty = TRUE;
    }
}

void music_state_mgr::SetOneShot( void )
{
    m_bOneShot = TRUE;
}

void music_state_mgr::ClearOneShot( void )
{
    m_bOneShot = FALSE;
}

void music_state_mgr::Update( void )
{
    if( m_bDirty )
    {
        s32 i;

        // One shot specified?
        if( m_bOneShot )
            i = action_music_intensity::ONESHOT_MUSIC;
        else
            i = m_CurrentState;

        // Check for whacked out state.
        if( (i < 0) || (i >= action_music_intensity::NUM_MUSIC_STATES) )
        {
            LOG_WARNING( "music_state_mgr::Update", "Undefined MusicState!!! Using AMBIENT!!!" );
            i = action_music_intensity::AMBIENT_MUSIC;
        }

        const char* pText = "???";
        switch( m_Transitions[i].TransitionMode )
        {
            case music_mgr::SWITCH_END_OF_TRACK: pText = "EndOfTrack"; break;
            case music_mgr::SWITCH_BREAKPOINT:   pText = "Breakpoint"; break;
            case music_mgr::SWITCH_IMMEDIATE:    pText = "Immediate";  break;
            case music_mgr::SWITCH_VOLUME_FADE:  pText = "VolumeFade"; break;                                          
        }
/*
        LOG_MESSAGE( "music_state_mgr::Update", 
                     "Request transition! State: %d, Track: %s, Mode: %s(%d), FadeOut: %6.3f, Silence: %6.3f, FadeIn: %06.3f",
                     i,
                     m_Transitions[i].TrackName, 
                     pText, 
                     m_Transitions[i].TransitionMode, 
                     m_Transitions[i].FadeOutTime, 
                     m_Transitions[i].DelayTime, 
                     m_Transitions[i].FadeInTime );
        LOG_FLUSH();
*/
        // Make the transition request.
        xbool bOk = g_MusicMgr.Request( m_Transitions[i].TrackName,
                                        m_Transitions[i].TransitionMode,
                                        m_Transitions[i].FadeOutTime,
                                        m_Transitions[i].DelayTime,
                                        m_Transitions[i].FadeInTime,
                                        !m_bOneShot );

        // Request accepted?
        if( bOk )
        {
            // Clear the flags.
            m_bDirty = m_bOneShot = FALSE;
        }
    }
}
