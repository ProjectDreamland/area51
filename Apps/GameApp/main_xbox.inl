//=============================================================================
//
//  XBOX Specific Routines
//
//=============================================================================

#define PLATFORM_PATH  "XBOX"

#include "Menu/DebugMenu2.hpp"
//=============================================================================
//  GLOBALS
//=============================================================================

xbool g_DumpLoadMap = FALSE;

//=============================================================================

void InitRenderPlatform( void )
{
    CONTEXT( "InitRenderPlatform" );

    eng_SetViewport( g_View );
}

//=============================================================================

f32 Spd = 1000.0f;

void FreeCam( f32 DeltaTime )
{
    CONTEXT( "FreeCam" );
    {
        view& View = g_View;

        f32 Move = Spd   * DeltaTime;
        f32 Rot  = R_180 * DeltaTime;
        f32 Y    = 0.0f;

        if( input_IsPressed( INPUT_XBOX_L_TRIGGER ))
        {
            if( input_IsPressed( INPUT_XBOX_BTN_BLACK ))
                Move *= 0.2f;
            else
                Move *= 4.0f;
        }
        if( input_IsPressed( INPUT_XBOX_R_TRIGGER ))
        {
            if( input_IsPressed( INPUT_XBOX_BTN_BLACK ))
                Y =  Move;
            else
                Y = -Move;
        }

        f32 X = input_GetValue( INPUT_XBOX_STICK_LEFT_X ) * Move;
        f32 Z = input_GetValue( INPUT_XBOX_STICK_LEFT_Y ) * Move;

        View.Translate( vector3(   -X, 0.0f,    Z ), view::VIEW  );
        View.Translate( vector3( 0.0f,    Y, 0.0f ), view::WORLD );

        radian Pitch, Yaw;
        View.GetPitchYaw( Pitch, Yaw );

        Pitch += input_GetValue( INPUT_XBOX_STICK_RIGHT_Y ) * Rot;
        Yaw   -= input_GetValue( INPUT_XBOX_STICK_RIGHT_X ) * Rot;

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
}

//=============================================================================

xbool HandleInputPlatform( f32 DeltaTime )
{
    (void)DeltaTime;

    ///////////////////////////////////////////////////////////////////////////

    #if !defined( CONFIG_RETAIL )  && ( !CONFIG_IS_DEMO )
    {
        CONTEXT( "HandleInputPlatform" );
        //if( g_NetworkMgr.GetLocalPlayerCount() < 2 )
        {
            if( input_IsPressed ( INPUT_XBOX_BTN_L_STICK ) &&
                input_WasPressed( INPUT_XBOX_BTN_R_STICK ))
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

                if( input_IsPressed ( INPUT_XBOX_BTN_START ) &&
                    input_WasPressed( INPUT_XBOX_BTN_A ))
                    SaveCamera();

                if( input_IsPressed ( INPUT_XBOX_BTN_START ) &&
                    input_WasPressed( INPUT_XBOX_BTN_B ))
                    LoadCamera();
            }
        }
    }
    #endif // !defined( CONFIG_RETAIL )

    ///////////////////////////////////////////////////////////////////////////

    /*#if defined( ENABLE_DEBUG_MENU )
    {
        extern xbool g_ShowGammaInfo;
        extern f32   g_Brightness;
        extern f32   g_Contrast;
        extern bool  s_bSet;

        if( g_ShowGammaInfo && input_IsPressed( INPUT_XBOX_BTN_BACK  ))
        {
            if( input_IsPressed( INPUT_XBOX_BTN_RIGHT )) g_Brightness += 0.01f;
            if( input_IsPressed( INPUT_XBOX_BTN_LEFT  )) g_Brightness -= 0.01f;
            if( input_IsPressed( INPUT_XBOX_BTN_BLACK )) g_Contrast   += 0.5f;
            if( input_IsPressed( INPUT_XBOX_BTN_WHITE )) g_Contrast   -= 0.5f;
        }
    }
    #endif*/

    return( TRUE );
}

//=============================================================================

void PrintStatsPlatform( void )
{
    CONTEXT( "PrintStatsPlatform" );
}

//=============================================================================

void EndRenderPlatform( void )
{
    CONTEXT( "EndRenderPlatform" );
}
