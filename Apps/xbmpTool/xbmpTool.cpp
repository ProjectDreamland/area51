//==============================================================================
//==============================================================================
//  xbmpTool
//==============================================================================
//==============================================================================
//
//  Bitmap conversion tool
//
//
//
//==============================================================================
//==============================================================================

#include "x_files.hpp"
#include "Auxiliary/CommandLine/CommandLine.hpp"
#include "Auxiliary/Bitmap/aux_Bitmap.hpp"
#include "x_bitmap.hpp"

#define D3DFMT_LIN_A8R8G8B8         0x00000012
#define D3DFMT_LIN_X8R8G8B8         0x0000001E
#define D3DFMT_DXT1                 0x0000000C
#define D3DFMT_DXT2                 0x0000000E
#define D3DFMT_DXT3                 0x0000000E
#define D3DFMT_DXT4                 0x0000000F
#define D3DFMT_DXT5                 0x0000000F

//==============================================================================
//  Defines
//==============================================================================

enum target
{
    BITMAP_TARGET_GENERIC,
    BITMAP_TARGET_XBOX,
    BITMAP_TARGET_PS2,
    BITMAP_TARGET_PC,
};

//==============================================================================
//  Conversion Table
//==============================================================================

struct conversion
{
    const char* FormatName;
    s32         Format;
};

