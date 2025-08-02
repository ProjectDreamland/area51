//==============================================================================
//
// wav_file.cpp
//
//==============================================================================

#include "stdafx.h"
#include "wav_file.hpp"

//==============================================================================
//  Defines
//==============================================================================
#define WAVE_FORMAT_PCM 1

//==============================================================================
//  Types
//==============================================================================

#pragma pack( push, 1 )

struct wav_header
{
    u32 RIFF;
    u32 FileSize;
    u32 WAVE;
    u32 fmt;
    u32 FormatHeaderSize;
    u16 FormatTag;
    u16 nChannels;
    u32 SampleRate;
    u32 BytesPerSec;
    u16 BlockAlign;
    u16 BitsPerSample;
};

struct wav_chunk
{
    u32 ID;
    u32 Size;
};

struct wav_smpl_chunk
{
    u32 Manufacturer;
    u32 Product;
    u32 SamplePeriod;
    u32 MIDIUnityNote;
    u32 MIDIPitchFraction;
    u32 SMPTEFormat;
    u32 SMPTEOffset;
    u32 NumSampleLoops;
    u32 SamplerData;
};

struct wav_loop
{
    u32 CuePointID;
    u32 Type;
    u32 Start;
    u32 End;
    u32 Fraction;
    u32 PlayCount;
};

struct wav_cue_point
{
    u32 ID;
    u32 Position;
    u32 DataChunkID;
    u32 ChunkStart;
    u32 BlockStart;
    u32 SampleOffset;
};

struct wav_cue_chunk
{
    u32 NumCuePoints;
    wav_cue_point Points[1];
};

#pragma pack( pop )

//==============================================================================
//  wav_file
//==============================================================================

wav_file::wav_file( void )
{
    // Reset Data
    m_FileOwned         = TRUE;
    m_pFile             = NULL;
    m_LoopStartPosition = -1;
    m_LoopEndPosition   = -1;
    m_IsLooped          = FALSE;
}

//==============================================================================

wav_file::~wav_file( void )
{
    // Close the file
    if( m_pFile )
    {
        x_fclose( m_pFile );
        m_pFile = NULL;
    }
}

//==============================================================================

