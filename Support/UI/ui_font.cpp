//=========================================================================
//
//  ui_font.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "Bitmap\aux_bitmap.hpp"
#include "ui_font.hpp"
//#include "guid/guid.hpp"
#include "ui/ui_manager.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

//=========================================================================

#define OFFSET_X    (2048-(512/2))
#define OFFSET_Y    (2048-(512/2))

#define CHAR_WIDTH          13
#define CHAR_HEIGHT         18
#define XBORDER              8
#define YBORDER              8

#define HELP_TEXT_SPACING            8
#define BUTTON_START_SPRITE_WIDTH   24

#ifdef TARGET_XBOX
static xbool ScaleText = FALSE;
#else
static xbool ScaleText = FALSE;
#endif

#ifdef TARGET_PS2

struct header
{
    dmatag  DMA;        // DMA tag
	giftag  PGIF;       // GIF for setting PRIM register
	s64     Prim;       // PRIM register
    s64     Dummy;
	giftag  GIF;		// GIF for actual primitives
};

struct char_info
{
	s64     Color1;		// RGBAQ register
    s64     T0;
    s64     P0;
    s64     T1;
    s64     P1;
	s64     Color2;		// RGBAQ register
    s64     T2;
    s64     P2;
    s64     T3;
    s64     P3;
};

#endif

#ifdef TARGET_PS2

static header*          s_pHeader;
static char_info*       s_pChar;
static s32              s_NChars;
static s32              s_NCharsAlloced;
static giftag           s_GIF       PS2_ALIGNMENT(16);
static giftag           s_PGIF      PS2_ALIGNMENT(16);

static
void ps2_NewCharBatch( void )
{
    // make sure we can fit at least one character at a time into this batch
    s32 nBytes = DLIST.GetAvailable();
    if( nBytes < (s32)(sizeof(header) + sizeof(char_info)) )
    {
        DLIST.Flush();
        nBytes = DLIST.GetAvailable();
    }
    ASSERT( nBytes >= (s32)(sizeof(header) + sizeof(char_info)) );

    // figure out how many characters we can allocate for, figuring
    // the size of the header into the equation
    nBytes -= sizeof(header);
    s_NCharsAlloced = nBytes / sizeof(char_info);

    // allocate the space for the header and characters
    byte* pD = (byte*)DLIST.Alloc( sizeof(header) + s_NCharsAlloced*sizeof(char_info) );
    s_pHeader = (header*)pD;
    s_pChar   = (char_info*)(pD + sizeof(header));

    // and we've added no characters yet
    s_NChars = 0;
}

static
void ps2_EndCharBatch( void )
{
    // fill in the header
    s_pHeader->DMA.SetCont( sizeof(header) - sizeof(dmatag) + (s_NChars * sizeof(char_info)) );
    s_pHeader->DMA.MakeDirect();
    s_pHeader->PGIF      = s_PGIF;
    s_pHeader->GIF       = s_GIF;
    s_pHeader->GIF.NLOOP = s_NChars; 
    s_pHeader->Prim = SCE_GS_SET_PRIM(
        GIF_PRIM_TRIANGLESTRIP,    // type of primitive
        1,    // shading method (flat, gouraud)
        1,    // texture mapping (off, on)
        0,    // fogging (off, on)
        1,    // alpha blending (off, on)
        0,    // 1 pass anti-aliasing (off, on)
        1,    // tex-coord spec method (STQ, UV)
        0,    // context (1 or 2)
        0 );  // fragment value control (normal, fixed)

    // we may not have used all of the space available, so give back what
    // we can to the display list
    DLIST.Dealloc( (s_NCharsAlloced - s_NChars)*sizeof(char_info) );
}

#endif


//=========================================================================
//  Font
//=========================================================================

xbool ui_font::Load( const char* pPathName )
{
    xbitmap *pBitmap;

    xstring	FontName;

    // Make file name
    FontName = pPathName;
    FontName = FontName.Left(FontName.Find(".xbmp")) + ".font";

    // load the map
    X_FILE* pFontFile = x_fopen(g_RscMgr.FixupFilename(FontName), "rb");

    ASSERTS( pFontFile, xfs("ui_font::Load() failed %s", (const char*)FontName));

    if( pFontFile )
    {
        u16 num;

        // read the number of character map entries
        x_fread(&num, sizeof(u16), 1, pFontFile);
        m_CMapSize = num;
        ASSERT( m_CMapSize > 0 );

        // read the number of characters
        x_fread(&num, sizeof(u16), 1, pFontFile);
        m_NumChars = num;
        ASSERT( m_NumChars > 0 );

        // read font line height
        x_fread(&num, sizeof(u16), 1, pFontFile);
        m_Height = num;
        ASSERT(m_Height > 0);

        // allocate memory for the char list and registration data.
        m_CMap = (charmap*)x_malloc(sizeof(charmap) * m_CMapSize);
        ASSERT( m_CMap );
        x_memset( m_CMap, 0, sizeof(charmap) * m_CMapSize );

        m_Characters = (Character*)x_malloc( sizeof(Character) * m_NumChars );
        ASSERT( m_Characters );
        x_memset( m_Characters, 0, sizeof(Character) * m_NumChars );

        // and read it in
        s32 count = x_fread(m_CMap, sizeof(charmap), m_CMapSize, pFontFile);
        ASSERT( count == m_CMapSize );

        count = x_fread(m_Characters, sizeof(Character), m_NumChars, pFontFile);
        ASSERT( count == m_NumChars );

        x_fclose( pFontFile );
    }

    // Load font image
    m_Bitmap.SetName( pPathName );
    pBitmap = m_Bitmap.GetPointer();
    ASSERTS( pBitmap, xfs("ui_font::Load() failed %s",pPathName));

    // Get info from bitmap size
    m_BmWidth   = pBitmap->GetWidth();
    m_BmHeight  = pBitmap->GetHeight();

    // Set AvgWidth
    m_AvgWidth = m_Characters[ LookUpCharacter('x') ].W;

    return TRUE;
}

//=========================================================================

void ui_font::Kill( void )
{
    x_free( m_CMap );
    x_free( m_Characters );
    m_Bitmap.Destroy(); // Make sure this guy actually unloads
}

//=========================================================================

void ui_font::TextSize( irect& Rect, const char* pString, s32 Count ) const
{
    s32 Height    = m_Height;
    s32 BestWidth = 0;
    s32 Width     = 0;
    f32 ScaleX=1;
    f32 ScaleY=1;

    if( ScaleText )
    {
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        ScaleX = (f32)XRes / 512.0f;
        ScaleY = (f32)YRes / 448.0f;
    }

    ASSERT( pString );

    if( pString )
    {
        // Loop until end of string or end of count.
        while( *pString && (Count != 0) )
        {
            s32 c = *pString++;

            // Check for newline.
            if( c == '\n' )
            {
                BestWidth = MAX( BestWidth, Width-1 );
                Width     = 0;
                Height   += m_Height;
            }
            else
            // Normal character.
            {
                // Add character to width.
                if( ScaleText )
                {
                    Width += (u32)((f32)m_Characters[ LookUpCharacter(c) ].W * ScaleX );
                }
                else
                {
                    Width += m_Characters[ LookUpCharacter(c) ].W;
                }
                Width += 1;
            }             

            // Decrease character count
            Count--;
        }

        BestWidth = MAX( BestWidth, Width-1 );
    }

    // We have all we need.
    Rect.Set( 0, 0, BestWidth, Height );
}

//=========================================================================