conversion  ConversionTable[] =
{
    "32_RGBA_8888", xbitmap::FMT_32_RGBA_8888,  "32_BGRA_8888", xbitmap::FMT_32_BGRA_8888,
    "32_RGBU_8888", xbitmap::FMT_32_RGBU_8888,  "32_BGRU_8888", xbitmap::FMT_32_BGRU_8888,
    "32_ARGB_8888", xbitmap::FMT_32_ARGB_8888,  "32_ABGR_8888", xbitmap::FMT_32_ABGR_8888,
    "32_URGB_8888", xbitmap::FMT_32_URGB_8888,  "32_UBGR_8888", xbitmap::FMT_32_UBGR_8888,
    "24_RGB_888",   xbitmap::FMT_24_RGB_888  ,  "24_BGR_888",   xbitmap::FMT_24_BGR_888  ,
    "16_RGBA_4444", xbitmap::FMT_16_RGBA_4444,  "16_BGRA_4444", xbitmap::FMT_16_BGRA_4444,
    "16_ARGB_4444", xbitmap::FMT_16_ARGB_4444,  "16_ABGR_4444", xbitmap::FMT_16_ABGR_4444,
    "16_RGBA_5551", xbitmap::FMT_16_RGBA_5551,  "16_BGRA_5551", xbitmap::FMT_16_BGRA_5551,
    "16_RGBU_5551", xbitmap::FMT_16_RGBU_5551,  "16_BGRU_5551", xbitmap::FMT_16_BGRU_5551,
    "16_ARGB_1555", xbitmap::FMT_16_ARGB_1555,  "16_ABGR_1555", xbitmap::FMT_16_ABGR_1555,
    "16_URGB_1555", xbitmap::FMT_16_URGB_1555,  "16_UBGR_1555", xbitmap::FMT_16_UBGR_1555,
    "16_RGB_565",   xbitmap::FMT_16_RGB_565  ,  "16_BGR_565  ", xbitmap::FMT_16_BGR_565  ,
                                                
    "P8_RGBA_8888", xbitmap::FMT_P8_RGBA_8888,  "P8_BGRA_8888", xbitmap::FMT_P8_BGRA_8888,
    "P8_RGBU_8888", xbitmap::FMT_P8_RGBU_8888,  "P8_BGRU_8888", xbitmap::FMT_P8_BGRU_8888,
    "P8_ARGB_8888", xbitmap::FMT_P8_ARGB_8888,  "P8_ABGR_8888", xbitmap::FMT_P8_ABGR_8888,
    "P8_URGB_8888", xbitmap::FMT_P8_URGB_8888,  "P8_UBGR_8888", xbitmap::FMT_P8_UBGR_8888,
    "P8_RGB_888",   xbitmap::FMT_P8_RGB_888  ,  "P8_BGR_888",   xbitmap::FMT_P8_BGR_888  ,
    "P8_RGBA_4444", xbitmap::FMT_P8_RGBA_4444,  "P8_BGRA_4444", xbitmap::FMT_P8_BGRA_4444,
    "P8_ARGB_4444", xbitmap::FMT_P8_ARGB_4444,  "P8_ABGR_4444", xbitmap::FMT_P8_ABGR_4444,
    "P8_RGBA_5551", xbitmap::FMT_P8_RGBA_5551,  "P8_BGRA_5551", xbitmap::FMT_P8_BGRA_5551,
    "P8_RGBU_5551", xbitmap::FMT_P8_RGBU_5551,  "P8_BGRU_5551", xbitmap::FMT_P8_BGRU_5551,
    "P8_ARGB_1555", xbitmap::FMT_P8_ARGB_1555,  "P8_ABGR_1555", xbitmap::FMT_P8_ABGR_1555,
    "P8_URGB_1555", xbitmap::FMT_P8_URGB_1555,  "P8_UBGR_1555", xbitmap::FMT_P8_UBGR_1555,
    "P8_RGB_565",   xbitmap::FMT_P8_RGB_565  ,  "P8_BGR_565  ", xbitmap::FMT_P8_BGR_565  ,
                                                
    "P4_RGBA_8888", xbitmap::FMT_P4_RGBA_8888,  "P4_BGRA_8888", xbitmap::FMT_P4_BGRA_8888,
    "P4_RGBU_8888", xbitmap::FMT_P4_RGBU_8888,  "P4_BGRU_8888", xbitmap::FMT_P4_BGRU_8888,
    "P4_ARGB_8888", xbitmap::FMT_P4_ARGB_8888,  "P4_ABGR_8888", xbitmap::FMT_P4_ABGR_8888,
    "P4_URGB_8888", xbitmap::FMT_P4_URGB_8888,  "P4_UBGR_8888", xbitmap::FMT_P4_UBGR_8888,
    "P4_RGB_888",   xbitmap::FMT_P4_RGB_888  ,  "P4_BGR_888",   xbitmap::FMT_P4_BGR_888  ,
    "P4_RGBA_4444", xbitmap::FMT_P4_RGBA_4444,  "P4_BGRA_4444", xbitmap::FMT_P4_BGRA_4444,
    "P4_ARGB_4444", xbitmap::FMT_P4_ARGB_4444,  "P4_ABGR_4444", xbitmap::FMT_P4_ABGR_4444,
    "P4_RGBA_5551", xbitmap::FMT_P4_RGBA_5551,  "P4_BGRA_5551", xbitmap::FMT_P4_BGRA_5551,
    "P4_RGBU_5551", xbitmap::FMT_P4_RGBU_5551,  "P4_BGRU_5551", xbitmap::FMT_P4_BGRU_5551,
    "P4_ARGB_1555", xbitmap::FMT_P4_ARGB_1555,  "P4_ABGR_1555", xbitmap::FMT_P4_ABGR_1555,
    "P4_URGB_1555", xbitmap::FMT_P4_URGB_1555,  "P4_UBGR_1555", xbitmap::FMT_P4_UBGR_1555,
    "P4_RGB_565",   xbitmap::FMT_P4_RGB_565  ,  "P4_BGR_565",   xbitmap::FMT_P4_BGR_565  ,
};

//==============================================================================
//  Helper Functions
//==============================================================================

xstring FormatToString( s32 Format )
{
    s32     i;
    xstring FormatName;

    for( i=0 ; i<sizeof(ConversionTable)/sizeof(conversion) ; i++ )
    {
        if( ConversionTable[i].Format == Format )
            FormatName = ConversionTable[i].FormatName;
    }

    return FormatName;
}

s32 StringToFormat( const xstring& FormatName )
{
    s32     i;
    s32     Format = -1;

    for( i=0 ; i<sizeof(ConversionTable)/sizeof(conversion) ; i++ )
    {
        if( ConversionTable[i].FormatName == FormatName )
            Format = ConversionTable[i].Format;
    }

    return Format;
}

//==============================================================================
//  Display Help
//==============================================================================

