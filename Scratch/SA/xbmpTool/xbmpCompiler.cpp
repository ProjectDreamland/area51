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
#include "Auxiliary/bitmap/aux_bitmap.hpp"
#include "x_bitmap.hpp"

//==============================================================================
//  Defines
//==============================================================================

enum target
{
    BITMAP_TARGET_GENERIC,
    BITMAP_TARGET_PC,
    BITMAP_TARGET_PS2,
};

#define WRITE_XBMP		( 1<<1 )
#define WRITE_TGA		( 1<<2 )
#define WRITE_TEXTURE	( 1<<3 )

//==============================================================================
//  Globals
//==============================================================================
xbool	g_Progress				= FALSE;
u8		g_OutputFormat			= 0;
xbool	g_Debug					= FALSE;
xbool	g_Composite				= FALSE;
s32		g_CompositeCount		= 0;
s32		g_BitmapCompositedCount = 0;
xstring	AnimBitmap				= "_ANIM0000";
s32		g_MaxAnimBitmapWidth	= 0;
s32		g_MaxAnimBitmapHeight	= 0;
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
//  Write xbitmap
//==============================================================================

void Writexbmp( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OutputFolder, xbool Overwrite )
{
    xstring PathName = CommandLine.ChangeExtension( BitmapName, "xbmp" );
    xstring Path;
    xstring File;

    CommandLine.SplitPath( PathName, Path, File );
    PathName = CommandLine.JoinPath( OutputFolder, File );

	// Is the bitmap a power of 2.
	if( g_Debug )
	{
		if( (Bitmap.GetHeight() % 2) || (Bitmap.GetWidth() % 2) )
		{
			x_printf( "Warning: Bitmap[%s] is not a power of 2", BitmapName );
		}
	}

    // Check if the file already exists
    if( Overwrite || !CommandLine.FileExists( PathName ) )
    {
        // Save the file
        if( !Bitmap.Save( PathName ) && g_Debug)
        {
            x_printf( "Error - Saving XBMP \"%s\"\n", PathName );
        }
    }
    else
    {
        // Display error
        if( g_Debug )
			x_printf( "Error - File \"%s\" already exists\n", PathName );
    }
}

//==============================================================================
//  Write tga
//==============================================================================

void Writetga( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OutputFolder, xbool Overwrite  )
{
    xstring PathName = CommandLine.ChangeExtension( BitmapName, "tga" );
    xstring Path;
    xstring File;

    CommandLine.SplitPath( PathName, Path, File );
    PathName = CommandLine.JoinPath( OutputFolder, File );

	// Is the bitmap a power of 2.
	if( g_Debug )
	{
		if( (Bitmap.GetHeight() % 2) || (Bitmap.GetWidth() % 2) )
		{
			x_printf( "Warning: Bitmap[%s] is not a power of 2", BitmapName );
		}
	}

    // Check if the file already exists
    if( Overwrite || !CommandLine.FileExists( PathName ) )
    {
        // Save the file
        if( !Bitmap.SaveTGA( PathName ) && g_Debug)
        {
            x_printf( "Error - Saving TGA \"%s\"\n", PathName );
        }
    }
    else
    {
        // Display error
        if( g_Debug )
			x_printf( "Error - File \"%s\" already exists\n", PathName );
    }
}

//==============================================================================
//  Compile PC
//==============================================================================

void CompileTargetPC( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OptString, xbool Overwrite )
{
    xstring     OutputFolder;

    // Reset script state
    OutputFolder.Clear();

    auxbmp_ConvertToD3D( Bitmap );

    // Add a trailing slash to the OptString
    s32 OptStringLen = OptString.GetLength();
    if( (OptStringLen > 0) && ((OptString[OptStringLen-1] != '\\') && (OptString[OptStringLen-1] != '/')) )
        OptString += '\\';

    // Set the output folder
    OutputFolder    = OptString;

	// Check what format was specified for the output.
	if( g_OutputFormat & WRITE_XBMP )
	{
		Writexbmp( CommandLine, Bitmap, BitmapName, OutputFolder, Overwrite );
	}
	else if( g_OutputFormat & WRITE_TGA )
	{
		Writetga( CommandLine, Bitmap, BitmapName, OutputFolder, Overwrite );
	}
	else
	{
		if( g_Debug )
			x_printf( "Error: No Output format specified." );
	}
}

