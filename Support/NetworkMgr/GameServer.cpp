//==============================================================================
//
//  GameServer.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#include "GameServer.hpp"
#include "ServerVersion.hpp"

#include "MatchMgr.hpp"
#include "MsgMgr.hpp"
#include "GameMgr.hpp"
#include "NetworkMgr.hpp"
#include "MsgMgr.hpp"
#include "Configuration/GameConfig.hpp"
#include "ClientProxy.hpp"

#include "x_log.hpp"

#include "NetLimits.hpp"

//#include "Entropy/VoIP/VoIP.hpp"
//#include "../game/source/onlineui/OnlineUI.hpp"

//=========================================================================

xbool SHOW_SERVER_PACKETS = FALSE;

#ifdef X_DEBUG
//#define SHOW_PACKET_SENDS    
#endif

f32 KICK_PING_THRESHOLD = 1000.0f;

//extern void SetFriendlyFire( xbool Flag );

//=========================================================================

s32 g_ClientsDropped   = 0;
s32 g_ClientsConnected = 0;

//=========================================================================

game_server::game_server( void )
{
    s32 i;

    m_AllowClients  = TRUE;
    m_MaxClients    = 0;
    m_pClients      = NULL;
    m_State         = STATE_SERVER_IDLE;

    ClearBanList();

    g_ClientsDropped = 0;
    g_ClientsConnected = 0;

    m_LocalPlayerCount = 0;
    for( i=0; i<NET_MAX_PER_CLIENT; i++ )
        m_LocalPlayerSlot[i] = -1;
}
//=========================================================================

game_server::~game_server( void )
{
}

//=========================================================================

void game_server::Init( net_socket& LocalSocket, s32 LocalPlayerCount, s32 MaxClients )
{
    m_pLocal            = &LocalSocket;
    m_LocalPlayerCount  = LocalPlayerCount;
    m_UpdateIndex       = 0;
    m_MaxClients        = MaxClients;

    EnterState(STATE_SERVER_INIT);
}    

//=========================================================================

void game_server::Kill( void )
{
    ASSERT( GetState() == STATE_SERVER_IDLE );

    if( GetState() != STATE_SERVER_IDLE )
    {
        SetState( STATE_SERVER_SHUTDOWN );
    }

    while( GetState() != STATE_SERVER_IDLE )
    {
        Update( 0.0f );
    }

    SetState( STATE_SERVER_KILL );
    while( GetState() != STATE_SERVER_IDLE )
    {
        Update( 0.0f );
    } 
}

//=========================================================================

net_address game_server::GetAddress( void )
{
    return m_pLocal->GetAddress();
}

//------------------------------------------------------------------------------

void game_server::AcceptLogin( const net_address& Remote, s32 ClientIndex, s32 PlayerCount )
{
    ASSERT( ClientIndex < m_MaxClients );

    client_proxy&       Client = m_pClients[ClientIndex];
    netstream           Reply;
    s32                 j;

    // A login response is always sent unencrypted
    Client.m_ConnMgr.SetEncryption( FALSE );

    // Dummy packet to test security validation
#if defined(X_DEBUG) && defined(bwatson) && defined(TARGET_PS2)
    {
        s32 QueryTable[]=
        {
            0x100000, 0x100,
            0xabcde0, 0x12a,
            0xdeefbe, 0xbe0,
        };

        netstream Response;
        Response.Open( CONN_PACKET_IDENTIFIER, CONN_PACKET_LOGIN_RESPONSE );
        Response.WriteS32( GAME_EXIT_SECURITY_QUERY );
        s32 nQueries;
        s32 i;
        i=0;

        nQueries = sizeof(QueryTable)/sizeof(s32)/2;

        Response.WriteU32( nQueries, 8 );
        while( nQueries )
        {
            Response.WriteU32( QueryTable[i++], 26 );
            Response.WriteU32( QueryTable[i++], 16 );
            nQueries--;
        }
        Response.Close();
        Response.Send( *m_pLocal, Remote );
    }
#endif
    Reply.Open( CONN_PACKET_IDENTIFIER, CONN_PACKET_LOGIN_RESPONSE );
    Reply.WriteS32( PlayerCount );   // >=0 error code, means login succeeded. What else do we need to send?
    Reply.WriteU32( Client.m_ConnMgr.GetSessionID()     , 8 );
    Reply.WriteU32( ClientIndex                         , 8 );
    Reply.WriteU32( g_ActiveConfig.GetMaxPlayerCount()  , 8 );

    for( j = 0; j < PlayerCount; j++ )
    {
        Reply.WriteS32( Client.m_LocalPlayerSlot[j] );
        // ALSO NEED TO SEND THE NAME.
    }

    // MSK send Server's Public key back to Client Here!!
    char* pPublicKey = "";
        
    Reply.WriteString( pPublicKey );
    Reply.Close();

    Reply.Send(*m_pLocal, Remote);
    Client.m_ConnMgr.SetEncryption( TRUE );
}

