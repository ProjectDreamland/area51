//==============================================================================
//
//  Headset_XBOX.cpp
//
//==============================================================================
#include "x_types.hpp"
#include "x_log.hpp"
#include "x_string.hpp"

#if !defined(TARGET_XBOX)
#error This should only be compiled for XBOX. Check your dependancies.
#endif

#include "headset.hpp"
#include "xtl.h"
#include "dsstdfx.h"

static xbool s_UseSpeakers = FALSE;

//===================================================================

void headset::Init( xbool )
{
    m_pFifoBuffer = new u8[ FIFOSIZE * 2 ];

    m_ReadFifo.Init ( m_pFifoBuffer + (FIFOSIZE * 0), FIFOSIZE );
    m_WriteFifo.Init( m_pFifoBuffer + (FIFOSIZE * 1), FIFOSIZE );

    // These pointers will allow us to modify the headset object
    // from within the callback functions (yes, a bit hacky).
    m_ITitleXHV.m_pOutgoingFifo         = &m_ReadFifo;
    m_ITitleXHV.m_pHeadsetMask          = &m_HeadsetMask;
    m_ITitleXHV.m_pIsTalking            = &m_IsTalking;
    m_ITitleXHV.m_pLoopbackEnabled      = &m_LoopbackEnabled;
    m_ITitleXHV.m_VoiceMailDuration     = 0;
    m_ITitleXHV.m_VoiceMailSize         = 0;
    m_ITitleXHV.m_pVoiceMailData        = NULL;
    m_ITitleXHV.m_VoiceIsRecording      = FALSE;
    m_ITitleXHV.m_VoiceIsPlaying        = FALSE;
    m_ITitleXHV.m_VoiceCompleted        = FALSE;
    m_ITitleXHV.m_HeadsetUpdated        = FALSE;
    m_ITitleXHV.m_HeadsetJustInserted   = FALSE;

    m_pXHVEngine                        = NULL;
    m_VoiceBanned                       = FALSE;
    m_VoiceAudible                      = TRUE;
    m_VoiceEnabled                      = TRUE;
    m_VoiceThroughSpeaker               = FALSE;
    m_IsTalking                         = FALSE;
    m_LoopbackEnabled                   = FALSE;
    m_VoiceTimer                        = 0.0f;
    m_MicrophoneSensitivity             = 0.0f;
    m_HeadsetVolume                     = 0.0f;
    m_HeadsetMask                       = 0;
    m_ActiveHeadset                     = -1;
    m_OldActiveHeadset                  = -1;
    m_EncodeBlockSize                   = ENCODESIZE;

    {
        extern LPDSEFFECTIMAGEDESC g_pDesc;
        XHV_RUNTIME_PARAMS Params;

        Params.dwMaxRemoteTalkers       = 1;
        Params.dwMaxLocalTalkers        = 4;
        Params.dwMaxCompressedBuffers   = 4;
        Params.dwFlags                  = 0;
        Params.pEffectImageDesc         = g_pDesc;
        Params.dwEffectsStartIndex      = GraphVoice_Voice_0;
        Params.dwOutOfSyncThreshold     = 10;
        Params.bCustomVADProvided       = FALSE;
        Params.bHeadphoneAlwaysOn       = FALSE;

        HRESULT Result = XHVEngineCreate( &Params, &m_pXHVEngine );
        ASSERT( SUCCEEDED( Result ) == TRUE );

        m_pXHVEngine->EnableProcessingMode( XHV_VOICECHAT_MODE );
        m_pXHVEngine->EnableProcessingMode( XHV_VOICEMAIL_MODE );
        m_pXHVEngine->SetCallbackInterface( &m_ITitleXHV );

        m_pXHVEngine->RegisterLocalTalker( 0 );
        m_pXHVEngine->RegisterLocalTalker( 1 );
        m_pXHVEngine->RegisterLocalTalker( 2 );
        m_pXHVEngine->RegisterLocalTalker( 3 );

        // Construct a dummy XUID for the XHV library to use
        m_RemoteID.qwUserID    = 0x12345678;
        m_RemoteID.dwUserFlags = 0;

        m_pXHVEngine->RegisterRemoteTalker( m_RemoteID );
    }
}

//==============================================================================

void headset::Kill( void )
{
    m_pXHVEngine->Release();
    m_pXHVEngine = NULL;

    m_WriteFifo.Kill();
    m_ReadFifo.Kill();

    delete[] m_pFifoBuffer;
}

//==============================================================================

void headset::Update( f32 DeltaTime )
{
    XONLINE_USER* pUsers = XOnlineGetLogonUsers();

    // Wait until the player is logged on to the XBox Live service
    if( pUsers == NULL )
        return;

    m_pXHVEngine->DoWork();

    UpdateActiveHeadset();
    UpdateIncoming();
    UpdateLoopBack();
    UpdateOutputMode();
    UpdateVoiceMail( DeltaTime );
}

