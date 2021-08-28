//=============================================================================
//
//  Area 51 Main Program 
//  
//==============================================================================
// 
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
#include "Entropy.hpp"  
#include "Main.hpp"  
#include "ResourceMgr\ResourceMgr.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Render\Render.hpp"
#include "Objects\Player.hpp"  
#include "Objects\Corpse.hpp"
#include "Objects\LevelSettings.hpp"
#include "Objects\PlaySurface.hpp"
#include "Objects\Render\PostEffectMgr.hpp"
#include "Gamelib\Level.hpp"
#include "Gamelib\binLevel.hpp"
#include "Gamelib\Link.hpp"
#include "Gamelib\LevelLoader.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "inputmgr\inputmgr.hpp"
#include "IOManager\io_mgr.hpp"
#include "Audio\audio_stream_mgr.hpp"
#include "Audio\audio_hardware.hpp"
#include "x_files\x_context.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"
#include "..\Support\TriggerEx\TriggerEx_Manager.hpp"
#include "..\Support\Tracers\TracerMgr.hpp"
#include "..\Support\Render\LightMgr.hpp"
#include "navigation\nav_map.hpp"
#include "navigation\ng_connection2.hpp"
#include "navigation\ng_node2.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "PlaySurfaceMgr\PlaySurfaceMgr.hpp"
#include "gamelib\StatsMgr.hpp" 
#include "objects\ParticleEmiter.hpp"
#include "Menu\DebugMenu2.hpp"
#include "Music_Mgr\Music_mgr.hpp"
#include "MusicStateMgr\MusicStateMgr.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "StringMgr\StringMgr.hpp"
#include "GameTextMgr\GameTextMgr.hpp"
#include "Objects\SpawnPoint.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Templatemgr\TemplateMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Decals\DecalMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "DataVault\DataVault.hpp"
#include "TweakMgr\TweakMgr.hpp"
#include "PainMgr\PainMgr.hpp"
#include "PhysicsMgr\PhysicsMgr.hpp"
#include "Objects\AlienGlob.hpp"
#include "NetworkMgr\MsgMgr.hpp"
#include "Debris\debris_mgr.hpp"
#include "Audio\audio_voice_mgr.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"
#include "CheckPointMgr\CheckPointMgr.hpp"
#include "Objects\HudObject.hpp"
#include "NetworkMgr\Voice\VoiceMgr.hpp"

// frontend/ui includes
#include "UI\ui_manager.hpp"
#include "UI\ui_Font.hpp"
#include "StateMgr\StateMgr.hpp"

#include "MemCardMgr/MemCardMgr.hpp"

#include "Config.hpp"
#include "Configuration/GameConfig.hpp"

#if !defined(X_RETAIL) && !defined(TARGET_PC)
#include "InputMgr\GamePad.hpp"
#endif

#if (defined(ctetrick)||defined(ksaffel)) && defined(TARGET_DEV) && defined(TARGET_PS2)
#   include <libscf.h>
#endif

#ifdef TARGET_XBOX
#   ifndef CONFIG_RETAIL
#       include <D3d8perf.h>
#   endif
extern xbool g_b480P;
#endif


//=============================================================================
//  CONSTANTS
//=============================================================================

#define RELEASE_PATH            "C:\\GameData\\A51\\Release"

#if CONFIG_IS_DEMO
xtimer g_DemoIdleTimer;
#endif

//=============================================================================

#ifndef WIN32
#if ENABLE_RENDER_STATS
extern render::stats s_RenderStats;
#endif
#endif

struct profile
{
    s32     Rate;
    s32     DoPrint;
};

//=============================================================================
//  GLOBALS
//=============================================================================

xbool       g_bMemReports           = FALSE;
#if (!CONFIG_IS_DEMO)
xbool       g_FreeCam               = FALSE;
#endif
xbool       g_FreeCamPause          = TRUE;
xbool       g_MagentaColor          = FALSE;

xbool       g_RenderBoneBBoxes      = FALSE;

#if !defined( CONFIG_RETAIL )
xbool       g_AimAssist_Render_Reticle      = FALSE;
xbool       g_AimAssist_Render_Bullet       = FALSE;
xbool       g_AimAssist_Render_Turn         = FALSE;
xbool       g_AimAssist_Render_Bullet_Angle = FALSE;
xbool       g_AimAssist_Render_Player_Pills = FALSE;
#endif // !defined( CONFIG_RETAIL )

#if !defined( X_RETAIL ) && !defined(ctetrick)
xbool       g_bControllerCheck              = FALSE;
#else
xbool       g_bControllerCheck              = TRUE;
#endif

s32         g_MemoryLowWater = 0x7fffffff;
view        g_View;
u32         g_nLogicFramesAfterLoad =  0;
xtimer      g_GameTimer;

char        g_FullPath [ 256 ];

#if defined( ENABLE_DEBUG_MENU )
stats       g_Stats;
xbool       g_GameLogicDebug = FALSE;
xbool       g_DevWantsToSave = FALSE;
xbool       g_DevWantsToLoad = FALSE;
#endif // defined( ENABLE_DEBUG_MENU )

xbool       g_MirrorWeapon   = FALSE;
xbool       g_game_running   = TRUE;
xbool       g_first_person   = TRUE;

s32             g_Difficulty = 1; // Start out on Medium difficulty
const char* DifficultyText[] = { "Easy", "Medium", "Hard" };

#if !defined( X_RETAIL ) || defined( X_QA )
extern f32   g_TimeDilationFactor;
extern xbool g_RenderFrameRateInfo;
extern s32   g_ScreenShotSize;
extern xbool g_ScreenShotModeEnabled;
#endif

xbool   g_right_stick_swap_xy = FALSE;

xbool   g_bBloodEnabled    = TRUE;
xbool   g_bRagdollsEnabled = TRUE;

#if !defined( CONFIG_RETAIL )
xbool   g_CmdLineAutoServer = FALSE;
xbool   g_CmdLineAutoClient = FALSE;
xbool   g_CmdLineRTFHandler = FALSE;
s32     g_CmdLineLanguage   = -1;

f32     g_WorldTimeDilation = 1.0f;
#endif

//=============================================================================
//  STATICS
//=============================================================================

//=============================================================================
//  PROTOTYPES
//=============================================================================

void    LoadCamera  ( void );
void    SaveCamera  ( void );
void    Render      ( void );


//=============================================================================
//  PLATFORM SPECIFIC
//=============================================================================

#ifdef TARGET_PC
#include "main_pc.inl"
#endif

#ifdef TARGET_PS2
#include "main_ps2.inl"
#endif

#ifdef TARGET_XBOX
#include "main_xbox.inl"
#endif

//=============================================================================
//  FUNCTIONS
//=============================================================================

#if defined(TARGET_PS2) && !defined(X_RETAIL)
#include "Objects\Actor\Actor.hpp"

void DoDebugSuicide( void )
{
    slot_id ID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
    actor* pPlayer = (actor*)g_ObjMgr.GetObjectBySlot( ID );
    if( pPlayer )
        pPlayer->DebugSuicide();
}

void DoNukeEnemies( void )
{
    slot_id ID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
    actor* pPlayer = (actor*)g_ObjMgr.GetObjectBySlot( ID );
    if( !pPlayer )
        return;

    actor* pActor = actor::m_pFirstActive;
    while( pActor )
    {
        actor* pNext = pActor->m_pNextActive;
        if( pActor != pPlayer )
        {
            if( !pPlayer->IsFriendlyFaction( SMP_UTIL_GetFactionForGuid( pActor->GetGuid() ) ) )
            {
                if( (pPlayer->GetPosition() - pActor->GetPosition()).Length() <= 3000 )
                    pActor->DebugSuicide();
            }
        }

        pActor = pNext;
    }
}

void ReloadTweakTables( void )
{
    UnloadPain();
    UnloadTweaks();
    LoadTweaks(g_FullPath);
    LoadPain(g_FullPath);
}

#endif

//=============================================================================

