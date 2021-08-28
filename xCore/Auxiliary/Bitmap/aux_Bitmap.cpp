//==============================================================================
//
//  aux_Bitmap.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "aux_Bitmap.hpp"
#include "x_debug.hpp"
#include "x_string.hpp"
#include "x_math.hpp"

//#define DUMP_LOADS

#ifdef DUMP_LOADS
static s32 DumpLoadCount=0;
#endif

//==============================================================================
//  TYPES
//==============================================================================

typedef xbool info_fn(                        const char* pFileName, xbitmap::info& Info );
typedef xbool load_fn(       xbitmap& Bitmap, const char* pFileName );
typedef xbool save_fn( const xbitmap& Bitmap, const char* pFileName );

struct info_entry
{
    char      Extension[8];
    info_fn*  InfoFn;
};

struct load_entry
{
    char      Extension[8];
    load_fn*  LoadFn;
};

struct save_entry
{
    char      Extension[8];
    save_fn*  SaveFn;
};

//==============================================================================
//  FUNCTION DECLARATIONS - Instead of headers with one function declared.
//==============================================================================

       info_fn     tga_Info;
       info_fn     bmp_Info;
	   info_fn     psd_Info;
static info_fn    xbmp_Info;

       load_fn     tga_Load;
       load_fn     bmp_Load;
       load_fn     psd_Load;
static load_fn     xbmp_Load;

static save_fn     xbmp_Save;

//==============================================================================
//  LOCAL STORAGE
//==============================================================================

static info_entry InfoTable[] = 
{
    { ".TGA",       tga_Info    },
    { ".BMP",       bmp_Info    },
	{ ".PSD",       psd_Info    },
    { ".XBMP",     xbmp_Info	},
};

static load_entry LoadTable[] = 
{
    { ".TGA",       tga_Load    },
    { ".BMP",       bmp_Load    },
	{ ".PSD",       psd_Load    },
    { ".XBMP",     xbmp_Load    },
};

static save_entry SaveTable[] = 
{
    { ".XBMP",      xbmp_Save   },
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

xbool auxbmp_Info( const char* pFileName, xbitmap::info& Info )
{
    char    Ext[ X_MAX_EXT ];
    xbool   Result  = FALSE;
    s32     nProviders = sizeof(InfoTable) / sizeof(InfoTable[0]);
    s32     i;

    ASSERT( pFileName );

    // Break up the filename.
    x_splitpath( pFileName, NULL, NULL, NULL, Ext );

    // Upper case the extension.
    x_strtoupper( Ext );

    // Search for an info provider.
    for( i = 0; i < nProviders ; i++ )
    {
        if( x_strcmp( Ext, InfoTable[i].Extension ) == 0 )
        {
            Result = InfoTable[i].InfoFn( pFileName, Info );
            break;
        }
    }

	// Return the result
	return Result;
}

//==============================================================================

xbool auxbmp_Load( xbitmap& Bitmap, const char* pFileName )
{           
    char    Ext[ X_MAX_EXT ];
    xbool   Result  = FALSE;
    s32     Loaders = sizeof(LoadTable) / sizeof(load_entry);
    s32     i;

    ASSERT( pFileName );

    // Break up the filename.
    x_splitpath( pFileName, NULL, NULL, NULL, Ext );

    // Upper case the extension.
    x_strtoupper( Ext );

    // Search for a loader.
    for( i = 0; i < Loaders; i++ )
    {
        if( x_strcmp( Ext, LoadTable[i].Extension ) == 0 )
        {
            Result = LoadTable[i].LoadFn( Bitmap, pFileName );
            break;
        }
    }

    // If we did not successfully load, then setup the "default" bitmap.  Leave
    // the Result with a FALSE value, though, to inform the caller of the 
    // situation.

    if( Result == FALSE )
    {
        auxbmp_SetupDefault( Bitmap );
    }

#ifdef DUMP_LOADS
    Bitmap.SaveTGA(xfs("auxdump%04d.tga",DumpLoadCount++));
#endif

    return( Result );
}

//==============================================================================

xbool auxbmp_Save( const xbitmap& Bitmap, const char* pFileName )
{           
    char    Ext[ X_MAX_EXT ];
    xbool   Result = FALSE;
    s32     Savers = sizeof(SaveTable) / sizeof(save_entry);
    s32     i;

    ASSERT( pFileName );

    // Break up the filename.
    x_splitpath( pFileName, NULL, NULL, NULL, Ext );

    // Upper case the extension.
    x_strtoupper( Ext );

    // Search for a saver.
    for( i = 0; i < Savers; i++ )
    {
        if( x_strcmp( Ext, SaveTable[i].Extension ) == 0 )
        {
            Result = SaveTable[i].SaveFn( Bitmap, pFileName );
            break;
        }
    }

    // And we are outta here.
    return( Result );
}

//==============================================================================

static
xbool xbmp_Info( const char* pFileName, xbitmap::info& BitmapInfo )
{
    return xbitmap::Info( pFileName, BitmapInfo );
}

//==============================================================================

static
xbool xbmp_Load( xbitmap& Bitmap, const char* pFileName )
{
    // This is easy!  "XBMP" is "xbitmap", and they load themselves!
    return( Bitmap.Load( pFileName ) );
}

//==============================================================================

static
xbool xbmp_Save( const xbitmap& Bitmap, const char* pFileName )
{
    // This is easy!  "XBMP" is "xbitmap", and they save themselves!
    return( Bitmap.Save( pFileName ) );
}

//==============================================================================

void auxbmp_ConvertToD3D( xbitmap& Bitmap )
{
    xbitmap::format OldFormat;
    xbitmap::format NewFormat = xbitmap::FMT_NULL;

    // See if we got lucky and have a compatable format.
    OldFormat = Bitmap.GetFormat();
    if( (OldFormat == xbitmap::FMT_32_ARGB_8888 ) ||
        (OldFormat == xbitmap::FMT_32_URGB_8888 ) ||
        (OldFormat == xbitmap::FMT_16_ARGB_1555 ) ||
        (OldFormat == xbitmap::FMT_16_URGB_1555 ) ||
        (OldFormat == xbitmap::FMT_16_RGB_565   ) ||
        (OldFormat == xbitmap::FMT_DXT1 ) ||
        (OldFormat == xbitmap::FMT_DXT2 ) ||
        (OldFormat == xbitmap::FMT_DXT3 ) ||
        (OldFormat == xbitmap::FMT_DXT4 ) ||
        (OldFormat == xbitmap::FMT_DXT5 ) )
            return;

    //
    // Oh well, we need to convert.
    //
    // Since we don't support any palette based formats in DirectX, we only 
    // worry about "bits per color" rather than "bits per pixel".  Thus, a
    // palette based bitmap with 32 bits per color will be converted to the
    // same destination format as a bitmap with 32 bits per pixel.
    // 

    switch( Bitmap.GetFormatInfo().BPC )
    {
    case 32:
        // If there is alpha, use FMT_32_ARGB_8888, otherwise FMT_32_URGB_8888.
        if( Bitmap.GetFormatInfo().ABits > 0 )
            NewFormat = xbitmap::FMT_32_ARGB_8888;
        else
            NewFormat = xbitmap::FMT_32_URGB_8888;
        break;

    case 24:
        NewFormat = xbitmap::FMT_32_URGB_8888;
        break;

    case 16:
        // If there is 4 bits of alpha, then go to FMT_32_ARGB_8888.  Otherwise,
        // use closest match of FMT_16_ARGB_1555, FMT_16_URGB_1555, or
        // FMT_16_RGB_565 as appropriate.
        if( Bitmap.GetFormatInfo().ABits == 4 )
            NewFormat = xbitmap::FMT_32_ARGB_8888;
        else
        if( Bitmap.GetFormatInfo().GBits == 6 )
            NewFormat = xbitmap::FMT_16_RGB_565;
        else
        if( Bitmap.GetFormatInfo().ABits == 1 )
            NewFormat = xbitmap::FMT_16_ARGB_1555;
        else
            NewFormat = xbitmap::FMT_16_URGB_1555;
    }

    ASSERT( NewFormat != xbitmap::FMT_NULL );

    Bitmap.ConvertFormat( NewFormat );
}

//==============================================================================

xbool auxbmp_LoadD3D( xbitmap& Bitmap, const char* pFileName )
{
    xbool Result;
    Result = auxbmp_Load( Bitmap, pFileName );
    auxbmp_ConvertToD3D( Bitmap );
    return Result;
}

//==============================================================================

xbool auxbmp_LoadGCN( xbitmap& Bitmap, const char* pFileName )
{
    xbool Result;
    Result = auxbmp_Load( Bitmap, pFileName );
    auxbmp_ConvertToGCN( Bitmap );
    return Result;
}

//==============================================================================

void auxbmp_ConvertToPS2( xbitmap& Bitmap )
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
            pC[3]   = (pC[3]==255) ? 128 : pC[3]>>1;
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
                pC     += 4;
            }
        }
    }

    // Be sure the good stuff is swizzled.
    Bitmap.PS2SwizzleClut();

    // Make sure 4 bits per pixel data nibbles are flipped
    Bitmap.Flip4BitNibbles();
}