void ui_font::TextSize( irect& Rect, const xwchar* pString, s32 Count ) const
{
    s32 Height    = m_Height;
    s32 BestWidth = 0;
    s32 Width     = 0;
    f32 ScaleX=1;
    f32 ScaleY=1;

    if( ScaleText )
    {
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        ScaleX = (f32)XRes / 512.0f;
        ScaleY = (f32)YRes / 448.0f;
    }

    ASSERT( pString );

    if( pString )
    {
        // Loop until end of string or end of count.
        while( *pString && (Count != 0) )
        {
            s32 c = *pString++;

            // Check for embedded color code.
            if( (c & 0xFF00) == 0xFF00 )
            {
                // Skip 2nd character in embedded color code.
                pString++;

                // Decrease character count one extra for 2nd char in code.
                Count--;
            }
            else
            // Check for newline.
            if( c == '\n' )
            {
                BestWidth = MAX( BestWidth, Width-1 );
                Width     = 0;
                Height   += m_Height;
            }

            // Check for Control Code
            else if( c == 0xAB ) // '?'
            {
                c = *pString++;

                // If this is a ButtonIcon then Add Sprite Width
                s32 buttonCode = g_UiMgr->LookUpButtonCode( pString, 0 );

                if( buttonCode == CREDIT_TITLE_LINE || buttonCode == NEW_CREDIT_PAGE || buttonCode == CREDIT_END )
                    buttonCode = -1;

                if( buttonCode != -1 )
                {
#if defined(TARGET_XBOX)  
                    if( buttonCode == XBOX_BUTTON_START )
                    {
                        Width += BUTTON_START_SPRITE_WIDTH; 
                    }
                    else
                    {
                        Width += BUTTON_SPRITE_WIDTH; 
                    } 					
#elif defined(TARGET_PS2)
                    if( buttonCode == PS2_BUTTON_START )
                    {
                        Width += BUTTON_START_SPRITE_WIDTH; 
                    }
                    else
                    {
                        Width += BUTTON_SPRITE_WIDTH; 
                    }  
#elif defined(TARGET_PC)
                    if( buttonCode == PS2_BUTTON_START )
                    {
                        Width += BUTTON_START_SPRITE_WIDTH; 
                    }
                    else
                    {
                        Width += BUTTON_SPRITE_WIDTH; 
                    }  
#endif
                }

                // Loop past control code.
                while( c != 0xBB ) // '?'
                {
                    c = *pString++;
                }
            }
            else
            // Normal character.
            {
                if( ScaleText )
                {
                    Width += (u32)((f32)m_Characters[ LookUpCharacter(c) ].W * ScaleX );
                }
                else
                {
                    Width += m_Characters[ LookUpCharacter(c) ].W;
                }
                Width += 1;
            }             

            // Decrease character count
            Count--;
        }

        BestWidth = MAX( BestWidth, Width-1 );
    }

    // We have all we need.
    Rect.Set( 0, 0, BestWidth, Height );
}

//=========================================================================

s32 ui_font::TextWidth( const xwchar* pString, s32 Count ) const
{
    s32 BestWidth = 0;
    s32 Width     = 0;
    f32 ScaleX=1;
    f32 ScaleY=1;

    if( ScaleText )
    {
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        ScaleX = (f32)XRes / 512.0f;
        ScaleY = (f32)YRes / 448.0f;
    }

    ASSERT( pString );

    if( pString )
    {
        // Loop until end of string or end of count.
        while( *pString && (Count != 0) )
        {
            s32 c = *pString++;

            // Check for embedded color code.
            if( (c & 0xFF00) == 0xFF00 )
            {
                // Skip 2nd character in embedded color code.
                pString++;

                // Decrease character count one extra for 2nd char in code.
                Count--;
            }
            else
            // Check for newline.
            if( c == '\n' )
            {
                BestWidth = MAX( BestWidth, Width-1 );
                Width     = 0;
            }

            // Check for Control Code
            else if( c == 0xAB ) // '?'
            {

                // If this is a ButtonIcon then Add Sprite Width
                s32 ButtonCode = g_UiMgr->LookUpButtonCode( pString, 0 );

                if( ButtonCode == CREDIT_TITLE_LINE || ButtonCode == NEW_CREDIT_PAGE || ButtonCode == CREDIT_END )
                    ButtonCode = -1;

                if( ButtonCode != -1 )
                {
#if defined(TARGET_XBOX)  
                    if( ButtonCode == XBOX_BUTTON_START )
                    {
                        Width += BUTTON_START_SPRITE_WIDTH; 
                    }
                    else
                    {
                        Width += BUTTON_SPRITE_WIDTH; 
                    } 					
#elif defined(TARGET_PS2)
                    if( ButtonCode == PS2_BUTTON_START )
                    {
                        Width += BUTTON_START_SPRITE_WIDTH; 
                    }
                    else
                    {
                        Width += BUTTON_SPRITE_WIDTH; 
                    }  
#elif defined(TARGET_PC)
                    if( ButtonCode == PS2_BUTTON_START )
                    {
                        Width += BUTTON_START_SPRITE_WIDTH; 
                    }
                    else
                    {
                        Width += BUTTON_SPRITE_WIDTH; 
                    }  
#endif
                }

                // Loop past control code.
                while( c != 0xBB ) // '?'
                {
                    c = *pString++;
                }
            }

            else
            // Normal character.
            {
                // Add character to width
                if( ScaleText )
                {
                    Width += (u32)((f32)m_Characters[ LookUpCharacter(c) ].W * ScaleX );
                }
                else
                {
                    Width += m_Characters[ LookUpCharacter(c) ].W;
                }
                Width += 1;
            }             

            // Decrease character count.
            Count--;
        }

        BestWidth = MAX( BestWidth, Width-1 );
    }

    // Return best width.
    return( BestWidth );
}

//=========================================================================

s32 ui_font::TextHeight( const xwchar* pString, s32 Count ) const
{
    s32 Height = 0; 
    s32 ButtonHeight = 22;
    xbool LineHasButton = FALSE;

    ASSERT( pString );

    if( pString )
    {
        // Loop until end of string or end of count.
        while( pString && *pString && (Count != 0) )
        {
            s32 c = *pString++;

            // Check for embedded color code.
            if( (c & 0xFF00) == 0xFF00 )
            {
                // Skip 2nd character in embedded color code.
                pString++;

                // Decrease character count one extra for 2nd char in code.
                Count--;
            }
            // Check for Control Code
            else if( c == 0xAB ) // '?'
            {
                // We have found a button code so we need to make the height of this
                // string the height of the button bmps.
                 LineHasButton = TRUE;

                // Loop past control code.
                while( c != 0xBB ) // '?'
                {
                    c = *pString++;
                }
            }
            else
            // Check for newline.
            if( c == '\n' )
            {
                if( LineHasButton )
                    Height += ButtonHeight;
                else
                    Height += m_Height;

                LineHasButton = FALSE;
            }

            // Decrease character count.
            Count--;
        }
    }

    // Return height
    if( LineHasButton )
        Height += ButtonHeight;
    else
        Height += m_Height;

    return( Height );
}

//=========================================================================

