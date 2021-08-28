//=========================================================================
//
// RTF_HANDLER.CPP
//
//=========================================================================
 
#include "Entropy.hpp"

#include "StateMgr/StateMgr.hpp"

// This define control whether RTF is present or not.
#if !defined(X_RETAIL)
#define RTF_PRESENT
#endif

#define MAX_FILENAME_CHARS 30

// These defines control the input buttons
#ifdef TARGET_PS2
#define RTF_BUTTON_CONTINUE     INPUT_PS2_BTN_CIRCLE
#define RTF_BUTTON_ABORT        INPUT_PS2_BTN_TRIANGLE
#define RTF_BUTTON_IGNORE       INPUT_PS2_BTN_SQUARE
#endif

#ifdef TARGET_XBOX
#define RTF_BUTTON_CONTINUE     INPUT_XBOX_BTN_X
#define RTF_BUTTON_ABORT        INPUT_XBOX_BTN_B
#define RTF_BUTTON_IGNORE       INPUT_XBOX_BTN_Y
#endif

#ifdef TARGET_PC
#define RTF_BUTTON_CONTINUE     INPUT_PS2_BTN_CIRCLE
#define RTF_BUTTON_ABORT        INPUT_PS2_BTN_TRIANGLE
#define RTF_BUTTON_IGNORE       INPUT_PS2_BTN_SQUARE
#endif

//=============================================================================

#if defined(RTF_PRESENT)

static
xbool fn_RTFHandler( const char* pFileName,
                  s32         LineNumber,
                  const char* pExprString,
                  const char* pMessageString );

#endif

//=============================================================================

void InstallRTFHandler( void )
{
#if defined(RTF_PRESENT)
    x_SetRTFHandler( fn_RTFHandler );
#endif
}

//=============================================================================
#if !defined(RTF_PRESENT)
//=============================================================================

//=============================================================================
#else
//=============================================================================

struct ignore_entry
{
    const char  *pFilename;
    s32         Line;
};

#define MAX_IGNORE_ENTRIES 32
static ignore_entry     s_IgnoreList[ MAX_IGNORE_ENTRIES ];
static s32              s_iIgnore = 0;
xbool                   g_bInsideRTF = FALSE;

//=============================================================================

