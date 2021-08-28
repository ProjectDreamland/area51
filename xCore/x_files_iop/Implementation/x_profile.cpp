//==============================================================================
//  
//  x_profile.cpp
//
//==============================================================================

#include "..\x_files.hpp"
#include "..\x_profile.hpp"

//==============================================================================

static xbool    s_Initialized     = FALSE;
static xbool    s_BufferAllocated = FALSE;
static s32      s_BufferSize;
static X_FILE*  s_fh;

static byte*    s_pBufferStart;
static byte*    s_pBufferEnd;
static byte*    s_pCurrent;
static byte*    s_pLastFrame;

// Define the platform
#if   defined(TARGET_PC)
static u8       s_Platform = PROFILE_PLATFORM_PC;
#elif defined(TARGET_PS2)
static u8       s_Platform = PROFILE_PLATFORM_PS2;
#elif defined(TARGET_GCN)
static u8       s_Platform = PROFILE_PLATFORM_GCN;
#elif defined(TARGET_XBOX)
static u8       s_Platform = PROFILE_PLATFORM_XBOX;
#endif

//==============================================================================
//  profile_Init
//==============================================================================

void profile_Init( const char* pFileName, s32 BufferSize, byte* pBuffer )
{
    ASSERT( !s_Initialized );

    // Setup the data buffer, either handed to us or allocated
    if( pBuffer )
    {
        s_pBufferStart    = pBuffer;
        s_BufferAllocated = FALSE;
    }
    else
    {
        s_pBufferStart = (byte*)x_malloc( BufferSize );
        ASSERT( s_pBufferStart );
        s_BufferAllocated = TRUE;
    }

    s_BufferSize      = BufferSize;
    s_pBufferEnd      = s_pBufferStart + s_BufferSize;
    s_pCurrent        = s_pBufferStart;
    s_pLastFrame      = s_pBufferStart;

    // Now initialized
    s_Initialized     = TRUE;

    // Open file for write
    s_fh = x_fopen( pFileName, "wb" );
    ASSERT( s_fh );

    // Write profile header into stream
    *s_pCurrent++     = 'X';
    *s_pCurrent++     = 'P';
    *s_pCurrent++     = 'R';
    *s_pCurrent++     = 'O';
    *s_pCurrent++     = s_Platform;
    s64 TicksPerMs    = x_GetTicksPerMs();
    x_memcpy( s_pCurrent, &TicksPerMs, sizeof(s64) );
    s_pCurrent += sizeof(s64);

    // Write first frame marker
    profile_FrameBegin();
}

void profile_Kill( void )
{
    ASSERT( s_Initialized );

    // Close file
    x_fclose( s_fh );
    s_fh = 0;

    // Free allocated buffer
    if( s_BufferAllocated )
    {
        x_free( s_pBufferStart );
        s_pBufferStart    = NULL;
        s_BufferAllocated = FALSE;
    }

    // No longer initialized
    s_Initialized = FALSE;
}

//==============================================================================
//  profile_Flush
//==============================================================================

void profile_Flush( void )
{
    ASSERT( s_Initialized );

    // Write current buffer data
    x_fwrite( s_pBufferStart, 1, s_pCurrent-s_pBufferStart, s_fh );

    // Reset buffer pointers
    s_pCurrent   = s_pBufferStart;
    s_pLastFrame = s_pBufferStart;
}

void profile_DiscardFrame( void )
{
    ASSERT( s_Initialized );

    // Roll back buffer pointer
    s_pCurrent = s_pLastFrame;
}

//==============================================================================
//  profile_Frame
//==============================================================================

void profile_FrameBegin( void )
{
    ASSERT( s_Initialized );
    ASSERT( s_pCurrent < (s_pBufferEnd-(1+sizeof(xtick))) );

    // Save position for end of last frame
    s_pLastFrame = s_pCurrent;

    // Insert frame tag into stream
    *s_pCurrent++ = PROFILE_FRAME_BEGIN;

    // Write timestamp into stream
    xtick Tick = x_GetTime();
    x_memcpy( s_pCurrent, &Tick, sizeof(xtick) );
    s_pCurrent += sizeof(xtick);
}

void profile_FrameEnd( void )
{
    ASSERT( s_Initialized );
    ASSERT( s_pCurrent < (s_pBufferEnd-(1+sizeof(xtick))) );

    // Save position for end of last frame
    s_pLastFrame = s_pCurrent;

    // Insert frame tag into stream
    *s_pCurrent++ = PROFILE_FRAME_END;

    // Write timestamp into stream
    xtick Tick = x_GetTime();
    x_memcpy( s_pCurrent, &Tick, sizeof(xtick) );
    s_pCurrent += sizeof(xtick);
}

