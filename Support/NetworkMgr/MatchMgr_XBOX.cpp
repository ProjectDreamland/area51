//==============================================================================
//
// MatchMgr.cpp - Matchup manager interface functions.
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

#include "x_files.hpp"
#include "x_threads.hpp"
#include "e_Network.hpp"
#include "NetworkMgr/MatchMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "Voice/VoiceMgr.hpp"
#include "Configuration/GameConfig.hpp"
#include "ServerVersion.hpp"
#include "StateMgr/StateMgr.hpp"
#include "x_log.hpp"

xbool g_EnableLANLookup = FALSE;

//=============================================================================

XONLINE_ATTRIBUTE_SPEC AttributeSpec[ ATTR_KEY_COUNT ]=
{
    { X_ATTRIBUTE_DATATYPE_STRING,  sizeof( xwchar ) * NET_SERVER_NAME_LENGTH       },  // HostName
    { X_ATTRIBUTE_DATATYPE_STRING,  sizeof( xwchar ) * NET_MISSION_NAME_LENGTH      },  // MapName
    { X_ATTRIBUTE_DATATYPE_STRING,  sizeof( xwchar ) * NET_GAME_TYPE_LENGTH         },  // GameType
    { X_ATTRIBUTE_DATATYPE_STRING,  sizeof( xwchar ) * NET_SHORT_GAME_TYPE_LENGTH   },  // ShortGameType
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // GameVersion
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // Flags
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // GameTime
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // FragLimit
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // Port
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // GameType Id
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // Min Players
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // Mutation mode
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // Password 
    { X_ATTRIBUTE_DATATYPE_INTEGER, sizeof( u64 )                                   },  // Headset
};

static DWORD s_LogonServiceIds[]=
{
    XONLINE_ARBITRATION_SERVICE,
        XONLINE_CONTENT_AVAILABLE_SERVICE,
        XONLINE_FEEDBACK_SERVICE,
        XONLINE_MATCHMAKING_SERVICE,
        XONLINE_MESSAGING_SERVICE,
        XONLINE_NAT_TYPE_DETECTION_SERVICE,
        XONLINE_QUERY_SERVICE,
        XONLINE_SIGNATURE_SERVICE,
        XONLINE_STATISTICS_SERVICE, 
        XONLINE_STRING_SERVICE
};

static const DWORD numServices = sizeof( s_LogonServiceIds ) / sizeof( DWORD );

#if defined(X_LOGGING) && !defined(X_SUPPRESS_LOGS)
const char* SessionIDString( const XNKID& SessionID )
{
    static char KeyBuffer[128];
    
    x_sprintf( KeyBuffer, "0x%08x:%08x", *(s32*)&SessionID.ab[0], *(s32*)&SessionID.ab[4] );
    return KeyBuffer;

}
#else
#define SessionIDString(a)  ""
#endif

//
// DIRTY DIRTY DIRTY! We cannot properly check to see if the memory units have
// been inserted or removed so that is done from the xbox input code. This flag
// gets set when an insertion or removal occurs and gets reset once we have
// acquired all the accounts.
extern xbool g_NeedAccountEnumeration;
s32 RegisteredKeyCount=0;

//=============================================================================

struct s_query_result
{
    xwchar  Name            [ NET_SERVER_NAME_LENGTH     ];
    xwchar  MapName         [ NET_MISSION_NAME_LENGTH    ];
    xwchar  GameType        [ NET_GAME_TYPE_LENGTH       ];
    xwchar  ShortGameType   [ NET_SHORT_GAME_TYPE_LENGTH ];
    u64     Version;
    u64     Flags;
    u64     GameTime;
    u64     FragLimit;
    u64     Port;
    u64     GameTypeId;
    u64     MinPlayers;
    u64     MutationMode;
    u64     Headset;

};

//=============================================================================

#ifdef X_LOGGING
static void LogWhichStat( const char* pChannel, gamestats mode, u32 value )
{
    const char* pLabel = "UNKNOWN";
    switch (mode)
    {
        case GAMESTAT_KILLS_AS_HUMAN:  pLabel = "KILLS_AS_HUMAN";  break;
        // rmb-stats case GAMESTAT_KILLS_AS_MUTANT: pLabel = "KILLS_AS_MUTANT"; break;
        case GAMESTAT_DEATHS:          pLabel = "DEATHS";          break;
        case GAMESTAT_PLAYTIME:        pLabel = "PLAYTIME";        break;
        case GAMESTAT_GAMESPLAYED:     pLabel = "GAMESPLAYED";     break;
        case GAMESTAT_WINS:            pLabel = "WINS";            break;
        case GAMESTAT_FIRST:           pLabel = "FIRST";           break;
        case GAMESTAT_SECOND:          pLabel = "SECOND";          break;
        case GAMESTAT_THIRD:           pLabel = "THIRD";           break;
        // rmb-stats case GAMESTAT_KICKS:           pLabel = "KICKS";           break;
        // rmb-stats case GAMESTAT_VOTES:           pLabel = "VOTES";           break;
        default:
            break;
    }
    LOG_MESSAGE( pChannel, "GAMESTAT_%s %d",pLabel, value );
}
#endif // X_LOGGING

//=============================================================================
// This "version" of Init now does practically nothing. The early init process
// which uses Init(void), may end up putting the matchmgr in to a state where
// it will wait for silent sign in.
xbool match_mgr::Init( net_socket& Local, const net_address Broadcast )
{
    m_pSocket                   = &Local;
    m_BroadcastAddress          = Broadcast;
    m_Initialized               = TRUE;
    return( GetAuthStatus() == AUTH_STAT_CONNECTED );
}

//=============================================================================

void match_mgr::Init(void)
{
    m_pSocket                   = NULL;
    m_LocalIsServer             = FALSE;
    m_ShowOnlineStatus          = TRUE;
    m_LastShowOnlineStatus      = TRUE;
    m_AccumulatedTime           = 0.0f;
    m_IsLoggedOn                = FALSE;
    m_KeysRegistered            = 0;
    m_UserStatus                = BUDDY_STATUS_OFFLINE;
    m_PendingUserStatus         = BUDDY_STATUS_OFFLINE;
    m_PendingAcquisitionMode    = ACQUIRE_INVALID;
    m_BuddyState                = MATCH_BUDDY_IDLE;
    m_MessageState              = MATCH_MESSAGE_IDLE;
    m_ExtendedServerInfoOwner   = -1;
    m_GenericTaskHandle         = NULL;
    m_LogonTaskHandle           = NULL;
    m_SearchTaskHandle          = NULL;
    m_SessionTaskHandle         = NULL;
    m_BuddyTaskHandle           = NULL;
    m_BuddyEnumerateHandle      = NULL;
    m_InviteTaskHandle          = NULL;
    m_FriendRequestTaskHandle   = NULL;
    m_UpdateTaskHandle          = NULL;
    m_MessageTaskHandle         = NULL;
    m_DownloadTaskHandle        = NULL;
    m_MutelistTaskHandle        = NULL;
    m_MutelistEnumerateHandle   = NULL;
    m_pFriendBuffer             = NULL;
    m_UpdateRegistration        = FALSE;
    m_pVoiceMessageSend         = NULL;
    m_pVoiceMessageRec          = NULL;
    m_hMessage                  = NULL;
    m_ReadStatsHandle            = NULL;
    m_WriteStatsHandle           = NULL;
    m_FeedbackHandle            = NULL;
    m_IsVoiceCapable            = FALSE;
    m_State                     = MATCH_IDLE;
    m_AuthStatus                = AUTH_STAT_DISCONNECTED;
    m_ConnectStatus             = MATCH_CONN_UNAVAILABLE;
    m_StatsStatus               = 0;
    m_StatsReadThrottleTime     = 0.0f;

    m_Mutelist.State            = mutelist::MUTELIST_IDLE;
    m_Mutelist.NumUsers         = 0;
    m_Mutelist.ReadIndex        = 0;
    m_Mutelist.WriteIndex       = 0;

    x_memset( &m_Mutelist.Pending, 0, sizeof( m_Mutelist.Pending ) );

    net_Init();
    XONLINE_STARTUP_PARAMS Params =
    {
        0,
    };

    s32 Status = XOnlineStartup( &Params );
    ASSERT( Status == S_OK );

    m_IsDuplicateLogin              = FALSE;
    m_OptionalMessage               = FALSE;
    m_bAcquisitionComplete          = TRUE;
    m_RecvNotifications             = 0;
    m_nFriendCount                  = 0;
    m_UserIndex                     = 0;

    // Get any stored game invitations from cross-title invites
    m_PendingInviteAccepted = (XOnlineGameInviteGetLatestAccepted( &m_PendingInvite ) == S_OK);

    memset(&m_CareerStats,0,sizeof(m_CareerStats));
    memset(&m_Stats,0,sizeof(m_Stats));
    memset(&m_GameStats,0,sizeof(m_GameStats));

    g_NeedAccountEnumeration = TRUE;
    // Start Silent Sign-In

    if( g_EnableLANLookup == FALSE )
    {
        SetState( MATCH_SILENT_LOGON );
    }
}

//=============================================================================
void match_mgr::SignOut( void )
{
    if( m_LogonTaskHandle )
    {
        LOG_MESSAGE( "match_mgr::SignOut", "Closing logon task handle 0x%08x", m_LogonTaskHandle );
        XOnlineTaskClose( m_LogonTaskHandle );
        m_LogonTaskHandle = NULL;
        m_LogonTaskResult = S_OK;
        SetAuthStatus( AUTH_STAT_DISCONNECTED );
    }
    m_StatsStatus = 0;
    LOG_MESSAGE( "match_mgr::SignOut", "User has been signed out." );
    SetUserStatus( BUDDY_STATUS_OFFLINE );
    m_RecentPlayers.Clear();
}

//=============================================================================

void  match_mgr::Kill( void )
{
    if( m_Initialized )
    {
        if( GetState()==MATCH_SILENT_LOGON )
        {
            SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
        }
        else 
        {
            SetAuthStatus( AUTH_STAT_DISCONNECTED );
        }
        
        if( GetState() != MATCH_IDLE )
        {
            DisconnectFromMatchmaker();
        }
        SetState( MATCH_IDLE );
    }
    m_Initialized = FALSE;
    m_IsLoggedOn = FALSE;

    if( m_GenericTaskHandle )       XOnlineTaskClose( m_GenericTaskHandle );
    if( m_SearchTaskHandle )        XOnlineTaskClose( m_SearchTaskHandle );
    if( m_SessionTaskHandle )       XOnlineTaskClose( m_SessionTaskHandle );
    if( m_BuddyTaskHandle )         XOnlineTaskClose( m_BuddyTaskHandle );
    if( m_BuddyEnumerateHandle )    XOnlineTaskClose( m_BuddyEnumerateHandle );
    if( m_InviteTaskHandle )        XOnlineTaskClose( m_InviteTaskHandle );
    if( m_FriendRequestTaskHandle ) XOnlineTaskClose( m_FriendRequestTaskHandle );
    if( m_UpdateTaskHandle )        XOnlineTaskClose( m_UpdateTaskHandle );
    if( m_MessageTaskHandle )       XOnlineTaskClose( m_MessageTaskHandle );
    if( m_DownloadTaskHandle )      XOnlineTaskClose( m_DownloadTaskHandle );
    if( m_MutelistTaskHandle )      XOnlineTaskClose( m_MutelistTaskHandle );
    if( m_MutelistEnumerateHandle ) XOnlineTaskClose( m_MutelistEnumerateHandle );
    if( m_FeedbackHandle )          XOnlineTaskClose( m_FeedbackHandle );

    // Don't close the task handle if we've got an invalid user account, on reboot.
    if( m_LogonTaskHandle && (m_LogonTaskResult!=XONLINE_E_LOGON_INVALID_USER) )
    {
        XOnlineTaskClose( m_LogonTaskHandle );
    }

    m_GenericTaskHandle         = NULL;
    m_LogonTaskHandle           = NULL;
    m_SearchTaskHandle          = NULL;
    m_SessionTaskHandle         = NULL;
    m_BuddyTaskHandle           = NULL;
    m_BuddyEnumerateHandle      = NULL;
    m_InviteTaskHandle          = NULL;
    m_FriendRequestTaskHandle   = NULL;
    m_UpdateTaskHandle          = NULL;
    m_MessageTaskHandle         = NULL;
    m_DownloadTaskHandle        = NULL;
    m_MutelistTaskHandle        = NULL;
    m_MutelistEnumerateHandle   = NULL;
    m_FeedbackHandle            = NULL;

    m_Mutelist.State = mutelist::MUTELIST_IDLE;
    ClearAllKeys( TRUE );
    SetConnectStatus( MATCH_CONN_UNAVAILABLE );
    if( m_pFriendBuffer != NULL )
    {
        x_free( m_pFriendBuffer );
        m_pFriendBuffer = NULL;
    }
}

//=============================================================================

void match_mgr::Reset( void )
{
    LOG_MESSAGE( "match_mgr", "Reset" );

    // Reset everything, get rid of all the servers in the server list and
    // re-initiate the acquisition
    if( !m_pSocket )
        return;


    m_UserAccounts.Clear();
    ClearAllKeys( TRUE );
    m_LobbyList.Clear();
    m_PendingResponseList.Clear();
    m_ExtendedServerInfoOwner = -1;
}

//=============================================================================

void  match_mgr::Update( f32 DeltaTime )
{
    m_AccumulatedTime += DeltaTime;
    m_NeedNewPing      = TRUE;

    if( m_StatsReadThrottleTime > 0.0f )
    {
        m_StatsReadThrottleTime -= DeltaTime;
    }
    UpdateAcquisition();
    UpdateTasks();
    UpdateState( DeltaTime );
    UpdateNotifications();
    UpdateMessageState();
    UpdateUserStatus();
    UpdateMutelist();

    if( m_LogonTaskHandle && FAILED(m_LogonTaskResult) && (GetAuthStatus()==AUTH_STAT_CONNECTED) )
    {
        if( m_LogonTaskResult==XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON )
        {
            m_IsDuplicateLogin = TRUE;

            //g_ActiveConfig.SetExitReason( GAME_EXIT_DUPLICATE_LOGIN );
        }
        else
        {
            if( g_NetworkMgr.IsOnline() )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_NETWORK_DOWN );
            }
        }
        if( (GetAuthStatus()==AUTH_STAT_CONNECTING)||(GetAuthStatus()==AUTH_STAT_CONNECTED) )
        {
            SetAuthStatus( AUTH_STAT_DISCONNECTED );
        }
        SetState( MATCH_IDLE );
        if( m_LogonTaskResult!=XONLINE_E_LOGON_INVALID_USER )
        {
            XOnlineTaskClose( m_LogonTaskHandle );
            m_LogonTaskHandle = NULL;
        }
    }
}

//=============================================================================

void match_mgr::UpdateTasks( void )
{
    if( m_LogonTaskHandle )         m_LogonTaskResult           = XOnlineTaskContinue( m_LogonTaskHandle );
    if( m_BuddyTaskHandle )         m_BuddyTaskResult           = XOnlineTaskContinue( m_BuddyTaskHandle );
    if( m_SessionTaskHandle )       m_SessionTaskResult         = XOnlineTaskContinue( m_SessionTaskHandle );
    if( m_UpdateTaskHandle )        m_UpdateTaskResult          = XOnlineTaskContinue( m_UpdateTaskHandle );
    if( m_BuddyEnumerateHandle )    m_BuddyEnumerateResult      = XOnlineTaskContinue( m_BuddyEnumerateHandle );
    if( m_SearchTaskHandle )        m_SearchTaskResult          = XOnlineTaskContinue( m_SearchTaskHandle );
    if( m_InviteTaskHandle )        m_InviteTaskResult          = XOnlineTaskContinue( m_InviteTaskHandle );
    if( m_FriendRequestTaskHandle ) m_FriendRequestResult       = XOnlineTaskContinue( m_FriendRequestTaskHandle );
    if( m_MessageTaskHandle )       m_MessageTaskResult         = XOnlineTaskContinue( m_MessageTaskHandle );
    if( m_DownloadTaskHandle )      m_DownloadTaskResult        = XOnlineTaskContinue( m_DownloadTaskHandle );
    if( m_MutelistTaskHandle )      m_MutelistTaskResult        = XOnlineTaskContinue( m_MutelistTaskHandle );
    if( m_MutelistEnumerateHandle ) m_MutelistEnumerateResult   = XOnlineTaskContinue( m_MutelistEnumerateHandle );
    if( m_FeedbackHandle )          m_FeedbackResult            = XOnlineTaskContinue( m_FeedbackHandle );

    interface_info Info;
    net_GetInterfaceInfo( -1, Info );
    if( Info.IsAvailable==FALSE )
    {
        Kill();
        return;
    }
    UpdateStatsRead();
    UpdateStatsWrite();
}

//=============================================================================

void match_mgr::UpdateAcquisition( void )
{
    if( (m_PendingAcquisitionMode != ACQUIRE_INVALID) && ((m_State == MATCH_IDLE)||(m_State == MATCH_ACQUIRE_IDLE)) )
    {
        m_AcquisitionMode        = m_PendingAcquisitionMode;
        m_PendingAcquisitionMode = ACQUIRE_INVALID;

        switch( m_AcquisitionMode )
        {
        case ACQUIRE_SERVERS :
            SetState( MATCH_ACQUIRE_SERVERS );
            break;

        case ACQUIRE_EXTENDED_SERVER_INFO :
            SetState( MATCH_ACQUIRE_EXTENDED_INFO );
            break;

        default :
            ASSERT( FALSE );
        }
    }
}

//=============================================================================

void match_mgr::UpdateState( f32 DeltaTime )
{
    if( m_StateTimeout >= 0.0f )
    {
        m_StateTimeout -= DeltaTime;
    }

    switch( m_State )
    {
    case MATCH_IDLE                           :                                             break;
    case MATCH_CONNECT_MATCHMAKER             : UpdateConnectMatchMaker();                  break;
    case MATCH_SILENT_LOGON                   : UpdateSilentLogon();                        break;
    case MATCH_AUTHENTICATE_USER              : UpdateAuthenticateUser();                   break;
    case MATCH_AUTHENTICATE_MACHINE           : UpdateAuthenticateMachine();                break;
    case MATCH_SECURITY_CHECK                 : UpdateSecurityCheck();                      break;
    case MATCH_GET_MESSAGES                   : UpdateGetMessages();                        break;
    case MATCH_VALIDATE_PLAYER_NAME           : UpdateValidatePlayerName();                 break;
    case MATCH_INDIRECT_LOOKUP                : UpdateIndirectLookup();                     break;
    case MATCH_ACQUIRE_SERVERS                :
    case MATCH_ACQUIRE_IDLE                   : UpdateAcquireServers( DeltaTime );          break;
    case MATCH_ACQUIRE_EXTENDED_INFO          : UpdateAcquireExtendedInfo( DeltaTime );     break;
    case MATCH_BECOME_SERVER                  : UpdateBecomeServer();                       break;
    case MATCH_VALIDATE_SERVER_NAME           : UpdateValidateServerName();                 break;
    case MATCH_UPDATE_SERVER                  : UpdateUpdateServer();                       break;
    case MATCH_REGISTER_SERVER                : UpdateRegisterServer();                     break;
    case MATCH_SERVER_CHECK_VISIBLE           : UpdateServerCheckVisible();                 break;
    case MATCH_SERVER_ACTIVE                  : UpdateServerActive();                       break;
    case MATCH_UNREGISTER_SERVER              : UpdateUnregisterServer();                   break;
    case MATCH_NAT_NEGOTIATE                  : UpdateNATNegotiate();                       break;
    case MATCH_LOGIN                          : UpdateLogin();                              break;
    case MATCH_CLIENT_ACTIVE                  :                                             break;
    default                                   :                                             break;
    }

    switch( m_BuddyState )
    {
    case MATCH_BUDDY_IDLE               : BuddyIdle();                                      break;
    case MATCH_BUDDY_START_ACQUISITION  : BuddyStartAcquisition();                          break;
    case MATCH_BUDDY_ACQUIRING          : BuddyAcquiring();                                 break;
    case MATCH_BUDDY_SHUTDOWN           : BuddyShutdown();                                  break;
    default                             : ASSERT( FALSE );                                  break;
    }
}

//=============================================================================

xbool match_mgr::UpdateLiveAccounts( void )
{
    HRESULT EnumerationStatus;

    // Wait until all devices have been enumerated after insertion/removal
    EnumerationStatus = XGetDeviceEnumerationStatus();
    if( (EnumerationStatus == XDEVICE_ENUMERATION_BUSY) || (g_NeedAccountEnumeration==FALSE) )
    {
        return( FALSE );
    }

    g_NeedAccountEnumeration = FALSE;

    HRESULT Result = XOnlineGetUsers( m_UserList, &m_NumUsers );

    if( Result == S_OK )
    {
        m_UserAccounts.Clear();
        for( s32 i=0; i < (s32)m_NumUsers; i++ )
        {
            online_user& Profile = m_UserAccounts.Append();
            // Convert an XBOX user profile to internal user profile
            x_memset ( &Profile, 0, sizeof(Profile) );
            x_mstrcpy( Profile.Name, m_UserList[i].szGamertag );
            x_memcpy ( Profile.Password, m_UserList[i].passcode, sizeof( Profile.Password ) );
            Profile.Flags = 0;
            if( (m_UserList[i].dwUserOptions     & XONLINE_USER_OPTION_REQUIRE_PASSCODE) ) Profile.Flags |= USER_HAS_PASSWORD;
            if( (m_UserList[i].xuid.dwUserFlags  & XONLINE_USER_VOICE_NOT_ALLOWED) == 0 )  Profile.Flags |= USER_VOICE_ENABLED;
            if( (m_UserList[i].xuid.dwUserFlags  & XONLINE_USER_GUEST_MASK) )              Profile.Flags |= USER_IS_GUEST;
        }
    }

    return( TRUE );
}

//=============================================================================

void match_mgr::UpdateConnectMatchMaker( void )
{
    // Don't do anything while the devices are being enumerated
    if( UpdateLiveAccounts() == FALSE )
        return;

    // Check for no accounts
    if( m_UserAccounts.GetCount() == 0 )
    {
        SetAuthStatus( AUTH_STAT_SELECT_USER );
        SetState( MATCH_IDLE );
        return;
    }

    {
        // This should pick out the auto-signin user if needed.
        m_ActiveUserAccount = m_UserAccounts[m_ActiveAccount];
        SetUniqueId( (const byte*)(m_ActiveUserAccount.Name), (USER_NAME_LENGTH * sizeof( xwchar )) );
        LOG_MESSAGE( "match_mgr::UpdateConnectMatchMaker", " Active user account name set to %s", (const char*)xstring(m_ActiveUserAccount.Name) );
    }

    if( m_LogonTaskHandle==NULL )
    {
        LOG_MESSAGE( "match_mgr::UpdateConnectMatchMaker", "Logon task handle was NULL, selecting user" );
        SetAuthStatus( AUTH_STAT_SELECT_USER );
        SetState( MATCH_IDLE );
    }
    else
    {
        LOG_MESSAGE( "match_mgr::UpdateConnectMatchMaker", "Logon task was active. Skipping user select." );
        SetState( MATCH_SECURITY_CHECK );
    }
}

//=============================================================================