//==============================================================================
//  Compile PS2
//==============================================================================

void CompileTargetPS2( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OptString, xbool Overwrite )
{
    xstring     OutputFolder;

    // Reset script state
    OutputFolder.Clear();

    auxbmp_ConvertToPS2( Bitmap );

    // Add a trailing slash to the OptString
    s32 OptStringLen = OptString.GetLength();
    if( (OptStringLen > 0) && ((OptString[OptStringLen-1] != '\\') && (OptString[OptStringLen-1] != '/')) )
        OptString += '\\';

    // Set the output folder
    OutputFolder    = OptString;

	// Check what format was specified for the output.
	if( g_OutputFormat & WRITE_XBMP )
	{
		Writexbmp( CommandLine, Bitmap, BitmapName, OutputFolder, Overwrite );
	}
	else if( g_OutputFormat & WRITE_TGA )
	{
		Writetga( CommandLine, Bitmap, BitmapName, OutputFolder, Overwrite );
	}
	else
	{
		if( g_Debug )
			x_printf( "Error: No Output format specified." );
	}
}

//==============================================================================
//  Comile Animated Bitmaps for PC
//==============================================================================

void CompileAnimBitmapTargetPC( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OptString, xbool Overwrite, s32 Index )
{
    xstring     OutputFolder;

    // Reset script state
    OutputFolder.Clear();

    // Add a trailing slash to the OptString
    s32 OptStringLen = OptString.GetLength();
    if( (OptStringLen > 0) && ((OptString[OptStringLen-1] != '\\') && (OptString[OptStringLen-1] != '/')) )
        OptString += '\\';

    // Set the output folder
    OutputFolder    = OptString;

	// Split the string up in to it 3 component, ( Normal name, Anim Bitmap Naming scheme, extension )
	xstring FirstHalf  = BitmapName.Left ( Index );
	xstring SecondHalf = BitmapName.Right( (BitmapName.GetLength() - Index) );
	s32     ExtIndex   = SecondHalf.Find ( '.' );
	xstring Ext        = SecondHalf.Right( (SecondHalf.GetLength() - ExtIndex) );

	// We already have one of the bitmap loaded.
	s32		AnimCount  = 1;
	xbool	Loaded     = TRUE;

	// Stores the new name of the file to open.
	xstring AnimBitmapName;
	
	
	// Find out how many bitmaps belong to the animation sequence.
	while( Loaded )
	{
		// Format the string to check how many animated bitmap does this sequence have.
		SecondHalf.Format( "_ANIM%04d", AnimCount);
		AnimBitmapName = FirstHalf + SecondHalf;
		AnimBitmapName = AnimBitmapName + Ext;

		// Does the bitmap exist.
		if( x_fopen( AnimBitmapName, "rb" ) )
		{
			AnimCount++;
			Loaded = TRUE;
		}
		else
		{
			Loaded = FALSE;
		}
	}

	xbitmap* pAnimBitmaps = new xbitmap	[ AnimCount ];

	// Load all the bitmap up.
	s32 i = 0;
	for(; i < AnimCount; i++ )
	{
		// Format the string and load all the bitmaps that belong to that animated bitmap sequence.
		SecondHalf.Format( "_ANIM%04d", i);
		AnimBitmapName = FirstHalf + SecondHalf;
		AnimBitmapName = AnimBitmapName + Ext;

		Loaded = auxbmp_Load( pAnimBitmaps[i], AnimBitmapName );

		if( Loaded )
		{
			auxbmp_ConvertToD3D( pAnimBitmaps[i] );
			// Is the bitmap a power of 2.
			if( g_Debug )
			{
				if( (pAnimBitmaps[i].GetHeight() % 2) || (pAnimBitmaps[i].GetWidth() % 2) )
				{
					x_printf( "Warning: Bitmap[%s] is not a power of 2", AnimBitmapName );
				}
			}
		}
	}

	// Check what format was specified for the output.
	if( g_OutputFormat & WRITE_TEXTURE )
	{		
		xstring PathName = CommandLine.ChangeExtension( BitmapName, "animxbmp" );
		xstring Path;
		xstring File;

		CommandLine.SplitPath( PathName, Path, File );
		PathName = CommandLine.JoinPath( OutputFolder, File );

		X_FILE* pFile;
		
		// Check if the file already exist, if it does then check if we have the overwrite turned on.
		pFile = x_fopen( PathName, "rb" );
		if( pFile )
		{
			// Can we overwrite.
			if( Overwrite )
			{
				pFile = x_fopen( PathName, "wb" );
			}
			else
			{
				// Display error
				if( g_Debug )
					x_printf( "Error - File \"%s\" already exists\n", PathName );
				
				delete [] pAnimBitmaps;
				return;
			}
		}
		else
		{
			pFile = x_fopen( PathName, "wb" );
		}
		
		// Store the count.
		x_fwrite( &AnimCount, sizeof( s32 ), 1, pFile );
		
		s32 i;
		for( i = 0; i < AnimCount; i++ )
		{
			// Did we save.
			if( !pAnimBitmaps[i].Save( pFile ) && g_Debug )
			{
				x_printf( "Error - Saving XBMP \"%s\"\n", PathName );
			}
		}

		// Can the memory.
		delete [] pAnimBitmaps;
	}
	else
	{
		if( g_Debug )
			x_printf( "Error: No Output format specified." );
	}
}