//==============================================================================

xbool auxbmp_LoadPS2( xbitmap& Bitmap, const char* pFileName )
{
    xbool Result;
    Result = auxbmp_Load( Bitmap, pFileName );

#ifdef DUMP_LOADS
    Bitmap.SaveTGA( xfs("LoadPS2A%04d.tga",DumpCount) );
    Bitmap.SavePaletteTGA(xfs("LoadPS2PALA%04d.tga",DumpLoadCount) );
#endif

    auxbmp_ConvertToPS2( Bitmap );

#ifdef DUMP_LOADS
    Bitmap.SaveTGA( xfs("LoadPS2B%04d.tga",DumpCount) );
    Bitmap.SavePaletteTGA(xfs("LoadPS2PALB%04d.tga",DumpLoadCount) );
    DumpLoadCount++;
#endif

    return( Result );
}

//==============================================================================

void auxbmp_ConvertToGCN( xbitmap& Bitmap, xbool Swizzle )
{
    xbitmap::format OldFormat;
    xbitmap::format NewFormat = xbitmap::FMT_NULL;

    // See if we got lucky and have a compatable format.
    OldFormat = Bitmap.GetFormat();
    if( (OldFormat != xbitmap::FMT_P4_RGBA_8888 ) &&
        (OldFormat != xbitmap::FMT_P8_RGBA_8888 ) &&
        (OldFormat != xbitmap::FMT_P4_RGBU_8888 ) &&
        (OldFormat != xbitmap::FMT_P8_RGBU_8888 ) &&
        (OldFormat != xbitmap::FMT_16_RGB_565   ) &&
        (OldFormat != xbitmap::FMT_32_RGBA_8888 ) &&
        (OldFormat != xbitmap::FMT_32_RGBU_8888 ) &&
        (OldFormat != xbitmap::FMT_DXT1         ) &&
        (OldFormat != xbitmap::FMT_DXT3         ))
    {
        //
        // Oh well, we need to convert.
        //

        xbool HasAlpha = Bitmap.HasAlphaBits();
        
        // Force everything to 32bit for now
        NewFormat = (HasAlpha)?(xbitmap::FMT_32_RGBA_8888):(xbitmap::FMT_32_RGBU_8888);

        switch( Bitmap.GetFormatInfo().BPP )
        {
            
            case 32:
            case 24:
                NewFormat = (HasAlpha)?(xbitmap::FMT_32_RGBA_8888):(xbitmap::FMT_32_RGBU_8888);
                break;                
            case 16:
                NewFormat = xbitmap::FMT_16_RGB_565;
                break;                
            case 8:
                NewFormat = (HasAlpha)?(xbitmap::FMT_P4_RGBA_8888):(xbitmap::FMT_P8_RGBU_8888);
                break;                
            case 4:
                NewFormat = (HasAlpha)?(xbitmap::FMT_P4_RGBA_8888):(xbitmap::FMT_P4_RGBU_8888);
                break;
                
        }
                
        ASSERT( NewFormat != xbitmap::FMT_NULL );

        Bitmap.ConvertFormat( NewFormat );
    }

    // Be sure the good stuff is swizzled.
    if (Swizzle)
        Bitmap.GCNSwizzleData();
}

//==============================================================================

void auxbmp_ConvertToNative( xbitmap& Bitmap )
{
    #ifdef TARGET_PC
    auxbmp_ConvertToD3D( Bitmap );
    return;
    #endif

    #ifdef TARGET_XBOX
    auxbmp_ConvertToD3D( Bitmap );
    return;
    #endif

    #ifdef TARGET_PS2
    auxbmp_ConvertToPS2( Bitmap );
    return;
    #endif  

    #ifdef TARGET_GCN
    auxbmp_ConvertToGCN( Bitmap );
    return;
    #endif

    ASSERT( FALSE );
}

//==============================================================================

xbool auxbmp_LoadNative( xbitmap& Bitmap, const char* pFileName )
{
    xbool Result;
    Result = auxbmp_Load( Bitmap, pFileName );
    auxbmp_ConvertToNative( Bitmap );
#ifdef DUMP_LOADS
    Bitmap.SaveTGA(xfs("auxdumpN%04d.tga",DumpLoadCount++));
#endif
    return Result;
}

//==============================================================================

struct alpha_match_table
{
    xbitmap::format     WithAlpha;
    xbitmap::format     WithoutAlpha;
};

