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

#if !defined(TARGET_PC)
#error This should only be included for PC gamespy support.
#endif
#include "x_files.hpp"
#include "x_threads.hpp"
#include "e_Network.hpp"
#include "NetworkMgr/MatchMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/GameSpy/callbacks.hpp"
#include "StateMgr/StateMgr.hpp"
#include "ServerVersion.hpp"
#include "Configuration/GameConfig.hpp"
#include "x_log.hpp"
#include "../Apps/GameApp/Config.hpp"

const s32 A51_MIDWAY_FPS_CATEGORY = 1010000032;

#include "NetworkMgr/GameSpy/serverbrowsing/sb_serverbrowsing.h"
#include "NetworkMgr/GameSpy/available.h"
#include "NetworkMgr/GameSpy/qr2/qr2regkeys.h"
#include "NetworkMgr/GameSpy/qr2/qr2.h"
#include "NetworkMgr/GameSpy/GStats/gstats.h"
#include "NetworkMgr/GameSpy/GStats/gpersist.h"
#include "ResourceMgr/ResourceMgr.hpp"


//=========================================================================
//  Defines
//=========================================================================
//#define ENABLE_LAN_LOOKUP

#define LABEL_STRING(x) case x: return ( #x );
// Authentication data filename. This is short for obfuscation reason.

const s32   GAMESPY_PRODUCT_ID  = 10384;
const char* GAMESPY_GAMENAME    = "area51ps2";
const char* GAMESPY_SECRETKEY   = "eR48fP";
const char* EMAIL_POSTFIX       = "a51";

extern s32 g_Changelist;

#ifdef GSI_UNICODE
#define _T(a) L##a
#define _tprintf wprintf
#else
#define _T(a) a
#define _tprintf x_DebugMsg
#endif

//=========================================================================
//  External function and data prototypes
//=========================================================================

extern const char* MANIFEST_LOCATION;
extern const char* HDD_MANIFEST_FILENAME;

const char* TIMESTAMP_FILENAME = "HDD:lasttime.txt";
#if !defined(ENABLE_LAN_LOOKUP)
/* todo BISCUIT - this needs data for the other product codes. */
static unsigned char SLUS_20595_pass_phrase[] = { 0xb9, 0xfd, 0xb2, 0xce, 0x5c, 0xd9, 0x0e, 0x0c };
static unsigned char SLES_52570_pass_phrase[] = { 0xb9, 0xfd, 0xb2, 0xce, 0x5c, 0xd9, 0x0e, 0x0c };
static unsigned char SLES_53075_pass_phrase[] = { 0xb9, 0xfd, 0xb2, 0xce, 0x5c, 0xd9, 0x0e, 0x0c };
#endif

static byte* s_pAuthData;
#if defined(X_DEBUG) && defined(bwatson)
// This will create a temporary patch
static void MakePatch( void );
#endif


//=========================================================================
//  Data
//=========================================================================
#if defined(X_LOGGING)
const char* ServerBrowserError( SBError Error )
{
    switch( Error )
    {
    case sbe_noerror:       return "sbe_noerror";
    case sbe_socketerror:   return "sbe_socketerror";
    case sbe_dnserror:      return "sbe_dnserror";
    case sbe_connecterror:  return "sbe_connecterror";
    case sbe_dataerror:     return "sbe_dataerror";
    case sbe_allocerror:    return "sbe_allocerror";
    case sbe_paramerror:    return "sbe_paramerror";
    default:                ASSERT(FALSE);

    }
    return("Unknown");
}
#endif

//------------------------------------------------------------------------------
void s_MatchPeriodicUpdater( s32, char** argv )
{
    match_mgr& MatchMgr = (match_mgr&)*argv;
    xtimer deltatime;

    deltatime.Start();

    while( TRUE )
    {
        MatchMgr.Update( deltatime.TripSec() );
        ASSERT( (MatchMgr.m_UpdateInterval>0) && (MatchMgr.m_UpdateInterval<100) );
        x_DelayThread( MatchMgr.m_UpdateInterval );
    }
}

//------------------------------------------------------------------------------
xbool match_mgr::Init( net_socket& Local, const net_address Broadcast )
{
    f32 Timeout;

    ASSERT( !m_Initialized );
    ASSERT( AreListsLocked() == FALSE );
    ASSERT( IsBrowserLocked() == FALSE );
    LockLists();
    m_ShowOnlineStatus          = TRUE;
    m_Initialized               = TRUE;
    m_LocalIsServer             = FALSE;
    m_pSocket                   = &Local;
    m_AccumulatedTime           = 0.0f;
    m_ConnectStatus             = MATCH_CONN_UNAVAILABLE;
    m_pBrowser                  = NULL;
    m_IsLoggedOn                = FALSE;
    m_AcquisitionMode           = ACQUIRE_INVALID;            // Set it invalid to help asserts kick off.
    m_ExtendedServerInfoOwner   = -1;
    m_PendingAcquisitionMode    = ACQUIRE_INVALID;
    m_pDownloader               = NULL;
    m_UpdateInterval            = 50;
    m_HasNewContent             = FALSE;
    m_AuthStatus                = AUTH_STAT_IDLE;
    m_UserStatus                = BUDDY_STATUS_OFFLINE;
    m_PendingUserStatus         = BUDDY_STATUS_OFFLINE;
    m_pMessageOfTheDay          = NULL;
    m_MessageOfTheDayReceived   = FALSE;
    m_pSecurityChallenge        = NULL;
    m_Presence                  = NULL;
    m_IsVoiceCapable            = TRUE;

    m_FilterMaxPlayers          = -1;                       // Means no maximum players
    m_FilterMinPlayers          = -1;                       // Means no minimum players
    m_FilterGameType            = GAME_MP;                  // Means any game type
    m_RecentPlayers.Clear();

    x_memset( m_Nickname, 0, sizeof(m_Nickname) );
    //  m_pDefaultEULA      = NULL;
    // THIS IS JUST A HACK FOR NOW TO GIVE US BUDDY INFORMATION TO
    // SEARCH FOR.
#if defined(bwatson)
    SetBuddies( "<TNT>;DEV.BISCUIT;Cray" );
#else
    SetBuddies( "" );
#endif
    SetState(MATCH_IDLE);

    xtimer Delta;

    Delta.Start();
    Timeout = 0.0f;
    m_pThread = new xthread( s_MatchPeriodicUpdater, "MatchMgr periodic Updater", 8192, 1, 1, (char**)this );

#if defined(X_DEBUG) && defined(bwatson)
    // This will create a temporary patch
    MakePatch();
#endif
    UnlockLists();
    return GetConnectStatus() == MATCH_CONN_CONNECTED;
}

//------------------------------------------------------------------------------
void  match_mgr::Kill                ( void )
{
    if( m_Initialized )
    {
        ASSERT( AreListsLocked() == FALSE );
        LockLists();
        m_Initialized = FALSE;
        DisconnectFromMatchmaker();

        if( m_Presence )
        {
            gpDisconnect( &m_Presence );
            gpDestroy( &m_Presence );
        }
        if( m_pBrowser )
        {
            ServerBrowserFree( m_pBrowser );
            m_pBrowser = NULL;
        }

        xtimer timeout;
        timeout.Start();
        while( g_MatchMgr.IsBusy() )
        {
            x_DelayThread(10);
            if( timeout.ReadSec() > 1.0f )
            {
                ASSERTS(FALSE,"MatchMgr did not shutdown when killed");
                break;
            }
        }
        g_MatchMgr.SetState( MATCH_IDLE );
        delete m_pThread;
        m_pThread = NULL;
        UnlockLists();
        m_RecentPlayers.Clear();
        ResetServerList();
        m_Buddies.Clear();
        m_RecentPlayers.Clear();
        if( m_pMessageOfTheDayBuffer )
        {
            x_free( m_pMessageOfTheDayBuffer );
            m_pMessageOfTheDay = NULL;
            m_pMessageOfTheDayBuffer = NULL;
        }
        ReleasePatch();
    }
    else
    {
        SetState( MATCH_IDLE );
    }
}

//------------------------------------------------------------------------------
extern "C" s32 MEM_totalunused(s32 pool=0);

