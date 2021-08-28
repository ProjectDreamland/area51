
//==============================================================================
//  INCLUDES
//==============================================================================

#include "StateMgr\StateMgr.hpp"

#include "e_audio.hpp"
#include "music_mgr.hpp"
#include "audio\audio_stream_mgr.hpp"
#include "x_types.hpp"
#include "ResourceMgr/ResourceMgr.hpp"
#include <stdio.h>
#include "x_string.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"

music_mgr g_MusicMgr;

music_mgr::music_mgr( void )
{
}

music_mgr::~music_mgr( void )
{
}

void music_mgr::Init( void )
{
    m_bIsLooping        = FALSE;
    m_bIsWarming        = FALSE;
    m_bIsSilent         = TRUE;
    m_bInTransition     = FALSE;
    m_bStartTransition  = FALSE;
    m_CurrentTrack[0]   = 0;
    m_TransTrack[0]     = 0;
    m_CurrentVoiceID    = 0;
    m_TransVoiceID      = 0;
    m_Volume            = 1.0f;
    m_TransMode         = SWITCH_NONE;
}

void music_mgr::Kill( void )
{
    g_AudioMgr.Release( m_CurrentVoiceID, 0.0f );
    g_AudioMgr.Release( m_TransVoiceID,   0.0f );
    Init();
}

xbool music_mgr::Request( char* pTrackName,
                          s32   TransitionMode,
                          f32   FadeOutTime,
                          f32   DelayTime,
                          f32   FadeInTime,
                          xbool bLoop )
{
    // Playing the same track? If so, dont do a damn thing (but act like you did!)
    if( (x_stricmp( m_CurrentTrack, pTrackName ) == 0) && !m_bIsSilent )
        return TRUE;

    // Ignore null or empty tracknames.
    if( (pTrackName == NULL) || (*pTrackName == 0) )
        return TRUE;

    // Is it an invalid descriptor? Pretend everything is great!
    if( !g_AudioMgr.IsValidDescriptor( pTrackName ) )
    {
        //ASSERTS( 0, "Invalid Music Descriptor" );
        return TRUE;
    }

    // Determine if we transition.
    xbool bTransition = CanTransition( TransitionMode, FadeOutTime );

    // Can we transition?
    if( bTransition )
    {
        const char* pText = "???";
        switch( TransitionMode )
        {
            case SWITCH_END_OF_TRACK: pText = "EndOfTrack"; break;
            case SWITCH_BREAKPOINT:   pText = "Breakpoint"; break;
            case SWITCH_IMMEDIATE:    pText = "Immediate";  break;
            case SWITCH_VOLUME_FADE:  pText = "VolumeFade"; break;                                          
        }
        LOG_MESSAGE( "music_mgr::Request", 
                     "TRANSITION! Mode: %s(%d), Track: %s, FadeOut: %6.3f, Silence: %6.3f, FadeIn: %6.3f",
                     pText, TransitionMode, pTrackName, FadeOutTime, DelayTime, FadeInTime );
        LOG_FLUSH();

        // Mark it dirty.
        m_bStartTransition = TRUE;

        // Set up the transition.
        x_strcpy( m_TransTrack, pTrackName );
        m_TransMode         = TransitionMode;
        m_TransFadeOutTime  = FadeOutTime;
        m_TransDelayTime    = DelayTime;
        m_TransFadeInTime   = FadeInTime;
        m_TransIsLooped     = bLoop;
    }

    // Tell the world.
    return bTransition;
}