static alpha_match_table    AlphaMatchTable[] = 
{
    { xbitmap::FMT_32_RGBA_8888,        xbitmap::FMT_32_RGBU_8888 },
    { xbitmap::FMT_32_ARGB_8888,        xbitmap::FMT_32_URGB_8888 },
    { xbitmap::FMT_16_RGBA_5551,        xbitmap::FMT_16_RGBU_5551 },
    { xbitmap::FMT_16_ARGB_1555,        xbitmap::FMT_16_URGB_1555 },
    { xbitmap::FMT_32_BGRA_8888,        xbitmap::FMT_32_BGRU_8888 },
    { xbitmap::FMT_32_ABGR_8888,        xbitmap::FMT_32_UBGR_8888 },
    { xbitmap::FMT_16_BGRA_5551,        xbitmap::FMT_16_BGRU_5551 },
    { xbitmap::FMT_16_ABGR_1555,        xbitmap::FMT_16_UBGR_1555 },
    { xbitmap::FMT_P8_RGBA_8888,        xbitmap::FMT_P8_RGBU_8888 },
    { xbitmap::FMT_P8_ARGB_8888,        xbitmap::FMT_P8_URGB_8888 },
    { xbitmap::FMT_P8_RGBA_5551,        xbitmap::FMT_P8_RGBU_5551 },
    { xbitmap::FMT_P8_ARGB_1555,        xbitmap::FMT_P8_URGB_1555 },
    { xbitmap::FMT_P8_BGRA_8888,        xbitmap::FMT_P8_BGRU_8888 },
    { xbitmap::FMT_P8_ABGR_8888,        xbitmap::FMT_P8_UBGR_8888 },
    { xbitmap::FMT_P8_BGRU_5551,        xbitmap::FMT_P8_BGRU_5551 },
    { xbitmap::FMT_P8_ABGR_1555,        xbitmap::FMT_P8_UBGR_1555 },
    { xbitmap::FMT_P4_RGBA_8888,        xbitmap::FMT_P4_RGBU_8888 },
    { xbitmap::FMT_P4_ARGB_8888,        xbitmap::FMT_P4_URGB_8888 },
    { xbitmap::FMT_P4_RGBA_5551,        xbitmap::FMT_P4_RGBU_5551 },
    { xbitmap::FMT_P4_ARGB_1555,        xbitmap::FMT_P4_URGB_1555 },
    { xbitmap::FMT_P4_BGRA_8888,        xbitmap::FMT_P4_BGRU_8888 },
    { xbitmap::FMT_P4_ABGR_8888,        xbitmap::FMT_P4_UBGR_8888 },
    { xbitmap::FMT_P4_BGRA_5551,        xbitmap::FMT_P4_BGRU_5551 },
    { xbitmap::FMT_P4_ABGR_1555,        xbitmap::FMT_P4_UBGR_1555 },

    { xbitmap::FMT_NULL,                xbitmap::FMT_NULL },
};

//==============================================================================

static 
xbitmap::format GetAlphaFormat( xbitmap::format Src )
{
    s32 i = 0;
    while (AlphaMatchTable[i].WithAlpha != xbitmap::FMT_NULL)
    {
        if ((AlphaMatchTable[i].WithAlpha    == Src) ||
            (AlphaMatchTable[i].WithoutAlpha == Src))
        {
            return AlphaMatchTable[i].WithAlpha;
        }
        i++;
    }
    return xbitmap::FMT_NULL;
}

//==============================================================================

xbool auxbmp_SetColorAlpha( xbitmap& Bitmap, xcolor Clr, u8 NewAlpha )
{
    s32                 i;
    s32                 nMips = Bitmap.GetNMips();
    xbitmap::format     NewFmt;

    NewFmt = GetAlphaFormat( Bitmap.GetFormat() );

    if (NewFmt == xbitmap::FMT_NULL)
        return FALSE;

    Bitmap.ConvertFormat( NewFmt );

    if (!Bitmap.IsClutBased())
    {
        for (i=0;i<=nMips;i++)
        {
            s32 W,H;
            W = Bitmap.GetWidth(i);
            H = Bitmap.GetHeight(i);
        
            s32 x,y;

            for (y=0;y<H;y++)
            {
                for (x=0;x<W;x++)
                {
                    xcolor C = Bitmap.GetPixelColor( x, y, i );
                    if ((C.R == Clr.R) &&
                        (C.G == Clr.G) &&
                        (C.B == Clr.B))
                    {
                        C.A = NewAlpha;
                        Bitmap.SetPixelColor( C, x, y, i );
                    }                    
                }
            }
        }
    }
    else
    {
        s32 nClutColors = 1 << Bitmap.GetBPP();

        for (i=0;i<nClutColors;i++)
        {
            xcolor C = Bitmap.GetClutColor(i);
            if ((C.R == Clr.R) &&
                (C.G == Clr.G) &&
                (C.B == Clr.B))
            {
                C.A = NewAlpha;
                Bitmap.SetClutColor( C, i );
            }           
        }
    }

    return TRUE;
}

//==============================================================================

void auxbmp_MakeDxDvBump( xbitmap& Dest, const xbitmap& Source, platform Platfrom )
{
    s32 x,y;
    const s32 W       = Source.GetWidth();
    const s32 H       = Source.GetHeight();

    ASSERT( &Dest != &Source );

    // Create new bitmap for dx, dv, lum
    if( Platfrom == PLATFORM_GCN )
    {
        Dest.Setup( xbitmap::FMT_32_RGBA_8888,
                    W, H,
                    TRUE,
                    (byte*)new u32[ W*H ] );
    }
    else
    {
        Dest.Setup( xbitmap::FMT_32_ARGB_8888,
                    W, H,
                    TRUE,
                    (byte*)new u32[ W*H ] );
    }
                
    // Build the bump map
    for( y=0; y<H; y++ )
    for( x=0; x<W; x++ )
    {
        //xcolor v00 = Source.GetPixelColor( x,                         y            );
        xcolor v01 = Source.GetPixelColor( (s32)x_lpr(x+1.0f,(f32)W), y            ); // Should it wrap or clamp?
        xcolor vM1 = Source.GetPixelColor( (s32)x_lpr(x-1.0f,(f32)W), y            ); // Should it wrap or clamp?
        xcolor v10 = Source.GetPixelColor( x,                        (s32)x_lpr(y+1.0f,(f32)H) );
        xcolor v1M = Source.GetPixelColor( x,                        (s32)x_lpr(y-1.0f,(f32)H) );
        s8     Du  = (vM1.R-v01.R);                     // The delta-u bump value
        s8     Dv  = (v1M.R-v10.R);                     // The delta-v bump value
//        s8     Lum = ( v00>1 ) ? 63 : 127;              // Put some generic luminance value
        u8     A,B;

        if( Platfrom == PLATFORM_GCN )
        {
            A = (u8)(Du + 127l);
            B = (u8)(Dv + 127l);

            // Write the final color
            Dest.SetPixelColor( xcolor( 0, A, B, 0), x, y );
        }
        else if( Platfrom == PLATFORM_PC || Platfrom == PLATFORM_XBOX )
        {
            Dest.SetPixelColor( xcolor( 0, Du, Dv, 0), x, y );
        }
    }
}