void match_mgr::UpdateSilentLogon( void )
{
    interface_info Info;

    net_GetInterfaceInfo( -1, Info );
#if 0
    if( Info.IsAvailable==FALSE )
    {
        if( m_LogonTaskHandle )
        {
            XOnlineTaskClose( m_LogonTaskHandle );
            m_LogonTaskHandle = NULL;
            SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
        }
        else
        {
            SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
        }
        SetState( MATCH_IDLE );
        return;
    }
#endif
    // If we have a network address, then we need to start initializing the silent login process
    if( Info.Address!=0 )
    {
        net_socket& LocalSocket = g_NetworkMgr.GetSocket();
        s32 m_Status = LocalSocket.Bind( NET_GAME_PORT, NET_FLAGS_BROADCAST );
        net_address     Broadcast;
        Broadcast = net_address( Info.Broadcast, LocalSocket.GetPort() );
        Init( LocalSocket, Broadcast );
    }
    else
    {
        if( HasTimedOut() )
        {
            SetAuthStatus( AUTH_STAT_DISCONNECTED );
            SetState( MATCH_IDLE );
            return;
        }
    }

    {
        // Login with the selected user
        s32 Status = XOnlineSilentLogon( s_LogonServiceIds, numServices, 0, &m_LogonTaskHandle );
        LOG_MESSAGE( "match_mgr::UpdateSilentLogon", "Started Silent Logon. Status:0x%08x, TaskHandle:0x%08x", Status, m_LogonTaskHandle );

        // The logon process is starting so default to no optional message until we know differently
        SetOptionalMessage( FALSE );

        switch( Status )
        {
        case S_OK:
            SetAuthStatus( AUTH_STAT_CONNECTING);
            break;
        case XONLINE_E_SILENT_LOGON_DISABLED:
            SetAuthStatus( AUTH_STAT_DISCONNECTED );
            break;

        case XONLINE_E_LOGON_NOT_LOGGED_ON:
            SetAuthStatus( AUTH_STAT_DISCONNECTED );
            break;
        case XONLINE_E_SILENT_LOGON_NO_ACCOUNTS:
            SetAuthStatus( AUTH_STAT_NO_ACCOUNT );
            break;
        case XONLINE_E_SILENT_LOGON_PASSCODE_REQUIRED:
            SetAuthStatus( AUTH_STAT_NEED_PASSWORD );
            break;
        case XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT:
        case XONLINE_E_LOGON_NO_NETWORK_CONNECTION:
            SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
            break;
        case XONLINE_E_LOGON_SERVERS_TOO_BUSY:
            SetAuthStatus( AUTH_STAT_SERVER_BUSY );
            break;
        default:
            ASSERT( FALSE );
            SetAuthStatus( AUTH_STAT_DISCONNECTED );
            break;
        }

        if( Status == S_OK )
        {
            m_LogonTaskResult = XOnlineTaskContinue( m_LogonTaskHandle );
            SetState( MATCH_SECURITY_CHECK );
        }
        else
        {
            SetState( MATCH_IDLE );
        }
    }
}

//=============================================================================

void match_mgr::UpdateAuthenticateUser( void )
{
    if( g_EnableLANLookup == TRUE )
    {
        m_IsLoggedOn = TRUE;
        SetUserStatus( BUDDY_STATUS_ONLINE );
        SetAuthStatus( AUTH_STAT_CONNECTED );
        SetState( MATCH_IDLE );
    }
    else
    {
        if( m_LogonTaskHandle )
        {
            LOG_MESSAGE( "match_mgr::UpdateAuthenticateUser", "Closed logon task handle 0x%08x", m_LogonTaskHandle );
            XOnlineTaskClose( m_LogonTaskHandle );
            m_LogonTaskHandle = NULL;
            m_IsLoggedOn = FALSE;
        }

        HRESULT Status;
        
        XONLINE_USER Users[XONLINE_MAX_LOGON_USERS];

        x_memset( Users, 0, sizeof(Users) );
        Users[0] = m_UserList[m_ActiveAccount];
        Status = XOnlineLogon( Users, s_LogonServiceIds, numServices, NULL, &m_LogonTaskHandle );
        LOG_MESSAGE( "match_mgr::UpdateAuthenticateUser", "Logging on user %s, result:0x%08x, TaskHandle:0x%08x\n", m_UserList[m_ActiveAccount].szGamertag, Status, m_LogonTaskHandle );

        // The logon process is starting so default to no optional message until we know differently
        SetOptionalMessage( FALSE );

        HRESULT Result = XOnlineGetUsers( m_UserList, &m_NumUsers );

        if( Result == S_OK )
        {   
            m_ActiveAccount = 0;
            m_UserAccounts.Clear();
            for( s32 i=0; i < (s32)m_NumUsers; i++ )
            {
                online_user& Profile = m_UserAccounts.Append();
                // Convert an XBOX user profile to internal user profile
                x_memset ( &Profile, 0, sizeof(Profile) );
                x_mstrcpy( Profile.Name, m_UserList[i].szGamertag );
                x_memcpy ( Profile.Password, m_UserList[i].passcode, sizeof( Profile.Password ) );
                Profile.Flags = 0;
                if( (m_UserList[i].dwUserOptions     & XONLINE_USER_OPTION_REQUIRE_PASSCODE) ) Profile.Flags |= USER_HAS_PASSWORD;
                if( (m_UserList[i].xuid.dwUserFlags  & XONLINE_USER_VOICE_NOT_ALLOWED) == 0 )  Profile.Flags |= USER_VOICE_ENABLED;
                if( (m_UserList[i].xuid.dwUserFlags  & XONLINE_USER_GUEST_MASK) )              Profile.Flags |= USER_IS_GUEST;
            }
        }
        else
        {
            ASSERT( FALSE );
        }

        switch( Status )
        {
        case S_OK:
            SetAuthStatus( AUTH_STAT_CONNECTING );
            break;
        case XONLINE_E_SILENT_LOGON_DISABLED:
            SetAuthStatus( AUTH_STAT_BANNED );
            break;

        case XONLINE_E_LOGON_NOT_LOGGED_ON:
        case XONLINE_E_LOGON_NO_NETWORK_CONNECTION:
            SetAuthStatus( AUTH_STAT_DISCONNECTED );
            break;
        case XONLINE_E_SILENT_LOGON_NO_ACCOUNTS:
            SetAuthStatus( AUTH_STAT_NO_ACCOUNT );
            break;
        case XONLINE_E_SILENT_LOGON_PASSCODE_REQUIRED:
            SetAuthStatus( AUTH_STAT_NEED_PASSWORD );
            break;
        case XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT:
            SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
            break;
        case XONLINE_E_LOGON_SERVERS_TOO_BUSY:
            SetAuthStatus( AUTH_STAT_SERVER_BUSY );
            break;
        default:
            ASSERT( FALSE );
            SetAuthStatus( AUTH_STAT_DISCONNECTED );
            break;
        }

        if( Status == S_OK )
        {
            m_LogonTaskResult = XOnlineTaskContinue( m_LogonTaskHandle );
            SetState( MATCH_SECURITY_CHECK );
        }
        else
        {
            SetState( MATCH_IDLE );
        }
    }
}

//=============================================================================

void match_mgr::UpdateAuthenticateMachine( void )
{
    // This is faked. The xbox no longer needs a machine authentication stage. So we
    // have to fake it to thinking the machine is fully authenticated.
    if( (GetState() != MATCH_SECURITY_CHECK) && (GetState() != MATCH_SILENT_LOGON) )
    {
        SetAuthStatus( AUTH_STAT_CONNECTED );
        SetState( MATCH_IDLE );
    }
}

//=============================================================================

void match_mgr::UpdateSecurityCheck( void )
{
    // We need to verify the login succeeded and then make sure
    // there are no service errors. See sample program MatchMaking.cpp,
    // around line 663.
    if( m_LogonTaskResult != XONLINETASK_S_RUNNING )
    {
        HRESULT OldTaskResult = m_LogonTaskResult;
        m_LogonTaskResult = XOnlineLogonTaskGetResults( m_LogonTaskHandle );
        LOG_MESSAGE( "match_mgr::UpdateSecurityCheck", "OldTaskResult:0x%08x, LogonTaskResult:0x%08x", OldTaskResult, m_LogonTaskResult );
        (void)OldTaskResult;
    }

    switch( m_LogonTaskResult )
    {
    case XONLINETASK_S_RUNNING:
        break;

    case XONLINE_S_LOGON_CONNECTION_ESTABLISHED:
        LOG_MESSAGE( "match_mgr::UpdateSecurityCheck", "XONLINE_S_LOGON_CONNECTION_ESTABLISHED");
        LogonConnectionEstablished();
        VerifyServices();
        break;

    case XONLINE_E_LOGON_INVALID_USER:
        LOG_MESSAGE( "match_mgr::UpdateSecurityCheck", "XONLINE_E_LOGON_INVALID_USER");
        SetAuthStatus( AUTH_STAT_INVALID_ACCOUNT );
        break;

    case XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON:
        LOG_MESSAGE( "match_mgr::UpdateSecurityCheck", "XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON");
        SetAuthStatus( AUTH_STAT_ALREADY_LOGGED_ON );
        break;

    case XONLINE_E_LOGON_SERVERS_TOO_BUSY:
        LOG_MESSAGE( "match_mgr::UpdateSecurityCheck", "XONLINE_E_LOGON_SERVERS_TOO_BUSY");
        SetAuthStatus( AUTH_STAT_SERVER_BUSY );
        break;

    case XONLINE_E_LOGON_UPDATE_REQUIRED:
        LOG_MESSAGE( "match_mgr::UpdateSecurityCheck", "XONLINE_E_LOGON_UPDATE_REQUIRED");
        SetAuthStatus( AUTH_STAT_NEED_UPDATE );
        break;
    case XONLINE_E_LOGON_CANNOT_ACCESS_SERVICE:
        LOG_MESSAGE( "match_mgr::UpdateSecurityCheck", "XONLINE_E_LOGON_CANNOT_ACCESS_SERVICE");
        SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
        break;

    default:
        LOG_MESSAGE( "match_mgr::UpdateSecurityCheck", "<UNKNOWN ERROR CODE>");
        SetAuthStatus( AUTH_STAT_DISCONNECTED );
        ASSERT( FALSE );
        break;
    }

    if( m_LogonTaskResult != XONLINETASK_S_RUNNING )
    {
        SetState( MATCH_IDLE );
    }
}

//=============================================================================

void match_mgr::LogonConnectionEstablished( void )
{
    XONLINE_USER*   pUsers  = XOnlineGetLogonUsers();

    if( m_MutelistTaskHandle )
    {
        XOnlineTaskClose( m_MutelistTaskHandle );
        m_MutelistTaskHandle = NULL;
    }
    if( m_MutelistEnumerateHandle )
    {
        XOnlineTaskClose( m_MutelistEnumerateHandle );
        m_MutelistEnumerateHandle = NULL;
    }
    HRESULT         Result1 = XOnlineFriendsStartup ( NULL, &m_BuddyTaskHandle    );
    HRESULT         Result2 = XOnlineMutelistStartup( NULL, &m_MutelistTaskHandle );
    m_Mutelist.State = mutelist::MUTELIST_GET;

    ASSERT( Result1 == S_OK );
    ASSERT( Result2 == S_OK );

    // NOTE: We only have one player logged in during play so use index 0
    m_BuddyState            = MATCH_BUDDY_START_ACQUISITION;
    m_IsLoggedOn            = TRUE;
    m_PlayerIdentifier[ 0 ] = pUsers[0].xuid.qwUserID;

    switch( pUsers[0].hr )
    {
    case XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT:
        SetAuthStatus( AUTH_STAT_URGENT_MESSAGE );
        break;

    case XONLINE_S_LOGON_USER_HAS_MESSAGE:

        SetOptionalMessage( TRUE );

        //
        // Deliberate fallthrough!
        //

    case S_OK:
        SetAuthStatus( AUTH_STAT_CONNECTED );
        SetUserStatus( BUDDY_STATUS_ONLINE );           //klkl: need this to update invite status
        StartDelayedStatsRead();
        break;

    default:
        ASSERTS( FALSE, "Logon returned an unexpected error code" );
        break;
    }

    LOG_MESSAGE( "match_mgr::LogonConnectionEstablished",  "Logon connection established" );
}

//=============================================================================

void match_mgr::UpdateGetMessages( void )
{
    SetState( MATCH_IDLE );
}

//=============================================================================

void match_mgr::UpdateValidatePlayerName( void )
{
}

//=============================================================================

void match_mgr::UpdateAcquireServers( f32 DeltaTime )
{
    // Get server list from matchmaker.
    if( m_PendingAcquisitionMode != ACQUIRE_INVALID )
    {
        SetState( MATCH_IDLE );
        return;
    }

    if( g_EnableLANLookup == TRUE )
        LocalLookups ( DeltaTime );
    else
        RemoteLookups( DeltaTime );
}

//=============================================================================

void match_mgr::UpdateBecomeServer( void )
{
    LOG_APP_NAME( "SERVER" );
    //m_pMatchMaker->Clear();
    ClearAllKeys( TRUE );
    if( g_PendingConfig.GetConfig().KeyIsRegistered )
    {
        VERIFY( XNetUnregisterKey( &g_PendingConfig.GetConfig().SessionID )==S_OK );
        RegisteredKeyCount--;
        g_PendingConfig.GetConfig().KeyIsRegistered=FALSE;
    }
    m_LocalIsServer = TRUE;
    g_ActiveConfig.SetRemoteAddress( m_pSocket->GetAddress() );
    SetState( MATCH_VALIDATE_SERVER_NAME );
}

//=============================================================================

void match_mgr::UpdateValidateServerName( void )
{
    SetState( MATCH_REGISTER_SERVER );
}

//=============================================================================

void match_mgr::UpdateUpdateServer( void )
{
    if( g_EnableLANLookup == TRUE )
    {
        SetState( MATCH_SERVER_ACTIVE );
    }
    else
    {
        if( FAILED( m_UpdateTaskResult ) )
        {
            ASSERTS( FALSE, "Unable to update registration with matchmaking service." );
            ASSERT( m_UpdateTaskHandle );
            if( m_UpdateTaskHandle )
            {
                XOnlineTaskClose( m_UpdateTaskHandle );
                m_UpdateTaskHandle = NULL;
            }
            SetState( MATCH_REGISTER_SERVER );
        }

        if( m_UpdateTaskResult == XONLINETASK_S_SUCCESS )
        {
            ASSERT( m_UpdateTaskHandle );
            if( m_UpdateTaskHandle )
            {
                XOnlineTaskClose( m_UpdateTaskHandle );
                m_UpdateTaskHandle = NULL;
            }
            m_UpdateTaskHandle = NULL;
            SetState( MATCH_SERVER_ACTIVE );
        }
    }
}

//=============================================================================

void match_mgr::UpdateRegisterServer( void )
{
    if( g_EnableLANLookup == TRUE )
    {
        HRESULT Result = XNetCreateKey( &g_ActiveConfig.GetConfig().SessionID, &m_KeyExchangeKey );
        ASSERT( SUCCEEDED( Result ) );

        x_memcpy( &g_PendingConfig.GetConfig().SessionID, &g_ActiveConfig.GetConfig().SessionID, sizeof(g_ActiveConfig.GetConfig().SessionID) );

        VERIFY( XNetRegisterKey( &g_ActiveConfig.GetConfig().SessionID, &m_KeyExchangeKey )==S_OK );
        RegisteredKeyCount++;
        #ifndef X_RETAIL
        LOG_MESSAGE( "match_mgr::UpdateState", "Key registered: 0x%08x:%08x:%08x:%08x, SessionID: %s",
            *(s32*)&m_KeyExchangeKey.ab[4],
            *(s32*)&m_KeyExchangeKey.ab[8],
            *(s32*)&m_KeyExchangeKey.ab[12],
            SessionIDString( g_ActiveConfig.GetConfig().SessionID ) );
        #endif
        SetState( MATCH_SERVER_ACTIVE );
    }
    else
    {
        if( FAILED( m_SessionTaskResult ) )
        {
            // We may want to notify the game player if the registration
            // fails. It means no one can join the game except on a local
            // network.
            LOG_ERROR( "match_mgr::UpdateState", "Server registration failed. Error code:0x%08x.", m_SessionTaskResult );
            m_RegistrationComplete = FALSE;
            SetConnectStatus( MATCH_CONN_REGISTER_FAILED );
            g_ActiveConfig.SetExitReason( GAME_EXIT_NETWORK_DOWN );
            SetState( MATCH_IDLE );
            XOnlineTaskClose( m_SessionTaskHandle );
            m_SessionTaskHandle = NULL;
        }
        else
        {
            // Task completed. Now, let's see what happened.
            if( m_SessionTaskResult != XONLINETASK_S_RUNNING )
            {
                // Extract the new session ID and Key-Exchange Key
                HRESULT Result = XOnlineMatchSessionGetInfo( m_SessionTaskHandle, &g_ActiveConfig.GetConfig().SessionID, &m_KeyExchangeKey );
                ASSERT( SUCCEEDED( Result ) );

                x_memcpy( &g_PendingConfig.GetConfig().SessionID, &g_ActiveConfig.GetConfig().SessionID, sizeof(g_ActiveConfig.GetConfig().SessionID) );

                VERIFY( XNetRegisterKey( &g_ActiveConfig.GetConfig().SessionID, &m_KeyExchangeKey )==S_OK );
                RegisteredKeyCount++;
               
                #ifndef X_RETAIL
                LOG_MESSAGE( "match_mgr::UpdateState", "Key registered: 0x%08x:%08x:%08x:%08x, SessionID: %s", 
                    *(s32*)&m_KeyExchangeKey.ab[0],
                    *(s32*)&m_KeyExchangeKey.ab[4],
                    *(s32*)&m_KeyExchangeKey.ab[8],
                    *(s32*)&m_KeyExchangeKey.ab[12],
                    SessionIDString( g_ActiveConfig.GetConfig().SessionID ) );
                #endif
                g_ActiveConfig.GetConfig().KeyIsRegistered  = TRUE;
                g_PendingConfig.GetConfig().KeyIsRegistered = TRUE;
                m_RegistrationComplete = TRUE;
                m_IsVisible = TRUE;
                SetConnectStatus( MATCH_CONN_CONNECTED );
                SetState( MATCH_SERVER_ACTIVE );
            }
        }
    }
}

//=============================================================================

void match_mgr::UpdateServerCheckVisible( void )
{
    // Send an echo request to the matchmaker udp echo port
    // Transition out of this state any earlier is performed
    // by ReceivePacket() when the echo response is received.
    if( HasTimedOut() )
    {
        m_StateRetries--;
        if( m_StateRetries >= 0 )
        {
            SetState(MATCH_SERVER_CHECK_VISIBLE);
        }
        else
        {
            // Notify the game host that this server was
            // not visible outside the firewall. We still go ahead
            // and try and register anyway.
            SetConnectStatus( MATCH_CONN_NOT_VISIBLE );
            m_IsVisible = FALSE;
            SetState( MATCH_IDLE );
        }
    }
    if( m_IsVisible )
    {
        SetState( MATCH_IDLE );
    }
}

//=============================================================================

void match_mgr::UpdateServerActive( void )
{
    if( g_EnableLANLookup == FALSE )
    {
        // Force a refresh with the matchmaking service if the number of players on the server changes.
        // Note: To avoid hammering the XBox Live service every time a player joins or drops, a delay is
        // used before updating the new state of the server.
        if( m_UpdateRegistration && (m_StateTimeout > UPDATE_SERVER_INTERVAL) )
        {
            m_StateTimeout = UPDATE_SERVER_INTERVAL;
        }

        if( m_SessionTaskHandle && FAILED( m_SessionTaskResult ) )
        {
            g_ActiveConfig.SetExitReason( GAME_EXIT_NETWORK_DOWN );
            SetConnectStatus( MATCH_CONN_REGISTER_FAILED );
            SetState( MATCH_IDLE );
        }

        if( HasTimedOut() )
        {
            SetState(MATCH_UPDATE_SERVER);
        }
    }
}

//=============================================================================

void match_mgr::UpdateUnregisterServer( void )
{
    if( g_EnableLANLookup == TRUE )
    {
        VERIFY( XNetUnregisterKey( &g_ActiveConfig.GetConfig().SessionID )==S_OK );
        RegisteredKeyCount--;
        #ifndef X_RETAIL
        LOG_MESSAGE( "match_mgr::UpdateUnregisterServer", "Unregistered key %s", SessionIDString( g_ActiveConfig.GetConfig().SessionID ) );
        #endif
        g_ActiveConfig.GetConfig().KeyIsRegistered  = FALSE;
        g_PendingConfig.GetConfig().KeyIsRegistered = FALSE;
        SetState( MATCH_IDLE );
    }
    else
    {
        if( (m_SessionTaskResult != XONLINETASK_S_RUNNING) && m_SessionTaskHandle )
        {
            if( m_SessionTaskHandle )
            {
                XOnlineTaskClose( m_SessionTaskHandle );
                m_SessionTaskHandle = NULL;
            }
            ASSERT( g_ActiveConfig.GetConfig().KeyIsRegistered );
        }
        if( g_ActiveConfig.GetConfig().KeyIsRegistered )
        {
            VERIFY( XNetUnregisterKey( &g_ActiveConfig.GetConfig().SessionID )==S_OK );
            RegisteredKeyCount--;
        }
        g_ActiveConfig.GetConfig().KeyIsRegistered  = FALSE;
        g_PendingConfig.GetConfig().KeyIsRegistered = FALSE;
        SetState( MATCH_IDLE );
    }
}

//=============================================================================
void match_mgr::UpdateIndirectLookup( void )
{
    if( m_SearchTaskHandle == NULL )
    {
        g_ActiveConfig.SetExitReason( GAME_EXIT_SESSION_ENDED );
        SetConnectStatus( MATCH_CONN_SESSION_ENDED );
        SetState( MATCH_IDLE );
        return;
    }

    if( m_SearchTaskResult == XONLINETASK_S_RUNNING )
    {
        return;
    }

    if( SUCCEEDED( m_SearchTaskResult ) )
    {
        XONLINE_MATCH_SEARCHRESULT**    ppSearchResults;
        XONLINE_MATCH_SEARCHRESULT*     pSearchResult;
        IN_ADDR                         Addr;
        DWORD                           SessionCount;

        LOG_APP_NAME( "CLIENT" );

        // If the find session call completes, then we need to get results. We will only have 1 result
        // but this is the one we want.
        HRESULT Result = XOnlineMatchSearchGetResults( m_SearchTaskHandle, &ppSearchResults, &SessionCount );
        if( SessionCount == 0 )
        {
            g_ActiveConfig.SetExitReason( GAME_EXIT_SESSION_ENDED );
            SetConnectStatus( MATCH_CONN_SESSION_ENDED );
        }
        else
        {
            if( SUCCEEDED( Result ) )
            {
                server_info& Info = g_PendingConfig.GetConfig();
                pSearchResult = ppSearchResults[0];

                ClearAllKeys( TRUE );

                x_memcpy( &Info.ExchangeKey, &pSearchResult->KeyExchangeKey, sizeof( Info.ExchangeKey) );
                x_memcpy( &Info.Address,     &pSearchResult->HostAddress,    sizeof( Info.Address) );

                VERIFY( XNetRegisterKey( &Info.SessionID, &Info.ExchangeKey )==S_OK );
                RegisteredKeyCount++;
#ifndef X_RETAIL
                LOG_MESSAGE( "match_mgr::UpdateIndirectLookup", "Key registered: 0x%08x:%08x:%08x:%08x, SessionID: %s", 
                    *(s32*)&Info.ExchangeKey.ab[0],
                    *(s32*)&Info.ExchangeKey.ab[4],
                    *(s32*)&Info.ExchangeKey.ab[8],
                    *(s32*)&Info.ExchangeKey.ab[12],
                    SessionIDString( Info.SessionID ) );
#endif
                Result = XNetXnAddrToInAddr( &Info.Address, &Info.SessionID, &Addr );
                ASSERT( Result == S_OK );
                Info.Remote.Setup( ntohl( Addr.s_addr ), NET_GAME_PORT );
                Info.KeyIsRegistered = TRUE;
                LOG_MESSAGE( "match_mgr::UpdateIndirectLookup", "Remote address:%s", Info.Remote.GetStrAddress() );

                m_LocalIsServer = FALSE;
            }
            else
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_CONNECTION_LOST );
                SetConnectStatus( MATCH_CONN_UNAVAILABLE );
            }
        }
    }
    else
    {
        LOG_WARNING( "match_mgr::UpdateIndirectLookup", "Lookup failed." );
        g_ActiveConfig.SetExitReason( GAME_EXIT_CONNECTION_LOST );
        SetConnectStatus( MATCH_CONN_UNAVAILABLE );
    }

    XOnlineTaskClose( m_SearchTaskHandle );
    m_SearchTaskHandle = NULL;
    SetState( MATCH_IDLE );
}

