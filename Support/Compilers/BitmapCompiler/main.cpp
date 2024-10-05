
#include "x_files.hpp"
#include "CommandLine.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"

//=========================================================================
// VARIABLES
//=========================================================================

static bool s_ReqPalOnly = false;

static xbool    s_bIsShadow[256];
static xbitmap  s_Bitmap[256];

//=========================================================================
// FUNCTIONS
//=========================================================================

void ConvertToShadowMap( xbitmap& BMP )
{
    BMP.ConvertFormat( xbitmap::FMT_P4_ABGR_8888 );
}

//=========================================================================

void ConvertToPaletteOnly( xbitmap& BMP, s32 nColors )
{
    s32 i;
    ASSERT( (nColors==256) || (nColors==16) );

    // allocate space for the new palette
    byte* pPalette = (byte*)x_malloc( nColors*4 );
    x_memset( pPalette, 255, nColors*4 );
    if ( (nColors == 256) && (BMP.GetWidth() != 256) )
    {
        x_throw( "Bitmap must be 256 pixels wide for a Palette Only 8-bit" );
        return;
    }
    else
    if ( (nColors == 16) && (BMP.GetWidth() != 16) )
    {
        x_throw( "Bitmap must be 16 pixels wide for a Palette Only 4-bit" );
        return;
    }

    // copy the first row of pixels into our new palette
    xcolor* pCol = (xcolor*)pPalette;
    for ( i = 0; i < nColors; i++ )
        *pCol++ = BMP.GetPixelColor( i, 0 );

    // create an image that just repeats the palette over and over (the
    // image isn't used, but must hang around to have a valid bitmap)
    byte* pData = (byte*)x_malloc( (nColors==256) ? (8*8) : (8*8/2) );
    x_memset( pData, 0, (nColors==256) ? (8*8) : (8*8/2) );
    if ( nColors == 256 )
    {
        for ( i = 0; i < 8*8; i++ )
            pData[i] = i%nColors;
    }
    if ( nColors == 16 )
    {
        for ( i = 0; i < 8*8; i++ )
        {
            if ( i&0x1 )
                pData[i>>1] = (pData[i/2]&0x0f) | ((i%nColors)<<4);
            else
                pData[i>>1] = (pData[i/2]&0xf0) | ((i%nColors)<<0);
        }
    }

    // set that data into our bitmap
    BMP.Kill();
    if ( nColors == 256 )
    {
        BMP.Setup( xbitmap::FMT_P8_ARGB_8888, 8, 8, TRUE, pData, TRUE, pPalette );
    }
    else
    {
        BMP.Setup( xbitmap::FMT_P4_ARGB_8888, 8, 8, TRUE, pData, TRUE, pPalette );
    }
}

//=========================================================================

static u32 CalcSize( const xbitmap& Bitmap )
{
    u32 NMips = (u32)Bitmap.GetNMips( );
    if(!NMips )
        return Bitmap.GetDataSize( );
    u32 i,Size=0;
    for( i=0;i<=NMips;i++ )
        Size += Bitmap.GetMipDataSize(i);
    return Size;
}

//=========================================================================

