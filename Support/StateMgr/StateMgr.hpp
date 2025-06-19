//==============================================================================
//  
//  StateMgr.hpp
//  
//==============================================================================

#ifndef STATEMGR_HPP
#define STATEMGR_HPP

//==============================================================================
//  Notes
//==============================================================================
//
// Adding a new state
// ==================
//
// 1. Add the state to the sm_states enum below
// 2. Add an entry in the GetStateName function for debugging 
// 3. Add Enter, Update and Exit functions for the state to the class
// 4. Add a call to each of the new state functions from the EnterState,
//    UpdateState and ExitState functions.
// 5. Fill out the body of the new functions as required
//
//==============================================================================

//==============================================================================
//  Includes
//==============================================================================
#include "Entropy.hpp"
#include "ui/ui_dialog.hpp"
#include "dialogs/dlg_PopUp.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Configuration/GameConfig.hpp"
#include "Inventory\Inventory2.hpp"
#include "CheckPointMgr/CheckpointMgr.hpp"
#include "LoreList.hpp"
#include "StateMgr/PlayerProfile.hpp"

//==============================================================================
//  Defines
//==============================================================================

//#define LAN_PARTY_BUILD  1
//#define OPM_REVIEW_BUILD  1

enum sm_states
{
    SM_IDLE = 0,

    SM_ESRB_NOTICE,
    SM_INEVITABLE_INTRO,
#if defined(TARGET_PS2)
    SM_AUTOSAVE_DIALOG,
    SM_CONTROLLER_CHECK,
#endif
    SM_MEMCARD_BOOT_CHECK,

    SM_PRESS_START_SCREEN,
    
    SM_MAIN_MENU,
    SM_SETTINGS_MENU,
    SM_SETTINGS_HEADSET,
    SM_SETTINGS_MEMCARD_SELECT,
    SM_MANAGE_PROFILES,
    SM_MANAGE_PROFILE_OPTIONS,
    SM_MANAGE_PROFILE_CONTROLS,
    SM_MANAGE_PROFILE_AVATAR,
#if defined(TARGET_PS2)
    SM_MANAGE_MEMCARD_SELECT,
#endif
    SM_MANAGE_PROFILE_SAVE_SELECT,
    SM_MANAGE_PROFILE_MEMCARD_RESELECT,
    SM_DEMO_EXIT,
#if defined(TARGET_PS2)
    SM_MEMCARD_SELECT,
#endif
    SM_CAMPAIGN_MENU,
    SM_CAMPAIGN_PROFILE_OPTIONS,
    SM_CAMPAIGN_PROFILE_CONTROLS,
    SM_CAMPAIGN_PROFILE_AVATAR,
#if defined(TARGET_PS2)
    SM_CAMPAIGN_MEMCARD_SELECT,
#endif
    SM_CAMPAIGN_PROFILE_SAVE_SELECT,
    SM_CAMPAIGN_MEMCARD_RESELECT,
    SM_LOAD_CAMPAIGN,
    SM_SAVE_CAMPAIGN,
    SM_RESUME_CAMPAIGN,
    
    SM_LORE_MAIN_MENU,
    SM_SECRETS_MENU,
    SM_EXTRAS_MENU,
    SM_CREDITS_SCREEN,

    SM_MULTI_PLAYER_MENU,
    SM_MULTI_PLAYER_OPTIONS,
    SM_MULTI_PLAYER_EDIT,
    SM_MP_LEVEL_SELECT,
    SM_MP_SAVE_SETTINGS,
    SM_PROFILE_CONTROLS_MP,
    SM_PROFILE_AVATAR_MP,
#if defined(TARGET_PS2)
    SM_MEMCARD_SELECT_MP,
#endif
    SM_PROFILE_SAVE_SELECT_MP,
    SM_MEMCARD_RESELECT_MP,

    SM_PROFILE_SELECT,
    SM_PROFILE_OPTIONS,
    SM_PROFILE_CONTROLS,
    SM_PROFILE_AVATAR,

    SM_ONLINE_SILENT_LOGON,
#ifdef TARGET_PS2
    SM_ONLINE_EULA,
#endif
    SM_ONLINE_CONNECT,
    SM_ONLINE_AUTHENTICATE,
#ifndef TARGET_XBOX
    SM_ONLINE_COPA,
    SM_ONLINE_COPA_SAVE_SELECT,
    SM_ONLINE_COPA_MEMCARD_RESELECT,
#endif
    SM_ONLINE_PROFILE_SELECT,
    SM_ONLINE_PROFILE_OPTIONS,
    SM_ONLINE_PROFILE_CONTROLS,
    SM_ONLINE_PROFILE_AVATAR,
#if defined(TARGET_PS2)
    SM_ONLINE_MEMCARD_SELECT,
#endif
    SM_ONLINE_MAIN_MENU,
    SM_ONLINE_QUICKMATCH,
    SM_ONLINE_OPTIMATCH,
    SM_ONLINE_JOIN_FILTER,
    SM_ONLINE_JOIN_MENU,
    SM_ONLINE_JOIN_SAVE_SETTINGS,
    SM_SERVER_PLAYERS_MENU,
    SM_ONLINE_PLAYERS_MENU,
    SM_ONLINE_FEEDBACK_MENU,
    SM_ONLINE_FEEDBACK_MENU_FRIEND,
    SM_ONLINE_FRIENDS_MENU,
    SM_ONLINE_DOWNLOAD,
    SM_ONLINE_STATS,
    SM_ONLINE_EDIT_PROFILE,
    SM_ONLINE_EDIT_CONTROLS,
    SM_ONLINE_EDIT_AVATAR,
    SM_ONLINE_PROFILE_SAVE_SELECT,
    SM_ONLINE_MEMCARD_RESELECT,
    SM_ONLINE_LOGIN,    

    SM_ONLINE_HOST_MENU,
    SM_ONLINE_HOST_MENU_OPTIONS,
    SM_HOST_LEVEL_SELECT,
    SM_HOST_SAVE_SETTINGS,

#ifndef CONFIG_RETAIL
    SM_LEVEL_SELECT,
#endif
    SM_START_GAME,
    SM_START_SAVE_GAME,
 
    SM_SINGLE_PLAYER_LOAD_MISSION,
    SM_MULTI_PLAYER_LOAD_MISSION,
    SM_CLIENT_WAIT_FOR_MISSION,
    SM_SERVER_SYNC,
    SM_CLIENT_SYNC,
    SM_SERVER_COOLDOWN,
    SM_CLIENT_COOLDOWN,

    SM_SERVER_DISCONNECT,
    SM_CLIENT_DISCONNECT,

    SM_PLAYING_GAME,

