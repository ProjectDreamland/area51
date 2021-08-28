#include "x_target.hpp"

#if !defined(TARGET_XBOX)
#error This is for a XBOX target build. Please exclude from build rules.
#endif

#include "audio_channel_mgr.hpp"
#include "audio_hardware.hpp"
#include "audio_package.hpp"
#include "audio_inline.hpp"
#include "audio_stream_mgr.hpp"
#include "e_ScratchMem.hpp"
#include "e_SimpleAllocator.hpp"
#include "e_virtual.hpp"
#include "dsound.h"
#include "dsstdfx.h"

#define MAX_AUDIO_RAM 0//( 1048576*8 )

#define SAMPLES_TO_ADPCM_BYTES(x)  ((x)*36/64)
#define ADPCM_BYTES_TO_SAMPLES(x)  ((x)*64/36)

//------------------------------------------------------------------------------
// XBOX specific defines. 

#define MAX_HARDWARE_CHANNELS   (64)
static void* s_pOs;

//------------------------------------------------------------------------------

static channel          s_Channels[ MAX_HARDWARE_CHANNELS ];    // Channel buffer
static xbool            s_IsInitialized = FALSE;                // Sentinel
static xthread *        s_AudioUpdateThread = NULL;
static LPDIRECTSOUND8   s_lpDs8;
static CRITICAL_SECTION s_ListCrit;

LPDSEFFECTIMAGEDESC     g_pDesc;

//------------------------------------------------------------------------------

audio_hardware   g_AudioHardware;
simple_allocator s_AramAllocator;

//==============================================================================
// Hardware specific functions.
//==============================================================================

LPDIRECTSOUND8 xbox_GetDSound( void )
{
    return( s_lpDs8 );
}

//==============================================================================

static void xbox_UpdateStreamADPCM( channel* pChannel )
{
    u32   CurrentPosition  = pChannel->CurrBufferPosition;
    u32   PreviousPosition = pChannel->StreamData.PreviousPosition; 
    s32   Transition;
    xbool bTransition;

    // The pStream pointer was coming up NULL in a retail build.
    // We have to survive this.
    if( !pChannel )
        return;

    // Update previous.
    pChannel->StreamData.PreviousPosition = CurrentPosition;

    // Which buffer are we in?
    if( PreviousPosition <= pChannel->MidPoint )
    {
        // Did a buffer transition occur?
        bTransition = (CurrentPosition > pChannel->MidPoint);
        Transition  = 1;
    }
    else
    {
        // Did a buffer transition occur?
        bTransition = (CurrentPosition <= pChannel->MidPoint);
        Transition  = 2;
    }

    // Transition occur?
    if( bTransition )
    {
        // Update the base position if buffer wrap occured.
        if( Transition == 2 )
        {
            pChannel->Hardware.BasePosition += ((STREAM_BUFFER_SIZE * 2 / 36) * 36);
        }

        // Bail if an error
        if( !pChannel->StreamData.pStream )
        {
            ASSERT(0);
            return;
        }

        // Stream done?
        if( pChannel->StreamData.pStream->StreamDone )
        {
        }
        // Need to read from the stream?
        else if( pChannel->StreamData.StreamControl )
        {
            // Fill the read buffer.
            g_AudioStreamMgr.ReadStream( pChannel->StreamData.pStream );
        }
    }

    // Bail if an error
    if( !pChannel->StreamData.pStream )
    {
        ASSERT(0);
        return;
    }

    if( pChannel->StreamData.pStream->StreamDone )
	{
		// Calculate absolute position
        pChannel->Hardware.CurrentPosition = pChannel->Hardware.BasePosition + pChannel->CurrBufferPosition;

        // Get actual number of bytes in the sample.
        s32 nSampleBytes = SAMPLES_TO_ADPCM_BYTES( pChannel->Sample.pColdSample->nSamples );

        // Need to release it?
        if( pChannel->Hardware.CurrentPosition >= nSampleBytes )
        {
            // Nuke it.
            g_AudioHardware.ReleaseChannel( pChannel );
        }
	}
}

//------------------------------------------------------------------------------

s32 GetAudioLevel( void )
{
    DSOUTPUTLEVELS Dol;
    s_lpDs8->GetOutputLevels( &Dol,FALSE );
static s32 Scalar = 100;
    return(s32)x_max( Dol.dwDigitalFrontLeftPeak/Scalar,Dol.dwDigitalFrontRightPeak/Scalar );
}

//------------------------------------------------------------------------------

