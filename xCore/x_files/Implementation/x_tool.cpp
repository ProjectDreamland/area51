//==============================================================================
// xtool.cpp
//==============================================================================

#include "../x_files.hpp"
#include "../x_threads.hpp"
#include "x_tool_private.hpp"

#include <stdio.h>

#ifdef TARGET_PS2
#include "sifdev.h"
#endif

#ifdef TARGET_GCN
#include "libsn.h"
#endif

//==============================================================================
//  Make this whole file go away in X_RETAIL builds
//==============================================================================

#if !defined(X_RETAIL) || defined(X_EDITOR) || defined(X_LOGGING)

//==============================================================================
//  Globals
//==============================================================================

xtool   g_Tool;

s32     s_WriteOffset;
s32     s_ReadOffset;
//==============================================================================
//  Static data
//==============================================================================

#if defined(TARGET_PS2) && defined(X_DEBUG)
#define BUFFER_SIZE (32768) //(1*1024*1024) //(32768)
#else
#define BUFFER_SIZE 32768
#endif // defined(TARGET_PS2) && defined(X_DEBUG)

static u8   s_Buffer[BUFFER_SIZE];

//==============================================================================
//  Helper functions
//==============================================================================

static inline s32 BYTESWAP( s32 a )
{
    return ENDIAN_SWAP_32(a);
}

static inline u32 BYTESWAP( u32 a )
{
    return ENDIAN_SWAP_32(a);
}

static inline s64 BYTESWAP( s64 a )
{
    return ENDIAN_SWAP_64(a);
}

//==============================================================================
//  PC specifics
//==============================================================================

#ifdef TARGET_PC

HANDLE OpenPipe( void )
{
    return CreateFile( "\\\\.\\pipe\\xlink",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL );
}

xbool WritePipe( HANDLE hPipe, const void* pData, s32 nBytes, s32* pnWritten )
{
    BOOL    result;
    DWORD   bytes_written   = 0;
    s32     bytes_remaining = nBytes;
    s32     total_written   = 0;
    u8*     pBuffer         = (u8*)pData;

    do{
        result = WriteFile( hPipe, pBuffer, bytes_remaining, &bytes_written, NULL );
        if( !result )
        {
            *pnWritten = total_written;
            return FALSE;
        }

        bytes_remaining -= bytes_written;
        total_written   += bytes_written;
        pBuffer         += bytes_written;
    } while( bytes_remaining > 0 );

    *pnWritten = total_written;
    return TRUE;
}

xbool ReadPipe( HANDLE hPipe, void* pData, s32 nBytes, s32* pnRead )
{
    return ReadFile( hPipe, pData, nBytes, (DWORD*)pnRead, NULL );
}

xbool ClosePipe( HANDLE hPipe )
{
    return CloseHandle( hPipe );
}

#endif

//==============================================================================
//  XBOX specifics
//==============================================================================

#ifdef TARGET_XBOX

// Note:
// For the xbox only, we need to make sure that logging is disabled during the
// logging calls to the x_files. This is because the x_files will try to log!

// ***NOTE*** logging will only work for xbox if entropy is included and netfs
// has been enabled.

xbool   s_Busy = FALSE;

void*   netfs_Open  ( const char* pFilename, const char* pMode );
s32     netfs_Write ( void* Handle, const void* pBuffer, s32 Offset, s32 Length );
s32     netfs_Read  ( void* Handle, void* pBuffer, s32 Offset, s32 Length );
void    netfs_Close ( void* Handle );


HANDLE OpenPipe( void )
{
    ASSERT( s_Busy == FALSE );
    s_Busy = TRUE;
    HANDLE Handle;
    Handle = netfs_Open( "\\\\.\\pipe\\xlink", "wb" );

    s_Busy = FALSE;
    if( Handle == 0 )
    {
        return INVALID_HANDLE_VALUE;
    }
    s_WriteOffset   = 0;
    s_ReadOffset    = 0;
    return Handle;
}

xbool WritePipe( HANDLE hPipe, const void* pData, s32 nBytes, s32* pnWritten )
{
    ASSERT( s_Busy == FALSE );
    s_Busy = TRUE;
    *pnWritten = netfs_Write( (void*)hPipe, pData, s_WriteOffset, nBytes );
    s_WriteOffset+= *pnWritten;
    s_Busy = FALSE;

    return (*pnWritten>=0);
}

