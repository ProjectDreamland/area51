//==============================================================================
//  
//  x_bitmap_convert.cpp
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_BITMAP_HPP
#include "..\x_bitmap.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "..\x_memory.hpp"
#endif

//==============================================================================

#if !( defined( TARGET_PS2 ) && defined( CONFIG_RETAIL ) )

//==============================================================================
//  PROTOTYPES FROM QUANTIZER AND COLOR MAPPER
//==============================================================================

void quant_Begin            ( void );
void quant_SetPixels        ( const xcolor* pColor, s32 NColors );
void quant_End              ( xcolor* pPalette, s32 NColors, xbool UseAlpha );

void cmap_Begin     ( const xcolor* pColor, s32 NColors, xbool UseAlpha );
s32  cmap_GetIndex  ( xcolor Color );
void cmap_End       ( void );

#define USE_CMAP

//==============================================================================
//  FUNCTIONS
//==============================================================================

static
xcolor* xbmp_DecodeToColor( const byte*           pSource, 
                                  xbitmap::format SourceFormat, 
                                  s32             Count )
{
    xcolor* pWrite;
    xcolor* pResult;
    const xbitmap::format_info& Format = xbitmap::GetFormatInfo( SourceFormat );

    pResult = (xcolor*)x_malloc( Count * sizeof( xcolor ) );
    pWrite  = pResult;

    //
    // Decode 32 bit color information.
    //
    if( Format.BPC == 32 )
    {
        u32* pRead = (u32*)pSource;

        // Since we are reading 32 bit data, we can take a few shortcuts.  For 
        // example, we know that all components have a full 8 bits, so we don't 
        // need to worry about padding out or even "shifting left" when 
        // isolating the component during the read.

        while( Count > 0 )
        {
            pWrite->R  = (u8)((*pRead & Format.RMask) >> Format.RShiftR);
            pWrite->G  = (u8)((*pRead & Format.GMask) >> Format.GShiftR);
            pWrite->B  = (u8)((*pRead & Format.BMask) >> Format.BShiftR);
            pWrite->A  = (u8)((*pRead & Format.AMask) >> Format.AShiftR);
            pWrite->A |= (u8)((*pRead & Format.UMask) >> Format.UShiftR);

            pRead++;
            pWrite++;
            Count--;
        }
    }

    //
    // Decode 24 bit color information.
    //
    if( Format.BPC == 24 )
    {
        byte* pRead = (byte*)pSource;

        // There are only two 24 bit configurations: RGB and BGR.  
        // Just write code for each case.

        // We can't just ask if "DestFormat == FMT_24_RGB_888" because the
        // target format may be paletted like FMT_P8_RGB_888.  So, we will
        // check the right shift on the red.  If it is 16, then we have a
        // RGB format.  And if it is 0, then we have a BGR format.

        if( Format.RShiftR == 16 )
        {
            while( Count > 0 )
            {
                pWrite->R = *pRead++;
                pWrite->G = *pRead++;
                pWrite->B = *pRead++;
                pWrite->A = 255;
                pWrite++;
                Count--;
            }
        }
        
        if( Format.RShiftR == 0 )
        {
            while( Count > 0 )
            {
                pWrite->B = *pRead++;
                pWrite->G = *pRead++;
                pWrite->R = *pRead++;
                pWrite->A = 255;
                pWrite++;
                Count--;
            }
        }
    }

    //
    // Decode 16 bit color information.
    //
    if( Format.BPC == 16 )
    {
        u16* pRead;
        u32  R, G, B, A;
        s32  Bits;

        // Since we are reading 16 bit data, we can take a few shortcuts (though
        // not as many as if we were reading 32 bit data).  We know that the
        // smallest R,G,B values are 4 bit, so we will only have to "duplicate 
        // and pad" once in order to fill out all 8 bits of data.  However, the A
        // component can have as little as 1 bit, so we are stuck for the full
        // ride there.

        while( Count > 0 )
        {
            pRead = (u16*)pSource;

            R   = *pRead;
            R  &= Format.RMask;
            R >>= Format.RShiftR;
            R <<= Format.RShiftL;
            R  |= (R >> Format.RBits);

            G   = *pRead;
            G  &= Format.GMask;
            G >>= Format.GShiftR;
            G <<= Format.GShiftL;
            G  |= (G >> Format.GBits);

            B   = *pRead;
            B  &= Format.BMask;
            B >>= Format.BShiftR;
            B <<= Format.BShiftL;
            B  |= (B >> Format.BBits);

            if( Format.ABits )
            {
                A   = *pRead;
                A  &= Format.AMask;
                A >>= Format.AShiftR;
                A <<= Format.AShiftL;
    
                Bits = Format.ABits;
                while( Bits < 8 )
                {
                    A |= (A >> Bits);
                    Bits <<= 1;
                }
            }
            else
            {
                A = 0xFF;
            }

            pWrite->Set( (u8)R, (u8)G, (u8)B, (u8)A );

            pSource += 2;
            pWrite++;
            Count--;
        }
    }

    //
    // Done!
    //
    return( pResult );
}

