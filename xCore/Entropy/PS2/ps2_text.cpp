//=========================================================================
//
// E_TEXT.CPP
//
//=========================================================================
#include "e_engine.hpp"

//=========================================================================

#define BUFFER_WIDTH    (38+1)
#define BUFFER_HEIGHT   23
#define BUFFER_SIZE     (BUFFER_WIDTH*BUFFER_HEIGHT)
#define SCROLL_LINES    8
#define SCROLL_SIZE     (BUFFER_WIDTH*SCROLL_LINES)

static char* s_CharBuffer;
static char* s_ScrollBuffer;
static char* s_ScrollLine;
static s32   s_Initialized=0;
static s32   s_ScrollCursor;

//=========================================================================

void TEXT_SwapBuffers ( void )
{
    ASSERT(s_Initialized);
    x_memset(s_CharBuffer,0,BUFFER_SIZE);
}

//=========================================================================

void TEXT_Render ( void )
{
    ASSERT(s_Initialized);

    // "Stamp" the scroll text onto the small text buffer.
    {
        s32 DestOffset = BUFFER_WIDTH * (BUFFER_HEIGHT-SCROLL_LINES);
        s32 Characters = SCROLL_SIZE;

        char*   s = s_ScrollBuffer;  
        char*   d = s_CharBuffer + DestOffset;

        while( Characters )
        {
            if( *s )    *d = *s;
            s++;
            d++;
            Characters--;
        }
    }
/*
    // Do text dump of screen
    {
        s32     X, Y;
        char*   S;
        S = s_CharBuffer;
        printf("-------------------- TEXT ------------------------\n");
        for( Y=0; Y<BUFFER_HEIGHT; Y++ )
        {
            for( X=0; X<BUFFER_WIDTH; X++ )
                if( S[X]==0 ) S[X] = '.';

            S[ BUFFER_WIDTH-1 ] = 0;
            printf("%s\n",S);
            S+=BUFFER_WIDTH;
        }
        printf("-------------------- TEXT ------------------------\n");
    }
*/
    if( 1 )
    {
        s32     X, Y;
        char*   S;
        S = s_CharBuffer;
        font_BeginRender();
        for( Y=0; Y<BUFFER_HEIGHT; Y++ )
        {
            for( X=0; X<BUFFER_WIDTH; X++ )
            //if( (S[X]!=0) && (S[X]!=32) )
            if (S[X]!=0)    // Draw spaces!!
            {
                font_RenderC(S[X],X,Y);
            }
            
            S+=BUFFER_WIDTH;
        }
        font_EndRender();
    }
}

//=========================================================================

static 
void ScrollTextOneLineUp( void )
{
    x_memmove( s_ScrollBuffer, 
               s_ScrollBuffer + BUFFER_WIDTH,
               BUFFER_WIDTH * (SCROLL_LINES-1) );

    x_memset ( s_ScrollLine, 0, BUFFER_WIDTH );

    s_ScrollCursor = 0;
}

//=========================================================================

void TEXT_Print ( const char* pStr )
{
    ASSERT( s_Initialized );

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
                if( s_ScrollCursor < (BUFFER_WIDTH-1) )
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
            if( s_ScrollCursor < (BUFFER_WIDTH-1) )
                s_ScrollLine[s_ScrollCursor] = ' ';
            s_ScrollCursor++;
        }
        else
        {
            // Handle all other characters
            if( (s_ScrollCursor == -1) || (s_ScrollCursor>=(BUFFER_WIDTH-1)) )
                ScrollTextOneLineUp();

            // Stick the character in the data buffer.
            s_ScrollLine[s_ScrollCursor] = *pStr;

            s_ScrollCursor++;
        }           

        pStr++;
    }
}

//=========================================================================

void TEXT_PrintXY ( const char* pStr, s32 X, s32 Y )
{
    s32     OriginalX = X;
    char*   Buffer;

    ASSERT( s_Initialized );

    if( pStr == NULL )
        return;

    Buffer = s_CharBuffer;

    // Too far right?  Don't even bother.
    if( X >= (BUFFER_WIDTH-1) )
        return;

    // While there's still some string left AND we are not off the bottom.
    while( (*pStr) && (Y<BUFFER_HEIGHT) )
    {
        if( (*pStr) == 0x08 )
        {
            // Expand the tab here.
            // Space to next multiple of 8 column.
            do
            {
                if( (X<(BUFFER_WIDTH-1)) && (X>=0) && (Y>=0) )
                    Buffer[ (Y*(BUFFER_WIDTH)) + X ] = ' ';
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
            if( (X<(BUFFER_WIDTH-1)) && (X>=0) && (Y>=0) )
                Buffer[ (Y*(BUFFER_WIDTH)) + X ] = *pStr;
            X++;
        }

        pStr++;
    }
}

//=========================================================================

void TEXT_Init ( void )
{
    ASSERT(!s_Initialized);

    // Allocate and clear buffers
    s_CharBuffer = (char*)x_malloc(BUFFER_SIZE);
    ASSERT( s_CharBuffer );
    x_memset(s_CharBuffer,0,BUFFER_SIZE);

    // Allocate and clear scroll buffer
    s_ScrollBuffer = (char*)x_malloc(SCROLL_SIZE);
    ASSERT( s_ScrollBuffer );
    x_memset(s_ScrollBuffer,0,SCROLL_SIZE);
    s_ScrollLine = s_ScrollBuffer + SCROLL_SIZE - BUFFER_WIDTH;

    s_Initialized  = 1;
    s_ScrollCursor = 0;
    x_SetPrintHook( TEXT_Print );
    x_SetPrintAtHook( TEXT_PrintXY );
}

//=========================================================================

void TEXT_Kill ( void )
{
    ASSERT(s_Initialized);
    x_free(s_CharBuffer);
    s_Initialized=0;
}

//=========================================================================