void  match_mgr::Update( f32 DeltaTime )
{
    m_Random.rand();

    if( !m_pSocket )
        return;

    if( m_pSocket->IsEmpty() )
    {
        Reset();
        m_pSocket = NULL;
        return;
    }

    m_AccumulatedTime += DeltaTime;
    m_NeedNewPing = TRUE;
    if( (m_PendingAcquisitionMode != ACQUIRE_INVALID) && ( (m_State==MATCH_IDLE) || (m_State==MATCH_ACQUIRE_IDLE) ) )
    {
        m_AcquisitionMode = m_PendingAcquisitionMode;
        m_PendingAcquisitionMode = ACQUIRE_INVALID;
        switch( m_AcquisitionMode )
        {
        case MATCH_ACQUIRE_SERVERS:
#if defined( ENABLE_LAN_LOOKUP )
            SetState( MATCH_ACQUIRE_LAN_SERVERS );
#else
            SetState( MATCH_ACQUIRE_SERVERS );
#endif
            break;
        case ACQUIRE_EXTENDED_SERVER_INFO:
            SetState( MATCH_ACQUIRE_EXTENDED_SERVER_INFO );
            break;
        case MATCH_ACQUIRE_BUDDIES:
            SetState( MATCH_ACQUIRE_BUDDIES );
            break;
        case MATCH_ACQUIRE_REGIONS:
            ASSERT( FALSE );
            break;
        case MATCH_ACQUIRE_LOBBIES:
            ASSERT( FALSE );
            break;
        default:
            ASSERT( FALSE );
        }
    }

    UpdateState(DeltaTime);
    LockBrowser();
    if( m_LocalIsServer )
    {
        qr2_think( NULL );
        NNThink();
    }

    if( m_pBrowser )
    {
        ASSERT( m_pBrowser );
        ServerBrowserThink( m_pBrowser );
    }

    switch( m_AuthStatus )
    {
    case AUTH_STAT_OK:

        // This will stop clients connecting to us when in campaign or split-screen
        if( (m_PendingUserStatus == BUDDY_STATUS_INGAME) && (GameMgr.IsGameOnline() == FALSE) )
             m_PendingUserStatus  = BUDDY_STATUS_ONLINE;

        if( m_PendingUserStatus != m_UserStatus )
        {
            GPEnum Status = GP_OFFLINE;

            LOG_MESSAGE("match_mgr::Update","User online status change from %s to %s", GetStatusName( m_UserStatus ), GetStatusName( m_PendingUserStatus ));
            switch( m_PendingUserStatus )
            {
            case BUDDY_STATUS_OFFLINE:      Status = GP_OFFLINE;        break;
            case BUDDY_STATUS_ONLINE:       Status = GP_ONLINE;         break;
            case BUDDY_STATUS_INGAME:       Status = GP_PLAYING;        break;
            case BUDDY_STATUS_ADD_PENDING:  
            case BUDDY_STATUS_REQUEST_ADD:
            default:                        ASSERT( FALSE );

            }
            char    TempString[64];
            s32     Flags = 0;          // To be done: Add user flags from the user profile.

            if( m_PendingUserStatus == BUDDY_STATUS_INGAME )
            {
                x_sprintf( TempString, "%s:%s/%s/%d", (const char*)xstring(g_ActiveConfig.GetShortGameType()),
                    (const char*)xstring(g_ActiveConfig.GetLevelName()),
                    g_ActiveConfig.GetConfig().External.GetStrAddress(),
                    Flags );
            }
            else
            {
                x_sprintf( TempString, "%d", Flags );
            }
            gpSetStatus( &m_Presence, Status, "", TempString );
            m_UserStatus = m_PendingUserStatus;
            LOG_MESSAGE( "match_mgr::Update", "User location reported as %s", TempString );
        }
    case AUTH_STAT_BUSY:
        gpProcess( &m_Presence );
        break;
    default:
        break;
    }

    m_UpdateInterval = 50;
    // If we're downloading from the matchmgr, lets make the update interval
    // really short. We should be able to get lots more data through using a
    // short interval.
    if( m_pDownloader )
    {
        m_pDownloader->Update( DeltaTime );
        m_UpdateInterval = 1;
    }
    UnlockBrowser();
}

//------------------------------------------------------------------------------
void match_mgr::Reset( void )
{
    LOG_MESSAGE( "match_mgr", "Reset" );

    // Reset everything, get rid of all the servers in the server list and
    // re-initiate the acquisition
    if( !m_pSocket )
        return;

    LockLists();
    ResetServerList();
    m_LobbyList.Clear();
    UnlockLists();

    LockBrowser();
    if( m_pBrowser )
    {
        ServerBrowserFree( m_pBrowser );
        m_pBrowser = NULL;
    }
    if( m_RegistrationComplete )
    {
        qr2_shutdown(NULL);
        m_RegistrationComplete = FALSE;
        m_UpdateRegistration = FALSE;
    }
    m_LocalIsServer = FALSE;
    UnlockBrowser();
}

