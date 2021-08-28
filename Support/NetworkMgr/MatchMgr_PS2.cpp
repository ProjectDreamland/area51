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

#if !defined(TARGET_PS2)
#error This should only be included for PS2 gamespy support.
#endif
#include "x_files.hpp"
#include "x_threads.hpp"
#include "e_Network.hpp"
#include "NetworkMgr/MatchMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/GameSpy/callbacks.hpp"
#include "Network/ps2/dnas.hpp"
#include "StateMgr/StateMgr.hpp"
#include "ServerVersion.hpp"
#include "Configuration/GameConfig.hpp"
#include "x_log.hpp"
#include "../Apps/GameApp/Config.hpp"
#include "IOManager/Device_DVD/io_device_cache.hpp"
#include "libscf.h"
#include "Parsing/textin.hpp"

const s32 A51_MIDWAY_FPS_CATEGORY = 1010000032;

#include "NetworkMgr/GameSpy/serverbrowsing/sb_serverbrowsing.h"
#include "NetworkMgr/GameSpy/available.h"
#include "NetworkMgr/GameSpy/qr2/qr2regkeys.h"
#include "NetworkMgr/GameSpy/qr2/qr2.h"
#include "NetworkMgr/GameSpy/GStats/gstats.h"
#include "NetworkMgr/GameSpy/GStats/gpersist.h"
#include "ResourceMgr/ResourceMgr.hpp"


//=========================================================================
// How patching works.
// 1. DNAS authentication is completed
// 2. Message of the day file is fetched, This file is called <lang>_Message.txt
//    for example, ENG_Message.txt
// 3. First line of MOTD file contains the patch version number
// 4. If the patch version number is greater than the local patch
//    then start downloading the new patch. Otherwise go to #7.
// 5. Once download it complete, mark online settings as dirty so the patch
//    will get saved on next settings save.
// 6. Encrypt the patch and store it in the patch buffer
// 7. Decrypt the patch in the patch buffer to another buffer then apply the
//    patch.
// 8. Unload the DNAS overlay.
//
// The MOTD file take the form:
//  <version#>
//  <motd-text>
// The patches are named as <product_code>_<changelist>_UPDATE.BIN, for example, 
// 20595_83119_UPDATE.BIN, and are stored on the server pointed to by 
// MANIFEST_LOCATION (currently http://216.201.187.178/DownloadTest)
//
//=========================================================================
// How patching works.
// 1. DNAS authentication is completed
// 2. Message of the day file is fetched, This file is called <lang>_Message.txt
//    for example, ENG_Message.txt
// 3. First line of MOTD file contains the patch version number
// 4. If the patch version number is greater than the local patch
//    then start downloading the new patch. Otherwise go to #7.
// 5. Once download it complete, mark online settings as dirty so the patch
//    will get saved on next settings save.
// 6. Encrypt the patch and store it in the patch buffer
// 7. Decrypt the patch in the patch buffer to another buffer then apply the
//    patch.
// 8. Unload the DNAS overlay.
//
// The MOTD file take the form:
//  <version#>
//  <motd-text>
// The patches are named as <product_code>_<changelist>_UPDATE.BIN, for example, 
// 20595_83119_UPDATE.BIN, and are stored on the server pointed to by 
// MANIFEST_LOCATION (currently http://216.201.187.178/DownloadTest)
//

void PersAuthCallback( int localid, int profileid, int authenticated, gsi_char *errmsg, void *instance);
void PersDataWriteCallback( int localid, int profileid, persisttype_t type, int index, int success, time_t modified, void *instance);
void PersDataReadCallback( int localid, int profileid, persisttype_t type, int index, int success, time_t modified, char *data, int len, void *instance);

void PersAuthCallback(int localid, int profileid, int authenticated, gsi_char *errmsg, void *instance)
{
    LOG_MESSAGE( "PersAuthCallback", "Auth callback: localid: %d profileid: %d auth: %d err: %s", localid, profileid, authenticated, errmsg );
    /**********
    instance is a pointer to the authcount var. We increment it here to tell the main loop that
    another authentication response came in.
    **********/
    (*(int *)instance)++;
    (void)localid;
    (void)profileid;
    (void)authenticated;
    (void)errmsg;
}

void PersDataWriteCallback(int localid, int profileid, persisttype_t type, int index, int success, time_t modified, void *instance)
{
    LOG_MESSAGE( "PersDataWriteCallback","Data write callback: localid: %d profileid: %d success: %d mod: %d", localid, profileid, success, (int)modified);
    /*********
    instance is a pointer to the callback counter, increment it
    **********/
    (*(int *)instance)++;

    (void)localid;
    (void)profileid;
    (void)index;
    (void)modified;
    (void)type;
    (void)success;
}

void PersDataReadCallback(int localid, int profileid, persisttype_t type, int index, int success, time_t modified, char *data, int len, void *instance)
{
    /* we copy it off, instead of reading directly, since the data may not be aligned correctly for the SH4/other processors */
    x_memcpy(&g_MatchMgr.m_CareerStats, data, sizeof(g_MatchMgr.m_CareerStats));

    if (success)
    {
        LOG_MESSAGE( "PersDataReadCallback", "Data read callback SUCCESS!" );

        // Lets make sure the crc is correct.
        s32 CheckSum = x_chksum( &g_MatchMgr.m_CareerStats.Stats, sizeof(g_MatchMgr.m_CareerStats.Stats) );
        if( CheckSum != g_MatchMgr.m_CareerStats.Checksum )
        {
            LOG_MESSAGE( "PersDataReadCallback", "CRC FAILURE!" );
            x_memset( &g_MatchMgr.m_CareerStats, 0, sizeof(g_MatchMgr.m_CareerStats) );
        }
    }
    else
    {
        LOG_MESSAGE( "PersDataReadCallback", "Data read callback FAILURE!" );
        x_memset( &g_MatchMgr.m_CareerStats, 0, sizeof(g_MatchMgr.m_CareerStats) );
    }

    LOG_MESSAGE( "PersDataReadCallback",
                 "PlayTime: %d - Games:%d", 
                 g_MatchMgr.m_CareerStats.Stats.PlayTime, 
                 g_MatchMgr.m_CareerStats.Stats.Games );

    /*********
    instance is a pointer to the callback counter, increment it
    **********/
    (*(int *)instance)++;

    GSI_UNUSED(type);
    GSI_UNUSED(index);
    (void)localid;
    (void)profileid;
    (void)len;
    (void)modified;
}

//=========================================================================
//  Defines
//=========================================================================
//#define ENABLE_LAN_LOOKUP

#define LABEL_STRING(x) case x: return ( #x );
// Authentication data filename. This is short for obfuscation reason.