    SM_PAUSE_MAIN_MENU,
    SM_PAUSE_OPTIONS,
    SM_PAUSE_CONTROLS,
    SM_PAUSE_SETTINGS,
    SM_PAUSE_HEADSET,
    SM_PAUSE_SETTINGS_SELECT,
#ifdef TARGET_XBOX
    SM_PAUSE_FRIENDS,
#endif
#if defined(TARGET_PS2)
    SM_PAUSE_MEMCARD_SELECT,
#endif
    SM_PAUSE_PROFILE_SAVE_SELECT,
    SM_PAUSE_MEMCARD_RESELECT,
    SM_PAUSE_MP,
    SM_PAUSE_MP_SCORE,
    SM_PAUSE_MP_OPTIONS,
    SM_PAUSE_MP_CONTROLS,
    SM_PAUSE_MP_SETTINGS,
    SM_PAUSE_MP_HEADSET,
    SM_PAUSE_MP_SETTINGS_SELECT,
#ifdef TARGET_XBOX
    SM_PAUSE_MP_FRIENDS,
#endif
#if defined(TARGET_PS2)
    SM_PAUSE_MP_MEMCARD_SELECT,
#endif
    SM_PAUSE_MP_PROFILE_SAVE_SELECT,
    SM_PAUSE_MP_MEMCARD_RESELECT,

    SM_PAUSE_ONLINE,
    SM_PAUSE_ONLINE_VOTE_MAP,
    SM_PAUSE_ONLINE_VOTE_KICK,
    SM_PAUSE_ONLINE_FRIENDS,
    SM_PAUSE_ONLINE_PLAYERS,
    SM_PAUSE_ONLINE_FEEDBACK,
    SM_PAUSE_ONLINE_FEEDBACK_FRIEND,
    SM_PAUSE_ONLINE_SERVER_CONFIG,
    SM_PAUSE_ONLINE_OPTIONS,
    SM_PAUSE_ONLINE_CONTROLS,
    SM_PAUSE_ONLINE_SETTINGS,
    SM_PAUSE_ONLINE_HEADSET,
#if defined(TARGET_PS2)
    SM_PAUSE_ONLINE_MEMCARD_SELECT,
#endif
    SM_PAUSE_ONLINE_SAVE_SELECT,
    SM_PAUSE_ONLINE_MEMCARD_RESELECT,

    SM_PAUSE_ONLINE_CHANGE_MAP,
    SM_PAUSE_ONLINE_KICK_PLAYER,

    SM_PAUSE_ONLINE_TEAM_CHANGE,
    SM_PAUSE_ONLINE_RECONFIG_ONE,
    SM_PAUSE_ONLINE_RECONFIG_TWO,
    SM_PAUSE_ONLINE_RECONFIG_MAP,
    SM_PAUSE_ONLINE_SAVE_SETTINGS,

    SM_END_PAUSE,
    SM_EXIT_GAME,
    SM_POST_GAME,

    SM_AUTOSAVE_MENU,
    SM_AUTOSAVE_PROFILE_RESELECT,
#if defined(TARGET_PS2)
    SM_AUTOSAVE_MEMCARD_SELECT,
#endif
    SM_END_AUTOSAVE,

    SM_FINAL_SCORE,
    SM_ADVANCE_LEVEL,
    SM_RELOAD_CHECKPOINT,
    SM_REPORT_ERROR,

    SM_GAME_EXIT_PROMPT_FOR_SAVE,
    SM_GAME_EXIT_SAVE_SELECT,
    SM_GAME_EXIT_MEMCARD_RESELECT,
    SM_GAME_EXIT_SAVE_SETTINGS,
    SM_GAME_EXIT_SETTINGS_OVERWRITE,
    SM_GAME_EXIT_SETTINGS_SELECT,
    SM_GAME_EXIT_REDIRECT,
    SM_GAME_OVER,
    SM_FOLLOW_BUDDY,

    SM_NUM_STATES
};

enum sm_game_levels
{
    SM_LEVEL_WELCOME_TO_DREAMLAND,
    SM_LEVEL_DEEP_UNDERGROUND,
    SM_LEVEL_THE_HOT_ZONE,
    SM_LEVEL_THE_SEARCH,
    SM_LEVEL_THEY_GET_BIGGER,
    SM_LEVEL_THE_LAST_STAND,
    SM_LEVEL_ONE_OF_THEM,
    SM_LEVEL_INTERNAL_CHANGES,
    SM_LEVEL_LIFE_OR_DEATH,
    SM_LEVEL_DR_CRAY,
    SM_LEVEL_HATCHING_PARASITES,
    SM_LEVEL_FLYING_OBJECTS,
    SM_LEVEL_LIES_OF_THE_PAST,
    SM_LEVEL_BURIED_SECRETS,
    SM_LEVEL_NOW_BOARDING,
    SM_LEVEL_THE_GREYS,
    SM_LEVEL_THE_ASCENSION,
    SM_LEVEL_THE_LAST_EXIT,
};

enum sm_ai_types
{
    SM_AI_TYPE_NONE,
    SM_AI_TYPE_DUSTMITE,
    SM_AI_TYPE_MUTANT,
    SM_AI_TYPE_THETA,
    SM_AI_TYPE_HAZMAT,
    SM_AI_TYPE_SPEC_4,
    SM_AI_TYPE_BLACK_OPS,
    SM_AI_TYPE_RANDOM,
};

enum sm_sensitivity_controls
{
    SM_X_SENSITIVITY,
    SM_Y_SENSITIVITY
};

enum sm_headset_controls
{
};

enum sm_leaderboards
{
    SM_LEADERBOARD,
    SM_TEAM_LEADERBOARD,
    SM_BIG_LEADERBOARD
};

enum campaign_type
{
    SM_NEW_CAMPAIGN_GAME,
    SM_LOAD_CAMPAIGN_GAME,
    SM_DEBUG_SELECT_GAME,
};

enum sm_system_error
{
    SM_SYS_ERR_DISK,
    SM_SYS_ERR_CONTROLLER,
    SM_SYS_ERR_DUPLICATE_LOGIN,
};

enum card_data_mode
{
    SM_CARDMODE_PROFILE,
    SM_CARDMODE_CONTENT,
    SM_CARDMODE_SETTINGS,
};


// Where did a login originate from? This is used to determine where to go on a login failure
// from ReportError
enum login_source
{
    LOGIN_FROM_FRIENDS,
    LOGIN_FROM_PLAYERS,
    LOGIN_FROM_JOIN_MENU,
    LOGIN_FROM_INVITE,
    LOGIN_FOLLOW_FRIEND,
    LOGIN_FROM_MAIN,
};

#ifdef TARGET_PS2
#define SM_PROFILE_COUNT        2
#else
#define SM_PROFILE_COUNT        4
#endif

#define SM_PROFILE_NAME_LENGTH  16

#define SM_MAX_AVATARS          16

#ifdef TARGET_XBOX
#define SM_MAX_PLAYERS          4
#define USE_MOVIES              1
#else
#define SM_MAX_PLAYERS          2
#endif

#ifdef TARGET_PS2
#define USE_MOVIES              1
#endif

#ifdef TARGET_PC
#define USE_MOVIES              1
#endif

