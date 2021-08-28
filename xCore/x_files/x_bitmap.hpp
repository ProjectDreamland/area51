//==============================================================================
//  
//  x_bitmap.hpp
//  
//==============================================================================

#ifndef X_BITMAP_HPP
#define X_BITMAP_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

#ifndef X_COLOR_HPP
#include "x_color.hpp"
#endif

#ifndef X_STDIO_HPP
#include "x_stdio.hpp"
#endif

//==============================================================================
//  EXTERNS
//==============================================================================

#ifdef TARGET_XBOX
void  xbox_SetAllocationName( const char* pResourceName );
void* xbox_AllocateTexels   ( class xbitmap&,X_FILE* pFile );
void  xbox_FreeTexels       ( class xbitmap& );
#endif

//==============================================================================
//  TYPES
//==============================================================================

class xbitmap
{

//------------------------------------------------------------------------------
//  Public types

public:

    enum format
    {
        FMT_NULL = 0,

        // NON-PALETTE BASED FORMATS

        // RGB color data               // BGR color data
        FMT_32_RGBA_8888 =  1,          FMT_32_BGRA_8888 = 13,
        FMT_32_RGBU_8888 =  2,          FMT_32_BGRU_8888 = 14,
        FMT_32_ARGB_8888 =  3,          FMT_32_ABGR_8888 = 15,
        FMT_32_URGB_8888 =  4,          FMT_32_UBGR_8888 = 16,
        FMT_24_RGB_888   =  5,          FMT_24_BGR_888   = 17,
        FMT_16_RGBA_4444 =  6,          FMT_16_BGRA_4444 = 18,
        FMT_16_ARGB_4444 =  7,          FMT_16_ABGR_4444 = 19,
        FMT_16_RGBA_5551 =  8,          FMT_16_BGRA_5551 = 20,
        FMT_16_RGBU_5551 =  9,          FMT_16_BGRU_5551 = 21,
        FMT_16_ARGB_1555 = 10,          FMT_16_ABGR_1555 = 22,
        FMT_16_URGB_1555 = 11,          FMT_16_UBGR_1555 = 23,
        FMT_16_RGB_565   = 12,          FMT_16_BGR_565   = 24,
                              
        // CLUT (PALETTE) BASED FORMATS   
                                            
        // 8 bit data with RGB clut     // 8 bit data with BGR clut
        FMT_P8_RGBA_8888 = 25,          FMT_P8_BGRA_8888 = 37,        
        FMT_P8_RGBU_8888 = 26,          FMT_P8_BGRU_8888 = 38,        
        FMT_P8_ARGB_8888 = 27,          FMT_P8_ABGR_8888 = 39,        
        FMT_P8_URGB_8888 = 28,          FMT_P8_UBGR_8888 = 40,        
        FMT_P8_RGB_888   = 29,          FMT_P8_BGR_888   = 41,        
        FMT_P8_RGBA_4444 = 30,          FMT_P8_BGRA_4444 = 42,        
        FMT_P8_ARGB_4444 = 31,          FMT_P8_ABGR_4444 = 43,        
        FMT_P8_RGBA_5551 = 32,          FMT_P8_BGRA_5551 = 44,        
        FMT_P8_RGBU_5551 = 33,          FMT_P8_BGRU_5551 = 45,        
        FMT_P8_ARGB_1555 = 34,          FMT_P8_ABGR_1555 = 46,        
        FMT_P8_URGB_1555 = 35,          FMT_P8_UBGR_1555 = 47,        
        FMT_P8_RGB_565   = 36,          FMT_P8_BGR_565   = 48,          
                                          