const s32   GAMESPY_PRODUCT_ID  = 10384;
const char* GAMESPY_GAMENAME    = "area51ps2";
const char* GAMESPY_SECRETKEY   = "eR48fP";
const char* EMAIL_POSTFIX       = "a52";

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

#if !defined(ENABLE_LAN_LOOKUP)
static unsigned char SLUS_20595_pass_phrase[] = { 0xb9, 0xfd, 0xb2, 0xce, 0x5c, 0xd9, 0x0e, 0x0c };
static unsigned char SLES_52570_pass_phrase[] = { 0x25, 0x57, 0x94, 0xb3, 0x15, 0x92, 0xfd, 0x1f };
static unsigned char SLES_53075_pass_phrase[] = { 0x14, 0x27, 0x38, 0x99, 0xf8, 0xfb, 0xf7, 0x2d };
#endif

static byte* s_pAuthData;
#if defined(X_DEBUG) && defined(bwatson)
// This will create a temporary patch
static void MakePatch( void );
#endif


//=========================================================================
//  Data
//=========================================================================
dnas_authenticate g_Dnas;

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
    xthread* pThread = x_GetCurrentThread();

    deltatime.Start();

    while( pThread->IsActive() )
    {
        if( MatchMgr.m_ForceShutdown )
        {
            ASSERT( pThread->IsActive() );
            MatchMgr.m_ForceShutdown = FALSE;
            break;
        }
        MatchMgr.Update( deltatime.TripSec() );
        ASSERT( (MatchMgr.m_UpdateInterval>0) && (MatchMgr.m_UpdateInterval<100) );
        x_DelayThread( MatchMgr.m_UpdateInterval );
    }
}

//------------------------------------------------------------------------------
xbool match_mgr::Init( net_socket& Local, const net_address Broadcast )
{
    (void)PRELOAD_PS2_FILE("20595.rel");    (void)PRELOAD_PS2_FILE("20595.dat");
    (void)PRELOAD_PS2_FILE("52570.rel");    (void)PRELOAD_PS2_FILE("52570.dat");
    (void)PRELOAD_PS2_FILE("53075.rel");    (void)PRELOAD_PS2_FILE("53075.dat");
    f32 Timeout;

    ASSERT( !m_Initialized );
    ASSERT( AreListsLocked() == FALSE );
    ASSERT( IsBrowserLocked() == FALSE );
    LockLists();
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
    m_AuthStatus                = AUTH_STAT_DISCONNECTED;
    m_UserStatus                = BUDDY_STATUS_OFFLINE;
    m_PendingUserStatus         = BUDDY_STATUS_OFFLINE;
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
    SetState(MATCH_IDLE);

    xtimer Delta;

    Delta.Start();
    Timeout = 0.0f;
    m_ForceShutdown = FALSE;
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
        m_Initialized = FALSE;
        ASSERT( AreListsLocked() == FALSE );
        g_MatchMgr.SetState( MATCH_IDLE );
        m_ForceShutdown = TRUE;
        xtimer t;
        t.Start();
        while( m_ForceShutdown && m_pThread->IsActive() )
        {
            x_DelayThread( 2 );
            if( t.ReadMs() > 3000.0f )
            {
                ASSERT( FALSE );
                break;
            }
        }
        t.Stop();
        LOG_MESSAGE( "match_mgr::Kill", "MatchMgr took %2.02fms to shutdown.", t.ReadMs() );
        delete m_pThread;
        m_pThread = NULL;

        LockLists();
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
        UnlockLists();
        m_RecentPlayers.Clear();

        ResetServerList();

        m_Buddies.Clear();
        m_RecentPlayers.Clear();
        m_MessageOfTheDay.Clear();
        if( m_DNASRunning )
        {
            g_Dnas.Kill();
            m_DNASRunning = FALSE;
            ASSERT( s_pAuthData );
            x_free( s_pAuthData );
            s_pAuthData = NULL;
        }
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
        case ACQUIRE_SERVERS:
#if defined( ENABLE_LAN_LOOKUP )
            SetState( MATCH_ACQUIRE_LAN_SERVERS );
#else
            SetState( MATCH_ACQUIRE_SERVERS );
#endif
            break;
        case ACQUIRE_EXTENDED_SERVER_INFO:
            SetState( MATCH_ACQUIRE_EXTENDED_INFO );
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
    case AUTH_STAT_CONNECTED:

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
                x_memset( TempString, 0, sizeof(TempString) );
            }
            gpSetStatus( &m_Presence, Status, "", TempString );
            m_UserStatus = m_PendingUserStatus;
            LOG_MESSAGE( "match_mgr::Update", "User location reported as %s", TempString );
        }
    case AUTH_STAT_CONNECTING:
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
    StatsUpdate();
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
        if( m_DNASRunning )
        {
            s32     EncryptedLength=0;
            void*   pPatch;
            s32     Version;
            global_settings& Settings = g_StateMgr.GetActiveSettings();

            pPatch = Settings.GetPatchData( EncryptedLength, Version );

            if( pPatch && EncryptedLength )
            {
                xbool   ok;
                s32     DecryptedLength;
                byte*   pBuffer;

                g_Dnas.InitEncryption();
                DecryptedLength = g_Dnas.GetDecryptedLength( pPatch, EncryptedLength );
                // We will be decrypting in-place so we need to make sure that the decrypted data
                // will be shorter than the encrypted data. No nasty memory overruns please!
                ASSERT( EncryptedLength >= DecryptedLength );

                pBuffer = (byte*)x_malloc( EncryptedLength );
                ASSERT( pBuffer );
                x_memcpy( pBuffer, pPatch, EncryptedLength );
                ok = g_Dnas.Decrypt( pBuffer, EncryptedLength, DecryptedLength );
                if( ok )
                {
                    LOG_MESSAGE( "match_mgr::UpdateState", "Applying patch after decryption." );
                    ApplyPatch( pBuffer, DecryptedLength );
                }
                else
                {
                    LOG_WARNING( "match_mgr::UpdateState", "Patch did not decrypt properly." );
                }
                x_free( pBuffer );
            }
            g_Dnas.KillEncryption();
            g_Dnas.Kill();
            m_DNASRunning = FALSE;
            ASSERT( s_pAuthData );
            x_free( s_pAuthData );
            s_pAuthData = NULL;
        }
        break;

    //-----------------------------------------------------
    case MATCH_AUTHENTICATE_MACHINE:
#if defined(ENABLE_LAN_LOOKUP)
        SetAuthStatus( AUTH_STAT_CONNECTED );
        SetState( MATCH_IDLE );
