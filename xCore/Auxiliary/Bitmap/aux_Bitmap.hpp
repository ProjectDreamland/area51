//==============================================================================
//
//  aux_Bitmap.hpp
//
//==============================================================================

#ifndef AUX_BITMAP_HPP
#define AUX_BITMAP_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_BITMAP_HPP
#include "x_bitmap.hpp"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================

xbool   auxbmp_Info         (                        const char* pFileName, xbitmap::info& BitmapInfo );
xbool   auxbmp_Load         (       xbitmap& Bitmap, const char* pFileName );
xbool   auxbmp_Save         ( const xbitmap& Bitmap, const char* pFileName );

void    auxbmp_ConvertToD3D (       xbitmap& Bitmap );
void    auxbmp_ConvertToPS2 (       xbitmap& Bitmap );
void    auxbmp_ConvertToGCN (       xbitmap& Bitmap, xbool Swizzle = TRUE );

void    auxbmp_ConvertToNative(     xbitmap& Bitmap );

xbool   auxbmp_LoadD3D      (       xbitmap& Bitmap, const char* pFileName );
xbool   auxbmp_LoadPS2      (       xbitmap& Bitmap, const char* pFileName );
xbool   auxbmp_LoadGCN      (       xbitmap& Bitmap, const char* pFileName );
xbool   auxbmp_LoadNative   (       xbitmap& Bitmap, const char* pFileName );

void    auxbmp_SetupDefault (       xbitmap& Bitmap );

xbool   auxbmp_SetColorAlpha(       xbitmap& Bitmap, xcolor Clr, u8 NewAlpha );

void    auxbmp_MakeDxDvBump (       xbitmap& Dest, const xbitmap& Source, platform Platfrom );

        // The MakeDiffuseFrom--- functions generate
        // a RGB888 xbitmap
void    auxbmp_MakeDiffuseFromRGB(  xbitmap& Dest, const xbitmap& Source );
void    auxbmp_MakeDiffuseFromA  (  xbitmap& Dest, const xbitmap& Source );
void    auxbmp_MakeDiffuseFromR  (  xbitmap& Dest, const xbitmap& Source );
void    auxbmp_MakeDiffuseFromG  (  xbitmap& Dest, const xbitmap& Source );
void    auxbmp_MakeDiffuseFromB  (  xbitmap& Dest, const xbitmap& Source );

void    auxbmp_CreateAFromR      (  xbitmap& Dest, const xbitmap& Source );

		// MergeDiffuseAndOpacity Creates an RGBA888 xbitmap from a diffuse and opacity map (Opacity map must be exactly the same size as diffuse map)
void    auxbmp_MergeDiffuseAndOpacity( xbitmap& Dest, const xbitmap& diffuse, const xbitmap& opacity, xbool useRedChannelForOpacity = FALSE );

        // forces alpha values to 0x00 or 0xff - sets color of transparent texel to be average of surrounding opaque texels
void    auxbmp_ForcePunchthrough (  xbitmap& Bitmap, u32 alphaThreshold = 0x80 ); 

// Compresses bitmap by splitting it up into 2 bitmaps:
// A base bitmap (a quarter of the size of the original) and a 
// luminance bitmap (same size as original, but only 4 bits per pixel)
// NOTE: The format of the base bitmap is the same as the source
// The compression ratio is approx 5.3 for 32bit textures, and 1.7 for 8bit textures
void auxbmp_CompressPS2( const xbitmap& SrcBMP, xbitmap& BaseBMP, xbitmap& LumBMP, xbool Subtractive ) ;

// Creates a dot3 map from the source grey scale height map
void auxbmp_ConvertToDot3(const xbitmap& SrcBMP, xbitmap& DstBMP ) ;

//==-------------------------------
//  Compression
//==-------------------------------
typedef void auxcompress_fn( f32 PercentComplete );
void    auxbmp_SetCompressCallback  ( auxcompress_fn* pFn );

//==-------------------------------
//  Xbox/PC compression
//==-------------------------------

xbool   auxbmp_IsPunchthruAlpha ( xbitmap& BMP );
xbitmap auxbmp_ConvertRGBToA8   ( xbitmap& BMP );
f32     auxbmp_GetDotProduct    ( xbitmap& BMP,xbitmap& DXT );
xbitmap auxbmp_CompressRect     ( xbitmap& BMP,s32 DXTFormat );
void    auxbmp_Compress         ( xbitmap& BMP,const char* pName,s32 nMips );
void    auxbmp_Decompress       ( xbitmap& BMP );

//==============================================================================
#endif // BITMAP_HPP
//==============================================================================