u32 ui_font::LookUpCharacter(u32 c ) const
{
    #define UNDEFINED_CHARACTER (0x7F)

    if( c < 256 )
    {
        //ASSERTS((c < 0x10) || (m_CMap[c].character != 0), "character not in font.");
        // All our fonts should contain a character (a square) to designate an unsupported character.
        // in the event that even this character is not present, display an 'x'.
        // (that character MUST be present, as it's used for 'average width').
        if( !((c < 0x10) || (m_CMap[c].character != 0)) )
        {
            if( m_CMap[UNDEFINED_CHARACTER].character != 0 )
                c = UNDEFINED_CHARACTER;
            else
                c = 'x';
        }

        // c < 256 are direct mapped.
        return m_CMap[c].bitmap;
    }
    else
    {
        ASSERTS( m_CMapSize > 256, "No extended characters (>256) in font." );
        s32 imax = m_CMapSize;
        s32 imin = 256;
        xbool bFound = FALSE;

        while( imax >= imin )
        {
            s32 i = (imin + imax)/2;

            if( imax == imin + 1 )
            {
                if( m_CMap[i].character == c )
                    bFound = TRUE;
                else  //-- NOT FOUND
                    break;
            }

            if( m_CMap[i].character == c )
                bFound = TRUE;
            else if ( m_CMap[i].character > c )
                imax = i-1;
            else
                imin = i+1;

            if( bFound )
            {
                return( m_CMap[i].bitmap );    
            }
        }

        ASSERTS((0), "could not look up character");

        // return unknown character (see above)
        if( m_CMap[UNDEFINED_CHARACTER].character != 0 )
            return( m_CMap[UNDEFINED_CHARACTER].bitmap );
        else
            return( m_CMap['x'].bitmap );
    }
}


//=========================================================================
const ui_font::Character& ui_font::GetCharacter( s32 Index ) const
{

    return m_Characters[ LookUpCharacter(Index) ];
}

//=========================================================================
//  Return a clipped string with ellipsis that fits in the supplied rect
//=========================================================================

const xwchar* ui_font::ClipEllipsis( const xwchar* pString, const irect& Rect ) const
{
    static xwstring ClippedString;
    f32 ScaleX=1;
    f32 ScaleY=1;

    if( ScaleText )
    {
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        ScaleX = (f32)XRes / 512.0f;
        ScaleY = (f32)YRes / 448.0f;
    }

    // Should we be clipping?
    if( TextWidth( pString ) > Rect.GetWidth() )
    {
        s32     FieldWidth  = Rect.GetWidth() - 15;
        s32     Width       = 0;
        xbool   Clipping    = FALSE;

        // Clear the string
        ClippedString.Clear();

        // Clip the string
        while( *pString )
        {
            xwchar c = *pString++;

            // Check for embedded color code.
            if( (c & 0xFF00) == 0xFF00 )
            {
                // Copy into clipped string
                ClippedString += (xwchar)0xFF00;
                ClippedString += *pString++;
            }
            else
            // Check for newline.
            if( c == '\n' )
            {
                ClippedString  += (xwchar)'\n';
                Width           = 0;
                Clipping        = FALSE;
            }
            else
            // Normal character.
            {
                if( !Clipping )
                {
                    // Add character to width
                    if( ScaleText )
                    {
                        Width += (u32)((f32)m_Characters[ LookUpCharacter(c) ].W * ScaleX );
                    }
                    else
                    {
                        Width += m_Characters[ LookUpCharacter(c) ].W;
                    }
                    Width += 1;

                    // Width still in range?
                    if( Width < FieldWidth )
                    {
                        // Add to string
                        ClippedString += c;
                    }
                    else
                    {
                        // Add ellipsis
                        ClippedString += '.';
                        ClippedString += '.';
                        ClippedString += '.';
                        Clipping = TRUE;
                    }
                }
            }
        }

        return (const xwchar*)ClippedString;
    }
    else
    {
        return pString;
    }
}

//=========================================================================
void ui_font::TextWrap( const xwchar* pString, const irect& Rect, xwstring& WrappedString ) 
{
    const xwchar* pStart = pString;
    f32 ScaleX=1;
    f32 ScaleY=1;

    if( ScaleText )
    {
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        ScaleX = (f32)XRes / 512.0f;
        ScaleY = (f32)YRes / 448.0f;
    }

    // Should we be clipping?
    if( TextWidth( pString ) > Rect.GetWidth() )
    {
        s32     FieldWidth  = Rect.GetWidth() - 15;
        s32     Width       = 0;
        xbool   Clipping    = FALSE;

        // Clear the string
        WrappedString.Clear();

        // Wrap the string
        while( *pString )
        {
            xwchar c = *pString++;

            // Check for embedded color code.
            if( (c & 0xFF00) == 0xFF00 )
            {
                // Copy into Wrap string
                WrappedString += c;
                WrappedString += *pString++;
            }
            else
            // Check for newline.
            if( c == '\n' )
            {
                WrappedString  += (xwchar)'\n';
                Width           = 0;
                Clipping        = FALSE;
            }
            else if( c == 0xAB ) // '?'
            {
                // If this is a ButtonIcon then Add Sprite Width
                s32 ButtonCode = g_UiMgr->LookUpButtonCode( pString, 0 );

                if( ButtonCode == CREDIT_TITLE_LINE || ButtonCode == NEW_CREDIT_PAGE || ButtonCode == CREDIT_END )
                    ButtonCode = -1;

                if( ButtonCode != -1 )
                {
#if defined(TARGET_XBOX)  
                    if( ButtonCode == XBOX_BUTTON_START )
                    {
                        Width += BUTTON_START_SPRITE_WIDTH; 
                    }
                    else
                    {
                        Width += BUTTON_SPRITE_WIDTH; 
                    } 					
#elif defined(TARGET_PS2)
                    if( ButtonCode == PS2_BUTTON_START )
                    {
                        Width += BUTTON_START_SPRITE_WIDTH; 
                    }
                    else
                    {
                        Width += BUTTON_SPRITE_WIDTH; 
                    }  
#elif defined(TARGET_PC)
                    if( ButtonCode == PS2_BUTTON_START )
                    {
                        Width += BUTTON_START_SPRITE_WIDTH; 
                    }
                    else
                    {
                        Width += BUTTON_SPRITE_WIDTH; 
                    }  
#endif
                }
                
                WrappedString += c;
                
                // Loop past control code.
                while( c != 0xBB ) // '?'
                {                 
                    c = *pString++;
                    WrappedString += c;
                }
            }
            else
            // Normal character.
            {
                if( !Clipping )
                {
                    // Add character to width
                    if( ScaleText )
                    {
                        Width += (u32)((f32)m_Characters[ LookUpCharacter(c) ].W * ScaleX );
                    }
                    else
                    {
                        Width += m_Characters[ LookUpCharacter(c) ].W;
                    }
                    Width += 1;

                    // Width still in range?
                    if( Width < FieldWidth )
                    {
                        // Add to string
                        WrappedString += c;
                    }
                    else
                    {
                        // Over Size.. backup to a space and add a newLine.
                        c = *--pString;
                        s32 NumChars = 0;

                        while(  (c != ' ') && 
                                (c != '\n') && 
                                (pString > pStart) && 
                                (NumChars < 7)         ) 
                        {
                            c = *--pString;
                            NumChars++;
                        }

                        // Bingo, we backed up to a space!
                        if( c == ' ' )
                        {
                            WrappedString.Delete(WrappedString.GetLength()-NumChars,NumChars);
                            pString++;
                        }

                        // No such luck, just cleave the word in twain.
                        else
                        {
                            WrappedString += '-';
                            pString += NumChars;
                        }

                        Width            = 0;
                        WrappedString   += '\n';
                     //   Clipping = TRUE;
                    }
                }
            }
        }
    }
    else
    {
        // no wrapping required
        WrappedString = pString;
    }
}


//=========================================================================

