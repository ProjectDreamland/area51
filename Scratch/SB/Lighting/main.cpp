//==============================================================================
//
//  Main.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "LightShaftEffect.hpp"

//==============================================================================
//  DATA
//==============================================================================

// Edit these in the debugger!
static xcolor DIR_COL(50,50,50,50);
static xcolor AMB_COL(20,20,20,20);
static xbool  COL16  = TRUE;
static xbool  DITHER = FALSE;
static s32    STEP   = 64;

light_shaft_effect g_LightShaftEffect;

//==============================================================================
//  DEFINES
//==============================================================================

#ifdef TARGET_PS2
void    eng_GetStats(s32 &Count, f32 &CPU, f32 &GS, f32 &INT, f32 &FPS);
#endif

#ifdef TARGET_PC

void pc_PreResetCubeMap( void ) {}
void pc_PostResetCubeMap( void ) {}

#endif

//==============================================================================
// FUNCTIONS
//==============================================================================

static xbool AppInput( f32 DeltaTime )
{
    CONTEXT("Input");
    (void)DeltaTime;

#ifdef TARGET_PC
    while( input_UpdateState() )
#else
    input_UpdateState();
#endif
    {
        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );
    }

    return TRUE;
}

//==============================================================================

xcolor LightPoint( s32 X, s32 Y )
{
    // Compute normal
    vector3 N;
    //N.GetX() = ((f32)X - 256.0f) / 256.0f;
    //N.GetY() = ((f32)Y - 224.0f) / 224.0f;
    N.GetX() = -(f32)X / 512.0f;
    N.GetY() = -(f32)Y / 448.0f;
    N.GetZ() = x_sqrt(1.0f - x_sqr(N.GetX()) - x_sqr(N.GetY()));
    N.Normalize();

    // Setup light direction
    vector3 L(0.5f, 0.5f, 1.0f);
    L.Normalize();

    // Compute intensity
    f32 I = x_max(0.0f, N.Dot(L));
    
    // Compute color
    xcolor C;
    C.R = (u8)x_max(0, x_min(255, (s32)AMB_COL.R + (s32)((f32)DIR_COL.R * I)));
    C.G = (u8)x_max(0, x_min(255, (s32)AMB_COL.G + (s32)((f32)DIR_COL.G * I)));
    C.B = (u8)x_max(0, x_min(255, (s32)AMB_COL.B + (s32)((f32)DIR_COL.B * I)));
    C.A = 255;

    // Convert to 16 bit?
    if (COL16)
    {
        C.R &= ~3;
        C.G &= ~3;
        C.B &= ~3;
    }

    return C;
}

//==============================================================================

void AppRender( void )
{
    CONTEXT("Render");

    //s32 x,y;

    eng_Begin( "Render" );
    {
        draw_ClearL2W();

        // Turn on dither?
        matrix4 DM;
        DM.Zero();
        DM(0,0) = -4;
        DM(0,1) = 2;
        DM(0,2) = -3;
        DM(0,3) = 3;

        DM(1,0) = 0;
        DM(1,1) = -2;
        DM(1,2) = 1;
        DM(1,3) = -1;

        DM(2,0) = -3;
        DM(2,1) = 3;
        DM(2,2) = -4;
        DM(2,3) = 2;

        DM(3,0) = 1;
        DM(3,1) = -1;
        DM(3,2) = 0;
        DM(3,3) = -2;
        u64 Data = 0;
        for (s32 i = 0; i < 4; i++)
        {
            for (s32 j = 0; j < 4; j++)
            {
                s32 V = (s32)DM(i,j);
                Data <<= 4;
                Data |= (V & 3);
            }
        }

#ifdef TARGET_PS2
        gsreg_Begin();
        gsreg_Set(SCE_GS_DTHE, DITHER);
        gsreg_Set(SCE_GS_DIMX, Data);
        gsreg_End();
#endif

/*
        // Colors
        xcolor TL, BL, BR, TR;
        for (x = 0; x < 512; x += STEP)
        {
            for (y = 0; y < 448; y += STEP)
            {
                // Compute colors
                TL = LightPoint(x,y);
                TR = LightPoint(x+STEP,y);
                BL = LightPoint(x,y+STEP);
                BR = LightPoint(x+STEP,y+STEP);

                // Compute rect
                irect Rect;
                Rect.l = x;
                Rect.t = y;
                Rect.r = x+STEP;
                Rect.b = y+STEP;
                draw_GouraudRect(Rect, TL, BL, BR, TR, FALSE);
            }
        }

        // Draw lines
        draw_Begin( DRAW_LINES, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER);
        draw_Color( xcolor(0,0,0,48) );
        for (x = 0; x < 512; x += STEP)
        {
            draw_Vertex( vector3((f32)x,0,0) );
            draw_Vertex( vector3((f32)x,448,0) );
        }
        for (y = 0; y < 448; y += STEP)
        {
            draw_Vertex( vector3(0,(f32)y,0)   );
            draw_Vertex( vector3(512,(f32)y,0) );
        }
        draw_End();
*/
    }

    // Render light shaft text
    g_LightShaftEffect.Render( 2.0f );

    eng_End();
}

//==============================================================================

void AppAdvance( f32 DeltaTime )
{
    CONTEXT("Advance");
    
    // Cap incase we are in the debugger
    if (DeltaTime > (1.0f / 5.0f))
        DeltaTime = (1.0f / 5.0f);

    // Run at PS2 speed
    static f32 AccumTime = 0;
    static f32 Step = 1.0f / 30.0f;
    AccumTime += DeltaTime;
    if (AccumTime >= Step)
    {
        AccumTime -= Step;
        DeltaTime = Step;

        // Update light shaft effect
        g_LightShaftEffect.Update( DeltaTime );
    }
    else
        DeltaTime = 0;
}

//==============================================================================

void AppInit( void )
{
    // Init engine
    eng_Init();
    eng_SetBackColor(xcolor(0,0,0,0));

    // Init light shaft effect
    g_LightShaftEffect.Init( "Text.psd", "Fog.psd" );
}

//==============================================================================

void AppKill( void )
{
    // Kill light shaft effect
    g_LightShaftEffect.Kill();

    // Kill engine
    eng_Kill();
}

//==============================================================================

void AppMain( s32, char** )
{
    AppInit();

    xtimer Timer;
    Timer.Start();
    view    View;

    while( TRUE )
    {
        eng_MaximizeViewport( View );
        eng_SetView         ( View );

        f32 DeltaTime = Timer.TripSec();

        if( !AppInput( DeltaTime ) )
            break;

        AppRender();

        AppAdvance( DeltaTime );

        // DONE!
        eng_PageFlip();

        // Profile
        x_ContextPrintProfile();
        x_ContextResetProfile();
    }

    AppKill();
}

//==============================================================================
