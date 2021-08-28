#ifndef X_TOOL_H
#define X_TOOL_H
//==============================================================================
// xtool.hpp
//==============================================================================

#ifdef TARGET_PC
#include "windows.h"
#endif

#ifdef TARGET_XBOX
#   ifdef CONFIG_RETAIL
#       define D3DCOMPILE_PUREDEVICE 1
#   endif
#   include <xtl.h>
#endif

#include "../x_types.hpp"

//#if !defined(X_RETAIL)

//==============================================================================
//  xtool
//==============================================================================

class xtool
{
public:
    struct connect_message
    {
        char    Magic[4];               // "XDBG" for little endian, "GBDX" for big endian
        s32     Version;
        s32     Platform;               // See enum in x_target.hpp
        s32     HeapStart;              // Start of heap
        s32     HeapEnd;                // End of heap (if both start and end are 0 then it is unknown)
        s64     TicksPerSecond;         // Ticks per second for the client
        s64     BaselineTicks;          // Ticks at connect time
        char    ApplicationName[32];    // Name of the application that is connecting

        void ByteSwap( void );
    };

    struct packet_header
    {
        s32     Type;                   // See enum packet_type
        s32     Length;                 // Number of bytes following this header for the body of the packet
        u32     CRC;                    // CRC for the data

        void ByteSwap( void );
    };

    enum packet_type
    {
        PACKET_LOG          = 0,        // Log packet from client to server
        PACKET_FILESERVER,              // Fileserver packet, from client or server
    };

    enum log_type
    {
        LOG_TYPE_MESSAGE    = 0,        // Log message
        LOG_TYPE_MEMORY,                // Log memory allocator operation
        LOG_TYPE_IO,                    // Log IO operations
        LOG_TYPE_APPLICATION_NAME,      // Log application name change
        LOG_TYPE_TIMER_PUSH,            // Log timer push
        LOG_TYPE_TIMER_POP,             // Log timer pop
    };

    // Severity for LOG_TYPE_MESSAGE entries
    enum log_severity
    {
        LOG_SEVERITY_MESSAGE,
        LOG_SEVERITY_WARNING,
        LOG_SEVERITY_ERROR,
        LOG_SEVERITY_ASSERT,
    };

    // Type for LOG_TYPE_MEMORY entries
    enum log_mem_operation
    {
        LOG_MEMORY_MALLOC,
        LOG_MEMORY_REALLOC,
        LOG_MEMORY_FREE,
        LOG_MEMORY_MARK,
    };

protected:
    xbool           m_Initialized;
#if defined(TARGET_PC) || defined(TARGET_XBOX)
    HANDLE          m_hPipe;
#endif
#if defined(TARGET_PS2) || defined(TARGET_GCN)
    s32             m_hPipe;
#endif
    packet_header   m_PacketHeader;
    void*           m_pPacketData;
    s32             m_PacketDataLength;

    u8*             m_pBufferBase;
    u8*             m_pBuffer;
    s32             m_BufferOffset;
    s32             m_BufferSize;

protected:
    xbool       Send            ( s32 PacketType, void* pData, s32 Length );
    xbool       Receive         ( void );
    void        EnsureSpace     ( s32 BytesNeeded );
    void        Log             ( s32 Severity, const char* pChannel, const char* pMessage, const char* pFile, s32 Line );

public:
                xtool           ( void );
    virtual    ~xtool           ( void );

    xbool       Init            ( void );
    void        Kill            ( void );
    xbool       IsInitialized   ( void );

    xbool       IsConnected     ( void );
    xbool       Flush           ( void );

    void        LogMessage      ( const char* pChannel, const char* pMessage, const char* pFile, s32 Line );
    void        LogWarning      ( const char* pChannel, const char* pMessage, const char* pFile, s32 Line );
    void        LogError        ( const char* pChannel, const char* pMessage, const char* pFile, s32 Line );
    void        LogAssert       (                       const char* pMessage, const char* pFile, s32 Line );

    void        LogTimerPush    ( void );
    void        LogTimerPop     ( const char* pChannel, f32 TimeLimitMS, const char* pMessage );

    void        LogMalloc       ( u32 Address, u32 Size, const char* pFile, s32 Line );
    void        LogRealloc      ( u32 Address, u32 OldAddress, s32 Size, const char* pFile, s32 Line );
    void        LogFree         ( u32 Address, const char* pFile, s32 Line );
    void        LogMemMark      ( const char* pMarkName );

    void        SetAppName      ( const char* pName );
};

//==============================================================================
//  Globals
//==============================================================================

extern xtool   g_Tool;

//==============================================================================

//#endif // !defined(X_RETAIL)

//==============================================================================
#endif // X_TOOL_H
