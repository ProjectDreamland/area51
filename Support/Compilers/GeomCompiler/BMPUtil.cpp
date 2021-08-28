
#include "BMPUtil.hpp"

//=============================================================================

void bmp_util::ConvertToPalettized( xbitmap& Bitmap, s32 BPP )
{
    s32 nColors;
    if ( BPP == 8 )
    {
        nColors = 256;
        Bitmap.ConvertFormat( xbitmap::FMT_P8_ABGR_8888 );
    }
    else if ( BPP == 4 )
    {
        nColors = 16;
        Bitmap.ConvertFormat( xbitmap::FMT_P4_ABGR_8888 );
    }
    else
    {
        nColors = 0;
        ASSERT( FALSE );
    }

    // any alpha's pretty close to zero were probably meant to be zero
    // the quantizer doesn't always handle that well
    s32 i;
    for ( i = 0; i < nColors; i++ )
    {
        xcolor C = Bitmap.GetClutColor( i );
        if ( C.A < 3 )
            C.A = 0;
        Bitmap.SetClutColor( C, i );
    }
}

//=============================================================================

//#### DS: TODO: Blech...talk to D.Michael about the auxbmp_ConvertToPS2 function
//               so that maybe we don't have to duplicate it.
void bmp_util::ConvertToPS2( xbitmap& Bitmap, xbool ReduceAlpha )
{
    xbitmap::format OldFormat;
    xbitmap::format NewFormat = xbitmap::FMT_NULL;

    // See if we got lucky and have a compatable format.
    OldFormat = Bitmap.GetFormat();
    if( (OldFormat != xbitmap::FMT_P4_ABGR_8888 ) &&
        (OldFormat != xbitmap::FMT_P8_ABGR_8888 ) &&
        (OldFormat != xbitmap::FMT_P4_UBGR_8888 ) &&
        (OldFormat != xbitmap::FMT_P8_UBGR_8888 ) &&
        (OldFormat != xbitmap::FMT_16_ABGR_1555 ) &&
        (OldFormat != xbitmap::FMT_16_UBGR_1555 ) &&
        (OldFormat != xbitmap::FMT_32_ABGR_8888 ) &&
        (OldFormat != xbitmap::FMT_32_UBGR_8888 ) )
    {
        //
        // Oh well, we need to convert.
        //

        xbool HasAlpha = Bitmap.HasAlphaBits();
        switch( Bitmap.GetFormatInfo().BPP )
        {
            case 32:
            case 24:
                NewFormat = (HasAlpha)?(xbitmap::FMT_32_ABGR_8888):(xbitmap::FMT_32_UBGR_8888);
                break;
            case 16:
                NewFormat = (HasAlpha)?(xbitmap::FMT_16_ABGR_1555):(xbitmap::FMT_16_UBGR_1555);
                break;
            case 8:
                NewFormat = (HasAlpha)?(xbitmap::FMT_P8_ABGR_8888):(xbitmap::FMT_P8_UBGR_8888);
                break;
            case 4:
                NewFormat = (HasAlpha)?(xbitmap::FMT_P4_ABGR_8888):(xbitmap::FMT_P4_UBGR_8888);
                break;
        }

        ASSERT( NewFormat != xbitmap::FMT_NULL );

        Bitmap.ConvertFormat( NewFormat );
    }

    if ( ReduceAlpha )
    {
        // Shift alphas down
        // (even when not present since when drawing with alpha blending eg. fading player, it will get used!!)

	    // Clut based?
        if (Bitmap.GetFormatInfo().ClutBased)
        {
            s32    NColors = 0;
            byte*  pC      = NULL;

            switch( Bitmap.GetFormatInfo().BPP )
            {
                case 8:    
                    NColors = 256;
                    pC      = (byte*)Bitmap.GetClutData();
                    break;
                case 4:    
                    NColors = 16;
                    pC      = (byte*)Bitmap.GetClutData();
                    break;

                default:
                    ASSERTS(0, "Unsupported clut format!") ;
                    break ;
            }

            while( NColors-- )
            {
                pC[3] >>= 1;
                if ( pC[3] == 127 )
                    pC[3] = 128;
                pC     += 4;
            }
        }
        else
        {
            // Non-clut based

            // Loop through all mip levels
            for (s32 Mip = 0 ; Mip <= Bitmap.GetNMips() ; Mip++)
            {
                s32    NColors = 0;
                byte*  pC      = NULL;

                switch( Bitmap.GetFormatInfo().BPP )
                {
                    case 32:    
                        NColors = Bitmap.GetWidth(Mip) * Bitmap.GetHeight(Mip);
                        pC      = (byte*)Bitmap.GetPixelData(Mip) ;
                        break;
                }

                while( NColors-- )
                {
                    pC[3] >>= 1;
                    if ( pC[3] == 127 )
                        pC[3] = 128;
                    pC     += 4;
                }
            }
        }
    }

    // Be sure the good stuff is swizzled.
    Bitmap.PS2SwizzleClut();

    // Make sure 4 bits per pixel data nibbles are flipped
    Bitmap.Flip4BitNibbles();
}