//==============================================================================

void headset::UpdateActiveHeadset( void )
{
    // Check if the active headset has been changed
    if( m_OldActiveHeadset != m_ActiveHeadset )
    {
        if( m_ActiveHeadset == -1 )
        {
            m_pXHVEngine->SetProcessingMode( m_OldActiveHeadset, XHV_INACTIVE_MODE );
        }
        else
        {
            m_pXHVEngine->SetProcessingMode( m_ActiveHeadset, XHV_VOICECHAT_MODE );
        }

        m_OldActiveHeadset = m_ActiveHeadset;
    }

    // Determine if the players controller has a headset inserted to it
    m_HeadsetCount = 0;
    if( m_ActiveHeadset == -1 )
    {
        // No controller has been selected so check for ANY headset being present
        if( m_HeadsetMask != 0 )
            m_HeadsetCount = 1;
    }
    else
    {
        // We only support 1 usable headset plugged into the current players controller
        if( m_HeadsetMask & (1 << m_ActiveHeadset) )
            m_HeadsetCount = 1;
    }
}

//==============================================================================

void headset::UpdateIncoming( void )
{
    char IncomingBuffer[ FIFOSIZE ];

    // Consume any incoming audio data from other players one packet at a time
    while( m_WriteFifo.Remove( IncomingBuffer, ENCODESIZE, ENCODESIZE ) == TRUE )
    {
        // Ensure a headset is selected before playing back audio
        if( m_ActiveHeadset == -1 )
            continue;

        // Submit any incoming audio data for playback
        m_pXHVEngine->SubmitIncomingVoicePacket( m_RemoteID, IncomingBuffer, ENCODESIZE );
    }
}

//==============================================================================

void headset::UpdateOutputMode( void )
{
    if( m_ActiveHeadset == -1 )
        return;

    // Direct the audio output through the headset or speakers
    s_UseSpeakers = m_VoiceThroughSpeaker;

    // When no Headset is present we must use the speakers instead
    if( IsHardwarePresent() == FALSE )
        s_UseSpeakers = TRUE;

    // Determine if we can turn on audio output
    xbool IsEnabled = m_VoiceEnabled & m_VoiceAudible & !m_VoiceBanned;

    XHV_PLAYBACK_PRIORITY Priority = (IsEnabled == TRUE)            ?
                                      XHV_PLAYBACK_PRIORITY_MAX     :
                                      XHV_PLAYBACK_PRIORITY_NEVER;

    if( s_UseSpeakers == TRUE )
    {
        static DSMIXBINVOLUMEPAIR VolumePair = { DSMIXBIN_FXSEND_15, DSBVOLUME_MAX };
        static DSMIXBINS          MixBin     = { 1, &VolumePair };

        // Map the MixBin to the speakers
        m_pXHVEngine->SetMixBinMapping( m_RemoteID,
                                        XHV_PLAYBACK_TO_SPEAKERS,
                                        &MixBin );

        m_pXHVEngine->SetMixBinVolumes( m_RemoteID,
                                        &MixBin );

        m_pXHVEngine->SetPlaybackPriority( m_RemoteID, XHV_PLAYBACK_TO_SPEAKERS, Priority );
        m_pXHVEngine->SetPlaybackPriority( m_RemoteID, m_ActiveHeadset,          XHV_PLAYBACK_PRIORITY_NEVER );

        extern LPDIRECTSOUND8 xbox_GetDSound( void );
        LPDIRECTSOUND8 pDSound = xbox_GetDSound();

        DWORD GainParam = DWORD( m_HeadsetVolume * 0x7FFFFF );
        pDSound->SetEffectData( UserVoiceRateConverter_VoiceRateConverter,
                                32,
                                &GainParam,
                                sizeof( DWORD ),
                                DSFX_IMMEDIATE );
    }
    else
    {
        m_pXHVEngine->SetPlaybackPriority( m_RemoteID, XHV_PLAYBACK_TO_SPEAKERS, XHV_PLAYBACK_PRIORITY_NEVER );
        m_pXHVEngine->SetPlaybackPriority( m_RemoteID, m_ActiveHeadset,          Priority );
    }
}

//==============================================================================