        // 4 bit data with RGB clut     // 4 bit data with BGR clut
        FMT_P4_RGBA_8888 = 49,          FMT_P4_BGRA_8888 = 61,        
        FMT_P4_RGBU_8888 = 50,          FMT_P4_BGRU_8888 = 62,        
        FMT_P4_ARGB_8888 = 51,          FMT_P4_ABGR_8888 = 63,        
        FMT_P4_URGB_8888 = 52,          FMT_P4_UBGR_8888 = 64,        
        FMT_P4_RGB_888   = 53,          FMT_P4_BGR_888   = 65,        
        FMT_P4_RGBA_4444 = 54,          FMT_P4_BGRA_4444 = 66,        
        FMT_P4_ARGB_4444 = 55,          FMT_P4_ABGR_4444 = 67,        
        FMT_P4_RGBA_5551 = 56,          FMT_P4_BGRA_5551 = 68,        
        FMT_P4_RGBU_5551 = 57,          FMT_P4_BGRU_5551 = 69,        
        FMT_P4_ARGB_1555 = 58,          FMT_P4_ABGR_1555 = 70,        
        FMT_P4_URGB_1555 = 59,          FMT_P4_UBGR_1555 = 71,        
        FMT_P4_RGB_565   = 60,          FMT_P4_BGR_565   = 72,          

        // DXT Formats
        // DXT1 = 4-bit RGB plus 1 bit of alpha (punchthrough)
        // DXT2 = (NOT SUPPORTED) Like DXT3 with premultiplied alpha
        // DXT4 = (NOT SUPPORTED) Like DXT5 with premultiplied alpha
        // DXT3 = 4-bit RGB plus 4 bit alpha channel
        // DXT5 = 4-bit RGB plus 4-bit interpolated alpha
        FMT_DXT1         = 73,          FMT_DXT2         = 74,
        FMT_DXT3         = 75,          FMT_DXT4         = 76,
        FMT_DXT5         = 77,

        // Intensity formats

        FMT_A8          = 78, // used for pure alpha textures

        // TERMINATOR

        FMT_END_OF_LIST,

        // SPECIAL NAMES
        
        #ifdef LITTLE_ENDIAN
            FMT_XCOLOR = FMT_32_ARGB_8888,
        #endif
        #ifdef BIG_ENDIAN
            FMT_XCOLOR = FMT_32_BGRA_8888,
        #endif
    };

    struct format_info
    {
        format          Format;         // Format
        const char*     pString;        // String version of Format
        u8              ClutBased;      // Is a clut used?
        s8              BPP;            // Bits Per Pixel
        s8              BPC;            // Bits Per Color
        s8              BitsUsed;       // Bits used
        s8              RBits;          // Bits of Red
        s8              GBits;          // Bits of Green
        s8              BBits;          // Bits of Blue
        s8              ABits;          // Bits of Alpha
        s8              UBits;          // Bits of Unused
        s8              RShiftR;        // Shift right for Red
        s8              GShiftR;        // Shift right for Green
        s8              BShiftR;        // Shift right for Blue
        s8              AShiftR;        // Shift right for Alpha
        s8              UShiftR;        // Shift right for Unused
        s8              RShiftL;        // Shift left  for Red
        s8              GShiftL;        // Shift left  for Green
        s8              BShiftL;        // Shift left  for Blue
        s8              AShiftL;        // Shift left  for Alpha
        s8              UShiftL;        // Shift left  for Unused
        u32             RMask;          // Mask for Red
        u32             GMask;          // Mask for Green 
        u32             BMask;          // Mask for Blue  
        u32             AMask;          // Mask for Alpha 
        u32             UMask;          // Mask for Unused
    };

	// Bitmap info structure as returned for xbitmap::Info()
	struct info
	{
		s32		W;
		s32		H;
		s32		nMips;
		format	Format;
	};

/*
** This is not currently in use due to a bug in the PS2 compiler.
**
//------------------------------------------------------------------------------
//  Forward announcements

public:

    enum format;
**
**
*/

//------------------------------------------------------------------------------
//  Public functions

public:

                    xbitmap         ( void );        
                    xbitmap         ( const xbitmap& Bitmap );
                   ~xbitmap         ( void );

const   xbitmap&    operator =      ( const xbitmap& Bitmap );

