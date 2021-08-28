//==============================================================================
//
//  GameClient.cpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

//#define SHOW_DMT_STUFF

//==============================================================================

#include "x_files.hpp"
#include "x_color.hpp"
#include "e_Network.hpp"
#include "x_bitstream.hpp"
//nclude "Entropy/VoIP/VoIP.hpp"
#include "GameServer.hpp"
#include "GameClient.hpp"
#include "GameMgr.hpp"
#include "MatchMgr.hpp"
#include "x_log.hpp"
#include "NetworkMgr.hpp"
#include "MsgMgr.hpp"
#include "StateMgr/MapList.hpp"
#include "StateMgr/StateMgr.hpp"
#include "Configuration/GameConfig.hpp"
#include "NetworkMgr/Voice/VoiceMgr.hpp"

//#include "../game/source/onlineui/OnlineUI.hpp"

#include "ServerVersion.hpp"

xbool SHOW_CLIENT_PACKETS = FALSE;

extern xwchar g_JoinPassword[];

//extern int g_NumPlayers;
//extern int g_levelWeaponSetID;

extern void SetFriendlyFire( xbool Flag );

//=========================================================================

game_client::game_client( void )
{
    m_LoggedIn          = FALSE;
    m_InSync            = FALSE;
    m_LastConnectCheck  = 0;
    m_LocalPlayerCount  = 0;

//  m_PlayerObjID[0]    = obj_mgr::NullID;
//  m_PlayerObjID[1]    = obj_mgr::NullID;
//  m_NPlayers          = tgl.NRequestedPlayers;
}

//=========================================================================

game_client::~game_client( void )
{
}

//=========================================================================

void game_client::Init  ( net_socket&           Local,
                          const net_address&    Remote,
                          s32                   LocalPlayerCount )
{
    m_pLocal            = &Local;
    m_RemoteServer      = Remote;
	m_ClientIndex       = -10;
    m_State             = STATE_CLIENT_IDLE;
    m_PacketSendDelay   = 0.0f;
    m_LocalPlayerCount  = LocalPlayerCount;
    m_Connected         = TRUE;

    SetState( STATE_CLIENT_INIT );
}

//=========================================================================

void game_client::Kill( void )
{
    if( GetState() != STATE_CLIENT_IDLE )
    {
        SetState( STATE_CLIENT_KILL );
        while( GetState() != STATE_CLIENT_IDLE )
        {
            Update( 0.0f );
        }
    }
}

//=========================================================================

void game_client::SetClientIndex( s32 ClientIndex )
{
    m_ClientIndex = ClientIndex;
    m_UpdateMgr.Init( m_ClientIndex );
    // TO DO - ConnMgr.SetClientIndex()?
}

//=========================================================================

net_address game_client::GetAddress( void )
{
    return( m_RemoteServer );
}

//=========================================================================
// Returns FALSE when no longer connected.

xbool game_client::Disconnect( void )
{
    // NOTE: This does not take in to account the sending interval to
    // the game server. This is probably the principle reason the server
    // gets flooded with disconnect requests.

    if( ( m_StateTime > 1.0f ) || (m_State < STATE_CLIENT_COOLDOWN) )
    {
        SetState( STATE_CLIENT_DISCONNECT );
    }

    m_LoggedIn = FALSE;
    return( m_ConnMgr.IsConnected() );
}

//=========================================================================
// This is part of the 2nd level security check. 

extern s32 g_Changelist;