//=============================================================================
void match_mgr::UpdateNATNegotiate( void )
{
    if( g_EnableLANLookup == TRUE )
    {
        ClearAllKeys( TRUE );

        server_info& Info = g_PendingConfig.GetConfig();

        VERIFY( XNetRegisterKey( &Info.SessionID, &Info.ExchangeKey )==S_OK );
        RegisteredKeyCount++;

        Info.KeyIsRegistered = TRUE;
        SetState( MATCH_LOGIN );
        m_LocalIsServer = FALSE;
    }
    else
    {
        if( m_SearchTaskHandle == NULL )
        {
            // Issue another matchmaker lookup so that we can get the absolutely correct encryption keys
            // for this particular console.
            HRESULT Result = XOnlineMatchSessionFindFromID( g_PendingConfig.GetConfig().SessionID, NULL, &m_SearchTaskHandle );
            if( Result!=S_OK )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_SESSION_ENDED );
                SetConnectStatus( MATCH_CONN_SESSION_ENDED );
                SetState( MATCH_IDLE );
            }
        }
        else
        {
            if( m_SearchTaskResult != XONLINETASK_S_RUNNING )
            {
                if( SUCCEEDED( m_SearchTaskResult ) )
                {
                    XONLINE_MATCH_SEARCHRESULT**    ppSearchResults;
                    XONLINE_MATCH_SEARCHRESULT*     pSearchResult;
                    IN_ADDR                         Addr;
                    DWORD                           SessionCount;

                    LOG_APP_NAME( "CLIENT" );

                    // If the find session call completes, then we need to get results. We will only have 1 result
                    // but this is the one we want.
                    HRESULT Result = XOnlineMatchSearchGetResults( m_SearchTaskHandle, &ppSearchResults, &SessionCount );
                    if( SessionCount == 0 )
                    {
                        g_ActiveConfig.SetExitReason( GAME_EXIT_SESSION_ENDED );
                        SetConnectStatus( MATCH_CONN_SESSION_ENDED );
                        SetState( MATCH_IDLE );
                    }
                    else
                    {
                        if( SUCCEEDED( Result ) )
                        {
                            server_info& Info = g_PendingConfig.GetConfig();
                            pSearchResult = ppSearchResults[0];

                            ClearAllKeys( TRUE );

                            x_memcpy( &Info.ExchangeKey, &pSearchResult->KeyExchangeKey, sizeof( Info.ExchangeKey) );
                            x_memcpy( &Info.Address,     &pSearchResult->HostAddress,    sizeof( Info.Address) );

                            VERIFY( XNetRegisterKey( &Info.SessionID, &Info.ExchangeKey )==S_OK );
                            RegisteredKeyCount++;
                            #ifndef X_RETAIL
                            LOG_MESSAGE( "match_mgr::UpdateNATNegotiate", "Key registered: 0x%08x:%08x:%08x:%08x, SessionID: %s", 
                                *(s32*)&Info.ExchangeKey.ab[0],
                                *(s32*)&Info.ExchangeKey.ab[4],
                                *(s32*)&Info.ExchangeKey.ab[8],
                                *(s32*)&Info.ExchangeKey.ab[12],
                                SessionIDString( Info.SessionID ) );
                            #endif
                            Result = XNetXnAddrToInAddr( &Info.Address, &Info.SessionID, &Addr );
                            ASSERT( Result == S_OK );
                            Info.Remote.Setup( ntohl( Addr.s_addr ), NET_GAME_PORT );
                            Info.KeyIsRegistered = TRUE;
                            LOG_MESSAGE( "match_mgr::UpdateNATNegotiate", "NAT Negotiation complete. Remote address:%s", Info.Remote.GetStrAddress() );

                            SetState( MATCH_LOGIN );
                            m_LocalIsServer = FALSE;
                        }
                        else
                        {
                            g_ActiveConfig.SetExitReason( GAME_EXIT_CONNECTION_LOST );
                            SetConnectStatus( MATCH_CONN_UNAVAILABLE );
                            SetState( MATCH_IDLE );
                        }
                    }
                }
                else
                {
                    LOG_WARNING( "match_mgr::UpdateNATNegotiate", "NAT Negotiation failed." );
                    SetState( MATCH_IDLE );
                    g_ActiveConfig.SetExitReason( GAME_EXIT_CONNECTION_LOST );
                    SetConnectStatus( MATCH_CONN_UNAVAILABLE );
                }

                XOnlineTaskClose( m_SearchTaskHandle );
                m_SearchTaskHandle = NULL;
            }
        }
    }
}

//=============================================================================

void match_mgr::UpdateLogin( void )
{
    if( g_NetworkMgr.IsLoggedIn() )
    {
        // Now we need to supply our own user id
        // This is the state that the client must wait for prior
        // to trying to fetch the map.
        SetState( MATCH_CLIENT_ACTIVE );
        SetConnectStatus( MATCH_CONN_CONNECTED );
    }
    else
    {
        if( g_ActiveConfig.GetExitReason() != GAME_EXIT_CONTINUE )
        {
            server_info& Config = g_PendingConfig.GetConfig();

            // The pendingconfig structure is used here because it has not yet been commited to
            // the active config until the login process completes.
            ASSERT( Config.KeyIsRegistered );
            VERIFY( XNetUnregisterKey( &Config.SessionID )==S_OK );
            Config.KeyIsRegistered = FALSE;
            RegisteredKeyCount--;
            SetConnectStatus( MATCH_CONN_DISCONNECTED );
            SetState( MATCH_IDLE );
        }
    }
}

//=============================================================================

void match_mgr::UpdateNotifications( void )
{
    XONLINE_NOTIFICATION_EX_INFO notEXInfo;
    DWORD stateFlags;
    //klkl: this needs to work for controller port that entered Live not just controller port 0
    if (IsAuthenticated())
    {
    m_RecvNotifications = 0;
    // Check for old messages
    if( XOnlineGetNotification( m_UserIndex, XONLINE_NOTIFICATION_FRIEND_REQUEST ) )
    {
        m_RecvNotifications |= NOTIFY_FRIEND_REQUEST;
    }

    if( XOnlineGetNotification( m_UserIndex, XONLINE_NOTIFICATION_GAME_INVITE ) )
    {
        m_RecvNotifications |= NOTIFY_GAME_INVITE;
    }

    if( XOnlineGetNotification( m_UserIndex, XONLINE_NOTIFICATION_NEW_GAME_INVITE ) )
    {
        m_RecvNotifications |= NOTIFY_NEW_GAME_INVITE;
    }

    if( XOnlineGetNotification( m_UserIndex, XONLINE_NOTIFICATION_GAME_INVITE_ANSWER ) )
    {
        m_RecvNotifications |= NOTIFY_GAME_INVITE_ANSWER;
    }

    // Check for new messages
    if( XOnlineGetNotificationEx( m_UserIndex, &notEXInfo, &stateFlags ) )
    {
        if( stateFlags != XONLINE_NOTIFICATION_STATE_FLAG_PENDING_SYNC )
        {
            // We've received some new type of message
            if( SUCCEEDED( XOnlineMessageSummary( m_UserIndex, notEXInfo.dwMessageID, &m_MsgSummary ) ) )
            {
                // Check if there is a voice attachment
                if( m_MsgSummary.dwMessageFlags & XONLINE_MSG_FLAG_HAS_VOICE )
                {
                    m_RecvNotifications |= NOTIFY_VOICE_ATTACHMENT;

                    // We received a voice message so it MUST be from a friend.
                    // Lookup the friend in our friend list and store the message ID
                    for( s32 i=0; i < GetBuddyCount(); i++ )
                    {
                        buddy_info& Buddy = GetBuddy( i );

                        // Compare the friend's name to the message sender's name
                        if( x_stricmp( Buddy.Name, m_MsgSummary.szSenderName ) == 0 )
                        {
                            Buddy.MessageID  = notEXInfo.dwMessageID;
                            Buddy.Flags     |= USER_VOICE_MESSAGE;

                            LOG_MESSAGE( "match_mgr::UpdateNotifications",
                                         "Received voicemail notification from %s",
                                         Buddy.Name );
                        }
                    }
                }

                if( m_MsgSummary.bMsgType == XONLINE_MSG_TYPE_FRIEND_REQUEST )
                {
                    //flag for receiving a friend request
                    m_RecvNotifications |= NOTIFY_FRIEND_REQUEST;   
                }
                else
                {
                    if( m_MsgSummary.bMsgType == XONLINE_MSG_TYPE_GAME_INVITE )
                    {
                        //flag for receiving a game invite
                        m_RecvNotifications |= NOTIFY_GAME_INVITE;  
                    }
                }
            }

            if( stateFlags == XONLINE_NOTIFICATION_STATE_FLAG_MORE_ITEMS )
            {
                //received a bunch of messages
                m_RecvNotifications |= NOTIFY_BUNCH_OF_MESSAGES;
            }
        }
    }
}

    //
    // Ignore any game invites to the current session
    //

    if( (GameMgr.GameInProgress() == TRUE) &&
        (GameMgr.IsGameOnline()   == TRUE) )
    {
        for( s32 i=0; i < GetBuddyCount(); i++ )
        {
            buddy_info& Buddy = GetBuddy( i );

            if( Buddy.Flags & USER_HAS_INVITE )
            {
                // If friend is in the same session as the player then clear the invite
                if( x_memcmp( &Buddy.SessionID,
                              &g_ActiveConfig.GetConfig().SessionID,
                              sizeof( Buddy.SessionID ) ) == 0 )
                {
                    Buddy.Flags &= ~USER_HAS_INVITE;
                }
            }
        }
    }
}

//=============================================================================

void match_mgr::UpdateMessageState( void )
{
    switch( m_MessageState )
    {
    case MATCH_MESSAGE_SUCCESS  : 
    case MATCH_MESSAGE_FAILED   : 
    case MATCH_MESSAGE_IDLE     : UpdateMessageIdle();      break;
    case MATCH_MESSAGE_DETAILS  : UpdateMessageDetails();   break;
    case MATCH_MESSAGE_SEND     : UpdateMessageSend();      break;
    case MATCH_MESSAGE_RECEIVE  : UpdateMessageReceive();   break;
    default                     : ASSERT( FALSE );          break;
    };

    if( m_FeedbackHandle && (m_FeedbackResult != XONLINETASK_S_RUNNING ) )
    {
        XOnlineTaskClose( m_FeedbackHandle );
        m_FeedbackHandle = NULL;
    }
    // Check if the invite task has completed
    if( m_InviteTaskHandle && (m_InviteTaskResult != XONLINETASK_S_RUNNING) )
    {
        XOnlineTaskClose( m_InviteTaskHandle );
        m_InviteTaskHandle = NULL;
    }

    // Check if the friend request task has completed
    if( m_FriendRequestTaskHandle && (m_FriendRequestResult != XONLINETASK_S_RUNNING) )
    {
        XOnlineTaskClose( m_FriendRequestTaskHandle );
        m_FriendRequestTaskHandle = NULL;
    }
}

//=============================================================================

xbool match_mgr::IsDownloadingMessage( void )
{
    xbool IsDownloading = (m_MessageState == MATCH_MESSAGE_DETAILS) ||
                          (m_MessageState == MATCH_MESSAGE_RECEIVE);

    return( IsDownloading );
}

//=============================================================================

xbool match_mgr::IsSendingMessage( void )
{
    xbool   IsSending = (m_MessageState == MATCH_MESSAGE_SEND);
    return( IsSending );
}

//=============================================================================

void match_mgr::UpdateMessageIdle( void )
{
    // Cleanup everything except memory for voice message
    if( m_MessageTaskHandle != NULL )
    {
        XOnlineTaskClose( m_MessageTaskHandle );
        m_MessageTaskHandle = NULL;
    }

    if( m_DownloadTaskHandle != NULL )
    {
        XOnlineTaskClose( m_DownloadTaskHandle );
        m_DownloadTaskHandle = NULL;
    }

    if( m_hMessage != NULL )
    {
        XOnlineMessageDestroy( m_hMessage );
        m_hMessage = NULL;
    }
}

//==============================================================================

void match_mgr::SetupMessage( BYTE MessageType )
{
    // Ensure that there is no message still waiting to be sent
    ASSERT( m_hMessage == NULL );

    DWORD MessageFlags  = 0;
    WORD  NumProperties = (MessageType == XONLINE_MSG_TYPE_GAME_INVITE) ? 1 : 0;
    xbool VoiceMessage  = (m_pVoiceMessageSend != NULL) && (m_VoiceMessageNumBytes > 0);
    
    // Check if there is a voice attachment to be sent with the message
    if( VoiceMessage == TRUE )
    {
        MessageFlags   = XONLINE_MSG_FLAG_HAS_VOICE;
        NumProperties += 3;
    }

    XOnlineMessageCreate( MessageType,
                          NumProperties,
                          0,
                          0,
                          MessageFlags,
                          0,
                          &m_hMessage );

    if( MessageType == XONLINE_MSG_TYPE_GAME_INVITE )
    {
        // Set the session ID in the message.
        // NOTE: This property may not be required - I include it just to be safe.
        // See description of XOnlineMessageCreate in XDK docs for more info.
        XNKID& SessionID = g_ActiveConfig.GetConfig().SessionID;
        XOnlineMessageSetProperty( m_hMessage,
                                   XONLINE_MSG_PROP_SESSION_ID,
                                   sizeof( SessionID ),
                                   &SessionID,
                                   0 );
    }

    // Check if there is a voice attachment to be sent with the message
    if( VoiceMessage == TRUE )
    {
        WORD   Codec        = XONLINE_PROP_VOICE_DATA_CODEC_WMAVOICE_V90;
        DWORD  DurationInMs = (DWORD)(m_VoiceMessageDuration * 1000.0f);

        // Set the voice message
        XOnlineMessageSetProperty( m_hMessage,
                                   XONLINE_MSG_PROP_VOICE_DATA,
                                   m_VoiceMessageNumBytes,
                                   m_pVoiceMessageSend,
                                   0 );

        // Set the audio codec
        XOnlineMessageSetProperty( m_hMessage,
                                   XONLINE_MSG_PROP_VOICE_DATA_CODEC,
                                   sizeof( WORD ),
                                   &Codec,
                                   0 );

        // Set the audio duration
        XOnlineMessageSetProperty( m_hMessage,
                                   XONLINE_MSG_PROP_VOICE_DATA_DURATION,
                                   sizeof( DWORD ),
                                   &DurationInMs,
                                   0 );
    }
}

//=============================================================================

void match_mgr::UpdateMessageDetails( void )
{
    if( FAILED( m_MessageTaskResult ) == TRUE )
    {
        m_MessageState = MATCH_MESSAGE_FAILED;
        LOG_WARNING( "match_mgr::UpdateMessageDetails", "Unable to retrieve message details!" );
        return;
    }

    if( m_MessageTaskResult == XONLINETASK_S_SUCCESS )
    {
        HRESULT Result;
        XNKID   SessionID;
        DWORD   ReceivedSize  = 0;
        DWORD   DurationMS    = 0;

        // Retrieve the SessionID property.
        // Note: this will not be present in a friend request message!
        Result = XOnlineMessageDetailsGetResultsProperty( m_MessageTaskHandle,
            XONLINE_MSG_PROP_SESSION_ID,
            sizeof( SessionID ),
            &SessionID,
            &ReceivedSize,
            NULL );

        // Retrieve the voice message duration property
        Result = XOnlineMessageDetailsGetResultsProperty( m_MessageTaskHandle,
            XONLINE_MSG_PROP_VOICE_DATA_DURATION,
            sizeof( DurationMS ),
            &DurationMS,
            &ReceivedSize,
            NULL );

        // Retrieve the size of the attached voice message in bytes
        Result = XOnlineMessageDetailsGetResultsProperty( m_MessageTaskHandle,
            XONLINE_MSG_PROP_VOICE_DATA,
            0,
            NULL,
            (DWORD*)&m_VoiceMessageNumBytes,
            NULL );

        m_VoiceMessageDuration = DurationMS / 1000.0f;

        // We should only be getting message details on our voice messages
        ASSERT( Result == XONLINE_E_MESSAGE_PROPERTY_DOWNLOAD_REQUIRED );

        m_pVoiceMessageRec = new byte[ m_VoiceMessageNumBytes ];

        if( m_pVoiceMessageRec == NULL )
        {
            LOG_WARNING( "match_mgr::UpdateMessageDetails",
                "Unable to allocate %d bytes!", m_VoiceMessageNumBytes );
            return;
        }

        XOnlineMessageDownloadAttachmentToMemory( m_MessageTaskHandle,
            XONLINE_MSG_PROP_VOICE_DATA,
            m_pVoiceMessageRec,
            m_VoiceMessageNumBytes,
            NULL,
            &m_DownloadTaskHandle );

        m_MessageState = MATCH_MESSAGE_RECEIVE;

        LOG_MESSAGE( "match_mgr::UpdateMessageDetails",
            "Downloading %f second voice attachment: Size = %d bytes\n",
            m_VoiceMessageDuration,
            m_VoiceMessageNumBytes );
    }
}

//=============================================================================

void match_mgr::UpdateMessageSend( void )
{
    if( m_InviteTaskHandle != NULL )
    {
        // Wait until the message sending finishes or fails
        if( m_InviteTaskResult != XONLINETASK_S_RUNNING )
        {
            if( FAILED( m_InviteTaskResult ) == TRUE )
            {
                m_MessageState = MATCH_MESSAGE_FAILED;
                LOG_WARNING( "match_mgr::UpdateMessageSend", "Unable to send message!" );
            }
            else
            {
                m_MessageState = MATCH_MESSAGE_SUCCESS;
                LOG_MESSAGE( "match_mgr::UpdateMessageSend", "Message was sent successfully" );
            }
        }
    }

    if( m_FriendRequestTaskHandle != NULL )
    {
        // Wait until the message sending finishes or fails
        if( m_FriendRequestResult != XONLINETASK_S_RUNNING )
        {
            if( FAILED( m_FriendRequestResult ) == TRUE )
            {
                m_MessageState = MATCH_MESSAGE_FAILED;
                LOG_WARNING( "match_mgr::UpdateMessageSend", "Unable to send message!" );
            }
            else
            {
                m_MessageState = MATCH_MESSAGE_SUCCESS;
                LOG_MESSAGE( "match_mgr::UpdateMessageSend", "Message was sent successfully" );
            }
        }
    }
}

//=============================================================================

void match_mgr::UpdateMessageReceive( void )
{
    // Wait until attachment has downloaded or failed
    if( m_DownloadTaskResult != XONLINETASK_S_RUNNING )
    {
        if( FAILED( m_DownloadTaskResult ) == TRUE )
            m_MessageState = MATCH_MESSAGE_FAILED;
        else
        {
            m_MessageState = MATCH_MESSAGE_SUCCESS;

            BYTE*   pReceivedData = NULL;
            DWORD   ReceivedSize  = 0;
            DWORD   RequestedSize = 0;

            // This function will actually transfer the attachment to our buffer
            XOnlineMessageDownloadAttachmentToMemoryGetResults( m_DownloadTaskHandle,
                &pReceivedData,
                &ReceivedSize,
                &RequestedSize );

            if( ReceivedSize != RequestedSize )
            {
                LOG_WARNING( "match_mgr::UpdateMessageReceive",
                    "Only received %d of %d bytes in voice attachment!\n",
                    ReceivedSize,
                    RequestedSize );

                // Set the state to failed so that client code can detect the problem
                m_MessageState = MATCH_MESSAGE_FAILED;
            }
            else
            {
                LOG_MESSAGE( "match_mgr::UpdateMessageReceive",
                    "Downloaded voice attachment of size %d bytes",
                    ReceivedSize );
            }
        }
    }
}

//==============================================================================

xbool match_mgr::StartMessageDownload( buddy_info& Buddy )
{
    // Ensure no download is already in progress
    ASSERT( m_pVoiceMessageRec  == NULL );
    ASSERT( m_MessageTaskHandle == NULL );

    HRESULT Result = XOnlineMessageDetails( m_UserIndex,
                                            Buddy.MessageID,
                                            XONLINE_MSG_FLAG_READ,
                                            0,
                                            NULL,
                                            &m_MessageTaskHandle );

    if( SUCCEEDED( Result ) == TRUE )
    {
        // Set the state to wait until we get the message details
        m_MessageState         = MATCH_MESSAGE_DETAILS;
        m_VoiceMessageNumBytes = 0;
        m_VoiceMessageDuration = 0.0f;

        LOG_MESSAGE( "match_mgr::StartMessageDownload",
                     "Starting voice attachment download from friend %s",
                     Buddy.Name );

        return( TRUE );
    }
    else
    {
        // This is the first time we have tried to use the message ID since it was set
        // in UpdateNotifications.  At this point we have discovered that the message ID is invalid.
        // This can happen if the user who sent the message revokes it before we download it.
        // We must therefore lookup this user and clear the voice message flag.
        {
            buddy_info* pBuddy = const_cast< buddy_info* >( FindBuddy( Buddy.Identifier ) );
            if( pBuddy != NULL )
            {
                pBuddy->MessageID  = 0;
                pBuddy->Flags     &= ~USER_VOICE_MESSAGE;
                LOG_MESSAGE( "match_mgr::StartMessageDownload",
                             "Clearing voicemail flag for %s",
                             pBuddy->Name );
            }
        }

        LOG_WARNING( "match_mgr::StartMessageDownload",
                     "Failed to start downloading voice attachment. Error = %08X",
                     Result );

        return( FALSE );
    }
}

//=============================================================================

void match_mgr::FreeMessageDownload( void )
{
    if( m_pVoiceMessageRec != NULL )
    {
        // Free the memory we allocated for the received voice message
        delete[] m_pVoiceMessageRec;
        m_pVoiceMessageRec     = NULL;
        m_VoiceMessageNumBytes = 0;
        m_VoiceMessageDuration = 0.0f;

        LOG_MESSAGE( "match_mgr::FreeMessageDownload", "Freeing voice message memory" );
    }
}

//=============================================================================