//------------------------------------------------------------------------------

void game_server::RefuseLogin(const net_address& Remote, exit_reason Reason)
{
    netstream Reply;

    Reply.Open(CONN_PACKET_IDENTIFIER, CONN_PACKET_LOGIN_RESPONSE);
    Reply.WriteS32(Reason);                            // Reason for refusal
    Reply.Close();
    Reply.Send(*m_pLocal, Remote);
}

//------------------------------------------------------------------------------

exit_reason s_ForceRefusal = GAME_EXIT_CONTINUE;

void game_server::ProcessLoginRequest( const net_address& Remote, netstream& BitStream )
{
    s32 i,j;

    s32     ServerVersion;
    s32     PlayerCount;
    char    Password[16];
    u64     PlayerIdentifier[NET_MAX_PER_CLIENT];
    char    PlayerNames[NET_MAX_PER_CLIENT][NET_NAME_LENGTH];
    char    ClientPublicKey[32];
    byte    UniqueId[NET_MAX_ID_LENGTH];
    s32     Skin = SKIN_HAZMAT_0;

    Password[15] = '\0';

/*
    // Data per player
    xwchar  Name[16];
    s32     CharacterType;
    s32     SkinType;
    s32     VoiceType;
*/
    if( s_ForceRefusal != GAME_EXIT_CONTINUE )
    {
        RefuseLogin( Remote, s_ForceRefusal );
    }
    BitStream.ReadS32       ( ServerVersion );
    BitStream.ReadRangedS32 ( PlayerCount, 0, NET_MAX_PER_CLIENT-1 );
    BitStream.ReadRangedS32 ( Skin, SKIN_BEGIN_PLAYERS, SKIN_END_PLAYERS );
    BitStream.ReadBits      ( UniqueId, NET_MAX_ID_LENGTH*8 );
    BitStream.ReadString    ( Password );

    // MSK Parse out Client's Public Key Here!!!!
    BitStream.ReadString    ( ClientPublicKey );

    LOG_MESSAGE( "game_server::ProcessLoginRequest",
                 "Players:%d - Client Public Key:'%s'",
                 PlayerCount, ClientPublicKey );

    for( i=0; i<PlayerCount; i++ )
    {
        BitStream.ReadU64( PlayerIdentifier[i] );
        BitStream.ReadString( PlayerNames[i], NET_NAME_LENGTH );
    }
 
    BitStream.ReadMarker();

    // Tell client if wrong version
    if( ServerVersion != g_ServerVersion )
    {
        RefuseLogin( Remote, GAME_EXIT_LOGIN_REFUSED );
        LOG_WARNING( "game_server::ProcessLoginRequest",
                     "Server(%d) refused Client(%d) due to version.\n",
                     g_ServerVersion, ServerVersion );
        return;
    }

    // Validate password if server has one.
    if( g_ActiveConfig.IsPasswordEnabled() && x_strcmp( g_ActiveConfig.GetPassword(), Password ) )
    {
        RefuseLogin( Remote, GAME_EXIT_BAD_PASSWORD );
        return;
    }

    // Check to see if client machine has been banned.
    if( IsBanned( UniqueId ) )
    {
        RefuseLogin( Remote, GAME_EXIT_CLIENT_BANNED );
        LOG_WARNING( "game_server::ProcessLoginRequest", 
                     "Server refused banned client(%d).\n", *(s32*)UniqueId );
        return;
    }

    if( GetClientCount() >= g_ActiveConfig.GetMaxPlayerCount() )
    {
        RefuseLogin( Remote, GAME_EXIT_SERVER_FULL );
        LOG_WARNING( "game_server::ProcessLoginRequest", 
                     "Client dropped because too many clients(%d).\n", *(s32*)UniqueId );
        return;
    }

    // Check if client is already in list
    xbool FoundClient=FALSE;

    s32 Index = -1;
    for( i=0; i < m_MaxClients; i++ )
    {
        if( Remote == m_pClients[i].GetAddress() )
        {
            Index = i;
            FoundClient = TRUE;
            break;
        }
    }

    if( !FoundClient )
    {
        for( i=0; i<m_MaxClients; i++ )
        {
            if( !m_pClients[i].IsConnected() )
            {        
                Index = i;
                break;
            }
        }
    }

    // If we could not find a client slot to put this client in, just
    // bail out now and refuse the login request.
    if( Index == -1 )
    {
        LOG_WARNING( "game_server::ProcessLoginRequest", "Login refused because Server is full.\n" );
        RefuseLogin( Remote, GAME_EXIT_SERVER_FULL );
        return;
    }

    if( GetAveragePing() > 600.0f )
    {
        RefuseLogin( Remote, GAME_EXIT_SERVER_BUSY );
        g_ClientsConnected++;
        g_ClientsDropped++;
        LOG_WARNING( "game_server::ProcessLoginRequest", "Server dropped because it was too busy client(%d).\n", *(s32*)UniqueId );
        return;
    }

    // It wasn't in the list
    if( !FoundClient )
    {
        // Check if we have room to add one
        if( GameMgr.RoomForPlayers( PlayerCount ) )
        {
            // MSK move conn_manager.Init into ClientInstance.Init()
            // MSK and save BF Key.
            char* pBFKey = "";

#if defined(ENABLE_ENCRYPTION)
            LOG_MESSAGE( "game_server::ProcessLoginRequest()", "Computed BFKey was '%s' \n", pBFKey );
#endif
            
            m_pClients[Index].Init( m_pLocal,             // Local socket
                                    Remote,               // Remote address
                                    Index,                // Client index
                                    UniqueId,             // Unique Identifier for that system
                                    (byte*)"",            // Identifier provided by matchmaking service
                                    pBFKey,               // Encryption key
                                    TRUE);                // Enable encryption

            s32 PlayerIndex;
            for( j = 0; j < PlayerCount; j++ )
            {
                xwchar Name[ 16 ];
                x_mstrcpy( Name, PlayerNames[j] );
                GameMgr.Connect( PlayerIndex, 
                                 Index, 
                                 PlayerIdentifier[j], 
                                 Name,
                                 Skin, 
                                 FALSE );
                ASSERT( PlayerIndex != -1 );
                m_pClients[Index].AddPlayer( PlayerIndex, PlayerNames[j], PlayerIdentifier[j] );
            }                        

            g_ActiveConfig.SetPlayerCount( g_ActiveConfig.GetPlayerCount() + PlayerCount );
            g_PendingConfig.SetPlayerCount( g_ActiveConfig.GetPlayerCount() );
            // We only change state if we were already registered. Otherwise,
            // the lack of registration would mean that timeouts in matchmgr
            // will deal with registration.
            UpdateRegistration();

            // Add to empty slot
//          m_pClients[Index].LastHeartbeat   = tgl.LogicTimeMs;
//          m_pClients[Index].HeartbeatDelay  = 0;
//          m_pClients[Index].PacketShipDelay = 0;

//          m_pClients[Index].UpdateManager.SetClientControlled( m_pClients[Index].PlayerIndex[0],
//                                                               m_pClients[Index].PlayerIndex[1]);

//          LOG_WARNING( "game_server::ProcessLoginRequest", "ConnManager inited %d\n", Index );

            // Decide on session ID
            s32 SessionID = x_irand(0,65536);
            m_pClients[Index].m_ConnMgr.SetSessionID(SessionID);

            // Send login succeeded message back

            AcceptLogin( Remote, Index, PlayerCount );

            LOG_WARNING( "game_server::ProcessLoginRequest", 
                         "Client added, index:%d, SessionID:%d",
                         Index, SessionID );
        }
        else
        {
            RefuseLogin( Remote, GAME_EXIT_SERVER_FULL );
            LOG_WARNING( "game_server::ProcessLoginRequest", "Server refused Client due to overcrowding\n");
        }
    }
    else
    // It was in the list
    {
        AcceptLogin( Remote, Index, PlayerCount );
        LOG_WARNING( "game_server::ProcessLoginRequest", "Got a double client login?\n");
    }
}