xbool HandleInput( f32 DeltaTime )
{
    CONTEXT( "HandleInput" );
    g_InputMgr.Update( DeltaTime );

    // Check for exit message
    if( input_IsPressed( INPUT_MSG_EXIT ) )
        return( FALSE );

#if defined( ENABLE_DEBUG_MENU )
    if( g_DebugMenu.IsActive() )
    {
        return( TRUE );
    }
#endif // defined( ENABLE_DEBUG_MENU )

    static s32 FreeCamDebounce = 0;

    #ifdef TARGET_XBOX
    {
        #if defined( ENABLE_DEBUG_MENU )
        if( input_IsPressed ( INPUT_XBOX_BTN_BACK ) && 
            input_WasPressed( INPUT_XBOX_BTN_START ) &&
            g_StateMgr.IsPaused() != TRUE ) 
        {
            g_DebugMenu.Enable();
            g_InputMgr.Update( DeltaTime ); // do the update again, otherwise, the update will drop out.
            return( TRUE );
        }
        #endif // defined( ENABLE_DEBUG_MENU )

        if( !g_StateMgr.InSystemError() )
        {
            // Player has hit the back button.. trigger off the objective text.
            if( input_IsPressed(INPUT_XBOX_BTN_BACK) && !g_StateMgr.IsPaused() && ( GameMgr.GetGameType() == GAME_CAMPAIGN ) ) 
            {
                slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

                if( SlotID != SLOT_NULL )
                {
                    object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
                    hud_object& Hud = hud_object::GetSafeType( *pObj );

                    if( !Hud.m_Initialized )
                    {
                        return FALSE;
                    }

                    Hud.RenderObjectiveText();
                }
            }        

            // Check for pause
            s32 PausingController;

            PausingController = g_InputMgr.WasPausePressed();

            if( PausingController != -1 )
            {
                // Toggle the pause state.
                g_StateMgr.SetPaused( !g_StateMgr.IsPaused(), PausingController );
            #if CONFIG_IS_DEMO
                g_DemoIdleTimer.Reset();
                g_DemoIdleTimer.Start();
            #endif
            }

            //  Check for pulled controllers.
            g_StateMgr.CheckControllers();
        }
    }
    #else // end ifdef TARGET_XBOX
    {
    #if defined( TARGET_PC )
        if( input_WasPressed( INPUT_KBD_GRAVE ) )
        {
            FreeCamDebounce++;
            if( FreeCamDebounce == 1 )
            {
                g_FreeCam ^= 1;
                if( !g_FreeCam )
                {
                    // Move the player.
                    player* pPlayer = SMP_UTIL_GetActivePlayer();
                    if( pPlayer )
                    {
                        vector3 vPos = g_View.GetPosition();
                        pPlayer->OnExitFreeCam( vPos );
                    }
                }

            }
        }
        else
    #endif // defined(TARGET_PC)
        {
            FreeCamDebounce = 0;
        }
        
    #if defined( ENABLE_DEBUG_MENU )
        if( input_IsPressed(INPUT_PS2_BTN_SELECT) && 
            input_WasPressed(INPUT_PS2_BTN_START ) &&
            g_StateMgr.IsPaused() != TRUE ) 
        {
            g_DebugMenu.Enable();
            g_InputMgr.Update( DeltaTime ); // do the update again, otherwise, the update will drop out.
            return( TRUE );
        }
        
        #if !defined(X_RETAIL) && !defined(TARGET_PC)
        // if screenshot enabled, take screenshot on L3 + Select or CROSS on controller 2
        if( (g_ScreenShotModeEnabled == TRUE) &&
            (!eng_ScreenShotActive()) &&
            (g_StateMgr.IsPaused() != TRUE) &&
            (g_NetworkMgr.GetLocalPlayerCount() == 1) &&
            (   (   input_IsPressed(INPUT_PS2_BTN_L_STICK, 0) 
                 && input_IsPressed(INPUT_PS2_BTN_R_STICK, 0)) 
              ||    input_IsPressed(INPUT_PS2_BTN_CROSS,   1) ) )
        {
            eng_ScreenShot(DEBUG_SCREEN_SHOT_DIR, g_ScreenShotSize + 1);

            // these reset the logical inputs associated with the button.
            player* pPlayer = SMP_UTIL_GetActivePlayer();
            if( pPlayer )
            {
                g_IngamePad[pPlayer->GetActivePlayerPad()].SetLogical( INPUT_PLATFORM_PS2, INPUT_PS2_BTN_L_STICK );
                g_IngamePad[pPlayer->GetActivePlayerPad()].SetLogical( INPUT_PLATFORM_PS2, INPUT_PS2_BTN_R_STICK );
            }
            return (TRUE);
        }
        #endif // !defined(X_RETAIL) && !defined(TARGET_PC)

    #endif // defined( ENABLE_DEBUG_MENU )

        if( !g_StateMgr.InSystemError() )
        {
            // Player has hit the Select button.. trigger off the objective text.
            if( input_IsPressed(INPUT_PS2_BTN_SELECT) && !g_StateMgr.IsPaused() && ( GameMgr.GetGameType() == GAME_CAMPAIGN ) ) 
            {
                slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

                if( SlotID != SLOT_NULL )
                {
                    object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
                    hud_object& Hud = hud_object::GetSafeType( *pObj );

                    if( !Hud.m_Initialized )
                    {
                        return FALSE;
                    }

                    Hud.RenderObjectiveText();
                }
            }        

            // check for pause 
            s32 PausingController;

            PausingController = g_InputMgr.WasPausePressed();

            if( PausingController != -1 )
            {
                // Toggle the pause state.
                g_StateMgr.SetPaused( !g_StateMgr.IsPaused(), PausingController );
            #if CONFIG_IS_DEMO
                g_DemoIdleTimer.Reset();
                g_DemoIdleTimer.Start();
            #endif
            }

            //  Check for pulled controllers.
            g_StateMgr.CheckControllers();
        
        }   // !InSystemError()
    }
    #endif // end else defined( TARGET_XBOX )

    #if defined(TARGET_PS2) && !defined(X_RETAIL)
    if( input_IsPressed(INPUT_PS2_BTN_L2) &&
        input_IsPressed(INPUT_PS2_BTN_L_UP) &&
        input_WasPressed(INPUT_PS2_BTN_CROSS) )
    {
        DoDebugSuicide();
    }

    if( input_IsPressed(INPUT_PS2_BTN_L2) &&
        input_IsPressed(INPUT_PS2_BTN_L_UP) &&
        input_WasPressed(INPUT_PS2_BTN_CIRCLE) )
    {
        DoNukeEnemies();
    }

    if( input_IsPressed(INPUT_PS2_BTN_L2) &&
        input_IsPressed(INPUT_PS2_BTN_L_UP) &&
        input_WasPressed(INPUT_PS2_BTN_SELECT) )
    {
        ReloadTweakTables();
        x_printfxy(10,10,"TWEAKS_LOADED");
    }

    #endif

    // Handle any platform specific input requirements
    if( HandleInputPlatform( DeltaTime ) == FALSE )
        return( FALSE );
 
    return( TRUE );
}



//=============================================================================

void UpdateAudio( f32 DeltaTime )
{
    (void)DeltaTime;
    STAT_LOGGER( temp, k_stats_Sound );

    CONTEXT( "UpdateAudio" );

#ifdef AUDIO_ENABLE
    
    g_ConverseMgr.Update( DeltaTime );
    g_MusicStateMgr.Update();
    g_MusicMgr.Update( DeltaTime );
    g_AudioMgr.Update( DeltaTime );

#endif // AUDIO_ENABLE
}

//=============================================================================
//
//  Game Update Function
//
//=============================================================================

#if !defined(X_RETAIL)
xbool s_ForceGameComplete = FALSE;
#endif

static f32 GAME_MAX_DELTA_TIME = 0.1f ;     // .05f

void Update( f32 DeltaTime )
{
    CONTEXT( "Update" );
    DeltaTime = fMin( GAME_MAX_DELTA_TIME, DeltaTime );

#ifndef X_RETAIL
    g_PolyCache.Update();
#endif

    if( HandleInput( DeltaTime )==FALSE )
    {
        g_ActiveConfig.SetExitReason( GAME_EXIT_PLAYER_QUIT );
    }

#if !defined(X_RETAIL)
    if( s_ForceGameComplete )
    {
        g_ActiveConfig.SetExitReason( GAME_EXIT_GAME_COMPLETE );
        s_ForceGameComplete = FALSE;
    }
#endif

#if defined( ENABLE_DEBUG_MENU )
    // run debug menu
    g_DebugMenu.Update( DeltaTime );
#endif // defined( ENABLE_DEBUG_MENU )

    ASSERT( g_StateMgr.IsBackgroundThreadRunning()==FALSE );
    // run pause menu
    g_StateMgr.Update( DeltaTime );

    // should not pause the game logic for multiplayer or online games!
    if( 
#if (!CONFIG_IS_DEMO)
        (!g_FreeCamPause || (g_FreeCam == FALSE)) && 
#endif
#if defined( ENABLE_DEBUG_MENU )
        (g_DebugMenu.IsActive() == FALSE) &&
        (!eng_ScreenShotActive()) &&
#endif // defined( ENABLE_DEBUG_MENU )
        ( (g_StateMgr.IsPaused() == FALSE) || 
          ( g_NetworkMgr.IsOnline() == TRUE ) ) 
      )
    {
        g_nLogicFramesAfterLoad++;

        render::Update( DeltaTime );
        g_TracerMgr.OnUpdate( DeltaTime );
        
        // Limit dynamic dead bodies before advancing physics so that it doesn't get overloaded 
        // and or/run out of constraints
        corpse::LimitCount();
        
        g_PhysicsMgr.Advance( DeltaTime );
        {
            STAT_LOGGER( temp, k_stats_OnAdvance );
            g_ObjMgr.AdvanceAllLogic( DeltaTime );
        }
        g_LightMgr.OnUpdate( DeltaTime );
        g_PostEffectMgr.OnUpdate( DeltaTime );
        g_DecalMgr.OnUpdate( DeltaTime );
        g_AlienGlobMgr.Advance( DeltaTime );
    }

    if(
#if defined( ENABLE_DEBUG_MENU )
        (g_DebugMenu.IsActive() == FALSE) &&
        (!eng_ScreenShotActive()) &&
#endif // defined( ENABLE_DEBUG_MENU )
        ( ( g_StateMgr.IsPaused() == FALSE ) || 
          ( g_NetworkMgr.IsOnline() == TRUE ) )
      )
    { 
        g_TriggerExMgr.OnUpdate( DeltaTime );
    }

    g_NetworkMgr.Update( DeltaTime );
    g_GameTextMgr.Update( DeltaTime );
    UpdateAudio( DeltaTime );

    //handle save/load
    if ( g_BinLevelMgr.WantsToSave() )
    {
        g_BinLevelMgr.SaveRuntimeDynamicData();
        g_VarMgr.SaveRuntimeData();
    }
    else if ( g_BinLevelMgr.WantsToLoad() )
    {
        g_BinLevelMgr.LoadRuntimeDynamicData();
        g_VarMgr.LoadRuntimeData();
    }

#ifdef OLD_SAVE
    // check for save
    if( g_SaveMgr.WantsToSave() )
    {
        //g_SaveMgr.SaveGame();
        g_SaveMgr.CancelSave();
        // bring up the save dialog
        g_StateMgr.SetState( SM_SAVE_CAMPAIGN );
    }

    // check for load
    if( g_SaveMgr.WantsToLoad() )
    {
        g_SaveMgr.RestoreGame( TRUE );
    }
#endif
}

//=============================================================================

void SetupViewAndFog( zone_mgr::zone_id StartZone )
{
    CONTEXT( "SetupView" );

    texture::handle FogPalette;

    // get the default fog palette and z buffer settings
    xbool   QuickFog = FALSE;
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_LEVEL_SETTINGS );
    if ( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        ASSERT( pObject );
        level_settings& Settings = level_settings::GetSafeType( *pObject );
        g_View.SetZLimits( 10.0f, Settings.GetFarPlane() );

        FogPalette = Settings.GetFogPalette();
    }
    else
    {
        g_View.SetZLimits( 10.0f, 8000.0f );
    }

    // if the zone we're in has a different fog from the default, use
    // that one instead
    const char* pFog = g_ZoneMgr.GetZoneFog(StartZone,QuickFog);
    if ( *pFog != '\0' )
    {
        FogPalette.SetName( pFog );
    }

    // set the pixel scale (aspect ratio)
