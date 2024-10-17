//=========================================================================
//
// E_TEXT.CPP
//
// LineTouched holds farthest character written for each line
// -1 means no chars were written to line
//=========================================================================

#include "Entropy.hpp"

//=========================================================================

#if !defined( X_RETAIL ) || defined( X_QA )

//=========================================================================

static s32      s_Initialized=FALSE;

static char*    s_BufferMemory;
static char*    s_Buffer;
static char*    s_ScrollBuffer;
static char*    s_ScrollLine;
static s16*     s_LineTouched;
static s32      s_ScrollCursor;

static s32      s_ScreenWidth;
static s32      s_ScreenHeight;
static s32      s_XBorderWidth;
static s32      s_YBorderWidth;
static s32      s_CharWidth;
static s32      s_CharHeight;
static s32      s_NScrollLines;
static s32      s_ScrollBufferSize;

static s32      s_BufferWidth;
static s32      s_BufferHeight;
static s32      s_BufferSize;

static xbool    s_NewParameters;

static xbool    s_TextOff;

static xcolor s_ColorStack[8];
static s32    s_ColorStackIndex=0;

struct text_str
{
    s32 X;
    s32 Y;
    s32 Len;
    xcolor Color;
    text_str* pNext;
};

static text_str* s_TextStringFirst;

//=========================================================================

void text_Init( void )
{
    ASSERT( !s_Initialized );
    s_Initialized = TRUE;

    // Setup some default params but don't allocate buffers
    /*text_SetParams( 640, 480,
                      0,   0,
                      8,   8, 
                      8 );*/
    text_SetParams( 512, 512,
                      0,   0,
                      8,   8, 
                      8 );

    s_Initialized = TRUE;
    s_TextOff = FALSE;
    s_BufferMemory = NULL;
    s_TextStringFirst = NULL;
    s_ColorStackIndex = -1;

    x_SetPrintHook( text_Print );
    x_SetPrintAtHook( text_PrintXY );
}

//=========================================================================

void text_Kill( void )
{
    ASSERT( s_Initialized );
    s_Initialized = FALSE;
    x_free(s_BufferMemory);
    s_BufferMemory = NULL;
}

//=========================================================================

void text_Off( void )
{
    s_TextOff = TRUE;
}

//=========================================================================

void text_On( void )
{
    s_TextOff = FALSE;
}

//=========================================================================

void text_GetParams( s32& ScreenWidth,
                     s32& ScreenHeight,
                     s32& XBorderWidth,
                     s32& YBorderWidth,
                     s32& CharacterWidth,
                     s32& CharacterHeight,
                     s32& NScrollLines )
{
    ScreenWidth     = s_ScreenWidth;
    ScreenHeight    = s_ScreenHeight;
    XBorderWidth    = s_XBorderWidth; 
    YBorderWidth    = s_YBorderWidth; 
    CharacterWidth  = s_CharWidth;    
    CharacterHeight = s_CharHeight;   
    NScrollLines    = s_NScrollLines; 
}

//=========================================================================

void text_SetParams( s32 ScreenWidth,
                     s32 ScreenHeight,
                     s32 XBorderWidth,
                     s32 YBorderWidth,
                     s32 CharacterWidth,
                     s32 CharacterHeight,
                     s32 NScrollLines )
{
    s_ScreenWidth   = ScreenWidth;
    s_ScreenHeight  = ScreenHeight;
    s_XBorderWidth  = XBorderWidth;
    s_YBorderWidth  = YBorderWidth;
    s_CharWidth     = CharacterWidth;
    s_CharHeight    = CharacterHeight;
    s_NScrollLines  = NScrollLines;

    s_BufferWidth   = (s_ScreenWidth - (2*s_XBorderWidth)) / (s_CharWidth);
    s_BufferHeight  = (s_ScreenHeight - (2*s_YBorderWidth)) / (s_CharHeight);
    s_BufferSize    = s_BufferWidth * s_BufferHeight;
    s_ScrollBufferSize = s_BufferWidth * s_NScrollLines;

    s_NewParameters = TRUE;
}

//=========================================================================

