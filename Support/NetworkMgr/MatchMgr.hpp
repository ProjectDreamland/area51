#ifndef MATCH_MGR_HPP
#define MATCH_MGR_HPP


#include "x_files.hpp"
#include "e_Network.hpp"
#include "x_bitstream.hpp"
#include "x_threads.hpp"
#include "GameMgr.hpp"
#include "downloader/Downloader.hpp"
#include "NetLimits.hpp"

//
// Define which provider we are using for matchmaking services
//
// NOTE: If the provider is not defined, then only local network lookups will be performed.
//
//
#if defined(TARGET_PS2)||defined(TARGET_PC)
#define ENABLE_GAMESPY
#endif
    
#if defined(TARGET_XBOX)
#define ENABLE_XBOX_LIVE
#endif

#if defined(ENABLE_GAMESPY)
#include "NetworkMgr/GameSpy/serverbrowsing/sb_serverbrowsing.h"
#include "NetworkMgr/GameSpy/natneg/natneg.h"
#include "NetworkMgr/GameSpy/qr2/qr2.h"
#include "NetworkMgr/GameSpy/gp/gp.h"
#endif

enum auth_status
{
    AUTH_STAT_DISCONNECTED,
    AUTH_STAT_CONNECTED,
    AUTH_STAT_CONNECTING,
    AUTH_STAT_INVALID_USER,
    AUTH_STAT_NO_ACCOUNT,
    AUTH_STAT_CANNOT_CONNECT,
    AUTH_STAT_DROPPED,
    AUTH_STAT_RETIRED,
    AUTH_STAT_BANNED,
    AUTH_STAT_SECURITY_FAILED,
    AUTH_STAT_NEED_PASSWORD,
    AUTH_STAT_SELECT_USER,
    AUTH_STAT_NEED_UPDATE,
    AUTH_STAT_INVALID_ACCOUNT,
    AUTH_STAT_URGENT_MESSAGE,
    AUTH_STAT_NO_USERS,
    AUTH_STAT_ALREADY_LOGGED_ON,
    AUTH_STAT_SERVER_BUSY,
    // PC test
    AUTH_STAT_IDLE,
    AUTH_STAT_BUSY,
    AUTH_STAT_OK,
    AUTH_STAT_FAILED,
};

//------------------------------------------------------------------------------
enum match_conn_status
{
    MATCH_CONN_IDLE,
    MATCH_CONN_UNAVAILABLE,
    MATCH_CONN_ACQUIRING_LOBBIES,
    MATCH_CONN_ACQUIRING_SERVERS,
    MATCH_CONN_ACQUIRING_BUDDIES,
    MATCH_CONN_DOWNLOADING,
    MATCH_CONN_REGISTERING,
    MATCH_CONN_CONNECTED,
    MATCH_CONN_DISCONNECTED,
    MATCH_CONN_NOT_VISIBLE,
    MATCH_CONN_REGISTER_FAILED,
    MATCH_CONN_CONNECTING,
    MATCH_CONN_SESSION_ENDED,
    // PC test
    MATCH_CONN_INVALID_ACCOUNT,
    MATCH_CONN_SECURITY_FAILED,
};

#if defined(ENABLE_XBOX_LIVE)
#include <xtl.h>
#include <XOnline.h>
#endif

//------------------------------------------------------------------------------
enum lookup_type
{
    NET_PKT_LOOKUP_REQUEST=0x1234,
    NET_PKT_LOOKUP_RESPONSE,
    NET_PKT_EXTENDED_LOOKUP_REQUEST,
    NET_PKT_EXTENDED_LOOKUP_RESPONSE,
    NET_PKT_ECHO_REQUEST,
};

enum 
{
    HAS_FAVORITE_PLAYER = (1<<0),
    HAS_FAVORITE_SERVER = (1<<1),
};

enum player_feedback
{
    FB_GREAT_SESSION = 0,
    FB_GOOD_ATTITUDE,
    FB_BAD_NAME,
    FB_CURSING,
    FB_SCREAMING,
    FB_CHEATING,
    FB_THREATS,
    FB_OFFENSIVE_MESSAGE,
};

#if defined(ENABLE_XBOX_LIVE)

const s32   ATTR_HOSTNAME_KEY       = 1 | X_ATTRIBUTE_DATATYPE_STRING;
const s32   ATTR_MISSIONNAME_KEY    = 2 | X_ATTRIBUTE_DATATYPE_STRING;
const s32   ATTR_GAMETYPE_KEY       = 3 | X_ATTRIBUTE_DATATYPE_STRING;
const s32   ATTR_SHORTGAMETYPE_KEY  = 4 | X_ATTRIBUTE_DATATYPE_STRING;
const s32   ATTR_VERSION_KEY        = 5 | X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_FLAGS_KEY          = 6 | X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_GAMETIME_KEY       = 7 | X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_FRAGLIMIT_KEY      = 8 | X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_PORT_KEY           = 9 | X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_GAMETYPE_ID_KEY    = 10| X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_MIN_PLAYERS_KEY    = 11| X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_MUTATION_MODE_KEY  = 12| X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_PASSWORD_KEY       = 13| X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_HEADSET            = 14| X_ATTRIBUTE_DATATYPE_INTEGER;
const s32   ATTR_KEY_COUNT          = 14;
const s32   ATTR_QUERY_KEY_COUNT    = 5;


// Specify X_MATCH_NULL_INTEGER for optional integer query arguments
const ULONGLONG X_MATCH_NULL_INTEGER  = 0x7FFFFFFFFFFFFFFFui64;

//user stats

#endif

#if defined(ENABLE_GAMESPY)
extern const s32   GAMESPY_PRODUCT_ID;
extern const char* GAMESPY_GAMENAME;
extern const char* GAMESPY_SECRETKEY;