#ifndef X_RETAIL
    if ( eng_ScreenShotActive() )
    {   
        g_View.SetPixelScale( 1.0f );
    }
    else
#endif // X_RETAIL
    {
    #ifndef TARGET_XBOX
        g_View.SetPixelScale();
    #endif
    }

    // Set the viewport
    eng_SetView( g_View );

    // Set the fog
    render::SetCustomFogPalette( FogPalette, QuickFog, g_RenderContext.LocalPlayerIndex );
}

//=============================================================================
//
//  Game Render Function
//
//=============================================================================

void RenderGame( void )
{
    s32 i;

    CONTEXT( "Render" );
    LOG_STAT( k_stats_OtherRender );

    ASSERT( g_StateMgr.IsBackgroundThreadRunning() == FALSE );

    // Set background clear color
#if (!CONFIG_IS_DEMO)
    if(g_MagentaColor)
    {

        static int count = 0;
        count++;
        if( count%2 )
            eng_SetBackColor( XCOLOR_BLACK );
        else
            eng_SetBackColor( xcolor(255,0,255) );
    }
    else
#endif
    {
        eng_SetBackColor( XCOLOR_BLACK );
    }

    // Make sure we have full access to the framebuffer so that the splitscreen 
    // cleansing rects can render properly
    {
        view TempView;
        {
            eng_MaximizeViewport( TempView );
            eng_SetViewport     ( TempView );        
        }
    }
    
    // Get pointers to each of the players, we'll need them for setting up the
    // viewports.

    player* pPlayers[MAX_LOCAL_PLAYERS] = { 0 };
    slot_id ID                          = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
    s32     nPlayers                    = 0;

    while( ID != SLOT_NULL )
    {
        ASSERT( nPlayers < g_ActiveConfig.GetPlayerCount() );

        object* pObj    = g_ObjMgr.GetObjectBySlot(ID);
        player* pPlayer = &player::GetSafeType( *pObj );

        if( pPlayer && (pPlayer->GetLocalSlot() != -1) )
        {
            pPlayers[ pPlayer->GetLocalSlot() ] = pPlayer;
            nPlayers++;
        }

        ID = g_ObjMgr.GetNext(ID);
    }
    
    // If we don't have all of the local players yet, then we don't want to try
    // to render.  So, just return.
    if( nPlayers != g_NetworkMgr.GetLocalPlayerCount() )
        return;

    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    switch( nPlayers )
    {
        case 0:
        default:
            break;
    
        case 1:
        if ( MAX_LOCAL_PLAYERS >= nPlayers )
        {
            // one view, set it to the entire screen
            view& rView0 = pPlayers[0]->GetView();
            rView0.SetViewport( 0, 0, XRes, YRes );
            pPlayers[0]->ComputeView( rView0 );
            #ifdef TARGET_XBOX
            // High definition needs to adjust the aspect ratio.
            if( g_b480P )
                rView0.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
            #endif
        }
        break;

        case 2:
        if ( MAX_LOCAL_PLAYERS >= nPlayers )
        {
            // two views, set them to a horizontal split
            view& rView0 = pPlayers[0]->GetView();
            view& rView1 = pPlayers[1]->GetView();
            rView0.SetViewport( 0,0       ,XRes,YRes/2-1 ); // top
            rView1.SetViewport( 0,YRes/2+1,XRes,YRes     ); // bottom
            pPlayers[0]->ComputeView( rView0 );
            pPlayers[1]->ComputeView( rView1 );

            #ifdef TARGET_XBOX
            // High definition needs to adjust the aspect ratio.
            if( g_b480P )
            {
                rView0.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
                rView1.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
            }
            #endif

            if( !g_StateMgr.IsPaused() && eng_Begin( "Kill last rect" ) )
            {
                draw_Rect( irect( 0,YRes/2-2,XRes,YRes/2+2 ),XCOLOR_BLACK,FALSE );
                eng_End( );
            }
        }
        break;

        case 3:
        if ( MAX_LOCAL_PLAYERS >= nPlayers )
        {
            // four views, set them to a 4-way split
            view& rView0 = pPlayers[0]->GetView();
            view& rView1 = pPlayers[1]->GetView();
            view& rView2 = pPlayers[2]->GetView();
            rView0.SetViewport( 0       ,0       ,XRes/2-1,YRes/2-1 );   // upper-left
            rView1.SetViewport( XRes/2+1,0       ,XRes    ,YRes/2-1 );   // upper-right
            rView2.SetViewport( 0       ,YRes/2+1,XRes    ,YRes     );   // bottom

            #ifdef TARGET_XBOX
            // High definition needs to adjust the aspect ratio.
            if( g_b480P )
            {
                rView0.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
                rView1.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
                rView2.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
            }
            #endif

            pPlayers[0]->ComputeView( rView0 );
            pPlayers[1]->ComputeView( rView1 );
            pPlayers[2]->ComputeView( rView2 );

            if( !g_StateMgr.IsPaused() && eng_Begin( "Kill last rect" ) )
            {
                //draw_Rect( irect( XRes/2  ,YRes/2  ,XRes    ,YRes     ),XCOLOR_BLACK,FALSE ); // missing square
                draw_Rect( irect( 0       ,YRes/2-2,XRes    ,YRes/2+2 ),XCOLOR_BLACK,FALSE ); // horizontal line
                draw_Rect( irect( XRes/2-2,0       ,XRes/2+2,YRes/2+2 ),XCOLOR_BLACK,FALSE ); // vertical line
                eng_End( );
            }
        }
        break;

        case 4:
        if ( MAX_LOCAL_PLAYERS >= nPlayers )
        {
            // four views, set them to a 4-way split
            view& rView0 = pPlayers[0]->GetView();
            view& rView1 = pPlayers[1]->GetView();
            view& rView2 = pPlayers[2]->GetView();
            view& rView3 = pPlayers[3]->GetView(); 

            rView0.SetViewport( 0       ,0,       XRes/2-1,YRes/2-1 );   // upper-left
            rView1.SetViewport( XRes/2+1,0,       XRes    ,YRes/2-1 );   // upper-right
            rView2.SetViewport( 0       ,YRes/2+1,XRes/2-1,YRes     );   // lower-left
            rView3.SetViewport( XRes/2+1,YRes/2+1,XRes    ,YRes     );   // lower-right

            #ifdef TARGET_XBOX
            // High definition needs to adjust the aspect ratio.
            if( g_b480P )
            {
                rView0.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
                rView1.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
                rView2.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
                rView3.SetPixelScale( (9.0f/16.0f)*(640.0f/480.0f) );
            }
            #endif

            pPlayers[0]->ComputeView( rView0 );
            pPlayers[1]->ComputeView( rView1 );
            pPlayers[2]->ComputeView( rView2 );
            pPlayers[3]->ComputeView( rView3 );

            if( !g_StateMgr.IsPaused() && eng_Begin( "Kill last rect" ) )
            {
                draw_Rect( irect( 0       ,YRes/2-2,XRes    ,YRes/2+2 ),XCOLOR_BLACK,FALSE ); // horizontal line
                draw_Rect( irect( XRes/2-2,0       ,XRes/2+2,YRes     ),XCOLOR_BLACK,FALSE ); // vertical line
                eng_End( );
            }
        }
        break;
    }

    // Make all the players inactive in anticipation of the render...
    for( i = 0; i < nPlayers; i++ )
        pPlayers[i]->SetAsActivePlayer( FALSE );

    for( i = 0; i < nPlayers; i++ )
    {
        g_RenderContext.Set( i,                                 // Local slot.
                             pPlayers[i]->net_GetSlot(),        // Net slot.
                             pPlayers[i]->net_GetTeamBits(), 
                             pPlayers[i]->IsMutantVisionOn(),
                             FALSE );

        // set this player as the active one
        pPlayers[i]->SetAsActivePlayer( TRUE );

#if (!CONFIG_IS_DEMO)
        if ( g_FreeCam == FALSE )
#endif
        {
            view& rView0 = pPlayers[i]->GetView();
            pPlayers[i]->ComputeView( rView0, player::VIEW_NULL );
            g_View = rView0;
        }

        SetupViewAndFog(pPlayers[i]->GetPlayerViewZone());

        // Perform any platform specific render initialization
        InitRenderPlatform();

        // render all objects
        g_ObjMgr.Render(TRUE,g_View,pPlayers[i]->GetPlayerViewZone());

        EndRenderPlatform();
        pPlayers[i]->SetAsActivePlayer( FALSE );
    }

    // Make all the players active again so their input will function.
    for( i = 0; i < nPlayers; i++ )
        pPlayers[i]->SetAsActivePlayer( TRUE );

#if defined( ENABLE_DEBUG_MENU )
    // Debug menu rendering
    g_DebugMenu.Render();
#endif // defined( ENABLE_DEBUG_MENU )
}

xbool SHOW_STREAM_INFO    = FALSE;
xbool SHOW_AUDIO_LEVELS   = FALSE;
xbool SHOW_AUDIO_CHANNELS = FALSE;

#if !defined(X_RETAIL) || defined(X_QA)

f32 g_PeakTime      = 0.0f;
f32 PEAK_HOLD_TIME  = 1.0f;
f32 g_ClipTime      = 0.0f;
f32 CLIP_HOLD_TIME  = 3.0f;
xbool g_Clipped     = FALSE;
s32   g_Peak        = 0;
s32   g_ChannelPeak = 0;
f32   g_ChannelTime = 0.0f;
f32   CHANNEL_PEAK_HOLD_TIME = 5.0f;
f32   CHANNEL_PEAK_CLIP_TIME = 8.0f;
s32   g_bChannelFlash = 0;
char* g_ChannelText[48];
f32   g_ChannelVolume[48];
s32   g_x = 5;
s32   g_y = 100;
s32   g_limit = 30;

