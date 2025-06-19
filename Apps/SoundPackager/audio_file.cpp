//==============================================================================
//
// audio_file.cpp
//
//==============================================================================

#include "stdafx.h"
#include "audio_file.hpp"
#include "aiff_file.hpp"
#include "wav_file.hpp"

//==============================================================================

audio_file* audio_file::Create(const char* pFileName)
{
    // Get file extension
    char Drive[10];
    char Dir[256];
    char Filename[128];
    char Ext[64];
    
    x_splitpath(pFileName, Drive, Dir, Filename, Ext);
    
    // Check if it's a WAV file
    if (x_stricmp(Ext, ".wav") == 0)
    {
        return new wav_file();
    }
    
    // Default to AIFF file
    return new aiff_file();
}

audio_file* audio_file::Create(X_FILE* pFile)
{
    // Check for WAV header signature (RIFF + WAVE)
    u32 riffSignature = 0;
    u32 waveSignature = 0;
    s32 currentPos = x_ftell(pFile);
    
    // Read RIFF signature
    x_fread(&riffSignature, sizeof(u32), 1, pFile);
    
    // Skip file size (4 bytes)
    x_fseek(pFile, 4, X_SEEK_CUR);
    
    // Read WAVE signature
    x_fread(&waveSignature, sizeof(u32), 1, pFile);
    
    // Reset file position
    x_fseek(pFile, currentPos, X_SEEK_SET);
    
    // Check if it's a WAV file (RIFF + WAVE)
    if (riffSignature == 'FFIR' && waveSignature == 'EVAW')
    {
        return new wav_file();
    }
    
    // Default to AIFF file
    return new aiff_file();
}

//==============================================================================