void ui_font::RenderHelpText( const irect&  Rect, 
                                    u32     Flags, 
                              const xcolor& aColor, 
                              const xwchar* pString, 
                                    xbool   IgnoreEmbeddedColor,
                                    xbool   UseGradient, 
                                    f32     FlareAmount ) const
{
    (void)UseGradient;
    (void)FlareAmount;


    s32     c;
    s32     sx;
    s32     sy;
    s32     tx       = Rect.l;
    s32     ty       = Rect.t;
    s32     iStart   = 0;
    //s32     iEnd     = 0;
    s32     Width    = 0;
    s32     MaxWidth = 0;
    s32     CurrWidth = 0;
    s32     Height;    
    xcolor  Color    = aColor;
    xbool   bFirstButton = TRUE;
    f32 ScaleX=1;
    f32 ScaleY=1;
    xbitmap *pBitmap;

    ASSERT( pString );
    (void)IgnoreEmbeddedColor;

    // calculate scale factors
    if( ScaleText )
    {
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        ScaleX = (f32)XRes / 512.0f;
        ScaleY = (f32)YRes / 448.0f;
    }

    vector2 uv0;
    vector2 uv1;   
    vector2 Size( 0, (f32)m_Height );

    // Prepare to draw characters.
    draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_2D | DRAW_NO_ZBUFFER | DRAW_UV_CLAMP );

    pBitmap = m_Bitmap.GetPointer();
    draw_SetTexture( *pBitmap );

    // Get size for vertical positioning.
    Height = TextHeight( pString );

    // Position start vertically.
    if( Flags & v_center )
    {
        ty += (Rect.GetHeight() - Height + 4) / 2;
    }
    else if( Flags & v_bottom )
    {
        ty += (Rect.GetHeight() - Height);
    }


    if (Flags & h_left)
    {
        tx = Rect.l;
    }
    else
    {
        // Determine width of the help text
        while( pString[iStart] )
        {
            c = pString[iStart];

    
            if ((c & 0xff) == 0xAB ) // '?'
            {
                // get the button code
                s32 buttonCode = g_UiMgr->LookUpButtonCode( pString, iStart );

                while( (c & 0xff) != 0xBB ) // '?'
                {
                    iStart++;
                    c = pString[iStart];
                }

                if (CurrWidth > MaxWidth)
                {
                    Width   += CurrWidth;
                }
                else
                {
                    Width   += MaxWidth;
                }

                if (bFirstButton)
                {
                    bFirstButton = FALSE;
                }
                else
                {
                    Width   += HELP_TEXT_SPACING;
                }				
#if defined(TARGET_XBOX)  
                if( buttonCode == XBOX_BUTTON_START )
                {
                    Width   += BUTTON_START_SPRITE_WIDTH; 
                }
                else
				{
                    Width   += BUTTON_SPRITE_WIDTH;
				}				
#elif defined(TARGET_PS2)
                if( buttonCode == PS2_BUTTON_START )
                {
                    Width   += BUTTON_START_SPRITE_WIDTH;
                }
                else
                {
                    Width   += BUTTON_SPRITE_WIDTH;
                }  
#elif defined(TARGET_PC)
                if( buttonCode == PS2_BUTTON_START )
                {
                    Width   += BUTTON_START_SPRITE_WIDTH;
                }
                else
                {
                    Width   += BUTTON_SPRITE_WIDTH;
                }  
#endif
                CurrWidth    = 0;
                MaxWidth     = 0;
            }
            else if ( pString[iStart] == '\n' )
            {
                if (CurrWidth > MaxWidth)
                {
                    MaxWidth = CurrWidth;
                    CurrWidth = 0;
                }
            }
            else
            {
                // add the width of the character
                s32 w  = m_Characters[ LookUpCharacter(c) ].W;
                if( ScaleText )
                {
                    w = (u32)((f32)w * ScaleX);
                }
                CurrWidth += w + 1;
            }

            iStart++;
        }
        // add the width of the last word to the total
        if (CurrWidth > MaxWidth)
        {
            Width   += CurrWidth;
        }
        else
        {
            Width   += MaxWidth;
        }
        
        // Adjust lateral position for alignment flags.
        if( Flags & h_center )
        {
            tx = Rect.l + (Rect.GetWidth() - Width) / 2;
        }
        else if( Flags & h_right )
        {
            tx = Rect.l + (Rect.GetWidth() - Width);
        }
    }
   
    
    // initialize block start co-ords
    sx           = tx;
    sy           = ty;

    // reset string controls
    iStart       = 0;
    CurrWidth    = 0;
    MaxWidth     = 0;
    bFirstButton = TRUE;

    // Render text one item block at a time
    while( pString[iStart] )
    {
        c = pString[iStart];

        // Look for an embedded color code.
        //if( (c & 0xFF00) == 0xFF00 )
        //{
        //    if( IgnoreEmbeddedColor )
        //    {
        //        iStart++;
        //    }
        //    else
        //    {
        //        Color.R = (c & 0x00FF);
        //        iStart++;
        //        c = pString[iStart];
        //        Color.G = (c & 0xFF00) >> 8;
        //        Color.B = (c & 0x00FF);
        //    }
        //    continue;
        //}
        if( (c & 0xff) == 0xAB ) // '?' 
        {
            // Check for Command Codes
            iStart++;

            // get the button code
            s32 buttonCode = g_UiMgr->LookUpButtonCode( pString, iStart );

            if( buttonCode == CREDIT_TITLE_LINE || buttonCode == NEW_CREDIT_PAGE || buttonCode == CREDIT_END)
                buttonCode = -1;
            
            // If we found a button code then render it.
            if( buttonCode > -1 )
            {
                while( (c & 0xff) != 0xBB ) // '?'
                {
                    iStart++;
                    c = pString[iStart];
                }

                // calculate start pos of icon
                if (CurrWidth > MaxWidth)
                {
                    sx += CurrWidth;
                }
                else
                {
                    sx += MaxWidth;
                }

                // add spacing
                if (bFirstButton)
                {
                    bFirstButton = FALSE;
                }
                else
                {
                    sx += HELP_TEXT_SPACING;
                }

                // reset max block width
                MaxWidth  = 0;
                CurrWidth = 0;

                draw_End();
                draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_2D | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER );
                
                xbitmap* button = g_UiMgr->GetButtonTexture( buttonCode );
        		draw_SetTexture( *button );				
#if defined(TARGET_XBOX)
                if( buttonCode == XBOX_BUTTON_START )
                {
                    draw_Sprite( vector3((f32)sx+1, (f32)sy+1, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(0,0,0,255) );
                    draw_Sprite( vector3((f32)sx, (f32)sy, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(255,255,255) );

                    // set new block start
                    sx += BUTTON_START_SPRITE_WIDTH;
                }
                else
                {
                    draw_Sprite( vector3((f32)sx+1, (f32)sy+1, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(0,0,0,255) );
                    draw_Sprite( vector3((f32)sx, (f32)sy, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(255,255,255) );

                    // set new block start
                    sx += BUTTON_SPRITE_WIDTH;
                }
#elif defined(TARGET_PS2)
                if( buttonCode == PS2_BUTTON_START )
                {
                    draw_SpriteImmediate( vector2((f32)sx+1, (f32)sy+1),
                                          vector2(BUTTON_SPRITE_WIDTH*2, BUTTON_SPRITE_WIDTH),
                                          vector2(0.0f,0.0f),
                                          vector2(1.0f,1.0f),
                                          xcolor(0,0,0,255) );
                    draw_SpriteImmediate( vector2((f32)sx, (f32)sy),
                                          vector2(BUTTON_SPRITE_WIDTH * 2, BUTTON_SPRITE_WIDTH),
                                          vector2(0.0f,0.0f),
                                          vector2(1.0f,1.0f),
                                          xcolor(255,255,255) );
                    //draw_Sprite( vector3((f32)sx+1, (f32)sy+1, 0), vector2(BUTTON_SPRITE_WIDTH * 2, BUTTON_SPRITE_WIDTH), xcolor(0,0,0,255) );
                    //draw_Sprite( vector3((f32)sx, (f32)sy, 0), vector2(BUTTON_SPRITE_WIDTH * 2, BUTTON_SPRITE_WIDTH), xcolor(255,255,255) );

                    // set new block start
                    sx += BUTTON_START_SPRITE_WIDTH; 
                }
                else
                {
                    draw_SpriteImmediate( vector2((f32)sx+1, (f32)sy+1),
                                          vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH),
                                          vector2(0.0f,0.0f),
                                          vector2(1.0f,1.0f),
                                          xcolor(0,0,0,255) );
                    draw_SpriteImmediate( vector2((f32)sx, (f32)sy),
                                          vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH),
                                          vector2(0.0f,0.0f),
                                          vector2(1.0f,1.0f),
                                          xcolor(255,255,255) );
                    //draw_Sprite( vector3((f32)sx+1, (f32)sy+1, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(0,0,0,255) );
                    //draw_Sprite( vector3((f32)sx, (f32)sy, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(255,255,255) );

                    // set new block start
                    sx += BUTTON_SPRITE_WIDTH;
                }
#elif defined(TARGET_PC)
                if( buttonCode == PS2_BUTTON_START )
                {
                    draw_Sprite( vector3((f32)sx+1, (f32)sy+1, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(0,0,0,255) );
                    draw_Sprite( vector3((f32)sx, (f32)sy, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(255,255,255) );

                    // set new block start
                    sx += BUTTON_START_SPRITE_WIDTH;
                }
                else
                {
                    draw_Sprite( vector3((f32)sx+1, (f32)sy+1, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(0,0,0,255) );
                    draw_Sprite( vector3((f32)sx, (f32)sy, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(255,255,255) );

                    // set new block start
                    sx += BUTTON_SPRITE_WIDTH;
                }
#endif
                tx = sx;
                ty = sy;

                draw_End();

                draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_2D | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER );
		        
                pBitmap = m_Bitmap.GetPointer();
                draw_SetTexture( *pBitmap );
            }
        }
        else if ( pString[iStart] == '\n' )
        {
            // found a new line
            tx = sx;
            ty += m_Height;

            if (CurrWidth > MaxWidth)
            {
                MaxWidth = CurrWidth;
                CurrWidth = 0;
            }
        }
        else
        {
            // render the character
            s32 ci = LookUpCharacter(c);
            s32 x  = m_Characters[ ci ].X;
            s32 y  = m_Characters[ ci ].Y;
            s32 w  = m_Characters[ ci ].W;

            f32 u0 = (x            + 0.5f) / m_BmWidth;
            f32 u1 = (x + w        + 0.5f) / m_BmWidth;
            f32 v0 = (y            + 0.5f) / m_BmHeight;
            f32 v1 = (y + m_Height + 0.5f) / m_BmHeight;

            Size.X = (f32)w;
            Size.Y = (f32)m_Height;

            // scale for resolution
            if( ScaleText )
            {
                Size.X *= ScaleX;
                Size.Y *= ScaleY;
                
                w = (u32)((f32)w * ScaleX);
            }
            // end scale

            uv0.Set( u0, v0 );
            uv1.Set( u1, v1 );
 
            #ifdef TARGET_PS2
            draw_SpriteImmediate( vector2((f32)tx, (f32)ty), Size, uv0, uv1, Color );
            #else
            draw_SpriteUV( vector3((f32)tx,(f32)ty,10.0f), Size, uv0, uv1, Color );
            #endif
        
            tx        += w + 1;
            CurrWidth += w + 1;
        }

        // get new character
        iStart++;
    }

    #if defined(TARGET_PC) || defined(TARGET_XBOX) || defined(TARGET_PS2) || defined(TARGET_GCN)
    draw_End();
    #endif
}
//=========================================================================

void ui_font::RenderText( const irect&  Rect, 
                                u32     Flags, 
                          const xcolor& aColor, 
                          const xwchar* pString, 
                                xbool   IgnoreEmbeddedColor,
                                xbool   UseGradient,
                                f32     FlareAmount ) const
{
    xwchar  c;
    s32     tx       = Rect.l;
    s32     ty       = Rect.t;
    s32     iStart   = 0;
    s32     iEnd     = 0;
    s32     Width;
    s32     Height;    
    s32     buttonCode = -1;
    xcolor  Color1    = aColor;
    xcolor  Color2    = aColor;
    xbool   WasEllipsisClipped = FALSE;

    const s32 MaxButtons = 10;
    s32 NumButtons = 0;
    s32 ButtonCodes [ MaxButtons ];
    f32 Button_X    [ MaxButtons ];
    f32 Button_Y    [ MaxButtons ];


    if( UseGradient )
    {
        if( (aColor.R + aColor.G + aColor.B) / 3.0f > 59.0f )
        {
            Color2.R = 255; //(255 + Color1.R) / 2;
            Color2.G = 255; //(255 + Color1.G) / 2;
            Color2.B = 255; //(255 + Color1.B) / 2;
            Color2.A = aColor.A;
        } 
        else
        {
            Color1.R = 0; //(2
            Color1.G = 0; //(2
            Color1.B = 0; //(2
            Color1.A = aColor.A;
        }


    }

    // Do the flare thing
    if( FlareAmount > 0.0f )
    {
        s32 BrightnessDelta = (s32)(FlareAmount * 75);

        Color1.R = (Color1.R + BrightnessDelta) > 255 ? 255 : Color1.R + BrightnessDelta;
        Color1.G = (Color1.G + BrightnessDelta) > 255 ? 255 : Color1.G + BrightnessDelta;
        Color1.B = (Color1.B + BrightnessDelta) > 255 ? 255 : Color1.B + BrightnessDelta;

        Color2.R = (Color2.R + BrightnessDelta) > 255 ? 255 : Color2.R + BrightnessDelta;
        Color2.G = (Color2.G + BrightnessDelta) > 255 ? 255 : Color2.G + BrightnessDelta;
        Color2.B = (Color2.B + BrightnessDelta) > 255 ? 255 : Color2.B + BrightnessDelta;
    }

    xbitmap* pBitmap =  m_Bitmap.GetPointer();
    
    f32 ScaleX=1;
    f32 ScaleY=1;

    if( ScaleText )
    {
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        ScaleX = (f32)XRes / 512.0f;
        ScaleY = (f32)YRes / 448.0f;
    }

    ASSERT( pString );

    if (Flags & ui_font::is_help_text)
    {
        RenderHelpText( Rect, Flags, aColor, pString, IgnoreEmbeddedColor, UseGradient, FlareAmount );
        return;
    }

    #if defined TARGET_PC || defined TARGET_XBOX
        vector2 uv0;
        vector2 uv1;   
        vector2 Size( 0, (f32)m_Height );
//        f32     BmWidth  = 1.0f / (f32)m_BmWidth;
//        f32     BmHeight = 1.0f / (f32)m_BmHeight;

        // Prepare to draw characters.
        s32 DrawFlags = DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_2D | DRAW_NO_ZBUFFER | DRAW_NO_ZWRITE | DRAW_XBOX_WRITE_A | DRAW_UV_CLAMP | DRAW_CULL_NONE;
        if( Flags & ui_font::blend_additive )
            DrawFlags |= DRAW_BLEND_ADD;
        draw_Begin( DRAW_TRIANGLES, DrawFlags );
        draw_SetTexture( *pBitmap );

        // Turn off BILINEAR.
//        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
//        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
//        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_POINT );
    #endif

    #ifdef TARGET_PS2
        // Setup Texture
        //vram_Activate( m_Bitmap );

        vram_Activate( *pBitmap );

        s32 B = (Flags & ui_font::blend_additive) ? C_ZERO : C_DST;
        gsreg_Begin( 5 );
        gsreg_SetClamping( TRUE );
        gsreg_SetMipEquation( FALSE, 1.0f, 0, MIP_MAG_POINT, MIP_MIN_POINT );
        gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC, B, A_SRC, C_DST) );
        gsreg_SetZBufferTest( ZBUFFER_TEST_ALWAYS );
        gsreg_SetZBufferUpdate( FALSE );
        gsreg_End();

        // Build GIF Tags
        s_PGIF.Build( GIF_MODE_REGLIST, 2, 1, 0, 0, 0, 1 );
        s_PGIF.Reg  ( GIF_REG_PRIM, GIF_REG_NOP );
        s_GIF.Build ( GIF_MODE_REGLIST, 10, 0, 0, 0, 0, 1 );
        s_GIF.Reg   ( GIF_REG_RGBAQ, GIF_REG_UV, GIF_REG_XYZ3, GIF_REG_UV, GIF_REG_XYZ3, 
                        GIF_REG_RGBAQ, GIF_REG_UV, GIF_REG_XYZ2, GIF_REG_UV, GIF_REG_XYZ2  );

        //draw_DisableBilinear(); // location??? TODO: ctetrick
        // start a character batch
        ps2_NewCharBatch();
    #endif

    // Get size for vertical positioning.
    Height = TextHeight( pString );

    // Position start vertically.
    if( Flags & v_center )
    {
        ty += (Rect.GetHeight() - Height + 4) / 2;
    }
    else if( Flags & v_bottom )
    {
        ty += (Rect.GetHeight() - Height);
    }

    // Check for clipping with ellipsis
    if( Flags & clip_ellipsis )
    {
        const xwchar* pNewString = ClipEllipsis( pString, Rect );
        if( pNewString != pString )
        {
            pString = pNewString;
            WasEllipsisClipped = TRUE;
        }
    }

    // Render strips of text on same line.
    while( pString[iStart] )
    {
        if( pString[iStart] == '\n' )
        {
            iEnd = iStart;
        }
        else
        {
            // Find end of line.
            iEnd = iStart+1;
            while( pString[iEnd] && (pString[iEnd] != '\n') )
                iEnd++;
        }

        // Determine width of line.
        Width = TextWidth( &pString[iStart], iEnd-iStart );

        // Adjust lateral position for alignment flags.
        if( Flags & h_center )
        {
            tx = Rect.l + (Rect.GetWidth() - Width) / 2;
        }
        else if( Flags & h_right )
        {
            tx = Rect.l + (Rect.GetWidth() - Width);
        }
        else
        {
            tx = Rect.l;
        }

        // Check for justification when clipping.
        if( (Width > Rect.GetWidth()) || WasEllipsisClipped )
        {
            if( Flags & clip_l_justify ) 
                tx = Rect.l;
            else
            if( Flags & clip_r_justify ) 
                tx = Rect.r - Width;
        }

        //
        // Render each character.
        //
        for( ; iStart < iEnd; iStart++ )
        {
            c = pString[ iStart ];            
            //
            // Button code stuff.

            // Check for Command Codes
            if( c == 0x00AB )
            {
                iStart++;

                if( pString[ iStart ] == 0 ) continue; // shouldn't happen, but much safer.

                // get the button code
                buttonCode = g_UiMgr->LookUpButtonCode( pString, iStart );

                if( buttonCode == CREDIT_TITLE_LINE || buttonCode == NEW_CREDIT_PAGE || buttonCode == CREDIT_END )
                    buttonCode = -1;

                while( c && (c != 0x00BB) ) // '?'
                {
                    iStart++;
                    c = pString[iStart];
                }

                
                // If we found a button code then render it.
                if( buttonCode > -1 )
                {
                    while( c && (c != 0x00BB) ) // '?'
                    {
                        iStart++;
                        c = pString[iStart];
                    }

                    if( NumButtons >= MaxButtons )
                    {
                        ASSERTS( FALSE, "Too many buttons in this string!" );
                        continue;
                    }
                    //draw_End();
                    //draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_2D | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER );
                    
                    //xbitmap* button = g_UiMgr->GetButtonTexture( buttonCode );
        		    //draw_SetTexture( *button );

                    ButtonCodes[ NumButtons ] = buttonCode;
                    Button_X[ NumButtons ] = (f32)(tx);
	        	    Button_Y[ NumButtons ] = (f32)(ty);

                    //draw_Sprite( vector3((f32)tx+1, (f32)ty+1, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(0,0,0,255) );
	        	    //draw_Sprite( vector3((f32)tx, (f32)ty, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(255,255,255) );
#if defined(TARGET_XBOX)  
                if( buttonCode == XBOX_BUTTON_START )
                {
                    tx += BUTTON_SPRITE_WIDTH; 
                }
                else
				{
                    tx += BUTTON_SPRITE_WIDTH;
				}				
#elif defined(TARGET_PS2)
                if( buttonCode == PS2_BUTTON_START )
                {
                    tx += BUTTON_SPRITE_WIDTH;
                }
                else
                {
                    tx += BUTTON_START_SPRITE_WIDTH;
                }  
#elif defined(TARGET_PC)
                if( buttonCode == PS2_BUTTON_START )
                {
                    tx += BUTTON_SPRITE_WIDTH;
                }
                else
                {
                    tx += BUTTON_START_SPRITE_WIDTH;
                }  
#endif               
                    //draw_End();
                    
                    NumButtons++;

                    //draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_2D | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER );
		            
                    //xbitmap* pBitmap = m_Bitmap.GetPointer();
                    //draw_SetTexture( *pBitmap );
                }
                continue;
            }

            // Look for an embedded color code.
            else if( c && ((c & 0xFF00) == 0xFF00) )
            {
                if( IgnoreEmbeddedColor )
                {
                    iStart++;
                }
                else
                {
                    Color1.R = (c & 0x00FF);
                    iStart++;
                    c = pString[iStart];
                    Color1.G = (c & 0xFF00) >> 8;
                    Color1.B = (c & 0x00FF);
                }
                continue;
            } 

            // if we got the the end of the line, bail.
            if( c == 0 ) break;

            //
            // We have a normal character if we've made it this far
            //
            s32 ci = LookUpCharacter(c);
            s32 x  = m_Characters[ ci ].X;
            s32 y  = m_Characters[ ci ].Y;
            s32 w  = m_Characters[ ci ].W;

            //Color.A = 255;

            #if defined TARGET_PC || defined TARGET_XBOX
            {
                f32 u0 = (x            + 0.5f) / m_BmWidth;
                f32 u1 = (x + w        + 0.5f) / m_BmWidth;
                f32 v0 = (y            + 0.5f) / m_BmHeight;
                f32 v1 = (y + m_Height + 0.5f) / m_BmHeight;

                if( ScaleText )
                {
                    w = (u32)((f32)w * ScaleX);
                }
                Size.X = (f32)w;
                Size.Y = (f32)m_Height;
                uv0.Set( u0, v0 );
                uv1.Set( u1, v1 );

                draw_Color( Color2 );
                draw_UV( uv0.X, uv0.Y );
                draw_Vertex( tx, ty, 0.0f );
                draw_UV( uv1.X, uv0.Y );
                draw_Vertex( tx+w, ty, 0.0f );
                draw_Color( Color1 );
                draw_UV( uv0.X, uv1.Y );
                draw_Vertex( tx, ty+m_Height, 0.0f );

                draw_Vertex( tx, ty+m_Height, 0.0f );
                draw_UV( uv1.X, uv1.Y );
                draw_Vertex( tx+w, ty+m_Height, 0.0f );
                draw_Color( Color2 );
                draw_UV( uv1.X, uv0.Y );
                draw_Vertex( tx+w, ty, 0.0f );
                
            }
            #endif
            
            #ifdef TARGET_PS2
            {
                // Text is often rendered in it's entirety with a clipping region set. 
                // problem is on PS2, the EULA text is very large (thanks again lawyers!)
                // the rendering region for this text exceeds the world coordinates and wraps.
                // so for this problem we'll skip any characters that go offscreen. 
                // This is a limited case, so button codes are not considered.
                s32 XRes, YRes;
                eng_GetRes( XRes, YRes );

                if( ((tx > 0) && ((tx + w) < XRes)) && ((ty > 0) && ((ty + m_Height) < YRes)))
                {    
	                s32         X0,Y0,X1,Y1;
                    char_info*  pCH = s_pChar;

	                X0 = (OFFSET_X<<4) + ((tx)<<4);
	                Y0 = (OFFSET_Y<<4) + ((ty)<<4);
	                X1 = (OFFSET_X<<4) + ((tx+w)<<4);
	                Y1 = (OFFSET_Y<<4) + ((ty+m_Height)<<4);

                    pCH->Color1 = SCE_GS_SET_RGBAQ( Color2.R>>1, Color2.G>>1, Color2.B>>1, Color2.A>>1, 0x3F800000 );	                
                    pCH->T0     = SCE_GS_SET_UV( (x<<4)+8, (y<<4)+8 );
	                pCH->P0     = SCE_GS_SET_XYZ(X0,Y0,0xFFFFFFFF);
    	            
                    pCH->T1     = SCE_GS_SET_UV( ((x+w)<<4)+8, (y<<4)+8 );
	                pCH->P1     = SCE_GS_SET_XYZ(X1,Y0,0xFFFFFFFF);

	                pCH->Color2 = SCE_GS_SET_RGBAQ( Color1.R>>1, Color1.G>>1, Color1.B>>1, Color1.A>>1, 0x3F800000 );	                
                    pCH->T2     = SCE_GS_SET_UV( (x<<4)+8, ((y+m_Height)<<4)+8 );
	                pCH->P2     = SCE_GS_SET_XYZ(X0,Y1,0xFFFFFFFF);
    	            
                    pCH->T3     = SCE_GS_SET_UV( ((x+w)<<4)+8, ((y+m_Height)<<4)+8 );
	                pCH->P3     = SCE_GS_SET_XYZ(X1,Y1,0xFFFFFFFF);

                    s_pChar++;
                    s_NChars++;
                    if( s_NChars == s_NCharsAlloced )
                    {
                        // start a new batch
                        ps2_EndCharBatch();
                        ps2_NewCharBatch();
                    }
                }
            }
            #endif

            tx += w + 1;
        }

        // Process newline.
        if( pString[iStart] == '\n' )
        {
            ty += m_Height;
            iStart++;
        }
    }

    #if defined TARGET_PC || defined TARGET_XBOX
    draw_End();
    #endif

    #ifdef TARGET_PS2
    // finish off any pending batches
    ps2_EndCharBatch();
    #endif

#if !defined(APP_EDITOR)
    if( NumButtons > 0 && UseGradient )
    {
        // now draw the buttons
        draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_2D | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER );

        s32 i;
        for( i = 0; i < NumButtons; i++ )
        {
                        
            xbitmap* button = g_UiMgr->GetButtonTexture( ButtonCodes[ i ] );
            draw_SetTexture( *button );

            #ifdef TARGET_PS2
            if( buttonCode == PS2_BUTTON_START )
            {
                draw_SpriteImmediate( vector2(Button_X[i]+1, Button_Y[i]+1),
                    vector2(BUTTON_SPRITE_WIDTH * 2, BUTTON_SPRITE_WIDTH),
                    vector2(0.0f,0.0f),
                    vector2(1.0f,1.0f),
                    xcolor(0,0,0,255) );
                draw_SpriteImmediate( vector2(Button_X[i], Button_Y[i]),
                    vector2(BUTTON_SPRITE_WIDTH * 2, BUTTON_SPRITE_WIDTH),
                    vector2(0.0f,0.0f),
                    vector2(1.0f,1.0f),
                    xcolor(255,255,255,255) );
            }
            else
            {
                draw_SpriteImmediate( vector2(Button_X[i]+1, Button_Y[i]+1),
                    vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH),
                    vector2(0.0f,0.0f),
                    vector2(1.0f,1.0f),
                    xcolor(0,0,0,255) );
                draw_SpriteImmediate( vector2(Button_X[i], Button_Y[i]),
                    vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH),
                    vector2(0.0f,0.0f),
                    vector2(1.0f,1.0f),
                    xcolor(255,255,255,255) );
            }
            #else
            draw_Sprite( vector3(Button_X[ i ]+1, Button_Y[ i ]+1, 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(0,0,0,255) );
	        draw_Sprite( vector3(Button_X[ i ]  , Button_Y[ i ]  , 0), vector2(BUTTON_SPRITE_WIDTH, BUTTON_SPRITE_WIDTH), xcolor(255,255,255) );           
            #endif
        }
        draw_End();
    }