void AudioStats( f32 DeltaTime )
{
    (void)DeltaTime;
#if defined(TARGET_PS2)
    irect Rect;
    irect VolumeRect;
    s32 XRes,YRes;

    if( SHOW_AUDIO_CHANNELS )
    {
        g_AudioHardware.Lock();
        channel* pChannel = g_AudioHardware.GetChannelBuffer();
        for( s32 i=0 ; i<48 ; i++, pChannel++ )
        {
            g_ChannelText[i]   = "*";
            g_ChannelVolume[i] = 0.0f;

            if( pChannel->Hardware.ChannelId != -1 )
            {
                if( pChannel->pElement && pChannel->pElement->pVoice )
                {
                    g_ChannelText[i]   = pChannel->pElement->pVoice->pDescriptorName;
                    g_ChannelVolume[i] = pChannel->Volume;
                    
                }
            }
        }
        g_AudioHardware.Unlock();

        eng_GetRes(XRes,YRes);

        s32 x = 5;
        s32 y = 20;
        s32 font = g_UiMgr->FindFont("small");

        //eng_Begin( "Channel Debug" );

        Rect.Set( x, y, x + 500, y + ((g_UiMgr->GetLineHeight(font)-0) * 24) );
        draw_Rect( Rect, xcolor(0,0,0,128), FALSE );

        for( s32 i=0 ; i<24 ; i++ )
        {
            if( g_ChannelVolume[i] > 0.005f )
            {
                VolumeRect.Set( x+20, y+15, x+20+(230*g_ChannelVolume[i]), y+17 );
                draw_Rect( VolumeRect, XCOLOR_RED, FALSE );
            }

            char buff[256];
            x_sprintf( buff, "%02d %s", i, g_ChannelText[i] );
            buff[28] = 0;
            xwstring Text1 = (const char*)buff;
            g_UiMgr->TextSize( font, Rect, Text1, Text1.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, XCOLOR_WHITE, Text1 );
            y += g_UiMgr->GetLineHeight(font)-0;
        }

        x = 260;
        y = 20;

        for( s32 i=24 ; i<48 ; i++ )
        {
            if( g_ChannelVolume[i] > 0.005f )
            {
                VolumeRect.Set( x+20, y+15, x+20+(230*g_ChannelVolume[i]), y+17 );
                draw_Rect( VolumeRect, XCOLOR_RED, FALSE );
            }

            char buff[256];
            x_sprintf( buff, "%02d %s", i, g_ChannelText[i] );
            buff[28] = 0;
            xwstring Text1 = (const char*)buff;
            g_UiMgr->TextSize( font, Rect, Text1, Text1.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, XCOLOR_WHITE, Text1 );

            y += g_UiMgr->GetLineHeight(font)-0;
        }

        //eng_End();
    }

    if( SHOW_STREAM_INFO )
    {
        eng_GetRes(XRes,YRes);

        s32 x = 60;
        s32 y = YRes - 130;
        s32 font = g_UiMgr->FindFont("small");

        //eng_Begin( "Stream Debug" );

        Rect.Set( x, y, x + 450, y + (g_UiMgr->GetLineHeight(font) * MAX_AUDIO_STREAMS) );
        draw_Rect( Rect, xcolor(0,0,0,128), FALSE );

        // For every stream...
        for( s32 i=0 ; i<MAX_AUDIO_STREAMS ; i++ )
        {
            audio_stream* pStream = &g_AudioStreamMgr.m_AudioStreams[ i ];
            char Type[64];
            char Channel[2][64];
            char ChannelState[2][64];
            char Time[64];

            char*  pName  = NULL;
            voice* pVoice = NULL;

            if( pStream->pChannel[0] && (pStream->Type != INACTIVE) )
            {
                x_sprintf( Channel[0], "%02d", pStream->pChannel[0] - g_AudioHardware.GetChannelBuffer() );
                switch( pStream->pChannel[0]->State )
                {
                case STATE_NOT_STARTED: strcpy( ChannelState[0], " WAITN" ); break;
                case STATE_STARTING:    strcpy( ChannelState[0], "STARTN" ); break;
                case STATE_RESUMING:    strcpy( ChannelState[0], "RESUME" ); break;
                case STATE_RUNNING:     strcpy( ChannelState[0], "RUNNIN" ); break;
                case STATE_PAUSING:     strcpy( ChannelState[0], "PAUSEN" ); break;
                case STATE_PAUSED:      strcpy( ChannelState[0], "PAUSED" ); break;
                default:                strcpy( ChannelState[0], "******" ); break;
                }

                if( pStream->pChannel[0]->pElement )
                {
                    pVoice = pStream->pChannel[0]->pElement->pVoice;
                }
            }
            else
            {
                x_strcpy( Channel[0], "" );
                strcpy( ChannelState[0], "" );
            }

            if( pStream->pChannel[1] && (pStream->Type != INACTIVE) )
            {
                x_sprintf( Channel[1], "%2d", pStream->pChannel[1] - g_AudioHardware.GetChannelBuffer() );
                switch( pStream->pChannel[1]->State )
                {
                    case STATE_NOT_STARTED: strcpy( ChannelState[1], " WAITN" ); break;
                    case STATE_STARTING:    strcpy( ChannelState[1], "STARTN" ); break;
                    case STATE_RESUMING:    strcpy( ChannelState[1], "RESUME" ); break;
                    case STATE_RUNNING:     strcpy( ChannelState[1], "RUNNIN" ); break;
                    case STATE_PAUSING:     strcpy( ChannelState[1], "PAUSEN" ); break;
                    case STATE_PAUSED:      strcpy( ChannelState[1], "PAUSED" ); break;
                    default:                strcpy( ChannelState[1], "******" ); break;
                }
            }
            else
            {
                x_strcpy( Channel[1], "" );
                strcpy( ChannelState[1], "" );
            }

            switch( pStream->Type )
            {
                case INACTIVE:
                    x_strcpy( Type, "0" );
                    break;

                case MONO_STREAM:
                    x_strcpy( Type, "1" );
                    break;

                case STEREO_STREAM:
                    x_strcpy( Type, "2" );
                    break;

                default:
                    ASSERT( 0 );
                    break;
            }

            if( pVoice && (pStream->Type!=INACTIVE) )
            {
                f32 time = g_AudioVoiceMgr.GetCurrentPlayTime( pVoice );
                x_sprintf( Time, "%06.03f", time );
                pName = pVoice->pDescriptorName;
            }
            else
            {
                x_strcpy( Time, "00.000" );
                pName = "";
            }

            xwstring Text1 = (const char*)xfs( "%s %s %s [%s/%s %s/%s]", 
                Type, Time, pName, Channel[0], ChannelState[0], Channel[1], ChannelState[1] );
            g_UiMgr->TextSize( font, Rect, Text1, Text1.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text1 );
            
            y += g_UiMgr->GetLineHeight(font);
        }
        //eng_End();
    }

    if( SHOW_AUDIO_LEVELS )
    {
        extern volatile s32 g_ActiveHardwareChannels;
        s32 nChannels = g_ActiveHardwareChannels;
        s32 font = g_UiMgr->FindFont("large");
        s32 x;
        s32 y;
        s32 i;
       
        //eng_Begin( "Audio Levels" );
        eng_GetRes(XRes,YRes);

        g_ChannelTime += DeltaTime;
        if( g_ChannelTime > CHANNEL_PEAK_HOLD_TIME )
        {
            if( g_ChannelPeak >= 48 )
            {
                if( g_ChannelTime > CHANNEL_PEAK_CLIP_TIME )
                {
                    g_ChannelPeak = 0;
                }
            }
            else
            {
                g_ChannelPeak = 0;
            }
        }

        if( nChannels > g_ChannelPeak )
        {
            g_ChannelPeak = nChannels;
            g_ChannelTime = 0.0f;
        }

        x = 30;
        y = YRes - 90;

        Rect.Set( x, y, x+21, y+21 );
        xwstring Text1 = (const char*)xfs( "%d", nChannels );
        g_UiMgr->TextSize( font, Rect, Text1, Text1.GetLength());
        Rect.Translate(x, y);
        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text1 );

        y -= g_UiMgr->GetLineHeight(font);

        Rect.Set( x, y, x+21, y+21 );
        xwstring Text2 = (const char*)xfs( "%d", g_ChannelPeak );
        g_UiMgr->TextSize( font, Rect, Text2, Text2.GetLength());
        Rect.Translate(x, y);
        if( g_ChannelPeak < 48 )
        {
            if( g_ChannelPeak < 40 )
            {
                g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text2 );
            }
            else
            {
                g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_RED), Text2 );
            }
        }
        else
        {
            g_bChannelFlash++;
            if( g_bChannelFlash > 20 )
                g_bChannelFlash = 0;
            if( g_bChannelFlash > 15 )
                g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text2 );
            else
                g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_RED), Text2 );
        }

        extern s32 GetAudioLevel( void );
        s32 AudioLevel = GetAudioLevel();
        g_PeakTime += DeltaTime;
        if( g_PeakTime >= PEAK_HOLD_TIME )
        {
            g_Peak = 0;
        }

        if( g_Clipped )
        {
            g_ClipTime += DeltaTime;
            if( g_ClipTime > CLIP_HOLD_TIME )
                g_Clipped = FALSE;
        }

        if( AudioLevel > g_Peak )
        {
            g_Peak     = AudioLevel;
            g_PeakTime = 0.0f;
        }
         
        if( AudioLevel >= 32000 )
        {
            g_Clipped  = TRUE;
            g_ClipTime = 0.0f;
        }

        // convert to range [0..32]
        s32 ScaledLevel = AudioLevel / 1024; 
        s32 PeakLevel   = g_Peak / 1024;

        x = 5;
        y = YRes - 20;

        Rect.Set( x-1, y-32*4-1-10, x+21, y+1 );
        draw_Rect( Rect, xcolor(0,0,0,128), FALSE );
        Rect.Set( x-1, y-32*4-1, x+21, y+1 );
        draw_Rect( Rect, xcolor(255,255,255,255), TRUE );

        xcolor c;

        for( i=0 ; i<ScaledLevel ; i++ )
        {
            s32 r;
            s32 g;

            if( i < 16 )
            {
                g = 255;
                r = 255 - ((16 - i) * 16);
            }
            else if( i > 16 )
            {
                g = 255 - ((i - 16) * 16);
                r = 255;
            }
            else
            {
                r = 255;
                g = 255;
            }

            if( g < 0 )
                g = 0;
            if( g > 255 )
                g = 255;
            if( r < 0 )
                r = 0;
            if( r > 255 )
                r = 255;

            c.Set( (u8)r, (u8)g, 0, 255 );
            Rect.Set( x, y-4*(i+1), x + 20, y-4*i );

            draw_Rect( Rect, c, FALSE );
        }

        if( PeakLevel > ScaledLevel )
        {
            i = PeakLevel;
            s32 r;
            s32 g;

            if( i < 16 )
            {
                g = 255;
                r = 255 - ((16 - i) * 16);
            }
            else if( i > 16 )
            {
                g = 255 - ((i - 16) * 16);
                r = 255;
            }
            else
            {
                r = 255;
                g = 255;
            }

            if( g < 0 )
                g = 0;
            if( g > 255 )
                g = 255;
            if( r < 0 )
                r = 0;
            if( r > 255 )
                r = 255;

            c.Set( (u8)r, (u8)g, 0, 255 );
            Rect.Set( x, y-4*(i+1), x + 20, y-4*i );
            draw_Rect( Rect, c, FALSE );
        }

        if( g_Clipped )
        {
            Rect.Set( x-1, y-32*4-1-10, x+21, y-32*4-1 );
            draw_Rect( Rect, xcolor(255,0,0,255), FALSE );
        }

        //eng_End();
    }