//=============================================================================

xbool bmp_util::CopyIntensityToDiffuse( xbitmap& D, const xbitmap& M )
{
    // Check the dimensions
    if( (D.GetWidth() != M.GetWidth()) || (D.GetHeight() != M.GetHeight()) )
        return( FALSE );

    // Copy map into Alpha channel of Diffuse
    for( s32 y=0; y<D.GetHeight(); y++ )
    {
        for( s32 x=0; x<D.GetWidth(); x++ )
        {
            xcolor C0 = D.GetPixelColor( x, y );
            xcolor C1 = M.GetPixelColor( x, y );
            
            // Limit the range of Alpha values to improve the image quality of the 8-bit Diffuse
            //C0.A = (u8)( MIN( 255, ((s32)( C1.R * (16.0f / 255.0f) + 0.5f )) << (3+1)) );
            
            // Just copy alpha value across - above code was causing banding on opacity
            C0.A = C1.R;
            //C0.A = MAX( C0.A, 2 );

            D.SetPixelColor( C0, x, y );
        }
    }

    return( TRUE );
}

//=============================================================================

xbool bmp_util::CopyIntensityToAlpha( xbitmap& Bitmap )
{
    for( s32 y=0; y<Bitmap.GetHeight(); y++ )
    {
        for( s32 x=0; x<Bitmap.GetWidth(); x++ )
        {
            xcolor C = Bitmap.GetPixelColor( x, y );

            C.A = C.R;
            C.A = MAX( C.A, 2 );

            Bitmap.SetPixelColor( C, x, y );
        }
    }

    return( TRUE );
}

//=============================================================================

struct color_cell
{
    s32 Min;
    s32 Max;
    s32 Total;
};