#endif
}
//=========================================================================

void ui_font::RenderText( const irect&  Rect, 
                                u32     Flags, 
                                s32     Alpha, 
                          const xwchar* pString,
                                xbool   IgnoreEmbeddedColor,
                                xbool   UseGradient, 
                                f32     FlareAmount ) const
{
    xcolor Color = XCOLOR_PURPLE;
    Color.A = Alpha;

    if (Flags & ui_font::is_help_text)
    {
        RenderHelpText( Rect, Flags, Color, pString, IgnoreEmbeddedColor, UseGradient, FlareAmount );
    }
    else
    {
        RenderText( Rect, Flags, Color, pString, IgnoreEmbeddedColor, UseGradient, FlareAmount );
    }
}

//=========================================================================

void ui_font::RenderText( const irect&  R, 
                                u32     Flags, 
                          const xcolor& Color, 
                          const char*   pString,
                                xbool   IgnoreEmbeddedColor,
                                xbool   UseGradient,
                                f32     FlareAmount 
                          ) const
{
    xwstring t( pString );

    if (Flags & ui_font::is_help_text)
    {
        RenderHelpText( R, Flags, Color, (const xwchar*)t, IgnoreEmbeddedColor, UseGradient, FlareAmount );
    }
    else
    {
        RenderText( R, Flags, Color, (const xwchar*)t, IgnoreEmbeddedColor, UseGradient, FlareAmount );
    }
}