xbool match_mgr::GetVoiceMessage( BYTE** ppVoiceMessage,
                                  s32*   pNumBytes,
                                  f32*   pDuration )
{
    // Ensure we have a valid voice message before proceeding
    if( (m_pVoiceMessageRec     == NULL) ||
        (m_VoiceMessageNumBytes == 0   ) ||
        (m_VoiceMessageDuration == 0.0f) )
        return( FALSE );

    if(  ppVoiceMessage != NULL )
        *ppVoiceMessage  = m_pVoiceMessageRec;

    if(  pNumBytes != NULL )
        *pNumBytes  = m_VoiceMessageNumBytes;

    if(  pDuration != NULL )
        *pDuration  = m_VoiceMessageDuration;

    return( TRUE );
}

//=============================================================================

void match_mgr::BuddyIdle( void )
{
    if( m_BuddyEnumerateResult == XONLINETASK_S_RESULTS_AVAIL )
    {
        m_BuddyState = MATCH_BUDDY_ACQUIRING;
    }
}

//=============================================================================

void match_mgr::BuddyStartAcquisition( void )
{
    if( m_BuddyTaskResult != XONLINETASK_S_RUNNING )
    {
        // need to finish current task before starting new
        while( m_BuddyEnumerateHandle )
        {
            if( SUCCEEDED( XOnlineTaskContinue( m_BuddyEnumerateHandle ) ) )
            {
                // Close down the task handle
                XOnlineTaskClose( m_BuddyEnumerateHandle );
                m_BuddyEnumerateHandle = NULL;
            }
        };

        ASSERT( (m_BuddyTaskResult == XONLINETASK_S_SUCCESS) || (m_BuddyTaskResult == XONLINETASK_S_RUNNING_IDLE) );

        HRESULT Result = XOnlineFriendsEnumerate( 0, NULL, &m_BuddyEnumerateHandle );

        if ( Result == S_OK )
        {
            m_BuddyState = MATCH_BUDDY_ACQUIRING;   
        }

        if( m_pFriendBuffer == NULL )
        {
            m_pFriendBuffer = (XONLINE_FRIEND*)x_malloc( sizeof( XONLINE_FRIEND ) * MAX_FRIENDS );
            ASSERT( m_pFriendBuffer );
            x_memset( m_pFriendBuffer, 0, sizeof( XONLINE_FRIEND ) * MAX_FRIENDS );
        }
    }
}

//=============================================================================

void match_mgr::BuddyAcquiring( void )
{
    if( m_BuddyEnumerateResult == XONLINETASK_S_RESULTS_AVAIL )
    {
        xarray< buddy_info >    NewBuddies;

        s32             FriendCount = XOnlineFriendsGetLatest( m_UserIndex, MAX_FRIENDS, m_pFriendBuffer );
        XONLINE_FRIEND* pFriend     = m_pFriendBuffer;

        NewBuddies.Clear();
        NewBuddies.SetCapacity( FriendCount );

        LOG_MESSAGE( "match_mgr::BuddyAcquiring", "XONLINETASK_S_RESULTS_AVAIL FriendCount %d ", FriendCount );

        for( s32 i=0; i < FriendCount; i++ )
        {
            buddy_info&     Buddy  = NewBuddies.Append();
            buddy_status    Status = BUDDY_STATUS_OFFLINE;
            s32             Flags  = 0;

            x_memset( &Buddy, 0, sizeof( Buddy ) );

            x_strcpy( Buddy.Name, pFriend->szGamertag );
            Buddy.Location[0] = 0x0;
            s32 Language;

            switch( x_GetLocale() )
            {
            case XL_LANG_ENGLISH:       Language = XC_LANGUAGE_ENGLISH;     break;
            case XL_LANG_GERMAN:        Language = XC_LANGUAGE_GERMAN;      break;
            case XL_LANG_FRENCH:        Language = XC_LANGUAGE_FRENCH;      break;
            case XL_LANG_SPANISH:       Language = XC_LANGUAGE_SPANISH;     break;
            case XL_LANG_ITALIAN:       Language = XC_LANGUAGE_ITALIAN;     break;
            case XL_LANG_JAPANESE:      Language = XC_LANGUAGE_JAPANESE;    break;
            default:                    Language = XC_LANGUAGE_ENGLISH;
            }

            // Now convert his status and flags
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_ONLINE          ) Status = BUDDY_STATUS_ONLINE;
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_PLAYING         ) Status = BUDDY_STATUS_INGAME;
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_VOICE           ) Flags |= USER_VOICE_ENABLED;
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTINVITE      ) Flags |= USER_SENT_INVITE;
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDINVITE  ) Flags |= USER_HAS_INVITE;
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_INVITEACCEPTED  ) Flags |= USER_ACCEPTED_INVITE;
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_INVITEREJECTED  ) Flags |= USER_REJECTED_INVITE;
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTREQUEST     ) Flags |= USER_REQUEST_SENT;
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST ) Flags |= USER_REQUEST_RECEIVED;
            if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_JOINABLE        ) Flags |= USER_IS_JOINABLE;

            XOnlineFriendsGetTitleName( pFriend->dwTitleID,
                                        Language,
                                        sizeof( Buddy.Location ) / sizeof( xwchar ),
                                        Buddy.Location );

            // I have commented this out since it was causing the "Insert Disc" message to have
            // the level name and game type appended to the game title. (JP)
            //
            //// If this is our game, then we can set the map type
            //if( XOnlineTitleIdIsSameTitle( pFriend->dwTitleID ) && (Status == BUDDY_STATUS_INGAME) )
            //{
            //    x_mstrcat( Buddy.Location, " " );
            //    x_mstrcat( Buddy.Location, (const char*)pFriend->StateData );
            //}

            Buddy.Identifier = pFriend->xuid.qwUserID;
            Buddy.SessionID  = pFriend->sessionID;
            Buddy.Status     = Status;
            Buddy.Flags      = (user_flags)Flags;
            Buddy.TitleID    = pFriend->dwTitleID;
            Buddy.MessageID  = 0;

            LOG_MESSAGE( "match_mgr::BuddyAcquiring","Acquired %s friendstate %08X", Buddy.Name, pFriend->dwFriendState );

            // Check if we already have this buddy in our list
            const buddy_info* pBuddy = FindBuddy( Buddy.Identifier );

            if( pBuddy != NULL )
            {
                // We are building the buddy list from scratch each time the Live service
                // tells us there are results available.  For this reason, we must check
                // whether the buddy received a voice message previously and copy this
                // information into the new buddy object.
                Buddy.MessageID  =  pBuddy->MessageID;
                Buddy.Flags     &= ~USER_VOICE_MESSAGE;
                Buddy.Flags     |= (USER_VOICE_MESSAGE & pBuddy->Flags);
            }

            pFriend++;
        }
        
        // Clear out the old buddy list and substitute it with the newly built one
        m_Buddies.Clear();
        m_Buddies = NewBuddies;
    }
}

//=============================================================================

void match_mgr::BuddyShutdown( void )
{
    XOnlineTaskClose( m_BuddyEnumerateHandle );
    m_BuddyEnumerateHandle = 0;
}

//=============================================================================

void match_mgr::ExitState( match_mgr_state OldState )
{
    switch( OldState )
    {
    case MATCH_ACQUIRE_EXTENDED_INFO:
        if( m_ExtendedServerInfoOwner != -1 )
        {
            server_info* pServer = GetServerInfo( m_ExtendedServerInfoOwner );
            if( pServer && pServer->KeyIsRegistered )
            {
                pServer->KeyIsRegistered = FALSE;
                VERIFY( XNetUnregisterKey( &pServer->SessionID )==S_OK );
                RegisteredKeyCount--;
            }
        }
    case MATCH_ACQUIRE_SERVERS:
    case MATCH_ACQUIRE_IDLE:
        if( m_SearchTaskHandle )
        {
            XOnlineTaskClose( m_SearchTaskHandle );
            m_SearchTaskHandle = NULL;
        }
        if( m_ExtendedServerInfoOwner != -1 )
        {
            server_info* pServer = GetServerInfo( m_ExtendedServerInfoOwner );
            if( pServer && pServer->KeyIsRegistered )
            {
                pServer->KeyIsRegistered = FALSE;
                VERIFY( XNetUnregisterKey( &pServer->SessionID )==S_OK );
                RegisteredKeyCount--;
            }
        }
        break;

    case MATCH_CLIENT_ACTIVE:
        SetConnectStatus( MATCH_CONN_DISCONNECTED );
        VERIFY( XNetUnregisterKey( &g_ActiveConfig.GetConfig().SessionID )==S_OK );
        RegisteredKeyCount--;
        g_ActiveConfig.GetConfig().KeyIsRegistered = FALSE;
        g_PendingConfig.GetConfig().KeyIsRegistered = FALSE;
        break;

    default:
        break;
    }
}

//=============================================================================

void match_mgr::EnterState( match_mgr_state NewState )
{
    m_StateRetries  = STATE_RETRIES;
    SetTimeout(STATE_TIMEOUT);

    switch( NewState )
    {
    case MATCH_CONNECT_MATCHMAKER:
        SetTimeout( 20.0f );
        break;
    case MATCH_SILENT_LOGON:
        m_ActiveAccount = 0;
        break;
    case MATCH_NAT_NEGOTIATE:
        SetConnectStatus( MATCH_CONN_CONNECTING );
        break;
    case MATCH_INDIRECT_LOOKUP:
        {
            SetConnectStatus( MATCH_CONN_CONNECTING );
            if( m_SearchTaskHandle )
            {
                XOnlineTaskClose( m_SearchTaskHandle );
            }
            // Issue another matchmaker lookup so that we can get the absolutely correct encryption keys
            // for this particular console.
            HRESULT Result = XOnlineMatchSessionFindFromID( g_ActiveConfig.GetConfig().SessionID, NULL, &m_SearchTaskHandle );
            // If this returns some sort of error, then this means the session id is no longer valid but the error
            // must be detected in the Update call instead of in an EnterState call since the state will be changed.
            if( Result!=S_OK )
            {
                m_SearchTaskHandle=NULL;
            }
        }
        break;

    case MATCH_SERVER_ACTIVE:
        if( m_RegistrationComplete )
        {
            SetTimeout( REGISTRATION_INTERVAL );
        }
        else
        {
            SetTimeout( REGISTRATION_INTERVAL / 4.0f );
        }
        break;

    case MATCH_ACQUIRE_SERVERS:
        m_ExtendedServerInfoOwner = -1;
        ClearAllKeys( TRUE );
        break;

    case MATCH_LOGIN:
        // ** NOTE ** the negotiation callback will advance the state to either
        // MATCH_IDLE if there is an error, or MATCH_CLIENT_ACTIVE if negotiation completed.
        LOG_APP_NAME( "CLIENT" );

        // Make sure we're not trying to get any data
        g_NetworkMgr.BeginLogin();
        LockLists();
        ClearAllKeys( FALSE );
        m_LobbyList.Clear();
        UnlockLists();
        m_LocalIsServer = FALSE;
        break;

    case MATCH_ACQUIRE_EXTENDED_INFO:
        m_StateRetries = 3;
        SetConnectStatus( MATCH_CONN_CONNECTED );
    default:
        break;
    }

    m_State = NewState;
}

//=============================================================================

void match_mgr::CheckVisibility(void)
{
    if( !m_IsVisible )
    {
    }
}

//=============================================================================
// SetState will change internal state and initiate any packets that are
// required for the processing of the state just entered.
//=============================================================================

void match_mgr::SetState( match_mgr_state NewState )
{
    SetTimeout( STATE_TIMEOUT );

    LOG_MESSAGE( "match_mgr::SetState", "Old:%s - New:%s", GetStateName(m_State), GetStateName(NewState) );

    if( NewState != m_State )
    {
        ExitState ( m_State  );
        EnterState( NewState );
    }

    // The following actions happen every time the state is set whether or not it has just
    // been entered. Good place to initiate requests which may get a retry.
    switch( NewState )
    {
    case MATCH_CONNECT_MATCHMAKER       : SetStateConnectMatchMaker();      break;
    case MATCH_GET_MESSAGES             : SetStateGetMessages();            break;
    case MATCH_VALIDATE_PLAYER_NAME     : SetStateValidatePlayerName();     break;
    case MATCH_ACQUIRE_SERVERS          : SetStateAcquireServers();         break;
    case MATCH_REGISTER_SERVER          : SetStateRegisterServer();         break;
    case MATCH_UPDATE_SERVER            : SetStateUpdateServer();           break;
    case MATCH_VALIDATE_SERVER_NAME     : SetStateValidateServerName();     break;
    case MATCH_SERVER_CHECK_VISIBLE     : SetStateServerCheckVisible();     break;
    case MATCH_UNREGISTER_SERVER        : SetStateUnregisterServer();       break;
    case MATCH_SERVER_ACTIVE            : SetStateServerActive();           break;
    default                             :                                   break;
    }

    LOG_FLUSH();
}

//=============================================================================

void match_mgr::SetStateConnectMatchMaker( void )
{
    SetConnectStatus( MATCH_CONN_CONNECTING );
    g_NeedAccountEnumeration = TRUE;
}

//=============================================================================

void match_mgr::SetStateGetMessages( void )
{
    SetConnectStatus( MATCH_CONN_UNAVAILABLE );
}

//=============================================================================

void match_mgr::SetStateValidatePlayerName( void )
{
}

//=============================================================================

void match_mgr::SetStateAcquireServers( void )
{
    SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
    SetTimeout( LOOKUP_RETRY_INTERVAL );
    ClearAllKeys( TRUE );
    m_LookupTimeout = 0.0f;

    if( g_EnableLANLookup == FALSE )
    {
        LOG_MESSAGE( "match_mgr::SetStateAcquireServers", "Before m_SearchTaskHandle 0x%", m_SearchTaskHandle);
        // Set up the query attributes. Right now, we only use gametypeid and min number of players
        if( m_FilterMinPlayers != -1 )
        {
            m_QueryAttributes[0].dwAttributeID          = X_ATTRIBUTE_DATATYPE_INTEGER;
            m_QueryAttributes[0].info.integer.qwValue   = m_FilterMinPlayers;
        }
        else
        {
            m_QueryAttributes[0].dwAttributeID          = X_ATTRIBUTE_DATATYPE_NULL;
            m_QueryAttributes[0].info.integer.qwValue   = X_MATCH_NULL_INTEGER;
        }
        // gametype
        if( m_FilterGameType != GAME_MP )
        {
            m_QueryAttributes[1].dwAttributeID          = X_ATTRIBUTE_DATATYPE_INTEGER;
            m_QueryAttributes[1].info.integer.qwValue   = m_FilterGameType;
        }
        else
        {
            m_QueryAttributes[1].dwAttributeID          = X_ATTRIBUTE_DATATYPE_NULL;
            m_QueryAttributes[1].info.integer.qwValue   = X_MATCH_NULL_INTEGER;
        }
        LOG_MESSAGE( "match_mgr::SetStateAcquireServers", "Filter GameTypeID:%d", m_FilterGameType );
        // mutation mode
        if (m_FilterMutationMode != -1)
        {
            m_QueryAttributes[2].dwAttributeID          = X_ATTRIBUTE_DATATYPE_INTEGER;
            m_QueryAttributes[2].info.integer.qwValue   = m_FilterMutationMode;
        }
        else
        {
            m_QueryAttributes[2].dwAttributeID          = X_ATTRIBUTE_DATATYPE_NULL;
            m_QueryAttributes[2].info.integer.qwValue   = X_MATCH_NULL_INTEGER;
       }
        // password
        if (m_FilterPassword != -1)
        {
            m_QueryAttributes[3].dwAttributeID          = X_ATTRIBUTE_DATATYPE_INTEGER;
            m_QueryAttributes[3].info.integer.qwValue   = m_FilterPassword;
        }
        else
        {
            m_QueryAttributes[3].dwAttributeID          = X_ATTRIBUTE_DATATYPE_NULL;
            m_QueryAttributes[3].info.integer.qwValue   = X_MATCH_NULL_INTEGER;
       }
        // headset
        if (m_FilterHeadset != -1)
        {
            m_QueryAttributes[4].dwAttributeID          = X_ATTRIBUTE_DATATYPE_INTEGER;
            m_QueryAttributes[4].info.integer.qwValue   = m_FilterHeadset;
        }
        else
        {
            m_QueryAttributes[4].dwAttributeID          = X_ATTRIBUTE_DATATYPE_NULL;
            m_QueryAttributes[4].info.integer.qwValue   = X_MATCH_NULL_INTEGER;
        }

        s32 ResultsLength = XOnlineMatchSearchResultsLen( MAX_MATCH_RESULTS, sizeof(AttributeSpec)/sizeof(AttributeSpec[0]), AttributeSpec );
        HRESULT Result    = XOnlineMatchSearch( 1,                         // dwProcedureIndex (for sample program, 
            MAX_MATCH_RESULTS,         // dwNumResults
            ATTR_QUERY_KEY_COUNT,      // dwNumAttributes
            m_QueryAttributes,         // pAttributes
            ResultsLength,             // dwResultsLength
            NULL,                      // hWorkEvent
            &m_SearchTaskHandle);      // phTask

        LOG_MESSAGE( "match_mgr::SetStateAcquireServers", "m_SearchTaskHandle: 0x%x  Result: 0x%x", m_SearchTaskHandle,Result);
        ASSERT( Result == S_OK );
    }

    SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
}

//=============================================================================

void match_mgr::InitServerChanges( void )
{
    s32 AttribCount;
    server_info& Info = g_ActiveConfig.GetConfig();

    AttribCount = 0;

    x_memset(m_Attributes,0,sizeof(m_Attributes));

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_HOSTNAME_KEY;
    m_Attributes[ AttribCount++].info.string.lpValue   = Info.Name;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_MISSIONNAME_KEY;
    m_Attributes[ AttribCount++].info.string.lpValue   = Info.MissionName;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_GAMETYPE_KEY;
    m_Attributes[ AttribCount++].info.string.lpValue   = Info.GameType;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_SHORTGAMETYPE_KEY;
    m_Attributes[ AttribCount++].info.string.lpValue   = Info.ShortGameType;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_VERSION_KEY;
    m_Attributes[ AttribCount++].info.integer.qwValue  = g_ServerVersion;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_FLAGS_KEY;
    m_Attributes[ AttribCount++].info.integer.qwValue  = Info.Flags;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_GAMETIME_KEY;
    m_Attributes[ AttribCount++].info.integer.qwValue  = Info.GameTime;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_FRAGLIMIT_KEY;
    m_Attributes[ AttribCount++].info.integer.qwValue  = Info.FragLimit;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_PORT_KEY;
    m_Attributes[ AttribCount++].info.integer.qwValue  = m_pSocket->GetPort();

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_GAMETYPE_ID_KEY;
    m_Attributes[ AttribCount++].info.integer.qwValue  = Info.GameTypeID;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_MIN_PLAYERS_KEY;
    m_Attributes[ AttribCount++].info.integer.qwValue  = Info.MaxPlayers;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_MUTATION_MODE_KEY;
    m_Attributes[ AttribCount++].info.integer.qwValue  = Info.MutationMode;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_PASSWORD_KEY;
    m_Attributes[ AttribCount++].info.integer.qwValue  = Info.PasswordEnabled;

    m_Attributes[ AttribCount  ].fChanged              = TRUE;
    m_Attributes[ AttribCount  ].dwAttributeID         = ATTR_HEADSET;
    m_Attributes[ AttribCount++].info.integer.qwValue  = Info.VoiceEnabled;

    ASSERT( AttribCount == ATTR_KEY_COUNT );

    m_RegistrationComplete = FALSE;
    m_UpdateRegistration   = FALSE;
    SetConnectStatus( MATCH_CONN_REGISTERING );
}

//=============================================================================

void match_mgr::SetStateRegisterServer( void )
{
    InitServerChanges();

    if( (g_EnableLANLookup == FALSE) && (GetAuthStatus()==AUTH_STAT_CONNECTED) )
    {
        server_info& Info = g_ActiveConfig.GetConfig();
        LOG_MESSAGE( "match_mgr::SetStateRegisterServer", "SetStateRegisterServer" );

        s32 OpenPrivateSlots, UsedPrivateSlots;
        s32 OpenPublicSlots, UsedPublicSlots;

        if( Info.Flags & SERVER_IS_PRIVATE )
        {
            UsedPrivateSlots = Info.Players;
            OpenPrivateSlots = Info.MaxPlayers - Info.Players;
            UsedPublicSlots  = 0;
            OpenPublicSlots  = 0;
        }
        else
        {
            UsedPrivateSlots = 0;
            OpenPrivateSlots = 0;
            UsedPublicSlots  = Info.Players;
            OpenPublicSlots  = Info.MaxPlayers - Info.Players;
        }

        HRESULT Result = XOnlineMatchSessionCreate( UsedPublicSlots,                                // Public filled
                                                    OpenPublicSlots,                                // Public open
                                                    UsedPrivateSlots,                               // Private filled
                                                    OpenPrivateSlots,                               // Private open
                                                    ATTR_KEY_COUNT,                                 // Number of Attributes
                                                    m_Attributes,                                   // Attribute list
                                                    NULL,                                           // Work Event
                                                    &m_SessionTaskHandle );                         // Task handle
        ASSERT( Result == S_OK );
    }
}

//=============================================================================

void match_mgr::SetStateUpdateServer( void )
{
    InitServerChanges();

    if( (g_EnableLANLookup == FALSE) && (GetAuthStatus()==AUTH_STAT_CONNECTED) )
    {
        XNKID SessionID;
        XOnlineMatchSessionGetInfo( m_SessionTaskHandle, &SessionID, &m_KeyExchangeKey );

        #ifndef X_RETAIL
        LOG_MESSAGE( "match_mgr::SetStateUpdateServer", "XOnlineMatchSessionGetInfo SessionID: %s" , SessionIDString( g_ActiveConfig.GetConfig().SessionID ) );
        #endif

        server_info& Info = g_ActiveConfig.GetConfig();
        s32 OpenPrivateSlots, UsedPrivateSlots;
        s32 OpenPublicSlots, UsedPublicSlots;

        if( Info.Flags & SERVER_IS_PRIVATE )
        {
            UsedPrivateSlots = Info.Players;
            OpenPrivateSlots = Info.MaxPlayers - Info.Players;
            UsedPublicSlots  = 0;
            OpenPublicSlots  = 0;
        }
        else
        {
            UsedPrivateSlots = 0;
            OpenPrivateSlots = 0;
            UsedPublicSlots  = Info.Players;
            OpenPublicSlots  = Info.MaxPlayers - Info.Players;
        }

        HRESULT Result = XOnlineMatchSessionUpdate( SessionID, 
            UsedPublicSlots,                                // Public filled
            OpenPublicSlots,                                // Public open
            UsedPrivateSlots,                               // Private filled
            OpenPrivateSlots,                               // Private open
            ATTR_KEY_COUNT,                                 // Number of Attributes
            m_Attributes,                                   // Attribute list
            NULL,                                           // Work Event
            &m_UpdateTaskHandle );                          // Task handle
        ASSERT( Result == S_OK );
    }
}

//=============================================================================

void match_mgr::SetStateValidateServerName( void )
{
    /*
    if (m_LastConnectionState == CMatchmaker::CONNECTION_CONNECTED)
    {
    LOG_MESSAGE( "MatchMaker", "GetCleanText( \"%s\" )", m_ActiveServerConfig.Name );
    m_pMatchMaker->GetCleanText(m_ActiveServerConfig.Name);
    }
    */
}