xbool ReadPipe( HANDLE hPipe, void* pData, s32 nBytes, s32* pnRead )
{
    ASSERT( s_Busy == FALSE );
    s_Busy = TRUE;
    *pnRead = netfs_Read( (void*)hPipe, pData, s_ReadOffset, nBytes );
    s_ReadOffset += *pnRead;
    s_Busy = FALSE;
    return (*pnRead>=0);
}

xbool ClosePipe( HANDLE hPipe )
{
    ASSERT( s_Busy == FALSE );
    s_Busy = TRUE;
    netfs_Close( (void*)hPipe );
    s_Busy = FALSE;
    return TRUE;
}

#endif

//==============================================================================
//  PS2 specifics
//==============================================================================

#ifdef TARGET_PS2

typedef s32 HANDLE;
#define INVALID_HANDLE_VALUE    -1

HANDLE OpenPipe( void )
{
    s32 hPipe = sceOpen( "host0:\\\\.\\pipe\\xlink", SCE_RDWR );
    return hPipe;
}

xbool WritePipe( HANDLE hPipe, const void* pData, s32 nBytes, s32* pnWritten )
{
    s32     FailCount       = 0;
    s32     bytes_written   = 0;
    s32     bytes_remaining = nBytes;
    s32     total_written   = 0;
    u8*     pBuffer         = (u8*)pData;
    xbool   IsAtomic;

    //****BAD BAD BAD BAD!
    // We should not be trying to write to a file while the system is in an
    // atomic state. This can happen like so: Allocations are threadsafe so
    // they are done in an atomic state. All allocations are logged if memory
    // logging is enabled. When enough go by, it trys to flush the log when it's
    // in the atomic state. This causes the PS2 to mess up royally!
    //
    // NOTE: This has been set up to clear the atomic state since it's used
    // in all sorts of different places.

    IsAtomic = x_IsAtomic();
    if( IsAtomic )
    {
        x_EndAtomic();
    }

    do
    {
        bytes_written = sceWrite( hPipe, pBuffer, bytes_remaining );

        if( (bytes_written == 1) && (bytes_remaining != 1) )
        {
            FailCount++;
        }

        if( (bytes_written < 0) || (FailCount > 10) )
        {
            *pnWritten = total_written;
            if( IsAtomic )
            {
                x_BeginAtomic();
            }
            return FALSE;
        }

        bytes_remaining -= bytes_written;
        total_written   += bytes_written;
        pBuffer         += bytes_written;
    } while( bytes_remaining > 0 );

    if( IsAtomic )
    {
        x_BeginAtomic();
    }
    *pnWritten = total_written;
    return TRUE;
}

xbool ReadPipe( HANDLE hPipe, void* pData, s32 nBytes, s32* pnRead )
{
    s32 bytes_read      = 0;
    s32 bytes_remaining = nBytes;
    s32 total_read      = 0;
    u8* pBuffer         = (u8*)pData;
    xbool IsAtomic;

    IsAtomic = x_IsAtomic();
    if( IsAtomic )
    {
        x_EndAtomic();
    }
    do
    {
        bytes_read = sceRead( hPipe, pBuffer, bytes_remaining );
        if( bytes_read < 0 )
        {
            *pnRead = total_read;
            if( IsAtomic )
            {
                x_BeginAtomic();
            }
            return FALSE;
        }

        bytes_remaining -= bytes_read;
        total_read      += bytes_read;
        pBuffer         += bytes_read;
    } while( bytes_remaining > 0 );

    *pnRead = total_read;
    if( IsAtomic )
    {
        x_BeginAtomic();
    }
    return TRUE;
}

xbool ClosePipe( HANDLE hPipe )
{
    return sceClose( hPipe );
}

#endif

//==============================================================================
//  GCN specifics
//==============================================================================

#ifdef TARGET_GCN

typedef s32 HANDLE;
#define INVALID_HANDLE_VALUE    -1

HANDLE OpenPipe( void )
{
    HANDLE hPipe = PCopen( "\\\\.\\pipe\\xlink", 2, 0 );
    return hPipe;
}

