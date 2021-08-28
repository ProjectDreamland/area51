//==============================================================================
//
//  Headset_PS2.cpp
//
//==============================================================================
#include "x_types.hpp"
#include "Headset.hpp"
#if !defined(TARGET_PS2)
#error This is for the PS2 platform only. Please exclude this file from your build rules.
#endif

//==============================================================================
//  INCLUDES
//==============================================================================
#include "Network/NetStream.hpp"
#include "ps2/IopManager.hpp"
#include "NetworkMgr.hpp"
#include "audio/hardware/audio_hardware_ps2_private.hpp"
#include "Logitech/Include/liblgaud.h"

//#define USE_SPEEX

#if defined(USE_SPEEX)
#include "Speex.hpp"
#else
#include "lpc10.hpp"
#endif


//******************************************************************************
// Each encoded frame is 7 bytes in length. This requires 360 bytes (or 180 samples)
// of source data to encode to 7 bytes. 180 samples covers a period of 22.5ms at 8khz.
// The interval at which the decoder will be called is 50ms, so we need at least 3
// blocks to be encoded or decoded every cycle.
//
// It takes 2.8ms to encode 4 blocks (covers a period of 90ms).
// 1.85ms to decode 4 blocks.

#define FRAMES_ENCODED_PER_READ 4
#define LGAUD_IRX_PARAMS            "maxstream=2048"
#define USB_IRX_PARAMS              "dev=11""\0""ed=24""\0""gtd=48""\0""ioreq=96""\0""hub=1"
#define BLOCKS_IN_SPEAKER_STREAM    16
//#define USB_IRX_PARAMS              ""

//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
static void* s_Alloc( u32 Length )
{
    return x_malloc( Length );
}

//==============================================================================
static void s_Free( void* Ptr )
{
    x_free( Ptr );
}


static void s_HeadsetReader( s32 argc, char** argv )
{
    headset* pClass;
    xtimer   dt;

    (void)argc;

    pClass = (headset*)argv;
    ASSERT(argc==1);
    
    dt.Start();
    while( x_GetCurrentThread()->IsActive() )
    {
        pClass->PeriodicUpdate(dt.TripSec());
        x_DelayThread(20);
    }
}

//==============================================================================
void headset::Init( xbool EnableHardware )
{
    s32     Status;
    byte*   ptr;

    if( EnableHardware )
    {
#if defined(USE_SPEEX)
        m_EncodeBlockSize = SPEEX8_BYTES_PER_EFRAME * FRAMES_ENCODED_PER_READ;
        m_DecodeBlockSize = SPEEX8_SAMPLES_PER_FRAME * sizeof(s16) * FRAMES_ENCODED_PER_READ;
#else
        m_EncodeBlockSize = LPC10_BYTES_PER_EFRAME * FRAMES_ENCODED_PER_READ;
        m_DecodeBlockSize = LPC10_SAMPLES_PER_FRAME * sizeof(s16) * FRAMES_ENCODED_PER_READ;
#endif
    }
    else
    {
        m_EncodeBlockSize = 0;
        m_DecodeBlockSize = 0;
    }
    m_DeviceHandle          = LGAUD_INVALID_DEVICE;
    ptr                     = new byte[m_EncodeBlockSize + m_DecodeBlockSize + 256 * 2];
    m_pDecodeBuffer         = ptr;          ptr += m_DecodeBlockSize;
    m_pEncodeBuffer         = ptr;          ptr += m_EncodeBlockSize;
    m_ReadFifo.Init(  ptr, 256 );           ptr += 256;
    m_WriteFifo.Init( ptr, 256 );           ptr += 256;
    m_HeadsetCount          = 0;
    m_HardwareEnabled       = EnableHardware;
    m_IsTalking             = FALSE;
    m_VoiceThroughSpeaker   = FALSE;
    m_HeadsetMask           = -1;               // Used to mask out other controllers, if needed.

    if( m_HardwareEnabled )
    {
        //m_UsbIrxHandle    = g_IopManager.LoadModule("usbd.irx",USB_IRX_PARAMS,sizeof(USB_IRX_PARAMS));
        m_LgAudIrxHandle  = g_IopManager.LoadModule("lgaud.irx",LGAUD_IRX_PARAMS,sizeof(LGAUD_IRX_PARAMS) );

        Status = lgAudInit(s_Alloc,s_Free);
        ASSERT( Status == LGAUD_SUCCESS );
#if defined(USE_SPEEX)
        SpeexInit();
#else
        LPC10Init();
#endif

        m_pThread         = new xthread(s_HeadsetReader,"Headset Reader", 8192, 2, 1, (char**)this);
    }
}