//=============================================================================

void match_mgr::SetStateServerCheckVisible( void )
{
    /*
    {
    // If we already know we're visible, then we're not interested in doing this test
    // again.
    if (m_IsVisible && (!m_ActiveServerConfig.Remote.IsEmpty()) )
    {
    SetState(MATCH_IDLE);
    break;
    }

    u32 ip,port;

    m_pMatchMaker->ReadMatchmakerAddress(ip,port);

    ip = BIG_ENDIAN_32(ip);
    LOG_MESSAGE("match_mgr::SetState","ReadMatchmakerAddress returned %s",net_address(ip,port).GetStrAddress(Buffer));
    if ( (ip != 0) && (port != 0) )
    {
    m_EchoAddress = net_address(ip,port);
    }
    else
    {
    m_EchoAddress = s_DefaultEchoServer;
    }

    m_pSocket->Send(m_EchoAddress,ECHO_REQUEST_MESSAGE,sizeof(ECHO_REQUEST_MESSAGE));
    LOG_MESSAGE("match_mgr::SetState","Echo request sent to %s via %s",m_EchoAddress.GetStrAddress(Buffer), m_pSocket->GetStrAddress(Buffer2) );
    }
    */
}

//=============================================================================

void match_mgr::SetStateUnregisterServer( void )
{
    SetConnectStatus( MATCH_CONN_DISCONNECTED );
    m_LocalIsServer = FALSE;

    if( g_EnableLANLookup == FALSE )
    {
        if( m_SessionTaskHandle )
        {
            XOnlineTaskClose( m_SessionTaskHandle );
            HRESULT Result = XOnlineMatchSessionDelete( g_ActiveConfig.GetConfig().SessionID, NULL, &m_SessionTaskHandle );
        }
    }

    LOG_MESSAGE("match_mgr::SetState","Unregister Server");
}

//=============================================================================

void match_mgr::SetStateServerActive( void )
{
    SetUserStatus( BUDDY_STATUS_INGAME );
}

//=============================================================================

xbool match_mgr::ReceivePacket( net_address& Remote, bitstream& Bitstream )
{
    net_request* pPacket;

    // If no local socket set up, we have not yet been initialized and so if we
    // receive a packet, let's forget about it.
    if( !m_pSocket )
        return( FALSE );

    pPacket = (net_request*)Bitstream.GetDataPtr();

    switch( pPacket->LookupRequest.Type )
    {
    case NET_PKT_EXTENDED_LOOKUP_REQUEST:
        if( Remote == m_pSocket->GetAddress() || (GameMgr.GameInProgress()==FALSE) || (GameMgr.GetGameType() == GAME_CAMPAIGN) )
        {
            return FALSE;
        }

        if( m_LocalIsServer )
        {
            net_extended_lookup_response Response;
            const game_score& Score=GameMgr.GetScore();
            s32 i;

            x_memset( &Response, 0, sizeof(Response) );

            Response.Type       = NET_PKT_EXTENDED_LOOKUP_RESPONSE;

            Response.PlayerCount    = Score.NPlayers;
            Response.IsTeamBased    = Score.IsTeamBased;
            Response.ScoreFieldMask = Score.ScoreFieldMask;
            x_strcpy( Response.TeamName[0], Score.Team[0].NName );
            x_strcpy( Response.TeamName[1], Score.Team[1].NName );

            s32 PlayerIndex = 0;

            for( i=0; i<NET_MAX_PLAYERS; i++ )
            {
                if( Score.Player[i].IsInGame )
                {
                    Response.PlayerScore[PlayerIndex]   = Score.Player[i].Score;
                    Response.PlayerTeam[PlayerIndex]    = Score.Player[i].Team;
                    Response.PlayerDeaths[PlayerIndex]  = Score.Player[i].Deaths;
                    x_strcpy( Response.PlayerName[PlayerIndex], Score.Player[i].NName );
                    PlayerIndex++;
                }
            }
            Response.PingIndex  = pPacket->ExtendedLookupRequest.PingIndex;
            Response.Owner      = pPacket->ExtendedLookupRequest.Owner;

            LOG_MESSAGE( "match_mgr::ReceivePacket", "Extended Lookup Response sent %s via %s", Remote.GetStrAddress(), m_pSocket->GetStrAddress(), Response.PingIndex );
            m_pSocket->Send( Remote, &Response, sizeof(Response) );

        }
        break;
    case NET_PKT_LOOKUP_REQUEST:
        if( Remote == m_pSocket->GetAddress() )
        {
            return FALSE;
        }

        if( (GameMgr.GameInProgress()==FALSE) || (GameMgr.GetGameType() == GAME_CAMPAIGN) )
        {
            return FALSE;
        }

        if( m_LocalIsServer && (pPacket->LookupRequest.Version == g_ServerVersion) )
        {
            net_lookup_response Response;
            const server_info& Config = g_ActiveConfig.GetConfig();
            const game_score& Score = GameMgr.GetScore();

            Response.Type                     = NET_PKT_LOOKUP_RESPONSE;
            Response.Version                  = g_ServerVersion;
            Response.PingIndex                = pPacket->LookupRequest.PingIndex;
            Response.CurrentPlayers           = Score.NPlayers;
            Response.MaxPlayers               = Config.MaxPlayers;
            Response.CurrentLevel             = Config.Level;
            Response.FragLimit                = Config.FragLimit;
            Response.TimeLimit                = Config.GameTime;
            Response.MutationMode             = Config.MutationMode;
            Response.Progress                 = (s8)(GameMgr.GetGameProgress()*100.0f);
            Response.Flags                    = Config.Flags;
            x_wstrcpy(Response.GameType,        Config.GameType);
            x_wstrcpy(Response.ShortGameType,   Config.ShortGameType);
            x_wstrcpy(Response.MissionName,     Config.MissionName );
            x_wstrcpy(Response.Name,            Config.Name);
            x_memcpy(&Response.SessionID,      &Config.SessionID,    sizeof(Response.SessionID) );
            x_memcpy(&Response.Address,        &Config.Address,      sizeof(Response.Address) );
            x_memcpy(&Response.ExchangeKey,    &Config.ExchangeKey,  sizeof(Response.ExchangeKey) );

            LOG_MESSAGE( "match_mgr::ReceivePacket", "Lookup Response sent %s via %s, PingIndex:%d", Remote.GetStrAddress(), m_pSocket->GetStrAddress(), Response.PingIndex );
            m_pSocket->Send( Remote, &Response, sizeof(Response) );
        }
        break;

    case NET_PKT_LOOKUP_RESPONSE:
        //x_DebugMsg("Got a lookup response from %s\n",pResponse->Name);
        // Now go through and add this to the list of servers we currently know
        // about.
        AppendServer(Remote,pPacket->LookupResponse);
        break;

    case NET_PKT_EXTENDED_LOOKUP_RESPONSE:
        if( pPacket->ExtendedLookupResponse.Owner == m_ExtendedServerInfoOwner )
        {
            game_score& Score = m_ExtendedServerInfo.Score;
            s32 i;

            x_memset( &Score, 0, sizeof(Score) );

            Score.NPlayers          = pPacket->ExtendedLookupResponse.PlayerCount;
            Score.IsTeamBased       = pPacket->ExtendedLookupResponse.IsTeamBased;
            Score.ScoreFieldMask    = pPacket->ExtendedLookupResponse.ScoreFieldMask;
            x_strcpy( Score.Team[0].NName, pPacket->ExtendedLookupResponse.TeamName[0] );
            x_mstrcpy( Score.Team[0].Name, pPacket->ExtendedLookupResponse.TeamName[0] );
            if( Score.IsTeamBased )
            {
                x_strcpy( Score.Team[1].NName, pPacket->ExtendedLookupResponse.TeamName[1] );
                x_mstrcpy( Score.Team[1].Name, pPacket->ExtendedLookupResponse.TeamName[1] );
            }
            for( i=0; i<Score.NPlayers; i++ )
            {
                Score.Player[i].IsConnected = TRUE;
                Score.Player[i].IsInGame    = TRUE;
                Score.Player[i].Score       = pPacket->ExtendedLookupResponse.PlayerScore[i];
                Score.Player[i].Team        = pPacket->ExtendedLookupResponse.PlayerTeam[i];
                Score.Player[i].Deaths      = pPacket->ExtendedLookupResponse.PlayerDeaths[i];
                x_strcpy( Score.Player[i].NName, pPacket->ExtendedLookupResponse.PlayerName[i] );
                x_mstrcpy( Score.Player[i].Name, pPacket->ExtendedLookupResponse.PlayerName[i] );
            }
            SetConnectStatus( MATCH_CONN_CONNECTED );
            SetState( MATCH_ACQUIRE_IDLE );

            server_info* pServer = GetServerInfo( m_ExtendedServerInfoOwner );
            // The server key should have been registeed when the extended request was sent
            if( pServer->KeyIsRegistered )
            {
                VERIFY( XNetUnregisterKey( &pServer->SessionID )==S_OK );
                RegisteredKeyCount--;
            }

            pServer->KeyIsRegistered = FALSE;
        }
        break;
    default:
        break;
    }

    return( TRUE );
}

//=============================================================================

xbool match_mgr::SendLookup( net_address& Remote )
{
    net_request Request;

    Request.LookupRequest.Type       = NET_PKT_LOOKUP_REQUEST;
    Request.LookupRequest.Version    = g_ServerVersion;
    if( m_NeedNewPing )
    {
        m_PingIndex++;
        if (m_PingIndex >= MAX_PING_OUTSTANDING)
        {
            m_PingIndex = 0;
        }
        m_PingSendTimes[m_PingIndex]=x_GetTime();
        m_NeedNewPing = FALSE;
    }
    Request.LookupRequest.PingIndex  = m_PingIndex;
    x_strcpy(Request.LookupRequest.Name,"Local User");

    LOG_MESSAGE( "match_mgr::SendLookup", "Lookup request sent to %s via %s, PingIndex:%d", Remote.GetStrAddress(), m_pSocket->GetStrAddress(), m_PingIndex );
    m_pSocket->Send( Remote, &Request.LookupRequest, sizeof( Request.LookupRequest ) );

    return FALSE;
}

//=============================================================================
// This append server instance is called when a response is received from a lookup
// request OR a machine has responded to the local network lookup.
//=============================================================================

void match_mgr::AppendServer( const net_address& Remote, const net_lookup_response& Response )
{
    // Do some validation
    if( Response.Version != g_ServerVersion )
        return;

    s32     Index;
    server_info Info;

    x_memset( &Info, 0, sizeof(Info) );

    Info.PingDelay                = GetPingTime( Response.PingIndex );
    Info.Remote                   = Remote;
    Info.Retries                  = 0;
    Info.RetryTimeout             = 0.0f;
    Info.Version                  = Response.Version;
    Info.Players                  = Response.CurrentPlayers;
    Info.MaxPlayers               = Response.MaxPlayers;
    Info.Level                    = Response.CurrentLevel;
    Info.FragLimit                = Response.FragLimit;
    Info.GameTime                 = Response.TimeLimit;
    Info.Flags                    = Response.Flags;
    Info.PercentComplete          = Response.Progress;
    Info.MutationMode             = (mutation_mode)Response.MutationMode;
    x_wstrncpy( Info.GameType,      Response.GameType,      sizeof(Info.GameType) / sizeof(xwchar) );
    x_wstrncpy( Info.ShortGameType, Response.ShortGameType, sizeof(Info.ShortGameType) / sizeof(xwchar) );
    x_wstrncpy( Info.MissionName,   Response.MissionName,   sizeof(Info.MissionName) / sizeof(xwchar) );
    x_wstrncpy( Info.Name,          Response.Name,          sizeof(Info.Name) / sizeof(xwchar) );
    x_memcpy( &Info.SessionID,      &Response.SessionID,    sizeof(Response.SessionID) );
    x_memcpy( &Info.Address,        &Response.Address,      sizeof(Response.Address) );
    x_memcpy( &Info.ExchangeKey,    &Response.ExchangeKey,  sizeof(Response.ExchangeKey) );

    LOG_MESSAGE( "match_mgr::AppendServer", "Lookup response received from %s, name:%s, PingIndex:%d, ping:%2.02f", Remote.GetStrAddress(), (const char*)xstring(Info.Name), Response.PingIndex, Info.PingDelay );


    // Now go through the Pending list and remove this lookup
    Index = m_PendingResponseList.Find(Info);
    if( Index >= 0 )
    {
        if( m_PendingResponseList[Index].KeyIsRegistered )
        {
            HRESULT Result;
            LOG_MESSAGE( "match_mgr::AppendServer", "Unregistered key %s", SessionIDString(m_PendingResponseList[Index].SessionID)  );
            Result = XNetUnregisterKey( &m_PendingResponseList[Index].SessionID );
            RegisteredKeyCount--;
            m_PendingResponseList[Index].KeyIsRegistered = FALSE;
        }
        x_memcpy( &Info.ExchangeKey, &m_PendingResponseList[Index].ExchangeKey, sizeof(Info.ExchangeKey) );
        x_memcpy( &Info.Address, &m_PendingResponseList[Index].Address, sizeof(Info.Address) ); 
        m_PendingResponseList.Delete(Index);
    }

    //
    // see if any of our buddies are on this host
    //
    Info.Flags &= ~SERVER_HAS_BUDDY; // not at first...
    s32 i;
    const s32 nBuddies = GetBuddyCount();
    for ( i = 0; i < nBuddies; ++i )
    {
        buddy_info Buddy = GetBuddy( i );

        const void* s1 = (const void*)(&(Buddy.SessionID));
        const void* s2 = (const void*)(&(Info.SessionID ));
        const xbool HasBuddy = (x_memcmp( s1, s2, sizeof( XNKID ) ) == 0);

        if ( HasBuddy  )
        {
            // It only takes one, set the flag, and get out
            Info.Flags |= SERVER_HAS_BUDDY;
            break;
        }
    }


    // Search the server list to see if we already have an entry. If we do, then
    // just update the individual fields, otherwise append this entry to the main
    // list.
    Index = FindServerInfo(Info);
    if( Index >=0 )
    {
        server_info* pServerInfo = GetServerInfo( Index );
        
        ASSERT( pServerInfo->KeyIsRegistered == FALSE );
        // Some fields should NOT be overwritten when an update has come in.
        Info.Flags = pServerInfo->Flags;

        LOG_MESSAGE( "match_mgr::AppendServer", "Already in list, replaced entry %d with id %d", Index, pServerInfo->ID );

        Info.ID = Index;

        x_memcpy( pServerInfo, &Info, sizeof(server_info) );
    }
    else
    {
        Info.ID = GetServerCount();

        LOG_MESSAGE( "match_mgr::AppendServer", "Appended as entry %d", Info.ID );


        AppendToServerList(Info);
    }
}

//=============================================================================

void match_mgr::LocalLookups( f32 DeltaTime )
{
    if( g_EnableLANLookup == TRUE )
    {
        m_LookupTimeout -= DeltaTime;

        if( m_LookupTimeout < 0.0f )
        {
            SendLookup( m_BroadcastAddress );
            m_LookupTimeout = LOOKUP_RETRY_INTERVAL;
        }
    }
}
//=============================================================================

f32 match_mgr::GetPingTime( s32 Index )
{
    return x_TicksToMs( x_GetTime() - m_PingSendTimes[Index % MAX_PING_OUTSTANDING] );
}

//=============================================================================

void match_mgr::DisconnectFromMatchmaker( void )
{
    // XBOX never actually disconnects UNLESS it is explicitly told to do so. This
    // is used to make sure you remain signed on when you go offline.
    SetConnectStatus( MATCH_CONN_DISCONNECTED );
}

//=============================================================================

void match_mgr::ConnectToMatchmaker( match_mgr_state PendingState )
{
    m_PendingState = PendingState;
    SetConnectStatus( MATCH_CONN_CONNECTING );
}

//------------------------------------------------------------------------------
//==============================================================================

xbool match_mgr::ValidateLobbyInfo( const lobby_info &info )
{
    /*
    struct lobby_info
    {
    s32             ID;
    s32             ServerCount;
    s32             Flags;
    char            Name[SERVER_NAME_LENGTH];
    };
    */

    ASSERT( info.Name[0] != 0 );
    if( !( info.Name[0] != 0 ) )
    {
        LOG_WARNING( "match_mgr::ValidateLobbyInfo", "Invalid Lobby" );
        return FALSE;
    }

    return TRUE;
}

//==============================================================================

xbool match_mgr::CheckSecurityChallenge( const char* pChallenge )
{
    (void)pChallenge;
    SetConnectStatus( MATCH_CONN_CONNECTED );
    SetState( m_PendingState );
    m_PendingState = MATCH_IDLE;
    return TRUE;
}

//==============================================================================

const byte* SAFE_LOW = (const byte*)(512*1024);
const byte* SAFE_HIGH= (const byte*)(32*1024*1024);

void match_mgr::IssueSecurityChallenge( void )
{
}

//==============================================================================

void match_mgr::NotifyKick( const char* UniqueId )
{
    (void)UniqueId;
}

//==============================================================================

void match_mgr::RemoteLookups( f32 DeltaTime )
{
    DWORD                           SessionCount;
    s32                             i;
    HRESULT                         Result;
    XONLINE_MATCH_SEARCHRESULT**    ppSearchResults;
    XONLINE_MATCH_SEARCHRESULT*     pSearchResult;
    server_info                     ServerInfo;
    IN_ADDR                         Addr;
    net_address                     Remote;
    s32                             Index;
    s_query_result                  QueryResults;
    s32                             Count;
    xbool                           StillPending;

    // First, send any retries to servers to make sure they are visible to us.
    Count = 0;
    StillPending = FALSE;
    for( i = 0; i < m_PendingResponseList.GetCount(); i++ )
    {
        server_info& Info = m_PendingResponseList[i];

        // We can only send to a server if we have it's key registered. If it's
        // not registered, then we try to register it.
        if( Info.Retries >= 0 )
        {
            StillPending = TRUE;
            if( Info.KeyIsRegistered )
            {
                Info.RetryTimeout -= DeltaTime;
                if( Info.RetryTimeout < 0.0f )
                {
                    // Need to do a retry?
                    Info.Retries--;
                    if( Info.Retries >= 0 )
                    {
                        SendLookup( Info.Remote );
                        // Keep track of the # of lookups sent this time, if too many
                        // then bail now (2 per game cycle should be enough)
                        Count++;
                    }
                    else
                    {
                        // At this point, we *should* remove it from the pending response list but we do not.
                        // There is a really good reason for this but I cannot recall at the moment.
                        LOG_WARNING( "match_mgr::RemoteLookups","Server expired. Name:%s, Addr:%s, GameType:%s, ShortGameType:%s, MapName:%s",
                            (const char*)xstring(Info.Name),
                            Info.Remote.GetStrAddress(),
                            (const char*)xstring(Info.GameType),
                            (const char*)xstring(Info.ShortGameType),
                            (const char*)xstring(Info.MissionName) );
                        VERIFY( XNetUnregisterKey( &Info.SessionID )==S_OK );
                        Info.KeyIsRegistered = FALSE;
                    }
                    Info.RetryTimeout = LOOKUP_RETRY_INTERVAL;
                    if( Count > 2 )
                    {
                        break;
                    }
                }
            }
            else
            {
                VERIFY( XNetRegisterKey( &Info.SessionID, &Info.ExchangeKey )==S_OK );
                RegisteredKeyCount++;
                LOG_MESSAGE( "match_mgr::RemoteLookups", "Key registered: 0x%08x:%08x:%08x:%08x, SessionID: 0x%08x:%08x", 
                    *(s32*)&Info.ExchangeKey.ab[0],
                    *(s32*)&Info.ExchangeKey.ab[4],
                    *(s32*)&Info.ExchangeKey.ab[8],
                    *(s32*)&Info.ExchangeKey.ab[12],
                    *(s32*)&Info.SessionID.ab[0],
                    *(s32*)&Info.SessionID.ab[4]);

                Result = XNetXnAddrToInAddr( &Info.Address, &Info.SessionID, &Addr );
                ASSERT( Result == S_OK );
                Info.Remote.SetIP( ntohl(Addr.s_addr) );
                Info.KeyIsRegistered = TRUE;
            }
        }
    }

    // Now, deal with a search result should it come in.
    if( m_SearchTaskHandle == NULL )
    {
        if( StillPending==FALSE )
        {
            SetState( MATCH_IDLE );
            m_bAcquisitionComplete = TRUE;
        }
        return;
    }

    if( g_EnableLANLookup == TRUE )
        ASSERT( FALSE );

    if( (m_SearchTaskResult != XONLINETASK_S_RUNNING) && SUCCEEDED( m_SearchTaskResult ) )
    {
        Result = XOnlineMatchSearchGetResults( m_SearchTaskHandle, &ppSearchResults, &SessionCount );

        // For each succeeded search, scan through the search results, change them
        // to the internal format and then add that to our matched server list.
        if( SUCCEEDED( Result ) )
        {
            for( i=0; i < (s32)SessionCount; i++ )
            {
                pSearchResult = ppSearchResults[i];

                // Grab all the attributes for that server
                XOnlineMatchSearchParse( pSearchResult, ATTR_KEY_COUNT, AttributeSpec, &QueryResults );
                x_memset( &ServerInfo, 0, sizeof(ServerInfo) );

                if( QueryResults.Version == g_ServerVersion )
                {
                    IN_ADDR Addr;

                    ServerInfo.PingDelay      = 0;
                    ServerInfo.Retries        = 3;
                    ServerInfo.RetryTimeout   = 0.0f;
                    ServerInfo.Players        = pSearchResult->dwPublicFilled;
                    ServerInfo.MaxPlayers     = pSearchResult->dwPublicOpen + pSearchResult->dwPublicFilled;
                    ServerInfo.Version        = (s32)QueryResults.Version;
                    ServerInfo.FragLimit      = (s32)QueryResults.FragLimit;
                    ServerInfo.GameTime       = (s32)QueryResults.GameTime;
                    ServerInfo.Flags          = (s32)QueryResults.Flags;

                    x_wstrncpy( ServerInfo.GameType,        QueryResults.GameType,          sizeof(ServerInfo.GameType)/sizeof(xwchar) );
                    x_wstrncpy( ServerInfo.ShortGameType,   QueryResults.ShortGameType,     sizeof(ServerInfo.ShortGameType)/sizeof(xwchar) );
                    x_wstrncpy( ServerInfo.MissionName,     QueryResults.MapName,           sizeof(ServerInfo.MissionName)/sizeof(xwchar) );
                    x_wstrncpy( ServerInfo.Name,            QueryResults.Name,              sizeof(ServerInfo.Name)/sizeof(xwchar) );
                    x_memcpy(  &ServerInfo.SessionID,       &pSearchResult->SessionID,      sizeof(ServerInfo.SessionID) );
                    x_memcpy(  &ServerInfo.ExchangeKey,     &pSearchResult->KeyExchangeKey, sizeof(ServerInfo.ExchangeKey) );
                    x_memcpy(  &ServerInfo.Address,         &pSearchResult->HostAddress,    sizeof(ServerInfo.Address) );

                    // If this server is already in the server list, then it must have responded from a local
                    // lookup. So, we do nothing since that entry will be more up-to-date from the local lookup
                    // rather than from the matchmaker service lookup.
                    Index = FindServerInfo( ServerInfo );
                    if( Index < 0 )
                    {
                        Index = m_PendingResponseList.Find(ServerInfo);
                        if( Index < 0 )
                        {
                            VERIFY( XNetRegisterKey( &ServerInfo.SessionID, &ServerInfo.ExchangeKey )==S_OK );
                            RegisteredKeyCount++;
                            ServerInfo.KeyIsRegistered = TRUE;
#ifndef X_RETAIL
                            LOG_MESSAGE( "match_mgr::RemoteLookups", "Key registered: 0x%08x:%08x:%08x:%08x, SessionID: %s", 
                                *(s32*)&ServerInfo.ExchangeKey.ab[0],
                                *(s32*)&ServerInfo.ExchangeKey.ab[4],
                                *(s32*)&ServerInfo.ExchangeKey.ab[8],
                                *(s32*)&ServerInfo.ExchangeKey.ab[12],
                                SessionIDString( ServerInfo.SessionID ) );
#endif  
                            Result = XNetXnAddrToInAddr( &ServerInfo.Address, &ServerInfo.SessionID, &Addr );
                            ASSERT( Result == S_OK );
                            ServerInfo.Remote.Setup( ntohl( Addr.s_addr ), (s32)QueryResults.Port );

                            m_PendingResponseList.Append(ServerInfo);
                            LOG_MESSAGE( "match_mgr::RemoteLookups","Server appended. Ver:%d, Addr:%s, Name:%s, GameType:%s, GameTypeID:%d, ShortGameType:%s, MapName:%s",
                                ServerInfo.Version,
                                ServerInfo.Remote.GetStrAddress(),
                                (const char*)xstring(ServerInfo.Name),
                                (const char*)xstring(ServerInfo.GameType),
                                ServerInfo.GameTypeID,
                                (const char*)xstring(ServerInfo.ShortGameType),
                                (const char*)xstring(ServerInfo.MissionName) );

                        }
                    }
                    else
                    {
                        x_memcpy( GetServerInfo( Index ), &ServerInfo, sizeof(server_info) );
                    }
                }
            }
            XOnlineTaskClose( m_SearchTaskHandle );
            m_SearchTaskHandle = NULL;
        }
    }

    if( FAILED( m_SearchTaskResult ) )
    {
        XOnlineTaskClose( m_SearchTaskHandle );
        ASSERT(FALSE);
        SetState( MATCH_IDLE );
        m_SearchTaskHandle = NULL;
        m_bAcquisitionComplete = true;
    }
}
//==============================================================================