//==============================================================================

void auxbmp_MakeDiffuseFromRGB( xbitmap& Dest, const xbitmap& Source )
{
    ASSERT( &Dest != &Source );

    s32 W = Source.GetWidth();
    s32 H = Source.GetHeight();

    Dest.Setup( xbitmap::FMT_24_RGB_888,
                W,H,                
                TRUE,
                new byte[W*H*3] );

    s32     x,y;
    xcolor  Src;
    for (y=0;y<H;y++)
    {
        for (x=0;x<W;x++)
        {
            Src = Source.GetPixelColor( x,y );
            Dest.SetPixelColor( xcolor( Src.R, Src.G, Src.B, (u8)255 ), x, y );
        }
    }
}

//==============================================================================

void auxbmp_MakeDiffuseFromA( xbitmap& Dest, const xbitmap& Source )
{
    ASSERT( &Dest != &Source );

    s32 W = Source.GetWidth();
    s32 H = Source.GetHeight();

    Dest.Setup( xbitmap::FMT_24_RGB_888,
                W,H,                
                TRUE,
                new byte[W*H*3] );

    s32     x,y;
    xcolor  Src;
    for (y=0;y<H;y++)
    {
        for (x=0;x<W;x++)
        {
            Src = Source.GetPixelColor( x,y );
            Dest.SetPixelColor( xcolor( Src.A, Src.A, Src.A, (u8)255 ), x, y );
        }
    }
}

//==============================================================================

void auxbmp_MakeDiffuseFromR( xbitmap& Dest, const xbitmap& Source )
{
    ASSERT( &Dest != &Source );

    s32 W = Source.GetWidth();
    s32 H = Source.GetHeight();

    Dest.Setup( xbitmap::FMT_24_RGB_888,
                W,H,                
                TRUE,
                new byte[W*H*3] );

    s32     x,y;
    xcolor  Src;
    for (y=0;y<H;y++)
    {
        for (x=0;x<W;x++)
        {
            Src = Source.GetPixelColor( x,y );
            Dest.SetPixelColor( xcolor( Src.R, Src.R, Src.R, (u8)255 ), x, y );
        }
    }
}

//==============================================================================

void auxbmp_MakeDiffuseFromG( xbitmap& Dest, const xbitmap& Source )
{
    ASSERT( &Dest != &Source );

    s32 W = Source.GetWidth();
    s32 H = Source.GetHeight();

    Dest.Setup( xbitmap::FMT_24_RGB_888,
                W,H,                
                TRUE,
                new byte[W*H*3] );

    s32     x,y;
    xcolor  Src;
    for (y=0;y<H;y++)
    {
        for (x=0;x<W;x++)
        {
            Src = Source.GetPixelColor( x,y );
            Dest.SetPixelColor( xcolor( Src.G, Src.G, Src.G, (u8)255 ), x, y );
        }
    }
}

//==============================================================================

void auxbmp_MakeDiffuseFromB( xbitmap& Dest, const xbitmap& Source )
{
    ASSERT( &Dest != &Source );

    s32 W = Source.GetWidth();
    s32 H = Source.GetHeight();

    Dest.Setup( xbitmap::FMT_24_RGB_888,
                W,H,                
                TRUE,
                new byte[W*H*3] );

    s32     x,y;
    xcolor  Src;
    for (y=0;y<H;y++)
    {
        for (x=0;x<W;x++)
        {
            Src = Source.GetPixelColor( x,y );
            Dest.SetPixelColor( xcolor( Src.B, Src.B, Src.B, (u8)255 ), x, y );
        }
    }
}

//==============================================================================

void auxbmp_CreateAFromR( xbitmap& Dest, const xbitmap& Source )
{
    ASSERT( &Dest != &Source );

    s32 W = Source.GetWidth();
    s32 H = Source.GetHeight();

    Dest.Setup( xbitmap::FMT_32_ARGB_8888,
                W,H,                
                TRUE,
                new byte[W*H*4] );

    s32     x,y;
    xcolor  Src;
    for (y=0;y<H;y++)
    {
        for (x=0;x<W;x++)
        {
            Src = Source.GetPixelColor( x,y );
            Dest.SetPixelColor( xcolor( Src.R, Src.G, Src.B, Src.R  ), x, y );
        }
    }
}

//==============================================================================

void auxbmp_MergeDiffuseAndOpacity( xbitmap& dest, const xbitmap& diffuse, const xbitmap& opacity, xbool red )
{
    ASSERT(( &dest != &diffuse ) && ( &dest != &opacity ));
	ASSERT(( diffuse.GetHeight() == opacity.GetHeight() ) && ( diffuse.GetWidth() == opacity.GetWidth() ));

    s32 W = diffuse.GetWidth();
    s32 H = diffuse.GetHeight();

    dest.Setup( xbitmap::FMT_32_RGBA_8888,
		W,H,                
		TRUE,
		new byte[W*H*4] );
	
    xcolor  src;
	xcolor  alpha;
    for ( s32 y = 0; y < H; ++y )
    {
        for ( s32 x = 0; x < W; ++x )
        {
            src = diffuse.GetPixelColor( x, y );
			alpha = opacity.GetPixelColor( x, y );
            dest.SetPixelColor( xcolor( src.R, src.G, src.B, red ? alpha.R : alpha.A ), x, y );
        }
    }
}

//==============================================================================

static
xcolor GetAvgSurroundingOpaqueColor( const xbitmap& Bitmap, s32 alphaThreshold, s32 TexelX, s32 TexelY, s32 cMip )
{
    ASSERT(( 0 <= TexelX ) && ( TexelX < Bitmap.GetWidth()  ));
    ASSERT(( 0 <= TexelY ) && ( TexelY < Bitmap.GetHeight() ));
    ASSERT(( 0 <= cMip   ) && ( cMip   < ( Bitmap.GetNMips() + 1 )));

    // setup color accumlators
    xcolor rVal    = 0;
    s32    accR    = 0;
    s32    accG    = 0;
    s32    accB    = 0;
    s32    nOpaque = 0;

    for ( s32 y = TexelY - 1; y <= ( TexelY + 1 ); ++y )
    {
        if (( Bitmap.GetHeight( cMip ) <= y ) || ( y < 0 ))
            continue; // skip this line - out of range

        for ( s32 x = TexelX - 1; x <= ( TexelX + 1 ); ++x )
        {
            if (( Bitmap.GetWidth( cMip ) <= x ) || ( x < 0 ) || (( TexelX == x ) && ( TexelY == y )))
                continue; // skip this Texel - either out of range, or is the center texel
            xcolor clr = Bitmap.GetPixelColor( x, y, cMip );
            if ( clr.A < alphaThreshold )
                continue; // transparent pixel

            accR += clr.R;
            accG += clr.G;
            accB += clr.B;
            ++nOpaque;
        }
    }

    if ( nOpaque > 0 )
    {
        rVal.R = accR / nOpaque;
        rVal.G = accG / nOpaque;
        rVal.B = accB / nOpaque;
    }

    return rVal;
}