#endif // TARGET_PS2
}

#endif // X_RETAIL

//=============================================================================

#if defined( ENABLE_DEBUG_MENU )
void Stats( f32 DeltaTime )
{
    static f32 s_StatTimer = 0.0f;
    static s32 s_AmountFree = 0;
    static s32 s_LargestFree = 0;
    static s32 s_NFragments = 0;
    static s32 s_ObjectCount = 0;
    s_StatTimer += DeltaTime;

    xbool bPrint = FALSE;
    if( s_StatTimer > g_Stats.Interval )
    {
        s_StatTimer = 0.0f;

        #ifndef X_RETAIL
        if( g_Stats.EngineStats == TRUE )
        {
            eng_PrintStats();       
            bPrint = TRUE;
        }
        #endif

        #ifdef TARGET_PS2
        if ( g_Stats.PS2_MemStats == TRUE )
        {
            x_MemGetFree(s_AmountFree, s_LargestFree, s_NFragments );
            s_ObjectCount = 0;
            for (u32 i = object::TYPE_NULL + 1; i < object::TYPE_END_OF_LIST; i++)
            {
                s_ObjectCount += g_ObjMgr.GetNumInstances( (object::type)i );
            }
        }
        #endif
    }

    if( g_Stats.RenderStats == TRUE )
    {
        #if ENABLE_RENDER_STATS
        s32 Mode = render::stats::OUTPUT_TO_SCREEN;
        s32 Flags = 0;
        
        //if( g_Stats.RenderVerbose == TRUE )
        //    Flags = render::stats::FLAG_VERBOSE;

        render::GetStats().Print( Mode, Flags );
        bPrint = TRUE;
        #endif
    }
    #ifdef TARGET_PS2
    else if ( g_Stats.PS2_MemStats == TRUE )
    {
        x_printfxy(1,3,"Total Free Memory     = %d\n",s_AmountFree);
        x_printfxy(1,4,"Largest Free Block    = %d\n",s_LargestFree);
        x_printfxy(1,5,"Num Fragmented Blocks = %d\n",s_NFragments);
        x_printfxy(1,6,"Scratch Mem Max Used  = %d\n",smem_GetMaxUsed());
        x_printfxy(1,7,"Scratch Mem Free      = %d\n",smem_GetBufferSize()-smem_GetMaxUsed());
        x_printfxy(1,8,"Current Object Count  = %d\n",s_ObjectCount);
        x_printfxy(1,9,"Current Cell Count    = %d\n",g_SpatialDBase.GetCellCount());
        x_printfxy(1,10,"Audio Memory Free    = %d\n",g_AudioHardware.GetAudioRamFree());
        x_printfxy(1,11,"LowMemory Minimum    = %d\n",g_MemoryLowWater );
    }
    #endif

    if (bPrint)
    {
        PrintStatsPlatform();
    }

    #if ENABLE_STATS_MGR
    stats_mgr::GetStatsMgr()->OnGameUpdate(DeltaTime);

    if ( !eng_ScreenShotActive() )
    {
        if( eng_Begin( "StatsMgr" ) )
        {
            stats_mgr::GetStatsMgr()->DrawFPS();
            stats_mgr::GetStatsMgr()->DrawCPULegend();
            stats_mgr::GetStatsMgr()->DrawGPULegend();
            if( g_Stats.MemVertBars )
            {
                stats_mgr::GetStatsMgr()->DrawSmallBars();
                stats_mgr::GetStatsMgr()->DrawSmallBarLegend();
            }
            eng_End();
        }
    }
    #endif

}
#endif // !defined(X_RETAIL) || defined(CONFIG_PROFILE)

//=============================================================================

void SaveCamera( void )
{
    X_FILE* fp;
    
    const view* pView = eng_GetView();

    if( !(fp = x_fopen( xfs( "%s\\camera.dat", g_FullPath ), "wb" ))) ASSERT( FALSE );
    x_fwrite( pView, sizeof( view ), 1, fp );
    x_fclose( fp );
    x_DebugMsg( "Camera saved\n" );
}

//=============================================================================

void LoadCamera( void )
{
    X_FILE* fp;
    view    TheView;

    if( !(fp = x_fopen( xfs( "%s\\camera.dat", g_FullPath ), "rb" ))) return;
    x_fread( &TheView, sizeof( view ), 1, fp );
    g_View = TheView;
    x_fclose( fp );
    x_DebugMsg( "Camera loaded\n" );

    // Move the player.
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if ( pPlayer )
    {
        pPlayer->OnMoveFreeCam( g_View );
    }
}

/*
//=============================================================================
void ForceLoad( const char* pPath )
{

    X_FILE* file;  
    char* pFilenames;
    u32 Count = 0;
    xtimer deltatime;

    file = x_fopen(pPath, "rt" );
    if (!file)
        return;

    // WARNING! There WILL be a problem with this code should it be placed in a background thread
    // since it is using smem_Alloc and that is switched on eng_PageFlip(). But, smem_Toggle should
    // catch the fact there are still some markers pushed. SO BE AWARE!
    // Reason for this: Originally, this routine allocated 96K on the stack. That's really bad.
//    smem_StackPushMarker();

    deltatime.Start();

    if( file )
    {

        s32 actualSizeRead;
        actualSizeRead = x_flength(file);

        f32 pctPerRead;
        pctPerRead = (50.0f / actualSizeRead);

        pFilenames = (char*)x_malloc(actualSizeRead);
        ASSERT(pFilenames);

        actualSizeRead = x_fread(pFilenames,1,actualSizeRead,file );
        s32 currentFileIndex = 0;
        s32 lastFileIndex = 0;

        while (currentFileIndex <actualSizeRead )
        {
            s32 startingPoint = currentFileIndex;
            while( pFilenames[currentFileIndex] != '\n' &&  pFilenames[currentFileIndex] != 0 )
            {
                currentFileIndex++;
            }
            char    pDrive[256],
                    pDir[256],
                    pName[256], 
                    pExtension[256];

            pFilenames[currentFileIndex] = 0;
            if(pFilenames[currentFileIndex-1] == 0x0D )
            {
                pFilenames[currentFileIndex-1] = 0;
            }
            x_splitpath( &pFilenames[startingPoint],pDrive,pDir,pName,pExtension );

            char fullName[128];
            x_sprintf(fullName,"%s%s",pName,pExtension );
            //LOG_MESSAGE( "ForceLoad", "Filename: [%s]", fullName );
            rhandle_base aHandle;
            //if( x_stricmp(fullName, "wpn_smp_pnchthru_002315134400.xbmp")==0)
            //{
            //    BREAK;
            //}
            aHandle.SetName( fullName );
            aHandle.GetPointer();
                
            currentFileIndex++;
            Count++;

            g_NetworkMgr.Update(deltatime.TripSec());
            //g_UiMgr->AddPercentLoaded( pctPerRead * (currentFileIndex - lastFileIndex) );
            lastFileIndex = currentFileIndex;
        }
        x_fclose(file);
    }
    x_free(pFilenames);

    f32 pctPerItem;
    pctPerItem = (5.0f / Count);
}
*/

//=============================================================================
// returns a default language if the one given is not supported.
x_language CheckLanguageSupport( x_language lang )
{

#if defined (X_EDITOR)
    (void)lang;
    return XL_LANG_ENGLISH;
#else

    // temp fix for programmers
#if defined (TARGET_DEV)
    switch( lang ) 
    {
    case XL_LANG_ENGLISH:
    case XL_LANG_FRENCH:
    case XL_LANG_ITALIAN:
    case XL_LANG_SPANISH:
    case XL_LANG_GERMAN:
        return lang;
    default:
        return XL_LANG_ENGLISH;
    }
#else

    switch( x_GetTerritory() )
    {
        case XL_TERRITORY_AMERICA:
            return XL_LANG_ENGLISH;

        case XL_TERRITORY_EUROPE:
            switch( lang ) 
            {
                case XL_LANG_ENGLISH:
                case XL_LANG_FRENCH:
                case XL_LANG_ITALIAN:
                case XL_LANG_SPANISH:
                case XL_LANG_GERMAN:
                    return lang;
                default:
                    return XL_LANG_ENGLISH;
            }

        default:
            return XL_LANG_ENGLISH;
    }
#endif

#endif   // !defined (X_EDITOR)
}

//=============================================================================
#if defined(TARGET_XBOX) && defined(X_MEM_DEBUG)
void SetLoggerBounds( void );
#endif