void headset::UpdateVoiceMail( f32 DeltaTime )
{
    if( m_ITitleXHV.m_VoiceIsPlaying == TRUE )
    {
        // ALERT! (Unresolved issue swept under the carpet by JP)
        // For some reason XHV won't always give us a callback when playback has ended.
        // To be safe we will manually monitor our timer and stop playback if we overshoot.
        if( (m_VoiceTimer * 1000.0f) > m_ITitleXHV.m_VoiceMailDuration )
        {
            StopVoicePlaying();
        }
    }

    if( (m_ITitleXHV.m_VoiceIsRecording == TRUE) ||
        (m_ITitleXHV.m_VoiceIsPlaying   == TRUE) )
        m_VoiceTimer += DeltaTime;

    // Wait until a callback was triggered to signify the voice mail ended
    if( m_ITitleXHV.m_VoiceCompleted == TRUE )
    {
        EndVoiceMail();
    }

    // Check for the player inserting or removing the headset
    if( m_ITitleXHV.m_HeadsetUpdated == TRUE )
    {
        m_ITitleXHV.m_HeadsetUpdated = FALSE;

        if( m_ITitleXHV.m_VoiceIsRecording == TRUE )
            StopVoiceRecording();

        if( m_ITitleXHV.m_VoiceIsPlaying == TRUE )
            StopVoicePlaying();
    }
}

//==============================================================================

HRESULT headset::titlexhv::CommunicatorStatusUpdate( DWORD Port, XHV_VOICE_COMMUNICATOR_STATUS Status )
{
    if( Status == XHV_VOICE_COMMUNICATOR_STATUS_INSERTED )
    {
        *m_pHeadsetMask |=  (1 << Port);
        LOG_MESSAGE( "headset::CommunicatorStatusUpdate", "Headset inserted into port %d", Port );

        m_HeadsetJustInserted = TRUE;
    }
    else
    {
        *m_pHeadsetMask &= ~(1 << Port);
        LOG_MESSAGE( "headset::CommunicatorStatusUpdate", "Headset removed from port %d", Port );
    }

    m_HeadsetUpdated = TRUE;

    return( S_OK );
}

//==============================================================================

HRESULT headset::titlexhv::LocalChatDataReady( DWORD Port, DWORD Size, VOID* pData )
{
    if( (*m_pIsTalking == TRUE) || (*m_pLoopbackEnabled == TRUE) )
    {
        // Copy the data from the microphone into the outgoing FIFO
        m_pOutgoingFifo->Insert( pData, Size, ENCODESIZE );
    }

    return( S_OK );
}

//==============================================================================

HRESULT headset::titlexhv::VoiceMailDataReady( DWORD Port, DWORD DurationMS, DWORD Size )
{
    ASSERT( m_VoiceIsRecording == TRUE );

    m_VoiceCompleted    = TRUE;
    m_VoiceMailDuration = DurationMS;
    m_VoiceMailSize     = Size;

    return( S_OK );
}

//==============================================================================

HRESULT headset::titlexhv::VoiceMailStopped( DWORD Port )
{
    m_VoiceCompleted = TRUE;

    return( S_OK );
}

//==============================================================================

void headset::EndVoiceMail( void )
{
    m_ITitleXHV.m_VoiceCompleted   = FALSE;
    m_ITitleXHV.m_VoiceIsRecording = FALSE;
    m_ITitleXHV.m_VoiceIsPlaying   = FALSE;
    m_pXHVEngine->SetProcessingMode( m_ActiveHeadset, XHV_VOICECHAT_MODE );

    m_VoiceTimer = 0.0f;
}

//==============================================================================

xbool headset::InitVoiceRecording( void )
{
    ASSERT( m_ITitleXHV.m_VoiceIsRecording == FALSE );
    ASSERT( m_ITitleXHV.m_pVoiceMailData   == NULL  );

    s32 MaxTimeInMS = 1000 * 15;
    s32 BufferSize  = XHVGetVoiceMailBufferSize( MaxTimeInMS );

    m_ITitleXHV.m_pVoiceMailData = new byte[ BufferSize ];
    VERIFY( m_ITitleXHV.m_pVoiceMailData != NULL );

    x_memset( m_ITitleXHV.m_pVoiceMailData, 0, BufferSize );

    return( TRUE );
}

//==============================================================================

void headset::KillVoiceRecording( void )
{
    ASSERT( m_ITitleXHV.m_VoiceIsRecording == FALSE );
    ASSERT( m_ITitleXHV.m_pVoiceMailData   != NULL  );
    
    delete[] m_ITitleXHV.m_pVoiceMailData;
    m_ITitleXHV.m_pVoiceMailData    = NULL;
    m_ITitleXHV.m_VoiceMailSize     = 0;
    m_ITitleXHV.m_VoiceMailDuration = 0;
}

//==============================================================================

