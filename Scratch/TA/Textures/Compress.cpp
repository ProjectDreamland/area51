
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"
#include "x_math.hpp"
#include "Compress.hpp"
#include "NewQuantizer.hpp"
#include "Resize.hpp"

//#include "profecy\prcore.hpp"
//#include "profecy\bitmap.hpp"
//using namespace prcore;

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================
// BlockSize=8, Ratio=0.51:1
// BlockSize=4, Ratio=0.56:1
// BlockSize=2, Ratio=0.75:1
//=========================================================================
void CompressPS2( const xbitmap& SrcBMP, xbitmap& BaseBMP, xbitmap& LumBMP )
{
    //
    // Create base texture
    //
    s32 BlockSize = 4;

    ResizeBitmap( SrcBMP, BaseBMP, 
                  SrcBMP.GetWidth() / BlockSize, 
                  SrcBMP.GetHeight()/ BlockSize );

    //
    // Now we can palettice the base image
    //
    if( 1 )
    {
        QuanticeImage( BaseBMP, 0.999999f );
    }
    else
    {
        BaseBMP.ConvertFormat( xbitmap::FMT_P8_ARGB_8888 );
    }

    //
    // Initialize the elum texture
    //

    // Use even spread of colors/luminances for the 16 palette entries
    s32 CLUsed[16];
    s32 CL[16] ;
    int Palette4bit[] = { -128, -84, -52, -32, -20, -12, -8, -4, 
		                   0, 5, 11, 17, 29, 47, 76, 123 };

    for( s32 i = 0 ; i < 16 ; i++)
    {
        CL[i] = (Palette4bit[i] + 128)&0xff;
        CLUsed[i]=0;
    }

    //
    // Create lum texture
    //

    // Create a palettized lum bitmap same size as source, but only 4bits per pixel
    s32     LumWidth  = SrcBMP.GetWidth();
    s32     LumHeight = SrcBMP.GetHeight();
    byte*   LumData   = (byte *)x_malloc((LumWidth * LumHeight * 4) / 8) ;  // 4bits per poixel
    byte*   LumClut   = (byte *)x_malloc(16*4) ;                            // 16 clut entries
    ASSERT(LumData) ;
    ASSERT(LumClut) ;
    LumBMP.Setup(xbitmap::FMT_P4_ABGR_8888,  // Format (PS2)
                 LumWidth,                   // Width
                 LumHeight,                  // Height
                 TRUE,                       // DataOwned
                 LumData,                    // PixelData
                 TRUE,                       // Clut owned
                 LumClut) ;                  // Clut data

    // Setup lum bitmap palette
    for (i = 0 ; i < 16 ; i++)
    {
        u8 C = (u8)CL[i] ;
        LumBMP.SetClutColor(xcolor(C, C, C, C), i) ;    // RGB doesn't matter...
    }

    // Setup all luminance pixels in this mip
    for( s32 y=0; y < LumHeight; y++ )
    for( s32 x=0; x < LumWidth;  x++ )
    {
        // Get base map color
        // NOTE: Read with bi-linear just like the hardware will render!
        f32 u = (f32)x / (f32)LumWidth ;
        f32 v = (f32)y / (f32)LumHeight ;
        xcolor BaseCol = BaseBMP.GetBilinearColor( u, v, TRUE, 0 );

        // Lookup the real actual color that we are trying to look like!
        xcolor SrcCol = SrcBMP.GetPixelColor( x, y );

        // Find best matching luminance from palette
        s32 BI = 0;
        s32 BD = S32_MAX;

        for( i=0; i<16; i++ )
        {
            s32 C[3];
            s32 L = CL[i] ;

            // Get color that GS will work out
            C[0] = ((s32)BaseCol.R * L) >> 7;
            C[1] = ((s32)BaseCol.G * L) >> 7;
            C[2] = ((s32)BaseCol.B * L) >> 7;

            // Valid color?
            if ( (C[0] >= 0)   && (C[1] >= 0)   && (C[2] >= 0) &&
                 (C[0] <= 255) && (C[1] <= 255) && (C[2] <= 255) )
            {
                // If closest to output color, then keep it!
                s32 D = SQR(C[0]-(s32)SrcCol.R) +
                        SQR(C[1]-(s32)SrcCol.G) +
                        SQR(C[2]-(s32)SrcCol.B) ;
                if( D < BD )
                {
                    BI = i ;
                    BD = D ;
                }
            }
        }

        // Set luminance color
        LumBMP.SetPixelIndex(BI, x,y ) ;

        // How many one of the colors of the intensity maps gets used
        CLUsed[BI]++;
    }

    //
    // Make it ps2 happy
    //

    // Put base map into ps2 format
    auxbmp_ConvertToPS2( BaseBMP ) ;

    // Put luminance map into PS2 format
    // (NOTE: Can't call auxbmp_ConvertToPS2 because it halfs all the alphas,
    //        and besides, there's no need since we are already almost have a ps2 format!)
    LumBMP.PS2SwizzleClut();
    LumBMP.Flip4BitNibbles();
}
