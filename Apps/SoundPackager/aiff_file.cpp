//==============================================================================
//
// aiff_file.cpp
//
//==============================================================================

#include "stdafx.h"
#include "aiff_file.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Types
//==============================================================================

#pragma pack( push, 1 )

struct aiff_header
{
    u32 FORM;
    u32 Len;
    u32 AIFF;
};

struct aiff_chunk
{
    u32 ID;
    u32 Len;
};

struct aiff_chunk_common
{
    u16 nChannels;
    u32 nFrames;
    u16 SampleSize;
    u8  SampleRate[10];
};

struct aiff_marker
{
    s16 id;
    u32 Position;
    u8  NameLength;
};

struct aiff_chunk_marker
{
    u16 nMarkers;
};

struct aiff_loop
{
    s16 PlayMode;
    s16 beginLoop;
    s16 endLoop;
};

struct aiff_chunk_inst
{
    s8          baseNote;
    s8          detune;
    s8          lowNote;
    s8          highNote;
    s8          lowVelocity;
    s8          highVelocity;
    s16         gain;
    aiff_loop   sustainLoop;
    aiff_loop   releaseLoop;
};

#pragma pack( pop )

//==============================================================================
//  aiff_file
//==============================================================================

aiff_file::aiff_file( void )
{
    // Reset Data
    m_FileOwned         = TRUE;
    m_pFile             = NULL;
    m_LoopStartPosition = -1;
    m_LoopEndPosition   = -1;
    m_IsLooped          = FALSE;
}

//==============================================================================

aiff_file::~aiff_file( void )
{
    // Close the file
    if( m_pFile )
    {
        x_fclose( m_pFile );
        m_pFile = NULL;
    }
}

//==============================================================================

