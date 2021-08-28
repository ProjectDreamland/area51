#if 0

#include "e_Audio.hpp"
#include "audio_debug.hpp"
#include "audio_hardware.hpp"
#include "debug\NotFound_gcn.hpp"
#include "debug\NotLoaded_gcn.hpp"

audio_debug g_AudioDebug;

extern "C" {
    void WriteARAMAsynch( void* MRAM, u32 ARAM, s32 Length ); 
};

unsigned char* s_DebugSamples[NUM_DEBUG_SAMPLES] =
{
    AUDIO_WARN_NOTFOUND,
    AUDIO_WARN_NOTLOADED
};

audio_debug::audio_debug( void )
{
    for( s32 i=0 ; i<NUM_DEBUG_SAMPLES ; i++ )
        x_memset( &m_DebugSamples[i], 0, sizeof(debug_sample) );
}

audio_debug::~audio_debug( void )
{
}


void audio_debug::Init( void )
{
    for( s32 i=0 ; i<NUM_DEBUG_SAMPLES ; i++ )
    {
        unsigned char* pData = s_DebugSamples[i];
        unsigned char  n;

        // Read string length.
        n = *pData;

        // Point to the string.
        m_DebugSamples[i].pName = (char*)(pData+1);
        pData += (u32)n;

        // Read header size
        n = *pData;

        // Point to sample header.
        m_DebugSamples[i].pSample = (sample_header*)(pData+4);
        pData += (u32)n;
        
        // Point to sample data
        m_DebugSamples[i].pSampleBuffer = (char*)pData;
        
        // Now allocate aram for the sample.
        m_DebugSamples[i].pAram = g_AudioHardware.AllocAudioRam( m_DebugSamples[i].pSample->WaveformLength );
        ASSERT( m_DebugSamples[i].pAram );

        // Set the aram up.
        m_DebugSamples[i].pSample->AudioRam = (u32)m_DebugSamples[i].pAram;

        // DMA the sample down.
        WriteARAMAsynch( m_DebugSamples[i].pSampleBuffer, m_DebugSamples[i].pSample->AudioRam, m_DebugSamples[i].pSample->WaveformLength );
    }
}

void audio_debug::Kill( void )
{
}

void audio_debug::InitVoice( voice* pVoice )
{
    (void)pVoice;
}

void audio_debug::UpdateVoice( voice* pVoice )
{
    (void)pVoice;
}

sample_header* audio_debug::GetDebugSampleHeader ( s32 Index )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= NUM_DEBUG_SAMPLES );

    return m_DebugSamples[Index].pSample;
}

char* audio_debug::GetDebugSampleName( s32 Index )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= NUM_DEBUG_SAMPLES );

    return m_DebugSamples[Index].pName;
}

void* audio_debug::GetDebugSampleARAM( s32 Index )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= NUM_DEBUG_SAMPLES );

    return m_DebugSamples[Index].pAram;
}

#endif