xbool music_mgr::CanTransition( s32 TransitionMode, f32& FadeOutTime )
{
    // Can't transition if we are warming up a transition.
    if( m_bIsWarming )
        return FALSE;

    // Always transition if we are silent.
    if( m_bIsSilent )
        return TRUE;

    // Transition in progress?
    if( m_bInTransition )
    {
        // What happens now depends on what kind of transition we are currently in!
        switch( m_TransMode )
        {
            // Gotta wait for these to complete, no exceptions!
            case SWITCH_END_OF_TRACK:
            case SWITCH_BREAKPOINT: 
            case SWITCH_IMMEDIATE:
                return FALSE;

            // Can only break out of SWITCH_VOLUME_FADE if the new transition is SWITCH_IMMEDIATE.
            case SWITCH_VOLUME_FADE:
                return (TransitionMode == SWITCH_IMMEDIATE);

            // First time?
            case SWITCH_NONE:
                return TRUE;

            // Should *never* get here.
            default:
                ASSERT( 0 );
                return TRUE;
        }
    }

    // Initialize the breakpoints.
    f32* pBreakPoints = NULL;
    s32  nBreakPoints = 0;

    // Calculate length, playtime and remaining time.
    f32 Length        = g_AudioMgr.GetLengthSeconds( m_CurrentVoiceID );
    f32 PlayTime      = g_AudioMgr.GetCurrentPlayTime( m_CurrentVoiceID );
    f32 RemainingTime = Length - PlayTime;

    switch( TransitionMode )
    {
        case SWITCH_BREAKPOINT:
            // Get the number and address for the breakpoints.
            nBreakPoints = g_AudioMgr.GetBreakPoints( m_CurrentVoiceID, pBreakPoints );

            // Lets find the breakpoint.
            for( s32 i=0 ; i<nBreakPoints ; i++ )
            {
                // Calculate time until this break point.
                f32 Time = *(pBreakPoints+i) - PlayTime;

                // Inside the window?
                if( (Time > MINIMUM_TRANSITION_TIME) && (Time < MINIMUM_TRANSITION_TIME+MINIMUM_TRANSITION_DELTA) )
                {
                    // Note the breakpoint.
                    m_TransBreakPoint = *(pBreakPoints+i);

                    // Transition baby!
                    return TRUE;
                }
            }
            
            //=======================================================
            // No good breakpoint found? *FALL THRU* to end of track!
            //=======================================================

        case SWITCH_END_OF_TRACK:
            // Time to start transition? 
            if( RemainingTime < (MINIMUM_TRANSITION_TIME+MINIMUM_TRANSITION_DELTA) )
            {
                // If we *can* loop and we are cutting it close, then chill out.
                if( m_bIsLooping && (RemainingTime < MINIMUM_TRANSITION_TIME) )
                {
                    // Wait for the loop to start again...
                    return FALSE;
                }
                else
                {
                    // Transition at end of track!
                    m_TransBreakPoint = Length;

                    // Transition baby!
                    return TRUE;
                }
            }

            // Sorry Charlie only the best tasting tuna...
            return FALSE;

        // Can always SWITCH_IMMEDIATE
        case SWITCH_IMMEDIATE:
            // Woot! All good!
            return TRUE;

        // Can always SWITCH_VOLUME_FADE
        case SWITCH_VOLUME_FADE:
            // Is the fade out too long for a non-looped track? Clamp it!
            if( (FadeOutTime > RemainingTime) && !m_bIsLooping )
                FadeOutTime = RemainingTime;

            // Woot! All good!
            return TRUE;

        // Should *never* get here.
        default:
            ASSERT( 0 );
            return TRUE;
    }

    // Oops - default is cant transition.
    return FALSE;
}

void music_mgr::StartTransition( void )
{
    // Do different stuff based on transition mode.
    switch( m_TransMode )
    {
        case SWITCH_END_OF_TRACK:
        case SWITCH_BREAKPOINT:
        case SWITCH_IMMEDIATE:
            // Special case for UpdateTransition.
            m_bImmediateSwitch = (m_TransMode == SWITCH_IMMEDIATE);

            // Better not be in transition!
            ASSERT( !m_bIsWarming );
            if( g_AudioMgr.IsValidVoiceId( m_TransVoiceID ) )
            {
                // Should never get here!
                ASSERT( 0 );

                // Safety (retail builds) - No losing any voices!
                g_AudioMgr.Release( m_TransVoiceID, 0.0f );
            }

            // Full volume!
            m_Volume = 1.0f;

            // Play it! AutoStart depends on m_bIsSilent
            m_TransVoiceID = g_AudioMgr.Play( m_TransTrack, m_bIsSilent );
            ASSERT( m_TransVoiceID );

            LOG_MESSAGE( "music_mgr::StartTransition",
                         "Starting: %s, AUTOSTART: %d",
                         m_TransTrack, m_bIsSilent );

            // Success?
            if( m_TransVoiceID )
            {
                // Woot! We are warming up!
                m_bIsWarming = TRUE;

                // Need segue?
                if( !m_bIsSilent )
                {
                    LOG_MESSAGE( "music_mgr::StartTransition",
                                  "Segue: %08x -> %08x",
                                  m_CurrentVoiceID, m_TransVoiceID );

                    // Set up the segue.
                    g_AudioMgr.Segue( m_CurrentVoiceID, m_TransVoiceID );

                    // Breakpoint?
                    if( m_TransMode == SWITCH_BREAKPOINT )
                    {
                        // Set the release time for the segue at the breakpoint.
                        g_AudioMgr.SetReleaseTime( m_CurrentVoiceID, m_TransBreakPoint );
                    }
                }
            }
            LOG_FLUSH();
            break;

        case SWITCH_VOLUME_FADE:
            // Reset the envelope time.
            m_EnvelopeTime = 0.0f;

            // Set flags.
            m_bWarmFadeIn = m_bStartFadeIn = TRUE;

            // Currently silent?
            if( m_bIsSilent )
            {
                // Shhh...quiet!
                m_Volume = 0.0f;

                // Transition silence now!
                m_EnvelopeState = VOLUME_SILENT;
            }
            else
            {
                // Full volume.
                m_Volume = 1.0f;

                // Start the fade out.
                m_EnvelopeState = VOLUME_FADEOUT;
            }
            break;

        default:
            // Should never get here. Code will survive this in retail build.
            ASSERT( 0 );
            break;
    }

    // Done starting the transition!
    m_bStartTransition = FALSE;

    // Transition is *on*!
    m_bInTransition = TRUE;
}