//------------------------------------------------------------------------------
void match_mgr::UpdateState( f32 DeltaTime)
{
    if( m_StateTimeout >= 0.0f )
    {
        m_StateTimeout -= DeltaTime;
    }

    switch( m_State )
    {
        //-----------------------------------------------------
    case MATCH_IDLE:
        break;

        //-----------------------------------------------------
    case MATCH_AUTHENTICATE_MACHINE:
        SetConnectStatus( MATCH_CONN_CONNECTED );
        SetState( MATCH_GET_MESSAGES );
        break;
        //-----------------------------------------------------
    case MATCH_AUTH_DONE:
        break;

        //-----------------------------------------------------
    case MATCH_CONNECT_MATCHMAKER:
        {
            GSIACResult Result;

            Result = GSIAvailableCheckThink();
            if( Result != GSIACWaiting )
            {
                if( Result != GSIACAvailable )
                {
                    SetState( MATCH_IDLE );
                    SetConnectStatus( MATCH_CONN_UNAVAILABLE );
                    BREAK;
                }
                else
                {
#if defined(ENABLE_LAN_LOOKUP)
                    SetConnectStatus( MATCH_CONN_CONNECTED );
                    SetState( MATCH_IDLE );
#else
                    SetState( MATCH_AUTHENTICATE_USER );
#endif
                }
            }
        }
        break;

        //-----------------------------------------------------
    case MATCH_AUTHENTICATE_USER:
        switch( m_AuthStatus )
        {
        case AUTH_STAT_BUSY:
            break;
        case AUTH_STAT_OK:
            LOG_MESSAGE("match_mgr::UpdateState","AUTH_AUTHENTICATE_USER: Email address verified ok");
            SetState( MATCH_AUTH_USER_CONNECT );
            SetConnectStatus( MATCH_CONN_CONNECTED );
            break;
        case AUTH_STAT_NO_ACCOUNT:
            LOG_MESSAGE("match_mgr::UpdateState","AUTH_AUTHENTICATE_USER: No such email account");
            SetState( MATCH_AUTH_USER_CREATE );
            break;
        case AUTH_STAT_CANNOT_CONNECT:
            LOG_MESSAGE("match_mgr::UpdateState","AUTH_AUTHENTICATE_USER: Authentication failed");
            SetConnectStatus( MATCH_CONN_UNAVAILABLE );
            SetState( MATCH_IDLE );
            m_PendingState = MATCH_IDLE;
            break;
        default:
            break;
        }
        break;

        //------------------------------------------------------
    case MATCH_AUTH_USER_CONNECT:
        switch( m_AuthStatus )
        {
        case AUTH_STAT_BUSY:
            break;
        case AUTH_STAT_OK:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CONNECT: User account connected ok");
            SetState( MATCH_SECURITY_CHECK );
            SetUserStatus( BUDDY_STATUS_ONLINE );
            break;
        case AUTH_STAT_NO_ACCOUNT:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CONNECT: Account does not exist");
            SetState( MATCH_AUTH_USER_CREATE );
            break;
        case AUTH_STAT_CANNOT_CONNECT:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CONNECT: Authentication failed");
            SetConnectStatus( MATCH_CONN_UNAVAILABLE );
            SetState( MATCH_IDLE );
            break;
        default:
            break;
        }
        break;

        //------------------------------------------------------
    case MATCH_AUTH_USER_CREATE:
        switch( m_AuthStatus )
        {
        case AUTH_STAT_BUSY:
            break;
        case AUTH_STAT_OK:
            SetState( MATCH_SECURITY_CHECK );
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CREATE: User account created ok");
            SetUserStatus( BUDDY_STATUS_ONLINE );
            break;
        case AUTH_STAT_NO_ACCOUNT:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CREATE: User account creation failed");
            SetConnectStatus( MATCH_CONN_INVALID_ACCOUNT );
            SetState( MATCH_IDLE );
            break;
        case AUTH_STAT_CANNOT_CONNECT:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CREATE: Authentication failed");
            SetConnectStatus( MATCH_CONN_UNAVAILABLE );
            SetState( MATCH_IDLE );
            break;
        default:
            break;
        }
        break;

        //-----------------------------------------------------
    case MATCH_SECURITY_CHECK:

        if( m_SecurityChallengeReceived )
        {
            if( m_pSecurityChallenge )
            {
                xbool Result;
                Result = CheckSecurityChallenge( m_pSecurityChallenge );
                x_free( m_pSecurityChallenge );
                // Security challenge succeeded
                if( Result )
                {
                    SetConnectStatus( MATCH_CONN_CONNECTED );
                }
                else
                {
                    SetConnectStatus( MATCH_CONN_SECURITY_FAILED );
                    SetState( MATCH_IDLE );
                    break;
                }
                m_pSecurityChallenge = NULL;
            }
            m_HasNewContent = m_LocalManifestVersion < m_RemoteManifestVersion;
            if( m_HasNewContent )
            {
                LOG_MESSAGE( "match_mgr::UpdateState", "New patch available. LocalVersion:%d, RemoteVersion:%d", m_LocalManifestVersion, m_RemoteManifestVersion );
                x_sprintf(m_DownloadFilename,"%s/%d_%d_UPDATE.BIN",MANIFEST_LOCATION, eng_GetProductCode(), g_Changelist );
                InitDownload( m_DownloadFilename );
                ReleasePatch();
                SetState( MATCH_GET_PATCH );
            }
            else
            {
                LOG_MESSAGE( "match_mgr::UpdateState", "No new patch available. LocalVersion:%d, RemoteVersion:%d", m_LocalManifestVersion, m_RemoteManifestVersion );
                SetState( MATCH_IDLE );
            }
            break;
        }

        break;
        //-----------------------------------------------------
    case MATCH_GET_MESSAGES:
        // Fetch all the info on user motd@area51.midway.com. We will
        // use a special field to provide us with message of the day
        // information.
        switch( GetDownloadStatus() )
        {
        case DL_STAT_BUSY:
            break;
        case DL_STAT_OK:
            {
                s32     Length;
                char*   pData;
                m_MessageOfTheDayReceived = TRUE;
                pData = (char*)GetDownloadData( Length );
                if( Length )
                {
                    m_pMessageOfTheDayBuffer = (char*)x_malloc( Length+1 );
                    m_pMessageOfTheDayBuffer[Length] = 0x0;
                    ASSERT( m_pMessageOfTheDayBuffer );
                    m_pMessageOfTheDay = m_pMessageOfTheDayBuffer;
                    ASSERT( FALSE );
                    x_memcpy( m_pMessageOfTheDayBuffer, pData, Length );
                    //m_RemoteManifestVersion = manifest_GetNumber( (const char*)m_pMessageOfTheDay );
                    while( *m_pMessageOfTheDay )
                    {
                        if( *m_pMessageOfTheDay++ == '\n' )
                        {
                            break;
                        }
                    }
                    if( x_strlen( m_pMessageOfTheDay )==0 )
                    {
                        x_free( m_pMessageOfTheDayBuffer );
                        m_pMessageOfTheDayBuffer = NULL;
                        m_pMessageOfTheDay = NULL;
                    }
                    LOG_MESSAGE( "match_mgr::UpdateState", "Manifest Version:%d MOTD:%s", m_RemoteManifestVersion, GetMessageOfTheDay() );
                }
                else
                {
                    m_pMessageOfTheDay = NULL;
                    m_RemoteManifestVersion = m_LocalManifestVersion;
                }
                m_HasNewContent = (m_RemoteManifestVersion > m_LocalManifestVersion );
            }
            break;
        case DL_STAT_ERROR:
        case DL_STAT_NOT_FOUND:
            m_MessageOfTheDayReceived = TRUE;
            ASSERT( m_pMessageOfTheDay == NULL );
            ASSERT( m_pMessageOfTheDayBuffer == NULL );
            m_pMessageOfTheDay = NULL;
            m_pMessageOfTheDayBuffer = NULL;
            LOG_MESSAGE( "match_mgr::UpdateState", "There is no message of the day" );
            break;
        default:
            ASSERT( FALSE );
            break;
        }
        if( m_MessageOfTheDayReceived )
        {

            KillDownload();
            SetState( MATCH_AUTH_DONE );
        }
        break;

        //-----------------------------------------------------
    case MATCH_GET_PATCH:
        ASSERT( FALSE );
        break;
        //-----------------------------------------------------
    case MATCH_ACQUIRE_REGIONS:
        //        ReadRegions();
        break;

        //-----------------------------------------------------
    case MATCH_ACQUIRE_LOBBIES:
        ASSERT(FALSE);
        break;

        //-----------------------------------------------------
    case MATCH_ACQUIRE_SERVERS:
        // 
        // Get server list from matchmaker.
        //
        if( m_PendingAcquisitionMode != ACQUIRE_INVALID )
        {
            SetState( MATCH_IDLE );
        }
        else if( m_ConnectStatus == MATCH_CONN_IDLE )
        {
            SetState( MATCH_IDLE );
            StartAcquisition(static_cast<match_acquire>(MATCH_ACQUIRE_BUDDIES));
        }
        else if( m_StateTimeout <= 0.0f )
        {
            LOG_WARNING("match_mgr::UpdateState","Server browser did not go idle and timed out.");
            SetState( MATCH_IDLE );
            StartAcquisition(static_cast<match_acquire>(MATCH_ACQUIRE_BUDDIES));
        }
        else if( m_ConnectStatus != MATCH_CONN_ACQUIRING_SERVERS )
        {
            SetState( MATCH_IDLE );
        }
        else
        {
            RemoteLookups( DeltaTime );
            LocalLookups(DeltaTime);
        }
        break;

        //-----------------------------------------------------
    case MATCH_INDIRECT_LOOKUP:
        if( m_ConnectStatus != MATCH_CONN_ACQUIRING_SERVERS )
        {
            // We have some additional server information. We now wait until the main 
            // thread actually advances the connect sequence.
        }
        break;
        //-----------------------------------------------------
    case MATCH_ACQUIRE_EXTENDED_SERVER_INFO:
        break;

        //-----------------------------------------------------
    case MATCH_ACQUIRE_IDLE:
        LocalLookups( DeltaTime );
        break;

        //-----------------------------------------------------
    case MATCH_ACQUIRE_BUDDIES:
        LocalLookups( DeltaTime );
        RemoteLookups( DeltaTime );
        if( x_strlen(m_BuddyList)==0 )
        {
            SetConnectStatus( MATCH_CONN_IDLE );
        }

        if( m_ConnectStatus == MATCH_CONN_IDLE )
        {
            // We're done
            SetState( MATCH_IDLE );
            break;
        }
        else if( m_ConnectStatus != MATCH_CONN_ACQUIRING_BUDDIES )
        {
            // An error occurred
            SetState( MATCH_IDLE );
        }
        break;

        //-----------------------------------------------------
    case MATCH_ACQUIRE_LAN_SERVERS:
#if defined(ENABLE_LAN_LOOKUP)
        LocalLookups( DeltaTime );
        RemoteLookups( DeltaTime );
        if( m_ConnectStatus == MATCH_CONN_IDLE )
        {
            // We're done
            SetState( MATCH_ACQUIRE_IDLE );
        }
        else if( m_ConnectStatus != MATCH_CONN_ACQUIRING_SERVERS )
        {
            // An error occurred
            SetState( MATCH_IDLE );
        }
#else
        SetState(MATCH_ACQUIRE_IDLE);
#endif
        break;

        //-----------------------------------------------------
    case MATCH_BECOME_SERVER:
        LOG_APP_NAME( "SERVER" );
        LockLists();
        ResetServerList();
        m_LobbyList.Clear();
        m_LocalIsServer = TRUE;
        g_ActiveConfig.SetRemoteAddress( m_pSocket->GetAddress() );
        SetState(MATCH_VALIDATE_SERVER_NAME);
        UnlockLists();
        break;

        //-----------------------------------------------------
    case MATCH_VALIDATE_SERVER_NAME:
        SetState(MATCH_REGISTER_SERVER);
        break;

        //-----------------------------------------------------
    case MATCH_REGISTER_SERVER:
        //        if( m_pMatchMaker->IsRegistered() )
        {
            SetState( MATCH_SERVER_ACTIVE );
        }

        if( HasTimedOut() )
        {
            // We may want to notify the game player if the registration
            // fails. It means no one can join the game except on a local
            // network.
            LOG_ERROR("match_mgr::UpdateState","Server registration timed out.");
            SetConnectStatus( MATCH_CONN_REGISTER_FAILED );
            m_StateRetries--;
            if( m_StateRetries >= 0 )
            {
                // Kick off another registration attempt
                SetState( MATCH_REGISTER_SERVER );
            }
            else
            {
                // Becoming a server again resets all timeouts for retries.
                // We want to keep trying to register with the server. As the game
                // hoster may have decided to continue becoming a server.
                // 
                SetState( MATCH_BECOME_SERVER );
            }
        }
        break;

        //-----------------------------------------------------
    case MATCH_SERVER_CHECK_VISIBLE:
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
                SetState(MATCH_IDLE);
            }
        }
        if( m_IsVisible )
        {
            SetState(MATCH_IDLE);
        }
        break;

        //-----------------------------------------------------
    case MATCH_SERVER_ACTIVE:
        // Send an update to the matchmaking service so that it knows we're still here.
        if( m_UpdateRegistration )
        {
            // Some data has been changed on the server so we need to let
            // gamespy know about it.
            LockBrowser();
            LOG_MESSAGE("match_mgr::UpdateState", "Server registration update sent to GameSpy" );
            qr2_send_statechanged(NULL);
            m_UpdateRegistration = FALSE;
            UnlockBrowser();
        }
        break;

        //-----------------------------------------------------
    case MATCH_UNREGISTER_SERVER:
        {
            SetConnectStatus( MATCH_CONN_IDLE );
            SetState(MATCH_IDLE);
        }

        break;

        //-----------------------------------------------------
    case MATCH_NAT_NEGOTIATE:
        if( m_ConnectStatus == MATCH_CONN_CONNECTED )
        {
            // The main thread will advance us to the next stage
        }
        else if( m_ConnectStatus != MATCH_CONN_CONNECTING )
        {
            SetState( MATCH_IDLE );
        }
        else
        {
            NNThink();
        }
        break;

        //-----------------------------------------------------
    case MATCH_LOGIN:
        if( g_NetworkMgr.IsLoggedIn() )
        {
            // This is the state that the client must wait for prior
            // to trying to fetch the map.
            SetState( MATCH_CLIENT_ACTIVE );
        }
        else if( g_ActiveConfig.GetExitReason() != GAME_EXIT_CONTINUE )
        {
            SetState( MATCH_IDLE );
        }
        break;
        //-----------------------------------------------------
    case MATCH_CLIENT_ACTIVE:
        // Clients don't do anything.
        break;

        //-----------------------------------------------------
    default:
        break;
    }
}
//------------------------------------------------------------------------------
// Called whenever we transition from one state to another
void match_mgr::ExitState( match_mgr_state OldState )
{
    switch( OldState )
    {
    case MATCH_ACQUIRE_SERVERS:
#if 0
        // This object needs to be kept around so that we can get extended server information
        ASSERT( m_pBrowser );
        ServerBrowserFree( m_pBrowser );
        m_pBrowser = NULL;
#endif
        break;
    default:
        break;
    }
}