//==============================================================================
void headset::Kill( void )
{
    if( m_HardwareEnabled )
    {
        delete m_pThread;
        m_pThread = NULL;
        if( m_DeviceHandle != LGAUD_INVALID_DEVICE )
        {
            lgAudClose( m_DeviceHandle );
            m_DeviceHandle = LGAUD_INVALID_DEVICE;
        }
#if defined(USE_SPEEX)
        SpeexKill();
#else
        LPC10Kill();
#endif

        g_IopManager.UnloadModule( m_LgAudIrxHandle );
        //g_IopManager.UnloadModule( m_UsbIrxHandle );
    }
    m_ReadFifo.Kill();
    m_WriteFifo.Kill();
    delete[] m_pDecodeBuffer;
    m_pEncodeBuffer = NULL;
    m_pDecodeBuffer = NULL;
}

//==============================================================================
const char* s_State;
//==============================================================================
void headset::PeriodicUpdate( f32 DeltaTime )
{
    s32             Index;
    s32             Hint = LGAUD_HINT_IKNOWNUTTIN;
    s32             Status;
    s32             Length;
    s32             Size;

    lgAudDeviceDesc Descriptor;

    (void)DeltaTime;
    lgAudEnumHint( &Hint );

    if ( Hint == LGAUD_HINT_ENUMNEEDED )
    {
        if ( m_DeviceHandle != LGAUD_INVALID_DEVICE )
        {
            LOG_WARNING( "headset::Update","Closing headset device, handle %d",m_DeviceHandle );
            lgAudClose( m_DeviceHandle );
            m_DeviceHandle = LGAUD_INVALID_DEVICE;
        }

        Index = 0;

        while (1)
        {
            Status = lgAudEnumerate( Index, &Descriptor );
            if (Status != LGAUD_SUCCESS)
            {
                break;
            }
            Index++;
        }
        m_HeadsetCount = Index;

        LOG_MESSAGE( "headset::Update", "Detected %d headset device(s)", m_HeadsetCount );
    }

    // Any headsets present?
    if( m_HeadsetCount )
    {
        if( m_DeviceHandle == LGAUD_INVALID_DEVICE )
        {
            lgAudOpenParam  OpenParam;
            OpenParam.Mode                               = LGAUD_MODE_PLAYRECORD;
            OpenParam.PlaybackFormat.Channels            = 1;
            OpenParam.PlaybackFormat.BitResolution       = 16;
            OpenParam.PlaybackFormat.SamplingRate        = VOICE_SAMPLE_RATE;
            OpenParam.PlaybackFormat.BufferMilliseconds  = 200;
            OpenParam.RecordingFormat.Channels           = 1;
            OpenParam.RecordingFormat.BitResolution      = 16;
            OpenParam.RecordingFormat.SamplingRate       = VOICE_SAMPLE_RATE;
            OpenParam.RecordingFormat.BufferMilliseconds = 100;

            Status = lgAudOpen( 0, &OpenParam, &m_DeviceHandle );
            if( Status == LGAUD_SUCCESS )
            {
		        lgAudSetPlaybackVolume( m_DeviceHandle, LGAUD_CH_BOTH, 75 );
                lgAudStartPlayback( m_DeviceHandle );

                lgAudSetRecordingVolume( m_DeviceHandle, LGAUD_CH_BOTH, 75 );
                lgAudStartRecording( m_DeviceHandle );


                LOG_MESSAGE( "headset::Update", "Successfully opened headset device. Handle %d", m_DeviceHandle );
            }
            else
            {
                m_DeviceHandle = LGAUD_INVALID_DEVICE;
                
                LOG_ERROR( "headset::Update", "Failed to open headset device, reason %d.", Status );
            }
        }

        if( m_IsTalking )
        {
            s_State = "Reading from headset";
            Length = m_DecodeBlockSize;
            Status = lgAudRead( m_DeviceHandle, LGAUD_BLOCKMODE_BLOCKING, m_pDecodeBuffer, &Length );
            s_State = "Idle";

            if( (Status == LGAUD_SUCCESS) && (Length == m_DecodeBlockSize) )
            {
                xtimer t;

                Size = m_EncodeBlockSize;
                t.Start();
                s_State = "Encoding";

#if defined(USE_SPEEX)
                SpeexEncode( (s16*)m_pDecodeBuffer, m_DecodeBlockSize, m_pEncodeBuffer, &Size );
#else
                LPC10Encode( (s16*)m_pDecodeBuffer, m_DecodeBlockSize, m_pEncodeBuffer, &Size );
#endif
                t.Stop();
                if( Size % m_EncodeBlockSize )
                {
                    Size -= (Size % m_EncodeBlockSize);
                }
                m_ReadFifo.Insert( m_pEncodeBuffer, Size, m_EncodeBlockSize );
                LOG_MESSAGE("headset::PeriodicUpdate","Encoded %d bytes to %d bytes in %2.02fms.", m_DecodeBlockSize, Size, t.ReadMs());
            }
        }
        else
        {
            // Inject blank data if not talking.
            x_memset( m_pEncodeBuffer, 0, m_EncodeBlockSize );
            m_ReadFifo.Insert( m_pEncodeBuffer, m_EncodeBlockSize, m_EncodeBlockSize );
        }
    }

    Status = m_WriteFifo.Remove( m_pEncodeBuffer, m_EncodeBlockSize, m_EncodeBlockSize );
    if( Status )
    {
        xtimer t;

        Size = m_DecodeBlockSize;
        t.Start();
        s_State = "Decoding";
#if defined(USE_SPEEX)
        SpeexDecode( m_pEncodeBuffer, m_EncodeBlockSize, (s16*)m_pDecodeBuffer, &Size );
#else
        LPC10Decode( m_pEncodeBuffer, m_EncodeBlockSize, (s16*)m_pDecodeBuffer, &Size );
#endif
        t.Stop();
        s_State = "Writing";
        if( m_HeadsetCount )
        {
            lgAudWrite( m_DeviceHandle, LGAUD_BLOCKMODE_BLOCKING, m_pDecodeBuffer, &Size );
        }
        s_State = "Idle";
        LOG_MESSAGE("headset::PeriodicUpdate","Decoded %d bytes to %d bytes in %2.02fms.", m_EncodeBlockSize, Size, t.ReadMs() );
        if( m_VoiceThroughSpeaker )
        {
            s_ChannelManager.PushPCMData( m_pDecodeBuffer, Size, 8000 );
        }
    }
}

//==============================================================================
void headset::Update( f32 DeltaTime )
{
    (void)DeltaTime;

    UpdateLoopBack();
    (void)DeltaTime;
}

void headset::OnHeadsetInsert( void )
{
}

void headset::OnHeadsetRemove( void )
{
}

void headset::SetLoopback( xbool IsEnabled )
{
    m_IsTalking = IsEnabled;
    m_LoopbackEnabled = IsEnabled;
}

void headset::SetSpeakerVolume( f32 SpeakerVolume )
{
    s_ChannelManager.SetPCMVolume( SpeakerVolume );
}