#ifdef TARGET_PS2
// size of profile on PS2 memory card
#define PROFILE_DIR_SIZE       131 * 1024 
#define SETTINGS_DIR_SIZE      131 * 1024 
#elif defined TARGET_XBOX
// size of profile on XBOX HD
#define PROFILE_DIR_SIZE        32 * 1024  
#define SETTINGS_DIR_SIZE       32 * 1024 
#elif defined TARGET_PC
// size of profile on PC HD
#define PROFILE_DIR_SIZE        32 * 1024 
#define SETTINGS_DIR_SIZE       32 * 1024 
#endif


//==============================================================================
//  state_mgr
//==============================================================================
class state_mgr
{
public:
                            state_mgr                       ( void );
                           ~state_mgr                       ( void );

    void                    Init                            ( void );
    void                    Kill                            ( void );
    void                    ReInit                          ( void );

    void                    SetState                        ( sm_states State, xbool ForceStateChange = FALSE );
    s32                     GetState                        ( void )                                { return m_State; }

    const char*             GetStateName                    ( sm_states State );

    void                    SimplePopUp                     ( f32 Timeout, const xwchar* Title, const xwchar* Message, const xwchar* Message2 = NULL ); 
    xbool                   InSystemError                   ( void ){ return m_bDoSystemError; }
    void                    SystemError                     ( sm_system_error MessageID, void* pUserData );
    void                    CheckControllers                ( void );

    void                    Update                          ( f32 DeltaTime );
    void                    Render                          ( void );
    void                    DummyScreen                     ( const char* message, xbool canSkip, s32 waitTime);  

    void                    SetTimeoutTime                  ( f32 Time )                            { m_TimeoutTime = Time; }

    s32                     GetCurrentControl               ( void )                                { return m_CurrentControl[m_State]; }

    // Pause controls
    xbool                   IsPaused                        ( void )                                { return m_bIsPaused; }
    void                    SetPaused                       ( xbool bPause, s32 PausingController );
    xbool                   IsExiting                       ( void )                                { return m_Exit; }
    void                    ClearPause                      ( void )                                { m_bIsPaused = FALSE; }

    // Game controls        
    campaign_type           GetCampaignGameType             ( void )                                { return m_CampaignType;       }
    void                    SetLevelIndex                   ( s32 index )                           { m_LevelIndex = index;        }
    s32                     GetLevelIndex                   ( void )                                { return m_LevelIndex;         }
    xbool                   IsRestoredGame                  ( void )                                { return m_bStartSaveGame;     }
    void                    SetActiveControllerID           ( s32 ID );
    s32                     GetActiveControllerID           ( void )                                { return m_ActiveControllerID; }
    void                    SetControllerRequested          ( s32 Controller, xbool State )         { m_bControllerRequested[Controller] = State; }
    xbool                   GetControllerRequested          ( s32 Controller )                      { return m_bControllerRequested[Controller]; }

    // movie controls
    void                    EnableBackgroundMovie           ( void );
    void                    DisableBackgoundMovie           ( void );
    void                    PlayMovie                       ( const char* pFilename, xbool bResident, xbool bLooped );
    void                    CloseMovie                      ( void );

    // player inventory
    void                    BackupPlayerInventory           ( void );
    void                    RestorePlayerInventory          ( void );

    void                    EnableDefaultLoadOut            ( void )                                { m_bDoDefaultLoadOut = TRUE;  }
    void                    DisableDefaultLoadOut           ( void )                                { m_bDoDefaultLoadOut = FALSE; }
    xbool                   UseDefaultLoadOut               ( void )                                { return m_bDoDefaultLoadOut;  }

    // lore 
    void                    SetLoreAcquired                 ( s32 LoreID );
    s32                     SetLoreAcquired                 ( s32 MapID, s32 LoreID );

    u32                     GetTotalLoreAcquired            ( void );
    u32                     GetLevelLoreAcquired            ( s32 MapID );


    // map cycle functions
    void                    ClearMapCycle                   ( void );

    void                    IncrementMapCycle               ( void );
    void                    DecrementMapCycle               ( void );

    void                    InsertMapInCycle                ( s32 MapID );
    void                    DeleteMapFromCycle              ( s32 Index );

    s32                     GetMapCycleMapID                ( s32 Index );

    s32                     GetMapCycleIndex                ( void )                                { return m_MapCycleIdx;         }
    void                    SetMapCycleIndex                ( s32 Index )                           { ASSERT( Index < m_MapCycleCount ); ASSERT( Index >= 0 ); m_MapCycleIdx = Index; }
    s32                     GetMapCycleCount                ( void )                                { return m_MapCycleCount;       }
    void                    SetMapCycleCount                ( s32 Count )                           { ASSERT( Count<=MAP_CYCLE_SIZE ); ASSERT( Count>=0 ); m_MapCycleCount = Count; }
    xbool                   GetUseDefaultMapCycle           ( void )                                { return m_bUseDefault;         }
    void                    SetUseDefaultMapCycle           ( xbool UseDefault )                    { m_bUseDefault = UseDefault;   }


    // Profile functions
    void                    SetSelectedProfile              ( s32 playerID, u32 hash )              { ASSERT( playerID >= 0 ); ASSERT( playerID < SM_PROFILE_COUNT ); m_SelectedProfile[playerID] = hash; }
    u32                     GetSelectedProfile              ( s32 playerID )                        { ASSERT( playerID >= 0 ); ASSERT( playerID < SM_PROFILE_COUNT ); return m_SelectedProfile[playerID]; }
    void                    ClearSelectedProfile            ( s32 playerID )                        { ASSERT( playerID >= 0 ); ASSERT( playerID < SM_PROFILE_COUNT ); m_SelectedProfile[playerID] = 0;    }

    void                    SetProfileNotSaved              ( s32 playerID, xbool bNotSaved );
    xbool                   GetProfileNotSaved              ( s32 playerID )                        { ASSERT( playerID >= 0 ); ASSERT( playerID < SM_PROFILE_COUNT ); return m_ProfileNotSaved[playerID]; }

    player_profile&         GetActiveProfile                ( u32 playerID )                        { ASSERT( playerID < SM_PROFILE_COUNT ); return m_Profiles[playerID]; }
    
    void                    ResetProfile                    ( s32 index );
    const char*             GetProfileName                  ( s32 index )                           { ASSERT( index >= 0 ); ASSERT( index < SM_PROFILE_COUNT ); return m_Profiles[index].GetProfileName(); }

    // profiles are managed explicitly on the Front-end side.
    // These functions help to correctly map the indices on the client/server end.
    void                    ResetProfileListIndex           ( void );
    void                    SetupProfileListIndex           ( void );
    s32                     GetProfileListIndex             ( s32 index )                           { ASSERT( index >= 0 ); ASSERT(index < SM_PROFILE_COUNT); return m_ProfileListIndex[index]; }
        
    // pending profile functions
    void                    InitPendingProfile              ( s32 index );
    void                    ActivatePendingProfile          ( xbool MarkDirty = FALSE );
    player_profile&         GetPendingProfile               ( void )                                { return m_PendingProfile; }
    s32                     GetPendingProfileIndex          ( void )                                { return m_PendingProfileIndex; }