xbool aiff_file::Open( X_FILE* pFile )
{
    ASSERT( !m_pFile );
    ASSERT( pFile );

    xbool   Error           = FALSE;
    xbool   GotCommonChunk  = FALSE;
    xbool   GotInstChunk    = FALSE;
    xbool   GotMarkers      = FALSE;
    s32     LoopStartMarkerID;
    s32     LoopEndMarkerID;

    // Store the file handle
    m_pFile = pFile;

    // Get length of file
    s32 FileLen = x_flength( m_pFile );

    // Parse header
    aiff_header Header;
    if( x_fread( &Header, sizeof(Header), 1, m_pFile ) == 1 )
    {
        // Swap endian
        Header.FORM = BIG_ENDIAN_32( Header.FORM );
        Header.Len  = BIG_ENDIAN_32( Header.Len  );
        Header.AIFF = BIG_ENDIAN_32( Header.AIFF );

        // Valid header?
        if( (Header.FORM == 'FORM') && (Header.AIFF == 'AIFF') )
        {
            // Parse chunks
            aiff_chunk Chunk;
            while( !Error && (x_fread( &Chunk, sizeof(Chunk), 1, m_pFile ) == 1) )
            {
                // Swap endian
                Chunk.ID  = BIG_ENDIAN_32( Chunk.ID  );
                Chunk.Len = BIG_ENDIAN_32( Chunk.Len );

                // Check for chunk
                switch( Chunk.ID )
                {
                case 'COMM':
                    {
                        aiff_chunk_common ChunkCommon;
                        if( x_fread( &ChunkCommon, sizeof(ChunkCommon), 1, m_pFile ) )
                        {
                            // Swap endian
                            ChunkCommon.nChannels  = BIG_ENDIAN_16( ChunkCommon.nChannels  );
                            ChunkCommon.nFrames    = BIG_ENDIAN_32( ChunkCommon.nFrames    );
                            ChunkCommon.SampleSize = BIG_ENDIAN_16( ChunkCommon.SampleSize );

                            // Read SampleRate
                            u32 mantissa = BIG_ENDIAN_32(*(u32*)&ChunkCommon.SampleRate[2]);
                            u32 last     = 0;
                            u8  exp      = 30 - ChunkCommon.SampleRate[1];
                            while( exp-- )
                            {
                                last       = mantissa;
                                mantissa >>= 1;
                            }
                            if( last & 1 ) mantissa++;
                            s32 SampleRate = (s32)mantissa;

                            // Save details
                            m_NumChannels = ChunkCommon.nChannels;
                            m_NumSamples  = ChunkCommon.nFrames;
                            m_SampleSize  = ChunkCommon.SampleSize;
                            m_SampleRate  = SampleRate;

                            GotCommonChunk = TRUE;
                        }
                        else
                        {
                            // Failed to read chunk
                            Error = TRUE;
                        }
                    }
                    break;
                case 'SSND':
                    // Save file offset to sound data
                    m_DataOffset = x_ftell( m_pFile ) + 8;

                    // Seek through chunk
                    x_fseek( m_pFile, Chunk.Len, X_SEEK_CUR );
                    break;
                case 'MARK':
                    {
                        aiff_chunk_marker    ChunkMarker;
                        if( x_fread( &ChunkMarker, sizeof(ChunkMarker), 1, m_pFile ) )
                        {
                            // Swap endian
                            ChunkMarker.nMarkers = BIG_ENDIAN_16( ChunkMarker.nMarkers );
                            
                            // Does it have markers?
                            if( ChunkMarker.nMarkers )
                            {
                                GotMarkers = TRUE;
                            }

                            // Read all the markers
                            for( s32 i=0 ; i<ChunkMarker.nMarkers ; i++ )
                            {
                                // Create a new marker
                                marker& Marker = m_Markers.Append();

                                // Load marker from aiff
                                aiff_marker AiffMarker;
                                x_fread( &AiffMarker, sizeof(AiffMarker), 1, m_pFile );

                                // Swap endian
                                AiffMarker.id       = BIG_ENDIAN_16( AiffMarker.id );
                                AiffMarker.Position = BIG_ENDIAN_32( AiffMarker.Position );

                                // Read name of marker
                                char Name[256];
                                x_fread( &Name[0], 1, AiffMarker.NameLength, m_pFile );
                                // Null terminate the marker
                                Name[AiffMarker.NameLength] = 0;
                                x_fseek( m_pFile, (AiffMarker.NameLength+1)&1, X_SEEK_CUR );

                                // Setup marker
                                Marker.ID       = AiffMarker.id;
                                Marker.Name     = Name;
                                Marker.Position = AiffMarker.Position;
                            }
                        }
                        else
                        {
                            // Failed to read chunk
                            Error = TRUE;
                        }
                    }
                    break;
                case 'INST':
                    {
                        aiff_chunk_inst ChunkInst;
                        if( x_fread( &ChunkInst, sizeof(ChunkInst), 1, m_pFile ) )
                        {
                            // Swap endian
                            ChunkInst.gain                  = BIG_ENDIAN_16( ChunkInst.gain );
                            ChunkInst.sustainLoop.PlayMode  = BIG_ENDIAN_16( ChunkInst.sustainLoop.PlayMode  );
                            ChunkInst.sustainLoop.beginLoop = BIG_ENDIAN_16( ChunkInst.sustainLoop.beginLoop );
                            ChunkInst.sustainLoop.endLoop   = BIG_ENDIAN_16( ChunkInst.sustainLoop.endLoop   );
                            ChunkInst.releaseLoop.PlayMode  = BIG_ENDIAN_16( ChunkInst.releaseLoop.PlayMode  );
                            ChunkInst.releaseLoop.beginLoop = BIG_ENDIAN_16( ChunkInst.releaseLoop.beginLoop );
                            ChunkInst.releaseLoop.endLoop   = BIG_ENDIAN_16( ChunkInst.releaseLoop.endLoop   );

                            LoopStartMarkerID = ChunkInst.sustainLoop.beginLoop;
                            LoopEndMarkerID   = ChunkInst.sustainLoop.endLoop;

                            GotInstChunk = TRUE;
                        }
                        else
                        {
                            // Failed to read chunk
                            Error = TRUE;
                        }
                    }
                    break;
                default:
                    x_fseek( m_pFile, Chunk.Len, X_SEEK_CUR );
                    break;
                }
            } // while chunk

            // Error in file if no common chunk
            if( !GotCommonChunk )
            {
                Error = TRUE;
            }
        }
        else
        {
            // Invalid header
            Error = TRUE;
        }
    }
    else
    {
        // Failed to read header
        Error = TRUE;
    }

    // Check for loop points if we got and inst chunk
    if( !Error && GotInstChunk )
    {
        // Look for loop markers
        for( s32 i=0 ; i<m_Markers.GetCount() ; i++ )
        {
            if( m_Markers[i].ID == LoopStartMarkerID )
                m_LoopStartPosition = m_Markers[i].Position;
            if( m_Markers[i].ID == LoopEndMarkerID )
                m_LoopEndPosition = m_Markers[i].Position;
        }

        // If loop markers found then flag aiff as looped
        if( (m_LoopStartPosition != -1) && (m_LoopEndPosition != -1) )
        {
            m_IsLooped = TRUE;
        }
    }

    // Check for markers.
    if( !Error && GotMarkers )
    {
        // Look for loop markers
        for( s32 i=0 ; i<m_Markers.GetCount() ; i++ )
        {
            if( x_strcmp( m_Markers[i].Name, "BP" ) == 0 )
            {
                breakpoint& Breakpoint = m_BreakPoints.Append();

                // Get name and position.
                Breakpoint.Position = (f32)m_Markers[i].Position / (f32)m_SampleRate;
                Breakpoint.Name     = m_Markers[i].Name;
            }
        }
    }

    // Return success code
    return !Error;
}