s32 WRITE_CHUNK_SIZE = 2048; //1024;

xbool WritePipe( HANDLE hPipe, const void* pData, s32 nBytes, s32* pnWritten )
{
    s32 bytes_written   = 0;
    s32 bytes_remaining = nBytes;
    s32 total_written   = 0;
    u8* pBuffer         = (u8*)pData;

    do
    {
        s32 bytes_to_write = MIN( bytes_remaining, WRITE_CHUNK_SIZE );
        bytes_written = PCwrite( hPipe, pBuffer, bytes_to_write );
        if( bytes_written < 0 )
        {
            *pnWritten = total_written;
            return FALSE;
        }

        bytes_remaining -= bytes_written;
        total_written   += bytes_written;
        pBuffer         += bytes_written;
    } while( bytes_remaining > 0 );

    *pnWritten = total_written;
    return TRUE;
}

xbool ReadPipe( HANDLE hPipe, void* pData, s32 nBytes, s32* pnRead )
{
    s32 bytes_read      = 0;
    s32 bytes_remaining = nBytes;
    s32 total_read      = 0;
    u8* pBuffer         = (u8*)pData;

    do
    {
        bytes_read = PCread( hPipe, pBuffer, bytes_remaining );
        if( bytes_read < 0 )
        {
            *pnRead = total_read;
            return FALSE;
        }

        bytes_remaining -= bytes_read;
        total_read      += bytes_read;
        pBuffer         += bytes_read;
    } while( bytes_remaining > 0 );

    *pnRead = total_read;
    return TRUE;
}

xbool ClosePipe( HANDLE hPipe )
{
    return PCclose( hPipe );
}

#endif

//==============================================================================
//  connect_message
//==============================================================================

void xtool::connect_message::ByteSwap( void )
{
    Version         = BYTESWAP( Version        );
    Platform        = BYTESWAP( Platform       );
    HeapStart       = BYTESWAP( HeapStart      );
    HeapEnd         = BYTESWAP( HeapEnd        );
    TicksPerSecond  = BYTESWAP( TicksPerSecond );
    BaselineTicks   = BYTESWAP( BaselineTicks  );
}

//==============================================================================
//  packet_header
//==============================================================================

void xtool::packet_header::ByteSwap( void )
{
    Type    = BYTESWAP( Type   );
    Length  = BYTESWAP( Length );
    CRC     = BYTESWAP( CRC    );
}

//==============================================================================
//  xtool
//==============================================================================

xtool::xtool( void )
{
    /*
    m_hPipe             = INVALID_HANDLE_VALUE;
    m_pPacketData       = NULL;
    m_PacketDataLength  = 0;

    m_pBufferBase       = NULL;
    m_pBuffer           = NULL;
    m_BufferOffset      = 0;
    m_BufferSize        = 0;

    m_Initialized       = FALSE;
    */
}

//==============================================================================

xtool::~xtool( void )
{
}

//==============================================================================

#if defined(TARGET_PS2) || defined(TARGET_GCN)
extern void* __heap_start;
extern void* __heap_end;
#endif

