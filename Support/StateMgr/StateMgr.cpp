//=========================================================================
//
//  StateMgr.cpp
//
//=========================================================================

#include "Entropy.hpp"

#include "StateMgr/MapList.hpp"
#include "StateMgr/LoreList.hpp"
#include "StateMgr/SecretList.hpp"

#include "StateMgr/StateMgr.hpp"
#include "Configuration/GameConfig.hpp"
#include "StringMgr/StringMgr.hpp"
#include "Inventory/Inventory2.hpp"
#include "Objects/NewWeapon.hpp"
#include "Objects/Actor/Actor.hpp"
#include "Objects/Player.hpp"
#include "InputMgr/GamePad.hpp"


#include "ui/ui_button.hpp"
#include "ui/ui_dialog.hpp"
#include "ui/ui_manager.hpp"

#ifndef TARGET_XBOX
#include "dialogs/dlg_COPA.hpp"
#endif
#include "dialogs/dlg_ESRB.hpp"
#include "dialogs/dlg_MainMenu.hpp"
#include "dialogs/dlg_AVSettings.hpp"
#include "dialogs/dlg_Headset.hpp"
#include "dialogs/dlg_PressStart.hpp"
#include "dialogs/dlg_MultiOptions.hpp"
#include "dialogs/dlg_LevelSelect.hpp"
#include "dialogs/dlg_ProfileSelect.hpp"
#include "dialogs/dlg_ProfileOptions.hpp"
#include "dialogs/dlg_ProfileControls.hpp"
#include "dialogs/dlg_ProfileAV.hpp"
#include "dialogs/dlg_ProfileHeadset.hpp"
#ifdef TARGET_PS2
#include "dialogs/dlg_OnlineEULA.hpp"
#endif
#include "dialogs/dlg_OnlineConnect.hpp"
#include "dialogs/dlg_OnlineHost.hpp"
#include "dialogs/dlg_OnlineHostOptions.hpp"
#include "dialogs/dlg_OnlineJoin.hpp"
#include "dialogs/dlg_JoinFilter.hpp"
#include "dialogs/dlg_OnlinePlayers.hpp"
#include "dialogs/dlg_OnlineLevelSelect.hpp"
#include "dialogs/dlg_Friends.hpp"
#include "dialogs/dlg_Players.hpp"
#include "dialogs/dlg_StartGame.hpp"
#include "dialogs/dlg_LoadGame.hpp"
#include "dialogs/dlg_LevelDesc.hpp"
#include "dialogs/dlg_PauseMain.hpp"
#include "dialogs/dlg_EndPause.hpp"
#include "dialogs/dlg_PopUp.hpp"
#include "dialogs/dlg_ResumeGame.hpp"
#include "dialogs/dlg_PauseMp.hpp"
#include "dialogs/dlg_PauseOnline.hpp"
#include "dialogs/dlg_Leaderboard.hpp"
#include "dialogs/dlg_TeamLeaderboard.hpp"
#include "dialogs/dlg_BigLeaderboard.hpp"
#include "dialogs/dlg_DemoMainMenu.hpp"
#include "dialogs/dlg_AvatarSelect.hpp"
#include "dialogs/dlg_ReportError.hpp"
#if defined(TARGET_PS2)
#include "dialogs/dlg_Download.hpp"
#endif
#include "dialogs/dlg_MCMessage.hpp"
#include "dialogs/dlg_CampaignMenu.hpp"
#if defined(TARGET_PS2)
#include "dialogs/dlg_MemcardSelect.hpp"
#endif
#include "dialogs/dlg_VoteMap.hpp"
#include "dialogs/dlg_VoteKick.hpp"
#include "dialogs/dlg_ServerConfig.hpp"
#include "dialogs/dlg_ChangeMap.hpp"
#include "dialogs/dlg_KickPlayer.hpp"
#include "dialogs/dlg_OnlineLogin.hpp"
#include "dialogs/dlg_LoreMenu.hpp"
#include "dialogs/dlg_TeamChange.hpp"
#include "dialogs/dlg_SubMenu.hpp"
#include "dialogs/dlg_Feedback.hpp"
#include "dialogs/dlg_SecretsMenu.hpp"
#include "dialogs/dlg_Stats.hpp"
#include "dialogs/dlg_Autosave.hpp"
#include "dialogs/dlg_Credits.hpp"
#include "dialogs/dlg_Extras.hpp"

#ifdef TARGET_XBOX
#include "dialogs/XBOX/dlg_MultiPlayerXBOX.hpp"
#include "dialogs/XBOX/dlg_OnlineMainXBOX.hpp"
#include "dialogs/XBOX/dlg_QuickMatch.hpp"
//#include "dialogs/XBOX/dlg_OptiMatch.hpp"
#include "entropy\xbox\xbox_private.hpp"
#else
#include "dialogs/dlg_MultiPlayer.hpp"
#include "dialogs/dlg_OnlineMain.hpp"
#endif

#include "../MemCardMgr/MemCardMgr.hpp"

#ifdef USE_MOVIES
#include "MoviePlayer/MoviePlayer.hpp"
#endif

#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/GameServer.hpp"
#include "NetworkMgr/GameClient.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr/MatchMgr.hpp"


#include "Parsing/textin.hpp"
#include "AudioMgr/AudioMgr.hpp"
#include "NetworkMgr/Voice/VoiceMgr.hpp"
#include "inputmgr\inputmgr.hpp"
#include "GameLib/LevelLoader.hpp"
#include "e_Memcard.hpp"
#include "IOManager/io_mgr.hpp"

#ifdef TARGET_PS2
#include "PS2/ps2_dlist.hpp"
#endif

#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif

#if defined(CONFIG_IS_DEMO)
extern xtimer g_DemoIdleTimer;
#define DEMO_ENDGAME_TIMEOUT (15.0f)
#endif

//=========================================================================
//  Defines
//=========================================================================

#ifdef TARGET_XBOX
extern u32 g_TEdge,g_PhysW,g_PhysH,g_PhysFPS;
#define DIALOG_TOP (g_TEdge - 10)
#define DIALOG_BOTTOM (DIALOG_TOP+(448-72))
#else
#define DIALOG_TOP 24
#define DIALOG_BOTTOM 448-72
#endif

//=========================================================================
//  Structs                                                                
//=========================================================================
                                                                           
//=========================================================================
//  Globals                                                                
//=========================================================================

extern xbool    g_bControllerCheck;
state_mgr       g_StateMgr;

extern s32      g_Difficulty;

static s32      s_Delay    = 0;
static s32      s_VoiceID  = 0;
static xbool    s_FirstMap = TRUE;

static s32 s_ProfileOptLeft    = 106;
static s32 s_ProfileOptRight   = 406;

//=========================================================================
//  Player Profile Functions
//=========================================================================
//=========================================================================
void InitFrontEndMusic( void );
void KillFrontEndMusic( void );

void GetMissionName( s32 MapIndex, xwchar* pBuffer )
{
    text_in     TextIn;
    char        Buffer[128];
    s32         i;

    TextIn.OpenFile( xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "config.txt") );
    TextIn.ReadHeader();
    
    s32  nLevel = TextIn.GetHeaderCount();

    for( i=0; i < nLevel; i++ )
    {
        TextIn.ReadFields();
        TextIn.GetString( "Level", Buffer );
        if( i==MapIndex )
            break;
    }
    TextIn.CloseFile();
    x_wstrcpy(pBuffer,xwstring(Buffer));
}

//=========================================================================
//  State Manager Functions
//=========================================================================
state_mgr::state_mgr( void )
{   
    m_bInited = FALSE;
    m_ProfileNames.SetCapacity( 32 );  // Max number of profiles read from both memcards
}

//=========================================================================

state_mgr::~state_mgr( void )
{
    ASSERT( !m_bInited );
}

//=========================================================================

void state_mgr::Kill( )
{
    ASSERT( m_bInited == TRUE );
    m_bInited = FALSE;
}

//=========================================================================

void state_mgr::Init( void )
{
    ASSERT( !m_bInited );

    // Register dialogs
#if defined(TARGET_PS2)
    dlg_copa_register               ( g_UiMgr );
#endif
    dlg_esrb_register               ( g_UiMgr );
    dlg_main_menu_register          ( g_UiMgr );
    dlg_av_settings_register        ( g_UiMgr );
    dlg_headset_register            ( g_UiMgr );
    dlg_profile_select_register     ( g_UiMgr );
    dlg_profile_options_register    ( g_UiMgr );
    dlg_profile_controls_register   ( g_UiMgr );
    dlg_profile_av_register         ( g_UiMgr );
    dlg_profile_headset_register    ( g_UiMgr );
    dlg_press_start_register        ( g_UiMgr );
    dlg_multi_player_register       ( g_UiMgr );
    dlg_multi_options_register      ( g_UiMgr );
#ifndef CONFIG_RETAIL
    dlg_level_select_register       ( g_UiMgr );
#endif
#if defined(TARGET_PS2)
    dlg_online_eula_register        ( g_UiMgr );
#endif
    dlg_online_connect_register     ( g_UiMgr );
    dlg_online_main_register        ( g_UiMgr );
    dlg_online_host_register        ( g_UiMgr );
    dlg_online_host_options_register( g_UiMgr );
    dlg_online_join_register        ( g_UiMgr );
    dlg_join_filter_register        ( g_UiMgr );
    dlg_online_level_select_register( g_UiMgr );
    dlg_online_players_register     ( g_UiMgr );
    dlg_friends_register            ( g_UiMgr );
    dlg_players_register            ( g_UiMgr );
    dlg_start_game_register         ( g_UiMgr );
    dlg_load_game_register          ( g_UiMgr );
    dlg_level_desc_register         ( g_UiMgr );
    dlg_pause_main_register         ( g_UiMgr );
    dlg_pause_mp_register           ( g_UiMgr );
    dlg_pause_online_register       ( g_UiMgr );
    dlg_end_pause_register          ( g_UiMgr );
    dlg_popup_register              ( g_UiMgr );
    dlg_resume_game_register        ( g_UiMgr );
    dlg_leaderboard_register        ( g_UiMgr );
    dlg_team_leaderboard_register   ( g_UiMgr );
    dlg_big_leaderboard_register    ( g_UiMgr );
    dlg_demo_main_menu_register     ( g_UiMgr );
    dlg_avatar_select_register      ( g_UiMgr );

    dlg_report_error_register       ( g_UiMgr );
#if defined(TARGET_PS2)
    dlg_download_register           ( g_UiMgr );
#endif
    dlg_mcmessage_register          ( g_UiMgr );
    dlg_campaign_menu_register      ( g_UiMgr );
    dlg_lore_menu_register          ( g_UiMgr );
#if defined(TARGET_PS2)
    dlg_memcard_select_register     ( g_UiMgr );
#endif
    dlg_vote_map_register           ( g_UiMgr );
    dlg_vote_kick_register          ( g_UiMgr );
    dlg_server_config_register      ( g_UiMgr );
    dlg_change_map_register         ( g_UiMgr );
    dlg_kick_player_register        ( g_UiMgr );
    dlg_team_change_register        ( g_UiMgr );
    dlg_online_login_register       ( g_UiMgr );
    dlg_submenu_register            ( g_UiMgr );
    dlg_feedback_register           ( g_UiMgr );
    dlg_secrets_menu_register       ( g_UiMgr );
    dlg_stats_register              ( g_UiMgr );
    dlg_autosave_register           ( g_UiMgr );
    dlg_credits_register            ( g_UiMgr );
    dlg_extras_register             ( g_UiMgr );

#if defined(TARGET_XBOX)
    dlg_quick_match_register        ( g_UiMgr );
#endif
    // Setup state mgr view
    m_View.SetZLimits( 10, 100.0f*500.0f );
    m_View.SetPosition( vector3(1356,-1922,-285) );
    m_View.LookAtPoint( vector3(0,0,0) );

    // set movie position/size
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    m_MovieSize.Set( (f32)XRes, (f32)YRes );
    m_MoviePosition.Set( 0.0f, 0.0f );

    // Set initial states
    m_bInited                   = TRUE;
    m_bDoSystemError            = FALSE;
    m_bIsDuplicateLogin         = FALSE;
    m_bIsPaused                 = FALSE;
    m_bRetrySilentAutoSave      = FALSE;
    m_bRetryAutoSaveMenu        = FALSE;
    m_bStartSaveGame            = FALSE;
    m_State                     = SM_IDLE;
    m_PrevState                 = SM_IDLE;
    m_Exit                      = FALSE;
    m_TimeoutTime               = 30.0f;
    m_CurrentDialog             = NULL;
    m_bPlayMovie                = FALSE;
    m_LeaderboardID             = SM_LEADERBOARD;
    m_CampaignType              = SM_NEW_CAMPAIGN_GAME;
    m_bInventoryIsStored        = FALSE;
    m_bDoDefaultLoadOut         = FALSE;
    m_bCreatingProfile          = FALSE;
    m_pBackgroundRenderer       = NULL;
    m_pLeaderboardDialog        = NULL;
    m_BackgroundRendererRunning = FALSE;
    m_ActiveControllerID        = -1;
    m_SettingsCardSlot          = -1;
    m_bAutosaveInProgress       = FALSE;
    m_bDisableMemcardDialogs    = FALSE;
    m_bSilentSigninStarted      = FALSE;

    // calculate save sizes
#ifdef TARGET_XBOX
    m_ProfileSaveSize  = ( ( sizeof( player_profile  ) + 16383 ) &~ 16383 ); // round up to nearest block
    m_SettingsSaveSize = ( ( sizeof( global_settings ) + 16383 ) &~ 16383 ); // round up to nearest block
#else
    m_ProfileSaveSize  = ( ( sizeof( player_profile  ) + 1023 ) &~ 1023 ); // round up to nearest KB
    m_SettingsSaveSize = ( ( sizeof( global_settings ) + 1023 ) &~ 1023 ); // round up to nearest KB
#endif
    // add extra space required by platform directory structure
    m_ProfileSaveSize  += PROFILE_DIR_SIZE;
    m_SettingsSaveSize += SETTINGS_DIR_SIZE;

    // initialize dialog controls
    s32 c;
    for( c=0; c<SM_NUM_STATES; c++ )
    {
        m_CurrentControl[c] = -1;
    }

    // initialize map cycle
    ClearMapCycle();

    // Initialize game controls
    m_PlayerCount     =  0;
    m_LocalPlayerSlot = -1;

    // initialize profiles 
    s32 i;
    for( i=0; i<SM_PROFILE_COUNT; i++ )
    {
        m_Profiles[i].SetProfileName( "Empty" );
        m_ProfileListIndex[i] = -1;
        m_SelectedProfile[i]  = 0;
        m_ProfileNotSaved[i]  = TRUE;
    }

    for ( i=0; i<MAX_LOCAL_PLAYERS; i++ )
    {
        m_bControllerRequested[i] = FALSE;
    }
   
    m_PendingProfileIndex = -1;

    // Enable user
    g_UiMgr->EnableUser(g_UiUserID,TRUE);

    // Set up the front end theme - should be moved to UI
    ui_button::SetTextColorNormal       (xcolor(126,220,60,255));   
    ui_button::SetTextColorHightlight   (xcolor(255,252,204,255));
    ui_button::SetTextColorDisabled     (XCOLOR_GREY);
    ui_button::SetTextColorShadow       (xcolor(0,0,0,0));

    ui_dialog::SetTextColorNormal       (xcolor(255,252,204,255));
    ui_dialog::SetTextColorShadow       (XCOLOR_BLACK);

    // determine what levels we can load
    g_MapList.Init();
    g_MapList.LoadDefault();

    // initialize the lore list
    g_LoreList.Init();

    // initialize the secrets list
    g_SecretList.Init();

    g_RscMgr.Load( PRELOAD_FILE("DX_FrontEnd.audiopkg"    ) );
    g_RscMgr.Load( PRELOAD_FILE("SFX_FrontEnd.audiopkg"   ) );
    g_RscMgr.Load( PRELOAD_FILE("MUSIC_FrontEnd.audiopkg" ) );

    // Let's get started!
#if defined(TARGET_XBOX)
#if 0
    // JP: To be TCR compliant we must not initiate the pending accepted invite here.
    // The session must be joinable only AFTER the player *manually* signs in to XBox Live.
    // For this reason the code below is commented out - see UpdateOnlineMain instead.
    {
        // Deal with cross title invite.
        XONLINE_LATEST_ACCEPTED_GAMEINVITE Invitation;
        HRESULT Result;

        Result = XOnlineGameInviteGetLatestAccepted( &Invitation );
        if( Result==S_OK )
        {
            server_info& Config = g_PendingConfig.GetConfig();
            x_memset( &Config, 0, sizeof(server_info) );
            x_memcpy( &Config.SessionID, &Invitation.SessionID, sizeof(Config.SessionID) );
            ASSERT( sizeof(m_FeedbackName) >= XONLINE_GAMERTAG_SIZE );
            x_strncpy( m_FeedbackName, Invitation.szInvitingUserGamertag, XONLINE_GAMERTAG_SIZE );
            // These values are actually unknown but we fill in these just to stop asserts.
            // The MaxPlayers and Players is irrelevent but usually we have this information.
            Config.GameTypeID = GAME_MP;
            Config.MaxPlayers = 32;
            Config.Players    = 1;
            game_config::Commit();
            m_bFollowBuddy = TRUE;
        }
        else
        {
            m_bFollowBuddy = FALSE;
        }
    }
#endif
#endif
#if CONFIG_IS_DEMO
    SetState( SM_INEVITABLE_INTRO );
#else
    if( CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT || CONFIG_IS_AUTOCAMPAIGN || CONFIG_IS_AUTOSPLITSCREEN )
    {
        // bypass movies and go directly to the MP game using the autoClient/Server
        SetState( SM_PRESS_START_SCREEN );
    }
    else
    {
    #ifdef TARGET_XBOX
        // The ESRB notice is done elsewhere
        SetState( SM_INEVITABLE_INTRO );
    #else
        // Normal build.
        SetState( SM_ESRB_NOTICE );
    #endif
    }
#endif

#ifndef X_EDITOR
    switch( x_GetLocale() )
    {    
    case XL_LANG_ENGLISH:    // English uses default
        break;

    default:  // PAL
        {
            // set up new profile options dialog size
            s_ProfileOptLeft    = 63;
            s_ProfileOptRight   = 449;
        }
        break;
    }
#endif
}

//=========================================================================

void state_mgr::ReInit( void )
{
    // re-enable the pause menu
    m_Exit = FALSE;
}


//=========================================================================

void state_mgr::SetState( sm_states State, xbool ForceStateChange )
{
    // ForceStateChange can be set TRUE to make sure the change happens, even to the same state.
    if( ForceStateChange || (State != m_State) )
    {
        x_MemSanity();
        LOG_MESSAGE( "state_mgr::SetState", "From:%s, to:%s, MemFree:%d", GetStateName( m_State ), GetStateName( State ), x_MemGetFree() );
        // Reset current dialog flag
        if( m_CurrentDialog )
        {
            m_CurrentDialog->SetState( DIALOG_STATE_ACTIVE );

            // store the current control id
            m_CurrentControl[m_State] = m_CurrentDialog->GetControl();
        }
#if CONFIG_IS_DEMO
    g_DemoIdleTimer.Trip();
#endif

        // Do clean up for current state
        ExitState   ( m_State );

        // reset dialog pointer
        m_CurrentDialog = NULL;
        
        // Set the current state and reset the timeout
        m_Timeout   = m_TimeoutTime;
        m_PrevState = m_State;
        m_State     = State;

        // Initialize new state
        EnterState  ( State );
    }
}

//=========================================================================
#define LABEL_STRING(x) case x: return ( #x );

const char* state_mgr::GetStateName( sm_states State )
{
    switch( State )
    {
        LABEL_STRING( SM_IDLE );

        LABEL_STRING( SM_ESRB_NOTICE );
        LABEL_STRING( SM_INEVITABLE_INTRO );
#if defined(TARGET_PS2)
        LABEL_STRING( SM_CONTROLLER_CHECK );
        LABEL_STRING( SM_AUTOSAVE_DIALOG );
#endif
        LABEL_STRING( SM_MEMCARD_BOOT_CHECK );

        LABEL_STRING( SM_PRESS_START_SCREEN );

        LABEL_STRING( SM_MAIN_MENU );
        LABEL_STRING( SM_SETTINGS_MENU );
        LABEL_STRING( SM_SETTINGS_HEADSET );
        LABEL_STRING( SM_SETTINGS_MEMCARD_SELECT );
        LABEL_STRING( SM_MANAGE_PROFILES );
        LABEL_STRING( SM_MANAGE_PROFILE_OPTIONS );
        LABEL_STRING( SM_MANAGE_PROFILE_CONTROLS );
        LABEL_STRING( SM_MANAGE_PROFILE_AVATAR );
#if defined(TARGET_PS2)
        LABEL_STRING( SM_MANAGE_MEMCARD_SELECT );
#endif
        LABEL_STRING( SM_MANAGE_PROFILE_SAVE_SELECT );
        LABEL_STRING( SM_MANAGE_PROFILE_MEMCARD_RESELECT );

        LABEL_STRING( SM_DEMO_EXIT );
#if defined(TARGET_PS2)
        LABEL_STRING( SM_MEMCARD_SELECT );
#endif
        LABEL_STRING( SM_CAMPAIGN_MENU );
        LABEL_STRING( SM_CAMPAIGN_PROFILE_OPTIONS );
        LABEL_STRING( SM_CAMPAIGN_PROFILE_CONTROLS );
        LABEL_STRING( SM_CAMPAIGN_PROFILE_AVATAR );
#if defined(TARGET_PS2)
        LABEL_STRING( SM_CAMPAIGN_MEMCARD_SELECT );
#endif
        LABEL_STRING( SM_CAMPAIGN_PROFILE_SAVE_SELECT );
        LABEL_STRING( SM_CAMPAIGN_MEMCARD_RESELECT );
        LABEL_STRING( SM_LOAD_CAMPAIGN );
        LABEL_STRING( SM_SAVE_CAMPAIGN );

        LABEL_STRING( SM_LORE_MAIN_MENU );
        LABEL_STRING( SM_SECRETS_MENU );
        LABEL_STRING( SM_EXTRAS_MENU );
        LABEL_STRING( SM_CREDITS_SCREEN );

        LABEL_STRING( SM_MULTI_PLAYER_MENU );
        LABEL_STRING( SM_MULTI_PLAYER_OPTIONS );
        LABEL_STRING( SM_MULTI_PLAYER_EDIT );
        LABEL_STRING( SM_MP_LEVEL_SELECT );
        LABEL_STRING( SM_MP_SAVE_SETTINGS );
        LABEL_STRING( SM_PROFILE_CONTROLS_MP );
        LABEL_STRING( SM_PROFILE_AVATAR_MP );
#if defined(TARGET_PS2)
        LABEL_STRING( SM_MEMCARD_SELECT_MP );
#endif
        LABEL_STRING( SM_PROFILE_SAVE_SELECT_MP );
        LABEL_STRING( SM_MEMCARD_RESELECT_MP );

        LABEL_STRING( SM_PROFILE_SELECT );
        LABEL_STRING( SM_PROFILE_OPTIONS );
        LABEL_STRING( SM_PROFILE_CONTROLS );
        LABEL_STRING( SM_PROFILE_AVATAR );

#if defined(TARGET_PS2)
        LABEL_STRING( SM_ONLINE_EULA );
#endif
        LABEL_STRING( SM_ONLINE_CONNECT );
        LABEL_STRING( SM_ONLINE_AUTHENTICATE );
#ifndef TARGET_XBOX
        LABEL_STRING( SM_ONLINE_COPA );
        LABEL_STRING( SM_ONLINE_COPA_SAVE_SELECT );
        LABEL_STRING( SM_ONLINE_COPA_MEMCARD_RESELECT );
#endif
        LABEL_STRING( SM_ONLINE_PROFILE_SELECT );
        LABEL_STRING( SM_ONLINE_PROFILE_OPTIONS );
        LABEL_STRING( SM_ONLINE_PROFILE_CONTROLS );
        LABEL_STRING( SM_ONLINE_PROFILE_AVATAR );
#if defined(TARGET_PS2)
        LABEL_STRING( SM_ONLINE_MEMCARD_SELECT );
#endif
        LABEL_STRING( SM_ONLINE_PROFILE_SAVE_SELECT );
        LABEL_STRING( SM_ONLINE_MEMCARD_RESELECT );

        LABEL_STRING( SM_ONLINE_MAIN_MENU );
        LABEL_STRING( SM_ONLINE_QUICKMATCH );
        LABEL_STRING( SM_ONLINE_OPTIMATCH );
        LABEL_STRING( SM_ONLINE_JOIN_FILTER );
        LABEL_STRING( SM_ONLINE_JOIN_MENU );
        LABEL_STRING( SM_ONLINE_JOIN_SAVE_SETTINGS );
        LABEL_STRING( SM_SERVER_PLAYERS_MENU );
        LABEL_STRING( SM_ONLINE_PLAYERS_MENU );
        LABEL_STRING( SM_ONLINE_FEEDBACK_MENU );
        LABEL_STRING( SM_ONLINE_FEEDBACK_MENU_FRIEND );
        LABEL_STRING( SM_ONLINE_FRIENDS_MENU );
        LABEL_STRING( SM_ONLINE_DOWNLOAD );
        LABEL_STRING( SM_ONLINE_STATS );
        LABEL_STRING( SM_ONLINE_EDIT_PROFILE );
        LABEL_STRING( SM_ONLINE_EDIT_CONTROLS );
        LABEL_STRING( SM_ONLINE_EDIT_AVATAR );
        LABEL_STRING( SM_ONLINE_LOGIN );    

        LABEL_STRING( SM_ONLINE_HOST_MENU );
        LABEL_STRING( SM_ONLINE_HOST_MENU_OPTIONS );
        LABEL_STRING( SM_HOST_LEVEL_SELECT );
        LABEL_STRING( SM_HOST_SAVE_SETTINGS );

#ifndef CONFIG_RETAIL
        LABEL_STRING( SM_LEVEL_SELECT );
#endif
        LABEL_STRING( SM_START_GAME );
        LABEL_STRING( SM_START_SAVE_GAME );
        LABEL_STRING( SM_SINGLE_PLAYER_LOAD_MISSION );
        LABEL_STRING( SM_MULTI_PLAYER_LOAD_MISSION );
        LABEL_STRING( SM_CLIENT_WAIT_FOR_MISSION );
        LABEL_STRING( SM_SERVER_SYNC );
        LABEL_STRING( SM_CLIENT_SYNC );
        LABEL_STRING( SM_SERVER_COOLDOWN );
        LABEL_STRING( SM_CLIENT_COOLDOWN );
        LABEL_STRING( SM_SERVER_DISCONNECT );
        LABEL_STRING( SM_CLIENT_DISCONNECT );

        LABEL_STRING( SM_RESUME_CAMPAIGN );

        LABEL_STRING( SM_PLAYING_GAME );

        LABEL_STRING( SM_PAUSE_MAIN_MENU );
        LABEL_STRING( SM_PAUSE_OPTIONS );
        LABEL_STRING( SM_PAUSE_CONTROLS );
        LABEL_STRING( SM_PAUSE_SETTINGS );
        LABEL_STRING( SM_PAUSE_HEADSET );
        LABEL_STRING( SM_PAUSE_SETTINGS_SELECT );
#ifdef TARGET_XBOX
        LABEL_STRING( SM_PAUSE_FRIENDS );
#endif
#if defined(TARGET_PS2)
        LABEL_STRING( SM_PAUSE_MEMCARD_SELECT );
#endif
        LABEL_STRING( SM_PAUSE_PROFILE_SAVE_SELECT );
        LABEL_STRING( SM_PAUSE_MEMCARD_RESELECT );

        LABEL_STRING( SM_PAUSE_MP );
        LABEL_STRING( SM_PAUSE_MP_SCORE );
        LABEL_STRING( SM_PAUSE_MP_OPTIONS );
        LABEL_STRING( SM_PAUSE_MP_CONTROLS );
        LABEL_STRING( SM_PAUSE_MP_SETTINGS );
        LABEL_STRING( SM_PAUSE_MP_HEADSET );
        LABEL_STRING( SM_PAUSE_MP_SETTINGS_SELECT );
#ifdef TARGET_XBOX
        LABEL_STRING( SM_PAUSE_MP_FRIENDS );
#endif
#if defined(TARGET_PS2)
        LABEL_STRING( SM_PAUSE_MP_MEMCARD_SELECT );
#endif
        LABEL_STRING( SM_PAUSE_MP_PROFILE_SAVE_SELECT );
        LABEL_STRING( SM_PAUSE_MP_MEMCARD_RESELECT );

        LABEL_STRING( SM_PAUSE_ONLINE );
        LABEL_STRING( SM_PAUSE_ONLINE_VOTE_MAP );
        LABEL_STRING( SM_PAUSE_ONLINE_VOTE_KICK );
        LABEL_STRING( SM_PAUSE_ONLINE_FRIENDS );
        LABEL_STRING( SM_PAUSE_ONLINE_PLAYERS );
        LABEL_STRING( SM_PAUSE_ONLINE_FEEDBACK );
        LABEL_STRING( SM_PAUSE_ONLINE_FEEDBACK_FRIEND );
        LABEL_STRING( SM_PAUSE_ONLINE_SERVER_CONFIG );
        LABEL_STRING( SM_PAUSE_ONLINE_OPTIONS );
        LABEL_STRING( SM_PAUSE_ONLINE_CONTROLS );
        LABEL_STRING( SM_PAUSE_ONLINE_SETTINGS );
        LABEL_STRING( SM_PAUSE_ONLINE_HEADSET );
#if defined(TARGET_PS2)
        LABEL_STRING( SM_PAUSE_ONLINE_MEMCARD_SELECT );
#endif
        LABEL_STRING( SM_PAUSE_ONLINE_SAVE_SELECT );
        LABEL_STRING( SM_PAUSE_ONLINE_MEMCARD_RESELECT );
        LABEL_STRING( SM_PAUSE_ONLINE_CHANGE_MAP );
        LABEL_STRING( SM_PAUSE_ONLINE_KICK_PLAYER );

        LABEL_STRING( SM_PAUSE_ONLINE_TEAM_CHANGE );
        LABEL_STRING( SM_PAUSE_ONLINE_RECONFIG_ONE );
        LABEL_STRING( SM_PAUSE_ONLINE_RECONFIG_TWO );
        LABEL_STRING( SM_PAUSE_ONLINE_RECONFIG_MAP );
        LABEL_STRING( SM_PAUSE_ONLINE_SAVE_SETTINGS );

        LABEL_STRING( SM_END_PAUSE );
        LABEL_STRING( SM_EXIT_GAME );
        LABEL_STRING( SM_POST_GAME );

        LABEL_STRING( SM_AUTOSAVE_MENU );
        LABEL_STRING( SM_AUTOSAVE_PROFILE_RESELECT );
#if defined(TARGET_PS2)
        LABEL_STRING( SM_AUTOSAVE_MEMCARD_SELECT );
#endif
        LABEL_STRING( SM_END_AUTOSAVE );

        LABEL_STRING( SM_FINAL_SCORE );
        LABEL_STRING( SM_ADVANCE_LEVEL );
        LABEL_STRING( SM_RELOAD_CHECKPOINT );
        LABEL_STRING( SM_REPORT_ERROR );

        LABEL_STRING( SM_GAME_EXIT_PROMPT_FOR_SAVE );
        LABEL_STRING( SM_GAME_EXIT_SAVE_SELECT );
        LABEL_STRING( SM_GAME_EXIT_MEMCARD_RESELECT );
        LABEL_STRING( SM_GAME_EXIT_SAVE_SETTINGS );
        LABEL_STRING( SM_GAME_EXIT_SETTINGS_OVERWRITE );
        LABEL_STRING( SM_GAME_EXIT_SETTINGS_SELECT );
        LABEL_STRING( SM_GAME_EXIT_REDIRECT );
        LABEL_STRING( SM_GAME_OVER );
        LABEL_STRING( SM_FOLLOW_BUDDY );

        default:                                ASSERTS( FALSE, "Unknown state" );
    }

    return NULL;
}

//=========================================================================

void state_mgr::SimplePopUp( f32 Timeout, const xwchar* Title, const xwchar* Message, const xwchar* Message2 )
{
    irect r = g_UiMgr->GetUserBounds(g_UiUserID);
    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog( g_UiUserID, "popup", r, m_CurrentDialog, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER);
    m_PopUp->Configure( Timeout, Title, Message, Message2 );
    
}

//=========================================================================