static
void Pong( void )
{
    static f32      Top     = 14;
    static f32      Bottom  = 22;
    static f32      Left    = 0;
    static f32      Right   = 37;
    static f32      Paddle1 = Top;
    static f32      Paddle2 = Top;
    static f32      PaddleY = 0.7f;
    static vector2  Ball(30,16);
    static vector2  dBall(1,1);
    static s32      Score1  = 0;
    static s32      Score2  = 0;

    x_printfxy( (s32)Ball.X, (s32)(Ball.Y),    "o" );
    x_printfxy( (s32)Left,   (s32)(Paddle1+0), ")" );
    x_printfxy( (s32)Left,   (s32)(Paddle1+1), ")" );
    x_printfxy( (s32)Left,   (s32)(Paddle1+2), ")" );
    x_printfxy( (s32)Right,  (s32)(Paddle2+0), "(" );
    x_printfxy( (s32)Right,  (s32)(Paddle2+1), "(" );
    x_printfxy( (s32)Right,  (s32)(Paddle2+2), "(" );

    x_printfxy( (s32)Left,   (s32)Bottom+1,"%ld",Score1);
    x_printfxy( (s32)Right-5-1,   (s32)Bottom+1,"%ld",Score2);
 
    Ball += dBall;
    if (Ball.X <= 0)
    {
        if ((Paddle1 <= Ball.Y) && ((Paddle1+3) >= Ball.Y))
        {
            Ball.X = 0;
            dBall.X = x_frand(0.5f, 1.5f);
            dBall.Y *= x_frand(0.9f,1.1f);
        }
        else
        {
            Ball.Set(Right/2,0);
            dBall.Set( x_frand(-1,1), x_frand(-1,1) );
            Score2++;
        }
    }

    if (Ball.X >= Right)
    {
        if ((Paddle2 <= Ball.Y) && ((Paddle2+3) >= Ball.Y))
        {
            Ball.X = Right;
            dBall.X = -x_frand(0.5f, 1.5f);
            dBall.Y *= x_frand(0.9f,1.1f);
        }
        else
        {
            Ball.Set(Right/2,0);
            dBall.Set( x_frand(-1,1), x_frand(-1,1) );
            Score1++;
        }
    }
    
    if (Ball.Y <= Top)
    {
        Ball.Y = Top;
        dBall.Y *= -x_frand(0.8f, 1.2f);
    }
    if (Ball.Y >= Bottom)
    {
        Ball.Y = Bottom;
        dBall.Y *= -x_frand(0.8f, 1.2f);
    }

    if (dBall.X < 0)
    {
        if (Ball.Y > (Paddle1+2))
            Paddle1 += PaddleY;
        if (Ball.Y < Paddle1)
            Paddle1 -= PaddleY;
    }
    else
    {
        if (Ball.Y > (Paddle2+2))
            Paddle2 += PaddleY;
        if (Ball.Y < Paddle2)
            Paddle2 -= PaddleY;
    }

    Paddle1 = MIN(Bottom-2,MAX(Top,Paddle1));
    Paddle2 = MIN(Bottom-2,MAX(Top,Paddle2));

    
    dBall.X = MIN(1.5f,MAX(-1.5f,dBall.X));
    dBall.Y = MIN(1.5f,MAX(-1.5f,dBall.Y));

    if (dBall.X >=0 && dBall.X < 0.5f)  dBall.X = 0.5f;
    if (dBall.X <0  && dBall.X > -0.5f) dBall.X = -0.5f;
    if (dBall.Y >=0 && dBall.Y < 0.5f)  dBall.Y = 0.5f;
    if (dBall.Y <0  && dBall.Y > -0.5f) dBall.Y = -0.5f;
}
 
//=============================================================================

static
void Breakout( void )
{
    static const s32      Top     = 14;
    static const s32      Bottom  = 22;
    static const s32      Left    = 0;
    static const f32      Right   = 37;
    static vector2        Ball(16,15);
    static vector2        dBall(1,1);
    static const s32      nBricksAcross = 18;
    static const s32      nBricksDown   = 3;
    static       s32      Paddle        = 16;
    static       u8       Bricks[nBricksDown][nBricksAcross] =
    {
        { 1,1,1,1,1,1,  1,1,1,1,1,1,  1,1,1,1,1,1 },
        { 1,1,1,1,1,1,  1,1,1,1,1,1,  1,1,1,1,1,1 },
        { 1,1,1,1,1,1,  1,1,1,1,1,1,  1,1,1,1,1,1 }
    };

    // update the ball and paddle
    //vector2 Prev = Ball;
    Ball += dBall;

    // check for collisions against bricks
    xbool bChangeHorzDirection = FALSE;
    xbool bChangeVertDirection = FALSE;
    if ( Ball.Y <  (f32)(Top+4) )
    {
        s32 BrickY = (s32)Ball.Y-Top;
        s32 BrickX = (s32)Ball.X / 2;
        BrickY = MIN(BrickY,nBricksDown-1);
        BrickY = MAX(BrickY,0);
        BrickX = MIN(BrickX,nBricksAcross-1);
        BrickX = MAX(BrickX,0);

        if ( Bricks[BrickY][BrickX] )
        {
            Bricks[BrickY][BrickX] = 0;
            bChangeVertDirection = dBall.Y < 0.0f;
        }
    }
    
    if ( (Ball.Y > (f32)Bottom) || (Ball.Y < (f32)(Top+1)) )
    {
        bChangeVertDirection = TRUE;
    }

    if ( (Ball.X > (f32)Right) || (Ball.X < (f32)Left) )
    {
        bChangeHorzDirection = TRUE;
    }

    if ( bChangeVertDirection )
    {
        dBall.Y = -dBall.Y;
        dBall.Rotate( x_frand(-R_1, R_1) );
        radian Angle = dBall.Angle();
        s32 SafetyBreak = 0;
        while ( (Angle < R_0) && (SafetyBreak++ < 10) )
            Angle += R_360;
        while ( Angle >= R_360 && (SafetyBreak++ < 10) )
            Angle -= R_360;

        if ( (SafetyBreak > 8) ||
             ((Angle >= R_360-R_1) || (Angle < R_1)) ||
             ((Angle >= R_90-R_1) && (Angle < R_90+R_1)) )
        {
            // ball got very horizontal, which is a bad thing.
            // easy way out. Reset it.
            dBall.Set( 1.0f, 1.0f );
            Ball.Set(16.0f,15.0f);
        }
    }
    if ( bChangeHorzDirection )
        dBall.X = -dBall.X;       

    // just make sure we're always in the playing court
    Ball.X = MAX(Ball.X, (f32)Left);
    Ball.X = MIN(Ball.X, (f32)Right);
    Ball.Y = MAX(Ball.Y, (f32)Top);
    Ball.Y = MIN(Ball.Y, (f32)Bottom);

    // render the bricks, and simultaneously figure out if they need to be reset
    xbool ResetBricks = TRUE;
    for ( s32 y = 0; y < nBricksDown; y++ )
    {
        for ( s32 x = 0; x < nBricksAcross; x++ )
        {
            if ( Bricks[y][x] )
            {
                x_printfxy( Left + x*2 + 0, Top + y, "[" );
                x_printfxy( Left + x*2 + 1, Top + y, "]" );
                ResetBricks = FALSE;
            }
        }
    }

    if ( ResetBricks )
    {
        Ball.Set(16.0f,15.0f);
        Paddle = 16;
        dBall.Set(1.0f,1.0f);
        for ( s32 y = 0; y < nBricksDown; y++ )
        {
            for ( s32 x = 0; x < nBricksAcross; x++ )
            {
                Bricks[y][x] = 1;
            }
        }
    }

    // render the paddle
    Paddle = (s32)Ball.X - 1;
    x_printfxy( Paddle, Bottom, "___" );

    // render the ball
    x_printfxy( (s32)Ball.X, (s32)Ball.Y, "o" );
}

