//==============================================================================
//  
//  x_bitmap_resize.cpp
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

#include "..\x_math.hpp"

//==============================================================================
//  SampleBitmap
//==============================================================================

static xcolor SampleBitmap( const xbitmap& BMP, f32 x0, f32 y0, f32 x1, f32 y1 )
{
    f32 SW = (f32)BMP.GetWidth();
    f32 SH = (f32)BMP.GetHeight();

    // Offset pixel
    x0 -= 0.5f;
    x1 -= 0.5f;
    y0 -= 0.5f;
    y1 -= 0.5f;

    // Clip coordinates
    if( x0 < 0.0f ) x0 = 0.0f;
    if( x0 > SW   ) x0 = SW;
    if( x1 < 0.0f ) x1 = 0.0f;
    if( x1 > SW   ) x1 = SW;
    if( y0 < 0.0f ) y0 = 0.0f;
    if( y0 > SH   ) y0 = SH;
    if( y1 < 0.0f ) y1 = 0.0f;
    if( y1 > SH   ) y1 = SH;

    // Zero out color accumulators
    s32 R = 0;
    s32 G = 0;
    s32 B = 0;
    s32 A = 0;

    // Determine number of supersamples on X and Y
    s32 XSAMPLES = (MAX((s32)(x1-x0),1)) * 2;
    s32 YSAMPLES = (MAX((s32)(y1-y0),1)) * 2;

    // Loop on Y & X
    for( s32 ly=0 ; ly<YSAMPLES ; ly++ )
    {
        for( s32 lx=0 ; lx<XSAMPLES ; lx++ )
        {
            // Get dx,dy (0.0 -> 1.0) between x0-x1 and y0-y1 that we are sampling
            f32 dx = (f32)lx/XSAMPLES;
            f32 dy = (f32)ly/YSAMPLES;

            // Get the x,y that we are sampling
            f32 x  = (x0+(x1-x0)*dx);
            f32 y  = (y0+(y1-y0)*dy);

            // Get fractional texel location into fx,fy (0.0 -> 1.0)
            f32 fx = x - x_floor(x);
            f32 fy = y - x_floor(y);

            // Get integer texel locations on either side of the location we want
            s32 ix0 = (s32)x;
            s32 ix1 = (s32)MIN(x+1,SW-1);
            s32 iy0 = (s32)y;
            s32 iy1 = (s32)MIN(y+1,SH-1);

            // Sample all 4 texels
            xcolor CA = BMP.GetPixelColor( ix0, iy0 );
            xcolor CB = BMP.GetPixelColor( ix1, iy0 );
            xcolor CC = BMP.GetPixelColor( ix0, iy1 );
            xcolor CD = BMP.GetPixelColor( ix1, iy1 );

            // Compute the bilinear interpolaters.
            f32 IA = (1.0f-fx)*(1.0f-fy);
            f32 IB = (     fx)*(1.0f-fy);
            f32 IC = (1.0f-fx)*(     fy);
            f32 ID = (     fx)*(     fy);

            // Interpolate colors & accumulate into totals
            R += (s32)(IA*CA.R + IB*CB.R + IC*CC.R + ID*CD.R);
            G += (s32)(IA*CA.G + IB*CB.G + IC*CC.G + ID*CD.G);
            B += (s32)(IA*CA.B + IB*CB.B + IC*CC.B + ID*CD.B);
            A += (s32)(IA*CA.A + IB*CB.A + IC*CC.A + ID*CD.A);
        }
    }

    // Round, divide, clamp
    R += (XSAMPLES*YSAMPLES/2);
    G += (XSAMPLES*YSAMPLES/2);
    B += (XSAMPLES*YSAMPLES/2);
    A += (XSAMPLES*YSAMPLES/2);
    R /= (XSAMPLES*YSAMPLES);
    G /= (XSAMPLES*YSAMPLES);
    B /= (XSAMPLES*YSAMPLES);
    A /= (XSAMPLES*YSAMPLES);
    R = MIN(R,255);
    G = MIN(G,255);
    B = MIN(B,255);
    A = MIN(A,255);

    // return the color
    return xcolor(R,G,B,A);
}

//==============================================================================
//  SampleBitmap
//==============================================================================