void ExecuteScript( command_line& CommandLine )
{
    xbool           bBitmapLoaded = FALSE;
    const char*     pFileName     = NULL;
    s32             iOptLevel     = 0;
    xbool           bIsXbox       = FALSE;
    s32             Count         = 0;

    x_try;

    x_memset( s_bIsShadow, 0, sizeof(s_bIsShadow) );

    //
    // Parse all the options
    //
    for( s32 i=0; i<CommandLine.GetNumOptions(); i++ )
    {
        // Get option name and string
        xstring OptName   = CommandLine.GetOptionName( i );
        xstring OptString = CommandLine.GetOptionString( i );
        xbool   bOption   = FALSE;

        if( OptName == xstring( "APPEND" ) )
        {
            pFileName = (const char*)OptString;
            bBitmapLoaded = auxbmp_Load( s_Bitmap[Count++], OptString );             
            if( bBitmapLoaded == FALSE )
                x_throw( xfs("Unable to load bitmap (%s)", (const char*)OptString) );
            continue;
        }

        if( OptName == xstring( "O3" ))
        {
            iOptLevel = 3;
            continue;
        }

        if( bBitmapLoaded == FALSE )
            x_throw( "You must specify a source bitmap as the first parameter" );

        if( OptName == xstring( "XBOX" ) )
        {
            X_FILE* Fp;
            bIsXbox = TRUE;
            Fp = x_fopen( OptString, "wb" );
            if( Fp == NULL ) 
                x_throw( xfs("Unable to save bitmap (%s)", (const char*)OptString) );

            for( s32 i=0; i<Count; i++ )
            {
                xbitmap Xbox;
                Xbox = s_Bitmap[i];
                s32 h = Xbox.GetHeight();
                s32 w = Xbox.GetWidth ();

                if( ! s_ReqPalOnly )
                {
                    auxbmp_ConvertToD3D( Xbox );

                    // produce projective shadow maps *************************

                    if( s_bIsShadow[i] )
                    {
                        // resize if necessary (256x256) ......................

                        s32 W = (w>256) ? 256:w;
                        s32 H = (h>256) ? 256:h;
                        if( (W!=w) || (H!=h) )
                        {
                            //Xbox.Resize( W,H );
                            x_throw( "Image resolution is not a w256 x h256!");
                        }
                        // convert to A8 ......................................

                        Xbox.BuildMips( 4 );
                        Xbox = auxbmp_ConvertRGBToA8( Xbox );
                    }
                    else
                    {
                        if( iOptLevel == 3 )
                            auxbmp_Compress( Xbox,pFileName,0 );
                        else
                        {
                            // Forget DXT due to registration marks
                            if( auxbmp_IsPunchthruAlpha( Xbox ))
                                Xbox.ConvertFormat( xbitmap::FMT_16_ARGB_1555 );
                        }
                    }
                }
                else // create fog table
                {
                    auxbmp_ConvertToD3D( Xbox );

                    u32* p = (u32*)Xbox.GetPixelData( );
                    s32  n = x_min( w*h,64 );
                    s32  j;

                    for( j=0;j<n;j++ )
                    {
                        p[j]=*( u32* )&s_Bitmap[i].GetClutColor( j*4 );
                    }
                }
                Xbox.Save( Fp );
            }
            x_fclose( Fp );
            continue;
        }

        if( OptName == xstring( "PC" ) )
        {
            X_FILE* Fp;

            Fp = x_fopen( OptString, "wb" );
            if( Fp == NULL ) 
                x_throw( xfs("Unable to save bitmap (%s)", (const char*)OptString) );

            for( s32 i=0; i<Count; i++ )
            {
                xbitmap PC;
                PC = s_Bitmap[i];

                auxbmp_ConvertToD3D( PC );

                PC.Save( Fp );
            }

            x_fclose( Fp );
            continue;
        }

        if( OptName == xstring( "PS2" ) )
        {
            X_FILE* Fp;

            Fp = x_fopen( OptString, "wb" );
            if( Fp == NULL ) 
                x_throw( xfs("Unable to save bitmap (%s)", (const char*)OptString) );

            for( s32 i=0; i<Count; i++ )
            {
                xbitmap PS2;

                PS2 = s_Bitmap[i];
                if( s_bIsShadow[i] )
                {
                    // shadow textures are already in an acceptable format, but
                    // do need to have their nibbles flipped
                    PS2.Flip4BitNibbles();
                }
                else
                {
                    // put the texture into an acceptable ps2 format
                    auxbmp_ConvertToPS2( PS2 );
                }

                PS2.Save( Fp );
            }

            x_fclose( Fp );
            continue;
        }

        if( OptName == xstring( "P8" ) )
        {
            s_Bitmap[Count-1].ConvertFormat( xbitmap::FMT_P8_ARGB_8888 );
            continue;
        }

        if( OptName == xstring( "P4" ) )
        {
            s_Bitmap[Count-1].ConvertFormat( xbitmap::FMT_P4_ARGB_8888 );
            continue;
        }

        if( OptName == xstring( "TC" ) )
        {
            s_Bitmap[Count-1].ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
            continue;
        }

        if( OptName == xstring( "PAL8" ) )
        {
            ConvertToPaletteOnly( s_Bitmap[Count-1], 256 );
            s_ReqPalOnly = true;
            continue;
        }

        if( OptName == xstring( "PAL4" ) )
        {
            ConvertToPaletteOnly( s_Bitmap[Count-1], 16 );
            s_ReqPalOnly = true;
            continue;
        }

        if( OptName == xstring( "COM" ) )
        {
            x_throw( "Can't convert to a Compress texture. It is not suported yet.");
        }

        if( OptName == xstring( "SHAD" ) )
        {
            s_bIsShadow[Count-1] = TRUE;
            ConvertToShadowMap( s_Bitmap[Count-1] );
            continue;
        }
    }

    x_catch_begin;
    #ifdef X_EXCEPTIONS
        x_printf( "Error: %s\n", xExceptionGetErrorString() );
    #endif
    x_catch_end;
}

//=========================================================================

void PrintHelp( void )
{
    x_printf( "Error: Compiling...\n" );
    x_printf( "    Platform and file to compile: -XBOX -PS2 or -PC bitmap.xbmp         \n" );   
    x_printf( "    Type of Bitmap:               -O3   Aggressive optimisation         \n" );
    x_printf( "    Type of Bitmap:               -P8   Palettice to 8 bits             \n" );
    x_printf( "    Type of Bitmap:               -P4   Palettice to 4 bits             \n" );
    x_printf( "    Type of Bitmap:               -COM  Compress textures 4bpp (s8/ps2) \n" );
    x_printf( "    Type of Bitmap:               -TC   True color 32 bits per pixel    \n" );
    x_printf( "    Type of Bitmap:               -SHAD For projected shadows           \n" );
    x_printf( "    Type of Bitmap:               -PAL8 Save 8-bit Palette only         \n" );
    x_printf( "    Type of Bitmap:               -PAL4 Save 4-bit Palette only         \n" );
    x_printf( "    Verbose                       -LOG                                  \n" );
}

//=========================================================================

void main( s32 argc, char* argv[] )
{
    x_try;

    // save out the exe timestamp for doing dependancy checks
    command_line CommandLine;
    
    // Specify all the options
    CommandLine.AddOptionDef( "P8" );
    CommandLine.AddOptionDef( "P4" );
    CommandLine.AddOptionDef( "COM" );
    CommandLine.AddOptionDef( "TC" );
    CommandLine.AddOptionDef( "SHAD" );
    CommandLine.AddOptionDef( "O3" );
    CommandLine.AddOptionDef( "PAL8" );
    CommandLine.AddOptionDef( "PAL4" );

    CommandLine.AddOptionDef( "APPEND", command_line::STRING );
    CommandLine.AddOptionDef( "XBOX",   command_line::STRING );
    CommandLine.AddOptionDef( "PS2",    command_line::STRING );
    CommandLine.AddOptionDef( "PC",     command_line::STRING );

    // Parse the command line
    if( CommandLine.Parse( argc, argv ) )
    {
        PrintHelp();
        return;
    }
    
    // Do the script
    ExecuteScript( CommandLine );

    x_catch_begin;
    #ifdef X_EXCEPTIONS
        x_printf( "Error: %s\n", xExceptionGetErrorString() );
    #endif
    x_catch_end;
}