xbool xtool::Init( void )
{
    m_hPipe             = INVALID_HANDLE_VALUE;
    m_pPacketData       = NULL;
    m_PacketDataLength  = 0;

    m_pBufferBase       = NULL;
    m_pBuffer           = NULL;
    m_BufferOffset      = 0;
    m_BufferSize        = 0;

    m_Initialized       = FALSE;

    ASSERT( !m_Initialized );

    // Open the pipe
    m_hPipe = OpenPipe();

    // Connected?
    if( m_hPipe != INVALID_HANDLE_VALUE )
    {
        connect_message Connect;
        s32             Written;
        xbool           Success;

        // Setup connection message
        x_memset( &Connect, 0, sizeof(Connect) );
#ifdef BIG_ENDIAN
        Connect.Magic[0]        = 'G';
        Connect.Magic[1]        = 'B';
        Connect.Magic[2]        = 'D';
        Connect.Magic[3]        = 'X';
#else
        Connect.Magic[0]        = 'X';
        Connect.Magic[1]        = 'D';
        Connect.Magic[2]        = 'B';
        Connect.Magic[3]        = 'G';
#endif
        Connect.Version         = 1;
        Connect.Platform        = TARGET_PLATFORM;

#if defined(TARGET_PS2) || defined(TARGET_GCN)
        Connect.HeapStart       = (u32)__heap_start;
        Connect.HeapEnd         = (u32)__heap_end;
#else
        Connect.HeapStart       = 0;
        Connect.HeapEnd         = 0;
#endif
        Connect.TicksPerSecond  = x_GetTicksPerSecond();
        Connect.BaselineTicks   = x_GetTime();
        x_memset( Connect.ApplicationName, 0, sizeof(Connect.ApplicationName) );
        x_strncpy( Connect.ApplicationName, "", sizeof(Connect.ApplicationName)-1 );

        // Write the connection message & check for success
        Success = WritePipe( m_hPipe, &Connect, sizeof(Connect), &Written );
        if( !Success || (Written != sizeof(Connect)) )
        {
            // Connection failed, close the handle
            ClosePipe( m_hPipe );
            m_hPipe = INVALID_HANDLE_VALUE;
        }
        else
        {
            // Create buffer
            m_pBufferBase   = (u8*)s_Buffer;
            ASSERT( m_pBufferBase );
            m_pBuffer       = m_pBufferBase;
            m_BufferOffset  = 0;
            m_BufferSize    = BUFFER_SIZE;
        }
    }

    // Done initializing
    m_Initialized = TRUE;

    // Return TRUE if we succeeded
    return( m_hPipe != INVALID_HANDLE_VALUE );
}

//==============================================================================

void xtool::Kill( void )
{
    ASSERT( m_Initialized );

    if( IsConnected() )
    {
        // Flush the buffer
        Flush();

        // Close the handle
        ClosePipe( m_hPipe );
        m_hPipe = INVALID_HANDLE_VALUE;
    }

    // No longer initialized
    m_Initialized = FALSE;
}

//==============================================================================

xbool xtool::IsInitialized( void )
{
    return m_Initialized;
}

//==============================================================================