    // profile names list functions
    xarray<profile_info*>&  GetProfileList                  ( void )                                 { return m_ProfileNames; }

    // profile game exit save callback
    void                    OnGameExitSaveProfileCB         ( void );
    // settings game exit save callback
    void                    OnGameExitSaveSettingsCB        ( void );

    // save sizes
    s32                     GetProfileSaveSize              ( void )                                { return m_ProfileSaveSize;  }
    s32                     GetSettingsSaveSize             ( void )                                { return m_SettingsSaveSize; }

    xbool                   LoadConfiguration               ( void );
    
    void                    SetLocalPlayerSlot              ( s32 SlotID )                          { m_LocalPlayerSlot = SlotID; }
    s32                     GetLocalPlayerSlot              ( void )                                { return m_LocalPlayerSlot; }

    void                    StartLogin                      ( const server_info* pInfo, login_source LoginSource );

    const char*             GetFeedbackPlayer               ( u64& Identifier );
    void                    SetFeedbackPlayer               ( const char* pName, u64 Identifier );
    xbool                   IsShowingScores                 ( void )                                { return m_bShowingScores;      }
    void                    SetShowingScores                ( xbool Flag )                          { m_bShowingScores = Flag;      }

    void                    StartBackgroundRendering        ( void );
    void                    StopBackgroundRendering         ( void );
    xbool                   IsBackgroundThreadRunning       ( void );

    void                    SilentSaveProfile               ( void );
    xbool                   IsAutosaveInProgress            ( void )                                { return m_bAutosaveInProgress;     }
    xbool                   DisplayMemcardDialogs           ( void )                                { return !m_bDisableMemcardDialogs; }
    
    xbool                   GetAutosaveProfile              ( void )                                { return m_bAutosaveProfile;        }
    void                    SetAutosaveProfile              ( xbool IsEnabled )                     { m_bAutosaveProfile = IsEnabled;   }
    void                    Reboot                          ( reboot_reason Reason );
    global_settings&        GetPendingSettings              ( void )                                { return m_PendingSettings;         }
    global_settings&        GetActiveSettings               ( void )                                { return m_ActiveSettings;          }
    s32                     GetSettingsCardSlot             ( void )                                { return m_SettingsCardSlot;        }
    void                    SetSettingsCardSlot             ( s32 CardSlot )                        { m_SettingsCardSlot = CardSlot;    }
    void                    InitPendingSettings             ( void );                                
    void                    ActivatePendingSettings         ( xbool MarkDirty = FALSE );                                
    xbool                   IsFollowingBuddy                ( void )                                { return m_bFollowBuddy;            }

private:

    void                    EnterState                      ( sm_states State );
    void                    ExitState                       ( sm_states State );
    void                    UpdateState                     ( sm_states State, f32 DeltaTime ); 

    void                    EnterESRBNotice                 ( void );
    void                    UpdateESRBNotice                ( void );
    void                    ExitESRBNotice                  ( void );

    void                    EnterInevitableIntro            ( void );
    void                    UpdateInevitableIntro           ( void );
    void                    ExitInevitableIntro             ( void );
#if defined(TARGET_PS2)
    void                    EnterControllerCheck            ( void );
    void                    UpdateControllerCheck           ( void );
    void                    ExitControllerCheck             ( void );

    void                    EnterAutoSaveDialog             ( void );
    void                    UpdateAutoSaveDialog            ( void );
    void                    ExitAutoSaveDialog              ( void );
    rhandle<xbitmap>        m_AutoSave;
#endif
    void                    EnterMemcardBootCheck           ( void );
    void                    UpdateMemcardBootCheck          ( void );
    void                    ExitMemcardBootCheck            ( void );

    void                    EnterPressStart                 ( void );
    void                    UpdatePressStart                ( void );
    void                    ExitPressStart                  ( void );

    void                    EnterMainMenu                   ( void );
    void                    UpdateMainMenu                  ( void );
    void                    ExitMainMenu                    ( void );

    void                    EnterSettingsMenu               ( void );
    void                    UpdateSettingsMenu              ( void );
    void                    ExitSettingsMenu                ( void );

    void                    EnterSettingsHeadset            ( void );
    void                    UpdateSettingsHeadset           ( void );
    void                    ExitSettingsHeadset             ( void );

    void                    EnterSettingsMemcardSelect      ( void );
    void                    UpdateSettingsMemcardSelect     ( void );
    void                    ExitSettingsMemcardSelect       ( void );

    void                    EnterManageProfiles             ( void );
    void                    UpdateManageProfiles            ( void );
    void                    ExitManageProfiles              ( void );

    void                    EnterManageProfileOptions       ( void );
    void                    UpdateManageProfileOptions      ( void );
    void                    ExitManageProfileOptions        ( void );

    void                    EnterManageProfileControls      ( void );
    void                    UpdateManageProfileControls     ( void );
    void                    ExitManageProfileControls       ( void );

    void                    EnterManageProfileAvatar        ( void );
    void                    UpdateManageProfileAvatar       ( void );
    void                    ExitManageProfileAvatar         ( void );
#if defined(TARGET_PS2)
    void                    EnterManageMemcardSelect        ( void );
    void                    UpdateManageMemcardSelect       ( void );
    void                    ExitManageMemcardSelect         ( void );
#endif
    void                    EnterManageProfileSaveSelect    ( void );
    void                    UpdateManageProfileSaveSelect   ( void );
    void                    ExitManageProfileSaveSelect     ( void );

    void                    EnterManageMemcardReselect      ( void );
    void                    UpdateManageMemcardReselect     ( void );
    void                    ExitManageMemcardReselect       ( void );

    void                    EnterDemoExit                   ( void );
    void                    UpdateDemoExit                  ( void );
    void                    ExitDemoExit                    ( void );
#if defined(TARGET_PS2)
    void                    EnterMemcardSelect              ( void );
    void                    UpdateMemcardSelect             ( void );
    void                    ExitMemcardSelect               ( void );
#endif
    void                    EnterCampaignMenu               ( void );
    void                    UpdateCampaignMenu              ( void );
    void                    ExitCampaignMenu                ( void );

    void                    EnterCampaignProfileOptions     ( void );
    void                    UpdateCampaignProfileOptions    ( void );
    void                    ExitCampaignProfileOptions      ( void );

    void                    EnterCampaignProfileControls    ( void );
    void                    UpdateCampaignProfileControls   ( void );
    void                    ExitCampaignProfileControls     ( void );

    void                    EnterCampaignProfileAvatar      ( void );
    void                    UpdateCampaignProfileAvatar     ( void );
    void                    ExitCampaignProfileAvatar       ( void );

#if defined(TARGET_PS2)
    void                    EnterCampaignMemcardSelect      ( void );
    void                    UpdateCampaignMemcardSelect     ( void );
    void                    ExitCampaignMemcardSelect       ( void );
#endif
    void                    EnterCampaignProfileSaveSelect  ( void );
    void                    UpdateCampaignProfileSaveSelect ( void );
    void                    ExitCampaignProfileSaveSelect   ( void );