//==============================================================================

void auxbmp_ForcePunchthrough( xbitmap& Bitmap, u32 alphaThreshold )
{
    xcolor clr; // temporary color 

    // if the bitmap has a CLUT, then force all alpha values int the CLUT to be 0 or 255
    if ( Bitmap.IsClutBased() )
    {
        for ( s32 cEntry = 0; cEntry < Bitmap.GetClutSize(); ++cEntry )
        {
            clr = Bitmap.GetClutColor( cEntry );
            clr.A = ( clr.A < alphaThreshold ) ? 0x00 : 0xff;
            Bitmap.SetClutColor( clr, cEntry ); 
        }

        return;
    }

    // the texture is not clut based - loop through all texels of all MIPS - 
    // and set the color of all transparent pixels to be the average of 
    // all surrounding opaque pixels
    for ( s32 cMip = 0; cMip < ( Bitmap.GetNMips() + 1 ); ++cMip )
    {
        for ( s32 y = 0; y < Bitmap.GetHeight( cMip ); ++y )
        {
            for ( s32 x = 0; x < Bitmap.GetWidth( cMip ); ++x )
            {
                clr = Bitmap.GetPixelColor( x, y, cMip );

                if ( alphaThreshold < clr.A )
                {
                    clr.A = 0xff;
                    Bitmap.SetPixelColor( clr, x, y, cMip );
                    continue;
                }

                clr   = GetAvgSurroundingOpaqueColor( Bitmap, alphaThreshold, x, y, cMip );
                clr.A = 0x00; // just to be sure
                Bitmap.SetPixelColor( clr, x, y, cMip );
            }
        }
    }
}

//==============================================================================

// Quantizer utils...
void quant_Begin            ( void );
void quant_SetPixels        ( const xcolor* pColor, s32 NColors );
void quant_End              ( xcolor* pPalette, s32 NColors, xbool UseAlpha );

// Color map utils...
void cmap_Begin     ( const xcolor* pColor, s32 NColors, xbool UseAlpha );
s32  cmap_GetIndex  ( xcolor Color );
void cmap_End       ( void );

// Converts to new format with all the generated mips intact...
void auxbmp_ConvertFormat( xbitmap& BMP, xbitmap::format Format )
{
    s32 i, x,y,mip, NColors ;

    // Lookup useful info
    ASSERT( (Format > xbitmap::FMT_NULL       ) && 
            (Format < xbitmap::FMT_END_OF_LIST) );
    const xbitmap::format_info& NewFormat = xbitmap::GetFormatInfo( Format ) ;

    // Create a copy
    xbitmap SrcBMP = BMP ;

    // Let xbitmap class do all the work to setup a bitmap in the new format with space for mips...
    BMP.BuildMips(0) ;
    BMP.ConvertFormat(Format) ;
    BMP.BuildMips(SrcBMP.GetNMips()) ;

    // Converting to a clut format?
    if (NewFormat.ClutBased)
    {
        // O-oh - time for the quantizer to do it's stuff....

        // Create space for new palette
        s32     NPaletteColors = 1 << NewFormat.BPP ;
        xcolor* Palette = new xcolor[NPaletteColors] ;
        ASSERT(Palette) ;

        // For the quantizer we need to build a single list of colors which includes all mips!
        NColors = 0 ;
        for (mip = 0 ; mip <= SrcBMP.GetNMips() ; mip++)
            NColors += SrcBMP.GetWidth(mip) * SrcBMP.GetHeight(mip) ;

        // Allocate space for all the colors, and set them up
        xcolor* Colors = new xcolor[NColors] ;
        ASSERT(Colors) ;
        i = 0 ;
        for (mip = 0 ; mip <= SrcBMP.GetNMips() ; mip++)
        for (y = 0 ; y < SrcBMP.GetHeight(mip) ; y++)
        for (x = 0 ; x < SrcBMP.GetWidth(mip) ;  x++)
            Colors[i++] = SrcBMP.GetPixelColor(x,y, mip) ;
        ASSERT(i == NColors) ;

        // Let the quantizer do it's thing
        quant_Begin() ;
        quant_SetPixels(Colors, NColors) ;
        quant_End(Palette, NPaletteColors, (NewFormat.ABits > 0)) ;

        // Copy the new generated palette
        for (i = 0 ; i < NPaletteColors ; i++)
            BMP.SetClutColor(Palette[i], i) ;

        // Now setup the pixels (yuk this is slow!) letting xbitmap do all the format conversion work...
        cmap_Begin( Palette, NPaletteColors, (NewFormat.ABits > 0)) ;
        for (mip = 0 ; mip <= BMP.GetNMips() ; mip++)
        for (y = 0 ; y < BMP.GetHeight(mip) ; y++)
        for (x = 0 ; x < BMP.GetWidth(mip) ;  x++)
            BMP.SetPixelIndex(cmap_GetIndex(SrcBMP.GetPixelColor(x,y, mip)), x,y, mip) ;
        cmap_End() ;

        // Free allocated memory
        delete [] Colors ;
        delete [] Palette ;
    }
    else
    {
        // Easy - convert to a non-clut format

        // Now setup all the mips letting xbitmap do all the format conversion work...
        for (mip = 0 ; mip <= BMP.GetNMips() ; mip++)
        for (y = 0 ; y < BMP.GetHeight(mip) ; y++)
        for (x = 0 ; x < BMP.GetWidth(mip) ;  x++)
            BMP.SetPixelColor(SrcBMP.GetPixelColor(x,y, mip), x,y, mip) ;
    }
}