void DoStartup( void )
{
    MEMORY_OWNER( "STARTUP" );

// TEST CODE - checks ps2 config.
#if (defined(ctetrick) || defined(ksaffel)) && defined(TARGET_DEV) && defined(TARGET_PS2) 
    // some generalized time values.
    // Dutch    - UTC + 1 (+60)
    // English  - UTC - 7 (-420)      (central)
    // German   - UTC + 1 (+60)
    // French   - UTC + 1 (+60)
    // Italian  - UTC + 1 (+60)
    // Japanese - UTC + 9 (+540)
    // Korean   - UTC + 9 (+540)
    // spanish  - UTC + 1 (+60)      (spain)
    // chinese  - UTC + 8 (+480)

    sceScfT10kConfig config;

    config.TimeZone         = -420;
    config.Aspect           = SCE_ASPECT_FULL;  
    if( x_GetTerritory() == XL_TERRITORY_EUROPE )
    {
        config.DateNotation     = SCE_DATE_DDMMYYYY;
        config.Language         = SCE_SPANISH_LANGUAGE;
    }
    else
    {
        config.DateNotation     = SCE_DATE_MMDDYYYY;
        config.Language         = SCE_ENGLISH_LANGUAGE;

        //config.Language         = SCE_SPANISH_LANGUAGE;        
        //config.Language         = SCE_GERMAN_LANGUAGE;
        //config.Language         = SCE_FRENCH_LANGUAGE;        
        //config.Language         = SCE_ITALIAN_LANGUAGE;

        //config.Language         = SCE_JAPANESE_LANGUAGE;
        //config.Language         = SCE_DUTCH_LANGUAGE;
        //config.Language         = SCE_PORTUGUESE_LANGUAGE;
        
    }
    config.Spdif            = SCE_SPDIF_OFF;
    config.SummerTime       = SCE_SUMMERTIME_ON;
    config.TimeNotation     = SCE_TIME_12HOUR;
    sceScfSetT10kConfig(&config);
#endif // ctetrick defined

    ForceLink();
  
    //
    // Initialize general systems.
    //

    x_DebugMsg( "Entered app.\n" );

    // get language setting and check for default language.
    x_language DefaultLanguage = x_GetConsoleLanguage();
    x_SetLocale( CheckLanguageSupport( DefaultLanguage ) );

#if !defined( CONFIG_RETAIL )
    if( g_CmdLineLanguage != -1 )
    {
        x_SetLocale( (x_language)g_CmdLineLanguage );
    }
#endif

    g_UIMemCardMgr.Init();

    eng_Init();
    guid_Init();

    // Enable rtf handler based on a command-line parameter.
    // By default, the rtf handler will be disabled. We need
    // to be fixing ASSERTs, not ignoring them.
    #if !defined(X_RETAIL) && !defined(X_QA)
    if( g_CmdLineRTFHandler )
    {
        extern void InstallRTFHandler( void );
        InstallRTFHandler();
    }
    #endif

    x_DebugMsg( "Initialize io system\n" );

    g_IoMgr.Init();

    // Mount the default file system.
    #ifdef TARGET_XBOX
    xtimer Timing;
    #   ifdef ENABLE_NETFS
            g_IoMgr.SetDevicePathPrefix( "C:\\GAMEDATA\\A51\\RELEASE\\XBOX\\DVD\\",IO_DEVICE_DVD );
    #   else
    #       if CONFIG_IS_DEMO
                g_IoMgr.SetDevicePathPrefix( "D:\\AREA51\\",IO_DEVICE_DVD );
    #       else
                g_IoMgr.SetDevicePathPrefix( "D:\\",IO_DEVICE_DVD );
    #       endif
    #   endif
    #endif

#if defined(TARGET_PS2)
    #if !defined(TARGET_DVD)
        g_IoMgr.SetDevicePathPrefix( xfs("host:%s\\%s\\DVD\\", RELEASE_PATH, PLATFORM_PATH),IO_DEVICE_DVD );
        //g_IoMgr.SetDevicePathPrefix( "cdrom0:\\AREA51\\",IO_DEVICE_DVD );
    #else
        g_IoMgr.SetDevicePathPrefix( xfs("cdrom0:\\", RELEASE_PATH, PLATFORM_PATH),IO_DEVICE_DVD );
    #endif
#endif
#if defined(TARGET_XBOX) && defined(X_MEM_DEBUG)
    SetLoggerBounds();
#endif

    #ifdef TARGET_XBOX

        // The legal screen is allowed to only be up for a maximum
        // of five seconds. The Xbox logo stays up until all previous
        // initialisations are complete, the legal comes up and waits
        // until the caching code transfers PRELOAD and BOOT to the
        // utility partition. This takes 3.5 seconds.

        extern void xbox_DrawXbmpFromSection( const char* );
        const char* SectionName;
        extern xbool g_bPAL;

        if( x_GetTerritory() == XL_TERRITORY_AMERICA )
        {
            SectionName="NTSC_Legal";
        }
        else
        {
            switch( DefaultLanguage )
            {
                case XL_LANG_FRENCH:
                    SectionName="PAL_Legal_F";
                    break;
                case XL_LANG_GERMAN:
                    SectionName="PAL_Legal_G";
                    break;
                case XL_LANG_ITALIAN:
                    SectionName="PAL_Legal_I";
                    break;
                case XL_LANG_SPANISH:
                    SectionName="PAL_Legal_S";
                    break;
                default:
                    SectionName="PAL_Legal_E";
                    break;
            }
        }
        xbox_DrawXbmpFromSection( SectionName );

        Timing.Reset();
        Timing.Start();

    #endif

    // Xbox: cache to the utility partition.
    g_LevelLoader.MountDefaultFilesystems();

    #ifdef TARGET_XBOX

        xbool ParentalControlFailed = FALSE;

        // This could potentially delay the legal screen too long but I
        // have to move the code here for the UI to be available for
        // the message.

        if( x_GetTerritory() == XL_TERRITORY_AMERICA )
        {
            switch( XGetParentalControlSetting() )
            {
                case XC_PC_ESRB_MATURE: // All games with an ESRB rating of Mature, or less, may be played. 
                case XC_PC_ESRB_ADULT : // All games with an ESRB rating of Adult, or less, may be played. 
                case XC_PC_ESRB_ALL:
                {
                    // Now we need to throw up the ESRB notice from out
                    // of the XBE section.

                    xbox_DrawXbmpFromSection( "ESRB" );
                    break;
                }

                default:
                    ParentalControlFailed = TRUE;
                    break;
            }
        }
        else
        {
            xbox_DrawXbmpFromSection( "PAL_NonESRB" );
        }

    #endif

    // Load up the global configuration options from the ini file.
    // This MUST happen AFTER the io system has been initialized. Otherwise we can't
    // load the config.ini file on a viewer build.
    #ifndef CONFIG_RETAIL
    g_Config.Load( xfs( "%s\\%s\\Config.ini", RELEASE_PATH, PLATFORM_PATH ) );
    if( g_CmdLineAutoClient )
        g_Config.AutoClient = TRUE;
    if( g_CmdLineAutoServer )
        g_Config.AutoServer = TRUE;
    #endif

    //
    // Fire up some of the major system managers.
    //

    g_ObjMgr.Init();
    g_SpatialDBase.Init( 400.0f );
    g_PostEffectMgr.Init();
    g_PlaySurfaceMgr.Init();
    g_DecalMgr.Init();
    g_AudioManager.Init( 5512*1024 );

    #ifdef TARGET_XBOX
    g_VoiceMgr.GetHeadset().Init( TRUE );
    #endif

    // Initialize animation system
    anim_event::Init();

#if defined(AUDIO_ENABLE)
    g_MusicMgr.Init();
    g_ConverseMgr.Init();
#endif

    g_NetworkMgr.Init();
    g_GameTextMgr.Init();

#if defined( ENABLE_DEBUG_MENU )
    // Init stats
    x_memset( &g_Stats, 0, sizeof( g_Stats ) );
    g_Stats.Interval = 1;
#endif

    // Setup the full path to the platform specific release data
    x_strcpy( g_FullPath, xfs( "%s\\%s", RELEASE_PATH, PLATFORM_PATH ) );

    // Initialize the resource system
    x_DebugMsg( "Starting to initialize resource manager\n" );
    g_RscMgr.Init();
    g_RscMgr.SetRootDirectory( g_FullPath );
    g_RscMgr.SetOnDemandLoading( FALSE );
    x_DebugMsg( "Finished initializing resource manager\n" );

    g_LevelLoader.LoadDFS( "BOOT" );
    g_LevelLoader.LoadDFS( "PRELOAD" );

    // Initialize the render system
    render::Init();
    x_DebugMsg( "Finished initializing rendering\n" );

    // Init systems
    g_TracerMgr.Init();  
    g_PhysicsMgr.Init();

    x_DebugMsg( "Loaded projected textures\n" );

    // Load the debug camera
    LoadCamera();
    x_DebugMsg( "Loaded camera\n" );

    // initialize ui manager
	g_UiMgr =  new ui_manager;
    g_UiMgr->Init();
    g_UiMgr->SetRes();

    // load strings for inventory items.
    g_StringTableMgr.LoadTable( "Inventory", xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "ENG_Inventory_strings.stringbin" ) );

    #ifdef TARGET_XBOX
    if( ParentalControlFailed )
    {
        irect rb( 0, 0, 640, 480 );
        g_pd3dDevice->Clear( 0,NULL,D3DCLEAR_TARGET,0,0.0f,0 );
        g_UiMgr->RenderText( g_UiMgr->FindFont( "large" ),rb,
            ui_font::h_center|ui_font::v_center, XCOLOR_WHITE,
            "You can't play this disc because of the\n"
            "current parental control setting.\n"
            "Remove the disc to continue." );
        g_pd3dDevice->Present( 0,0,0,0 );
        while(1);// intentionally empty
    }
    #endif


    // initialize state manager
    // MUST be done AFTER resource manager init
    g_StateMgr.Init();

    // initialize memcard manager
    //g_MemcardMgr.Init();
    //g_UIMemCardMgr.Init();

    g_RscMgr.TagResources();

#if !defined(X_RETAIL) && defined(RSC_MGR_COLLECT_STATS)
    {
        MEMORY_OWNER( "STATS" );
        g_RscMgr.DumpStats();
    }
#endif // X_RETAIL

    // initialize save manager
#ifdef OLD_SAVE
    g_SaveMgr.Init();
#endif
    g_CheckPointMgr.Init(0);

    // Init the debug menu
#if defined( ENABLE_DEBUG_MENU )
    g_DebugMenu.Init();
#endif

