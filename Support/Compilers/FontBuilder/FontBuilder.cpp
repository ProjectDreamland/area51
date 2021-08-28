//==============================================================================
//
//  FontBuilder - Font conversion tool
//
//==============================================================================

#include "x_files.hpp"
#include "commandline/CommandLine.hpp"
#include "x_bitmap.hpp"
#include "bitmap/aux_bitmap.hpp"
#include "x_bytestream.hpp"
#include "TextIn.hpp"
#include "TextOut.hpp"

#include <windows.h>
#include <tchar.h>

//==============================================================================
//  Defines
//==============================================================================

#define VERSION             "v1.2b"

#define WRITE_XBMP		        ( 1<<1 )
#define WRITE_TGA		        ( 1<<2 )

#define TARGET_PLATFORM_XBOX    ( 1<<3 )
#define TARGET_PLATFORM_PC	    ( 1<<4 )
#define TARGET_PLATFORM_PS2	    ( 1<<5 )
#define TARGET_PLATFORM_GCN	    ( 1<<6 )

#define CONVERT_TO_P8           ( 1<<7 )
#define CONVERT_TO_P4           ( 1<<8 )
#define CONVERT_TO_TC           ( 1<<9 )

//==============================================================================
//  Structures
//==============================================================================

struct Character
{
    u16     X;
    u16     Y;
    u16     W;
};

struct charmap
{
    u16     character;
    u16     bitmap;
    u16     count;
};

//==============================================================================
//  Static Data
//==============================================================================

// Windows GDI stuff 

HDC         s_dc = 0;
TEXTMETRIC  s_tm;
HBITMAP     s_Bitmap;
HBRUSH      s_FillBrush;
RECT        s_FillRect;
HFONT       s_Font;

// parameter defaults
xstring     s_FontName		= "";
s32         s_FirstChar		= 32;
s32         s_NumChars		= 96;
s32         s_Antialias		= 1;
s32         s_PointSize		= 12;
s32         s_CharHeight    = s_PointSize;
s32         s_Weight		= 400;      // 400 is the windows "normal" font. range 0 (don't care) - 1000.
xbool       s_Italic		= FALSE;
xbool       s_Underline		= FALSE;
xbool		s_Debug			= FALSE;
xbool       s_CopyToRGB     = FALSE;
xbool       s_OverWrite     = FALSE;
u16			s_OutputFormat	= 0;
xbool       s_ReadFromFile  = FALSE;
s32         s_BitmapWidth   = 256;
xstring     BitmapInName;
xstring     s_MapName       = "";

X_FILE      *s_MapFile      = NULL;     
X_FILE      *s_OutFile      = NULL;

xarray<Character>   s_CharacterData;
xarray<charmap>     s_CMap;


//==============================================================================
//  NextPower2
//==============================================================================

s32 NextPower2( s32 v )
{
    s32 p=1;

    while( p<v )
        p<<=1;

    return p;
}

//==============================================================================
//  GetGlyph
//==============================================================================

xbitmap GetGlyph( _TCHAR Glyph )
{
    s32     wSrc, hSrc;
    s32     wDst, hDst;
    s32     x;
    s32     y;
    s32     sx;
    s32     sy;
    s32     Intensity;

    // Print character
    SetBkMode   ( s_dc, TRANSPARENT );
    SetBkColor  ( s_dc, RGB(0,0,0) );
    SetTextColor( s_dc, RGB(250,250,250) );
    SetTextAlign( s_dc, TA_LEFT|TA_TOP );
    FillRect    ( s_dc, &s_FillRect, s_FillBrush );
    TextOut     ( s_dc, 0, 0, &Glyph, 1);

    // Get source size and dest size
    s32 CharWidth;
    GetCharWidth32( s_dc, Glyph, Glyph, &CharWidth );
    wSrc = CharWidth;
    hSrc = s_tm.tmHeight;
    wDst = (wSrc+(s_Antialias-1)) / s_Antialias;
    hDst = hSrc / s_Antialias;

    // Create xbitmap for character
    xbitmap bmp;
    /* TODO: Need to remove the crt0 code. the default memory allocator was called here, instead of the
       x_malloc() - so debug memory checking asserts when it fails to find a header. Passing in NULL
       causes the xbitmap to allocate it's own memory using x_malloc(). ctetrick */
    bmp.Setup( xbitmap::FMT_32_RGBA_8888, wDst, hDst, TRUE, /*(byte*)new char[ wDst*hDst*4 ]*/ NULL );

    // Loop through pixels and antialias
    for( y=0 ; y<hDst ; y++ )
    {
        for( x=0 ; x<wDst ; x++ )
        {
            s32 px    = x * s_Antialias;
            s32 py    = y * s_Antialias;
            s32 Count = 0;
            Intensity = 0;
            for( sy=0 ; (sy<s_Antialias) && ((py+sy)<hSrc) ; sy++ )
            {
                for( sx=0 ; (sx<s_Antialias) && ((px+sx)<wSrc) ; sx++ )
                {
                    COLORREF cr = GetPixel( s_dc, px+sx, py+sy );
                    Intensity += (cr&0xff);
                    Count++;
                }
            }
            Intensity /= Count;
            bmp.SetPixelColor( xcolor(Intensity,Intensity,Intensity), x, y );
        }
    }

    return bmp;
}


