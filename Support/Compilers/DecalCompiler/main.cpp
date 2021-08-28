#include <io.h>
#include <stdio.h>
#include "x_files.hpp"
#include "CommandLine.hpp"
#include "Auxiliary\Bitmap\Aux_bitmap.hpp"
#include "..\Support\Decals\DecalPackage.hpp"
#include "..\Support\Compilers\GeomCompiler\BMPUtil.hpp"

//=========================================================================
// Types
//=========================================================================

enum export_platform
{
    EXPORT_UNKNOWN = 0,
    EXPORT_PC,
    EXPORT_PS2,
    EXPORT_XBOX
};

enum format_flags
{
    BITMAP_FORMAT_8BIT      = 0x1,
    BITMAP_FORMAT_INTENSITY = 0x2,
};

//=========================================================================
// Statics
//=========================================================================

static          xbool       s_Verbose         = FALSE;
static          s32         s_Platform        = EXPORT_UNKNOWN;
static struct   _finddata_t s_ExeData;
static struct   _finddata_t s_SrcBitmapData;
static struct   _finddata_t s_DstBitmapData;

//=========================================================================
// Implementation
//=========================================================================

void ForceDecalLoaderLink( void )
{
}

//=========================================================================

const char* CompileBitmap( const char* pOutputPath, const char* pSourceBitmap, u32 FormatFlags )
{
    // figure out what the name of the final bitmap should be
    char SrcDrive[X_MAX_DRIVE];
    char SrcDir[X_MAX_DIR];
    char SrcFName[X_MAX_FNAME];
    char SrcExt[X_MAX_EXT];
    char DstDrive[X_MAX_DRIVE];
    char DstDir[X_MAX_DIR];
    char DstFName[X_MAX_FNAME];
    char DstExt[X_MAX_EXT];
    static char FinalPath[X_MAX_PATH];

    FinalPath[0] = '\0';
    x_splitpath( pSourceBitmap, SrcDrive, SrcDir, SrcFName, SrcExt );
    x_splitpath( pOutputPath, DstDrive, DstDir, DstFName, DstExt );
    if ( FormatFlags & BITMAP_FORMAT_INTENSITY )
        x_strcat( SrcFName, "[I]" );
    else if ( FormatFlags & BITMAP_FORMAT_8BIT )
        x_strcat( SrcFName, "[8]" );
    else
        x_strcat( SrcFName, "[4]" );
    x_makepath( FinalPath, DstDrive, DstDir, SrcFName, "xbmp" );

    // see if the output file is already up-to-date by comparing it to
    // the source bitmap and to the timestamp of this exe
    xbool bOutOfDate = FALSE;
    s_SrcBitmapData.time_write = 0;
    s_DstBitmapData.time_write = 0;
    if ( _findfirst( pSourceBitmap, &s_SrcBitmapData ) == -1 )
    {
        x_throw( "Unable to locate source bitmap." );
        return FinalPath;
    }
    
    if ( _findfirst( FinalPath, &s_DstBitmapData ) == -1 )
    {
        // the destination bitmap doesn't exist yet, so obviously we need
        // to compile it
        bOutOfDate = TRUE;
    }
    else
    {
        // compare timestamps to see if we should compile the xbmp
        if ( s_DstBitmapData.time_write <= s_SrcBitmapData.time_write )
            bOutOfDate = TRUE;

        if ( s_DstBitmapData.time_write <= s_ExeData.time_write )
            bOutOfDate = TRUE;
    }

    // handle the bitmap compilation
    if ( bOutOfDate == TRUE )
    {
        //load the bitmap
        xbitmap BMP;
        xbool   Result = auxbmp_Load( BMP, pSourceBitmap );
        if ( !Result )
        {
            x_throw( "Unable to load source bitmap." );
            return FinalPath;
        }

        // convert to the platform's native format
        switch ( s_Platform )
        {
        default:
        case EXPORT_UNKNOWN:
            x_throw( "Internal error: platform not specified" );
            break;
        case EXPORT_PC:
            if ( FormatFlags & BITMAP_FORMAT_INTENSITY )
                bmp_util::ProcessDetailMap( BMP, FALSE );
            auxbmp_ConvertToD3D( BMP );
            break;
        case EXPORT_PS2:
            if ( FormatFlags & BITMAP_FORMAT_INTENSITY )
            {
                bmp_util::ProcessDetailMap( BMP, TRUE );
                ASSERT( BMP.GetBPP() <= 8 );
                BMP.ConvertFormat( xbitmap::FMT_P4_ABGR_8888 );
                bmp_util::ConvertToPS2( BMP, FALSE );
            }
            else
            if ( FormatFlags & BITMAP_FORMAT_8BIT )
            {
                BMP.ConvertFormat( xbitmap::FMT_P8_ABGR_8888 );
                bmp_util::ConvertToPS2( BMP, TRUE );
            }
            else
            {
                BMP.ConvertFormat( xbitmap::FMT_P4_ABGR_8888 );
                bmp_util::ConvertToPS2( BMP, TRUE );
            }
            break;

            case EXPORT_XBOX:
            {
                s32 nMips = 4;
                if( FormatFlags & BITMAP_FORMAT_INTENSITY )
                {
                    bmp_util::ProcessDetailMap( BMP,FALSE );
                    auxbmp_ConvertToD3D( BMP );
                    BMP.BuildMips( nMips );
                    auxbmp_ConvertRGBToA8( BMP );
                }
                else
                {
                    auxbmp_Compress( BMP,pSourceBitmap,nMips );
                }
                break;
            }
        }

        // save out the bitmap
        Result = BMP.Save( FinalPath );
        if ( !Result )
            x_throw( xfs("Unable to save %s.", FinalPath) );
    }

    x_makepath( FinalPath, NULL, NULL, SrcFName, "xbmp" );
    return FinalPath;
}