// Compresses bitmap by splitting it up into 2 bitmaps:
// A base bitmap (a quarter of the size of the original) and a 
// luminance bitmap (same size as original, but only 4 bits per pixel)
void auxbmp_CompressPS2( const xbitmap& SrcBMP, xbitmap& BaseBMP, xbitmap& LumBMP, xbool Subtractive )
{
    s32 i,x,y,bx,by, mip, MipWidth, MipHeight, BaseMip ;
    s32 BlockSize, BBP ;

    // Setup compression type
    // BlockSize=4, BBP=8,  Ratio=0.56635:1
    // BlockSize=4, BBP=16, Ratio=0.625:1
    // BlockSize=2, BBP=8,  Ratio=0.75:1
    BlockSize=4 ; BBP=8 ;   // best compression

    // Lookup source size
    s32 SrcWidth  = SrcBMP.GetWidth() ;
    s32 SrcHeight = SrcBMP.GetHeight() ;

    // Create a 32bit base bitmap
    s32     BaseWidth  = SrcWidth  / BlockSize ;
    s32     BaseHeight = SrcHeight / BlockSize ;
    byte*   BaseData = (byte *)x_malloc(BaseWidth * BaseHeight * 4) ; // 32bit
    ASSERT(BaseData) ;
    BaseBMP.Setup(SrcBMP.HasAlphaBits() ? xbitmap::FMT_32_ARGB_8888 : xbitmap::FMT_32_URGB_8888,  // Format
                 BaseWidth,                   // Width
                 BaseHeight,                  // Height
                 TRUE,                        // DataOwned
                 BaseData,                    // PixelData
                 FALSE,                       // Clut owned
                 NULL) ;                      // Clut data

    // Create space for mips?
    if (SrcBMP.GetNMips())
        BaseBMP.BuildMips() ;

    // Loop through and generate all mips
    // NOTE - Mips can't be generated with the normal xbitmap stuff since we need the best
    //        pixel match for the compression algorithm - not the blur xbitmap mip generating algorithm!
    //        (If you use the regular xbitmap build bitmaps, then the mips will incorrectly get darker
    //         for subtractive compression)
    for (mip = 0 ; mip <= BaseBMP.GetNMips() ; mip++)
    {
        // Lookup base size
        MipWidth  = BaseBMP.GetWidth(mip) ;
        MipHeight = BaseBMP.GetHeight(mip) ;

        // Get block size to average
        // (use corresponding mip of source texture since that's what we want to look like!)
        BlockSize = SrcBMP.GetWidth(mip) / MipWidth ;

        // Create 32bit base colour map
        for( by=0; by < MipHeight ; by++ )
        for( bx=0; bx < MipWidth  ; bx++ )
        {
            // Calculate average color for block
            s32    Max[4]={0};
            s32    Min[4]={255,255,255,255};
            xcolor BaseCol;

            if (Subtractive)
            {
                // For subtractive, take the brightest color
                s32 L=-1 ;
                for( y=0; y<BlockSize; y++ )
                for( x=0; x<BlockSize; x++ )
                {
                    // Keep brightest color (always use top level mip for best color!)
                    xcolor PC = SrcBMP.GetPixelColor( (bx*BlockSize)+x, (by*BlockSize)+y, mip );
                    s32 B = SQR(PC.R) + SQR(PC.G) + SQR(PC.B) ;
                    if (B > L)
                    {
                        L = B ;
                        BaseCol = PC ;
                    }
                }
            }
            else
            {
                // For multiplicative, take the middle of the min/max of each component
                for( y=0; y<BlockSize; y++ )
                for( x=0; x<BlockSize; x++ )
                {
                    // Always use top level mip for best color!
                    xcolor PC = SrcBMP.GetPixelColor( (bx*BlockSize)+x, (by*BlockSize)+y, mip );
                    Max[0] = MAX(Max[0],PC.R);
                    Max[1] = MAX(Max[1],PC.G);
                    Max[2] = MAX(Max[2],PC.B);
                    Max[3] = MAX(Max[3],PC.A);

                    Min[0] = MIN(Min[0],PC.R);
                    Min[1] = MIN(Min[1],PC.G);
                    Min[2] = MIN(Min[2],PC.B);
                    Min[3] = MIN(Min[3],PC.A);
                }

                BaseCol.R = (Max[0] + Min[0]) >> 1;
                BaseCol.G = (Max[1] + Min[1]) >> 1;
                BaseCol.B = (Max[2] + Min[2]) >> 1;
                BaseCol.A = (Max[3] + Min[3]) >> 1;
            }

            // Set base color
            BaseBMP.SetPixelColor(BaseCol, bx, by, mip) ;
        }
    }

    // Convert back to original format keeping the mips as they have been calculated!
    auxbmp_ConvertFormat( BaseBMP, SrcBMP.GetFormat() ) ;

    // Use even spread of colors/luminances for the 16 palette entries
    s32 CL[16] ;
    for (i = 0 ; i < 16 ; i++)
        CL[i] = (s32)((f32)i * (255.0f/15.0f)) ;

    // Create a palettized lum bitmap same size as source, but only 4bits per pixel
    s32     LumWidth  = SrcWidth  ;
    s32     LumHeight = SrcHeight ;
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

    // Create space for mips?
    if (SrcBMP.GetNMips())
        LumBMP.BuildMips() ;

    // Setup lum bitmap palette
    for (i = 0 ; i < 16 ; i++)
    {
        u8 C = (u8)CL[i] ;
        if (Subtractive)
            LumBMP.SetClutColor(xcolor(C, C, C, 128), i) ;  // Make SrcA represent one for GS alpha blend mode
        else
            LumBMP.SetClutColor(xcolor(C, C, C, C), i) ;    // RGB doesn't matter...
    }

    // Create luminance mip maps
    for( mip=0; mip <= LumBMP.GetNMips() ; mip++)
    {
        // Lookup base mip to use
        BaseMip    = MIN(BaseBMP.GetNMips(), mip) ;

        // Lookup lum map size
        LumWidth   = LumBMP.GetWidth(mip) ;
        LumHeight  = LumBMP.GetHeight(mip) ;

        // Setup all luminance pixels in this mip
        for( y=0; y < LumHeight ; y++ )
        for( x=0; x < LumWidth ; x++ )
        {
            // Get base map color
            // NOTE: Read with bi-linear just like the hardware will render!
            f32 u = (f32)x / (f32)LumWidth ;
            f32 v = (f32)y / (f32)LumHeight ;
            xcolor BaseCol = BaseBMP.GetBilinearColor(u,v,TRUE, BaseMip) ;

            // Lookup the real actual color that we are trying to look like!
            xcolor SrcCol = SrcBMP.GetPixelColor(x, y, mip) ;

            // Find best matching luminance from palette
            s32 BI=0;
            s32 BD=S32_MAX ;
            for( i=0; i<16; i++ )
            {
                s32 C[3];
                s32 L = CL[i] ;
                if (Subtractive)
                {
                    // Get color that GS will work out
                    C[0] = BaseCol.R - L ;
                    C[1] = BaseCol.G - L ;
                    C[2] = BaseCol.B - L ;
                
                    // Since the GS can clamp to zero, we can do that here
                    for (s32 k = 0 ; k < 3 ; k++)
                    {
                        if (C[k] < 0)
                            C[k] = 0 ;
                    }
                }
                else
                {
                    // Get color that GS will work out
                    C[0] = ((s32)BaseCol.R * L) >> 7;
                    C[1] = ((s32)BaseCol.G * L) >> 7;
                    C[2] = ((s32)BaseCol.B * L) >> 7;
                }

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
                        //BI = (((i * SrcCol.A)+64) >> 7) ; // TAKE SRC ALPHA INTO ACCOUNT!
                        BI = i ;
                        BD = D ;
                    }
                }
            }

            // Set luminance color
            LumBMP.SetPixelIndex(BI, x,y, mip) ;
        }
    }

    // Put base map into ps2 format
    auxbmp_ConvertToPS2( BaseBMP ) ;

    // Put luminance map into PS2 format
    // (NOTE: Can't call auxbmp_ConvertToPS2 because it halfs all the alphas,
    //        and besides, there's no need since we are already almost have a ps2 format!)
    LumBMP.PS2SwizzleClut();
    LumBMP.Flip4BitNibbles() ;
}

