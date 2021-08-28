//==============================================================================
//
//  FontBuilder.cpp
//
//==============================================================================

#include "x_files.hpp"
#include "CommandLine.hpp"
#include "x_bitmap.hpp"
#include "../../Auxiliary/bitmap/aux_bitmap.hpp"

#include <windows.h>
#include <tchar.h>

//==============================================================================
//  Defines
//==============================================================================

#define WRITE_XBMP		( 1<<1 )
#define WRITE_TGA		( 1<<2 )


#define PLATFORM_PC		( 1<<4 )
#define PLATFORM_PS2	( 1<<5 )

//==============================================================================
//  Static Data
//==============================================================================

HDC         s_dc = 0;
TEXTMETRIC  s_tm;
HBITMAP     s_Bitmap;
HBRUSH      s_FillBrush;
RECT        s_FillRect;
HFONT       s_Font;

xstring     s_FontName		= "";
s32         s_FirstChar		= 32;
s32         s_NumChars		= 16;
s32         s_Antialias		= 1;
s32         s_PointSize		= 12;
s32         s_Weight		= 400;
xbool       s_Italic		= FALSE;
xbool       s_Underline		= FALSE;
xbool       s_RtoA			= FALSE;
xbool		s_Debug			= FALSE;
u8			s_OutputFormat	= 0;
xstring     BitmapName		= "Font.tga";
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
    SetTextColor( s_dc, RGB(255,255,255) );
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
    bmp.Setup( xbitmap::FMT_32_RGBA_8888, wDst, hDst, TRUE, (byte*)new char[ wDst*hDst*4 ] );

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
//  Write xbitmap
//==============================================================================

void Writexbmp( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OutputFolder)
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
    if( !CommandLine.FileExists( PathName ) )
    {
        // Save the file
        if( !Bitmap.Save( PathName ) && s_Debug)
        {
            x_printf( "Error - Saving XBMP \"%s\"\n", PathName );
        }
    }
    else
    {
        // Display error
        if( s_Debug )
			x_printf( "Error - File \"%s\" already exists\n", PathName );
    }
}

//==============================================================================
//  Write tga
//==============================================================================

void Writetga( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OutputFolder)
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
    if( !CommandLine.FileExists( PathName ) )
    {
        // Save the file
        if( !Bitmap.SaveTGA( PathName ) && s_Debug)
        {
            x_printf( "Error - Saving TGA \"%s\"\n", PathName );
        }
    }
    else
    {
        // Display error
        if( s_Debug )
			x_printf( "Error - File \"%s\" already exists\n", PathName );
    }
}

//==============================================================================
//  Compile PC
//==============================================================================

void CompileTargetPC( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OptString)
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
	if( s_OutputFormat & WRITE_XBMP )
	{
		Writexbmp( CommandLine, Bitmap, BitmapName, OutputFolder);
	}
	if( s_OutputFormat & WRITE_TGA )
	{
		Writetga( CommandLine, Bitmap, BitmapName, OutputFolder);
	}
	else
	{
		if( s_Debug )
			x_printf( "Error: No Output format specified." );
	}
}

//==============================================================================
//  Compile PS2
//==============================================================================

void CompileTargetPS2( command_line& CommandLine, xbitmap& Bitmap, const xstring& BitmapName, xstring& OptString)
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
	if( s_OutputFormat & WRITE_XBMP )
	{
		Writexbmp( CommandLine, Bitmap, BitmapName, OutputFolder);
	}
	if( s_OutputFormat & WRITE_TGA )
	{
		Writetga( CommandLine, Bitmap, BitmapName, OutputFolder );
	}
	else
	{
		if( s_Debug )
			x_printf( "Error: No Output format specified." );
	}
}


//==============================================================================