void game_client::ProcessSecurityRequest( netstream& BitStream )
{
    u32         NumberOfQueries;
    netstream   Response;

    BitStream.ReadU32( NumberOfQueries, 8 );
    Response.Open( CONN_PACKET_IDENTIFIER, CONN_PACKET_SECURITY_RESPONSE );
    Response.WriteU32( eng_GetProductCode(), 16 );
    Response.WriteU32( g_Changelist, 18 );

    LOG_MESSAGE( "game_client::ProcessSecurityRequest", "Security request received. %d queries.", NumberOfQueries );
    LOG_MESSAGE( "game_client::ProcessSecurityRequest", "Response product code:%d, changelist:%d", eng_GetProductCode(), g_Changelist );

    while( NumberOfQueries )
    {
        u32 Base;
        u32 Length;
        u32 Checksum;

        BitStream.ReadU32( Base, 26 );              // Address 0..32M
        BitStream.ReadU32( Length, 16 );            // Length 0..65535
        Checksum = x_chksum( (void*)Base, Length );
        Response.WriteU32( Checksum );

        LOG_MESSAGE( "game_client::ProcessSecurityRequest", 
                     "Query. Address:0x%08x, Length:0x%04x, Checksum:0x%08x", 
                     Base, Length, Checksum );
        if( BitStream.Overwrite() || Response.Overwrite() )
        {
            SetExitReason( GAME_EXIT_SECURITY_FAILED );
            SetState( STATE_CLIENT_DISCONNECT );
            return;
        }
        NumberOfQueries--;
    }

    Response.Close();
    Response.Send( *m_pLocal, m_RemoteServer );
}

//=========================================================================

void game_client::ProcessLoginResponse( netstream& BitStream )
{
    s32             PlayerCount;
    u32             ClientIndex;
    u32             SessionId;
    u32             MaxPlayers;
//  u32             MapIndex;
    s32             i;
    s32             Slot;
    char            ServerPublicDHKey[32];
    char*           pBFKey;

    // We're passed the initial packet header, now grab the remaining 
    // information within the netstream.

    if( IsLoggedIn() )
    {
        LOG_WARNING("game_client::ProcessLoginResponse","Login response received when already logged in.");
        return;
    }
    BitStream.ReadS32( PlayerCount );
    if( PlayerCount == GAME_EXIT_SECURITY_QUERY )
    {
        ProcessSecurityRequest( BitStream );
        return;
    }

    if( PlayerCount < 0 )
    {
        // Some sort of error occurred during login, set the
        // error code and bail out.
        SetExitReason( (exit_reason)PlayerCount );
        SetState( STATE_CLIENT_DISCONNECT );

        LOG_ERROR( "game_client::ProcessLoginResponse", 
                   "LOGIN FAILED - ErrorCode:%d", PlayerCount );

        return;
    }

    BitStream.ReadU32( SessionId  , 8 );
    BitStream.ReadU32( ClientIndex, 8 );
    BitStream.ReadU32( MaxPlayers , 8 );

    LOG_MESSAGE( "game_client::ProcessLoginResponse", 
                 "Session:%d - Client:%d - MaxPlayers:%d",
                 SessionId, ClientIndex, MaxPlayers );

    ASSERT( MaxPlayers > 0 );

    SetClientIndex( ClientIndex );

    g_PendingConfig.SetMaxPlayerCount( MaxPlayers );
    g_PendingConfig.SetPlayerCount( PlayerCount );

    LOG_APP_NAME( xfs( "CLIENT:%d", ClientIndex ) );

    for( i = 0; i < PlayerCount; i++ )
    {
        BitStream.ReadS32( Slot );
        m_LocalPlayerSlot[i] = Slot;
        LOG_MESSAGE( "game_client::ProcessLoginResponse", 
                     "LocalPlayerSlot[%d]:%d", i, Slot );
    }

    m_LocalPlayerCount = PlayerCount;

    // Now we're actually logged in. Do anything else here required
    // to set up game state to get going. Now we advance to get the map.
    
    m_ConnMgr.SetClientIndex( ClientIndex );
    m_ConnMgr.SetSessionID( SessionId );
    m_LoggedIn = TRUE;

    // TODO Use Server Public Key to Calculate BF Key.
    BitStream.ReadString( ServerPublicDHKey );
    
#if defined( ENABLE_ENCRYPTION )
    if( x_strlen( ServerPublicDHKey ) == 0 )
    {
        x_strcpy( ServerPublicDHKey, "<unset>" );
    }
    pBFKey = "";
    m_ConnMgr.SetEncryption( TRUE );
    LOG_MESSAGE( "game_client::ProcessLoginResponse", 
                 " Public DH Key:'%s' - Computed DH Key:'%s'",
                 ServerPublicDHKey, pBFKey );
#else
    (void)pBFKey;
#endif

}

