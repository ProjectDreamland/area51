//=========================================================================
//
// PS2_VRAM.H
//
//=========================================================================

#ifndef PS2_VRAM_HPP
#define PS2_VRAM_HPP

#include "entropy.hpp"

//==============================================================================
// ps2-specific functions
//==============================================================================

s32  vram_AllocatePermanent ( s32               nPages          );
void vram_FreePermanent     ( void                              );
s32  vram_GetPermanentSize  ( void                              );
s32  vram_GetPermanentArea  ( void                              )   X_SECTION(render_deferred);
void vram_SetStartAddress   ( s32               BlockAddr       );
void vram_SetMipK           ( f32               MipK            );
s32  vram_AllocateBank      ( s32               nPages          );
void vram_FlushBank         ( s32               Bank            );
s32  vram_GetBankAddr       ( s32               Bank            );

void vram_Activate          ( s32               VramId          );
void vram_Activate          ( s32               VramId, 
                              s32               Bank            );

void vram_Upload            ( const xbitmap&    BMP,
                              s32               Bank            )   X_SECTION(render_deferred);
void vram_Activate          ( const xbitmap&    BMP,
                              s32               Bank            );

s32  vram_GetLog2           ( s32               N               );
f32  vram_Log2              ( f32               x               );
f32  vram_ComputeMipK       ( f32               WorldPixelSize,
                              f32               ScreenDist      )   X_SECTION(render_deferred);

s32  vram_RegisterLocked    ( s32               Width, 
                              s32               Height, 
                              s32               BPP );

void vram_Unregister        ( s32               VramId );

s32  vram_RegisterClut      ( const byte*       pClut,
                              s32               nColors         );
void vram_UnRegisterClut    ( s32               Handle          );
void vram_LoadClut          ( s32               Handle,
                              s32               Bank=0          );
s32  vram_GetClutBaseAddr   ( const xbitmap&    BMP             );
s32  vram_GetClutBaseAddr   ( s32               Handle          );
s32  vram_GetPixelBaseAddr  ( s32               VramId          );

void vram_LoadTextureImage  ( s32               Width,
                              s32               Height,
                              s32               PageWidth,
                              const byte*       pPixel,
                              s32               PixelFormat,
                              s32               VRAMPixelAddr   );
void vram_LoadClutData      ( s32               BPP,
                              s32               VRAMAddr,
                              const byte*       pClut           );

u64  vram_GetActiveTex0     ( const xbitmap&    BMP             )   X_SECTION(render_deferred);
u64  vram_GetActiveTex1     ( const xbitmap&    BMP             )   X_SECTION(render_deferred);
u64  vram_GetActiveMip1     ( const xbitmap&    BMP             )   X_SECTION(render_deferred);
u64  vram_GetActiveMip2     ( const xbitmap&    BMP             )   X_SECTION(render_deferred);

//=========================================================================
// VRAM INLINES
//=========================================================================

extern f32 VRAM_LogTable[256];

//=========================================================================

typedef union 
{
    f32 v;
    struct 
    {
        u32 mant : 23;
        u32 exp  :  8;
        u32 sign :  1;
    } s;
} ieee32;

//=========================================================================

inline
f32 vram_Log2( f32 x )
{
    ieee32 F;
    F.v = x;
    return ( ((f32)F.s.exp) + VRAM_LogTable[ (F.s.mant>>15)&0xFF ] );
}

//=========================================================================

inline
f32 vram_ComputeMipK( f32 WorldPixelSize, f32 ScreenDist )
{
    return( -vram_Log2( ScreenDist*WorldPixelSize ) );
}

//=========================================================================

#endif // PS2_VRAM_HPP