//==============================================================================
//  Comile Animated Bitmaps for PS2
//==============================================================================

void CompileAnimBitmapTargetPS2( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OptString, xbool Overwrite, s32 Index )
{
    xstring     OutputFolder;

    // Reset script state
    OutputFolder.Clear();

    // Add a trailing slash to the OptString
    s32 OptStringLen = OptString.GetLength();
    if( (OptStringLen > 0) && ((OptString[OptStringLen-1] != '\\') && (OptString[OptStringLen-1] != '/')) )
        OptString += '\\';

    // Set the output folder
    OutputFolder    = OptString;

	// Split the string up in to it 3 component, ( Normal name, Anim Bitmap Naming scheme, extension )
	xstring FirstHalf  = BitmapName.Left ( Index );
	xstring SecondHalf = BitmapName.Right( (BitmapName.GetLength() - Index) );
	s32     ExtIndex   = SecondHalf.Find ( '.' );
	xstring Ext        = SecondHalf.Right( (SecondHalf.GetLength() - ExtIndex) );

	// We already have one of the bitmap loaded.
	s32		AnimCount  = 1;
	xbool	Loaded     = TRUE;

	// Stores the new name of the file to open.
	xstring AnimBitmapName;
	
	// Find out how many bitmaps belong to the animation sequence.
	while( Loaded )
	{
		// Format the string to check how many animated bitmap does this sequence have.
		SecondHalf.Format( "_ANIM%04d", AnimCount);
		AnimBitmapName = FirstHalf + SecondHalf;
		AnimBitmapName = AnimBitmapName + Ext;

		// Does the bitmap exist.
		if( x_fopen( AnimBitmapName, "rb" ) )
		{
			AnimCount++;
			Loaded = TRUE;
		}
		else
		{
			Loaded = FALSE;
		}
	}

	xbitmap* pAnimBitmaps = new xbitmap	[ AnimCount ];

	s32 i = 0;
	for(; i < AnimCount; i++ )
	{
		// Format the string and load all the bitmaps that belong to that animated bitmap sequence.
		SecondHalf.Format( "_ANIM%04d", i);
		AnimBitmapName = FirstHalf + SecondHalf;
		AnimBitmapName = AnimBitmapName + Ext;

		Loaded = auxbmp_Load( pAnimBitmaps[i], AnimBitmapName );

		if( Loaded )
		{
			auxbmp_ConvertToPS2( pAnimBitmaps[i] );
			// Is the bitmap a power of 2.
			if( g_Debug )
			{
				if( (pAnimBitmaps[i].GetHeight() % 2) || (pAnimBitmaps[i].GetWidth() % 2) )
				{
					x_printf( "Warning: Bitmap[%s] is not a power of 2", AnimBitmapName );
				}
			}
		}
	}

	// Check what format was specified for the output.
	if( g_OutputFormat & WRITE_TEXTURE )
	{		
		xstring PathName = CommandLine.ChangeExtension( BitmapName, "animxbmp" );
		xstring Path;
		xstring File;

		CommandLine.SplitPath( PathName, Path, File );
		PathName = CommandLine.JoinPath( OutputFolder, File );

		X_FILE* pFile;
		
		// Check if the file already exist, if it does then check if we have the overwrite turned on.
		pFile = x_fopen( PathName, "rb" );
		if( pFile )
		{
			// Can we overwrite.
			if( Overwrite )
			{
				pFile = x_fopen( PathName, "wb" );
			}
			else
			{
				// Display error
				if( g_Debug )
					x_printf( "Error - File \"%s\" already exists can't Overwrite.\n", PathName );
				
				delete [] pAnimBitmaps;
				return;
			}
		}
		else
		{
			pFile = x_fopen( PathName, "wb" );
		}
		
		// Store the count.
		x_fwrite( &AnimCount, sizeof( s32 ), 1, pFile );
		
		s32 i;
		for( i = 0; i < AnimCount; i++ )
		{
			// Did we save.
			if( !pAnimBitmaps[i].Save( pFile ) && g_Debug )
			{
				x_printf( "Error - Saving XBMP \"%s\"\n", PathName );
			}
		}

		// Can the memory.
		delete [] pAnimBitmaps;
	}
	else
	{
		if( g_Debug )
			x_printf( "Error: No Output format specified." );
	}
}

