//==============================================================================
//  
//  e_VRAM.hpp
//
//==============================================================================

#ifndef E_VRAM_HPP
#define E_VRAM_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_bitmap.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
//  Public functions
//------------------------------------------------------------------------------

s32         vram_Register       ( const xbitmap& Bitmap  );
void        vram_Unregister     ( const xbitmap& Bitmap  );
void        vram_Unregister     (       s32      VRAM_ID );

void        vram_Activate       ( const xbitmap& Bitmap  )  X_SECTION(render);
void        vram_Activate       (       s32      VRAM_ID )  X_SECTION(render);
void        vram_Activate       ( void );

xbool       vram_IsActive       ( const xbitmap& Bitmap )   X_SECTION(render);

void        vram_Flush          ( void );

//------------------------------------------------------------------------------
//  Debugging functions
//------------------------------------------------------------------------------

s32         vram_GetNRegistered ( void );
s32         vram_GetRegistered  ( s32 ID );

void        vram_PrintStats     ( void );
void        vram_SanityCheck    ( void );

//------------------------------------------------------------------------------
//  Private functions
//------------------------------------------------------------------------------

void        vram_Init           ( void );
void        vram_Kill           ( void );

//------------------------------------------------------------------------------
//  PC specific functions
//------------------------------------------------------------------------------

#ifdef TARGET_PC

#include "D3DEngine\d3deng_private.hpp"

s32                 vram_LoadTexture    ( const char*    pFileName );

IDirect3DTexture9*  vram_GetSurface     ( const xbitmap& Bitmap  );
IDirect3DTexture9*  vram_GetSurface     (       s32      VRAM_ID );

// Register bitmap as a dudv bump map (bitmap should be grey scale height)
// Until xbitmap supports such formats, the conversion happens when creating the D3D texture 
s32                 vram_RegisterDuDv   ( const xbitmap& Bitmap ) ;

#endif

//------------------------------------------------------------------------------
//  XBOX specific functions
//------------------------------------------------------------------------------

#ifdef TARGET_XBOX

#include "xbox\xbox_private.hpp"

texture_factory::handle vram_GetSurface ( const xbitmap& Bitmap );
texture_factory::handle vram_GetSurface (       s32      VramId );

s32  vram_RegisterLocked( s32   Width, 
                          s32   Height, 
                          s32   BPP );

// Register bitmap as a dudv bump map (bitmap should be grey scale height)
// Until xbitmap supports such formats, the conversion happens when creating the D3D texture 
s32 vram_RegisterDuDv ( const xbitmap& Bitmap ) ;

#endif

//==============================================================================
#endif // E_VRAM_HPP
//==============================================================================