//=============================================================================

static
xbool fn_RTFHandler( const char* pFileName,
                  s32         LineNumber,
                  const char* pExprString,
                  const char* pMessageString )
{
    //==-----------------------------------------
    // RTF handler will not work if we are
    // rendering in a separate thread.
    //==-----------------------------------------
    if( g_StateMgr.IsBackgroundThreadRunning() )
        return TRUE;

    //==-----------------------------------------
    // Handle cascade of RTFs
    //==-----------------------------------------
    if( g_bInsideRTF ) return TRUE;
    else               g_bInsideRTF = TRUE;

    //==-----------------------------------------
    // Shut off monkey if you have one!!!
    //==-----------------------------------------

    //==-----------------------------------------   
    //  FIXUP FILENAME  
    //==-----------------------------------------
    if( pFileName && (x_strlen(pFileName) > MAX_FILENAME_CHARS) )
    {
        pFileName = pFileName+x_strlen(pFileName)-MAX_FILENAME_CHARS;
    }

    //==-----------------------------------------
    //  If we're ignoring this assert, just return
    //  without doing anything
    //==-----------------------------------------
    s32 i;
    for( i=0; i<s_iIgnore; i++ )
    {
        if( (s_IgnoreList[i].pFilename == pFileName) && 
            (s_IgnoreList[i].Line == LineNumber) )
        {
            g_bInsideRTF = FALSE;
            return FALSE;
        }
    }

    //==-----------------------------------------
    // Display output to TTY
    //==-----------------------------------------
    x_DebugMsg( "*** RUNTIME FAILURE\n" );
    if( pFileName )         x_DebugMsg( "*** File: %s on line %d\n", pFileName, LineNumber );
    else                    x_DebugMsg( "*** File: <unknown> on line %d\n", LineNumber );
    if( pExprString )       x_DebugMsg( "*** Expr: %s\n", pExprString );
    if( pMessageString )    x_DebugMsg( "*** Msg : %s\n", pMessageString );

    //==-----------------------------------------
    // Report to logging system
    //==-----------------------------------------
    #ifdef X_LOGGING
    LOG_ERROR( "ASSERT",  "*** RUNTIME FAILURE\n" );
    if( pFileName )         LOG_ERROR( "ASSERT",  "*** File: %s on line %d\n", pFileName, LineNumber );
    else                    LOG_ERROR( "ASSERT",  "*** File: <unknown> on line %d\n", LineNumber );
    if( pExprString )       LOG_ERROR( "ASSERT",  "*** Expr: %s\n", pExprString );
    if( pMessageString )    LOG_ERROR( "ASSERT",  "*** Msg : %s\n", pMessageString );
    LOG_FLUSH();
    #endif

    //==-----------------------------------------
    // Ensure we're looking at back buffer
    //==-----------------------------------------
#ifdef TARGET_XBOX
    IDirect3DSurface8* pSurface;
    g_pd3dDevice->GetBackBuffer( 0,D3DBACKBUFFER_TYPE_MONO,&pSurface );
    g_pd3dDevice->SetRenderTarget( pSurface,NULL );
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,D3DCOLOR_RGBA(32,0,0,0),0,0 );
    pSurface->Release();
