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
#include "audio_file.hpp"

//==============================================================================
//  aiff_file
//==============================================================================

class aiff_file : public audio_file
{
private:
    class marker
    {
    public:
        s32     ID;
        s32     Position;
        xstring Name;
    };

public:
                    aiff_file           ( void );
    virtual        ~aiff_file           ( void );

public:
    virtual xbool   Open                ( const char* pFileName );
    virtual xbool   Open                ( X_FILE* pFile );
    virtual s32     GetSampleRate       ( void );
    virtual s32     GetNumChannels      ( void );
    virtual s32     GetNumSamples       ( void );
    virtual void    GetChannelData      ( s16* pData, s32 iChannel );
    virtual xbool   IsLooped            ( void );
    virtual s32     GetLoopStart        ( void );
    virtual s32     GetLoopEnd          ( void );
    virtual s32     GetBreakpoints      ( xarray<breakpoint>& BreakPoints );

    virtual void    Close               ( void );

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
    xarray<breakpoint> m_BreakPoints;
};

//==============================================================================
#endif // AIFF_FILE_HPP
//==============================================================================