//=========================================================================
extern void FetchManifest( void );

void game_client::ProcessMissionResponse( netstream& BitStream )
{
    s32                 MapIndex;
    s32                 GameType;
    xbool               FriendlyFire;
    xbool               IsVoiceEnabled;

    BitStream.ReadS32 ( MapIndex     );
    BitStream.ReadFlag( FriendlyFire );
    BitStream.ReadFlag( IsVoiceEnabled );
    BitStream.ReadS32 ( GameType );

    g_VoiceMgr.SetIsGameVoiceEnabled( IsVoiceEnabled );

    // This is a HACK to get the circuits properly initialized.
    GameMgr.SetGameType( (game_type)GameType );

    LOG_MESSAGE( "game_client::ProcessMapResponse", "MapIndex:%d", MapIndex );

    g_ActiveConfig.SetLevelID( MapIndex );
    g_ActiveConfig.SetFriendlyFire( FriendlyFire );
    g_ActiveConfig.SetVoiceEnabled( IsVoiceEnabled );
    g_ActiveConfig.SetGameTypeID ( (game_type)GameType );
    g_PendingConfig.SetGameTypeID( (game_type)GameType );   // HACK TO MOVE FORWARD
    SetState( STATE_CLIENT_LOAD_MISSION );
}

//=========================================================================

void game_client::ProcessVerifyResponse( netstream& BitStream )
{
    s32                 MapIndex;
    s32                 MapChecksum;
    const map_entry*    pMapEntry;

    BitStream.ReadS32 ( MapIndex );
    BitStream.ReadS32 ( MapChecksum );

    LOG_MESSAGE( "game_client::ProcessVerifyResponse", 
                 "MapIndex:%d, checksum:0x%08x", MapIndex, MapChecksum );

    pMapEntry = g_MapList.Find( MapIndex );
    if( pMapEntry == NULL )
    {
        SetExitReason( GAME_EXIT_INVALID_MISSION );
        SetState( STATE_CLIENT_DISCONNECT );
        return;
    }

    if( g_ActiveConfig.GetLevelID() != MapIndex )
    {
        g_ActiveConfig.SetLevelID( MapIndex );
        SetExitReason( GAME_EXIT_RELOAD_LEVEL );
        SetState( STATE_CLIENT_LOAD_MISSION );
    }
    else
    {
        if( pMapEntry->GetFlags() & MF_DOWNLOAD_MAP )
        {
            LOG_MESSAGE( "game_client::ProcessVerifyResponse", 
                         "Custom map verify. Expected Checksum:0x%08x Actual Checksum:0x%08x", 
                         MapChecksum, g_ActiveConfig.GetLevelChecksum() );

            if( MapChecksum != g_ActiveConfig.GetLevelChecksum() )
            {
                LOG_WARNING( "game_client::ProcessVerifyResponse", 
                             "Checksum failure on custom map." );
                SetExitReason( GAME_EXIT_INVALID_MISSION );
                SetState( STATE_CLIENT_DISCONNECT );
            }
            else
            {
                SetState( STATE_CLIENT_SYNC );
            }
        }
        else
        {
            SetState( STATE_CLIENT_SYNC );
        }
    }
}

//=========================================================================

void game_client::ProcessDisconnect( netstream& BitStream )
{
    s32         Error;
    exit_reason Reason;

    BitStream.ReadS32( Error );

    Reason = (exit_reason)Error;
    LOG_WARNING( "game_client::ProcessDisconnect", "Received disconnect request, reason:%s", GetExitReasonName(Reason) );
    if( Reason == GAME_EXIT_PLAYER_KICKED )
    {
        GameMgr.AddKickStat();
    }
    SetExitReason( Reason );
    m_Connected = FALSE;
}