#ifdef TARGET_XBOX
    // intialize audio settings from the XBOX dashboard
    u32 SpeakerConfig = XGetAudioFlags();
    if ( XC_AUDIO_FLAGS_ENCODED( SpeakerConfig ) & XC_AUDIO_FLAGS_ENABLE_AC3 )
    {
        g_AudioMgr.SetSpeakerConfig( SPEAKERS_DOLBY_DIGITAI_5_1 );
    }
    else if ( XC_AUDIO_FLAGS_BASIC( SpeakerConfig ) == XC_AUDIO_FLAGS_SURROUND )
    {
        g_AudioMgr.SetSpeakerConfig( SPEAKERS_PROLOGIC );
    }
    else if ( XC_AUDIO_FLAGS_BASIC( SpeakerConfig ) == XC_AUDIO_FLAGS_STEREO )
    {
        g_AudioMgr.SetSpeakerConfig( SPEAKERS_STEREO );
    }
    else
    {
        g_AudioMgr.SetSpeakerConfig( SPEAKERS_MONO );
    }
#endif
}

//=============================================================================

void DoShutdown( void )
{
    g_UIMemCardMgr.Kill();

    g_NetworkMgr.Kill();

    g_GameTextMgr.Kill();

    g_DecalMgr.Kill();
    g_PlaySurfaceMgr.Kill();

    #ifdef TARGET_XBOX
    g_VoiceMgr.GetHeadset().Kill();
    #endif

    #ifdef AUDIO_ENABLE
    g_ConverseMgr.Kill();
    g_AudioMgr.UnloadAllPackages();
    g_AudioMgr.Kill();
    #endif

    // Kill systems
    g_StateMgr.Kill();
    g_TracerMgr.Kill();
    g_PhysicsMgr.Kill();
    g_IoMgr.Kill();
#ifdef OLD_SAVE
    g_SaveMgr.Kill();
#endif
    render::Kill();
}

xtimer s_FrontEndDelta;
//=============================================================================
void UpdateFrontEnd( void )
{
    f32 DeltaTime;

    DeltaTime = s_FrontEndDelta.TripSec();
    if (DeltaTime > 0.25f)
    {
        DeltaTime = 1.0f/30.0f;
    }

    ASSERT( g_StateMgr.IsBackgroundThreadRunning() == FALSE );

    input_UpdateState();

    g_StateMgr.CheckControllers();

#ifdef TARGET_PC
    // Bail if the app is closed
    if( input_IsPressed( INPUT_MSG_EXIT ) )
        return;
#endif
    g_NetworkMgr.Update(DeltaTime);
    g_StateMgr.Update(DeltaTime);
    g_StateMgr.Render();

    // update audio manager
    UpdateAudio( DeltaTime );

    eng_PageFlip();
}

void RunFrontEnd( void )
{
    MEMORY_OWNER( "RunFrontEnd()" );

    // load lore strings
    g_StringTableMgr.LoadTable( "lore", xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "ENG_lore_strings.stringbin") );
#ifdef TARGET_PS2
    if( x_GetTerritory() == XL_TERRITORY_AMERICA )
    {
        g_StringTableMgr.LoadTable( "EULA", xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "ENG_EULA_SCEA.stringbin") );
    }
    else
    {
        g_StringTableMgr.LoadTable( "EULA", xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "ENG_EULA.stringbin") );
    }
#endif

    // On XBox we need to be able to detect the headset presence in realtime in the frontend.
    // Initializing the voice manager here will cause the headset to be updated.
    #ifdef TARGET_XBOX
    g_VoiceMgr.Init( TRUE, TRUE );
    #endif

	// Run the FrontEnd
	//while( g_FrontEndMgr.GetFrontEndState() != FE_TRIGGER_LOAD ) 
	//	g_FrontEndMgr.Update( 1 );
    s_FrontEndDelta.Reset();
    s_FrontEndDelta.Start();

    while( (g_StateMgr.GetState() != SM_MULTI_PLAYER_LOAD_MISSION) &&
           (g_StateMgr.GetState() != SM_SINGLE_PLAYER_LOAD_MISSION) &&
           (g_StateMgr.GetState() != SM_RELOAD_CHECKPOINT) &&
           (g_StateMgr.GetState() != SM_DEMO_EXIT) )
    {
        UpdateFrontEnd( );
    }

    // update input - flush last keypress
    input_UpdateState();

    #ifdef TARGET_XBOX
    g_VoiceMgr.Kill();
    #endif

    // unload string tables
#ifdef TARGET_PS2
    g_StringTableMgr.UnloadTable( "EULA" );
#endif
    g_StringTableMgr.UnloadTable( "lore" );
}

//=============================================================================

void RunGame( void )
{
    MEMORY_OWNER( "INGAME" );
    f32 GameTime = 0.0f;

#if !defined( CONFIG_RETAIL )
    g_MemoryLowWater = 0x7fffffff;
#endif

    // XBox requires the voice manager to be initialized differently since the
    // Headset can be used during the frontend menus.
    #ifdef TARGET_XBOX
    g_VoiceMgr.Init( g_NetworkMgr.IsServer(), TRUE );
    #endif

    // Commit the settings again. Some of the fields require knowledge as to whether or
    // not we're a server.
    g_StateMgr.GetActiveSettings().Commit();
    // Find the level settings.
    slot_id         ID        = g_ObjMgr.GetFirst( object::TYPE_LEVEL_SETTINGS );
    level_settings* pSettings = (level_settings*)g_ObjMgr.GetObjectBySlot( ID );

    // All good?
    if( pSettings && pSettings->IsKindOf(level_settings::GetRTTI()) )
    {
        // Get the startup trigger.
        guid    GUID    = pSettings->GetStartupGuid();
        object* pObject = g_ObjMgr.GetObjectByGuid( GUID );

        // All good?
        if( pObject && pObject->IsKindOf(trigger_ex_object::GetRTTI()) )
        {
            trigger_ex_object &Trigger = trigger_ex_object::GetSafeType( *pObject );

            // Force it to be active and such!
            Trigger.ForceStartTrigger();

            // Run the trigger logic once.
            Trigger.OnAdvanceLogic( 0.033f );

            // Now NUKE it!
            g_ObjMgr.DestroyObjectEx( GUID, TRUE );
        }
    }

    f32 DeltaTime = MAX( g_GameTimer.ReadSec(), 0.001f );

    // Run!  At least until we stop, that is.
    while( TRUE )
    {
        LOG_STAT( k_stats_CPU_Time );

#if !defined( CONFIG_RETAIL )
        s32 LowWater = x_MemGetFree();
        if( LowWater < g_MemoryLowWater )
            g_MemoryLowWater = LowWater;
#endif // !defined( CONFIG_RETAIL )

        //
        // Delta time and game timer management.
        //
        {
            // Compute the duration of the last frame.
            DeltaTime = MAX( g_GameTimer.ReadSec(), 0.001f );

            #ifdef TARGET_PC
            s32 DelayTime = 32 - (s32)(DeltaTime * 1000.0f);
//          x_DelayThread( DelayTime );
            DeltaTime = MAX( g_GameTimer.ReadSec(), 0.001f );
            #endif

            g_PerceptionMgr.Update( DeltaTime );

            DeltaTime *= g_PerceptionMgr.GetGlobalTimeDialation();

#if !defined( CONFIG_RETAIL )
            // add in world time dialation
            DeltaTime *= g_WorldTimeDilation;
#endif

            // Beware negative delta time!
            if( DeltaTime < 0.0f )
            {
                LOG_ERROR( "RunGame", "NEGATIVE DELTA TIME" );
                DeltaTime = 33.0f / 1000.0f;
            }

		    g_GameTimer.Reset();
		    g_GameTimer.Start();

            GameTime += DeltaTime;
        }

        //
        // We are "behind the times".  Catch up to the present.
        //
        Update( DeltaTime );

        //
        // During the logic, the game could have "ended".  If it did, get out
        // NOW!  Do not pass GO.  Do not attempt to render.
        //

        #ifdef TARGET_PC
        if( input_IsPressed( INPUT_MSG_EXIT ) )
            break; // GAME OVER, DUDE!
        #endif

        if( g_ActiveConfig.GetExitReason() != GAME_EXIT_CONTINUE )
            break; // GAME OVER, DUDE!

        //
        // We are now reasonably caught up time-wise.  Lets show the situation.
        //

        // give the level a few frames for triggers and other objects to get
        // started before trying to render anything
        if( GameMgr.GameInProgress() )
        {
            if( g_nLogicFramesAfterLoad > 10 )
            {
                RenderGame();
            }

            // render the pause
            g_StateMgr.Render();

            // Some extra stuff...
            {
                // Update context stats.
                x_ContextPrintProfile();
                x_ContextResetProfile();

                #if defined( ENABLE_DEBUG_MENU )
                Stats( DeltaTime );
                //AudioStats( DeltaTime );
                #endif // X_RETAIL
            }
            eng_PageFlip();
        }
        else
        {
            if( g_ActiveConfig.GetExitReason()==GAME_EXIT_CONTINUE )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_ADVANCE_LEVEL );
            }
        }
    }
    //
    // Was the user in the pause menu - unexpected exit condition.
    //
    if( g_StateMgr.IsPaused() )
    {
        // reset the pause flag
        g_StateMgr.ClearPause();
        // set the state to idle whilst we wait
        g_StateMgr.SetState( SM_IDLE );
    }

    //
    // Enable the user interface for the primary user.
    //
    g_UiMgr->EnableUser( g_UiUserID, TRUE );

#ifndef CONFIG_RETAIL
    // Stop our automated client/server stuff while trying to test the game exit 
    // code.
    g_Config.AutoClient = FALSE;
    g_Config.AutoServer = FALSE;
#endif

    #ifdef TARGET_XBOX
    g_VoiceMgr.Kill();
    #endif
}