void bmp_util::ProcessDetailMap( xbitmap& Bitmap, xbool bPalettize )
{
    static const s32 kDetailMips       = 2;
    static const s32 kDetailColorCount = 16;

    s32 y, x;

    // force a purely greyscale image...the pixels colors don't matter in the
    // end since only the alpha channel is used, but forcing greyscale will
    // help out the mipping.
    CopyIntensityToAlpha( Bitmap );
    for ( y = 0; y < Bitmap.GetHeight(); y++ )
    {
        for ( x = 0; x < Bitmap.GetWidth(); x++ )
        {
            xcolor C = Bitmap.GetPixelColor( x, y );
            C.R = C.G = C.B = C.A;
            Bitmap.SetPixelColor( C, x, y );
        }
    }

    // set up the mips so that they tend towards an alpha of 128...this will
    // keep the detail map from getting muddied and causing splotches as we
    // move away from the object
    Bitmap.BuildMips(kDetailMips);
    s32 NMips = Bitmap.GetNMips();
    if ( NMips )
    {
        for ( s32 Mip = 1; Mip <= NMips; Mip++ )
        {
            f32 t = (f32)Mip / (f32)NMips;
            for ( y = 0; y < Bitmap.GetHeight( Mip ); y++ )
            {
                for ( x = 0; x < Bitmap.GetWidth( Mip ); x++ )
                {
                    xcolor C = Bitmap.GetPixelColor( x, y, Mip );
                    C.A = (u8)(((f32)C.A*(1.0f-t)) + (128.0f*t));
                    C.R = C.G = C.B = C.A;
                    Bitmap.SetPixelColor( C, x, y, Mip );
                }
            }
        }
    }

    // now the conversion to palettized for the ps2 will destroy our 1 custom
    // mip, so let's do the palettization manually...easy since its just a
    // greyscale image anyway
    if ( bPalettize )
    {
        s32         Mip, i;
        s32         OrigCounts[256];
        s32         NewColors[kDetailColorCount];
        color_cell  Cells[kDetailColorCount];
        s32         nCells = 1;

        // count up the colors used
        x_memset(OrigCounts,0,sizeof(s32)*256);
        x_memset(Cells,0,sizeof(color_cell)*kDetailColorCount);
        Cells[0].Min = 256;
        Cells[0].Max = 0;
        for ( Mip = 0; Mip <= Bitmap.GetNMips(); Mip++ )
        {
            for ( y = 0; y < Bitmap.GetHeight(Mip); y++ )
            {
                for ( x = 0; x < Bitmap.GetWidth(Mip); x++ )
                {
                    xcolor C = Bitmap.GetPixelColor( x, y, Mip );
                    OrigCounts[C.A]++;
                    Cells[0].Total++;
                    Cells[0].Min = MIN(Cells[0].Min, C.A);
                    Cells[0].Max = MAX(Cells[0].Max, C.A+1);
                }
            }
        }

        // make sure the value 128 gets its own entry by jacking the count WAY up
        s32 BiasAmount = Bitmap.GetWidth() * Bitmap.GetHeight();
        OrigCounts[128] += BiasAmount;
        Cells[0].Total  += BiasAmount;

        // keep splitting the range at the median point until we have
        // a max number of colors
        while ( nCells < kDetailColorCount )
        {
            // find the cell with the widest range
            s32 WidestRange = 0;
            for ( i = 1; i < nCells; i++ )
            {
                if ( (Cells[i].Max-Cells[i].Min) >
                     (Cells[WidestRange].Max-Cells[WidestRange].Min) )
                {
                    WidestRange = i;
                }
            }

            // if the widest range is one, then there's no need to split any more
            if ( (Cells[WidestRange].Max-Cells[WidestRange].Min) == 1 )
                break;

            // find the median point of the cell to cut
            s32 Median    = Cells[WidestRange].Total / 2;
            s32 PrevValid = -1;
            for ( i = Cells[WidestRange].Min; i < Cells[WidestRange].Max; i++ )
            {
                Median -= OrigCounts[i];
                if ( (Median < 0) && (PrevValid != -1) )
                {
                    // we've found our split point
                    break;
                }

                if ( OrigCounts[i] )
                    PrevValid = i;
            }

            if ( (i == Cells[WidestRange].Max) || (PrevValid==-1) )
            {
                // should never get in here, but as a safety precaution...
                ASSERT( FALSE );
                break;
            }

            // split the cell
            Cells[nCells].Min      = i;
            Cells[nCells].Max      = Cells[WidestRange].Max;
            Cells[WidestRange].Max = i;

            // recalculate the totals
            s32 MinColor = 256;
            s32 MaxColor = 0;
            Cells[WidestRange].Total = 0;
            for ( i = Cells[WidestRange].Min; i < Cells[WidestRange].Max; i++ )
            {
                if ( OrigCounts[i] )
                {
                    MinColor = MIN( MinColor, i );
                    MaxColor = MAX( MaxColor, i+1 );
                    Cells[WidestRange].Total += OrigCounts[i];
                }
            }
            Cells[WidestRange].Min = MinColor;
            Cells[WidestRange].Max = MaxColor;

            MinColor = 256;
            MaxColor = 0;
            Cells[nCells].Total = 0;
            for ( i = Cells[nCells].Min; i < Cells[nCells].Max; i++ )
            {
                if ( OrigCounts[i] )
                {
                    MinColor = MIN( MinColor, i );
                    MaxColor = MAX( MaxColor, i+1 );
                    Cells[nCells].Total += OrigCounts[i];
                }
            }
            Cells[nCells].Min = MinColor;
            Cells[nCells].Max = MaxColor;

            // done
            nCells++;
        }

        // build a palette from the cells
        for ( i = 0; i < nCells; i++ )
        {
            s32 ColorTotal = 0;
            s32 nColors    = 0;
            for ( x = Cells[i].Min; x < Cells[i].Max; x++ )
            {
                if ( OrigCounts[x] )
                {
                    ColorTotal += OrigCounts[x] * x;
                }
            }

            ColorTotal = ColorTotal / Cells[i].Total;
            NewColors[i] = ColorTotal;

            // if this is the cell that contains 128, then force the issue,
            // and make it 128
            if ( (Cells[i].Min <= 128) && (Cells[i].Max>128) )
            {
                NewColors[i] = 128;
            }
        }

        // now create a new bitmap that is the right bit depth
        xbitmap NewBMP;
        s32     DataSize   = Bitmap.GetWidth() * Bitmap.GetHeight() / 2;
        s32     ClutSize   = kDetailColorCount*4;
        byte*   pPixelData = (byte*)x_malloc( DataSize );
        byte*   pClutData  = (byte*)x_malloc( ClutSize );
        x_memset( pPixelData, 0, DataSize );
        x_memset( pClutData,  0, ClutSize );
        NewBMP.Setup( xbitmap::FMT_P4_ABGR_8888, Bitmap.GetWidth(), Bitmap.GetHeight(), TRUE, pPixelData, TRUE, pClutData );
        NewBMP.BuildMips(kDetailMips);

        // copy the data from our old bitmap into the new
        for ( Mip = 0; Mip <= NewBMP.GetNMips(); Mip++ )
        {
            for ( y = 0; y < Bitmap.GetHeight(Mip); y++ )
            {
                for ( x = 0; x < Bitmap.GetWidth(Mip); x++ )
                {
                    xcolor C = Bitmap.GetPixelColor(x,y,Mip);
                    s32 BestMatch = 0;
                    s32 BestScore = x_abs(C.A - NewColors[0]);
                    for ( i = 1; i < nCells; i++ )
                    {
                        // find the best match
                        s32 Score = x_abs(C.A - NewColors[i]);
                        if ( Score < BestScore )
                        {
                            BestScore = Score;
                            BestMatch = i;
                        }
                    }

                    NewBMP.SetPixelIndex( BestMatch, x, y, Mip );
                }
            }
        }

        // fill in the clut
        for ( i = 0; i < nCells; i++ )
        {
            NewBMP.SetClutColor( xcolor(NewColors[i],NewColors[i],NewColors[i],NewColors[i]),i );
        }

        // we have our new bitmap
        Bitmap.Kill();
        Bitmap = NewBMP;

/*
        xbitmap NewBMP;
        byte* pPixelData = (byte*)x_malloc( Bitmap.GetWidth() * Bitmap.GetHeight() );
        byte* pClutData = (byte*)x_malloc( 256 * 4 );
        x_memset( pPixelData, 0, Bitmap.GetWidth() * Bitmap.GetHeight() );
        x_memset( pClutData, 0, 256 * 4 );
        NewBMP.Setup( xbitmap::FMT_P8_ABGR_8888, Bitmap.GetWidth(), Bitmap.GetHeight(), TRUE, pPixelData, TRUE, pClutData );
        NewBMP.BuildMips(kDetailMips);

        // copy the data from our old bitmap into the new
        for ( s32 Mip = 0; Mip <= NewBMP.GetNMips(); Mip++ )
        {
            for ( y = 0; y < Bitmap.GetHeight(Mip); y++ )
            {
                for ( x = 0; x < Bitmap.GetWidth(Mip); x++ )
                {
                    xcolor C = Bitmap.GetPixelColor( x, y, Mip );
                    NewBMP.SetPixelIndex( C.A, x, y, Mip );
                }
            }
        }

        // set up the new greyscale alpha clut
        for ( s32 col = 0; col < 256; col++ )
        {
            NewBMP.SetClutColor( xcolor(col,col,col,col), col );
        }

        // finally we have our new bitmap
        Bitmap.Kill();
        Bitmap = NewBMP;
    */
    }
}

