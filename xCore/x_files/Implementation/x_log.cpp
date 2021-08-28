//==============================================================================
//
//  x_log.cpp
//
//==============================================================================

#ifndef X_FILES_HPP
#include "../x_files.hpp"
#endif

#ifndef X_THREADS_HPP
#include "../x_threads.hpp"
#endif

#ifndef X_LOG_HPP
#include "../x_log.hpp"
#endif

#ifndef X_TOOL_HPP
#include "x_tool_private.hpp"
#endif

#ifndef X_LOG_PRIVATE_HPP
#include "x_log_private.hpp"
#endif

//==============================================================================
//  Make this whole file go away in X_RETAIL builds
//==============================================================================

#if defined(X_LOGGING)

#if defined(cgalley) || defined(rbrannon) || defined(mreed) || defined(dstewart) ||defined(shird)
static bool s_EnableMemory = true;
#else
static bool s_EnableMemory = false;
#endif

//==============================================================================
// Global variables
//==============================================================================
log_control  g_LogControl = {
#if defined(TARGET_XBOX)
                              false,            // Disable logging system - must be disabled until NETFS initializes.
#else
                              true,             // Enable logging system
#endif
                              true,             // Enable messages
                              true,             // Enable warnings
                              true,             // Enable errors
                              true,             // Enable asserts
                              s_EnableMemory,   // Enable memory
                              false,            // Immediate Flush
                              false             // Enable callback for messages
                            };

//==============================================================================
// Static variables
//==============================================================================

static xbool            s_Initialized           = FALSE;
static xbool            s_Connected             = FALSE;
static char             s_Mutex[sizeof(xmutex)] PS2_ALIGNMENT(4);
static xmutex*          s_pMutex                = NULL;
static const char*      s_pFileName;
static u32              s_LineNumber;
static log_display_fn*  s_DisplayCallBack       = NULL;
static s32              s_DisplayCallBackThread = -1;

//==============================================================================

void log_Init( void )
{
    // Check if xtool is initialized or initializes successfully
    xbool IsInitialized = g_Tool.IsInitialized();
    if( (IsInitialized && g_Tool.IsConnected()) || (!IsInitialized && g_Tool.Init()) )
    {
        s_Connected = TRUE;
    }

    // Create a mutex for thread safety in logging
#undef new
    new((void*)&s_Mutex) xmutex;
    s_pMutex        = (xmutex*)&s_Mutex;

    // Done
    s_Initialized   = TRUE;
}

//==============================================================================

void log_Kill( void )
{
    s_Initialized = FALSE;
}

//==============================================================================

void log_FLUSH( void )
{
    if( !g_LogControl.Enable )
        return;

    ASSERT( s_Initialized );

    if( s_Connected )
        g_Tool.Flush();

    // Was locked in macros in x_log_private.hpp
    log_UNLOCK();
}

//==============================================================================

void log_TTY( const char* pChannel, const char* pSeverity, const char* pMessage )
{
    (void)pChannel;
    (void)pSeverity;
    (void)pMessage;
/*
    if( !g_LogControl.Enable )
        return;

    x_DebugMsg( "%s %6.3f \"%s\" \"%s\" \"%s\" %d\n",
                pSeverity,
                (f32)x_GetTimeSec(),
                pChannel,
                pMessage,
                s_pFileName,
                s_LineNumber
              );
*/
}

//==============================================================================

void log_FL( const char* pFileName, s32 LineNumber )
{
    if( !g_LogControl.Enable )
        return;

    s_pFileName  = pFileName;
    s_LineNumber = LineNumber;
}

//==============================================================================

void log_LOCK( void )
{
    if( !g_LogControl.Enable )
        return;

    if( !s_Initialized )
        log_Init();

    s_pMutex->Enter();
}

//==============================================================================

void log_UNLOCK( void )
{
    if( !g_LogControl.Enable )
        return;

    s_pMutex->Exit();
}

//==============================================================================

void log_RegisterCallBack( log_display_fn* CallBack )
{
    s_DisplayCallBackThread = x_GetThreadID();
    s_DisplayCallBack = CallBack;
}

//==============================================================================

void log_APP_NAME( const char* pName )
{
    if( g_LogControl.Enable )
    {
        ASSERT( s_Initialized );

        if( s_Connected )
        {
            g_Tool.SetAppName( pName );
        }
    }

    // Was locked in macros in x_log_private.hpp
    log_UNLOCK();
}

//==============================================================================