//=============================================================================
//  MAIN
//=============================================================================
void AppMain( s32 argc, char* argv[] )
{
    // Parse out the command line arguments
#ifndef X_RETAIL

    extern xbool AUDIO_TWEAK;
    {
        for( s32 i=1; i<argc; i++ )
        {
            if( x_stricmp( argv[i], "autoserver" ) == 0 )
                g_CmdLineAutoServer = TRUE;

            if( x_stricmp( argv[i], "autoclient" ) == 0 )
                g_CmdLineAutoClient = TRUE;

            if( x_stricmp( argv[i], "audiotweak" ) == 0 )
                AUDIO_TWEAK = TRUE;

            if( x_stricmp( argv[i], "rtfhandler" ) == 0 )
                g_CmdLineRTFHandler = TRUE;

            if( x_stricmp( argv[i], "language" ) == 0 )
            {
                i++;
                if( i<argc )
                {
                    if( x_stricmp( argv[i], "eng" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_ENGLISH;
                    }
                    if( x_stricmp( argv[i], "fre" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_FRENCH;
                    }
                    if( x_stricmp( argv[i], "ger" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_GERMAN;
                    }
                    if( x_stricmp( argv[i], "ita" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_ITALIAN;
                    }
                    if( x_stricmp( argv[i], "spa" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_SPANISH;
                    }
                    if( x_stricmp( argv[i], "dut" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_DUTCH;
                    }
                    if( x_stricmp( argv[i], "jpn" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_JAPANESE;
                    }
                    if( x_stricmp( argv[i], "kor" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_KOREAN;
                    }
                    if( x_stricmp( argv[i], "por" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_PORTUGUESE;
                    }
                    if( x_stricmp( argv[i], "chi" ) == 0 )
                    {
                        g_CmdLineLanguage = XL_LANG_TCHINESE;
                    }
                }
            }
        }
    }
#endif

    (void)argc;
    (void)argv;
    xbool bFullLevelLoad = TRUE;

    MEMORY_OWNER( "DYNAMIC" );

    //
    // Do core startup
    //
    DoStartup();

#if 0   // **BISCUIT** This is voice test code - I will remove once I am done testing.
    extern void VoiceTestCode( void );
    VoiceTestCode();
#endif

#if 0
#ifdef TARGET_XBOX
    extern void XBoxLiveTest( void );
    XBoxLiveTest();
#endif
#endif

/*
    // SB 2/28/05
    // Useful test to check "ComputeTriSphereMovement" function when asserts fire off
    extern xbool ComputeTriSphereMovement( const vector3& aP0,
        const vector3& aP1,
        const vector3& aP2,
        const vector3& SphereCenter,
        f32      SphereRadius,
        f32      SphereHalfHeight,
        vector3& FinalMovement );

    vector3 P0( 0x3F800000452629C2C447BFD94500BE60LL );
    vector3 P1( 0x3F800000452703D0C447400044FDCD23LL );
    vector3 P2( 0x3F80000045284260C447400044FBD536LL );
    vector3 SC( 0x7F9D92654526B828C4311FFF44FE1DD3LL );
    f32 SR  = 30.0f;
    f32 SHH = 90.0f;
    vector3 FM;
    ComputeTriSphereMovement( P0, P1, P2, SC, SR, SHH, FM );
*/    
   

    // We have to reset these here as we should now have the stringtable
    g_StateMgr.GetActiveSettings().Reset( RESET_ALL );
    g_StateMgr.GetPendingSettings().Reset( RESET_ALL );

    //
    // We're starting off with no exit condition. This will force the 'RunFrontEnd' to
    // start afresh. RunFrontEnd will decide where we need to go next.
    //
    g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
    //
    // Loop forever playing our fabulous game
    //
    s32 nLoops = 0;
    sm_states CooldownState;

    while( TRUE )
    {
        nLoops++;

        //
        // Did a level trigger request a new level or do we
        // need to talk to the frontend?
        //
        LOG_MEMMARK( "RunFrontEnd" );
        RunFrontEnd();

        if( g_NetworkMgr.IsServer() )
        {
            CooldownState = SM_SERVER_COOLDOWN;
        }
        else
        {
            CooldownState = SM_CLIENT_COOLDOWN;
        }

        LOG_MEMMARK( "LoadLevel" );
        // Bail if the app is closed
#ifdef TARGET_PC
        if( input_IsPressed( INPUT_MSG_EXIT ) )
            break;
#endif

        // Is this a new game or are we restoring from save data?
        if( g_StateMgr.IsRestoredGame() )
        {
            // Not a full level load...
            bFullLevelLoad = FALSE;

            // Load the selected level (load the entire thing however.)
            g_LevelLoader.LoadLevel( TRUE );
        }
        else
        {
            // Load the selected level.
            g_LevelLoader.LoadLevel( bFullLevelLoad );
        }

        //*BW*
        // This causes us to bail early in this special case as it would mean we do not have
        // the mission on the memory card. However, ideally LoadLevel would abort early should
        // something like a disconnect happen and so we may have to end up fixing UnloadLevel
        // so it will deal with partially, or no, loaded level data.
        if( g_ActiveConfig.GetExitReason() == GAME_EXIT_INVALID_MISSION )
        {
            g_StateMgr.SetState( CooldownState );
            continue;
        }
        // Tell network manager load is complete!
        g_NetworkMgr.LoadMissionComplete();


        s_FrontEndDelta.Reset();
        s_FrontEndDelta.Start();
        //
        // We have to wait until the statemgr has said that everything is ready to go. This is what detects whether or
        // not the sync phase has completed.
        //
        while( (g_StateMgr.GetState() != SM_PLAYING_GAME) && (g_ActiveConfig.GetExitReason() == GAME_EXIT_CONTINUE) )
        {
            UpdateFrontEnd();
        }

        // Finish off the last-minute loading stuff that cannot occur
        // from a background thread or that would cause a pause
        // to happen in the slide show.
        g_LevelLoader.LoadLevelFinish();

        // During the level load process it is possible for an error such as duplicate logon to occur.
        // When this happens the exit reason will be set to something other than GAME_EXIT_CONTINUE.
        // This has the effect of denying the game from creating all the necessary objects such as players etc.
        // We must therefore only attempt a checkpoint restore if we are sure there were no errors on load.
        if( g_ActiveConfig.GetExitReason() == GAME_EXIT_CONTINUE )
        {
            // Full level load?
            if( bFullLevelLoad )
            {
                // Restore the player inventory (if necessary)
                g_StateMgr.RestorePlayerInventory();
            }
            else
            {
                // Suck in the check point info.
                g_CheckPointMgr.Restore( FALSE );
            }
        }

#if defined( ENABLE_DEBUG_MENU ) && !defined( X_RETAIL ) && !defined( TARGET_XBOX )
        if( g_bMemReports )
        {
            void    A51_MemReport( const char* pFileName );
            xstring FindNextFileInSequence( const char* pFile, const char* pExtension );
            xstring PathName; 
            char    LevelName[64];

            x_splitpath( g_ActiveConfig.GetLevelPath(),NULL,NULL,LevelName,NULL);
            {
                xstring PathName = FindNextFileInSequence( xfs("c:\\MemoryDump_%s",LevelName), "csv" );
                x_MemDump( PathName, TRUE );
            }
            {
                xstring PathName = FindNextFileInSequence( xfs("c:\\MemorySummary_%s",LevelName), "csv" );
                A51_MemReport( PathName );
            }
            {
                #ifdef RSC_MGR_COLLECT_STATS
                xstring PathName = FindNextFileInSequence( xfs("c:\\ResourceSummary_%s",LevelName), "csv" );
                g_RscMgr.DumpStatsToFile( PathName );
                #endif
            }
            {
                xstring PathName = FindNextFileInSequence( xfs("c:\\ObjMgrSummary_%s",LevelName), "csv" );
                g_ObjMgr.DumpStatsToFile( PathName );
            }
        }
#endif

        LOG_MEMMARK( "RunGame" );
        if( g_ActiveConfig.GetExitReason() == GAME_EXIT_CONTINUE )
        {
            // Only for campaign and full level loads set the initial checkpoint.
            if( (GameMgr.GetGameType() == GAME_CAMPAIGN) && bFullLevelLoad )
            {
                // Start again!
                g_CheckPointMgr.Reinit( g_ActiveConfig.GetLevelID() );

                // Set the initial checkpoint!
                g_CheckPointMgr.SetCheckPoint( NULL_GUID, NULL_GUID, -1, -1 );
                g_StateMgr.SilentSaveProfile();
            }

            // Play the game until we hit an exit condition
            RunGame();
        }

        // Clean out any existing feedback info.
        // This prevents us from rumbling when input updates next time.
        input_ClearFeedback();

        // Bail if the app is closed
#ifdef TARGET_PC
        if( input_IsPressed( INPUT_MSG_EXIT ) )
        {
            g_LevelLoader.UnloadLevel( TRUE );
            break;
        }
#endif

        // Default to complete level load.
        bFullLevelLoad = TRUE;

        g_StateMgr.SetPaused( FALSE, g_StateMgr.GetActiveControllerID() );
        // Special cases for campaign games.
        if( GameMgr.GetGameType() == GAME_CAMPAIGN )
        {
            // Decide what to do based on the exit reason!
            switch( g_ActiveConfig.GetExitReason() )
            {
                case GAME_EXIT_ADVANCE_LEVEL:                                          
                    // Backup the player inventory!
                    g_StateMgr.BackupPlayerInventory();
                    break;

                case GAME_EXIT_RELOAD_CHECKPOINT:
                    // Just load the objects.
                    bFullLevelLoad = FALSE;
                    break;

                default:
                    break;
            }
        }

        s_FrontEndDelta.Reset();
        s_FrontEndDelta.Start();

        //
        // On the PS2, wait for the GS to finish up. We NEVER want the GS rendering from data that might
        // have been deleted. That could definitely cause random lockups or other glitches between
        // level loads.
        //
        #ifdef TARGET_PS2
        DLIST.WaitForTasks();
        #endif

        //
        // We have to wait until the statemgr has said all the subsystems have cooled down. This makes sure
        // that we do not have any game traffic going on while the level is unloading.
        //
        if( g_StateMgr.GetState() != CooldownState )
        {
            g_StateMgr.SetState( CooldownState );
        }

        g_NetworkMgr.Update( 0.01f );
        g_NetworkMgr.Update( 0.01f );
        g_NetworkMgr.Update( 0.01f );

        //
        // Unload the level
        //
        LOG_MEMMARK( "UnloadLevel" );
        g_LevelLoader.UnloadLevel( bFullLevelLoad );
        LOG_FLUSH();
        while( g_StateMgr.GetState() == CooldownState )
        {
            UpdateFrontEnd();
        }
    }

    //
    // Shutdown systems involved in Startup
    //
    DoShutdown();
}