//=============================================================================

xbool bmp_util::SetPunchThrough( xbitmap& D, const xbitmap& M )
{
    // Check the dimensions
    if( (D.GetWidth() != M.GetWidth()) || (D.GetHeight() != M.GetHeight()) )
        return( FALSE );

    for( s32 y=0; y<D.GetHeight(); y++ )
    {
        for( s32 x=0; x<D.GetWidth(); x++ )
        {
            xcolor C0 = D.GetPixelColor( x, y );
            xcolor C1 = M.GetPixelColor( x, y );
                
            if( C1.R > 0 )
            {
                // No Punch-Through on this pixel so clamp the alpha
                C0.A = MAX( 2, C0.A );
            }
            else
            {
                // Alpha of Zero means Punch-Through
                C0.A = 0;
            }
            
            D.SetPixelColor( C0, x, y );
        }
    }
    
    return( TRUE );
}

//=============================================================================

extern void quant_Begin    ( void );
extern void quant_SetPixels( const xcolor* pColor, s32 NColors );
extern void quant_End      ( xcolor* pPalette, s32 NColors, xbool UseAlpha );
extern void cmap_Begin     ( const xcolor* pColor, s32 NColors, xbool UseAlpha );
extern s32  cmap_GetIndex  ( xcolor Color );
extern void cmap_End       ( void );