//==============================================================================

// Util function for getting height
s32 auxbmp_GetHeight(const xbitmap& BMP, s32 x, s32 y)
{
    xcolor Col = BMP.GetPixelColor(x,y) ;
    s32    H   = (Col.R + Col.G + Col.B) / 3 ;
    return H ;
}

//=============================================================================

// Creates a dot3 map from the source grey scale height map
void auxbmp_ConvertToDot3(const xbitmap& SrcBMP, xbitmap& DstBMP)
{
    // Keep original format
    xbitmap::format DstFormat = DstBMP.GetFormat() ;

    // Convert to 32bit if it's a clut format
    if (DstBMP.IsClutBased())
        DstBMP.ConvertFormat(xbitmap::FMT_32_ARGB_8888) ;

    // Get sizes
    s32 BumpWidth  = SrcBMP.GetWidth() ;
    s32 BumpHeight = SrcBMP.GetHeight() ;

    // Create dot 3 map
    for( s32 y=0; y< BumpHeight; y++ )
    {
        for( s32 x=0; x< BumpWidth; x++ )
        {
            s32 H00 = auxbmp_GetHeight(SrcBMP, x,y) ;                       // Get the current pixel
            s32 H10 = auxbmp_GetHeight(SrcBMP, x, MIN(y+1, BumpHeight-1));  // and the pixel one line below.
            s32 H01 = auxbmp_GetHeight(SrcBMP, MIN(x+1, BumpWidth-1), y) ;  // and the pixel to the right

            vector3 vPoint00( (f32)x+0.0f, (f32)y+0.0f, (f32)H00 );
            vector3 vPoint10( (f32)x+1.0f, (f32)y+0.0f, (f32)H10 );
            vector3 vPoint01( (f32)x+0.0f, (f32)y+1.0f, (f32)H01 );
            vector3 v10 = vPoint10 - vPoint00;
            vector3 v01 = vPoint01 - vPoint00;

            vector3 vNormal;
            vNormal = v3_Cross( v10, v01 );
            vNormal.Normalize() ;

            // Scale -1 -> 1 to 0->255
            vNormal += vector3(1,1,1) ;
            vNormal *= 0.5f * 255.0f ;

            // Set final color
            xcolor Col ;
            Col.R = (u8)vNormal.GetX() ;
            Col.G = (u8)vNormal.GetY() ;
            Col.B = (u8)vNormal.GetZ() ;
            Col.A = (u8)H00 ;
            DstBMP.SetPixelColor(Col, x,y) ;
        }
    }

    // Convert back to original format
    DstBMP.ConvertFormat(DstFormat) ;
}

//=============================================================================

#ifndef TARGET_PS2

#if 1
#   define VERBOSE0(s        ) x_fprintf( File,s         )
#   define VERBOSE1(s,a      ) x_fprintf( File,s,a       )
#   define VERBOSE2(s,a,b    ) x_fprintf( File,s,a,b     )
#   define VERBOSE3(s,a,b,c  ) x_fprintf( File,s,a,b,c   )
#   define VERBOSE4(s,a,b,c,d) x_fprintf( File,s,a,b,c,d )
#else
#   define VERBOSE0(s        )
#   define VERBOSE1(s,a      )
#   define VERBOSE2(s,a,b    )
#   define VERBOSE3(s,a,b,c  )
#   define VERBOSE4(s,a,b,c,d)
#endif

//==============================================================================

static f32 GetAlphaError( xbitmap& BMP,xbitmap& DXT )
{
    s32 h = BMP.GetHeight();
    s32 w = BMP.GetWidth ();
    s32 x;
    s32 y;

    f32 AccError = 0.0f;
    f32 Count    = 0.0f;

    for( y=0;y<h;y++ )
    for( x=0;x<w;x++ )
    {
        xcolor a = BMP.GetPixelColor( x,y,0 );
        xcolor b = DXT.GetPixelColor( x,y,0 );
        f32 e0 = f32( a.A )/255.0f;
        f32 e1 = f32( b.A )/255.0f;
        AccError += x_abs( e1-e0 );
        Count += 1.0f;
    }
    if( Count )
        return AccError / Count;

    return 0.0f;
}

//=========================================================================

xbool auxbmp_IsPunchthruAlpha( xbitmap& Bmp )
{
    s32 h = Bmp.GetHeight();
    s32 w = Bmp.GetWidth ();
    s32 c = 0;
    s32 x;
    s32 y;

    for( y=0;y<h;y++ )
    for( x=0;x<w;x++ )
    {
        xcolor C = Bmp.GetPixelColor( x,y,0 );
        if( C.A && (C.A != 255) )
            return FALSE;
        if( !C.A )
        {
            if( C.R || C.G || C.B )
                return FALSE;
            c++;
        }
    }
    return ( c != (w*h) );
}

//=============================================================================

xbitmap auxbmp_ConvertRGBToA8( xbitmap& BMP )
{
    xbitmap TMP;
    s32 nMips = BMP.GetNMips ();
    s32 W     = BMP.GetWidth ();
    s32 H     = BMP.GetHeight();
    TMP.Setup(
         xbitmap::FMT_A8,
         W,
         H,
         TRUE,
         NULL,
         FALSE,
         NULL,
         -1,
         nMips );
    s32  iMip;
    for( iMip=0;iMip<=nMips;iMip++ )
    {
        u8* pData = (u8*)TMP.GetPixelData( iMip );
        s32 x,y;
        for( y=0;y<H;y++ )
        {
            for( x=0;x<W;x++ )
            {
                pData[0] = BMP.GetPixelColor( x,y,iMip ).A;
                pData++;
            }
        }
        W >>= 1;
        H >>= 1;
    }
    return TMP;
}

//=============================================================================