#else
        s32     Status;
        s32     Progress;

        Status = g_Dnas.Update( DeltaTime, Progress );
        //
        // If it's still working, just let it do it's thing.
        //
        if( Status == DNAS_STATUS_BUSY )
            break;

        if( Status == DNAS_STATUS_OK )
        {
            s32 DnasIdLength;
            g_Dnas.GetUniqueId( m_UniqueId, DnasIdLength );

            SetAuthStatus( AUTH_STAT_CONNECTED );
            SetState( MATCH_GET_MESSAGES );
            g_Dnas.KillAuthentication();
            break;
        }

        //
        // Still in progress but with some progression information, may be downloading?
        //
        if( Status > 0 )
        {
            break;
        }

        // Some error occurred during DNAS validation. So, get the identifier of the text
        // to display and store it off somewhere prior to killing DNAS.
        SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
        m_ConnectErrorCode = Status;

        x_strcpy( m_ConnectErrorMessage, g_Dnas.GetErrorLabel( m_ConnectErrorCode ) );
        g_Dnas.KillAuthentication();
        SetState( MATCH_IDLE );
#endif

        break;
    //-----------------------------------------------------
    case MATCH_AUTH_DONE:
        break;

    //-----------------------------------------------------
    case MATCH_CONNECT_MATCHMAKER:
        if( m_StartedConnect == FALSE )
        {
            GSIStartAvailableCheck( GAMESPY_GAMENAME );
            m_StartedConnect = TRUE;
            break;
        }

        {
            GSIACResult Result;

            Result = GSIAvailableCheckThink();
            if( Result != GSIACWaiting )
            {
                if( Result != GSIACAvailable )
                {
                    SetState( MATCH_IDLE );
                    SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
                }
                else
                {
#if defined(ENABLE_LAN_LOOKUP)
                    SetAuthStatus( AUTH_STAT_CONNECTED );
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
        case AUTH_STAT_CONNECTING:
            break;
        case AUTH_STAT_CONNECTED:
            LOG_MESSAGE("match_mgr::UpdateState","AUTH_AUTHENTICATE_USER: Email address verified ok");
            SetState( MATCH_AUTH_USER_CONNECT );
            break;
        case AUTH_STAT_NO_ACCOUNT:
            LOG_MESSAGE("match_mgr::UpdateState","AUTH_AUTHENTICATE_USER: No such email account");
            SetState( MATCH_AUTH_USER_CREATE );
            break;
        case AUTH_STAT_CANNOT_CONNECT:
            LOG_MESSAGE("match_mgr::UpdateState","AUTH_AUTHENTICATE_USER: Authentication failed");
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
        case AUTH_STAT_CONNECTING:
            break;
        case AUTH_STAT_CONNECTED:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CONNECT: User account connected ok");
            SetState( MATCH_SECURITY_CHECK );
            m_UserStatus = BUDDY_STATUS_OFFLINE;        //*** HACK HACK - will force a full update of user status to gamespy
            SetUserStatus( BUDDY_STATUS_ONLINE );
            break;
        case AUTH_STAT_NO_ACCOUNT:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CONNECT: Account does not exist");
            SetState( MATCH_AUTH_USER_CREATE );
            break;
        case AUTH_STAT_CANNOT_CONNECT:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CONNECT: Authentication failed");
            SetState( MATCH_IDLE );
            break;
        default:
            break;
        }
        break;

        //------------------------------------------------------
    case MATCH_AUTH_USER_CREATE:
        switch( GetAuthStatus() )
        {
        case AUTH_STAT_CONNECTING:
            break;
        case AUTH_STAT_CONNECTED:
            SetState( MATCH_SECURITY_CHECK );
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CREATE: User account created ok");
            m_UserStatus = BUDDY_STATUS_OFFLINE;        //*** HACK HACK - will force a full update of user status to gamespy
            SetUserStatus( BUDDY_STATUS_ONLINE );
            break;
        case AUTH_STAT_NO_ACCOUNT:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CREATE: User account creation failed");
            SetState( MATCH_IDLE );
            break;
        case AUTH_STAT_CANNOT_CONNECT:
            LOG_MESSAGE("match_mgr::UpdateState","MATCH_AUTH_USER_CREATE: Authentication failed");
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
                    SetAuthStatus( AUTH_STAT_CONNECTED );
                    // Now enable the stats connection.
                    EnableStatsConnection();
                }
                else
                {
                    SetAuthStatus( AUTH_STAT_SECURITY_FAILED );
                }
                m_pSecurityChallenge = NULL;
            }
            else
            {
                SetAuthStatus( AUTH_STAT_CONNECTED );
                // Now enable the stats connection.
                EnableStatsConnection();
            }
            SetState( MATCH_IDLE );
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
                    ParseMessageOfTheDay( pData );
                }
            }
            break;
        case DL_STAT_ERROR:
        case DL_STAT_NOT_FOUND:
            m_MessageOfTheDayReceived = TRUE;
            m_MessageOfTheDay.Clear();
            LOG_MESSAGE( "match_mgr::UpdateState", "There is no message of the day" );
            break;
        default:
            ASSERT( FALSE );
            break;
        }
        if( m_MessageOfTheDayReceived )
        {
            KillDownload();
            global_settings& Settings = g_StateMgr.GetActiveSettings();
            s32 Length;
            Settings.GetPatchData( Length, m_LocalPatchVersion );

            if( m_RemotePatchVersion > m_LocalPatchVersion )
            {
                SetState( MATCH_GET_PATCH );
            }
            else
            {
                SetState( MATCH_AUTH_DONE );
            }
        }
        break;

        //-----------------------------------------------------
    case MATCH_GET_PATCH:
        switch( GetDownloadStatus() )
        {
        case DL_STAT_BUSY:
            break;
        case DL_STAT_OK:
            {
                byte*                       pDecryptedData;
                byte*                       pEncryptionBuffer;
                s32                         DecryptedLength;
                s32                         EncryptedLength;
                global_settings& Settings = g_StateMgr.GetActiveSettings();

                g_Dnas.InitEncryption();
                pDecryptedData = (u8*)GetDownloadData( DecryptedLength );
                LOG_MESSAGE( "match_mgr::UpdateState", "Patch version:%d received.", m_RemotePatchVersion );
                // This will do an internal allocation and copy the patch data to there. We want this done with the
                // encrypted size as this will be what is saved on to the memory card.

                EncryptedLength = g_Dnas.GetEncryptedLength( pDecryptedData, DecryptedLength );
                if( EncryptedLength > 0 )
                {
                    s32 BufferLength = MAX( EncryptedLength, DecryptedLength );
                    pEncryptionBuffer = (byte*)x_malloc( BufferLength );

                    x_memcpy( pEncryptionBuffer, pDecryptedData, DecryptedLength );

                    if( EncryptedLength > 0 )
                    {
                        g_Dnas.Encrypt( pEncryptionBuffer, DecryptedLength, EncryptedLength );
                        ASSERT( (EncryptedLength >= 0) && (EncryptedLength<NET_MAX_PATCH_SIZE) );
                        Settings.SetPatchData( pEncryptionBuffer, EncryptedLength, m_RemotePatchVersion );
                    }
                    x_free( pEncryptionBuffer );
                }

                KillDownload();
                g_Dnas.KillEncryption();
                SetState( MATCH_IDLE );
            }
            break;
        case DL_STAT_ERROR:
        case DL_STAT_NOT_FOUND:
            LOG_MESSAGE( "match_mgr::UpdateState", "There is no patch available" );
            KillDownload();
            SetState( MATCH_IDLE );
            break;
        default:
            ASSERT( FALSE );
            KillDownload();
            SetState( MATCH_IDLE );
            break;
        }
        break;
    //-----------------------------------------------------
    case MATCH_ACQUIRE_SERVERS:
        // 
        // Get server list from matchmaker.
        //
        LOG_MESSAGE("match_mgr::UpdateState", "Match Acquire Servers" );
        if( m_PendingAcquisitionMode != ACQUIRE_INVALID )
        {
            SetState( MATCH_IDLE );
        }
        else if( m_ConnectStatus == MATCH_CONN_IDLE )
        {
            SetState( MATCH_ACQUIRE_IDLE );
        }
        else if( m_StateTimeout <= 0.0f )
        {
            LOG_WARNING("match_mgr::UpdateState","Server browser did not go idle and timed out.");
            SetState( MATCH_IDLE );
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
            LockBrowser();
            ASSERT( m_pBrowser );
            ServerBrowserFree( m_pBrowser );
            m_pBrowser = NULL;
            // We have some additional server information. We now wait until the main 
            // thread actually advances the connect sequence.
            SetState( MATCH_IDLE );
            UnlockBrowser();
        }
        break;
        //-----------------------------------------------------
    case MATCH_ACQUIRE_EXTENDED_INFO:
        break;

        //-----------------------------------------------------
    case MATCH_ACQUIRE_IDLE:
        LocalLookups( DeltaTime );
        RemoteLookups( DeltaTime );
        // **Note** The AppendServer function will reset the state timeout because a new server was added to the list.
        // this is only set to 1 second so if we don't get any more within the next second, we go totally idle.
        if( HasTimedOut() )
        {
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
        LOG_MESSAGE("match_mgr::UpdateState", "Become Server" );
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

        // GameSpy does not inform us that a registration attempt has completed. Only if it failed.
        // So, we always give it 5 seconds to complete.
        if( m_StateTimeout < 0.0f )
        {
            SetState( MATCH_SERVER_ACTIVE );
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
            LOG_MESSAGE("match_mgr::UpdateState", "Server unregistered" );
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
            SetConnectStatus( MATCH_CONN_CONNECTED );
            SetState( MATCH_CLIENT_ACTIVE );
        }
        else if( g_ActiveConfig.GetExitReason() != GAME_EXIT_CONTINUE )
        {
            SetConnectStatus( MATCH_CONN_DISCONNECTED );
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
            char UniqueId[32];
            x_sprintf( UniqueId, "%08x", net_GetSystemId() );
            SetUniqueId( (byte*)UniqueId, x_strlen(UniqueId) );
            SetAuthStatus( AUTH_STAT_DISCONNECTED );
#if !defined(ENABLE_LAN_LOOKUP)
            dnas_init   Init;
            X_FILE*     pAuthFile;
            s32         AuthLength;

            pAuthFile = x_fopen( xfs("%s/%d.dat", g_RscMgr.GetRootDirectory(), eng_GetProductCode()), "rb" );
            ASSERT( pAuthFile );
            AuthLength = x_flength(pAuthFile);

            s_pAuthData = (byte*)x_malloc( AuthLength );
            ASSERT(s_pAuthData);
            x_fread(s_pAuthData, AuthLength, 1, pAuthFile );
            x_fclose(pAuthFile);

            switch( eng_GetProductCode() )
            {
            case 20595:
                Init.pPassPhrase        = SLUS_20595_pass_phrase;
                Init.PassPhraseLength   = sizeof(SLUS_20595_pass_phrase);
                break;
            case 52570:
                Init.pPassPhrase        = SLES_52570_pass_phrase;
                Init.PassPhraseLength   = sizeof(SLES_52570_pass_phrase);
                break;
            case 53075:
                Init.pPassPhrase        = SLES_53075_pass_phrase;
                Init.PassPhraseLength   = sizeof(SLES_53075_pass_phrase);
                break;
            default:
                Init.pPassPhrase        = SLUS_20595_pass_phrase;
                Init.PassPhraseLength   = sizeof(SLUS_20595_pass_phrase);
                break;
            }

            Init.pAuthData          = s_pAuthData;
            Init.AuthLength         = AuthLength;
            Init.Category           = A51_MIDWAY_FPS_CATEGORY;
            ASSERT( Init.pPassPhrase );
            g_Dnas.Init( xfs("%s/%d.rel",g_RscMgr.GetRootDirectory(), eng_GetProductCode() ) );
            g_Dnas.InitAuthentication( &Init );
            m_DNASRunning = TRUE;
#endif
        }
        break;

        //------------------------------------------------------
    case MATCH_CONNECT_MATCHMAKER:
        m_StartedConnect = FALSE;
        break;

        //-----------------------------------------------------
    case MATCH_AUTHENTICATE_USER:
#if defined(ENABLE_LAN_LOOKUP)
        SetAuthStatus( AUTH_STAT_CONNECTED );
#else
        {
            // Start user authentication check
            Status = gpInitialize( &m_Presence, GAMESPY_PRODUCT_ID, 0 );
            ASSERT( Status == GP_NO_ERROR );
            if( Status != GP_NO_ERROR )
            {
                SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
                SetState( MATCH_IDLE );
            }
            //
            // Maybe should use different callback for presence error?
            //

            player_profile& Profile = g_StateMgr.GetActiveProfile(0);
            s32         ProfileIdLength;
            const byte* pUniqueId;

            pUniqueId = Profile.GetUniqueId( ProfileIdLength );
            if( ProfileIdLength )
            {
                SetUniqueId( pUniqueId, ProfileIdLength );
            }
            else
            {
                
                Profile.SetUniqueId( m_UniqueId, x_strlen((char*)m_UniqueId) );
            }

            SetAuthStatus( AUTH_STAT_CONNECTING );
            x_sprintf( m_DownloadFilename, "%s@%s", m_UniqueId, EMAIL_POSTFIX );
            Status = gpSetCallback(&m_Presence, GP_ERROR, (GPCallback)gamespy_error, this );
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
            if( Status != GP_NO_ERROR )
            {
                SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
                SetState( MATCH_IDLE );
            }
        }
#endif
        break;

        //-----------------------------------------------------
    case MATCH_AUTH_USER_CREATE:
        {

            ASSERT( x_strlen(m_Nickname) > 0 );
            // Make sure we copy a maximum of 29 characters for the password
            // field. This is a limitation of GameSpy.
            char Password[64];
            x_memset( Password, 0, sizeof(Password) );
            x_strncpy( Password, (const char*)m_UniqueId, 29 );
            SetAuthStatus( AUTH_STAT_CONNECTING );
            LOG_MESSAGE("match_mgr::EnterState","MATCH_AUTH_USER_CREATE: AccountName:%s, Nickname:%s",m_DownloadFilename, m_Nickname );
            Status = gpConnectNewUser( &m_Presence, 
                                        m_Nickname,                     // Nickname
                                        NULL,                           // Unique Nick
                                        m_DownloadFilename,             // Email address
                                        Password,                       // Password
                                        NULL,                           // CD Key
                                        GP_FIREWALL,                    // Is firewalled
                                        GP_NON_BLOCKING,                // Is a blocking call
                                        (GPCallback)gamespy_connect,    // Callback when done
                                        this );
            ASSERT( Status == GP_NO_ERROR );
            if( Status != GP_NO_ERROR )
            {
                SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
                SetState( MATCH_IDLE );
            }
        }
        break;

        //-----------------------------------------------------
    case MATCH_AUTH_USER_CONNECT:
        {
            ASSERT( x_strlen(m_Nickname) > 0 );
            SetAuthStatus( AUTH_STAT_CONNECTING );
            char Password[64];
            x_memset( Password, 0, sizeof(Password) );
            x_strncpy( Password, (const char*)m_UniqueId, 29 );

            LOG_MESSAGE("match_mgr::EnterState","MATCH_AUTH_USER_CONNECT: AccountName:%s, Nickname:%s",m_DownloadFilename, m_Nickname );
            Status = gpConnect(    &m_Presence, 
                                    m_Nickname,                         // Nickname - this will come from a profile
                                    m_DownloadFilename,                 // Email address
                                    Password,                           // Password
                                    GP_FIREWALL,                        // Is Firewalled
                                    GP_NON_BLOCKING,                    // Is Blocking
                                    (GPCallback)gamespy_connect,        // Completion callback
                                    this );
            if( Status != GP_NO_ERROR )
            {
                SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
                SetState( MATCH_IDLE );
            }
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
            if( Status != sbe_noerror )
            {
                SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
                SetState( MATCH_IDLE );
            }
            else
            {
                NNBeginNegotiationWithSocket( (SOCKET)m_pSocket, m_SessionID, 1, gamespy_nat_progress, gamespy_nat_complete, this );
            }
        }
        break;
        //-----------------------------------------------------
    case MATCH_LOGIN:
        // ** NOTE ** the negotiation callback will advance the state to either
        // MATCH_IDLE if there is an error, or MATCH_CLIENT_ACTIVE if negotiation completed.
        LOG_APP_NAME( "CLIENT" );
        // Make sure we're not trying to get any data
        g_NetworkMgr.BeginLogin();
        LockLists();
        ResetServerList();
        m_LobbyList.Clear();
        UnlockLists();
        m_LocalIsServer = FALSE;
        break;
    case MATCH_ACQUIRE_IDLE:
        SetTimeout(1.0f);
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
    case MATCH_GET_PATCH:
        x_sprintf(m_DownloadFilename,"%s/%d_%d_UPDATE.BIN",MANIFEST_LOCATION, eng_GetProductCode(), g_Changelist );
        InitDownload( m_DownloadFilename );
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
    case ACQUIRE_SERVERS:
        ResetServerList();
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
    case ACQUIRE_SERVERS:
        return (m_State==MATCH_ACQUIRE_IDLE)||(m_State==MATCH_IDLE);
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
            m_MessageOfTheDay.Clear();
            x_sprintf(m_DownloadFilename,"%s/%s_%d_Message.txt",MANIFEST_LOCATION, x_GetLocaleString(), g_Changelist );
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
                                            MUTATION_MODE_KEY,
                                            PASSWORD_SET_KEY,
                                            VOICE_KEY,                                            
                                            };
            int numFields = sizeof(basicFields) / sizeof(basicFields[0]);

            SBError Result;

            qr2_register_key( GAMETYPE_ID_KEY, "gametypeid" );
            qr2_register_key( LEVEL_KEY, "level" );
            qr2_register_key( PLAYERS_KEY, "players" );
            qr2_register_key( FLAGS_KEY, "svrflags" );      // Can't use FLAGS because it's taken by the internal state
            qr2_register_key( AVGPING_KEY, "avgping" );
            qr2_register_key( MUTATION_MODE_KEY, "mutation" );
            qr2_register_key( PASSWORD_SET_KEY, "passworden" );
            qr2_register_key( VOICE_KEY, "voice" );            

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

            if( m_FilterMutationMode != -1 )
            {
                if( !FirstClause ) 
                { 
                    x_strcat( Filter," and " );
                }
                FirstClause = FALSE;
                x_strcat( Filter, xfs("mutation=%d", m_FilterMutationMode) );
            }

            if( m_FilterPassword != -1 )
            {
                if( !FirstClause ) 
                { 
                    x_strcat( Filter," and " );
                }
                FirstClause = FALSE;
                x_strcat( Filter, xfs("passworden=%d", m_FilterPassword) );
            }

            if( m_FilterHeadset != -1 )
            {
                if( !FirstClause ) 
                { 
                    x_strcat( Filter," and " );
                }
                FirstClause = FALSE;
                x_strcat( Filter, xfs("voice=%d", m_FilterHeadset) );
            }

            Result = ServerBrowserUpdate(m_pBrowser, SBTrue, SBFalse, basicFields, numFields, Filter );
            LOG_MESSAGE("match_mgr::SetState","ServerBrowserUpdate() returned %d(%s)", Result, ServerBrowserError( Result ) );
            if( Result == sbe_noerror )
            {
                SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
            }
            else
            {
                // Something not good happened. Probably network down or gamespy not responding.
                g_ActiveConfig.SetExitReason( GAME_EXIT_NETWORK_DOWN );
                SetState( MATCH_IDLE );
            }

        }
        UnlockBrowser();
        break;

        //------------------------------------------------------
    case MATCH_ACQUIRE_EXTENDED_INFO:
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
                for( i=0; i< GetServerCount(); i++ )
                {
                    if( m_ExtendedServerInfoOwner == GetServerInfo( i )->ID )
                    {
                        break;
                    }
                }
                ASSERT( i!=GetServerCount() );
                const net_address& Remote = GetServerInfo( i )->Remote;
                Result = ServerBrowserAuxUpdateIP(  m_pBrowser,                  // Browser object
                                                    Remote.GetStrIP(),           // Browser IP address in string format
                                                    Remote.GetPort(),            // Browser port number
                                                    SBTrue,                      // viaMaster
                                                    SBTrue,                      // async
                                                    SBFalse                      // fullUpdate 
                    );
                LOG_MESSAGE("match_mgr::SetState","ServerBrowserAuxUpdateIP(%s) returned %d(%s)", Remote.GetStrAddress(), Result, ServerBrowserError( Result ) );
                SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
            }
            UnlockBrowser();
        }
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

            qr2_register_key( GAMETYPE_ID_KEY, "gametypeid" );
            qr2_register_key( LEVEL_KEY, "level" );
            qr2_register_key( PLAYERS_KEY, "players" );
            qr2_register_key( FLAGS_KEY, "svrflags" );      // Can't use FLAGS because it's taken by the internal state
            qr2_register_key( AVGPING_KEY, "avgping" );
            qr2_register_key( MUTATION_MODE_KEY, "mutation" );
            qr2_register_key( PASSWORD_SET_KEY, "passworden" );
            qr2_register_key( VOICE_KEY, "voice" );            

            Result = ServerBrowserLANUpdate( m_pBrowser, SBTrue, m_pSocket->GetPort(), m_pSocket->GetPort()+16 );
            LOG_MESSAGE("match_mgr::SetState","ServerBrowserLANUpdate() returned %d(%s)", Result, ServerBrowserError( Result ) );
            SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
        }
        else
        {
            SetConnectStatus( MATCH_CONN_IDLE );
        }
        UnlockBrowser();