//------------------------------------------------------------------------------

xbool game_server::ReceivePacket( const net_address& Remote, netstream& BitStream )
{
    u16     PacketType;
    u16     Header;
    s32     i;

    // Was it destined for the connection manager of any of our
    // currently connected clients?
    for (i=0;i<m_MaxClients;i++)
    {
        if ( m_pClients[i].IsConnected() && (Remote == m_pClients[i].GetAddress()) )
        {
            if (m_pClients[i].ReceivePacket(BitStream))
                return TRUE;
        }
    }


    if (BitStream.Validate()==FALSE)
    {
        return FALSE;
    }

    BitStream.ReadU16   (Header);

    if (Header != CONN_PACKET_IDENTIFIER)
    {
        return FALSE;
    }

    BitStream.ReadU16   (PacketType);

    switch( PacketType )
    {
    case CONN_PACKET_LOGIN_REQUEST:
        // MSK NOTE 2
        ProcessLoginRequest(Remote, BitStream);
        break;

    default:
        x_DebugMsg( "Unknown packet 0x%04X received from %s\n", 
                    PacketType, Remote.GetStrAddress() );
        return FALSE;
        break;
    }
    return TRUE;
}

//==============================================================================

xbool   s_ForceClientDrop    = FALSE;
f32     g_ShipDelayPingLimit = 500.0f;
f32     g_ShipDelayModifier  =   2.5f;
f32        s_KickTimeout        =  10.0f;
xtimer  s_KickTimer;