//==============================================================================

static
void xbmp_EncodeFromColor(       byte*           pDestination, 
                                 xbitmap::format DestinationFormat, 
                           const xcolor*         pSource, 
                                 s32             Count )
{
    const xcolor* pRead = pSource;
    const xbitmap::format_info& Format = xbitmap::GetFormatInfo( DestinationFormat );

    u32 R, G, B, A, U;

    U = Format.UMask;

    //
    // NOTE - The shift information in xbitmap::m_FormatInfo is for "reading". 
    //        Since we are "writing", we reverse the shift directions and order.
    //        As it turns out, we do not have to do any masking for the RGB 
    //        data.  The shifts will get rid of any extra bits.
    //

    //
    // Encode 32 bit color information.
    //
    if( Format.BPC == 32 )
    {
        u32* pWrite = (u32*)pDestination;

        while( Count > 0 )
        {
            R = (u32)pRead->R;
            G = (u32)pRead->G;
            B = (u32)pRead->B;
            A = (u32)pRead->A;

            R >>= Format.RShiftL;
            R <<= Format.RShiftR;

            G >>= Format.GShiftL;
            G <<= Format.GShiftR;

            B >>= Format.BShiftL;
            B <<= Format.BShiftR;
            
            A >>= Format.AShiftL;
            A <<= Format.AShiftR;
            A  &= Format.AMask;

            *pWrite = R | G | B | A | U;

            pWrite++;
            pRead++;
            Count--;
        }
    }

    //
    // Encode 24 bit color information.
    //
    if( Format.BPC == 24 )
    {
        byte* pWrite = pDestination;

        // There are only two 24 bit configurations: RGB and BGR.  
        // Just write code for each case.

        // We can't just ask if "DestFormat == FMT_24_RGB_888" because the
        // target format may be paletted like FMT_P8_RGB_888.  So, we will
        // check the right shift on the red.  If it is 16, then we have a
        // RGB format.  And if it is 0, then we have a BGR format.

        if( Format.RShiftR == 16 )
        {
            while( Count > 0 )
            {
                *pWrite++ = pRead->R;
                *pWrite++ = pRead->G;
                *pWrite++ = pRead->B;
                pRead++;
                Count--;
            }
        }
                
        if( Format.RShiftR == 0 )
        {
            while( Count > 0 )
            {
                *pWrite++ = pRead->B;
                *pWrite++ = pRead->G;
                *pWrite++ = pRead->R;
                pRead++;
                Count--;
            }
        }
    }

    //
    // Encode 16 bit color information.
    //
    if( Format.BPC == 16 )
    {
        u32  Encoded;
        u16* pWrite = (u16*)pDestination;

        while( Count > 0 )
        {
            R = (u32)pRead->R;
            G = (u32)pRead->G;
            B = (u32)pRead->B;
            A = (u32)pRead->A;

            R >>= Format.RShiftL;
            R <<= Format.RShiftR;

            G >>= Format.GShiftL;
            G <<= Format.GShiftR;

            B >>= Format.BShiftL;
            B <<= Format.BShiftR;
            
            A >>= Format.AShiftL;
            A <<= Format.AShiftR;
            A  &= Format.AMask;

            Encoded = R | G | B | A | U;
            *pWrite = (u16)Encoded;
            
            pWrite++;
            pRead++;
            Count--;
        }
    }
}

//==============================================================================

static
xcolor* xbmp_DecodeIndexToColor( const byte*   pSource,
                                 const xcolor* pPalette,
                                       s32     SourceBitsPer,
                                       s32     Count )
{
    xcolor*     pWrite;
    xcolor*     pResult;
    const byte* pRead = pSource;

    pResult = (xcolor*)x_malloc( Count * sizeof( xcolor ) );
    pWrite  = pResult;

    // There are only two sizes for indexed information: 4 bit and 8 bit.  We 
    // have code for each case.

    if( SourceBitsPer == 4 )
    {
        while( Count > 0 )
        {   
            *pWrite++ = pPalette[ *pRead >> 4    ];
            *pWrite++ = pPalette[ *pRead &  0x0F ];
            pRead += 1;
            Count -= 2;
        }
    }

    if( SourceBitsPer == 8 )
    {
        while( Count > 0 )
        {   
            *pWrite++ = pPalette[ *pRead++ ];
            Count--;
        }
    }

    return( pResult );
}