//==============================================================================
//  Write Character Map file
//==============================================================================

xbool Writemap( command_line& CommandLine, const xstring& BitmapName, xstring& OutputFolder)
{
    xstring PathName = CommandLine.ChangeExtension( BitmapName, "map" );
    xstring Path;
    xstring File;

    CommandLine.SplitPath( PathName, Path, File );
    PathName = CommandLine.JoinPath( OutputFolder, File );

    // Check if the file already exists
    if( !CommandLine.FileExists( PathName ) || s_OverWrite )
    {
        x_printf("Writing character map file (%s)\n", PathName);

        // save the file
        s_MapFile = x_fopen( PathName, "wt");
        ASSERT( s_MapFile );

        if( !x_fprintf( s_MapFile, "%d\n", s_NumChars ) )
        {
            x_printf( "Error - Saving MAP \"%s\"\n", PathName );
            x_fclose( s_MapFile );
            return FALSE;
        }

        // dump the map.
        for( int c = 0; c < s_CMap.GetCount(); c++ )
        {
            if( s_CMap[c].count > 0 )
            {
                if( !x_fprintf( s_MapFile, "0x%04X\t%d\n", s_CMap[c].character, s_CMap[c].count ) )
                {
                    x_printf( "Error - Saving MAP \"%s\"\n", PathName );
                    x_fclose( s_MapFile );
                    return FALSE;
                }
            }
        }
    }
    else
    {
        // Display error
        if( s_Debug )
        {
            x_printf( "Error - File \"%s\" already exists\n", PathName );
            return FALSE;
        }
    }

    x_fclose( s_MapFile );
    
    return TRUE;
}

//==============================================================================
//  Read Character Map file
//==============================================================================

xbool Readmap( void )
{
    ASSERT( s_MapFile );

    token_stream mapFile;
    mapFile.OpenFile( s_MapFile );

    s_NumChars = mapFile.ReadInt();
    ASSERT( s_NumChars > 0 );

    mapFile.SkipToNextLine();

    // read the map.
    for( int c = 0; c < s_NumChars; c++ )
    {
        charmap cm;
        cm.bitmap = c + 1;
        cm.character = mapFile.ReadHex();
        cm.count = mapFile.ReadInt();
        s32 chr = cm.character;
        
        if( chr < 16 )
        {
            x_printf("WARNING: character code %X in map reserved for control codes\n", chr);
        }

        if( chr < 256 )
        {
            s_CMap[chr] = cm;
        }
        else
        {
            s_CMap.Append() = cm;
        }
        mapFile.SkipToNextLine();
        if( mapFile.IsEOF() && (c < (s_NumChars - 1)))   
        {
            if( s_Debug )
            {
                x_printf("ERROR: unexpected end of character map file.");
                return FALSE;
            }
            break;
        }
    }
    mapFile.CloseFile();
    return TRUE;
}


//==============================================================================
//  Write Font Data
//==============================================================================

xbool Writefont( command_line& CommandLine, const xstring& BitmapName, xstring& OutputFolder)
{
    xstring PathName = CommandLine.ChangeExtension( BitmapName, "font" );
    xstring Path;
    xstring File;

    CommandLine.SplitPath( PathName, Path, File );
    PathName = CommandLine.JoinPath( OutputFolder, File );

    // Check if the file already exists
    //if( !CommandLine.FileExists( PathName ) || s_OverWrite )
    {
        // Save the file
        xbytestream b;
        u16 num;

        if( s_OutputFormat & TARGET_PLATFORM_GCN )    //or other platforms...
        {
            num = s_CMap.GetCount();
            b << ENDIAN_SWAP_16(num);
            num = s_NumChars;
            b << ENDIAN_SWAP_16(num);
            num = s_CharHeight;
            b << ENDIAN_SWAP_16(num);

            // character mapping first.
            for( int c = 0; c < s_CMap.GetCount(); c++ )
            {
                b << ENDIAN_SWAP_16(s_CMap[c].character);
                b << ENDIAN_SWAP_16(s_CMap[c].bitmap);
            }

            // registration data.
            for( int c = 0; c < s_CharacterData.GetCount(); c++ )
            {
                b << ENDIAN_SWAP_16(s_CharacterData[c].X);
                b << ENDIAN_SWAP_16(s_CharacterData[c].Y);
                b << ENDIAN_SWAP_16(s_CharacterData[c].W);
            }
        }
        else
        {
            num = s_CMap.GetCount();
            b << num;
            num = s_NumChars;
            b << num;
            num = s_CharHeight;
            b << num;

            // character mapping first.
            for( int c = 0; c < s_CMap.GetCount(); c++ )
            {
                b << s_CMap[c].character;
                b << s_CMap[c].bitmap;
            }

            // registration data.
            for( int c = 0; c < s_CharacterData.GetCount(); c++ )
            {
                b << s_CharacterData[c].X;
                b << s_CharacterData[c].Y;
                b << s_CharacterData[c].W;
            }
        }

        x_printf("Writing font file (%s)\n", PathName);

        if( !b.SaveFile(PathName) )
        {
            x_printf( "Error - Saving FONT \"%s\"\n", PathName );
            return FALSE;
        }
    }
#if 0
    else
    {
        // Display error
        if( s_Debug )
            x_printf( "Error - File \"%s\" already exists\n", PathName );
    }
#endif

    return TRUE;
}