void main( int argc, char** argv )
{
    command_line    cl;
	xstring OptStringPC;
	xstring OptStringPS2;

    // Setup the options to look for
//    cl.AddOptionDef( "FONT",      command_line::STRING );
    cl.AddOptionDef( "FONT",      command_line::STRING );
    cl.AddOptionDef( "FIRSTCHAR", command_line::STRING );
    cl.AddOptionDef( "NUMCHARS",  command_line::STRING );
    cl.AddOptionDef( "ANTIALIAS", command_line::STRING );
    cl.AddOptionDef( "SIZE",      command_line::STRING );
    cl.AddOptionDef( "WEIGHT",    command_line::STRING );
    cl.AddOptionDef( "ITALIC",    command_line::SWITCH );
    cl.AddOptionDef( "UNDERLINE", command_line::SWITCH );
    cl.AddOptionDef( "REDTOALPHA"					   );
    cl.AddOptionDef( "PC",        command_line::STRING );
    cl.AddOptionDef( "PS2",       command_line::STRING );
    cl.AddOptionDef( "NGC",	      command_line::STRING );
    cl.AddOptionDef( "XBOX",      command_line::STRING );
    cl.AddOptionDef( "WRITETGA"					       );
    cl.AddOptionDef( "WRITEXBMP"                       );
    cl.AddOptionDef( "LOG"		                       );


    // Parse the command line
    if( cl.Parse( argc, argv ) )
    {
        // Parse error, display help
        x_printf( "FontBuilder <font_name>\n" );
    }
    else
    {
        s32     i;

        // Process Options
        for( i=0 ; i<cl.GetNumOptions() ; i++ )
        {
            xstring Option = cl.GetOptionName( i );
            if( Option == "FIRSTCHAR" )
            {
                s_FirstChar = atoi( cl.GetOptionString( i ) );
                if( s_FirstChar < 0  ) s_FirstChar = 0;
            }
            if( Option == "NUMCHARS" )
            {
                s_NumChars = atoi( cl.GetOptionString( i ) );
                if( s_NumChars < 1  ) s_NumChars = 1;
            }
            if( Option == "ANTIALIAS" )
            {
                s_Antialias = atoi( cl.GetOptionString( i ) );
                if( s_Antialias < 1  ) s_Antialias = 1;
            }
            if( Option == "FONT" )
                s_FontName  = cl.GetOptionString( i );
            if( Option == "SIZE" )
                s_PointSize = atoi( cl.GetOptionString( i ) );
            if( Option == "WEIGHT" )
                s_Weight = atoi( cl.GetOptionString( i ) );
            if( Option == "ITALIC" )
                s_Italic = TRUE;
            if( Option == "UNDERLINE" )
                s_Underline = TRUE;
            if( Option == "REDTOALPHA" )
                s_RtoA = TRUE;

            if( Option == "PC" )
			{
                OptStringPC = cl.GetOptionString( i );
				s_OutputFormat |= PLATFORM_PC;
			}
            
			if( Option == "PS2" )
			{
                OptStringPS2 = cl.GetOptionString( i );
				s_OutputFormat |= PLATFORM_PS2;
			}
            
			if( Option == "NGC" )
                ASSERTS( FALSE, "Under Construction" );
            
			if( Option == "XBOX" )
                ASSERTS( FALSE, "Under Construction" );

            if( Option == "WRITETGA" )
                s_OutputFormat |= WRITE_XBMP;
            
			if( Option == "WRITEXBMP" )
                s_OutputFormat |= WRITE_TGA;
            
			if( Option == "LOG" )
                s_Debug = TRUE;
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
/*
        x_printf( "tmHeight           %d\n", s_tm.tmHeight           );
        x_printf( "tmAscent           %d\n", s_tm.tmAscent           );
        x_printf( "tmDescent          %d\n", s_tm.tmDescent          );
        x_printf( "tmInternalLeading  %d\n", s_tm.tmInternalLeading  );
        x_printf( "tmExternalLeading  %d\n", s_tm.tmExternalLeading  );
        x_printf( "tmAveCharWidth     %d\n", s_tm.tmAveCharWidth     );
        x_printf( "tmMaxCharWidth     %d\n", s_tm.tmMaxCharWidth     );
        x_printf( "tmWeight           %d\n", s_tm.tmWeight           );
        x_printf( "tmOverhang         %d\n", s_tm.tmOverhang         );
        x_printf( "tmDigitizedAspectX %d\n", s_tm.tmDigitizedAspectX );
        x_printf( "tmDigitizedAspectY %d\n", s_tm.tmDigitizedAspectY );
        x_printf( "tmFirstChar        %d\n", s_tm.tmFirstChar        );
        x_printf( "tmLastChar         %d\n", s_tm.tmLastChar         );
        x_printf( "tmDefaultChar      %d\n", s_tm.tmDefaultChar      );
        x_printf( "tmBreakChar        %d\n", s_tm.tmBreakChar        );
        x_printf( "tmItalic           %d\n", s_tm.tmItalic           );
        x_printf( "tmUnderlined       %d\n", s_tm.tmUnderlined       );
        x_printf( "tmStruckOut        %d\n", s_tm.tmStruckOut        );
        x_printf( "tmPitchAndFamily   %d\n", s_tm.tmPitchAndFamily   );
        x_printf( "tmCharSet          %d\n", s_tm.tmCharSet          );
*/
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

        if( s_RtoA )
        {
            // Create bitmaps for each character
            xarray<xbitmap> Bitmaps;
            for( s32 c=32 ; c<128 ; c++ )
            {
                _TCHAR Glyph = c;

                // Convert to an xbitmap
                xbitmap b = GetGlyph( Glyph );
                for( s32 y=0 ; y<b.GetHeight() ; y++ )
                {
                    for( s32 x=0 ; x<b.GetWidth() ; x++ )
                    {
                        xcolor c = b.GetPixelColor( x, y );
                        c.A = c.R;
                        c.R = c.G = c.B = 0xff;
                        if( (x==0) && (y==0) )
                            c.R = c.G = 0x00;
                        b.SetPixelColor( c, x, y );
                    }
                }
                Bitmaps.Append() = b;
            }

            // Calculate size of final bitmap & create
            s32 fw = 0;
            s32 lw = 0;
            for( c=0 ; c<96 ; c++ )
            {
                lw += Bitmaps[c].GetWidth() ;
                if( lw > fw ) fw = lw;
                if( (c&15) == 15 ) lw = 0;
            }
            xbitmap fbm;
            s32     fbw = NextPower2(fw);
            s32     fbh = NextPower2((s_tm.tmHeight/s_Antialias)*6);
            fbm.Setup( xbitmap::FMT_32_RGBA_8888, fbw, fbh, TRUE, (byte*)new char[fbw*fbh*6*4] );
            {
                xcolor c(0,0,0,0);
                for( s32 y=0 ; y<fbh ; y++ )
                {
                    for( s32 x=0 ; x<fbw ; x++ )
                    {
                        fbm.SetPixelColor( c, x, y );
                    }
                }
            }

            // Build final bitmap
            s32 x = 0;
            s32 y = 0;
            for( c=0 ; c<96 ; c++ )
            {
                fbm.Blit( x, y, 0, 0, Bitmaps[c].GetWidth(), Bitmaps[c].GetHeight(), Bitmaps[c] );
                x += Bitmaps[c].GetWidth();
                fbm.SetPixelColor( xcolor(0,0,255,0), x, y );
                if( (c&15) == 15 )
                {
                    x = 0;
                    y += s_tm.tmHeight/s_Antialias;
                }
            }
			
			if( s_OutputFormat & PLATFORM_PC )
			{	
				CompileTargetPS2( cl, fbm, BitmapName, OptStringPC );
			}

			if( s_OutputFormat & PLATFORM_PS2 )
			{
				CompileTargetPS2( cl, fbm, BitmapName, OptStringPS2 );
			}
			
			//fbm.SaveTGA( "t.tga" );
        }
        else
        {
            // Create bitmaps for each character
            xarray<xbitmap> Bitmaps;
            for( s32 c=0 ; c<s_NumChars ; c++ )
            {
                _TCHAR Glyph = c+s_FirstChar;

                // Convert to an xbitmap
                Bitmaps.Append() = GetGlyph( Glyph );
            }

            // Calculate size of final bitmap & create
            s32 fw = 0;
            for( c=0 ; c<s_NumChars ; c++ )
            {
                fw += Bitmaps[c].GetWidth() ;
            }
            xbitmap fbm;
            fbm.Setup( xbitmap::FMT_32_RGBA_8888, fw, s_tm.tmHeight/s_Antialias, TRUE, (byte*)new char[fw*s_tm.tmHeight/s_Antialias*4] );

            // Build final bitmap
            s32 x = 0;
            for( c=0 ; c<s_NumChars ; c++ )
            {
                fbm.Blit( x, 0, 0, 0, Bitmaps[c].GetWidth(), Bitmaps[c].GetHeight(), Bitmaps[c] );
                x += Bitmaps[c].GetWidth();
            }

			if( s_OutputFormat & PLATFORM_PC )
			{	
				CompileTargetPS2( cl, fbm, BitmapName, OptStringPC );
			}

			if( s_OutputFormat & PLATFORM_PS2 )
			{
				CompileTargetPS2( cl, fbm, BitmapName, OptStringPS2 );
			}

            //fbm.SaveTGA( "t.tga" );
        }

        // Destroy objects
        SelectObject( s_dc, OldFont );
        SelectObject( s_dc, OldBitmap );
        DeleteObject( s_FillBrush );
        DeleteObject( s_Font );
        DeleteObject( s_Bitmap );
        DeleteDC( s_dc );
    }
}