//=========================================================================

xbool game_client::ReceivePacket( const net_address& Remote, netstream& BitStream )
{
    u16     Header;
    u16     PacketType;

    // The connection manager ALWAYS gets first go at the packet
    if( Remote != m_RemoteServer )
    {
        return FALSE;
    }

    if( m_ConnMgr.ReceivePacket(BitStream) )
    {
        return TRUE;
    }

    if( BitStream.Validate() == FALSE )
    {
        return FALSE;
    }

    BitStream.ReadU16( Header );
    BitStream.ReadU16( PacketType );

    if( Header != CONN_PACKET_IDENTIFIER )
    {
        return FALSE;
    }

    // The packet we should have received is dependent on game state. We probably should do some
    // considerable validity checks as this point to try and detect spoofing. The mantra should be
    // If in doubt, go to STATE_CLIENT_DISCONNECT state.
    // We deal with special case packets here.
    switch( PacketType )
    {
    case CONN_PACKET_LOGIN_RESPONSE:
        if( m_State == STATE_CLIENT_LOGIN )
        {
            ProcessLoginResponse( BitStream );
            return TRUE;
        }
        break;
    case CONN_PACKET_MISSION_RESPONSE:
        if( m_State == STATE_CLIENT_REQUEST_MISSION )
        {
            ProcessMissionResponse( BitStream );
            return TRUE;
        }
        break;
    case CONN_PACKET_VERIFY_RESPONSE:
        if( m_State == STATE_CLIENT_VERIFY_MISSION )
        {
            ProcessVerifyResponse( BitStream );
            return TRUE;
        }
        break;
    case CONN_PACKET_DISCONNECT:
        if( m_Connected )
        {
            ProcessDisconnect( BitStream );
            return TRUE;
        }
        break;
    case CONN_PACKET_SECURITY_REQUEST:
        ProcessSecurityRequest( BitStream );
        return TRUE;
    }

    //
    // Nobody used this packet!
    //
    LOG_WARNING( "game_client::ReceivePacket", 
                 "Packet not used. Type:%d, state:%s", 
                 PacketType, GetStateName(m_State) );
    return FALSE;
}

//==============================================================================

exit_reason game_client::Update( f32 DeltaTime )
{
    m_StateTime += DeltaTime;

    if( g_MatchMgr.GetConnectStatus() == MATCH_CONN_CONNECTED )
    {
        m_ConnMgr.Update( DeltaTime );
    }

    if( m_State == STATE_CLIENT_INGAME )
    {
        //
        // For each object, update its dirty bits with respect to the server.
        //

        s32 i, Lo, Hi;

        Lo  = NET_MAX_OBJECTS_ON_SERVER;
        Lo += m_ClientIndex * NET_MAX_OBJECTS_ON_CLIENT;
        Hi  = Lo            + NET_MAX_OBJECTS_ON_CLIENT;

        for( i = Lo; i < Hi; i++ )
        {
            netobj* pObject = NetObjMgr.GetObjFromSlot( i );
            u32     Bits    = 0x00000000;

            if( pObject )
            {
                ASSERT( pObject->net_GetSlot() == i );
                Bits  = pObject->net_GetClearDirty();
                Bits &= pObject->net_GetUpdateMask( -1 );
                m_UpdateMgr.AddDirtyBits( i, Bits );
            }

            if( NetObjMgr.JustRemoved( i ) )
            {
                m_UpdateMgr.ObjectRemoved( i );
            }
        }

        for( s32 i = 0; i < NET_MAX_OBJECTS_ON_SERVER; i++ )
        {
            netobj* pObject = NetObjMgr.GetObjFromSlot( i );
            u32     Bits    = 0x00000000;

            if( pObject )
            {
                ASSERT( pObject->net_GetSlot() == i );
                Bits  = pObject->net_GetClearDirty();
                Bits &= pObject->net_GetUpdateMask( -1 );
                m_UpdateMgr.AddDirtyBits( i, Bits );
            }
        }
    }

    server_state ServerState = m_ConnMgr.GetServerState();
    if( ((m_State == STATE_CLIENT_SYNC) || (m_State == STATE_CLIENT_INGAME)) && 
        ((ServerState==STATE_SERVER_SYNC)||(ServerState==STATE_SERVER_INGAME)) )
    {
        m_PacketSendDelay -= DeltaTime;
        if( m_PacketSendDelay <= 0.0f )
        {
            while( m_PacketSendDelay < 0.0f )
            {
                m_PacketSendDelay += m_ConnMgr.GetShipInterval();
            }
            m_ConnMgr.SendPacket();
        }
    }

    // Did we just detect a dropped connection? To do so, we need to have been connected then the connmgr
    // needs to have disconnected.
    if( (g_MatchMgr.GetConnectStatus()==MATCH_CONN_CONNECTED) && (m_ConnMgr.IsConnected()==FALSE) )
    {
        if( IsConnected() )
        {
            SetExitReason( GAME_EXIT_CONNECTION_LOST );
            m_Connected = FALSE;
        }
        if( (GetExitReason()==GAME_EXIT_CONTINUE) && (!IsConnected()) )
        {
            SetExitReason( GAME_EXIT_CONNECTION_LOST );
        }
    }

    UpdateState( DeltaTime );

    return( GetExitReason() );
}