enum gamespy_user_keys
{
    LEVEL_KEY = 50,
    PLAYERS_KEY,
    FLAGS_KEY,
    GAMETYPE_ID_KEY,
    AVGPING_KEY,
    MUTATION_MODE_KEY,
    PASSWORD_SET_KEY,
    VOICE_KEY,    
};

#endif

const s32   MATCH_FLAG_PASSWORD     = (1<<0);
const s32   MATCH_FLAG_VOICE        = (1<<1);
const f32   LOOKUP_RETRY_INTERVAL  = 2.0f;
const s32   MAX_PING_OUTSTANDING   = 32;
const f32   STATE_TIMEOUT          = 10.0f;
const s32   STATE_RETRIES          = 3;
const f32   REGISTRATION_INTERVAL  = 60.0f*4.0f;
const f32   UPDATE_SERVER_INTERVAL = 1.0f;
const s32   MAX_MATCH_RESULTS      = 50;
const s32   BUDDY_SEARCH_LENGTH    = 160;
const f32   USER_UPDATE_TIME       = 10.0f;
const f32   INITIAL_HOLDOFF_TIME   = 0.5f;

// Constants that are different depending on version
#if defined(ENABLE_XBOX_LIVE)
const s32   USER_PASSWORD_LENGTH = XONLINE_PASSCODE_LENGTH;
const s32   USER_NAME_LENGTH     = XONLINE_GAMERTAG_SIZE;
#else
const s32   USER_PASSWORD_LENGTH = 4;
const s32   USER_NAME_LENGTH     = 16;
#endif

enum server_flags
{
    SERVER_HAS_PASSWORD         = (1<<0),       // 0x001
    SERVER_VOICE_ENABLED        = (1<<1),       // 0x002
    SERVER_IS_DEDICATED         = (1<<2),       // 0x004
    SERVER_HAS_BUDDY            = (1<<3),       // 0x008
    SERVER_IS_A_FAVORITE        = (1<<4),       // 0x010
    SERVER_MUTATION_ENABLED     = (1<<5),       // 0x020
    SERVER_ENABLE_TEAM_BALANCE  = (1<<6),       // 0x040
    SERVER_FRIENDLY_FIRE        = (1<<7),       // 0x080
    SERVER_ELIMINATION_MODE     = (1<<8),       // 0x100
    SERVER_ENABLE_MAP_SCALING   = (1<<9),       // 0x200
    SERVER_IS_PRIVATE           = (1<<10),      // 0x400
};

//------------------------------------------------------------------------------
class game_config;

enum connect_type
{
    CONNECT_DIRECT,
    CONNECT_NAT,
    CONNECT_INDIRECT,
};

typedef struct 
{
    player_stats    Stats;
    s32             Checksum;
} player_stats_chk;

struct server_info
{
    s32             ID;
    s32             Retries;
    f32             RetryTimeout;
    net_address     Remote;
    net_address     External;
    s32             Version;
    s32             MaxPlayers;
    s32             Players;
    s32             Level;
    f32             PingDelay;
    xwchar          Name[NET_SERVER_NAME_LENGTH];
    char            Password[NET_PASSWORD_LENGTH];
    xwchar          MissionName[NET_MISSION_NAME_LENGTH];
    xwchar          GameType[NET_GAME_TYPE_LENGTH];
    xwchar          ShortGameType[NET_SHORT_GAME_TYPE_LENGTH];
    s32             GameTypeID;                     // MAB: Game type enumerated type defined in GameMgr.hpp
    s32             FragLimit;
    s32             GameTime;
    s32             EliminationMode;
    mutation_mode   MutationMode;                   // MAB: this is new. was previously an on/off flag
    skill_level     SkillLevel;                     // MAB: new field for skill level
    s32             PercentComplete;                // MAB: please use this field for % game complete
    s32             VotePassPercent;                // MAB: how many percent we need for a vote to pass
    s32             AIType;                         // MAB: enumerated type to specify what types of AI are present
    s32             ScoreLimit;                     // MAB: score limit for this game
    s32             CapCook;                        // MAB: cap cook for this game
    s32             NumRounds;                      // MAB: number of rounds for the match
    s32             ConvertTime;                    // MAB: time to convert in CNH
    s32             FirePercent;                    // MAB: friendly fire percentage
    s32             Flags;
    connect_type    ConnectionType;
    s32             PasswordEnabled;                //klkl: needed for optimatch filtering
    s32             VoiceEnabled;                   //klkl: needed for optimatch filtering

    friend xbool    operator == ( const server_info& Left, const server_info& Right );
    friend xbool    operator != ( const server_info& Left, const server_info& Right );

#if defined(ENABLE_XBOX_LIVE)
    XNADDR          Address;
    XNKID           SessionID;
    XNKEY           ExchangeKey;
    xbool           KeyIsRegistered;
#endif

#if defined(ENABLE_GAMESPY)
    s32             SessionID;
#endif
};

struct llnode
{
    llnode*         pNext;
    server_info*    pServerInfo;
};

struct extended_info
{
    game_score      Score;                          // Has player names and score information
};

//------------------------------------------------------------------------------
enum user_flags
{
    USER_HAS_PASSWORD       = (1<<0),
    USER_VOICE_ENABLED      = (1<<1),
    USER_IS_GUEST           = (1<<2),
    USER_HAS_MESSAGE        = (1<<3),
    USER_HAS_INVITE         = (1<<4),
    USER_SENT_INVITE        = (1<<5),
    USER_INVITE_IGNORED     = (1<<6),
    USER_ACCEPTED_INVITE    = (1<<7),
    USER_REJECTED_INVITE    = (1<<8),
    USER_REQUEST_SENT       = (1<<9),
    USER_REQUEST_RECEIVED   = (1<<10),
    USER_REQUEST_IGNORED    = (1<<11),
    USER_VOICE_MESSAGE      = (1<<12),
    USER_IS_JOINABLE        = (1<<13),
};