//==============================================================================

exit_reason game_server::Update( f32 DeltaTime )
{
    s32     i, j;
//  byte    VoiceBuffer[128];
    s32     Length;
    (void)Length;

    UpdateState(DeltaTime);
    UpdateVoice(DeltaTime);

    //
    // Do heartbeats, and kicks too
    //

    for( i=0; i<m_MaxClients; i++ )
    {
        if( m_pClients[i].IsConnected() )
        {
            m_pClients[i].Update(DeltaTime,GetState());
        }
    }

    // If our average client ping time is greater than 1000ms, 
    // for 10 seconds then we look for the last client that 
    // joined and kick them.
    if ( (GetAveragePing() > KICK_PING_THRESHOLD) || s_ForceClientDrop)
    {
        if (s_KickTimer.IsRunning() || s_ForceClientDrop)
        {
            if ( (s_KickTimer.ReadSec() > s_KickTimeout) || s_ForceClientDrop)
            {
                s32 Index;

                Index = m_MaxClients-1;
                s_ForceClientDrop = FALSE;
                for( i=0; i<m_MaxClients; i++ )
                {
                    if( m_pClients[Index].IsConnected() )
                    {
                        LOG_ERROR("game_server::Update","Disconnecting client %d, Average Ping:%2.02f, KickTimer:%2.02f",Index,GetAveragePing(),(f32)s_KickTimer.ReadSec());
                        {
                            s32 j;
                            for (j=0;j<m_MaxClients;j++)
                            {
                                if (m_pClients[j].IsConnected())
                                {
                                    LOG_WARNING("game_server::Update","Client %d, ping %2.02f",j,m_pClients[j].m_ConnMgr.GetPing());
                                }
                            }
                        }
                        m_pClients[Index].Disconnect( GAME_EXIT_SERVER_BUSY );
                        // 20 seconds until we drop another client
                        s_KickTimeout = 15.0f;
                        g_ClientsDropped++;
                        break;
                    }
                    s_KickTimer.Stop();
                    s_KickTimer.Reset();
                    Index--;
                }
            }
        }
        else
        {
            s_KickTimer.Reset();
            s_KickTimer.Start();
        }
    }
    else
    {
        s_KickTimer.Stop();
        s_KickTimer.Reset();
        s_KickTimeout = 10.0f;
    }

    // Now figure out if it is time to send an update packet to
    // each of the attached clients
    if( m_State == STATE_SERVER_INGAME )          /// g_SM.MissionRunning
    {
        //
        // Dish out any net pain we may have accumulated from the clients.
        //
        {
            for( j=0; j<m_MaxClients; j++ )
            {
                if( m_pClients[j].m_IsConnected )
                {
                    while( m_pClients[j].m_PainQueue.GetCount() )
                    {
                        // Ship this pain off to the unlucky recipient.
                        net_pain NetPain;
                        m_pClients[j].m_PainQueue.GetPain( NetPain );
                        NetObjMgr.ApplyNetPain( NetPain );
                    }
                }
            }
        }

        s32 nClients=0;
        for( j = 0; j < m_MaxClients; j++ )
        {
            if( m_pClients[j].m_ConnMgr.IsConnected() )
                nClients++;
        }

        if( nClients > 0 )
        {
            //
            // For each object, update its dirty bits with respect to every 
            // client.
            //
            for( i = 0; i < NET_MAX_OBJECTS; i++ )
            {
                netobj* pObject = NetObjMgr.GetObjFromSlot( i );
                u32     Bits    = 0x00000000;

                if( pObject )
                {
                    ASSERT( pObject->net_GetSlot() == i );
                    Bits = pObject->net_GetClearDirty();

                    if( Bits )
                    {
                        for( j = 0; j < m_MaxClients; j++ )
                        {
                            if( m_pClients[j].m_ConnMgr.IsConnected() )
                            {
                                u32 Mask = pObject->net_GetUpdateMask( j );
                                m_pClients[j].m_UpdateMgr.AddDirtyBits( i, Bits & Mask );
                            }
                        }
                    }
                }

                if( NetObjMgr.JustRemoved( i ) )
                {
                    for( j = 0; j < m_MaxClients; j++ )
                    {
                        if( m_pClients[j].m_ConnMgr.IsConnected() )
                        {
                            m_pClients[j].m_UpdateMgr.ObjectRemoved( i );
                        }
                    }
                }
            }
        }

        //
        // Setup updates for game manager.
        //
        {
            u32 DirtyBits;
            u32 ScoreBits;
            u32 PlayerBits;

            GameMgr.GetClearDirty( DirtyBits, ScoreBits, PlayerBits );

            for( j=0; j<m_MaxClients; j++ )
            {
                if( m_pClients[j].m_IsConnected )
                {
                    m_pClients[j].m_GMMgr.AddDirtyBits( DirtyBits, 
                                                        ScoreBits, 
                                                        PlayerBits );
                }
            }
        }

        //
        // Now, ship packets out to the clients as needed.
        //
        // We want to prevent "outbound packet harmonic" problems where, for
        // whatever reason, the server starts trying to send packets in "bursts
        // with gaps" rather than evenly divided among clients and frames.
        //
        // The fastest we want to update is 10 packets per second, or 1 packet 
        // every 3rd frame.  If we have 3 clients, then we would (under optimal
        // conditions) send 1 packet per frame.  If we have 30 clients, then 10.
        // 
        // So, the maximum number of packets we want to send is 1/3 the number
        // of clients.  Round uneven results up.
        // 

        s32 PacketsAllowed = (nClients + 2) / 3;

        for( i=0; i<m_MaxClients; i++ )
        {
            if( m_pClients[m_UpdateIndex].ProvideUpdate() )
            {
                PacketsAllowed--;
            }
            m_UpdateIndex++;
            if( m_UpdateIndex >= m_MaxClients )
            {
                m_UpdateIndex = 0;
            }
            if( PacketsAllowed == 0 )
                break;
        }
    }

    return GetExitReason();
}

