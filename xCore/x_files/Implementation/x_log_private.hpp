//==============================================================================
//
//  x_log_private.hpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================


#ifndef X_LOG_PRIVATE_HPP
#define X_LOG_PRIVATE_HPP

//==============================================================================
// Types
//==============================================================================

enum log_type
{
    LOG_TYPE_NULL,
    LOG_TYPE_MESSAGE,
    LOG_TYPE_WARNING,
    LOG_TYPE_ERROR,
    LOG_TYPE_ASSERT,
};

typedef void log_display_fn( const char* pChannel, log_type Type, const char* pMsg, 
                             const char* pFileName, s32 LineNumber );

//==============================================================================
// Log control structure
//==============================================================================

struct log_control
{
    bool    Enable;
    bool    EnableMessages;
    bool    EnableWarnings;
    bool    EnableErrors;
    bool    EnableAsserts;
    bool    EnableMemory;
    bool    ImmediateFlush;
    bool    EnableCallbackForMessages;
};

extern log_control g_LogControl; 

//==============================================================================
// Functions and defines
//==============================================================================

void    log_RegisterCallBack( log_display_fn* CallBack );

void    log_APP_NAME    ( const char* pName );

void    log_MESSAGE     ( const char* pChannel, const char* pFormatStr, ... );
void    log_WARNING     ( const char* pChannel, const char* pFormatStr, ... );
void    log_ERROR       ( const char* pChannel, const char* pFormatStr, ... );
void    log_ASSERT      ( const char* pMessage, const char* pFile, s32 Line );

void    log_TIMER_PUSH  ( );
void    log_TIMER_POP   ( const char* pChannel, f32 TimeLimitMS, const char* pFormatStr, ... );

void    clog_MESSAGE    ( xbool Condition, const char* pChannel, const char* pFormatStr, ... );
void    clog_WARNING    ( xbool Condition, const char* pChannel, const char* pFormatStr, ... );
void    clog_ERROR      ( xbool Condition, const char* pChannel, const char* pFormatStr, ... );
void    clog_ASSERT     ( xbool Condition, const char* pMessage, const char* pFile, s32 Line );

void    log_FL          ( const char* pFileName, s32 LineNumber );
void    log_LOCK        ( void );
void    log_UNLOCK      ( void );

void    log_MALLOC      ( void* Address, u32 Size, const char* pFile, s32 Line );
void    log_REALLOC     ( void* Address, void* OldAddress, u32 Size, const char* pFile, s32 Line );
void    log_FREE        ( void* Address, const char* pFile, s32 Line );
void    log_MEMMARK     ( const char* pMarkName );

void    log_FLUSH       ( void );

//==============================================================================
//  Make LOG calls do something or compile away to nothing
//==============================================================================

#if defined(X_LOGGING) && !defined(X_SUPPRESS_LOGS)

    #define LOG_APP_NAME    log_LOCK(), log_APP_NAME

    #define LOG_MESSAGE     log_LOCK(), log_FL( __FILE__, __LINE__ ), log_MESSAGE
    #define LOG_WARNING     log_LOCK(), log_FL( __FILE__, __LINE__ ), log_WARNING
    #define LOG_ERROR       log_LOCK(), log_FL( __FILE__, __LINE__ ), log_ERROR
    #define LOG_ASSERT      log_LOCK(), log_FL( __FILE__, __LINE__ ), log_ASSERT

    #define LOG_TIMER_PUSH  log_LOCK(), log_TIMER_PUSH
    #define LOG_TIMER_POP   log_LOCK(), log_TIMER_POP

    #define CLOG_MESSAGE    log_LOCK(), log_FL( __FILE__, __LINE__ ), clog_MESSAGE
    #define CLOG_WARNING    log_LOCK(), log_FL( __FILE__, __LINE__ ), clog_WARNING
    #define CLOG_ERROR      log_LOCK(), log_FL( __FILE__, __LINE__ ), clog_ERROR
    #define CLOG_ASSERT     log_LOCK(), log_FL( __FILE__, __LINE__ ), clog_ASSERT

    #define LOG_MALLOC      log_LOCK(), log_MALLOC
    #define LOG_REALLOC     log_LOCK(), log_REALLOC
    #define LOG_FREE        log_LOCK(), log_FREE
    #define LOG_MEMMARK     log_LOCK(), log_MEMMARK

    #define LOG_FLUSH       log_LOCK(), log_FLUSH

#else // defined(X_LOGGING) && !defined(X_SUPPRESS_LOGS)

    #if defined(VENDOR_SN) && !defined(_MSC_VER)

        // This is the way to make the calls disappear with GNU
        #define LOG_APP_NAME(...)   ((void)0)
        #define LOG_MESSAGE(...)    ((void)0)
        #define LOG_WARNING(...)    ((void)0)
        #define LOG_ERROR(...)      ((void)0)
        #define LOG_ASSERT(...)     ((void)0)

        #define LOG_TIMER_PUSH(...) ((void)0)
        #define LOG_TIMER_POP(...)  ((void)0)

        #define CLOG_MESSAGE(...)   ((void)0)
        #define CLOG_WARNING(...)   ((void)0)
        #define CLOG_ERROR(...)     ((void)0)
        #define CLOG_ASSERT(...)    ((void)0)

        #define LOG_MALLOC(...)     ((void)0)
        #define LOG_REALLOC(...)    ((void)0)
        #define LOG_FREE(...)       ((void)0)
        #define LOG_MEMMARK(...)    ((void)0)
        #define LOG_FLUSH(...)      ((void)0)

    #else // defined(VENDOR_SN) && !defined(_MSC_VER)

        // This is the way to make the calls disappear with VC
        inline void log_NULL(...) {}
        #define LOG_APP_NAME        log_NULL
        #define LOG_MESSAGE         log_NULL
        #define LOG_WARNING         log_NULL
        #define LOG_ERROR           log_NULL
        #define LOG_ASSERT          log_NULL

        #define LOG_TIMER_PUSH      log_NULL
        #define LOG_TIMER_POP       log_NULL

        #define CLOG_MESSAGE        log_NULL
        #define CLOG_WARNING        log_NULL
        #define CLOG_ERROR          log_NULL
        #define CLOG_ASSERT         log_NULL

        #define LOG_MALLOC          log_NULL
        #define LOG_REALLOC         log_NULL
        #define LOG_FREE            log_NULL
        #define LOG_MEMMARK         log_NULL
        #define LOG_FLUSH           log_NULL

    #endif // defined(VENDOR_SN) && !defined(_MSC_VER)

#endif // defined(X_LOGGING) && !defined(X_SUPPRESS_LOGS)

//==============================================================================
#endif // X_LOG_PRIVATE_HPP
//==============================================================================