xbool xtool::Send( s32 PacketType, void* pData, s32 Length )
{
    ASSERT( m_Initialized );
    ASSERT( m_hPipe != INVALID_HANDLE_VALUE );

    s32         Written;
    xbool       Success;

    // CRC the data
    u32 CRC = 0;
    /*
    for( s32 i=0 ; i<Length ; i++ )
    {
    CRC = CRC ^ ((CRC >> 31) & 1);
    CRC <<= 1;
    CRC += ((u8*)pData)[i];
    }
    */

    // Write packet header
    m_PacketHeader.Type    = PacketType;
    m_PacketHeader.Length  = Length;
    m_PacketHeader.CRC     = CRC;
    Success = WritePipe( m_hPipe, &m_PacketHeader, sizeof(m_PacketHeader), &Written );
    if( !Success || (Written != sizeof(m_PacketHeader)) )
    {
        // Close pipe
        ClosePipe( m_hPipe );
        m_hPipe = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    // Write the message body
    Success = WritePipe( m_hPipe, pData, Length, &Written );
    if( !Success || (Written != Length) )
    {
        // Close pipe
        ClosePipe( m_hPipe );
        m_hPipe = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    // Completed successfully
    return TRUE;
}

//==============================================================================

xbool xtool::Receive( void )
{
    ASSERT( m_Initialized );
    ASSERT( m_hPipe != INVALID_HANDLE_VALUE );

    s32         Read;
    xbool       Success;

    // Read packet header
    Success = ReadPipe( m_hPipe, &m_PacketHeader, sizeof(m_PacketHeader), &Read );
    if( !Success || (Read != sizeof(m_PacketHeader)) )
    {
        // Close pipe
        ClosePipe( m_hPipe );
        m_hPipe = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    // Resize packet buffer if necessary
    if( m_PacketHeader.Length > m_PacketDataLength )
    {
        m_pPacketData      = x_realloc( m_pPacketData, m_PacketHeader.Length );
        m_PacketDataLength = m_PacketHeader.Length;
    }

    // Read the message body
    Success = ReadPipe( m_hPipe, m_pPacketData, m_PacketHeader.Length, &Read );
    if( !Success || (Read != m_PacketHeader.Length) )
    {
        // Close pipe
        ClosePipe( m_hPipe );
        m_hPipe = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    // Completed successfully
    return TRUE;
}

//==============================================================================

xbool xtool::IsConnected( void )
{
    ASSERT( m_Initialized );

    return( m_hPipe != INVALID_HANDLE_VALUE );
}

//==============================================================================

xbool xtool::Flush( void )
{
    ASSERT( m_Initialized );

    xbool Success = TRUE;

    // Check if the buffer contains data
    if( m_BufferOffset != 0 )
    {
        // Send current packet
        Success = Send( PACKET_LOG, m_pBufferBase, m_BufferOffset );

        // TODO: Double buffer swap?

        // Start new packet
        m_pBuffer       = m_pBufferBase;
        m_BufferOffset  = 0;
    }

    // Return success code
    return Success;
}

//==============================================================================

void xtool::EnsureSpace( s32 BytesNeeded )
{
    ASSERT( m_Initialized );
    ASSERT( (m_BufferSize-8) >= BytesNeeded );

    // Flush packet if not enough space left
    if( (m_BufferSize - m_BufferOffset) < BytesNeeded )
    {
        // Flush buffer
        Flush();
    }
}

//==============================================================================

void xtool::Log( s32 Severity, const char* pChannel, const char* pMessage, const char* pFile, s32 Line )
{
    ASSERT( m_Initialized );
    ASSERT( pChannel );

    // Exit if not connected to tool
    if( !IsConnected() )
        return;

    // Make sure we have valid strings
    if( !pMessage )
        pMessage = "<no message>";
    if( !pFile )
        pFile = "<unknown>";

    s32 LenChannel  = x_strlen( pChannel );
    s32 LenMessage  = x_strlen( pMessage );
    s32 LenFile     = x_strlen( pFile );
    s32 BytesNeeded = sizeof(u32) + sizeof(xtick) + sizeof(s32) + sizeof(u32) + 1 + LenChannel + 1 + LenMessage + 1 + LenFile + 1;
    BytesNeeded = (BytesNeeded + 3) & -4;

    // Check if there is enough space in the buffer for this log message
    EnsureSpace( BytesNeeded );

    // Cache buffer pointer
    u8* pBuffer = m_pBuffer;

    // Write the data
    *(u32*)pBuffer = LOG_TYPE_MESSAGE;
    pBuffer += 4;

    xtick Tick = x_GetTime();
    x_memcpy( pBuffer, &Tick, sizeof(Tick) );
    pBuffer += sizeof(Tick);

    *(u32*)pBuffer = (u32)x_GetThreadID();
    pBuffer += sizeof(u32);

    *(s32*)pBuffer = Line;
    pBuffer += sizeof(s32);

    *pBuffer++ = Severity;

    x_memcpy( pBuffer, pChannel, LenChannel+1 );
    pBuffer += LenChannel+1;

    x_memcpy( pBuffer, pMessage, LenMessage+1 );
    pBuffer += LenMessage+1;

    x_memcpy( pBuffer, pFile, LenFile+1 );
    pBuffer += LenFile+1;

    m_BufferOffset += BytesNeeded;
    m_pBuffer      += BytesNeeded;
}

//==============================================================================

void xtool::SetAppName( const char* pName )
{
    ASSERT( m_Initialized );

    // Exit if not connected to tool
    if( !IsConnected() )
        return;

    // Make sure we have valid strings
    if( !pName )
        pName = "<no name>";

    s32 LenName = x_strlen( pName );
    s32 BytesNeeded = sizeof(u32) + LenName + 1;
    BytesNeeded = (BytesNeeded + 3) & -4;

    // Check if there is enough space in the buffer for this message
    EnsureSpace( BytesNeeded );

    // Cache buffer pointer
    u8* pBuffer = m_pBuffer;

    // Write the data
    *(u32*)pBuffer = LOG_TYPE_APPLICATION_NAME;
    pBuffer += 4;

    x_memcpy( pBuffer, pName, LenName+1 );
    pBuffer += LenName+1;

    m_BufferOffset += BytesNeeded;
    m_pBuffer      += BytesNeeded;
}

//==============================================================================

void xtool::LogMessage( const char* pChannel, const char* pMessage, const char* pFile, s32 Line )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    Log( LOG_SEVERITY_MESSAGE, pChannel, pMessage, pFile, Line );
}

//==============================================================================

void xtool::LogWarning( const char* pChannel, const char* pMessage, const char* pFile, s32 Line )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    Log( LOG_SEVERITY_WARNING, pChannel, pMessage, pFile, Line );
}

//==============================================================================

void xtool::LogError( const char* pChannel, const char* pMessage, const char* pFile, s32 Line )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    Log( LOG_SEVERITY_ERROR, pChannel, pMessage, pFile, Line );
}

//==============================================================================

void xtool::LogAssert( const char* pMessage, const char* pFile, s32 Line )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    Log( LOG_SEVERITY_ASSERT, "ASSERT", pMessage, pFile, Line );
}

