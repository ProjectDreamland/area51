//==============================================================================
//
// Main.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "x_files.hpp"
#include "..\..\Support\audiomgr\audio.hpp"
#include "FontManager.h"
#include "Font.hpp"

//==============================================================================
// GLOBALS
//==============================================================================
xbool   g_bQuit = FALSE;
FontManager g_FontManager;
//==============================================================================
// PROTOTYPES
//==============================================================================
void Update( void );

//==============================================================================

void AppMain( s32 argc, char* argv[] )
{
    (void)argc;
    (void)argv;

    // Initialize;
    eng_Init();
    g_FontManager.Init();
//    audio_Init();
//    s32 ID = audio_LoadContainer( "effects.pkg" );
    view View;

    View.SetXFOV( R_70 );                               
    View.SetPosition( vector3(0.0f, 0.0f, 0.0f) );
    View.SetRotation( radian3(0.2573f, -0.4316f, 0) );  
    View.SetZLimits ( 0.1f, 1000.0f );                  

    eng_SetView( View );

//    audio_SetEarView( &View );
    xcolor Blue(255,255,255,255);


    // Setup GS registers for rendering:
    // - Set Scissor region
    // - Set Z Buffer
    // - Set Alpha test to allow for Punch Thru textures

    s32 ate =1;
    s32 atst=ALPHA_TEST_GEQUAL;
    s32 aref=1;


    xwstring t( "This is a wide string" );

    while( !g_bQuit )
    {

        s32 Width, Height;
        eng_GetRes( Width, Height);

        eng_Begin("GS Settings");
        gsreg_Begin();
    
        gsreg_SetScissor( 0, 0, Width, Height );
        gsreg_SetAlphaAndZBufferTests( ate, atst, aref, ALPHA_TEST_FAIL_KEEP,
                                       FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
        eng_PushGSContext( 1 );
        gsreg_SetScissor( 0, 0, Width, Height );
        gsreg_SetAlphaAndZBufferTests( ate, atst, aref, ALPHA_TEST_FAIL_KEEP,
                                       FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
        eng_PopGSContext();
    
        gsreg_End();
        eng_End();

        eng_MaximizeViewport( View );
        eng_SetView         ( View, 0 );    
        eng_ActivateView    ( 0 );

        irect rect1(0,0, 400, 400 );
        g_FontManager.RenderText( rect1, FontManager::H_CENTER | FontManager::V_CENTER, 255, "Test" );

        irect rect2(50,50, 400, 400 );
        g_FontManager.RenderText( rect2, FontManager::H_CENTER | FontManager::V_CENTER, Blue, "Test String 32" );

        irect rect3(100,100, 400, 400 );
        g_FontManager.RenderText( rect3, FontManager::H_CENTER | FontManager::V_CENTER, Blue, "This is a Test String for 128 characters" );

        irect rect4(100, 100, 500, 500 );
        g_FontManager.RenderText( rect4, FontManager::H_CENTER | FontManager::V_CENTER, 255, (const xwchar*)t, FontManager::LARGE );

//        audio_Update();
//        Update();
        
        eng_Begin( "Rect" );
        draw_Rect( rect1 );
        draw_Rect( rect2 );
        draw_Rect( rect3 );
        eng_End();

        g_FontManager.Render();

        irect rect5(150,150, 400, 400 );
        g_FontManager.RenderText( rect5, FontManager::H_CENTER | FontManager::V_CENTER, 255, "T" );
        g_FontManager.Render();


        eng_PageFlip();
    }
    
//    audio_UnloadContainer( ID );
    // Destroy.
//    audio_Kill();
    g_FontManager.Kill();
    eng_Kill();

}
s32 ChannelID = 0;
void Update( void )
{
    input_UpdateState();
    
    if( input_IsPressed ( INPUT_PS2_BTN_START ) )
        g_bQuit = TRUE;
    
    if( audio_IsPlaying( ChannelID ) )
        return;

    if( input_IsPressed( INPUT_PS2_BTN_CROSS ) )
    {
        switch( x_irand( 1, 5 ) )
        {
            case 1:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_WALK_STONE01 );
            break;
            case 2:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_WALK_STONE02 );
            break;
            case 3:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_WALK_STONE03 );
            break;
            case 4:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_WALK_STONE04 );
            break;
            case 5:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_WALK_STONE05 );
            break;
        };
    }
    else if( input_IsPressed( INPUT_PS2_BTN_CIRCLE ) )
    {
        switch( x_irand( 1, 5 ) )
        {
            case 1:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_RUN_STONE01 );
            break;
            case 2:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_RUN_STONE02 );
            break;
            case 3:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_RUN_STONE03 );
            break;
            case 4:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_RUN_STONE04 );
            break;
            case 5:
                ChannelID = audio_Play( SFX_PLAYER_HUMAN_RUN_STONE05 );
            break;
        };

    }
    else if( input_IsPressed( INPUT_PS2_BTN_TRIANGLE ) )
    {
        switch( x_irand( 1, 2 ) )
        {
            case 1:
                ChannelID = audio_Play( SFX_WEAPONS_GRENADE_EXPLOSION01 );
            break;
            case 2:
                ChannelID = audio_Play( SFX_WEAPONS_GRENADE_EXPLOSION03 );
            break;
        }
    }
    else if( input_IsPressed( INPUT_PS2_BTN_SQUARE ) )
    {
        switch( x_irand( 1, 5 ) )
        {
            case 1:
                ChannelID = audio_Play( SFX_WEAPONS_GAUSSFIRE01 );
            break;
            case 2:
                ChannelID = audio_Play( SFX_WEAPONS_GAUSSFIRE02 );
            break;
            case 3:
                ChannelID = audio_Play( SFX_WEAPONS_GAUSSFIRE03 );
            break;
            case 4:
                ChannelID = audio_Play( SFX_WEAPONS_GAUSSFIRE04 );
            break;
            case 5:
                ChannelID = audio_Play( SFX_WEAPONS_GAUSSFIRE05 );
            break;
        };
    }

    else if( input_IsPressed( INPUT_PS2_BTN_L1 ) )
    {
        ChannelID = audio_Play( SFX_WEAPONS_GAUSS_RELOAD );
    }
    else if( input_IsPressed( INPUT_PS2_BTN_L2 ) )
    {
        ChannelID = audio_Play( SFX_WEAPONS_GAUSS_GRENADE_RELOAD );
    }

    else if( input_IsPressed( INPUT_PS2_BTN_R1 ) )
    {
        switch( x_irand( 1, 3 ) )
        {
            case 1:
                ChannelID = audio_Play( SFX_WEAPONS_GAUSS_BULLET_IMP_STONE01 );
            break;
            case 2:
                ChannelID = audio_Play( SFX_WEAPONS_GAUSS_BULLET_IMP_STONE02 );
            break;
            case 3:
                ChannelID = audio_Play( SFX_WEAPONS_GAUSS_BULLET_IMP_STONE03 );
            break;
        }
    }
    else if( input_IsPressed( INPUT_PS2_BTN_R2 ) )
    {
        switch( x_irand( 1, 3 ) )
        {
            case 1:
                ChannelID = audio_Play( SFX_WEAPONS_GRENADE_BOUNCE_STONE01 );
            break;
            case 2:
                ChannelID = audio_Play( SFX_WEAPONS_GRENADE_BOUNCE_STONE02 );
            break;
            case 3:
                ChannelID = audio_Play( SFX_WEAPONS_GRENADE_BOUNCE_STONE03 );
            break;
        }

    }

}