void DisplayHelp( void )
{
    x_printf( "\n" );
    x_printf( "xbmpTool (c)2001 Inevitable Entertainment Inc.\n" );
    x_printf( "\n" );
    x_printf( "  usage:\n" );
    x_printf( "         xbmpTool [-opt [param]] [filenames]\n" );
    x_printf( "\n" );
    x_printf( "options:\n" );
    x_printf( "         -output  <folder>   - Set output folder for writing\n" );
    x_printf( "         -convert <format>   - Convert to <format> see list below\n" );
    x_printf( "         -mips    <num mips> - Build mips\n" );
    x_printf( "         -r                  - Recurse on subdirectories\n" );
    x_printf( "         -pc                 - Set PC mode for writing xbmp\n" );
    x_printf( "         -ps2                - Set PS2 mode for writing xbmp\n" );
    x_printf( "         -xbox               - Set Xbox mode for writing xbmp\n" );
    x_printf( "         -ps2swizzle         - Swizzle Clut for ps2\n");
    x_printf( "         -info               - Display info about bitmap\n" );
    x_printf( "         -overwrite          - Force overwrite of files\n" );
    x_printf( "         -writexbmp          - Write as xbmp\n" );
    x_printf( "         -writetga           - Write as tga\n" );
    x_printf( "formats:\n" );
    x_printf( "         RGB Color      BGR Color\n" );
    x_printf( "         ---------      ---------\n" );
    x_printf( "         32_RGBA_8888   32_BGRA_8888\n" );
    x_printf( "         32_RGBU_8888   32_BGRU_8888\n" );
    x_printf( "         32_ARGB_8888   32_ABGR_8888\n" );
    x_printf( "         32_URGB_8888   32_UBGR_8888\n" );
    x_printf( "         24_RGB_888     24_BGR_888  \n" );
    x_printf( "         16_RGBA_4444   16_BGRA_4444\n" );
    x_printf( "         16_ARGB_4444   16_ABGR_4444\n" );
    x_printf( "         16_RGBA_5551   16_BGRA_5551\n" );
    x_printf( "         16_RGBU_5551   16_BGRU_5551\n" );
    x_printf( "         16_ARGB_1555   16_ABGR_1555\n" );
    x_printf( "         16_URGB_1555   16_UBGR_1555\n" );
    x_printf( "         16_RGB_565     16_BGR_565  \n" );
    x_printf( "\n" );
    x_printf( "         RGB 8Bit CLUT  BGR 8Bit CLUT\n" );
    x_printf( "         -------------  -------------\n" );
    x_printf( "         P8_RGBA_8888   P8_BGRA_8888\n" );
    x_printf( "         P8_RGBU_8888   P8_BGRU_8888\n" );
    x_printf( "         P8_ARGB_8888   P8_ABGR_8888\n" );
    x_printf( "         P8_URGB_8888   P8_UBGR_8888\n" );
    x_printf( "         P8_RGB_888     P8_BGR_888  \n" );
    x_printf( "         P8_RGBA_4444   P8_BGRA_4444\n" );
    x_printf( "         P8_ARGB_4444   P8_ABGR_4444\n" );
    x_printf( "         P8_RGBA_5551   P8_BGRA_5551\n" );
    x_printf( "         P8_RGBU_5551   P8_BGRU_5551\n" );
    x_printf( "         P8_ARGB_1555   P8_ABGR_1555\n" );
    x_printf( "         P8_URGB_1555   P8_UBGR_1555\n" );
    x_printf( "         P8_RGB_565     P8_BGR_565  \n" );
    x_printf( "\n" );
    x_printf( "         RGB 4Bit CLUT  BGR 4Bit CLUT\n" );
    x_printf( "         -------------  -------------\n" );
    x_printf( "         P4_RGBA_8888   P4_BGRA_8888\n" );
    x_printf( "         P4_RGBU_8888   P4_BGRU_8888\n" );
    x_printf( "         P4_ARGB_8888   P4_ABGR_8888\n" );
    x_printf( "         P4_URGB_8888   P4_UBGR_8888\n" );
    x_printf( "         P4_RGB_888     P4_BGR_888  \n" );
    x_printf( "         P4_RGBA_4444   P4_BGRA_4444\n" );
    x_printf( "         P4_ARGB_4444   P4_ABGR_4444\n" );
    x_printf( "         P4_RGBA_5551   P4_BGRA_5551\n" );
    x_printf( "         P4_RGBU_5551   P4_BGRU_5551\n" );
    x_printf( "         P4_ARGB_1555   P4_ABGR_1555\n" );
    x_printf( "         P4_URGB_1555   P4_UBGR_1555\n" );
    x_printf( "         P4_RGB_565     P4_BGR_565  \n" );
}