void audio_hardware::UpdateStream( channel* pChannel )
{
    // Error check.
    ASSERT( pChannel );
#ifndef bhapgood    
//    ASSERT( pChannel->StreamData.pStream );
#endif
    // Cold, active, running channel?
    if( pChannel && (pChannel->Type == COLD_SAMPLE) && (pChannel->State == STATE_RUNNING) )
    { 
        // What kind of compression?
        switch( pChannel->Sample.pHotSample->CompressionType )
        {
            // ADPCM?
            case ADPCM:
                xbox_UpdateStreamADPCM( pChannel );
                break;

            default:
                ASSERT( 0 );
                break;
        }
    }
}

//------------------------------------------------------------------------------

audio_hardware::audio_hardware( void )
{
    m_FirstChannel   = s_Channels;
    m_LastChannel    = s_Channels + (MAX_HARDWARE_CHANNELS - 1);
    m_InterruptLevel = 0;
    m_InterruptState = FALSE;
}

//------------------------------------------------------------------------------

audio_hardware::~audio_hardware( void )
{
}

//------------------------------------------------------------------------------

static LPDIRECTSOUNDBUFFER8 CreateBuffer( channel& Channel )
{   
    //
    //  Create wave format
    //
    WAVEFORMATEX wfx;
    ZeroMemory( &wfx,sizeof(WAVEFORMATEX) );
    wfx.nSamplesPerSec  = 44100;
    wfx.nAvgBytesPerSec = 0;
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.wBitsPerSample  = 16;
    wfx.nBlockAlign     = 2;
    wfx.nChannels       = 1;

    //
    //  Load buffer desc
    //
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd,sizeof(DSBUFFERDESC) );
    dsbd.dwBufferBytes = 0;
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = 0; //DSBCAPS_LOCDEFER; //DSBCAPS_CTRL3D;
    dsbd.lpwfxFormat   = &wfx;
	dsbd.lpMixBins     = &Channel.Hardware.dsmb;  // Set the mix bins address.

	Channel.Hardware.dsmb.dwMixBinCount = 4;
	Channel.Hardware.dsmb.lpMixBinVolumePairs = Channel.Hardware.dsmbvp;

    // Our default mapping is to send to every speaker at minimum
    // volume.  This way, any time we switch sounds, we'll be 
    // sending to all speakers.  
    Channel.Hardware.dsmbvp[0].dwMixBin = DSMIXBIN_3D_FRONT_LEFT;
    Channel.Hardware.dsmbvp[0].lVolume  = DSBVOLUME_MIN;
    Channel.Hardware.dsmbvp[1].dwMixBin = DSMIXBIN_3D_FRONT_RIGHT;
    Channel.Hardware.dsmbvp[1].lVolume  = DSBVOLUME_MIN;
    Channel.Hardware.dsmbvp[2].dwMixBin = DSMIXBIN_3D_BACK_RIGHT;
    Channel.Hardware.dsmbvp[2].lVolume  = DSBVOLUME_MIN;
    Channel.Hardware.dsmbvp[3].dwMixBin = DSMIXBIN_3D_BACK_LEFT;
    Channel.Hardware.dsmbvp[3].lVolume  = DSBVOLUME_MIN;
    Channel.Hardware.dsmbvp[4].dwMixBin = DSMIXBIN_FRONT_CENTER;
    Channel.Hardware.dsmbvp[4].lVolume  = DSBVOLUME_MIN;
    Channel.Hardware.dsmbvp[5].dwMixBin = DSMIXBIN_LOW_FREQUENCY;
    Channel.Hardware.dsmbvp[5].lVolume  = DSBVOLUME_MIN;

    //
    //  Load ds interface
    //
    LPDIRECTSOUNDBUFFER8 pdsb;
    HRESULT hr = s_lpDs8->CreateSoundBuffer( &dsbd, &pdsb, NULL );
    ASSERT(!hr );

	pdsb->SetMixBins( &Channel.Hardware.dsmb );

	// Nuke the headroom, its built into the audio levels.
	pdsb->SetHeadroom( 0 );

    return pdsb;
}

//------------------------------------------------------------------------------