//==============================================================================
#if 0
//==============================================================================

void game_client::ProcessHeartbeat( f32 DeltaSec )
{
    //
    // Do heartbeat
    //

    if( !m_LoginRefused )
    {
        m_HeartbeatDelay -= DeltaSec;
        if( m_HeartbeatDelay <= 0 )
        {
            //if( tgl.GameStage == GAME_STAGE_INGAME )
                m_HeartbeatDelay = tgl.HeartbeatDelay;
            //else
            //    m_HeartbeatDelay = 0.125f;

            m_ConnManager.SendHeartbeat();
        }
    }
}

//==============================================================================
#endif
//==============================================================================

f32 game_client::GetPing( void )
{
    return m_ConnMgr.GetPing();
}

//==============================================================================

void game_client::SendVerifyRequest( void )
{
    netstream BS;

    BS.Open( CONN_PACKET_IDENTIFIER, CONN_PACKET_VERIFY_REQUEST );
    BS.WriteS32( g_ActiveConfig.GetLevelChecksum() );
    BS.Close();
    BS.Send( *m_pLocal, m_RemoteServer );

    LOG_MESSAGE( "game_client::SendMissionRequest", 
                 "Checksum sent:0x%08x", 
                 g_ActiveConfig.GetLevelChecksum() );
}

//==============================================================================

void game_client::SendMissionRequest( void )
{
    netstream BS;

    BS.Open( CONN_PACKET_IDENTIFIER, CONN_PACKET_MISSION_REQUEST );
    BS.Close();
    BS.Send( *m_pLocal, m_RemoteServer );
}

//==============================================================================