//==============================================================================

inline s32 xbmp_ColorSquareDistance( const xcolor& C1, const xcolor& C2 )
{
    return( (((s32)C1.R - (s32)C2.R) * ((s32)C1.R - (s32)C2.R)) +
            (((s32)C1.G - (s32)C2.G) * ((s32)C1.G - (s32)C2.G)) +
            (((s32)C1.B - (s32)C2.B) * ((s32)C1.B - (s32)C2.B)) +
            (((s32)C1.A - (s32)C2.A) * ((s32)C1.A - (s32)C2.A)) );    
}

//==============================================================================

static
void xbmp_EncodeColorToIndex(       byte*   pDestination,
                                    s32     DestinationBitsPer, 
                              const xcolor* pSource,
                                    s32     Count )
{
    byte*         pWrite = pDestination;
    const xcolor* pRead  = pSource;

    // Build color table.
    while( Count > 0 )
    {
        // Find the best match for the Read color in the Palette.
        s32 BestIndex = cmap_GetIndex( *pRead );

        // We have separate code for encoding 4 bit and 8 bit.
        if( DestinationBitsPer == 4 )
        {
            if( Count & 1 )
            {
                // Odd pixels are in lower nibble.
                *pWrite |= BestIndex;
                pWrite++;
            }
            else
            {
                // Even pixels are in upper nibble.
                *pWrite = (byte)(BestIndex << 4);
            }
        }
        else
        {
            *pWrite = (byte)BestIndex;
            pWrite++;
        }

        pRead++;
        Count--;
    }
}

//==============================================================================

static
void xbmp_ConvertIndexData(       byte* pDestination, 
                                  s32   DestinationBitsPer,
                            const byte* pSource, 
                                  s32   SourceBitsPer, 
                                  s32   Count )
{
    if( DestinationBitsPer == SourceBitsPer )
    {
        // Caller should probably have checked this before calling.  Oh well.
        x_memcpy( pDestination, pSource, (Count * SourceBitsPer) >> 3 );
    }

    if( (DestinationBitsPer == 8) && (SourceBitsPer == 4) )
    {
        while( Count > 0 )
        {
            *pDestination++ = *pSource >> 4;
            *pDestination++ = *pSource &  0x0F;
            pSource++;
            Count -= 2;
        }
    }

    if( (DestinationBitsPer == 4) && (SourceBitsPer == 8) )
    {
        while( Count > 0 )
        {
            *pDestination  = *pSource++ << 4;
            *pDestination |= *pSource++ &  0x0F;
            pDestination++;
            Count -= 2;
        }
    }
}

//==============================================================================