enum notification_flags
{
    NOTIFY_FRIEND_REQUEST           =   (1<<0),
    NOTIFY_NEW_GAME_INVITE          =   (1<<1),
    NOTIFY_GAME_INVITE              =   (1<<2),
    NOTIFY_GAME_INVITE_ANSWER       =   (1<<3),
    NOTIFY_BUNCH_OF_MESSAGES        =   (1<<4),
    NOTIFY_VOICE_ATTACHMENT         =   (1<<5),
};

enum stats_flags
{
    STATS_READ_IN_PROGRESS              =   (1<<0),                 //actively reading from Live server
    STATS_READ_READY                    =   (1<<1),                 //finished reading from Live server, user stats not refreshed
    STATS_READ_COMPLETE                 =   (1<<2),                 //user stats refreshed, ready for use
    STATS_READ_PENDING                  =   (1<<3),                 //need to read once throttle timeout has elapsed
    STATS_WRITE_IN_PROGRESS             =   (1<<4),                 //actively writing to Live server
    STATS_WRITE_COMPLETE                =   (1<<5),                 //finished writing
    STATS_WRITE_READY                   =   (1<<6),                 //
    STATS_WRITE_PENDING                 =   (1<<7),                 //need to write once throttle timeout has elapsed
};

enum gamestats
{
    GAMESTAT_KILLS_AS_HUMAN,
    //GAMESTAT_KILLS_AS_MUTANT,
    GAMESTAT_DEATHS,
    GAMESTAT_PLAYTIME,
    GAMESTAT_GAMESPLAYED,
    GAMESTAT_WINS,
    GAMESTAT_FIRST,
    GAMESTAT_SECOND,
    GAMESTAT_THIRD,
    //GAMESTAT_KICKS,
    //GAMESTAT_VOTES,
    GAMESTAT_RATING,        
    GAMESTAT_RANK,              //Note: must be last
    GAMESTAT_MAX
};
//------------------------------------------------------------------------------
/*
struct stats_info
{
    u32     KillsAsHuman;
    u32     KillsAsMutant;
    u32     Deaths;
    u32     PlayTime;   // In minutes.
    u32     Games;      // Only count games completed.
    u32     Wins;       // Only count 'valid' participation on winning team.
    u32     Gold;
    u32     Silver;
    u32     Bronze;
    u32     Kicks;
    u32     VotesStarted;
};
*/
//------------------------------------------------------------------------------
struct online_user
{
    xwchar          Name[USER_NAME_LENGTH];
    byte            Password[USER_PASSWORD_LENGTH];
    s32             Flags;
};

//------------------------------------------------------------------------------
struct lobby_info
{
    s32             ID;
    s32             ServerCount;
    s32             Flags;
    s32             LocaleMatch;
    char            Name[NET_SERVER_NAME_LENGTH];
};

//------------------------------------------------------------------------------

struct net_lookup_request
{
    lookup_type     Type;
    s32             Version;
    s16             PingIndex;
    char            Name[NET_SERVER_NAME_LENGTH]; //** THIS IS TEMPORARY UNTIL PROPER LOOKUPS ARE WORKING
    char            BuddySearchString[BUDDY_SEARCH_LENGTH];           // Single character but will be expanded as needed.
};

//------------------------------------------------------------------------------
struct net_lookup_response
{
    lookup_type     Type;
    s32             Version;
    s16             PingIndex;
    s16             CurrentLevel;
    s8              MaxPlayers;
    s8              CurrentPlayers;
    s8              MutationMode;
    s8              Progress;
    s8              FragLimit;
    s8              TimeLimit;

    xwchar          Name[NET_SERVER_NAME_LENGTH];
    xwchar          MissionName[NET_MISSION_NAME_LENGTH];
    xwchar          GameType[NET_GAME_TYPE_LENGTH];
    xwchar          ShortGameType[NET_SHORT_GAME_TYPE_LENGTH];
    s32             Flags;
#if defined(ENABLE_XBOX_LIVE)
    XNADDR          Address;
    XNKID           SessionID;
    XNKEY           ExchangeKey;
#endif
#if defined(ENABLE_GAMESPY)
    s32             SessionID;
#endif
};

//------------------------------------------------------------------------------
struct net_extended_lookup_request
{
    lookup_type     Type;
    s16             PingIndex;
    s32             Owner;
};

//------------------------------------------------------------------------------
struct net_extended_lookup_response
{   
    lookup_type     Type;
    s16             PingIndex;
    s32             Owner;
    char            TeamName[2][ NET_NAME_LENGTH ];
    s32             TeamScore[2];
    s32             ScoreFieldMask;
    u8              IsTeamBased;
    u8              PlayerCount;
    u8              PlayerTeam[ NET_MAX_PLAYERS ];
    s32             PlayerScore[ NET_MAX_PLAYERS ];
    s32             PlayerDeaths[ NET_MAX_PLAYERS ];
    char            PlayerName[ NET_MAX_PLAYERS ][NET_NAME_LENGTH];
};

//------------------------------------------------------------------------------
struct net_echo_request
{
    lookup_type     Type;
    s32             ServerID;
};

//------------------------------------------------------------------------------
union net_request
{
    net_lookup_response             LookupResponse;
    net_lookup_request              LookupRequest;
    net_extended_lookup_response    ExtendedLookupResponse;
    net_extended_lookup_request     ExtendedLookupRequest;
    net_echo_request                EchoRequest;
};

