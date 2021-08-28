//=============================================================================
//
//  PS2 Specific Routines
//
//=============================================================================

#define PLATFORM_PATH   "PS2"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

//=============================================================================
//  GLOBALS
//=============================================================================

//=============================================================================

// Setup GS registers for rendering:
// - Set Scissor region
// - Set Z Buffer
// - Set Alpha test to allow for Punch Thru textures

void InitRenderPlatform( void )
{
    CONTEXT( "InitRenderPlatform" );

    // as a safety mechanism for any REF'd data used by the MFIFO, flush the
    // cache between logic and render
    FlushCache( WRITEBACK_DCACHE );

    eng_Begin("GSSettings0");
    
    // calculate the scissor region
    s32 X0, X1, Y0, Y1;
    g_View.GetViewport(X0,Y0,X1,Y1);

    // Clear FrontBuffer Alpha Channel
    eng_ClearFrontBuffer();
    eng_WriteToBackBuffer();

    gsreg_Begin( 4 );
    gsreg_SetScissor( X0, Y0, X1, Y1 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PushGSContext( 1 );
    gsreg_SetScissor( X0, Y0, X1, Y1 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PopGSContext();
    
    gsreg_End();
    eng_End();

    // split vram into banks for the different texture types
    vram_Flush();
    vram_AllocateBank( 25 );     // detail maps
    vram_AllocateBank( 25 );     // specular maps
    vram_AllocateBank( 25 );     // spotlight/shadow maps
}

//=============================================================================

#if !defined( CONFIG_RETAIL )

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

    Pitch = MAX(Pitch, -(R_90-R_2));
    Pitch = MIN(Pitch,  (R_90-R_2));
    View.SetRotation( radian3( Pitch, Yaw, R_0 ) );
    
    // Move the player.
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if ( pPlayer )
    {
        pPlayer->OnMoveFreeCam( View );
    }
}

#endif // !defined( CONFIG_RETAIL )

//=============================================================================

xbool HandleInputPlatform( f32 DeltaTime )
{
    (void)DeltaTime;
#if !defined( CONFIG_RETAIL ) && (!CONFIG_IS_DEMO)
    if (  input_IsPressed ( INPUT_PS2_BTN_SELECT ) &&
          input_WasPressed ( INPUT_PS2_BTN_SQUARE )
       )
    {
        g_FreeCam ^= 1;
        if ( ! g_FreeCam )
        {
            // Move the player.
            player* pPlayer = SMP_UTIL_GetActivePlayer();
            if ( pPlayer )
            {
                vector3 vPos = g_View.GetPosition();
                pPlayer->OnExitFreeCam( vPos );
            }
        }
    }

    if( g_FreeCam )
    {
        FreeCam( DeltaTime );

        if( input_IsPressed ( INPUT_PS2_BTN_SELECT ) &&
            input_WasPressed( INPUT_PS2_BTN_CIRCLE ) )
            SaveCamera();

        if( input_IsPressed ( INPUT_PS2_BTN_SELECT ) &&
            input_WasPressed( INPUT_PS2_BTN_SQUARE ) )
            LoadCamera();
    }
#endif // !defined( CONFIG_RETAIL )

    return( TRUE );
}

//=============================================================================

void PrintStatsPlatform( void )
{
#if defined(ENABLE_DEBUG_MENU)
    if( g_Stats.PS2.MemoryUsage == TRUE )
    {
        s32 Free, Largest, Fragments;
        x_MemGetFree( Free, Largest, Fragments );
        
        x_DebugMsg( "================== Memory Stats ==================\n" );
        x_DebugMsg( "Free = %dKb  Largest = %dKb  Fragments = %d\n",
            Free / 1024, Largest / 1024, Fragments );
    }
#endif
}

//=============================================================================

void EndRenderPlatform( void )
{
    CONTEXT( "EndRenderPlatform" );

    // reset the viewport
    s32 XRes, YRes;
    eng_GetRes(XRes, YRes);
    eng_Begin("GSSettings1");
    gsreg_Begin( 1 );
    gsreg_SetScissor(0,0,XRes,YRes);
    gsreg_End();
    eng_End();

}