void audio_hardware::Init( s32 MemSize )
{
    (void)MemSize;
    InitializeCriticalSection( &s_ListCrit );

    // Create direct sound object
    HRESULT hr;
    ASSERTS( !s_IsInitialized, "Already initialized" );
    hr = DirectSoundCreate( NULL, &s_lpDs8, NULL );
    ASSERTS( !hr, "Cannot create Direct Sound");

    // Initialise 3D sound
    DirectSoundUseLightHRTF();

    // ok its done.
    s_IsInitialized = TRUE;

    //
    //  Initialise all the hardware channels
    //
    for (s32 i=0;i<MAX_HARDWARE_CHANNELS; i++ )
    {
        channel& Channel                 = s_Channels[ i ];
        Channel.Hardware.pdsBuffer       = CreateBuffer( Channel );
        Channel.Hardware.CurrentPosition = 0;
        Channel.Hardware.BasePosition    = 0;
        Channel.Hardware.IsStarted       = FALSE;
        Channel.Hardware.InUse           = FALSE;
    }

    PVOID pBuffer = XLoadSection( "dsp_program" );
    s32 nlen = 18608;
    //
    //  Download to DSP
    //
    DSEFFECTIMAGELOC    EffectLoc ;
    EffectLoc.dwI3DL2ReverbIndex = GraphI3DL2_I3DL2Reverb;
    EffectLoc.dwCrosstalkIndex   = GraphXTalk_XTalk;
    hr = s_lpDs8->DownloadEffectsImage(
        pBuffer,
        nlen,
        &EffectLoc,
        &g_pDesc
    ) ;
    XFreeSection( "dsp_program" );

    m_PitchFactor  = 1.0f;
    m_VolumeFactor = 1.0f;

    //
    //  Initialise simple allocator
    //

    #if MAX_AUDIO_RAM
    s_pOs = x_malloc( MAX_AUDIO_RAM );
    ASSERT( s_pOs );
    s_AramAllocator.Init( s_pOs, MAX_AUDIO_RAM );
    #endif
/*
	// Headroom stuff
    u32 SpeakerConfig = XGetAudioFlags();
    if ( 1 ) //|| XC_AUDIO_FLAGS_ENCODED( SpeakerConfig ) & XC_AUDIO_FLAGS_ENABLE_AC3 )
    {
	    s_lpDs8->SetMixBinHeadroom( DSMIXBIN_3D_BACK_LEFT, 7 );
	    s_lpDs8->SetMixBinHeadroom( DSMIXBIN_3D_BACK_RIGHT, 7 );
	    s_lpDs8->SetMixBinHeadroom( DSMIXBIN_3D_FRONT_LEFT, 7 );
	    s_lpDs8->SetMixBinHeadroom( DSMIXBIN_3D_FRONT_RIGHT, 7 );
    }
*/
}

//------------------------------------------------------------------------------

void audio_hardware::Kill( void )
{
    ASSERT( s_IsInitialized );

    delete s_AudioUpdateThread;

    s_IsInitialized = FALSE;

    for (s32 i=0;i<MAX_HARDWARE_CHANNELS; i++ )
    {
        channel& Channel = s_Channels[ i ];
        Channel.Hardware.pdsBuffer->Release();
        Channel.Hardware.pdsBuffer = NULL;
        Channel.Hardware.InUse = 0;
    }

    #if MAX_AUDIO_RAM
    s_AramAllocator.Kill();
    x_free( s_pOs );
    #endif

    s_lpDs8->Release();

    DeleteCriticalSection( &s_ListCrit );
}

//------------------------------------------------------------------------------
// Doesn't do anything on xbox.
//
void audio_hardware::ResizeMemory( s32 )
{
}

//------------------------------------------------------------------------------

s32 audio_hardware::NumChannels( void )
{
    return MAX_HARDWARE_CHANNELS;
}

//------------------------------------------------------------------------------

channel* audio_hardware::GetChannelBuffer( void )
{
    return s_Channels;
}

//------------------------------------------------------------------------------

xbool audio_hardware::AcquireChannel( channel* pChannel )
{
    EnterCriticalSection( &s_ListCrit );

    ASSERT( !pChannel->Hardware.InUse );
    pChannel->Hardware.InUse = TRUE;

    LeaveCriticalSection( &s_ListCrit );
    return TRUE;
}

//------------------------------------------------------------------------------