//==============================================================================

void xtool::LogTimerPush( )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    // Exit if not connected to tool
    if( !IsConnected() )
        return;

    // Get the number of buffer bytes needed
    s32 BytesNeeded = sizeof(u32) +
        sizeof(xtick) +
        sizeof(u32);
    BytesNeeded = (BytesNeeded + 3) & -4;

    // Check if there is enough space in the buffer for this log message
    EnsureSpace( BytesNeeded );

    // Cache buffer pointer
    u8* pBuffer = m_pBuffer;

    // Write the data
    *(u32*)pBuffer = LOG_TYPE_TIMER_PUSH;
    pBuffer += 4;

    xtick Tick = x_GetTime();
    x_memcpy( pBuffer, &Tick, sizeof(Tick) );
    pBuffer += sizeof(Tick);

    *(u32*)pBuffer = (u32)x_GetThreadID();
    pBuffer += sizeof(u32);

    m_BufferOffset += BytesNeeded;
    m_pBuffer      += BytesNeeded;
}

//==============================================================================

void xtool::LogTimerPop( const char* pChannel, f32 TimeLimitMS, const char* pMessage )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    // Exit if not connected to tool
    if( !IsConnected() )
        return;

    // Make sure we have valid strings
    if( !pChannel )
        pChannel = "<unknown>";

    // Get the number of buffer bytes needed
    s32 LenChannel  = x_strlen( pChannel );
    s32 LenMessage  = x_strlen( pMessage );
    s32 BytesNeeded  = sizeof(u32) +
        sizeof(xtick) +
        sizeof(u32) +
        sizeof(f32) +
        LenChannel + 1 +
        LenMessage + 1;
    BytesNeeded = (BytesNeeded + 3) & -4;

    // Check if there is enough space in the buffer for this log message
    EnsureSpace( BytesNeeded );

    // Cache buffer pointer
    u8* pBuffer = m_pBuffer;

    // Write the data
    *(u32*)pBuffer = LOG_TYPE_TIMER_POP;
    pBuffer += 4;

    xtick Tick = x_GetTime();
    x_memcpy( pBuffer, &Tick, sizeof(Tick) );
    pBuffer += sizeof(Tick);

    *(u32*)pBuffer = (u32)x_GetThreadID();
    pBuffer += sizeof(u32);

    *(f32*)pBuffer = TimeLimitMS;
    pBuffer += sizeof(f32);

    x_memcpy( pBuffer, pChannel, LenChannel+1 );
    pBuffer += LenChannel+1;

    x_memcpy( pBuffer, pMessage, LenMessage+1 );
    pBuffer += LenMessage+1;

    m_BufferOffset += BytesNeeded;
    m_pBuffer      += BytesNeeded;
}

//==============================================================================

void GetCallStack( s32& CallStackDepth, u32*& pCallStack )
{
#if defined( X_RETAIL )
    (void)pCallStack;
    CallStackDepth = 0;
#else
    x_DebugGetCallStack( CallStackDepth, pCallStack );
    CallStackDepth -= 4;
    pCallStack += 4;
    if( CallStackDepth < 0 )
        CallStackDepth = 0;
#endif
}

//==============================================================================