void music_mgr::UpdateTransition( f32 DeltaTime )
{
    switch( m_TransMode )
    {
        case SWITCH_IMMEDIATE:
            // Transition voice valid? 
            if( g_AudioMgr.IsValidVoiceId( m_TransVoiceID ) )
            {
                // All warmed up?
                if( g_AudioMgr.GetIsReady( m_TransVoiceID ) )
                {
                    // Only do this once...
                    if( m_bImmediateSwitch )
                    {
                        // Clear the flag.
                        m_bImmediateSwitch = FALSE;

                        // Stop the current track (transition should automatically segue).
                        g_AudioMgr.Release( m_CurrentVoiceID, 0.0f );
                    }
                }
            }
            // Transition voice is invalid! How?
            else
            {
                // Should never get here.
                ASSERT( 0 );

                // Safety (retail build) - Force transition done!
                m_bInTransition = FALSE;
            }
            break;

        case SWITCH_VOLUME_FADE:
            // What is the volume direction?
            switch( m_EnvelopeState )
            {
                case VOLUME_FADEOUT:
                    // Only warm up track once.
                    if( m_bWarmFadeIn )
                    {
                        // Calculate time until fade in is supposed to start.
                        f32 TimeToFadeIn = m_TransFadeOutTime + m_TransDelayTime - m_EnvelopeTime;

                        // Time to start warming up?
                        if( TimeToFadeIn < (MINIMUM_TRANSITION_TIME+MINIMUM_TRANSITION_DELTA) )
                        {
                            // Dont warm it up again!
                            m_bWarmFadeIn = FALSE;

                            // Warm it up!
                            m_TransVoiceID = g_AudioMgr.Play( m_TransTrack, FALSE );
                            ASSERT( m_TransVoiceID );

                            // Success?
                            if( m_TransVoiceID )
                            {
                                // Woot, we are warming it up!
                                m_bIsWarming = TRUE;
                            }
                            // Safety (retail build) - Don't get stuck in a transition.
                            else
                            {
                                // Transition is over!
                                m_bInTransition = FALSE;

                                // Done.
                                return;
                            }
                        }
                    }

                    // Bump the time in this state.
                    m_EnvelopeTime += DeltaTime;

                    // Volume fade out complete?
                    if( (m_EnvelopeTime >= m_TransFadeOutTime) || (m_TransFadeOutTime <= 0.0f) )
                    {
                        // Silent!
                        m_Volume = 0.0f;

                        // Reset time for next state.
                        m_EnvelopeTime = 0.0f;

                        // Change states.
                        m_EnvelopeState = VOLUME_SILENT;

                        // Stop the current track.
                        g_AudioMgr.Release( m_CurrentVoiceID, 0.0f );
                    }
                    else
                    {
                        // Calculate the new volume.
                        m_Volume = (m_TransFadeOutTime - m_EnvelopeTime) / m_TransFadeOutTime;
                    }
                    break;

                case VOLUME_SILENT:
                    // Silent!
                    m_Volume = 0.0f;

                    // Only warm up track once.
                    if( m_bWarmFadeIn )
                    {
                        // Calculate time until fade is is supposed to start.
                        f32 TimeToFadeIn = m_TransDelayTime - m_EnvelopeTime;

                        // Time to start warming up?
                        if( TimeToFadeIn < (MINIMUM_TRANSITION_TIME+MINIMUM_TRANSITION_DELTA) )
                        {
                            // Dont warm it up again!
                            m_bWarmFadeIn = FALSE;

                            // Warm it up!
                            m_TransVoiceID = g_AudioMgr.Play( m_TransTrack, FALSE );
                            ASSERT( m_TransVoiceID );

                            // Success?
                            if( m_TransVoiceID )
                            {
                                // Woot, we are warming it up!
                                m_bIsWarming = TRUE;
                            }
                            // Safety (retail build) - Don't get stuck in a transition.
                            else
                            {
                                // Transition is over!
                                m_bInTransition = FALSE;

                                // Done.
                                return;
                            }
                        }
                    }

                    // Bump the time in this state.
                    m_EnvelopeTime += DeltaTime;

                    // Silent long enough?
                    if( (m_EnvelopeTime >= m_TransDelayTime) || (m_TransDelayTime <= 0.0f) )
                    {
                        // Reset time for next state.
                        m_EnvelopeTime = 0.0f;

                        // Change states.
                        m_EnvelopeState = VOLUME_FADEIN;
                    }
                    break;

                case VOLUME_FADEIN:
                    // Start the fade when the transition track is warmed up.
                    if( !m_bIsWarming )
                    {
                        // Only start it once
                        if( m_bStartFadeIn )
                        {
                            // Is the fade in too long for a non-looped track? Clamp it!
                            if( (m_TransFadeInTime > g_AudioMgr.GetLengthSeconds( m_TransVoiceID )) && !m_TransIsLooped )
                                 m_TransFadeInTime = g_AudioMgr.GetLengthSeconds( m_TransVoiceID );

                            // Clear flag.
                            m_bStartFadeIn = FALSE;

                            // Start it off quiet.
                            m_Volume = 0.0f;
                            g_AudioMgr.SetVolume( m_TransVoiceID, 0.0f );

                            // Start it up!
                            g_AudioMgr.Start( m_TransVoiceID );
                        }
                        else
                        {
                            // Bump the time in this state.
                            m_EnvelopeTime += DeltaTime;

                            // Volume fade in complete?
                            if( (m_EnvelopeTime >= m_TransFadeInTime) || (m_TransFadeInTime <= 0.0f) )
                            {
                                // Full volume!
                                m_Volume = 1.0f;

                                // Transition is over!
                                m_bInTransition = FALSE;
                            }
                            else
                            {
                                // Calculate the new volume.
                                m_Volume = 1.0f - ((m_TransFadeInTime - m_EnvelopeTime) / m_TransFadeInTime);
                            }
                        }
                    }        
                    break;

                default:
                    // Should never get here.
                    ASSERT( 0 );

                    // Safety (retail build) - Force transition done!
                    m_bInTransition = FALSE;
                    break;
            }
            break;

        case SWITCH_END_OF_TRACK:
        case SWITCH_BREAKPOINT:
            // Nothing to do here...
            break;

        default:
            // Should never get here.
            ASSERT( 0 );

            // Safety (retail build) - Force transition done!
            m_bInTransition = FALSE;
            break;
    }
}