//=========================================================================

void game_server::DisplayPings( s32 L )
{
    x_printfxy( 0, L++, "SERVER'S CLIENTS" );

    for( s32 i=0; i<m_MaxClients; i++ )
    {
        if( m_pClients[i].IsConnected() )
        {
            x_printfxy( 0, L++, "%2d] %1f",
                        i, m_pClients[i].m_ConnMgr.GetPing() );
        }
    }
}

//=========================================================================

f32 game_server::GetAveragePing( void )
{
    f32 average = 0.0f;
    s32 count   = 0;
    s32 i;

    for( i=0; i<m_MaxClients; i++ )
    {
        if( m_pClients[i].IsConnected() && m_pClients[i].HasMapLoaded() )
        {
            count++;
            average += m_pClients[i].m_ConnMgr.GetPing();
        }
    }

    if( count )
    {
        return( average / count );
    }

    return( 0.0f );
}

//=========================================================================

void game_server::ShowLifeDelay( void )
{
    s32 L = 0;
    for( s32 i=0; i<m_MaxClients; i++ )
    {
        if( m_pClients[i].IsConnected() )
        {
            x_printfxy( 0, L, "%2d] %5.1f", i, m_pClients[i].m_ConnMgr.GetPing() );
        //  m_pClients[i].UpdateManager.ShowLifeDelay(L);
            L++;
        }
    }
}