//==============================================================================

xbool aiff_file::Open( const char* pFileName )
{
    ASSERT( !m_pFile );

    xbool   Error           = FALSE;
    xbool   GotCommonChunk  = FALSE;

    // Open the file
    X_FILE* pFile = x_fopen( pFileName, "rb" );
    if( pFile )
    {
        m_FileOwned = TRUE;
        Error = !Open( pFile );
    }
    else
    {
        // Error opening file
        Error = TRUE;
    }

    // Close file on error
    if( Error && m_pFile )
    {
        x_fclose( m_pFile );
        m_pFile = NULL;
    }

    // Return success code
    return( Error == FALSE );
}

//==============================================================================

s32 aiff_file::GetSampleRate( void )
{
    ASSERT( m_pFile );
    return m_SampleRate;
}

//==============================================================================

s32 aiff_file::GetNumChannels( void )
{
    ASSERT( m_pFile );
    return m_NumChannels;
}

//==============================================================================

s32 aiff_file::GetNumSamples( void )
{
    ASSERT( m_pFile );
    return m_NumSamples;
}

//==============================================================================

void aiff_file::GetChannelData( s16* pData, s32 iChannel )
{
    ASSERT( m_pFile );
    ASSERT( (iChannel >= 0) && (iChannel < m_NumChannels) );

    // Seek to data offset
    x_fseek( m_pFile, m_DataOffset, X_SEEK_SET );

    if( m_NumChannels == 1 )
    {
        // Read into buffer
        x_fread( pData, 2, m_NumSamples, m_pFile );

        // Swap data
        for( s32 i=0 ; i<m_NumSamples ; i++ )
        {
            pData[i] = BIG_ENDIAN_16( pData[i] );
        }
    }
    else
    {
        // Create a temporary buffer
        s16* pSamples = (s16*)x_malloc( m_NumSamples * m_NumChannels * sizeof(s16) );
        ASSERT( pSamples );

        // Read data
        x_fread( pSamples, 2, m_NumSamples*m_NumChannels, m_pFile );

        // Copy to buffer
        for( s32 i=0 ; i<m_NumSamples ; i++ )
        {
            pData[i] = BIG_ENDIAN_16( pSamples[i*m_NumChannels+iChannel] );
        }

        // Free temporary buffer
        x_free( pSamples );
    }
}

//==============================================================================

xbool aiff_file::IsLooped( void )
{
    ASSERT( m_pFile );
    return m_IsLooped;
}

//==============================================================================

s32 aiff_file::GetLoopStart( void )
{
    ASSERT( m_pFile );
    return m_LoopStartPosition;
}

//==============================================================================

s32 aiff_file::GetLoopEnd( void )
{
    ASSERT( m_pFile );
    return m_LoopEndPosition;
}

//==============================================================================

void aiff_file::Close( void )
{
    ASSERT( m_pFile );
    if( m_FileOwned )
        x_fclose( m_pFile );
    m_pFile = NULL;
}

//==============================================================================

s32 aiff_file::GetBreakpoints( xarray<breakpoint>& BreakPoints )
{
    ASSERT( m_pFile );
    BreakPoints = m_BreakPoints;
    return BreakPoints.GetCount();
}

//==============================================================================
