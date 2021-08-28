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
#include "e_Network.hpp"
#include "NetworkMgr/MatchMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "StateMgr/StateMgr.hpp"
#include "ServerVersion.hpp"
#include "Configuration/GameConfig.hpp"
#include "x_log.hpp"

//=========================================================================
//  Defines
//=========================================================================

#define LABEL_STRING(x) case x: return ( #x );

// Authentication data filename. This is short for obfuscation reason.

//=========================================================================
//  External function and data prototypes
//=========================================================================

extern s32 g_Changelist;

//=========================================================================
//  Data
//=========================================================================

match_mgr g_MatchMgr;

//------------------------------------------------------------------------------
match_mgr::match_mgr( void )
{
    m_Initialized               = FALSE;
    m_State                     = MATCH_IDLE;
    m_pSocket                   = NULL;
    m_LocalIsServer             = FALSE;
    m_LocalManifestVersion      = 0;
    m_RemoteManifestVersion     = 0;
    m_LocalPatchVersion         = 0;
    m_RemotePatchVersion        = 0;
    m_UserStatus                = BUDDY_STATUS_OFFLINE;
    m_PendingUserStatus         = BUDDY_STATUS_OFFLINE;
    m_PendingAcquisitionMode    = ACQUIRE_INVALID;
    m_BuddyState                = MATCH_BUDDY_IDLE;

    m_ServerCount               = 0;

    m_pListHead                 = NULL;

#if defined(ENABLE_XBOX_LIVE)
#endif
    SetConnectStatus( MATCH_CONN_UNAVAILABLE );
#ifdef TARGET_PS2
    m_StatsState                = 0;
    m_bIsWriting                = 0;
    m_bIsReading                = 0;
#endif
}

//------------------------------------------------------------------------------
match_mgr::~match_mgr( void )
{
}

//------------------------------------------------------------------------------
s32 match_mgr::GetServerCount( void )
{
    return m_ServerCount;
}

//------------------------------------------------------------------------------
server_info* match_mgr::GetServerInfo( s32 Index )
{
    llnode* pNode = m_pListHead;

    for( s32 i = 0; (i < Index) && pNode; i++ )
    {
        pNode = pNode->pNext;
        ASSERT( pNode );
    }

    if( pNode )
        return pNode->pServerInfo;
    else
        return NULL;
}

//------------------------------------------------------------------------------
s32 match_mgr::FindServerInfo( const server_info& Response )
{
    llnode* pNode = m_pListHead;

    s32 ServerNum = 0;

    while( pNode )
    {
        ASSERT( ServerNum < m_ServerCount );

        if( *(pNode->pServerInfo) == Response )
        {
            return ServerNum;
        }

        pNode = pNode->pNext;
        ServerNum++;
    }

    // Couldn't find it.
    return -1;
}

//------------------------------------------------------------------------------
void match_mgr::ResetServerList( void )
{
    llnode* pCurr = m_pListHead;
    while( pCurr )
    {
        delete pCurr->pServerInfo;
        llnode* pOld = pCurr;
        pCurr = pCurr->pNext;
        delete pOld;
    }
    m_ServerCount = 0;
    m_pListHead   = NULL;
}

//------------------------------------------------------------------------------
void match_mgr::AppendToServerList( const server_info& Response )
{
    llnode* pNode       = new llnode;
    pNode->pServerInfo  = new server_info;

    x_memcpy( pNode->pServerInfo, &Response, sizeof(server_info) );

    pNode->pNext        = NULL;

    if( m_pListHead )
    {
        llnode* pLast;
        pLast = m_pListHead;
        while( pLast->pNext )
        {
           pLast = pLast->pNext;
        }
        pLast->pNext = pNode;
    }
    else
    {
        m_pListHead = pNode;
    }

    m_ServerCount++;
}