void xtool::LogMalloc( u32 Address, u32 Size, const char* pFile, s32 Line )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    // Exit if not connected to tool
    if( !IsConnected() )
        return;

    // Make sure we have valid strings
    if( !pFile )
        pFile = "<unknown>";

    // Get CallStack info
    s32     CallStackDepth;
    u32*    pCallStack;
    GetCallStack( CallStackDepth, pCallStack );

    // Get the number of buffer bytes needed
    s32 LenFile     = x_strlen( pFile );
    s32 BytesNeeded = sizeof(u32) +
        sizeof(u32) +
        sizeof(xtick) +
        sizeof(u32) +
        sizeof(u32) +
        sizeof(u32) +
        sizeof(s32) +
        sizeof(s32) + sizeof(u32)*CallStackDepth +
        LenFile + 1;
    BytesNeeded = (BytesNeeded + 3) & -4;

    // Check if there is enough space in the buffer for this log message
    EnsureSpace( BytesNeeded );

    // Cache buffer pointer
    u8* pBuffer = m_pBuffer;

    // Write the data
    *(u32*)pBuffer = LOG_TYPE_MEMORY;
    pBuffer += 4;

    *(u32*)pBuffer = LOG_MEMORY_MALLOC;
    pBuffer += 4;

    xtick Tick = x_GetTime();
    x_memcpy( pBuffer, &Tick, sizeof(Tick) );
    pBuffer += sizeof(Tick);

    *(u32*)pBuffer = (u32)x_GetThreadID();
    pBuffer += sizeof(u32);

    *(u32*)pBuffer = Address;
    pBuffer += sizeof(u32);

    *(u32*)pBuffer = Size;
    pBuffer += sizeof(u32);

    *(s32*)pBuffer = Line;
    pBuffer += sizeof(s32);

    *(s32*)pBuffer = CallStackDepth;
    pBuffer += sizeof(s32);

    x_memcpy( pBuffer, pCallStack, sizeof(u32)*CallStackDepth );
    pBuffer += sizeof(u32)*CallStackDepth;

    x_memcpy( pBuffer, pFile, LenFile+1 );
    pBuffer += LenFile+1;

    m_BufferOffset += BytesNeeded;
    m_pBuffer      += BytesNeeded;
}

//==============================================================================

void xtool::LogRealloc( u32 Address, u32 OldAddress, s32 Size, const char* pFile, s32 Line )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    // Exit if not connected to tool
    if( !IsConnected() )
        return;

    // Make sure we have valid strings
    if( !pFile )
        pFile = "<unknown>";

    // Get CallStack info
    s32     CallStackDepth;
    u32*    pCallStack;
    GetCallStack( CallStackDepth, pCallStack );

    // Get the number of buffer bytes needed
    s32 LenFile     = x_strlen( pFile );
    s32 BytesNeeded = sizeof(u32) +
        sizeof(u32) +
        sizeof(xtick) +
        sizeof(u32) +
        sizeof(u32) +
        sizeof(u32) +
        sizeof(u32) +
        sizeof(s32) +
        sizeof(s32) + sizeof(u32)*CallStackDepth +
        LenFile + 1;
    BytesNeeded = (BytesNeeded + 3) & -4;

    // Check if there is enough space in the buffer for this log message
    EnsureSpace( BytesNeeded );

    // Cache buffer pointer
    u8* pBuffer = m_pBuffer;

    // Write the data
    *(u32*)pBuffer = LOG_TYPE_MEMORY;
    pBuffer += 4;

    *(u32*)pBuffer = LOG_MEMORY_REALLOC;
    pBuffer += 4;

    xtick Tick = x_GetTime();
    x_memcpy( pBuffer, &Tick, sizeof(Tick) );
    pBuffer += sizeof(Tick);

    *(u32*)pBuffer = (u32)x_GetThreadID();
    pBuffer += sizeof(u32);

    *(u32*)pBuffer = Address;
    pBuffer += sizeof(u32);

    *(u32*)pBuffer = OldAddress;
    pBuffer += sizeof(u32);

    *(u32*)pBuffer = Size;
    pBuffer += sizeof(u32);

    *(s32*)pBuffer = Line;
    pBuffer += sizeof(s32);

    *(s32*)pBuffer = CallStackDepth;
    pBuffer += sizeof(s32);

    x_memcpy( pBuffer, pCallStack, sizeof(u32)*CallStackDepth );
    pBuffer += sizeof(u32)*CallStackDepth;

    x_memcpy( pBuffer, pFile, LenFile+1 );
    pBuffer += LenFile+1;

    m_BufferOffset += BytesNeeded;
    m_pBuffer      += BytesNeeded;
}