void profile_Begin( s32 SectionID )
{
    ASSERT( s_Initialized );
    ASSERT( s_pCurrent < (s_pBufferEnd-(2+sizeof(xtick))) );

    // Insert tag into stream
    *s_pCurrent++ = PROFILE_BEGIN;
    *s_pCurrent++ = (u8)SectionID;

    // Write timestamp into stream
    xtick Tick = x_GetTime();
    x_memcpy( s_pCurrent, &Tick, sizeof(xtick) );
    s_pCurrent += sizeof(xtick);
}

void profile_End( s32 SectionID )
{
    ASSERT( s_Initialized );
    ASSERT( s_pCurrent < (s_pBufferEnd-(2+sizeof(xtick))) );

    // Insert tag into stream
    *s_pCurrent++ = PROFILE_END;
    *s_pCurrent++ = (u8)SectionID;

    // Write timestamp into stream
    xtick Tick = x_GetTime();
    x_memcpy( s_pCurrent, &Tick, sizeof(xtick) );
    s_pCurrent += sizeof(xtick);
}

void profile_Event( s32 EventID )
{
    ASSERT( s_Initialized );
    ASSERT( s_pCurrent < (s_pBufferEnd-(2+sizeof(xtick))) );

    // Insert tag into stream
    *s_pCurrent++ = PROFILE_EVENT;
    *s_pCurrent++ = (u8)EventID;

    // Write timestamp into stream
    xtick Tick = x_GetTime();
    x_memcpy( s_pCurrent, &Tick, sizeof(xtick) );
    s_pCurrent += sizeof(xtick);
}

void profile_Stat( s32 StatID, s32 Value )
{
    ASSERT( s_Initialized );
    ASSERT( s_pCurrent < (s_pBufferEnd-(6+sizeof(xtick))) );

    // Insert tag into stream
    *s_pCurrent++ = PROFILE_STAT_S32;
    *s_pCurrent++ = (u8)StatID;
    x_memcpy( s_pCurrent, &Value, 4 );
    s_pCurrent += 4;

    // Write timestamp into stream
    xtick Tick = x_GetTime();
    x_memcpy( s_pCurrent, &Tick, sizeof(xtick) );
    s_pCurrent += sizeof(xtick);
}

void profile_Stat( s32 StatID, f32 Value )
{
    ASSERT( s_Initialized );
    ASSERT( s_pCurrent < (s_pBufferEnd-(6+sizeof(xtick))) );

    // Insert tag into stream
    *s_pCurrent++ = PROFILE_STAT_F32;
    *s_pCurrent++ = (u8)StatID;
    x_memcpy( s_pCurrent, &Value, 4 );
    s_pCurrent += 4;

    // Write timestamp into stream
    xtick Tick = x_GetTime();
    x_memcpy( s_pCurrent, &Tick, sizeof(xtick) );
    s_pCurrent += sizeof(xtick);
}

//==============================================================================
//  x_profile::stat
//==============================================================================

void x_profile::stat::Reset( void )
{
    Min   = F32_MAX;
    Max   = F32_MIN;
    Total = 0;
    Count = 0;
};

void x_profile::stat::Add( f32 Value )
{
    if( Value < Min ) Min = Value;
    if( Value > Max ) Max = Value;
    Total += Value;
    Count++;
};

f32 x_profile::stat::GetAverage( void )
{
    return Total / Count;
}

//==============================================================================
//  DATA
//==============================================================================

static s32 TokenLengths[PROFILE_TOKEN_END] =
{
    1+8,
    1+8,
    1+1+8,
    1+1+8,
    1+1+8,
    1+1+4+8,
    1+1+4+8,
};

//==============================================================================
//  x_profile
//==============================================================================

x_profile::x_profile( )
{
    m_DataLoaded = FALSE;

    m_DataSize   = 0;
    m_pData      = NULL;

    m_nFrames    = 0;
    m_pFrames    = NULL;
}

//==============================================================================

x_profile::~x_profile()
{
    Reset();
}

//==============================================================================

void x_profile::Reset( void )
{
    m_DataLoaded = FALSE;

    if( m_pData )
        x_free( m_pData );

    m_nFrames = 0;
    x_free( m_pFrames );
    m_pFrames = NULL;
}

//==============================================================================