//==============================================================================
//  Execute Script
//==============================================================================

void ExecuteScript( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName )
{
    s32         i;
    s32         Target          = BITMAP_TARGET_GENERIC;
    xbool       Overwrite       = FALSE;
    xbool       OutputFolderSet = FALSE;
    xbool       IsRecursive     = FALSE;
    xstring     OutputFolder;

    // Reset script state
    OutputFolder.Clear();
    Target = BITMAP_TARGET_PC;

    // Build commands from command line
    for( i=0 ; i<CommandLine.GetNumOptions() ; i++ )
    {
        // Get option name and string
        xstring OptName   = CommandLine.GetOptionName( i );
        xstring OptString = CommandLine.GetOptionString( i );

        //==============================================================================
        //==============================================================================
        if( OptName == "INFO" )
        {
            x_printf( "\n" );
            x_printf( "File     = \"%s\"\n", BitmapName );
            x_printf( "Format   = %s\n", FormatToString( Bitmap.GetFormat() ) );
            x_printf( "Width    = %d\n", Bitmap.GetWidth() );
            x_printf( "Height   = %d\n", Bitmap.GetHeight() );
            x_printf( "BPP      = %d\n", Bitmap.GetBPP() );
            x_printf( "BPC      = %d\n", Bitmap.GetBPC() );
            x_printf( "HasAlpha = %s\n", Bitmap.HasAlphaBits() ? "TRUE" : "FALSE" );
            x_printf( "HasClut  = %s\n", Bitmap.IsClutBased() ? "TRUE" : "FALSE" );
            x_printf( "NMips    = %d\n", Bitmap.GetNMips() );
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "PC" )
        {
            Target = BITMAP_TARGET_PC;
            auxbmp_ConvertToD3D( Bitmap );
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "PS2" )
        {
            Target = BITMAP_TARGET_PS2;
            auxbmp_ConvertToPS2( Bitmap );
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "XBOX" )
        {
            Target = BITMAP_TARGET_XBOX;
            auxbmp_Compress( Bitmap,"Name",4 );
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "PS2SWIZZLE" )
        {
            Bitmap.PS2SwizzleClut();
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "WRITETGA" )
        {
            xstring PathName = CommandLine.ChangeExtension( BitmapName, "tga" );
            xstring Path;
            xstring File;

            // Change Path is output folder set
            if( OutputFolderSet )
            {
                CommandLine.SplitPath( PathName, Path, File );
                PathName = CommandLine.JoinPath( OutputFolder, File );
            }

            // Check if the file already exists
            if( Overwrite || !CommandLine.FileExists( PathName ) )
            {
                // Save the file
                if( !Bitmap.SaveTGA( PathName ) )
                {
                    x_printf( "Error - Saving TGA \"%s\"\n", PathName );
                }
                else
                {
                    x_printf( "        Saving TGA \"%s\"\n", PathName );
                }
            }
            else
            {
                // Display error
                x_printf( "Error - File \"%s\" already exists\n", PathName );
            }
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "WRITEXBMP" )
        {
            xstring PathName = CommandLine.ChangeExtension( BitmapName, "xbmp" );
            xstring Path;
            xstring File;

            // Change Path is output folder set
            if( OutputFolderSet )
            {
                CommandLine.SplitPath( PathName, Path, File );
                PathName = CommandLine.JoinPath( OutputFolder, File );
            }

            // Check if the file already exists
            if( Overwrite || !CommandLine.FileExists( PathName ) )
            {
                // Save the file
                if( !Bitmap.Save( PathName ) )
                {
                    x_printf( "Error - Saving XBMP \"%s\"\n", PathName );
                }
                else
                {
                    x_printf( "        Saving XBMP \"%s\"\n", PathName );
                }
            }
            else
            {
                // Display error
                x_printf( "Error - File \"%s\" already exists\n", PathName );
            }
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "OUTPUT" )
        {
            // Add a trailing slash to the OptString
            s32 OptStringLen = OptString.GetLength();
            if( (OptStringLen > 0) && ((OptString[OptStringLen-1] != '\\') && (OptString[OptStringLen-1] != '/')) )
                OptString += '\\';

            // Set the output folder
            OutputFolder    = OptString;
            OutputFolderSet = TRUE;
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "OVERWRITE" )
        {
            // Set overwrite flag
            Overwrite = TRUE;
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "CONVERT" )
        {
            s32 Format = StringToFormat( OptString );
            if( Format != -1 )
            {
                // Do the conversion
                Bitmap.ConvertFormat( (xbitmap::format)Format );
            }
            else
            {
                // Illegal format
                x_printf( "Error - Unsupported format \"%s\"\n", OptString );
            }
        }
        //==============================================================================
        //==============================================================================
        if( OptName == "MIPS" )
        {
            s32 nMips = x_atoi( OptString );
            if( (nMips > 0) && (nMips <= 16) )
            {
                // Build the mips
                Bitmap.BuildMips( nMips );
            }
            else
            {
                // Illegal number of mips
                x_printf( "Error - Illegal number of mips requested %d\n", nMips );
            }
        }
    }
}