void xbitmap::ConvertFormat( xbitmap::format DestinationFormat )
{
    const format_info& OldFormat = m_FormatInfo[ m_Format ];
    const format_info& NewFormat = m_FormatInfo[ DestinationFormat ];

    ASSERT( (DestinationFormat > FMT_NULL       ) && 
            (DestinationFormat < FMT_END_OF_LIST) );

//  ASSERT( m_NMips == 0 );     // We don't handle conversions with mips.  Yet.

    if( DestinationFormat == m_Format )
        return;

    byte*   pOldData       = NULL;
    byte*   pNewData       = NULL;
    xcolor* pColorData     = NULL;
    s32     DataPixels;
    s32     DataBytes;

    byte*   pOldPalette    = NULL;
    byte*   pNewPalette    = NULL;
    xcolor* pColorPalette  = NULL;
    s32     PaletteEntries = 0;
    s32     PaletteBytes   = 0;

    // Unswizzle as needed.
    if( m_Flags & FLAG_PS2_CLUT_SWIZZLED )
        PS2UnswizzleClut();

    // Unflip as needed
    if ( m_Flags & FLAG_4BIT_NIBBLES_FLIPPED )
        Unflip4BitNibbles() ;

    // There are four conversion paths:
    //  - Color to Color
    //  - Color to Paletted
    //  - Paletted to Color
    //  - Paletted to Paletted
    //
    // The paths have many common portions, so, rather than code each path 
    // individually, we will attack each portion.
    //
    //  Operation                                  C-C     C-P     P-C     P-P
    //  ----------------------------------------------------------------------
    //  Decode palette                                              X       X
    //  Decode color data                           X       X
    //  Build a palette from colors                         X
    //  Convert indexed data to color data                          X
    //  Encode color data                           X               X
    //  Convert indexed data to indexed data                                X
    //  Force color data into a palette                     X
    //  Encode palette data                                                 X

    //--------------------------------------------------------------------------
    //
    // Set up some values which we will need no matter what.
    //
    DataPixels = m_PW * m_Height;
    DataBytes  = (DataPixels * NewFormat.BPP) >> 3;
    pNewData   = (byte*)x_malloc( DataBytes );
    pOldData   = m_Data.pPixel;
    pOldData  += m_NMips ? m_Data.pMip[0].Offset : 0;

    //--------------------------------------------------------------------------
    //
    // Do we need to decode a palette?  This applies to:
    //  - Paletted to Color
    //  - Paletted to Paletted
    //
    if( OldFormat.ClutBased )
    {
        PaletteEntries = m_ClutSize / (OldFormat.BPC >> 3);
        pOldPalette    = m_pClut;
        pColorPalette  = xbmp_DecodeToColor( pOldPalette, 
                                            (xbitmap::format)m_Format,
                                            PaletteEntries );
    }

    //--------------------------------------------------------------------------
    //
    // Do we need to decode color data?  This applies to:
    //  - Color to Color
    //  - Color to Paletted
    //
    if( !(OldFormat.ClutBased) )
    {
        // Decode to xcolor format.
        pColorData = xbmp_DecodeToColor( pOldData, 
                                         (xbitmap::format)m_Format, 
                                         DataPixels );
    }

    //--------------------------------------------------------------------------
    //
    // Do we need to build a palette from colors?  This applies to:
    //  - Color to Paletted
    //
    if( !(OldFormat.ClutBased) && (NewFormat.ClutBased) )
    {
        PaletteEntries = (NewFormat.BPP < 8) ? 16 : 256;
        pColorPalette  = (xcolor*)x_malloc( sizeof(xcolor) * PaletteEntries );

        quant_Begin();
        quant_SetPixels( pColorData, DataPixels );
        quant_End( pColorPalette, PaletteEntries, (NewFormat.ABits>0) );
    }

    //--------------------------------------------------------------------------
    //
    // Do we need to convert indexed data to color data?  This applies to:
    //  - Paletted to Color
    //
    if( (OldFormat.ClutBased) && !(NewFormat.ClutBased) )
    {
        // Expand the indexed data into color data using the color palette.
        pColorData = xbmp_DecodeIndexToColor( pOldData, 
                                              pColorPalette, 
                                              OldFormat.BPP, 
                                              DataPixels );

        // Don't need the color palette any more.
        x_free( pColorPalette );
    }

    //--------------------------------------------------------------------------
    //
    // Do we need to encode color data?  This applies to:
    //  - Color to Color
    //  - Paletted to Color
    //
    if( !(NewFormat.ClutBased) )
    {
        xbmp_EncodeFromColor( pNewData, 
                              DestinationFormat, 
                              pColorData, 
                              DataPixels );

        // Don't need the color data any more.
        x_free( pColorData );
    }

    //--------------------------------------------------------------------------
    // 
    // Do we need to convert indexed pixel data?  This applies to:
    //  - Paletted to Paletted
    //
    if( (OldFormat.ClutBased) && (NewFormat.ClutBased) )
    {
        xbmp_ConvertIndexData( pNewData, NewFormat.BPP, 
                               pOldData, OldFormat.BPP, 
                               DataPixels );

        // Careful!  We may adjust the PaletteEntries here!
        if( (OldFormat.BPP == 8) && (NewFormat.BPP == 4) )
            PaletteEntries = 16;
    }

    //--------------------------------------------------------------------------
    //
    // Do we need to force color data into a palette?  This applies to:
    //  - Color to Paletted
    //
    if( !(OldFormat.ClutBased) && (NewFormat.ClutBased) )
    {
        // Convert palette.
        PaletteBytes = (PaletteEntries * NewFormat.BPC) >> 3;
        pNewPalette  = (byte*)x_malloc( PaletteBytes );
        xbmp_EncodeFromColor( pNewPalette, 
                              DestinationFormat, 
                              pColorPalette, 
                              PaletteEntries );

        // Build indices.
        cmap_Begin( pColorPalette, PaletteEntries, (NewFormat.ABits>0) );
        xbmp_EncodeColorToIndex( pNewData,
                                 NewFormat.BPP,
                                 pColorData,
                                 DataPixels );
        cmap_End();

        // Don't need the color data or palette any more.
        x_free( pColorData );
        x_free( pColorPalette );
    }

    //--------------------------------------------------------------------------
    //
    // Do we need to encode palette data?  This applies to:
    //  - Paletted to Paletted
    //
    if( OldFormat.ClutBased && NewFormat.ClutBased )
    {
        // Get number of colors in new palette in case the new palette has more 
        // entries than the old!
        s32 NewPaletteEntries = 1 << NewFormat.BPP;

        PaletteBytes = (NewPaletteEntries * NewFormat.BPC) >> 3;
        pNewPalette  = (byte*)x_malloc( PaletteBytes );
        xbmp_EncodeFromColor( pNewPalette, 
                              DestinationFormat, 
                              pColorPalette, 
                              PaletteEntries );

        // Don't need the color palette any more.
        x_free( pColorPalette );
    }

    //--------------------------------------------------------------------------
    // 
    // We are done converting data!  Now we just need to update the header 
    // information.
    //

    if( m_Flags & FLAG_DATA_OWNED )   
        x_free( m_Data.pPixel );

    if( (m_Flags & FLAG_CLUT_OWNED) && (m_pClut) )   
        x_free( m_pClut );

    m_Data.pPixel = pNewData;
    m_DataSize    = DataBytes;
    m_NMips       = 0;
    m_Flags      |= FLAG_DATA_OWNED;

    if( NewFormat.ClutBased )
    {
        m_pClut    = pNewPalette;
        m_ClutSize = PaletteBytes;
        m_Flags   |= FLAG_CLUT_OWNED;
    }
    else
    {
        m_pClut    = NULL;
        m_ClutSize = 0;
        m_Flags   &= ~FLAG_CLUT_OWNED;
    }

    m_Format = (s8)DestinationFormat;
}