#endif
        break;

        //------------------------------------------------------
    case MATCH_INDIRECT_LOOKUP:
        // Start a lookup to the matchmaking service about this particular server
        SetTimeout( LOOKUP_RETRY_INTERVAL );
        LockLists();
        ResetServerList();
        m_ExtendedServerInfoOwner = -1;
        m_LookupTimeout = 0.0f;
        UnlockLists();

        LockBrowser();
        SetConnectStatus( MATCH_CONN_ACQUIRING_SERVERS );
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
                                                SBTrue                              // fullUpdate 
                                              );
            LOG_MESSAGE("match_mgr::SetState","ServerBrowserAuxUpdateIP(%s) returned %d(%s)", Config.Remote.GetStrAddress(), Result, ServerBrowserError( Result ) );

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
            qr2_register_key( GAMETYPE_ID_KEY, "gametypeid" );
            qr2_register_key( LEVEL_KEY, "level" );
            qr2_register_key( PLAYERS_KEY, "players" );
            qr2_register_key( FLAGS_KEY, "svrflags" );      // Can't use FLAGS because it's taken by the internal state
            qr2_register_key( AVGPING_KEY, "avgping" );
            qr2_register_key( MUTATION_MODE_KEY, "mutation" );
            qr2_register_key( PASSWORD_SET_KEY, "passworden" );
            qr2_register_key( VOICE_KEY, "voice" );            

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
            if( QueryError!=e_qrnoerror )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_NETWORK_DOWN );
                SetConnectStatus( MATCH_CONN_DISCONNECTED );
                SetState( MATCH_IDLE );
                return;
            }
            SetTimeout( 5.0f );
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
        struct sockaddr sender;

        // Convert our internal addresses from host endian to network endian as the gamespy
        // libs require it to be network endian.
        sender.s_port = htons(Remote.GetPort());
        sender.s_addr = htonl(Remote.GetIP());
        LockBrowser();
        qr2_parse_query(NULL, (char*)Bitstream.GetDataPtr(), Bitstream.GetNBytes(), &sender);
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
    s32             Index;
    server_info*    pServerInfo=NULL;

    m_StateTimeout = 1.0f;

    if( Response.Version != g_ServerVersion )
    {
        return;
    }
    // Search the server list to see if we already have an entry. If we do, then
    // just update the individual fields, otherwise append this entry to the main
    // list.
    LockLists();

    Index = FindServerInfo( Response );
    if( Index >=0 )
    {
        s32 Flags;

        pServerInfo = GetServerInfo( Index );

        Flags = pServerInfo->Flags;
        x_memcpy( (void *)pServerInfo, &Response, sizeof(server_info) );
        pServerInfo->Flags = Flags;
    }
    else
    {
        AppendToServerList( Response );
        Index = FindServerInfo( Response );
        ASSERT( Index >= 0 );
        pServerInfo = GetServerInfo( Index );
    }

    ASSERT( pServerInfo );
    // Go through our local buddy list and see if any of those buddies are on this server
    for( Index=0; Index< m_Buddies.GetCount(); Index++ )
    {
        if( m_Buddies[Index].Remote==pServerInfo->Remote )
        {
            pServerInfo->Flags |= SERVER_HAS_BUDDY;
        }
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
    SetAuthStatus( AUTH_STAT_DISCONNECTED );
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
void match_mgr::BecomeClient( void )
{
    //** BW - NEED TO CHECK IF THIS IS RELEVANT ANYMORE. 
    ASSERT( FALSE );
    game_config::Commit();

    switch( g_PendingConfig.GetConfig().ConnectionType )
    {
    case CONNECT_NAT:
        LOG_MESSAGE("match_mgr::BecomeClient","Server is behind a firewall. Starting NAT negotiation.");
        SetState( MATCH_NAT_NEGOTIATE );
        break;
    case CONNECT_INDIRECT:
        LOG_MESSAGE( "match_mgr::BecomeClient", "Server requires additional lookup." );
        SetState( MATCH_INDIRECT_LOOKUP );
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

    for( i=0; i< GetServerCount(); i++ )
    {
        server_info* pServerInfo = GetServerInfo( i );
        if( pServerInfo->Remote == Remote )
        {
            pServerInfo->Flags |= SERVER_HAS_BUDDY;
            LOG_MESSAGE( "match_mgr::MarkBuddyPresent","Buddy found on server %s",(const char*)xstring(pServerInfo->Name) );
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
        ASSERT( (GetState() == MATCH_ACQUIRE_EXTENDED_INFO) ||
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
            gpRevokeBuddyAuthorization( &m_Presence, Buddy.Identifier );
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
        if( gpIsBuddy( &m_Presence, Buddy.Identifier )==FALSE )
        {
            LOG_MESSAGE( "match_mgr::AnswerBuddyRequest", "Sent implicit buddy request." );
            // Now request this dude to be my buddy.
            m_Buddies[BuddyIndex].Status = BUDDY_STATUS_ADD_PENDING;
            m_Buddies[BuddyIndex].Flags  = USER_REQUEST_SENT;
            gpSendBuddyRequest( &m_Presence, Buddy.Identifier, "" );
        }
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
    }
    return FALSE;
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


    u32 DataPtr;

    //
    // Dump out a little string patchup to the version string to show a patched build.
    //
    const char* pReplacementString="Patched Build";
    extern const char* g_pBuildDate;
    DataPtr = ((u32)g_pBuildDate)>>2;
    Length=x_strlen(pReplacementString)+1;

    x_fwrite( &Length, 1, 1, pFile );                   // Write Length byte
    x_fwrite( &DataPtr, 1, 3, pFile );                  // Write 3 byte address (shifted by 2)
    x_fwrite( pReplacementString, 1, Length, pFile );   // Actual patch
    
#if 0
    u8* pData;
    s32 i;
    for( i=0; i<20; i++ )
    {

        Length = x_irand( 4, 256 );
        pData = (u8*)(x_irand( 0, 1048576 ) + 1048576);
        x_fwrite( &Length, 1, sizeof(Length), pFile );

        DataPtr = ((u32)pData)>>2;
        x_fwrite( &DataPtr, 1, 3, pFile );
        x_fwrite( pData, 1, Length, pFile );
    }
#endif
    Length = 0;
    x_fwrite( &Length, 1, sizeof(Length), pFile );
    x_fclose( pFile );
}
#endif

//==============================================================================
const char* match_mgr::GetConnectErrorMessage( void )
{
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

char* s_pAuth = 0;
char* s_pChallenge = 0;
u64   s_PID = 0;

//==============================================================================

void match_mgr::InitiateStatsConnection( void )
{
    GSIACResult ac_result;

    // State is now disconnected...
    m_StatsState = GS_STATS_DISCONNECTED;

    // Nuke the career stats
    x_memset( &m_CareerStats, 0, sizeof(m_CareerStats) );
    
    // Init flags.
    m_ConnectCount   = 0;
    m_ReadCount      = 0;
    m_WriteCount     = 0;
    m_bIsReading     = FALSE;
    m_bReadComplete  = FALSE;
    m_bIsWriting     = FALSE;
    m_bWriteComplete = FALSE;

    // First step, set our game authentication info
    // We could do:
    //
    //    strcpy(gcd_gamename,"area51ps2");
    //    strcpy(gcd_secret_key,"eR48fP");
    //
    // ...but this is more secure:

    gcd_gamename[0]='a';
    gcd_gamename[1]='r';
    gcd_gamename[2]='e';
    gcd_gamename[3]='a';
    gcd_gamename[4]='5';
    gcd_gamename[5]='1';
    gcd_gamename[6]='p';
    gcd_gamename[7]='s';
    gcd_gamename[8]='2';
    gcd_gamename[9]='\0';

    gcd_secret_key[0]='e';
    gcd_secret_key[1]='R';
    gcd_secret_key[2]='4';
    gcd_secret_key[3]='8';
    gcd_secret_key[4]='f';
    gcd_secret_key[5]='P';
    gcd_secret_key[6]='\0';

    // Before initializing the connection, check that the backend is available.
    GSIStartAvailableCheck(_T(gcd_gamename));
    while((ac_result = GSIAvailableCheckThink()) == GSIACWaiting)
    {
        x_DelayThread(5);
        if( m_ForceShutdown )
            return;
    }
    if(ac_result == GSIACAvailable)
    {
        /*********
        Next, open the stats connection. This may block for
        a 1-2 seconds, so it should be done before the actual game starts.
        **********/
        int result = InitStatsAsync(0, 0);
        while( result == GE_CONNECTING )
        {
            result = InitStatsThink();
            x_DelayThread( 5 );
            if( m_ForceShutdown ) 
                return;
        }

        if (result == GE_NOERROR)
        {
            static char validate[33];
            /***********
            While the first authentication is in progress, we'll go ahead and start the second
            one, using a Presence & Messaging SDK profileid / password.
            To generate the new validation token, we'll need to pass in the password for the
            profile we are authenticating.
            Again, if this is done in a client/server setting, with the Persistent Storage
            access being done on the server, and the P&M SDK is used on the client, the
            server will need to send the challenge (GetChallenge(NULL)) to the client, the
            client will create the validation token using GenerateAuth, and send it
            back to the server for use in PreAuthenticatePlayerPM
            ***********/
            char Password[64];
            x_memset( Password, 0, sizeof(Password) );
            x_strncpy( Password, (const char*)m_UniqueId, 29 );

            s_PID = GetPlayerIdentifier(0);
            s_pChallenge = GetChallenge(NULL);
            s_pAuth = GenerateAuth(s_pChallenge,Password,validate);

            /************
            After we get the validation token, we pass it and the profileid of the user
            we are authenticating into PreAuthenticatePlayerPM.
            We pass the same authentication callback as for the first user, but a different
            localid this time.
            ************/
            PreAuthenticatePlayerPM(1,GetPlayerIdentifier(0),validate,PersAuthCallback,&m_ConnectCount);

            // We are now connecting...
            m_StatsState = GS_STATS_CONNECTING;
        }
    }
}

//==============================================================================

void match_mgr::InitiateCareerStatsRead( void )
{
    ASSERT( m_bIsReading == FALSE );
    m_ReadCount     = 0;
    m_bIsReading    = TRUE;
    m_bReadComplete = FALSE;
    GetPersistData(1,GetPlayerIdentifier(0),pd_public_rw,0,PersDataReadCallback,&m_ReadCount);
}

//==============================================================================

xbool match_mgr::IsCareerStatsReadComplete( void )
{
    return m_bReadComplete;
}

//==============================================================================

void match_mgr::InitiateCareerStatsWrite( void )
{
    ASSERT( m_bIsWriting == FALSE );
    m_WriteCount    = 0;
    m_bIsWriting    = TRUE;
    m_bWriteComplete = FALSE;
    g_MatchMgr.m_CareerStats.Checksum = x_chksum( &g_MatchMgr.m_CareerStats.Stats, sizeof(g_MatchMgr.m_CareerStats.Stats) );
    SetPersistData(1,GetPlayerIdentifier(0),pd_public_rw,0,(char *)&m_CareerStats,sizeof(m_CareerStats),PersDataWriteCallback,&m_WriteCount);
}

//==============================================================================

xbool match_mgr::IsCareerStatsWriteComplete( void )
{
    return m_bWriteComplete;
}

//==============================================================================

void match_mgr::StatsUpdate( void )
{
    switch( m_StatsState )
    {
        case GS_STATS_NOT_CONNECTED:
            break;

        case GS_STATS_DISCONNECTED:
            LOG_MESSAGE( "match_mgr::StatsUpdate", "State: GS_STATS_DISCONNECTED" );
            InitiateStatsConnection();
            break;

        case GS_STATS_INITIATE_CONNECT:
            LOG_MESSAGE( "match_mgr::StatsUpdate", "State: GS_STATS_INITIATE_CONNECT" );
            InitiateStatsConnection();
            break;

        case GS_STATS_CONNECTING:
            LOG_MESSAGE( "match_mgr::StatsUpdate", "State: GS_STATS_CONNECTING" );
            StatsUpdateConnect();
            break;

        case GS_STATS_CONNECTED:
            //LOG_MESSAGE( "match_mgr::StatsUpdate", "State: GS_STATS_CONNECTED" );
            StatsUpdateRead();
            StatsUpdateWrite();
            break;
    }
}

//==============================================================================

void match_mgr::SetAllGameStats( const player_stats& Stats )
{
    m_GameStats = Stats;

    LOG_MESSAGE( "match_mgr::SetAllGameStats", 
                 "PlayTime: %d - Games:%d", Stats.PlayTime, Stats.Games );
}

//==============================================================================

void match_mgr::SetAllCareerStats( const player_stats& Stats )
{
    m_CareerStats.Stats = Stats;
}

//==============================================================================

void match_mgr::EnableStatsConnection( void )
{
    LOG_MESSAGE( "match_mgr::EnableStatsConnection", "Changing state to: GS_STATS_INITIATE_CONNECT" );
    m_StatsState = GS_STATS_INITIATE_CONNECT;
}

//==============================================================================

void match_mgr::StatsUpdateConnect( void )
{
    if( IsStatsConnected() )
    {
        if( m_ConnectCount==0 )
        {
            LOG_MESSAGE( "match_mgr::StatsUpdateConnect", "Thinking..." );
            PersistThink();
        }
        else
        {
            LOG_MESSAGE( "match_mgr::StatsUpdateConnect", "Connected!" );
            m_StatsState = GS_STATS_CONNECTED;
            InitiateCareerStatsRead();
        }
    }
    else
    {
        LOG_MESSAGE( "match_mgr::StatsUpdateConnect", "Disconnected!" );
        m_StatsState = GS_STATS_DISCONNECTED;
    }
}

//==============================================================================

void match_mgr::StatsUpdateRead( void )
{
    if( IsStatsConnected() )
    {
        if( m_bIsReading )
        {
            if( m_ReadCount==0 )
            {
                LOG_MESSAGE( "match_mgr::StatsUpdateRead", "Thinking..." );
                PersistThink();
            }
            else
            {
                LOG_MESSAGE( "match_mgr::StatsUpdateRead", "Read Complete!" );
                m_bIsReading    = FALSE;
                m_bReadComplete = TRUE;
            }
        }
    }
    else
    {
        LOG_MESSAGE( "match_mgr::StatsUpdateRead", "Disconnected!" );
        m_StatsState    = GS_STATS_DISCONNECTED;
        m_bIsReading    = FALSE;
        m_bReadComplete = FALSE;
    }
}

//==============================================================================

void match_mgr::StatsUpdateWrite( void )
{
    if( IsStatsConnected() )
    {
        if( m_bIsWriting )
        {
            if( m_WriteCount==0 )
            {
                LOG_MESSAGE( "match_mgr::StatsUpdateWrite", "Thinking..." );
                PersistThink();
            }
            else
            {
                LOG_MESSAGE( "match_mgr::StatsUpdateWrite", "Write Complete!" );
                m_bIsWriting     = FALSE;
                m_bWriteComplete = TRUE;
                InitiateCareerStatsRead();
            }
        }
    }
    else
    {
        LOG_MESSAGE( "match_mgr::StatsUpdateWrite", "Disconnected" );
        m_StatsState     = GS_STATS_DISCONNECTED;
        m_bIsWriting     = FALSE;
        m_bWriteComplete = FALSE;
    }
}

//==============================================================================

void match_mgr::UpdateCareerStatsWithGameStats( void )
{
    g_MatchMgr.m_CareerStats.Stats.KillsAsHuman  += g_MatchMgr.m_GameStats.KillsAsHuman;
    g_MatchMgr.m_CareerStats.Stats.KillsAsMutant += g_MatchMgr.m_GameStats.KillsAsMutant;
    g_MatchMgr.m_CareerStats.Stats.Deaths        += g_MatchMgr.m_GameStats.Deaths;
    g_MatchMgr.m_CareerStats.Stats.PlayTime      += g_MatchMgr.m_GameStats.PlayTime;
    g_MatchMgr.m_CareerStats.Stats.Games         += g_MatchMgr.m_GameStats.Games;
    g_MatchMgr.m_CareerStats.Stats.Wins          += g_MatchMgr.m_GameStats.Wins;
    g_MatchMgr.m_CareerStats.Stats.Gold          += g_MatchMgr.m_GameStats.Gold;
    g_MatchMgr.m_CareerStats.Stats.Silver        += g_MatchMgr.m_GameStats.Silver;
    g_MatchMgr.m_CareerStats.Stats.Bronze        += g_MatchMgr.m_GameStats.Bronze;
    g_MatchMgr.m_CareerStats.Stats.Kicks         += g_MatchMgr.m_GameStats.Kicks;
    g_MatchMgr.m_CareerStats.Stats.VotesStarted  += g_MatchMgr.m_GameStats.VotesStarted;

    LOG_MESSAGE( "match_mgr::UpdateCareerStatsWithGameStats",
                 "PlayTime: %d - Games:%d", 
                 g_MatchMgr.m_CareerStats.Stats.PlayTime,
                 g_MatchMgr.m_CareerStats.Stats.Games );
}

//==============================================================================

void match_mgr::ParseMessageOfTheDay( const char* pBuffer )
{
    token_stream    Stream;

    Stream.OpenText( pBuffer );

    m_MessageOfTheDay.Clear();
    m_RemoteManifestVersion = 0;
    m_RemotePatchVersion = 0;

    while( TRUE )
    {
        Stream.Read();
        if( Stream.IsEOF() )
        {
            break;
        }
        if( x_stricmp( Stream.String(), "[PATCH]" ) == 0 )
        {
            m_RemotePatchVersion = Stream.ReadInt();
        }
        else if( x_stricmp( Stream.String(), "[CONTENT]" ) == 0 )
        {
            m_RemoteManifestVersion = Stream.ReadInt();
        }
        else if( x_stricmp( Stream.String(), "[MESSAGE]" ) == 0 )
        {
            m_MessageOfTheDay += Stream.ReadString();
            Stream.Read();
            while( Stream.Type() == token_stream::TOKEN_STRING )
            {
                m_MessageOfTheDay+='\n';
                m_MessageOfTheDay+= Stream.String();
                Stream.Read();

            }
        }
        else
        {
            ASSERTS( FALSE, "Expected [PATCH]|[CONTENT]|[MESSAGE]" );
        }
    }
    Stream.CloseText();

    LOG_MESSAGE( "match_mgr::ParseMessageOfTheDay", "Manifest Version:%d MOTD:%s", m_RemoteManifestVersion, GetMessageOfTheDay() );
    m_HasNewContent = (m_RemoteManifestVersion > m_LocalManifestVersion );
}