    void                    EnterCampaignMemcardReselect    ( void );
    void                    UpdateCampaignMemcardReselect   ( void );
    void                    ExitCampaignMemcardReselect     ( void );

    void                    EnterLoadCampaign               ( void );
    void                    UpdateLoadCampaign              ( void );
    void                    ExitLoadCampaign                ( void );

    void                    EnterSaveCampaign               ( void );
    void                    UpdateSaveCampaign              ( void );
    void                    ExitSaveCampaign                ( void );

    void                    EnterLoreMainMenu               ( void );
    void                    UpdateLoreMainMenu              ( void );
    void                    ExitLoreMainMenu                ( void );

    void                    EnterSecretsMenu                ( void );
    void                    UpdateSecretsMenu               ( void );
    void                    ExitSecretsMenu                 ( void );

    void                    EnterExtrasMenu                 ( void );
    void                    UpdateExtrasMenu                ( void );
    void                    ExitExtrasMenu                  ( void );

    void                    EnterCreditsScreen              ( void );
    void                    UpdateCreditsScreen             ( void );
    void                    ExitCreditsScreen               ( void );

    void                    EnterProfileSelect              ( void );
    void                    UpdateProfileSelect             ( void );
    void                    ExitProfileSelect               ( void );

    void                    EnterProfileOptions             ( void );
    void                    UpdateProfileOptions            ( void );
    void                    ExitProfileOptions              ( void );

    void                    EnterProfileControls            ( void );
    void                    UpdateProfileControls           ( void );
    void                    ExitProfileControls             ( void );

    void                    EnterProfileAvatar              ( void );
    void                    UpdateProfileAvatar             ( void );
    void                    ExitProfileAvatar               ( void );

    void                    EnterMultiPlayer                ( void );
    void                    UpdateMultiPlayer               ( void );
    void                    ExitMultiPlayer                 ( void );

    void                    EnterMultiPlayerOptions         ( void );
    void                    UpdateMultiPlayerOptions        ( void );
    void                    ExitMultiPlayerOptions          ( void );

    void                    EnterMultiPlayerEdit            ( void );
    void                    UpdateMultiPlayerEdit           ( void );
    void                    ExitMultiPlayerEdit             ( void );

    void                    EnterMPLevelSelect              ( void );
    void                    UpdateMPLevelSelect             ( void );
    void                    ExitMPLevelSelect               ( void );

    void                    EnterMPSaveSettings             ( void );
    void                    UpdateMPSaveSettings            ( void );
    void                    ExitMPSaveSettings              ( void );

    void                    EnterProfileControlsMP          ( void );
    void                    UpdateProfileControlsMP         ( void );
    void                    ExitProfileControlsMP           ( void );

    void                    EnterProfileAvatarMP            ( void );
    void                    UpdateProfileAvatarMP           ( void );
    void                    ExitProfileAvatarMP             ( void );
#if defined(TARGET_PS2)
    void                    EnterMemcardSelectMP            ( void );
    void                    UpdateMemcardSelectMP           ( void );
    void                    ExitMemcardSelectMP             ( void );
#endif
    void                    EnterProfileSaveSelectMP        ( void );
    void                    UpdateProfileSaveSelectMP       ( void );
    void                    ExitProfileSaveSelectMP         ( void );

    void                    EnterMemcardReselectMP          ( void );
    void                    UpdateMemcardReselectMP         ( void );
    void                    ExitMemcardReselectMP           ( void );

    void            		EnterOnlineSilentLogin          ( void );

#ifdef TARGET_PS2
    void                    EnterOnlineEULA                 ( void );
    void                    UpdateOnlineEULA                ( void );
    void                    ExitOnlineEULA                  ( void );
#endif

    void                    EnterOnlineConnect              ( void );
    void                    UpdateOnlineConnect             ( void );
    void                    ExitOnlineConnect               ( void );

    void                    EnterOnlineProfileSelect        ( void );
    void                    UpdateOnlineProfileSelect       ( void );
    void                    ExitOnlineProfileSelect         ( void );

    void                    EnterOnlineProfileOptions       ( void );
    void                    UpdateOnlineProfileOptions      ( void );
    void                    ExitOnlineProfileOptions        ( void );

    void                    EnterOnlineProfileControls      ( void );
    void                    UpdateOnlineProfileControls     ( void );
    void                    ExitOnlineProfileControls       ( void );

    void                    EnterOnlineProfileAvatar        ( void );
    void                    UpdateOnlineProfileAvatar       ( void );
    void                    ExitOnlineProfileAvatar         ( void );
#if defined(TARGET_PS2)
    void                    EnterOnlineMemcardSelect        ( void );
    void                    UpdateOnlineMemcardSelect       ( void );
    void                    ExitOnlineMemcardSelect         ( void );
#endif
    void                    EnterOnlineProfileSaveSelect    ( void );
    void                    UpdateOnlineProfileSaveSelect   ( void );
    void                    ExitOnlineProfileSaveSelect     ( void );

    void                    EnterOnlineMemcardReselect      ( void );
    void                    UpdateOnlineMemcardReselect     ( void );
    void                    ExitOnlineMemcardReselect       ( void );

    void                    EnterOnlineAuthenticate         ( void );
    void                    UpdateOnlineAuthenticate        ( void );
    void                    ExitOnlineAuthenticate          ( void );
#ifndef TARGET_XBOX
    void                    EnterOnlineCOPA                 ( void );
    void                    UpdateOnlineCOPA                ( void );
    void                    ExitOnlineCOPA                  ( void );

    void                    EnterOnlineCOPASaveSelect       ( void );
    void                    UpdateOnlineCOPASaveSelect      ( void );
    void                    ExitOnlineCOPASaveSelect        ( void );

    void                    EnterOnlineCOPAMemcardReselect  ( void );
    void                    UpdateOnlineCOPAMemcardReselect ( void );
    void                    ExitOnlineCOPAMemcardReselect   ( void );
#endif
    void                    EnterOnlineMain                 ( void );
    void                    UpdateOnlineMain                ( void );
    void                    ExitOnlineMain                  ( void );

    void                    EnterOnlineHost                 ( void );
    void                    UpdateOnlineHost                ( void );
    void                    ExitOnlineHost                  ( void );

    void                    EnterOnlineHostOptions          ( void );
    void                    UpdateOnlineHostOptions         ( void );
    void                    ExitOnlineHostOptions           ( void );

    void                    EnterHostSaveSettings           ( void );
    void                    UpdateHostSaveSettings          ( void );
    void                    ExitHostSaveSettings            ( void );

    void                    EnterOnlineQuickMatch           ( void );
    void                    UpdateOnlineQuickMatch          ( void );
    void                    ExitOnlineQuickMatch            ( void );