//------------------------------------------------------------------------------
enum buddy_status
{
    BUDDY_STATUS_OFFLINE,
    BUDDY_STATUS_ONLINE,
    BUDDY_STATUS_INGAME,
    BUDDY_STATUS_ADD_PENDING,
    BUDDY_STATUS_REQUEST_ADD,
    BUDDY_STATUS_JOINABLE,
};

//------------------------------------------------------------------------------
enum buddy_answer
{
    BUDDY_ANSWER_NO,
    BUDDY_ANSWER_YES,
    BUDDY_ANSWER_BLOCK,
    BUDDY_ANSWER_REMOVE = BUDDY_ANSWER_BLOCK,
};

//------------------------------------------------------------------------------
struct buddy_info
{
    char            Name[NET_NAME_LENGTH];
    char            UniqueNick[NET_NAME_LENGTH];
    xwchar          Location[64];
    xwchar          LevelName[32];
    u32             Flags;
    buddy_status    Status;
    net_address     Remote;
#if defined(TARGET_XBOX)
    XNKID           SessionID;
    s32             TitleID;
    DWORD           MessageID;
#endif
    u64             Identifier;
    xbool           operator == ( const buddy_info& Right )
    {
        return( Identifier == Right.Identifier );
    }
};

//------------------------------------------------------------------------------
enum match_buddy_state
{
    MATCH_BUDDY_IDLE,
    MATCH_BUDDY_START_ACQUISITION,
    MATCH_BUDDY_ACQUIRING,
    MATCH_BUDDY_SHUTDOWN,
};

//------------------------------------------------------------------------------
enum match_message_state
{
    MATCH_MESSAGE_IDLE,
    MATCH_MESSAGE_SUCCESS,
    MATCH_MESSAGE_FAILED,
    MATCH_MESSAGE_DETAILS,
    MATCH_MESSAGE_RECEIVE,
    MATCH_MESSAGE_SEND,
};

//------------------------------------------------------------------------------
enum match_mgr_state
{
    MATCH_IDLE,
    MATCH_AUTHENTICATE_MACHINE,
    MATCH_GET_MESSAGES,
    MATCH_SILENT_LOGON,
    MATCH_CONNECT_MATCHMAKER,
    MATCH_AUTHENTICATE_USER,
    MATCH_AUTH_USER_CONNECT,
    MATCH_AUTH_USER_CREATE,
    MATCH_AUTH_DONE,
    MATCH_SECURITY_CHECK,
    MATCH_GET_PATCH,
    MATCH_SAVE_PATCH,
    MATCH_DOWNLOADING,
    MATCH_ACQUIRE_IDLE,
    MATCH_ACQUIRE_SERVERS,
    MATCH_ACQUIRE_LAN_SERVERS,
    MATCH_ACQUIRE_EXTENDED_INFO,
    // PC test
    MATCH_ACQUIRE_BUDDIES,
    MATCH_ACQUIRE_LOBBIES,
    MATCH_ACQUIRE_REGIONS,
    MATCH_ACQUIRE_EXTENDED_SERVER_INFO,
    MATCH_INDIRECT_CONNECT,
    // Start being a server
    MATCH_BECOME_SERVER,
    MATCH_VALIDATE_SERVER_NAME,
    MATCH_VALIDATE_PLAYER_NAME,
    // Registration
    MATCH_UPDATE_SERVER,
    MATCH_REGISTER_SERVER,
    MATCH_SERVER_CHECK_VISIBLE,
    MATCH_SERVER_ACTIVE,
    // Unregister
    MATCH_UNREGISTER_SERVER,
    // Client states
    MATCH_NAT_NEGOTIATE,
    MATCH_LOGIN,
    MATCH_INDIRECT_LOOKUP,
    MATCH_CLIENT_ACTIVE,
};

//------------------------------------------------------------------------------
enum match_acquire
{
    ACQUIRE_INVALID = -1,
    ACQUIRE_SERVERS = 0,
    ACQUIRE_EXTENDED_SERVER_INFO,
};

//------------------------------------------------------------------------------

#ifdef TARGET_XBOX

struct mutelist
{
    enum state 
    {
        MUTELIST_IDLE,
        MUTELIST_GET,
        MUTELIST_UPDATE,
    };

    struct pending 
    {
        u64     Identifier;
        xbool   IsMuted;
    };

    state                   State;
    pending                 Pending[ NET_MAX_PLAYERS ];
    s32                     ReadIndex;
    s32                     WriteIndex;

    XONLINE_MUTELISTUSER    List[ MAX_MUTELISTUSERS ];
    DWORD                   NumUsers;
};

#endif

//------------------------------------------------------------------------------
class match_mgr
{
public:
                            match_mgr                   ( void );
                           ~match_mgr                   ( void );
        xbool               Init                        ( net_socket& Local, const net_address Broadcast );
        void                Kill                        ( void );
        // Returns TRUE if the packet was consumed
        xbool               ReceivePacket               ( net_address& Remote, bitstream& Bitstream );
        xbool               SendLookup                  ( net_address& Remote );
        xbool               IsEnabled                   ( void )                    { return m_pSocket != NULL;         }
        s32                 GetServerCount              ( void );
        server_info*        GetServerInfo               ( s32 Index );
        s32                 FindServerInfo              ( const server_info& Response );
        void                AppendToServerList          ( const server_info& Response );
        void                ResetServerList             ( void );
  const extended_info*      GetExtendedServerInfo       ( s32 Index );             // Returns NULL if the extended information has not been received yet.
        void                Reset                       ( void );
        void                SetState                    ( match_mgr_state Mode );
        match_mgr_state     GetState                    ( void )                    { return m_State;                   }

        xbool               IsRegistered                ( void )                    { return m_RegistrationComplete;    }
        void                RefreshRegistration         ( void )                    { m_UpdateRegistration = TRUE;      }