void state_mgr::SystemError( sm_system_error MessageID, void* pUserData )
{
#ifndef X_EDITOR
    xbool CanExit = TRUE;
    xwstring MessageText;
    xwstring NavText;

    // No controller errors while profile operations are pending.
    if(( MessageID == SM_SYS_ERR_CONTROLLER ) && 
       ( m_CurrentDialog ) && 
       ( m_CurrentDialog->GetState() == DIALOG_STATE_WAIT_FOR_MEMCARD )) 
    {
        return;
    }

    // Silent auto-save - must wait for dialog to animate
    if(( MessageID == SM_SYS_ERR_CONTROLLER ) &&
        ( GetState() == SM_AUTOSAVE_MENU ) &&
        ( g_UiMgr->IsScreenScaling() ))
    {
        return;
    }

    // enter system error state
    g_StateMgr.m_bDoSystemError    = TRUE;

    // save current paused state, and then set it to pause.
    g_StateMgr.m_bWasPaused        = g_StateMgr.m_bIsPaused;
    g_StateMgr.m_bIsPaused         = TRUE;
    if( (m_State == SM_PLAYING_GAME) && (!g_NetworkMgr.IsOnline()) )
    {
        g_AudioMgr.PauseAll();
    }

    if( g_UiMgr->IsUserEnabled( g_UiUserID ) )
    {
        g_StateMgr.m_bUserWasEnabled = TRUE;
    }
    else
    {
        g_StateMgr.m_bUserWasEnabled = FALSE;
        g_UiMgr->EnableUser( g_UiUserID, TRUE );
    }

    irect r = g_UiMgr->GetUserBounds(g_UiUserID);

    g_StateMgr.m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(
        g_UiUserID,
        "popup",
        r,
        NULL,
        ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

    r.Set( 0, 0, 320, 180 );

    switch( MessageID )
    {
        case SM_SYS_ERR_DISK:
            // Xbox disk error
            MessageText = g_StringTableMgr( "ui", "IDS_HARD_DISK_ERROR" );
            CanExit = FALSE;

            g_StateMgr.m_PopUp->Configure(
                r,
                g_StringTableMgr( "ui", "IDS_SYSTEM_MESSAGE" ), 
                CanExit,
                FALSE,  
                FALSE, 
                MessageText,
                NavText,
                &g_StateMgr.m_PopUpResult );

            break;
        case SM_SYS_ERR_CONTROLLER:
        {
            s32 ControllerID = *(s32*)pUserData;

            g_UiMgr->SetUserController(g_UiUserID, ControllerID);

            if( ControllerID == -1 )
            {   // no controllers connected (not port specific).
                MessageText = g_StringTableMgr( "ui", "IDS_NO_CONTROLLERS" );
            }
            else
            {
                MessageText = g_StringTableMgr( "ui", "IDS_LOST_CONTROLLER" );
                s32 Index = MessageText.Find( 0x0023 );
                ASSERT( Index > -1 );

                // !! if we don't find our string, this will crash. "<NULL>" is better.
                if( Index > -1 )
                {
                    char temp[32];
                    x_sprintf( temp,"%d",ControllerID+1 );

                    xwstring Final;
                    Final += MessageText.Left( Index );
                    Final += xwstring( temp );
                    Final += MessageText.Right( MessageText.GetLength( )-Index-1 );

                    MessageText = Final;
                }
            }
#if defined(TARGET_PS2) 
            NavText = g_StringTableMgr( "ui", "IDS_NAV_CONTROLLER_PS2" );
#elif defined(TARGET_XBOX)
            NavText = g_StringTableMgr( "ui", "IDS_NAV_CONTROLLER_XBOX" );
#endif
#if defined(TARGET_XBOX)
            g_StateMgr.m_PopUp->Configure(
                r,
                g_StringTableMgr( "ui", "IDS_SYSTEM_MESSAGE" ), 
                FALSE,
                FALSE,  
                FALSE,
                TRUE,   // START Button.
                MessageText,
                NavText,
                &g_StateMgr.m_PopUpResult );
#else
            g_StateMgr.m_PopUp->Configure(
                r,
                g_StringTableMgr( "ui", "IDS_SYSTEM_MESSAGE" ), 
                CanExit,
                FALSE,  
                FALSE, 
                MessageText,
                NavText,
                &g_StateMgr.m_PopUpResult );
#endif

            break;
        }

        case SM_SYS_ERR_DUPLICATE_LOGIN :

            MessageText = g_StringTableMgr( "ui", "IDS_ONLINE_DUPLICATE_LOGIN" );
            NavText     = g_StringTableMgr( "ui", "IDS_NAV_NETWORK_CONTINUE"   );

            g_StateMgr.m_PopUp->Configure(
                r,
                g_StringTableMgr( "ui", "IDS_ONLINE_MAIN_XBOX" ), 
                TRUE,
                FALSE,  
                FALSE,
                MessageText,
                NavText,
                &g_StateMgr.m_PopUpResult );

            break;

        default:
            ASSERTS( FALSE, "Unexpected system message!" );
            break;
    }

    // Bad Disc, render the dialog forever...
    if( MessageID == SM_SYS_ERR_DISK ) 
    {
        for(;;)
        {
            g_UiMgr->Render();
            eng_PageFlip();
        }
    }
#else // not editor
    (void)MessageID;
    (void)pUserData; 
#endif
}

//=========================================================================
//
//  Check for pulled controllers.
//

void state_mgr::CheckControllers( void )
{
#if !defined(X_EDITOR)

#ifdef TARGET_PC
    input_gadget ControllerQuery;
    input_gadget AnalogQuery;
    return;
#endif

    if( !g_bControllerCheck )
        return;

    if( !InSystemError() )
    {
        s32 ControllerID;
#if defined(TARGET_PS2)
        input_gadget ControllerQuery = INPUT_PS2_QRY_PAD_PRESENT;
        input_gadget AnalogQuery     = INPUT_PS2_QRY_ANALOG_MODE;
#elif defined(TARGET_XBOX)
        input_gadget ControllerQuery = INPUT_XBOX_QRY_PAD_PRESENT;
        input_gadget AnalogQuery     = INPUT_XBOX_QRY_ANALOG_MODE;
#endif

        switch (m_State)
        {
        case SM_START_GAME:
        case SM_START_SAVE_GAME:
        case SM_SINGLE_PLAYER_LOAD_MISSION:
        case SM_MULTI_PLAYER_LOAD_MISSION:
        case SM_SERVER_SYNC:
        case SM_SERVER_COOLDOWN:
        case SM_SERVER_DISCONNECT:
        case SM_CLIENT_SYNC:
        case SM_CLIENT_COOLDOWN:
        case SM_CLIENT_DISCONNECT:
        case SM_EXIT_GAME:
        case SM_POST_GAME:
            // Not Safe! Not Safe! 
            break;
        case SM_PLAYING_GAME:
            // check all ingame pads with valid IDs
            for( s32 i = 0; i < MAX_LOCAL_PLAYERS; i++ )
            {
                ControllerID = g_IngamePad[i].GetControllerID();
                if( ControllerID == -1 ) 
                    continue;
                if( !(input_IsPresent( ControllerQuery, ControllerID ) &&
                     input_IsPresent( AnalogQuery, ControllerID ) ) )
                {
                    SystemError( SM_SYS_ERR_CONTROLLER, &ControllerID );
                    break;
                }
            }
            break;
#ifdef TARGET_PS2
        case SM_CONTROLLER_CHECK:
            // check for all controllers removed
            ControllerID = -1;
            if( !(input_IsPresent( ControllerQuery, ControllerID ) && 
                  input_IsPresent( AnalogQuery, ControllerID ) ) )
            {
                SystemError( SM_SYS_ERR_CONTROLLER, &ControllerID );
            }
            break;
#endif
        default:
            // only check for "locked" controllers
            if( (ControllerID = GetActiveControllerID()) != -1 )
            {
                if( !(input_IsPresent( ControllerQuery, ControllerID ) &&
                      input_IsPresent( AnalogQuery, ControllerID ) ) )
                {
                    SystemError( SM_SYS_ERR_CONTROLLER, &ControllerID );
                }
            }
        }
    }

#endif // !X_EDITOR
}

//=========================================================================

void state_mgr::Update( f32 DeltaTime )
{
    if( s_Delay )
        s_Delay--;

    // update timeout
    m_Timeout -= DeltaTime;

    // Do the memory card update
    g_UIMemCardMgr.Update( DeltaTime );

    // Process UI input
    g_UiMgr->ProcessInput( DeltaTime );

    // Update based on state
    UpdateState( m_State, DeltaTime );

    // update UI
    g_UiMgr->Update( DeltaTime );

    if( m_bDoSystemError && m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if (( m_PopUpResult == DLG_POPUP_YES ) ||
                ( m_PopUpResult == DLG_POPUP_HELP )) // Used for Xbox on lost controllers.
            {
                m_bDoSystemError = FALSE;
                m_bIsPaused = m_bWasPaused;
                m_PopUp = NULL;
                if( !m_bUserWasEnabled )
                {
                    g_UiMgr->EnableUser( g_UiUserID, FALSE );
                }

                if( m_State == SM_PLAYING_GAME )
                {
                    m_bIsPaused = FALSE;
                    g_AudioMgr.ResumeAll(); 
                }

                if( m_bRetrySilentAutoSave )
                {
                    SilentSaveProfile();
                }

                if( m_bRetryAutoSaveMenu )
                {
                    // goto autosave menu
                    m_bRetryAutoSaveMenu = FALSE;
                    SetState( SM_AUTOSAVE_MENU );
                }
                
                if( m_bIsDuplicateLogin == TRUE )
                {
                    m_bIsDuplicateLogin = FALSE;
                    SetState( SM_MAIN_MENU, TRUE );
                }
            }
        }
    }

    // update menu music
    if( ( m_State > SM_PRESS_START_SCREEN ) && ( m_State < SM_END_PAUSE ) && ( !IsPaused() ) )
    {
        switch( m_State ) 
        {
            case SM_PLAYING_GAME:
            case SM_MULTI_PLAYER_LOAD_MISSION:
            case SM_SINGLE_PLAYER_LOAD_MISSION:
            case SM_START_GAME:
            case SM_START_SAVE_GAME:
            {
                // kill background music
                KillFrontEndMusic();
            }
            break;

            case SM_SECRETS_MENU:
            {
                if( m_bPlayMovie )
                {
                    // keep it looping
                    if( !g_AudioMgr.IsValidVoiceId( s_VoiceID ) )
                    {
                        if( !s_Delay )
                        {
                            InitFrontEndMusic();

                            s_Delay = 5;
                        }
                    }
                }
            }
            break;

            default:
            {
                // keep it looping
                if( !g_AudioMgr.IsValidVoiceId( s_VoiceID ) )
                {
                    if( !s_Delay )
                    {
                        InitFrontEndMusic();
                        s_Delay = 5;
                    }
                }
            }
            break;
        }
    }
    else
    {
        // kill background music
        KillFrontEndMusic();
    }

    #ifdef TARGET_XBOX
    //
    // Detect duplicate logins
    //
    {
        s32   State     = GetState();
        xbool IsLoading = (State == SM_SINGLE_PLAYER_LOAD_MISSION)  ||
                          (State == SM_MULTI_PLAYER_LOAD_MISSION)   ||
                          (State == SM_SERVER_SYNC)                 ||
                          (State == SM_CLIENT_SYNC);

        // We must wait until loading has finished before showing the duplicate logon error
        if( !IsLoading )
        {
            if( g_MatchMgr.IsDuplicateLogin() == TRUE )
            {
                g_MatchMgr.ClearDuplicateLogin();

                // There are 2 possible ways of informing the player of a duplicate login.
                // The first method is used if the player is in an online game.  The game
                // exits immediately and the error dialog is displayed.  The second method
                // uses the System Error dialog and it is used for ALL other cases.
                xbool ShowErrorDialog = FALSE;

                if( GameMgr.GameInProgress() == TRUE )
                {
                    // If we are playing in an online game then we must exit that game and
                    // use the error report dialog to inform the player why the game exited.
                    if( GameMgr.IsGameOnline() == TRUE )
                    {
                        g_ActiveConfig.SetExitReason( GAME_EXIT_DUPLICATE_LOGIN );
                        ShowErrorDialog = TRUE;
                    }
                }

                if( ShowErrorDialog == FALSE )
                {
                    // If we are in the frontend then we must quit out to the main menu after showing
                    // the system error dialog.  If the player is in campaign or split screen however,
                    // we must return back to the game in progress.
                    if( GameMgr.GameInProgress() == FALSE )
                    {
                        g_StateMgr.SetActiveControllerID( -1 );
                        g_UiMgr->SetUserController      ( g_UiUserID, -1 );
                        g_UiMgr->EndUsersDialogs        ( g_UiUserID );

                        m_bIsDuplicateLogin = TRUE;
                    }

                    s32 Var = 0;
                    SystemError( SM_SYS_ERR_DUPLICATE_LOGIN, &Var );
                }
            }
        }
    }

    //
    // Detect headset being inserted
    //

    headset& Headset = g_VoiceMgr.GetHeadset();
    
    if( Headset.HeadsetJustInserted() == TRUE )
    {
        Headset.ClearHeadsetJustInserted();

        // The headset was just inserted so toggle the UI option to NORMAL
        global_settings& Settings = GetActiveSettings();
        Settings.SetHeadsetMode( HEADSET_HEADSET_ONLY );
        Settings.Commit();
    }

    #endif
}


//=========================================================================

void state_mgr::Render( void )
{
#ifdef TARGET_XBOX
    // SB:
    // For xbox, make sure the sat compensation is disabled before rendering
    // the dialogs otherwise if a server closes down and the client was in
    // mutant mode, the sat compensation dims down all the dialogs.
    draw_DisableSatCompensation();
#endif    

   if ( ( m_State != SM_PLAYING_GAME ) || ( m_bDoSystemError ) )
   {
        // Setup camera view
        eng_MaximizeViewport( m_View );
        eng_SetView ( m_View );
        eng_SetBackColor( xcolor( 0,0,0 ) );

#if defined( USE_MOVIES )
        //vector3 Pos( 0.0f, 0.0f, 0.0f );
        //vector2 Size ( 512, 448 );

        // render the movie in the background
        if( m_bPlayMovie )
        {
            if( Movie.IsPlaying() )
            {
                Movie.Render( m_MoviePosition, m_MovieSize );
            }
        }
#endif

        // Render UI
        g_UiMgr->Render();
    }
}

//=========================================================================

void state_mgr::DummyScreen( const char* message, xbool canSkip, s32 waitTime )
{
    s32 X,Y;
    eng_GetRes( X,Y );
    f32 YRes = (f32)Y;

#if (defined(bwatson)||defined(jpcossigny)) && defined(X_DEBUG)
    waitTime = 0;
#endif
    //==-----------------------------------------
    //  Run
    //==-----------------------------------------
    xtimer  Time;
    xbool   bSkip  = FALSE;

    Time.Start();
    while (Time.ReadSec() < waitTime && !bSkip)
    {
        eng_MaximizeViewport( m_View );
        eng_SetView ( m_View );
        eng_SetBackColor( xcolor(0,0,0) );
        eng_Begin("DummyScreen");

        f32 Size = YRes;
        f32 HalfSize = Size/2;
        f32 W,H;

        W = HalfSize;
        H = W/1.3f;

        s32     Line = 12;
#ifdef TARGET_PS2
        x_printfxy( 19-x_strlen(message)/2, Line++, message );
#else
        x_printfxy( 30-x_strlen(message)/2, Line++, message );
#endif

        eng_End();
        eng_PageFlip();
        if( canSkip )
        {
            g_InputMgr.Update( 0.0167f );
            { 
#ifdef TARGET_PC
                if( input_IsPressed( INPUT_MSG_EXIT ) )
                    bSkip = TRUE;
                if( input_WasPressed( INPUT_KBD_RETURN, 0 ) || input_WasPressed( INPUT_KBD_ESCAPE, 0 ) || input_WasPressed( INPUT_KBD_SPACE, 0 ) ) 
                    bSkip = TRUE;
#endif

#ifdef TARGET_XBOX
                if (input_WasPressed( INPUT_XBOX_BTN_START ))
                    bSkip = TRUE;
#endif

#ifdef TARGET_PS2
                if (input_WasPressed( INPUT_PS2_BTN_CROSS ))
                    bSkip = TRUE;
#endif
            }    
        }
    }
}

//=========================================================================

void state_mgr::EnterState( sm_states State )
{
    switch( State )
    {
        case SM_ESRB_NOTICE:                    EnterESRBNotice();                  break;
        case SM_INEVITABLE_INTRO:               EnterInevitableIntro();             break;
#if defined(TARGET_PS2)
        case SM_AUTOSAVE_DIALOG:                EnterAutoSaveDialog();              break;
        case SM_CONTROLLER_CHECK:               EnterControllerCheck();             break;
#endif
        case SM_MEMCARD_BOOT_CHECK:             EnterMemcardBootCheck();            break;
        case SM_PRESS_START_SCREEN:             EnterPressStart();                  break;
        case SM_MAIN_MENU:                      EnterMainMenu();                    break;
        case SM_SETTINGS_MENU:                  EnterSettingsMenu();                break;
        case SM_SETTINGS_HEADSET:               EnterSettingsHeadset();             break;
        case SM_SETTINGS_MEMCARD_SELECT:        EnterSettingsMemcardSelect();       break;
        case SM_MANAGE_PROFILES:                EnterManageProfiles();              break;
        case SM_MANAGE_PROFILE_OPTIONS:         EnterManageProfileOptions();        break;
        case SM_MANAGE_PROFILE_CONTROLS:        EnterManageProfileControls();       break;
        case SM_MANAGE_PROFILE_AVATAR:          EnterManageProfileAvatar();         break;
#if defined(TARGET_PS2)
        case SM_MANAGE_MEMCARD_SELECT:          EnterManageMemcardSelect();         break;
#endif
        case SM_MANAGE_PROFILE_SAVE_SELECT:     EnterManageProfileSaveSelect();     break;
        case SM_MANAGE_PROFILE_MEMCARD_RESELECT:EnterManageMemcardReselect();       break;
        case SM_CAMPAIGN_MENU:                  EnterCampaignMenu();                break;
        case SM_CAMPAIGN_PROFILE_OPTIONS:       EnterCampaignProfileOptions();      break;
        case SM_CAMPAIGN_PROFILE_CONTROLS:      EnterCampaignProfileControls();     break;
        case SM_CAMPAIGN_PROFILE_AVATAR:        EnterCampaignProfileAvatar();       break;
#if defined(TARGET_PS2)
        case SM_CAMPAIGN_MEMCARD_SELECT:        EnterCampaignMemcardSelect();       break;
#endif
        case SM_CAMPAIGN_PROFILE_SAVE_SELECT:   EnterCampaignProfileSaveSelect();   break;
        case SM_CAMPAIGN_MEMCARD_RESELECT:      EnterCampaignMemcardReselect();     break;
#if defined(TARGET_PS2)
        case SM_MEMCARD_SELECT:                 EnterMemcardSelect();               break;
#endif
        case SM_LOAD_CAMPAIGN:                  EnterLoadCampaign();                break;
        case SM_SAVE_CAMPAIGN:                  EnterSaveCampaign();                break;
        case SM_LORE_MAIN_MENU:                 EnterLoreMainMenu();                break;
        case SM_SECRETS_MENU:                   EnterSecretsMenu();                 break;
        case SM_EXTRAS_MENU:                    EnterExtrasMenu();                  break;
        case SM_CREDITS_SCREEN:                 EnterCreditsScreen();               break;
        case SM_PROFILE_SELECT:                 EnterProfileSelect();               break;
        case SM_PROFILE_OPTIONS:                EnterProfileOptions();              break;
        case SM_PROFILE_CONTROLS:               EnterProfileControls();             break;
        case SM_PROFILE_AVATAR:                 EnterProfileAvatar();               break;
        case SM_PROFILE_AVATAR_MP:              EnterProfileAvatarMP();             break;
#if defined(TARGET_PS2)
        case SM_MEMCARD_SELECT_MP:              EnterMemcardSelectMP();             break;
#endif
        case SM_PROFILE_SAVE_SELECT_MP:         EnterProfileSaveSelectMP();         break;
        case SM_MEMCARD_RESELECT_MP:            EnterMemcardReselectMP();           break;
        case SM_MULTI_PLAYER_MENU:              EnterMultiPlayer();                 break;
        case SM_MULTI_PLAYER_OPTIONS:           EnterMultiPlayerOptions();          break;
        case SM_MULTI_PLAYER_EDIT:              EnterMultiPlayerEdit();             break;
        case SM_MP_LEVEL_SELECT:                EnterMPLevelSelect();               break;
        case SM_MP_SAVE_SETTINGS:               EnterMPSaveSettings();              break;
        case SM_PROFILE_CONTROLS_MP:            EnterProfileControlsMP();           break;
        case SM_ONLINE_SILENT_LOGON:            EnterOnlineSilentLogin();           break;
#ifdef TARGET_PS2
        case SM_ONLINE_EULA:                    EnterOnlineEULA();                  break;
#endif
        case SM_ONLINE_CONNECT:                 EnterOnlineConnect();               break;
        case SM_ONLINE_AUTHENTICATE:            EnterOnlineAuthenticate();          break;
#ifndef TARGET_XBOX
        case SM_ONLINE_COPA:                    EnterOnlineCOPA();                  break;
        case SM_ONLINE_COPA_SAVE_SELECT:        EnterOnlineCOPASaveSelect();        break;
        case SM_ONLINE_COPA_MEMCARD_RESELECT:   EnterOnlineCOPAMemcardReselect();   break;
#endif
        case SM_ONLINE_PROFILE_SELECT:          EnterOnlineProfileSelect();         break;
        case SM_ONLINE_PROFILE_OPTIONS:         EnterOnlineProfileOptions();        break;
        case SM_ONLINE_PROFILE_CONTROLS:        EnterOnlineProfileControls();       break;
        case SM_ONLINE_PROFILE_AVATAR:          EnterOnlineProfileAvatar();         break;
#if defined(TARGET_PS2)
        case SM_ONLINE_MEMCARD_SELECT:          EnterOnlineMemcardSelect();         break;
#endif
        case SM_ONLINE_PROFILE_SAVE_SELECT:     EnterOnlineProfileSaveSelect();     break;
        case SM_ONLINE_MEMCARD_RESELECT:        EnterOnlineMemcardReselect();       break;
        case SM_ONLINE_MAIN_MENU:               EnterOnlineMain();                  break;
        case SM_ONLINE_HOST_MENU:               EnterOnlineHost();                  break;
        case SM_ONLINE_HOST_MENU_OPTIONS:       EnterOnlineHostOptions();           break;
        case SM_HOST_SAVE_SETTINGS:             EnterHostSaveSettings();            break;
        case SM_ONLINE_QUICKMATCH:              EnterOnlineQuickMatch();            break;
        case SM_ONLINE_OPTIMATCH:               EnterOnlineOptiMatch();             break;
        case SM_ONLINE_JOIN_FILTER:             EnterOnlineJoinFilter();            break;
        case SM_ONLINE_JOIN_MENU:               EnterOnlineJoin();                  break;
        case SM_ONLINE_JOIN_SAVE_SETTINGS:      EnterOnlineJoinSaveSettings();      break;
        case SM_SERVER_PLAYERS_MENU:            EnterServerPlayers();               break;
        case SM_ONLINE_PLAYERS_MENU:            EnterOnlinePlayers();               break;
        case SM_ONLINE_FEEDBACK_MENU:           EnterOnlineFeedback();              break;
        case SM_ONLINE_FEEDBACK_MENU_FRIEND:    EnterOnlineFeedbackFriend();        break;
        case SM_ONLINE_FRIENDS_MENU:            EnterOnlineFriends();               break;
        case SM_ONLINE_STATS:                   EnterOnlineStats();                 break;
        case SM_ONLINE_EDIT_PROFILE:            EnterOnlineEditProfile();           break;
        case SM_ONLINE_EDIT_CONTROLS:           EnterOnlineEditControls();          break;
        case SM_ONLINE_EDIT_AVATAR:             EnterOnlineEditAvatar();            break;
        case SM_HOST_LEVEL_SELECT:              EnterHostLevelSelect();             break;
#ifndef CONFIG_RETAIL
        case SM_LEVEL_SELECT:                   EnterLevelSelect();                 break;
#endif
        case SM_RESUME_CAMPAIGN:                EnterResumeCampaign();              break;
        case SM_START_SAVE_GAME:                EnterStartSaveGame();               break;
        case SM_START_GAME:                     EnterStartGame();                   break;
        case SM_SERVER_SYNC:                    EnterServerSync();                  break;
        case SM_CLIENT_SYNC:                    EnterClientSync();                  break;
        case SM_MULTI_PLAYER_LOAD_MISSION:      EnterMultiPlayerLoadMission();      break;
        case SM_SINGLE_PLAYER_LOAD_MISSION:     EnterSinglePlayerLoadMission();     break;
        case SM_SERVER_COOLDOWN:                EnterServerCooldown();              break;
        case SM_CLIENT_COOLDOWN:                EnterClientCooldown();              break;
        case SM_SERVER_DISCONNECT:              EnterServerDisconnect();            break;
        case SM_CLIENT_DISCONNECT:              EnterClientDisconnect();            break;
        case SM_PAUSE_MAIN_MENU:                EnterPauseMain();                   break;
        case SM_PAUSE_OPTIONS:                  EnterPauseOptions();                break;
        case SM_PAUSE_CONTROLS:                 EnterPauseControls();               break;
        case SM_PAUSE_SETTINGS:                 EnterPauseSettings();               break;
        case SM_PAUSE_HEADSET:                  EnterPauseHeadset();                break;
        case SM_PAUSE_SETTINGS_SELECT:          EnterPauseSettingsSelect();         break;
#ifdef TARGET_XBOX
        case SM_PAUSE_FRIENDS:                  EnterPauseFriends();                break;
#endif
#if defined(TARGET_PS2)
        case SM_PAUSE_MEMCARD_SELECT:           EnterPauseMemcardSelect();          break;
#endif
        case SM_PAUSE_PROFILE_SAVE_SELECT:      EnterPauseMemcardSaveSelect();      break;
        case SM_PAUSE_MEMCARD_RESELECT:         EnterPauseMemcardReselect();        break;
        case SM_PAUSE_MP:                       EnterPauseMP();                     break;
        case SM_PAUSE_MP_OPTIONS:               EnterPauseMPOptions();              break;
        case SM_PAUSE_MP_CONTROLS:              EnterPauseMPControls();             break;
        case SM_PAUSE_MP_SETTINGS:              EnterPauseMPSettings();             break;
        case SM_PAUSE_MP_HEADSET:               EnterPauseMPHeadset();              break;
        case SM_PAUSE_MP_SETTINGS_SELECT:       EnterPauseMPSettingsSelect();       break;
#ifdef TARGET_XBOX
        case SM_PAUSE_MP_FRIENDS:               EnterPauseMPFriends();              break;
#endif
#if defined(TARGET_PS2)
        case SM_PAUSE_MP_MEMCARD_SELECT:        EnterPauseMPMemcardSelect();        break;
#endif
        case SM_PAUSE_MP_PROFILE_SAVE_SELECT:   EnterPauseMPMemcardSaveSelect();    break;
        case SM_PAUSE_MP_MEMCARD_RESELECT:      EnterPauseMPMemcardReselect();      break;
        case SM_PAUSE_ONLINE:                   EnterPauseOnline();                 break;
        case SM_PAUSE_ONLINE_VOTE_MAP:          EnterPauseOnlineVoteMap();          break;
        case SM_PAUSE_ONLINE_VOTE_KICK:         EnterPauseOnlineVoteKick();         break;
        case SM_PAUSE_ONLINE_FRIENDS:           EnterPauseOnlineFriends();          break;
        case SM_PAUSE_ONLINE_PLAYERS:           EnterPauseOnlinePlayers();          break;
        case SM_PAUSE_ONLINE_FEEDBACK:          EnterPauseOnlineFeedback();         break;
        case SM_PAUSE_ONLINE_FEEDBACK_FRIEND:   EnterPauseOnlineFeedbackFriend();   break;
        case SM_PAUSE_ONLINE_SERVER_CONFIG:     EnterPauseServerConfig();           break;
        case SM_PAUSE_ONLINE_OPTIONS:           EnterPauseOnlineOptions();          break;
        case SM_PAUSE_ONLINE_CONTROLS:          EnterPauseOnlineControls();         break;
        case SM_PAUSE_ONLINE_SETTINGS:          EnterPauseOnlineSettings();         break;
        case SM_PAUSE_ONLINE_HEADSET:           EnterPauseOnlineHeadset();          break;
#if defined(TARGET_PS2)
        case SM_PAUSE_ONLINE_MEMCARD_SELECT:    EnterPauseOnlineMemcardSelect();    break;
#endif
        case SM_PAUSE_ONLINE_SAVE_SELECT:       EnterPauseOnlineSaveSelect();       break;
        case SM_PAUSE_ONLINE_MEMCARD_RESELECT:  EnterPauseOnlineMemcardReselect();  break;
        case SM_PAUSE_ONLINE_CHANGE_MAP:        EnterPauseOnlineChangeMap();        break;
        case SM_PAUSE_ONLINE_KICK_PLAYER:       EnterPauseOnlineKickPlayer();       break;
        case SM_PAUSE_ONLINE_TEAM_CHANGE:       EnterPauseOnlineTeamChange();       break;
        case SM_PAUSE_ONLINE_RECONFIG_ONE:      EnterPauseOnlineReconfigOne();      break;
        case SM_PAUSE_ONLINE_RECONFIG_TWO:      EnterPauseOnlineReconfigTwo();      break;
        case SM_PAUSE_ONLINE_RECONFIG_MAP:      EnterPauseOnlineReconfigMap();      break;
        case SM_PAUSE_ONLINE_SAVE_SETTINGS:     EnterPauseOnlineSaveSettings();     break;
        case SM_PAUSE_MP_SCORE:                 EnterPauseMPScore();                break;
        case SM_END_PAUSE:                      EnterEndPause();                    break;
        case SM_AUTOSAVE_MENU:                  EnterAutosaveMenu();                break;
        case SM_AUTOSAVE_PROFILE_RESELECT:      EnterAutosaveProfileReselect();     break;
#if defined(TARGET_PS2)
        case SM_AUTOSAVE_MEMCARD_SELECT:        EnterAutosaveMemcardSelect();       break;
#endif
        case SM_END_AUTOSAVE:                   EnterEndAutosave();                 break;
        case SM_POST_GAME:                      EnterPostGame();                    break;
        case SM_EXIT_GAME:                      EnterExitGame();                    break;
        case SM_ADVANCE_LEVEL:                  EnterAdvanceLevel();                break;
        case SM_RELOAD_CHECKPOINT:              EnterReloadCheckpoint();            break;
        case SM_FINAL_SCORE:                    EnterFinalScore();                  break;
#if CONFIG_IS_DEMO
        case SM_DEMO_EXIT:                      EnterDemoExit();                    break;
#endif
        case SM_REPORT_ERROR:                   EnterReportError();                 break;
        case SM_GAME_EXIT_PROMPT_FOR_SAVE:      EnterGameExitPromptForSave();       break;
        case SM_GAME_EXIT_SAVE_SELECT:          EnterGameExitSaveSelect();          break;
        case SM_GAME_EXIT_MEMCARD_RESELECT:     EnterGameExitMemcardReselect();     break;
        case SM_GAME_EXIT_SAVE_SETTINGS:        EnterGameExitSaveSettings();        break;
        case SM_GAME_EXIT_SETTINGS_OVERWRITE:   EnterGameExitSettingsOverwrite();   break;
        case SM_GAME_EXIT_SETTINGS_SELECT:      EnterGameExitSettingsSelect();      break;
        case SM_GAME_EXIT_REDIRECT:             EnterGameExitRedirect();            break;
        case SM_GAME_OVER:                      EnterGameOver();                    break;

#if defined(TARGET_PS2)
        case SM_ONLINE_DOWNLOAD:                EnterOnlineDownload();              break;
#endif
        case SM_ONLINE_LOGIN:                   EnterOnlineLogin();                 break;
        case SM_FOLLOW_BUDDY:                   EnterFollowBuddy();                 break;
        case SM_PLAYING_GAME:                   EnterPlayingGame();                 break;

        // No setup required
        default:
            break;
    }
}

//=========================================================================

void state_mgr::ExitState( sm_states State )
{
    switch( State )
    {
        case SM_END_PAUSE:                      ExitEndPause();                 break;
        case SM_END_AUTOSAVE:                   ExitEndAutosave();              break;
        case SM_PAUSE_MP:                       ExitPauseMP();                  break;
        case SM_PAUSE_ONLINE:                   ExitPauseOnline();              break;
        case SM_SAVE_CAMPAIGN:                  ExitSaveCampaign();             break;
        case SM_EXIT_GAME:                      ExitExitGame();                 break;
        case SM_MULTI_PLAYER_LOAD_MISSION:      ExitMultiPlayerLoadMission();   break;
        case SM_SINGLE_PLAYER_LOAD_MISSION:     ExitSinglePlayerLoadMission();  break;
        case SM_SERVER_COOLDOWN:                ExitServerCooldown();           break;
        case SM_CLIENT_COOLDOWN:                ExitClientCooldown();           break;
        case SM_SERVER_DISCONNECT:              ExitServerDisconnect();         break;
        case SM_CLIENT_DISCONNECT:              ExitClientDisconnect();         break;
        case SM_SERVER_SYNC:                    ExitServerSync();               break;
        case SM_CLIENT_SYNC:                    ExitClientSync();               break;
        case SM_PRESS_START_SCREEN:             ExitPressStart();               break;
        case SM_ONLINE_LOGIN:                   ExitOnlineLogin();              break;
        case SM_PLAYING_GAME:                   ExitPlayingGame();              break;
        case SM_ONLINE_QUICKMATCH:              ExitOnlineQuickMatch();         break;

        // No clean up required
        default:
            break;
    }
}

//=========================================================================

