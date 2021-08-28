//=========================================================================
//
// E_TEXT.HPP
//
//=========================================================================

#ifndef E_TEXT_HPP
#define E_TEXT_HPP

//=========================================================================

#include "x_types.hpp"

//=========================================================================

#if defined( X_RETAIL ) && !defined( X_QA )

#define text_PtToCell( a,b,c,d )
#define text_CellToPt( a,b,c,d )
#define text_Print( a )
#define text_PrintXY( a,b,c )
#define text_PrintPixelXY( a,b,c )
#define text_On()
#define text_Off()
#define text_SetParams( a,b,c,d,e,f,g )
#define text_GetParams( a,b,c,d,e,f,g )
#define text_PushColor( a )
#define text_PopColor()
#define text_GetColor()

#define text_Init()
#define text_Kill()
#define text_ClearBuffers()
#define text_Render()

#define text_BeginRender()
#define text_RenderStr()
#define text_EndRender()

//=========================================================================

#else

// Public calls
void text_PtToCell      ( s32 PixelX, s32 PixelY, s32& CellX, s32& CellY );
void text_CellToPt      ( s32 CellX, s32 CellY, s32& PixelX, s32& PixelY );
void text_Print         ( const char* pStr );
void text_PrintXY       ( const char* pStr, s32 CellX, s32 CellY );
void text_PrintPixelXY  ( const char* pStr, s32 PixelX, s32 PixelY );
void text_On            ( void );
void text_Off           ( void );

void text_SetParams     ( s32 ScreenWidth,
                          s32 ScreenHeight,
                          s32 XBorderWidth,
                          s32 YBorderWidth,
                          s32 CharacterWidth,
                          s32 CharacterHeight,
                          s32 NScrollLines );

void text_GetParams     ( s32& ScreenWidth,
                          s32& ScreenHeight,
                          s32& XBorderWidth,
                          s32& YBorderWidth,
                          s32& CharacterWidth,
                          s32& CharacterHeight,
                          s32& NScrollLines );

void   text_PushColor   ( xcolor C );
void   text_PopColor    ( void );
xcolor text_GetColor    ( void );

//=========================================================================

// Private platform independent functions
void text_Init          ( void );
void text_Kill          ( void );
void text_ClearBuffers  ( void );
void text_Render        ( void );

//=========================================================================

// Private platform specific functions
void text_BeginRender   ( void );
void text_RenderStr     ( char* pStr, s32 NChars, xcolor Color, s32 PixelX, s32 PixelY );
void text_EndRender     ( void );

//=========================================================================

#endif // ! X_RETAIL

#endif