void music_mgr::UpdateNormal( void )
{
    f32 Pitch = 1.0f / g_PerceptionMgr.GetAudioTimeDialation();

    // Update the volume (for fades).
    g_AudioMgr.SetVolume( m_CurrentVoiceID, m_Volume );
    g_AudioMgr.SetPitch ( m_CurrentVoiceID, Pitch );
    g_AudioMgr.SetVolume( m_TransVoiceID, m_Volume );
    g_AudioMgr.SetPitch ( m_TransVoiceID, Pitch );

    // Current track voice valid?
    if( g_AudioMgr.IsValidVoiceId( m_CurrentVoiceID ) )
    {
        //==================================================================================
        // This code loops the current track when we get near the end of the current track.
        //
        // NOTE: Looping is only possible if we are not warming up a track. This means that
        //       looping will not occur when we are warming up an END_OF_TRACK, BREAKPOINT
        //       or IMMEDIATE transition track.  This should not present a problem as 
        //       transition permission is not granted unless there is time to perform the 
        //       specified transition.
        //==================================================================================

        // Looped? Not warming? 
        if( m_bIsLooping && !m_bIsWarming )
        {      
            // Calculate the remaining time.
            f32 RemainingTime = g_AudioMgr.GetLengthSeconds( m_CurrentVoiceID ) - 
                                g_AudioMgr.GetCurrentPlayTime( m_CurrentVoiceID );

            // Close to end of track?
            if( RemainingTime < (MINIMUM_TRANSITION_TIME+MINIMUM_TRANSITION_DELTA) )
            {
                // Attempt to warm that bad boy up.
                m_TransVoiceID = g_AudioMgr.Play( m_CurrentTrack, FALSE );
                ASSERT( m_TransVoiceID );

                // Valid?
                if( m_TransVoiceID )
                {
                    // Warming it up, yeah!
                    m_bIsWarming = TRUE;

                    // Setup the segue.
                    g_AudioMgr.Segue( m_CurrentVoiceID, m_TransVoiceID );
                }
            }
        }
    }
    else
    {
        //========================================================================
        // This code terminates the warming state and ensures that the transition
        // track actually starts up (if there was no segue or the segue failed).
        //========================================================================

        // Nuke it since its invalid.
        m_CurrentVoiceID = 0;

        // Warming a track up?
        if( m_bIsWarming )
        {
            // This voice should always be valid (Survive in retail builds).
            if( !g_AudioMgr.IsValidVoiceId( m_TransVoiceID ) )
            {
                x_DebugMsg( "m_TransVoiceID: %08x\n", m_TransVoiceID );

                // Nuke it.
                m_TransVoiceID = 0;

                // Force transition done.
                m_bInTransition = FALSE;

                // Nuke it.
                m_bIsWarming = FALSE;

                // Restart the music.
                m_CurrentVoiceID = g_AudioMgr.Play( m_TransTrack, TRUE );
                x_strcpy( m_CurrentTrack, m_TransTrack );

                // Bail out.
                return;
            }

            // Is the transition track playing? (this means the segue fired)
            if( g_AudioMgr.GetCurrentPlayTime( m_TransVoiceID ) > 0.0f )
            {
                // New current voice!
                m_CurrentVoiceID = m_TransVoiceID;

                // Nuke it!
                m_TransVoiceID = 0;

                LOG_MESSAGE( "music_mgr::UpdateNormal",
                             "Transition is playing! (segue/autostart)" );
            }
            // Is the transition track ready? (no segue/segue failed, so need to manually start the track)
            else if( g_AudioMgr.GetIsReady( m_TransVoiceID ) )
            {
                // New current voice!
                m_CurrentVoiceID = m_TransVoiceID;

                // Nuke it!
                m_TransVoiceID = 0;

                // Start it!
                g_AudioMgr.Start( m_CurrentVoiceID );

                LOG_MESSAGE( "music_mgr::UpdateNormal",
                             "Transition FORCED to playing!" );
            }

            LOG_FLUSH();

            // Still warming or done?
            m_bIsWarming = (m_TransVoiceID != 0);

            // Transition done?
            if( m_bInTransition && !m_bIsWarming )
            {
                // Update current data from the transition data!
                x_strcpy( m_CurrentTrack, m_TransTrack );
                m_bIsLooping = m_TransIsLooped;

                // No longer in transition! (volume fade is special case).
                if( m_TransMode != SWITCH_VOLUME_FADE )
                    m_bInTransition = FALSE;
            }
        }
        else
        {
            // This voice should be invalid!
            ASSERT( !g_AudioMgr.IsValidVoiceId( m_TransVoiceID ) );
            
            // For safety (retail) - no losing voices!
            g_AudioMgr.Release( m_TransVoiceID, 0.0f );

            // Since not warming, nuke it.
            m_TransVoiceID = 0;
        }
    }
}

void music_mgr::Update( f32 DeltaTime )
{
#ifndef X_EDITOR
    if( g_StateMgr.IsPaused() )
    {
        return;
    }
#endif

    // Update silent flag on entry.
    m_bIsSilent = !g_AudioMgr.IsValidVoiceId( m_CurrentVoiceID );

    // Start a transition?
    if( m_bStartTransition )
        StartTransition();

    // Are we in transition?
    if( m_bInTransition )
        UpdateTransition( DeltaTime );

    // Normal update.
    UpdateNormal();

    // Update silent flag on exit.
    m_bIsSilent = !g_AudioMgr.IsValidVoiceId( m_CurrentVoiceID );
}