//=============================================================================
// Note: when the full master server stuff is in place, each individual registered
// server will have a unique ID # which will be used for the actual equivalence 
// comparison. For now, we use the name and the IP as the uniqueness identifier.
//------------------------------------------------------------------------------
xbool operator == ( const server_info& Left, const server_info& Right )
{
#if defined(TARGET_PS2)
    return ( Left.Remote == Right.Remote );
#else
    return x_memcmp( &Left.SessionID, &Right.SessionID, sizeof(Left.SessionID) )==0;
#endif
}

//------------------------------------------------------------------------------
xbool operator != ( const server_info& Left, const server_info& Right )
{
#if defined(TARGET_PS2)
    return( Left.Remote != Right.Remote );
#else
    return x_memcmp( &Left.SessionID, &Right.SessionID, sizeof(Left.SessionID) )!=0;
#endif
}

//------------------------------------------------------------------------------
const char* GetStateName(match_mgr_state Mode)
{
    switch( Mode )
    {
    case MATCH_IDLE:                       return "MATCH_IDLE";
    case MATCH_AUTHENTICATE_MACHINE:       return "MATCH_AUTHENTICATE_MACHINE";
    case MATCH_SILENT_LOGON:			   return "MATCH_SILENT_LOGON";
    case MATCH_CONNECT_MATCHMAKER:         return "MATCH_CONNECT_MATCHMAKER";
    case MATCH_SECURITY_CHECK:             return "MATCH_SECURITY_CHECK";
    case MATCH_AUTH_DONE:                  return "MATCH_AUTH_DONE";
    case MATCH_DOWNLOADING:                return "MATCH_DOWNLOADING";
    case MATCH_GET_PATCH:                  return "MATCH_GET_PATCH";
    case MATCH_SAVE_PATCH:                 return "MATCH_SAVE_PATCH";
    case MATCH_GET_MESSAGES:               return "MATCH_GET_MESSAGES";
    case MATCH_ACQUIRE_IDLE:               return "MATCH_ACQUIRE_IDLE";
    case MATCH_ACQUIRE_SERVERS:            return "MATCH_ACQUIRE_SERVERS";
    case MATCH_ACQUIRE_LAN_SERVERS:        return "MATCH_ACQUIRE_LAN_SERVERS";
    case MATCH_ACQUIRE_EXTENDED_INFO:      return "MATCH_ACQUIRE_EXTENDED_INFO";
    case MATCH_VALIDATE_PLAYER_NAME:       return "MATCH_VALIDATE_PLAYER_NAME";
    case MATCH_BECOME_SERVER:              return "MATCH_BECOME_SERVER";
    case MATCH_VALIDATE_SERVER_NAME:       return "MATCH_VALIDATE_SERVER_NAME";
    case MATCH_UPDATE_SERVER:              return "MATCH_UPDATE_SERVER";
    case MATCH_REGISTER_SERVER:            return "MATCH_REGISTER_SERVER";
    case MATCH_SERVER_CHECK_VISIBLE:       return "MATCH_SERVER_CHECK_VISIBLE";
    case MATCH_SERVER_ACTIVE:              return "MATCH_SERVER_ACTIVE";
    case MATCH_UNREGISTER_SERVER:          return "MATCH_UNREGISTER_SERVER";
    case MATCH_NAT_NEGOTIATE:              return "MATCH_NAT_NEGOTIATE";
    case MATCH_LOGIN:                      return "MATCH_LOGIN";
    case MATCH_CLIENT_ACTIVE:              return "MATCH_CLIENT_ACTIVE";
    case MATCH_AUTHENTICATE_USER:          return "MATCH_AUTHENTICATE_USER";
    case MATCH_AUTH_USER_CONNECT:          return "MATCH_AUTH_USER_CONNECT";
    case MATCH_AUTH_USER_CREATE:           return "MATCH_AUTH_USER_CREATE";
    case MATCH_INDIRECT_LOOKUP:             return "MATCH_INDIRECT_LOOKUP";
    default:                               ASSERT(FALSE);return "<unknown>";
    }
}