void match_mgr::SetUserAccount( s32 Index )
{
    // Establish the user profile we are to use and then advance to authentication
    ASSERT( GetState() == MATCH_IDLE );

    ASSERT( Index < XONLINE_MAX_STORED_ONLINE_USERS );
    m_ActiveAccount = Index;
    m_ActiveUserAccount = m_UserAccounts[Index];
    SetUniqueId( (const byte*)(m_ActiveUserAccount.Name), (USER_NAME_LENGTH * sizeof( xwchar )) );

    // MAB: Should we still be doing this???
    SetState( MATCH_AUTHENTICATE_USER );
}

//==============================================================================

const char* match_mgr::GetUserAccountName( s32 Index )
{
    ASSERT( Index < XONLINE_MAX_STORED_ONLINE_USERS );
    return (const char*)m_UserList[Index].szGamertag;
}

//==============================================================================

void match_mgr::BecomeClient( void )
{
    const server_info& Config = g_PendingConfig.GetConfig();
    ASSERT( Config.SessionID.ab[0] || Config.SessionID.ab[1] || Config.SessionID.ab[2] );
    game_config::Commit( Config );
    SetState( MATCH_NAT_NEGOTIATE );
}

//==============================================================================
void match_mgr::StartLogin( void )
{
    SetState( MATCH_LOGIN );
}

//==============================================================================
void match_mgr::StartIndirectLookup( void )
{
    game_config::Commit();
    SetState( MATCH_INDIRECT_LOOKUP );
}

//==============================================================================

xbool match_mgr::AddBuddy( const buddy_info& Buddy )
{
    if( g_EnableLANLookup == TRUE )
        return( FALSE );

    HRESULT Result;
    XUID    UserId;

    UserId.qwUserID     = Buddy.Identifier;
    UserId.dwUserFlags  = 0;

    if( m_FriendRequestTaskHandle != NULL )
    {
        XOnlineTaskClose( m_FriendRequestTaskHandle );
    }

    SetupMessage( XONLINE_MSG_TYPE_FRIEND_REQUEST );

    Result = XOnlineFriendsRequestEx( m_UserIndex,
                                      UserId,
                                      m_hMessage,
                                      NULL,
                                      &m_FriendRequestTaskHandle );

    if( Result == S_OK )
    {
        m_MessageState = MATCH_MESSAGE_SEND;
        return( TRUE );
    }
    else
    {
        m_MessageState = MATCH_MESSAGE_FAILED;
        return( FALSE );
    }
}

//==============================================================================

xbool match_mgr::DeleteBuddy( const buddy_info& Buddy )
{
    XONLINE_FRIEND  Friend;
    HRESULT         Result;

    x_memset( &Friend, 0, sizeof(Friend) );
    Friend.xuid.qwUserID = Buddy.Identifier;
    Result = XOnlineFriendsRemove( 0, &Friend );
    return ( Result == S_OK );
}

//==============================================================================

void match_mgr::AnswerBuddyRequest( buddy_info& Buddy, buddy_answer Answer )
{
    HRESULT         Result;
    XONLINE_FRIEND  Friend;

    x_memset( &Friend, 0, sizeof(Friend) );
    Friend.xuid.qwUserID = Buddy.Identifier;
    Result = XOnlineFriendsAnswerRequest( 0, &Friend, (XONLINE_REQUEST_ANSWER_TYPE)Answer );
    ASSERT( Result == S_OK );
}

//==============================================================================

xbool match_mgr::InviteBuddy( buddy_info& Buddy )
{
    if( g_EnableLANLookup == TRUE )
        return( FALSE );

    HRESULT Result;
    XUID    UserId;

    UserId.qwUserID     = Buddy.Identifier;
    UserId.dwUserFlags  = 0;

    if( m_InviteTaskHandle != NULL )
    {
        XOnlineTaskClose( m_InviteTaskHandle );
    }

    SetupMessage( XONLINE_MSG_TYPE_GAME_INVITE );

    Result = XOnlineGameInviteSend( m_UserIndex,
        1,
        &UserId,
        g_ActiveConfig.GetConfig().SessionID,
        0,
        m_hMessage,
        NULL,
        &m_InviteTaskHandle );

    m_MessageState = MATCH_MESSAGE_SEND;

    return( Result == S_OK );
}

//==============================================================================

void match_mgr::CancelBuddyInvite( buddy_info& Buddy )
{
    XUID    UserId;

    UserId.qwUserID     = Buddy.Identifier;
    UserId.dwUserFlags  = 0;

    if( m_InviteTaskHandle != 0 )
    {
        XOnlineTaskClose( m_InviteTaskHandle );
    }

    HRESULT Result = XOnlineGameInviteRevoke( 0,
        1,
        &UserId,
        g_ActiveConfig.GetConfig().SessionID,
        NULL,
        &m_InviteTaskHandle );
    ASSERT( Result == S_OK );
}

//==============================================================================

xbool match_mgr::AnswerBuddyInvite( buddy_info& Buddy, buddy_answer Answer )
{
    if( g_EnableLANLookup == TRUE )
        return( FALSE );

    XUID    UserId;
    XONLINE_GAMEINVITE_ANSWER_INFO Info;

    UserId.qwUserID     = Buddy.Identifier;
    UserId.dwUserFlags  = 0;

    if( m_InviteTaskHandle != 0 )
    {
        XOnlineTaskClose( m_InviteTaskHandle );
    }

    x_memset( &Info, 0, sizeof(Info) );
    x_strcpy( Info.szInvitingUserGamertag, Buddy.Name );
    Info.xuidInvitingUser.qwUserID  = Buddy.Identifier;
    Info.SessionID                  = Buddy.SessionID;
    Info.dwTitleID                  = Buddy.TitleID;

    HRESULT Result = XOnlineGameInviteAnswer( 0,
        &Info,
        (XONLINE_PEER_ANSWER_TYPE)Answer,
        NULL,
        &m_InviteTaskHandle );
    ASSERT( Result == S_OK );

    server_info& Config   = g_PendingConfig.GetConfig();
    Config.ConnectionType = CONNECT_INDIRECT;
    Config.SessionID      = Buddy.SessionID;

    return( TRUE );
}

//==============================================================================
// Returns TRUE if the buddy is in exactly the same game as the current player.
xbool match_mgr::JoinBuddy( buddy_info& Buddy )
{
    if( g_EnableLANLookup == TRUE )
        return FALSE;

    // We set up the g_PendingProfile structure with all the information we have
    // on the session we are trying to go to.
    server_info& Config     = g_PendingConfig.GetConfig();
    Config.SessionID        = Buddy.SessionID;
    Config.ConnectionType   = CONNECT_INDIRECT;

    const game_score& Score = GameMgr.GetScore();

    if( GameMgr.GameInProgress()==FALSE )
    {
        return FALSE;
    }
    s32 i;
    for( i=0; i<NET_MAX_PLAYERS; i++ )
    {
        if( Score.Player[i].IsConnected && (Score.Player[i].Identifier==Buddy.Identifier) )
        {
            return TRUE;
        }
    }

    return( FALSE );
}

//==============================================================================

void match_mgr::UpdateUserStatus( void )
{
    xbool DoUpdate       = FALSE;
    xbool IsVoiceAllowed = FALSE;

    // Check the signed-in user is not voice banned.
    // NOTE: we can only have one local user logged in at a time hence the index of 0.
    XONLINE_USER* pUsers = XOnlineGetLogonUsers();
    if( pUsers != NULL )
        IsVoiceAllowed = XOnlineIsUserVoiceAllowed( pUsers[0].xuid.dwUserFlags );

    // Now we know the current voice banning status we can update the voice manager
    g_VoiceMgr.SetVoiceBanned( !IsVoiceAllowed );

    // Voice chat is only available if the headset is plugged in,
    // enabled in the UI, and player has not been voice banned.
    xbool IsVoiceCapable = g_VoiceMgr.IsHeadsetPresent() &
                           g_VoiceMgr.IsVoiceEnabled()   &
                           IsVoiceAllowed;

    // Check if the voice capable status has changed
    if( m_IsVoiceCapable != IsVoiceCapable )
    {
        m_IsVoiceCapable = IsVoiceCapable;
        DoUpdate         = TRUE;

        // On the server we must update the voice peripheral status in the game manager directly.
        if( g_NetworkMgr.IsServer() == TRUE )
            GameMgr.SetVoiceCapable( 0, m_IsVoiceCapable );
    }

    // Check if the user status has changed
    buddy_status PendingStatus = m_PendingUserStatus;

    // This will stop clients connecting to us when in campaign or split-screen
    if( (PendingStatus == BUDDY_STATUS_INGAME) && (GameMgr.IsGameOnline() == FALSE) )
         PendingStatus  = BUDDY_STATUS_ONLINE;

    if( m_UserStatus != PendingStatus )
    {
        // We need to wait until the server is fully authenticated before sending the
        // new buddy status as we will not have the sessionid until then.
        if( m_LocalIsServer && (GetConnectStatus() != MATCH_CONN_CONNECTED) )
        {
            return;
        }
        m_UserStatus = PendingStatus;
        DoUpdate     = TRUE;
    }

    if( m_ShowOnlineStatus != m_LastShowOnlineStatus )
    {
        m_LastShowOnlineStatus = m_ShowOnlineStatus;
        DoUpdate = TRUE;
    }

    if( DoUpdate == TRUE )
    {
        s32                 StatusFlags;
        XNKID               SessionID;
        char                Location[64];

        if( m_UserStatus == BUDDY_STATUS_INGAME )
        {
            x_memcpy( &SessionID, &g_ActiveConfig.GetConfig().SessionID, sizeof( SessionID ) );
        }
        else
        {
            // Clearing the SessionID will expire any previously sent game invitations
            x_memset( &SessionID, 0, sizeof( SessionID ) );
        }

        StatusFlags = 0;

        switch( m_UserStatus )
        {
        case BUDDY_STATUS_OFFLINE       : StatusFlags = 0;                                  break;
        case BUDDY_STATUS_ONLINE        : StatusFlags = XONLINE_FRIENDSTATE_FLAG_ONLINE;    break;
        case BUDDY_STATUS_INGAME        : StatusFlags = XONLINE_FRIENDSTATE_FLAG_ONLINE  |
                                                        XONLINE_FRIENDSTATE_FLAG_PLAYING |
                                                        XONLINE_FRIENDSTATE_FLAG_JOINABLE;  break;
        case BUDDY_STATUS_ADD_PENDING   :
        case BUDDY_STATUS_REQUEST_ADD   :
        default                         : ASSERT( FALSE );
        }

        x_sprintf( Location, "---" );

        if( m_ShowOnlineStatus==FALSE )
        {
            StatusFlags &= ~XONLINE_FRIENDSTATE_FLAG_ONLINE;
        }
        else
        {

            // Ensure user is not banned and the voice hardware is connected
            if( m_IsVoiceCapable == TRUE )
            {
                StatusFlags |= XONLINE_FRIENDSTATE_FLAG_VOICE;
            }

            s32     Flags = 0;          // To be done: Add user flags from the user profile.
            const net_address& Remote = g_ActiveConfig.GetRemoteAddress();

            if( m_UserStatus == BUDDY_STATUS_INGAME )
            {
                x_sprintf( Location, "%s:%s", (const char*)xstring( g_ActiveConfig.GetShortGameType() ),
                                            (const char*)xstring( g_ActiveConfig.GetLevelName())    );
            }
        }

        if( g_EnableLANLookup == FALSE )
        {
            // Can only update the notifications when a user is logged on
            if( (m_IsLoggedOn == TRUE) && m_LogonTaskHandle )
            {
                // Notify the XBox Live service of the changes to the user's status
                HRESULT Result = XOnlineNotificationSetState( 0,
                                                              StatusFlags,
                                                              SessionID,
                                                              x_strlen( Location )+1,
                                                              (byte*)Location );

                // Log the status change
                {
                    const char* pStatus = "";

                    switch( m_UserStatus )
                    {
                    case BUDDY_STATUS_OFFLINE:         pStatus = "BUDDY_STATUS_OFFLINE";                    break;
                    case BUDDY_STATUS_ONLINE:          pStatus = "BUDDY_STATUS_ONLINE" ;                    break;
                    case BUDDY_STATUS_INGAME:          pStatus = "BUDDY_STATUS_INGAME" ;                    break;
                    default:                           pStatus = "<unknown>";                               break;
                    }

                    LOG_MESSAGE( "match_mgr::UpdateUserStatus",
                                 "Status:%s  Location:%s  Voice:%d",
                                 pStatus,
                                 Location,
                                 IsVoiceCapable );
                }
            }
        }
    }
}

//==============================================================================

void match_mgr::InitDownload( const char* pURL )
{
    ASSERT( FALSE );
    (void)pURL;
}

//==============================================================================

void match_mgr::KillDownload( void )
{
    ASSERT( FALSE );
}

//==============================================================================

download_status match_mgr::GetDownloadStatus( void )
{
    ASSERT( FALSE );
    return DL_STAT_ERROR;
}

//==============================================================================

f32 match_mgr::GetDownloadProgress( void )
{
    ASSERT( FALSE );
    return 0.0f;
}

//==============================================================================

void* match_mgr::GetDownloadData( s32& Length )
{
    ASSERT( FALSE );
    Length = 0;
    return NULL;
}

//==============================================================================

const extended_info* match_mgr::GetExtendedServerInfo( s32 Index )
{
    if( Index != m_ExtendedServerInfoOwner )
    {
        LOG_MESSAGE( "match_mgr::GetExtendedServerInfo", "Getting extended info for server %d", Index );

        m_ExtendedServerInfoOwner = Index;
        StartAcquisition( ACQUIRE_EXTENDED_SERVER_INFO );
    }
    else
    {
        if( (GetState() == MATCH_ACQUIRE_IDLE) || (GetState() == MATCH_IDLE) )
        {
            return &m_ExtendedServerInfo;
        }
        ASSERT( (GetState() == MATCH_ACQUIRE_EXTENDED_INFO) ||
            (m_PendingAcquisitionMode == ACQUIRE_EXTENDED_SERVER_INFO) );
    }
    return NULL;
}

//==============================================================================

void match_mgr::StartAcquisition( match_acquire AcquisitionMode )
{
    LockLists();

    switch( AcquisitionMode )
    {
    case ACQUIRE_SERVERS:
        break;
    case ACQUIRE_EXTENDED_SERVER_INFO:
        break;

    default:
        ASSERT( FALSE );
    }
    m_PendingAcquisitionMode = AcquisitionMode;
    m_bAcquisitionComplete   = FALSE;
    UnlockLists();
}

//==============================================================================

xbool match_mgr::IsAcquireComplete( void )
{   
    return( m_bAcquisitionComplete );
};

//==============================================================================

const char* match_mgr::GetConnectErrorMessage( void )
{
    switch( GetConnectStatus() )
    {
    default:                                //*** DELIBERATE FALLTHROUGH ***
    case MATCH_CONN_UNAVAILABLE:            return "IDS_ONLINE_CONNECT_MATCHMAKER_REFUSED";
    case MATCH_CONN_DISCONNECTED:           return "IDS_ONLINE_CHECK_CABLE_XBOX ";
    }

    return( m_ConnectErrorMessage );
}

//==============================================================================

void match_mgr::SetCrossTitleInvite( void )
{
    m_CrossTitleInvite = TRUE;
}

//==============================================================================
#if 0   // Cross Title invites are handled differently now so they can be TCR compliant (JP)
void match_mgr::FindCrossTitleGame( void )
{
    // Turn off the cross title invite flag
    m_CrossTitleInvite= FALSE;

    if( m_PendingInviteAccepted )
    {
        // Copy the info into the last invite
        m_LastInvite.InvitingFriend.xuid        = m_PendingInvite.xuidInvitingUser; //person that sent invite
        m_LastInvite.InvitingFriend.sessionID   = m_PendingInvite.SessionID;
        m_LastInvite.xuidAcceptedFriend         = m_PendingInvite.xuidAcceptedUser; //person that accepted invite
        m_LastInvite.InviteAcceptTime           = m_PendingInvite.InviteAcceptTime;

        strncpy( m_LastInvite.InvitingFriend.szGamertag,
            m_PendingInvite.szInvitingUserGamertag,
            XONLINE_GAMERTAG_SIZE );

        // Check all the users that were logged in when the invite was accepted
        for( s32 i=0; i < XONLINE_MAX_LOGON_USERS; i++ )
        {
            m_LastInvite.xuidLogonUsers[i] = m_PendingInvite.xuidLogonUsers[i];
        }

        m_PendingInviteAccepted = FALSE;
    }

    // check if friend still online
    s32 FriendIdx = GetFriendIdx( m_LastInvite.InvitingFriend.xuid );

    if( IsFriendOnline( FriendIdx ) )
    {
        if( IsFriendPlayingSameGame( FriendIdx ) )
        {
            // accepted invitation to play on same title id, same as regular join
            g_StateMgr.SetState( SM_START_GAME );
        }
        else
        {
            // accepted invitation to play on different title id
            WCHAR friendGameName[MAX_TITLENAME_LEN] = { 0, };
            if( m_LastInvite.InvitingFriend.dwTitleID )
            {
                XOnlineFriendsGetTitleName(m_LastInvite.InvitingFriend.dwTitleID, XGetLanguage(), MAX_TITLENAME_LEN, friendGameName);
            }
            //xwstring friendStatusString( g_StringTableMgr( "ui", "IDS_POPUP_INSERT_GAME_DISC_FOR" ) );
            //friendStatusString += friendGameName;

            //popup for "Insert game disc for <<title name>>, press A to continue

            XOnlineFriendsJoinGame( m_UserIndex,&m_LastInvite.InvitingFriend );

            // shutdown
            g_StateMgr.Reboot( REBOOT_QUIT );
        }
    }
    else
    {
        //popup for THIS_GAME_SESSION_NA, press A to continue
    }
}

//==============================================================================

xbool match_mgr::GetStoredGameInvites( void )
{
    // Get any stored game invitations from cross-title invites
    if( XOnlineGameInviteGetLatestAccepted( &m_PendingInvite ) == S_OK )
    {
        // We had a stored game invite
        SetCrossTitleInvite();
        //      XOnlineCleanup();
        return( TRUE );
    }

    // TEMP: shouldn't have to do this, but double check the other function
    // Check for accepted invitations from a friend
    XONLINE_ACCEPTED_GAMEINVITE acceptedInvite;
    if( XOnlineFriendsGetAcceptedGameInvite( &acceptedInvite ) == S_OK )
    {
        // Copy the info into the last invite
        m_LastInvite = acceptedInvite;

        // We had a stored game invite
        SetCrossTitleInvite();
        //      XOnlineCleanup();
        return( TRUE );
    }

    return( FALSE );
}
#endif
//==============================================================================

xbool match_mgr::GetPendingInviteAccepted( s32 PlayerIndex )
{
    if( GetPendingInviteAccepted() == TRUE )
    {
        // Check to see if there is a pending accepted invite for the signed in player
        if( GetPlayerIdentifier( PlayerIndex ) == m_PendingInvite.xuidAcceptedUser.qwUserID )
            return( TRUE );
    }

    return( FALSE );
}

//==============================================================================

xbool match_mgr::GetInviteAcceptedUsers( xwstring& InvitingUser, xwstring& AcceptedUser )
{
    ASSERT( GetPendingInviteAccepted() == TRUE );

    HRESULT Result = XOnlineGetUsers( m_UserList, &m_NumUsers );
    ASSERT( SUCCEEDED( Result ) == TRUE );

    DWORD i;
    for( i=0; i < m_NumUsers; i++ )
    {
        if( m_UserList[i].xuid.qwUserID == m_PendingInvite.xuidAcceptedUser.qwUserID )
            break;
    }

    if( i < m_NumUsers )
    {
        AcceptedUser = xwstring( m_UserList[i].szGamertag );
        InvitingUser = xwstring( m_PendingInvite.szInvitingUserGamertag );
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }
}

//==============================================================================

s32 match_mgr::GetFriendIdx( XUID FriendXUID )
{
    s32 FriendIndex = -1;   //invalid

    if( m_pFriendBuffer )
    {
        m_nFriendCount = XOnlineFriendsGetLatest( m_UserIndex, MAX_FRIENDS, m_pFriendBuffer );

        for( s32 i=0; i < (s32)m_nFriendCount; i++ )
        {
            if( XOnlineAreUsersIdentical( &FriendXUID, &m_pFriendBuffer[i].xuid ) )
            {
                FriendIndex = i;
            }
        }
    }

    return( FriendIndex );
}

//==============================================================================

xbool match_mgr::IsFriendOnline( s32 FriendIndex )
{
    xbool IsOnline = FALSE;

    if( m_pFriendBuffer )
    {
        m_nFriendCount = XOnlineFriendsGetLatest( m_UserIndex, MAX_FRIENDS, m_pFriendBuffer );

        if( (FriendIndex >= 0) && (FriendIndex < (s32)m_nFriendCount) )
        {
            IsOnline = (m_pFriendBuffer[ FriendIndex ].dwFriendState & XONLINE_FRIENDSTATE_FLAG_ONLINE);
        }
    }

    return( IsOnline );
}