static xcolor SampleBitmapPunchthrough( const xbitmap& BMP, f32 x0, f32 y0, f32 x1, f32 y1 )
{
    // Opaque threshold: this could be a parameter.
    const s32 OpaqueThreshold = 128;

     // These are samples that do not contribute to the final color
    s32 NonContributingSampleCount = 0;
    f32 SW = (f32)BMP.GetWidth();
    f32 SH = (f32)BMP.GetHeight();

    // Offset pixel
    x0 -= 0.5f;
    x1 -= 0.5f;
    y0 -= 0.5f;
    y1 -= 0.5f;

    // Clip coordinates
    if( x0 < 0.0f ) x0 = 0.0f;
    if( x0 > SW   ) x0 = SW;
    if( x1 < 0.0f ) x1 = 0.0f;
    if( x1 > SW   ) x1 = SW;
    if( y0 < 0.0f ) y0 = 0.0f;
    if( y0 > SH   ) y0 = SH;
    if( y1 < 0.0f ) y1 = 0.0f;
    if( y1 > SH   ) y1 = SH;

    // Zero out color accumulators
    s32 R = 0;
    s32 G = 0;
    s32 B = 0;
    s32 A = 0;

    // Determine number of supersamples on X and Y
    s32 XSAMPLES = (MAX((s32)(x1-x0),1)) * 2;
    s32 YSAMPLES = (MAX((s32)(y1-y0),1)) * 2;

    // Loop on Y & X
    for( s32 ly=0 ; ly<YSAMPLES ; ly++ )
    {
        for( s32 lx=0 ; lx<XSAMPLES ; lx++ )
        {
            // Get dx,dy (0.0 -> 1.0) between x0-x1 and y0-y1 that we are sampling
            f32 dx = (f32)lx/XSAMPLES;
            f32 dy = (f32)ly/YSAMPLES;

            // Get the x,y that we are sampling
            f32 x  = (x0+(x1-x0)*dx);
            f32 y  = (y0+(y1-y0)*dy);

            // Get fractional texel location into fx,fy (0.0 -> 1.0)
            f32 fx = x - x_floor(x);
            f32 fy = y - x_floor(y);

            // Get integer texel locations on either side of the location we want
            s32 ix0 = (s32)x;
            s32 ix1 = (s32)MIN(x+1,SW-1);
            s32 iy0 = (s32)y;
            s32 iy1 = (s32)MIN(y+1,SH-1);

            // Sample all 4 texels
            xcolor Color[4];
                   Color[0] = BMP.GetPixelColor( ix0, iy0 );
                   Color[1] = BMP.GetPixelColor( ix1, iy0 );
                   Color[2] = BMP.GetPixelColor( ix0, iy1 );
                   Color[3] = BMP.GetPixelColor( ix1, iy1 );

            // Compute the bilinear interpolaters.
            f32 Interp[4];
                Interp[0] = (1.0f-fx)*(1.0f-fy);
                Interp[1] = (     fx)*(1.0f-fy);
                Interp[2] = (1.0f-fx)*(     fy);
                Interp[3] = (     fx)*(     fy);

            ASSERT( x_abs(( Interp[0] + Interp[1] + Interp[2] + Interp[3] ) - 1.0f ) < 0.01f ); // We assume that the total interpolation weght is 1.0f

            // Determine number of Opaque pixels
            // Assume all pixles are Opaque to start
            s32 nOpaque = 4;
            s32 OpaqueMask = 0xf;
            for ( s32 cInterp = 0; cInterp < 4; ++cInterp )
            {
                if ( Color[cInterp].A < OpaqueThreshold )
                {
                    --nOpaque;
                    OpaqueMask &= ~( 0x01 << cInterp );
                }
            }

            // Interpolate colors & accumulate into totals
            //
            if (( nOpaque > 0 ) && ( nOpaque < 4 ))
            {
                s32 accR = 0;
                s32 accG = 0;
                s32 accB = 0;
                f32 TotalWeight = 1.0f;     // Assume total weight of 1.0f to start
                for ( s32 cInterp = 0; cInterp < 4; ++cInterp )
                {
                    if ( OpaqueMask & ( 0x01 << cInterp ))
                    {   // the pixel is Opaque - so we want it to influnce the final color
                        accR += (s32)(Interp[cInterp] * Color[cInterp].R );
                        accG += (s32)(Interp[cInterp] * Color[cInterp].G );
                        accB += (s32)(Interp[cInterp] * Color[cInterp].B );
                    }
                    else
                    { // the pixel is transparent - we don't want it to effect the final color - so don't accumlate the color - and remove the weight
                        TotalWeight -= Interp[cInterp];
                    }
                    // Always do full interpolation of alpha
                    A += (s32)( Interp[cInterp] * Color[cInterp].A );
                }

                if (( TotalWeight < 0.99f ) && ( TotalWeight > 0.01f ))
                { // total weight is less than 1.0f and greater than 0.0f
                    f32 ooTotalWeight = 1.0f / TotalWeight;
                    R += (s32)((float)(accR) * ooTotalWeight);
                    G += (s32)((float)(accG) * ooTotalWeight);
                    B += (s32)((float)(accB) * ooTotalWeight);
                }
                else
                {
                    ++NonContributingSampleCount;
                }
            }
            else
            {
                // Interpolate colors & accumulate into totals
                for ( s32 cInterp = 0; cInterp < 4; ++cInterp )
                {
                    R += (s32)(Interp[cInterp] * Color[cInterp].R );
                    G += (s32)(Interp[cInterp] * Color[cInterp].G );
                    B += (s32)(Interp[cInterp] * Color[cInterp].B );
                    A += (s32)(Interp[cInterp] * Color[cInterp].A );
                }
            }
        }
    }


    // Calculate final color value
    s32 TotalSamples             = (XSAMPLES*YSAMPLES);
    s32 TotalContributingSamples = (XSAMPLES*YSAMPLES) - NonContributingSampleCount;
    ASSERT( TotalContributingSamples && "Something is wacky in resizing a bitmap" );
    // Round
    R += (TotalContributingSamples / 2);
    G += (TotalContributingSamples / 2);
    B += (TotalContributingSamples / 2);
    A += (TotalSamples / 2);
    // Divide
    R /= TotalContributingSamples;
    G /= TotalContributingSamples;
    B /= TotalContributingSamples;
    A /= TotalSamples;


    // clamp
    R = MIN(R,255);
    G = MIN(G,255);
    B = MIN(B,255);
    A = (OpaqueThreshold < A ) ? 0xff : 0x00;

    // return the color
    return xcolor(R,G,B,A);
}

