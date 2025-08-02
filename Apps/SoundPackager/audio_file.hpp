//==============================================================================
//
//  audio_file.hpp
//
//==============================================================================

#ifndef AUDIO_FILE_HPP
#define AUDIO_FILE_HPP

//==============================================================================
// INCLUDES
//==============================================================================

#include "x_files.hpp"

//==============================================================================
//  audio_file
//==============================================================================

class audio_file
{
public:
    class breakpoint
    {
    public:
        f32     Position;
        xstring Name;
    };

public:
    virtual                ~audio_file          ( void ) {}

public:
    virtual xbool           Open                ( const char* pFileName ) = 0;
    virtual xbool           Open                ( X_FILE* pFile ) = 0;
    virtual s32             GetSampleRate       ( void ) = 0;
    virtual s32             GetNumChannels      ( void ) = 0;
    virtual s32             GetNumSamples       ( void ) = 0;
    virtual void            GetChannelData      ( s16* pData, s32 iChannel ) = 0;
    virtual xbool           IsLooped            ( void ) = 0;
    virtual s32             GetLoopStart        ( void ) = 0;
    virtual s32             GetLoopEnd          ( void ) = 0;
    virtual s32             GetBreakpoints      ( xarray<breakpoint>& BreakPoints ) = 0;

    virtual void            Close               ( void ) = 0;

    // Factory methods to create correct file type based on extension or file pointer
    static audio_file*      Create              ( const char* pFileName );
    static audio_file*      Create              ( X_FILE* pFile );
};

//==============================================================================
#endif // AUDIO_FILE_HPP
//==============================================================================