//=========================================================================

void CompileBitmaps( const char*      pOutputPath,
                     decal_package&   DecalPkg,
                     xarray<xstring>& SourceBitmapNames,
                     xarray<u32>&     PreferredBitmapFormats )
{
    if ( DecalPkg.GetNDecalDefs() != SourceBitmapNames.GetCount() )
    {
        x_throw( "# of decals doesn't match number of bitmaps" );
        return;
    }

    // compile each of the bitmaps in the appropriate formats (this will
    // also check timestamps for us, and skip the compile if it doesn't
    // really need to do it)
    s32 i;
    for ( i = 0; i < DecalPkg.GetNDecalDefs(); i++ )
    {
        decal_definition& DecalDef = DecalPkg.GetDecalDef(i);

        const char* pBitmapName = CompileBitmap( pOutputPath, SourceBitmapNames[i], PreferredBitmapFormats[i] );
        x_strsavecpy( DecalDef.m_BitmapName, pBitmapName, 256 );
    }
}

//=========================================================================

void ExecuteScript( command_line& CommandLine )
{  
    s32                     i, j;
    decal_package           DecalPkg;
    xarray<xstring>         SourceBitmapNames;
    xarray<u32>             PreferredBitmapFormats;
    xbool                   bPlatform = FALSE;
    s32                     CurrGroup = -1;
    s32                     CurrDecal = -1;

    x_try;

    for( i=0; i<CommandLine.GetNumOptions(); i++ )
    {
        // Get option name and string
        xstring OptName = CommandLine.GetOptionName( i );

        // should we be verbose?
        if ( OptName == xstring( "LOG" ) )
            s_Verbose = TRUE;

        // number of groups
        if ( OptName == xstring( "NUMGROUPS" ) )
        {
            xstring NumGroups = CommandLine.GetOptionString( i );
            s32     nGroups = 0;
            sscanf( NumGroups, "%d", &nGroups );
            if ( nGroups )
                DecalPkg.AllocGroups( nGroups );
            CurrGroup = -1;
        }

        // number of decals
        if ( OptName == xstring( "NUMDECALS" ) )
        {
            xstring NumDecals = CommandLine.GetOptionString( i );
            s32     nDecals = 0;
            sscanf( NumDecals, "%d", &nDecals );
            if ( nDecals )
                DecalPkg.AllocDecals( nDecals );
            CurrDecal = -1;

            // all decal flags start off as zero until we OR in the
            // option-specified ones (rather than just using defaults)
            for ( j = 0; j < DecalPkg.GetNDecalDefs(); j++ )
            {
                decal_definition& DecalDef = DecalPkg.GetDecalDef(j);
                DecalDef.m_Flags = 0;
            }


            // allocate enough bitmap names and format info data
            SourceBitmapNames.SetCount( nDecals );
            PreferredBitmapFormats.SetCount( nDecals );
            for ( j = 0; j < nDecals; j++ )
            {
                SourceBitmapNames[j].Clear();
                PreferredBitmapFormats[j] = 0;
            }
        }

        // start of a new group
        if( OptName == xstring( "GROUP" ) )
        {
            if ( !DecalPkg.GetNGroups() )
                continue;

            // set the current index to the next group
            CurrGroup++;
            CurrGroup = MIN( CurrGroup, DecalPkg.GetNGroups()-1 );

            // fill in the group name
            DecalPkg.SetGroupName( CurrGroup, CommandLine.GetOptionString( i ) );
        }

        // group color
        if( OptName == xstring( "GROUPCOLOR" ) )
        {
            if ( !DecalPkg.GetNGroups() )
                continue;

            xstring ColorString = CommandLine.GetOptionString( i );
            xcolor  Color = XCOLOR_WHITE;
            sscanf( ColorString, "%x", &Color );
            DecalPkg.SetGroupColor( CurrGroup, Color );
        }

        // group decal start offset
        if( OptName == xstring( "DECALSTART" ) )
        {
            if ( !DecalPkg.GetNGroups() )
                continue;

            xstring DecalStart = CommandLine.GetOptionString( i );
            s32     iDecal = 0;
            sscanf( DecalStart, "%d", &iDecal );
            DecalPkg.SetGroupDecalDefStart( CurrGroup, iDecal );
        }

        // group decal count
        if( OptName == xstring( "DECALCOUNT" ) )
        {
            if ( !DecalPkg.GetNGroups() )
                continue;

            xstring DecalCount = CommandLine.GetOptionString( i );
            s32     nDecals = 0;
            sscanf( DecalCount, "%d", &nDecals );
            DecalPkg.SetGroupDecalDefCount( CurrGroup, nDecals );
        }

        // start of a new decal
        if( OptName == xstring( "DECAL" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            // set the current index to the next decal
            CurrDecal++;
            CurrDecal = MIN( CurrDecal, DecalPkg.GetNDecalDefs()-1 );

            // fill in the decal name
            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            x_strsavecpy( DecalDef.m_Name, CommandLine.GetOptionString( i ), 32 );
        }

        // min width
        if ( OptName == xstring( "MINWIDTH" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            xstring           MinWidth = CommandLine.GetOptionString( i );
            sscanf( MinWidth, "%f", &DecalDef.m_MinSize.X );
        }

        // min height
        if ( OptName == xstring( "MINHEIGHT" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef  = DecalPkg.GetDecalDef( CurrDecal );
            xstring           MinHeight = CommandLine.GetOptionString( i );
            sscanf( MinHeight, "%f", &DecalDef.m_MinSize.Y );
        }

        // max width
        if ( OptName == xstring( "MAXWIDTH" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            xstring           MaxWidth = CommandLine.GetOptionString( i );
            sscanf( MaxWidth, "%f", &DecalDef.m_MaxSize.X );
        }

        // max height
        if ( OptName == xstring( "MAXHEIGHT" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef  = DecalPkg.GetDecalDef( CurrDecal );
            xstring           MaxHeight = CommandLine.GetOptionString( i );
            sscanf( MaxHeight, "%f", &DecalDef.m_MaxSize.Y );
        }

        // min roll
        if ( OptName == xstring( "MINROLL" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            xstring           MinRoll  = CommandLine.GetOptionString( i );
            sscanf( MinRoll, "%f", &DecalDef.m_MinRoll );
        }

        // max roll
        if ( OptName == xstring( "MAXROLL" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            xstring           MaxRoll  = CommandLine.GetOptionString( i );
            sscanf( MaxRoll, "%f", &DecalDef.m_MaxRoll );
        }

        // color
        if ( OptName == xstring( "COLOR" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            xstring           Color    = CommandLine.GetOptionString( i );
            sscanf( Color, "%x", &DecalDef.m_Color );
        }

        // max vis
        if ( OptName == xstring( "MAXVIS" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            xstring           MaxVis   = CommandLine.GetOptionString( i );
            sscanf( MaxVis, "%d", &DecalDef.m_MaxVisible );
        }

        // bitmap
        if ( OptName == xstring( "BITMAP" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            SourceBitmapNames[CurrDecal] = CommandLine.GetOptionString( i );
        }

        // format
        if ( OptName == xstring( "P8" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            PreferredBitmapFormats[CurrDecal] |= BITMAP_FORMAT_8BIT;
        }

        if ( OptName == xstring( "P4" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            PreferredBitmapFormats[CurrDecal] &= ~BITMAP_FORMAT_8BIT;
        }

        // flags
        if ( OptName == xstring( "USE_TRI" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_Flags |= decal_definition::DECAL_FLAG_USE_TRI;
        }

        if ( OptName == xstring( "NO_CLIP" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_Flags |= decal_definition::DECAL_FLAG_NO_CLIP;
        }

        if ( OptName == xstring( "USE_PROJECTION" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_Flags |= decal_definition::DECAL_FLAG_USE_PROJECTION;
        }

        if ( OptName == xstring( "KEEP_SIZE_RATIO" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_Flags |= decal_definition::DECAL_FLAG_KEEP_SIZE_RATIO;
        }

        if ( OptName == xstring( "PERMANENT" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_Flags |= decal_definition::DECAL_FLAG_PERMANENT;
        }

        if ( OptName == xstring( "FADE_OUT" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_Flags |= decal_definition::DECAL_FLAG_FADE_OUT;

            xstring FadeTime = CommandLine.GetOptionString( i );
            sscanf( FadeTime, "%f", &DecalDef.m_FadeTime );
        }

        if( OptName == xstring( "ADD_GLOW" ) )
        {
            if( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_Flags |= decal_definition::DECAL_FLAG_ADD_GLOW;
        }

        if( OptName == xstring( "ENV_MAPPED" ) )
        {
            if( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_Flags |= decal_definition::DECAL_FLAG_ENV_MAPPED;
        }

        // blend mode
        if ( OptName == xstring( "BLEND_NORMAL" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_BlendMode = decal_definition::DECAL_BLEND_NORMAL;
        }

        if ( OptName == xstring( "BLEND_ADD" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_BlendMode = decal_definition::DECAL_BLEND_ADD;
        }

        if ( OptName == xstring( "BLEND_SUBTRACT" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_BlendMode = decal_definition::DECAL_BLEND_SUBTRACT;
        }

        if ( OptName == xstring( "BLEND_INTENSITY" ) )
        {
            if ( !DecalPkg.GetNDecalDefs() )
                continue;

            decal_definition& DecalDef = DecalPkg.GetDecalDef( CurrDecal );
            DecalDef.m_BlendMode = decal_definition::DECAL_BLEND_INTENSITY;
            PreferredBitmapFormats[CurrDecal] |= BITMAP_FORMAT_INTENSITY;
        }

        // handle xbox compilation
        if( OptName == xstring( "XBOX" ) )
        {
            xstring OutputFile = CommandLine.GetOptionString( i );
            s_Platform = EXPORT_XBOX;
            CompileBitmaps( OutputFile, DecalPkg, SourceBitmapNames, PreferredBitmapFormats );
            bPlatform = TRUE;

            fileio File;
            File.Save( OutputFile, DecalPkg, FALSE );
        }

        // handle ps2 compilation
        if ( OptName == xstring( "PS2" ) )
        {
            xstring OutputFile = CommandLine.GetOptionString( i );
            s_Platform = EXPORT_PS2;
            CompileBitmaps( OutputFile, DecalPkg, SourceBitmapNames, PreferredBitmapFormats );
            bPlatform = TRUE;

            fileio File;
            File.Save( OutputFile, DecalPkg, FALSE );
        }

        // handle pc compilation
        if ( OptName == xstring( "PC" ) )
        {
            xstring OutputFile = CommandLine.GetOptionString( i );
            s_Platform = EXPORT_PC;
            CompileBitmaps( OutputFile, DecalPkg, SourceBitmapNames, PreferredBitmapFormats );
            bPlatform = TRUE;

            fileio File;
            File.Save( OutputFile, DecalPkg, FALSE );
        }
    }
    
    if( !bPlatform )
    {
        x_throw( "No platform specified!" );
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
    x_printf( "Error: Compiling\n" );
    x_printf( "-LOG                     Verbose mode                        \n" );
    x_printf( "-XBOX <filename>         Output XBox decal                   \n" );
    x_printf( "-PS2 <filename>          Output PS2 decal                    \n" );
    x_printf( "-PC <filename>           Output PC decal                     \n" );
    x_printf( "-NUMGROUPS <int>         Number of groups to expect          \n" );
    x_printf( "-NUMDECALS <int>         Number of decals to expect          \n" );
    x_printf( "-GROUP <string>          Start of a group named <string>     \n" );
    x_printf( "-GROUPCOLOR <hex>        Color of current group              \n" );
    x_printf( "-DECALSTART <int>        Start index of this group's decals  \n" );
    x_printf( "-DECALCOUNT <int>        Number of decals in this group      \n" );
    x_printf( "-DECAL <string>          Start of a decal named <string>     \n" );
    x_printf( "-MINWIDTH <float>        Min Width of decal                  \n" );
    x_printf( "-MINHEIGHT <float>       Min Height of decal                 \n" );
    x_printf( "-MAXWIDTH <float>        Max Width of decal                  \n" );
    x_printf( "-MAXHEIGHT <float>       Max Height of decal                 \n" );
    x_printf( "-MINROLL <float>         Min Roll of decal (in radians)      \n" );
    x_printf( "-MAXROLL <float>         Max Roll of decal (in radians)      \n" );
    x_printf( "-COLOR <hex>             Color of decal                      \n" );
    x_printf( "-MAXVIS <decimal>        Max # of decals visible             \n" );
    x_printf( "-BITMAP <filename>       Bitmap to use (TGA)                 \n" );
    x_printf( "-P8                      Compile bitmap as 8-bit             \n" );
    x_printf( "-P4                      Compile bitmap as 4-bit             \n" );
    x_printf( "-USE_TRI                 Decal can use triangle primitive    \n" );
    x_printf( "-NO_CLIP                 Don't clip decal to polys           \n" );
    x_printf( "-USE_PROJECTION          Use projection mapping to stretch   \n" );
    x_printf( "-KEEP_SIZE_RATIO         Maintain width/height ratio         \n" );
    x_printf( "-PERMANENT               Decal never fades out or disappears \n" );
    x_printf( "-BLEND_NORMAL            Use normal blending                 \n" );
    x_printf( "-BLEND_ADD               Use additive blending               \n" );
    x_printf( "-BLEND_SUBTRACT          Use subtractive blending            \n" );
    x_printf( "-BLEND_INTENSITY         Use intensity mode blending         \n" );
}

//=========================================================================

void main( s32 argc, char* argv[] )
{
    x_try;
    
    x_Init( argc,argv );

    // save out the exe timestamp for doing dependancy checks
    xstring ExePath(argv[0]);
    s_ExeData.time_write = 0;
    _findfirst( ExePath, &s_ExeData );

    command_line CommandLine;
    
    // Specify all the options
    CommandLine.AddOptionDef( "LOG" );
    CommandLine.AddOptionDef( "XBOX",       command_line::STRING );
    CommandLine.AddOptionDef( "PS2",        command_line::STRING );
    CommandLine.AddOptionDef( "PC",         command_line::STRING );
    CommandLine.AddOptionDef( "NUMGROUPS",  command_line::STRING );
    CommandLine.AddOptionDef( "NUMDECALS",  command_line::STRING );
    CommandLine.AddOptionDef( "GROUP",      command_line::STRING );
    CommandLine.AddOptionDef( "GROUPCOLOR", command_line::STRING );
    CommandLine.AddOptionDef( "DECALSTART", command_line::STRING );
    CommandLine.AddOptionDef( "DECALCOUNT", command_line::STRING );
    CommandLine.AddOptionDef( "DECAL",      command_line::STRING );
    CommandLine.AddOptionDef( "MINWIDTH",   command_line::STRING );
    CommandLine.AddOptionDef( "MINHEIGHT",  command_line::STRING );
    CommandLine.AddOptionDef( "MAXWIDTH",   command_line::STRING );
    CommandLine.AddOptionDef( "MAXHEIGHT",  command_line::STRING );
    CommandLine.AddOptionDef( "MINROLL",    command_line::STRING );
    CommandLine.AddOptionDef( "MAXROLL",    command_line::STRING );
    CommandLine.AddOptionDef( "COLOR",      command_line::STRING );
    CommandLine.AddOptionDef( "MAXVIS",     command_line::STRING );
    CommandLine.AddOptionDef( "BITMAP",     command_line::STRING );
    CommandLine.AddOptionDef( "P8" );
    CommandLine.AddOptionDef( "P4" );
    CommandLine.AddOptionDef( "USE_TRI" );
    CommandLine.AddOptionDef( "NO_CLIP" );
    CommandLine.AddOptionDef( "USE_PROJECTION" );
    CommandLine.AddOptionDef( "KEEP_SIZE_RATIO" );
    CommandLine.AddOptionDef( "PERMANENT" );
    CommandLine.AddOptionDef( "FADE_OUT",   command_line::STRING );
    CommandLine.AddOptionDef( "ADD_GLOW" );
    CommandLine.AddOptionDef( "ENV_MAPPED" );
    CommandLine.AddOptionDef( "BLEND_NORMAL" );
    CommandLine.AddOptionDef( "BLEND_ADD" );
    CommandLine.AddOptionDef( "BLEND_SUBTRACT" );
    CommandLine.AddOptionDef( "BLEND_INTENSITY" );

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
