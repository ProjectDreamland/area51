//=========================================================================
//
// PS2_MISC.HPP
//
//=========================================================================

#ifndef PS2_MISC_HPP
#define PS2_MISC_HPP

//=========================================================================

#include "eekernel.h"
#include "stdlib.h"
#include "stdio.h"
#include "eeregs.h"
#include "libgraph.h"
#include "libdma.h"
#include "libpkt.h"
#include "sifdev.h"
#include "libdev.h"
#include "sifrpc.h"
#include "libpad.h"
#include "libcdvd.h"
#include "libvu0.h"
#include "x_math.hpp"
#include "x_color.hpp"
#include "ps2_dlist.hpp"

//=========================================================================

#define VRAM_FRAME_BUFFER_WIDTH     512
#define VRAM_FRAME_BUFFER_HEIGHT    512
#define VRAM_FRAME_BUFFER_BPP       4

#define VRAM_ZBUFFER_WIDTH          VRAM_FRAME_BUFFER_WIDTH
#define VRAM_ZBUFFER_HEIGHT         VRAM_FRAME_BUFFER_HEIGHT
#define VRAM_ZBUFFER_BPP            4

#define VRAM_FRAME_BUFFER_SIZE      (VRAM_FRAME_BUFFER_WIDTH * VRAM_FRAME_BUFFER_HEIGHT * VRAM_FRAME_BUFFER_BPP)
#define VRAM_ZBUFFER_SIZE           (VRAM_ZBUFFER_WIDTH      * VRAM_ZBUFFER_HEIGHT      * VRAM_ZBUFFER_BPP     )

#define VRAM_FRAME_BUFFER_START     0
#define VRAM_FRAME_BUFFER0_START    (VRAM_FRAME_BUFFER_START)
#define VRAM_FRAME_BUFFER1_START    (VRAM_FRAME_BUFFER0_START + VRAM_FRAME_BUFFER_SIZE)
#define VRAM_ZBUFFER_START          (VRAM_FRAME_BUFFER_SIZE * 2)
#define VRAM_FREE_MEMORY_START      (VRAM_ZBUFFER_START + VRAM_ZBUFFER_SIZE)
#define VRAM_STENCIL_BUFFER_START   (VRAM_FREE_MEMORY_START)

#define VRAM_BUFFER_MEMORY_SIZE     ((VRAM_FRAME_BUFFER_SIZE * 2) + VRAM_ZBUFFER_SIZE)
#define VRAM_TOTAL_MEMORY_SIZE      (4 * 1024 * 1024)

#define VRAM_FREE_MEMORY_SIZE       (VRAM_TOTAL_MEMORY_SIZE - VRAM_BUFFER_MEMORY_SIZE)

#define VRAM_FBPTR(n)               (((n) / 4) / 2048)

//=========================================================================
// 2D Draw sprite routines that run immediately rather than batching like
// the draw system does, and also doesn't use a vector3. Since vector3's
// are not thread-safe, this is the method to use when rendering from
// outside of the main thread.
//=========================================================================

void    draw_SpriteImmediate        ( const vector2& Position,  // Hot spot (2D Left-Top), (2D Center)
                                      const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                                      const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                                      const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                                      const xcolor & Color );
void    draw_RectImmediate          ( const irect&   Rect,
                                      xcolor         Color = XCOLOR_WHITE,
                                      xbool          IsWire = FALSE );
void    draw_GouraudRectImmediate   ( const irect&   Rect,
                                      const xcolor&  c1,
                                      const xcolor&  c2,
                                      const xcolor&  c3,
                                      const xcolor&  c4,
                                      xbool          DoAdditive );

//=========================================================================
// GS CONTEXT SWITCHING
//=========================================================================

void    eng_PushGSContext           ( s32 Context );
void    eng_PopGSContext            ( void );
s32     eng_GetGSContext            ( void );
s32     eng_GetFrameIndex           ( void );
s32     eng_GetFrameCount           ( void );
u64     eng_GetFRAMEReg             ( void );
u64     eng_GetFRAMEReg             ( s32 Buffer ) ; // 0=back buffer, 1=front buffer
u32     eng_GetFrameBufferAddr      ( s32 Buffer ) ;

//=========================================================================
// MAIN DISPLAY LIST
//=========================================================================

extern dlist DLIST;

//=========================================================================

void    eng_EnableScreenClear       ( xbool Enabled );
void    eng_ClearFrontBuffer        ( u32 Mask = 0xFF000000 );
void    eng_WriteToBackBuffer       ( u32 Mask = 0xFFFFFFFF );

//=========================================================================
// Magical macro which defines the entry point of the app. Make sure that 
// the use has the entry point: void AppMain( s32 argc, char* argv[] ) 
// define somewhere.
//=========================================================================
void ps2eng_Begin( s32 argc, char* argv[] );
void ps2eng_End( void );

#define AppMain AppMain( s32 argc, char* argv[] );                          \
int main( s32 argc, char* argv[] )                                          \
{                                                                           \
    ps2eng_Begin( argc, argv );                                             \
    x_StartMain(AppMain, argc, argv );                                      \
    ps2eng_End();                                                           \
    return 0;                                                               \
}                                                                           \
void AppMain                                                                \


//=========================================================================
// INLINES FOR BETTER PERFORMANCE
//=========================================================================

extern s32 s_ContextStack[8];
extern s32 s_ContextStackIndex;
extern s32 s_Context;
extern s32 s_FrameCount;

//=========================================================================

inline
void    eng_PushGSContext   ( s32 Context )
{
    ASSERT( (Context==0) || (Context==1) );
    ASSERT( s_ContextStackIndex < 7 );
    s_ContextStackIndex++;
    s_ContextStack[s_ContextStackIndex] = Context;
    s_Context = Context;
}

//=========================================================================

inline
void    eng_PopGSContext    ( void )
{
    if( s_ContextStackIndex==0 ) return;
    s_ContextStackIndex--;
    s_Context = s_ContextStack[s_ContextStackIndex];
}

//=========================================================================

inline
s32     eng_GetGSContext    ( void )
{
    return s_Context;
}

//=========================================================================

inline
s32     eng_GetFrameCount   ( void )
{
    return s_FrameCount;
}

//=========================================================================

#include "ps2_dma.hpp"
#include "ps2_vifgif.hpp"
#include "ps2_framebuf.hpp"
#include "ps2_text.hpp"
#include "ps2_font.hpp"
#include "ps2_vram.hpp"
#include "ps2_gsreg.hpp"

#endif