    void                    EnterOnlineOptiMatch            ( void );
    void                    UpdateOnlineOptiMatch           ( void );
    void                    ExitOnlineOptiMatch             ( void );

    void                    EnterOnlineJoinFilter           ( void );
    void                    UpdateOnlineJoinFilter          ( void );
    void                    ExitOnlineJoinFilter            ( void );

    void                    EnterOnlineJoin                 ( void );
    void                    UpdateOnlineJoin                ( void );
    void                    ExitOnlineJoin                  ( void );

    void                    EnterOnlineJoinSaveSettings     ( void );
    void                    UpdateOnlineJoinSaveSettings    ( void );
    void                    ExitOnlineJoinSaveSettings      ( void );

    void                    EnterServerPlayers              ( void );
    void                    UpdateServerPlayers             ( void );
    void                    ExitServerPlayers               ( void );

    void                    EnterOnlinePlayers              ( void );
    void                    UpdateOnlinePlayers             ( void );
    void                    ExitOnlinePlayers               ( void );

    void                    EnterOnlineFeedback             ( void );
    void                    UpdateOnlineFeedback            ( void );
    void                    ExitOnlineFeedback              ( void );

    void                    EnterOnlineFeedbackFriend       ( void );
    void                    UpdateOnlineFeedbackFriend      ( void );
    void                    ExitOnlineFeedbackFriend        ( void );

    void                    EnterOnlineFriends              ( void );
    void                    UpdateOnlineFriends             ( void );
    void                    ExitOnlineFriends               ( void );

    void                    EnterOnlineStats                ( void );
    void                    UpdateOnlineStats               ( void );
    void                    ExitOnlineStats                 ( void );

    void                    EnterOnlineEditProfile          ( void );
    void                    UpdateOnlineEditProfile         ( void );
    void                    ExitOnlineEditProfile           ( void );

    void                    EnterOnlineEditControls         ( void );
    void                    UpdateOnlineEditControls        ( void );
    void                    ExitOnlineEditControls          ( void );

    void                    EnterOnlineEditAvatar           ( void );
    void                    UpdateOnlineEditAvatar          ( void );
    void                    ExitOnlineEditAvatar            ( void );

    void                    EnterHostLevelSelect            ( void );
    void                    UpdateHostLevelSelect           ( void );
    void                    ExitHostLevelSelect             ( void );

    void                    EnterLevelSelect                ( void );
    void                    UpdateLevelSelect               ( void );
    void                    ExitLevelSelect                 ( void );

    void                    EnterStartSaveGame              ( void );
    void                    UpdateStartSaveGame             ( void );
    void                    ExitStartSaveGame               ( void );
    
    void                    EnterStartGame                  ( void );
    void                    UpdateStartGame                 ( void );
    void                    ExitStartGame                   ( void );

    void                    EnterLoadGame                   ( void );
    void                    UpdateLoadGame                  ( void );
    void                    ExitLoadGame                    ( void );

    void                    EnterResumeCampaign             ( void );
    void                    UpdateResumeCampaign            ( void );
    void                    ExitResumeCampaign              ( void );

    void                    EnterPlayingGame                ( void );
    void                    UpdatePlayingGame               ( void );
    void                    ExitPlayingGame                 ( void );

    void                    EnterPauseMain                  ( void );
    void                    UpdatePauseMain                 ( void );
    void                    ExitPauseMain                   ( void );

    void                    EnterPauseOptions               ( void );
    void                    UpdatePauseOptions              ( void );
    void                    ExitPauseOptions                ( void );

    void                    EnterPauseControls              ( void );
    void                    UpdatePauseControls             ( void );
    void                    ExitPauseControls               ( void );

    void                    EnterPauseSettings              ( void );
    void                    UpdatePauseSettings             ( void );
    void                    ExitPauseSettings               ( void );

    void                    EnterPauseHeadset               ( void );
    void                    UpdatePauseHeadset              ( void );
    void                    ExitPauseHeadset                ( void );

    void                    EnterPauseSettingsSelect        ( void );
    void                    UpdatePauseSettingsSelect       ( void );
    void                    ExitPauseSettingsSelect         ( void );
#ifdef TARGET_XBOX
    void                    EnterPauseFriends               ( void );
    void                    UpdatePauseFriends              ( void );
    void                    ExitPauseFriends                ( void );
#endif
#if defined(TARGET_PS2)
    void                    EnterPauseMemcardSelect         ( void );
    void                    UpdatePauseMemcardSelect        ( void );
    void                    ExitPauseMemcardSelect          ( void );
#endif
    void                    EnterPauseMemcardSaveSelect     ( void );
    void                    UpdatePauseMemcardSaveSelect    ( void );
    void                    ExitPauseMemcardSaveSelect      ( void );

    void                    EnterPauseMemcardReselect         ( void );
    void                    UpdatePauseMemcardReselect        ( void );
    void                    ExitPauseMemcardReselect          ( void );

    void                    EnterPauseMP                    ( void );
    void                    UpdatePauseMP                   ( void );
    void                    ExitPauseMP                     ( void );

    void                    EnterPauseMPSettings            ( void );
    void                    UpdatePauseMPSettings           ( void );
    void                    ExitPauseMPSettings             ( void );

    void                    EnterPauseMPHeadset             ( void );
    void                    UpdatePauseMPHeadset            ( void );
    void                    ExitPauseMPHeadset              ( void );

    void                    EnterPauseMPSettingsSelect      ( void );
    void                    UpdatePauseMPSettingsSelect     ( void );
    void                    ExitPauseMPSettingsSelect       ( void );
#ifdef TARGET_XBOX
    void                    EnterPauseMPFriends             ( void );
    void                    UpdatePauseMPFriends            ( void );
    void                    ExitPauseMPFriends              ( void );
#endif
    void                    EnterPauseMPOptions             ( void );
    void                    UpdatePauseMPOptions            ( void );
    void                    ExitPauseMPOptions              ( void );

    void                    EnterPauseMPControls            ( void );
    void                    UpdatePauseMPControls           ( void );
    void                    ExitPauseMPControls             ( void );
#if defined(TARGET_PS2)
    void                    EnterPauseMPMemcardSelect       ( void );
    void                    UpdatePauseMPMemcardSelect      ( void );
    void                    ExitPauseMPMemcardSelect        ( void );
#endif
    void                    EnterPauseMPMemcardSaveSelect   ( void );
    void                    UpdatePauseMPMemcardSaveSelect  ( void );
    void                    ExitPauseMPMemcardSaveSelect    ( void );

    void                    EnterPauseMPMemcardReselect     ( void );
    void                    UpdatePauseMPMemcardReselect    ( void );
    void                    ExitPauseMPMemcardReselect      ( void );

    void                    EnterPauseOnline                ( void );
    void                    UpdatePauseOnline               ( void );
    void                    ExitPauseOnline                 ( void );

    void                    EnterPauseOnlineVoteMap         ( void );
    void                    UpdatePauseOnlineVoteMap        ( void );
    void                    ExitPauseOnlineVoteMap          ( void );