void headset::StartVoiceRecording( void )
{
    ASSERT( m_ITitleXHV.m_VoiceIsRecording == FALSE );
    ASSERT( m_ITitleXHV.m_pVoiceMailData   != NULL  );

    if( IsHardwarePresent() == FALSE )
        return;

    DWORD MaxTimeInMS = 15 * 1000;
    DWORD BufferSize  = XHVGetVoiceMailBufferSize( MaxTimeInMS );

    m_pXHVEngine->SetProcessingMode( m_ActiveHeadset, XHV_VOICEMAIL_MODE );
    m_pXHVEngine->VoiceMailRecord  ( m_ActiveHeadset,
                                     MaxTimeInMS,
                                     BufferSize,
                                     m_ITitleXHV.m_pVoiceMailData );

    // Assume the recording will get aborted until we know otherwise
    m_ITitleXHV.m_VoiceIsRecording  = TRUE;
    m_ITitleXHV.m_VoiceCompleted    = FALSE;
    m_ITitleXHV.m_VoiceMailSize     = 0;
    m_ITitleXHV.m_VoiceMailDuration = 0;
    m_VoiceTimer                    = 0.0f;
}

//==============================================================================

void headset::StopVoiceRecording( xbool DoForcefullStop )
{
    m_pXHVEngine->VoiceMailStop( m_ActiveHeadset );

    // NOTE: In the normal case we call StopVoiceRecording( FALSE ) to end the voice
    // mail gracefully.  The next time the headset Update() function is called, the
    // XHV DoWork() will then trigger the VoiceMailStopped callback.
    // Finally, the following headset Update will detect the voice mail ended and call
    // VoiceMailEnded().  However, if there was a duplicate login whilst in the middle
    // of recording a voice mail, the recording must be stopped.  The current user will
    // have been logged out of Xbox Live so we are not able to call XHV DoWork anymore.
    // This has the unfortunate effect of causing the VoiceMailStopped callback to never
    // trigger.  The solution is to allow a forcefull stop like so:
    if( DoForcefullStop == TRUE )
        EndVoiceMail();
}

//==============================================================================

f32 headset::GetVoiceRecordingProgress( void )
{
    f32 T = 0.0f;
    
    if( m_ITitleXHV.m_VoiceIsRecording == TRUE )
    {
        T = MIN( (m_VoiceTimer / 15.0f), 1.0f );
    }

    return( T );
}

//==============================================================================

byte* headset::GetVoiceMessageRec( void )
{
    return( m_ITitleXHV.m_pVoiceMailData );
}

//==============================================================================

s32 headset::GetVoiceNumBytesRec( void )
{
    return( m_ITitleXHV.m_VoiceMailSize );
}

//==============================================================================

xbool headset::GetVoiceIsRecording( void )
{
    return( m_ITitleXHV.m_VoiceIsRecording );
}

//==============================================================================

void headset::StartVoicePlaying( byte* pVoiceMessage,
                                 s32   DurationMS,
                                 s32   NumBytes )
{
    ASSERT( m_ITitleXHV.m_VoiceIsPlaying == FALSE );

    m_ITitleXHV.m_VoiceIsPlaying    = TRUE;
    m_ITitleXHV.m_VoiceCompleted    = FALSE;
    m_ITitleXHV.m_VoiceMailDuration = DurationMS;
    m_VoiceTimer                    = 0.0f;

    m_pXHVEngine->SetProcessingMode( m_ActiveHeadset, XHV_VOICEMAIL_MODE );
    m_pXHVEngine->VoiceMailPlay    ( m_ActiveHeadset,
                                     NumBytes,
                                     pVoiceMessage,
                                     s_UseSpeakers );
}

//==============================================================================

void headset::StopVoicePlaying( xbool DoForcefullStop )
{
    m_pXHVEngine->VoiceMailStop( m_ActiveHeadset );

    // See note in StopVoiceRecording
    if( DoForcefullStop == TRUE )
        EndVoiceMail();
}

//==============================================================================

f32 headset::GetVoicePlayingProgress( void )
{
    f32 T = 0.0f;

    if( m_ITitleXHV.m_VoiceIsPlaying == TRUE )
    {
        if( m_ITitleXHV.m_VoiceMailDuration > 0.0f )
        {
            T = (m_VoiceTimer * 1000.0f) / m_ITitleXHV.m_VoiceMailDuration;
            T = MIN( T, 1.0f );
        }
    }

    return( T );
}

//==============================================================================

xbool headset::GetVoiceIsPlaying( void )
{
    return( m_ITitleXHV.m_VoiceIsPlaying );
}

//==============================================================================

s32 headset::GetVoiceDurationMS( void )
{
    return( m_ITitleXHV.m_VoiceMailDuration );
}

//==============================================================================

void headset::OnHeadsetInsert( void )
{
}

//==============================================================================

void headset::OnHeadsetRemove( void )
{
}

//==============================================================================

void headset::PeriodicUpdate( f32 )
{
}

//==============================================================================

xbool headset::HeadsetJustInserted( void )
{
    return( m_ITitleXHV.m_HeadsetJustInserted );
}

//==============================================================================

void headset::ClearHeadsetJustInserted( void )
{
    m_ITitleXHV.m_HeadsetJustInserted = FALSE;
}

//==============================================================================