//------------------------------------------------------------------------------
const char* GetStatusName( match_conn_status Status )
{
    switch( Status )
    {
        LABEL_STRING( MATCH_CONN_IDLE );
        LABEL_STRING( MATCH_CONN_UNAVAILABLE );
        LABEL_STRING( MATCH_CONN_ACQUIRING_LOBBIES );
        LABEL_STRING( MATCH_CONN_ACQUIRING_SERVERS );
        LABEL_STRING( MATCH_CONN_ACQUIRING_BUDDIES );
        LABEL_STRING( MATCH_CONN_DOWNLOADING );
        LABEL_STRING( MATCH_CONN_REGISTERING );
        LABEL_STRING( MATCH_CONN_CONNECTED );
        LABEL_STRING( MATCH_CONN_DISCONNECTED );
        LABEL_STRING( MATCH_CONN_NOT_VISIBLE );
        LABEL_STRING( MATCH_CONN_REGISTER_FAILED );
        LABEL_STRING( MATCH_CONN_CONNECTING );
        LABEL_STRING( MATCH_CONN_SESSION_ENDED );
default:                               ASSERT( FALSE );
    }
    return "<undefined>";
}

//------------------------------------------------------------------------------
const char* GetStatusName( auth_status Status )
{
    switch( Status )
    {
        LABEL_STRING( AUTH_STAT_DISCONNECTED );
        LABEL_STRING( AUTH_STAT_CONNECTED );
        LABEL_STRING( AUTH_STAT_CONNECTING );
        LABEL_STRING( AUTH_STAT_INVALID_USER );
        LABEL_STRING( AUTH_STAT_NO_ACCOUNT );
        LABEL_STRING( AUTH_STAT_CANNOT_CONNECT );
        LABEL_STRING( AUTH_STAT_BANNED );
        LABEL_STRING( AUTH_STAT_RETIRED );
        LABEL_STRING( AUTH_STAT_DROPPED );
        LABEL_STRING( AUTH_STAT_SECURITY_FAILED );
        LABEL_STRING( AUTH_STAT_NEED_PASSWORD );
        LABEL_STRING( AUTH_STAT_SELECT_USER );
        LABEL_STRING( AUTH_STAT_NEED_UPDATE );
        LABEL_STRING( AUTH_STAT_INVALID_ACCOUNT );
        LABEL_STRING( AUTH_STAT_URGENT_MESSAGE );
        LABEL_STRING( AUTH_STAT_NO_USERS );
        LABEL_STRING( AUTH_STAT_ALREADY_LOGGED_ON );
        LABEL_STRING( AUTH_STAT_SERVER_BUSY );
    default:                    ASSERT( FALSE );
    }
    return "<undefined>";
}

//------------------------------------------------------------------------------
void match_mgr::SetUniqueId(const byte* Id, s32 IdLength)
{
    x_memset(m_UniqueId, 0, sizeof(m_UniqueId));

    if( IdLength >= (s32)sizeof(m_UniqueId) )
    {
        IdLength = sizeof(m_UniqueId)-1;
    }
    x_memcpy( m_UniqueId, Id, IdLength );
}

//------------------------------------------------------------------------------
const byte* match_mgr::GetUniqueId(s32& IdLength)
{
    IdLength = sizeof(m_UniqueId);
    return m_UniqueId;
}

//==============================================================================

xbool match_mgr::ValidateServerInfo( const server_info &info )
{
    ASSERT( info.Version == g_ServerVersion );
    if( info.Version != g_ServerVersion ) return FALSE;

    ASSERT( info.MaxPlayers > 1 && info.MaxPlayers <= 32 );
    if( !( info.MaxPlayers > 1 && info.MaxPlayers <= 32 ) ) return FALSE;

    ASSERT( info.Name[0] != 0 );
    if( !( info.Name[0] != 0 ) ) return FALSE;

    ASSERT( info.FragLimit >= 0 );
    if( !( info.FragLimit >= 0 ) ) return FALSE;

    ASSERT( info.GameTime >= 0 );
    if( !( info.GameTime >= 0 ) ) return FALSE;
   
    return TRUE;
}

//==============================================================================

s32 match_mgr::GetUserAccountCount( void )
{
    return m_UserAccounts.GetCount();
}

//==============================================================================