    void                    EnterPauseOnlineVoteKick        ( void );
    void                    UpdatePauseOnlineVoteKick       ( void );
    void                    ExitPauseOnlineVoteKick         ( void );

    void                    EnterPauseOnlineFriends         ( void );
    void                    UpdatePauseOnlineFriends        ( void );
    void                    ExitPauseOnlineFriends          ( void );

    void                    EnterPauseOnlineFeedback        ( void );
    void                    UpdatePauseOnlineFeedback       ( void );
    void                    ExitPauseOnlineFeedback         ( void );

    void                    EnterPauseOnlineFeedbackFriend  ( void );
    void                    UpdatePauseOnlineFeedbackFriend ( void );
    void                    ExitPauseOnlineFeedbackFriend   ( void );

    void                    EnterPauseOnlinePlayers         ( void );
    void                    UpdatePauseOnlinePlayers        ( void );
    void                    ExitPauseOnlinePlayers          ( void );

    void                    EnterPauseOnlineOptions         ( void );
    void                    UpdatePauseOnlineOptions        ( void );
    void                    ExitPauseOnlineOptions          ( void );

    void                    EnterPauseOnlineControls        ( void );
    void                    UpdatePauseOnlineControls       ( void );
    void                    ExitPauseOnlineControls         ( void );

    void                    EnterPauseOnlineSettings        ( void );
    void                    UpdatePauseOnlineSettings       ( void );
    void                    ExitPauseOnlineSettings         ( void );

    void                    EnterPauseOnlineHeadset         ( void );
    void                    UpdatePauseOnlineHeadset        ( void );
    void                    ExitPauseOnlineHeadset          ( void );
#if defined(TARGET_PS2)
    void                    EnterPauseOnlineMemcardSelect   ( void );
    void                    UpdatePauseOnlineMemcardSelect  ( void );
    void                    ExitPauseOnlineMemcardSelect    ( void );
#endif

    void                    EnterPauseOnlineSaveSelect      ( void );
    void                    UpdatePauseOnlineSaveSelect     ( void );
    void                    ExitPauseOnlineSaveSelect       ( void );

    void                    EnterPauseOnlineMemcardReselect ( void );
    void                    UpdatePauseOnlineMemcardReselect( void );
    void                    ExitPauseOnlineMemcardReselect  ( void );

    void                    EnterPauseServerConfig          ( void );
    void                    UpdatePauseServerConfig         ( void );
    void                    ExitPauseServerConfig           ( void );

    void                    EnterPauseOnlineChangeMap       ( void );
    void                    UpdatePauseOnlineChangeMap      ( void );
    void                    ExitPauseOnlineChangeMap        ( void );

    void                    EnterPauseOnlineKickPlayer      ( void );
    void                    UpdatePauseOnlineKickPlayer     ( void );
    void                    ExitPauseOnlineKickPlayer       ( void );

    void                    EnterPauseOnlineTeamChange      ( void );
    void                    UpdatePauseOnlineTeamChange     ( void );
    void                    ExitPauseOnlineTeamChange       ( void );

    void                    EnterPauseOnlineReconfigOne     ( void );
    void                    UpdatePauseOnlineReconfigOne    ( void );
    void                    ExitPauseOnlineReconfigOne      ( void );

    void                    EnterPauseOnlineReconfigTwo     ( void );
    void                    UpdatePauseOnlineReconfigTwo    ( void );
    void                    ExitPauseOnlineReconfigTwo      ( void );

    void                    EnterPauseOnlineReconfigMap     ( void );
    void                    UpdatePauseOnlineReconfigMap    ( void );
    void                    ExitPauseOnlineReconfigMap      ( void );

    void                    EnterPauseOnlineSaveSettings    ( void );
    void                    UpdatePauseOnlineSaveSettings   ( void );
    void                    ExitPauseOnlineSaveSettings     ( void );

    void                    EnterPauseMPScore               ( void );
    void                    UpdatePauseMPScore              ( void );
    void                    ExitPauseMPScore                ( void );

    void                    EnterEndPause                   ( void );
    void                    UpdateEndPause                  ( void );
    void                    ExitEndPause                    ( void );

    void                    EnterExitGame                   ( void );
    void                    UpdateExitGame                  ( void );
    void                    ExitExitGame                    ( void );

    void                    EnterPostGame                   ( void );
    void                    UpdatePostGame                  ( void );
    void                    ExitPostGame                    ( void );

    void                    EnterAutosaveMenu               ( void );
    void                    UpdateAutosaveMenu              ( void );
    void                    ExitAutosaveMenu                ( void );

    void                    EnterAutosaveProfileReselect    ( void );
    void                    UpdateAutosaveProfileReselect   ( void );
    void                    ExitAutosaveProfileReselect     ( void );

#if defined(TARGET_PS2)
    void                    EnterAutosaveMemcardSelect      ( void );
    void                    UpdateAutosaveMemcardSelect     ( void );
    void                    ExitAutosaveMemcardSelect       ( void );
#endif

    void                    EnterEndAutosave                ( void );
    void                    UpdateEndAutosave               ( void );
    void                    ExitEndAutosave                 ( void );

    void                    EnterFinalScore                 ( void );
    void                    UpdateFinalScore                ( void );
    void                    ExitFinalScore                  ( void );

    void                    EnterSinglePlayerLoadMission    ( void );
    void                    UpdateSinglePlayerLoadMission   ( void );
    void                    ExitSinglePlayerLoadMission     ( void );

    void                    EnterMultiPlayerLoadMission     ( void );
    void                    UpdateMultiPlayerLoadMission    ( void );
    void                    ExitMultiPlayerLoadMission      ( void );

    void                    EnterServerSync                 ( void );
    void                    UpdateServerSync                ( void );
    void                    ExitServerSync                  ( void );

    void                    EnterClientSync                 ( void );
    void                    UpdateClientSync                ( void );
    void                    ExitClientSync                  ( void );
        
    void                    EnterServerCooldown             ( void );
    void                    UpdateServerCooldown            ( void );
    void                    ExitServerCooldown              ( void );

    void                    EnterClientCooldown             ( void );
    void                    UpdateClientCooldown            ( void );
    void                    ExitClientCooldown              ( void );

    void                    EnterServerDisconnect           ( void );
    void                    UpdateServerDisconnect          ( void );
    void                    ExitServerDisconnect            ( void );

    void                    EnterClientDisconnect           ( void );
    void                    UpdateClientDisconnect          ( void );
    void                    ExitClientDisconnect            ( void );

    void                    EnterAdvanceLevel               ( void );
    void                    UpdateAdvanceLevel              ( void );
    void                    ExitAdvanceLevel                ( void );

    void                    EnterReloadCheckpoint           ( void );
    void                    UpdateReloadCheckpoint          ( void );
    void                    ExitReloadCheckpoint            ( void );

    void                    EnterReportError                ( void );
    void                    UpdateReportError               ( void );
    void                    ExitReportError                 ( void );