//==============================================================================
//  Display Help
//==============================================================================

void DisplayHelp( void )
{
    x_printf( "\n" );
    x_printf( "xbmpCompiler (c)2001 Inevitable Entertainment Inc.\n" );
    x_printf( "\n" );
    x_printf( "  usage:\n" );
    x_printf( "         xbmpCompiler [-opt [param]] [filenames]\n" );
    x_printf( "\n" );
    x_printf( "options:\n" );
    x_printf( "         -convert		  <format>	- Convert to <format> see list below\n" );
    x_printf( "         -mips			  <num mips>- Build mips\n" );
    x_printf( "         -r							- Recurse on subdirectories\n" );
    x_printf( "         -pc				  <folder>	- Set PC mode and set output folder\n" );
    x_printf( "         -ps2			  <folder>	- Set PS2 mode and set output folder\n" );
    x_printf( "         -ngc			  <folder>	- Set NGC mode and set output folder\n" );
    x_printf( "         -xbox			  <folder>	- Set XBOX mode  and set output folder\n" );
    x_printf( "         -ps2swizzle					- Swizzle Clut for ps2\n");
    x_printf( "         -info						- Display info about bitmap\n" );
    x_printf( "         -log						- Display debug info while compiling\n" );
	x_printf( "         -composite					- Copies R value from Dest to Src A value\n" );
	x_printf( "         -progress					- What percentage of the file has been compilied\n" );
    x_printf( "         -writexbmp					- Write as xbmp\n" );
    x_printf( "         -writetga					- Write as tga\n" );
    x_printf( "         -animbitmapwidth  <width>	- Maximum width of the generated bitmap\n" );
    x_printf( "         -animbitmapheight <height>  - Maximum height of the generated bitmap\n" );
    x_printf( "         -writetexture				- Write animated bitmaps as texture\n" );
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
			
			s32 Index = BitmapName.Find( AnimBitmap );
			if( Index != -1 )
			{
				CompileAnimBitmapTargetPC( CommandLine, Bitmap, BitmapName, OptString, Overwrite, Index);
			}
			else
			{
				CompileTargetPC( CommandLine, Bitmap, BitmapName, OptString, Overwrite );
			}
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "PS2" )
        {
            Target = BITMAP_TARGET_PS2;

			s32 Index = BitmapName.Find( AnimBitmap );
			if( Index != -1 )
			{
				CompileAnimBitmapTargetPS2( CommandLine, Bitmap, BitmapName, OptString, Overwrite, Index);
			}
			else
			{
				CompileTargetPS2( CommandLine, Bitmap, BitmapName, OptString, Overwrite );
			}
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "NGC" )
        {
			ASSERTS( FALSE, "Unsupported exprot platform" );
            
			// Add a trailing slash to the OptString
            s32 OptStringLen = OptString.GetLength();
            if( (OptStringLen > 0) && ((OptString[OptStringLen-1] != '\\') && (OptString[OptStringLen-1] != '/')) )
                OptString += '\\';

            // Set the output folder
            OutputFolder    = OptString;

			// Check what format was specified for the output.
			if( g_OutputFormat & WRITE_XBMP )
			{
				Writexbmp( CommandLine, Bitmap, BitmapName, OutputFolder, Overwrite );
			}
			else if( g_OutputFormat & WRITE_TGA )
			{
				Writetga( CommandLine, Bitmap, BitmapName, OutputFolder, Overwrite );
			}
			else
			{
				if( g_Debug )
					x_printf( "Error: No Output format specified." );
			}
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "XBOX" )
        {
			ASSERTS( FALSE, "Unsupported exprot platform" );

            // Add a trailing slash to the OptString
            s32 OptStringLen = OptString.GetLength();
            if( (OptStringLen > 0) && ((OptString[OptStringLen-1] != '\\') && (OptString[OptStringLen-1] != '/')) )
                OptString += '\\';

            // Set the output folder
            OutputFolder    = OptString;

			// Check what format was specified for the output.
			if( g_OutputFormat & WRITE_XBMP )
			{
				Writexbmp( CommandLine, Bitmap, BitmapName, OutputFolder, Overwrite );
			}
			else if( g_OutputFormat & WRITE_TGA )
			{
				Writetga( CommandLine, Bitmap, BitmapName, OutputFolder, Overwrite );
			}
			else
			{
				if( g_Debug )
				x_printf( "Error: No Output format specified." );
			}
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "PS2SWIZZLE" )
        {
            Bitmap.PS2SwizzleClut();
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "PROGRESS" )
        {
            g_Progress = TRUE;
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "LOG" )
        {
            g_Debug = TRUE;
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "WRITETGA" )
        {
            g_OutputFormat |= WRITE_TGA;
        }
        //==============================================================================
        //==============================================================================
        if( OptName == "WRITETEXTURE" )
        {
			g_OutputFormat |= WRITE_TEXTURE;
		}

        //==============================================================================
        //==============================================================================
        if( OptName == "WRITEXBMP" )
        {
            g_OutputFormat |= WRITE_XBMP;
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
                if( g_Debug )
					x_printf( "Error: Unsupported format \"%s\"\n", OptString );
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
				if( g_Debug )
					x_printf( "Error: Illegal number of mips requested %d\n", nMips );
            }
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "COMPOSITE" )
        {
			s32 j = 0;
			if( g_CompositeCount == 0 )
			{
				for( s32 j=0; j < CommandLine.GetNumOptions(); j++ )
				{
					// Get option name and string
					xstring Name   = CommandLine.GetOptionName( j );
					if( Name == "COMPOSITE" )
						g_CompositeCount++;

				}
			}

			if( g_BitmapCompositedCount == g_CompositeCount )
				continue;
	
			xstring OptString = CommandLine.GetOptionString( i );
			xstring     SrcName;
			xstring     DestName;

			// Reset script state
			SrcName.Clear();
			DestName.Clear();

			// Find out where the space was and sperate the string.
			j = 0;
			while( OptString[j] != ' ' )
				j++;
	
			SrcName = OptString.Left( j );
	
			// Get rid of the space.
			j++;

			DestName = OptString.Right( (OptString.GetLength() - j) );

			xbitmap Dest;
			xbitmap Src;
			xbool DestLoaded = auxbmp_Load( Dest, DestName );
			xbool SrcLoaded = auxbmp_Load( Src, SrcName );
			
			// Check if loaded correctly and that both the bitmaps have the same size.
			if( DestLoaded &&  SrcLoaded )
			{
				if( (Dest.GetHeight() == Src.GetHeight()) && (Dest.GetWidth() == Src.GetWidth()) ) 
				{
					xcolor SrcPixel;
					xcolor DestPixel;
					// Get all the pixels and set there alpha.
					for( s32 x = 0; x < Src.GetWidth(); x++ )
					{
						for( s32 y = 0; y < Src.GetHeight(); y++ )
						{
							// Copy the R value from the destination bitmap into the A value of the Source bitmap.
							SrcPixel = Src.GetPixelColor( x, y );
							DestPixel = Dest.GetPixelColor( x, y );
							SrcPixel.A = DestPixel.R;
							Src.SetPixelColor( SrcPixel, x, y );							
						}
					}
				}
				else
				{
					// No - failed to load
					if( g_Debug )
						x_printf( "Error: The Source[%s] and Destination[%s] bitmap are unproportional \n", BitmapName, DestName );					
				}
			}
			else
			{
				// No - failed to load
				if( g_Debug )
				{
					if( !SrcLoaded )
						x_printf( "Error: Can't load Source \"%s\"\n", SrcName );
					else
						x_printf( "Error: Can't load Dest \"%s\"\n", DestName );
				}
			}

			for( j=0; j < CommandLine.GetNumOptions(); j++ )
			{
				// Get option name and string
				xstring Name   = CommandLine.GetOptionName( j );
				xstring OptString = CommandLine.GetOptionString( j );
				if( Name == "PC" )
				{
					CompileTargetPC( CommandLine, Src, SrcName, OptString, Overwrite );
				}
				else if( Name == "PS2" )
				{
					CompileTargetPS2( CommandLine, Src, SrcName, OptString, Overwrite );
				}
				else if( Name == "NGC" )
				{
					
				}
				else if( Name == "XBOX" )
				{
					
				}
			}

			// Keep track of how many bitmaps have been composited.
			g_BitmapCompositedCount++;
        }

        //==============================================================================
        //==============================================================================
        if( OptName == "ANIMBITMAPWIDTH" )
		{
			g_MaxAnimBitmapWidth = x_atoi( OptString );

			// Make sure that the Width is a power of 2.
			if( g_MaxAnimBitmapWidth % 2 )
			{
				if( g_Debug )
				{
						x_printf( "Error: AnimBitmapWidth[%d] is not a power of 2 \n", g_MaxAnimBitmapWidth );
				}
				
				// Reset the Height so we don't construct an incorrect bitmap.
				g_MaxAnimBitmapWidth = 0;
			}				
		}

        //==============================================================================
        //==============================================================================
        if( OptName == "ANIMBITMAPHEIGHT" )
		{
			g_MaxAnimBitmapHeight = x_atoi( OptString );

			if( g_MaxAnimBitmapHeight % 2 )
			{
				if( g_Debug )
				{
						x_printf( "Error: AnimBitmapHeight[%d] is not a power of 2 \n", g_MaxAnimBitmapHeight );
				}
				
				// Reset the Height so we don't construct an incorrect bitmap.
				g_MaxAnimBitmapHeight = 0;
			}				
		}

    }
}

