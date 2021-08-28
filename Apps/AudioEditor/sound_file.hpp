//==============================================================================
//
//  sound_file.hpp
//
//==============================================================================

#ifndef SOUND_FILE_HPP
#define SOUND_FILE_HPP

//==============================================================================
// INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "aiff_file.hpp"

//==============================================================================
//  sound_file
//==============================================================================

class sound_file
{
public:
                    sound_file          ( void );
                   ~sound_file          ( void );

public:
    xbool           Load                ( const char* pFileName );
    void            UnLoad              ( void );

    xbool           IsLoaded            ( void );
    s32             GetSampleRate       ( void );
    s32             GetNumChannels      ( void );
    s32             GetNumSamples       ( void );
    s16*            GetChannelData      ( s32 iChannel );
    f32             GetTime             ( void);
    s32             GetBreakPoints      ( xarray<aiff_file::breakpoint>& BreakPoints );
    xbool           IsLooped            ( void );
    s32             GetLoopStart        ( void );
    s32             GetLoopEnd          ( void );


protected:
    xbool               m_Loaded;
    s32                 m_SampleRate;
    s32                 m_NumChannels;
    s32                 m_NumSamples;
    s16**               m_pChannels;
    xbool               m_IsLooped;
    s32                 m_LoopStartPosition;
    s32                 m_LoopEndPosition;

    xarray<aiff_file::breakpoint>  m_BreakPoints;
};

//==============================================================================
#endif // SOUND_FILE_HPP
//==============================================================================