const online_user& match_mgr::GetActiveUserAccount( void )         
{ 
    return m_ActiveUserAccount;
}
                    
//==============================================================================

const online_user& match_mgr::GetUserAccount( s32 UserIndex )
{
    return m_UserAccounts[UserIndex];
}

//==============================================================================

void match_mgr::LockLists( void )
{
    m_ListMutex.Enter();
}

//==============================================================================

void match_mgr::UnlockLists( void )
{
    ASSERT( m_ListMutex.IsLocked() );
    m_ListMutex.Exit();
}

//==============================================================================

xbool match_mgr::AreListsLocked( void )
{
    return m_ListMutex.IsLocked();
}

//==============================================================================

void match_mgr::LockBrowser( void )
{
    m_BrowserMutex.Enter();
}

//==============================================================================

void match_mgr::UnlockBrowser( void )
{
    ASSERT( m_BrowserMutex.IsLocked() );
    m_BrowserMutex.Exit();
}

//==============================================================================

xbool match_mgr::IsBrowserLocked( void )
{
    return m_BrowserMutex.IsLocked();
}

//==============================================================================
const buddy_info* match_mgr::FindBuddy( u64 Identifier )
{
    s32 Index;
    buddy_info Buddy;

    Buddy.Identifier = Identifier;

    Index = m_Buddies.Find( Buddy );
    if( Index == -1 )
        return NULL;

    if( m_Buddies[Index].Flags & USER_REQUEST_IGNORED )
    {
        return NULL;
    }

    return &m_Buddies[Index];
}


//==============================================================================

const char* GetStatusName( buddy_status Status )
{
    switch( Status )
    {
    case BUDDY_STATUS_OFFLINE:      return "BUDDY_STATUS_OFFLINE";
    case BUDDY_STATUS_ONLINE:       return "BUDDY_STATUS_ONLINE";
    case BUDDY_STATUS_INGAME:       return "BUDDY_STATUS_INGAME";
    case BUDDY_STATUS_ADD_PENDING:  return "BUDDY_STATUS_ADD_PENDING";
    case BUDDY_STATUS_REQUEST_ADD:  return "BUDDY_STATUS_REQUEST_ADD";
    default:                        return "<unknown>";
    }
}

//==============================================================================

void match_mgr::SetFilter( game_type GameType, s32 MinPlayers, s32 MaxPlayers )
{
    m_FilterGameType = GameType;
    m_FilterMinPlayers = MinPlayers;
    m_FilterMaxPlayers = MaxPlayers;
}
//==============================================================================

void match_mgr::SetFilter( game_type GameType, s32 MinPlayers, s32 MaxPlayers, s32 MutationMode,  s32 PasswordEnabled,  s32 VoiceEnabled )
{
    m_FilterGameType        = GameType;
    m_FilterMinPlayers      = MinPlayers;
    m_FilterMaxPlayers      = MaxPlayers;
    m_FilterMutationMode    = MutationMode;
    m_FilterPassword        = PasswordEnabled;
    m_FilterHeadset         = VoiceEnabled; 
}

//==============================================================================
s32 match_mgr::GetRecentPlayerCount( void )
{
        return m_RecentPlayers.GetCount();
}

//==============================================================================
const buddy_info& match_mgr::GetRecentPlayer( s32 Index )
{
    return m_RecentPlayers[Index];
}

//==============================================================================
buddy_info* match_mgr::FindRecentPlayer( u64 Identifier )
{
    buddy_info* pBuddy = NULL;

    for( s32 i=0; i < m_RecentPlayers.GetCount(); i++ )
    {
        if( m_RecentPlayers[i].Identifier == Identifier )
        {
            pBuddy = &m_RecentPlayers[i];
            break;
        }
    }

    return( pBuddy );
}