        match_conn_status   GetConnectStatus            ( void )                    { return m_ConnectStatus;           }
        void                SetConnectStatus            ( match_conn_status Status );
        s32                 GetAuthResult               ( char* pLabelBuffer );
        auth_status         GetAuthStatus               ( void )                    { return m_AuthStatus;              }
        void                SetAuthStatus               ( auth_status Status );
        const char*         GetMessageOfTheDay          ( void );
        void                SetUniqueId                 ( const byte* Id, s32 IdLength);
        const byte*         GetUniqueId                 ( s32& IdLength );
        void                CheckVisibility             ( void );
        xbool               IsVisible                   ( void )                    { return m_IsVisible;               }
        void                NotifyKick                  ( const char* UniqueId );
        void                ShowOnlineStatus            ( xbool IsEnabled );
        
#ifdef TARGET_PC
        void                SetBuddies                  (const char* buddies);
        void                ReleasePatch                ( void );
#endif        

        xbool               ValidateServerInfo          ( const server_info &info );
        xbool               ValidateLobbyInfo           ( const lobby_info &info );
        // User accounts
        const online_user&  GetUserAccount              ( s32 UserIndex );
        void                SetUserAccount              ( s32 UserIndex );
        const online_user&  GetActiveUserAccount        ( void );
        void                SignOut                     ( void );
#ifdef TARGET_XBOX
        const char*         GetUserAccountName          ( s32 UserIndex );
#endif
        s32                 GetUserAccountCount         ( void );
        xbool               IsBusy                      ( void );
        xbool               IsAuthenticated             ( void )                    { return m_IsLoggedOn;              }
        s32                 GetConnectErrorCode         ( void )                    { return m_ConnectErrorCode;        }
        const char*         GetConnectErrorMessage      ( void );
        void                StartAcquisition            ( match_acquire AcquisitionMode );
        xbool               IsAcquireComplete           ( void );
        void                StartIndirectLookup         ( void );
        void                StartLogin                  ( void );
#ifdef TARGET_PC
        void                BecomeClient                ( const server_info& Config );
#else
        void                BecomeClient                ( void );
#endif        
        u64                 GetPlayerIdentifier         ( s32 Index )               { return m_PlayerIdentifier[Index]; }
        const char*         GetNickname                 ( void )                    { return m_Nickname;                }
        void                SetNickname                 ( const char* pName )       { x_strcpy( m_Nickname, pName );    }
        buddy_info&         GetBuddy                    ( s32 Index );
        s32                 GetBuddyCount               ( void );
        void                SetUserStatus               ( buddy_status Status );
        xbool               IsBuddy                     ( const buddy_info& Buddy );
        const buddy_info*   FindBuddy                   ( u64 Identifier );

        xbool               AddBuddy                    ( const buddy_info& Buddy );
        xbool               DeleteBuddy                 ( const buddy_info& Buddy );
        void                AnswerBuddyRequest          ( buddy_info& Info, buddy_answer Answer );

        xbool               InviteBuddy                 ( buddy_info& Info );
        void                CancelBuddyInvite           ( buddy_info& Info );
        xbool               AnswerBuddyInvite           ( buddy_info& Info, buddy_answer Answer );
        xbool               JoinBuddy                   ( buddy_info& Info );
        void                SetFilter                   ( game_type GameType, s32 MinPlayers, s32 MaxPlayers );

        void                LockLists                   ( void );
        void                UnlockLists                 ( void );
        xbool               AreListsLocked              ( void );
        void                LockBrowser                 ( void );
        void                UnlockBrowser               ( void );
        xbool               IsBrowserLocked             ( void );
        void                InitDownload                ( const char* pURL );
        download_status     GetDownloadStatus           ( void );
        f32                 GetDownloadProgress         ( void );
        void                KillDownload                ( void );
        void*               GetDownloadData             ( s32& Length );
        xbool               NewContentAvailable         ( void )                    { return m_HasNewContent;           }
        s32                 GetRecentPlayerCount        ( void );
        buddy_info*         FindRecentPlayer            ( u64 Identifier );
        const buddy_info&   GetRecentPlayer             ( s32 Index );
        void                AddRecentPlayer             ( const player_score& Player );
        void                SetLocalManifestVersion     ( s32 Version );
        void                SetRemoteManifestVersion    ( s32 Version );
        s32                 GetLocalManifestVersion     ( void );
        s32                 GetRemoteManifestVersion    ( void );
        s32                 GetLocalPatchVersion        ( void );
        s32                 GetRemotePatchVersion       ( void );
        void                SetLocalPatchVersion        ( s32 Version );

        xbool               IsPlayerMuted               ( u64   Identifier  );
        void                SetIsPlayerMuted            ( u64   Identifier,
                                                          xbool IsMuted     );
        void                SetFilter               ( game_type GameType, s32 MinPlayers, s32 MaxPlayers, s32 MutationMode,  s32 PasswordEnabled,  s32 VoiceEnabled  );
        void                SendFeedback                ( u64 Identifier, const char* pName, player_feedback Type );   

#if defined(TARGET_XBOX)
        void                Update                      ( f32 DeltaTime );
        void                Init                        ( void );
        void                SetFilter                   ( game_type GameType, ULONGLONG MinPlayers, ULONGLONG MaxPlayers );
        s32                 GetNotificationFlags        ( void ) { return m_RecvNotifications; }
        XONLINE_FRIEND*     Buddy2Friend                ( buddy_info& Info );
        void                SetCrossTitleInvite         ( void );
        xbool               GetStoredGameInvites        ( void );
        void                FindCrossTitleGame          ( void );
        xbool               IsFriendOnline              ( s32 friendIndex );
        xbool               IsFriendPlayingSameGame     ( s32 friendIndex );
        xbool               IsFriendPlayingSameTitleID  ( s32 titleID );
        xbool               GetPendingInviteAccepted    ( s32 PlayerIndex );
        xbool               GetPendingInviteAccepted    ( void ) {return m_PendingInviteAccepted; }
        void                SetPendingInviteAccepted    ( xbool flag ) {m_PendingInviteAccepted = flag; }
        s32                 GetFriendIdx                ( XUID friendxuid );
        void                SetControllerPort           ( s32 controllerPort ) { m_UserIndex = controllerPort; }
        void                ClearAllKeys                ( xbool IncludeConfigs );