//------------------------------------------------------------------------------
// Called whenever we transition from one state to another
void match_mgr::EnterState( match_mgr_state NewState )
{
    GPResult    Status;

    m_StateRetries  = STATE_RETRIES;
    SetTimeout(STATE_TIMEOUT);
    switch( NewState )
    {
        //-----------------------------------------------------
    case MATCH_AUTHENTICATE_MACHINE:
        {
        }
        break;

        //------------------------------------------------------
    case MATCH_CONNECT_MATCHMAKER:
        GSIStartAvailableCheck(_T("gmtest"));
        break;

        //-----------------------------------------------------
    case MATCH_AUTHENTICATE_USER:
#if defined(ENABLE_LAN_LOOKUP)
        m_AuthStatus = AUTH_STAT_OK;
#else
        {
            // Start user authentication check
            Status = gpInitialize( &m_Presence, GAMESPY_PRODUCT_ID, 0 );
            ASSERT( Status == GP_NO_ERROR );
            if( Status != GP_NO_ERROR )
            {
                m_AuthStatus = AUTH_STAT_CANNOT_CONNECT;
            }
            //
            // Maybe should use different callback for presence error?
            //

            ASSERT( m_AuthStatus == AUTH_STAT_IDLE );
            m_AuthStatus = AUTH_STAT_BUSY;
            x_sprintf( m_DownloadFilename, "%s@%s", m_UniqueId, EMAIL_POSTFIX );
            Status = gpSetCallback(&m_Presence, GP_ERROR, (GPCallback)gamespy_adderror, this );
            ASSERT( Status == GP_NO_ERROR );
            Status = gpSetCallback(&m_Presence, GP_RECV_BUDDY_REQUEST, (GPCallback)gamespy_buddy_request, this );
            ASSERT( Status == GP_NO_ERROR );
            Status = gpSetCallback(&m_Presence, GP_RECV_BUDDY_STATUS, (GPCallback)gamespy_buddy_status, this );
            ASSERT( Status == GP_NO_ERROR );
            Status = gpSetCallback(&m_Presence, GP_RECV_GAME_INVITE, (GPCallback)gamespy_buddy_invite, this );
            ASSERT( Status == GP_NO_ERROR );

            LOG_MESSAGE("match_mgr::EnterState","MATCH_AUTHENTICATE_USER: AccountName:%s",m_DownloadFilename );
            // Check to see if we have an account
            Status = gpIsValidEmail(   &m_Presence, 
                m_DownloadFilename, 
                GP_NON_BLOCKING, 
                (GPCallback)gamespy_emailcheck, 
                this );
            ASSERT( Status == GP_NO_ERROR );
            if( Status != GP_NO_ERROR )
            {
                m_AuthStatus = AUTH_STAT_CANNOT_CONNECT;
            }
        }
#endif
        break;

        //-----------------------------------------------------
    case MATCH_AUTH_USER_CREATE:
        {
            ASSERT( x_strlen(m_Nickname) > 0 );
            m_AuthStatus = AUTH_STAT_BUSY;
            LOG_MESSAGE("match_mgr::EnterState","MATCH_AUTH_USER_CREATE: AccountName:%s, Nickname:%s",m_DownloadFilename, m_Nickname );
            Status = gpConnectNewUser( &m_Presence, 
                m_Nickname,                     // Nickname
                NULL,                           // Unique Nick
                m_DownloadFilename,                  // Email address
                (const char*)m_UniqueId,        // Password
                NULL,                           // CD Key
                GP_FIREWALL,                    // Is firewalled
                GP_NON_BLOCKING,                // Is a blocking call
                (GPCallback)gamespy_connect,    // Callback when done
                this );
            ASSERT( Status == GP_NO_ERROR );
            if( Status != GP_NO_ERROR )
            {
                m_AuthStatus = AUTH_STAT_CANNOT_CONNECT;
            }
        }
        break;

        //-----------------------------------------------------
    case MATCH_AUTH_USER_CONNECT:
        {
            ASSERT( x_strlen(m_Nickname) > 0 );
            m_AuthStatus = AUTH_STAT_BUSY;
            LOG_MESSAGE("match_mgr::EnterState","MATCH_AUTH_USER_CONNECT: AccountName:%s, Nickname:%s",m_DownloadFilename, m_Nickname );
            Status = gpConnect(    &m_Presence, 
                m_Nickname,                         // Nickname - this will come from a profile
                m_DownloadFilename,                      // Email address
                (const char*)m_UniqueId,            // Password
                GP_FIREWALL,                        // Is Firewalled
                GP_NON_BLOCKING,                    // Is Blocking
                (GPCallback)gamespy_connect,        // Completion callback
                this );
            ASSERT( Status == GP_NO_ERROR );
            if( Status != GP_NO_ERROR )
            {
                m_AuthStatus = AUTH_STAT_CANNOT_CONNECT;
            }
            //**** TEMPORARY TEST CODE ****
            // Create a buddy.
        }
        break;

        //-----------------------------------------------------
    case MATCH_NAT_NEGOTIATE:
        {
            SBError Status;
            const net_address& Remote = g_ActiveConfig.GetRemoteAddress();

            LOG_MESSAGE( "match_mgr::BecomeClient","Begin NAT negotiation with %s", Remote.GetStrAddress() );
            m_SessionID = (m_Random.rand() << 16) | m_Random.rand();

            Status = ServerBrowserSendNatNegotiateCookieToServer( m_pBrowser,
                Remote.GetStrIP(),
                Remote.GetPort(), 
                m_SessionID );
            ASSERT( Status == sbe_noerror );
            NNBeginNegotiationWithSocket( (SOCKET)m_pSocket, m_SessionID, 1, gamespy_nat_progress, gamespy_nat_complete, this );
        }
        break;
        //-----------------------------------------------------
    case MATCH_LOGIN:
        // ** NOTE ** the negotiation callback will advance the state to either
        // MATCH_IDLE if there is an error, or MATCH_CLIENT_ACTIVE if negotiation completed.
        LOG_APP_NAME( "CLIENT" );
        // Make sure we're not trying to get any data
        DisconnectFromMatchmaker();
        g_NetworkMgr.BeginLogin();
        LockLists();
        ResetServerList();
        m_LobbyList.Clear();
        UnlockLists();
        m_LocalIsServer = FALSE;
        break;

        //-----------------------------------------------------
    case MATCH_SECURITY_CHECK:
        break;

        //-----------------------------------------------------
    case MATCH_SERVER_ACTIVE:
        break;

        //-----------------------------------------------------
    case MATCH_ACQUIRE_SERVERS:
        break;

        //-----------------------------------------------------
    case MATCH_ACQUIRE_LOBBIES:
    case MATCH_ACQUIRE_REGIONS:
        break;

        //-----------------------------------------------------
    default:
        break;
    }
    m_State         = NewState;
}

//------------------------------------------------------------------------------
void match_mgr::CheckVisibility(void)
{
    if( !m_IsVisible )
    {

    }
}

//------------------------------------------------------------------------------
void match_mgr::StartAcquisition( match_acquire AcquisitionMode )
{
    LockLists();
    switch( AcquisitionMode )
    {
    case MATCH_ACQUIRE_SERVERS:
        ResetServerList();
        break;
    case MATCH_ACQUIRE_BUDDIES:
        break;
    case MATCH_ACQUIRE_REGIONS:
        break;
    case MATCH_ACQUIRE_LOBBIES:
        m_LobbyList.Clear();
        break;

    case ACQUIRE_EXTENDED_SERVER_INFO:
        break;
    default:
        ASSERT( FALSE );
    }
    m_PendingAcquisitionMode = AcquisitionMode;
    UnlockLists();
}