//=========================================================================

void game_server::SyncMissionLoad( void )
{
#if 0
    s32 i;
    for( i = 0; i < obj_mgr::MAX_SERVER_OBJECTS; i++ )
    {
        m_MissionLoaded   [i] = FALSE;
        m_MissionLoadedSeq[i] = -1;
    }

    for( i = 32; i < obj_mgr::MAX_SERVER_OBJECTS; i++ )
    {
        object::id ObjID( i, -1 );
        object* pObj = ObjMgr.GetObjectFromID(ObjID);
        if( pObj )
        {
            m_MissionLoaded[i]    = TRUE;
            m_MissionLoadedSeq[i] = pObj->GetObjectID().Seq;
            m_MissionLoadedIndex  = i;
        }
    }
#endif
}

//=========================================================================

#if 0
void game_server::SendMsg( const msg& Msg )
{
    if( Msg.Target < tgl.NLocalPlayers )
    {
        if( (g_SM.CurrentState == STATE_SERVER_INGAME) || 
            (g_SM.CurrentState == STATE_CAMPAIGN_INGAME) )
        {
            MsgMgr.ProcessMsg( Msg );
        }
    }
    else
    {
        s32 i, j;

        // Send it to indicated player.
        for( j = 0; j < m_MaxClients; j++ )
        if( (m_pClients[j].Connected) &&
            (m_pClients[j].ConnMgr.GetDestState() == STATE_CLIENT_INGAME) )
        {
            for( i = 0; i < m_pClients[j].NPlayers; i++ )
            if( m_pClients[j].PlayerIndex[i] == Msg.Target )
            {
                m_pClients[j].UpdateManager.SendMsg( Msg );
                return;
            }
        }
    }
}
#endif

//=========================================================================