//==============================================================================
//  Resize
//==============================================================================

void xbitmap::Resize( s32 NewW, s32 NewH, xbool bPunchthrough /* = FALSE */ )
{
    xbitmap DST;
    DST.Setup( xbitmap::FMT_32_RGBA_8888, NewW, NewH, TRUE, (byte*)x_malloc(NewW*NewH*4) );

    f32 SW = (f32)GetWidth();
    f32 SH = (f32)GetHeight();

    for( s32 y=0 ; y<NewH ; y++ )
    {
        for( s32 x=0 ; x<NewW ; x++ )
        {
            f32 x0 = (f32)(x  )/NewW * SW;
            f32 x1 = (f32)(x+1)/NewW * SW;
            f32 y0 = (f32)(y  )/NewH * SH;
            f32 y1 = (f32)(y+1)/NewH * SH;

            xcolor c = bPunchthrough ? SampleBitmapPunchthrough( *this, x0, y0, x1, y1 ) : SampleBitmap( *this, x0, y0, x1, y1 );

            DST.SetPixelColor( c, x, y );
        }
    }

    // Replace the current bitmap
    *this = DST;
}

//==============================================================================
//  Crop
//==============================================================================

void xbitmap::Crop( s32 x, s32 y, s32 w, s32 h )
{
    ASSERT( x >= 0 );
    ASSERT( y >= 0 );
    ASSERT( w >  0 );
    ASSERT( h >  0 );
    ASSERT( (x+w) <= m_Width  );
    ASSERT( (y+h) <= m_Height );

    xbitmap DST;
    DST.Setup( xbitmap::FMT_32_RGBA_8888, w, h, TRUE, (byte*)x_malloc(w*h*4) );

    for( s32 iy=0 ; iy<h ; iy++ )
    {
        for( s32 ix=0 ; ix<w ; ix++ )
        {
            xcolor c = GetPixelColor( x+ix, y+iy );
            DST.SetPixelColor( c, ix, iy );
        }
    }

    // Replace the current bitmap
    *this = DST;
}

//==============================================================================