//==============================================================================

xbool match_mgr::IsFriendPlayingSameGame( s32 friendIndex )
{
    xbool IsSameGame = FALSE;

    if( m_pFriendBuffer )
    {
        m_nFriendCount = XOnlineFriendsGetLatest( m_UserIndex, MAX_FRIENDS, m_pFriendBuffer );

        // Check to see if the friend is playing the same game
        // Used for Cross-title invitations
        IsSameGame = XOnlineTitleIdIsSameTitle( m_pFriendBuffer[friendIndex].dwTitleID );
    }

    return( IsSameGame );
};

//==============================================================================

xbool match_mgr::IsFriendPlayingSameTitleID( s32 TitleID )
{
    xbool IsSameGame ;

    IsSameGame = XOnlineTitleIdIsSameTitle( TitleID );

    return( IsSameGame );
};

//==============================================================================

void match_mgr::SetVoiceMessage( byte* pVoiceMessage,
                                s32   NumBytes,
                                f32   Duration )
{
    m_pVoiceMessageSend     = pVoiceMessage;
    m_VoiceMessageNumBytes  = NumBytes;
    m_VoiceMessageDuration  = Duration;
}

//==============================================================================

void match_mgr::ClearVoiceMessage( void )
{
    m_pVoiceMessageSend     = NULL;
    m_VoiceMessageNumBytes  = 0;
    m_VoiceMessageDuration  = 0.0f;
}

//==============================================================================

f32 match_mgr::GetMessageSendProgress( void )
{
    f32 Progress = 0.0f;

    if( m_InviteTaskHandle != NULL )
    {
        if( (m_MessageState == MATCH_MESSAGE_SEND)    ||
            (m_MessageState == MATCH_MESSAGE_SUCCESS) )
        {
            DWORD PercentDone;
            XOnlineMessageSendGetProgress( m_InviteTaskHandle, &PercentDone, NULL, NULL );

            Progress = PercentDone / 100.0f;
        }
    }

    return( Progress );
}

//==============================================================================

f32 match_mgr::GetMessageRecProgress( void )
{
    f32 Progress = 0.0f;

    if( m_DownloadTaskHandle != NULL )
    {
        if( (m_MessageState == MATCH_MESSAGE_RECEIVE) ||
            (m_MessageState == MATCH_MESSAGE_SUCCESS) )
        {
            DWORD PercentDone;
            XOnlineMessageDownloadAttachmentGetProgress( m_DownloadTaskHandle, &PercentDone, NULL, NULL );

            Progress = PercentDone / 100.0f;
        }
    }

    return( Progress );
}

//==============================================================================

xbool match_mgr::IsPlayerMuted( u64 Identifier )
{
    xbool IsMuted = FALSE;

    for( s32 i=0; i < (s32)m_Mutelist.NumUsers; i++ )
    {
        if( m_Mutelist.List[i].xuid.qwUserID == Identifier )
        {
            IsMuted = TRUE;
            break;
        }
    }

    return( IsMuted );
}

//==============================================================================

void match_mgr::SetIsPlayerMuted( u64 Identifier, xbool IsMuted )
{
    // Ensure we don't write too far ahead and overflow the circular buffer
    ASSERT( (m_Mutelist.WriteIndex - m_Mutelist.ReadIndex) < NET_MAX_PLAYERS );

    // Convert linear index to a circular buffer index
    s32 Index = m_Mutelist.WriteIndex % NET_MAX_PLAYERS;

    mutelist::pending& Pending = m_Mutelist.Pending[ Index ];
    Pending.Identifier         = Identifier;
    Pending.IsMuted            = IsMuted;

    m_Mutelist.WriteIndex++;

    // Signify to the mutelist that there is a pending change
    m_Mutelist.State = mutelist::MUTELIST_UPDATE;
}

//==============================================================================

void match_mgr::UpdateMutelist( void )
{
    XUID UserID;
    UserID.dwUserFlags = 0;

    switch( m_Mutelist.State )
    {
    case mutelist::MUTELIST_IDLE :
        break;

    case mutelist::MUTELIST_GET :

        // Check if there is a task already in progress
        if( m_MutelistEnumerateHandle == NULL )
        {
            // Start the asynchronous task to get the mutelist from the XBox Live service
            HRESULT Result = XOnlineMutelistGet( m_UserIndex,
                                                 MAX_MUTELISTUSERS,
                                                 NULL,
                                                 &m_MutelistEnumerateHandle,
                                                 m_Mutelist.List,
                                                 &m_Mutelist.NumUsers );
            if( FAILED(Result) )
            {
                ASSERT( m_MutelistEnumerateHandle );
                m_Mutelist.State = mutelist::MUTELIST_IDLE;
            }
        }
        else
        {
            // Wait until the mutelist is acquired
            if( m_MutelistEnumerateResult != XONLINETASK_S_RUNNING )
            {
                XOnlineTaskClose( m_MutelistEnumerateHandle );
                m_MutelistEnumerateHandle   = NULL;
                m_Mutelist.State            = mutelist::MUTELIST_IDLE;

                if( SUCCEEDED( m_MutelistEnumerateResult ) )
                {
                    if( m_Mutelist.NumUsers > 0 )
                    {
                        for( s32 i=0; i < (s32)m_Mutelist.NumUsers; i++ )
                        {
                            u32* pU32 = (u32*)&m_Mutelist.List[i].xuid.qwUserID;

                            LOG_MESSAGE( "match_mgr::UpdatePlayerIsMuted",
                                        "Mutelist entry %d : ID = %08X%08X",
                                        i,
                                        pU32[1],
                                        pU32[0] );
                        }
                    }
                    else
                    {
                        LOG_MESSAGE( "match_mgr::UpdatePlayerIsMuted", "Mutelist is empty!" );
                    }
                }
            }
        }
        break;

    case mutelist::MUTELIST_UPDATE :

        // Process all pending mute changes
        while( m_Mutelist.ReadIndex < m_Mutelist.WriteIndex )
        {
            // Convert linear index to a circular buffer index
            s32 Index = m_Mutelist.ReadIndex % NET_MAX_PLAYERS;

            // Lookup the current pending change
            mutelist::pending& Pending = m_Mutelist.Pending[ Index ];
            UserID.qwUserID = Pending.Identifier;

            // Add or remove player from the mutelist
            if( Pending.IsMuted == TRUE )
            {
                HRESULT Result = XOnlineMutelistAdd( m_UserIndex, UserID );
                ASSERT( Result == S_OK );
            }
            else
            {
                HRESULT Result = XOnlineMutelistRemove( m_UserIndex, UserID );
                ASSERT( Result == S_OK );
            }

            m_Mutelist.ReadIndex++;
        }

        // Re-acquire the mutelist
        m_Mutelist.State = mutelist::MUTELIST_GET;
        break;

    default :
        ASSERT( FALSE );
        break;
    };
}

//==============================================================================
//
//  JP TEST CODE
//
//==============================================================================

void XBoxLiveUpdate( f32 DeltaTime )
{
    g_MatchMgr.Update( DeltaTime );
    g_VoiceMgr.Update( DeltaTime );

    x_DelayThread( 33 );
}

//==============================================================================

static byte s_pBuffer[ 420 * 15 ];  // Encoded data takes 420 bytes per second

//==============================================================================

void XBoxLiveTest( void )
{
    g_MatchMgr.Init();
    g_VoiceMgr.Init( TRUE, TRUE );

    f32 DeltaTime = 1.0f / 30.0f;

    // Start up XBox Live
    x_DebugMsg( "Starting...\n" );
    g_MatchMgr.SetState( MATCH_CONNECT_MATCHMAKER );
    while( g_MatchMgr.IsBusy() == TRUE ) XBoxLiveUpdate( DeltaTime );

    // Log on to XBox Live
    x_DebugMsg( "Logging on...\n" );
    g_MatchMgr.SetUserAccount( 0 );
    while( g_MatchMgr.GetConnectStatus() != MATCH_CONN_CONNECTED ) XBoxLiveUpdate( DeltaTime );

    // Acquire friends
    x_DebugMsg( "Getting buddies...\n" );
    while( g_MatchMgr.GetBuddyCount() == 0 ) XBoxLiveUpdate( DeltaTime );
    x_DebugMsg( "Connected!\n" );

    // Headset should now be registered, so we can enable talking
    g_VoiceMgr.SetTalking( TRUE );

    PXONLINE_USER pUser = XOnlineGetLogonUsers();
    const char*   pName = pUser->szGamertag;
    xbool IsServer = (x_strcmp( pName, "Who is dat" ) == 0);
    //    xbool IsServer = (x_strcmp( pName, "JAPES" ) == 0);

    // Create a dummy match
    server_info& Info  = g_ActiveConfig.GetConfig();
    x_wstrcpy( Info.Name,           xwstring( pName ) );
    x_wstrcpy( Info.MissionName,    xwstring( "[TEST] TinyMP" ) );
    x_wstrcpy( Info.GameType,       xwstring( "DeathMatch" ) );
    x_wstrcpy( Info.ShortGameType,  xwstring( "DM" ) );
    Info.Flags          =  0;
    Info.GameTime       = -1;
    Info.FragLimit      =  0;
    Info.GameTypeID     =  0;

    if( IsServer == TRUE )
    {
        // Startup server
        x_DebugMsg( "Starting server \"%s\"\n", pName );
        g_MatchMgr.SetState( MATCH_BECOME_SERVER );
        while( g_MatchMgr.GetState() != MATCH_SERVER_ACTIVE ) XBoxLiveUpdate( DeltaTime );
        x_DebugMsg( "Server started ok!\n" );

        // Load sample voice message
        if( 1 )
        {
            X_FILE* pFile = x_fopen( "\\Audio.bin", "rb" );
            ASSERT( pFile != NULL );
            s32 NumBytes = x_fread( s_pBuffer, 1, sizeof( s_pBuffer ), pFile );
            x_fclose( pFile );

            g_MatchMgr.SetVoiceMessage( s_pBuffer, NumBytes, 15.0f );
        }
    }

    enum mode
    {
        INVITING,
        SENDING,
        DONE_SENDING,

        WAITING,
        DOWNLOADING,
        PLAYING,
        DONE_PLAYING,
    };

    mode SendMode = INVITING;
    mode RecvMode = WAITING;

    buddy_info& Buddy   = g_MatchMgr.GetBuddy( 0 );
    headset&    Headset = g_VoiceMgr.GetHeadset();

    while( 1 )
    {
        if( IsServer == TRUE )
        {
            switch( SendMode )
            {
            case INVITING :
                g_MatchMgr.InviteBuddy( Buddy );
                SendMode  = SENDING;
                break;

            case SENDING :
                if( g_MatchMgr.IsSendingMessage() == TRUE )
                {
                    // Display the message upload progress
                    f32 Progress = g_MatchMgr.GetMessageSendProgress();
                    x_DebugMsg( "Sending message.... [%f]\n", Progress );
                }
                else
                {
                    SendMode = DONE_SENDING;
                    x_DebugMsg( "Message sent!\n" );
                }
                break;

            case DONE_SENDING :
                // Task complete!
                break;
            };
        }
        else
        {
            switch( RecvMode )
            {
            case WAITING : 
                // Wait until a voice message is received
                if( Buddy.Flags & USER_VOICE_MESSAGE )
                {
                    x_DebugMsg( "Received voice message from [%s]\n", Buddy.Name );
                    x_DebugMsg( "Starting download...\n" );

                    g_MatchMgr.StartMessageDownload( Buddy );

                    RecvMode = DOWNLOADING;
                }
                break;

            case DOWNLOADING : 
                if( g_MatchMgr.IsDownloadingMessage() == TRUE )
                {
                    // Display the message download progress
                    f32 Progress = g_MatchMgr.GetMessageRecProgress();
                    x_DebugMsg( "Receiving message... [%f]\n", Progress );
                }
                else
                {
                    // Wait until message is downloaded
                    byte* pVoiceMessage;
                    s32   NumBytes;
                    f32   Duration;

                    g_MatchMgr.GetVoiceMessage( &pVoiceMessage, &NumBytes, &Duration );
                    ASSERT( pVoiceMessage != NULL );

                    x_DebugMsg( "Received voice message of %d bytes and %f seconds duration\n",
                        NumBytes, Duration );

                    x_DebugMsg( "Playing voice message\n" );
                    Headset.StartVoicePlaying( pVoiceMessage, (s32)(Duration * 1000.0f), NumBytes );

                    RecvMode = PLAYING;
                }
                break;

            case PLAYING : 
                // Wait until the voice message has finished playing
                if( Headset.GetVoiceIsPlaying() == FALSE )
                {
                    Headset.StopVoicePlaying();

                    // Free the memory used by the voice message
                    g_MatchMgr.FreeMessageDownload();
                    x_DebugMsg( "Voice message played!\n" );

                    RecvMode = DONE_PLAYING;
                }
                break;

            case DONE_PLAYING :
                // Task complete!
                break;
            }
        }

        XBoxLiveUpdate( DeltaTime );
    }
}

XONLINE_STAT_SPEC s_ReadSpec;
XONLINE_STAT      s_ReadUserStat[GAMESTAT_MAX];

//==============================================================================
void match_mgr::UpdateStatsRead( void )
{
    if (m_ReadStatsHandle)
    {
        m_ReadStatsResult = XOnlineTaskContinue(m_ReadStatsHandle); 
        if (m_ReadStatsResult == XONLINETASK_S_SUCCESS)
        {
            m_StatsStatus |= STATS_READ_READY;
            m_StatsStatus &= ~STATS_READ_IN_PROGRESS;

        }
        else if (FAILED(m_ReadStatsResult))
        {
            if( m_ReadStatsHandle )
            {
                // If it failed, how can you close it? This was crashing on load.
                XOnlineTaskClose( m_ReadStatsHandle );
                m_ReadStatsHandle = NULL;
            }
            m_StatsStatus &= ~STATS_READ_IN_PROGRESS;
        }
        if (IsUserStatsReady())
        {
            GetStatsRead();

            if( m_StatsStatus & STATS_READ_COMPLETE )
            {
                m_CareerStats.KillsAsHuman  = s_ReadUserStat[GAMESTAT_KILLS_AS_HUMAN].lValue;
                // rmb-stats m_CareerStats.KillsAsMutant = s_ReadUserStat[GAMESTAT_KILLS_AS_MUTANT].lValue;
                m_CareerStats.Deaths        = s_ReadUserStat[GAMESTAT_DEATHS].lValue;
                m_CareerStats.PlayTime      = s_ReadUserStat[GAMESTAT_PLAYTIME].lValue;
                m_CareerStats.Games         = s_ReadUserStat[GAMESTAT_GAMESPLAYED].lValue;
                m_CareerStats.Wins          = s_ReadUserStat[GAMESTAT_WINS].lValue;
                m_CareerStats.Gold          = s_ReadUserStat[GAMESTAT_FIRST].lValue;
                m_CareerStats.Silver        = s_ReadUserStat[GAMESTAT_SECOND].lValue;
                m_CareerStats.Bronze        = s_ReadUserStat[GAMESTAT_THIRD].lValue;
                // rmb-stats m_CareerStats.Kicks         = s_ReadUserStat[GAMESTAT_KICKS].lValue;
                // rmb-stats m_CareerStats.VotesStarted  = s_ReadUserStat[GAMESTAT_VOTES].lValue;
            }
        }
    }
    else
    {
        if( GetAuthStatus()==AUTH_STAT_CONNECTED )
        {
            if ((m_StatsStatus & STATS_READ_PENDING) && IsReadReady())
            {
                m_StatsStatus &= ~STATS_READ_PENDING;
                StartStatsRead();
            }
        }
    }
}
//==============================================================================
void match_mgr::UpdateStatsWrite( void )
{
    if (m_WriteStatsHandle)
    {
        m_WriteStatsResult = XOnlineTaskContinue(m_WriteStatsHandle); 
        if (m_WriteStatsResult == XONLINETASK_S_SUCCESS)
        {
           XOnlineTaskClose(m_WriteStatsHandle);
           m_WriteStatsHandle = NULL;
           m_StatsStatus &= ~STATS_WRITE_IN_PROGRESS;
            m_StatsStatus |= STATS_WRITE_COMPLETE;
           LOG_MESSAGE("match_mgr::UpdateStatsWrite","UpdateStatsWrite XONLINETASK_S_SUCCESS write completed");
        }
        else if (FAILED(m_WriteStatsResult))
        {
            XOnlineTaskClose(m_WriteStatsHandle);
            m_WriteStatsHandle = NULL;
            m_StatsStatus &= ~STATS_WRITE_IN_PROGRESS;
            LOG_MESSAGE("match_mgr::UpdateStatsWrite","UpdateStatsWrite FAILED");
        }
    }
}

//==============================================================================

void match_mgr::StatsCloseCurrentRead( void )
{
   if (m_ReadStatsHandle)
    {
        //need to finish or server will bark
        while (XOnlineTaskContinue(m_ReadStatsHandle) == XONLINETASK_S_RUNNING)
        {
            // check for cable pull
        }
        XOnlineTaskClose(m_ReadStatsHandle);
        m_ReadStatsHandle = NULL;
    }
}

//==============================================================================

void match_mgr::StatsCloseCurrentWrite( void )
{
    if (m_WriteStatsHandle)
    {
        //need to finish or server will bark
        while (XOnlineTaskContinue(m_WriteStatsHandle) == XONLINETASK_S_RUNNING)
        {
            // check for cable pull
        }
        XOnlineTaskClose(m_WriteStatsHandle);
        m_WriteStatsHandle = NULL;
    }

}

//==============================================================================
void match_mgr::StatsShutdown( void )
{
    StatsCloseCurrentRead();
    StatsCloseCurrentWrite();
}

//==============================================================================
void match_mgr::ClearStatsGotten( void )
{
     m_StatsStatus &= ~STATS_READ_COMPLETE;
}

//==============================================================================

void match_mgr::ClearStatsReady( void )
{
     m_StatsStatus &= ~STATS_READ_READY;
}

//==============================================================================
xbool match_mgr::IsUserStatsReady( void )
{
    xbool rdy ;

    rdy = (m_StatsStatus & STATS_READ_READY)?  true : false;
    
    return rdy;
}

//==============================================================================
xbool match_mgr::IsUserStatsGotten( void )
{
    xbool gotten ;

    gotten = (m_StatsStatus & (STATS_READ_IN_PROGRESS|STATS_READ_READY|STATS_READ_COMPLETE))?  true : false;
    
    return gotten;
}

//==============================================================================
xbool match_mgr::IsReadReady( void )
{
    if( m_StatsStatus & STATS_READ_IN_PROGRESS )
    {
        return FALSE;
    }
    if( m_StatsReadThrottleTime > 0.0f )
    {
        return FALSE;
    }
    return TRUE;
}
//==============================================================================

void match_mgr::GetUserStats( void )
{
    m_StatsStatus &= ~STATS_READ_READY;
}

//==============================================================================

void match_mgr::ResetStats( DWORD numStatSpecs, const XONLINE_STAT_SPEC * pStatSpecs )
{
    XONLINETASK_HANDLE resetHandle;

    PXONLINE_USER pUsers = XOnlineGetLogonUsers();

    ULONGLONG qwUserID = pUsers[0].xuid.qwUserID;
    pUsers[0].xuid.qwUserID = 0;

    HRESULT hr = XOnlineStatReset(pUsers[0].xuid,0,NULL, &resetHandle);

    while (SUCCEEDED(hr))
    {
        if (hr == XONLINETASK_S_SUCCESS)
        {
            XOnlineTaskClose(resetHandle);
            break;
        }
        hr = XOnlineTaskContinue(resetHandle);
    }
    pUsers[0].xuid.qwUserID = qwUserID;

    StartStatsRead(numStatSpecs,pStatSpecs);

}


//==============================================================================

void match_mgr::StartStatsRead( DWORD numStatSpecs, const XONLINE_STAT_SPEC * pStatSpecs )
{
    if (!IsUserStatsGotten())
    {
        // start stats read if haven't done so already
        StatsCloseCurrentRead();

        m_ReadStatsResult = XOnlineStatRead(numStatSpecs,pStatSpecs,NULL,&m_ReadStatsHandle);

        if (FAILED(m_ReadStatsResult))
        {
            XOnlineTaskClose(m_ReadStatsHandle);
            m_ReadStatsHandle = NULL;
        }
        else
        {
            m_StatsStatus &= ~STATS_READ_COMPLETE;
            m_StatsStatus |= STATS_READ_IN_PROGRESS;
        }
    }
}

void InitStatSpec(XONLINE_STAT * pUserStat,XONLINE_STAT_SPEC * pSpec)
{
    XONLINE_USER* pUsers = XOnlineGetLogonUsers();
    ASSERT( pUsers );

    for (s32 i = GAMESTAT_KILLS_AS_HUMAN;i< GAMESTAT_MAX;i++)
    {
        switch (i)
        {
            case GAMESTAT_RATING:
            {
                pUserStat[i].wID    = XONLINE_STAT_RATING;
                pUserStat[i].type   = XONLINE_STAT_LONGLONG;
            }
            break;
            case GAMESTAT_RANK:
            {
                pUserStat[i].wID    = XONLINE_STAT_RANK;
                pUserStat[i].type   = XONLINE_STAT_LONG;
            }
            break;
            default:
            {
                pUserStat[i].wID    = i+1;                     // value 0 reserved for reset flag
                pUserStat[i].type   = XONLINE_STAT_LONG;
            }
            break;
        }
        //pUserStat[i].type   = XONLINE_STAT_NONE;
        //pUserStat[i].llValue = 0;
    }


    pSpec->xuidUser           = pUsers[ 0 ].xuid;
    pSpec->dwNumStats         = GAMESTAT_MAX-2;
    pSpec->dwLeaderBoardID    = 1;
    pSpec->pStats             = pUserStat;


}
//==============================================================================

void match_mgr::StartStatsRead(void)
{
    if (IsAuthenticated())
    {
        if (!IsReadReady())
        {
            m_StatsStatus |= STATS_READ_PENDING;
            LOG_MESSAGE("match_mgr::StartStatsRead","ENABLE STATS_READ_PENDING");
        }
        else
        {
            memset(s_ReadUserStat,0,sizeof(s_ReadUserStat));
            InitStatSpec(s_ReadUserStat,&s_ReadSpec);

            StartStatsRead(1,&s_ReadSpec);

            LOG_MESSAGE("match_mgr::StartStatsRead","StartStatsRead");
        }
    }
}

//==============================================================================

void match_mgr::StartDelayedStatsRead(void)
{
    // need to wait for some time to elapse after login before getting user stats
    // otherwise server error will occur

    m_StatsStatus |= STATS_READ_PENDING;
    m_StatsReadThrottleTime = INITIAL_HOLDOFF_TIME;
    LOG_MESSAGE( "match_mgr::StartDelayedStatsRead", "m_StatsReadThrottleTime : %8.4f", m_StatsReadThrottleTime);

#if 0
    // test hook to verify write
    PostGameStatsToLive();
#endif
}

//==============================================================================