void WritefontLog( command_line& CommandLine, const xstring& BitmapName, xstring& OutputFolder)
{
    // ascii debug version of font data
    xstring PathName = CommandLine.ChangeExtension( BitmapName, "txt" );
    xstring Path;
    xstring File;

    x_printf("Writing font log (%s)\n", PathName);

    CommandLine.SplitPath( PathName, Path, File );
    PathName = CommandLine.JoinPath( OutputFolder, File );

    X_FILE* pFontLog = x_fopen(PathName, "wt");

    // Save the file
    u16 num;
    char buffer[256];

    num = s_NumChars;

    x_sprintf(buffer, "num chars: %d\n", num);
    x_fwrite(buffer, 1, x_strlen(buffer), pFontLog);

    num = s_CharHeight;

    x_sprintf(buffer, "line height: %d\n", num);
    x_fwrite(buffer, 1, x_strlen(buffer), pFontLog);

    // character mapping first.
    x_sprintf(buffer, "character list:\nCode\tindex\tcount\tx\ty\twidth\n");
    x_fwrite(buffer, 1, x_strlen(buffer), pFontLog);
    for( int c = 0; c < s_CMap.GetCount(); c++ )
    {
        s32 b = s_CMap[c].bitmap;
        x_sprintf(buffer, "0x%04X\t%d\t%d\t%d\t%d\t%d\n", 
            s_CMap[c].character, 
            s_CMap[c].bitmap,
            s_CMap[c].count,
            s_CharacterData[b].X,
            s_CharacterData[b].Y,
            s_CharacterData[b].W );
        x_fwrite(buffer, 1, x_strlen(buffer), pFontLog);
    }

    // registration data.
    x_fclose(pFontLog);
}

//==============================================================================
//  Write xbitmap
//==============================================================================

xbool Writexbmp( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OutputFolder)
{
    xstring PathName = CommandLine.ChangeExtension( BitmapName, "xbmp" );
    xstring Path;
    xstring File;

    CommandLine.SplitPath( PathName, Path, File );
    PathName = CommandLine.JoinPath( OutputFolder, File );

	// Is the bitmap a power of 2.
	if( s_Debug )
	{
		if( (Bitmap.GetHeight() % 2) || (Bitmap.GetWidth() % 2) )
		{
			x_printf( "Warning: Bitmap[%s] is not a power of 2", BitmapName );
		}
	}

    // Check if the file already exists
    //if( !CommandLine.FileExists( PathName ) || s_OverWrite )
    {
        x_printf("Saving xbmp file (%s)\n", PathName);

        // Save the file
        if( !Bitmap.Save( PathName ) && s_Debug)
        {
            x_printf( "Error - Saving XBMP \"%s\"\n", PathName );
            return FALSE;
        }
    }
#if 0
    else
    {
        // Display error
        if( s_Debug )
			x_printf( "Error - File \"%s\" already exists\n", PathName );
    }
#endif

    return TRUE;
}

//==============================================================================
//  Write tga
//==============================================================================

xbool Writetga( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OutputFolder)
{
    xstring PathName = CommandLine.ChangeExtension( BitmapName, "tga" );
    xstring Path;
    xstring File;

    CommandLine.SplitPath( PathName, Path, File );
    PathName = CommandLine.JoinPath( OutputFolder, File );

	// Is the bitmap a power of 2.
	if( s_Debug )
	{
		if( (Bitmap.GetHeight() % 2) || (Bitmap.GetWidth() % 2) )
		{
			x_printf( "Warning: Bitmap[%s] is not a power of 2", BitmapName );
		}
	}

    // Check if the file already exists
    if( !CommandLine.FileExists( PathName ) || s_OverWrite )
    {
        x_printf("Saving TGA file (%s)\n", PathName);

        // Save the file
        if( !Bitmap.SaveTGA( PathName ) && s_Debug)
        {
            x_printf( "Error - Saving TGA \"%s\"\n", PathName );
            return FALSE;
        }
    }
    else
    {
        // Display error
        if( s_Debug )
        {
            x_printf( "Error - File \"%s\" already exists\n", PathName );
            return FALSE;
        }
    }

    return TRUE;
}

//==============================================================================
//  Compile PC
//==============================================================================

