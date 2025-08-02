#include "audio_stream_mgr.hpp"
#include "audio_channel_mgr.hpp"
#include "audio_hardware.hpp"
#include "audio_mp3_mgr.hpp"
#include "codecs\mp3api.hpp"
#include "x_bytestream.hpp"
#include "x_log.hpp"

//------------------------------------------------------------------------------

#define VALID_STREAM( pStream ) ((pStream >= &g_AudioStreamMgr.m_AudioStreams[0]) && (pStream <= &g_AudioStreamMgr.m_AudioStreams[MAX_AUDIO_STREAMS-1]))

//------------------------------------------------------------------------------

static xbool s_Initialized = FALSE;

//------------------------------------------------------------------------------

audio_mp3_mgr g_AudioMP3Mgr;

//------------------------------------------------------------------------------

void* AIL_mem_alloc_lock( u32 nBytes )
{
    return x_malloc( nBytes );
}

//------------------------------------------------------------------------------

void AIL_mem_free_lock( void* pBuffer )
{
    x_free( pBuffer );
}

//------------------------------------------------------------------------------

S32 mp3_fetch_data( U32 UserData, void* pBuffer, S32 nBytes, S32 Offset )
{
    (void)Offset;

    ASSERT( (Offset == 0) || (Offset == -1) );

    audio_stream* pStream     = (audio_stream*)UserData;
    if( Offset == 0 )
    {
        ASSERT( pStream->CursorMP3 < MP3_BUFFER_SIZE );
        pStream->CursorMP3 = 0;
    }

    s32           Previous    = pStream->CursorMP3;
    s32           Current     = Previous + (s32)nBytes;
    xbool         bTransition = FALSE;

    // Need to wrap to front?
    if( Current > (MP3_BUFFER_SIZE*2) )
    {    
        ASSERT( Previous <= (MP3_BUFFER_SIZE*2) );

        // Copy the data from the cursor to the end of the buffer.
        s32 Length = (MP3_BUFFER_SIZE*2) - Previous;
        if( Length )
        {
            x_memcpy( pBuffer, (void*)(pStream->MainRAM[0] + Previous), Length );
			x_memset( (void*)(pStream->MainRAM[0] + Previous), 0, Length );
            pBuffer = (void*)((s32)pBuffer + Length);
        }

        // Now wrap it and copy data from start of buffer to cursor.
        Current -= (MP3_BUFFER_SIZE*2);
        x_memcpy( pBuffer, (void*)pStream->MainRAM[0], Current );
		x_memset( (void*)pStream->MainRAM[0], 0, Current );
    }
    // Did not wrap, so just copy the data.
    else
    {
        // Copy it.
        x_memcpy( pBuffer, (void*)(pStream->MainRAM[0] + Previous), nBytes );
		x_memset( (void*)(pStream->MainRAM[0] + Previous), 0, nBytes );
    }

    // Update the mp3 buffer cursor.
    pStream->CursorMP3 = Current;

    // Determine if a transition has ocured.
    // Which buffer are we in?
    if( Previous <= MP3_BUFFER_SIZE )
    {
        // Did a buffer transition occur?
        bTransition = (Current > MP3_BUFFER_SIZE );
    }
    else
    {
        // Did a buffer transition occur?
        bTransition = (Current <= MP3_BUFFER_SIZE );
    }

    // Transition occur?
    if( bTransition )
    {
        // Stream done?
        if( !pStream->StreamDone )
        {
            // Fill the read buffer.
            g_AudioStreamMgr.ReadStream( pStream );
        }
    }

    return nBytes;
}

//------------------------------------------------------------------------------

audio_mp3_mgr::audio_mp3_mgr( void )
{
}

//------------------------------------------------------------------------------

audio_mp3_mgr::~audio_mp3_mgr( void )
{
}

//------------------------------------------------------------------------------

void audio_mp3_mgr::Init( void )
{
    ASSERT( s_Initialized == FALSE );
    ASI_startup();
    s_Initialized = TRUE;
}

//------------------------------------------------------------------------------

void audio_mp3_mgr::Kill( void )
{
    ASSERT( s_Initialized );
    ASI_shutdown();
    s_Initialized = FALSE;
}

//------------------------------------------------------------------------------

void audio_mp3_mgr::Open( audio_stream* pStream )
{
    ASSERT( s_Initialized );
    ASSERT( VALID_STREAM(pStream) );
    pStream->CursorMP3 = 0;
    pStream->HandleMP3 = (void*)ASI_stream_open( (U32)pStream, mp3_fetch_data, pStream->Samples[0].Sample.WaveformLength );
}

//------------------------------------------------------------------------------

void audio_mp3_mgr::Close( audio_stream* pStream )
{
    ASSERT( s_Initialized );
    ASSERT( VALID_STREAM(pStream) );

    if( pStream->HandleMP3 )
        ASI_stream_close( (s32)pStream->HandleMP3 );
    pStream->HandleMP3 = NULL;
}

//------------------------------------------------------------------------------

static s16 s_DecodeBuffer[1024];

void audio_mp3_mgr::Decode( audio_stream* pStream, s16* pBufferL, s16* pBufferR, s32 nSamples )
{
    ASSERT( s_Initialized );
    ASSERT( VALID_STREAM(pStream) );
    ASSERT( nSamples >= 0 );
    ASSERT( nSamples <= 512 );

    if( (nSamples <= 0) || (nSamples > 512) )
        nSamples = 512;

    // MP3 Stream closed?
    if( (pStream==NULL) || (pStream->HandleMP3 == NULL) )
    {   
        if( pBufferL )
            x_memset( pBufferL, 0, nSamples * sizeof(s16) );
        if( pBufferR )
            x_memset( pBufferR, 0, nSamples * sizeof(s16) );
    }
    else
    {
        // Lock the audio hardware.
        g_AudioHardware.Lock();

        // Is it stereo?
        xbool bIsStereo = (pStream->Type == STEREO_STREAM);
        s16*  pDest;
        s32   nBytes;

        // Stereo?
        if( bIsStereo )
        {
            // Stick in buffer so we can "un-interleave" it.
            pDest  = s_DecodeBuffer;
            nBytes = nSamples * 4;
        }
        else
        {
            ASSERT( pBufferL );
            // Decode directly to the buffer.
            pDest  = pBufferL;
            nBytes = nSamples * 2;
        }

        // Decode it.
        ASI_stream_process( (s32)pStream->HandleMP3, pDest, nBytes );

        // Need to "un-interleave"?
        if( bIsStereo )
        {
            ASSERT( pBufferL );
            ASSERT( pBufferR );

            s16* pSrc = s_DecodeBuffer;

            while( nSamples-- )
            {
                *pBufferL++ = *pSrc++;
                *pBufferR++ = *pSrc++;
            }
        }

        // Unlock it now.
        g_AudioHardware.Unlock();
    }
}

 