//==============================================================================
xbool g_CompileMipTest = FALSE;
static xcolor s_MipTestColor[7] =
{
    XCOLOR_WHITE,
    XCOLOR_RED,
    XCOLOR_GREEN,
    XCOLOR_BLUE,
    XCOLOR_YELLOW,
    XCOLOR_PURPLE,
    XCOLOR_AQUA,
};

//==============================================================================

static
xcolor* xbmp_GenerateColorMip( const xcolor* pSource, s32 W, s32 H, s32 Mip )
{
    s32     Shift;      // See comments below.    
    s32     x, y;       // Coordinate in mip image.
    s32     X, Y;       // Coordinate in original source image.
    s32     w, h;       // Width and height of mip image.
    xcolor* pMipImage;  // Memory for mip image.
    xcolor* pWrite;     // Write pointer for mip image.

    //
    // To make a mip, we sum up each data channel for a bunch of pixels and then
    // divide to get the average.  Rather than divide, we will shift.  If the 
    // mip is 2, we sum up 16 pixels and then shift down by 4 which is the same
    // as dividing by 16.  Of course, we prefer to get a "rounded" divide.  If
    // we were dealing with floats, we would add 0.5f before truncating.  To
    // avoid getting into floats, though, we can do this:  shift down by 1 less
    // than the needed amount, add 1, then shift off the last bit.  This will
    // have desired effect.
    //
    Shift = (Mip<<1) - 1;

    w = W >> Mip;
    h = H >> Mip;

    pMipImage = (xcolor*)x_malloc( w * h * sizeof(xcolor) );
    ASSERT( pMipImage );

    pWrite = pMipImage;

    for( y = 0; y < h; y++ )
    for( x = 0; x < w; x++ )
    {
        s32 R, G, B, A;
        R = G = B = A = 0;

        for( Y = (y << Mip); Y < ((y+1) << Mip); Y++ )
        for( X = (x << Mip); X < ((x+1) << Mip); X++ )
        {
            xcolor C = pSource[ (Y * W) + X ];
            R += C.R;
            G += C.G;
            B += C.B;
            A += C.A;
        }              

        pWrite->R = ((R >> Shift) + 1) >> 1;
        pWrite->G = ((G >> Shift) + 1) >> 1;
        pWrite->B = ((B >> Shift) + 1) >> 1;
        pWrite->A = ((A >> Shift) + 1) >> 1;
        
        if( g_CompileMipTest )
            pWrite[0] = s_MipTestColor[ MIN(Mip,6) ]; 

        pWrite++;
    }

    return( pMipImage );
}

//==============================================================================
//
//  This version of GenerateColorMip is specially designed for "punch through"
//  alpha textures.  Or, said another way, where there is only 1 bit of Alpha 
//  used (even if more bits are available).  Hence the name "A1".
//