        void                UpdateMutelist              ( void );
        void                UpdateUserStatus            ( void );
        void                UpdateTasks                 ( void );
        void                UpdateAcquisition           ( void );
        void                UpdateConnectMatchMaker     ( void );
        void                UpdateSilentLogon           ( void );
        void                UpdateAuthenticateUser      ( void );
        void                UpdateAuthenticateMachine   ( void );
        void                UpdateSecurityCheck         ( void );
        void                UpdateGetMessages           ( void );
        void                UpdateValidatePlayerName    ( void );
        void                UpdateAcquireServers        ( f32 DeltaTime );
        void                UpdateAcquireExtendedInfo   ( f32 DeltaTime );
        void                UpdateBecomeServer          ( void );
        void                UpdateValidateServerName    ( void );
        void                UpdateUpdateServer          ( void );
        void                UpdateRegisterServer        ( void );
        void                UpdateServerCheckVisible    ( void );
        void                UpdateServerActive          ( void );
        void                UpdateUnregisterServer      ( void );
        void                UpdateNATNegotiate          ( void );
        void                UpdateIndirectLookup        ( void );
        void                UpdateLogin                 ( void );
        xbool               UpdateLiveAccounts          ( void );

        void                UpdateNotifications         ( void );
        void                UpdateMessageState          ( void );
        void                UpdateMessageIdle           ( void );  
        void                UpdateMessageDetails        ( void );
        void                UpdateMessageSend           ( void );  
        void                UpdateMessageReceive        ( void );

        void                BuddyIdle                   ( void );
        void                BuddyStartAcquisition       ( void );
        void                BuddyAcquiring              ( void );
        void                BuddyShutdown               ( void );

        void                SetStateConnectMatchMaker   ( void );
        void                SetStateGetMessages         ( void );
        void                SetStateValidatePlayerName  ( void );
        void                SetStateAcquireServers      ( void );
        void                SetStateRegisterServer      ( void );
        void                SetStateUpdateServer        ( void );
        void                SetStateValidateServerName  ( void );
        void                SetStateServerCheckVisible  ( void );
        void                SetStateUnregisterServer    ( void );
        void                SetStateServerActive        ( void );

        void                InitServerChanges           ( void );
        void                LogonConnectionEstablished  ( void );

        void                SetVoiceMessage             ( byte*     pVoiceMessage,
                                                          s32       NumBytes,
                                                          f32       Duration );

        void                ClearVoiceMessage           ( void );
        f32                 GetMessageSendProgress      ( void );
        f32                 GetMessageRecProgress       ( void );
        xbool               StartMessageDownload        ( buddy_info& Buddy );
        void                FreeMessageDownload         ( void );
        xbool               IsDownloadingMessage        ( void );
        xbool               IsSendingMessage            ( void );
        xbool               GetVoiceMessage             ( BYTE**    ppVoiceMessage  = NULL,
                                                          s32*      pNumBytes       = NULL ,
                                                          f32*      pDuration       = NULL );

        void                ClearStatsGotten            (void);
        void                ClearStatsReady             (void);
        xbool               IsUserStatsReady            (void);
        xbool               IsUserStatsGotten           (void);
        void                GetUserStats                (void);
        void                IsStatsBeingWritten         (void);
        void                StartStatsWrite             (DWORD numStatSpecs, const XONLINE_STAT_PROC * pStatProcs);
        void                StartStatsWrite             (void);
        void                ResetStats                  (DWORD numStatSpecs, const XONLINE_STAT_SPEC * pStatSpecs);
        void                StartStatsRead              (DWORD numStatSpecs, const XONLINE_STAT_SPEC * pStatSpecs);
        void                StartStatsRead              (void);
        void                StartDelayedStatsRead       (void);
        void                GetStatsRead                (DWORD numStatSpecs, PXONLINE_STAT_SPEC pStatSpecs, DWORD extraBufferSize, BYTE * pExtraBuffer);
        void                GetStatsRead                (void);
        void                UpdateStatsRead             (void);
        void                UpdateStatsWrite            (void);
        void                StatsShutdown               (void);
        void                StatsCloseCurrentRead       (void);
        void                StatsCloseCurrentWrite      (void);
        void                VerifyServices              (void);
        xbool               IsReadReady                 (void);
        // total career stats
        void                SetCareerStats              (gamestats mode, u32 value);
        u32                 GetCareerStats              (gamestats mode);

        // this game stats
        void                SetGameStats                (gamestats mode, u32 value);
        u32                 GetGameStats                (gamestats mode);

        void                SetAllGameStats             ( const player_stats& Stats );
        void                SetAllCareerStats           ( const player_stats& Stats );
        const player_stats& GetAllGameStats             ( void ) { return m_GameStats; }
        const player_stats& GetAllCareerStats           ( void ) { return m_CareerStats; }

        void                PostGameStatsToLive         ( void );

        XONLINE_LATEST_ACCEPTED_GAMEINVITE&
                            GetInviteAccepted           ( void ) { return m_PendingInvite; }

        xbool               GetInviteAcceptedUsers      ( xwstring& InvitingUser,
                                                          xwstring& AcceptedUser );

        xbool               IsDuplicateLogin            ( void );
        void                ClearDuplicateLogin         ( void );