//==============================================================================
//  main
//==============================================================================

int main( int argc, char** argv )
{
    x_Init();
	s32             i;
    command_line    CommandLine;
    xbool           NeedHelp;
    xbool           IsRecursive;
    xarray<xstring> BitmapFiles;

    // Setup recognized command line options
    CommandLine.AddOptionDef( "R"											);
    CommandLine.AddOptionDef( "INFO"										);
    CommandLine.AddOptionDef( "LOG" 										);
    CommandLine.AddOptionDef( "PC",					command_line::STRING	);
    CommandLine.AddOptionDef( "PS2",				command_line::STRING	);
    CommandLine.AddOptionDef( "NGC",				command_line::STRING	);
    CommandLine.AddOptionDef( "XBOX",				command_line::STRING	);
    CommandLine.AddOptionDef( "PS2SWIZZLE"									);
    CommandLine.AddOptionDef( "WRITETGA"									);
    CommandLine.AddOptionDef( "WRITEXBMP"									);
	CommandLine.AddOptionDef( "WRITETEXTURE"								);
    CommandLine.AddOptionDef( "OVERWRITE"									);
    CommandLine.AddOptionDef( "PROGRESS"									);
    CommandLine.AddOptionDef( "CONVERT",			command_line::STRING	);
    CommandLine.AddOptionDef( "MIPS",				command_line::STRING	);
    CommandLine.AddOptionDef( "COMPOSITE",			command_line::STRING	);
	CommandLine.AddOptionDef( "ANIMBITMAPWIDTH",	command_line::STRING	);	
	CommandLine.AddOptionDef( "ANIMBITMAPHEIGHT",	command_line::STRING	);	

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
            if( g_Debug )
				x_printf( "Error: Can't load bitmap \"%s\"\n", BitmapName );
        }

		if( g_Progress )
		{
			s32 Percentage = (s32)(( (f32)i / (f32)BitmapFiles.GetCount() ) * 100.0f);

			// Display the progress of the file that is being compiled.
			x_printf( "%: %d\n", Percentage );
		}
    }
	
	x_Kill();
    // Return Success
    return 0;
}


