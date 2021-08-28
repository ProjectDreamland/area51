#include "x_target.hpp"

#if !defined(TARGET_GCN)
#error This is for a GCN target build. Please exclude from build rules.
#endif

#include "audio_channel_mgr.hpp"
#include "audio_hardware.hpp"
#include "audio_package.hpp"
#include "audio_inline.hpp"
#include "audio_stream_mgr.hpp"
#include "e_ScratchMem.hpp"
#include "e_SimpleAllocator.hpp"
#include "e_virtual.hpp"
#include <dolphin.h>
#include <dolphin/mix.h>
#include "audio_mp3_mgr.hpp"
#include "x_log.hpp"
#include "x_bytestream.hpp"

#if defined(rbrannon)
extern voice*   g_DebugVoice;
extern element* g_DebugElement;
extern channel* g_DebugChannel;
extern f32      g_DebugTime;
#endif

//------------------------------------------------------------------------------
// GCN specific defines. 

#define MAX_HARDWARE_CHANNELS   (48)
#define MINIMUM_PRIORITY        (1)
#define MAXIMUM_PRIORITY        (31)
#define DSP_SAMPLE_RATE         (32000)     // DSP running at 32kHz

//------------------------------------------------------------------------------

static xbool        s_IsInitialized         = FALSE;        // Sentinel          
static channel      s_Channels[ MAX_HARDWARE_CHANNELS ];    // Channel buffer

#define CHANNEL_NUMBER( p ) (p-s_Channels)

//------------------------------------------------------------------------------
// GCN Specific variables.

#define MAX_ARQ_REQUESTS         (MAX_AUDIO_STREAMS*2+2)

static          simple_allocator s_AramAllocator;
static          ARQRequest       s_ARQ_Request[MAX_ARQ_REQUESTS];
static          s32              s_ARQ_RequestID = 0;
static volatile s32              s_ARQ_RequestInProcess = 0;
static          u32              s_ARAM_ZeroBase;

//------------------------------------------------------------------------------

audio_hardware g_AudioHardware;

//==============================================================================
// Hardware specific functions.
//==============================================================================

//------------------------------------------------------------------------------
#ifdef TARGET_GCN
extern "C"
{ 
    void WriteARAMAsynch( void* MRAM, u32 ARAM, s32 Length ); 
    void WriteARAMSynchronous( void* MRAM, u32 ARAM, s32 Length );
};
#endif

static void gcn_UpdateCallback( void );
static void gcn_VoiceDropCallback( void *p );

//------------------------------------------------------------------------------

static void gcn_UpdateStreamADPCM( channel* pChannel )
{
    xbool bTransition      = FALSE;
    u32   CurrentPosition  = pChannel->CurrBufferPosition;
    u32   PreviousPosition = pChannel->StreamData.PreviousPosition; 

    // Which buffer are we in?
    if( PreviousPosition <= pChannel->MidPoint )
    {
        // Did a buffer transition occur?
        bTransition = (CurrentPosition > pChannel->MidPoint);
    }
    else
    {
        // Did a buffer transition occur?
        bTransition = (CurrentPosition <= pChannel->MidPoint);
    }

    // Transition occur?
    if( bTransition )
    {
        // Stream done?
        if( pChannel->StreamData.pStream->StreamDone )
        {
            AXVPB*      pVPB       = pChannel->Hardware.pVPB;
            hot_sample* pSample    = pChannel->Sample.pHotSample;
            u32         ARAM       = pSample->AudioRam;
            u32         EndAddress = (((pSample->nSamples-1) / 14) * 16) + ((pSample->nSamples-1) % 14) + 2;

            // Calculate the samples end address.
            EndAddress = EndAddress % ((STREAM_BUFFER_SIZE * 2) << 1);
            EndAddress += (ARAM << 1);

            // Turn looping off.
            g_AudioHardware.Lock();
            AXSetVoiceLoop    ( pVPB, AXPBADDR_LOOP_OFF );
            AXSetVoiceEndAddr ( pVPB, EndAddress );
            AXSetVoiceLoopAddr( pVPB, (s_ARAM_ZeroBase << 1) + 2 );
            g_AudioHardware.Unlock();
        }
        // Need to read from the stream?
        else if( pChannel->StreamData.StreamControl )
        {
            // Fill the read buffer.
            g_AudioStreamMgr.ReadStream( pChannel->StreamData.pStream );
        }
    }

    // Update previous.
    pChannel->StreamData.PreviousPosition = CurrentPosition;
}

//------------------------------------------------------------------------------