        void                SetOptionalMessage          ( xbool OptionalMessage );
        xbool               GetOptionalMessage          ( void );

private:
        void                SetupMessage                ( BYTE MessageType );

#endif

#if defined(TARGET_PS2) || defined(TARGET_PC)

enum 
{
    GS_STATS_NOT_CONNECTED = 0,
    GS_STATS_DISCONNECTED,
    GS_STATS_INITIATE_CONNECT,
    GS_STATS_CONNECTING,
    GS_STATS_CONNECTED
};

        void                EnableStatsConnection       ( void );
        void                InitiateStatsConnection     ( void );
        void                InitiateCareerStatsRead     ( void );
        xbool               IsCareerStatsReadComplete   ( void );
        void                InitiateCareerStatsWrite    ( void );
        xbool               IsCareerStatsWriteComplete  ( void );
        void                StatsUpdate                 ( void );

        void                SetAllGameStats             ( const player_stats& Stats );
        void                SetAllCareerStats           ( const player_stats& Stats );
        void                UpdateCareerStatsWithGameStats( void );
        const player_stats& GetAllGameStats             ( void ) { return m_GameStats; }
        const player_stats& GetAllCareerStats           ( void ) { return m_CareerStats.Stats; }
        void                StatsUpdateConnect          ( void );
        void                StatsUpdateRead             ( void );
        void                StatsUpdateWrite            ( void );
#endif // defined(TARGET_PS2) || defined(TARGET_PC)

private:
#if !defined(TARGET_XBOX)
        void                Update                      ( f32 DeltaTime );
#endif
        void                UpdateState                 ( f32 DeltaTime );
        void                AppendServer                ( const net_address& Remote, const net_lookup_response& Response );
        void                AppendServer                ( const server_info& Response );
        void                LocalLookups                ( f32 DeltaTime );
        void                RemoteLookups               ( f32 DeltaTime );
        f32                 GetPingTime                 ( s32 Index );
        void                EnterState                  ( match_mgr_state NewState );
        void                ExitState                   ( match_mgr_state OldState );
        void                SetTimeout                  ( f32 Timeout )             { m_StateTimeout = Timeout;         }
        xbool               HasTimedOut                 ( void )                    { return m_StateTimeout < 0.0f;     }
        void                ConnectToMatchmaker         ( match_mgr_state TargetState );
        void                DisconnectFromMatchmaker    ( void );
        xbool               CheckSecurityChallenge      ( const char* pChallenge );
        void                IssueSecurityChallenge      ( void );
        void                ReadRegions                 ( void );
        void                ReadLobbies                 ( void );
        void                ProcessLookupRequest        ( net_address& Remote, bitstream& BitStream );
        void                MarkBuddyPresent            ( const net_address& Remote );
        void                ApplyPatch                  ( void* pBuffer, s32 Length );
        void                ParseMessageOfTheDay        ( const char* pBuffer );

        xbool               m_Initialized;
        xbool               m_ForceShutdown;
        f32                 m_AccumulatedTime;
        match_mgr_state     m_State;
        match_mgr_state     m_PendingState;
        auth_status         m_AuthStatus;
        match_conn_status   m_ConnectStatus;
        match_buddy_state   m_BuddyState;
        f32                 m_StateTimeout;                 // Set depending on target state
        s32                 m_StateRetries;                 // Will be set to 3 if a state transition occurs

        f32                 m_LookupTimeout;
        net_socket*         m_pSocket;
        net_address         m_BroadcastAddress;

        s32                 m_ServerCount;
        llnode*             m_pListHead;

        xarray<server_info> m_PendingResponseList;
        xarray<lobby_info>  m_LobbyList;
        xarray<online_user> m_UserAccounts;
#ifdef TARGET_PC
        xarray<server_info> m_ServerList;
#endif
        s32                 m_ActiveAccount;
        online_user         m_ActiveUserAccount;
        char                m_Nickname[32];
        u64                 m_PlayerIdentifier[NET_MAX_PER_CLIENT];
        char                m_DownloadFilename[128];         
        byte                m_UniqueId[NET_MAX_ID_LENGTH];
        xbool               m_DNASRunning;
       
        game_type           m_FilterGameType;
#ifdef TARGET_XBOX
        ULONGLONG           m_FilterMinPlayers;
        ULONGLONG           m_FilterMaxPlayers;
        ULONGLONG           m_FilterMutationMode;
        ULONGLONG           m_FilterPassword;
        ULONGLONG           m_FilterHeadset;


        s32                 m_StatsStatus;
        f32                 m_StatsReadThrottleTime;
        f32                 m_StatsStartTime;
        XONLINE_STAT        m_Stats[GAMESTAT_MAX];
        player_stats        m_GameStats;
        player_stats        m_CareerStats;
#else
        s32                 m_FilterMinPlayers;
        s32                 m_FilterMaxPlayers;
        s32                 m_FilterMutationMode;
        s32                 m_FilterPassword;
        s32                 m_FilterHeadset;
public:
        player_stats        m_GameStats;
        player_stats_chk    m_CareerStats;
        s32                 m_StatsState;
        xbool               m_bIsReading;
        xbool               m_bReadComplete;
        xbool               m_bIsWriting;
        xbool               m_bWriteComplete;
        s32                 m_ConnectCount;
        s32                 m_ReadCount;
        s32                 m_WriteCount;
private:
#endif
        buddy_status        m_PendingUserStatus;
        buddy_status        m_UserStatus;
        xbool               m_ShowOnlineStatus;
        xbool               m_LastShowOnlineStatus;

        xarray<buddy_info>  m_Buddies;
         
        xtick               m_PingSendTimes[MAX_PING_OUTSTANDING];
        s32                 m_PingIndex;
        xbool               m_NeedNewPing;
        xbool               m_NeedToNotifyBan;
        random              m_Random;

