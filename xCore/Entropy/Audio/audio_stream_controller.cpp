#include "audio_stream_controller.hpp"

//------------------------------------------------------------------------------

audio_stream_controller::audio_stream_controller( void )
{
    m_nStreams = 0;
    for( s32 i=0 ; i<MAX_AUDIO_STREAMS ; i++ )
    {
        m_Data[ i ].Priority = -1;
        m_Data[ i ].VoiceID  = 0;
    }
}

//------------------------------------------------------------------------------

audio_stream_controller::~audio_stream_controller( void )
{
}

//------------------------------------------------------------------------------

void audio_stream_controller::Init( void )
{
}

//------------------------------------------------------------------------------

void audio_stream_controller::Kill( void )
{
    // Kill any audio playing.
    for( s32 i=0 ; i<m_nStreams ; i++ )
    {
        g_AudioMgr.Release( m_Data[ i ].VoiceID, 0.0f );
    }

    // Give em up!
    if( m_nStreams )
        g_AudioStreamMgr.UnReserveStreams( m_nStreams );

    // None left!
    m_nStreams = 0;
}

//------------------------------------------------------------------------------

void audio_stream_controller::ReserveStreams( s32 nStreams )
{
    // Kill any audio playing
    for( s32 i=0 ; i<m_nStreams ; i++ )
    {
        g_AudioMgr.Release( m_Data[ i ].VoiceID, 0.0f );
    }

    // Free up any currently reserved streams.
    if( m_nStreams )
        g_AudioStreamMgr.UnReserveStreams( m_nStreams );

    // Now reserve the streams.
    g_AudioStreamMgr.ReserveStreams( nStreams );
    m_nStreams = nStreams;
    
    // Reset.
    for( s32 i=0 ; i<m_nStreams ; i++ )
    {
        m_Data[ i ].Priority = -1;
        m_Data[ i ].VoiceID  = 0;
    }
}

//------------------------------------------------------------------------------

voice_id audio_stream_controller::Play( const char* pIdentifier,
                                        xbool       AutoStart )
{
    vector3  Dummy;
    vector3& DummyRef = Dummy;
    return Play( pIdentifier, AutoStart, DummyRef, -1, FALSE, FALSE );
}

//------------------------------------------------------------------------------

voice_id audio_stream_controller::Play( const char*     pIdentifier,
                                        const vector3&  Position,
                                        s32             ZoneID,
                                        xbool           AutoStart )
{
    return Play( pIdentifier, AutoStart, Position, ZoneID, TRUE, FALSE );
}

//------------------------------------------------------------------------------

voice_id audio_stream_controller::PlayVolumeClipped( const char*    pIdentifier,
                                                     const vector3& Position,
                                                     s32             ZoneID,
                                                     xbool          AutoStart )
{
    return Play( pIdentifier, AutoStart, Position, ZoneID, TRUE, TRUE );
}

//------------------------------------------------------------------------------

voice_id audio_stream_controller::Play( const char*    pIdentifier, 
                                        xbool          AutoStart, 
                                        const vector3& Position, 
                                        s32            ZoneID,
                                        xbool          IsPositional, 
                                        xbool          bVolumeClip )
{
    voice_id Result      = 0;
    s32      iStream     = -1;
    s32      LowStream   = -1;
    s32      LowPriority = 256;
    s32      Priority    = g_AudioMgr.GetPriority( pIdentifier );


    // Ok lets look for an available stream.
    for( s32 i=0 ; (i<m_nStreams) && (iStream == -1) ; i++ )
    {
        // Valid voice?
        if( g_AudioMgr.IsValidVoiceId( m_Data[ i ].VoiceID ) )
        {
            // Gotta be lower priority...
            if( (m_Data[ i ].Priority < Priority) && (m_Data[ i ].Priority < LowPriority) )
            {
                // Note the lowest priority found so far.
                LowPriority = m_Data[ i ].Priority;
                LowStream   = i;
            }
        }
        else
        {
            // Ah! Founda free one!
            iStream = i;
        }
    }

    // Any streams available?
    if( iStream == -1 )
    {
        // No streams available! Hmm...was a lower priority stream found?
        if( LowStream != -1 )
        {
            // Attempt to play the sound.
            Result = g_AudioMgr.PlayInternal( pIdentifier, AutoStart, Position, ZoneID, IsPositional, bVolumeClip );

            // Success?
            if( Result )
            {
                // Ok, take over the low priority one.
                iStream = LowStream;

                // Kill current stream.
                g_AudioMgr.Release( m_Data[ iStream ].VoiceID, 0.0f );

                // Note the *new* priority and voice id.
                m_Data[ iStream ].Priority = Priority;
                m_Data[ iStream ].VoiceID  = Result;
            }
        }
    }
    // Woot! A stream is available!
    else
    {
        // Attempt to play the sound.
        Result = g_AudioMgr.PlayInternal( pIdentifier, AutoStart, Position, ZoneID, IsPositional, bVolumeClip );

        // Success?
        if( Result )
        {
            // Note the priority and voice id.
            m_Data[ iStream ].Priority = Priority;
            m_Data[ iStream ].VoiceID  = Result;
        }
    }

    LOG_MESSAGE( "audio_stream_controller::Play", "Identifier: %s, Result: %08x", pIdentifier, Result );

    // Tell the world!
    return Result;
}

//------------------------------------------------------------------------------

s32 audio_stream_controller::GetAvailableStreams( void )
{
    s32 Result = 0;

    // Ok lets look for an available stream.
    for( s32 i=0 ; i<m_nStreams ; i++ )
    {
        // Valid voice?
        if( g_AudioMgr.IsValidVoiceId( m_Data[ i ].VoiceID ) )
        {
            Result++;
        }
    }

    return Result;
}

//------------------------------------------------------------------------------

s32 audio_stream_controller::GetMaxStreams( void )
{
    return m_nStreams;
}

//------------------------------------------------------------------------------
void audio_stream_controller::ClearStreams( void )
{
    // Ok lets look for an available stream.
    for( s32 i=0 ; i<m_nStreams ; i++ )
    {
        // Valid voice?
        if( g_AudioMgr.IsValidVoiceId( m_Data[ i ].VoiceID ) )
        {
            // Stop it.
            g_AudioMgr.Release( m_Data[i].VoiceID, 0.0f );
        }
    }
}


