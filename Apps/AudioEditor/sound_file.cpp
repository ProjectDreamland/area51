//==============================================================================
//
// sound_file.cpp
//
//==============================================================================

#include "stdafx.h"
#include "sound_file.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Types
//==============================================================================

//==============================================================================
//  sound_file
//==============================================================================

sound_file::sound_file( void )
{
    m_Loaded    = FALSE;
    m_pChannels = NULL;
}

//==============================================================================

sound_file::~sound_file( void )
{

}

//==============================================================================

xbool sound_file::Load( const char* pFileName )
{
    ASSERT( !m_Loaded );

    // Open the file
    aiff_file AIFF;
    if( AIFF.Open( pFileName ) )
    {
        // Setup data
        m_Loaded      = TRUE;
        m_SampleRate  = AIFF.GetSampleRate();
        m_NumChannels = AIFF.GetNumChannels();
        m_NumSamples  = AIFF.GetNumSamples();

        m_IsLooped          = AIFF.IsLooped();
        m_LoopStartPosition = AIFF.GetLoopStart();
        m_LoopEndPosition   = AIFF.GetLoopEnd();

        AIFF.GetBreakpoints( m_BreakPoints );

        m_pChannels   = (s16**)x_malloc( sizeof(s16*) * m_NumChannels );
        ASSERT( m_pChannels );

        // Read channel data
        for( s32 i=0 ; i<m_NumChannels ; i++ )
        {
            m_pChannels[i] = (s16*)x_malloc( sizeof(s16) * m_NumSamples );
            ASSERT( m_pChannels[i] );

            AIFF.GetChannelData( m_pChannels[i], i );
        }

        // Close the AIFF
        AIFF.Close();
    }

    // Return success code
    return m_Loaded;
}

//==============================================================================

xbool sound_file::IsLoaded( void )
{
    return m_Loaded;
}

//==============================================================================

s32 sound_file::GetSampleRate( void )
{
    ASSERT( m_Loaded );
    return m_SampleRate;
}

//==============================================================================

s32 sound_file::GetNumChannels( void )
{
    ASSERT( m_Loaded );
    return m_NumChannels;
}

//==============================================================================

s32 sound_file::GetNumSamples( void )
{
    ASSERT( m_Loaded );
    return m_NumSamples;
}

//==============================================================================

s16* sound_file::GetChannelData( s32 iChannel )
{
    ASSERT( m_Loaded );
    ASSERT( (iChannel >= 0) && (iChannel < m_NumChannels) );

    return m_pChannels[iChannel];
}

//==============================================================================

void sound_file::UnLoad( void )
{
    // Delete all the channels.
    if( m_pChannels )
    {
        for( s32 i=0 ; i<m_NumChannels ; i++ )
        {
            x_free( m_pChannels[i] );
            m_pChannels[i] = NULL;
        }
        x_free( m_pChannels );
        m_pChannels = NULL;
    }
    m_Loaded    = FALSE;
}

//==============================================================================

f32 sound_file::GetTime( void )
{
    return (f32)m_NumSamples/(f32)m_SampleRate;
}

//==============================================================================

s32 sound_file::GetBreakPoints( xarray<aiff_file::breakpoint>& BreakPoints )
{
    BreakPoints = m_BreakPoints;
    return m_BreakPoints.GetCount();
}

//==============================================================================

xbool sound_file::IsLooped( void )
{
    ASSERT( m_Loaded );
    return m_IsLooped;
}

//==============================================================================

s32 sound_file::GetLoopStart( void )
{
    ASSERT( m_Loaded );
    return m_LoopStartPosition;
}

//==============================================================================

s32 sound_file::GetLoopEnd( void )
{
    ASSERT( m_Loaded );
    return m_LoopEndPosition;
}

//==============================================================================