static void gcn_UpdataStreamMP3( channel* pChannel )
{
    hot_sample* pSample = pChannel->Sample.pHotSample;

    // Played an entire buffers worth of samples?
    if( (pChannel->StreamData.PreviousPosition >= pChannel->MidPoint) &&  
        (pChannel->CurrBufferPosition  <  pChannel->MidPoint) )
    {
        // Update the number of samples played (pcm adressing is
        // word based sp DONT multiply STREAM_BUFFER_SIZE by 2).
        pChannel->StreamData.nSamplesPlayed += STREAM_BUFFER_SIZE;
    }

    // Update previous.
    pChannel->StreamData.PreviousPosition = pChannel->CurrBufferPosition;

    // Only if its the control
    if( pChannel->StreamData.StreamControl )
    {
        // Get the current position within the sound, position is in WORDS.
        u32   ARAM            = pSample->AudioRam;
        u32   CurrentPosition = (pChannel->CurrBufferPosition * 2) - ARAM;
        u32   Cursor          = pChannel->StreamData.WriteCursor;
        xbool bDoUpdate       = FALSE;

        if( Cursor > 0 )
        {
            bDoUpdate = (Cursor-CurrentPosition) < (1024*sizeof(s16));
        }
        else
        {
            bDoUpdate = (STREAM_BUFFER_SIZE*2 - CurrentPosition) < (1024*sizeof(s16));
        }

        if( bDoUpdate )
        {
            g_AudioHardware.UpdateMP3( pChannel->StreamData.pStream );
        }
    }

    // Less than one buffers worth of samples left?
    if( ((pSample->nSamples-pChannel->StreamData.nSamplesPlayed) < STREAM_BUFFER_SIZE) && pChannel->StreamData.bStopLoop )
    {
        u32    ARAM            = pSample->AudioRam;
        u32    nSamplesRem     = pSample->nSamples % STREAM_BUFFER_SIZE;
        u32    EndAddressBytes = (ARAM + nSamplesRem*2);
        u32    EndAddressWords = (EndAddressBytes / 2) - 1;

        // Is the play position PAST the end of the actual sample?
        if( EndAddressWords < pChannel->CurrBufferPosition )
        {
            // Release the channel. NOTE: Some amount of audio 
            // corruption MIGHT occur when this happens.
            g_AudioHardware.ReleaseChannel( pChannel );
        }
        else
        {
            // Turn looping off.
            g_AudioHardware.Lock();
            AXVPB* pVPB = pChannel->Hardware.pVPB;
            AXSetVoiceLoop    ( pVPB, AXPBADDR_LOOP_OFF );
            AXSetVoiceEndAddr ( pVPB, EndAddressWords );
            AXSetVoiceLoopAddr( pVPB, s_ARAM_ZeroBase / 2 );
            g_AudioHardware.Unlock();
        }

        // Clear flag.
        pChannel->StreamData.bStopLoop = FALSE;
    }
}

//------------------------------------------------------------------------------

