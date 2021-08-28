//==============================================================================
//
//  x_log.hpp
//
//==============================================================================

#ifndef X_LOG_HPP
#define X_LOG_HPP

//==============================================================================
// Logging Functions
//==============================================================================

void    LOG_APP_NAME    ( const char* pName );

void    LOG_ERROR       ( const char* pChannel, const char* pFormatStr, ... );
void    LOG_WARNING     ( const char* pChannel, const char* pFormatStr, ... );
void    LOG_MESSAGE     ( const char* pChannel, const char* pFormatStr, ... );
void    LOG_ASSERT      ( const char* pMessage, const char* pFile, s32 Line );

void    CLOG_ERROR      ( xbool Condition, const char* pChannel, const char* pFormatStr, ... );
void    CLOG_WARNING    ( xbool Condition, const char* pChannel, const char* pFormatStr, ... );
void    CLOG_MESSAGE    ( xbool Condition, const char* pChannel, const char* pFormatStr, ... );
void    CLOG_ASSERT     ( xbool Condition, const char* pMessage, const char* pFile, s32 Line );

void    LOG_TIMER_PUSH  ( );
void    LOG_TIMER_POP   ( const char* pChannel, f32 TimeLimit, const char* pFormatStr, ... );

void    LOG_MEMMARK     ( const char* pMarkName );

void    LOG_FLUSH       ( void );

//==============================================================================
//  PRIVATE
//==============================================================================

#include "implementation/x_log_private.hpp"

//==============================================================================
#endif // X_LOG_HPP
//==============================================================================