//------------------------------------------------------------------------------
xbool match_mgr::IsAcquireComplete( void )
{
    switch( m_AcquisitionMode )
    {
    case MATCH_ACQUIRE_SERVERS:
        return (m_State==MATCH_ACQUIRE_IDLE);
        break;
    case MATCH_ACQUIRE_BUDDIES:
        return (m_State==MATCH_IDLE);
        break;
    case MATCH_ACQUIRE_REGIONS:
        ASSERT( FALSE );
        break;
    case MATCH_ACQUIRE_LOBBIES:
        ASSERT( FALSE );
        break;
    default:
        //ASSERT( FALSE );
        // MAB: Removed because it takes a couple of frames for m_AcquisitionMode
        //      to get set after start acquisition has been called.
        break;
    }
    return FALSE;
}

//------------------------------------------------------------------------------
// SetState will change internal state and initiate any packets that are
// required for the processing of the state just entered.

void match_mgr::SetState( match_mgr_state NewState )
{

    SetTimeout( STATE_TIMEOUT );

    LOG_MESSAGE( "match_mgr::SetState", "Old:%s - New:%s",
        GetStateName(m_State), GetStateName(NewState) );

    if( NewState != m_State )
    {
        ExitState(m_State);
        EnterState(NewState);
    }

    // The following actions happen every time the state is set whether or not it has just
    // been entered. Good place to initiate requests which may get a retry.
    switch( NewState )
    {

        //------------------------------------------------------
    case MATCH_GET_MESSAGES:
        if( m_MessageOfTheDayReceived == FALSE )
        {
            if( m_pMessageOfTheDay )
            {
                x_free( m_pMessageOfTheDay );
                m_pMessageOfTheDay = NULL;
            }
            x_sprintf(m_DownloadFilename,"%s/%s_MOTD.txt",MANIFEST_LOCATION, x_GetLocaleString() );
            InitDownload( m_DownloadFilename );
        }
        break;
        //------------------------------------------------------
    case MATCH_SECURITY_CHECK:
        {
            if( m_pBrowser == NULL )
            {
                m_pBrowser = ServerBrowserNew ( GAMESPY_GAMENAME, GAMESPY_GAMENAME, GAMESPY_SECRETKEY, 0, 40, QVERSION_QR2, gamespy_server_query, this);
                ASSERT( m_pBrowser );
            }


            m_SecurityChallengeReceived = FALSE;
            ASSERT( m_pSecurityChallenge == NULL );

#if defined(X_DEBUG)
            xfs Email( "sec_%d_%d@area51.midway.com", eng_GetProductCode(), 0 );
#else
            xfs Email( "sec_%d_%d@area51.midway.com", eng_GetProductCode(), g_Changelist );
#endif
            LOG_MESSAGE( "match_mgr::SetState", "Looking up user %s", (const char*)Email );
            gpProfileSearch( &m_Presence, 
                NULL,                                   // Nickname
                NULL,                                   // UniqueNick
                Email,                                  // Email
                NULL,                                   // Firstname
                NULL,                                   // Lastname
                0,                                      // Icquin
                GP_NON_BLOCKING,                        // Blocking
                (GPCallback)gamespy_security_complete,  // Callback
                this );                                 // Callback data
        }
        break;
        //------------------------------------------------------
    case MATCH_VALIDATE_PLAYER_NAME:
        break;

        //------------------------------------------------------
    case MATCH_ACQUIRE_REGIONS:
        SetConnectStatus( MATCH_CONN_ACQUIRING_LOBBIES );
        m_LobbyList.Clear();
        ConnectToMatchmaker(MATCH_ACQUIRE_REGIONS);
        break;

        //------------------------------------------------------
    case MATCH_ACQUIRE_LOBBIES:
        m_LobbyList.Clear();
        break;

        //------------------------------------------------------
    case MATCH_ACQUIRE_SERVERS:
        SetTimeout( LOOKUP_RETRY_INTERVAL );
        LockLists();
        ResetServerList();
        m_ExtendedServerInfoOwner = -1;
        m_LookupTimeout = 0.0f;
        UnlockLists();
        LockBrowser();

        if( m_pBrowser )
        {
            ServerBrowserFree( m_pBrowser );
            m_pBrowser = NULL;
        }
        m_StateTimeout = 5.0f;
        // Do we have a server browser object reinit?
        m_pBrowser = ServerBrowserNew ( GAMESPY_GAMENAME, GAMESPY_GAMENAME, GAMESPY_SECRETKEY, 0, 40, QVERSION_QR2, gamespy_server_query, this);
        ASSERT( m_pBrowser );
        if( m_pBrowser )
        {
            unsigned char basicFields[] = { HOSTNAME_KEY, 
                GAMETYPE_KEY,  
                GAMEMODE_KEY, 
                MAPNAME_KEY, 
                NUMPLAYERS_KEY, 
                MAXPLAYERS_KEY, 
                GAMEVER_KEY,
                GAMETYPE_ID_KEY,
                FRAGLIMIT_KEY,
                TIMELIMIT_KEY,
                LEVEL_KEY,
                FLAGS_KEY,
                PLAYERS_KEY,
                AVGPING_KEY,
            };
            int numFields = sizeof(basicFields) / sizeof(basicFields[0]);

            SBError Result;

            qr2_register_key( GAMETYPE_ID_KEY, "gametypeid" );
            qr2_register_key( LEVEL_KEY, "level" );
            qr2_register_key( PLAYERS_KEY, "players" );
            qr2_register_key( FLAGS_KEY, "svrflags" );      // Can't use FLAGS because it's taken by the internal state
            qr2_register_key( AVGPING_KEY, "avgping" );

            char Filter[128];
            xbool FirstClause = TRUE;

            x_memset(Filter, 0, sizeof(Filter) );

            if( m_FilterGameType != GAME_MP )
            {
                x_strcat( Filter, xfs("gametypeid=%d", m_FilterGameType) );
                FirstClause = FALSE;
            }

            if( m_FilterMinPlayers != -1 )
            {
                if( !FirstClause ) 
                { 
                    x_strcat( Filter," and " );
                }
                FirstClause = FALSE;
                x_strcat( Filter, xfs("numplayers >= %d", m_FilterMinPlayers) );
            }

            if( m_FilterMaxPlayers != -1 )
            {
                if( !FirstClause ) 
                { 
                    x_strcat( Filter," and " );
                }
                FirstClause = FALSE;
                x_strcat( Filter, xfs("numplayers <= %d", m_FilterMaxPlayers) );
            }

            Result = ServerBrowserUpdate(m_pBrowser, SBTrue, SBFalse, basicFields, numFields, Filter );
			#ifdef X_LOGGING
            LOG_MESSAGE("match_mgr::SetState","ServerBrowserUpdate() returned %d(%s)", Result, ServerBrowserError( Result ) );
			#endif
            SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );

        }
        UnlockBrowser();
        break;

        //------------------------------------------------------
    case MATCH_ACQUIRE_EXTENDED_SERVER_INFO:
        {
            LockBrowser();
            if( m_pBrowser )
            {
                ServerBrowserFree( m_pBrowser );
                m_pBrowser = NULL;
            }
            m_StateTimeout = 5.0f;
            // Do we have a server browser object reinit?
            m_pBrowser = ServerBrowserNew ( GAMESPY_GAMENAME, GAMESPY_GAMENAME, GAMESPY_SECRETKEY, 0, 40, QVERSION_QR2, gamespy_extended_info_query, this);
            ASSERT( m_pBrowser );
            if( m_pBrowser )
            {
                SBError Result;
                s32 i;
                for( i=0; i< m_ServerList.GetCount(); i++ )
                {
                    if( m_ExtendedServerInfoOwner == m_ServerList[i].ID )
                    {
                        break;
                    }
                }
                ASSERT( i!=m_ServerList.GetCount() );
                const net_address& Remote = m_ServerList[ i ].Remote;
                Result = ServerBrowserAuxUpdateIP(  m_pBrowser,                  // Browser object
                    Remote.GetStrIP(),           // Browser IP address in string format
                    Remote.GetPort(),            // Browser port number
                    SBTrue,                      // viaMaster
                    SBTrue,                      // async
                    SBFalse                      // fullUpdate 
                    );
				#ifdef X_LOGGING	
                LOG_MESSAGE("match_mgr::SetState","ServerBrowserAuxUpdateIP(%s) returned %d(%s)", Remote.GetStrAddress(), Result, ServerBrowserError( Result ) );
				#endif
                SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
            }
            UnlockBrowser();
        }
        break;

        //------------------------------------------------------
    case MATCH_ACQUIRE_BUDDIES:
        if( x_strlen(m_BuddyList)!=0 )
        {
            SetTimeout( LOOKUP_RETRY_INTERVAL );

            LockBrowser();
            m_LookupTimeout = 0.0f;
            if( m_pBrowser )
            {
                ServerBrowserFree( m_pBrowser );
                m_pBrowser = NULL;
            }
            // Do we have a server browser object reinit?
            ///**** THIS NEEDS TO BE CHANGED TO FORCE BUDDY UPDATE? *****
            m_pBrowser = ServerBrowserNew ( GAMESPY_GAMENAME, GAMESPY_GAMENAME, GAMESPY_SECRETKEY, 0, 40, QVERSION_QR2, gamespy_server_query, this);
            ASSERT( m_pBrowser );
            if( m_pBrowser )
            {
                SBError Result;

                Result = ServerBrowserUpdate(m_pBrowser, SBTrue, SBFalse, NULL, 0, m_BuddyList);
                SetConnectStatus( MATCH_CONN_ACQUIRING_BUDDIES );
				#ifdef X_LOGGING
                LOG_MESSAGE("match_mgr::SetState","ServerBrowserUpdate(Buddies) returned %d(%s)", Result, ServerBrowserError( Result ) );
				#endif

            }
            else
            {
                SetConnectStatus( MATCH_CONN_IDLE );
            }
            UnlockBrowser();

        }
        // The state callback will change the state back to idle when
        // the buddy list acquisition is complete.

        break;

        //------------------------------------------------------
    case MATCH_ACQUIRE_LAN_SERVERS:
#if defined(ENABLE_LAN_LOOKUP)
        LockBrowser();
        if( m_pBrowser )
        {
            ServerBrowserFree( m_pBrowser );
            m_pBrowser = NULL;
        }
        // Do we have a server browser object reinit?
        m_pBrowser = ServerBrowserNew ( GAMESPY_GAMENAME, GAMESPY_GAMENAME, GAMESPY_SECRETKEY, 0, 40, QVERSION_QR2, gamespy_server_query, this );
        ASSERT( m_pBrowser );
        if( m_pBrowser )
        {
            SBError Result;

            qr2_register_key( LEVEL_KEY, "level" );
            qr2_register_key( PLAYERS_KEY, "players" );
            qr2_register_key( FLAGS_KEY, "svrflags" );      // Can't use FLAGS because it's taken by the internal state
            qr2_register_key( AVGPING_KEY, "avgping" );

            Result = ServerBrowserLANUpdate( m_pBrowser, SBTrue, m_pSocket->GetPort(), m_pSocket->GetPort()+16 );
			#ifdef X_LOGGING
            LOG_MESSAGE("match_mgr::SetState","ServerBrowserLANUpdate() returned %d(%s)", Result, ServerBrowserError( Result ) );
            SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
			#endif
        }
        else
        {
            SetConnectStatus( MATCH_CONN_IDLE );
        }
        UnlockBrowser();
#endif
        break;

        //------------------------------------------------------
    case MATCH_INDIRECT_CONNECT:
        SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
        // Start a lookup to the matchmaking service about this particular server
        SetTimeout( LOOKUP_RETRY_INTERVAL );
        LockLists();
        ResetServerList();
        m_ExtendedServerInfoOwner = -1;
        m_LookupTimeout = 0.0f;
        UnlockLists();

        LockBrowser();
        if( m_pBrowser )
        {
            ServerBrowserFree( m_pBrowser );
            m_pBrowser = NULL;
        }
        m_StateTimeout = 5.0f;
        // Do we have a server browser object reinit?
        m_pBrowser = ServerBrowserNew ( GAMESPY_GAMENAME, GAMESPY_GAMENAME, GAMESPY_SECRETKEY, 0, 40, QVERSION_QR2, gamespy_indirect_server_query, this);
        ASSERT( m_pBrowser );
        if( m_pBrowser )
        {
            SBError Result;

            const server_info& Config = g_ActiveConfig.GetConfig();
            Result = ServerBrowserAuxUpdateIP(  m_pBrowser,                         // Browser object
                Config.Remote.GetStrIP(),           // Browser IP address in string format
                Config.Remote.GetPort(),            // Browser port number
                SBTrue,                             // viaMaster
                SBTrue,                             // async
                SBFalse                             // fullUpdate 
                );
			#ifdef X_LOGGING	
            LOG_MESSAGE("match_mgr::SetState","ServerBrowserAuxUpdateIP(%s) returned %d(%s)", Config.Remote.GetStrAddress(), Result, ServerBrowserError( Result ) );
			#endif
            SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );

        }
        UnlockBrowser();
        break;

        //------------------------------------------------------
    case MATCH_NAT_NEGOTIATE:
        SetConnectStatus( MATCH_CONN_CONNECTING );
        break;

        //------------------------------------------------------
    case MATCH_REGISTER_SERVER:
        {
            LockBrowser();

            qr2_error_t QueryError;
            qr2_register_key( LEVEL_KEY, "level" );
            qr2_register_key( PLAYERS_KEY, "players" );
            qr2_register_key( FLAGS_KEY, "svrflags" );      // Can't use FLAGS because it's taken by the internal state
            qr2_register_key( AVGPING_KEY, "avgping" );

            m_RegistrationComplete = TRUE;
            m_UpdateRegistration   = FALSE;
            QueryError = qr2_init_socket(   
                NULL,                   // query record?
                (SOCKET)m_pSocket,      // Local Socket
                m_pSocket->GetPort(),   // Local Port
                GAMESPY_GAMENAME,       // Game Name
                GAMESPY_SECRETKEY,      // Secret Key
#if defined( ENABLE_LAN_LOOKUP )
                SBFalse,                // Is Public
#else
                SBTrue,                 // Is Public
#endif
                SBTrue,                 // Perform NAT negotiation
                gamespy_serverkey,      // Server key probe
                gamespy_playerkey,      // Player key probe
                gamespy_teamkey,        // Team key probe
                gamespy_keylist,        // Give our list of keys?
                gamespy_count,          // Player counts?
                gamespy_adderror,       // Some error?
                this);                  // User data

            qr2_register_natneg_callback( NULL, gamespy_nat_negotiate );
            qr2_register_publicaddress_callback( NULL, gamespy_public_address );
            UnlockBrowser();
            LOG_MESSAGE("match_mgr::SetState","Started server registration. Error code: %d",QueryError );
        }
        break;

        //------------------------------------------------------
    case MATCH_VALIDATE_SERVER_NAME:
        break;

        //------------------------------------------------------
    case MATCH_SERVER_CHECK_VISIBLE:
        break;

        //------------------------------------------------------
    case MATCH_UNREGISTER_SERVER:
        Reset();
        SetConnectStatus( MATCH_CONN_DISCONNECTED );
        m_LocalIsServer = FALSE;
        // Tell gamespy our server is no longer active. We stop the browsing.
        LOG_MESSAGE("match_mgr::SetState","Kill server");
        break;

        //------------------------------------------------------
    case MATCH_SERVER_ACTIVE:
        break;
    default:
        break;
    }

}