void audio_hardware::ReleaseChannel( channel* pChannel )
{
    EnterCriticalSection( &s_ListCrit );

    // Look for segue.
    if( pChannel->pElement && pChannel->pElement->pVoice )
    {
        voice* pVoice = pChannel->pElement->pVoice;
        voice* pNext  = pVoice->pSegueVoiceNext;
        voice* pPrev  = pVoice->pSegueVoicePrev;

        // Take it out of list
        if( pPrev )
        {
            pPrev->pSegueVoiceNext = NULL;
        }

        if( pNext && (pNext->StartQ == 0) )
        {
            // Mark the queued voice to start.
            pNext->StartQ          = 1;
            pNext->pSegueVoicePrev = NULL;
        }
    }

	if( pChannel->Hardware.InUse )
    {
        // Stop it.
        LPDIRECTSOUNDBUFFER8 pdsb = pChannel->Hardware.pdsBuffer;
        pdsb->Stop( );

        // Does this channel have a stream?
        if( pChannel->StreamData.pStream && pChannel->StreamData.StreamControl )
        {
            // If so, nuke the stream
            g_AudioStreamMgr.ReleaseStream( pChannel->StreamData.pStream );
            pChannel->StreamData.pStream = NULL;
        }

        // No longer in use.
        pChannel->Hardware.InUse = FALSE;
    }

    LeaveCriticalSection( &s_ListCrit );
}

//------------------------------------------------------------------------------