void text_ClearBuffers( void )
{
    ASSERT( s_Initialized );

    // Clear buffers and return if text is off
    if( s_TextOff )
    {
        if( s_BufferMemory )
        {
            x_DebugMsg("TEXT: Freeing buffers\n");
            x_free(s_BufferMemory);
            s_BufferMemory = NULL;
        }
        return;
    }

    // Realloc buffers if they have changed
    if( s_NewParameters || (!s_BufferMemory) )
    {
        x_DebugMsg("TEXT: Allocating buffers\n");

        s32 Size = s_BufferSize + s_ScrollBufferSize + (2*s_BufferHeight);
        s_BufferMemory = (char*)x_realloc(s_BufferMemory,Size);
        ASSERT( s_BufferMemory );

        s_Buffer        = s_BufferMemory;
        s_ScrollBuffer  = s_Buffer + s_BufferSize;
        s_LineTouched   = (s16*)(s_ScrollBuffer + s_ScrollBufferSize);
        s_ScrollLine    = s_ScrollBuffer + s_ScrollBufferSize - s_BufferWidth;
        s_NewParameters = FALSE;
        x_memset(s_ScrollBuffer,0,s_ScrollBufferSize);  
    }


    // Clear the buffers
    x_memset(s_Buffer,0,s_BufferSize);
    x_memset(s_LineTouched,0xFF,2*s_BufferHeight);
    s_TextStringFirst = NULL;
}

//=========================================================================

void text_PtToCell( s32 SX, s32 SY, s32& CX, s32& CY )
{
    ASSERT( s_Initialized );
    CX = (SX - s_XBorderWidth) / s_CharWidth;
    CY = (SY - s_YBorderWidth) / s_CharHeight;
}

//=========================================================================

void text_CellToPt( s32 CX, s32 CY, s32& SX, s32& SY )
{
    ASSERT( s_Initialized );
    SX = (CX*s_CharWidth) + s_XBorderWidth;
    SY = (CY*s_CharHeight) + s_YBorderWidth;
}

//=========================================================================

static 
void ScrollTextOneLineUp( void )
{
    x_memmove( s_ScrollBuffer, 
               s_ScrollBuffer + s_BufferWidth,
               s_BufferWidth * (s_NScrollLines-1) );

    x_memset ( s_ScrollLine, 0, s_BufferWidth );

    s_ScrollCursor = 0;
}

//=========================================================================

void text_PrintPixelXY( const char* pStr, s32 PixelX, s32 PixelY )
{
    ASSERT( s_Initialized );
    if( s_TextOff ) return;
    if( s_NewParameters || (!s_BufferMemory) ) return;

    s32 Len  = x_strlen(pStr);
    s32 Size = sizeof(text_str) + Len + 1;
    text_str* pTS = (text_str*)smem_BufferAlloc(Size);
    if( !pTS )
        return;

    pTS->X = PixelX;
    pTS->Y = PixelY;
    pTS->Len = Len;
    pTS->Color = text_GetColor();
    pTS->pNext = s_TextStringFirst;
    s_TextStringFirst = pTS;

    x_strcpy((char*)(pTS+1),pStr);
}

//=========================================================================

void text_Print( const char* pStr )
{
    // Do asserts and confirm buffers are available
    ASSERT( s_Initialized );
    if( s_TextOff ) return;
    if( s_NewParameters || (!s_BufferMemory) ) return;

    if( pStr == NULL )
        return;

    // Add string to scroll text area.
    while( *pStr )
    {
        if( (*pStr) == 0x08 )
        {
            if( s_ScrollCursor == -1 )
                ScrollTextOneLineUp();

            // Expand the tab here.
            // Space to next multiple of 8 column.
            do
            {
                if( s_ScrollCursor < s_BufferWidth )
                    s_ScrollLine[s_ScrollCursor] = ' ';
                s_ScrollCursor++;
            } while( s_ScrollCursor & 0x07 );
        }
        else
        if( (*pStr) == 0x0A )
        {
            if( s_ScrollCursor == -1 )
                ScrollTextOneLineUp();

            // Handle line feed
            s_ScrollCursor = -1;
        }
        else
        if( (*pStr) == 0x0D )
        {
            // Handle carriage return
            // (Do nothing)
        }
        else
        if( (*pStr) == 0x20 )
        {
            if( s_ScrollCursor == -1 )
                ScrollTextOneLineUp();

            // Handle space
            if( s_ScrollCursor < s_BufferWidth )
                s_ScrollLine[s_ScrollCursor] = ' ';
            s_ScrollCursor++;
        }
        else
        {
            // Handle all other characters
            if( (s_ScrollCursor == -1) || (s_ScrollCursor>=s_BufferWidth) )
                ScrollTextOneLineUp();

            // Stick the character in the data buffer.
            s_ScrollLine[s_ScrollCursor] = *pStr;

            s_ScrollCursor++;
        }           

        pStr++;
    }

}

//=========================================================================