static
xcolor* xbmp_GenerateColorMipA1( const xcolor* pSource, s32 W, s32 H, s32 Mip )
{
    s32     x, y;       // Coordinate in mip image.
    s32     X, Y;       // Coordinate in original source image.
    s32     w, h;       // Width and height of mip image.
    xcolor* pMipImage;  // Memory for mip image.
    xcolor* pWrite;     // Write pointer for mip image.

    w = W >> Mip;
    h = H >> Mip;

    pMipImage = (xcolor*)x_malloc( w * h * sizeof(xcolor) );
    ASSERT( pMipImage );

    pWrite = pMipImage;

    for( y = 0; y < h; y++ )
    for( x = 0; x < w; x++ )
    {
        s32 TR, TG, TB, Transparent;
        s32 OR, OG, OB, Opaque;

        TR = TG = TB = Transparent = 0;
        OR = OG = OB = Opaque      = 0;

        for( Y = (y << Mip); Y < ((y+1) << Mip); Y++ )
        for( X = (x << Mip); X < ((x+1) << Mip); X++ )
        {
            xcolor C = pSource[ (Y * W) + X ];
            if( C.A >= 128 )
            {
                OR += C.R;
                OG += C.G;
                OB += C.B;
                Opaque += 1;
            }
            else
            {   
                TR += C.R;
                TG += C.G;
                TB += C.B;
                Transparent += 1;
            }
        }              
        
        // If a majority of the pixels were transparent, then the result is
        // transparent and we use only the average of the transparent pixels.
        // Otherwise, the pixel is opaque and we average the opaque pixels.
        if( Transparent > Opaque )
        {
            pWrite->R = TR / Transparent;
            pWrite->G = TG / Transparent;
            pWrite->B = TB / Transparent;
            pWrite->A = 0;
        }
        else
        {
            pWrite->R = OR / Opaque;
            pWrite->G = OG / Opaque;
            pWrite->B = OB / Opaque;
            pWrite->A = 255;
        }

        if( g_CompileMipTest )
        {
            pWrite->R = s_MipTestColor[ MIN(Mip,6) ].R; 
            pWrite->G = s_MipTestColor[ MIN(Mip,6) ].G; 
            pWrite->B = s_MipTestColor[ MIN(Mip,6) ].B; 
        }

        pWrite++;
    }

    return( pMipImage );
}

//==============================================================================
// #define DUMP_MIPS

#ifdef DUMP_MIPS
#include "..\x_string.hpp"
static s32 GlobalMipCount = 0;
#endif