        void        Setup           ( format    Format,    
                                      s32       Width,
                                      s32       Height,
                                      xbool     DataOwned,
                                      byte*     pPixelData,
                                      xbool     ClutOwned     = FALSE,
                                      byte*     pClutData     = NULL,
                                      s32       PhysicalWidth = -1,
                                      s32       nMips         =  0 );
                    
        void        Kill            ( void );
        
        //
        // Generic pixel/index access.
        //
        xcolor      GetPixelColor   ( s32 X, s32 Y, s32 Mip = 0 ) const;
        s32         GetPixelIndex   ( s32 X, s32 Y, s32 Mip = 0 ) const;
        xcolor      GetClutColor    ( s32 Index ) const;
        xcolor      GetBilinearColor( f32 ParamU, f32 ParamV, xbool Clamp=FALSE, s32 Mip = 0 ) const;

        void        SetPixelColor   ( xcolor Color, s32 X, s32 Y, s32 Mip = 0 );
        void        SetPixelIndex   ( s32    Index, s32 X, s32 Y, s32 Mip = 0 );
        void        SetClutColor    ( xcolor Color, s32 Index );
        void        SetPreSwizzled  ( void );

        void        Blit            ( s32 DestX, s32 DestY,
                                      s32 SrcX,  s32 SrcY,
                                      s32 Width, s32 Height,
                                      const xbitmap& SourceBitmap );

        //
        // File I/O.  File extension for xbitmap is ".xbmp".
        //
static	xbool		Info			( const char* pFileName, info& BitmapInfo );
        xbool       Load            ( X_FILE*     pFile     );
        xbool       Save            ( X_FILE*     pFile     ) const;
        xbool       Load            ( const char* pFileName );
        xbool       Save            ( const char* pFileName ) const;

        xbool       SaveTGA         ( const char* pFileName ) const;
        xbool       SaveMipsTGA     ( const char* pFileName ) const;
        xbool       SavePaletteTGA  ( const char* pFileName ) const;
        xbool       DumpSourceCode  ( const char* pFileName ) const;

        //
        // Basic interrogation functions.
        //
        s32         GetWidth        ( s32 Mip = 0 ) const;
        s32         GetHeight       ( s32 Mip = 0 ) const;
        s32         GetBPP          ( void ) const;
        s32         GetBPC          ( void ) const;
        xbool       HasAlphaBits    ( void ) const;
        xbool       IsClutBased     ( void ) const;
        xbool       IsCompressed    ( void ) const;
        xbool       IsPowerOf2      ( void ) const;

        //
        // Format conversion, mip building, intensity alpha.
        //
        void        ConvertFormat       ( format DestinationFormat );
        void        BuildMips           ( s32    MipsDesired = 15, xbool bForcePunchthrough = FALSE );
        void        MakeIntensityAlpha  ( void );
        void        Flip4BitNibbles     ( void );
        void        Unflip4BitNibbles   ( void );
        void        Resize              ( s32 NewW, s32 NewH, xbool bPunchThrough = FALSE );
        void        Crop                ( s32 x, s32 y, s32 w, s32 h );
        xbool       ReplaceAlphaWithRed ( void );

        //
        // Advanced interrogation functions (primarily for VRAM Manager).
        //
        s32             GetMipDataSize  ( s32 Mip = 0 ) const;
        s32             GetPWidth       ( s32 Mip = 0 ) const;
const   byte*           GetPixelData    ( s32 Mip = 0 ) const;
const   byte*           GetClutData     ( void ) const;
        s32             GetDataSize     ( void ) const;
        s32             GetClutSize     ( void ) const;
        u32             GetFlags        ( void ) const;
        s32             GetNMips        ( void ) const;
        format          GetFormat       ( void ) const;
const   format_info&    GetFormatInfo   ( void ) const;

static
const   format_info&    GetFormatInfo   ( format Format );
                            