void state_mgr::UpdateState( sm_states State, f32 DeltaTime )
{
    (void)DeltaTime;

    // Update
    switch( State )
    {
        case SM_ESRB_NOTICE:                    UpdateESRBNotice();                 break;
        case SM_INEVITABLE_INTRO:               UpdateInevitableIntro();            break;
#if defined(TARGET_PS2)
        case SM_CONTROLLER_CHECK:               UpdateControllerCheck();            break;
        case SM_AUTOSAVE_DIALOG:                UpdateAutoSaveDialog();             break;
#endif
        case SM_MEMCARD_BOOT_CHECK:             UpdateMemcardBootCheck();           break;
        case SM_PRESS_START_SCREEN:             UpdatePressStart();                 break;
        case SM_MAIN_MENU:                      UpdateMainMenu();                   break;
        case SM_SETTINGS_MENU:                  UpdateSettingsMenu();               break;
        case SM_SETTINGS_HEADSET:               UpdateSettingsHeadset();            break;
        case SM_SETTINGS_MEMCARD_SELECT:        UpdateSettingsMemcardSelect();      break;

        case SM_MANAGE_PROFILES:                UpdateManageProfiles();             break;
        case SM_MANAGE_PROFILE_OPTIONS:         UpdateManageProfileOptions();       break;
        case SM_MANAGE_PROFILE_CONTROLS:        UpdateManageProfileControls();      break;
        case SM_MANAGE_PROFILE_AVATAR:          UpdateManageProfileAvatar();        break;
#if defined(TARGET_PS2)
        case SM_MANAGE_MEMCARD_SELECT:          UpdateManageMemcardSelect();        break;
#endif
        case SM_MANAGE_PROFILE_SAVE_SELECT:     UpdateManageProfileSaveSelect();    break;
        case SM_MANAGE_PROFILE_MEMCARD_RESELECT:UpdateManageMemcardReselect();      break;

        case SM_CAMPAIGN_MENU:                  UpdateCampaignMenu();               break;
        case SM_CAMPAIGN_PROFILE_OPTIONS:       UpdateCampaignProfileOptions();     break;
        case SM_CAMPAIGN_PROFILE_CONTROLS:      UpdateCampaignProfileControls();    break;
        case SM_CAMPAIGN_PROFILE_AVATAR:        UpdateCampaignProfileAvatar();      break;
#if defined(TARGET_PS2)
        case SM_CAMPAIGN_MEMCARD_SELECT:        UpdateCampaignMemcardSelect();      break;
#endif
        case SM_CAMPAIGN_PROFILE_SAVE_SELECT:   UpdateCampaignProfileSaveSelect();  break;
        case SM_CAMPAIGN_MEMCARD_RESELECT:      UpdateCampaignMemcardReselect();    break;
        case SM_LOAD_CAMPAIGN:                  UpdateLoadCampaign();               break;
        case SM_SAVE_CAMPAIGN:                  UpdateSaveCampaign();               break;
        case SM_LORE_MAIN_MENU:                 UpdateLoreMainMenu();               break;
        case SM_SECRETS_MENU:                   UpdateSecretsMenu();                break;
        case SM_EXTRAS_MENU:                    UpdateExtrasMenu();                 break;
        case SM_CREDITS_SCREEN:                 UpdateCreditsScreen();              break;
#if defined(TARGET_PS2)
        case SM_MEMCARD_SELECT:                 UpdateMemcardSelect();              break;
#endif
        case SM_PROFILE_SELECT:                 UpdateProfileSelect();              break;
        case SM_PROFILE_OPTIONS:                UpdateProfileOptions();             break;
        case SM_PROFILE_CONTROLS:               UpdateProfileControls();            break;
        case SM_PROFILE_AVATAR:                 UpdateProfileAvatar();              break;
        case SM_MULTI_PLAYER_MENU:              UpdateMultiPlayer();                break;
        case SM_MULTI_PLAYER_OPTIONS:           UpdateMultiPlayerOptions();         break;
        case SM_MULTI_PLAYER_EDIT:              UpdateMultiPlayerEdit();            break;
        case SM_MP_LEVEL_SELECT:                UpdateMPLevelSelect();              break;
        case SM_MP_SAVE_SETTINGS:               UpdateMPSaveSettings();             break;
        case SM_PROFILE_CONTROLS_MP:            UpdateProfileControlsMP();          break;
        case SM_PROFILE_AVATAR_MP:              UpdateProfileAvatarMP();            break;
#if defined(TARGET_PS2)
        case SM_MEMCARD_SELECT_MP:              UpdateMemcardSelectMP();            break;
#endif
        case SM_PROFILE_SAVE_SELECT_MP:         UpdateProfileSaveSelectMP();        break;
        case SM_MEMCARD_RESELECT_MP:            UpdateMemcardReselectMP();          break;
#ifdef TARGET_PS2
        case SM_ONLINE_EULA:                    UpdateOnlineEULA();                 break;
#endif
        case SM_ONLINE_CONNECT:                 UpdateOnlineConnect();              break;
        case SM_ONLINE_AUTHENTICATE:            UpdateOnlineAuthenticate();         break;
#ifndef TARGET_XBOX
        case SM_ONLINE_COPA:                    UpdateOnlineCOPA();                 break;
        case SM_ONLINE_COPA_SAVE_SELECT:        UpdateOnlineCOPASaveSelect();       break;
        case SM_ONLINE_COPA_MEMCARD_RESELECT:   UpdateOnlineCOPAMemcardReselect();  break;
#endif
        case SM_ONLINE_PROFILE_SELECT:          UpdateOnlineProfileSelect();        break;
        case SM_ONLINE_PROFILE_OPTIONS:         UpdateOnlineProfileOptions();       break;
        case SM_ONLINE_PROFILE_CONTROLS:        UpdateOnlineProfileControls();      break;
        case SM_ONLINE_PROFILE_AVATAR:          UpdateOnlineProfileAvatar();        break;
#if defined(TARGET_PS2)
        case SM_ONLINE_MEMCARD_SELECT:          UpdateOnlineMemcardSelect();        break;
#endif
        case SM_ONLINE_PROFILE_SAVE_SELECT:     UpdateOnlineProfileSaveSelect();    break;
        case SM_ONLINE_MEMCARD_RESELECT:        UpdateOnlineMemcardReselect();      break;
        case SM_ONLINE_MAIN_MENU:               UpdateOnlineMain();                 break;
        case SM_ONLINE_HOST_MENU:               UpdateOnlineHost();                 break;
        case SM_ONLINE_HOST_MENU_OPTIONS:       UpdateOnlineHostOptions();          break;
        case SM_HOST_SAVE_SETTINGS:             UpdateHostSaveSettings();           break;
        case SM_ONLINE_QUICKMATCH:              UpdateOnlineQuickMatch();           break;
        case SM_ONLINE_OPTIMATCH:               UpdateOnlineOptiMatch();            break;
        case SM_ONLINE_JOIN_FILTER:             UpdateOnlineJoinFilter();           break;
        case SM_ONLINE_JOIN_MENU:               UpdateOnlineJoin();                 break;
        case SM_ONLINE_JOIN_SAVE_SETTINGS:      UpdateOnlineJoinSaveSettings();     break;
        case SM_SERVER_PLAYERS_MENU:            UpdateServerPlayers();              break;
        case SM_ONLINE_PLAYERS_MENU:            UpdateOnlinePlayers();              break;
        case SM_ONLINE_FEEDBACK_MENU:           UpdateOnlineFeedback();             break;
        case SM_ONLINE_FEEDBACK_MENU_FRIEND:    UpdateOnlineFeedbackFriend();       break;
        case SM_ONLINE_FRIENDS_MENU:            UpdateOnlineFriends();              break;
        case SM_ONLINE_STATS:                   UpdateOnlineStats();                break;
        case SM_ONLINE_EDIT_PROFILE:            UpdateOnlineEditProfile();          break;
        case SM_ONLINE_EDIT_CONTROLS:           UpdateOnlineEditControls();         break;
        case SM_ONLINE_EDIT_AVATAR:             UpdateOnlineEditAvatar();           break;
        case SM_HOST_LEVEL_SELECT:              UpdateHostLevelSelect();            break;
#ifndef CONFIG_RETAIL
        case SM_LEVEL_SELECT:                   UpdateLevelSelect();                break;
#endif
        case SM_RESUME_CAMPAIGN:                UpdateResumeCampaign();             break;
        case SM_START_SAVE_GAME:                UpdateStartSaveGame();              break;
        //
        //  Loading and Unloading
        //
        case SM_START_GAME:                     UpdateStartGame();                  break;     
        case SM_SERVER_SYNC:                    UpdateServerSync();                 break;
        case SM_CLIENT_SYNC:                    UpdateClientSync();                 break;
        case SM_MULTI_PLAYER_LOAD_MISSION:      UpdateMultiPlayerLoadMission();     break;
        case SM_SINGLE_PLAYER_LOAD_MISSION:     UpdateSinglePlayerLoadMission();    break;
        case SM_PLAYING_GAME:                   UpdatePlayingGame();                break;
        case SM_SERVER_COOLDOWN:                UpdateServerCooldown();             break;
        case SM_CLIENT_COOLDOWN:                UpdateClientCooldown();             break;
        case SM_SERVER_DISCONNECT:              UpdateServerDisconnect();           break;
        case SM_CLIENT_DISCONNECT:              UpdateClientDisconnect();           break;

        //
        // In game pause menu
        //
        case SM_PAUSE_MAIN_MENU:                UpdatePauseMain();                  break;
        case SM_PAUSE_OPTIONS:                  UpdatePauseOptions();               break;
        case SM_PAUSE_CONTROLS:                 UpdatePauseControls();              break;
        case SM_PAUSE_SETTINGS:                 UpdatePauseSettings();              break;
        case SM_PAUSE_HEADSET:                  UpdatePauseHeadset();               break;
        case SM_PAUSE_SETTINGS_SELECT:          UpdatePauseSettingsSelect();        break;
#ifdef TARGET_XBOX
        case SM_PAUSE_FRIENDS:                  UpdatePauseFriends();               break;
#endif
#if defined(TARGET_PS2)
        case SM_PAUSE_MEMCARD_SELECT:           UpdatePauseMemcardSelect();         break;
#endif
        case SM_PAUSE_PROFILE_SAVE_SELECT:      UpdatePauseMemcardSaveSelect();     break;
        case SM_PAUSE_MEMCARD_RESELECT:         UpdatePauseMemcardReselect();       break;
        case SM_PAUSE_MP:                       UpdatePauseMP();                    break;
        case SM_PAUSE_MP_OPTIONS:               UpdatePauseMPOptions();             break;
        case SM_PAUSE_MP_CONTROLS:              UpdatePauseMPControls();            break;
        case SM_PAUSE_MP_SETTINGS:              UpdatePauseMPSettings();            break;
        case SM_PAUSE_MP_HEADSET:               UpdatePauseMPHeadset();             break;
        case SM_PAUSE_MP_SETTINGS_SELECT:       UpdatePauseMPSettingsSelect();      break;
#ifdef TARGET_XBOX
        case SM_PAUSE_MP_FRIENDS:               UpdatePauseMPFriends();             break;
#endif
#if defined(TARGET_PS2)
        case SM_PAUSE_MP_MEMCARD_SELECT:        UpdatePauseMPMemcardSelect();       break;
#endif
        case SM_PAUSE_MP_PROFILE_SAVE_SELECT:   UpdatePauseMPMemcardSaveSelect();   break;
        case SM_PAUSE_MP_MEMCARD_RESELECT:      UpdatePauseMPMemcardReselect();     break;

        case SM_PAUSE_ONLINE:                   UpdatePauseOnline();                break;
        case SM_PAUSE_ONLINE_VOTE_MAP:          UpdatePauseOnlineVoteMap();         break;
        case SM_PAUSE_ONLINE_VOTE_KICK:         UpdatePauseOnlineVoteKick();        break;
        case SM_PAUSE_ONLINE_FRIENDS:           UpdatePauseOnlineFriends();         break;
        case SM_PAUSE_ONLINE_PLAYERS:           UpdatePauseOnlinePlayers();         break;
        case SM_PAUSE_ONLINE_FEEDBACK:          UpdatePauseOnlineFeedback();        break;
        case SM_PAUSE_ONLINE_FEEDBACK_FRIEND:   UpdatePauseOnlineFeedbackFriend();  break;
        case SM_PAUSE_ONLINE_SERVER_CONFIG:     UpdatePauseServerConfig();          break;
        case SM_PAUSE_ONLINE_OPTIONS:           UpdatePauseOnlineOptions();         break;
        case SM_PAUSE_ONLINE_CONTROLS:          UpdatePauseOnlineControls();        break;
        case SM_PAUSE_ONLINE_SETTINGS:          UpdatePauseOnlineSettings();        break;
        case SM_PAUSE_ONLINE_HEADSET:           UpdatePauseOnlineHeadset();         break;
#if defined(TARGET_PS2)
        case SM_PAUSE_ONLINE_MEMCARD_SELECT:    UpdatePauseOnlineMemcardSelect();   break;
#endif
        case SM_PAUSE_ONLINE_SAVE_SELECT:       UpdatePauseOnlineSaveSelect();      break;
        case SM_PAUSE_ONLINE_MEMCARD_RESELECT:  UpdatePauseOnlineMemcardReselect(); break;
        case SM_PAUSE_ONLINE_CHANGE_MAP:        UpdatePauseOnlineChangeMap();       break;
        case SM_PAUSE_ONLINE_KICK_PLAYER:       UpdatePauseOnlineKickPlayer();      break;
        case SM_PAUSE_ONLINE_TEAM_CHANGE:       UpdatePauseOnlineTeamChange();      break;
        case SM_PAUSE_ONLINE_RECONFIG_ONE:      UpdatePauseOnlineReconfigOne();     break;
        case SM_PAUSE_ONLINE_RECONFIG_TWO:      UpdatePauseOnlineReconfigTwo();     break;
        case SM_PAUSE_ONLINE_RECONFIG_MAP:      UpdatePauseOnlineReconfigMap();     break;
        case SM_PAUSE_ONLINE_SAVE_SETTINGS:     UpdatePauseOnlineSaveSettings();    break;
        case SM_PAUSE_MP_SCORE:                 UpdatePauseMPScore();               break;
        case SM_END_PAUSE:                      UpdateEndPause();                   break;
        case SM_AUTOSAVE_MENU:                  UpdateAutosaveMenu();               break;
        case SM_AUTOSAVE_PROFILE_RESELECT:      UpdateAutosaveProfileReselect();    break;
#if defined(TARGET_PS2)
        case SM_AUTOSAVE_MEMCARD_SELECT:        UpdateAutosaveMemcardSelect();      break;
#endif
        case SM_END_AUTOSAVE:                   UpdateEndAutosave();                break;
        case SM_EXIT_GAME:                      UpdateExitGame();                   break;
        case SM_POST_GAME:                      UpdatePostGame();                   break;
        case SM_ADVANCE_LEVEL:                  UpdateAdvanceLevel();               break;
        case SM_RELOAD_CHECKPOINT:              UpdateReloadCheckpoint();           break;
        case SM_FINAL_SCORE:                    UpdateFinalScore();                 break;
        case SM_GAME_OVER:                      UpdateGameOver();                   break;
#if CONFIG_IS_DEMO
        case SM_DEMO_EXIT:                      UpdateDemoExit();                   break;
#endif
        case SM_REPORT_ERROR:                   UpdateReportError();                break;
        case SM_GAME_EXIT_PROMPT_FOR_SAVE:      UpdateGameExitPromptForSave();      break;
        case SM_GAME_EXIT_SAVE_SELECT:          UpdateGameExitSaveSelect();         break;
        case SM_GAME_EXIT_MEMCARD_RESELECT:     UpdateGameExitMemcardReselect();    break;
        case SM_GAME_EXIT_SAVE_SETTINGS:        UpdateGameExitSaveSettings();       break;
        case SM_GAME_EXIT_SETTINGS_OVERWRITE:   UpdateGameExitSettingsOverwrite();  break;
        case SM_GAME_EXIT_SETTINGS_SELECT:      UpdateGameExitSettingsSelect();     break;
        case SM_GAME_EXIT_REDIRECT:             UpdateGameExitRedirect();           break;
#if defined(TARGET_PS2)
        case SM_ONLINE_DOWNLOAD:                UpdateOnlineDownload();             break;
#endif
        case SM_ONLINE_LOGIN:                   UpdateOnlineLogin();                break;
        case SM_FOLLOW_BUDDY:                   UpdateFollowBuddy();                break;
        // no update required
        default:
            break;
    }
#if CONFIG_IS_DEMO
    // **** Coverdisk timeout ****
    // If no input is received for 1 minute, then we will
    // bail out. **NOTE** this timer will only be started
    // when the load level is complete.
    s32         IdleTime;
    xbool       ForceQuit = FALSE;
    extern s32  g_DemoInactiveTimeout;

#if defined( TARGET_PS2 )
    if( input_IsPressed( INPUT_PS2_BTN_START ) && input_IsPressed( INPUT_PS2_BTN_SELECT ) )
    {
        ForceQuit = TRUE;
    }
#endif
    IdleTime = (s32)g_DemoIdleTimer.ReadSec();
    if( (IdleTime > g_DemoInactiveTimeout) || ForceQuit )
    {
        if( GameMgr.GameInProgress() )
        {
            SetState( SM_EXIT_GAME );
        }
        else
        {
            SetState( SM_DEMO_EXIT );
        }
    }
#endif
}

//=========================================================================

void state_mgr::SetPaused( xbool bPause, s32 PausingController )
{
    // check if we are autosaving
    if( m_bAutosaveInProgress )
    {
        // no can do!
        return;
    }

    // check if we're doing any other memcard processing
    if( g_UIMemCardMgr.IsActionDone() == FALSE )
    {
        return;
    }

    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if (pPlayer)
    {
        if (pPlayer->IsCinemaRunning())
        {
            // We no longer allow pausing during in game
            // cinematics
            return;
        }
#ifndef X_EDITOR
        if (!GameMgr.IsGameMultiplayer())
        {
            if (pPlayer->IsDead())
            {
                // We no longer allow pausing while you are dead.
                return;
            }
        }
#endif
    }

    if( !GameMgr.IsGameOnline() )
    {
        input_SuppressFeedback(bPause);
    }

    // check we are not exiting to the main menu
    if( m_Exit )
    {
        return;
    }

    // check we are not in the save menu
    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
    {
        if( m_State == SM_SAVE_CAMPAIGN )
        {
            return;
        }
    }

    // ok, see what we want to do
    if( bPause )
    {
        ASSERT( PausingController != -1 );
        // set the initial state
        if( GameMgr.GetGameType() == GAME_CAMPAIGN )
        {
            if( m_State != SM_END_PAUSE )
            {
                // set up the screen bars off screen
                s32 XRes, YRes;
                eng_GetRes( XRes, YRes );
                irect initSize( -75, DIALOG_TOP, 512+75, DIALOG_BOTTOM );
                        
                // Adjust the size according to the resolution.
                s32 midX = XRes>>1;
                s32 dx = midX - 256;

                // scale height of dialog
                s32 Y = initSize.b - initSize.t;
                Y = (s32)( (f32)Y * g_UiMgr->GetScaleY() );
                // position it
                initSize.t = (s32)((f32)initSize.t * g_UiMgr->GetScaleY());
                initSize.t += SAFE_ZONE;
                initSize.b = initSize.t + Y;
                initSize.Translate( dx, 0 );

                g_UiMgr->SetScreenSize(initSize);       
            }
            // force main pause menu to the active controller
            // (returns to all controllers in the EndPause state transition)
            SetState( SM_PAUSE_MAIN_MENU );
        }
        else
        {
            SetState( SM_PAUSE_MP_SCORE );

            // Pause Audio
        //  g_AudioMgr.PauseAll();
        }
        g_UiMgr->EnableUser( g_UiUserID, TRUE );

        // set pause state
        m_bIsPaused = bPause;
        g_UiMgr->SetUserController( g_UiUserID, PausingController );
        SetActiveControllerID( PausingController );
    }
    else
    {
        //
        // Someone MUST be in control of the pause menu. Only they can deactivate it.
        //
        if( (PausingController!=-1) && (PausingController != GetActiveControllerID()) )
        {
            return;
        }

        SetActiveControllerID( PausingController );
        g_UiMgr->SetUserController( g_UiUserID, -1 );
        //#ifdef mbillington
        //#error This is a HACK and needs to be fixed.
        //#endif
        if( (m_State == SM_PLAYING_GAME) && (m_bIsPaused) )
        {
            m_bIsPaused = FALSE;
            g_AudioMgr.ResumeAll(); 
            return;
        }

        if( m_State != SM_PAUSE_MP_SCORE )
        {
            if( m_State != SM_END_PAUSE )
            {
                // clean up the screen gracefully
                SetState( SM_END_PAUSE );
            }
        }
        else
        {
            // Kill the leaderboard and return to the game immediately
            g_UiMgr->EndDialog( g_UiUserID, TRUE );
            m_CurrentDialog = NULL;
            // Disable ui
            g_UiMgr->EnableUser( g_UiUserID, FALSE );

            // Set unpaused
            m_bIsPaused = FALSE;

            // back to playing the game
            SetState( SM_PLAYING_GAME );
        }

        // Resume Audio
        g_AudioMgr.ResumeAll();
    }
}

//=========================================================================

void state_mgr::SetProfileNotSaved( s32 playerID, xbool bNotSaved )
{
    ASSERT( playerID >= 0 );
    ASSERT( playerID < SM_PROFILE_COUNT );
    // flag the profile in memory as not saved
    m_ProfileNotSaved[playerID] = bNotSaved; 
}

//=========================================================================

void state_mgr::ResetProfile( s32 index )
{
    ASSERT( index >= 0 );
    ASSERT( index < SM_PROFILE_COUNT );
    player_profile Pp;
    x_memcpy( m_Profiles+index, &Pp, sizeof( Pp ) );
}

//=========================================================================

void state_mgr::InitPendingProfile( s32 index )
{ 
    // make a copy of the actual profile data selected
    ASSERT(index >= 0 );
    ASSERT(index < SM_PROFILE_COUNT); 
    m_PendingProfile = m_Profiles[index]; 
    m_PendingProfileIndex = index;
    //
    // Initialize the match manager filter to not filter anything.
    //
    g_MatchMgr.SetFilter( GAME_MP, -1, -1 );
}

//=========================================================================

void state_mgr::ActivatePendingProfile( xbool MarkDirty )
{
    // copy the pending profile into the actual profile data
    ASSERT( m_PendingProfileIndex >= 0 );
    ASSERT( m_PendingProfileIndex < SM_PROFILE_COUNT );
    m_Profiles[m_PendingProfileIndex] = m_PendingProfile; 
    m_Profiles[m_PendingProfileIndex].Checksum();

    if( MarkDirty )
    {
        m_Profiles[m_PendingProfileIndex].MarkDirty();
    }

    g_MatchMgr.ShowOnlineStatus( m_PendingProfile.m_bIsVisibleOnline );
    if( m_PendingProfile.m_bIsVisibleOnline )
    {
        if( GameMgr.GameInProgress() )
        {
            g_MatchMgr.SetUserStatus( BUDDY_STATUS_INGAME );
        }
        else
        {
            g_MatchMgr.SetUserStatus( BUDDY_STATUS_ONLINE );
        }
    }
    else
    {
        g_MatchMgr.SetUserStatus( BUDDY_STATUS_OFFLINE );
    }

    // set nickname
    g_MatchMgr.SetNickname( m_PendingProfile.GetProfileName() );

    // set difficulty
    g_Difficulty = m_PendingProfile.GetDifficultyLevel();

    // set autosave flag
    m_bAutosaveProfile = m_PendingProfile.m_bAutosaveOn;

    // finally clear the pending profile index
    m_PendingProfileIndex = -1;
}

//=========================================================================

void state_mgr::ResetProfileListIndex( void )
{
    // reset profile index list
    s32 i;
    for( i=0; i<SM_PROFILE_COUNT; i++ )
    {
        m_ProfileListIndex[i] = -1;
    }
}

//=========================================================================
// whatever this list's original purpose was, it was never implemented.
// The list is used to gather selected profile indexes into an order that
// will work with the game server/client code.
// it's primary use is for split screen mode where the profiles occupy the
// same slots as the initiating controller.
// This needs to be called after all players have selected profiles. 
void state_mgr::SetupProfileListIndex( void )
{
    ResetProfileListIndex();

    s32 i;
    s32 p = 0;
    s32 count = 0;
#if 1   // hack - A profile needs to be assigned for auto modes. this can be removed when that happens.
    if( CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT )
    {
        // until a better fix is found
        for( i = 0; i < SM_PROFILE_COUNT; i++ )
        {
            m_ProfileListIndex[i] = 0;
        }
        return;
    }
#endif
    for( i = 0; i< SM_PROFILE_COUNT; i++ )
    {
        if( m_SelectedProfile[i] != 0 )
        {
            count++;
        }
    }

    ASSERT(count > 0);

    // setup profile index list
    for( i = 0; (i < SM_PROFILE_COUNT) && (i < count); i++ )
    {
        // find a requested controller
        for( ; p < SM_PROFILE_COUNT; p++)
        {
            if( m_SelectedProfile[p] != 0 )
            {
                m_ProfileListIndex[i] = p++;    // increment p here too.
                break;
            }
        }
    }
}

//=========================================================================

void state_mgr::InitPendingSettings( void )
{
     m_PendingSettings = m_ActiveSettings;
}

//=========================================================================

void state_mgr::ActivatePendingSettings( xbool MarkDirty )
{
    m_ActiveSettings = m_PendingSettings;
    m_ActiveSettings.Commit();

    if( MarkDirty )
    {
        m_ActiveSettings.MarkDirty();
    }
}

//=========================================================================

// profile game exit save callback
void state_mgr::OnGameExitSaveProfileCB( void )
{
    // if the save was successful (OR user wants to continue without saving)
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_iCard );
#else
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );
#endif

    if( Condition.SuccessCode )
    {
        global_settings& ActiveSettings = GetActiveSettings();

        // continue without saving?
        if( Condition.bCancelled )
        {
            // flag the profile as not saved
            g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), TRUE ); 
        }

        // update the changes in the profile
        g_StateMgr.ActivatePendingProfile();

        // save successful - finish end game processing
        g_AudioMgr.Play( "Select_Norm" );        

        // check settings
        if( ActiveSettings.HasChanged() )
        {
            SetState( SM_GAME_EXIT_SAVE_SETTINGS );
        }
        else
        {
            SetState( SM_GAME_EXIT_REDIRECT );
        }
    }
    else
    {
        // save failed - goto save select
        g_AudioMgr.Play( "Select_Norm" );
        SetState( SM_GAME_EXIT_SAVE_SELECT );
    }

    // get the profile list
    xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();
    // get the current list from the memcard manager
    g_UIMemCardMgr.GetProfileNames( ProfileNames );
}

//=========================================================================

// settings game exit save callback
void state_mgr::OnGameExitSaveSettingsCB( void )
{
    // check if the save was successful (OR user wants to continue without saving)
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( g_StateMgr.GetSettingsCardSlot() );
#else
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );
#endif
    if( Condition1.SuccessCode )
    {
        // activate the new settings
        g_StateMgr.ActivatePendingSettings();

        // save successful - return to main menu
        g_AudioMgr.Play( "Select_Norm" );

        SetState( SM_GAME_EXIT_REDIRECT );
    }
    else
    {
        // save failed! - goto memcard select dialog
        g_AudioMgr.Play( "Backup" );
        SetState( SM_GAME_EXIT_SETTINGS_SELECT );
    }
}

//=========================================================================

void state_mgr::SetLoreAcquired( s32 LoreID )
{
    s32 VaultIndex;
    s32 LoreIndex;

    // find out which vault the lore is in
    if( g_LoreList.GetVaultByLoreID( LoreID, VaultIndex, LoreIndex ) )
    {
        // get the active profile for player 0
        player_profile& Profile = GetActiveProfile( GetProfileListIndex(0) );
        // set the lore as acquired
        Profile.SetLoreAcquired( VaultIndex, LoreIndex );
    }
    else
    {
        ASSERTS( FALSE, "Error: could not look up vault from lore ID acquired" );
    }
}

//=========================================================================

s32 state_mgr::SetLoreAcquired( s32 MapID, s32 LoreIndex )
{
    s32 VaultIndex;    
    s32 LoreID;

    lore_vault* pVault = g_LoreList.GetVaultByMapID( MapID, VaultIndex );

    // find out which vault the lore is in
    if( pVault )
    {
        LoreID = g_LoreList.GetLoreIDByVault(pVault, LoreIndex);

        // valid lore id?
        if( LoreID != -1 )
        {
            // get the active profile for player 0
            player_profile& Profile = GetActiveProfile( GetProfileListIndex(0) );

            // set the lore as acquired
            Profile.SetLoreAcquired( VaultIndex, LoreIndex );

            // this is the real index, not just the 0-n id per map
            return LoreID;
        }
    }

    return -1;
 
    ASSERTS( FALSE, "Error: could not look up vault from Lore Index acquired" );
}

//=========================================================================
u32 state_mgr::GetLevelLoreAcquired( s32 MapID )
{
    s32 VaultIndex;    
    u32 LoreCount = 0;
    u32 i = 0;

    // in a campaign, the profile will be zero, so the indirection should not change anything.
    ASSERT(GetProfileListIndex(0) == 0);

    player_profile& Profile = GetActiveProfile( GetProfileListIndex(0) );

    // loop through all the items in this vault and see if they've been acquired
    for( i = 0; i < NUM_PER_VAULT; i++ )
    {
        g_LoreList.GetVaultByMapID( MapID, VaultIndex );

        if( Profile.GetLoreAcquired(VaultIndex, i) )
        {
            LoreCount++;
        }
    }

    return LoreCount;
}

//=========================================================================
u32 state_mgr::GetTotalLoreAcquired( void )
{
    player_profile& Profile = GetActiveProfile( GetProfileListIndex(0) );

    return Profile.GetTotalLoreAcquired();
}

//=========================================================================

void state_mgr::EnterESRBNotice( void )
{
    // Create the ESRB screen
    irect mainarea(16, 16, 512-16, 448-16);
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "ESRB", mainarea, NULL, ui_win::WF_VISIBLE);
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
}

//=========================================================================

void state_mgr::UpdateESRBNotice( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( ( DialogState == DIALOG_STATE_SELECT ) || ( CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT || CONFIG_IS_AUTOCAMPAIGN || CONFIG_IS_AUTOSPLITSCREEN) )
        {
            SetState( SM_INEVITABLE_INTRO );
        }
    }
}

//=========================================================================

void state_mgr::ExitESRBNotice( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
}

//=========================================================================

void state_mgr::EnterInevitableIntro( void )
{
}

//=========================================================================

xstring SelectBestClip( const char* pName )
{
#ifdef TARGET_XBOX
    if( x_GetTerritory() == XL_TERRITORY_AMERICA )
    {
        return (const char*)xfs( "%s_640x480_%d",pName,g_PhysFPS/2 );
    }
    else
    {
        if( x_stristr( pName,"PromoUnreal2" ))
            return "PromoUnreal2_640x480_25";

        return (const char*)xfs( "%s_640x480_%d",pName,g_PhysFPS/2 );
    }
#endif

#ifdef TARGET_PC
        return (const char*)xfs( "%s_640x480_%d",pName,30 );
#endif

#ifdef TARGET_PS2
    xbool bIsPal;
    eng_GetPALMode( bIsPal );

    if( bIsPal )
    {
        return (const char*)xfs( "PAL_%s", pName );
    }
    else
    {
        return (const char*)xfs( "NTSC_%s", pName );
    }
#endif

    return pName;
}

//=========================================================================

void state_mgr::UpdateInevitableIntro( void )
{

#if CONFIG_IS_DEMO
  #if defined(USE_MOVIES)
    // play intro movie
    if( !PlaySimpleMovie(SelectBestClip("trailer")) )
    {
        DummyScreen( "Trailer Movie Here", TRUE, 5 );
    }
  #endif

    SetState( SM_PRESS_START_SCREEN );
#else
#ifdef USE_MOVIES
    // play midway movie
    if( !PlaySimpleMovie( SelectBestClip("Midway" )) )
    {
        DummyScreen( "Midway Movie Here", TRUE, 5 );
    }
#endif
    // change the state
#ifdef TARGET_PS2
    // PS2 needs to check that controllers are plugged in.
    SetState( SM_CONTROLLER_CHECK );
#else
    SetState( SM_MEMCARD_BOOT_CHECK );
//  SetState( SM_PRESS_START_SCREEN );
#endif

#endif
}

//=========================================================================

void state_mgr::ExitInevitableIntro( void )
{
}

#if defined(TARGET_PS2)
//=========================================================================
void state_mgr::EnterControllerCheck( void )
{
    //  Check for pulled controllers.
    CheckControllers();
}

//=========================================================================
void state_mgr::UpdateControllerCheck( void )
{
    if( !InSystemError() )
    {
        // change the state
        SetState( SM_AUTOSAVE_DIALOG );
    }
}
//=========================================================================
void state_mgr::ExitControllerCheck( void )
{
    
}

//=========================================================================
void state_mgr::EnterAutoSaveDialog( void )
{
    m_AutoSave.SetName      ( PRELOAD_PS2_FILE( "UI_PS2_autosave.xbmp"                  ) );
}

//=========================================================================

void state_mgr::UpdateAutoSaveDialog( void )
{
    xbool  Done = FALSE;
    xcolor IconColor(  63,  63,  63, 255 );
    xcolor White    ( 255, 255, 255, 255 );

    while( !Done )
    {
        eng_Begin( "AutoSaveDialog" );

        g_InputMgr.Update( 1.0f / 60.0f );

        if( input_WasPressed( INPUT_PS2_BTN_CROSS, 0 ) ||
            input_WasPressed( INPUT_PS2_BTN_CROSS, 1 ) )
        {
            Done = TRUE;
        }

        draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );

        irect rect(0+32,0,512-32,340);
        xwstring noticeString = g_StringTableMgr("ui","IDS_AUTOSAVE_NOTICE");
        g_UiMgr->RenderText_Wrap( 0, rect, ui_font::h_center|ui_font::v_center, White, noticeString );

        draw_End();

        rect.Set(0,0,512,448-20);
        xwstring navString = g_StringTableMgr("ui","IDS_NAV_NETWORK_CONTINUE");
        g_UiMgr->RenderText( g_UiMgr->FindFont("small"), rect, ui_font::h_center|ui_font::v_bottom, White, navString );

        xbitmap* pAutoSaveBitmap = NULL;
        pAutoSaveBitmap = m_AutoSave.GetPointer();

        if( pAutoSaveBitmap )
        {       
            draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );
            draw_SetTexture(*pAutoSaveBitmap);
            draw_DisableBilinear();                
            draw_Sprite( vector3( 256-(pAutoSaveBitmap->GetWidth()/2), 300, 0 ), 
                         vector2( (f32)pAutoSaveBitmap->GetWidth(), 
                                  (f32)pAutoSaveBitmap->GetHeight() ), 
                         IconColor );
            draw_End();
        }

        eng_End();

        eng_PageFlip();
    }

    // Done
    SetState( SM_MEMCARD_BOOT_CHECK );
}

//=========================================================================
void state_mgr::ExitAutoSaveDialog( void )
{

}

#endif // ndef TARGET_XBOX || TARGET_PC

//=========================================================================

void state_mgr::EnterMemcardBootCheck( void )
{
    // check the status of the memory cards
    g_UIMemCardMgr.BootCheck();
}

//=========================================================================

void state_mgr::UpdateMemcardBootCheck( void )
{

    // did the check finish?
    if( g_UIMemCardMgr.IsActionDone() )
    {
        // change the state
        SetState( SM_PRESS_START_SCREEN );
    }
}

//=========================================================================

void state_mgr::ExitMemcardBootCheck( void )
{
}

//=========================================================================

void state_mgr::EnterPressStart( void )
{
    // Create the press start screen
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    g_UiMgr->EnableUser( g_UiUserID, TRUE );
    irect mainarea(16, 16, 512-16, 448-16);
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "press start", mainarea, NULL, ui_win::WF_VISIBLE);//|ui_win::WF_BORDER );
//    g_UiMgr->SetUserBackground( g_UiUserID, "titlescreen" );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
#if CONFIG_IS_DEMO
    extern s32 g_DemoInactiveTimeout;

    m_TimeoutTime = g_DemoInactiveTimeout;
#endif
}

//=========================================================================