void xbitmap::BuildMips( s32 Mips, xbool bForcePunchthrough )
{
    s32 i;
    s32 PaletteEntries;
    s32 MipDataSize[16];
    mip Mip        [16];

    #ifdef DUMP_MIPS
    GlobalMipCount = ((GlobalMipCount/100)*100)+100;
    #endif

    xcolor* pColorMip      = NULL;
    xcolor* pColorData     = NULL;
    xcolor* pColorPalette  = NULL;

    s32     NewDataSize;
    byte*   pNewData       = NULL;
    byte*   pOldData       = (byte*)GetPixelData( 0 );

    xbool   PunchThruAlpha;

    const format_info& Format = m_FormatInfo[ m_Format ];

    ASSERT( (Mips == 0) || (((m_Width  - 1) & (m_Width )) == 0) );  // Must be power of 2.
    ASSERT( (Mips == 0) || (((m_Height - 1) & (m_Height)) == 0) );  // Must be power of 2.
    ASSERT( Mips >= 0 );

    // Is the image even large enough to bother with?
    if( (m_Width <= 8) || (m_Height <= 8) )
    {
        ASSERT( m_NMips == 0 );
        return;
    }

    // Compute information for Mip[0].
    Mip[0].Width   = m_Width;
    Mip[0].Height  = m_Height;
    MipDataSize[0] = (Mip[0].Width * Mip[0].Height * Format.BPP) >> 3;

    // Compute information for remaining mips.
    for( i = 1; i <= Mips; i++ )
    {
        Mip[i].Width   = Mip[i-1].Width   >> 1;     //  1/2 the width
        Mip[i].Height  = Mip[i-1].Height  >> 1;     //  1/2 the height
        MipDataSize[i] = MipDataSize[i-1] >> 2;     //  1/4 the data

        // See if we should bother building more mips.
        if( (Mip[i].Width < 8) || (Mip[i].Height < 8) )
        {
            Mips = i-1;
            break;
        }
    }

    // If the caller is requesting 0 mips (or if no mips can be built), then we 
    // may not have much to do.
    if( Mips == 0 )
    {
        // If there are already mips, we need to get rid of them.
        if( m_NMips > 0 )
        {
            NewDataSize = (m_Width * m_Height * Format.BPP) >> 3;
            pNewData    = (byte*)x_malloc( NewDataSize );
            x_memcpy( pNewData, pOldData, NewDataSize );

            if( m_Flags & FLAG_DATA_OWNED )
                x_free( m_Data.pPixel );

            m_Data.pPixel = pNewData;
            m_DataSize    = NewDataSize;    
            m_Flags      |= FLAG_DATA_OWNED;
            m_NMips       = 0;
        }
        return;
    }

    // Now that we know how many mips we are going to have, as well as the data
    // size for each mip, we can set the offsets (aligned to 16 byte bounds).
    Mip[0].Offset = ALIGN_16( sizeof( mip ) * (Mips+1) );
    for( i = 1; i <= Mips; i++ )
    {
        Mip[i].Offset = ALIGN_16( Mip[i-1].Offset + MipDataSize[i-1] );
    }

    // Allocate the new data block.  It must include all mips and the mip table.
    NewDataSize = Mip[Mips].Offset + MipDataSize[Mips];
    pNewData    = (byte*)x_malloc( NewDataSize );
    ASSERT( pNewData );

    //
    // Now we need to build a full color version of the image.  This will be
    // our master version from which all the mips will be built.
    //

    // If PS2 swizzling is on, unswizzle.
    xbool DoPS2SwizzleClut = FALSE;
    if( m_Flags & FLAG_PS2_CLUT_SWIZZLED )
    {
        DoPS2SwizzleClut = TRUE;
        PS2UnswizzleClut();
    }

    // If nibbles are flipped, unflip them..
    xbool DoFlip4BitNibbles = FALSE ;
    if ( m_Flags & FLAG_4BIT_NIBBLES_FLIPPED)
    {
        DoFlip4BitNibbles = TRUE ;
        Unflip4BitNibbles() ;
    }

    if( Format.ClutBased )
    {
        PaletteEntries = m_ClutSize / (Format.BPC >> 3);
        pColorPalette  = xbmp_DecodeToColor( m_pClut, (format)m_Format, PaletteEntries );
        pColorData     = xbmp_DecodeIndexToColor( pOldData, 
                                                  pColorPalette, 
                                                  Format.BPP, 
                                                  m_Width * m_Height );
    }
    else
    {
        pColorData = xbmp_DecodeToColor( pOldData, 
                                         (format)m_Format, 
                                         m_Width * m_Height );
    }

    // Decide how to mip alpha.
    {
        s32 NonOpaquePixels = 0;

        // Find out if alpha should be punch through.  That is, if all pixel 
        // alpha values are either 0 or 255, then use punch through in the mips.
        // Assume 'TRUE', then break on first counter example.

        PunchThruAlpha = TRUE;

        for( i = 0; i < m_Width * m_Height; i++ )
        {   
            if( pColorData[i].A != 255 )
                NonOpaquePixels++;

            if( (pColorData[i].A != 0) && (pColorData[i].A != 255) )
            {
                PunchThruAlpha = FALSE;
                break;
            }
        }

        if( NonOpaquePixels == 0 )
            PunchThruAlpha = FALSE;
    }

    // Install the first mip image (the original).
    x_memcpy( pNewData + Mip[0].Offset, pOldData, GetMipDataSize(0) );

    if( Format.ClutBased )
        cmap_Begin( pColorPalette, (1<<Format.BPP), (Format.ABits>0) );

    // Create and install all the other mips.
    for( i = 1; i <= Mips; i++ )
    {
        if( PunchThruAlpha || bForcePunchthrough )
        {
            pColorMip = xbmp_GenerateColorMipA1( pColorData, m_Width, m_Height, i );
        }
        else
        {
            pColorMip = xbmp_GenerateColorMip( pColorData, m_Width, m_Height, i );
        }

        // Test code.  Write out each mip as a TGA file.
        #ifdef DUMP_MIPS
        {
            xbitmap Temp;
            Temp.Setup( (format)2, m_Width >> i, m_Height >> i, FALSE, (byte*)pColorMip );
            Temp.SaveTGA( xfs( "Mip%05d.tga", GlobalMipCount++ ) );
        }
        #endif

        if( Format.ClutBased )
        {
            xbmp_EncodeColorToIndex( pNewData + Mip[i].Offset, 
                                     Format.BPP,
                                     pColorMip,
                                     (m_Width >> i) * (m_Height >> i) );
        }
        else
        {
            xbmp_EncodeFromColor( pNewData + Mip[i].Offset, 
                                  (format)m_Format, 
                                  pColorMip, 
                                  (m_Width >> i) * (m_Height >> i) );
        }

        x_free( pColorMip );
    }

    if( Format.ClutBased )
        cmap_End();


    //
    // Final details.
    //

    // Install the new data.
    if( m_Flags & FLAG_DATA_OWNED )
        x_free( m_Data.pPixel );
    m_Data.pPixel = pNewData;
    m_DataSize    = NewDataSize;    
    m_Flags      |= FLAG_DATA_OWNED;

    // Install the mip table.
    for( i = 0; i <= Mips; i++ )
    {
        m_Data.pMip[i].Width  = Mip[i].Width;
        m_Data.pMip[i].Height = Mip[i].Height;
        m_Data.pMip[i].Offset = Mip[i].Offset;
    } 

    // Free up temporary allocations.
    x_free( pColorData );
    if( pColorPalette )
        x_free( pColorPalette );

    // And the last touch!
    // NOTE - Must do before Flip4BitNibbles is called!
    m_NMips = Mips;

    // If PS2 swizzling is on, swizzle.
    if( DoPS2SwizzleClut )
        PS2SwizzleClut();

    // If nibble flipping is on, flip em!
    if (DoFlip4BitNibbles)
        Flip4BitNibbles() ;
}