static void vlog_MESSAGE( const char* pChannel, const char* pFormatStr, x_va_list& Args )
{
    if( g_LogControl.Enable && g_LogControl.EnableMessages )
    {
        ASSERT( s_Initialized );

        xvfs XVFS( pFormatStr, Args );

        if( s_DisplayCallBack && (x_GetThreadID() == s_DisplayCallBackThread) && g_LogControl.EnableCallbackForMessages )
        {
            s_DisplayCallBack( pChannel, LOG_TYPE_MESSAGE, XVFS, s_pFileName, s_LineNumber );
        }

        if( s_Connected )
        {
            g_Tool.LogMessage( pChannel, XVFS, s_pFileName, s_LineNumber );
        }
    }
}

void clog_MESSAGE( xbool Condition, const char* pChannel, const char* pFormatStr, ... )
{
    if( Condition )
    {
        // fetch the variable arguments
        x_va_list   Args;
        x_va_start( Args, pFormatStr );

        // Call the base function
        vlog_MESSAGE( pChannel, pFormatStr, Args );
    }

    // Was locked in macros in x_log_private.hpp
    if( g_LogControl.ImmediateFlush )
        log_FLUSH();
    else
        log_UNLOCK();
}

void log_MESSAGE( const char* pChannel, const char* pFormatStr, ... )
{
    // fetch the variable arguments
    x_va_list   Args;
    x_va_start( Args, pFormatStr );

    // Call the base function
    vlog_MESSAGE( pChannel, pFormatStr, Args );

    // Was locked in macros in x_log_private.hpp
    if( g_LogControl.ImmediateFlush )
        log_FLUSH();
    else
        log_UNLOCK();
}

//==============================================================================

static void vlog_WARNING( const char* pChannel, const char* pFormatStr, x_va_list& Args )
{
    if( g_LogControl.Enable && g_LogControl.EnableWarnings )
    {
        ASSERT( s_Initialized );

        xvfs XVFS( pFormatStr, Args );

        if( s_DisplayCallBack && (x_GetThreadID() == s_DisplayCallBackThread) )
        {
            s_DisplayCallBack( pChannel, LOG_TYPE_WARNING, XVFS, s_pFileName, s_LineNumber );
        }

        if( s_Connected )
        {
            g_Tool.LogWarning( pChannel, XVFS, s_pFileName, s_LineNumber );
        }
    }
}

void clog_WARNING( xbool Condition, const char* pChannel, const char* pFormatStr, ... )
{
    if( Condition )
    {
        // fetch the variable arguments
        x_va_list   Args;
        x_va_start( Args, pFormatStr );

        // Call the base function
        vlog_WARNING( pChannel, pFormatStr, Args );
    }

    // Was locked in macros in x_log_private.hpp
    if( g_LogControl.ImmediateFlush )
        log_FLUSH();
    else
        log_UNLOCK();
}

void log_WARNING( const char* pChannel, const char* pFormatStr, ... )
{
    if( g_LogControl.Enable && g_LogControl.EnableWarnings )
    {
        ASSERT( s_Initialized );

        // fetch the variable arguments
        x_va_list   Args;
        x_va_start( Args, pFormatStr );
        xvfs XVFS( pFormatStr, Args );

        if( s_DisplayCallBack )
        {
            s_DisplayCallBack( pChannel, LOG_TYPE_WARNING, XVFS, s_pFileName, s_LineNumber );
        }

        if( s_Connected )
        {
            g_Tool.LogWarning( pChannel, XVFS, s_pFileName, s_LineNumber );
        }
    }

    // Was locked in macros in x_log_private.hpp
    if( g_LogControl.ImmediateFlush )
        log_FLUSH();
    else
        log_UNLOCK();
}

//==============================================================================

static void vlog_ERROR( const char* pChannel, const char* pFormatStr, x_va_list& Args )
{
    if( g_LogControl.Enable && g_LogControl.EnableErrors )
    {
        ASSERT( s_Initialized );

        xvfs XVFS( pFormatStr, Args );

        if( s_DisplayCallBack && (x_GetThreadID() == s_DisplayCallBackThread) )
        {
            s_DisplayCallBack( pChannel, LOG_TYPE_ERROR, XVFS, s_pFileName, s_LineNumber );
        }

        if( s_Connected )
        {
            g_Tool.LogError( pChannel, XVFS, s_pFileName, s_LineNumber );
        }
    }
}

//==============================================================================