        s32             GetVRAMID       ( void ) const;
        void            SetVRAMID       ( s32 VRAMID ) const;

        void            PS2SwizzleClut  ( void );
        void            PS2UnswizzleClut( void );

        void            GCNSwizzleData  ( void );
        void            GCNUnswizzleData( void );

        void            XboxSwizzleData ( void );

        void            Preregistered   ( void ){ m_Flags |= FLAG_XBOX_PRE_REGISTERED; }
        void            DePreregister   ( void ){ m_Flags &=~FLAG_XBOX_PRE_REGISTERED; }

//------------------------------------------------------------------------------
//  Protected types

    struct mip
    {
        s32     Offset;         // Offset in xbitmap::m_pData for a mip's data
        s16     Width;          // Width  of mip in pixels
        s16     Height;         // Height of mip in pixels
    };

    enum flags
    {
        FLAG_VALID                = ( 1 <<  0 ),
        FLAG_DATA_OWNED           = ( 1 <<  1 ),
        FLAG_CLUT_OWNED           = ( 1 <<  2 ),

        FLAG_PS2_CLUT_SWIZZLED    = ( 1 <<  8 ),
        FLAG_4BIT_NIBBLES_FLIPPED = ( 1 <<  9 ),

        FLAG_GCN_DATA_SWIZZLED    = ( 1 << 11 ),

        FLAG_XBOX_DATA_SWIZZLED   = ( 1 << 12 ),
        FLAG_XBOX_PRE_REGISTERED  = ( 1 << 13 ),
    };

//------------------------------------------------------------------------------
//  Protected functions

protected:

        void        CopyFrom        ( const xbitmap& Source );   
        void        Init            ( void );
        void        ToggleEndian    ( void );
        xcolor      ReadColor       ( byte* pRead ) const;
        void        WriteColor      ( byte* pWrite, xcolor Color );

        byte*       GCNSwizzleRGBA8 ( byte* pDest );
        byte*       GCNSwizzleRGB565( byte* pDest );
        byte*       GCNSwizzleRGBC8 ( byte* pDest );
        byte*       GCNSwizzleRGBC4 ( byte* pDest );
        void        GCNSwizzleDXT1  ( void );

        void        GCNPackTileRGBA8 ( u32 x, u32 y, u8* dstPtr, s32 Mip );
        void        GCNPackTileRGB565( u32 x, u32 y, u8* dstPtr, s32 Mip );
        void        GCNPackTile_C8   ( u32 x, u32 y, u8* dstPtr, s32 Mip );
        void        GCNPackTile_C4   ( u32 x, u32 y, u8* dstPtr, s32 Mip );
//------------------------------------------------------------------------------
//  Fields

protected:

        union                       // Bytes used in structure thus far
        {                           //  |
            byte*   pPixel;         // 04 : Pointer to pixel data
            mip*    pMip;           // 04 : Pointer to mip table
        }
        m_Data;

        byte*       m_pClut;        // 08 : Pointer to CLUT (Color LookUp Table)

        s32         m_DataSize;     // 12 : Size of pixel data in bytes
        s32         m_ClutSize;     // 16 : Size of clut  data in bytes
mutable s32         m_VRAMID;       // 20 : ID for use by the VRAM Manager
        s16         m_Width;        // 22 : Width  in pixels
        s16         m_Height;       // 24 : Height in pixels
        s16         m_PW;           // 26 : Physical width in pixels
mutable u16         m_Flags;        // 28 : Flags
        s8          m_NMips;        // 29 : Number of mips present in pixel data
        s8          m_Format;       // 30 : Data format, comes from enumeration

//------------------------------------------------------------------------------
//  Static data

public:

    static const format_info  m_FormatInfo[ FMT_END_OF_LIST ];

};

#include "Implementation/x_bitmap_inline.hpp"

//==============================================================================
#endif // X_BITMAP_HPP
//==============================================================================