xbool CompileTargetPC( command_line& CommandLine, xbitmap& Bitmap, xstring& OptString)
{
    xstring     OutputFolder;
    xstring     BitmapName;
    char        Drive[X_MAX_DRIVE];
    char        Dir[X_MAX_DIR];
    char        Name[X_MAX_FNAME];
    char        Ext[X_MAX_EXT];

    x_splitpath(OptString, Drive, Dir, Name, Ext);

    BitmapName = xfs("%s%s", Name, Ext);

    // Reset script state
    OutputFolder.Clear();

    // Set the output folder
    OutputFolder = xfs("%s%s", Drive, Dir);

    // Add a trailing slash to the path
    s32 PathStringLen = OutputFolder.GetLength();
    if( (PathStringLen > 0) && ((OutputFolder[PathStringLen-1] != '\\') && (OutputFolder[PathStringLen-1] != '/')) )
        OutputFolder += '\\';

	// Check what format was specified for the output.
    if( s_OutputFormat & CONVERT_TO_P8 )
    {
        Bitmap.ConvertFormat( xbitmap::FMT_P8_ARGB_8888 );
    }
    else if( s_OutputFormat & CONVERT_TO_P4 )
    {
        Bitmap.ConvertFormat( xbitmap::FMT_P4_ARGB_8888 );
    }
    else if( s_OutputFormat & CONVERT_TO_TC )
    {
        Bitmap.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
    }

    if( s_Debug )
    {
        WritefontLog( CommandLine, BitmapName, OutputFolder);
    }

    // write the map file any time we don't have one.
    if( !s_ReadFromFile )
        if( !Writemap( CommandLine, BitmapName, OutputFolder) )
            return FALSE;

    if( s_OutputFormat & WRITE_XBMP )
	{
        auxbmp_ConvertToD3D( Bitmap );
		if( !Writexbmp( CommandLine, Bitmap, BitmapName, OutputFolder) )
            return FALSE;
        if( !Writefont( CommandLine, BitmapName, OutputFolder) )
            return FALSE;
	}
	if( s_OutputFormat & WRITE_TGA )
	{
		if( !Writetga( CommandLine, Bitmap, BitmapName, OutputFolder) )
            return FALSE;
	}
	if( !(s_OutputFormat & WRITE_XBMP) && !(s_OutputFormat & WRITE_TGA) )
	{
		if( s_Debug )
        {
            x_printf( "Error: No Output format specified." );
            return FALSE;
        }
	}

    return TRUE;
}

//==============================================================================
//  Compile PS2
//==============================================================================

xbool CompileTargetPS2( command_line& CommandLine, xbitmap& Bitmap, xstring& OptString)
{
    xstring     OutputFolder;
    xstring     BitmapName;
    char        Drive[X_MAX_DRIVE];
    char        Dir[X_MAX_DIR];
    char        Name[X_MAX_FNAME];
    char        Ext[X_MAX_EXT];

    x_splitpath(OptString, Drive, Dir, Name, Ext);

    BitmapName = xfs("%s%s", Name, Ext);

    // Reset script state
    OutputFolder.Clear();

    // Set the output folder
    OutputFolder = xfs("%s%s", Drive, Dir);

    // Add a trailing slash to the path
    s32 PathStringLen = OutputFolder.GetLength();
    if( (PathStringLen > 0) && ((OutputFolder[PathStringLen-1] != '\\') && (OutputFolder[PathStringLen-1] != '/')) )
        OutputFolder += '\\';

	// Check what format was specified for the output.
    if( s_OutputFormat & CONVERT_TO_P8 )
    {
        Bitmap.ConvertFormat( xbitmap::FMT_P8_ARGB_8888 );
    }
    else if( s_OutputFormat & CONVERT_TO_P4 )
    {
        Bitmap.ConvertFormat( xbitmap::FMT_P4_ARGB_8888 );
    }
    else if( s_OutputFormat & CONVERT_TO_TC )
    {
        Bitmap.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
    }

    if( s_Debug )
    {
        WritefontLog( CommandLine, BitmapName, OutputFolder);
    }

    // write the map file any time we don't have one.
    if( !s_ReadFromFile )
        if( !Writemap( CommandLine, BitmapName, OutputFolder) )
            return FALSE;

	if( s_OutputFormat & WRITE_XBMP )
	{
        auxbmp_ConvertToPS2( Bitmap );
		if( !Writexbmp( CommandLine, Bitmap, BitmapName, OutputFolder) )
            return FALSE;
        if( !Writefont( CommandLine, BitmapName, OutputFolder) )
            return FALSE;
	}
	if( s_OutputFormat & WRITE_TGA )
	{
		if( !Writetga( CommandLine, Bitmap, BitmapName, OutputFolder ) )
            return FALSE;
	}
    if( !(s_OutputFormat & WRITE_XBMP) && !(s_OutputFormat & WRITE_TGA) )
	{
		if( s_Debug )
        {
            x_printf( "Error: No Output format specified." );
            return FALSE;
        }
	}

    return TRUE;
}

//==============================================================================
//  Display Help
//==============================================================================