//==============================================================================

void xtool::LogFree( u32 Address, const char* pFile, s32 Line )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    // Exit if not connected to tool
    if( !IsConnected() )
        return;

    // Make sure we have valid strings
    if( !pFile )
        pFile = "<unknown>";

    // Get CallStack info
    s32     CallStackDepth;
    u32*    pCallStack;
    GetCallStack( CallStackDepth, pCallStack );

    // Get the number of buffer bytes needed
    s32 LenFile     = x_strlen( pFile );
    s32 BytesNeeded = sizeof(u32) +
        sizeof(u32) +
        sizeof(xtick) +
        sizeof(u32) +
        sizeof(u32) +
        sizeof(s32) +
        sizeof(s32) + sizeof(u32)*CallStackDepth +
        LenFile + 1;
    BytesNeeded = (BytesNeeded + 3) & -4;

    // Check if there is enough space in the buffer for this log message
    EnsureSpace( BytesNeeded );

    // Cache buffer pointer
    u8* pBuffer = m_pBuffer;

    // Write the data
    *(u32*)pBuffer = LOG_TYPE_MEMORY;
    pBuffer += 4;

    *(u32*)pBuffer = LOG_MEMORY_FREE;
    pBuffer += 4;

    xtick Tick = x_GetTime();
    x_memcpy( pBuffer, &Tick, sizeof(Tick) );
    pBuffer += sizeof(Tick);

    *(u32*)pBuffer = (u32)x_GetThreadID();
    pBuffer += sizeof(u32);

    *(u32*)pBuffer = Address;
    pBuffer += sizeof(u32);

    *(s32*)pBuffer = Line;
    pBuffer += sizeof(s32);

    *(s32*)pBuffer = CallStackDepth;
    pBuffer += sizeof(s32);

    x_memcpy( pBuffer, pCallStack, sizeof(u32)*CallStackDepth );
    pBuffer += sizeof(u32)*CallStackDepth;

    x_memcpy( pBuffer, pFile, LenFile+1 );
    pBuffer += LenFile+1;

    m_BufferOffset += BytesNeeded;
    m_pBuffer      += BytesNeeded;
}

//==============================================================================

void xtool::LogMemMark( const char* pMarkName )
{
    if( !m_Initialized )
        Init();
    ASSERT( m_Initialized );

    // Exit if not connected to tool
    if( !IsConnected() )
        return;

    // Make sure we have valid strings
    if( !pMarkName )
        pMarkName = "<unknown>";

    // Get CallStack info
    s32     CallStackDepth;
    u32*    pCallStack;
    GetCallStack( CallStackDepth, pCallStack );

    // Get the number of buffer bytes needed
    s32 LenMarkName = x_strlen( pMarkName );
    s32 BytesNeeded = sizeof(u32) +
        sizeof(u32) +
        sizeof(xtick) +
        sizeof(u32) +
        sizeof(s32) + sizeof(u32)*CallStackDepth +
        LenMarkName + 1;
    BytesNeeded = (BytesNeeded + 3) & -4;

    // Check if there is enough space in the buffer for this log message
    EnsureSpace( BytesNeeded );

    // Cache buffer pointer
    u8* pBuffer = m_pBuffer;

    // Write the data
    *(u32*)pBuffer = LOG_TYPE_MEMORY;
    pBuffer += 4;

    *(u32*)pBuffer = LOG_MEMORY_MARK;
    pBuffer += 4;

    xtick Tick = x_GetTime();
    x_memcpy( pBuffer, &Tick, sizeof(Tick) );
    pBuffer += sizeof(Tick);

    *(u32*)pBuffer = (u32)x_GetThreadID();
    pBuffer += sizeof(u32);

    *(s32*)pBuffer = CallStackDepth;
    pBuffer += sizeof(s32);

    x_memcpy( pBuffer, pCallStack, sizeof(u32)*CallStackDepth );
    pBuffer += sizeof(u32)*CallStackDepth;

    x_memcpy( pBuffer, pMarkName, LenMarkName+1 );
    pBuffer += LenMarkName+1;

    m_BufferOffset += BytesNeeded;
    m_pBuffer      += BytesNeeded;
}

//==============================================================================
#endif //!defined(X_RETAIL)