// TODO: Delete this function.
void audio_hardware::ClearChannel( channel* pChannel )
{
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::DuplicatePriority( channel* pDest, channel* pSrc )
{
    // Just until the callback runs.
    pDest->Hardware.Priority = pSrc->Hardware.Priority;
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
xbool audio_hardware::IsChannelActive( channel* pChannel )
{
    return pChannel->Hardware.InUse;
}

//------------------------------------------------------------------------------

void audio_hardware::StartChannel( channel* pChannel )
{
    //
    //  If the channel has hardware then start it!
    //
    hot_sample* pSample  = pChannel->Sample.pHotSample ;
    LPDIRECTSOUNDBUFFER8 pdsb = pChannel->Hardware.pdsBuffer;
    if( !pChannel->Hardware.IsStarted )
    {
        if( !pChannel->Hardware.IsLooped )
        {
            pdsb->Play( 0, 0, DSBPLAY_FROMSTART );
        }
        else
        {
            pdsb->Play( 0, 0, DSBPLAY_LOOPING );
        }
        pChannel->Hardware.IsStarted = TRUE;
    }
    else
    {
        pdsb->Pause( DSBPAUSE_RESUME );
    }
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::StopChannel( channel* pChannel )
{
    // If the hardware channel is active, stop it!
    if( pChannel->Hardware.IsStarted )
    {
        pChannel->Hardware.pdsBuffer->Pause( DSBPAUSE_PAUSE );
        pChannel->Hardware.IsStarted = FALSE;
    }
}
//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::PauseChannel( channel* pChannel )
{
    pChannel->Hardware.pdsBuffer->Pause( DSBPAUSE_PAUSE );
    pChannel->Hardware.IsStarted = FALSE;
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::ResumeChannel( channel* pChannel )
{
    pChannel->Hardware.pdsBuffer->Pause( DSBPAUSE_RESUME );
    pChannel->Hardware.IsStarted = TRUE;
}

//------------------------------------------------------------------------------

void audio_hardware::InitChannel( channel* pChannel )
{
    // setup the buffer pointer
    hot_sample* pSample  = pChannel->Sample.pHotSample ;
    
    // Is it looped?
    pChannel->Hardware.IsLooped = (pSample->LoopEnd != 0);

    // Not streaming...(just to be sure)
    pChannel->StreamData.pStream       = NULL;
    pChannel->StreamData.StreamControl = FALSE;
    pChannel->Hardware.CurrentPosition = 0;
    pChannel->CurrBufferPosition       = 0;
    pChannel->PrevBufferPosition       = 0;
	pChannel->Hardware.BasePosition    = 0;
    pChannel->Hardware.IsStarted       = FALSE;

    // Init.
    pChannel->nSamplesAdjust  =
    pChannel->nSamplesBase    =
    pChannel->PlayPosition    =
    pChannel->ReleasePosition = 0;

    LPDIRECTSOUNDBUFFER8 pdsb = pChannel->Hardware.pdsBuffer;
    pdsb->SetBufferData( LPVOID( pSample->AudioRam ), pSample->WaveformLength );
    pdsb->SetCurrentPosition( 0 );

    // setup the waveform
    WAVEFORMATEXTENSIBLE wfx;
    ZeroMemory( &wfx,sizeof(WAVEFORMATEXTENSIBLE) );
    wfx.Format.nSamplesPerSec       = pSample->SampleRate;
    wfx.Format.wFormatTag           = WAVE_FORMAT_XBOX_ADPCM;
    wfx.Format.wBitsPerSample       = 4;
    wfx.Format.nChannels            = 1;
    wfx.Format.nBlockAlign          = 36*wfx.Format.nChannels;
    wfx.Format.nAvgBytesPerSec      = wfx.Format.nSamplesPerSec*wfx.Format.nBlockAlign/64;
    wfx.Format.cbSize               = 2; // Supposed to be sizeof(WAVEFORMATEXTENSIBLE)–sizeof(WAVEFORMATEX)  except the compiler complains about unicode.
    wfx.Samples.wSamplesPerBlock    = 64;

    pdsb->SetFormat( (WAVEFORMATEX*)&wfx );

    // Is it looped?
    if( pChannel->Hardware.IsLooped )
    {
        // Loop at the end of the buffer.
        s32 LoopStart  = pSample->LoopStart / 64 * 36;
        s32 LoopEnd    = pSample->LoopEnd   / 64 * 36;
        s32 LoopLength = LoopEnd-LoopStart;
        pdsb->SetLoopRegion( LoopStart, LoopLength  );
    }

	// Max volume!
	pdsb->SetVolume( DSBVOLUME_MAX );

	// Set the mix bins.
	pdsb->SetMixBins( &pChannel->Hardware.dsmb );
}

//------------------------------------------------------------------------------

void audio_hardware::InitChannelStreamed( channel* pChannel )
{
    s32 nSampleBytes = (pChannel->Sample.pHotSample->nSamples + 63) / 64 * 36;
    s32 nBufferBytes = (STREAM_BUFFER_SIZE * 2 / 36) * 36;

    // Is it looped?
    pChannel->Hardware.IsLooped = (pChannel->Sample.pHotSample->LoopEnd != 0);

    // Set the current position
    pChannel->CurrBufferPosition = 0;
	pChannel->PrevBufferPosition = 0;
	pChannel->StreamData.PreviousPosition = 0;
    pChannel->Hardware.IsStarted = FALSE;

    // setup the buffer pointer
    hot_sample* pSample  = pChannel->Sample.pHotSample ;

    // MUST be looped!
    ASSERT( pChannel->Hardware.IsLooped );

    // Set the mid point, clear loop stop.
    pChannel->MidPoint                 = STREAM_BUFFER_SIZE;
    pChannel->Hardware.CurrentPosition = 0;
    pChannel->Hardware.BasePosition    = 0;
    pChannel->Hardware.IsStarted       = FALSE;

    // Init.
    pChannel->nSamplesAdjust  =
    pChannel->nSamplesBase    =
    pChannel->PlayPosition    =
    pChannel->ReleasePosition = 0;

	LPDIRECTSOUNDBUFFER8 pdsb = pChannel->Hardware.pdsBuffer;
    
    // Will sample fit in one buffer?
    if( nSampleBytes < nBufferBytes )
    {
        nBufferBytes                = nSampleBytes;
        pChannel->Hardware.IsLooped = FALSE;
    }

    pdsb->SetBufferData( LPVOID( pSample->AudioRam ), nBufferBytes );

    // setup the waveform
    WAVEFORMATEXTENSIBLE wfx;
    ZeroMemory( &wfx,sizeof(WAVEFORMATEXTENSIBLE) );
    wfx.Format.nSamplesPerSec       = pSample->SampleRate;
    wfx.Format.wFormatTag           = WAVE_FORMAT_XBOX_ADPCM;
    wfx.Format.wBitsPerSample       = 4;
    wfx.Format.nChannels            = 1;
    wfx.Format.nBlockAlign          = 36*wfx.Format.nChannels;
    wfx.Format.nAvgBytesPerSec      = wfx.Format.nSamplesPerSec*wfx.Format.nBlockAlign/64;
    wfx.Format.cbSize               = 2; // Supposed to be sizeof(WAVEFORMATEXTENSIBLE)–sizeof(WAVEFORMATEX)  except the compiler complains about unicode.
    wfx.Samples.wSamplesPerBlock    = 64;

    pdsb->SetFormat( (WAVEFORMATEX*)&wfx );
    pdsb->SetCurrentPosition( 0 );
    // Is it looped?
    if( pChannel->Hardware.IsLooped )
    {
        // Loop at the end of the buffer.
        s32 LoopStart  = (pSample->LoopStart / 36) * 36;
        s32 LoopEnd    = (pSample->LoopEnd   / 36) * 36;
        s32 LoopLength = LoopEnd-LoopStart;
        pdsb->SetLoopRegion( LoopStart, LoopLength  );
    }

	// Max volume!
	pdsb->SetVolume( DSBVOLUME_MAX );

	// Set the mix bins.
	pdsb->SetMixBins( &pChannel->Hardware.dsmb );
}

//------------------------------------------------------------------------------

void audio_hardware::Lock( void )
{
    m_LockMutex.Acquire();
    m_InterruptLevel++;
}

//------------------------------------------------------------------------------

void audio_hardware::Unlock( void )
{
    m_InterruptLevel--;
    m_LockMutex.Release();
}

//------------------------------------------------------------------------------

u32 audio_hardware::GetSamplesPlayed( channel* pChannel )
{
    // What kind of compression?
    switch( pChannel->Sample.pHotSample->CompressionType )
    {
        // ADPCM?
        case ADPCM:
			pChannel->nSamplesAdjust = ADPCM_BYTES_TO_SAMPLES( pChannel->CurrBufferPosition );
			break;

        default:
            ASSERT( 0 );
            break;
    }

    // Tell the world. (base is always 0 on pc until streaming is implemented).
    return pChannel->nSamplesBase + pChannel->nSamplesAdjust;
}


//------------------------------------------------------------------------------

static xbool UpdatePosition( channel* pChannel )
{
    // Only need to do special stuff for cold samples
    if( pChannel->Type == COLD_SAMPLE )
    {
        // Played an entire buffers worth of samples?
        if( (pChannel->PrevBufferPosition >= pChannel->MidPoint) &&  
            (pChannel->CurrBufferPosition  <  pChannel->MidPoint) )
        {
            // What kind of compression?
            switch( pChannel->Sample.pHotSample->CompressionType )
            {
                // ADPCM?
                case ADPCM:
                    // This is one messed up equation...
                    pChannel->nSamplesBase += ADPCM_BYTES_TO_SAMPLES((STREAM_BUFFER_SIZE*2/36)*36);
                    break;

                default:
                    ASSERT( 0 );
                    break;
            }
        }
    
        // Update previous.
        pChannel->PrevBufferPosition = pChannel->CurrBufferPosition;
    }

    // Release position specified?
    if( pChannel->ReleasePosition )
    {
        // Calculate number of samples played.
        pChannel->PlayPosition = g_AudioHardware.GetSamplesPlayed( pChannel );

        // Past the release position?
        if( pChannel->PlayPosition >= pChannel->ReleasePosition )
        {
            // All bad...
            return FALSE;
        }
    }

    // All good!
    return TRUE;
}

//------------------------------------------------------------------------------

void audio_hardware::Update( void )
{
    // Tell XTL to strut its stuff
    DirectSoundDoWork();

    u32      Dirty;
    channel* pChannel;
    channel* pHead;
    xbool    bCanStart;
	xbool    bQueueStart     = FALSE;
    xbool    bCalculatePitch = FALSE;
    xbool    bIsDolbyDigital = g_AudioMgr.GetSpeakerConfig() == SPEAKERS_DOLBY_DIGITAI_5_1;

    // Ok to do the hardwre update?
    bCanStart = g_AudioHardware.GetDoHardwareUpdate();

    // Clear it.
    g_AudioHardware.ClearDoHardwareUpdate();

    // Loop through active channels (starting with lowest priority).
    pHead    = g_AudioChannelMgr.UsedList();
    pChannel = pHead->Link.pPrev;

    // Get the pitch factor.
    if( g_AudioHardware.GetPitchFactor() != 1.0f )
        bCalculatePitch = TRUE;

    // Run thru them all!
    while( pChannel != pHead )
    {
        // Get the direct sound buffer pointer
        LPDIRECTSOUNDBUFFER8 pdsb = pChannel->Hardware.pdsBuffer;

        // Gonna walk backwards thru the list...
        channel* pPrevChannel = pChannel->Link.pPrev;

        // Check if anything needs updating for the voice.
        Dirty = pChannel->Dirty;
        
        // Force pitch calculation?
        if( bCalculatePitch )
            Dirty |= CHANNEL_DB_PITCH;

        if( Dirty )
        {
			LONG NewVolume[4] = {DSBVOLUME_MIN,DSBVOLUME_MIN,DSBVOLUME_MIN,DSBVOLUME_MIN};

            //  Volume or Pan dirty?
            if( Dirty & (CHANNEL_DB_VOLUME|CHANNEL_DB_PAN) )
            {
                if( pChannel->Volume )
                {
					f32 Volume;
					
                    if( bIsDolbyDigital && pChannel->pElement && pChannel->pElement->pStereoElement )
                    {
                        Volume = pChannel->Volume * pChannel->Pan3d.GetX();
                        if( Volume < 0.01f )
                        {    
                            NewVolume[0] = DSBVOLUME_MIN;
                            NewVolume[3] = DSBVOLUME_MIN;
                        }
                        else
                        {
                            NewVolume[0] = LONG(20.0f * x_log10( Volume ) * 100.0f);
                            NewVolume[3] = NewVolume[0] - 300;
                        }

                        Volume = pChannel->Volume * pChannel->Pan3d.GetY();
                        if( Volume < 0.01f )
                        {
                            NewVolume[1] = DSBVOLUME_MIN;
                            NewVolume[2] = DSBVOLUME_MIN;
                        }
                        else
                        {
                            NewVolume[1] = LONG(20.0f * x_log10( Volume ) * 100.0f);
                            NewVolume[2] = NewVolume[1] - 300;
                        }
                    }
                    else
                    {
					    Volume = pChannel->Volume * pChannel->Pan3d.GetX();
					    if( Volume < 0.01f )
						    NewVolume[0] = DSBVOLUME_MIN;
					    else
						    NewVolume[0] = LONG(20.0f * x_log10( Volume ) * 100.0f);

					    Volume = pChannel->Volume * pChannel->Pan3d.GetY();
					    if( Volume < 0.01f )
						    NewVolume[1] = DSBVOLUME_MIN;
					    else
						    NewVolume[1] = LONG(20.0f * x_log10( Volume ) * 100.0f);

					    Volume = pChannel->Volume * pChannel->Pan3d.GetZ();
					    if( Volume < 0.01f )
						    NewVolume[2] = DSBVOLUME_MIN;
					    else
						    NewVolume[2] = LONG(20.0f * x_log10( Volume ) * 100.0f);

					    Volume = pChannel->Volume * pChannel->Pan3d.GetW();
					    if( Volume < 0.01f )
						    NewVolume[3] = DSBVOLUME_MIN;
					    else
						    NewVolume[3] = LONG(20.0f * x_log10( Volume ) * 100.0f);
                    }
                }

                if( NewVolume[0] < DSBVOLUME_MIN )
					NewVolume[0] = DSBVOLUME_MIN;
				else if( NewVolume[0] > DSBVOLUME_MAX )
					NewVolume[0] = DSBVOLUME_MAX;

				if( NewVolume[1] < DSBVOLUME_MIN )
					NewVolume[1] = DSBVOLUME_MIN;
				else if( NewVolume[1] > DSBVOLUME_MAX )
					NewVolume[1] = DSBVOLUME_MAX;
                
				if( NewVolume[2] < DSBVOLUME_MIN )
					NewVolume[2] = DSBVOLUME_MIN;
				else if( NewVolume[2] > DSBVOLUME_MAX )
					NewVolume[2] = DSBVOLUME_MAX;

				if( NewVolume[3] < DSBVOLUME_MIN )
					NewVolume[3] = DSBVOLUME_MIN;
				else if( NewVolume[3] > DSBVOLUME_MAX )
					NewVolume[3] = DSBVOLUME_MAX;
				// Set the volume.
				pChannel->Hardware.dsmbvp[0].lVolume = NewVolume[0];
				pChannel->Hardware.dsmbvp[1].lVolume = NewVolume[1];
				pChannel->Hardware.dsmbvp[2].lVolume = NewVolume[2];
				pChannel->Hardware.dsmbvp[3].lVolume = NewVolume[3];

				// Set all speaker volumes
				pdsb->SetMixBinVolumes( &pChannel->Hardware.dsmb );

                // Clear dirty bit
                Dirty &= ~(CHANNEL_DB_VOLUME|CHANNEL_DB_PAN);
            }

            // Pitch Dirty?
            if( Dirty & CHANNEL_DB_PITCH )
            {
                if( pChannel->State == STATE_RUNNING )
                {
                    f32 PitchFactor        = g_AudioHardware.GetPitchFactor();
                    hot_sample* pHotSample = pChannel->Sample.pHotSample;
	                s32 Frequency;
                    
                    if( pChannel->pElement && pChannel->pElement->pVoice && pChannel->pElement->pVoice->bPitchLock )
                    {
                        Frequency = (s32)((f32)pHotSample->SampleRate );
                    }
                    else
                    {
                        Frequency = (s32)((f32)pHotSample->SampleRate * pChannel->Pitch * PitchFactor );
                    }

                    // Cap
                    if (Frequency < DSBFREQUENCY_MIN)
                        Frequency = DSBFREQUENCY_MIN ;
                    else
                    if (Frequency > DSBFREQUENCY_MAX)
                        Frequency = DSBFREQUENCY_MAX ;

                    if( pdsb )
                    {

    	                pdsb->SetFrequency(Frequency) ;
                    }
                }

                // Clear dirty bit.
                Dirty &= ~CHANNEL_DB_PITCH;
            }

            if( Dirty & CHANNEL_DB_EFFECTSEND )
            {
                // TODO: Put Effect send in.

                // Clear dirty bit.
                Dirty &= ~CHANNEL_DB_EFFECTSEND;
            }

            // Update the dirty bits
            pChannel->Dirty = Dirty;
        }

        //  Update state machine.
        switch( pChannel->State )
        {
            case STATE_NOT_STARTED:
                break;
            
            case STATE_STARTING:
			{
                xbool bStart = bCanStart;

                if( pChannel->pElement && pChannel->pElement->pVoice && (pChannel->pElement->pVoice->StartQ==2) )
                {
                    bStart      = TRUE;
                    bQueueStart = TRUE;
                }

                if( bStart )
                {
                    StartChannel( pChannel );
                    pChannel->State = STATE_RUNNING;
                }
                break;
			}

            case STATE_RUNNING:
                DWORD CurrentPosition;

                // Get the position in the buffer.
                pdsb->GetCurrentPosition( &CurrentPosition, NULL );
                pChannel->CurrBufferPosition = CurrentPosition;

                // Get the sound buffer and its play status.
                DWORD dwStatus;
                pdsb->GetStatus( &dwStatus );

                // Is it playing?
                if( !UpdatePosition(pChannel) || !(dwStatus & DSBSTATUS_PLAYING) )
                {
                    // Release the channel.
                    g_AudioHardware.ReleaseChannel( pChannel );
                    pChannel->State = STATE_STOPPED;
                }
                break;
            
            case STATE_PAUSING:
                if( bCanStart )
                {
                    g_AudioHardware.PauseChannel( pChannel );
                    pChannel->State = STATE_PAUSED;
                }                
                break;

            case STATE_RESUMING:
                if( bCanStart )
                {
                    g_AudioHardware.ResumeChannel( pChannel );
                    pChannel->State = STATE_RUNNING;
                }
                break;

			case STATE_STOPPED:
			case STATE_PAUSED:
                break;
            
            default:
                // should never get here
                ASSERT(0);
                break;
        }

        // Previous channel...
        pChannel = pPrevChannel;
    }
	
	// Special stuff when we start a queued sound.
    if( bQueueStart )
    {
        // Loop through active channels (starting with lowest priority).
        pHead    = g_AudioChannelMgr.UsedList();
        pChannel = pHead->Link.pPrev;

        while( pChannel != pHead )
        {    
            if( pChannel->pElement && pChannel->pElement->pVoice && (pChannel->pElement->pVoice->StartQ==2) )
                pChannel->pElement->pVoice->StartQ = 0;
            
            // Previous channel...
            pChannel = pChannel->Link.pPrev;
        }
    }
}

//------------------------------------------------------------------------------

void* audio_hardware::AllocAudioRam( s32 nBytes )
{
    nBytes = (nBytes + 31) & ~31;

    #if MAX_AUDIO_RAM
    // On the PS2, we can actually obtain an allocated Addresses of 0, so we make sure we
    // check the amount free first so we can verify allocations will have succeeded.
    u32 nFree = s_AramAllocator.GetFree();
    if ( nFree < u32( nBytes ) )
    {
        x_DebugMsg("AllocARAM: Allocation of size %u failed.\n",nBytes);
        s_AramAllocator.DumpList();
        return 0;
    }
    return s_AramAllocator.Alloc( nBytes );
    #else
    return x_malloc( nBytes );
    #endif
}

//------------------------------------------------------------------------------

void audio_hardware::FreeAudioRam( void* Address )
{
    #if MAX_AUDIO_RAM
    s_AramAllocator.Free( Address );
    #else
    x_free( Address );
    #endif
}

//------------------------------------------------------------------------------

s32 audio_hardware::GetAudioRamFree     ( void )
{
    #if MAX_AUDIO_RAM
    return s_AramAllocator.GetFree();
    #else
    return 0;
    #endif
}