void game_server::KickPlayer( s32 PlayerIndex )
{
    s32 i;

    for( i = 0; i < m_MaxClients; i++ )
    {
        if ( m_pClients[i].KickPlayer(PlayerIndex) )
        {
            BanClient( m_pClients[i].GetUniqueId(), m_pClients[i].GetTicket() );
            break;
        }
    }
}

//=========================================================================

void game_server::DropPlayer( s32 PlayerIndex )
{
    s32 i;
    (void)PlayerIndex;

    for( i = 0; i < m_MaxClients; i++ )
    {
        /*
        if( (m_pClients[i].IsConnected()) &&
            ((m_pClients[i].m_PlayerIndex[0] == PlayerIndex) ||
             (m_pClients[i].m_PlayerIndex[1] == PlayerIndex)) )
        {
            m_pClients[i].SetStatus(CLIENT_ERR_DROPPED);
        }
        */
    }
}

//=========================================================================

void game_server::BanClient( const byte* ClientUniqueId, const char* Ticket )
{
    (void)Ticket;
    LOG_MESSAGE( "game_server::BanClient", "" );

    s32 i;

    // Find either the entry for this Id or a free entry.
    for( i = 0; i < 16; i++ )
    {
        if( m_BanWait[i] == 0 )
        {
            x_memcpy(m_BanList[i],ClientUniqueId,sizeof(m_BanList[i]));
            m_BanWait[i] = 2;
            
            return;
        }
    }
}

//=========================================================================

xbool game_server::IsBanned( const byte* ClientUniqueId )
{
    s32 i;

    for( i = 0; i < 16; i++ )
    {
        if( (x_memcmp(m_BanList[i], ClientUniqueId, sizeof(m_BanList[i]))==0) && (m_BanWait[i] > 0) )
            return( TRUE );
    }

    return( FALSE );
}

//=========================================================================

void game_server::ClearBanList( void )
{   
    s32 i;

    for( i = 0; i < 16; i++ )
    {
        x_memset(m_BanList[i],0,sizeof(m_BanList[i]));
        m_BanWait[i] = 0;
    }
}

//=========================================================================

void game_server::RelaxBanList( void )
{
    s32 i;

    for( i = 0; i < 16; i++ )
    {
        if( m_BanWait[i] > 0 )
            m_BanWait[i]--;
    }
}

//=========================================================================

s32 game_server::GetBanCount( void )
{
    s32 i;
    s32 Banned = 0;

    for( i = 0; i < 16; i++ )
    {
        if( m_BanWait[i] > 0 )
            Banned++;
    }

    return( Banned );
}

//=========================================================================

xbool game_server::IsInGame( s32 clientIndex )
{
    return m_pClients[clientIndex].IsInGame();
}

//=========================================================================

s32 game_server::GetClientCount( void )
{
    s32 i;
    s32 Count;

    Count = 0;
    for (i=0;i<m_MaxClients;i++)
    {
        if (m_pClients[i].IsConnected())
        {
            Count++;
        }
    }
    return Count;
}

//=========================================================================

s32 game_server::GetClientIndex( s32 PlayerIndex )
{
    s32     i;
    s32     j;

    for( i=0; i<m_MaxClients; i++ )
    {
        for( j=0; j< m_pClients[i].GetPlayerCount(); j++ )
        {
            if( PlayerIndex == m_pClients[i].GetPlayerSlot( j ) )
            {
                return i;
            }
        }
    }

    for( i=0; i<m_LocalPlayerCount; i++ )
    {
        if( m_LocalPlayerSlot[i] == PlayerIndex )
            return -1;
    }

    return -2;
}

//=========================================================================

s32 game_server::GetClientPlayerSlot ( s32 ClientIndex, s32 PlayerIndex )
{
    if ( ClientIndex == m_MaxClients)
    {
        return GetLocalPlayerSlot(PlayerIndex);
    }

    ASSERT(m_pClients[ClientIndex].IsConnected());
    return m_pClients[ClientIndex].GetPlayerSlot(PlayerIndex);
}