f32 auxbmp_CompareRGB( xbitmap& BMP,xbitmap& CMP )
{
    f32 Result = 0.0f;
    f32 Num    = 0.0f;
    {
        s32 h = BMP.GetHeight();
        s32 w = BMP.GetWidth ();
        s32 x,y;

        for( y=0;y<h;y++ )
        for( x=0;x<w;x++ )
        {
            xcolor a = BMP.GetPixelColor( x,y,0 );
            xcolor b = CMP.GetPixelColor( x,y,0 );

            vector3 Src( f32(a.R)/255.0f,f32(a.G)/255.0f,f32(a.B)/255 );
            vector3 Dst( f32(b.R)/255.0f,f32(b.G)/255.0f,f32(b.B)/255 );

            f32 Distance = (Dst-Src).Length();
            Result += Distance;
            Num    += 1.0f;
        }
        if( Num )
            Result /= Num;
    }
    return Result;
}

//=============================================================================

f32 auxbmp_GetDotProduct( xbitmap& BMP,xbitmap& CMP )
{
    f32 Result = 0.0f;
    f32 Num    = 0.0f;
    {
        s32 h = BMP.GetHeight();
        s32 w = BMP.GetWidth ();
        s32 x,y;

        for( y=0;y<h;y++ )
        for( x=0;x<w;x++ )
        {
            xcolor a = BMP.GetPixelColor( x,y,0 );
            xcolor b = CMP.GetPixelColor( x,y,0 );

            vector3 Src( f32(a.R)/255.0f,f32(a.G)/255.0f,f32(a.B)/255 );
            vector3 Dst( f32(b.R)/255.0f,f32(b.G)/255.0f,f32(b.B)/255 );

            Src.SafeNormalize();
            Dst.SafeNormalize();

            Result += x_abs( Dst.Dot( Src ));
            Num    += 1.0f;
        }
        if( Num )
            Result /= Num;
    }
    return Result;
}

//=============================================================================

#define D3DFMT_LIN_A8R8G8B8         0x00000012
#define D3DFMT_LIN_X8R8G8B8         0x0000001E
#define D3DFMT_DXT1                 0x0000000C
#define D3DFMT_DXT2                 0x0000000E
#define D3DFMT_DXT3                 0x0000000E
#define D3DFMT_DXT4                 0x0000000F
#define D3DFMT_DXT5                 0x0000000F

#define XGCOMPRESS_PREMULTIPLY      1
#define XGCOMPRESS_NEEDALPHA0       2
#define XGCOMPRESS_NEEDALPHA1       4
#define XGCOMPRESS_PROTECTNONZERO   8

extern "C"
{
    s32 __stdcall XGCompressRect( const void*,s32,s32,s32,s32,const void*,s32,s32,f32,u32 );
    u32 __stdcall XGBytesPerPixelFromFormat( u32 );
}

//=============================================================================

xbitmap auxbmp_CompressRect( xbitmap& BMP,s32 Format )
{
    s32 H = BMP.GetHeight();
    s32 W = BMP.GetWidth ();
    s32 N = BMP.GetNMips ();
    s32 P;

    // create empty result ....................................................

    xbitmap Result;
    {
        xbitmap::format RForm; switch( Format )
        {
            case D3DFMT_DXT1: RForm = xbitmap::FMT_DXT1; P = W/4* 8; break;
            case D3DFMT_DXT3: RForm = xbitmap::FMT_DXT3; P = W/4*16; break;
            case D3DFMT_DXT5: RForm = xbitmap::FMT_DXT5; P = W/4*16; break;

            default:
                return BMP;
        }
        Result.Setup( RForm,W,H,TRUE,NULL,FALSE,NULL,-1,N );
    }

    // choose source D3D format ...............................................

    s32 SrcFormat; switch( BMP.GetFormat() )
    {
        case xbitmap::FMT_32_ARGB_8888: SrcFormat = D3DFMT_LIN_A8R8G8B8; break;
        case xbitmap::FMT_32_URGB_8888: SrcFormat = D3DFMT_LIN_X8R8G8B8; break;

        default:
            return BMP;
    }

    // compress all mip levels  ...............................................

    for( s32 Mip=0;Mip<=N;Mip++ )
    {
        XGCompressRect
        (
            Result.GetPixelData(Mip)                , // pDestBuf
            Format                                  , // DestFormat
            P                                       , // dwDestPitch
            W                                       , // dwWidth
            H                                       , // dwHeight
            BMP.GetPixelData(Mip)                   , // pSrcData
            SrcFormat                               , // SrcFormat
            XGBytesPerPixelFromFormat( SrcFormat )*W, // dwSrcPitch
            0.0f                                    , // fAlphaRef
            XGCOMPRESS_NEEDALPHA0                     // dwFlags
                |   XGCOMPRESS_PROTECTNONZERO
        );
        W >>= 1;
        H >>= 1;
        P >>= 1;
    }
    return Result;
}

//=============================================================================

void auxbmp_Decompress( xbitmap& BMP )
{
    extern xbitmap
        UnpackImage( const xbitmap& );
    BMP=UnpackImage( BMP );
}

//=============================================================================

static u32 BitScanReverse( u32 Mask )
{
    u32 Bit = 31;
    while( Bit )
    {
        if( Mask & (1<<Bit) )
            return (1<<Bit);
        Bit--;
    }
    ASSERT( 0 );
    return( 0 );
}

//=============================================================================

void auxbmp_Compress( xbitmap& BMP,const char* pName,s32 nMips )
{
    // BAIL IF NOTHING TO DO **************************************************

    switch( BMP.GetFormat() )
    {
        case xbitmap::FMT_DXT1:
        case xbitmap::FMT_DXT2:
        case xbitmap::FMT_DXT3:
        case xbitmap::FMT_DXT4:
        case xbitmap::FMT_DXT5:
            return;
    }
    
    // REDUCE NON POWER OF TWO TEXTURES ***************************************

    auxbmp_ConvertToD3D( BMP );
    if( nMips )
        BMP.BuildMips( nMips );

    s32 Width  = BMP.GetWidth();
    s32 Height = BMP.GetHeight();
    if( (((Width -1) & Width ) != 0) ||
        (((Height-1) & Height) != 0) )
    {
        return;
    }

    // ALPHA TEXTURES *********************************************************

    s32 bHasAlpha = BMP.HasAlphaBits();
    if( bHasAlpha )
    {
        if( !auxbmp_IsPunchthruAlpha( BMP ))
        {
            BMP = auxbmp_CompressRect( BMP,D3DFMT_DXT5 );
            return;
        }
    }

    // OPAQUE TEXTURES ********************************************************

    xbitmap DXT1 = auxbmp_CompressRect( BMP,D3DFMT_DXT1 );
    f32 DXT1_Distance = auxbmp_CompareRGB( BMP,DXT1 );
    if( DXT1_Distance < 0.5f )
    {
        BMP = DXT1;
        return;
    }
}
#endif