//------------------------------------------------------------------------------
xbool match_mgr::ReceivePacket( net_address& Remote, bitstream& Bitstream )
{
    byte*   pData;
    s32     Length;

    pData =     Bitstream.GetDataPtr();
    Length =    Bitstream.GetNBytes();

    if( (pData[0] == NN_MAGIC_0) && (pData[1] == NN_MAGIC_1) &&
        (pData[2] == NN_MAGIC_2) && (pData[3] == NN_MAGIC_3) &&
        (pData[4] == NN_MAGIC_4) && (pData[5] == NN_MAGIC_5) )
    {
        struct sockaddr_in from;

        // Convert our internal addresses from host endian to network endian as the gamespy
        // libs require it to be network endian.
        from.sin_port = htons(Remote.GetPort());
        from.sin_addr.s_addr = htonl(Remote.GetIP());
        LockBrowser();
        NNProcessData( (char*)Bitstream.GetDataPtr(), Bitstream.GetNBytes(), &from);
        UnlockBrowser();
        LOG_MESSAGE("match_mgr::ReceivePacket","Packet forwarded to Nat Negotiator, from:%s, Length:%d", Remote.GetStrAddress(), Length );
        return TRUE;
    }

    if( (pData[0] == QR_MAGIC_1) && (pData[1] == QR_MAGIC_2) &&
        m_LocalIsServer )
    {
        struct sockaddr_in sender;

        // Convert our internal addresses from host endian to network endian as the gamespy
        // libs require it to be network endian.
        sender.sin_port = htons(Remote.GetPort());
        sender.sin_addr.s_addr = htonl(Remote.GetIP());
        LockBrowser();
        qr2_parse_query(NULL, (char*)Bitstream.GetDataPtr(), Bitstream.GetNBytes(), (sockaddr*)&sender);
        UnlockBrowser();
        LOG_MESSAGE("match_mgr::ReceivePacket","Packet forwarded to QR2, from:%s, Length:%d", Remote.GetStrAddress(), Length );
        return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------------
// This append server instance is called when a fully complete response is received
// via the matchmaking service.
void match_mgr::AppendServer( const server_info& Response )
{
    s32 Index;

    if( Response.Version != g_ServerVersion )
    {
        return;
    }
    // Search the server list to see if we already have an entry. If we do, then
    // just update the individual fields, otherwise append this entry to the main
    // list.
    LockLists();

    Index = m_ServerList.Find( Response );

    if( Index >=0 )
    {
        server_info* pServerInfo = GetServerInfo( Index );

        s32 Flags = pServerInfo->Flags;

        *pServerInfo = Response;
        pServerInfo->Flags = Flags;
    }
    else
    {
        
        AppendToServerList( Response );
    }
    UnlockLists();
}

//------------------------------------------------------------------------------
void match_mgr::LocalLookups(f32 DeltaTime)
{
    //
    // Now deal with sending out a local lookup request
    //
    m_LookupTimeout -= DeltaTime;

    if( m_LookupTimeout < 0.0f )
    {
        // It appears the ServerBrowserLANUpdate does not function as expected when doing local
        // lookups after a list has already been acquired. Any new server that appears on the
        // local network does not appear to be getting added to the list. Since the 'auto' portion
        // is only used during AutoClient mode, the 'refresh' has been shifted to the onlinejoin
        // dialog.
        if( CONFIG_IS_AUTOCLIENT )
        {
            //ServerBrowserLANUpdate( m_pBrowser, SBTrue, m_pSocket->GetPort(), m_pSocket->GetPort() );
        }
        m_LookupTimeout = LOOKUP_RETRY_INTERVAL;
    }
}
//------------------------------------------------------------------------------
f32 match_mgr::GetPingTime(s32 Index)
{
    return x_TicksToMs( x_GetTime() - m_PingSendTimes[Index % MAX_PING_OUTSTANDING] );
}

//------------------------------------------------------------------------------
void match_mgr::DisconnectFromMatchmaker(void)
{
    SetConnectStatus( MATCH_CONN_DISCONNECTED );
}
//------------------------------------------------------------------------------
void match_mgr::ConnectToMatchmaker( match_mgr_state PendingState )
{
    m_PendingState = PendingState;
    SetState(MATCH_CONNECT_MATCHMAKER);
    SetConnectStatus( MATCH_CONN_CONNECTING );
}

//==============================================================================

xbool match_mgr::ValidateLobbyInfo( const lobby_info &info )
{
    ASSERT( info.Name[0] != 0 );
    if( !( info.Name[0] != 0 ) )
    {
        LOG_WARNING( "match_mgr::ValidateLobbyInfo", "Invalid Lobby" );
        return FALSE;
    }

    return TRUE;
}

//==============================================================================
static s32 GetHex( const char* &pChallenge, s32 Count )
{
    s32 Value;
    s32 Digit;

    Value = 0;
    while( Count )
    {
        Digit = *pChallenge-'0';
        if( Digit < 0 )
        {
            return 0;
        }
        if( Digit > 9 )
        {
            Digit -= ('a'-'9'-1);
        }
        if( Digit > 15 )
        {
            return 0;
        }

        Value = Value * 16 + Digit;
        Count--;
        pChallenge++;
    }
    return Value;
}

//==============================================================================
xbool match_mgr::CheckSecurityChallenge( const char* pChallenge )
{
    s32     Start;
    s32     Length;
    s32     DesiredResult;
    char    Polarity;
    s32     ChecksumResult;
    s32     ChecksumCount = 0;

    while( *pChallenge )
    {
        Start           = GetHex( pChallenge, 6 );
        Length          = GetHex( pChallenge, 4 );
        DesiredResult   = GetHex( pChallenge, 4 );
        Polarity        = *pChallenge++;
        if( (Polarity != '-') && (Polarity != '+') )
        {
            return FALSE;
        }

        if( (Start == 0) || (Length==0) )
        {
            return FALSE;
        }
        if( ((Start+Length) > 32*1048576) || (Start & 3) )
        {
            return FALSE;
        }
        ChecksumResult = (x_chksum( (void*)Start, Length ) & 0xffff);

        LOG_MESSAGE( "match_mgr::CheckSecurityChallenge", "Security challenge. Start:0x%06x, Length:0x%04x, Desired:0x%04x, Actual:0x%04x, Polarity:%c", Start, Length, DesiredResult, ChecksumResult, Polarity );

        if( ChecksumResult == DesiredResult )
        {
            if( Polarity == '-' )
            {
                return FALSE;
            }
        }
        else
        {
            if( Polarity == '+' )
            {
                return FALSE;
            }
        }
        ChecksumCount++;
    }

    return ChecksumCount > 0;
}

//==============================================================================
const byte* SAFE_LOW = (const byte*)(512*1024);
const byte* SAFE_HIGH= (const byte*)(32*1024*1024);

void match_mgr::IssueSecurityChallenge(void)
{
}

//==============================================================================
// The buddy string is a list of substrings seperated by semicolons
void match_mgr::SetBuddies( const char* pBuddyString )
{
    char    SingleBuddy[16];
    s32     LastMarker;
    xbool   NeedOr;

    ASSERT( x_strlen(pBuddyString) < sizeof(m_BuddyList)-1 );
    x_memset( m_BuddyList,0, sizeof(m_BuddyList) );

    NeedOr = FALSE;
    while( *pBuddyString )
    {
        char* pBuddy;
        pBuddy = SingleBuddy;
        while( *pBuddyString && (*pBuddyString != ';') )
        {
            *pBuddy++ = *pBuddyString++;
        }
        *pBuddy = 0x0;
        if( *pBuddyString ==';' )
        {
            pBuddyString++;
        }
        LastMarker = x_strlen( m_BuddyList );
        if( NeedOr )
        {
            x_strcpy( m_BuddyList+LastMarker, " or ");
        }
        x_sprintf( m_BuddyList+x_strlen( m_BuddyList ), "(players like '%%%s%%')", SingleBuddy );
        // Just a sanity check so we don't form a bad list of buddy names should we exceed the
        // length of the buddy string. This should *NOT* happen.
        if( x_strlen(m_BuddyList) > 480 )
        {
            m_BuddyList[ LastMarker ] = 0;
            break;
        }
        NeedOr = TRUE;
    }
}

//==============================================================================

void match_mgr::NotifyKick(const char* UniqueId)
{
    (void)UniqueId;
}

//==============================================================================

void match_mgr::RemoteLookups( f32 DeltaTime )
{
    (void)DeltaTime;
}

//==============================================================================

void match_mgr::SetUserAccount( s32 UserIndex )
{
    ASSERT( UserIndex < m_UserAccounts.GetCount() );
    m_ActiveAccount = UserIndex;
}

//==============================================================================

s32 match_mgr::GetAuthResult( char* pLabelBuffer )
{
    ASSERTS( m_ConnectStatus == MATCH_CONN_UNAVAILABLE, "This should only be called if authentication fails." );

    x_strcpy( pLabelBuffer, m_ConnectErrorMessage );
    return m_ConnectErrorCode;
}

//==============================================================================
void match_mgr::StartIndirectLookup( void )
{
    m_SessionID = x_rand();
    game_config::Commit();
    LockBrowser();
    SetState( MATCH_INDIRECT_LOOKUP );
    UnlockBrowser();
}

//==============================================================================
void match_mgr::StartLogin( void )
{
    LockBrowser();
    SetState( MATCH_LOGIN );
    UnlockBrowser();
}

//==============================================================================

void match_mgr::BecomeClient( const server_info& Config )
{
    m_SessionID = x_rand();

    game_config::Commit( Config );

    switch( Config.ConnectionType )
    {
    case CONNECT_DIRECT:
        LOG_MESSAGE("match_mgr::BecomeClient","Direct connection available.");
        SetState( MATCH_LOGIN );
        break;
    case CONNECT_NAT:
        LOG_MESSAGE("match_mgr::BecomeClient","Server is behind a firewall. Starting NAT negotiation.");
        SetState( MATCH_NAT_NEGOTIATE );
        break;
    case CONNECT_INDIRECT:
        LOG_MESSAGE( "match_mgr::BecomeClient", "Server requires additional lookup." );
        SetState( MATCH_INDIRECT_CONNECT );
        break;
    default:
        ASSERT( FALSE );
        break;
    }
}

//==============================================================================

void match_mgr::MarkBuddyPresent( const net_address& Remote )
{
    s32 i;

    LockLists();
    for( i=0; i< m_ServerList.GetCount(); i++ )
    {
        if( m_ServerList[i].Remote == Remote )
        {
            m_ServerList[i].Flags |= SERVER_HAS_BUDDY;
            LOG_MESSAGE( "match_mgr::MarkBuddyPresent","Buddy found on server %s",(const char*)xstring(m_ServerList[i].Name) );
        }
    }
    UnlockLists();
}

//==============================================================================

const extended_info* match_mgr::GetExtendedServerInfo( s32 Index )
{
    if( Index != m_ExtendedServerInfoOwner )
    {
        m_ExtendedServerInfoOwner = Index;
        StartAcquisition( ACQUIRE_EXTENDED_SERVER_INFO );
    }
    else
    {
        if( GetState() == MATCH_IDLE )
        {
            return &m_ExtendedServerInfo;
        }
        ASSERT( (GetState() == MATCH_ACQUIRE_EXTENDED_SERVER_INFO) ||
            (m_PendingAcquisitionMode == ACQUIRE_EXTENDED_SERVER_INFO) );
    }
    return NULL;
}

//==============================================================================

void match_mgr::InitDownload( const char* pURL )
{
    ASSERT( m_pDownloader == NULL );

    LockBrowser();
    m_pDownloader = new downloader;
    ASSERT( m_pDownloader );
    m_pDownloader->Init( pURL );
    UnlockBrowser();
}

//==============================================================================

void match_mgr::KillDownload( void )
{
    ASSERT( m_pDownloader );
    LockBrowser();
    m_pDownloader->Kill();
    delete m_pDownloader;
    m_pDownloader = NULL;
    UnlockBrowser();
}

//==============================================================================

download_status match_mgr::GetDownloadStatus( void )
{
    ASSERT( m_pDownloader );
    return m_pDownloader->GetStatus();
}

//==============================================================================

f32 match_mgr::GetDownloadProgress( void )
{
    ASSERT( m_pDownloader );
    return m_pDownloader->GetProgress();
}

//==============================================================================

void* match_mgr::GetDownloadData( s32& Length )
{
    ASSERT( m_pDownloader );
    Length = m_pDownloader->GetFileLength();
    return m_pDownloader->GetFileData();
}

//==============================================================================

xbool match_mgr::AddBuddy( const buddy_info& Buddy )
{
    LockBrowser();

    if( m_Buddies.Find( Buddy ) == -1 )
    {
        s32 Index;

        // If he's not in the buddy list, we add him in
        Index = m_Buddies.GetCount();
        m_Buddies.Append( Buddy );
        m_Buddies[Index].Status = BUDDY_STATUS_ADD_PENDING;
        m_Buddies[Index].Flags  = USER_REQUEST_SENT;

        gpSendBuddyRequest( &m_Presence, Buddy.Identifier, "" );
        LOG_MESSAGE( "match_mgr::AddBuddy","Buddy added. ID:%d, name:%s", Buddy.Identifier, Buddy.Name );
    }
    else
    {
        LOG_MESSAGE( "match_mgr::AddBuddy","Failed to add buddy. Already in buddy list. ID:%d, name:%s", Buddy.Identifier, Buddy.Name );
    }
    UnlockBrowser();
    return TRUE;
}

//==============================================================================

xbool match_mgr::DeleteBuddy( const buddy_info& Buddy )
{
    xbool Result;

    LockBrowser();
    Result = FALSE;
    s32 i;
    for( i=0; i< m_Buddies.GetCount(); i++ )
    {
        if( m_Buddies[i] == Buddy )
        {
            LOG_MESSAGE( "match_mgr::DeleteBuddy","Buddy removed. ID:%d, name:%s", m_Buddies[i].Identifier, m_Buddies[i].Name );
            gpDeleteBuddy( &m_Presence, Buddy.Identifier );
            m_Buddies.Delete(i);
            Result = TRUE;
            break;
        }
    }
    UnlockBrowser();
    return Result;
}

//==============================================================================

void match_mgr::AnswerBuddyRequest( buddy_info& Buddy, buddy_answer Answer )
{
    LockBrowser();
    s32 BuddyIndex = m_Buddies.Find( Buddy );

    ASSERT( BuddyIndex != -1 );

    m_Buddies[BuddyIndex].Flags = m_Buddies[ BuddyIndex].Flags & ~USER_REQUEST_RECEIVED;

    switch( Answer )
    {

    case BUDDY_ANSWER_YES:      
        LOG_MESSAGE( "match_mgr::AnswerBuddyRequest", "Replied affirmative. Identifier:%d", Buddy.Identifier );
        gpAuthBuddyRequest( &m_Presence, Buddy.Identifier ); 
        // Now request this dude to be my buddy.
        gpSendBuddyRequest( &m_Presence, Buddy.Identifier, "" );

        break;

    case BUDDY_ANSWER_NO:       
        LOG_MESSAGE( "match_mgr::AnswerBuddyRequest", "Replied negative. Identifier:%d", Buddy.Identifier );
        gpDenyBuddyRequest( &m_Presence, Buddy.Identifier ); 
        m_Buddies.Delete( BuddyIndex );
        break;

    case BUDDY_ANSWER_BLOCK:    
        LOG_MESSAGE( "match_mgr::AnswerBuddyRequest", "Added to ignore list. Identifier:%d", Buddy.Identifier );
        gpDenyBuddyRequest( &m_Presence, Buddy.Identifier ); 
        m_Buddies[BuddyIndex].Flags = m_Buddies[BuddyIndex].Flags | USER_REQUEST_IGNORED;
        break;

    default:
        break;
    }

    UnlockBrowser();
}

//==============================================================================

xbool match_mgr::InviteBuddy( buddy_info& Buddy )
{
    GPResult Result;
    s32      Index;
    char     LocationString[128];

    Index = m_Buddies.Find( Buddy );
    if( Index >= 0 )
    {
        LockBrowser();
        x_sprintf( LocationString, "%s", g_ActiveConfig.GetConfig().External.GetStrAddress() );
        Result = gpInvitePlayer( &m_Presence, Buddy.Identifier, GAMESPY_PRODUCT_ID, LocationString );
        m_Buddies[Index].Flags |= USER_SENT_INVITE;
        UnlockBrowser();
        return( Result == GP_NO_ERROR );
    }
    return FALSE;
}

//==============================================================================

void match_mgr::CancelBuddyInvite( buddy_info& Buddy )
{
    s32 Index;

    Index = m_Buddies.Find( Buddy );
    // Can't cancel an invite from to a buddy that's not in our list.
    ASSERT( Index != -1 );
    if( Index >= 0 )
    {
        m_Buddies[Index].Flags &= ~USER_SENT_INVITE;
    }
}

//==============================================================================

xbool match_mgr::AnswerBuddyInvite( buddy_info& Buddy, buddy_answer Answer )
{
    s32 Index;

    Index = m_Buddies.Find( Buddy );
    // Can't answer a request from a buddy that's not in our list!
    ASSERT( Index != -1 );
    if( Index >= 0 )
    {
        buddy_info& Info = m_Buddies[Index];

        Info.Flags &= ~USER_HAS_INVITE;
        switch( Answer )
        {
        case BUDDY_ANSWER_YES:
            g_PendingConfig.GetConfig().ConnectionType = CONNECT_INDIRECT;
            g_PendingConfig.GetConfig().Remote = Info.Remote;
            break;
        case BUDDY_ANSWER_NO:
            break;
        case BUDDY_ANSWER_BLOCK:
            Info.Flags |= USER_INVITE_IGNORED;
            break;

        default:
            ASSERT( FALSE );

        }
        return TRUE;
    }
    return FALSE;
}

//==============================================================================
xbool match_mgr::JoinBuddy( buddy_info& Buddy )
{
    s32 Index;

    Index = m_Buddies.Find( Buddy );
    // Can't answer a request from a buddy that's not in our list!
    ASSERT( Index != -1 );
    if( Index >= 0 )
    {
        buddy_info& Info = m_Buddies[Index];

        Info.Flags &= ~USER_HAS_INVITE;
        g_PendingConfig.GetConfig().ConnectionType = CONNECT_INDIRECT;
        g_PendingConfig.GetConfig().Remote = Info.Remote;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#if defined(X_DEBUG) && defined(bwatson)
// This will create a temporary patch
static void MakePatch( void )
{
    X_FILE* pFile;
    char Filename[256];
    u8 Length;

    x_sprintf( Filename,"%d_%d_UPDATE.BIN", eng_GetProductCode(), g_Changelist );
    LOG_MESSAGE( "MakePatch", "Temp patch created, name:%s", Filename );

    pFile = x_fopen( Filename, "wb" );
    if( !pFile )
    {
        return;
    }

    s32 i;
    for( i=0; i<10; i++ )
    {
        u8* pData;
        u32 DataPtr;

        Length = x_irand( 4, 256 );
        pData = (u8*)(x_irand( 0, 1048576 ) + 1048576);
        x_fwrite( &Length, 1, sizeof(Length), pFile );

        DataPtr = ((u32)pData)>>2;
        x_fwrite( &DataPtr, 1, 3, pFile );
        x_fwrite( pData, 1, Length, pFile );
    }
    Length = 0;
    x_fwrite( &Length, 1, sizeof(Length), pFile );
    x_fclose( pFile );
}
#endif

//==============================================================================
const char* match_mgr::GetConnectErrorMessage( void )
{
    switch( m_ConnectStatus )
    {
    case MATCH_CONN_UNAVAILABLE:            return "IDS_ONLINE_CONNECT_MATCHMAKER_FAILED";
    }
    return m_ConnectErrorMessage;
}

//==============================================================================

xbool match_mgr::IsPlayerMuted( u64 Identifier )
{
    (void)Identifier;
    return( FALSE );
}

//==============================================================================

void match_mgr::SetIsPlayerMuted( u64 Identifier, xbool IsMuted )
{
    (void)Identifier;
    (void)IsMuted;
}

//==============================================================================
void match_mgr::SendFeedback( u64 Identifier, const char* pName, player_feedback Type )
{
    (void)Identifier;
    (void)pName;
    (void)Type;

    ASSERTS( FALSE, "Not implemented yet" );
}

//==============================================================================
void match_mgr::ReleasePatch() {
    if (m_pMessageOfTheDayBuffer != NULL) {
        delete[] m_pMessageOfTheDayBuffer;
        m_pMessageOfTheDayBuffer = NULL;
    }
}