void game_client::SendLoginRequest( void )
{
    s32             i;
    netstream       BS;
    char*           pClientPublicKey="";
    const byte*     pUniqueId;
    s32             IdLength;

    // Prepare the profile list so they map to local players 
    g_StateMgr.SetupProfileListIndex();

    pUniqueId = g_MatchMgr.GetUniqueId( IdLength );

    BS.Open             ( CONN_PACKET_IDENTIFIER, CONN_PACKET_LOGIN_REQUEST );
    BS.WriteS32         ( g_ServerVersion );                   // Server version #
    BS.WriteRangedS32   ( m_LocalPlayerCount, 0, NET_MAX_PER_CLIENT-1 );
    BS.WriteRangedS32   ( g_StateMgr.GetActiveProfile(g_StateMgr.GetProfileListIndex(0)).GetAvatarID(), SKIN_BEGIN_PLAYERS, SKIN_END_PLAYERS );
    BS.WriteBits        ( pUniqueId, NET_MAX_ID_LENGTH * 8 );
    BS.WriteString      ( g_PendingConfig.GetPassword() );
    BS.WriteString( pClientPublicKey );

    ASSERTS( m_LocalPlayerCount == 1, "We only should have one local player online!" );
    for( i=0; i<m_LocalPlayerCount; i++ )
    {
        BS.WriteU64( g_MatchMgr.GetPlayerIdentifier(i) );
#ifdef TARGET_XBOX
        BS.WriteString( (const char*)xstring(g_MatchMgr.GetActiveUserAccount().Name) );
#else
        BS.WriteString( g_StateMgr.GetProfileName( g_StateMgr.GetProfileListIndex(i) ) );
#endif
    }

    BS.WriteMarker();
    BS.Close();
    BS.Send( *m_pLocal, m_RemoteServer );

    LOG_WARNING( "game_client::SendLoginRequest", 
                 "Players:%d, remote server:%s", 
                 m_LocalPlayerCount, 
                 g_ActiveConfig.GetConfig().Remote.GetStrAddress() );
}

//==============================================================================
/*
void game_client::UnloadMission( void )
{
    s32 i;

    m_UpdateMgr.Reset();

    for( i=0; i<2; i++ )
    {
//        m_PlayerObjID[i].Seq = -1;
    }
}
*/
//==============================================================================

void game_client::EndMission( void )
{
#if 0
    m_PlayerObjID[0].Seq = -1;
    m_PlayerObjID[1].Seq = -1;
    tgl.PC[0].PlayerIndex = -1;
    tgl.PC[1].PlayerIndex = -1;
    tgl.NLocalPlayers = 0;
#endif
}

//==============================================================================

s32 game_client::GetLocalPlayerCount( void )
{
    return( m_LocalPlayerCount );
}

//==============================================================================

s32 game_client::GetLocalPlayerSlot( s32 Player )
{
    ASSERT( IN_RANGE( 0, Player, m_LocalPlayerCount ) );
    return( m_LocalPlayerSlot[ Player ] );
}

//==============================================================================

s32 game_client::GetClientIndex( void )
{
    return( m_ClientIndex );
}

//==============================================================================

pain_queue* game_client::GetPainQueue( void )
{
    return( &m_PainQueue );
}

//==============================================================================
// If we ever set any sort of exit code, then we start the game cooling down.
//
void game_client::SetExitReason( exit_reason Reason )
{
    //
    // Store the exit reason in the network manager.
    //
    g_ActiveConfig.SetExitReason( Reason );

    switch( Reason )
    {
    case GAME_EXIT_RELOAD_CHECKPOINT:
        ASSERTS( FALSE, "This should never happen on a client" );
        break;
    case GAME_EXIT_ADVANCE_LEVEL:
    case GAME_EXIT_RELOAD_LEVEL:
    case GAME_EXIT_PLAYER_DROPPED:
    case GAME_EXIT_PLAYER_KICKED:
    case GAME_EXIT_CONNECTION_LOST:
    case GAME_EXIT_LOGIN_REFUSED:
    case GAME_EXIT_BAD_PASSWORD:
    case GAME_EXIT_CLIENT_BANNED:
    case GAME_EXIT_SERVER_FULL:
    case GAME_EXIT_PLAYER_QUIT:
    case GAME_EXIT_GAME_COMPLETE:
    case GAME_EXIT_DUPLICATE_LOGIN:
    case GAME_EXIT_NETWORK_DOWN:
    case GAME_EXIT_SERVER_BUSY:
    case GAME_EXIT_SERVER_SHUTDOWN:
        break;
    default:
        break;
    }
}

//==============================================================================

exit_reason game_client::GetExitReason( void )
{
    return g_ActiveConfig.GetExitReason();
}

//==============================================================================
