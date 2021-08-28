//=============================================================================
//
//  PS2 Specific Routines
//
//=============================================================================

#include "Render\vu1\vu1.hpp"

#define PLATFORM_PATH   "PS2"

//=============================================================================
//  GLOBALS
//=============================================================================

xbool g_DoFog       = FALSE;
xbool g_DumpLoadMap = FALSE;

//=============================================================================

// Setup GS registers for rendering:
// - Set Scissor region
// - Set Z Buffer
// - Set Alpha test to allow for Punch Thru textures

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
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PushGSContext( 1 );
    gsreg_SetScissor( 0, 0, Width, Height );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PopGSContext();
    
    gsreg_End();
    eng_End();

    // split vram into banks for the different texture types
    vram_Flush();
    vram_AllocateBank( 24, 1 );     // detail maps
    vram_AllocateBank( 24, 1 );     // specular maps
    vram_AllocateBank( 24, 1 );     // spotlight/shadow maps
}

//=============================================================================

f32 Spd = 1000.0f;

void FreeCam( f32 DeltaTime )
{
    view& View = g_View;

    f32 Move = Spd   * DeltaTime;
    f32 Rot  = R_180 * DeltaTime;
    f32 Y    = 0.0f;

    if( input_IsPressed( INPUT_PS2_BTN_L1 ) ) Move *= 4.0f;
    if( input_IsPressed( INPUT_PS2_BTN_L2 ) ) Move *= 0.2f;
    if( input_IsPressed( INPUT_PS2_BTN_R1 ) ) Y =  Move;
    if( input_IsPressed( INPUT_PS2_BTN_R2 ) ) Y = -Move;
    
    f32 X = input_GetValue( INPUT_PS2_STICK_LEFT_X ) * Move;
    f32 Z = input_GetValue( INPUT_PS2_STICK_LEFT_Y ) * Move;
    
    View.Translate( vector3(   -X, 0.0f,    Z ), view::VIEW  );
    View.Translate( vector3( 0.0f,    Y, 0.0f ), view::WORLD );
    
    radian Pitch, Yaw;
    View.GetPitchYaw( Pitch, Yaw );
    
    Pitch += input_GetValue( INPUT_PS2_STICK_RIGHT_Y ) * Rot;
    Yaw   -= input_GetValue( INPUT_PS2_STICK_RIGHT_X ) * Rot;
    View.SetRotation( radian3( Pitch, Yaw, R_0 ) );
}

//=============================================================================

xbool HandleInputPlatform( f32 DeltaTime )
{
    static s32 s_DisplayStats = 0;
    static s32 s_DisplayMode  = 0;
    
    if( g_FreeCam == FALSE )
        return( TRUE );
    
    if( input_IsPressed( INPUT_PS2_BTN_CROSS ) == FALSE )
        FreeCam( DeltaTime );

    if( input_IsPressed ( INPUT_PS2_BTN_START  ) &&
        input_WasPressed( INPUT_PS2_BTN_CIRCLE ) )
        SaveCamera();

    if( input_IsPressed ( INPUT_PS2_BTN_START  ) &&
        input_WasPressed( INPUT_PS2_BTN_SQUARE ) )
    {
        switch( s_DisplayStats )
        {
            case 0 : s_DisplayMode = render::stats::TO_SCREEN;
                     break;
            
            case 1 : s_DisplayMode = render::stats::TO_SCREEN | render::stats::VERBOSE;
                     break;
        
            case 2 : render_GetStats().Clear();
                     s_DisplayMode = 0;
                     break;
        }
        
        s_DisplayStats++;
        if( s_DisplayStats > 2 )
            s_DisplayStats = 0;
    }

    render_GetStats().Print( s_DisplayMode );
    
    if( input_IsPressed ( INPUT_PS2_BTN_SELECT   ) &&
        input_WasPressed( INPUT_PS2_BTN_TRIANGLE ) )
        render_ToggleProfiling();

    return( TRUE );
}

//=============================================================================