/*

		THIS CODE FOR TILING ANIMATED BITMAPS IN TO ONE BITMAP, ITS ABOUT 90% DONE, ALL THAT NEEDS TO GET DONE TO CHECK WHY
		xbitmap::WritePixelColor ISN'T WRITING THE NEW PIXEL COLOR OVER EXCEPT FOR THE FIRST 4 BYTES.  YOU ALSO NEED TO
		EXPORT THE UV DATA.

//==============================================================================
//  Comile Animated Bitmaps for PS2
//==============================================================================

void CompileAnimBitmapTargetPS2( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OptString, xbool Overwrite, s32 Index )
{
    xstring     OutputFolder;

    // Reset script state
    OutputFolder.Clear();

    // Add a trailing slash to the OptString
    s32 OptStringLen = OptString.GetLength();
    if( (OptStringLen > 0) && ((OptString[OptStringLen-1] != '\\') && (OptString[OptStringLen-1] != '/')) )
        OptString += '\\';

    // Set the output folder
    OutputFolder    = OptString;

	// Split the string up in to it 3 component, ( Normal name, Anim Bitmap Naming scheme, extension )
	xstring FirstHalf  = BitmapName.Left ( Index );
	xstring SecondHalf = BitmapName.Right( (BitmapName.GetLength() - Index) );
	s32     ExtIndex   = SecondHalf.Find ( '.' );
	xstring Ext        = SecondHalf.Right( (SecondHalf.GetLength() - ExtIndex) );

	// We already have one of the bitmap loaded.
	s32		AnimCount  = 1;
	xbool	Loaded     = TRUE;

	// Stores the new name of the file to open.
	xstring AnimBitmapName;
	
	while( Loaded )
	{
		// Format the string to check how many animated bitmap does this sequence have.
		SecondHalf.Format( "_ANIM%04d", AnimCount);
		AnimBitmapName = FirstHalf + SecondHalf;
		AnimBitmapName = AnimBitmapName + Ext;

		// Does the bitmap exist.
		if( x_fopen( AnimBitmapName, "rb" ) )
		{
			AnimCount++;
			Loaded = TRUE;
		}
		else
		{
			Loaded = FALSE;
		}
		
	}

	xbitmap* pAnimBitmaps	= new xbitmap	[ AnimCount ];
	pAnimBitmaps[0] = Bitmap;

	s32 i = 1;
	for(; i < AnimCount; i++ )
	{
		// Format the string and load all the bitmaps that belong to that animated bitmap sequence.
		SecondHalf.Format( "_ANIM%04d", i);
		AnimBitmapName = FirstHalf + SecondHalf;
		AnimBitmapName = AnimBitmapName + Ext;

		Loaded = auxbmp_Load( pAnimBitmaps[i], AnimBitmapName );

		if( Loaded )
		{
			// Is the bitmap a power of 2.
			if( g_Debug )
			{
				if( (pAnimBitmaps[i].GetHeight() % 2) || (pAnimBitmaps[i].GetWidth() % 2) )
				{
					x_printf( "Warning: Bitmap[%s] is not a power of 2", AnimBitmapName );
				}
			}
		}
	}
	
	s32 Area = (Bitmap.GetWidth() * Bitmap.GetHeight()) * AnimCount;
	
	// Make sure that all the bitmaps will fit in the specified size.
	if( (g_MaxAnimBitmapWidth * g_MaxAnimBitmapHeight) < Area )
	{
		if( g_Debug )
			x_printf( "Error: The current animated bitmaps[%s] cannot fit in the specified size Width:%d Height:%d", BitmapName, g_MaxAnimBitmapWidth, g_MaxAnimBitmapHeight );
		
		return;
	}
	
	// Get the starting width and height.
	s32 Width  = Bitmap.GetWidth();
	s32 Height = Bitmap.GetHeight();
	
	// Up the width or height ( depending on which is greater ) by the power of 2
	if( Width >= Height )
		Width *= 2;
	else
		Height *= 2;

	while( Area > ( Width*Height ) )
	{
		if( Width > Height )
		{
			Height *= 2;
			
			// Make sure that we don't go over the maxium Height.
			if( Height > g_MaxAnimBitmapHeight )
				Height = g_MaxAnimBitmapHeight;
		}
		else
		{
			Width *= 2;

			// Make sure that we don't go over the maxium Width.
			if( Width > g_MaxAnimBitmapWidth )
				Width = g_MaxAnimBitmapWidth;
		}
	}
	
	s32 Cols, Rows;
	Cols = Height / Bitmap.GetHeight();
	Rows = Width / Bitmap.GetWidth();

	if( AnimCount < ( Cols*Rows ) )
	{
		if( g_Debug )
			x_printf( " In animated bitmap [%s] you are wasting %d pixels", BitmapName, (((Cols*Rows)-AnimCount)*(Bitmap.GetWidth()*Bitmap.GetHeight())) );
	}
	
	xbitmap AnimatedBitmaps;
	byte* PixelData = (byte*)x_malloc( Bitmap.GetDataSize() * AnimCount );
	AnimatedBitmaps.Setup( Bitmap.GetFormat(), Width, Height, TRUE, PixelData );
	i = 0;

	for( ; i < AnimCount; i++ )
	{
		s32 LastPixelX = 0;
		s32 LastPixelY = 0;
		s32 j = 0;
		s32 p = 0;
		for( ; j < pAnimBitmaps[i].GetWidth(); j++ )
		{
			for( ; p < pAnimBitmaps[i].GetHeight(); p++ )
			{
				AnimatedBitmaps.SetPixelColor( pAnimBitmaps[i].GetPixelColor( j, p ), LastPixelX+j, LastPixelY+p );				
			}
		}
		
		LastPixelX = pAnimBitmaps[i].GetWidth();
		if( LastPixelX >= Width )
		{
			LastPixelX = 0;
			LastPixelY = pAnimBitmaps[i].GetHeight();
		}
	}

    auxbmp_ConvertToPS2( AnimatedBitmaps );
		
	// Check what format was specified for the output.
	if( g_OutputFormat & WRITE_XBMP )
	{
		Writexbmp( CommandLine, AnimatedBitmaps, BitmapName, OutputFolder, Overwrite );
	}
	else if( g_OutputFormat & WRITE_TGA )
	{
		Writetga( CommandLine, AnimatedBitmaps, BitmapName, OutputFolder, Overwrite );
	}
	else
	{
		if( g_Debug )
			x_printf( "Error: No Output format specified." );
	}

}

*/