void DisplayHelp( void )
{
    x_printf( "\n" );
    x_printf( "Fontbuilder (c)2001-2004 Inevitable Entertainment Inc.\n" );
    x_printf( xfs("Version %s\n",VERSION) );
    x_printf( "\n"
              "  usage:\n"
              "         Fontbuilder [-opt [param]]\n"
              "\n"
              "options: (Conversion)\n"
              "         -infile           - source bitmap (.tga)\n"
              "         -mapfile          - character mapping file (.map)\n"
              "         -pc               - Output path and filename for PC platform\n"
              "         -ps2              - Output path and filename for PS2 platform\n"
              "         -xbox             - Output path and filename for XBox platform\n"
              "              (Note that any given extension be changed for each output file.)\n"
              "         -writexbmp        - flag - write xbmp file\n"
              "         -p8               - flag - xbmp 8-bit format\n"
              "         -p4               - flag - xbmp 4-bit format\n"
              "         -tc               - flag - xbmp truecolor format\n"
              "\n"
              "Options: (generation)\n"
              "         -font             - windows font name\n"
              "         -size             - point size of font\n"
              "         -weight           - font weight: 400 = normal, 700 = bold\n"
              "         -italic           - render italic font\n"
              "         -underline        - render underlined font\n"
              "         -antialias        - oversample factor\n"
              "         -firstchar        - character code of first character\n"
              "         -numchars         - number of characters to generate\n"
              "         -mapwidth         - width in pixels of bitmap\n"
              "         -writetga         - flag - write tga file\n"
              "         -overwrite        - flag - overwrite tga/map files\n"
              "\n"
              "Options: (debug)\n"
              "         -rgba             - flag - outputs tga with font image in RGB)\n"
              "         -log              - flag - writes character data for debug.)\n"
              "\n"
              "Map File Structure\n"
              "         num chars               // number of characters\n"
              "         CharCode <tab> count    // character code (HEX) in order of appearance\n"
              "             // Each character line has a character code, then a tab, followed by\n"
              "             // a count number. This count is currently always 1.\n"
              "\n" 
             );
}

//==============================================================================
// MAIN
//==============================================================================