//==============================================================================

void xbitmap::MakeIntensityAlpha( void )
{
    s32   Count;
    byte* pStart;
    s32   R, G, B, A;

    // Are there any bits to work with?
    if( (m_FormatInfo[ m_Format ].ABits == 0) && 
        (m_FormatInfo[ m_Format ].UBits == 0) )
        return;

    // Force any U formats to corresponding A format.  The format order makes
    // this conversion trivial.
    if( m_FormatInfo[ m_Format ].UBits != 0 )
        m_Format--;

    // Now, grab a reference to the format for ease of use for the rest of the
    // function.
    const format_info& Format = m_FormatInfo[ m_Format ];

    // How many elements (pixels or palette entries) do we need to process?
    if( Format.ClutBased )
    {
        Count  = m_ClutSize / (Format.BPC >> 3);
        pStart = m_pClut;
    }
    else
    {
        Count  = m_PW * m_Height;
        pStart = m_Data.pPixel + (m_NMips ? m_Data.pMip[0].Offset : 0);
    }

    //
    // Process 32 bit color information.
    //
    if( Format.BPC == 32 )
    {
        u32* pAlter = (u32*)pStart;
        u32  Mask   = Format.RMask | Format.GMask | Format.BMask;

        // Since we are reading 32 bit data, we can take a few shortcuts.  For 
        // example, we know that all components have a full 8 bits, so we don't 
        // need to worry about padding out or even "shifting left" when 
        // isolating the component during the read.

        while( Count > 0 )
        {            
            R = (*pAlter & Format.RMask) >> Format.RShiftR;
            G = (*pAlter & Format.GMask) >> Format.GShiftR;
            B = (*pAlter & Format.BMask) >> Format.BShiftR;

            A = (R + G + B + 1) / 3;
            *pAlter = (*pAlter & Mask) | (A << Format.AShiftR);

            pAlter++;
            Count--;
        }
    }

    // 
    // No need to worry about 24 bit information!  
    //

    //
    // Process 16 bit color information.
    //
    if( Format.BPC == 16 )
    {
        u16* pAlter = (u16*)pStart;
        u16  Mask   = (u16)(Format.RMask | Format.GMask | Format.BMask);

        // Since we are reading 16 bit data, we can take a few shortcuts (though
        // not as many as if we were reading 32 bit data).  We know that the
        // smallest R,G,B values are 4 bit, so we will only have to "duplicate 
        // and pad" once in order to fill out all 8 bits of data.

        while( Count > 0 )
        {
            R   = *pAlter;
            R  &= Format.RMask;
            R >>= Format.RShiftR;
            R <<= Format.RShiftL;
            R  |= (R >> Format.RBits);

            G   = *pAlter;
            G  &= Format.GMask;
            G >>= Format.GShiftR;
            G <<= Format.GShiftL;
            G  |= (G >> Format.GBits);

            B   = *pAlter;
            B  &= Format.BMask;
            B >>= Format.BShiftR;
            B <<= Format.BShiftL;
            B  |= (B >> Format.BBits);

            A   = (R + G + B + 1) / 3;
            A >>= Format.AShiftL;
            *pAlter = (*pAlter & Mask) | (A << Format.AShiftR);

            pAlter++;
            Count--;
        }
    }
}

//==============================================================================

#endif // !( defined( TARGET_PS2 ) && defined( CONFIG_RETAIL ) )