//==============================================================================
//  main
//==============================================================================

int main( int argc, char** argv )
{
    s32             i;
    command_line    CommandLine;
    xbool           NeedHelp;
    xbool           IsRecursive;
    xarray<xstring> BitmapFiles;

    // Setup recognized command line options
    CommandLine.AddOptionDef( "R"                             );
    CommandLine.AddOptionDef( "INFO"                          );
    CommandLine.AddOptionDef( "PC"                            );
    CommandLine.AddOptionDef( "PS2"                           );
    CommandLine.AddOptionDef( "XBOX"                          );
    CommandLine.AddOptionDef( "PS2SWIZZLE"                    );
    CommandLine.AddOptionDef( "WRITETGA"                      );
    CommandLine.AddOptionDef( "WRITEXBMP"                     );
    CommandLine.AddOptionDef( "OVERWRITE"                     );
    CommandLine.AddOptionDef( "OUTPUT",  command_line::STRING );
    CommandLine.AddOptionDef( "CONVERT", command_line::STRING );
    CommandLine.AddOptionDef( "MIPS",    command_line::STRING );
    CommandLine.AddOptionDef( "D3DSWIZZLE"                    );
    // Parse command line
    NeedHelp = CommandLine.Parse( argc, argv );
    if( NeedHelp || (CommandLine.GetNumArguments() == 0) )
    {
        DisplayHelp();
        return 10;
    }

    // Determine if this is recursive for globbing
    IsRecursive = (CommandLine.FindOption( xstring("R") ) != -1);

    // Build list of files to process
    for( i=0 ; i<CommandLine.GetNumArguments() ; i++ )
    {
        // Glob each pattern to build a list of files to process
        const xstring& Pattern = CommandLine.GetArgument( i );
        CommandLine.Glob( Pattern, BitmapFiles, IsRecursive );
    }

    // Execute the script for each bitmap
    for( i=0 ; i<BitmapFiles.GetCount() ; i++ )
    {
        xbitmap Bitmap;

        // Get name and load bitmap
        const xstring& BitmapName = BitmapFiles[i];
        xbool Loaded = auxbmp_Load( Bitmap, BitmapName );

        // Check if loaded correctly
        if( Loaded )
        {
            // Yes - continue
            ExecuteScript( CommandLine, Bitmap, BitmapName );
        }
        else
        {
            // No - failed to load
            x_printf( "Error - Can't load bitmap \"%s\"\n", BitmapName );
        }
    }

    // Return Success
    return 0;
}