void PrintStatsPlatform( void )
{
    if( g_Stats.PS2.VU1Stats == TRUE )
        vu1_PrintStats( TRUE );

    if( g_Stats.PS2.MemoryUsage == TRUE )
    {
        s32 Free, Largest, Fragments;
        x_MemGetFree( Free, Largest, Fragments );
        
        x_DebugMsg( "================== Memory Stats ==================\n" );
        x_DebugMsg( "Free = %dKb  Largest = %dKb  Fragments = %d\n",
            Free / 1024, Largest / 1024, Fragments );
    }
}

//==============================================================================

#define WINDOW_LEFT    (2048-(512/2))
#define WINDOW_TOP     (2048-(512/2))

void DrawQuad( void )
{
    s32 X0 = WINDOW_LEFT + 128;
    s32 Y0 = WINDOW_TOP  + 128;
    s32 X1 = X0 + 256;
    s32 Y1 = Y0 + 256;
    
    gsreg_Begin();
    gsreg_Set( SCE_GS_TEST_1, SCE_GS_SET_TEST_1 ( 0, 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_PRIM,   SCE_GS_SET_PRIM   ( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,  SCE_GS_SET_RGBAQ  ( 0, 0, 0, 128, 0x3F800000 ) );
    gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ    ( (X0 << 4), (Y0 << 4), 0 ) );
    gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ    ( (X1 << 4), (Y1 << 4), 0 ) );
    gsreg_End();
}

//=============================================================================

s32    nBlur      = 2;
f32    BlurOffset = 2.0f;
xcolor BlurColor( 128, 128, 128, 15 );

//=============================================================================

void EndRenderPlatform( void )
{
    eng_Begin( "EndRender" );

    {
        // Render Self-Illum pixels to FrontBuffer Alpha Channel
        //eng_WriteToFrontBuffer( 0xFF000000 );
        //DrawQuad();
    }

    if( DoBlur == TRUE )
        eng_FilterScreen( nBlur, BlurOffset, BlurColor );

    // Copy Self-Illum mask to BackBuffer
    eng_CopyFrontBufferToBack( 0xFF000000 );

    eng_ClearStencilBuffer();
    
    // Render Shadows to Stencil Buffer Alpha Channel here!
    
    eng_ApplyStencilBuffer();
    
    if( g_DoFog == TRUE )
        eng_ApplyZFog();

    eng_End();

    // NOTE:
    // We need to do this to force Entropy to transfer our textures to VRAM every frame.
    // If we dont do this, the textures in VRAM will get corrupted by the stencil/self-illum pass.
    // Later on we will have asynchronous texture downloading anyway.
    //vram_Flush();
    
	if( g_DumpLoadMap )
	{
		g_DumpLoadMap = FALSE;

        /*
		extern xstring* x_fopen_log;

		x_fopen_log->SaveFile( xfs( "CDFS_LOG_%s.txt", g_LevelName ) );
		x_fopen_log->Clear();
        */
	}
}

//=============================================================================

void LightObjectPlatform( u16* pColorTable, const rigid_geom& RigidGeom, f32 a_Intensity )
{
    f32 Ambient   = a_Intensity *  0.5f;
    f32 Intensity = a_Intensity * 16.0f;

    s_LDir.Normalize();
    
    for( s32 i=0; i<RigidGeom.m_nSubMeshs; i++ )
    {
        rigid_geom::submesh&   GeomSubMesh = RigidGeom.m_pSubMesh[i];
        rigid_geom::dlist_ps2& DList       = RigidGeom.m_System.pPS2[ GeomSubMesh.iDList ];

        u16* pCol = &pColorTable[ DList.iColor ];
        
        for( s32 j=0; j<DList.nVerts; j++ )
        {
            s8* pNormal = &DList.pNormal[ j * 3 ];
            
            vector3 N( pNormal[0] / 128.0f, pNormal[1] / 128.0f, pNormal[2] / 128.0f );
            
            f32 D = N.Dot( s_LDir );
            s32 I = (s32)( MINMAX( Ambient, D, 1.0f ) * Intensity ) & 0x1F;
            
            *pCol++ = 0x8000 | (I << 10) | (I << 5) | I;
        }
    }
}

//=============================================================================