void match_mgr::GetStatsRead(DWORD numStatSpecs, PXONLINE_STAT_SPEC pStatSpecs, DWORD extraBufferSize, BYTE * pExtraBuffer)
{
    m_ReadStatsResult = XOnlineStatReadGetResult(m_ReadStatsHandle,numStatSpecs, pStatSpecs,extraBufferSize,pExtraBuffer);

    if (SUCCEEDED(m_ReadStatsResult))
    {
        XOnlineTaskClose(m_ReadStatsHandle);
        m_ReadStatsHandle = NULL;
        m_StatsStatus &= ~STATS_READ_READY;
        m_StatsStatus |= STATS_READ_COMPLETE;
        m_StatsReadThrottleTime = USER_UPDATE_TIME;
        LOG_MESSAGE("match_mgr::GetStatsRead","GetStatsRead SUCCEEDED");
    }
    else
    {
        XOnlineTaskClose(m_ReadStatsHandle);
        m_ReadStatsHandle = NULL;
        m_StatsStatus &= ~STATS_READ_READY;
        m_StatsStatus &= ~STATS_READ_COMPLETE;
        m_StatsReadThrottleTime = USER_UPDATE_TIME;
        LOG_MESSAGE("match_mgr::GetStatsRead","GetStatsRead FAILED 0x%x",m_ReadStatsResult);
    }
}

//==============================================================================

void match_mgr::GetStatsRead(void)
{
    memset(s_ReadUserStat,0,sizeof(s_ReadUserStat));
    InitStatSpec(s_ReadUserStat,&s_ReadSpec);

    GetStatsRead(1,&s_ReadSpec,0,NULL);
/*
    if (m_StatsStatus&STATS_READ_COMPLETE)
    {
        for (i = GAMESTAT_KILLS_AS_HUMAN;i< GAMESTAT_MAX;i++);
        {
            m_Stats[i] = s_ReadSpec.pStats[i];
            SetCareerStats((gamestats)i,m_Stats[i].lValue);
        }
    }
*/
 }
//==============================================================================

void match_mgr::StartStatsWrite(DWORD numStatSpecs, const XONLINE_STAT_PROC * pStatProcs)
{
    m_WriteStatsResult = XOnlineStatWriteEx(numStatSpecs, pStatProcs,NULL, &m_WriteStatsHandle);

    if (FAILED(m_WriteStatsResult))
    {
        XOnlineTaskClose(m_WriteStatsHandle);
        m_WriteStatsHandle = NULL;
        m_StatsStatus &= ~STATS_WRITE_IN_PROGRESS;
        LOG_MESSAGE("match_mgr::StartStatsWrite","StartStatsWrite FAILED");
   }
    else
    {
        m_StatsStatus |= STATS_WRITE_IN_PROGRESS;
        LOG_MESSAGE("match_mgr::StartStatsWrite","StartStatsWrite STATS_WRITE_IN_PROGRESS");
}
}

//==============================================================================

XONLINE_STAT_PROC   s_WriteStatProc;
XONLINE_STAT_UPDATE s_WriteStatUpdates;

void match_mgr::StartStatsWrite(void)
{

    if( g_EnableLANLookup )
        return;
    memset(&s_WriteStatProc,0,sizeof(s_WriteStatProc));
    memset(&s_WriteStatUpdates,0,sizeof(s_WriteStatUpdates));

    XONLINE_USER* pUsers = XOnlineGetLogonUsers();
    ASSERT( pUsers );

    s_WriteStatUpdates.dwConditionalIndex = 0;
    s_WriteStatUpdates.dwLeaderBoardID    = 1;
    s_WriteStatUpdates.xuid               = pUsers[ 0 ].xuid;
    s_WriteStatUpdates.dwNumStats         = GAMESTAT_MAX-1;
    s_WriteStatUpdates.pStats             = m_Stats;

    s_WriteStatProc.wProcedureID = XONLINE_STAT_PROCID_UPDATE_INCREMENT;
    s_WriteStatProc.Update       = s_WriteStatUpdates;

    StartStatsWrite(1,&s_WriteStatProc);
}
//==============================================================================

void match_mgr::VerifyServices(void)
{
    s32 i;
    HRESULT hr;

    XONLINE_SERVICE_INFO ServiceInfo;

    for (i=0;i<numServices;i++)
    {
        hr = XOnlineGetServiceInfo(s_LogonServiceIds[i],&ServiceInfo);
        switch (hr)
        {
            case S_OK:
                LOG_MESSAGE( "match_mgr::VerifyServices", "%d: S_OK 0x%x",i, s_LogonServiceIds[i]);
            break;
            case XONLINE_E_INTERNAL_ERROR:
                LOG_MESSAGE( "match_mgr::VerifyServices", "%d: XONLINE_E_INTERNAL_ERROR 0x%x",i, s_LogonServiceIds[i]);
            break;
            case XONLINE_E_LOGON_SERVICE_NOT_REQUESTED:
                LOG_MESSAGE( "match_mgr::VerifyServices", "%d: XONLINE_E_LOGON_SERVICE_NOT_REQUESTED 0x%x",i, s_LogonServiceIds[i]);
            break;
            case XONLINE_E_LOGON_SERVICE_NOT_AUTHORIZED:
                LOG_MESSAGE( "match_mgr::VerifyServices", "%d: XONLINE_E_LOGON_SERVICE_NOT_AUTHORIZED 0x%x",i, s_LogonServiceIds[i]);
            break;
            case XONLINE_E_LOGON_SERVICE_TEMPORARILY_UNAVAILABLE:
                LOG_MESSAGE( "match_mgr::VerifyServices", "%d: XONLINE_E_LOGON_SERVICE_TEMPORARILY_UNAVAILABLE 0x%x",i, s_LogonServiceIds[i]);
            break;
      }
    }
}

//==============================================================================

void match_mgr::SetCareerStats(gamestats mode, u32 value)
{
    switch (mode)
    {
        case GAMESTAT_KILLS_AS_HUMAN:
        {
            m_CareerStats.KillsAsHuman              += value;
            m_Stats[GAMESTAT_KILLS_AS_HUMAN].wID     = GAMESTAT_KILLS_AS_HUMAN+1;
            m_Stats[GAMESTAT_KILLS_AS_HUMAN].type    = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_KILLS_AS_HUMAN].lValue  = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.KillsAsHuman %d",m_CareerStats.KillsAsHuman);
        }
        break;
/* // rmb-stats 
        case GAMESTAT_KILLS_AS_MUTANT:
        {
            m_CareerStats.KillsAsMutant              += value;
            m_Stats[GAMESTAT_KILLS_AS_MUTANT].wID     = GAMESTAT_KILLS_AS_MUTANT+1;
            m_Stats[GAMESTAT_KILLS_AS_MUTANT].type    = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_KILLS_AS_MUTANT].lValue  = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.KillsAsMutant %d",m_CareerStats.KillsAsMutant);
        }
        break;
*/
        case GAMESTAT_DEATHS:
        {
            m_CareerStats.Deaths            += value;
            m_Stats[GAMESTAT_DEATHS].wID     = GAMESTAT_DEATHS+1;
            m_Stats[GAMESTAT_DEATHS].type    = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_DEATHS].lValue  = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.Deaths %d",m_CareerStats.Deaths);
       }
        break;
        case GAMESTAT_PLAYTIME:
        {
            m_CareerStats.PlayTime            += value;
            m_Stats[GAMESTAT_PLAYTIME].wID     = GAMESTAT_PLAYTIME+1;
            m_Stats[GAMESTAT_PLAYTIME].type    = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_PLAYTIME].lValue  = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.PlayTime %d",m_CareerStats.PlayTime);
       }
        break;
        case GAMESTAT_GAMESPLAYED:
        {
            m_CareerStats.Games                  += value;
            m_Stats[GAMESTAT_GAMESPLAYED].wID     = GAMESTAT_GAMESPLAYED+1;
            m_Stats[GAMESTAT_GAMESPLAYED].type    = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_GAMESPLAYED].lValue  = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.Games %d",m_CareerStats.Games);
        }
        break;
        case GAMESTAT_WINS:
        {
            m_CareerStats.Wins             += value;
            m_Stats[GAMESTAT_WINS].wID      = GAMESTAT_WINS+1;
            m_Stats[GAMESTAT_WINS].type     = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_WINS].lValue   = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.Wins %d",m_CareerStats.Wins);
       }
        break;
        case GAMESTAT_FIRST:
        {
            m_CareerStats.Gold              += value;
            m_Stats[GAMESTAT_FIRST].wID      = GAMESTAT_FIRST+1;
            m_Stats[GAMESTAT_FIRST].type     = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_FIRST].lValue   = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.Gold %d",m_CareerStats.Gold);
       }
        break;
        case GAMESTAT_SECOND:
        {
            m_CareerStats.Silver             += value;
            m_Stats[GAMESTAT_SECOND].wID      = GAMESTAT_SECOND+1;
            m_Stats[GAMESTAT_SECOND].type     = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_SECOND].lValue   = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.Silver %d",m_CareerStats.Silver);
        }
        break;
        case GAMESTAT_THIRD:
        {
            m_CareerStats.Bronze            += value;
            m_Stats[GAMESTAT_THIRD].wID      = GAMESTAT_THIRD+1;
            m_Stats[GAMESTAT_THIRD].type     = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_THIRD].lValue   = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.Bronze %d",m_CareerStats.Bronze);
        }
        break;
/* // rmb-stats
        case GAMESTAT_KICKS:
        {
            m_CareerStats.Kicks             += value;
            m_Stats[GAMESTAT_KICKS].wID      = GAMESTAT_KICKS+1;
            m_Stats[GAMESTAT_KICKS].type     = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_KICKS].lValue   = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.Kicks %d",m_CareerStats.Kicks);
        }
        break;
        case GAMESTAT_VOTES:
        {
            m_CareerStats.VotesStarted      += value;
            m_Stats[GAMESTAT_VOTES].wID      = GAMESTAT_VOTES+1;
            m_Stats[GAMESTAT_VOTES].type     = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_VOTES].lValue   = value;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats.VotesStarted %d",m_CareerStats.VotesStarted);
        }
        break;
*/
        case GAMESTAT_RATING:
        {
            m_Stats[GAMESTAT_RATING].wID      = XONLINE_STAT_RATING;
            m_Stats[GAMESTAT_RATING].type     = XONLINE_STAT_LONGLONG;
            m_Stats[GAMESTAT_RATING].lValue   = 0;
            m_Stats[GAMESTAT_RATING].llValue  = 0;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats GAMESTAT_RATING INIT");
        }
        break; // RMB - this break was not here before - was fall thru intentional?
        case GAMESTAT_RANK:
        {
            m_Stats[GAMESTAT_RANK].wID      = XONLINE_STAT_RANK;
            m_Stats[GAMESTAT_RANK].type     = XONLINE_STAT_LONG;
            m_Stats[GAMESTAT_RANK].lValue   = 0;
            LOG_MESSAGE("match_mgr::SetCareerStats","m_CareerStats GAMESTAT_RANK INIT");
        }
        default:
        break;
    }
}


//==============================================================================

u32 match_mgr::GetCareerStats(gamestats mode)
{
    u32 value = 0;
    switch (mode)
    {
        case GAMESTAT_KILLS_AS_HUMAN:  value = m_CareerStats.KillsAsHuman;  break;
        // rmb-stats case GAMESTAT_KILLS_AS_MUTANT: value = m_CareerStats.KillsAsMutant; break;
        case GAMESTAT_DEATHS:          value = m_CareerStats.Deaths;        break;
        case GAMESTAT_PLAYTIME:        value = m_CareerStats.PlayTime;      break;
        case GAMESTAT_GAMESPLAYED:     value = m_CareerStats.Games;         break;
        case GAMESTAT_WINS:            value = m_CareerStats.Wins;          break;
        case GAMESTAT_FIRST:           value = m_CareerStats.Gold;          break;
        case GAMESTAT_SECOND:          value = m_CareerStats.Silver;        break;
        case GAMESTAT_THIRD:           value = m_CareerStats.Bronze;        break;
        // rmb-stats case GAMESTAT_KICKS:           value = m_CareerStats.Kicks;         break;
        // rmb-stats case GAMESTAT_VOTES:           value = m_CareerStats.VotesStarted;  break;
        default:
        break;
    }

#ifdef X_LOGGING
    LogWhichStat( "match_mgr::GetCareerStats", mode, value );
#endif // X_LOGGING

    return value;
}

//==============================================================================

void match_mgr::PostGameStatsToLive(void)
{
    if( GetAuthStatus()==AUTH_STAT_CONNECTED )
    {
        s32 i;
        for (i=GAMESTAT_KILLS_AS_HUMAN;i<GAMESTAT_MAX;i++)
        {
            SetCareerStats((gamestats)i,GetGameStats((gamestats)i));
        }
        LOG_MESSAGE("match_mgr::PostGameStatsToLive","PostGameStatsToLive");
        StartStatsWrite();
    }
    else
    {
        LOG_ERROR( "match_mgr::PostGameStatsToLive", "Unable to post stats due to network disconnect." );
    }
}


//==============================================================================

u32 match_mgr::GetGameStats(gamestats mode)
{
    u32 value = 0;

    switch (mode)
    {
        case GAMESTAT_KILLS_AS_HUMAN:  value = m_GameStats.KillsAsHuman;  break;
        // rmb-stats case GAMESTAT_KILLS_AS_MUTANT: value = m_GameStats.KillsAsMutant; break;
        case GAMESTAT_DEATHS:          value = m_GameStats.Deaths;        break;
        case GAMESTAT_PLAYTIME:        value = m_GameStats.PlayTime;      break;
        case GAMESTAT_GAMESPLAYED:     value = m_GameStats.Games;         break;
        case GAMESTAT_WINS:            value = m_GameStats.Wins;          break;
        case GAMESTAT_FIRST:           value = m_GameStats.Gold;          break;
        case GAMESTAT_SECOND:          value = m_GameStats.Silver;        break;
        case GAMESTAT_THIRD:           value = m_GameStats.Bronze;        break;
        // rmb-stats case GAMESTAT_KICKS:           value = m_GameStats.Kicks;         break;
        // rmb-stats case GAMESTAT_VOTES:           value = m_GameStats.VotesStarted;  break;
        default:
        break;
    }

#ifdef X_LOGGING
    LogWhichStat( "match_mgr::GetGameStats", mode, value );
#endif // X_LOGGING
    
    return value;
}

//==============================================================================

void match_mgr::SetAllGameStats( const player_stats& Stats )
{
    m_GameStats = Stats;
}

//==============================================================================

void match_mgr::SetAllCareerStats( const player_stats& Stats )
{
    m_CareerStats = Stats;
}

//==============================================================================

void match_mgr::SetGameStats(gamestats mode, u32 value)
{
    switch (mode)
    {
        case GAMESTAT_KILLS_AS_HUMAN:  m_GameStats.KillsAsHuman  = value; break;
        // rmb-stats case GAMESTAT_KILLS_AS_MUTANT: m_GameStats.KillsAsMutant = value; break;
        case GAMESTAT_DEATHS:          m_GameStats.Deaths        = value; break;
        case GAMESTAT_PLAYTIME:        m_GameStats.PlayTime      = value; break;
        case GAMESTAT_GAMESPLAYED:     m_GameStats.Games         = value; break;
        case GAMESTAT_WINS:            m_GameStats.Wins          = value; break;
        case GAMESTAT_FIRST:           m_GameStats.Gold          = value; break;
        case GAMESTAT_SECOND:          m_GameStats.Silver        = value; break;
        case GAMESTAT_THIRD:           m_GameStats.Bronze        = value; break;
        // rmb-stats case GAMESTAT_KICKS:           m_GameStats.Kicks         = value; break;
        // rmb-stats case GAMESTAT_VOTES:           m_GameStats.VotesStarted  = value; break;
        default:
            break;
    }

#ifdef X_LOGGING
    LogWhichStat( "match_mgr::SetGameStats", mode, value );
#endif // X_LOGGING

}


//==============================================================================
void match_mgr::UpdateAcquireExtendedInfo( f32 DeltaTime )
{
    (void)DeltaTime;

    if( m_ExtendedServerInfoOwner==-1 )
    {
        SetState( MATCH_ACQUIRE_IDLE  );
        SetConnectStatus( MATCH_CONN_CONNECTED );
        return;
    }
    if( HasTimedOut() || (GetConnectStatus() != MATCH_CONN_ACQUIRING_SERVERS) )
    {
        // We need to try and send this request again. Faked for first time.
        m_StateRetries--;
        if( m_StateRetries<0 )
        {
            server_info* pServer = GetServerInfo(m_ExtendedServerInfoOwner);
            // The server key should have been registeed when the extended request was sent
            ASSERT( pServer->KeyIsRegistered );

            if( pServer->KeyIsRegistered )
            {
                VERIFY( XNetUnregisterKey( &pServer->SessionID )==S_OK );

                RegisteredKeyCount--;
                LOG_MESSAGE( "match_mgr::UpdateAcquireExtendedInfo", "Unregistered key %s", SessionIDString( pServer->SessionID ) );
            }

            SetState( MATCH_ACQUIRE_IDLE );
            SetConnectStatus( MATCH_CONN_CONNECTED );
            return;
        }
        m_StateTimeout = 2.0f;
        // Issue new lookup request
        net_extended_lookup_request Request;
        IN_ADDR                     Addr;
        if( m_NeedNewPing )
        {
            m_PingIndex++;
            if( m_PingIndex >= MAX_PING_OUTSTANDING )
            {
                m_PingIndex = 0;
            }
            m_PingSendTimes[m_PingIndex]=x_GetTime();
            m_NeedNewPing = FALSE;
        }
        Request.PingIndex   = m_PingIndex;
        Request.Owner       = m_ExtendedServerInfoOwner;
        Request.Type        = NET_PKT_EXTENDED_LOOKUP_REQUEST;
        if( (m_ExtendedServerInfoOwner==-1) || (m_ExtendedServerInfoOwner >= GetServerCount()) )
        {
            SetState( MATCH_ACQUIRE_IDLE  );
            SetConnectStatus( MATCH_CONN_CONNECTED );
            return;
        }
        server_info* pServer = GetServerInfo( m_ExtendedServerInfoOwner );

        if( pServer->KeyIsRegistered==FALSE )
        {
            HRESULT Result;
            VERIFY( XNetRegisterKey( &pServer->SessionID, &pServer->ExchangeKey )==S_OK );
            RegisteredKeyCount++;
            pServer->KeyIsRegistered = TRUE;
#ifndef X_RETAIL
            LOG_MESSAGE( "match_mgr::UpdateAcquireExtendedInfo", "Key registered: 0x%08x:%08x:%08x:%08x, SessionID: %s", 
                *(s32*)&pServer->ExchangeKey.ab[0],
                *(s32*)&pServer->ExchangeKey.ab[4],
                *(s32*)&pServer->ExchangeKey.ab[8],
                *(s32*)&pServer->ExchangeKey.ab[12],
                SessionIDString( pServer->SessionID ) );
#endif  
            Result = XNetXnAddrToInAddr( &pServer->Address, &pServer->SessionID, &Addr );
            ASSERT( Result == S_OK );
            pServer->Remote.SetIP( ntohl(Addr.s_addr) );
        }
        SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
        m_pSocket->Send( pServer->Remote, &Request, sizeof(Request) );
        LOG_MESSAGE( "match_mgr::UpdateAcquireExtendedInfo", "Extended lookup request sent to %s via %s, PingIndex:%d", pServer->Remote.GetStrAddress(), m_pSocket->GetStrAddress(), m_PingIndex );

   }
}

//==============================================================================
void match_mgr::SendFeedback( u64 Identifier, const char* pName, player_feedback Type )
{
    HRESULT                 Result;
    XUID                    TargetUser;
    XONLINE_FEEDBACK_TYPE   FeedbackType;
    XONLINE_FEEDBACK_PARAMS FeedbackParams;

    if( m_FeedbackHandle )
    {
        XOnlineTaskClose( m_FeedbackHandle );
    }

    x_memset( &TargetUser, 0, sizeof(TargetUser) );

    TargetUser.qwUserID     = Identifier;

    FeedbackType = XONLINE_FEEDBACK_NEG_NICKNAME;
    switch( Type )
    {
    case FB_GREAT_SESSION:      FeedbackType = XONLINE_FEEDBACK_POS_SESSION;                break;
    case FB_GOOD_ATTITUDE:      FeedbackType = XONLINE_FEEDBACK_POS_ATTITUDE;               break;
    case FB_BAD_NAME:           FeedbackType = XONLINE_FEEDBACK_NEG_NICKNAME;               break;
    case FB_CURSING:            FeedbackType = XONLINE_FEEDBACK_NEG_LEWDNESS;               break;
    case FB_SCREAMING:          FeedbackType = XONLINE_FEEDBACK_NEG_SCREAMING;              break;
    case FB_CHEATING:           FeedbackType = XONLINE_FEEDBACK_NEG_GAMEPLAY;               break;
    case FB_THREATS:            FeedbackType = XONLINE_FEEDBACK_NEG_HARASSMENT;             break;
    case FB_OFFENSIVE_MESSAGE:  FeedbackType = XONLINE_FEEDBACK_NEG_HARASSMENT;                break;
    default:
        ASSERT( FALSE );
        return;
    }

    xwstring Name( pName );
    FeedbackParams.lpStringParam = (const xwchar*)Name;

    Result = XOnlineFeedbackSend( 0, TargetUser, FeedbackType, &FeedbackParams, NULL, &m_FeedbackHandle );
    ASSERT( Result == S_OK );
    LOG_MESSAGE( "match_mgr::SendFeedback", "Feedback sent. Result=%d", Result );
    (void)Result;
}

//==============================================================================

xbool match_mgr::IsDuplicateLogin( void )
{
    return( m_IsDuplicateLogin );
}

//==============================================================================

void match_mgr::ClearDuplicateLogin( void )
{
    m_IsDuplicateLogin = FALSE;
}

//==============================================================================

void match_mgr::SetOptionalMessage( xbool OptionalMessage )
{
    m_OptionalMessage = OptionalMessage;
}

//==============================================================================

xbool match_mgr::GetOptionalMessage( void )
{
    return( m_OptionalMessage );
}

//==============================================================================

void match_mgr::ClearAllKeys( xbool IncludeConfigs )
{
    s32 i;
    for( i=0; i < m_PendingResponseList.GetCount(); i++ )
    {
        if( m_PendingResponseList[i].KeyIsRegistered == TRUE )
        {
#ifndef X_RETAIL
            LOG_MESSAGE( "match_mgr::Reset", "Unregistered key %s", SessionIDString( m_PendingResponseList[i].SessionID ) );
#endif
            VERIFY( XNetUnregisterKey( &m_PendingResponseList[i].SessionID )==S_OK );
            RegisteredKeyCount--;
            m_PendingResponseList[i].KeyIsRegistered = FALSE;
        }
    }

    m_PendingResponseList.Clear();

    llnode* pCurr = m_pListHead;
    while( pCurr )
    {
        if( pCurr->pServerInfo->KeyIsRegistered )
        {
            pCurr->pServerInfo->KeyIsRegistered=FALSE;
            VERIFY( XNetUnregisterKey( &pCurr->pServerInfo->SessionID )==S_OK );
            RegisteredKeyCount--;
        }
        delete pCurr->pServerInfo;
        llnode* pOld = pCurr;
        pCurr = pCurr->pNext;
        delete pOld;
    }
    m_ServerCount = 0;
    m_pListHead   = NULL;
    if( IncludeConfigs && g_PendingConfig.GetConfig().KeyIsRegistered )
    {
        VERIFY( XNetUnregisterKey( &g_PendingConfig.GetConfig().SessionID )==S_OK );
        RegisteredKeyCount--;
        g_PendingConfig.GetConfig().KeyIsRegistered=FALSE;
    }
}