void state_mgr::UpdatePressStart( void )
{
#if CONFIG_IS_DEMO
    // check for timeout
    if( m_Timeout < 0 )
    {      
        // This is really missnamed, this displays the post game splash
        SetState( SM_DEMO_EXIT );
    }
#endif

    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( ( DialogState == DIALOG_STATE_SELECT ) || ( CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT || CONFIG_IS_AUTOCAMPAIGN || CONFIG_IS_AUTOSPLITSCREEN) )
        {
            s32 XRes, YRes;
            eng_GetRes( XRes, YRes );
            irect initSize( -75, DIALOG_TOP, 512+75, DIALOG_BOTTOM );
                    
            // Adjust the size according to the resolution.
            s32 midX = XRes>>1;
            s32 dx = midX - 256;

            // scale height of dialog
            s32 Y = initSize.b - initSize.t;
            Y = (s32)( (f32)Y * g_UiMgr->GetScaleY() );
            // position it
            initSize.t = (s32)((f32)initSize.t * g_UiMgr->GetScaleY());
            initSize.t += SAFE_ZONE;
            initSize.b = initSize.t + Y;
            initSize.Translate( dx, 0 );

            g_UiMgr->SetScreenOn( FALSE );
            g_UiMgr->SetScreenSize( initSize );
            g_UiMgr->InitGlowBar();
            g_UiMgr->InitRefreshBar();

#if CONFIG_IS_DEMO
            // This is where we load the level in demo mode
            g_PendingConfig.SetLevelID          ( 1100 ); // Lies of the Past
            g_PendingConfig.SetPlayerCount      ( 1 );
            g_PendingConfig.SetMaxPlayerCount   ( 1 );
            g_PendingConfig.SetGameTypeID       ( GAME_CAMPAIGN );
            g_NetworkMgr.BecomeServer();

            extern s32 g_LastControllerID;
            ASSERT( g_LastControllerID != -1 );
            SetActiveControllerID( g_LastControllerID );
            g_UiMgr->SetUserController( g_UiUserID, g_LastControllerID );
            SetState( SM_LOAD_GAME );
#else
            if( m_bFollowBuddy )
            {
                SetState( SM_FOLLOW_BUDDY );
            }
            else
            {
                SetState( SM_MAIN_MENU );
            }
#endif
        }
    }
}

//=========================================================================

void state_mgr::ExitPressStart( void )
{
#if defined(TARGET_XBOX)
    if( m_bSilentSigninStarted==FALSE )
    {
        m_bSilentSigninStarted = TRUE;
        g_MatchMgr.Init();
    }
#endif

}

#if CONFIG_IS_DEMO
//=========================================================================
void state_mgr::EnterDemoExit( void )
{
    g_UiMgr->EnableUser( g_UiUserID, TRUE );
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    g_UiMgr->LoadBackground ( "background1", "PostGameSplash.xbmp" );
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
    m_TimeoutTime = DEMO_ENDGAME_TIMEOUT;
}

//=========================================================================
void state_mgr::UpdateDemoExit( void )
{
    if( m_Timeout <= 0.0f )
    {
        Reboot( REBOOT_QUIT );
    }

    if( m_Timeout < (DEMO_ENDGAME_TIMEOUT-4.0f) )
    {
#if defined(TARGET_PS2)
        if( input_WasPressed( INPUT_PS2_BTN_START ) )
        {
            Reboot( REBOOT_QUIT );
        }
#endif // TARGET_PS2

#if defined(TARGET_XBOX)
        if( input_WasPressed( INPUT_XBOX_BTN_START ) )
        {
            Reboot( REBOOT_QUIT );
        }
#endif // TARGET_XBOX
    }

}
//=========================================================================
void state_mgr::ExitDemoExit( void )
{
}
#endif
//=========================================================================

void state_mgr::EnterMainMenu( void )
{
    s32 i;

    // Make sure all pads are inactive for the game. They will be re-activated when
    // the game is actually started.
    for( i=0; i<MAX_LOCAL_PLAYERS; i++ )
    {
        g_IngamePad[i].SetControllerID(-1);
    }

#ifdef USE_MOVIES
    // initialize movie player
    if( !m_bPlayMovie )
    {
        Movie.Init();
        //Movie.SetLanguage(XL_LANG_ENGLISH);
        m_bPlayMovie = Movie.Open( SelectBestClip("MenuBackground"),TRUE,TRUE );
    }
#else
    // Load the background
    g_UiMgr->LoadBackground ( "background1", "A51Background.xbmp" );
#endif

    // Create main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );

    irect mainarea(136, DIALOG_TOP, 376, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "main menu", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
#else
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateMainMenu( void )
{
#if defined(TARGET_XBOX)
    if( CheckForDisconnect() )
    {
        return;
    }
#endif
    // check for menu automation
    if( CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT )
    {
        // activate controller 0
        SetControllerRequested( 0, TRUE );

        if( g_NetworkMgr.IsOnline() )
        {
            SetState( SM_ONLINE_MAIN_MENU );
        }
        else
        {
            // Can't have movie playing, or anything else, while
            // trying to go online.
#if defined(USE_MOVIES)
            if( m_bPlayMovie )
            {
                Movie.Close();
                Movie.Kill();
                m_bPlayMovie = FALSE;
            }
#endif
            SetState( SM_ONLINE_CONNECT );
            
        }
    }
    else if ( CONFIG_IS_AUTOCAMPAIGN )
    {
        // activate controller 0
        SetControllerRequested( 0, TRUE );
 
        g_PendingConfig.SetPlayerCount( 1 );
        g_PendingConfig.SetGameTypeID( GAME_CAMPAIGN );
        SetState( SM_PROFILE_SELECT );
    }
    else if ( CONFIG_IS_AUTOSPLITSCREEN )
    {
        // activate controller 0
        SetControllerRequested( 0, TRUE );
 
        SetState( SM_MULTI_PLAYER_MENU );
    }
    else
    {
        // Get the current dialog state
        if( m_CurrentDialog != NULL )
        {
            u32 DialogState = m_CurrentDialog->GetState();

            if( DialogState == DIALOG_STATE_SELECT )
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_MAIN_MENU_CAMPAIGN:
                    {
                        g_PendingConfig.SetGameTypeID( GAME_CAMPAIGN );
                        g_PendingConfig.SetPlayerCount( 1 );

                        if( GetSelectedProfile(0) != 0 )
                        {
                            SetState( SM_CAMPAIGN_MENU );
                        }
                        else
                        {
                            SetState( SM_PROFILE_SELECT );
                        }
                    }
                    break;

                    case IDC_MAIN_MENU_MULTI:
                    {
                        // clear the profiles and controller requests
                        // this will prevent issues associated with default profiles.
                        for(s32 p=0; p < SM_MAX_PLAYERS; p++)
                        {
                            ClearSelectedProfile( p );
                            SetControllerRequested(p, FALSE);
                        }

                        SetState( SM_MULTI_PLAYER_MENU );
                    }
                    break;

                    case IDC_MAIN_MENU_ONLINE:
                    {
#ifdef TARGET_PS2
                        SetState( SM_ONLINE_EULA );
#else
                        SetState( SM_ONLINE_CONNECT );
#endif
                    }
                    break;

                    case IDC_MAIN_MENU_SETTINGS:
                    {
                        // init pending settings
                        InitPendingSettings();
                        SetState( SM_SETTINGS_MENU );
                    }
                    break;

                    case IDC_MAIN_MENU_PROFILES:
                    {
                        SetState( SM_MANAGE_PROFILES );
                    }
                    break;

                    case IDC_MAIN_MENU_CREDITS:
                    {
                        SetState( SM_CREDITS_SCREEN );
                    }
                    break;
                }
            }
        }
    }
}

//=========================================================================

void state_mgr::ExitMainMenu( void )
{
}

//=========================================================================