void text_PrintXY( const char* pStr, s32 X, s32 Y )
{
    // Do asserts and confirm buffers are available
    ASSERT( s_Initialized );
    if( s_TextOff ) return;
    if( s_NewParameters || (!s_BufferMemory) ) return;

    s32 OriginalX = X;

    if( pStr == NULL )
        return;

    // Too far right?  Don't even bother.
    if( X >= s_BufferWidth )
        return;

    // While there's still some string left AND we are not off the bottom.
    while( (*pStr) && (Y<s_BufferHeight) )
    {
        if( (*pStr) == 0x08 )
        {
            // Expand the tab here.
            // Space to next multiple of 8 column.
            do
            {
                if( (X<s_BufferWidth) && (X>=0) && (Y>=0) )
                {
                    s_Buffer[ (Y*(s_BufferWidth)) + X ] = ' ';
                    s_LineTouched[Y] = MAX(X,s_LineTouched[Y]);
                }
                X++;
            } while( X & 0x07 );
        }
        else
        if( *pStr == 0x0D )
        {
            // Carriage return, do nothing.
        }
        else
        if( *pStr == 0x0A )
        {
            // Line feed.
            Y++;
            X = OriginalX;
        }
        else
        {
            // All other characters.
            if( (X<s_BufferWidth) && (X>=0) && (Y>=0) )
            {
                s_Buffer[ (Y*(s_BufferWidth)) + X ] = *pStr;
                s_LineTouched[Y] = MAX(X,s_LineTouched[Y]);
            }
            X++;
        }

        pStr++;
    }
}

//=========================================================================

void text_Render( void )
{
    ASSERT( s_Initialized );
    if( s_TextOff ) return;
    if( s_NewParameters || (!s_BufferMemory) ) return;

    // "Stamp" the scroll text onto the small text buffer.
    {
        s32 DestOffset = s_BufferWidth * (s_BufferHeight-s_NScrollLines);
        s32 Characters = s_ScrollBufferSize;

        char*   s = s_ScrollBuffer;  
        char*   d = s_Buffer + DestOffset;

        while( Characters )
        {
            if( *s )    *d = *s;
            s++;
            d++;
            Characters--;
        }

        // Touch all lines that scroll region overlaps
        for( s32 Y=(s_BufferHeight-s_NScrollLines); Y<s_BufferHeight; Y++ )
            s_LineTouched[Y] = s_BufferWidth-1;
    }

    text_BeginRender();

    // Loop through buffer and start spitting out strings to hardware
    // renderer
    if( 1 )
    {
        xcolor Color = text_GetColor();


        for( s32 Y=0; Y<s_BufferHeight; Y++ )
        if( s_LineTouched[Y] >= 0 )
        {
            s32 X = 0;
            s32 StartX = -1;
            s32 ScreenX=0;
            s32 ScreenY=0;
            s32 Len=0;
            char* S = s_Buffer + (Y*s_BufferWidth);

            while( X <= s_LineTouched[Y] )
            {
                if( S[X] != 0 )
                {
                    if( StartX==-1 )
                    {
                        StartX  = X;
                        ScreenX = (X*s_CharWidth) + s_XBorderWidth;
                        ScreenY = (Y*s_CharHeight) + s_YBorderWidth;
                        Len     = 1;
                    }
                    else
                    {
                        Len++;
                    }
                }
                else
                if( StartX!=-1 )
                {
                    // Flush this string
                    text_RenderStr( &S[StartX], Len, Color, ScreenX, ScreenY );
                    StartX = -1;
                }

                X++;
            }

            // Flush if str leftover
            if( StartX!=-1 )
            {
                text_RenderStr( &S[StartX], Len, Color, ScreenX, ScreenY );
            }
        }

    }

    // Run through the linked list of pixel strings
    text_str* pTS = s_TextStringFirst;
    while( pTS != NULL )
    {
        text_RenderStr( (char*)(pTS+1), pTS->Len, pTS->Color, pTS->X, pTS->Y );
        pTS = pTS->pNext;
    }

    text_EndRender();
}

//=========================================================================

void text_PushColor( xcolor C )
{
    ASSERT( s_ColorStackIndex < 7 );
    s_ColorStackIndex++;
    s_ColorStack[s_ColorStackIndex] = C;
}

//=========================================================================

void text_PopColor( void )
{
    ASSERT( s_ColorStackIndex >= 0 );
    s_ColorStackIndex--;
}

//=========================================================================

xcolor text_GetColor( void )
{
    if( s_ColorStackIndex == -1 )
        return XCOLOR_WHITE;
    return s_ColorStack[s_ColorStackIndex];
}

//=========================================================================
#endif // !defined( X_RETAIL ) || defined( X_QA )
