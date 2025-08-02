//=============================================================================
//
//  PC Specific Routines
//
//=============================================================================

#define PLATFORM_PATH   "PC"

//extern d3deng_ToggleWindowMode();

//=============================================================================

void InitRenderPlatform( void )
{
    CONTEXT( "InitRenderPlatform" );
}

//=============================================================================

f32 Spd = 2000.0f;

void FreeCam( f32 DeltaTime )
{
    view& View = g_View;

    f32 Move = Spd  * DeltaTime;
    f32 Rot  = R_10 * DeltaTime;
    f32 X    = 0.0f;
    f32 Y    = 0.0f;
    f32 Z    = 0.0f;

    if( input_IsPressed( INPUT_MOUSE_BTN_L ) ) Move *= 4.0f;
    if( input_IsPressed( INPUT_MOUSE_BTN_R ) ) Move *= 0.2f;
    
    if( input_IsPressed( INPUT_KBD_A ) ) X =  Move;
    if( input_IsPressed( INPUT_KBD_D ) ) X = -Move;
    if( input_IsPressed( INPUT_KBD_Q ) ) Y =  Move;
    if( input_IsPressed( INPUT_KBD_Z ) ) Y = -Move;
    if( input_IsPressed( INPUT_KBD_W ) ) Z =  Move;
    if( input_IsPressed( INPUT_KBD_S ) ) Z = -Move;
    
    View.Translate( vector3(    X, 0.0f,    Z ), view::VIEW  );
    View.Translate( vector3( 0.0f,    Y, 0.0f ), view::WORLD );
    
    radian Pitch, Yaw;
    View.GetPitchYaw( Pitch, Yaw );
    
    Pitch += input_GetValue( INPUT_MOUSE_Y_REL ) * Rot;
    Yaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * Rot;
    View.SetRotation( radian3( Pitch, Yaw, R_0 ) );
}

//=============================================================================

xbool HandleInputPlatform( f32 DeltaTime )
{
    static s32 s_DisplayStats = 0;
    static s32 s_DisplayMode  = 0;

    if( input_WasPressed( INPUT_KBD_F10 ) )
    {
        //d3deng_ToggleWindowMode();
    }

    if( g_FreeCam == FALSE )
        return( TRUE );

    FreeCam( DeltaTime );

    if( input_IsPressed ( INPUT_KBD_LSHIFT ) &&
        input_WasPressed( INPUT_KBD_C ) )
        SaveCamera();

#if ENABLE_RENDER_STATS
    if( input_IsPressed ( INPUT_KBD_LSHIFT ) &&
        input_WasPressed( INPUT_KBD_P ) )
    {
        switch( s_DisplayStats )
        {
            case 0 : s_DisplayMode = render::stats::TO_SCREEN;
                     break;
            
            case 1 : s_DisplayMode = render::stats::TO_SCREEN | render::stats::VERBOSE;
                     break;
        
            case 2 : render::GetStats().ClearAll();
                     s_DisplayMode = 0;
                     break;
        }
        
        s_DisplayStats++;
        if( s_DisplayStats > 2 )
            s_DisplayStats = 0;
    }
    
    render::GetStats().Print( s_DisplayMode );
#endif //ENABLE_RENDER_STATS

    return( TRUE );
}

//=============================================================================

void PrintStatsPlatform( void )
{
}

//=============================================================================

void EndRenderPlatform( void )
{
    CONTEXT( "EndRenderPlatform" );
}

//=============================================================================

