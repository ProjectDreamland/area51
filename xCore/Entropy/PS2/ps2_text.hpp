//=========================================================================
//
// E_TEXT.H
//
//=========================================================================

#ifndef E_TEXT_H
#define E_TEXT_H

//=========================================================================

#include "x_types.hpp"

//=========================================================================

void TEXT_Init          ( void );
void TEXT_Kill          ( void );

void TEXT_Render        ( void );
void TEXT_SwapBuffers   ( void );

void TEXT_Print         ( const char* pStr );
void TEXT_PrintXY       ( const char* pStr, s32 X, s32 Y );

/*
void x_printfxy( int X, int Y, char* Fstr, ... );
void x_printf( char* Fstr, ... );
*/

//=========================================================================

#endif