void audio_hardware::UpdateStream( channel* pChannel )
{
    // Cold, active, running channel?
    if( pChannel && (pChannel->Type == COLD_SAMPLE) && (pChannel->Hardware.pVPB) && (pChannel->State == STATE_RUNNING) )
    { 
        // What kind of compression?
        switch( pChannel->Sample.pHotSample->CompressionType )
        {
            // ADPCM?
            case ADPCM:
                gcn_UpdateStreamADPCM( pChannel );
                break;

            // MP3?
            case MP3:
                gcn_UpdataStreamMP3( pChannel );
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
    m_TickCount      = 0;
    m_TickTime       = 0.005f; // 5 msecond per audio tick.
}

//------------------------------------------------------------------------------

audio_hardware::~audio_hardware( void )
{
}

//------------------------------------------------------------------------------

void audio_hardware::Init( s32 MemSize )
{
    (void)MemSize;
    ASSERT( s_IsInitialized == FALSE );

    // Start up the GCN systems.
    // BW 12/05 - Removed the ARAlloc/ARFree functionality as we were overruning the end of VM space
    ARInit ( NULL,0);
    ARQInit( );
    AIInit ( NULL );
    AXInit ( );
    MIXInit( );

    // Set up aram allocator.
    s_AramAllocator.Init((void*)ARGetBaseAddress(),(s32)vm_GetSwapSpace()-ARGetBaseAddress());

    // Turn on debugging mode.
    AXSetStepMode( 1 );

    // Make a Zero Buffer.
    #define ZERO_SIZE (1024)
    s_ARAM_ZeroBase = (u32)AllocAudioRam( ZERO_SIZE );
    ASSERT( s_ARAM_ZeroBase == ARGetBaseAddress() );
    smem_StackPushMarker();
    u8* pZero = smem_StackAlloc( ZERO_SIZE );
    x_memset( pZero, 0, ZERO_SIZE );
    WriteARAMSynchronous( pZero, s_ARAM_ZeroBase, ZERO_SIZE );
    smem_StackPopToMarker();

    // Reset audio tick.
    m_TickCount = 0;

    // Register update callback.
    AXRegisterCallback( gcn_UpdateCallback );

    // Start up the MP3 decoder.
    g_AudioMP3Mgr.Init();

    // ok its done.
    s_IsInitialized = TRUE;
}

//------------------------------------------------------------------------------

void audio_hardware::Kill( void )
{
    ASSERT( s_IsInitialized );

    // Nuke AX
    AXQuit();

    // RMB NOTE: I am not shutting down the AR system cause this might screw with the VM system.

    // No longer initilized.
    s_IsInitialized = FALSE;
}

//------------------------------------------------------------------------------

void* audio_hardware::AllocAudioRam( s32 nBytes )
{
    u32 addr = NULL;
    nBytes = (nBytes + 31) & ~31;
    if( nBytes )
    {
        addr = (u32)s_AramAllocator.Alloc( nBytes );
        if (!addr)
        {
            x_DebugMsg("AllocARAM: Allocation of size %d failed.\n",nBytes);
            s_AramAllocator.DumpList();
        }
    }
    return (void*)addr;
}

//------------------------------------------------------------------------------

void audio_hardware::FreeAudioRam( void* Address )
{
    if( Address )
    {
        s_AramAllocator.Free( (void*)Address );
    }
}

//------------------------------------------------------------------------------

s32 audio_hardware::GetAudioRamFree     ( void )
{
    return s_AramAllocator.GetFree();
}

//------------------------------------------------------------------------------

static s16 s_LeftBuffer[MAX_AUDIO_STREAMS][512] GCN_ALIGNMENT(32);
static s16 s_RightBuffer[MAX_AUDIO_STREAMS][512] GCN_ALIGNMENT(32);
static s32 s_WhichBuffer = 0;

void audio_hardware::UpdateMP3( audio_stream* pStream )
{
    u32 ARAM;
    u32 Cursor;

    // Decode 512 samples.
    g_AudioMP3Mgr.Decode( pStream, s_LeftBuffer[ s_WhichBuffer ], s_RightBuffer[ s_WhichBuffer ], 512 );

    // DMA the left channel
    Cursor = pStream->pChannel[0]->StreamData.WriteCursor;
    ARAM   = pStream->Samples[0].Sample.AudioRam + Cursor; 
    WriteARAMAsynch( &s_LeftBuffer[ s_WhichBuffer ], ARAM, 512 * sizeof(s16) );
    Cursor += 512 * sizeof(s16);
    if( Cursor >= STREAM_BUFFER_SIZE*2 )
        Cursor = 0;
    pStream->pChannel[0]->StreamData.WriteCursor = Cursor;

    // Stereo?
    if( pStream->Type == STEREO_STREAM )
    {
        // DMA the right channel
        Cursor = pStream->pChannel[1]->StreamData.WriteCursor;
        ARAM   = pStream->Samples[1].Sample.AudioRam + Cursor;
        WriteARAMAsynch( &s_RightBuffer[ s_WhichBuffer ], ARAM, 512 * sizeof(s16) );
        Cursor += 512 * sizeof(s16);
        if( Cursor >= STREAM_BUFFER_SIZE*2 )
            Cursor = 0;
        pStream->pChannel[1]->StreamData.WriteCursor = Cursor;
    }

    // Switch buffers
     if( ++s_WhichBuffer >= MAX_AUDIO_STREAMS )
        s_WhichBuffer= 0;
}

//------------------------------------------------------------------------------

void audio_hardware::Update( void )
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

// TODO: Inline this function.
xbool audio_hardware::AcquireChannel( channel* pChannel )
{
    // Aquire the hardware voice.
    pChannel->Hardware.pVPB = AXAcquireVoice( pChannel->Hardware.Priority, gcn_VoiceDropCallback, 0 );

    // Tell the world!
    return( pChannel->Hardware.pVPB != NULL );
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::ReleaseChannel( channel* pChannel )
{
    element* pStereo;
    channel* pStereoChannel = NULL;

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

    // Free up the voice.
    if( pChannel->Hardware.pVPB )
        AXFreeVoice( pChannel->Hardware.pVPB );

    // Mark it free.
    pChannel->Hardware.pVPB = NULL;
    pChannel->State         = STATE_NOT_STARTED;
    pChannel->Type          = HOT_SAMPLE;

    // Stereo partner?
    pStereo = pChannel->pElement->pStereoElement;
    if( pStereo )
    {
        // Get the stereo channel.
        pStereoChannel = pStereo->pChannel;

        // Free up the voice.
        if( pStereoChannel && pStereoChannel->Hardware.pVPB )
        {
            // Free it.
            AXFreeVoice( pStereoChannel->Hardware.pVPB );

            // Mark it free.
            pStereoChannel->Hardware.pVPB = NULL;
        }
    }

    // Does this channel have a stream?
    if( pChannel->StreamData.pStream && pChannel->StreamData.StreamControl )
    {
        // If so, nuke the stream
        g_AudioStreamMgr.ReleaseStream( pChannel->StreamData.pStream );
        pChannel->StreamData.pStream = NULL;
    }

    // Does this channel have a stream?
    if( pStereoChannel && pStereoChannel->StreamData.pStream && pStereoChannel->StreamData.StreamControl )
    {
        // If so, nuke the stream
        g_AudioStreamMgr.ReleaseStream( pStereoChannel->StreamData.pStream );
        pStereoChannel->StreamData.pStream = NULL;
    }
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::ClearChannel( channel* pChannel )
{
    // Clear the hardware channel.
    pChannel->Hardware.pVPB = NULL;
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
    return( pChannel->Hardware.pVPB != NULL );
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::StartChannel( channel* pChannel )
{
    // If the hardware channel is active, start it!
    if( pChannel->Hardware.pVPB )
        AXSetVoiceState( pChannel->Hardware.pVPB, AX_PB_STATE_RUN );
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::StopChannel( channel* pChannel )
{
    // If the hardware channel is active, stop it!
    if( pChannel->Hardware.pVPB )
        AXSetVoiceState( pChannel->Hardware.pVPB, AX_PB_STATE_STOP );
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::PauseChannel( channel* pChannel )
{
    // If the hardware channel is active, stop it!
    if( pChannel->Hardware.pVPB )
        AXSetVoiceState( pChannel->Hardware.pVPB, AX_PB_STATE_STOP );
}

//------------------------------------------------------------------------------

// TODO: Inline this function.
void audio_hardware::ResumeChannel( channel* pChannel )
{
    // If the hardware channel is active, stop it!
    if( pChannel->Hardware.pVPB )
        AXSetVoiceState( pChannel->Hardware.pVPB, AX_PB_STATE_RUN );
}

//------------------------------------------------------------------------------

void audio_hardware::InitChannelStreamed( channel* pChannel )
{
    s32                 loopAddress;
    s32                 endAddress;
    s32                 currentAddress;
    s32                 SampleEndAddress;
    u16                 Format;
    hot_sample*         pSample  = pChannel->Sample.pHotSample;
    compression_header* pHeader  = (compression_header*)(pSample+1);
    xbool               IsLooped = (pSample->LoopEnd != 0);
    u32                 ARAM     = pSample->AudioRam;
    AXVPB*              pVPB     = pChannel->Hardware.pVPB;

    ASSERT( IsLooped );

    pChannel->State = STATE_NOT_STARTED;

    // How is the sample compressed?
    switch( pSample->CompressionType )
    {
        // "ADPCM" adressing is nibble-based (the +2 is to skip over the "header" data in the packets).
        case ADPCM:
            SampleEndAddress = (ARAM<<1) + (((pSample->nSamples-1) / 14) * 16) + ((pSample->nSamples-1) % 14) + 2;
            loopAddress      = (ARAM << 1) + 2;
            endAddress       = (ARAM << 1) + (pSample->LoopEnd << 1) - 1;
            currentAddress   = (ARAM << 1) + 2;

            // Special case for sample that fits entirely within the stream buffer.
            if( SampleEndAddress <= endAddress )
            {
                endAddress   = SampleEndAddress;
                loopAddress  = (s_ARAM_ZeroBase << 1) + 2;
                IsLooped     = FALSE;
            }

            // Set format to ADPCM.
            Format = AX_PB_FORMAT_ADPCM;
            break;

        // "16-bit PCM" adressing is word based.
        case MP3:
            SampleEndAddress = ((ARAM + pSample->nSamples*2) / 2) - 1;
            loopAddress      = ARAM / 2;
            endAddress       = ((ARAM + pSample->LoopEnd) / 2) - 1;
            currentAddress   = ARAM / 2;

            // Special case for sample that fits entirely within the stream buffer.
            if( SampleEndAddress <= endAddress )
            {
                endAddress  = SampleEndAddress;
                loopAddress = s_ARAM_ZeroBase / 2;
                IsLooped    = FALSE;
            }

            // Set format to 16-bit PCM.
            Format = AX_PB_FORMAT_PCM16;
            break;

        // Argh...wtf is this?
        default:
            SampleEndAddress = 
            loopAddress      = 
            endAddress       = 
            currentAddress   = 0;
            Format           = 0;
            ASSERT( 0 );
            break;
    }

    // Set the current, previous and midpoint.
    pChannel->StreamData.PreviousPosition =
    pChannel->PrevBufferPosition          = 
    pChannel->CurrBufferPosition          = currentAddress;
    pChannel->MidPoint                    = ((endAddress + currentAddress) >> 1);
    pChannel->StartPosition               = currentAddress;
    pChannel->EndPosition                 = endAddress;

    // Init.
    pChannel->nSamplesAdjust  =
    pChannel->nSamplesBase    =
    pChannel->PlayPosition    =
    pChannel->ReleasePosition = 0;

    // Reset the mp3 write cursor number of samples played.
    pChannel->StreamData.WriteCursor    = 0;
    pChannel->StreamData.nSamplesPlayed = 0;

    // Set addr struct
    AXPBADDR addr;
    addr.loopFlag           = IsLooped ? AXPBADDR_LOOP_ON : AXPBADDR_LOOP_OFF;
    addr.format             = Format;
    addr.loopAddressHi      = loopAddress    >> 16;
    addr.loopAddressLo      = loopAddress    &  0xffff;
    addr.endAddressHi       = endAddress     >> 16;
    addr.endAddressLo       = endAddress     &  0xffff;
    addr.currentAddressHi   = currentAddress >> 16;
    addr.currentAddressLo   = currentAddress &  0xffff;

    // Set src struct
    AXPBSRC src;
    s32 ratio               = (s32)(0x10000 * ((f32)pSample->SampleRate / (f32)DSP_SAMPLE_RATE));
    src.ratioHi             = ratio >> 16;
    src.ratioLo             = ratio & 0xffff;
    src.currentAddressFrac  = 0;
    src.last_samples[0]     = 0;
    src.last_samples[1]     = 0;
    src.last_samples[2]     = 0;
    src.last_samples[3]     = 0;

    // Set ve struct
    AXPBVE ve;
    ve.currentVolume    = (u16)(0x8000 * pChannel->Volume);
    ve.currentDelta     = 0;

    // Update hardware mirror.
    pChannel->Hardware.Ve = ve.currentVolume;

    // Set mix struct
    AXPBMIX mix;
    x_memset( &mix, 0, sizeof(mix) );
    mix.vL          = (u16)(0x8000 * pChannel->Pan3d.X);
    mix.vR          = (u16)(0x8000 * pChannel->Pan3d.Y);

    // Positional sound?
    if( pChannel->pElement->pVoice->IsPositional )
    {
        // TODO: Implement surround sound.
    }

    // Apply settings
    AXSetVoiceType   ( pVPB, AX_PB_TYPE_STREAM );
    AXSetVoiceAddr   ( pVPB, &addr );
    AXSetVoiceSrc    ( pVPB, &src );
    AXSetVoiceSrcType( pVPB, AX_SRC_TYPE_LINEAR );
    AXSetVoiceMix    ( pVPB, &mix );
    AXSetVoiceVe     ( pVPB, &ve );
    if( pSample->CompressionType == ADPCM )
        AXSetVoiceAdpcm( pVPB, (AXPBADPCM*)&pHeader->gcn_ADPCM.Info );
    AXSetVoiceState  ( pVPB, AX_PB_STATE_STOP );
}

//------------------------------------------------------------------------------

void audio_hardware::InitChannel( channel* pChannel )
{
    s32                 loopAddress;
    s32                 endAddress;
    s32                 currentAddress;
    hot_sample*         pSample    = pChannel->Sample.pHotSample;
    compression_header* pHeader    = (compression_header*)(pSample+1);
    xbool               IsLooped   = (pSample->LoopEnd != 0);
    u32                 ARAM       = pSample->AudioRam;
    AXVPB*              pVPB       = pChannel->Hardware.pVPB;

    pChannel->State = STATE_NOT_STARTED;

    // Determine ARAM addressed based on looping or not
    if( IsLooped )
    {
        loopAddress     = (ARAM << 1) + ((pSample->LoopStart / 14) * 16) + (pSample->LoopStart % 14) + 2;
        endAddress      = (ARAM << 1) + ((pSample->LoopEnd   / 14) * 16) + (pSample->LoopEnd   % 14) + 2;
        currentAddress  = (ARAM << 1) + 2;
    }
    else
    {
        loopAddress     = (s_ARAM_ZeroBase << 1) + 2;
        endAddress      = (ARAM << 1) + (((pSample->nSamples-1) / 14) * 16) + ((pSample->nSamples-1) % 14) + 2;
        currentAddress  = (ARAM << 1) + 2;
    }

    // Set the current, previous and midpoint.
    pChannel->StreamData.PreviousPosition =
    pChannel->PrevBufferPosition          = 
    pChannel->CurrBufferPosition          = currentAddress;
    pChannel->MidPoint                    = ((endAddress + currentAddress) >> 1);
    pChannel->StartPosition               = currentAddress;
    pChannel->EndPosition                 = endAddress;

    // Init.
    pChannel->nSamplesAdjust  =
    pChannel->nSamplesBase    =
    pChannel->PlayPosition    =
    pChannel->ReleasePosition = 0;

    // Set addr struct
    AXPBADDR addr;
    addr.loopFlag           = IsLooped ? AXPBADDR_LOOP_ON : AXPBADDR_LOOP_OFF;
    addr.format             = AX_PB_FORMAT_ADPCM;
    //addr.format             = AX_PB_FORMAT_PCM16;
    addr.loopAddressHi      = loopAddress    >> 16;
    addr.loopAddressLo      = loopAddress    &  0xffff;
    addr.endAddressHi       = endAddress     >> 16;
    addr.endAddressLo       = endAddress     &  0xffff;
    addr.currentAddressHi   = currentAddress >> 16;
    addr.currentAddressLo   = currentAddress &  0xffff;

    // Set src struct
    AXPBSRC src;
    s32 ratio               = (s32)(0x10000 * ((f32)pSample->SampleRate / (f32)DSP_SAMPLE_RATE));
    src.ratioHi             = ratio >> 16;
    src.ratioLo             = ratio & 0xffff;
    src.currentAddressFrac  = 0;
    src.last_samples[0]     = 0;
    src.last_samples[1]     = 0;
    src.last_samples[2]     = 0;
    src.last_samples[3]     = 0;

    // Set ve struct
    AXPBVE ve;
    ve.currentVolume    = (u16)(0x8000 * pChannel->Volume);
    ve.currentDelta     = 0;

    // Update the hardware mirror.
    pChannel->Hardware.Ve = ve.currentVolume;

    // Set mix struct
    AXPBMIX mix;
    x_memset( &mix, 0, sizeof(mix) );
    mix.vL          = (u16)(0x8000 * pChannel->Pan3d.X);
    mix.vR          = (u16)(0x8000 * pChannel->Pan3d.Y);

    // Positional sound?
    if( pChannel->pElement->pVoice->IsPositional )
    {
        // TODO: Implement surround sound.
    }

    // Apply settings
    AXSetVoiceType   ( pVPB, AX_PB_TYPE_NORMAL );
    AXSetVoiceAddr   ( pVPB, &addr );
    AXSetVoiceSrc    ( pVPB, &src );
    AXSetVoiceSrcType( pVPB, AX_SRC_TYPE_LINEAR );
    AXSetVoiceMix    ( pVPB, &mix );
    AXSetVoiceVe     ( pVPB, &ve );
    AXSetVoiceAdpcm  ( pVPB, (AXPBADPCM*)&pHeader->gcn_ADPCM.Info );
    if( IsLooped )
        AXSetVoiceAdpcmLoop( pVPB, (AXPBADPCMLOOP*)&pHeader->gcn_ADPCM.Loop );
    AXSetVoiceState( pVPB, AX_PB_STATE_STOP );
}

//------------------------------------------------------------------------------

void audio_hardware::Lock( void )
{
    m_LockMutex.Acquire();
    if( m_InterruptLevel==0 )
    {
        m_InterruptState = OSDisableInterrupts();
    }
    m_InterruptLevel++;
}

//------------------------------------------------------------------------------

void audio_hardware::Unlock( void )
{
    m_InterruptLevel--;
    if( m_InterruptLevel==0 )
    {
        OSRestoreInterrupts( m_InterruptState );
    }
    m_LockMutex.Release();
}

//------------------------------------------------------------------------------

u32 audio_hardware::GetSamplesPlayed( channel* pChannel )
{
    u32 ARAM = pChannel->Sample.pColdSample->AudioRam;

    // What kind of compression?
    switch( pChannel->Sample.pHotSample->CompressionType )
    {
        // ADPCM?
        case ADPCM:
        {
            // ADPCM is a messed up calculation!
            if( pChannel->CurrBufferPosition <= 2 )
            {
                pChannel->nSamplesAdjust = 0;
            }
            else
            {
                u32 n = pChannel->CurrBufferPosition-2;
                pChannel->nSamplesAdjust = ((n/16)*14 + (n%16)) - (ARAM << 1);
            }
            break;
        }

        // MP3?
        case MP3:
            // PCM, so convert from words to samples.
            if( pChannel->CurrBufferPosition >= (ARAM / 2) )
            {
                pChannel->nSamplesAdjust = pChannel->CurrBufferPosition - (ARAM / 2);
            }
            break;

        default:
            ASSERT( 0 );
            break;
    }

    // Tell the world.
    return pChannel->nSamplesBase + pChannel->nSamplesAdjust;
}

//------------------------------------------------------------------------------

static xbool UpdatePosition( channel* pChannel )
{
    xbool Result = TRUE;

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
                    pChannel->nSamplesBase += (STREAM_BUFFER_SIZE*2)*2*14/16;
                    break;

                // MP3?
                case MP3:
                    // Update the number of samples played (PCM is word 
                    // based so DONT multiply STREAM_BUFFER_SIZE by 2).
                    pChannel->nSamplesBase += STREAM_BUFFER_SIZE;
                    break;

                default:
                    ASSERT( 0 );
                    break;
            }
        }
    
        // Update previous.
        pChannel->PrevBufferPosition = pChannel->CurrBufferPosition;
    }

    // Bounds check the damn sound.
    if( pChannel->CurrBufferPosition < pChannel->StartPosition )
        Result = FALSE;
    if( pChannel->CurrBufferPosition > pChannel->EndPosition )
        Result = FALSE;

    // Release position specified?
    if( pChannel->ReleasePosition )
    {
        // Calculate number of samples played.
        pChannel->PlayPosition = g_AudioHardware.GetSamplesPlayed( pChannel );

        // Past the release position?
        if( pChannel->PlayPosition >= pChannel->ReleasePosition )
        {
            Result = FALSE;
        }
    }

    // All good!
    return Result;
}

//------------------------------------------------------------------------------

static void gcn_UpdateCallback( void )
{
    u32         Dirty;
    AXPBVE      ve;
    AXPBMIX     mix;
    channel*    pChannel;
    channel*    pHead;
    s32         Priority = MINIMUM_PRIORITY;
    xbool       SetPriority = FALSE;
    xbool       bCanStart;
    xbool       bQueueStart = FALSE;

    // Ok to do the hardwre update?
    bCanStart = g_AudioHardware.GetDoHardwareUpdate();

    // Clear it.
    g_AudioHardware.ClearDoHardwareUpdate();

    // Need to update the priorities? (only do this synchronized with the audio update).
    if( bCanStart && ((SetPriority = g_AudioHardware.GetDirtyBit( CALLBACK_DB_PRIORITY )) != 0) )
    {
        // Clear the dirty bit.
        g_AudioHardware.ClearDirtyBit( CALLBACK_DB_PRIORITY );

        // Start at lowest priority.
        Priority = MINIMUM_PRIORITY;
    }

    // Initialize ve & mix structures
    x_memset(  &ve, 0, sizeof(ve ) );
    x_memset( &mix, 0, sizeof(mix) );

    // Loop through active channels (starting with lowest priority).
    pHead    = g_AudioChannelMgr.UsedList();
    pChannel = pHead->Link.pPrev;

    while( pChannel != pHead )
    {
        channel* pPrevChannel = pChannel->Link.pPrev;
        AXVPB*   pVPB         = pChannel->Hardware.pVPB;

        // Hardware voice to update?
        if( pVPB )
        {
            // Check if anything needs updating for the voice.
            Dirty = pChannel->Dirty;
            if( Dirty )
            {
                // Volume Dirty?
                if( Dirty & CHANNEL_DB_VOLUME )
                {
                    if( pChannel->State == STATE_RUNNING )
                    {
                        // Calculate delta volume to acheive new volume.
                        s16 OldVe = pChannel->Hardware.Ve;
                        s16 NewVe = (u16)(0x7fff * pChannel->Volume);
                        s16 Delta = (NewVe-OldVe) / AX_IN_SAMPLES_PER_FRAME;

                        // Set volume delta to reach target volume.
                        AXSetVoiceVeDelta( pVPB, Delta );

                        // Update the hardware mirror.
                        pChannel->Hardware.Ve = OldVe + Delta * AX_IN_SAMPLES_PER_FRAME;

                        // Clear dirty bit if volume has reached target.
                        if( Delta == 0 )
                            Dirty &= ~CHANNEL_DB_VOLUME;
                    }
                    else
                    {
                        // Not running, so just set the volume directly.
                        ve.currentVolume      = (u16)(0x7fff * pChannel->Volume);
                        ve.currentDelta       = 0;
                        pChannel->Hardware.Ve = ve.currentVolume;
                        AXSetVoiceVe( pVPB, &ve );

                        // Update the hardware mirror.
                        pChannel->Hardware.Ve = ve.currentVolume;

                        // Clear dirty bit.
                        Dirty &= ~CHANNEL_DB_VOLUME;
                    }
                }

                // Pan Dirty?
                if( Dirty & CHANNEL_DB_PAN )
                {
                    // Calculate pan.
                    mix.vL = (u16)(0x8000 * pChannel->Pan3d.X);
                    mix.vR = (u16)(0x8000 * pChannel->Pan3d.Y);

                    // Set mix into hardware.
                    AXSetVoiceMix( pVPB, &mix );

                    // Clear dirty bit.
                    Dirty &= ~CHANNEL_DB_PAN;
                }

                // Pitch Dirty?
                if( Dirty & CHANNEL_DB_PITCH )
                {
                    // Set pitch into hardware.
                    AXSetVoiceSrcRatio( pVPB, ((f32)pChannel->Sample.pHotSample->SampleRate / (f32)DSP_SAMPLE_RATE) * pChannel->Pitch );

                    // Clear dirty bit.
                    Dirty &= ~CHANNEL_DB_PITCH;
                }

                if( Dirty & CHANNEL_DB_EFFECTSEND )
                {
                    // TODO: Put Effect send in.

                    // Clear dirty bit.
                    Dirty &= ~CHANNEL_DB_EFFECTSEND;
                }

                // Update the dirty bits.
                pChannel->Dirty = Dirty;
            }

            // Update state machine.
            switch( pChannel->State )
            {
                case STATE_NOT_STARTED:
                    // What, no love for me?
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
                        g_AudioHardware.StartChannel( pChannel );
                        pChannel->State = STATE_RUNNING;
                    }
                    break;
                }

                case STATE_RESUMING:
                    if( bCanStart )
                    {
                        g_AudioHardware.ResumeChannel( pChannel );
                        pChannel->State = STATE_RUNNING;
                    }
                    break;

                case STATE_RUNNING:
                    // Update the buffer position & stream current position.
                    pChannel->CurrBufferPosition = *((u32*)&pVPB->pb.addr.currentAddressHi);

                    // If voice stopped playing sound then release hardware channel.
                    if( !UpdatePosition( pChannel ) || (pVPB->pb.state == AX_PB_STATE_STOP) )
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
                
                case STATE_PAUSED:
                case STATE_STOPPED:
                    // Don't do a damned thing...
                    break;
            }

            // Set the hardware priorities?
            if( SetPriority )
            {
                // Set the channels hardware priority.
                pChannel->Hardware.Priority = Priority;

                // Hardware voice active?
                if( pChannel->Hardware.pVPB )
                {
                    // Set the voices priority.
                    AXSetVoicePriority( pChannel->Hardware.pVPB, pChannel->Hardware.Priority );

                    // Bump priority (but stay in range).
                    if( ++Priority > MAXIMUM_PRIORITY )
                        Priority = MAXIMUM_PRIORITY;
                }
            }
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

static void gcn_VoiceDropCallback( void *p )
{
    channel* pChannel = s_Channels;
    AXVPB*      pVPB;
  
    // Coerce.
    pVPB = (AXVPB*)p;
    
    for( s32 i=0 ; i<MAX_HARDWARE_CHANNELS ; i++ )
    {
        // Check voice.
        if( pChannel->Hardware.pVPB == pVPB )
        {
            // Nuke it.
            pChannel->Hardware.pVPB = NULL;
            return;
        }

        // walk array.
        pChannel++;
    }
}

//------------------------------------------------------------------------------

static void gcn_ARQCallback( u32 p )
{
    ARQRequest* pRequest = (ARQRequest*)p;
    (void)pRequest;
    ASSERT( s_ARQ_RequestInProcess );
    s_ARQ_RequestInProcess--;
}

//------------------------------------------------------------------------------

void WriteARAMSynchronous( void* MRAM, u32 ARAM, s32 Length )
{
    ASSERT( (Length & 31) == 0 );
    ASSERT( ((s32)MRAM & 3) == 0 );

    DCStoreRange( MRAM, Length );
    s_ARQ_RequestInProcess++;
    ARQPostRequest( &s_ARQ_Request[s_ARQ_RequestID], 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH, (u32)MRAM, ARAM, Length, gcn_ARQCallback );
    if( s_ARQ_RequestID >= MAX_ARQ_REQUESTS )
        s_ARQ_RequestID = 0;

    // Wait for transfer to complete
    while( s_ARQ_RequestInProcess )
    {
        // This loop intentionally left blank
    }
}

/*
//------------------------------------------------------------------------------

volatile s32 s_Pending    = 0;
volatile s32 s_PendingMax = 0;

static void Tracker( u32 p )
{
    (void)p;
    s_Pending--;
}

void WriteARAMAsynch( void* MRAM, u32 ARAM, s32 Length )
{
    ASSERT( (Length & 31) == 0 );
    ASSERT( ((s32)MRAM & 3) == 0 );

    s32 n = ++s_Pending;

    if( n > s_PendingMax )
    {
        s_PendingMax = n;
    }

    DCStoreRange( MRAM, Length );
    ARQPostRequest( &s_ARQ_Request[s_ARQ_RequestID], 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH, (u32)MRAM, ARAM, Length, Tracker );
    s_ARQ_RequestID++;
    if( s_ARQ_RequestID >= MAX_ARQ_REQUESTS )
        s_ARQ_RequestID = 0;
}
*/

//------------------------------------------------------------------------------

void WriteARAMAsynch( void* MRAM, u32 ARAM, s32 Length )
{
    ASSERT( (Length & 31) == 0 );
    ASSERT( ((s32)MRAM & 3) == 0 );

    DCStoreRange( MRAM, Length );
    ARQPostRequest( &s_ARQ_Request[s_ARQ_RequestID], 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH, (u32)MRAM, ARAM, Length, NULL );
    s_ARQ_RequestID++;
    if( s_ARQ_RequestID >= MAX_ARQ_REQUESTS )
        s_ARQ_RequestID = 0;
}