void bmp_util::ConvertToColoredMips( xbitmap& Bitmap )
{
    s32 iMip, x, y;

    // make a working copy
    xbitmap NewBMP = Bitmap;

    // build some mips if they're not already there
    if ( !NewBMP.GetNMips() )
        NewBMP.BuildMips();
    s32 nMips = NewBMP.GetNMips();

    // make a copy of all the colors, and include our tinting
    s32 ColorCount = 0;
    for ( iMip = 0; iMip <= NewBMP.GetNMips(); iMip++ )
        ColorCount += NewBMP.GetWidth(iMip)*NewBMP.GetHeight(iMip);
    xcolor* pTempColors = new xcolor[ColorCount];
    xcolor* pPalette    = new xcolor[256];
    ColorCount = 0;
    xcolor Tints[8] =
    {
        xcolor(255,255,255,255),        // mip 0 == no tinting
        xcolor(255,32,32,255),          // mip 1 == red tint
        xcolor(32,255,32,255),          // mip 2 == green tint
        xcolor(32,32,255,255),          // mip 3 == blue tint
        xcolor(255,255,32,255),         // mip 4 == yellow tint
        xcolor(32,255,255,255),         // mip 5 == cyan
        xcolor(255,32,255,255),         // mip 6 == magenta
        xcolor(0,0,0,255),              // mip 7+ == black
    };
    for ( iMip = 0; iMip <= NewBMP.GetNMips(); iMip++ )
    {
        xcolor MipTint = Tints[MIN(iMip,7)];
        for ( y = 0; y < NewBMP.GetHeight(iMip); y++ )
        {
            for ( x = 0; x < NewBMP.GetWidth(iMip); x++ )
            {
                xcolor C = NewBMP.GetPixelColor( x, y, iMip );
                s32 R = (C.R * MipTint.R) / 255;
                s32 G = (C.G * MipTint.G) / 255;
                s32 B = (C.B * MipTint.B) / 255;
                C.R = (u8)R;
                C.G = (u8)G;
                C.B = (u8)B;
                pTempColors[ColorCount++] = C;
            }
        }
    }

    // build a color palette that will work with the new colors
    quant_Begin();
    quant_SetPixels( pTempColors, ColorCount );
    quant_End( (xcolor*)pPalette, 256, TRUE );

    // build a new bitmap that will work
    NewBMP.ConvertFormat(xbitmap::FMT_P8_ABGR_8888);
    NewBMP.BuildMips(nMips);

    // map the the old colors into a new palettized version
    cmap_Begin( pPalette, 256, TRUE );
    s32 ColorIndex = 0;
    for ( iMip = 0; iMip <= NewBMP.GetNMips(); iMip++ )
    {
        for ( y = 0; y < NewBMP.GetHeight(iMip); y++ )
        {
            for ( x = 0; x < NewBMP.GetWidth(iMip); x++ )
            {
                ASSERT( ColorIndex < ColorCount );
                if ( ColorIndex > ColorCount )
                    ColorIndex = 0;
                xcolor C = pTempColors[ColorIndex++];
                NewBMP.SetPixelIndex( cmap_GetIndex(C), x, y, iMip );
            }
        }
    }
    cmap_End();

    // copy the clut over
    for ( x = 0; x < 256; x++ )
        NewBMP.SetClutColor( pPalette[x], x );

    // clean up the temps
    delete []pTempColors;
    delete []pPalette;

    // done
    Bitmap = NewBMP;
}
