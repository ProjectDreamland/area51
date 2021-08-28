//==============================================================================
//  
//  x_profile.hpp
//  
//==============================================================================

#ifndef X_PROFILE_HPP
#define X_PROFILE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

//==============================================================================
//  SWITCHES
//==============================================================================

#define ENABLE_PROFILING    1

//==============================================================================
//  TYPES
//==============================================================================

enum profile_platform
{
    PROFILE_PLATFORM_PC,
    PROFILE_PLATFORM_PS2,
    PROFILE_PLATFORM_GCN,
    PROFILE_PLATFORM_XBOX
};

enum profile_token
{
    PROFILE_FRAME_BEGIN,
    PROFILE_FRAME_END,
    PROFILE_BEGIN,
    PROFILE_END,
    PROFILE_EVENT,
    PROFILE_STAT_S32,
    PROFILE_STAT_F32,

    PROFILE_TOKEN_END
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void profile_Init           ( const char* pFileName, s32 BufferSize, byte* pBuffer = NULL );
void profile_Kill           ( void );

void profile_Flush          ( void );
void profile_DiscardFrame   ( void );

void profile_FrameBegin     ( void );
void profile_FrameEnd       ( void );
void profile_Begin          ( s32 SectionID );
void profile_End            ( s32 SectionID );
void profile_Event          ( s32 EventID );
void profile_Stat           ( s32 StatID, s32 Value );
void profile_Stat           ( s32 StatID, f32 Value );

//==============================================================================
//  FUNCTION MACROS
//==============================================================================

#if ENABLE_PROFILING
#define PROFILE_FLUSH()         profile_Flush       ()
#define PROFILE_DISCARDFRAME()  profile_DiscardFrame()
#define PROFILE_FRAME()         profile_Frame       ()
#define PROFILE_BEGIN( id )     profile_Begin       ( id )
#define PROFILE_END( id )       profile_End         ( id )
#define PROFILE_EVENT( id )     profile_Event       ( id )
#define PROFILE_STAT( id, v )   profile_Stat        ( id, v )
#else
#define PROFILE_FLUSH()
#define PROFILE_DISCARDFRAME()
#define PROFILE_FRAME()
#define PROFILE_BEGIN( id )
#define PROFILE_END( id )
#define PROFILE_EVENT( id )
#define PROFILE_STAT( id, v )
#endif

//==============================================================================
//  x_profile class
//==============================================================================

class x_profile
{
protected:
    struct stat
    {
        f32     Min;
        f32     Max;
        f32     Total;
        s32     Count;

        void    Reset       ( void );
        void    Add         ( f32 Value );
        f32     GetAverage  ( void );
    };

public:
    struct frame
    {
        f32     Duration;
        xbool   EventHighlight;
    };

public:
            x_profile       ( );
            ~x_profile      ( );

    void    Reset           ( void );
    xbool   Load            ( const char* pFileName );
    xbool   IsLoaded        ( void );
    void    AnalyzeRange    ( s32 StartFrame, s32 EndFrame );

    s32     GetNumFrames    ( void );
    frame*  GetFrame        ( s32 iFrame );

protected:
    // Loaded Data
    xstring         m_DataFileName;
    xbool           m_DataLoaded;
    s32             m_DataSize;
    byte*           m_pData;

    // Processed Data
    byte            m_Platform;
    s64             m_TicksPerMs;
    s32             m_TokenCounts[PROFILE_TOKEN_END];
    stat            m_FrameDuration;

    s32             m_nFrames;
    frame*          m_pFrames;
};

//==============================================================================
#endif // X_PROFILE_HPP
//==============================================================================