//=========================================================================

void ui_font::RenderStateControlledText( const irect& Rect, u32 Flags, const xcolor& Color, const xwchar* pString, void* StateData) const
{
    xwchar  c;
    s32     tx       = Rect.l;
    s32     ty       = Rect.t;
    s32     iStart   = 0;
    s32     iEnd     = 0;
    s32     Width;
    s32     Height;    
    s32     buttonCode = -1;
    xcolor  Color1    = Color;
    //xcolor  Color2    = Color;
    xbool   WasEllipsisClipped = FALSE;

    xbitmap* pBitmap =  m_Bitmap.GetPointer();

    ASSERT( pString );


#if defined TARGET_PC || defined TARGET_XBOX
    vector2 uv0;
    vector2 uv1;   
    vector2 Size( 0, (f32)m_Height );
    f32 ScaleX=1;
    f32 ScaleY=1;

    // Prepare to draw characters.
    s32 DrawFlags = DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_2D | DRAW_NO_ZBUFFER | DRAW_NO_ZWRITE | DRAW_XBOX_WRITE_A | DRAW_UV_CLAMP;
    if( Flags & ui_font::blend_additive )
        DrawFlags |= DRAW_BLEND_ADD;
    draw_Begin( DRAW_SPRITES, DrawFlags );
    draw_SetTexture( *pBitmap );
#endif

#ifdef TARGET_PS2
    // Setup Texture
    vram_Activate( *pBitmap );

    s32 B = (Flags & ui_font::blend_additive) ? C_ZERO : C_DST;
    gsreg_Begin( 5 );
    gsreg_SetClamping( TRUE );
    gsreg_SetMipEquation( FALSE, 1.0f, 0, MIP_MAG_POINT, MIP_MIN_POINT );
    gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC, B, A_SRC, C_DST) );
    gsreg_SetZBufferTest( ZBUFFER_TEST_ALWAYS );
    gsreg_SetZBufferUpdate( FALSE );
    gsreg_End();

    // Build GIF Tags
    s_PGIF.Build( GIF_MODE_REGLIST, 2, 1, 0, 0, 0, 1 );
    s_PGIF.Reg  ( GIF_REG_PRIM, GIF_REG_NOP );
    s_GIF.Build ( GIF_MODE_REGLIST, 10, 0, 0, 0, 0, 1 );
    s_GIF.Reg   ( GIF_REG_RGBAQ, GIF_REG_UV, GIF_REG_XYZ3, GIF_REG_UV, GIF_REG_XYZ3, 
        GIF_REG_RGBAQ, GIF_REG_UV, GIF_REG_XYZ2, GIF_REG_UV, GIF_REG_XYZ2  );

    // start a character batch
    ps2_NewCharBatch();