void clog_ERROR( xbool Condition, const char* pChannel, const char* pFormatStr, ... )
{
    if( Condition )
    {
        // fetch the variable arguments
        x_va_list   Args;
        x_va_start( Args, pFormatStr );

        // Call the base function
        vlog_ERROR( pChannel, pFormatStr, Args );
    }

    // Was locked in macros in x_log_private.hpp
    if( g_LogControl.ImmediateFlush )
        log_FLUSH();
    else
        log_UNLOCK();
}

//==============================================================================

void log_ERROR( const char* pChannel, const char* pFormatStr, ... )
{
    if( g_LogControl.Enable && g_LogControl.EnableErrors )
    {
        ASSERT( s_Initialized );

        // fetch the variable arguments
        x_va_list   Args;
        x_va_start( Args, pFormatStr );
        xvfs XVFS( pFormatStr, Args );

        if( s_DisplayCallBack )
        {
            s_DisplayCallBack( pChannel, LOG_TYPE_ERROR, XVFS, s_pFileName, s_LineNumber );
        }

        if( s_Connected )
        {
            g_Tool.LogError( pChannel, XVFS, s_pFileName, s_LineNumber );
        }
    }

    // Was locked in macros in x_log_private.hpp
    if( g_LogControl.ImmediateFlush )
        log_FLUSH();
    else
        log_UNLOCK();
}

//==============================================================================

void log_ASSERT( const char* pMessage, const char* pFile, s32 Line )
{
    if( g_LogControl.Enable && g_LogControl.EnableAsserts )
    {
        ASSERT( s_Initialized );

        if( s_DisplayCallBack )
        {
            s_DisplayCallBack( "ASSERT", LOG_TYPE_ASSERT, pMessage, pFile, Line );
        }

        if( s_Connected )
        {
            g_Tool.LogAssert( pMessage, pFile, Line );
        }
    }

    // Was locked in macros in x_log_private.hpp
    if( g_LogControl.ImmediateFlush )
        log_FLUSH();
    else
        log_UNLOCK();
}

//==============================================================================

void log_TIMER_PUSH(void )
{
    if( g_LogControl.Enable && g_LogControl.EnableWarnings )
    {
        ASSERT( s_Initialized );

        if( s_Connected )
        {
            g_Tool.LogTimerPush( );
        }
    }

    // Was locked in macros in x_log_private.hpp
    log_UNLOCK();
}

//==============================================================================

void log_TIMER_POP( const char* pChannel, f32 TimeLimitMS, const char* pFormatStr, ... )
{
    if( g_LogControl.Enable && g_LogControl.EnableWarnings )
    {
        ASSERT( s_Initialized );

        // fetch the variable arguments
        x_va_list   Args;
        x_va_start( Args, pFormatStr );
        xvfs XVFS( pFormatStr, Args );

        if( s_Connected )
        {
            g_Tool.LogTimerPop( pChannel, TimeLimitMS, XVFS );
        }
    }

    // Was locked in macros in x_log_private.hpp
    log_UNLOCK();
}

//==============================================================================

void log_MALLOC( void* Address, u32 Size, const char* pFile, s32 Line )
{
    if( g_LogControl.Enable && g_LogControl.EnableMemory )
    {
        ASSERT( s_Initialized );

        if( s_Connected )
        {
            g_Tool.LogMalloc( (u32)Address, Size, pFile, Line );
        }
    }

    // Was locked in macros in x_log_private.hpp
    log_UNLOCK();
}

//==============================================================================

void log_REALLOC( void* Address, void* OldAddress, u32 Size, const char* pFile, s32 Line )
{
    if( g_LogControl.Enable && g_LogControl.EnableMemory )
    {
        ASSERT( s_Initialized );

        if( s_Connected )
        {
            g_Tool.LogRealloc( (u32)Address, (u32)OldAddress, Size, pFile, Line );
        }
    }

    // Was locked in macros in x_log_private.hpp
    log_UNLOCK();
}

//==============================================================================

void log_FREE( void* Address, const char* pFile, s32 Line )
{
    if( g_LogControl.Enable && g_LogControl.EnableMemory )
    {
        ASSERT( s_Initialized );

        if( s_Connected )
        {
            g_Tool.LogFree( (u32)Address, pFile, Line );
        }
    }

    // Was locked in macros in x_log_private.hpp
    log_UNLOCK();
}

//==============================================================================

void log_MEMMARK( const char* pMarkName )
{
    if( g_LogControl.Enable && g_LogControl.EnableMemory )
    {
        ASSERT( s_Initialized );

        if( s_Connected )
        {
            g_Tool.LogMemMark( pMarkName );
        }
    }

    // Was locked in macros in x_log_private.hpp
    log_UNLOCK();
}

//==============================================================================

#endif //defined(X_LOGGING)