xbool wav_file::Open( X_FILE* pFile )
{
    ASSERT( !m_pFile );
    ASSERT( pFile );

    xbool   Error           = FALSE;
    xbool   GotFormatChunk  = FALSE;
    xbool   GotDataChunk    = FALSE;
    xbool   GotSampleChunk  = FALSE;
    xbool   GotCueChunk     = FALSE;

    // Store the file handle
    m_pFile = pFile;

    // Get length of file
    s32 FileLen = x_flength( m_pFile );
    
    // Reset loop points
    m_LoopStartPosition = -1;
    m_LoopEndPosition = -1;
    m_IsLooped = FALSE;

    // Parse header
    wav_header Header;
    if( x_fread( &Header, sizeof(Header), 1, m_pFile ) == 1 )
    {
        // Valid header?
        if( (Header.RIFF == 'FFIR') && (Header.WAVE == 'EVAW') && (Header.fmt == ' tmf') )
        {
            // Check format - only support PCM
            if (Header.FormatTag != WAVE_FORMAT_PCM)
            {
                Error = TRUE;
            }
            else
            {
                // Save format details
                m_NumChannels = Header.nChannels;
                m_SampleRate = Header.SampleRate;
                m_SampleSize = Header.BitsPerSample;
                
                // Prepare for parsing chunks
                GotFormatChunk = TRUE;
                
                // Parse chunks
                wav_chunk Chunk;
                while( !Error && (x_fread( &Chunk, sizeof(Chunk), 1, m_pFile ) == 1) )
                {
                    // Check for chunk type
                    switch( Chunk.ID )
                    {
                    case 'atad': // 'data'
                        {
                            // Found data chunk
                            m_DataOffset = x_ftell(m_pFile);
                            m_DataSize = Chunk.Size;
                            
                            // Calculate number of samples
                            s32 BytesPerSample = m_SampleSize / 8;
                            m_NumSamples = Chunk.Size / (BytesPerSample * m_NumChannels);
                            
                            // Skip data chunk
                            x_fseek(m_pFile, Chunk.Size, X_SEEK_CUR);
                            
                            GotDataChunk = TRUE;
                        }
                        break;
                        
                    case 'lpms': // 'smpl'
                        {
                            // Found sample chunk with loop info
                            wav_smpl_chunk SmplChunk;
                            if (x_fread(&SmplChunk, sizeof(wav_smpl_chunk), 1, m_pFile) == 1)
                            {
                                // Check if we have loops
                                if (SmplChunk.NumSampleLoops > 0)
                                {
                                    // Read first loop
                                    wav_loop Loop;
                                    if (x_fread(&Loop, sizeof(wav_loop), 1, m_pFile) == 1)
                                    {
                                        m_LoopStartPosition = Loop.Start;
                                        m_LoopEndPosition = Loop.End;
                                        m_IsLooped = TRUE;
                                        
                                        // Create a breakpoint for loop start
                                        breakpoint& Breakpoint = m_BreakPoints.Append();
                                        Breakpoint.Position = (f32)Loop.Start / (f32)m_SampleRate;
                                        Breakpoint.Name = "BP";
                                        
                                        // Create a breakpoint for loop end
                                        breakpoint& Breakpoint2 = m_BreakPoints.Append();
                                        Breakpoint2.Position = (f32)Loop.End / (f32)m_SampleRate;
                                        Breakpoint2.Name = "BP";
                                    }
                                }
                                
                                // Skip rest of chunk
                                s32 RemainingSize = Chunk.Size - sizeof(wav_smpl_chunk);
                                if (SmplChunk.NumSampleLoops > 0)
                                {
                                    RemainingSize -= sizeof(wav_loop);
                                }
                                if (RemainingSize > 0)
                                {
                                    x_fseek(m_pFile, RemainingSize, X_SEEK_CUR);
                                }
                            }
                            else
                            {
                                // Skip this chunk
                                x_fseek(m_pFile, Chunk.Size, X_SEEK_CUR);
                            }
                            
                            GotSampleChunk = TRUE;
                        }
                        break;
                        
                    case ' euc': // 'cue '
                        {
                            // Found cue chunk
                            wav_cue_chunk CueHeader;
                            if (x_fread(&CueHeader.NumCuePoints, sizeof(u32), 1, m_pFile) == 1)
                            {
                                // Read cue points and create markers
                                for (u32 i = 0; i < CueHeader.NumCuePoints; i++)
                                {
                                    wav_cue_point CuePoint;
                                    if (x_fread(&CuePoint, sizeof(wav_cue_point), 1, m_pFile) == 1)
                                    {
                                        // Create a marker
                                        marker& Marker = m_Markers.Append();
                                        Marker.ID = CuePoint.ID;
                                        Marker.Position = CuePoint.SampleOffset;
                                        Marker.Name = "BP";
                                        
                                        // Also create a breakpoint
                                        breakpoint& Breakpoint = m_BreakPoints.Append();
                                        Breakpoint.Position = (f32)CuePoint.SampleOffset / (f32)m_SampleRate;
                                        Breakpoint.Name = "BP";
                                    }
                                }
                            }
                            else
                            {
                                // Skip this chunk
                                x_fseek(m_pFile, Chunk.Size, X_SEEK_CUR);
                            }
                            
                            GotCueChunk = TRUE;
                        }
                        break;
                        
                    default:
                        // Skip unknown chunks
                        x_fseek(m_pFile, Chunk.Size, X_SEEK_CUR);
                        break;
                    }
                }
            }
            
            // Required chunks check
            if (!GotDataChunk)
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

    // Return success code
    return !Error;
}

//==============================================================================

xbool wav_file::Open( const char* pFileName )
{
    ASSERT( !m_pFile );

    xbool Error = FALSE;

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

s32 wav_file::GetSampleRate( void )
{
    ASSERT( m_pFile );
    return m_SampleRate;
}

//==============================================================================

s32 wav_file::GetNumChannels( void )
{
    ASSERT( m_pFile );
    return m_NumChannels;
}

//==============================================================================

s32 wav_file::GetNumSamples( void )
{
    ASSERT( m_pFile );
    return m_NumSamples;
}

//==============================================================================

void wav_file::GetChannelData( s16* pData, s32 iChannel )
{
    ASSERT( m_pFile );
    ASSERT( (iChannel >= 0) && (iChannel < m_NumChannels) );

    // Seek to data offset
    x_fseek( m_pFile, m_DataOffset, X_SEEK_SET );

    if( m_NumChannels == 1 )
    {
        // Read into buffer
        x_fread( pData, 2, m_NumSamples, m_pFile );
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
            pData[i] = pSamples[i*m_NumChannels+iChannel];
        }

        // Free temporary buffer
        x_free( pSamples );
    }
}

//==============================================================================

xbool wav_file::IsLooped( void )
{
    ASSERT( m_pFile );
    return m_IsLooped;
}

//==============================================================================

s32 wav_file::GetLoopStart( void )
{
    ASSERT( m_pFile );
    return m_LoopStartPosition;
}

//==============================================================================

s32 wav_file::GetLoopEnd( void )
{
    ASSERT( m_pFile );
    return m_LoopEndPosition;
}

//==============================================================================

void wav_file::Close( void )
{
    ASSERT( m_pFile );
    if( m_FileOwned )
        x_fclose( m_pFile );
    m_pFile = NULL;
}

//==============================================================================

s32 wav_file::GetBreakpoints( xarray<breakpoint>& BreakPoints )
{
    ASSERT( m_pFile );
    BreakPoints = m_BreakPoints;
    return BreakPoints.GetCount();
}

//==============================================================================