void state_mgr::EnterSettingsMenu( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(16, DIALOG_TOP, 496, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "av settings", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateSettingsMenu( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_SELECT:
            {
                SetState( SM_SETTINGS_HEADSET );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                SetState( SM_SETTINGS_MEMCARD_SELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitSettingsMenu( void )
{
}

//=========================================================================

void state_mgr::EnterSettingsHeadset( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "headset", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateSettingsHeadset( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_SETTINGS_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitSettingsHeadset( void )
{
}

//=========================================================================

void state_mgr::EnterSettingsMemcardSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->Configure( SM_CARDMODE_SETTINGS );
#endif
}

//=========================================================================

void state_mgr::UpdateSettingsMemcardSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save settings was successful
            SetState( SM_MAIN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // save failed or was aborted
            SetState( SM_MAIN_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitSettingsMemcardSelect( void )
{
}

//=========================================================================

void state_mgr::EnterManageProfiles( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_MANAGE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateManageProfiles( void )
{
#if defined(TARGET_XBOX)
    if( CheckForDisconnect() )
    {
        return;
    }
#endif
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected a profile
                SetState( SM_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                // do nothing
                SetState( SM_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // create a profile
                m_bCreatingProfile = TRUE;
                SetState( SM_MANAGE_PROFILE_OPTIONS );
            }
            break;

            case DIALOG_STATE_EDIT:
            {
                // edit a profile
                InitPendingProfile(0);
                m_bCreatingProfile = FALSE;
                SetState( SM_MANAGE_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitManageProfiles( void )
{
}

//=========================================================================
void state_mgr::EnterManageProfileOptions( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(s_ProfileOptLeft, DIALOG_TOP, s_ProfileOptRight, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // configure the dialog for creating a profile-
    ((dlg_profile_options*)m_CurrentDialog)->Configure( PROFILE_OPTIONS_NORMAL, m_bCreatingProfile );
}

//=========================================================================

void state_mgr::UpdateManageProfileOptions( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PROFILE_OPTIONS_CONTROLS:
                    {
                        SetState( SM_MANAGE_PROFILE_CONTROLS );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_AVATAR:
                    {
                        SetState( SM_MANAGE_PROFILE_AVATAR );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_ACCEPT:
                    {
#if defined(TARGET_PC) || defined(TARGET_XBOX)
                        SetState( SM_MAIN_MENU );
#else
                        // need to save new profile
                        SetState( SM_MANAGE_MEMCARD_SELECT );
#endif
                    }
                    break;
                }
            }
            break;

            case DIALOG_STATE_BACK:
            {
                // abort create/edit profile
                SetState( SM_MANAGE_PROFILES );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // save success - current profile
                SetState( SM_MANAGE_PROFILES );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                // save failed - select a profile to save to
                SetState( SM_MANAGE_PROFILE_SAVE_SELECT ); 
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitManageProfileOptions( void )
{
}

//=========================================================================

void state_mgr::EnterManageProfileControls( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile controls", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateManageProfileControls( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
        case DIALOG_STATE_SELECT:
        case DIALOG_STATE_BACK:
            {
                SetState( SM_MANAGE_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitManageProfileControls( void )
{
}

//=========================================================================

void state_mgr::EnterManageProfileAvatar( void )
{
    // Create avatar dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(121, DIALOG_TOP, 391, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "avatar select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateManageProfileAvatar( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
        case DIALOG_STATE_BACK:
            {
                SetState( SM_MANAGE_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitManageProfileAvatar( void )
{
}

//=========================================================================
#if defined(TARGET_PS2)

void state_mgr::EnterManageMemcardSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateManageMemcardSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the main menu
            SetState( SM_MAIN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the manage profiles menu
            SetState( SM_MANAGE_PROFILES );
        }
    }
}

//=========================================================================

void state_mgr::ExitManageMemcardSelect( void )
{
}

#endif
//=========================================================================

void state_mgr::EnterManageProfileSaveSelect( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateManageProfileSaveSelect( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected profile was overwritten
                SetState( SM_MANAGE_PROFILES );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // continue without saving
                SetState( SM_MANAGE_PROFILES );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // Select a card to save the profile to
                SetState( SM_MANAGE_PROFILE_MEMCARD_RESELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitManageProfileSaveSelect( void )
{
}

//=========================================================================

void state_mgr::EnterManageMemcardReselect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateManageMemcardReselect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            SetState( SM_MANAGE_PROFILES );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_MANAGE_PROFILE_SAVE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitManageMemcardReselect( void )
{
}

//=========================================================================

void state_mgr::EnterCampaignMenu( void )
{
    // Create main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(116, DIALOG_TOP, 396, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "campaign menu", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
#else
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateCampaignMenu( void )
{
#ifndef CONFIG_RETAIL
    // check for auto campaign
    if( CONFIG_IS_AUTOCAMPAIGN )
    {
        // use profile defaults
        SetState( SM_LEVEL_SELECT );        
    }
#endif
#if defined(TARGET_XBOX)
    if( CheckForDisconnect() )
    {
        return;
    }
#endif

    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            s32 Control = m_CurrentDialog->GetControl();

            switch( Control )
            {
                case IDC_CAMPAIGN_MENU_NEW_CAMPAIGN:
                {
                    // start new campaign
                    player_profile& ActiveProfile = GetActiveProfile(0);
                    // flag difficulty as unchanged
                    ActiveProfile.m_bDifficultyChanged = FALSE;

                    // set initial level
                    const map_entry* pMapEntry = g_MapList.Find( -1, GAME_CAMPAIGN);
                    ASSERTS( pMapEntry, "Cannot find a campaign map in the maplist" );
                    g_PendingConfig.SetLevelID( pMapEntry->GetMapID() );
                    g_PendingConfig.SetMaxPlayerCount( 1 );
                    g_StateMgr.SetLevelIndex( 0 );

#ifndef TARGET_XBOX
					//klkl: disable this check for Xbox because of silent login
                    ASSERT( g_NetworkMgr.IsOnline()==FALSE );
#endif
					g_NetworkMgr.BecomeServer();

                    // set campaign game type
                    m_CampaignType = SM_NEW_CAMPAIGN_GAME;

                    // set state to load it
                    SetState( SM_START_GAME );
                }
                break;

                case IDC_CAMPAIGN_MENU_RESUME_CAMPAIGN:
                {
                    // set campaign game type
                    m_CampaignType = SM_LOAD_CAMPAIGN_GAME;
                    // go to the campaign load screen
//                    SetState( SM_LOAD_CAMPAIGN );
                    SetState( SM_RESUME_CAMPAIGN );
                }
                break;

                case IDC_CAMPAIGN_MENU_EDIT_PROFILE:
                {
                    // edit the currently active profile
                    SetState( SM_CAMPAIGN_PROFILE_OPTIONS );
                }
                break;

                case IDC_CAMPAIGN_MENU_LORE:
                {
                    // go to the lore menu
                    SetState( SM_LORE_MAIN_MENU );
                }
                break;

                case IDC_CAMPAIGN_MENU_SECRETS:
                {
                    // go to the secrets menu
                    SetState( SM_SECRETS_MENU );
                }
                break;

                case IDC_CAMPAIGN_MENU_EXTRAS:
                {
                    // go to the extras menu
                    SetState( SM_EXTRAS_MENU );
                }
                break;
#ifndef CONFIG_RETAIL
                case IDC_CAMPAIGN_MENU_LEVEL_SELECT:
                {
                    // set campaign game type
                    m_CampaignType = SM_DEBUG_SELECT_GAME;
                    // debug only - select from the available levels
                    SetState( SM_LEVEL_SELECT );
                }
                break;
#endif
            }
        }

        if( DialogState == DIALOG_STATE_BACK )
        {
            if( GetSelectedProfile(0) != 0 )
            {
                SetState( SM_MAIN_MENU );
            }
            else
            {
                SetState( SM_PROFILE_SELECT );
            }
        }
    }
}

//=========================================================================

void state_mgr::ExitCampaignMenu( void )
{
}

//=========================================================================

void state_mgr::EnterLoadCampaign( void )
{
    //  Create load game screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 96, DIALOG_TOP, 416, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "load game", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateLoadCampaign( void )
{
#if defined(TARGET_XBOX)
    if( CheckForDisconnect() )
    {
        return;
    }
#endif
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // exit menu and restore save game data
            g_PendingConfig.SetMaxPlayerCount( 1 );

            g_NetworkMgr.BecomeServer();
            SetState( SM_START_SAVE_GAME );
        }
        else if( DialogState == DIALOG_STATE_BACK )
        {
            SetState( SM_CAMPAIGN_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitLoadCampaign( void )
{
}

//=========================================================================

void state_mgr::EnterSaveCampaign( void )
{
    //  Create save game screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 96, DIALOG_TOP, 416, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "save game", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );

    // scale height of dialog
    s32 Y = mainarea.b - mainarea.t;
    Y = (s32)( (f32)Y * g_UiMgr->GetScaleY() );
    // position it
    mainarea.t = (s32)((f32)mainarea.t * g_UiMgr->GetScaleY());
    mainarea.t += SAFE_ZONE;
    mainarea.b = mainarea.t + Y;

    g_UiMgr->SetScreenSize(mainarea);       

    // enabled user input
    g_UiMgr->EnableUser( g_UiUserID, TRUE );

    // pause the game
    m_bIsPaused = TRUE;
}

//=========================================================================

void state_mgr::UpdateSaveCampaign( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // saving complete
            SetState( SM_PLAYING_GAME );
        }
        else if( DialogState == DIALOG_STATE_BACK )
        {
            //Cancel saving and return to game
            SetState( SM_PLAYING_GAME );
        }
    }
}

//=========================================================================

void state_mgr::ExitSaveCampaign( void )
{
    // Kill the save menu and return to the game
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    m_CurrentDialog = NULL;

    // Disable ui
    g_UiMgr->EnableUser( g_UiUserID, FALSE );

    // unpause the game
    m_bIsPaused = FALSE;
}

//=========================================================================

void state_mgr::EnterResumeCampaign( void )
{
    //  Create load game screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 96, DIALOG_TOP, 416, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "resume game", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateResumeCampaign( void )
{
#if defined(TARGET_XBOX)
    if( CheckForDisconnect() )
    {
        return;
    }
#endif

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // The checkpoint is actually restored within the dialog code. But this is where
            // we need to tell the statemgr that we're going to restore a level from a save
            m_bStartSaveGame = TRUE;
            g_PendingConfig.SetMaxPlayerCount( 1 );

            g_NetworkMgr.BecomeServer();

            SetState( SM_START_SAVE_GAME );
        }
        else if( DialogState == DIALOG_STATE_BACK )
        {
            SetState( SM_CAMPAIGN_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitResumeCampaign( void )
{
}

//=========================================================================

void state_mgr::EnterLoreMainMenu ( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "lore main", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateLoreMainMenu( void )
{
#if defined(TARGET_XBOX)
    if( CheckForDisconnect() )
    {
        return;
    }
#endif

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_CAMPAIGN_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitLoreMainMenu  ( void )
{
}

//=========================================================================

void state_mgr::EnterSecretsMenu( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "secrets", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateSecretsMenu( void )
{
#if defined(TARGET_XBOX)
    if( CheckForDisconnect() )
    {
        return;
    }
#endif

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_CAMPAIGN_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitSecretsMenu( void )
{
}

//=========================================================================

void state_mgr::EnterExtrasMenu( void )
{
    //  Create extras menu dialog 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 96, DIALOG_TOP, 416, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "extras", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateExtrasMenu( void )
{
#if defined(TARGET_XBOX)
    if( CheckForDisconnect() )
    {
        return;
    }
#endif
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_BACK )
        {
            SetState( SM_CAMPAIGN_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitExtrasMenu( void )
{
}

//=========================================================================

void state_mgr::EnterCreditsScreen( void )
{
    // Create the press start screen
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(16, DIALOG_TOP, 512-16, DIALOG_BOTTOM);
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "credits screen", mainarea, NULL, ui_win::WF_VISIBLE);//|ui_win::WF_BORDER );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
}

//=========================================================================

void state_mgr::UpdateCreditsScreen( void )
{
#if defined(TARGET_XBOX)
    if( CheckForDisconnect() )
    {
        return;
    }
#endif
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_BACK ) 
        {
            SetState( SM_MAIN_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitCreditsScreen( void )
{
}

//=========================================================================

void state_mgr::EnterCampaignProfileOptions( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(s_ProfileOptLeft, DIALOG_TOP, s_ProfileOptRight, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateCampaignProfileOptions( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PROFILE_OPTIONS_CONTROLS:
                    {
                        SetState( SM_CAMPAIGN_PROFILE_CONTROLS );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_AVATAR:
                    {
                        SetState( SM_CAMPAIGN_PROFILE_AVATAR );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_ACCEPT:
                    {
                    #if defined(TARGET_PC) || defined(TARGET_XBOX)
                        SetState( SM_CAMPAIGN_MENU );
                    #else
                        // save to memcard and return to menu
                        SetState( SM_CAMPAIGN_MEMCARD_SELECT );
                    #endif
                    }
                    break;
                }
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_CAMPAIGN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // save success - current profile
                SetState( SM_CAMPAIGN_MENU );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                // save failed - select a profile to save to
                SetState( SM_CAMPAIGN_PROFILE_SAVE_SELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitCampaignProfileOptions( void )
{
}

//=========================================================================

void state_mgr::EnterCampaignProfileControls( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile controls", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateCampaignProfileControls( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_CAMPAIGN_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitCampaignProfileControls( void )
{
}

//=========================================================================

void state_mgr::EnterCampaignProfileAvatar( void )
{
    // Create avatar dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(121, DIALOG_TOP, 391, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "avatar select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateCampaignProfileAvatar( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_CAMPAIGN_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitCampaignProfileAvatar( void )
{
}

//=========================================================================

void state_mgr::EnterCampaignProfileSaveSelect( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateCampaignProfileSaveSelect( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected profile was overwritten
                SetState( SM_CAMPAIGN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // continue without saving
                SetState( SM_CAMPAIGN_MENU );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // Select a card to save the profile to
                SetState( SM_CAMPAIGN_MEMCARD_RESELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitCampaignProfileSaveSelect( void )
{
}

//=========================================================================

void state_mgr::EnterCampaignMemcardReselect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateCampaignMemcardReselect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the campaign menu
            SetState( SM_CAMPAIGN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_CAMPAIGN_PROFILE_SAVE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitCampaignMemcardReselect( void )
{
}

//=========================================================================

void state_mgr::EnterProfileSelect( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateProfileSelect( void )
{
    // check for auto campaign
    if( CONFIG_IS_AUTOCAMPAIGN )
    {
        // use profile defaults
        SetState( SM_CAMPAIGN_MENU );        
    }

    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                SetState( SM_CAMPAIGN_MENU );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitProfileSelect( void )
{
}

//=========================================================================

void state_mgr::EnterProfileOptions( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(s_ProfileOptLeft, DIALOG_TOP, s_ProfileOptRight, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // configure the dialog for creating a profile-
    ((dlg_profile_options*)m_CurrentDialog)->Configure( PROFILE_OPTIONS_NORMAL, TRUE );
}

//=========================================================================

void state_mgr::UpdateProfileOptions( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PROFILE_OPTIONS_CONTROLS:
                    {
                        SetState( SM_PROFILE_CONTROLS );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_AVATAR:
                    {
                        SetState( SM_PROFILE_AVATAR );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_ACCEPT:
                    {
                    #if defined(TARGET_PC) || defined(TARGET_XBOX)
                        SetState( SM_CAMPAIGN_MENU );
                    #else
                        // need to save new profile
                        SetState( SM_MEMCARD_SELECT );
                    #endif
                    }
                    break;
                }
            }
            break;

            case DIALOG_STATE_BACK:
            case DIALOG_STATE_MEMCARD_ERROR:
            {
                // abort create/edit profile
                SetState( SM_PROFILE_SELECT );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // save success - current profile
                SetState( SM_PROFILE_SELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitProfileOptions( void )
{
}

//=========================================================================

void state_mgr::EnterProfileControls( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile controls", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================
    
void state_mgr::UpdateProfileControls( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitProfileControls( void )
{
}

//=========================================================================

void state_mgr::EnterProfileAvatar( void )
{
    // Create avatar dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(121, DIALOG_TOP, 391, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "avatar select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateProfileAvatar( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitProfileAvatar( void )
{
}

//=========================================================================

void state_mgr::EnterMultiPlayer( void )
{
    // Create multi player menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
#ifdef TARGET_XBOX
    irect mainarea(-12, DIALOG_TOP, 524, DIALOG_BOTTOM );
#else
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
#endif
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "multiplayer menu", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateMultiPlayer( void )
{
    if ( CONFIG_IS_AUTOSPLITSCREEN )
    {
        SetState( SM_MULTI_PLAYER_OPTIONS );
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_EDIT:
            {
                // profile options
                m_bCreatingProfile = FALSE;
                SetState( SM_MULTI_PLAYER_EDIT );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // profile options
                m_bCreatingProfile = TRUE;
                SetState( SM_MULTI_PLAYER_EDIT );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // game options
                InitPendingSettings();
                SetState( SM_MULTI_PLAYER_OPTIONS );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                // back to main menu
                SetState( SM_MAIN_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitMultiPlayer( void )
{
}

//=========================================================================

void state_mgr::EnterMultiPlayerOptions( void )
{
    // Create online host menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "multi options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateMultiPlayerOptions( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                SetState( SM_MP_LEVEL_SELECT ); 
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_MULTI_PLAYER_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitMultiPlayerOptions( void )
{
}

//=========================================================================

void state_mgr::EnterMultiPlayerEdit( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(s_ProfileOptLeft, DIALOG_TOP, s_ProfileOptRight, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif

    // configure the dialog for MP options-
    ((dlg_profile_options*)m_CurrentDialog)->Configure( PROFILE_OPTIONS_NORMAL, m_bCreatingProfile );
}

//=========================================================================

void state_mgr::UpdateMultiPlayerEdit( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PROFILE_OPTIONS_CONTROLS:
                    {
                        SetState( SM_PROFILE_CONTROLS_MP );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_AVATAR:
                    {
                        SetState( SM_PROFILE_AVATAR_MP );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_ACCEPT:
                    {
                    #if defined(TARGET_PC) || defined(TARGET_XBOX)
                        SetState( SM_MULTI_PLAYER_MENU );
                    #else
                        SetState( SM_MEMCARD_SELECT_MP ); 
                    #endif
                    }
                    break;
                }
            }
            break;

            case DIALOG_STATE_BACK:
            {
                // abort create/edit profile
                SetState( SM_MULTI_PLAYER_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // save success - current profile
                SetState( SM_MULTI_PLAYER_MENU );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                // save failed - select a profile to save to
                SetState( SM_PROFILE_SAVE_SELECT_MP );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitMultiPlayerEdit( void )
{
}

//=========================================================================

void state_mgr::EnterMPLevelSelect( void )
{
    //  Create level select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "online level select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_online_level_select*)m_CurrentDialog)->Configure( MAP_SELECT_MP );
}

//=========================================================================

void state_mgr::UpdateMPLevelSelect( void )
{

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
        case DIALOG_STATE_BACK:
            {
                SetState( SM_MULTI_PLAYER_OPTIONS );
            }
            break;

        case DIALOG_STATE_SELECT:
            {
                // save to memcard and return to menu
                SetState( SM_MP_SAVE_SETTINGS );
            }
            break;

        case DIALOG_STATE_ACTIVATE:
            {
                ASSERT( g_NetworkMgr.IsOnline()==FALSE );

                g_NetworkMgr.BecomeServer();
                SetState( SM_START_GAME );
            }
            break;

        case DIALOG_STATE_MEMCARD_ERROR:
            {
                // save failed - select a profile to save to
                SetState( SM_MP_SAVE_SETTINGS );
            }
        }
    }
}

//=========================================================================

void state_mgr::ExitMPLevelSelect( void )
{
}

//=========================================================================

void state_mgr::EnterMPSaveSettings( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->Configure( SM_CARDMODE_SETTINGS );
#endif
}

//=========================================================================

void state_mgr::UpdateMPSaveSettings( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save settings was successful
            ASSERT( g_NetworkMgr.IsOnline()==FALSE );

            g_NetworkMgr.BecomeServer();
            SetState( SM_START_GAME );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // save settings failed or was aborted
            ASSERT( g_NetworkMgr.IsOnline()==FALSE );

            g_NetworkMgr.BecomeServer();
            SetState( SM_START_GAME );
        }
    }
}

//=========================================================================

void state_mgr::ExitMPSaveSettings( void )
{
}

//=========================================================================

void state_mgr::EnterProfileControlsMP( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile controls", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateProfileControlsMP( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            case DIALOG_STATE_BACK:
            {
                SetState( SM_MULTI_PLAYER_EDIT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitProfileControlsMP( void )
{
}

//=========================================================================

void state_mgr::EnterProfileAvatarMP( void )
{
    // Create avatar dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(121, DIALOG_TOP, 391, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "avatar select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateProfileAvatarMP( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_MULTI_PLAYER_EDIT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitProfileAvatarMP( void )
{
}

//=========================================================================

void state_mgr::EnterProfileSaveSelectMP( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateProfileSaveSelectMP( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected profile was overwritten
                SetState( SM_MULTI_PLAYER_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // continue without saving
                SetState( SM_MULTI_PLAYER_MENU );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // Select a card to save the profile to
                SetState( SM_MEMCARD_RESELECT_MP );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitProfileSaveSelectMP( void )
{
}

//=========================================================================

void state_mgr::EnterMemcardReselectMP( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateMemcardReselectMP( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the multiplayer main menu
            SetState( SM_MULTI_PLAYER_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_PROFILE_SAVE_SELECT_MP );
        }
    }
}

//=========================================================================

void state_mgr::ExitMemcardReselectMP( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineSilentLogin( void )
{
}

//=========================================================================
#ifdef TARGET_PS2
void state_mgr::EnterOnlineEULA( void )
{
    // Create online DNAS End User License Agreement dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(50, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "dnas eula", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}
#endif
//=========================================================================
#ifdef TARGET_PS2
void state_mgr::UpdateOnlineEULA( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                SetState( SM_ONLINE_CONNECT );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_MAIN_MENU );
            }
            break;
        }
    }
}
#endif
//=========================================================================
#ifdef TARGET_PS2
void state_mgr::ExitOnlineEULA( void )
{
}
#endif
//=========================================================================

void state_mgr::EnterOnlineConnect( void )
{
    // Create online main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(126, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "connect info", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_online_connect*)m_CurrentDialog)->Configure( CONNECT_MODE_CONNECT );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineConnect( void )
{

    if( m_BackgroundRendererRunning )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_EXIT:
            {
                // We got connected, so we make sure that we know we've not
                // been notified of a disconnect due to network being down.
                g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );

                if( (GetProfileListIndex(0) != -1) && 
                    (GetSelectedProfile( GetProfileListIndex(0) ) != 0 ) )
                {
                    // set nickname
                    player_profile& ActiveProfile = GetActiveProfile( GetProfileListIndex(0) );
                    g_MatchMgr.SetNickname( ActiveProfile.GetProfileName() );

#ifdef TARGET_XBOX
                    SetState( SM_ONLINE_AUTHENTICATE );
#else
                    if( ActiveProfile.m_bAgeVerified )
                    {
                        // authenticate this user account
                        SetState( SM_ONLINE_AUTHENTICATE );
                    }
                    else
                    {
                        // verify age of profile user
                        SetState( SM_ONLINE_COPA );
                    }
#endif
                }
                else
                {
                    SetState( SM_ONLINE_PROFILE_SELECT );
                }
            }
            break;

            case DIALOG_STATE_BACK:
            {
                DisconnectFromNetwork();
                g_UiMgr->EndDialog( g_UiUserID, TRUE );
                SetState( SM_MAIN_MENU );
            }
            break;
        }
    }    
}

//=========================================================================

void state_mgr::ExitOnlineConnect( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineProfileSelect( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineProfileSelect( void )
{
    // Prevent reentrancy problems if we're disconnecting from
    // the network.
    if( m_BackgroundRendererRunning )
    {
        return;
    }

    if( m_CurrentDialog->GetState() != DIALOG_STATE_WAIT_FOR_MEMCARD )
    {
        if( CheckForDisconnect() )
        {
            return;
        }
    }

    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
#ifdef TARGET_XBOX
                SetState( SM_ONLINE_AUTHENTICATE );
#else
                if( GetActiveProfile(0).m_bAgeVerified )
                {
                    // authenticate this user account
                    SetState( SM_ONLINE_AUTHENTICATE );
                }
                else
                {
                    // verify age of profile user
                    SetState( SM_ONLINE_COPA );
                }
#endif
            }
            break;

            case DIALOG_STATE_BACK:
            {
                DisconnectFromNetwork();
                SetState( SM_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_ONLINE_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineProfileSelect( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineProfileOptions( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(s_ProfileOptLeft, DIALOG_TOP, s_ProfileOptRight, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif

    // configure the dialog for MP options-
    ((dlg_profile_options*)m_CurrentDialog)->Configure( PROFILE_OPTIONS_NORMAL, TRUE );
}

//=========================================================================

void state_mgr::UpdateOnlineProfileOptions( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PROFILE_OPTIONS_CONTROLS:
                    {
                        SetState( SM_ONLINE_PROFILE_CONTROLS );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_AVATAR:
                    {
                        SetState( SM_ONLINE_PROFILE_AVATAR );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_ACCEPT:
                    {
                    #ifdef TARGET_PC
                        SetState( SM_ONLINE_MAIN_MENU );
                    #elif (defined TARGET_XBOX)
                        SetState( SM_ONLINE_AUTHENTICATE );
                    #else
                        // go to memcard select to save the profile
                        SetState( SM_ONLINE_MEMCARD_SELECT );  
                    #endif
                    }
                    break;
                }
            }
            break;

            case DIALOG_STATE_BACK:
            {
                // abort create/edit profile
                SetState( SM_ONLINE_PROFILE_SELECT );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineProfileOptions( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineProfileControls( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile controls", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineProfileControls( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineProfileControls( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineProfileAvatar( void )
{
    // Create avatar dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(121, DIALOG_TOP, 391, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "avatar select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineProfileAvatar( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_PROFILE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineProfileAvatar( void )
{
}

//=========================================================================
#ifndef TARGET_XBOX
void state_mgr::EnterOnlineCOPA( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(116, DIALOG_TOP, 396, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "COPA", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineCOPA( void )
{
    if( ( CONFIG_IS_AUTOSERVER ) || ( CONFIG_IS_AUTOCLIENT ) )
    {
        SetState( SM_ONLINE_AUTHENTICATE );
    }

    // Prevent reentrancy problems if we're disconnecting from
    // the network.
    if( m_BackgroundRendererRunning )
    {
        return;
    }

    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
        case DIALOG_STATE_SELECT:
            // profile age verification was successful
            // so authenticate this user account
            SetState( SM_ONLINE_AUTHENTICATE );
            break;

        case DIALOG_STATE_BACK:
            // age verification failed - return to offline land
#ifdef TARGET_PS2
            // MAB: clear selected profile
            // we MUST do this because the IOP is rebooted when we exit the online
            // component and this triggers a "card changed" message the first time
            // we mount the memory card.  Autosave will fail because of this.
            // forcing the user to reselect a profile solves the problem.
            ClearSelectedProfile(0);
#endif
            // disconnect
            DisconnectFromNetwork();
            // go back to the main menu
            SetState( SM_MAIN_MENU );
            break;

        case DIALOG_STATE_MEMCARD_ERROR:
            // failed when attempting to save the profile
            // select a profile to be overwritten
            SetState( SM_ONLINE_COPA_SAVE_SELECT );
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineCOPA( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineCOPASaveSelect( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineCOPASaveSelect( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected profile was overwritten
                SetState( SM_ONLINE_AUTHENTICATE );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // continue without saving
                SetState( SM_ONLINE_AUTHENTICATE );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // Select a card to save the profile to
                SetState( SM_ONLINE_COPA_MEMCARD_RESELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineCOPASaveSelect( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineCOPAMemcardReselect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateOnlineCOPAMemcardReselect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            SetState( SM_ONLINE_AUTHENTICATE );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_ONLINE_COPA_SAVE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineCOPAMemcardReselect( void )
{
}
#endif
//=========================================================================

void state_mgr::EnterOnlineAuthenticate( void )
{

    // Create online main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(126, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "connect info", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
 //   ASSERT( g_MatchMgr.GetState() == MATCH_AUTH_DONE );
    ((dlg_online_connect*)m_CurrentDialog)->Configure( CONNECT_MODE_AUTH_USER );
}

//=========================================================================

void state_mgr::UpdateOnlineAuthenticate( void )
{
    #ifdef TARGET_XBOX
    {
        // At this point the Xbox will have performed all required authentication.
        // No further work is necessary, so just go straight to the main menu.
        SetState( SM_ONLINE_MAIN_MENU );
        return;
    }    
    #endif

    // Prevent reentrancy problems if we're disconnecting from
    // the network.
    if( m_BackgroundRendererRunning )
    {
        return;
    }

    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
        case DIALOG_STATE_ACTIVE:
            break;
        case DIALOG_STATE_EXIT:
            g_UiMgr->EndDialog( g_UiUserID, TRUE );
            m_CurrentDialog = NULL;

            SetState( SM_ONLINE_MAIN_MENU );
            break;

        case DIALOG_STATE_BACK:
            g_UiMgr->EndDialog( g_UiUserID, TRUE );
            m_CurrentDialog = NULL;

            SetState( SM_MAIN_MENU );
            break;
        default:
            ASSERT( FALSE );
            g_UiMgr->EndDialog( g_UiUserID, TRUE );
            m_CurrentDialog = NULL;

            SetState( SM_MAIN_MENU );
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineAuthenticate( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineProfileSaveSelect( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineProfileSaveSelect( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected profile was overwritten
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // continue without saving
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // Select a card to save the profile to
                SetState( SM_ONLINE_MEMCARD_RESELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineProfileSaveSelect( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineMemcardReselect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateOnlineMemcardReselect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the online main menu
            SetState( SM_ONLINE_MAIN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_ONLINE_PROFILE_SAVE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineMemcardReselect( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineMain( void )
{
#ifdef USE_MOVIES
    if( !m_bPlayMovie )
    {
        Movie.Init();
        //Movie.SetLanguage(XL_LANG_ENGLISH);
        m_bPlayMovie = Movie.Open( SelectBestClip("MenuBackground"),TRUE,TRUE );
    }
#else
    s32 Background = g_UiMgr->FindBackground("background1");
    // Load the background. This *may* not have already been loaded depending on where entry to the online ui
    // happens. If it comes from main menu, then it will have been loaded but if it comes from post-game exit,
    // then it will have been unloaded so we need to load it up again.
    if( Background == -1 )
    {
        Background = g_UiMgr->LoadBackground ( "background1", "A51Background.xbmp" );
    }
#endif

    // Create online main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(106, DIALOG_TOP, 406, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "online menu", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
#else
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineMain( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }
    if( m_BackgroundRendererRunning )
    {
        return;
    }

    if( m_bFollowBuddy )
    {
        m_bFollowBuddy = FALSE;
        StartLogin( &g_PendingConfig.GetConfig(), LOGIN_FROM_INVITE );
        return;
    }


    // check for menu automation
    if( CONFIG_IS_AUTOSERVER )
    {
        SetState( SM_ONLINE_HOST_MENU );
    }
    else if( CONFIG_IS_AUTOCLIENT )
    {
        SetState( SM_ONLINE_JOIN_MENU );
    }
    else
    {
        if( m_CurrentDialog != NULL )
        {
            u32 DialogState = m_CurrentDialog->GetState();

            switch( DialogState )
            {
                #ifdef TARGET_XBOX
                // We have detected a pending accepted invite inside the online main dialog.
                {
                    case DIALOG_STATE_ACTIVATE:
                    {
                        XONLINE_LATEST_ACCEPTED_GAMEINVITE& Invitation = g_MatchMgr.GetInviteAccepted();

                        server_info& Config = g_PendingConfig.GetConfig();
                        x_memset( &Config, 0, sizeof(server_info) );
                        x_memcpy( &Config.SessionID, &Invitation.SessionID, sizeof(Config.SessionID) );
                        ASSERT( sizeof(m_FeedbackName) >= XONLINE_GAMERTAG_SIZE );
                        x_strncpy( m_FeedbackName, Invitation.szInvitingUserGamertag, XONLINE_GAMERTAG_SIZE );
                        // These values are actually unknown but we fill in these just to stop asserts.
                        // The MaxPlayers and Players is irrelevent but usually we have this information.
                        Config.GameTypeID = GAME_MP;
                        Config.MaxPlayers = 32;
                        Config.Players    = 1;

                        // We only ever want to see the accepted invite dialog once
                        g_MatchMgr.SetPendingInviteAccepted( FALSE );

                        SetState( SM_FOLLOW_BUDDY );
                        return;
                    }
                    break;
                }
                #endif
                
                case DIALOG_STATE_SELECT:
                {
                    s32 Control = m_CurrentDialog->GetControl();

                    switch( Control )
                    {
    #ifdef TARGET_XBOX
                        case IDC_ONLINE_QUICKMATCH:
                            #ifdef LAN_PARTY_BUILD
                                g_MatchMgr.SetFilter( (game_type)-1, -1, -1 );
                                SetState( SM_ONLINE_JOIN_MENU );
                            #else
                                SetState( SM_ONLINE_QUICKMATCH );
                            #endif

                            break;

                        case IDC_ONLINE_OPTIMATCH:
                            InitPendingSettings();
                            SetState( SM_ONLINE_JOIN_FILTER );
                            break;
    #else
                        case IDC_ONLINE_JOIN:
                            #ifdef LAN_PARTY_BUILD
                                g_MatchMgr.SetFilter( (game_type)-1, -1, -1 );
                                SetState( SM_ONLINE_JOIN_MENU );
                            #else
                                InitPendingSettings();
                                SetState( SM_ONLINE_JOIN_FILTER );
                            #endif

                            break;
    #endif
                        case IDC_ONLINE_HOST:
                            InitPendingSettings();
                            SetState( SM_ONLINE_HOST_MENU );
                            break;

                        case IDC_ONLINE_FRIENDS:
                            SetState( SM_ONLINE_FRIENDS_MENU );
                            break;

                        case IDC_ONLINE_PLAYERS:
                            SetState( SM_ONLINE_PLAYERS_MENU );
                            break;

                        case IDC_ONLINE_EDIT_PROFILE:
                            SetState( SM_ONLINE_EDIT_PROFILE );
                            break;

                        case IDC_ONLINE_VIEW_STATS:
                            SetState( SM_ONLINE_STATS );
                            break;
#if defined(TARGET_PS2)
                        case IDC_ONLINE_DOWNLOAD:
                            SetState( SM_ONLINE_DOWNLOAD );
#endif
                            break;

                        default:
                            ASSERTS( FALSE, "Selection not supported!" );
                            break;
                    }
                }
                break;

                case DIALOG_STATE_BACK:
                {
#ifdef TARGET_PS2
                    // MAB: clear selected profile
                    // we MUST do this because the IOP is rebooted when we exit the online
                    // component and this triggers a "card changed" message the first time
                    // we mount the memory card.  Autosave will fail because of this.
                    // forcing the user to reselect a profile solves the problem.
                    ClearSelectedProfile(0);
#endif
                    // disconnect
                    DisconnectFromNetwork();
                    SetState( SM_MAIN_MENU );
                }
                break;
            }
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineMain( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineHost( void )
{
    // Create online host menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(12, DIALOG_TOP, 500, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "online host", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineHost( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    // check for menu automation
    if( CONFIG_IS_AUTOSERVER )
    {
#ifndef CONFIG_RETAIL
        g_PendingConfig.SetGameTypeID( (game_type)g_Config.AutoServerType );
        g_PendingConfig.SetPlayerCount(1);
        g_PendingConfig.SetLevelID( g_Config.AutoLevel );
        g_PendingConfig.SetMutationMode( (mutation_mode)g_Config.AutoMutateMode );
        g_PendingConfig.SetMaxPlayerCount( 16 );

        switch( g_Config.AutoServerType )
        {
        case GAME_DM:   g_PendingConfig.SetGameType( "Deathmatch" );        g_PendingConfig.SetShortGameType( "DM" );       break;
        case GAME_TDM:  g_PendingConfig.SetGameType( "Team Deathmatch" );   g_PendingConfig.SetShortGameType( "TDM" );      break;
        case GAME_CTF:  g_PendingConfig.SetGameType( "Capture the Flag" );  g_PendingConfig.SetShortGameType( "CTF" );      break;
        case GAME_TAG:  g_PendingConfig.SetGameType( "Tag" );               g_PendingConfig.SetShortGameType( "Tag" );      break;
        case GAME_INF:  g_PendingConfig.SetGameType( "Infection" );         g_PendingConfig.SetShortGameType( "Inf" );      break;
        case GAME_CNH:  g_PendingConfig.SetGameType( "Capture and Hold" );  g_PendingConfig.SetShortGameType( "CNH" );      break;
        }

        // tell the network manager that we want to be a server
        g_NetworkMgr.BecomeServer();

        SetState( SM_ONLINE_HOST_MENU_OPTIONS );
#endif
    }
    else if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                SetState( SM_ONLINE_HOST_MENU_OPTIONS );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineHost( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineHostOptions( void )
{
    // Create online host menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(12, DIALOG_TOP, 500, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "host options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineHostOptions( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    // check for menu automation
    if( CONFIG_IS_AUTOSERVER )
    {
        SetState( SM_START_GAME );
    }
    else if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_HOST_MENU );
            }
            break;

            case DIALOG_STATE_SELECT:
            {
                SetState( SM_HOST_LEVEL_SELECT );               
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineHostOptions( void )
{
}

//=========================================================================

void state_mgr::EnterHostSaveSettings( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->Configure( SM_CARDMODE_SETTINGS );
#endif
}

//=========================================================================

void state_mgr::UpdateHostSaveSettings( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // start up the server
            g_NetworkMgr.BecomeServer();
            SetState( SM_START_GAME );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            g_NetworkMgr.BecomeServer();
            SetState( SM_START_GAME );
        }
    }
}

//=========================================================================

void state_mgr::ExitHostSaveSettings( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineQuickMatch( void )
{
    // Create quick match menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(101, DIALOG_TOP, 411, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "quick match", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineQuickMatch( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_SELECT:
            {
                StartLogin( &g_PendingConfig.GetConfig(), LOGIN_FROM_MAIN );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // could not find a match, create one
                SetState( SM_ONLINE_HOST_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineQuickMatch( void )
{
    g_UiMgr->EndUsersDialogs( g_UiUserID );
}

//=========================================================================

void state_mgr::EnterOnlineOptiMatch( void )
{
}

//=========================================================================

void state_mgr::UpdateOnlineOptiMatch( void )
{
}

//=========================================================================

void state_mgr::ExitOnlineOptiMatch( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineJoinFilter( void )
{
    // Create online host menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "join filter", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineJoinFilter( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                SetState( SM_ONLINE_JOIN_SAVE_SETTINGS );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_ONLINE_JOIN_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineJoinFilter( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineJoin( void )
{
    // Create online join menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(-20, DIALOG_TOP, 532, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "online join", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineJoin( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
#ifdef LAN_PARTY_BUILD
                SetState( SM_ONLINE_MAIN_MENU );
#else
                SetState( SM_ONLINE_JOIN_FILTER );
#endif
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_SERVER_PLAYERS_MENU );
            }
            break;

            case DIALOG_STATE_SELECT:
            {
                StartLogin( &g_PendingConfig.GetConfig(), LOGIN_FROM_JOIN_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineJoin( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineJoinSaveSettings( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->Configure( SM_CARDMODE_SETTINGS );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineJoinSaveSettings( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save was successful
            // goto the online join (server list)
            SetState( SM_ONLINE_JOIN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // save failed or was aborted
            // goto the online join (server list)
            SetState( SM_ONLINE_JOIN_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineJoinSaveSettings( void )
{
}

//=========================================================================

void state_mgr::EnterServerPlayers( void )
{
    // Create server players menu - list of players on the selected server
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(0, DIALOG_TOP, 512, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "server players", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateServerPlayers( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
                SetState( SM_ONLINE_JOIN_MENU );
            break;

            case DIALOG_STATE_SELECT:
                StartLogin( &g_PendingConfig.GetConfig(), LOGIN_FROM_JOIN_MENU );
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitServerPlayers( void )
{
}

//=========================================================================

void state_mgr::EnterOnlinePlayers( void )
{
    // Create players menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "players list", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );

    ((dlg_players*)m_CurrentDialog)->Configure( PLAYER_MODE_RECENT );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlinePlayers( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                // this probably should go in the enter state...
                // set controller back to any
                g_StateMgr.SetActiveControllerID( -1 );
                g_UiMgr->SetUserController( g_UiUserID, -1 );
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_ONLINE_FEEDBACK_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlinePlayers( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineFeedback( void )
{
    // Create feedback menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "feedback", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateOnlineFeedback( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_PLAYERS_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineFeedback( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineFeedbackFriend( void )
{
    // Create feedback menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "feedback", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );

    for( s32 i=0; i<g_MatchMgr.GetBuddyCount(); i++ )
    {
        buddy_info& Buddy = g_MatchMgr.GetBuddy(i);
        if( Buddy.Identifier == m_FeedbackIdentifier )
        {
            if( Buddy.Flags & USER_VOICE_MESSAGE )
            {
                ((dlg_feedback*)m_CurrentDialog)->ChangeConfig(dlg_feedback::ATTACHMENT_FEEDBACK);
                break;
            }
        }
    }
}

//=========================================================================

void state_mgr::UpdateOnlineFeedbackFriend( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_FRIENDS_MENU );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineFeedbackFriend( void )
{
}
//=========================================================================

void state_mgr::EnterOnlineFriends( void )
{
    // Create friends menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "friends list", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineFriends( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                // this probably should go in the enter state...
                // set controller back to any
                g_StateMgr.SetActiveControllerID( -1 );
                g_UiMgr->SetUserController( g_UiUserID, -1 );
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_SELECT:
            {
                StartLogin( &g_PendingConfig.GetConfig(), LOGIN_FROM_FRIENDS );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_ONLINE_FEEDBACK_MENU_FRIEND );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineFriends( void )
{
}



//=========================================================================

void state_mgr::EnterOnlineStats( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "stats", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateOnlineStats( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_BACK )
        {
            SetState( SM_ONLINE_MAIN_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineStats( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineEditProfile( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(s_ProfileOptLeft, DIALOG_TOP, s_ProfileOptRight, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif

    // configure the dialog for MP options-
    ((dlg_profile_options*)m_CurrentDialog)->Configure( PROFILE_OPTIONS_NORMAL, FALSE );
}

//=========================================================================

void state_mgr::UpdateOnlineEditProfile( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PROFILE_OPTIONS_CONTROLS:
                    {
                        SetState( SM_ONLINE_EDIT_CONTROLS );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_AVATAR:
                    {
                        SetState( SM_ONLINE_EDIT_AVATAR );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_ACCEPT:
                    {
                    #if defined(TARGET_PC) || defined(TARGET_XBOX)
                        SetState( SM_ONLINE_MAIN_MENU );
                    #else
                        // go to memcard select to save the profile
                        SetState( SM_ONLINE_MEMCARD_SELECT );  
                    #endif
                    }
                    break;
                }
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_ONLINE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                // save failed - select a profile to save to
                SetState( SM_ONLINE_PROFILE_SAVE_SELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineEditProfile( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineEditControls( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile controls", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineEditControls( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_EDIT_PROFILE );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineEditControls( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineEditAvatar( void )
{
    // Create avatar dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(121, DIALOG_TOP, 391, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "avatar select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateOnlineEditAvatar( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_EDIT_PROFILE );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineEditAvatar( void )
{
}

//=========================================================================

void state_mgr::EnterHostLevelSelect( void )
{
    //  Create level select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "online level select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateHostLevelSelect( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_ONLINE_HOST_MENU_OPTIONS );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
#ifdef TARGET_PC
                // advance to game
                g_NetworkMgr.BecomeServer();
                SetState( SM_START_GAME );
#else
                // save to memcard and return to menu
                SetState( SM_HOST_SAVE_SETTINGS );
#endif
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // tell the network manager that we want to be a server
                g_NetworkMgr.BecomeServer();
                SetState( SM_START_GAME );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitHostLevelSelect( void )
{
}

//=========================================================================

void state_mgr::EnterLevelSelect( void )
{
    //  Create level select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(116, DIALOG_TOP, 396, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "level select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateLevelSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                g_PendingConfig.SetMaxPlayerCount( 1 );

                g_NetworkMgr.BecomeServer();
                EnableDefaultLoadOut();
                SetState( SM_START_GAME );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( m_PrevState );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitLevelSelect( void )
{
}

//=========================================================================

void state_mgr::EnterStartSaveGame( void )
{
    //  Start new game option
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    s32 XRes, YRes;
    eng_GetRes(XRes, YRes);
    irect mainarea(-75, DIALOG_TOP, XRes+75, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "start game", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateStartSaveGame( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            m_bStartSaveGame = TRUE;
            SetState( SM_SINGLE_PLAYER_LOAD_MISSION );
        }
    }
}

//=========================================================================

void state_mgr::ExitStartSaveGame( void )
{
}
    
//=========================================================================

void state_mgr::EnterStartGame( void )
{
    ASSERT( m_BackgroundRendererRunning==FALSE );
    // prepare for loading the level
    m_bShowingScores = FALSE;
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    m_CurrentDialog = NULL;
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
    g_RscMgr.Unload( "DX_FrontEnd.audiopkg"    );
    g_RscMgr.Unload( "SFX_FrontEnd.audiopkg"   );
    g_RscMgr.Unload( "MUSIC_FrontEnd.audiopkg" );
    g_UiMgr->UnloadBackground( "titlescreen" );

#ifdef USE_MOVIES
    // Kill the background movie
    Movie.Close();
    Movie.Kill();
    m_bPlayMovie = FALSE;
#endif

    s32 XRes, YRes;
    eng_GetRes(XRes, YRes);
    irect mainarea(-75, DIALOG_TOP, XRes+75, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "start game", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdateStartGame( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // Calculate checksum for profile prior to starting game. This will mark the profile
            // as having not been modified.
            player_profile& ActiveProfile = GetActiveProfile( GetProfileListIndex(0) );
            ActiveProfile.Checksum();

            m_bStartSaveGame = FALSE;
            s_FirstMap = TRUE;

            // advance to either a load screen, or a level description 
            // screen (which also does loading in the background)
            if( GameMgr.GetGameType() == GAME_CAMPAIGN )
                SetState( SM_SINGLE_PLAYER_LOAD_MISSION );
            else
                SetState( SM_MULTI_PLAYER_LOAD_MISSION );
        }
    }
}

//=========================================================================

void state_mgr::ExitStartGame( void )
{
}

//=========================================================================
void state_mgr::EnterSinglePlayerLoadMission( void )
{
    //**BW** Bit of a hack here. We don't want to render anything new since
    //we're trying to display the score at this time.
    //if( m_State == SM_INTER_LEVEL_SCORE )
    //{
    //    m_bShowingScores = TRUE;
    //    return;
    //}

    // prepare for loading the level
    m_bShowingScores = FALSE;
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    m_CurrentDialog = NULL;
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
    g_RscMgr.Unload( "DX_FrontEnd.audiopkg"    );
    g_RscMgr.Unload( "SFX_FrontEnd.audiopkg"   );
    g_RscMgr.Unload( "MUSIC_FrontEnd.audiopkg" );
    g_UiMgr->UnloadBackground( "titlescreen" );

#ifdef USE_MOVIES
    // Kill the background movie
    Movie.Close();
    Movie.Kill();
    m_bPlayMovie = FALSE;
#endif

#if !defined( X_EDITOR )
    // are we starting a new campaign?
    if( g_PendingConfig.GetGameTypeID() ==  GAME_CAMPAIGN )
    {
#ifdef USE_MOVIES
        const map_entry* pEntry = g_MapList.GetByIndex( m_LevelIndex );
        s32 iCheckpoint = g_CheckPointMgr.GetCheckPointIndex();

        // iCheckpoint == -1 when entering the level for the very first time in campaign
        // iCheckpoint == 0 when using Resume Campaign and selecting the level itself instead of a checkpoint
        // In BOTH cases, we want to play the intro cine.
        // If the player has died since the level was initially loaded, don't play the cine.
        if ( ((iCheckpoint == -1) || (iCheckpoint == 0)) && (!player::s_bPlayerDied) )
        {
            switch( pEntry->GetMapID() )
            {
                case 1000:
                    // Play intro movie before starting "One of them".
                    if( !PlaySimpleMovie( SelectBestClip("CinemaIntro")) )
                    {
                        DummyScreen( "Intro Movie Here", TRUE, 5 );
                    }
                    break;
                case 1020:
                    // unlock a secret after deep underground
                    {
                        player_profile& ActiveProfile = GetActiveProfile(0);                    
                        ActiveProfile.AcquireSecret();
                        x_memcpy( &m_PendingProfile, &ActiveProfile, sizeof(ActiveProfile) );
                    }
                    break;
                case 1060:
                    // Play infection movie before starting "One of them".
                    if( !PlaySimpleMovie( SelectBestClip( "CinemaInfection" )) )
                    {
                        DummyScreen( "Infection Movie Here", TRUE, 5 );
                    }
                    break;
                case 1115:
                    // Play Edgar movie before starting "The Grays".
                    if( !PlaySimpleMovie( SelectBestClip( "CinemaEdgar")) )
                    {
                        DummyScreen( "Edgar Movie Here", TRUE, 5 );
                    }
                    break;
                default:
                    // nothing to do!
                    break;
            }
        }
#endif // USE_MOVIES
    }

#endif

    // Create a load game dialog
    irect mainarea(116, DIALOG_TOP, 396, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "load game", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateSinglePlayerLoadMission( void )
{
    if( IsBackgroundThreadRunning() )
        return;

    dlg_load_game*  pLoadGame   = (dlg_load_game*)g_UiMgr->GetTopmostDialog(g_UiUserID);
    server_state    ServerState = g_NetworkMgr.GetServerObject().GetState();
    dialog_states   State       = pLoadGame->GetState();

    if( ServerState != STATE_SERVER_LOAD_MISSION )
    {
        if( State == DIALOG_STATE_EXIT )
        {
            SetState( SM_SERVER_SYNC );
        }
        // let the dialog know that we have finished loading the game
        pLoadGame->LoadingComplete();
    }
}

//=========================================================================

void state_mgr::ExitSinglePlayerLoadMission( void )
{
    // end the load game dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );

#if CONFIG_IS_DEMO
    g_DemoIdleTimer.Reset();
    g_DemoIdleTimer.Start();
#endif
}

//=========================================================================
void state_mgr::EnterPlayingGame( void )
{
    m_bIsPaused      = FALSE;        
    m_bDoSystemError = FALSE;
    m_PopUp          = NULL;
    s_FirstMap       = FALSE;

    // This means that we have actually entered the game so if there is any sort of
    // failure from this point on, we need to go back to the main menu. This will be
    // used in ExitReportError to decide where to go next.
    m_LoginFailureDestination = LOGIN_FROM_MAIN;
}

//=========================================================================
void state_mgr::UpdatePlayingGame( void )
{
    if( g_ActiveConfig.GetExitReason()!=GAME_EXIT_CONTINUE )
    {
        if( g_NetworkMgr.IsServer() )
        {
            SetState( SM_SERVER_COOLDOWN );
        }
        else
        {
            SetState( SM_CLIENT_COOLDOWN );
        }
    }
}

//=========================================================================
void state_mgr::ExitPlayingGame( void )
{
}

//=========================================================================
// This is the state transition that occurs after a level has completed loading
// and we want to enter the game. It is the responsibility of this UpdateLoadSync
// function to make sure all the ducks are lined up and ready to go prior to jumping
// in to the game.

void state_mgr::EnterServerSync( void )
{
}

//=========================================================================

void state_mgr::UpdateServerSync( void )
{
    if( IsBackgroundThreadRunning() )
        return;
    server_state ServerState = g_NetworkMgr.GetServerObject().GetState();
    if( ServerState == STATE_SERVER_INGAME )
    {
        SetState( SM_PLAYING_GAME );
    }
    else if( ServerState != STATE_SERVER_SYNC )
    {
        SetState( SM_SERVER_COOLDOWN );
    }
}

//=========================================================================

void state_mgr::ExitServerSync( void )
{
}

//=========================================================================
// This is the state transition that occurs after a level has completed loading
// and we want to enter the game. It is the responsibility of this UpdateLoadSync
// function to make sure all the ducks are lined up and ready to go prior to jumping
// in to the game.

void state_mgr::EnterClientSync( void )
{
}

//=========================================================================

void state_mgr::UpdateClientSync( void )
{
    client_state ClientState = g_NetworkMgr.GetClientObject().GetState();
    if( ClientState == STATE_CLIENT_INGAME )
    {
        SetState( SM_PLAYING_GAME );
    }
    else if( ClientState != STATE_CLIENT_SYNC )
    {
        SetState( SM_MULTI_PLAYER_LOAD_MISSION );
    }
}

//=========================================================================

void state_mgr::ExitClientSync( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMain( void )
{
    // Create the main pause dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 136, DIALOG_TOP, 376, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "pause main", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );

    // Pause Audio
    g_AudioMgr.PauseAll();
}

//=========================================================================

void state_mgr::UpdatePauseMain( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            s32 Control = m_CurrentDialog->GetControl();

            if( Control == IDC_PAUSE_MENU_RESUME )
            {
                // return to game
                SetState( SM_END_PAUSE );
            }
            else if( Control == IDC_PAUSE_MENU_QUIT )
            {
                // exit from game to main menu
                SetState( SM_EXIT_GAME );
            }
            else if( Control == IDC_PAUSE_MENU_OPTIONS )
            {
                // edit profile
                g_StateMgr.InitPendingProfile( 0 );
                SetState( SM_PAUSE_OPTIONS );
            }
            else if( Control == IDC_PAUSE_MENU_SETTINGS )
            {
                // edit settings
                InitPendingSettings();
                SetState( SM_PAUSE_SETTINGS );
            }
#ifdef TARGET_XBOX
            else if( Control == IDC_PAUSE_MENU_FRIENDS )
            {
                SetState( SM_PAUSE_FRIENDS );
            }
#endif
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMain( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOptions( void )
{
    // Pause options screen
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(s_ProfileOptLeft, DIALOG_TOP, s_ProfileOptRight, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // configure the dialog for MP options-
    ((dlg_profile_options*)m_CurrentDialog)->Configure( PROFILE_OPTIONS_PAUSE, FALSE );
}

//=========================================================================

void state_mgr::UpdatePauseOptions( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PROFILE_OPTIONS_CONTROLS:
                    {
                        SetState( SM_PAUSE_CONTROLS );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_ACCEPT:
                    {
                    #if defined(TARGET_PC) || defined(TARGET_XBOX)
                        SetState( SM_PAUSE_MAIN_MENU );
                    #else
                        // need to save new profile
                        SetState( SM_PAUSE_MEMCARD_SELECT );
                    #endif
                    }
                    break;
                }
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_PAUSE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                // save failed - select a profile to save to
                SetState( SM_PAUSE_PROFILE_SAVE_SELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOptions( void )
{
}

//=========================================================================

void state_mgr::EnterPauseControls( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile controls", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_profile_controls*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseControls( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseControls( void )
{
}

//=========================================================================

void state_mgr::EnterPauseSettings ( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(16, DIALOG_TOP, 496, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "av settings", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_av_settings*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseSettings( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_SELECT:
            {
                SetState( SM_PAUSE_HEADSET );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                SetState( SM_PAUSE_SETTINGS_SELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseSettings  ( void )
{
}

//=========================================================================

void state_mgr::EnterPauseHeadset ( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "headset", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_headset*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseHeadset ( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_SETTINGS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseHeadset ( void )
{
}

//=========================================================================

void state_mgr::EnterPauseSettingsSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->Configure( SM_CARDMODE_SETTINGS );
#endif
}

//=========================================================================

void state_mgr::UpdatePauseSettingsSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save settings was successful
            SetState( SM_PAUSE_MAIN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // save failed or was aborted
            SetState( SM_PAUSE_MAIN_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseSettingsSelect( void )
{
}

//=========================================================================
#ifdef TARGET_XBOX
void state_mgr::EnterPauseFriends ( void )
{
    // Create friends menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "friends list", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_friends*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseFriends( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_PAUSE_ONLINE_FEEDBACK_FRIEND );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseFriends  ( void )
{
}
#endif
//=========================================================================

void state_mgr::EnterPauseMemcardSaveSelect ( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_profile_select*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseMemcardSaveSelect( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected profile was overwritten
                SetState( SM_PAUSE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // continue without saving
                SetState( SM_PAUSE_MAIN_MENU );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // Select a card to save the profile to
                SetState( SM_PAUSE_MEMCARD_RESELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMemcardSaveSelect  ( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMemcardReselect ( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->EnableBlackout();
#endif
}

//=========================================================================

void state_mgr::UpdatePauseMemcardReselect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the campaign menu
            SetState( SM_PAUSE_MAIN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_PAUSE_PROFILE_SAVE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMemcardReselect  ( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMP( void )
{
    // Create the main pause dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 136, DIALOG_TOP, 376, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "pause multiplayer", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
}

//=========================================================================

void state_mgr::UpdatePauseMP( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            s32 Control = m_CurrentDialog->GetControl();

            if( Control == IDC_PAUSE_MP_QUIT )
            {
                // exit from game to main menu
                SetState( SM_EXIT_GAME );
            }
            else if( Control == IDC_PAUSE_MP_SCORE )
            {
                // remove
                SetState( SM_PAUSE_MP_SCORE );
            }
            else if( Control == IDC_PAUSE_MP_OPTIONS )
            {
                // init pending profile for the player that paused the game
                g_StateMgr.InitPendingProfile( GetActiveControllerID() );
                SetState( SM_PAUSE_MP_OPTIONS );
            }
            else if( Control == IDC_PAUSE_MP_SETTINGS )
            {
                // edit settings
                InitPendingSettings();
                SetState( SM_PAUSE_MP_SETTINGS );
            }
#ifdef TARGET_XBOX
            else if( Control == IDC_PAUSE_MP_FRIENDS )
            {
                SetState( SM_PAUSE_MP_FRIENDS );
            }
#endif
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMP( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMPSettings( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(16, DIALOG_TOP, 496, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "av settings", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_av_settings*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseMPSettings( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_MP );
            }
            break;

            case DIALOG_STATE_SELECT:
            {
                SetState( SM_PAUSE_MP_HEADSET );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                SetState( SM_PAUSE_MP_SETTINGS_SELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPSettings( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMPHeadset ( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "headset", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_headset*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseMPHeadset ( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_MP_SETTINGS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPHeadset ( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMPSettingsSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->Configure( SM_CARDMODE_SETTINGS );
#endif
}

//=========================================================================

void state_mgr::UpdatePauseMPSettingsSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save settings was successful
            SetState( SM_PAUSE_MP );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // save failed or was aborted
            SetState( SM_PAUSE_MP );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPSettingsSelect( void )
{
}

//=========================================================================
#ifdef TARGET_XBOX
void state_mgr::EnterPauseMPFriends( void )
{
    // Create friends menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "friends list", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_friends*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseMPFriends( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_MP );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPFriends( void )
{
}
#endif
//=========================================================================

void state_mgr::EnterPauseMPOptions( void )
{
    // Pause options screen
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(s_ProfileOptLeft, DIALOG_TOP, s_ProfileOptRight, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // configure the dialog for MP options-
    ((dlg_profile_options*)m_CurrentDialog)->Configure( PROFILE_OPTIONS_PAUSE, FALSE );
}

//=========================================================================

void state_mgr::UpdatePauseMPOptions( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PROFILE_OPTIONS_CONTROLS:
                    {
                        SetState( SM_PAUSE_MP_CONTROLS );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_ACCEPT:
                    {
#if defined(TARGET_PC) || defined(TARGET_XBOX)
                        SetState( SM_PAUSE_MP );
#else
                        // need to save new profile
                        SetState( SM_PAUSE_MP_MEMCARD_SELECT );
#endif
                    }
                    break;
                }
            }
            break;

        case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_PAUSE_MP );
            }
            break;

        case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_MP );
            }
            break;

        case DIALOG_STATE_MEMCARD_ERROR:
            {
                // save failed - select a profile to save to
                SetState( SM_PAUSE_MP_PROFILE_SAVE_SELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPOptions( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMPControls( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile controls", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_profile_controls*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseMPControls( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_MP_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPControls( void )
{
}

//=========================================================================
#if defined(TARGET_PS2)
void state_mgr::EnterPauseMPMemcardSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );

    // enable background filter
    ((dlg_memcard_select*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseMPMemcardSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the pause main menu
            SetState( SM_PAUSE_MP );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save was aborted
            SetState( SM_PAUSE_MP );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPMemcardSelect( void )
{
}
#endif
//=========================================================================

void state_mgr::EnterPauseMPMemcardSaveSelect( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_profile_select*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseMPMemcardSaveSelect( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected profile was overwritten
                SetState( SM_PAUSE_MP );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // continue without saving
                SetState( SM_PAUSE_MP );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // Select a card to save the profile to
                SetState( SM_PAUSE_MP_MEMCARD_RESELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPMemcardSaveSelect( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMPMemcardReselect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->EnableBlackout();
#endif
}

//=========================================================================

void state_mgr::UpdatePauseMPMemcardReselect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            SetState( SM_PAUSE_MP );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            SetState( SM_PAUSE_MP_PROFILE_SAVE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPMemcardReselect( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnline( void )
{
    // Create the main pause dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 86, DIALOG_TOP, 426, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "pause online", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );

    // Pause Audio
//  g_AudioMgr.PauseAll();
}

//=========================================================================

void state_mgr::UpdatePauseOnline( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PAUSE_ONLINE_SWITCH_TEAM:
                    case IDC_PAUSE_ONLINE_SUICIDE:
                        // exit pause and rejoin game
                        SetPaused( FALSE, GetActiveControllerID() );
                        break;

                    case IDC_PAUSE_ONLINE_VOTE_MAP:
                        SetState( SM_PAUSE_ONLINE_VOTE_MAP );
                        break;

                    case IDC_PAUSE_ONLINE_VOTE_KICK:
                        SetState( SM_PAUSE_ONLINE_VOTE_KICK );
                        break;

                    case IDC_PAUSE_ONLINE_FRIENDS:
                        SetState( SM_PAUSE_ONLINE_FRIENDS );
                        break;

                    case IDC_PAUSE_ONLINE_PLAYERS:
                        SetState( SM_PAUSE_ONLINE_PLAYERS );
                        break;

                    case IDC_PAUSE_ONLINE_OPTIONS:
                        // init pending profile
                        g_StateMgr.InitPendingProfile( 0 );
                        SetState( SM_PAUSE_ONLINE_OPTIONS );
                        break;

                    case IDC_PAUSE_ONLINE_SETTINGS:
                        // init pending settings
                        InitPendingSettings();
                        SetState( SM_PAUSE_ONLINE_SETTINGS );
                        break;

                    case IDC_PAUSE_ONLINE_CONFIG:
                        SetState( SM_PAUSE_ONLINE_SERVER_CONFIG );
                        break;
                }
            }
            break;

            case DIALOG_STATE_DELETE:
            {
                // quit game
                g_ActiveConfig.SetExitReason( GAME_EXIT_PLAYER_QUIT );
                SetPaused( FALSE, -1 );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnline( void )
{
    // Resume Audio
    g_AudioMgr.ResumeAll();
}

//=========================================================================

void state_mgr::EnterPauseOnlineVoteMap( void )
{
    // Create vote map menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(81, DIALOG_TOP, 431, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "vote map", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdatePauseOnlineVoteMap( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // Initiated vote for map change - exit pause and rejoin game
                SetPaused( FALSE, GetActiveControllerID() );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineVoteMap( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineVoteKick( void )
{
    // Create vote kick menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(51, DIALOG_TOP, 461, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "vote kick", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdatePauseOnlineVoteKick( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // Initiated vote kick player - exit pause and rejoin game
                SetPaused( FALSE, GetActiveControllerID() );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineVoteKick( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineFriends( void )
{
    // Create friends menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "friends list", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_friends*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseOnlineFriends( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_PAUSE_ONLINE_FEEDBACK_FRIEND );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineFriends( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlinePlayers( void )
{
    // Create players menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "players list", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_players*)m_CurrentDialog)->EnableBlackout();
    ((dlg_players*)m_CurrentDialog)->Configure( PLAYER_MODE_INGAME );
}

//=========================================================================

void state_mgr::UpdatePauseOnlinePlayers( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_PAUSE_ONLINE_FEEDBACK );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlinePlayers( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineFeedback( void )
{
    // Create feedback menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "feedback", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_feedback*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseOnlineFeedback( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_PLAYERS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineFeedback( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineFeedbackFriend( void )
{
    // Create feedback menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "feedback", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );

    for( s32 i=0; i<g_MatchMgr.GetBuddyCount(); i++ )
    {
        buddy_info& Buddy = g_MatchMgr.GetBuddy(i);
        if( Buddy.Identifier == m_FeedbackIdentifier )
        {
            if( Buddy.Flags & USER_VOICE_MESSAGE )
            {
                ((dlg_feedback*)m_CurrentDialog)->ChangeConfig(dlg_feedback::ATTACHMENT_FEEDBACK);
                break;
            }
        }
    }
    ((dlg_feedback*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseOnlineFeedbackFriend( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
        case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_FRIENDS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineFeedbackFriend( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineOptions( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(s_ProfileOptLeft, DIALOG_TOP, s_ProfileOptRight, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // configure the dialog for MP options-
    ((dlg_profile_options*)m_CurrentDialog)->Configure( PROFILE_OPTIONS_PAUSE, FALSE );
}

//=========================================================================

void state_mgr::UpdatePauseOnlineOptions( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                s32 Control = m_CurrentDialog->GetControl();

                switch( Control )
                {
                    case IDC_PROFILE_OPTIONS_CONTROLS:
                    {
                        SetState( SM_PAUSE_ONLINE_CONTROLS );
                    }
                    break;

                    case IDC_PROFILE_OPTIONS_ACCEPT:
                    {
                    #if defined(TARGET_PC) || defined(TARGET_XBOX)
                        SetState( SM_PAUSE_MAIN_MENU );
                    #else
                        // need to save new profile
                        SetState( SM_PAUSE_ONLINE_MEMCARD_SELECT );
                    #endif
                    }
                    break;
                }
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                SetState( SM_PAUSE_ONLINE );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE );
            }
            break;

            case DIALOG_STATE_MEMCARD_ERROR:
            {
                // save failed - select a profile to save to
                SetState( SM_PAUSE_ONLINE_SAVE_SELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineOptions( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineControls( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile controls", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_profile_controls*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseOnlineControls( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_OPTIONS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineControls( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineSettings( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(16, DIALOG_TOP, 496, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "av settings", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_av_settings*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseOnlineSettings( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE );
            }
            break;

            case DIALOG_STATE_SELECT:
            {
                SetState( SM_PAUSE_ONLINE_HEADSET );
            }
            break;

        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineSettings( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineHeadset ( void )
{
    //  create options main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "headset", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_headset*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseOnlineHeadset ( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_SETTINGS );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineHeadset ( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineSaveSelect( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    ((dlg_profile_select*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseOnlineSaveSelect( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected profile was overwritten
                SetState( SM_PAUSE_ONLINE );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // continue without saving
                SetState( SM_PAUSE_ONLINE );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // Select a card to save the profile to
                SetState( SM_PAUSE_ONLINE_MEMCARD_RESELECT );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineSaveSelect( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineMemcardReselect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->EnableBlackout();
#endif
}

//=========================================================================

void state_mgr::UpdatePauseOnlineMemcardReselect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            SetState( SM_PAUSE_ONLINE );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_PAUSE_ONLINE_SAVE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineMemcardReselect( void )
{
}

//=========================================================================

void state_mgr::EnterPauseServerConfig( void )
{
    // Create the server config dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 86, DIALOG_TOP, 426, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "server config", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
}

//=========================================================================

void state_mgr::UpdatePauseServerConfig( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            s32 Control = m_CurrentDialog->GetControl();

            switch( Control )
            {
                case IDC_SERVER_CONFIG_CHANGE_MAP:
                    SetState( SM_PAUSE_ONLINE_CHANGE_MAP );
                    break;

                case IDC_SERVER_CONFIG_KICK_PLAYER:
                    SetState( SM_PAUSE_ONLINE_KICK_PLAYER );
                    break;

                case IDC_SERVER_CONFIG_CHANGE_TEAM:
                    SetState( SM_PAUSE_ONLINE_TEAM_CHANGE );
                    break;

                case IDC_SERVER_CONFIG_RECONFIGURE:
                    SetState( SM_PAUSE_ONLINE_RECONFIG_ONE );
                    break;

                case IDC_SERVER_CONFIG_SHUTDOWN:
                    g_ActiveConfig.SetExitReason( GAME_EXIT_PLAYER_QUIT );
                    // Shutdown the server
                    break;
                case IDC_SERVER_CONFIG_RESTART_MAP:
                    g_ActiveConfig.SetExitReason( GAME_EXIT_RELOAD_LEVEL );
                    break;
                default:
                    ASSERT( FALSE );
            }
        }

        if( DialogState == DIALOG_STATE_BACK )
        {
            SetState( SM_PAUSE_ONLINE );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseServerConfig( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineChangeMap( void )
{
    // Create change map dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(81, DIALOG_TOP, 431, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "change map", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdatePauseOnlineChangeMap( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // Change the map
                // anything to set here?
                SetState( SM_PAUSE_ONLINE_SERVER_CONFIG );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_SERVER_CONFIG );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineChangeMap( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineKickPlayer( void )
{
    // Create vote kick menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(51, DIALOG_TOP, 461, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "kick player", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdatePauseOnlineKickPlayer( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // Kicked a player
                // anything to set here?
                SetState( SM_PAUSE_ONLINE_SERVER_CONFIG );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_SERVER_CONFIG );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineKickPlayer( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineTeamChange( void )
{
    //  Create level select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "team change", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
}

//=========================================================================

void state_mgr::UpdatePauseOnlineTeamChange( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_SERVER_CONFIG );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineTeamChange( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineReconfigOne( void )
{
    // Create online host menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(12, DIALOG_TOP, 500, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "online host", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_online_host*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseOnlineReconfigOne( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                SetState( SM_PAUSE_ONLINE_RECONFIG_TWO );
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_SERVER_CONFIG );
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineReconfigOne( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineReconfigTwo( void )
{
    // Create online host menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(12, DIALOG_TOP, 500, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "host options", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_online_host_options*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseOnlineReconfigTwo( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_RECONFIG_ONE );
            }
            break;

            case DIALOG_STATE_SELECT:
            {
                SetState( SM_PAUSE_ONLINE_RECONFIG_MAP );               
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineReconfigTwo( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineReconfigMap( void )
{
    //  Create level select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "online level select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_online_level_select*)m_CurrentDialog)->EnableBlackout();
    ((dlg_online_level_select*)m_CurrentDialog)->Configure( MAP_SELECT_ONLINE_PAUSE );
}

//=========================================================================

void state_mgr::UpdatePauseOnlineReconfigMap( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_BACK:
            {
                SetState( SM_PAUSE_ONLINE_RECONFIG_TWO );
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                // return to game
                SetState( SM_END_PAUSE );
            }
            break;

#if 0 
            case DIALOG_STATE_MEMCARD_ERROR:
            {
                // save to memcard and return to menu
                SetState( SM_PAUSE_ONLINE_SAVE_SETTINGS );
            }
            break;
#endif

            case DIALOG_STATE_SELECT:
            {
                g_PendingConfig.Commit();
                g_ActiveConfig.SetExitReason( GAME_EXIT_RELOAD_LEVEL );
                // TODO: restart the game with the new settings
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineReconfigMap( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineSaveSettings( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->Configure( SM_CARDMODE_SETTINGS );
#endif
}

//=========================================================================

void state_mgr::UpdatePauseOnlineSaveSettings( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            SetState( SM_END_PAUSE );
            // save profile was successful
            // TODO: restart the game with the new settings
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // TODO: restart the game with the new settings
            SetState( SM_END_PAUSE );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineSaveSettings( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMPScore( void )
{
    // Create score dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    
    // determine which dialog to create
    const game_score& ScoreData = GameMgr.GetScore();
    
    s32 playerCount = ScoreData.NPlayers;

    if( playerCount > 16 )
    {
        irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
        m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "big leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
        m_LeaderboardID = SM_BIG_LEADERBOARD;
    }
    else
    {
        if( ScoreData.IsTeamBased ) 
        {
#ifdef TARGET_XBOX
            irect mainarea(8, DIALOG_TOP, 504, 448-36);
#else
            irect mainarea(8, 16, 504, 448-36);
#endif
            m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "team leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
            m_LeaderboardID = SM_TEAM_LEADERBOARD;
        }
        else
        {        
            irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
            m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
            m_LeaderboardID = SM_LEADERBOARD;
        }
    }
}

//=========================================================================

void state_mgr::UpdatePauseMPScore( void )
{
    const game_score& ScoreData = GameMgr.GetScore();
    
    s32 playerCount = ScoreData.NPlayers;

    // check if we need to switch the dialog
    switch( m_LeaderboardID )
    {
        case SM_BIG_LEADERBOARD:
            if( playerCount < 17 )
            {
                g_UiMgr->EndDialog( g_UiUserID, TRUE );
                if( ScoreData.IsTeamBased ) 
                {
#ifdef TARGET_XBOX
                    irect mainarea(8, DIALOG_TOP, 504, 448-36);
#else
                    irect mainarea(8, 16, 504, 448-36);
#endif
                    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "team leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
                    m_LeaderboardID = SM_TEAM_LEADERBOARD;
                }
                else
                {        
                    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
                    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
                    m_LeaderboardID = SM_LEADERBOARD;
                }
            }
            break;
        default:
            if( playerCount > 16 )
            {
                g_UiMgr->EndDialog( g_UiUserID, TRUE );
                irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
                m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "big leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
                m_LeaderboardID = SM_BIG_LEADERBOARD;
            }
            break;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                if( g_NetworkMgr.IsOnline() )
                {
                    // goto online pause main menu
                    SetState( SM_PAUSE_ONLINE );
                }
                else
                {
                    // goto multi player pause main menu
                    SetState( SM_PAUSE_MP );
                }
            }
            break;

            case DIALOG_STATE_BACK:
            {
                SetPaused( FALSE, -1 );
            }
            break;

            case DIALOG_STATE_EXIT:                
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_PLAYER_QUIT );
                SetPaused( FALSE, -1 );
            }
            break;

            default:
                break;
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMPScore( void )
{
}

//=========================================================================

void state_mgr::EnterEndPause( void )
{
    // gracefully exit from the pause menu (kill all dialogs)
    g_UiMgr->EndUsersDialogs( g_UiUserID );

    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    irect mainarea( -75, DIALOG_TOP, XRes+75, DIALOG_BOTTOM );

    //restore UI control to all controllers
    g_UiMgr->SetUserController( g_UiUserID, -1 );

    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "end pause", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
}

//=========================================================================

void state_mgr::UpdateEndPause( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            SetState( SM_PLAYING_GAME );
        }
    }
}

//=========================================================================

void state_mgr::ExitEndPause( void )
{
    // Kill the pause menu and return to the game
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    m_CurrentDialog = NULL;
    // Disable ui
    g_UiMgr->EnableUser( g_UiUserID, FALSE );

    // Resume Audio
    g_AudioMgr.ResumeAll();

    // Set unpaused
    input_SuppressFeedback( FALSE ); // SetPaused(xbool) is not always called!
    m_bIsPaused = FALSE;
    m_bDoSystemError = FALSE;
    m_PopUp = NULL;

}

//=========================================================================

void state_mgr::EnterExitGame( void )
{
    // gracefully exit from the pause menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    irect mainarea( -75, DIALOG_TOP, XRes+75, DIALOG_BOTTOM );
    g_UiMgr->EnableUser( g_UiUserID, TRUE );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "end pause", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
    m_Exit = TRUE;
    m_TimeoutTime = 0.25f;
}

//=========================================================================

void state_mgr::UpdateExitGame( void )
{
    // If any memory cards are currently processing, wait until it's done before quitting.
#if (!CONFIG_IS_DEMO)
    // If the save was successful OR user continues WITHOUT saving
    if( g_UIMemCardMgr.IsActionDone()==FALSE  )
    {
        LOG_MESSAGE( "state_mgr::UpdateExitGame", "Delaying game exit due to memcard operation." );
        return;
    }
#endif

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            SetState( SM_IDLE );
        }
    }
}

//=========================================================================

void state_mgr::ExitExitGame( void )
{
    // Kill the end pause dialog and exit the game
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    m_CurrentDialog = NULL;
    // Set unpaused
    m_bIsPaused = FALSE;
    m_Exit      = FALSE;
    g_ActiveConfig.SetExitReason( GAME_EXIT_PLAYER_QUIT );
    m_bDoSystemError = FALSE;
    m_PopUp = NULL;

    for( s32 i=0; i< SM_MAX_PLAYERS; i++ )
    {
        input_Feedback( 0.0f, 0.0f, i );
    }
}

//=========================================================================

void state_mgr::EnterPostGame( void )
{
    for( s32 i=0; i< SM_MAX_PLAYERS; i++ )
    {
        input_Feedback( 0.0f, 0.0f, i );
    }

    g_UIMemCardMgr.Clear();
    g_UiMgr->EnableUser( g_UiUserID, TRUE );
}

//=========================================================================

void state_mgr::UpdatePostGame( void )
{
    //=============================================================================
    // The ExitReason() is used to drive the starting point of the front end ui and
    // state machine. When we first enter the game loop, the GAME_EXIT_CONTINUE code is
    // set. This tells us that we've never been anywhere so far. When RunGame() exits,
    // (see below), the reason for the exit drives where we go next. The target is 
    // determined at this point.
    //
    // Note: Other networked players may still be connected to the game at this point.
    // In that case, we should only be going through the ADVANCE_LEVEL and RELOAD_LEVEL
    // exit paths. Otherwise, everyone should be disconnected.

    // handle any system errors
    if( m_bDoSystemError )
        return;

    xbool bClearControllers = FALSE;

    switch( g_ActiveConfig.GetExitReason() )
    {
        //-----------------------------------------------------
    case GAME_EXIT_ADVANCE_LEVEL:
#if CONFIG_IS_DEMO
            g_StateMgr.SetState( SM_DEMO_EXIT );
#else 
            // set the movie play flag to false!
            m_bPlayMovie = FALSE;
            g_StateMgr.SetState( SM_ADVANCE_LEVEL );
#endif
        break;

        //-----------------------------------------------------
    case GAME_EXIT_RELOAD_CHECKPOINT:
        // Tell the networkmgr we're about to re-enter the game. Then
        // tell the statemgr we're going to do that too.
        g_NetworkMgr.ReenterGame();
        SetState( SM_RELOAD_CHECKPOINT );
        break;
        //-----------------------------------------------------
    case GAME_EXIT_RELOAD_LEVEL:
        g_StateMgr.SetState( SM_ADVANCE_LEVEL );
        break;
        //-----------------------------------------------------
    case GAME_EXIT_PLAYER_DROPPED:
    case GAME_EXIT_PLAYER_KICKED:
    case GAME_EXIT_SESSION_ENDED:
    case GAME_EXIT_CONNECTION_LOST:
    case GAME_EXIT_SERVER_BUSY:
    case GAME_EXIT_SERVER_SHUTDOWN:
    case GAME_EXIT_NETWORK_DOWN:
    case GAME_EXIT_DUPLICATE_LOGIN:
        g_StateMgr.SetState( SM_REPORT_ERROR );
        bClearControllers = TRUE;        
        break;
        //-----------------------------------------------------
    case GAME_EXIT_GAME_COMPLETE:
        g_StateMgr.SetState( SM_GAME_OVER );
        bClearControllers = TRUE;        
        break;
        //-----------------------------------------------------
    case GAME_EXIT_PLAYER_QUIT:
        {
            g_RscMgr.Load( PRELOAD_FILE("DX_FrontEnd.audiopkg"    ) );
            g_RscMgr.Load( PRELOAD_FILE("SFX_FrontEnd.audiopkg"   ) );
            g_RscMgr.Load( PRELOAD_FILE("MUSIC_FrontEnd.audiopkg" ) );
            // initialize audio volumes from global settings
            global_settings& Settings = g_StateMgr.GetActiveSettings();
            Settings.CommitAudio();
            bClearControllers = TRUE;        

            EnableBackgroundMovie();

            if( g_NetworkMgr.IsOnline() )
            {
                SetState( SM_FINAL_SCORE );
            }
            else
            {
#if CONFIG_IS_DEMO
                SetState( SM_DEMO_EXIT );
#else
                if( GameMgr.GetGameType() == GAME_CAMPAIGN )
                {
                    // check if profile needs to be saved
                    SetState( SM_GAME_EXIT_PROMPT_FOR_SAVE );
                }
                else
                {
                    // split screen justs return to the split screen menu
                    SetState( SM_MULTI_PLAYER_MENU );
                }
#endif
            }
        }
        break;
        //-----------------------------------------------------
    case GAME_EXIT_BAD_PASSWORD:
    case GAME_EXIT_SERVER_FULL:
    case GAME_EXIT_CLIENT_BANNED:
    case GAME_EXIT_CANNOT_CONNECT:
    case GAME_EXIT_INVALID_MISSION:
    case GAME_EXIT_LOGIN_REFUSED:
        SetState( SM_REPORT_ERROR );
        bClearControllers = TRUE;        
        break;
        //-----------------------------------------------------
    case GAME_EXIT_FOLLOW_BUDDY:
        
        if( GameMgr.IsGameOnline()==FALSE )
        {
            m_bFollowBuddy = TRUE;
            // clear the controller requests
            // EXCEPT for the controller that was in the friends menu
            for(s32 p=0; p < SM_MAX_PLAYERS; p++)
            {
                if( p != GetActiveControllerID() )
                {
                    g_StateMgr.SetControllerRequested(p, FALSE);
                }
                // these all need to be reset - they will be reassigned.
                g_IngamePad[p].SetControllerID(-1);
            }
            game_config::Commit();
            SetState( SM_ONLINE_CONNECT );
        }
        else
        {
            StartLogin( &g_PendingConfig.GetConfig(), LOGIN_FOLLOW_FRIEND );
        }
        break;
        //-----------------------------------------------------
        // NOTE: DO NOT remove this assert. If we come out of the game with an unexpected error code,
        // then we need to handle it gracefully and understand WHY this error code appeared.
    default:
        ASSERT(FALSE);
        break;
    }

    if( bClearControllers )
    {
        // clear the locked controller ID
        SetActiveControllerID(-1);

        // clear the controller requests
        for(s32 p=0; p < SM_MAX_PLAYERS; p++)
        {
            g_StateMgr.SetControllerRequested(p, FALSE);
            g_IngamePad[p].SetControllerID(-1);
        }
    }
}

//=========================================================================

void state_mgr::ExitPostGame( void )
{
}

//=========================================================================

void state_mgr::EnterAutosaveMenu( void )
{
    input_SuppressFeedback( TRUE );

    // pause the ingame audio
    g_AudioMgr.PauseAll();

    // set up the screen bars off screen
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    irect initSize( -75, DIALOG_TOP, 512+75, DIALOG_BOTTOM );

    // Adjust the size according to the resolution.
    s32 midX = XRes>>1;
    s32 dx = midX - 256;

    // scale height of dialog
    s32 Y = initSize.b - initSize.t;
    Y = (s32)( (f32)Y * g_UiMgr->GetScaleY() );
    // position it
    initSize.t = (s32)((f32)initSize.t * g_UiMgr->GetScaleY());
    initSize.t += SAFE_ZONE;
    initSize.b = initSize.t + Y;
    initSize.Translate( dx, 0 );

    g_UiMgr->SetScreenSize(initSize);       
    g_UiMgr->EnableUser( g_UiUserID, TRUE );

    // set controller states
    //g_UiMgr->SetUserController( g_UiUserID, PausingController );
    //SetActiveControllerID( PausingController );

    // intialize the pending profile
    InitPendingProfile(0);

    // enable memcard dialogs
    m_bDisableMemcardDialogs = FALSE;

    // Create the main pause dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 106, DIALOG_TOP, 406, DIALOG_BOTTOM );

    if (x_GetLocale() != XL_LANG_ENGLISH)
    {
        mainarea.l = 71;
        mainarea.r = 441;
    }

    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "autosave", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_autosave*)m_CurrentDialog)->SetCard( m_iCard );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );

    // Pause Audio
    g_AudioMgr.PauseAll();
    // pause game
    m_bIsPaused = TRUE;
}

//=========================================================================

void state_mgr::UpdateAutosaveMenu( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // pick a profile to save to
            SetState( SM_AUTOSAVE_PROFILE_RESELECT );
        }
        else if( DialogState == DIALOG_STATE_BACK )
        {
            // continue without saving for now
            g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), TRUE );
            // update the changes in the profile
            g_StateMgr.ActivatePendingProfile();
            // return to game
            SetState( SM_END_AUTOSAVE );
        }
    }
}

//=========================================================================

void state_mgr::ExitAutosaveMenu( void )
{
}

//=========================================================================

void state_mgr::EnterAutosaveProfileReselect( void )
{
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
    // enable background filter
    ((dlg_profile_select*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdateAutosaveProfileReselect( void )
{
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            // selected profile was overwritten
            case DIALOG_STATE_SELECT: 
            {
                // return to game
                SetState( SM_END_AUTOSAVE );
            }
            break;

            // continue without saving
            case DIALOG_STATE_ACTIVATE:     
            {               
                // return to game
                SetState( SM_END_AUTOSAVE );
            }
            break;

            case DIALOG_STATE_CREATE:
            {
#ifdef TARGET_PS2
			    // Select a card to save the profile to
                SetState( SM_AUTOSAVE_MEMCARD_SELECT );
#else
                // create a new profile
#endif
            }
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitAutosaveProfileReselect( void )
{
}

#if defined(TARGET_PS2)
//=========================================================================

void state_mgr::EnterAutosaveMemcardSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    // enable background filter
    ((dlg_memcard_select*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdateAutosaveMemcardSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // return to game
            SetState( SM_END_AUTOSAVE );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_AUTOSAVE_PROFILE_RESELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitAutosaveMemcardSelect( void )
{
}

#endif // TARGET_XBOX

//=========================================================================

void state_mgr::EnterEndAutosave( void )
{
    // gracefully exit from the pause menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    irect mainarea( -75, DIALOG_TOP, XRes+75, DIALOG_BOTTOM );

    //restore UI control to all controllers
    g_UiMgr->SetUserController( g_UiUserID, -1 );

    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "end pause", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );

    input_SuppressFeedback( FALSE );
}

//=========================================================================

void state_mgr::UpdateEndAutosave( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            SetState( SM_PLAYING_GAME );
        }
    }
}

//=========================================================================

void state_mgr::ExitEndAutosave( void )
{
    // Kill the pause menu and return to the game
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    m_CurrentDialog = NULL;
    // Disable ui
    g_UiMgr->EnableUser( g_UiUserID, FALSE );

    // Resume Audio
    g_AudioMgr.ResumeAll();

    // Set unpaused
    m_bIsPaused = FALSE;
    m_bDoSystemError = FALSE;
    m_PopUp = NULL;

    // reset save in progress flag
    m_bAutosaveInProgress = FALSE;
}

//=========================================================================

void state_mgr::EnterFinalScore( void )
{
    // Create score dialog
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    
    // determine which dialog to create
    const game_score& ScoreData = GameMgr.GetScore();
    
    s32 playerCount = ScoreData.NPlayers;

    if( playerCount > 16 )
    {
        irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
        m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "big leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
        m_LeaderboardID = SM_BIG_LEADERBOARD;
        // configure for final score
        ((dlg_big_leaderboard*)m_CurrentDialog)->Configure( LEADERBOARD_FINAL );
    }
    else
    {
        if( ScoreData.IsTeamBased ) 
        {
#ifdef TARGET_XBOX
            irect mainarea(8, DIALOG_TOP, 504, 448-36);
#else
            irect mainarea(8, 16, 504, 448-36);
#endif
            m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "team leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
            m_LeaderboardID = SM_TEAM_LEADERBOARD;
            // configure for final score
            ((dlg_team_leaderboard*)m_CurrentDialog)->Configure( LEADERBOARD_FINAL );
        }
        else
        {        
            irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
            m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
            m_LeaderboardID = SM_LEADERBOARD;
            // configure for final score
            ((dlg_leaderboard*)m_CurrentDialog)->Configure( LEADERBOARD_FINAL );
        }
    }
}

//=========================================================================

void state_mgr::UpdateFinalScore( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
        case DIALOG_STATE_ACTIVE:
            break;
        case DIALOG_STATE_BACK:
        case DIALOG_STATE_SELECT:
        {
            // check if profile needs to be saved
            SetState( SM_GAME_EXIT_PROMPT_FOR_SAVE );
        }
        break;

        default:
            ASSERT(FALSE);
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitFinalScore( void )
{
}

//=========================================================================

void state_mgr::EnterAdvanceLevel( void )
{
}

//=========================================================================

void state_mgr::UpdateAdvanceLevel( void )
{
    xbool IsReloading;
    xbool IsFollowing;

    exit_reason Reason = g_ActiveConfig.GetExitReason();

    if( (Reason!=GAME_EXIT_RELOAD_LEVEL) && 
        (Reason!=GAME_EXIT_ADVANCE_LEVEL) && 
        (Reason!=GAME_EXIT_FOLLOW_BUDDY) )
    {
        g_StateMgr.SetState( SM_REPORT_ERROR );
        return;
    }

    IsReloading= (Reason == GAME_EXIT_RELOAD_LEVEL);
    IsFollowing= (Reason == GAME_EXIT_FOLLOW_BUDDY);

    //
    // We're playing campaign. This means we always go through the level list in sequence
    //
    if( g_PendingConfig.GetGameTypeID() == GAME_CAMPAIGN )
    {
        // Reset the death flag
        player::s_bPlayerDied = FALSE;

        const map_entry* pEntry;
        s32 OriginalIndex;

        OriginalIndex = m_LevelIndex;

        while( TRUE )
        {
            m_LevelIndex++;

            if( m_LevelIndex >= g_MapList.GetCount() )
            {
                m_LevelIndex = 0;
            }
            pEntry = g_MapList.GetByIndex( m_LevelIndex );
            ASSERT( pEntry );


            // If we hit a different map index, or we have gone through the list looking
            // for a new one and not found any others loaded, then we quit looking.
            xbool IsSameLevelId     = ( pEntry->GetMapID() == g_PendingConfig.GetLevelID() );
            xbool IsSameGameType    = (pEntry->GetGameType() == g_PendingConfig.GetGameTypeID() );
            xbool HasWrapped        = (m_LevelIndex == OriginalIndex );
            xbool IsPresent         = pEntry->IsAvailable();
            if( (!IsSameLevelId && IsSameGameType && IsPresent) || HasWrapped )
            {
                g_PendingConfig.SetLevelID( pEntry->GetMapID() );
                break;
            }
        }
        LOG_MESSAGE("state_mgr::UpdateAdvanceLevel","Campaign Default map cycle: GameType:%d, New Map:%s", g_PendingConfig.GetGameTypeID(), (const char*)xstring(g_PendingConfig.GetLevelName()) );
        game_config::Commit();
        g_NetworkMgr.ReenterGame();
        m_bStartSaveGame = FALSE;
        g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
        SetState( SM_SINGLE_PLAYER_LOAD_MISSION );
        return;
    }

    //
    // We're following a player in to a game
    //
    if( IsFollowing )
    {
        server_info& Config = g_PendingConfig.GetConfig();
        StartLogin( &Config, LOGIN_FOLLOW_FRIEND );
        return;
    }

    //
    // We're a client. We need to wait until the RequestingMission is complete.
    //
    if( g_NetworkMgr.IsClient() )
    {
        if( g_NetworkMgr.IsRequestingMission() == FALSE )
        {
            xbool MapPresent;

            // Check for disconnect while requesting mission.
            if( g_NetworkMgr.GetClientObject().IsConnected()== FALSE )
            {
                ASSERT( g_ActiveConfig.GetExitReason()!=GAME_EXIT_CONTINUE );
                ASSERT( g_ActiveConfig.GetExitReason()!=GAME_EXIT_ADVANCE_LEVEL );
                g_StateMgr.SetState( SM_REPORT_ERROR );
                return;
            }

            MapPresent = g_PendingConfig.SetLevelID( g_ActiveConfig.GetLevelID() );

            if( !MapPresent )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_INVALID_MISSION );
                g_StateMgr.SetState( SM_REPORT_ERROR );

            }
            else
            {
                game_config::Commit();
                g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
                SetState( SM_MULTI_PLAYER_LOAD_MISSION ); // was SM_LOAD_GAME
            }
        }
        return;
    }

    // Now we know we must be a server. This is either multiplayer mode or online
    //
    // At the moment, we're not storing the game type as an index in the server configuration structure. This
    // is because we want to display a gametype. But, what happens if we have a custom game type?
    //
    //
    // Only advance the level if we didn't try to reload a map. If we do reload, it's the UI responsibility
    // to make sure that the **Pending** configuration has been updated.
    //

    s32 OriginalIndex = m_LevelIndex;
    const map_entry* pEntry = NULL;

    // If the user has changed gametypes, don't use the map cycle list.
    if( g_PendingConfig.GetGameTypeID() != g_ActiveConfig.GetGameTypeID() )
    {
        LOG_WARNING( "state_mgr::UpdateAdvanceLevel","Disabled user defined map cycling due to game type change." );
        SetUseDefaultMapCycle( TRUE );
    }

    // Are we reloading a level?
    if( IsReloading )
    {
        // Since we're reloading, let's see if the map we desire is in the current map cycle list. If it is,
        // advance our index for the map cycle list to that point.
        if( GetUseDefaultMapCycle() )
        {
            s32 NewLevelIndex;
            NewLevelIndex = m_LevelIndex;

            while( TRUE )
            {
                NewLevelIndex++;
                if( NewLevelIndex >= g_MapList.GetCount() )
                {
                    NewLevelIndex = 0;
                }
                pEntry = g_MapList.GetByIndex( NewLevelIndex );

                if( (pEntry->GetMapID() == g_PendingConfig.GetLevelID()) &&
                    (pEntry->GetGameType() == g_PendingConfig.GetGameTypeID()) )
                {
                    LOG_MESSAGE("state_mgr::UpdateAdvanceLevel","Reload, Default map cycle: Old level index:%d, New Level Index:%d", m_LevelIndex, NewLevelIndex );
                    m_LevelIndex = NewLevelIndex;
                    break;
                }
                // If we wrapped, then we did not find it in the table. This is bad. Very, Very bad.
                // so we assert.
                ASSERT( NewLevelIndex != m_LevelIndex );
            }
        }
        else
        {
            // This is similar code to above. Except we go through the user defined map redirection table.
            OriginalIndex = m_MapCycleIdx;
            s32 NewMapIndex;

            NewMapIndex = OriginalIndex;
            while( TRUE )
            {
                if( m_MapCycle[NewMapIndex] == g_PendingConfig.GetLevelID() )
                {
                    LOG_MESSAGE("state_mgr::UpdateAdvanceLevel","Reload, user map cycle: Old cycle index:%d, New cycle Index:%d", m_MapCycleIdx, NewMapIndex );
                    m_MapCycleIdx = NewMapIndex;
                    break;
                }
                NewMapIndex++;
                if( NewMapIndex >= m_MapCycleCount )
                {
                    NewMapIndex = 0;
                }
                if( NewMapIndex == OriginalIndex )
                {
                    break;
                }
            }
        }
    }
    else
    {
        //
        // Not reloading a level. We're just doing a standard advance. 
        //
        // If we have not defined a user map list, then just go through the map table in order making sure
        // that we select a new map with the same game type. We don't want to be switching from Deathmatch
        // to campaign!
        if( GetUseDefaultMapCycle() )
        {

            while( TRUE )
            {
                m_LevelIndex++;

                if( m_LevelIndex >= g_MapList.GetCount() )
                {
                    m_LevelIndex = 0;
                }
                pEntry = g_MapList.GetByIndex( m_LevelIndex );


                // If we hit a different map index, or we have gone through the list looking
                // for a new one and not found any others loaded, then we quit looking.
                xbool IsSameLevelId     = ( pEntry->GetMapID() == g_PendingConfig.GetLevelID() );
                xbool IsSameGameType    = (pEntry->GetGameType() == g_PendingConfig.GetGameTypeID() );
                xbool HasWrapped        = (m_LevelIndex == OriginalIndex );
                xbool IsPresent         = pEntry->IsAvailable();
                if( (!IsSameLevelId && IsSameGameType && IsPresent) || HasWrapped )
                {
                    g_PendingConfig.SetLevelID( pEntry->GetMapID() );
                    break;
                }
            }
            LOG_MESSAGE("state_mgr::UpdateAdvanceLevel","Default map cycle: GameType:%d, New Map:%s", g_PendingConfig.GetGameTypeID(), (const char*)xstring(g_PendingConfig.GetLevelName()) );
        }
        else
        {
            // We've defined a user map list. Now make sure that we advance properly through it.
            IncrementMapCycle();

            g_PendingConfig.SetLevelID( GetMapCycleMapID( GetMapCycleIndex() ) );
            LOG_MESSAGE("state_mgr::UpdateAdvanceLevel","User map cycle: GameType:%d, New Map:%s", g_PendingConfig.GetGameTypeID(), (const char*)xstring(g_PendingConfig.GetLevelName()) );
        }
    }

    game_config::Commit();
    // Commit any changes to the server configuration. These need to be applied now so it is properly
    // reflected when you transition a level.
    if( g_NetworkMgr.IsServer() )
    {
        ActivatePendingSettings(FALSE);
    }
    g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
    //
    // A client will enter request map, and a server will start to load the next map. 
    // Campaign should do nothing.
    //
    g_NetworkMgr.ReenterGame();
    //
    // Start to load next level
    //
    SetState( SM_MULTI_PLAYER_LOAD_MISSION );
}

//=========================================================================

void state_mgr::ExitAdvanceLevel( void )
{
    g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
}

//=========================================================================

void state_mgr::EnterReloadCheckpoint( void )
{
    m_bStartSaveGame = FALSE;
}

//=========================================================================

void state_mgr::UpdateReloadCheckpoint( void )
{
    SetState( SM_SINGLE_PLAYER_LOAD_MISSION );
}

//=========================================================================

void state_mgr::ExitReloadCheckpoint( void )
{
}

//=========================================================================
// These states display the score table between online level loads.
//
void state_mgr::EnterMultiPlayerLoadMission( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    m_CurrentDialog = NULL;
    g_UiMgr->EnableUser( g_UiUserID, TRUE );
    ASSERT( m_pLeaderboardDialog == NULL );
    //*** MUST BE IN A THREADSAFE STATE WHILE OPENING DIALOGS ***
    ASSERT( IsBackgroundThreadRunning()==FALSE );

    //
    // Create score dialog
    //

    // determine which dialog to create
    const game_score& ScoreData = GameMgr.GetScore();

    m_Timeout = 0.25f;
    s32 playerCount = ScoreData.NPlayers;

#if defined(TARGET_XBOX)
    // The XBOX needs texture allocations to be redirected prior to level loading to tiled memory
    // otherwise the texture manager will lose the size of the bitmap every level cycling.
    extern void RedirectTextureAllocator( void );
    RedirectTextureAllocator();
#endif

    if( playerCount > 16 )
    {
#ifdef TARGET_XBOX            
        irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
#else
        irect mainarea(8, DIALOG_TOP-8, 504, DIALOG_BOTTOM );
#endif
        m_pLeaderboardDialog = g_UiMgr->OpenDialog( g_UiUserID, "big leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
        m_LeaderboardID = SM_BIG_LEADERBOARD;
        ((dlg_big_leaderboard*)m_pLeaderboardDialog)->Configure( LEADERBOARD_INTERLEVEL );
    }
    else
    {
        if( ScoreData.IsTeamBased ) 
        {
#ifdef TARGET_XBOX            
            irect mainarea(8, DIALOG_TOP, 504, 448-36);
#else
            irect mainarea(8, 16, 504, 448-36);
#endif
            m_pLeaderboardDialog = g_UiMgr->OpenDialog( g_UiUserID, "team leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
            m_LeaderboardID = SM_TEAM_LEADERBOARD;
            // configure for final score
            ((dlg_team_leaderboard*)m_pLeaderboardDialog)->Configure( LEADERBOARD_INTERLEVEL );
        }
        else
        {        
#ifdef TARGET_XBOX            
            irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM );
#else
            irect mainarea(8, DIALOG_TOP-8, 504, DIALOG_BOTTOM );
#endif
            m_pLeaderboardDialog = g_UiMgr->OpenDialog( g_UiUserID, "leaderboard", mainarea, NULL, ui_win::WF_VISIBLE );
            m_LeaderboardID = SM_LEADERBOARD;
            // configure for final score
            ((dlg_leaderboard*)m_pLeaderboardDialog)->Configure( LEADERBOARD_INTERLEVEL );
        }
    }

    // 
    // Create the level description dialog
    //
    // prepare for loading the level
    ASSERT( m_CurrentDialog == NULL );

    // Create a level description dialog
#ifdef TARGET_XBOX
    irect mainarea(8, DIALOG_TOP, 504, DIALOG_BOTTOM);
#else
    irect mainarea(8, DIALOG_TOP-8, 504, DIALOG_BOTTOM);
#endif

    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "level desc", mainarea, NULL, ui_win::WF_VISIBLE );
    ((dlg_level_desc*)m_CurrentDialog)->Configure( s_FirstMap ? LEVEL_DESC_INITIAL : LEVEL_DESC_INTERLEVEL );
    g_UiMgr->EnableBackground( FALSE );

    //
    // The description dialog does not show for now.
    //
    m_CurrentDialog->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_CurrentDialog->SetFlag( ui_win::WF_DISABLED, TRUE );
    if( s_FirstMap )
    {
        // Make the background go away for now.
        g_UiMgr->EnableBackground( TRUE );
        // goto the level description
        m_pLeaderboardDialog->SetState( DIALOG_STATE_ACTIVE );                   // Reset the pad pressed flag
        m_pLeaderboardDialog->SetFlag( ui_win::WF_VISIBLE, FALSE );                      // Make this dialog invisible
        m_pLeaderboardDialog->SetFlag( ui_win::WF_DISABLED, TRUE );

        m_CurrentDialog->SetFlag( ui_win::WF_VISIBLE, TRUE );                           // Make the description visible
        m_CurrentDialog->SetFlag( ui_win::WF_DISABLED, FALSE );
        m_bShowingScores = FALSE;
    }
    else
    {
        // goto the level description screen
        m_CurrentDialog->SetState( DIALOG_STATE_ACTIVE );                   // Reset the pad pressed flag
        m_CurrentDialog->SetFlag( ui_win::WF_VISIBLE, FALSE );                      // Make this dialog invisible
        m_CurrentDialog->SetFlag( ui_win::WF_DISABLED, TRUE );

        m_pLeaderboardDialog->SetFlag( ui_win::WF_VISIBLE, TRUE );                  // Make the leaderboard visible
        m_pLeaderboardDialog->SetFlag( ui_win::WF_DISABLED, FALSE );
        g_UiMgr->EnableBackground( FALSE );
        m_bShowingScores = TRUE;
    }
#if defined(TARGET_XBOX)
    extern void RestoreTextureAllocator( void );
    RestoreTextureAllocator();
#endif

    if( g_MatchMgr.GetState() == MATCH_SERVER_ACTIVE )
    {
        // Force an update of the server state since we are loading a new mission
        g_MatchMgr.SetState( MATCH_UPDATE_SERVER );
    }
}

//=========================================================================

void state_mgr::UpdateMultiPlayerLoadMission( void )
{
    if( m_bShowingScores==FALSE )
    {
        ASSERT( m_CurrentDialog != NULL );
        ASSERT( (m_CurrentDialog->GetFlags() & ui_win::WF_VISIBLE)  != 0 );
        ASSERT( (m_CurrentDialog->GetFlags() & ui_win::WF_DISABLED) == 0 );
        dialog_states DialogState = m_CurrentDialog->GetState();
        if( (DialogState == DIALOG_STATE_SELECT) && (!s_FirstMap) )
        {
            // goto the level description screen
            m_CurrentDialog->SetState( DIALOG_STATE_ACTIVE );                   // Reset the pad pressed flag
            m_CurrentDialog->SetFlag( ui_win::WF_VISIBLE, FALSE );                      // Make this dialog invisible
            m_CurrentDialog->SetFlag( ui_win::WF_DISABLED, TRUE );

            m_pLeaderboardDialog->SetFlag( ui_win::WF_VISIBLE, TRUE );                  // Make the leaderboard visible
            m_pLeaderboardDialog->SetFlag( ui_win::WF_DISABLED, FALSE );
            g_UiMgr->EnableBackground( FALSE );
            m_bShowingScores = TRUE;
        }
    }
    else
    {
        dialog_states DialogState = m_pLeaderboardDialog->GetState();
        if( DialogState == DIALOG_STATE_SELECT )
        {
            // Make the background go away for now.
            g_UiMgr->EnableBackground( TRUE );
            // goto the level description
            m_pLeaderboardDialog->SetState( DIALOG_STATE_ACTIVE );                   // Reset the pad pressed flag
            m_pLeaderboardDialog->SetFlag( ui_win::WF_VISIBLE, FALSE );                      // Make this dialog invisible
            m_pLeaderboardDialog->SetFlag( ui_win::WF_DISABLED, TRUE );

            m_CurrentDialog->SetFlag( ui_win::WF_VISIBLE, TRUE );                           // Make the description visible
            m_CurrentDialog->SetFlag( ui_win::WF_DISABLED, FALSE );
            m_bShowingScores = FALSE;
        }
    }

    if( IsBackgroundThreadRunning()==FALSE )
    {
        if( g_NetworkMgr.IsServer() )
        {
            server_state ServerState = g_NetworkMgr.GetServerObject().GetState();
            if( ServerState != STATE_SERVER_LOAD_MISSION )
            {
                SetState( SM_SERVER_SYNC );
            }
        }
        if( g_NetworkMgr.IsClient() )
        {
            client_state ClientState = g_NetworkMgr.GetClientObject().GetState();
            if( (ClientState != STATE_CLIENT_LOAD_MISSION) && 
                (ClientState != STATE_CLIENT_VERIFY_MISSION) &&
                (ClientState != STATE_CLIENT_REQUEST_MISSION) )
            {
                SetState( SM_CLIENT_SYNC );
            }
        }
    }
}

//=========================================================================

void state_mgr::ExitMultiPlayerLoadMission( void )
{
    if( m_bShowingScores )
    {
        dlg_mp_score* pScoreDlg = (dlg_mp_score*)g_UiMgr->GetTopmostDialog(g_UiUserID);

        // let the dialog know that we have finished loading the game
        pScoreDlg->LoadingComplete();
    }

    // end the dialog
    ASSERT( g_UiMgr->GetTopmostDialog(g_UiUserID) == m_CurrentDialog );
    g_UiMgr->EndDialog( g_UiUserID, TRUE );                                     // Close level description
    ASSERT( g_UiMgr->GetTopmostDialog(g_UiUserID) == m_pLeaderboardDialog );
    g_UiMgr->EndDialog( g_UiUserID, TRUE );                                     // Close the score dialog

    m_CurrentDialog         = NULL;
    m_pLeaderboardDialog    = NULL;
    g_UiMgr->UnloadBackground( "background1" );

#if CONFIG_IS_DEMO
    g_DemoIdleTimer.Reset();
    g_DemoIdleTimer.Start();
#endif
}

//=========================================================================
void state_mgr::EnterServerCooldown( void )
{
    game_server& Server = g_NetworkMgr.GetServerObject();
    if( Server.GetState()==STATE_SERVER_INGAME )
    {
        Server.SetState( STATE_SERVER_COOLDOWN );
    }
}

//=========================================================================
void state_mgr::UpdateServerCooldown( void )
{   
    server_state ServerState = g_NetworkMgr.GetServerObject().GetState();
    exit_reason Reason = g_ActiveConfig.GetExitReason();
    if( ServerState != STATE_SERVER_COOLDOWN )
    {
        switch( Reason )
        {
            //-----------------------------------------------------
        case GAME_EXIT_ADVANCE_LEVEL:
        case GAME_EXIT_RELOAD_LEVEL:
        case GAME_EXIT_RELOAD_CHECKPOINT:
            SetState( SM_POST_GAME );
            break;
            //-----------------------------------------------------
        case GAME_EXIT_NETWORK_DOWN:
        case GAME_EXIT_DUPLICATE_LOGIN:
        case GAME_EXIT_FOLLOW_BUDDY:
        case GAME_EXIT_PLAYER_DROPPED:
        case GAME_EXIT_PLAYER_KICKED:
        case GAME_EXIT_SESSION_ENDED:
        case GAME_EXIT_CONNECTION_LOST:
        case GAME_EXIT_SERVER_BUSY:
        case GAME_EXIT_PLAYER_QUIT:
        case GAME_EXIT_SERVER_SHUTDOWN:
        case GAME_EXIT_GAME_COMPLETE:
        case GAME_EXIT_BAD_PASSWORD:
        case GAME_EXIT_SERVER_FULL:
        case GAME_EXIT_CLIENT_BANNED:
        case GAME_EXIT_CANNOT_CONNECT:
        case GAME_EXIT_INVALID_MISSION:
        case GAME_EXIT_LOGIN_REFUSED:
            SetState( SM_SERVER_DISCONNECT );
            break;
            //-----------------------------------------------------
            // NOTE: DO NOT remove this assert. If we come out of the game with an unexpected error code,
            // then we need to handle it gracefully and understand WHY this error code appeared.
        default:
            ASSERT(FALSE);
            break;
        }
    }
}

//=========================================================================
void state_mgr::ExitServerCooldown( void )
{
}

//=========================================================================
void state_mgr::EnterClientCooldown( void )
{
    game_client& Client= g_NetworkMgr.GetClientObject();
    // If we never got off the ground and running, then there's no need to try and kill me.
    if( Client.GetState()!= STATE_CLIENT_IDLE )
    {
        Client.SetState( STATE_CLIENT_COOLDOWN );
    }
}

//=========================================================================
void state_mgr::UpdateClientCooldown( void )
{
    client_state ClientState = g_NetworkMgr.GetClientObject().GetState();
    exit_reason Reason = g_ActiveConfig.GetExitReason();

    if( ClientState != STATE_CLIENT_COOLDOWN )
    {
        switch( Reason )
        {
            //-----------------------------------------------------
        case GAME_EXIT_ADVANCE_LEVEL:
        case GAME_EXIT_RELOAD_LEVEL:
        case GAME_EXIT_RELOAD_CHECKPOINT:
            SetState( SM_POST_GAME );
            break;
            //-----------------------------------------------------
        case GAME_EXIT_NETWORK_DOWN:
        case GAME_EXIT_DUPLICATE_LOGIN:
        case GAME_EXIT_FOLLOW_BUDDY:
        case GAME_EXIT_PLAYER_DROPPED:
        case GAME_EXIT_PLAYER_KICKED:
        case GAME_EXIT_SESSION_ENDED:
        case GAME_EXIT_CONNECTION_LOST:
        case GAME_EXIT_SERVER_BUSY:
        case GAME_EXIT_PLAYER_QUIT:
        case GAME_EXIT_SERVER_SHUTDOWN:
        case GAME_EXIT_GAME_COMPLETE:
        case GAME_EXIT_BAD_PASSWORD:
        case GAME_EXIT_SERVER_FULL:
        case GAME_EXIT_CLIENT_BANNED:
        case GAME_EXIT_CANNOT_CONNECT:
        case GAME_EXIT_INVALID_MISSION:
        case GAME_EXIT_LOGIN_REFUSED:
            SetState( SM_CLIENT_DISCONNECT );
            break;
            //-----------------------------------------------------
            // NOTE: DO NOT remove this assert. If we come out of the game with an unexpected error code,
            // then we need to handle it gracefully and understand WHY this error code appeared.
        default:
            ASSERT(FALSE);
            break;
        }
    }
}

//=========================================================================
void state_mgr::ExitClientCooldown( void )
{
}


//=========================================================================
void state_mgr::EnterServerDisconnect( void )
{

    game_server& Server = g_NetworkMgr.GetServerObject();
    Server.SetState( STATE_SERVER_SHUTDOWN );
    if( g_NetworkMgr.IsOnline() )
    {
        g_MatchMgr.SetState( MATCH_UNREGISTER_SERVER );
    }
    else
    {
        g_MatchMgr.SetState( MATCH_IDLE );
    }
}

//=========================================================================
void state_mgr::UpdateServerDisconnect( void )
{
    game_server& Server = g_NetworkMgr.GetServerObject();
    if( (g_MatchMgr.GetState() == MATCH_IDLE) && (Server.GetState()==STATE_SERVER_IDLE) )
    {
        g_NetworkMgr.Disconnect();
        SetState( SM_POST_GAME );
    }
}

//=========================================================================
void state_mgr::ExitServerDisconnect( void )
{
}

//=========================================================================
void state_mgr::EnterClientDisconnect( void )
{
}

//=========================================================================
// Need to wait until disconnect is complete then we kill the client.
void state_mgr::UpdateClientDisconnect( void )
{
    client_state ClientState = g_NetworkMgr.GetClientObject().GetState();
    if( ClientState==STATE_CLIENT_IDLE )
    {
        g_NetworkMgr.Disconnect();
        SetState( SM_POST_GAME );
    }
}

//=========================================================================
void state_mgr::ExitClientDisconnect( void )
{
}


//=========================================================================
//  Background Movie Functions
//=========================================================================

void state_mgr::EnableBackgroundMovie( void )
{
#if !defined( X_EDITOR ) && (!CONFIG_IS_DEMO)
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    m_MovieSize.Set( (f32)XRes, (f32)YRes );

    m_MoviePosition.Set( 0.0f, 0.0f );
    m_bPlayMovie = Movie.Open( SelectBestClip("MenuBackground"),TRUE,TRUE );
#endif
}

//=========================================================================

void state_mgr::DisableBackgoundMovie( void )
{
#if !defined( X_EDITOR ) && (!CONFIG_IS_DEMO)
    Movie.Close();
    m_bPlayMovie = FALSE;
#endif
    // kill background music
    KillFrontEndMusic();
}

//=========================================================================

void state_mgr::PlayMovie( const char* pFilename, xbool bResident, xbool bLooped )
{
    if( !m_bPlayMovie )
    {
        // startup movie player
        Movie.Init();
    }
    else
    {
        // close current movie
        Movie.Close();
    }
    m_bPlayMovie = Movie.Open( SelectBestClip(pFilename), bResident, bLooped );
}

//=========================================================================

void state_mgr::CloseMovie( void )
{
    if( m_bPlayMovie )
    {
        // close movie and shutdown player
        m_bPlayMovie = FALSE;
        Movie.Close();
        Movie.Kill();
    }
}

//=========================================================================
//  Player Inventory Functions
//=========================================================================

void state_mgr::BackupPlayerInventory( void )
{
    ASSERT( GameMgr.GetGameType() == GAME_CAMPAIGN );

    // get the player object
    slot_id ID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
    player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot( ID );
    ASSERT( pPlayer );

    // store player settings
    inventory2& PlayerInventory = pPlayer->GetInventory2();
    m_StoredPlayerInventory = PlayerInventory;

    // store weapon ammo counts
    for( s32 i=0; i<INVEN_WEAPON_LAST; i++)
    {
        if( PlayerInventory.HasItem( (inven_item)(i+1) ) )
        {
            new_weapon* Weapon = ((actor*)(pPlayer))->GetWeaponPtr( (inven_item)(i+1) );

            if( Weapon )
            {
                Weapon->GetAmmoState( new_weapon::AMMO_PRIMARY, m_AmmoAmount[i], m_AmmoMax[i], m_AmmoPerClip[i], m_AmmoInCurrentClip[i] );
            }
        }
    }

    // store player health
    m_PlayerHealth    = ((actor*)pPlayer)->GetHealth();
    m_PlayerMaxHealth = ((actor*)pPlayer)->GetMaxHealth();

    // store player mutant abilities
    m_PlayerMutantMelee     = pPlayer->m_bMutationMeleeEnabled;
    m_PlayerMutantPrimary   = pPlayer->m_bPrimaryMutationFireEnabled;
    m_PlayerMutantSecondary = pPlayer->m_bSecondaryMutationFireEnabled;
    m_CurrentWeaponItem     = pPlayer->m_CurrentWeaponItem;
    m_PrevWeaponItem        = pPlayer->m_PrevWeaponItem;
    m_NextWeaponItem        = pPlayer->m_NextWeaponItem; 
    m_Mutagen               = pPlayer->m_Mutagen;

    // Switch to the correct weapon.
    pPlayer->SetNextWeapon2( pPlayer->m_CurrentWeaponItem, TRUE );

    // set flag
    m_bInventoryIsStored = TRUE;
}

//=========================================================================

void state_mgr::RestorePlayerInventory( void )
{
    // are we advancing levels?
    if( m_bInventoryIsStored )
    {
        // get the player object
        slot_id ID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
        player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot( ID );
        ASSERT( pPlayer );

        // create the weapon objects
        pPlayer->InitInventory();

        // restore player settings
        inventory2& PlayerInventory = pPlayer->GetInventory2();
        PlayerInventory = m_StoredPlayerInventory;

        // restore weapon ammo counts
        for( s32 i=0; i<INVEN_WEAPON_LAST; i++)
        {
            if( PlayerInventory.HasItem( (inven_item)(i+1) ) )
            {
                new_weapon* Weapon = pPlayer->GetWeaponPtr( (inven_item)(i+1) );

                if( Weapon )
                {
                    Weapon->SetAmmoState( new_weapon::AMMO_PRIMARY, m_AmmoAmount[i], m_AmmoMax[i], m_AmmoPerClip[i], m_AmmoInCurrentClip[i] );
                }
            }
        }

        // restore player health
        pPlayer->ResetHealth ( m_PlayerHealth    );
        pPlayer->SetMaxHealth( m_PlayerMaxHealth );

        // store player mutant abilities
        pPlayer->m_bMutationMeleeEnabled         = m_PlayerMutantMelee;
        pPlayer->m_bPrimaryMutationFireEnabled   = m_PlayerMutantPrimary;
        pPlayer->m_bSecondaryMutationFireEnabled = m_PlayerMutantSecondary;

        //
        // We don't want to start off with the mutation weapon
        //
        if ( m_CurrentWeaponItem == INVEN_WEAPON_MUTATION )
        {
            m_CurrentWeaponItem = INVEN_WEAPON_DESERT_EAGLE;
        }
        pPlayer->m_CurrentWeaponItem             = m_CurrentWeaponItem;

        pPlayer->m_PrevWeaponItem                = m_PrevWeaponItem;
        pPlayer->m_NextWeaponItem                = m_NextWeaponItem; 
        pPlayer->m_Mutagen                       = m_Mutagen;

        // clear flag
        m_bInventoryIsStored = FALSE;
    }
}

//=========================================================================
//  Map Cycle Functions
//=========================================================================

void state_mgr::ClearMapCycle( void )
{
    for( s32 m=0; m<MAP_CYCLE_SIZE; m++ )
    {
        m_MapCycle[m] = -1; 
    }
    m_bUseDefault   = TRUE;
    m_MapCycleCount = 0;
    m_MapCycleIdx   = 0;
}

//=========================================================================

void state_mgr::IncrementMapCycle( void )
{
    // move the map cycle index to the next map in the cycle
    if( m_MapCycleCount > 1 )
    {
        // increment index
        m_MapCycleIdx++;

        // check for wrapping
        if( m_MapCycleIdx >= m_MapCycleCount )
        {
            // wrap
            m_MapCycleIdx = 0;
        }

        // user changed the list so flag it so
        m_bUseDefault = FALSE;
    }
}

//=========================================================================

void state_mgr::DecrementMapCycle( void )
{
    // move the map cycle index to the previous map in the cycle
    if( m_MapCycleCount > 1 )
    {
        // decrement index
        m_MapCycleIdx--;

        // check for wrapping
        if( m_MapCycleIdx < 0 )
        {
            // wrap
            m_MapCycleIdx = m_MapCycleCount-1;
        }

        // user changed the list so flag it so
        m_bUseDefault = FALSE;
    }
}

//=========================================================================

void state_mgr::InsertMapInCycle( s32 MapID )
{
    ASSERTS( m_MapCycleCount < MAP_CYCLE_SIZE, "Map cycle is FULL - can not insert new map!" );

    if( m_MapCycleIdx == 0 )
    {
        // add the new map ID to the end of the map cycle
        m_MapCycle[m_MapCycleCount] = MapID;
    }
    else
    {
        // insert it after the last "logical" entry
        for( s32 m = MAP_CYCLE_SIZE-1; m>m_MapCycleIdx; m-- )
        {
            m_MapCycle[m] = m_MapCycle[m-1];
        }
        // insert the new map ID in the space we just created
        m_MapCycle[m_MapCycleIdx] = MapID;
        
        // move the idx to its new position
        m_MapCycleIdx++;
    }

    // increment the count
    m_MapCycleCount++;
}

//=========================================================================

void state_mgr::DeleteMapFromCycle( s32 Index )
{
    ASSERTS( Index < m_MapCycleCount, "Cannot delete - index is past end of map cycle!" );

    // move the list up to fill the gap
    if( m_MapCycleIdx == 0 )
    {
        for( s32 m=Index; m<MAP_CYCLE_SIZE-1; m++ )
        {
            m_MapCycle[m] = m_MapCycle[m+1];
        }
    }
    else
    {
        // determine the actual index of the map to be deleted
        s32 Count = Index;
        s32 CurrIdx = m_MapCycleIdx;

        while( Count > 0 )
        {
            // increment the index
            CurrIdx++;

            // check for wrapping
            if( CurrIdx >= m_MapCycleCount )
                CurrIdx = 0;

            // decrement count
            Count--;
        }

        // delete this entry
        for( s32 m=CurrIdx; m<MAP_CYCLE_SIZE-1; m++ )
        {
            m_MapCycle[m] = m_MapCycle[m+1];
        }

        // fix up cycle index
        if( m_MapCycleIdx >= CurrIdx )
        {
            // move it up a slot
            if( m_MapCycleIdx > 0 )
            {
                m_MapCycleIdx--;
            }
        }
    }

    // set last entry (always will be -1 )
    m_MapCycle[MAP_CYCLE_SIZE-1] = -1;

    // decrement count
    m_MapCycleCount--;
}

//=========================================================================

s32 state_mgr::GetMapCycleMapID( s32 Index )
{
    ASSERTS( Index >= 0, "Index out of range!" );
    ASSERTS( Index < MAP_CYCLE_SIZE, "Index out of range!" );

    return m_MapCycle[Index];
}

//=========================================================================
//=========================================================================

void state_mgr::EnterReportError( void )
{
    if( IsBackgroundThreadRunning() )
    {
        StopBackgroundRendering();
        return;
    }
    // reload the front-end sound effects
    g_RscMgr.Load( PRELOAD_FILE("DX_FrontEnd.audiopkg"    ) );
    g_RscMgr.Load( PRELOAD_FILE("SFX_FrontEnd.audiopkg"   ) );
    g_RscMgr.Load( PRELOAD_FILE("MUSIC_FrontEnd.audiopkg" ) );
    // initialize audio volumes from global settings
    global_settings& Settings = g_StateMgr.GetActiveSettings();
    Settings.CommitAudio();

    //
    //**NOTE** ReportError state is being used in two scenarios now. Initially,
    // it would just report a problem when we exited the game. As such, it needed
    // to do a couple of things to re-enable the user interface. However, it is
    // also used to report a login failure which will be reported prior to starting
    // up the game. So, we might introduce some problems if we try to restart the
    // movie and re-enable the user. However, they should be benign
    if( m_CurrentDialog )
    {
        g_UiMgr->EndDialog( g_UiUserID, TRUE );
        m_CurrentDialog = NULL;
    }
    else
    {
        g_UiMgr->EnableUser( g_UiUserID, TRUE );

    #ifdef USE_MOVIES
        if( !m_bPlayMovie )
        {
            Movie.Init();
            //Movie.SetLanguage(XL_LANG_ENGLISH);
            m_bPlayMovie = Movie.Open( SelectBestClip("MenuBackground"),TRUE,TRUE );
        }
    #endif
    }

    irect mainarea(126, DIALOG_TOP, 386, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "report error", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateReportError( void )
{
    // Don't allow re-entrancy
    if( m_BackgroundRendererRunning )
    {
        return;
    }
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState != DIALOG_STATE_ACTIVE )
        {
            switch( g_ActiveConfig.GetExitReason() )
            {
                //-----------------------------------------------------
            case GAME_EXIT_NETWORK_DOWN:
            case GAME_EXIT_DUPLICATE_LOGIN:
                DisconnectFromNetwork();
                SetState( SM_GAME_EXIT_PROMPT_FOR_SAVE );
                break;
                //-----------------------------------------------------
            case GAME_EXIT_PLAYER_DROPPED:
            case GAME_EXIT_PLAYER_KICKED:
            case GAME_EXIT_PLAYER_QUIT:
            case GAME_EXIT_INVALID_MISSION:
            case GAME_EXIT_SERVER_BUSY:
            case GAME_EXIT_SERVER_SHUTDOWN:
            case GAME_EXIT_CONNECTION_LOST:
                if( m_LoginFailureDestination == LOGIN_FROM_MAIN )
                {
                    // If the failure destination is the online main menu, then we need to go through
                    // prompt for save to make sure any in-game data got saved before aborting.
                    SetState( SM_GAME_EXIT_PROMPT_FOR_SAVE );
                    break;
                }
                //***** DELIBERATE FALLTHROUGH *****
            case GAME_EXIT_SESSION_ENDED:
            case GAME_EXIT_CANNOT_CONNECT:
            case GAME_EXIT_BAD_PASSWORD:
            case GAME_EXIT_SERVER_FULL:
            case GAME_EXIT_CLIENT_BANNED:
            case GAME_EXIT_LOGIN_REFUSED:
                switch( m_LoginFailureDestination )
                {
                case LOGIN_FROM_FRIENDS:        SetState( SM_ONLINE_FRIENDS_MENU );             break;
                case LOGIN_FROM_PLAYERS:        SetState( SM_ONLINE_PLAYERS_MENU );             break;
                case LOGIN_FROM_JOIN_MENU:      SetState( SM_ONLINE_JOIN_MENU );                break;
                case LOGIN_FROM_INVITE:         SetState( SM_MAIN_MENU );                       break;
                case LOGIN_FOLLOW_FRIEND:       SetState( SM_ONLINE_FRIENDS_MENU );             break;
                case LOGIN_FROM_MAIN:           SetState( SM_ONLINE_MAIN_MENU );                break;
                default:                        SetState( SM_ONLINE_MAIN_MENU ); ASSERT(FALSE); break;
                                
                }
                break;
                //-----------------------------------------------------
                // NOTE: DO NOT remove this assert. If we come out of the game with an unexpected error code,
                // then we need to handle it gracefully and understand WHY this error code appeared.
            default:
                ASSERT(FALSE);
                break;
            }
        }
    }
}

//=========================================================================

void state_mgr::ExitReportError( void )
{
}

//=========================================================================

void state_mgr::EnterGameExitPromptForSave( void )
{
    // We only try to prompt for save if we have a profile. The only time we
    // may not have a profile if the network is disconnected prior to going
    // online properly.
    if( GetProfileListIndex(0)!=-1 )
    {
        // calculate checksum for profile prior to starting game
        player_profile&     ActiveProfile  = GetActiveProfile( GetProfileListIndex(0) );

        // check for changes
        if( ActiveProfile.HasChanged() ) 
        {
            g_UiMgr->EndDialog( g_UiUserID, TRUE );

            // changes have been made - prompt to save
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );

            // configure message
            m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_PROFILE_CHANGED_TITLE" ), 
                TRUE, 
                TRUE, 
                FALSE, 
                g_StringTableMgr( "ui", "IDS_PROFILE_CHANGED_MSG" ),
                navText,
                &m_PopUpResult );
        }
        else
        {
            ASSERT( m_PopUp == NULL );
            m_PopUp = NULL;
        }
    }
}

//=========================================================================

void state_mgr::UpdateGameExitPromptForSave( void )
{
    if( m_PopUp == NULL )
    {
        global_settings& ActiveSettings = GetActiveSettings();
        if( ActiveSettings.HasChanged() )
        {
            SetState( SM_GAME_EXIT_SAVE_SETTINGS );
        }
        else
        {
            // No changes to profile - continue on
            SetState( SM_GAME_EXIT_REDIRECT );
        }
        return;
    }
    // check for user response
    if ( m_PopUpResult != DLG_POPUP_IDLE )
    {
        // save changes?
        if ( m_PopUpResult == DLG_POPUP_YES )
        {
            // save changes.  Copy the active profile to the pending profile for saving
            InitPendingProfile(0);

            // check if profile is saved            
            if( g_StateMgr.GetProfileNotSaved( g_StateMgr.GetPendingProfileIndex() ) )
            {
                // Select profile to save to.
                SetState( SM_GAME_EXIT_SAVE_SELECT );
            }
            else
            {
                // attempt to save to the preserved profile
                profile_info* pProfileInfo = &g_UIMemCardMgr.GetProfileInfo( 0 );
                m_iCard = pProfileInfo->CardID;
                g_UIMemCardMgr.SaveProfile( *pProfileInfo, 0, this, &state_mgr::OnGameExitSaveProfileCB );
                // goto idle whilst we wait for the callback
                SetState( SM_IDLE );
            }
        }
        else
        {
            global_settings& ActiveSettings = GetActiveSettings();
            if( ActiveSettings.HasChanged() )
            {
                SetState( SM_GAME_EXIT_SAVE_SETTINGS );
            }
            else
            {
                // continue without saving
                SetState( SM_GAME_EXIT_REDIRECT );
            }
        }
    }
}

//=========================================================================

void state_mgr::ExitGameExitPromptForSave( void )
{
}

//=========================================================================

void state_mgr::EnterGameExitSaveSelect( void )
{
#ifdef TARGET_XBOX
    // Xbox intercepts this keypress so it can prompt the user
    // to go to the dashboard to free up space.
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition(0);
    // open confirmation dialog
    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_DONT_FREE_BLOCKS" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_FREE_MORE_BLOCKS" );

    // calculate blocks required
    s32 BlocksRequired = ( (g_StateMgr.GetProfileSaveSize() - Condition.BytesFree) + 16383 ) / 16384;

    if( x_GetLocale() == XL_LANG_ENGLISH )
    {
        r.SetWidth(380);
        r.SetHeight(125);
    }
    else
    {
        r.SetWidth(400);
        r.SetHeight(145);
    }

    m_PopUp->Configure( r, g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ), 
                        TRUE, 
                        TRUE, 
                        FALSE, 
                        xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1_XBOX" )), BlocksRequired ) ),
                        navText,
                        &m_PopUpResult );
#else
    // Create profile menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea(91, DIALOG_TOP, 421, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "profile select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
    ((dlg_profile_select*)m_CurrentDialog)->Configure( PROFILE_SELECT_OVERWRITE );
#ifndef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif
#endif
}

//=========================================================================

void state_mgr::UpdateGameExitSaveSelect( void )
{
#ifdef TARGET_XBOX
    // handle popup result
    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( m_PopUpResult == DLG_POPUP_YES )
            {
                // continue without saving
                // update the changes in the profile
                g_StateMgr.ActivatePendingProfile();

                // continue onward
                global_settings& ActiveSettings = GetActiveSettings();
                if( ActiveSettings.HasChanged() )
                {
                    SetState( SM_GAME_EXIT_SAVE_SETTINGS );
                }
                else
                {
                    SetState( SM_GAME_EXIT_REDIRECT );
                }
            }
            else
            {
                // If the player chose to go to the Dash, go to memory area
                LD_LAUNCH_DASHBOARD LaunchDash;
                LaunchDash.dwReason = XLD_LAUNCH_DASHBOARD_MEMORY;
                // This value will be returned to the title via XGetLaunchInfo
                // in the LD_FROM_DASHBOARD struct when the Dashboard reboots
                // into the title. If not required, set to zero.
                LaunchDash.dwContext = 0;
                // Specify the logical drive letter of the region where
                // data needs to be removed; either T or U.
                LaunchDash.dwParameter1 = DWORD( 'U' );
                // Specify the number of 16-KB blocks that need to be freed
                LaunchDash.dwParameter2 = ( g_StateMgr.GetProfileSaveSize() + 16383 ) / 16384;
                // Launch the Xbox Dashboard
                XLaunchNewImage( NULL, (PLAUNCH_DATA)(&LaunchDash) );
            }
        }
    }
#else
    // Get the current dialog state
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
            case DIALOG_STATE_SELECT:
            {
                // selected profile was overwritten
                global_settings& ActiveSettings = GetActiveSettings();
                if( ActiveSettings.HasChanged() )
                {
                    SetState( SM_GAME_EXIT_SAVE_SETTINGS );
                }
                else
                {
                    SetState( SM_GAME_EXIT_REDIRECT );
                }
            }
            break;

            case DIALOG_STATE_ACTIVATE:
            {
                global_settings& ActiveSettings = GetActiveSettings();
                if( ActiveSettings.HasChanged() )
                {
                    SetState( SM_GAME_EXIT_SAVE_SETTINGS );
                }
                else
                {
                    // continue without saving
                    SetState( SM_GAME_EXIT_REDIRECT );
                }
            }
            break;

            case DIALOG_STATE_CREATE:
            {
                // Select a card to save the profile to
                SetState( SM_GAME_EXIT_MEMCARD_RESELECT );
            }
            break;
        }
    }
#endif
}

//=========================================================================

void state_mgr::ExitGameExitSaveSelect( void )
{
}

//=========================================================================

void state_mgr::EnterGameExitMemcardReselect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateGameExitMemcardReselect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            global_settings& ActiveSettings = GetActiveSettings();
            if( ActiveSettings.HasChanged() )
            {
                SetState( SM_GAME_EXIT_SAVE_SETTINGS );
            }
            else
            {
                // goto redirect
                SetState( SM_GAME_EXIT_REDIRECT );
            }
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_GAME_EXIT_SAVE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitGameExitMemcardReselect( void )
{
}

//=========================================================================

void state_mgr::EnterGameExitSaveSettings( void )
{
    g_UiMgr->EndDialog( g_UiUserID, TRUE );

    // changes have been made to settings - prompt to save
    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

    // set nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );

    // configure message
    if( g_NetworkMgr.IsServer() )
    {
        m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_GAMEMGR_POPUP" ), 
            TRUE, 
            TRUE, 
            FALSE, 
            g_StringTableMgr( "ui", "IDS_SAVE_SETTINGS_MSG" ),
            navText,
            &m_PopUpResult );
    }
    else
    {
        m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_GAMEMGR_POPUP" ), 
            TRUE, 
            TRUE, 
            FALSE, 
            g_StringTableMgr( "ui", "IDS_SETTINGS_CHANGED_MSG" ),
            navText,
            &m_PopUpResult );
    }
}

//=========================================================================

void state_mgr::UpdateGameExitSaveSettings( void )
{
    switch( m_PopUpResult )
    {
        case DLG_POPUP_YES:
        {
            // save changes
            g_AudioMgr.Play("Select_Norm");

            // check if the settings are saved 
            if( g_StateMgr.GetSettingsCardSlot() == -1 )
            {
#ifdef TARGET_PS2
                // not saved - goto memory card select screen
                SetState( SM_GAME_EXIT_SETTINGS_SELECT );
#else
                // check for corrupt settings
                if( g_UIMemCardMgr.FoundSettings() )
                {
                    SetState( SM_GAME_EXIT_SETTINGS_OVERWRITE );
                }
                else
                {
                    // attempt to create settings
                    g_StateMgr.SetSettingsCardSlot( 0 );
                    g_UIMemCardMgr.CreateSettings( this, &state_mgr::OnGameExitSaveSettingsCB );
                    // goto idle whilst we wait for the callback
                    SetState( SM_IDLE );
                }
#endif
            }
            else
            {                            
                // attempt to save the changes to the memcard
                g_UIMemCardMgr.SaveSettings( this, &state_mgr::OnGameExitSaveSettingsCB );
                // goto idle whilst we wait for the callback
                SetState( SM_IDLE );
            }
        }
        break;

        case DLG_POPUP_NO:
        {
            // continue without saving
            // Activate the pending settings right now. Even though the player opted not to
            // save, the settings should be preserved locally.
            g_StateMgr.ActivatePendingSettings();
            // goto redirect
            SetState( SM_GAME_EXIT_REDIRECT );
        }
        break;
    }
}

//=========================================================================

void state_mgr::ExitGameExitSaveSettings( void )
{
}

//=========================================================================

void state_mgr::EnterGameExitSettingsOverwrite( void )
{
    // ask permission to overwrite old settings
    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

    // set nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );

    // configure message
    m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_GAMEMGR_POPUP" ), 
        TRUE, 
        TRUE, 
        FALSE, 
        g_StringTableMgr( "ui", "IDS_SETTINGS_CHANGED_OVERWRITE" ),
        navText,
        &m_PopUpResult );
}

//=========================================================================

void state_mgr::UpdateGameExitSettingsOverwrite( void )
{
    switch( m_PopUpResult )
    {
        case DLG_POPUP_YES:
        {
            // overwrite old settings file
            g_AudioMgr.Play("Select_Norm");
            // set settings card slot
            g_StateMgr.SetSettingsCardSlot( 0 );
            // attempt to save the changes to the memcard
            g_UIMemCardMgr.SaveSettings( this, &state_mgr::OnGameExitSaveSettingsCB );
            // goto idle whilst we wait for the callback
            SetState( SM_IDLE );
        }
        break;

        case DLG_POPUP_NO:
        {
            // continue without saving
            // Activate the pending settings right now. Even though the player opted not to
            // save, the settings should be preserved locally.
            g_StateMgr.ActivatePendingSettings();
            // goto redirect
            SetState( SM_GAME_EXIT_REDIRECT );
        }
        break;
    }
}

//=========================================================================

void state_mgr::ExitGameExitSettingsOverwrite( void )
{
}

//=========================================================================

void state_mgr::EnterGameExitSettingsSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
#ifdef TARGET_PS2
    ((dlg_memcard_select*)m_CurrentDialog)->Configure( SM_CARDMODE_SETTINGS );
#endif
}

//=========================================================================

void state_mgr::UpdateGameExitSettingsSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save settings was successful
            SetState( SM_GAME_EXIT_REDIRECT );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // save failed or was aborted
            SetState( SM_GAME_EXIT_REDIRECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitGameExitSettingsSelect( void )
{
}

//=========================================================================

void state_mgr::EnterGameExitRedirect( void )
{
    // kill any old dialogs
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
}

//=========================================================================

void state_mgr::UpdateGameExitRedirect( void )
{
    // redirect the flow based on the exit reason
    switch( g_ActiveConfig.GetExitReason() )
    {
    case GAME_EXIT_PLAYER_QUIT:
    case GAME_EXIT_GAME_COMPLETE:
        if( g_NetworkMgr.IsOnline() )
        {
            SetState( SM_ONLINE_MAIN_MENU );
        }
        else
        {
            SetState( SM_CAMPAIGN_MENU );
        }
        break;

    case GAME_EXIT_NETWORK_DOWN:
    case GAME_EXIT_DUPLICATE_LOGIN:
        // This will only happen if the cable is pulled while quitting
        // a game.
        if( g_NetworkMgr.IsOnline() )
        {
            SetState( SM_REPORT_ERROR );
            return;
        }
        else
        {
            SetState( SM_MAIN_MENU );
        }
        break;

    case GAME_EXIT_PLAYER_DROPPED:
    case GAME_EXIT_PLAYER_KICKED:
    case GAME_EXIT_SESSION_ENDED:
    case GAME_EXIT_CONNECTION_LOST:
    case GAME_EXIT_SERVER_BUSY:
    case GAME_EXIT_SERVER_SHUTDOWN:
    case GAME_EXIT_INVALID_MISSION:
        SetState( SM_ONLINE_MAIN_MENU );
        break;

    default:
        ASSERTS( FALSE, "Unexpected exit reason in redirect!" );
        break;
    }
    g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
}

//=========================================================================

void state_mgr::ExitGameExitRedirect( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineDownload( void )
{
    // Create online host menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );

    irect mainarea(26, DIALOG_TOP, 486, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "download", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateOnlineDownload( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        switch( DialogState )
        {
        case DIALOG_STATE_ACTIVE:
            break;

        case DIALOG_STATE_BACK:
            SetState( SM_ONLINE_MAIN_MENU );
            break;

        default:
            ASSERT( FALSE );
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineDownload( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineLogin( void )
{
    // Create online host menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );

    irect mainarea(26, DIALOG_TOP, 286, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "login", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateOnlineLogin( void )
{
    s32     LevelID;

    if( CheckForDisconnect() )
    {
        return;
    }
    if( m_CurrentDialog != NULL )
    {
        dialog_states DialogState = (dialog_states)m_CurrentDialog->GetState();
        switch( DialogState )
        {
            // ------------------------------------------------
            // Login continuing
        case DIALOG_STATE_ACTIVE:
            break;
            // ------------------------------------------------
            // Login succeeded
        case DIALOG_STATE_SELECT:
            LevelID = g_ActiveConfig.GetLevelID();
            ASSERT( LevelID );
            g_PendingConfig.SetLevelID( LevelID );
            game_config::Commit();
            SetState( SM_START_GAME );
            break;

            // ------------------------------------------------
            // The child dialog sets the BACK state if any sort of error occurs. 
            // The ReportError dialogs can deal with all the explicit messages.
        case DIALOG_STATE_BACK:
            ASSERT( g_ActiveConfig.GetExitReason() != GAME_EXIT_CONTINUE );
            // Although this should be SM_REPORT_ERROR, SM_POST_GAME
            // performs some cleanup of game objects that still needs
            // to be performed when we drop out of the game early.
            SetState( SM_POST_GAME );
            break;
            // ------------------------------------------------
        case DIALOG_STATE_CANCEL:
            // 'Cancel' (or back) was pressed
            switch( m_LoginFailureDestination )
            {
            case LOGIN_FROM_FRIENDS:                SetState( SM_ONLINE_FRIENDS_MENU );         break;
            case LOGIN_FROM_JOIN_MENU:              SetState( SM_ONLINE_JOIN_MENU );            break;
            case LOGIN_FROM_INVITE:                 SetState( SM_ONLINE_MAIN_MENU );            break;
            case LOGIN_FROM_PLAYERS:                SetState( SM_ONLINE_JOIN_MENU );            break;
            case LOGIN_FOLLOW_FRIEND:               SetState( SM_ONLINE_FRIENDS_MENU );         break;
            default:                                ASSERT( FALSE );                            break;
            }
            break;
        default:
            ASSERT(FALSE);
            break;
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineLogin( void )
{
    if( g_UiMgr->GetTopmostDialog( g_UiUserID ) == m_CurrentDialog )
    {
        g_UiMgr->EndDialog( g_UiUserID, TRUE );
    }
    m_CurrentDialog = NULL;
}

//=========================================================================

void state_mgr::StartLogin( const server_info* pInfo, login_source LoginSource )
{
    m_LoginFailureDestination = LoginSource;

    server_info& Config = g_PendingConfig.GetConfig();
    
    Config = *pInfo;

    // clear server name if we don't have the data yet.
    if( (LoginSource != LOGIN_FROM_PLAYERS) && (LoginSource != LOGIN_FROM_JOIN_MENU) )
    {
        Config.Name[0] = '\0';
    }

    SetState( SM_ONLINE_LOGIN );
}

//=========================================================================
void state_mgr::EnterGameOver( void )
{
}

//=========================================================================
void state_mgr::UpdateGameOver( void )
{
#ifdef USE_MOVIES
    // play the outro movie
    if( !PlaySimpleMovie( SelectBestClip( "CinemaOutro" )) )
    {
        DummyScreen( "End Credits Here (just don't make them lame)", TRUE, 5 );
    }
#endif

    player_profile& ActiveProfile = GetActiveProfile( GetProfileListIndex(0) );

    // Set that player has finished the game. ( used to open up extra movie )
    ActiveProfile.m_bGameFinished = TRUE;

    // check that the difficulty level was unchanged for the entire campaign
    if( !ActiveProfile.m_bDifficultyChanged )
    {
        switch( ActiveProfile.GetDifficultyLevel() )
        {
            case DIFFICULTY_MEDIUM:
                // unlock difficulty hard for completing the game on medium
                ActiveProfile.m_bHardUnlocked = TRUE;
                break;

            case DIFFICULTY_HARD:   
                // unlock alien avatars for completing the game on hard
                ActiveProfile.m_bAlienAvatarsOn = TRUE;
                break;

            default:
                // you get nothing, nada, zip!
                break;
        }
    }

    // save changes to profile
    SilentSaveProfile();

    // load front end audio
    g_RscMgr.Load( PRELOAD_FILE("DX_FrontEnd.audiopkg"    ) );
    g_RscMgr.Load( PRELOAD_FILE("SFX_FrontEnd.audiopkg"   ) );
    g_RscMgr.Load( PRELOAD_FILE("MUSIC_FrontEnd.audiopkg" ) );
    // initialize audio volumes from global settings
    global_settings& Settings = g_StateMgr.GetActiveSettings();
    Settings.CommitAudio();
    EnableBackgroundMovie();

    SetState( SM_CAMPAIGN_MENU );
}

//=========================================================================
void state_mgr::ExitGameOver( void )
{
}

//=========================================================================

xbool state_mgr::CheckForDisconnect( void )
{
    // the memcard manager is doing something with the memcard  
    if( !g_UIMemCardMgr.IsActionDone() || m_bDoSystemError )
    {
        // if this is the case, don't check for a disconnect to force dialog 
        // shutdown or you'll hose everything.
        return FALSE;
    }



    // Ok, we noticed a disconnect occurred, let's just close every open
    // dialog and then kick them back to the main menu, after, of course,
    // telling them what just happened.
    exit_reason Reason = g_ActiveConfig.GetExitReason();
    if( (Reason == GAME_EXIT_NETWORK_DOWN) || (Reason==GAME_EXIT_DUPLICATE_LOGIN) )
    {
        g_UiMgr->EndUsersDialogs( g_UiUserID );
        SetState( SM_REPORT_ERROR );
        return TRUE;
    }
    return FALSE;
}

//=========================================================================
static volatile xbool s_BackgroundRendererStop;
static volatile xbool s_BackgroundRendererDone;

//=========================================================================
static void s_BackgroundRenderer( void )
{
    #ifdef TARGET_XBOX
    extern void xbox_DisableRumble();
                xbox_DisableRumble();
    #endif

    ASSERT( !s_BackgroundRendererDone );
    ASSERT( !s_BackgroundRendererStop );
    {
        xthread* pThread = x_GetCurrentThread();
        xtimer DeltaTime;
        f32    Delta;

        ASSERT( pThread );
        DeltaTime.Start();

        while( pThread->IsActive() && (!s_BackgroundRendererStop) )
        {
            Delta = DeltaTime.TripSec();
            // If we take longer than 200ms to do this loop, this is most likely
            // because we were single stepping so lets compensate for that.
            if( Delta > 0.2f )
            {
                Delta = 1.0f/30.0f;
            }
            // The order in which this is performed is quite important. Since this thread will typically be started
            // within the state_mgr Update() call, then, to preserve the order outlined at higher levels of the logic,
            // we need to do the Render, then the pageflip (maybe in different order for xbox) then we do the updates
            // as this may cause us to have a double update when the thread is first started as well as a render without
            // update when the thread is killed.
        #ifndef TARGET_PC
            g_StateMgr.Render();
            eng_PageFlip();
        #endif
            g_NetworkMgr.Update( Delta );

            // Update input mgr
            g_InputMgr.Update( Delta ); //0.0167f );

            // Update UI
            g_StateMgr.Update( Delta );
            //g_UiMgr->Update( Delta );

        #if defined(X_LOGGING)
            // This is a hack so that the lower priority thread gets a chance to run if logging is enabled since the
            // higher priority thread may take too much processing time when flushing a log.
            x_DelayThread(2);
        #endif
        }
    }
    s_BackgroundRendererDone = TRUE;

    #ifdef TARGET_XBOX
    extern void xbox_EnableRumble();
                xbox_EnableRumble();
    #endif
}

//=========================================================================
void state_mgr::StartBackgroundRendering( void )
{
#ifdef TARGET_XBOX
    // The rumble has to be turned off because it CANNOT
    // be called during another thread.
    for( s32 i=0;i<4;i++ )
    {
        input_EnableFeedback( FALSE,i );
    }
#endif

    s_BackgroundRendererStop = FALSE;
    s_BackgroundRendererDone = FALSE;
    {
        ASSERT( m_BackgroundRendererRunning == FALSE );
        ASSERT( m_pBackgroundRenderer == NULL );
        m_BackgroundRendererRunning = TRUE;
#ifdef TARGET_PS2
        DLIST.SetThreadID( -1 );
#endif

#if defined(TARGET_XBOX)
        m_pBackgroundRenderer = new xthread( s_BackgroundRenderer, "State Manager Background Renderer", 65536, 3 ); 
#else
        m_pBackgroundRenderer = new xthread( s_BackgroundRenderer, "State Manager Background Renderer", 16384, 3 ); 
#endif
        LOG_MESSAGE( "state_mgr::StartBackgroundRendering", "Background renderer started." );
        ASSERT( m_pBackgroundRenderer );

        // let scratchmem know that we are flipping in a thread other than the
        // main thread
    #ifdef TARGET_PS2
        DLIST.SetThreadID( m_pBackgroundRenderer->GetId() );
    #endif
        smem_SetThreadId( m_pBackgroundRenderer->GetId() );
    #if defined(USE_MOVIES)
        if( m_bPlayMovie && (Movie.CachingComplete()==FALSE) )
        {
            LOG_WARNING( "state_mgr::StartBackgroundRendering", "Background rendering waiting until movie has completed caching." );
            while( Movie.CachingComplete() == FALSE )
            {
                x_DelayThread( 32 );
            }
            LOG_WARNING( "state_mgr::StartBackgroundRendering", "Movie caching complete." );
        }
    #endif
    }
}

//=========================================================================
void state_mgr::StopBackgroundRendering( void )
{
    //** NOTE: The destructor will deal with trying to shutdown the thread gracefully.
    ASSERT( m_pBackgroundRenderer );
    ASSERT( m_BackgroundRendererRunning );
    xtimer t;

    t.Start();
    delete m_pBackgroundRenderer;
    m_pBackgroundRenderer = NULL;
    m_BackgroundRendererRunning = FALSE;
    t.Stop();

    LOG_MESSAGE( "state_mgr::StopBackgroundRendering", "Background renderer stopped in %2.02fms.", t.ReadMs() );

    // let scratchmem know that we are done flipping it on a separate thread
    smem_SetThreadId( -1 );

    //** NOTE: We cannot return until renderer has finished
#ifdef TARGET_PS2
    extern s32 x_GetMainThreadID    (void);
    DLIST.SetThreadID( x_GetMainThreadID() );
    DLIST.WaitForTasks();
#endif
#ifdef TARGET_XBOX
    // The rumble has to be turned off because it CANNOT
    // be called during another thread.
    for( s32 i=0;i<4;i++ )
    {
        input_EnableFeedback( TRUE,i );
    }
#endif
}

//=========================================================================
xbool state_mgr::IsBackgroundThreadRunning( void )
{
    return m_BackgroundRendererRunning;
}

//=========================================================================
void state_mgr::SilentSaveProfileReturn( void )
{
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_iCard );
#else
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );
#endif
    // If the save was successful OR user continues WITHOUT saving
    if( Condition.SuccessCode )
    {
        // continue without saving?
        if( Condition.bCancelled )
        {
            // flag the profile as not saved
            g_StateMgr.SetProfileNotSaved( 0, TRUE ); 
        }
        else
        {
            // update profile checksum
            player_profile& ActiveProfile = GetActiveProfile( 0 );
            ActiveProfile.Checksum();
        }

        // reset save in progress flag
        m_bAutosaveInProgress = FALSE;
        // enable memcard dialogs
        m_bDisableMemcardDialogs = FALSE;
    }
    else
    {
        // defer menu if a controller is pulled.
        m_bRetryAutoSaveMenu = m_bDoSystemError;
        if( m_bDoSystemError )
        {
            return;
        }

        // goto autosave menu
        SetState( SM_AUTOSAVE_MENU );
    }
}

//=========================================================================
// Make sure we package everything up that's needed for a save, then silently
// save the player profile.
void state_mgr::SilentSaveProfile( void )
{
    // if we've detected a pulled controller, defer the save until it's back in.
    // state_mgr::Update() will detect when we exit the controller error, and will
    // call this again, and cause this to reset.
    m_bRetrySilentAutoSave = m_bDoSystemError;
    if( m_bDoSystemError )
    {
        return;
    }

    // First, copy the checkpoint data from the checkpoint manager to the active
    // player profile.
    s32                     i;
    level_check_points*     pOldestCheckpoint;

    player_profile&         ActiveProfile = GetActiveProfile( GetProfileListIndex(0) );

    pOldestCheckpoint = &ActiveProfile.GetCheckpoint(0);

    for( i=0; i<MAX_SAVED_LEVELS;i++ )
    {
        level_check_points& ThisCheckpoint = ActiveProfile.GetCheckpoint(i);

        // Did we find one the same as the current map id? If so, bail out.
        if( (ThisCheckpoint.MapID == g_CheckPointMgr.m_Level.MapID) || (ThisCheckpoint.MapID == -1) )
        {
            pOldestCheckpoint = &ThisCheckpoint;
            break;
        }
        // If the mapid on the checkpoint is less than the current oldest, then we can
        // overwrite that one.
        if( ThisCheckpoint.MapID < pOldestCheckpoint->MapID )
        {
            pOldestCheckpoint = &ThisCheckpoint;
        }
    }

    ASSERTS( i<MAX_SAVED_LEVELS, "Max saved levels reached" );

    // This can only happen if we have, for some reason, tried to save a checkpoint that is for a level we
    // have not yet played but it is older than all those in the saved checkpoint list.
    ASSERT( pOldestCheckpoint->MapID <= g_CheckPointMgr.m_Level.MapID );

    // Store it and then start to save the profile
    *pOldestCheckpoint = g_CheckPointMgr.m_Level;

    // attempt to save the changes to the memcard
    if( m_bAutosaveProfile)
    {
        // set save in progress flag
        m_bAutosaveInProgress = TRUE;

        profile_info* pProfileInfo=NULL;
        // This is the last saved profile information stored in the
        // memory card subsystem.
        pProfileInfo = &g_UIMemCardMgr.GetProfileInfo( 0 );
        m_iCard = pProfileInfo->CardID;

        LOG_MESSAGE( "state_mgr::SilentSaveProfile", "Profile '%s' save initiated", (const char*)xstring(pProfileInfo->Name) );
        // We need to copy the active profile to the pending profile. This is what will
        // be saved to memory card.

        x_memcpy( &m_PendingProfile, &ActiveProfile, sizeof(ActiveProfile) );

        if( GetProfileNotSaved( 0 ) )
        {
            // goto autosave menu
            SetState( SM_AUTOSAVE_MENU );
        }
        else
        {
            // disable memcard dialogs
            m_bDisableMemcardDialogs = TRUE;
            // attempt to save the changes to the memcard
            g_UIMemCardMgr.SaveProfile( *pProfileInfo, 0, this, &state_mgr::SilentSaveProfileReturn );
        }
    }
    else
    {
        LOG_WARNING( "state_mgr::SilentSaveProfile", "Profile NOT saved (autosave was disabled)" );
    }
}

//=========================================================================
void state_mgr::DisconnectFromNetwork( void )
{
#if defined(TARGET_XBOX)
    {
        g_NetworkMgr.SetOnline( FALSE );

        // If there was a pending accepted game invite before the network went down,
        // then we must clear this flag to stop the game auto connecting the next
        // time the player logs in to Live.
        m_bFollowBuddy = FALSE;
    }
#else

    if( g_NetworkMgr.IsOnline() )
    {
        dlg_popup* pPopUp;
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
        pPopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|ui_win::WF_INPUTMODAL );

        r = irect(0,0,300,160);

        pPopUp->Configure( r,
            1000.0f,
            g_StringTableMgr( "ui", "IDS_NETWORK_POPUP"     ),
            g_StringTableMgr( "ui", "IDS_ONLINE_DISCONNECTING"),
            NULL );

#if defined(TARGET_PS2)
        KillFrontEndMusic();
#endif
        StartBackgroundRendering();
        g_NetworkMgr.SetOnline( FALSE );
        StopBackgroundRendering();
#if defined(TARGET_PS2)
        InitFrontEndMusic();
#endif

        g_UiMgr->EndDialog( g_UiUserID, TRUE );
    }
#endif

}

//=========================================================================
void state_mgr::Reboot( reboot_reason Reason )
{
    if( IsBackgroundThreadRunning() )
    {
        StopBackgroundRendering();
    }
    g_AudioMgr.Kill();
    g_LevelLoader.UnmountDefaultFilesystems();
    g_MemcardHardware.Kill();
    g_IoMgr.Kill();
    input_Kill();
    eng_Reboot( Reason );
}

///////////////////////////////////////////////////////////////////////////
//
//  PC/Xbox exclusions
//
///////////////////////////////////////////////////////////////////////////


#if defined(TARGET_PS2)

//=========================================================================

void state_mgr::EnterMemcardSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateMemcardSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the campaign menu
            SetState( SM_CAMPAIGN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_PROFILE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitMemcardSelect( void )
{
}

//=========================================================================

void state_mgr::EnterCampaignMemcardSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateCampaignMemcardSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the campaign menu
            SetState( SM_CAMPAIGN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_PROFILE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitCampaignMemcardSelect( void )
{
}

//=========================================================================

void state_mgr::EnterMemcardSelectMP( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateMemcardSelectMP( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            SetState( SM_MULTI_PLAYER_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            SetState( SM_MULTI_PLAYER_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitMemcardSelectMP( void )
{
}

//=========================================================================

void state_mgr::EnterOnlineMemcardSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
}

//=========================================================================

void state_mgr::UpdateOnlineMemcardSelect( void )
{
    if( CheckForDisconnect() )
    {
        return;
    }

    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
#ifdef TARGET_XBOX
            SetState( SM_ONLINE_AUTHENTICATE );
#else
            // save profile was successful
            if( GetActiveProfile(0).m_bAgeVerified )
            {
                // authenticate this user account
                SetState( SM_ONLINE_AUTHENTICATE );
            }
            else
            {
                // verify age of profile user
                SetState( SM_ONLINE_COPA );
            }
#endif
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            // go back to the profile select menu
            SetState( SM_ONLINE_PROFILE_SELECT );
        }
    }
}

//=========================================================================

void state_mgr::ExitOnlineMemcardSelect( void )
{
}

//=========================================================================

void state_mgr::EnterPauseMemcardSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );

    // enable background filter
    ((dlg_memcard_select*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================

void state_mgr::UpdatePauseMemcardSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the pause main menu
            SetState( SM_PAUSE_MAIN_MENU );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            SetState( SM_PAUSE_MAIN_MENU );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseMemcardSelect( void )
{
}

//=========================================================================

void state_mgr::EnterPauseOnlineMemcardSelect( void )
{
    //  Create memcard select screen 
    g_UiMgr->EndDialog( g_UiUserID, TRUE );
    irect mainarea( 46, DIALOG_TOP, 466, DIALOG_BOTTOM );
    m_CurrentDialog = g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );

    // enable background filter
    ((dlg_memcard_select*)m_CurrentDialog)->EnableBlackout();
}

//=========================================================================
void state_mgr::UpdatePauseOnlineMemcardSelect( void )
{
    if( m_CurrentDialog != NULL )
    {
        u32 DialogState = m_CurrentDialog->GetState();

        if( DialogState == DIALOG_STATE_SELECT )
        {
            // save profile was successful
            // goto the pause main menu
            SetState( SM_PAUSE_ONLINE );
        }
        if( DialogState == DIALOG_STATE_BACK )
        {
            // the profile save failed or was aborted
            SetState( SM_PAUSE_ONLINE );
        }
    }
}

//=========================================================================

void state_mgr::ExitPauseOnlineMemcardSelect( void )
{
}

#endif

//=========================================================================

void state_mgr::SetActiveControllerID( s32 ID )
{
    m_ActiveControllerID = ID;
#if !defined(TARGET_PC)
    g_VoiceMgr.GetHeadset().SetActiveHeadset( ID );
#endif
}

//=========================================================================

void state_mgr::EnterFollowBuddy( void )
{
    // Create main menu
    g_UiMgr->EndDialog( g_UiUserID, TRUE );

#ifdef USE_MOVIES
    // initialize movie player
    if( !m_bPlayMovie )
    {
        Movie.Init();
        //Movie.SetLanguage(XL_LANG_ENGLISH);
        m_bPlayMovie = Movie.Open( SelectBestClip("MenuBackground"),TRUE,TRUE );
    }
#else
    // Load the background
    g_UiMgr->LoadBackground ( "background1", "A51Background.xbmp" );
#endif

#ifdef USE_MOVIES
    g_UiMgr->SetUserBackground( g_UiUserID, "" );
#else
    g_UiMgr->SetUserBackground( g_UiUserID, "background1" );
#endif

    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

    // set nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );

    xwstring Title( g_StringTableMgr( "ui", "IDS_MAIN_MENU_INVITE_TITLE" ) );
    Title += m_FeedbackName;
    // configure message
    r.Set(0,0,400,200);
    m_PopUp->Configure( r, Title, 
        TRUE, 
        TRUE, 
        FALSE, 
        g_StringTableMgr( "ui", "IDS_ONLINE_MENU_PENDING_INVITE" ),
        navText,
        &m_PopUpResult );
}

//=========================================================================

void state_mgr::UpdateFollowBuddy( void )
{
    if ( m_PopUpResult != DLG_POPUP_IDLE )
    {
        m_PopUp = NULL;
        if ( m_PopUpResult == DLG_POPUP_YES )
        {
            StartLogin( &g_PendingConfig.GetConfig(), LOGIN_FROM_INVITE );
        }
        else
        {
            m_bFollowBuddy = FALSE;
            // Cancel invite and return to online main menu
            SetState( SM_ONLINE_MAIN_MENU );
        }
    }
}

//=========================================================================

const char* state_mgr::GetFeedbackPlayer( u64& Identifier )
{ 
    Identifier = m_FeedbackIdentifier;
    return m_FeedbackName; 
}

//=========================================================================

void state_mgr::SetFeedbackPlayer( const char* pName, u64 Identifier )
{
    x_strcpy( m_FeedbackName, pName );
    m_FeedbackIdentifier = Identifier;
}

//=========================================================================

void KillFrontEndMusic( void )
{
    if( g_AudioMgr.IsValidVoiceId( s_VoiceID ) )
    {
        g_AudioMgr.Release( s_VoiceID, 0.0f );
        s_VoiceID = -1;
    }
}

//=========================================================================

void InitFrontEndMusic( void )
{
    s_VoiceID = g_AudioMgr.Play( "MUSIC_MenuBackground" );
}