//==============================================================================
void match_mgr::AddRecentPlayer( const player_score& Player )
{
    buddy_info Buddy;

    x_memset( &Buddy, 0, sizeof(Buddy) );

    Buddy.Identifier = Player.Identifier;
    x_strcpy( Buddy.Name, Player.NName );

    if( m_RecentPlayers.Find( Buddy ) == -1 )
    {
        if( m_RecentPlayers.GetCount() == 10 )
        {
            m_RecentPlayers.Delete(0);
        }
        LOG_MESSAGE( "match_mgr::AddRecentPlayer", "Player %s added to recent player list.", Player.NName );
        m_RecentPlayers.Append( Buddy );
    }
}

//==============================================================================

buddy_info& match_mgr::GetBuddy( s32 Index )
{
    return m_Buddies[Index];
}

//==============================================================================

s32 match_mgr::GetBuddyCount( void )
{
    return m_Buddies.GetCount();
}

//==============================================================================

xbool match_mgr::IsBuddy( const buddy_info& Buddy )
{
    s32 i;
    for( i=0; i< m_Buddies.GetCount(); i++ )
    {
        if( m_Buddies[i] == Buddy )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//==============================================================================

const char* match_mgr::GetMessageOfTheDay( void )
{
    if( m_MessageOfTheDay.GetLength() )
    {
        return (const char*)m_MessageOfTheDay;
    }
    return NULL;
}

//==============================================================================

void match_mgr::ApplyPatch( void* pData, s32 Length )
{
    u8* pPatch;

    pPatch = (u8*)pData;
    if( pPatch )
    {
        while( Length>0 )
        {
            s32     PatchSize;
            u8*     pDest;
            PatchSize = *pPatch++;
            if( PatchSize == 0 )
            {
                break;
            }
            pDest     = (u8*)((pPatch[0]<<2)|(pPatch[1]<<10)|(pPatch[2]<<18));
            pPatch   += 3;
#if defined(X_RETAIL)
            x_memcpy( pDest, pPatch, PatchSize );
#endif
            LOG_MESSAGE("match_mgr::ApplyPatch", "Patch Address:0x%08x, Length:%d", pDest, PatchSize );
            pPatch += PatchSize;
            Length -= PatchSize;
        }
    }
}

//==============================================================================

void match_mgr::SetLocalManifestVersion ( s32 Version )
{
    m_LocalManifestVersion = Version;
}

//==============================================================================
s32 match_mgr::GetLocalPatchVersion( void )
{
    return m_LocalPatchVersion;
}

//==============================================================================
s32 match_mgr::GetRemotePatchVersion( void )
{
    return m_RemotePatchVersion;
}

//==============================================================================
void match_mgr::SetLocalPatchVersion( s32 Version )
{
    m_LocalPatchVersion = Version;
}
//==============================================================================

void match_mgr::SetRemoteManifestVersion( s32 Version )
{
    m_RemoteManifestVersion = Version;
}

//==============================================================================

s32 match_mgr::GetLocalManifestVersion ( void )
{
    return m_LocalManifestVersion;
}

//==============================================================================

s32 match_mgr::GetRemoteManifestVersion( void )
{
    return m_RemoteManifestVersion;
}

//==============================================================================

xbool match_mgr::IsBusy( void )
{
    return (GetState() != MATCH_IDLE) && (GetState() != MATCH_AUTH_DONE) && (GetState() != MATCH_ACQUIRE_IDLE);
}

//==============================================================================

void match_mgr::SetConnectStatus( match_conn_status Status )
{
    LOG_MESSAGE( "match_mgr::SetConnectStatus", "Connect status change from %s to %s", GetStatusName(m_ConnectStatus), GetStatusName(Status) );
    m_ConnectStatus = Status;
}

//==============================================================================
void match_mgr::SetAuthStatus( auth_status Status )
{
    LOG_MESSAGE( "match_mgr::SetAuthStatus", "Auth status changed from %s to %s", GetStatusName(m_AuthStatus), GetStatusName(Status) );
    m_AuthStatus = Status;
}

//==============================================================================
void match_mgr::ShowOnlineStatus( xbool Enabled )
{
    m_ShowOnlineStatus = Enabled;
}

//==============================================================================

void match_mgr::SetUserStatus( buddy_status Status )
{ 
    m_PendingUserStatus = Status;
}