    void                    EnterGameExitPromptForSave      ( void );
    void                    UpdateGameExitPromptForSave     ( void );
    void                    ExitGameExitPromptForSave       ( void );

    void                    EnterGameExitSaveSelect         ( void );
    void                    UpdateGameExitSaveSelect        ( void );
    void                    ExitGameExitSaveSelect          ( void );

    void                    EnterGameExitMemcardReselect    ( void );
    void                    UpdateGameExitMemcardReselect   ( void );
    void                    ExitGameExitMemcardReselect     ( void );

    void                    EnterGameExitSaveSettings       ( void );
    void                    UpdateGameExitSaveSettings      ( void );
    void                    ExitGameExitSaveSettings        ( void );

    void                    EnterGameExitSettingsOverwrite  ( void );
    void                    UpdateGameExitSettingsOverwrite ( void );
    void                    ExitGameExitSettingsOverwrite   ( void );

    void                    EnterGameExitSettingsSelect     ( void );
    void                    UpdateGameExitSettingsSelect    ( void );
    void                    ExitGameExitSettingsSelect      ( void );

    void                    EnterGameExitRedirect           ( void );
    void                    UpdateGameExitRedirect          ( void );
    void                    ExitGameExitRedirect            ( void );

    void                    EnterOnlineDownload             ( void );
    void                    UpdateOnlineDownload            ( void );
    void                    ExitOnlineDownload              ( void );

    void                    EnterOnlineLogin                ( void );
    void                    UpdateOnlineLogin               ( void );
    void                    ExitOnlineLogin                 ( void );

    void                    EnterGameOver                   ( void );
    void                    UpdateGameOver                  ( void );
    void                    ExitGameOver                    ( void );

    void                    EnterFollowBuddy                ( void );
    void                    UpdateFollowBuddy               ( void );
    void                    ExitFollowBuddy                 ( void );

    void                    DisconnectFromNetwork           ( void );
    // This should be used at the start of every online state update function. This will make
    // sure that, should the network connection be lost, recovery can start.
    // Should be used in the form:
    //
    //      if( CheckForDisconnect() )
    //      {
    //          return;
    //      }
    // It will have already removed all dialogs from the dialog stack and reset everything.
    // It is important that each dialog properly cleans up after itself on getting a Destroy() 
    // call.
    xbool                   CheckForDisconnect              ( void );
    void                    SilentSaveProfileReturn         ( void );

    xbool                   m_bInited;
    xbool                   m_bIsPaused;
    xbool                   m_bWasPaused;
    xbool                   m_bDoSystemError;
    xbool                   m_bIsDuplicateLogin;
    xbool                   m_bRetrySilentAutoSave;
    xbool                   m_bRetryAutoSaveMenu;
    xbool                   m_bPlayMovie;
    xbool                   m_bStartSaveGame;
    xbool                   m_bShowingScores;

    campaign_type           m_CampaignType;
    sm_states               m_State;
    sm_states               m_PrevState;
    f32                     m_Timeout;
    f32                     m_TimeoutTime;

    ui_dialog*              m_CurrentDialog;
    dlg_popup*              m_PopUp;
    s32                     m_PopUpResult;
    xbool                   m_bUserWasEnabled;

    xthread*                m_pBackgroundRenderer;
    xbool                   m_BackgroundRendererRunning;
    view                    m_View;
    xbool                   m_Exit;

    s32                     m_PlayerCount;
    s32                     m_LevelID;
    s32                     m_LevelIndex;
    s32                     m_ActiveControllerID;                   // -1 indicates no lock

    s32                     m_CurrentControl[SM_NUM_STATES];
    // movie player controls
    vector2                 m_MoviePosition;
    vector2                 m_MovieSize;

    // player inventory tracking
    inventory2              m_StoredPlayerInventory;
    xbool                   m_bInventoryIsStored;
    f32                     m_PlayerHealth;
    f32                     m_PlayerMaxHealth;
    f32                     m_Mutagen;
    xbool                   m_bDoDefaultLoadOut;
    xbool                   m_PlayerMutantMelee;
    xbool                   m_PlayerMutantPrimary;
    xbool                   m_PlayerMutantSecondary;
    inven_item              m_CurrentWeaponItem;
    inven_item              m_PrevWeaponItem;
    inven_item              m_NextWeaponItem;

    s32                     m_AmmoAmount        [INVEN_WEAPON_LAST];
    s32                     m_AmmoMax           [INVEN_WEAPON_LAST];
    s32                     m_AmmoPerClip       [INVEN_WEAPON_LAST];
    s32                     m_AmmoInCurrentClip [INVEN_WEAPON_LAST];

    global_settings         m_ActiveSettings;
    global_settings         m_PendingSettings;
    s32                     m_SettingsCardSlot;

    // map cycle data
    s32                     m_MapCycle[MAP_CYCLE_SIZE];     // an array of map ids
    s32                     m_MapCycleIdx;                  // curr index into the map cycle
    s32                     m_MapCycleCount;                // number of maps in the rotation
    xbool                   m_bUseDefault;                  // use the default map cycle

    // feedback player name 
    char                    m_FeedbackName[SM_PROFILE_NAME_LENGTH];                        
    u64                     m_FeedbackIdentifier;

    // leaderboard status
    u32                     m_LeaderboardID;
    ui_dialog*              m_pLeaderboardDialog;
    s32                     m_LocalPlayerSlot;

    // profiles
    player_profile          m_Profiles[SM_PROFILE_COUNT];           // profile array - one for each player
    s32                     m_ProfileListIndex[SM_PROFILE_COUNT];   // index in the list of profiles read from the card
    u32                     m_SelectedProfile[SM_PROFILE_COUNT];    // hash of the selected profile
    xbool                   m_ProfileNotSaved[SM_PROFILE_COUNT];    // is the selected profile on card or only in memory
    xarray<profile_info*>   m_ProfileNames;                         // xarray of profile names read from the memory cards
    player_profile          m_PendingProfile;                       // pending profile changes
    s32                     m_PendingProfileIndex;                  // pending profile index
    xbool                   m_bCreatingProfile;                     // we are creating a new profile
    xbool                   m_bAutosaveProfile;                     // Autosave profile is enabled.
    xbool                   m_bAutosaveInProgress;                  // Autosave is currently in progress
    xbool                   m_bDisableMemcardDialogs;               // Should we display memcard dialogs
    xbool                   m_bFollowBuddy;                         // Following your buddy in to a game, SessionID in g_Pending
    login_source            m_LoginFailureDestination;
    xbool                   m_bSilentSigninStarted;
    s32                     m_iCard;                                // Card slot in use for current operation

    // Save profile return callback
    xbool                   m_bControllerRequested[SM_MAX_PLAYERS];  // used to keep track of which controllers are supposed to be in use.

    // save sizes
    s32                     m_ProfileSaveSize;
    s32                     m_SettingsSaveSize;

};

extern state_mgr g_StateMgr;

//==============================================================================
#endif // STATEMGR_HPP
//==============================================================================