#endif

    //==-----------------------------------------
    // If inside eng_Begin try to shut it down
    //==-----------------------------------------
    if( eng_InBeginEnd() )
    {
        eng_End();
    }

    //==-----------------------------------------
    //  Process the event
    //==-----------------------------------------

    xbool   bExit = FALSE;
    xbool   bAbort = TRUE;
    xbool   bPong  = x_rand() & 1;

    while( !bExit )
    {
        eng_SetBackColor( xcolor(32,0,0) );

        input_UpdateState();
        {
            if( input_WasPressed( RTF_BUTTON_ABORT ) )
            {
                bExit  = TRUE;
                bAbort = TRUE;
            }

            if( input_WasPressed( RTF_BUTTON_CONTINUE ) )
            {
                bExit  = TRUE;
                bAbort = FALSE;
            }

            if( input_WasPressed( RTF_BUTTON_IGNORE ) )
            {
                bExit  = TRUE;
                bAbort = FALSE;
                if (s_iIgnore < MAX_IGNORE_ENTRIES)
                {
                    s_IgnoreList[s_iIgnore].pFilename = pFileName;
                    s_IgnoreList[s_iIgnore].Line      = LineNumber;
                    s_iIgnore++;
                }
            }
        }    
    
        s32 Y=1;
        x_printfxy(0,Y++, "!!! RUNTIME FAILURE !!!\n" );

        // Handle FileName
        if( pFileName )         
        {
            x_printfxy(0,Y++, "File: %s",pFileName );
            x_printfxy(0,Y++, "Line: %ld",LineNumber );
        }
        else                    
        {
            x_printfxy(0,Y++, "File: <unknown>\n", LineNumber );
            x_printfxy(0,Y++, "Line: %ld",LineNumber );
        }

        // Handle Expression
        if( pExprString )   
        {
            x_printfxy(0,Y++, "Expr: %s\n", pExprString );
        }

        // Handle Message
        if( pMessageString )
        {
            x_printfxy(0,Y, "Msg : %s\n", pMessageString );
            Y+=6;
        }

#ifdef TARGET_XBOX
        x_printfxy(0,Y++,"Press B to halt execution");
        x_printfxy(0,Y++,"Press X to continue execution");
        x_printfxy(0,Y++,"Press Y to ignore");
#endif

#if defined(TARGET_PS2) || defined(TARGET_PC)
        x_printfxy(0,Y++,"Press TRIANGLE to halt execution");
        x_printfxy(0,Y++,"Press CIRCLE to continue execution");
        x_printfxy(0,Y++,"Press SQUARE to ignore");
#endif

        if ( bPong )
            Pong();
        else
            Breakout();

        eng_PageFlip();
    }

    eng_SetBackColor( xcolor(0,0,0) );
    g_bInsideRTF = FALSE;
    return bAbort;
}

//=============================================================================
#endif
//=============================================================================

