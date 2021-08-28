//==============================================================================
//
//  aiff_file.hpp
//
//==============================================================================

#ifndef AIFF_FILE_HPP
#define AIFF_FILE_HPP

//==============================================================================
// INCLUDES
//==============================================================================

#include "x_files.hpp"

//==============================================================================
//  aiff_file
//==============================================================================

class aiff_file
{
    class marker
    {
    public:
        s32     ID;
        s32     Position;
        xstring Name;
    };

public:
                    aiff_file           ( void );
                   ~aiff_file           ( void );

public:
    xbool           Open                ( const char* pFileName );
    xbool           Open                ( X_FILE* pFile );
    s32             GetSampleRate       ( void );
    s32             GetNumChannels      ( void );
    s32             GetNumSamples       ( void );
    void            GetChannelData      ( s16* pData, s32 iChannel );
    xbool           IsLooped            ( void );
    s32             GetLoopStart        ( void );
    s32             GetLoopEnd          ( void );

    void            Close               ( void );

protected:
    xbool           m_FileOwned;
    X_FILE*         m_pFile;
    s32             m_NumChannels;
    s32             m_NumSamples;
    s32             m_SampleSize;
    s32             m_SampleRate;
    s32             m_DataOffset;

    xbool           m_IsLooped;
    s32             m_LoopStartPosition;
    s32             m_LoopEndPosition;
    xarray<marker>  m_Markers;
};

//==============================================================================
#endif // AIFF_FILE_HPP
//==============================================================================