//=========================================================================

s32 game_server::GetClientPlayerCount( s32 ClientIndex )
{
    ASSERT( ClientIndex != m_MaxClients );
    ASSERT(m_pClients[ClientIndex].IsConnected());
    return m_pClients[ClientIndex].GetPlayerCount();
}

//=========================================================================
s32 game_server::GetLocalPlayerCount( void )
{
    return( m_LocalPlayerCount );
}

//=========================================================================

s32 game_server::GetLocalPlayerSlot( s32 Player )
{
    ASSERT( IN_RANGE( 0, Player, NET_MAX_PER_CLIENT-1 ) );
    return( m_LocalPlayerSlot[ Player ] );
}

//=========================================================================

const char* game_server::GetClientTicket(s32 Client)
{
    ASSERT(Client < m_MaxClients);
    return m_pClients[Client].GetTicket();
}

//=========================================================================
// This only deals with the local server's headset.

void game_server::UpdateVoice( f32 DeltaTime )
{
    s32     VoiceDataLength = 0;
    byte    VoiceDataBuffer[ 256 ];

    (void)DeltaTime;

    if( g_NetworkMgr.IsOnline() == FALSE )
    {
        return;
    }

    /*
    // If the local server player is trying to speak, let him be arbitrated!
    if( g_VoiceMgr.IsTalking() )
    {
        g_VoiceMgr.Arbitrate( m_LocalPlayerSlot[0], g_VoiceMgr.GetLocalTalkMode() );
    }
    else
    {
        g_VoiceMgr.Arbitrate( m_LocalPlayerSlot[0], TALK_NONE );
    }
    */

    // On XBox when we are recording voice attachments we must not distribute the audio to clients.
    #ifdef TARGET_XBOX
    if( g_VoiceMgr.GetHeadset().GetVoiceIsRecording() == TRUE )
        return;
    #endif

    // If we have any local voice data, we need to pump that off to all who
    // may be interested.
    while( TRUE )
    {
        VoiceDataLength = g_VoiceMgr.ReadFromVoiceFifo( VoiceDataBuffer, g_VoiceMgr.GetHeadset().GetEncodedBlockSize() );
        if( VoiceDataLength <= 0 )
        {
            break;
        }
        g_VoiceMgr.Distribute( m_LocalPlayerSlot[0], VoiceDataBuffer, VoiceDataLength, g_VoiceMgr.GetLocalDesiredTalkMode() );
    }
}

//==============================================================================
voice_proxy& game_server::GetVoiceProxy( s32 ClientIndex )
{
    return m_pClients[ClientIndex].m_VoiceProxy;
}

//==============================================================================
xbool game_server::HasPlayerBuddy( const char* pSearch )
{
    s32 i;
    s32 j;
    for( i=0; i<m_MaxClients; i++ )
    {
        client_proxy& Client = m_pClients[i];
        if( Client.IsConnected() )
        {
            for( j=0; j<Client.m_LocalPlayerCount; j++ )
            {
                if( x_stristr( Client.m_PlayerNames[j], pSearch ) )
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

//==============================================================================
xbool game_server::IsServerABuddy( const char* pSearch )
{
    return x_mstristr( m_Title, pSearch ) != NULL;
}

//==============================================================================
void game_server::UpdateRegistration( void )
{
    // Go through, building whatever is needed to be sent to the matchmaking
    // service. We need to tell it what players are connected to this server.
    g_MatchMgr.RefreshRegistration();
}

//==============================================================================
f32 game_server::GetClientPing( s32 ClientIndex )
{
    ASSERT( ClientIndex < m_MaxClients );
    if( ClientIndex < 0 )
    {
        return 0.0f;
    }
    return m_pClients[ClientIndex].m_ConnMgr.GetPing();
}

//==============================================================================

xbool game_server::IsClientConnected( s32 ClientIndex )
{
    return m_pClients[ClientIndex].IsConnected();
}

//==============================================================================
