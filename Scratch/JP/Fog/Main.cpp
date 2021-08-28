//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"

view View;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void Initialize( void )
{
    eng_Init();

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(100,100,200) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 0.1f, 1000.0f );
}

//=========================================================================

void Shutdown( void )
{
}

//=========================================================================

void SaveCamera( void )
{
    matrix4 M = View.GetV2W();
    X_FILE* fp;
    
    if( !(fp = x_fopen( "camera.dat", "wb" ))) ASSERT( FALSE );
    x_fwrite( &M, sizeof( M ), 1, fp );
    x_fclose( fp );
    x_DebugMsg( "Camera saved\n" );
}

void LoadCamera( void )
{
    X_FILE* fp;

    matrix4 ViewMat;
    ViewMat.Identity();
    
    if( !(fp = x_fopen( "camera.dat", "rb" ))) return;
    x_fread( &ViewMat, sizeof( ViewMat ), 1, fp );
    x_fclose( fp );

    View.SetV2W( ViewMat );
    x_DebugMsg( "Camera loaded\n" );
}

//=========================================================================

xbool HandleInput( void )
{
    if( input_UpdateState() )
    {
        radian Pitch;
        radian Yaw;
        f32    S   = 0.50f;
        f32    R   = 0.025f;
        f32    Lateral;
        f32    Vertical;

        if( input_IsPressed( INPUT_PS2_BTN_L1 ) )  S *= 4.0f;
        if( input_IsPressed( INPUT_PS2_BTN_R1 ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
        if( input_IsPressed( INPUT_PS2_BTN_R2 ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );

        if( input_IsPressed( INPUT_PS2_BTN_CIRCLE ) ) SaveCamera();

        Lateral  = S * input_GetValue( INPUT_PS2_STICK_LEFT_X );
        Vertical = S * input_GetValue( INPUT_PS2_STICK_LEFT_Y );
        View.Translate( vector3(0,0,Vertical), view::VIEW );
        View.Translate( vector3(-Lateral,0,0), view::VIEW );

        View.GetPitchYaw( Pitch, Yaw );
        Pitch += input_GetValue( INPUT_PS2_STICK_RIGHT_Y ) * R;
        Yaw   -= input_GetValue( INPUT_PS2_STICK_RIGHT_X ) * R;
        View.SetRotation( radian3(Pitch,Yaw,0) );
    }

    return( TRUE );
}

//==============================================================================

void InitRenderPlatform( void )
{
    s32 Width, Height;
    eng_GetRes( Width, Height);

    eng_Begin("GS Settings");
    
    // Clear FrontBuffer Alpha Channel
    eng_ClearFrontBuffer();
    eng_WriteToBackBuffer();
    
    gsreg_Begin();
    gsreg_SetScissor( 0, 0, Width, Height );
    gsreg_SetAlphaAndZBufferTests( 1, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PushGSContext( 1 );
    gsreg_SetScissor( 0, 0, Width, Height );
    gsreg_SetAlphaAndZBufferTests( 1, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PopGSContext();
    
    gsreg_End();
    eng_End();
}

//==============================================================================

xcolor BackColor( XCOLOR_WHITE );

void Render( void )
{
    eng_MaximizeViewport( View );
    eng_SetView         ( View, 0 );
    eng_ActivateView    ( 0 );
    eng_SetBackColor    ( BackColor );

    {
        draw_ClearL2W();
        draw_Grid( vector3(  0,  0,  0), 
                   vector3(100,  0,  0), 
                   vector3(  0,  0,100), 
                   xcolor (  0,128,  0), 16 );
    }
}

//==============================================================================

#define WINDOW_LEFT    (2048-(512/2))
#define WINDOW_TOP     (2048-(512/2))

xcolor Color( 255, 0, 0, 128 );

void DrawQuad( void )
{
    s32 X0 = WINDOW_LEFT + 128;
    s32 Y0 = WINDOW_TOP  + 128;
    s32 X1 = X0 + 256;
    s32 Y1 = Y0 + 256;
    
    gsreg_Begin();
    gsreg_Set( SCE_GS_TEST_1, SCE_GS_SET_TEST_1 ( 0, 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_PRIM,   SCE_GS_SET_PRIM   ( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,  SCE_GS_SET_RGBAQ  ( Color.R, Color.G, Color.B, Color.A, 0x3F800000 ) );
    gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ    ( (X0 << 4), (Y0 << 4), 0 ) );
    gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ    ( (X1 << 4), (Y1 << 4), 0 ) );
    gsreg_End();
}

//==============================================================================

u32 Mask = 0xFF000000;
u32 msk  = 0xFF000000;

s32 fog = 0;

void AppMain( s32, char** )
{
    Initialize();

    LoadCamera();    

    for( s32 Frame = 0; TRUE; Frame++ )
    {
        if( !HandleInput() )
            break;

        InitRenderPlatform();
        
        eng_Begin();

        // Render to BackBuffer as normal
        Render();

        // Render Self-Illum pixels to FrontBuffer Alpha Channel
        eng_WriteToFrontBuffer( msk );
        DrawQuad();

        // End of rendering
        eng_CopyFrontBufferToBack( Mask );
        
        eng_ClearStencilBuffer();
        
        // Render Shadows to Stencil Buffer Alpha Channel
        
        eng_ApplyStencilBuffer();
        
        if( fog ) eng_ApplyZFog();

        eng_End();
        eng_PageFlip();
        
        if( (Frame % 60) == 0 )
        {
            eng_PrintStats();
        }
    }

    Shutdown();
}

//==============================================================================