xbool x_profile::Load( const char* pFileName )
{
    // Reset class
    Reset();

    // Load data file
    m_DataFileName = pFileName;
    X_FILE* pFile = x_fopen( m_DataFileName, "rb" );
    if( pFile )
    {
        m_DataSize = x_flength( pFile ); 
        m_pData = (byte*)x_malloc( m_DataSize );
        if( m_pData )
        {
            x_fread( m_pData, 1, m_DataSize, pFile );
            m_DataLoaded = TRUE;
        }
        x_fclose( pFile );
    }

    // Clear counts
    x_memset( &m_TokenCounts, 0, sizeof(m_TokenCounts) );

    // Process the loaded data to pull out global info and validate it
    xbool   Error               = FALSE;
    s32     InFrame             = 0;
    s64     FrameBeginTime      = 0;
    s64     FrameEndTime        = 0;
    byte*   pData               = m_pData;
    byte*   pDataEnd            = m_pData + m_DataSize;
    byte*   pDataTokens         = m_pData;
    byte*   pDataLastFrameEnd   = m_pData;
    s32     iFrame;

    // Read Header
    if( x_strncmp( (const char*)pData, "XPRO", 4 ) != 0 )
    {
        Error = TRUE;
    }
    pData += 4;

    // Read platform type
    m_Platform = *pData++;

    // Read ticks per ms
    m_TicksPerMs = *(s64*)pData;
    pData += 8;

    // Save pointer to beginning of token stream
    pDataTokens = pData;

    // Loop through tokens
    while( (pData < pDataEnd) && !Error )
    {
        // Read token
        byte Token = *pData++;
        ASSERT( (Token < PROFILE_TOKEN_END) );

        // Advance counts for token type
        m_TokenCounts[Token]++;

        // Process token
        switch( Token )
        {
        case PROFILE_FRAME_BEGIN:
            if( InFrame != 0 )
            {
                Error = TRUE;
                break;
            }
            InFrame++;
            break;
        case PROFILE_FRAME_END:
            if( InFrame == 0 )
            {
                Error = TRUE;
                break;
            }
            InFrame--;
            pDataLastFrameEnd = pData-1+TokenLengths[PROFILE_FRAME_END];
            break;
        case PROFILE_BEGIN:
            break;
        case PROFILE_END:
            break;
        case PROFILE_EVENT:
            break;
        case PROFILE_STAT_S32:
            break;
        case PROFILE_STAT_F32:
            break;
        }

        // Advance pointer past last token
        pData += TokenLengths[Token]-1;
    }

    // Check for an error
    if( !Error )
    {
        // Allocate frame data storage
        m_nFrames = m_TokenCounts[PROFILE_FRAME_END];
        m_pFrames = (frame*)x_realloc( m_pFrames, sizeof(frame) * m_nFrames );

        // Prepare to process
        pData  = pDataTokens;
        iFrame = -1;

        // Build processed data structures
        while( pData < pDataLastFrameEnd )
        {
            // Read token
            byte Token = *pData++;
            ASSERT( (Token < PROFILE_TOKEN_END) );

            // Process token
            switch( Token )
            {
            case PROFILE_FRAME_BEGIN:
                iFrame++;
                FrameBeginTime = *(s64*)pData;
                break;
            case PROFILE_FRAME_END:
                FrameEndTime = *(s64*)pData;
                m_pFrames[iFrame].Duration = (f32)(FrameEndTime-FrameBeginTime) / (f32)m_TicksPerMs;
                m_pFrames[iFrame].EventHighlight = FALSE;
                m_FrameDuration.Add( m_pFrames[iFrame].Duration );
                break;
            case PROFILE_BEGIN:
                break;
            case PROFILE_END:
                break;
            case PROFILE_EVENT:
                break;
            case PROFILE_STAT_S32:
                break;
            case PROFILE_STAT_F32:
                break;
            }

            // Advance pointer past last token
            pData += TokenLengths[Token]-1;
        }
    }
    else
    {
        // TODO: Error loading, cleanup
        ASSERT( 0 );
    }

    // Return success code
    return m_DataLoaded;
}

//==============================================================================

xbool x_profile::IsLoaded( void )
{
    return m_DataLoaded;
}

//==============================================================================

void x_profile::AnalyzeRange( s32 StartFrame, s32 EndFrame )
{
    (void)StartFrame;
    (void)EndFrame;
}

//==============================================================================

s32 x_profile::GetNumFrames( void )
{
    return m_nFrames;
}

//==============================================================================

x_profile::frame* x_profile::GetFrame( s32 iFrame )
{
    ASSERT( m_pFrames );
    ASSERT( iFrame >= 0 );
    ASSERT( iFrame <  m_nFrames );

    return &m_pFrames[iFrame];
}

//==============================================================================
