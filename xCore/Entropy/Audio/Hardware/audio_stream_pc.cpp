#include "x_target.hpp"

#if !defined(TARGET_PC)
#error This is for a PC target build. Please exclude from build rules.
#endif

#include "audio_hardware.hpp"
#include "audio_stream_mgr.hpp"
#include "e_Virtual.hpp"

//------------------------------------------------------------------------------

void audio_stream_read_callback( io_request* pRequest, audio_stream* pStream, s32 ReadBufferIndex )
{
    ASSERT( pRequest->GetStatus() == io_request::COMPLETED );
    (void) pRequest; // SKS: Prevent compiler from complaining that variable is not used (release builds)

    switch( pStream->Type )
    {
        case MONO_STREAM: 
        {
			switch( pStream->CompressionType )
			{
				case ADPCM:
					x_memcpy( (void*)pStream->ARAM[LEFT_CHANNEL][pStream->ARAMWriteBuffer], 
							(void*)g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex],
							STREAM_BUFFER_SIZE );
					break;

				case MP3:
					ASSERT( pRequest->GetLength() == MP3_BUFFER_SIZE );

					x_memcpy( (void*)pStream->MainRAM[pStream->ARAMWriteBuffer],
						      (void*)g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex],
							  MP3_BUFFER_SIZE );
					break;
			}
            break;
        }

        case STEREO_STREAM:
        {
			switch( pStream->CompressionType )
			{
				case ADPCM:
					x_memcpy( (void*)pStream->ARAM[LEFT_CHANNEL][pStream->ARAMWriteBuffer], 
							(void*)g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex],
							STREAM_BUFFER_SIZE );

					x_memcpy( (void*)pStream->ARAM[RIGHT_CHANNEL][pStream->ARAMWriteBuffer], 
							(void*)(g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex]+STREAM_BUFFER_SIZE),
							STREAM_BUFFER_SIZE );
					break;

				case MP3:
					ASSERT( pRequest->GetLength() == MP3_BUFFER_SIZE );

					x_memcpy( (void*)pStream->MainRAM[pStream->ARAMWriteBuffer],
						      (void*)g_AudioStreamMgr.m_ReadBuffers[ReadBufferIndex],
							  MP3_BUFFER_SIZE );
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
    // New write buffer.
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

	// Allocate the main ram buffers for mp3 de-compression
	nBytes = MAX_AUDIO_STREAMS * MP3_BUFFER_SIZE * 2;
	m_MainRam = (u32)x_malloc( nBytes );

	// Assign mp3 buffers to the stream
	BaseRam = m_MainRam;
	for( i=0 ; i<MAX_AUDIO_STREAMS ; i++ )
	{
		m_AudioStreams[i].MainRAM[0] = BaseRam;
		BaseRam += MP3_BUFFER_SIZE;
		m_AudioStreams[i].MainRAM[1] = BaseRam;
		BaseRam += MP3_BUFFER_SIZE;
	}

    // Allocate the aram
    nBytes = MAX_AUDIO_STREAMS * MAX_STREAM_CHANNELS * STREAM_BUFFER_SIZE * 2; 
    m_ARAM = (u32)g_AudioHardware.AllocAudioRam( nBytes );
    
	// MP3 streaming only!
    nBytes = MP3_BUFFER_SIZE;
	// nBytes = MAX_STREAM_CHANNELS * STREAM_BUFFER_SIZE; // for pcm streaming

    // Allocate the read buffers
    m_ReadBuffers[0] = (u32)x_malloc( nBytes );
    m_ReadBuffers[1] = (u32)x_malloc( nBytes );

    // Asign aram to the stream buffers
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

	// Free up the mp3 decompression buffers
	x_free( (void*)m_MainRam );

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
                                     io_request::READ_OP,
                                     pCallback );
}