        xbool               m_LocalIsServer;
        xbool               m_RegistrationComplete;
        xbool               m_UpdateRegistration;
        xbool               m_HasNewContent;
        s32                 m_ExtendedServerInfoOwner;
        extended_info       m_ExtendedServerInfo;

        xbool               m_SecurityChallengeReceived;
        char*               m_pSecurityChallenge;
        xstring             m_MessageOfTheDay;
        xbool               m_IsLoggedOn;
        xbool               m_IsVisible;
        xbool               m_StartedConnect;
        char                m_ConnectErrorMessage[64];
        s32                 m_ConnectErrorCode;
        match_acquire       m_AcquisitionMode;
        match_acquire       m_PendingAcquisitionMode;
        xthread*            m_pThread;
        xmutex              m_ListMutex;
        xmutex              m_BrowserMutex;
        downloader*         m_pDownloader;
        s32                 m_LocalManifestVersion;
        s32                 m_RemoteManifestVersion;
        s32                 m_RemotePatchVersion;
        s32                 m_LocalPatchVersion;
        s32                 m_UpdateInterval;
        xarray<buddy_info>  m_RecentPlayers;
        xbool               m_IsVoiceCapable;
        
#ifdef TARGET_PC
        char*               m_pMessageOfTheDay;
        char*               m_pMessageOfTheDayBuffer;
        char                m_BuddyList[100];
#endif        

#if defined(ENABLE_GAMESPY)
        xbool               m_MessageOfTheDayReceived;
        ServerBrowser       m_pBrowser;
        GPConnection        m_Presence;
        s32                 m_SessionID;

#define FRIEND friend
#include "NetworkMgr/GameSpy/CallbackPrototypes.hpp"
#undef FRIEND

#endif

#if defined(ENABLE_XBOX_LIVE)
        XONLINETASK_HANDLE  m_GenericTaskHandle;
        XONLINETASK_HANDLE  m_LogonTaskHandle;
        XONLINETASK_HANDLE  m_SearchTaskHandle;
        XONLINETASK_HANDLE  m_SessionTaskHandle;
        XONLINETASK_HANDLE  m_BuddyTaskHandle;
        XONLINETASK_HANDLE  m_BuddyEnumerateHandle;
        XONLINETASK_HANDLE  m_InviteTaskHandle;
        XONLINETASK_HANDLE  m_FriendRequestTaskHandle;
        XONLINETASK_HANDLE  m_UpdateTaskHandle;
        XONLINETASK_HANDLE  m_MessageTaskHandle;
        XONLINETASK_HANDLE  m_DownloadTaskHandle;
        XONLINETASK_HANDLE  m_MutelistTaskHandle;
        XONLINETASK_HANDLE  m_MutelistEnumerateHandle;
        XONLINETASK_HANDLE  m_ReadStatsHandle;
        XONLINETASK_HANDLE  m_WriteStatsHandle;
        XONLINETASK_HANDLE  m_FeedbackHandle;

        HRESULT             m_GenericTaskResult;
        HRESULT             m_LogonTaskResult;
        HRESULT             m_SearchTaskResult;
        HRESULT             m_SessionTaskResult;
        HRESULT             m_BuddyTaskResult;
        HRESULT             m_BuddyEnumerateResult;
        HRESULT             m_InviteTaskResult;
        HRESULT             m_FriendRequestResult;
        HRESULT             m_UpdateTaskResult;
        HRESULT             m_MessageTaskResult;
        HRESULT             m_DownloadTaskResult;
        HRESULT             m_FeedbackResult;
        HRESULT             m_MutelistTaskResult;
        HRESULT             m_MutelistEnumerateResult;
        HRESULT             m_ReadStatsResult;
        HRESULT             m_WriteStatsResult;

        XONLINE_ATTRIBUTE   m_Attributes[ATTR_KEY_COUNT];
        XONLINE_ATTRIBUTE   m_QueryAttributes[ATTR_QUERY_KEY_COUNT];
        XONLINE_USER        m_UserList[XONLINE_MAX_STORED_ONLINE_USERS];
        DWORD               m_NumUsers;
        XNKEY               m_KeyExchangeKey;

        s32                 m_KeysRegistered;
        XONLINE_FRIEND*     m_pFriendBuffer;
        xbool               m_bAcquisitionComplete;
        XONLINE_MSG_SUMMARY m_MsgSummary;
        s32                 m_RecvNotifications;
        DWORD               m_nFriendCount;
        s32                 m_UserIndex;            //this is the controller (zero based) used to enter Live
        xbool               m_PendingInviteAccepted;
        xbool               m_PendingAcceptInvitationTest;
        xbool               m_CrossTitleInvite;
        XONLINE_LATEST_ACCEPTED_GAMEINVITE  m_PendingInvite;    //pending invite
        XONLINE_ACCEPTED_GAMEINVITE         m_LastInvite;       //cross title invite

        byte*               m_pVoiceMessageSend;
        byte*               m_pVoiceMessageRec;
        s32                 m_VoiceMessageNumBytes;
        f32                 m_VoiceMessageDuration;
        xbool               m_VoiceMessageReceived;
        XONLINE_MSG_HANDLE  m_hMessage;
        match_message_state m_MessageState;

        mutelist            m_Mutelist;

        xbool               m_IsDuplicateLogin;
        xbool               m_OptionalMessage;

#endif

        friend void         s_MatchPeriodicUpdater( s32, char** );
        friend class        game_config;
};

//=============================================================================

extern match_mgr g_MatchMgr;

const char* GetStatusName   ( buddy_status      Status  );
const char* GetStateName    ( match_mgr_state   State   );
const char* GetStatusName   ( match_conn_status Status  );

//=============================================================================
#endif
//=============================================================================
