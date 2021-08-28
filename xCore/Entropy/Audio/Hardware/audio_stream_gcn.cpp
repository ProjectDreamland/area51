#include "x_target.hpp"

#if !defined(TARGET_GCN)
#error This is for a GCN target build. Please exclude from build rules.
#endif

#include "audio_hardware.hpp"
#include "audio_stream_mgr.hpp"
#include "e_Virtual.hpp"

//------------------------------------------------------------------------------

#define DISABLE_ADPCM_STREAMING

//------------------------------------------------------------------------------

extern "C" void WriteARAMAsynch( void* MRAM, u32 ARAM, s32 Length );

//------------------------------------------------------------------------------

void audio_stream_read_callback( io_request* pRequest, audio_stream* pStream, s32 ReadBufferIndex )
{
    (void)pRequest;

    // Make sure the status is completed!
    if( pRequest->GetStatus() != io_request::COMPLETED )
    {
        // Release the channel (eventually will release the voice)
        if( pStream->pChannel[0] )
            g_AudioHardware.ReleaseChannel( pStream->pChannel[0] );
    }
    else
    {

#ifdef DISABLE_ADPCM_STREAMING
        ASSERT( pStream->CompressionType != ADPCM );
#endif

        switch( pStream->Type )
        {
            case MONO_STREAM: 
            {
                // Write single channel (mono, uses left channel)
                switch( pStream->CompressionType )
                {
                    case ADPCM:
                        ASSERT( pRequest->GetLength() == STREAM_BUFFER_SIZE );

                        // Write the mono channel
                        WriteARAMAsynch( (void*)g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex], 
                                         pStream->ARAM[LEFT_CHANNEL][pStream->ARAMWriteBuffer], 
                                         STREAM_BUFFER_SIZE );
                        break;

                    case MP3:
                        ASSERT( pRequest->GetLength() == MP3_BUFFER_SIZE );

                        // Write the mono channel
                        x_memcpy( (void*)pStream->MainRAM[pStream->ARAMWriteBuffer], 
                                  (void*)g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex],
                                  MP3_BUFFER_SIZE );
                        break;

                    default: 
                        ASSERT( 0 );
                        break;
                }
            }

            case STEREO_STREAM:
            {
                switch( pStream->CompressionType )
                {
                    case ADPCM:
                        ASSERT( pRequest->GetLength() == STREAM_BUFFER_SIZE*2 );
                
                        // Write the left channel
                        WriteARAMAsynch( (void*)g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex], 
                                         pStream->ARAM[LEFT_CHANNEL][pStream->ARAMWriteBuffer], 
                                         STREAM_BUFFER_SIZE );

                        // Write the right channel
                        WriteARAMAsynch( (void*)(g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex]+STREAM_BUFFER_SIZE), 
                                         pStream->ARAM[RIGHT_CHANNEL][pStream->ARAMWriteBuffer], 
                                         STREAM_BUFFER_SIZE );
                        break;

                    case MP3:
                        ASSERT( pRequest->GetLength() == MP3_BUFFER_SIZE );
                
                        // Write the stereo channel (data is a stereo mp3 stream)
                        x_memcpy( (void*)pStream->MainRAM[pStream->ARAMWriteBuffer], 
                                  (void*)g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex],
                                  MP3_BUFFER_SIZE );
                        break;

                    default:
                        ASSERT( 0 );
                        break;
                }
                break;
            }

            case INACTIVE:
            {
                break;
            }

            default:
            {
                ASSERT( 0 );
                break;
            }
        }
    }

    // Toggle the write buffer.
    pStream->ARAMWriteBuffer ^= 1;
}

//------------------------------------------------------------------------------

void audio_stream_mgr::Init( void )
{
    s32 nBytes;
    u32 BaseRam;
    s32 i;

    // Nuke 'em.
    x_memset( m_AudioStreams, 0, sizeof( m_AudioStreams ) );

    // Save the index.
    for( i=0 ; i<MAX_AUDIO_STREAMS ; i++ )
    {
        m_AudioStreams[i].Index = i;
    }

    // Allocate the main ram buffers for mp3 de-compression.
    nBytes = MAX_AUDIO_STREAMS * MP3_BUFFER_SIZE * 2; 
    m_MainRam = (u32)vm_Alloc( nBytes );

    // Assign mp3 buffers to the stream.
    BaseRam = m_MainRam;
    for( i=0 ; i<MAX_AUDIO_STREAMS ; i++ )
    {
        m_AudioStreams[ i ].MainRAM[ 0 ] = BaseRam;
        BaseRam += MP3_BUFFER_SIZE;
        m_AudioStreams[ i ].MainRAM[ 1 ] = BaseRam;
        BaseRam += MP3_BUFFER_SIZE;
    }

    // Allocate the audio ram.
    nBytes = MAX_AUDIO_STREAMS * MAX_STREAM_CHANNELS * STREAM_BUFFER_SIZE * 2; 
    m_ARAM = (u32)g_AudioHardware.AllocAudioRam( nBytes );

    // Determine read buffer size.
#ifdef DISABLE_ADPCM_STREAMING
    nBytes = MP3_BUFFER_SIZE;
#else
    nBytes = STREAM_BUFFER_SIZE * MAX_STREAM_CHANNELS;
#endif

    // Allocate the read buffers
    m_ReadBuffers[0] = (u32)x_malloc( nBytes );
    m_ReadBuffers[1] = (u32)x_malloc( nBytes );

    // Assign aram to the stream buffers
    BaseRam = m_ARAM;
    for( i=0 ; i<MAX_AUDIO_STREAMS ; i++ )
    {
        // Stream has an io_request for a member.
        m_AudioStreams[ i ].pIoRequest = new io_request[1];

        // Asign ARAM to each stream channel.
        for( s32 j=0 ; j<MAX_STREAM_CHANNELS ; j++ )
        {
            // Asign both buffers.
            m_AudioStreams[ i ].ARAM[ j ][ 0 ] = BaseRam;
            BaseRam += STREAM_BUFFER_SIZE;
            m_AudioStreams[ i ].ARAM[ j ][ 1 ] = BaseRam;
            BaseRam += STREAM_BUFFER_SIZE;
        }
    }
}

//------------------------------------------------------------------------------

void audio_stream_mgr::Kill( void )
{
    // For each stream...
    for( s32 i=0 ; i<MAX_AUDIO_STREAMS ; i++ )
    {
        // Delete the streams io_request.
        delete [] m_AudioStreams[ i ].pIoRequest;
    }

    // Free up the aram buffers.
    g_AudioHardware.FreeAudioRam( (void*)m_ARAM );

    // Free up the mp3 decompression main ram buffers.
    vm_Free( (void*)m_MainRam );

    // Free the read buffers.
    x_free( (void*)m_ReadBuffers[0] );
    x_free( (void*)m_ReadBuffers[1] );

    // Nuke it!
    x_memset( m_AudioStreams, 0, MAX_AUDIO_STREAMS*sizeof(audio_stream) );
}

//------------------------------------------------------------------------------

void audio_stream_mgr::SetRequest( audio_stream* pStream, io_request::callback_fn* pCallback )
{
        // Set the request.
        pStream->pIoRequest->SetRequest( pStream->FileHandle, 
                                         (void*)m_ReadBuffers[m_ActiveReadBuffer], 
                                         pStream->FileHandle->Offset+pStream->WaveformOffset+pStream->WaveformCursor,
                                         pStream->ReadBufferSize,
                                         io_request::HIGH_PRIORITY,
                                         FALSE,
                                         0,
                                         0,
                                         pCallback );
}