#endif

    // Get size for vertical positioning.
    Height = TextHeight( pString );

    // Position start vertically.
    if( Flags & v_center )
    {
        ty += (Rect.GetHeight() - Height + 4) / 2;
    }
    else if( Flags & v_bottom )
    {
        ty += (Rect.GetHeight() - Height);
    }

    // Check for clipping with ellipsis
    if( Flags & clip_ellipsis )
    {
        const xwchar* pNewString = ClipEllipsis( pString, Rect );
        if( pNewString != pString )
        {
            pString = pNewString;
            WasEllipsisClipped = TRUE;
        }
    }

    // Render strips of text on same line.
    while( pString[iStart] )
    {
        if( pString[iStart] == '\n' )
        {
            iEnd = iStart;
        }
        else
        {
            // Find end of line.
            iEnd = iStart+1;
            while( pString[iEnd] && (pString[iEnd] != '\n') )
                iEnd++;
        }

        // Determine width of line.
        Width = TextWidth( &pString[iStart], iEnd-iStart );

        // Adjust lateral position for alignment flags.
        if( Flags & h_center )
        {
            tx = Rect.l + (Rect.GetWidth() - Width) / 2;
        }
        else if( Flags & h_right )
        {
            tx = Rect.l + (Rect.GetWidth() - Width);
        }
        else
        {
            tx = Rect.l;
        }

        // Check for justification when clipping.
        if( (Width > Rect.GetWidth()) || WasEllipsisClipped )
        {
            if( Flags & clip_l_justify ) 
                tx = Rect.l;
            else
                if( Flags & clip_r_justify ) 
                    tx = Rect.r - Width;
        }

        //
        // Render each character.
        //
        for( ; iStart < iEnd; iStart++ )
        {
            c = pString[ iStart ];            
            //
            // Button code stuff.

            // Check for Command Codes
            if( c == 0x00AB )
            {
                iStart++;

                if( pString[ iStart ] == 0 ) continue; // shouldn't happen, but much safer.

                // get the button code
                buttonCode = g_UiMgr->LookUpButtonCode( pString, iStart );

                if( buttonCode == CREDIT_TITLE_LINE || buttonCode == NEW_CREDIT_PAGE || buttonCode == CREDIT_END )
                    buttonCode = -1;

                while( c && (c != 0x00BB) ) // '?'
                {
                    iStart++;
                    c = pString[iStart];
                }

                // If we found a button code then render it.
                if( buttonCode > -1 )
                {
                    // Buttons not supported
                    ASSERT(0);
                }
                continue;
            }

            // Color Codes get thrown out.
            else if( c && ((c & 0xFF00) == 0xFF00) )
            {                
                iStart++;
                continue;
            } 

            // if we got the the end of the line, bail.
            if( c == 0 ) break;

            //
            // We have a normal character if we've made it this far
            //
            s32 ci = LookUpCharacter(c);
            s32 x  = m_Characters[ ci ].X;
            s32 y  = m_Characters[ ci ].Y;
            s32 w  = m_Characters[ ci ].W;

#if defined TARGET_PC || defined TARGET_XBOX
            {
                if( (((CustomRenderStruct*)StateData)[iStart]).m_State == s_render )
                {                
                    f32 u0 = (x            + 0.5f) / m_BmWidth;
                    f32 u1 = (x + w        + 0.5f) / m_BmWidth;
                    f32 v0 = (y            + 0.5f) / m_BmHeight;
                    f32 v1 = (y + m_Height + 0.5f) / m_BmHeight;

                    if( ScaleText )
                    {
                        w = (u32)((f32)w * ScaleX);
                    }
                    Size.X = (f32)w;
                    Size.Y = (f32)m_Height;
                    uv0.Set( u0, v0 );
                    uv1.Set( u1, v1 );

                    xcolor RenderColor = Color1;
                    RenderColor.A = (u8)(((CustomRenderStruct*)StateData)[iStart]).m_Value;

                    // aharp TODO need to add gradient font
                    draw_SpriteUV( vector3((f32)tx,(f32)ty,10.0f), Size, uv0, uv1, RenderColor );
                }
            }
#endif

#ifdef TARGET_PS2
            {
                if( ((CustomRenderStruct*)StateData)->m_State == s_render )
                {
                    s32         X0,Y0,X1,Y1;
                    char_info*  pCH = s_pChar;

                    X0 = (OFFSET_X<<4) + ((tx)<<4);
                    Y0 = (OFFSET_Y<<4) + ((ty)<<4);
                    X1 = (OFFSET_X<<4) + ((tx+w)<<4);
                    Y1 = (OFFSET_Y<<4) + ((ty+m_Height)<<4);

                    xcolor RenderColor = Color1;
                    RenderColor.A = (u8)(((CustomRenderStruct*)StateData)[iStart]).m_Value;

                    pCH->Color1 = SCE_GS_SET_RGBAQ( RenderColor.R>>1, RenderColor.G>>1, RenderColor.B>>1, RenderColor.A>>1, 0x3F800000 );	                
                    pCH->T0     = SCE_GS_SET_UV( (x<<4)+8, (y<<4)+8 );
                    pCH->P0     = SCE_GS_SET_XYZ(X0,Y0,0xFFFFFFFF);

                    pCH->T1     = SCE_GS_SET_UV( ((x+w)<<4)+8, (y<<4)+8 );
                    pCH->P1     = SCE_GS_SET_XYZ(X1,Y0,0xFFFFFFFF);

                    pCH->Color2 = SCE_GS_SET_RGBAQ( RenderColor.R>>1, RenderColor.G>>1, RenderColor.B>>1, RenderColor.A>>1, 0x3F800000 );	                
                    pCH->T2     = SCE_GS_SET_UV( (x<<4)+8, ((y+m_Height)<<4)+8 );
                    pCH->P2     = SCE_GS_SET_XYZ(X0,Y1,0xFFFFFFFF);

                    pCH->T3     = SCE_GS_SET_UV( ((x+w)<<4)+8, ((y+m_Height)<<4)+8 );
                    pCH->P3     = SCE_GS_SET_XYZ(X1,Y1,0xFFFFFFFF);

                    s_pChar++;
                    s_NChars++;
                    if( s_NChars == s_NCharsAlloced )
                    {
                        // start a new batch
                        ps2_EndCharBatch();
                        ps2_NewCharBatch();
                    }
                }
            }
#endif

            tx += w + 1;
        }

        // Process newline.
        if( pString[iStart] == '\n' )
        {
            ty += m_Height;
            iStart++;
        }
    }

#if defined TARGET_PC || defined TARGET_XBOX
    draw_End();
#endif

#ifdef TARGET_PS2
    // finish off any pending batches
    ps2_EndCharBatch();
#endif
}
