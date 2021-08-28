
#ifndef _PADLIB_H_
#define _PADLIB_H_

#include "x_types.hpp"
#include <eetypes.h>
#include <libpad.h>

#define PAD_L_UP        SCE_PADLup  
#define PAD_L_DOWN      SCE_PADLdown
#define PAD_L_LEFT      SCE_PADLleft
#define PAD_L_RIGHT     SCE_PADLright
#define PAD_R_UP        SCE_PADRup
#define PAD_R_DOWN      SCE_PADRdown
#define PAD_R_LEFT      SCE_PADRleft
#define PAD_R_RIGHT     SCE_PADRright
#define PAD_L1          SCE_PADL1
#define PAD_L2          SCE_PADL2
#define PAD_R1          SCE_PADR1
#define PAD_R2          SCE_PADR2
#define PAD_START       SCE_PADstart
#define PAD_SELECT      SCE_PADselect
#define PAD_TRIANGLE    PAD_R_UP
#define PAD_SQUARE      PAD_R_LEFT
#define PAD_CROSS       PAD_R_DOWN
#define PAD_CIRCLE      PAD_R_RIGHT

int     PAD_Init            ( void );
void    PAD_Update          ( void );
int     PAD_GetButton       ( int Pad, int ButtonID ) ;
int     PAD_GetPrevButton   ( int Pad, int ButtonID ) ;
int     PAD_IsPressed       ( int Pad, int ButtonID );
int     PAD_WasPressed      ( int Pad, int ButtonID );
int     PAD_GetIsButtons    ( int Pad );
int     PAD_GetWasButtons   ( int Pad );
int     PAD_GetState        ( void );
void    PAD_ShowState       ( void );
void    PAD_GetLStick       ( int Pad, float& X, float& Y );
void    PAD_GetRStick       ( int Pad, float& X, float& Y );

#endif