int main( int argc, char** argv )
{
    command_line    cl;
    xstring OptStringXBOX;
	xstring OptStringPC;
	xstring OptStringPS2;
	xstring OptStringGCN;

    // Setup the options to look for
    cl.AddOptionDef( "FONT",      command_line::STRING );   // specifies Windows font to extract from.
    cl.AddOptionDef( "SIZE",      command_line::STRING );   // font pointsize (is also the resulting height in pixels.)
    cl.AddOptionDef( "WEIGHT",    command_line::STRING );   // windows font weight - 400 = normal, 700 bold.
    cl.AddOptionDef( "ITALIC"                          );   // set italic
    cl.AddOptionDef( "UNDERLINE"                       );   // set underline
    cl.AddOptionDef( "ANTIALIAS", command_line::STRING );   // antialias level

    cl.AddOptionDef( "MAPFILE",   command_line::STRING );   // input file: character map definition.
    cl.AddOptionDef( "FIRSTCHAR", command_line::STRING );   // character code of first character if no map file supplied.
    cl.AddOptionDef( "NUMCHARS",  command_line::STRING );   // number of characters if no map file.

    cl.AddOptionDef( "INFILE",    command_line::STRING );   // TGA file for pass 2 conversion.
    cl.AddOptionDef( "MAPWIDTH",  command_line::STRING );   // width of output bitmap 
    cl.AddOptionDef( "OVERWRITE"                       );   // skips file exists checking

    cl.AddOptionDef( "PC",        command_line::STRING );   // specifies PC target location - flags target output
    cl.AddOptionDef( "PS2",       command_line::STRING );   // specifies PS2 target location - flags target output
    cl.AddOptionDef( "NGC",	      command_line::STRING );   // specifies NGC target location - flags target output
    cl.AddOptionDef( "XBOX",      command_line::STRING );   // specifies XBOX target location - flags target output

    cl.AddOptionDef( "WRITETGA"					       );   // output targa format (unconverted)
    cl.AddOptionDef( "WRITEXBMP"                       );   // output xbmp format (converted for platform

    cl.AddOptionDef( "RGBA"                            );   // copy rgb as well as alpha.

    cl.AddOptionDef( "P8"                              );   // sets conversion format to paletted 8 bit
    cl.AddOptionDef( "P4"                              );   // sets conversion format to paletted 4 bit
    cl.AddOptionDef( "TC"                              );   // sets conversion format to 'true color' (32 bit)

    cl.AddOptionDef( "LOG"		                       );   // warns if wrong bitmap size, or file overwrite.

    //cl.AddOptionDef( "DUMPCMDLINE"                     );   // TEMP - dump command line.

    xbool   bNeedHelp = FALSE;

    // Parse the command line
    bNeedHelp = cl.Parse( argc, argv );

    if( bNeedHelp || ((cl.GetNumOptions() == 0) && (cl.GetNumArguments() == 0)) )
    {
        // Parse error, display help
        DisplayHelp();
    }
    else
    {
        s32     i;

        // Process Options
        for( i=0 ; i<cl.GetNumOptions() ; i++ )
        {
            xstring Option = cl.GetOptionName( i );

#if 0 // debug
            if( Option == "DUMPCMDLINE" )
            {
                x_printf("Dumping commmand line:\n");
                for (int i = 0; i < argc; i++ )
                {
                    x_printf("arg %d = '%s'\n", i, argv[i]);
                }
                return 0;
            }
#endif
            if( Option == "FIRSTCHAR" )
            {
                s_FirstChar = atoi( cl.GetOptionString( i ) );
                if( s_FirstChar < 0  ) s_FirstChar = 0;
                if( s_FirstChar < 0x10 )
                {
                    x_printf("ERROR: characters < 0x10 are reserved for control characters\n");
                    exit(-1);
                }
                continue;
            }
            if( Option == "NUMCHARS" )
            {
                s_NumChars = atoi( cl.GetOptionString( i ) );
                if( s_NumChars < 1  ) s_NumChars = 1;
                continue;
            }
            if( Option == "MAPFILE" )   
            {
                // we want to pull a list of characters specified by a file
                s_ReadFromFile = TRUE;
                s_MapName = cl.GetOptionString( i );
                s_MapFile = x_fopen( s_MapName, "rt");
                if( !s_MapFile )
                    s_ReadFromFile = FALSE;
                continue;
            }
            if( Option == "INFILE" )
            {
                BitmapInName = cl.GetOptionString( i );
                continue;
            }
            if( Option == "ANTIALIAS" )
            {
                s_Antialias = atoi( cl.GetOptionString( i ) );
                if( s_Antialias < 1  ) s_Antialias = 1;
                continue;
            }
            if( Option == "FONT" )
            {
                s_FontName  = cl.GetOptionString( i );
                continue;
            }
            if( Option == "SIZE" )
            {
                s_PointSize = atoi( cl.GetOptionString( i ) );
                continue;
            }
            if( Option == "WEIGHT" )
            {
                s_Weight = atoi( cl.GetOptionString( i ) );
                continue;
            }
            if( Option == "ITALIC" )
            {
                s_Italic = TRUE;
                continue;
            }
            if( Option == "UNDERLINE" )
            {
                s_Underline = TRUE;
                continue;
            }
            if( Option == "MAPWIDTH" )
            {
                s_BitmapWidth = atoi( cl.GetOptionString( i ) );
                continue;
            }
            if( Option == "OVERWRITE" )
            {
                s_OverWrite = TRUE;
                continue;
            }
            if( Option == "PC" )
			{
                OptStringPC = cl.GetOptionString( i );
				s_OutputFormat |= TARGET_PLATFORM_PC;
                continue;
			}
        
			if( Option == "PS2" )
			{
                OptStringPS2 = cl.GetOptionString( i );
				s_OutputFormat |= TARGET_PLATFORM_PS2;
                continue;
			}
            
			if( Option == "NGC" )
			{
                OptStringGCN = cl.GetOptionString( i );
				s_OutputFormat |= TARGET_PLATFORM_GCN;
                continue;
      		}
            
			if( Option == "XBOX" )
            {
                OptStringXBOX = cl.GetOptionString( i );
                s_OutputFormat |= TARGET_PLATFORM_XBOX;
                continue;
            }

            if( Option == "WRITETGA" )
            {
                s_OutputFormat |= WRITE_TGA;
                continue;
            }            
			if( Option == "WRITEXBMP" )
            {
                s_OutputFormat |= WRITE_XBMP;
                continue;
            }            
            if( Option == "RGBA" )
            {
                s_CopyToRGB = TRUE;
                continue;
            }
            if( Option == "P8" )
            {
                s_OutputFormat |= CONVERT_TO_P8;
                continue;
            }
            if( Option == "P4" )
            {
                s_OutputFormat |= CONVERT_TO_P4;
                continue;
            }
            if( Option == "TC" )
            {
                s_OutputFormat |= CONVERT_TO_TC;
                continue;
            }
			if( Option == "LOG" )
            {
                s_Debug = TRUE;
                continue;
            }
        }   // end For(... GetNumOptions...)

        if( !(s_OutputFormat & TARGET_PLATFORM_PS2) &&
            !(s_OutputFormat & TARGET_PLATFORM_PC)  &&
            !(s_OutputFormat & TARGET_PLATFORM_XBOX) &&
            !(s_OutputFormat & TARGET_PLATFORM_GCN) )
        {
            s_OutputFormat = TARGET_PLATFORM_PC;
        }

        //==============================================================================
        //  Start processing here... 
        //==============================================================================

        x_printf(xfs("\nFontbuilder %s started.\n",VERSION));

        xbitmap fbm;

        // init character mapping data for single byte direct mapping
        // the first 16 characters are reserved for control characters.
        s_CMap.Clear();
        charmap cm;
        cm.character = 0;
        cm.bitmap = 0;
        cm.count = 0;
        for( s32 c = 0; c < 256; c++ )
        {
            s_CMap.Append() = cm;
        }

        // reserve first entry for non-printable characters
        Character ch; 
        ch.X = 0;
        ch.Y = 0;
        ch.W = 0;
        s_CharacterData.Clear();
        s_CharacterData.Append() = ch;

        // if we're writing a tga, check options and set up to create font.
        if( s_OutputFormat & WRITE_TGA )
        {
            x_printf("Pass 1 - Create Font.\n");

            if( s_FontName == "" )
            {
                x_printf("Error: no windows font name given for creating font.\n");
                return 1;
            }
            // Create the font
            _TCHAR* pFontUnicodeName = new _TCHAR[s_FontName.GetLength()+1];
            {
                for( i=0 ; i<s_FontName.GetLength() ; i++ )
                    pFontUnicodeName[i] = s_FontName[i];
                pFontUnicodeName[i] = 0;
            }
            s_Font = CreateFont( s_PointSize * s_Antialias,
                0,
                0,
                0,
                s_Weight,
                s_Italic,
                s_Underline,
                FALSE,
                DEFAULT_CHARSET,//RUSSIAN_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                NONANTIALIASED_QUALITY,
                VARIABLE_PITCH,
                pFontUnicodeName);
            ASSERT( s_Font );

            // Create the DC and select font into it
            s_dc = CreateCompatibleDC( NULL );
            ASSERT( s_dc );
            HFONT OldFont = (HFONT)SelectObject( s_dc, s_Font );

            // Determine text metrics
            GetTextMetrics( s_dc, &s_tm );

            // Create rectangle, bitmap and brush fill background
            s_FillRect.left   = 0;
            s_FillRect.top    = 0;
            s_FillRect.right  = s_tm.tmMaxCharWidth;
            s_FillRect.bottom = s_tm.tmHeight;

            s_Bitmap = CreateBitmap( s_FillRect.right, s_FillRect.bottom, 1, 32, NULL );
            ASSERT( s_Bitmap );
            HBITMAP OldBitmap = (HBITMAP)SelectObject( s_dc, s_Bitmap );
            s_FillBrush = CreateSolidBrush( RGB(0,0,0) );
            ASSERT( s_FillBrush );

            xarray<xbitmap> Bitmaps;

            // create from character list
            if (s_ReadFromFile)
            {
                // create bitmaps for each character in the file list
                x_printf("Create BitMaps for Each Character\n");

                if( !Readmap() )
                {
                    x_printf("unable to read mapfile, aborting.\n");
                    return 2;
                }

                // create glyphs for all chars in the list
                for( s32 c=0 ; c < s_CMap.GetCount() ; c++ )
                {
                    _TCHAR Glyph;

                    Glyph = s_CMap[c].character;

                    // check for bogus characters
                    if( Glyph < 0x10 )
                    {
                        if( Glyph > 0 )
                            x_printf("WARNING: character %X is reserved for control characters\n", Glyph );
                        continue;
                    }

                    // Convert to an xbitmap
                    xbitmap b = GetGlyph( Glyph );
                    for( s32 y=0 ; y<b.GetHeight() ; y++ )
                    {
                        for( s32 x=0 ; x<b.GetWidth() ; x++ )
                        {
                            xcolor c = b.GetPixelColor( x, y );
                            if( s_OutputFormat & TARGET_PLATFORM_GCN )
                            {
                                c.A = (c.R == 0) ? 0 : 255;
                            }
                            else
                            {
                                c.A = c.R;
                                if( s_CopyToRGB == FALSE )
                                    c.R = c.G = c.B = 0xfe;
                                if( (x==0) && (y==0) )
                                    c.R = c.G = 0x00;
                            }
                            b.SetPixelColor( c, x, y );
                        }
                    }
                    Bitmaps.Append() = b;
                }

                x_fclose(s_MapFile);
            }
            else // create default character set.
            {
                // Create bitmaps for each character
                x_printf("Create BitMaps for Each Character (default set)\n");

                for( s32 c = s_FirstChar, i = 1 ; c < (s_NumChars + s_FirstChar) ; c++, i++ )
                {
                    _TCHAR Glyph = c;

                    // Convert to an xbitmap
                    xbitmap b = GetGlyph( Glyph );
                    for( s32 y=0 ; y < b.GetHeight() ; y++ )
                    {
                        for( s32 x=0 ; x < b.GetWidth() ; x++ )
                        {
                            xcolor c = b.GetPixelColor( x, y );
                            c.A = c.R;
                            if( s_CopyToRGB == FALSE )
                                c.R = c.G = c.B = 0xfe;
                            if( (x==0) && (y==0) )
                                c.R = c.G = 0x00;
                            b.SetPixelColor( c, x, y );
                        }
                    }
                    Bitmaps.Append() = b;

                    charmap cm;

                    if( c < 256 )
                    {
                        // single byte characters 'direct' mapped
                        cm.character = c;
                        cm.bitmap = i;
                        cm.count = 1;   // default. 
                        s_CMap[c] = cm;
                    }
                    else
                    {
                        // double byte characters appended
                        cm.character = c;
                        cm.bitmap = i;
                        cm.count = 1;   // default. 
                        s_CMap.Append() = cm;
                    }
                }
            }

            x_printf("Calculate size of final bitmap & create\n");

            // Calculate size of final bitmap & create
            s32 fw = s_BitmapWidth;   // defaults to 256 wide.
            s32 lw = 0;
            s32 nLines = 1;

            for( s32 c=0 ; c < s_NumChars; c++ ) 
            {
                lw += Bitmaps[c].GetWidth();
                if( lw > fw ) 
                {
                    nLines++;
                    lw = Bitmaps[c].GetWidth();
                }
            }

            x_printf("Create Final Bitmap Size ");

            s_CharHeight = s_tm.tmHeight / s_Antialias;

            s32     fbw = NextPower2(fw);
            s32     fbh = NextPower2(s_CharHeight * nLines);  

            x_printf( "%ix%i\n",fbw, fbh );

            // make the final bitmap
            fbm.Setup( xbitmap::FMT_32_RGBA_8888, fbw, fbh, TRUE, /*(byte*)new char[fbw*fbh*6*4]*/ NULL );

            // set up a blank bitmap
            {
                xcolor c(0,0,0,0);
                for( s32 y=0 ; y < fbh ; y++ )
                {
                    for( s32 x=0 ; x<fbw ; x++ )
                    {
                        fbm.SetPixelColor( c, x, y );
                    }
                }
            }

            // put some characters on it...
            s32 x = 0;
            s32 y = 0;

            for( c=0 ; c < s_NumChars ; c++ ) 
            {
                // if we don't fit, go to the next line.
                if( (x + Bitmaps[c].GetWidth()) > fbw )
                {
                    x = 0;
                    y += s_CharHeight;
                }

                // record the data
                Character chr;
                chr.X = x;
                chr.Y = y;
                chr.W = Bitmaps[c].GetWidth();
                s_CharacterData.Append() = chr;

                fbm.Blit( x, y, 0, 0, Bitmaps[c].GetWidth(), Bitmaps[c].GetHeight(), Bitmaps[c] );
                fbm.SetPixelColor( xcolor(0,0,255,0), x, y );
                x += Bitmaps[c].GetWidth();
            }

            // Destroy objects
            SelectObject( s_dc, OldFont );
            SelectObject( s_dc, OldBitmap );
            DeleteObject( s_FillBrush );
            DeleteObject( s_Font );
            DeleteObject( s_Bitmap );
            DeleteDC( s_dc );
        }   // s_OutputFormat & WRITE_TGA

        // if we're running a pass2 only
        if( (s_OutputFormat & WRITE_XBMP) && !(s_OutputFormat & WRITE_TGA))
        {
            x_printf("Pass 2 - compile font data.\n");

            // load the TGA file and scan. 
            if( BitmapInName == "" )
            {
                x_printf("ERROR: FontBuilder pass 2 requires TGA file (-infile)\n");
                return 3;
            }

            x_printf("loading TGA file...\n");

            if( !auxbmp_Load( fbm, BitmapInName ) )
            {
                x_printf("ERROR: failed loading TGA file %s\n", BitmapInName);
                return 4;
            }

            x_printf("Scan TGA for registration marks...\n");
            
            // Scan through font building character map
            s32 y = 0;
            s_CharHeight = 0;

            while ( y < fbm.GetHeight() )
            {
                // Initialize for character row
                s32 x1 = 0;
                s32 x2 = 0;

                if( (s_CharHeight == 0) && (y != 0) )
                {
                    // record the first time y Changes here. that's the char height.
                    s_CharHeight = y;
                }

                while ( x2 < fbm.GetWidth() )     // scan row
                {
                    // Scan registration marks for character
                    x2 = x1 + 1;

                    while( (x2 < fbm.GetWidth()) && !(fbm.GetPixelColor( x2, y ).R < 32) ) 
                    {
                        x2++;
                    }

                    // Skip out if nothing on the row
                    if( (x2 - x1) > 1 )
                    {
                        // Add character
                        Character chr;
                        chr.X = x1;
                        chr.Y = y; // + 1;
                        chr.W = x2 - x1;
                        s_CharacterData.Append() = chr;
                    }
                    else
                    {
                        break;
                    }

                    // Set start of next character
                    x1 = x2;

                }   // while row not done

                // Scan down to next row
                y++;

                while( (y < fbm.GetHeight()) && !(fbm.GetPixelColor( 0, y ).R < 32) ) 
                {
                    y++;
                }

            }   // while y < bm height

            // read the character map if we have one.
            if (s_ReadFromFile)
            {
                x_printf("Reading character map file (%s)\n", s_MapName);
                if( !Readmap() )
                {
                    x_printf("unable to read mapfile, aborting.\n");
                    return 5;
                }
            }
            else
            {
                x_printf("generating default character map file\n");

                // generate a default char map if we failed to supply one.
                s_NumChars = s_CharacterData.GetCount() - 1;    // one extra character is for non-printables.
                if( s_NumChars == 0 )
                {
                    x_printf("ERROR: Supplied TGA file has no character markers.");
                    return 6;
                }

                for( s32 c = s_FirstChar, i = 1 ; c < s_NumChars ; c++, i++ )
                {
                    charmap cm;
                    if( c < 0xFF )
                    {
                        // single byte characters 'direct' mapped
                        cm.character = c;
                        cm.bitmap = i;
                        cm.count = 1;   // default. 
                        s_CMap[c] = cm;
                    }
                    else
                    {
                        // double byte characters appended
                        cm.character = c;
                        cm.bitmap = i;
                        cm.count = 1;   // default. 
                        s_CMap.Append() = cm;
                    }
                }
            }
        }   // (s_OutputFormat & WRITE_XBMP) && !(s_OutputFormat & WRITE_TGA) (i.e., pass2 only)

        if( s_OutputFormat & TARGET_PLATFORM_XBOX )
        {	
            CompileTargetPC( cl, fbm, OptStringXBOX );
        }

        if( s_OutputFormat & TARGET_PLATFORM_PC )
        {	
            CompileTargetPC( cl, fbm, OptStringPC );
        }

        if( s_OutputFormat & TARGET_PLATFORM_PS2 )
        {
            CompileTargetPS2( cl, fbm, OptStringPS2 );
        }

    }   // if parse succeeds

    return 0;
}
