//==============================================================================
//
//  ConnMgr.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#include "x_files.hpp"
#include "x_log.hpp"
#include "x_threads.hpp"

#include "ConnMgr.hpp"

#include "UpdateMgr.hpp"
#include "PainQueue.hpp"
#include "GameMgr.hpp"
#include "MsgMgr.hpp"
#include "MsgClient.hpp"
#include "GMMgr.hpp"
#include "Voice/VoiceProxy.hpp"
#include "Voice/VoiceMgr.hpp"

#include "ServerVersion.hpp"

//==============================================================================

#if defined(TARGET_DEV) && defined(X_DEBUG)
//#define ENABLE_DEBUG_TICK
#endif

//==============================================================================

//#define TRACK_HEARTBEATS
//#define TRACK_ACKS

#ifdef VERBOSE_NETWORK
extern const char* s_StateName[];
#endif
//#define VERBOSE_DEBUG_TICKS

//==============================================================================

xbool SHOW_PACKET_INFO = FALSE;
xbool s_SkipTickCheck  = FALSE;

//==============================================================================
// Acknowledge bits class
//==============================================================================

ack_bits::ack_bits( void )
{
    Clear();
}

//------------------------------------------------------------------------------

void ack_bits::Clear( void )
{
    for( s32 i = 0; i < ACKSLOTS; i++ )
        Bits[i] = 0xFFFF;
}

//------------------------------------------------------------------------------

void ack_bits::Print( void )
{
    x_DebugMsg( "%04X", Bits[ACKSLOTS-1] );
    for( s32 i = ACKSLOTS-2; i >= 0; i-- )
        x_DebugMsg( ",%04X", Bits[i] );
}

//------------------------------------------------------------------------------

void ack_bits::ShiftLeft( s32 Shift )
{
    while( Shift > 0 )
    {
        Shift--;
        Bits[ACKSLOTS-1] <<= 1;
        for( s32 i = ACKSLOTS-2; i >= 0; i-- )
        {
            if( Bits[i  ] &  0x8000 )
                Bits[i+1] |= 0x0001;
            Bits[i] <<= 1;
        }
    }
}

//------------------------------------------------------------------------------

xbool ack_bits::Bit( s32 Position )
{
    s32 Slot = Position / (16  );
    s32 Off  = Position & (16-1);

    return( (Bits[Slot] & (1<<Off)) != 0 );
}

//------------------------------------------------------------------------------

void ack_bits::Set( s32 Position )
{
    s32 Slot = Position / (16  );
    s32 Off  = Position & (16-1);

    Bits[Slot] |= (1 << Off);
}

//------------------------------------------------------------------------------

void ack_bits::Read( netstream& BS )
{
    s32 i;
    u32 Mask;

    if( BS.ReadFlag() )
    {
        // All bits are 1.  No problem.
        for( i = 0; i < ACKSLOTS; i++ )
            Bits[i] = 0xFFFF;
    }
    else
    {
        // There are some 0 bits.  Oh well.
        BS.ReadU32( Mask, ACKSLOTS );
        for( i = 0; i < ACKSLOTS; i++ )
            if( Mask & (1<<i) )
                BS.ReadU16( Bits[i] );
    }

    /*
    for( s32 i = 0; i < ACKSLOTS; i++ )
        BS.ReadU16( Bits[i] );
    */
}

//------------------------------------------------------------------------------

void ack_bits::Write( netstream& BS )
{
    s32 i;
    u32 Mask = 0;

    ASSERT( ACKSLOTS <= 32 );

    for( i = 0; i < ACKSLOTS; i++ )
        if( Bits[i] != 0xFFFF )
            Mask |= (1 << i);

    if( Mask )
    {
        // There were some 0 bits in there.  Crap.
        BS.WriteFlag( FALSE );
        BS.WriteU32( Mask, ACKSLOTS );
        for( i = 0; i < ACKSLOTS; i++ )
            if( Mask & (1<<i) )
                BS.WriteU16( Bits[i] );
    }
    else
    {
        // All bits were 1.  Hurray!
        BS.WriteFlag( TRUE );
    }

    /*
    for( s32 i = 0; i < ACKSLOTS; i++ )
        BS.WriteU16( Bits[i] );
    */
}

//==============================================================================

void PrintBits( u32 V, s32 NBits )
{
    for( s32 i=0; i<NBits; i++ )
    {
        x_DebugMsg("%1d",(V&(1<<(NBits-i-1)))?(1):(0) );
    }
    x_DebugMsg("\n");
}

//==============================================================================
//=== CONNECTION MANAGER CLASS BODY
//==============================================================================

conn_mgr::conn_mgr( void )
{
    m_IsConnected = FALSE;
    m_IsInitialized = FALSE;
    SetEncryption( FALSE );
    ClearConnection();
}

//==============================================================================

conn_mgr::~conn_mgr( void )
{
}

//==============================================================================

void conn_mgr::Init( net_socket&        Local,
                     const net_address& Remote,
                     update_mgr*        pUpdateMgr,
                     pain_queue*        pPainQueue,
                     gm_mgr*            pGMMgr,
                     voice_proxy*       pVoiceProxy,
                     s32                ClientIndex,
                     xbool              IsServer,
                     const char*        BFKey,
                     xbool              bEnableEncryption )
{
    ASSERT(!m_IsConnected);
    ClearConnection();
    (void)BFKey;

    m_pLocalSocket          = &Local;
    m_RemoteAddress         = Remote;
    m_PacketsSent           = 0;
    m_PacketsDropped        = 0;
    m_PacketsReceived       = 0;
    m_PacketsUnusable       = 0;
    m_ClientIndex           = ClientIndex;
    m_PacketShipInterval    = DEFAULT_SHIP_INTERVAL;
    m_HeartbeatInterval     = HEARTBEAT_INTERVAL;
    m_PacketLossIndex       = 0;
    m_ConnectTime           = 0;
    m_IsServer              = IsServer;
    m_ClientState           = STATE_CLIENT_INIT;
    x_memset(m_PacketAck,0,sizeof(m_PacketAck));
    x_memset(m_PacketNak,0,sizeof(m_PacketNak));

    m_pUpdateMgr            = pUpdateMgr;
    m_pPainQueue            = pPainQueue;
    m_pGMMgr                = pGMMgr;
    m_pVoiceProxy           = pVoiceProxy;
#if defined(TARGET_XBOX)
    m_VoiceSocket.Bind( Remote.GetPort(), NET_FLAGS_VDP );
    m_RemoteVoiceAddress.Clear();
#endif
//  m_VoiceFifo.Init(m_VoiceBuffer,sizeof(m_VoiceBuffer));

    /*
    m_pUpdateMgr->Init(ClientIndex);
    */

    m_DoLogging             = FALSE;
    m_LogOutgoing           = FALSE;
    m_LogIncoming           = FALSE;
//  m_DestState             = STATE_NULL;

    m_IsConnected           = TRUE;
    m_IsInitialized         = TRUE;
    m_LoginSessionID        = -1;

    // MSK TODO Save BF Key Here
//    x_strcpy(m_BlowfishKey, BFKey);
    m_bEnableEncryption     = bEnableEncryption;

    // Logging.
    {
        if( m_IsServer )
            LOG_MESSAGE( "conn_mgr::Init", "SERVER SIDE" );
        else
            LOG_MESSAGE( "conn_mgr::Init", "CLIENT SIDE" );

        LOG_MESSAGE( "conn_mgr::Init",
                     "Client:%d -- Address:%s",
                     m_ClientIndex, 
                     m_RemoteAddress.GetStrAddress() );
    }

//  x_DebugMsg("conn_mgr::Init() BlowFish Key '%s' was saved\n", m_BlowfishKey );
    
    if( m_bEnableEncryption )
    {
    }

#if defined(ENABLE_DEBUG_TICK)
    m_DebugTicker.Init(DEBUG_TICK_SOCKET);
#endif
}

//==============================================================================

void conn_mgr::ClearConnection( void )
{
    s32 i;
    m_LastSeqReceived       = -1;
    m_LastSeqSent           = -1;
    m_LastAckReceived       = -1;
    m_AckBits.Clear();

    m_HeartbeatsSent        =  0;
    m_AveragePing           = 16;

    ResetTimeouts();

    for( i=0; i<CONN_NUM_PACKETS; i++ )
        m_Packet[i].Seq = -1;

    m_LastHeartbeatRequest = -1;
    for( i=0; i<CONN_NUM_HEARTBEATS; i++ )
        m_HeartbeatPing[i] = -1;

//  m_StatsIndex = 0;
    m_StatsClock = 0.0f;
    x_memset( &m_Stats, 0, sizeof(m_Stats) );
#if !defined(X_RETAIL) || defined(X_QA)
    m_StatsClockDebug = 0.0f;
    x_memset( &m_StatsDebug, 0, sizeof(m_StatsDebug) );
#endif    
}

//==============================================================================

void conn_mgr::Kill( void )
{
    if( !m_IsInitialized )
        return;

    LOG_MESSAGE( "conn_mgr::Kill", "Client:%d -- Connection shutdown", m_ClientIndex );

#if defined(ENABLE_DEBUG_TICK)
    m_DebugTicker.Kill();
#endif
#if defined(TARGET_XBOX)
    m_VoiceSocket.Close();
    m_RemoteVoiceAddress.Clear();
#endif

    m_IsConnected   = FALSE;
    m_IsInitialized = FALSE;
    ClearConnection();
//  m_VoiceFifo.Kill();
    m_RemoteAddress.Clear();
    m_pUpdateMgr->Kill();
}

//==============================================================================

void conn_mgr::HandlePacketNotify( s32 PacketID, xbool Received )
{
    // Find the packet
    s32 P;
    for( P=0; P<CONN_NUM_PACKETS; P++ )
    {
        if( m_Packet[P].Seq == PacketID )
            break;
    }

    if( P==CONN_NUM_PACKETS )
    {
        //x_DebugMsg("**********************************\n");
        //x_DebugMsg("Notify for packet already cleared\n");
        //x_DebugMsg("**********************************\n");
        return;
    }

    //ASSERT( P < CONN_NUM_PACKETS );

#if defined(TRACK_ACKS)
    {
        if( Received )
            LOG_MESSAGE( "conn_mgr::HandlePacketNotify( ACK )",
                         "Client:%d -- Archive:%d -- PacketSeq:%d", 
                         m_ClientIndex, P, PacketID );
        else
            LOG_MESSAGE( "conn_mgr::HandlePacketNotify( NACK )",
                         "Client:%d -- Archive:%d -- PacketSeq:%d", 
                         m_ClientIndex, P, PacketID );
    }
#endif

    // Handle notify for this packet
    if( Received )
    {
        m_PacketAck[m_PacketLossIndex]++;
        //x_DebugMsg("Packet ack %1d %1d\n",P,m_Packet[P].Seq);
        if( SHOW_PACKET_INFO )
            x_DebugMsg("************** packet %1d acknowledged RECEIVED\n",PacketID);
    }
    else
    {
        m_PacketNak[m_PacketLossIndex]++;
        if( SHOW_PACKET_INFO )
            x_DebugMsg("************** packet %1d acknowledged DROPPED\n",PacketID);
        m_PacketsDropped++;
    }

    if( m_IsServer )
    {
        m_pUpdateMgr->PacketAck( m_Packet[P],     Received );
        m_pGMMgr    ->PacketAck( m_Packet[P],     Received );
        MsgMgr       .PacketAck( m_Packet[P].Seq, Received, m_ClientIndex );
    }
    else
    {
        m_pUpdateMgr->PacketAck( m_Packet[P], Received );
        m_pPainQueue->PacketAck( m_Packet[P], Received );
    }

    m_Packet[P].Seq = -1;
}

//==============================================================================

f32 conn_mgr::GetPacketLoss(void)
{
    s32 i;
    s32 naked;
    s32 acked;
    s32 total;
    f32 pc;

    naked = 0;
    acked = 0;
    total = 0;
    for (i=0;i<PACKET_LOSS_HISTORY;i++)
    {
        naked += m_PacketNak[i];
        acked += m_PacketAck[i];
    }

    if (naked+acked)
    {
        pc = (f32)naked / (f32)(naked+acked);
    }
    else
    {
        pc = 0.0f;
    }

    return pc;
}


//==============================================================================

xbool conn_mgr::DecryptBitStream( netstream& BitStream )
{
#if defined(ENABLE_ENCRYPTION)
    u8      TempBuffer[8];
    u32*    pData = (u32*)BitStream.GetDataPtr();
    s32     Size  = BitStream.GetNBytes();

    if( (Size & 7)!=0 )
    {
        LOG_MESSAGE( "conn_mgr::DecryptBitStream", "Decrypt failed due to bad packet size. Not multiple of 8 bytes." );
        return FALSE;
    }

    x_memcpy( TempBuffer, pData, sizeof(TempBuffer) );
    x_decrypt( TempBuffer, sizeof(TempBuffer), m_EncryptionKey );
    u16 Identifier;

    Identifier = (TempBuffer[2]<<8)|TempBuffer[3];
    if( Identifier != CONN_PACKET_IDENTIFIER )
    {
        LOG_WARNING( "conn_mgr::DecryptBitStream", "Bad packet id:0x%04x, expected:0x%04x", Identifier, CONN_PACKET_IDENTIFIER );
        BitStream.SetCursor( 16 );
        return FALSE;

    }

    x_decrypt( pData, Size, m_EncryptionKey );

    u16 TempPacketID;
    u16 TempPacketType;

    BitStream.SetCursor( 16 );
    BitStream.ReadU16(TempPacketID);
    BitStream.ReadU16(TempPacketType);              

    //
    // Later, we will check TempPacketType too.
    //
    if ( TempPacketID != CONN_PACKET_IDENTIFIER )
    {
        LOG_WARNING( "conn_mgr::DecryptBitStream", "Bad packet id:%d", TempPacketID );
        BitStream.SetCursor( 16 );
        return FALSE;

    }

    BitStream.SetCursor(16);
    return TRUE;
#else
    (void)BitStream;
    return TRUE;
#endif


}

//==============================================================================

void conn_mgr::EncryptBitStream( netstream& BitStream )
{
    // EncryptBitStream() will pad the netstream data to a multiple of 8 bytes, then checksum the data, and then encrypt the data. 
    // Pad the Packet to even 64 bits

#if defined(ENABLE_ENCRYPTION)
    s32 Size = BitStream.GetNBytesUsed();

    ASSERT( (Size & 7)==0 );

    x_encrypt( BitStream.GetDataPtr(), Size, m_EncryptionKey );
#else
    (void)BitStream;
#endif
}

//==============================================================================

s32 g_PacketSeq = 0;

xbool conn_mgr::ReceivePacket( netstream& BitStream )
{
    if (!m_IsConnected)
        return FALSE;

#if defined(TRACK_ACKS)
    LOG_MESSAGE( "conn_mgr::ReceivePacket", 
                 "Client:%d -- Version:%d", 
                 m_ClientIndex, net_GetVersionKey() );
#endif

    if (m_PacketLossTimer.ReadSec() > 0.1f)
    {
        m_PacketLossIndex++;
        if (m_PacketLossIndex >= PACKET_LOSS_HISTORY)
        {
            m_PacketLossIndex=0;
        }
        m_PacketAck[m_PacketLossIndex]=0;
        m_PacketNak[m_PacketLossIndex]=0;
        m_PacketLossTimer.Stop();
    }

    if (!m_PacketLossTimer.IsRunning())
    {
        m_PacketLossTimer.Reset();
        m_PacketLossTimer.Start();
    }

    if( m_DoLogging )
    {
        if( m_LogIncoming == NULL )
        {
            xfs  XFS( "To(%s)From(%s).bin",
                      m_pLocalSocket->GetStrIP(),
                      m_RemoteAddress.GetStrIP() );
            m_LogIncoming = x_fopen( XFS, "wb" );
        }

        if( m_LogIncoming )
        {
            s32 NBytes = BitStream.GetNBytesUsed();
            x_fwrite( &NBytes, 4, 1, m_LogIncoming );
            x_fwrite( BitStream.GetDataPtr(), NBytes, 1, m_LogIncoming );
        }
    }


    // Decrypt the Packet
    xbool ok = DecryptBitStream( BitStream );
    if( !ok )
    {
        // Any error would have been explained from the decrypt call
        return FALSE;
    }

    // Check the Checksum
    if( !BitStream.Validate() )
    {
        LOG_ERROR( "conn_mgr::ReceivePacket",  "Client:%d -- Checksum failed. Encryption key:0x%08x:%08x:%08x:%08x", m_ClientIndex, 
                                                m_EncryptionKey[0], m_EncryptionKey[1], m_EncryptionKey[2], m_EncryptionKey[3] );
        return FALSE;
    }

    // Throw out packet if not ours
    u16 Header;
    BitStream.ReadU16(Header);
    if (Header != CONN_PACKET_IDENTIFIER)
    {
        LOG_ERROR( "conn_mgr::ReceivePacket", 
                   "Client:%d -- Incorrect header!", 
                   m_ClientIndex );
        return FALSE;
    }

    s16 PacketType;
    BitStream.ReadS16(PacketType);

    switch (PacketType)
    {

    //----------------------------------------------------------------
    // Check if it is a heartbeat request
    case CONN_PACKET_HB_REQUEST:
        ProcessHeartbeatRequest(BitStream);
        ResetTimeouts();
        break;
    //----------------------------------------------------------------
    case CONN_PACKET_HB_RESPONSE:
        ProcessHeartbeatResponse( BitStream );
        ResetTimeouts();
        break;

    //----------------------------------------------------------------
    case CONN_PACKET_GAME_DATA:
        if( (m_ServerState==STATE_SERVER_INGAME) || (m_ServerState==STATE_SERVER_SYNC) )
        {
            ProcessGameData( BitStream );
        }
        break;

    //----------------------------------------------------------------
    // Any other packet type, we drop it.
    default:
        return FALSE;
        break;
    }

    m_PacketsReceived++;
    return TRUE;
}

//==============================================================================

void conn_mgr::Disconnect(exit_reason Reason)
{
    netstream Request;

    if (m_IsConnected)
    {
        // Send the client a disconnect request
        Request.Open    ( CONN_PACKET_IDENTIFIER, CONN_PACKET_DISCONNECT );
        Request.WriteS32( Reason );
        Request.Close();
        Send( Request );

        LOG_WARNING( "conn_mgr::Disconnect", "Client:%d -- Reason:%s", 
                     m_ClientIndex, GetExitReasonName(Reason) );
        m_IsConnected = FALSE;
    }
    else
    {
        LOG_ERROR( "conn_mgr::Disconnect(!)", "Client:%d -- Reason:%s -- NOT CONNECTED", 
                   m_ClientIndex, GetExitReasonName(Reason) );
    }
}

//==============================================================================

xbool conn_mgr::SendPacket( void )
{
    netstream BitStream;

    ASSERT( m_pUpdateMgr );

    if( m_IsConnected == FALSE )
        return FALSE;

    // Allocate a packet
    s32 P;
    for( P=0; P<CONN_NUM_PACKETS; P++ )
    {
        if( m_Packet[P].Seq == -1 )
            break;
    }

    if( P==CONN_NUM_PACKETS )
    {
        // Flush some of the packets
        for( s32 i=0; i<CONN_NUM_PACKETS; i++ )
        {
            HandlePacketNotify( m_LastAckReceived, FALSE );
            m_LastAckReceived++;
        }

        m_IsConnected = FALSE;
        LOG_ERROR( "conn_mgr::SendPacket(fail)",
                   "Client:%d -- Too many unacknowledged packets -- DISCONNECTED", 
                   m_ClientIndex );
        return FALSE;
    }

    m_LastSeqSent++;
    //x_DebugMsg("NSent %1d\n",m_LastSeqSent );

    m_Packet[P].Seq = m_LastSeqSent;

    // Clear packet values
    m_Packet[P].NUpdates     =  0;
    m_Packet[P].FirstUpdate  = -1;
    m_Packet[P].TargetClient = m_ClientIndex;

#if defined(TRACK_ACKS)
    LOG_MESSAGE( "conn_mgr::SendPacket", "Client:%d, PacketSeq:%d, LastReceived:%d", m_ClientIndex, m_LastSeqSent, m_LastSeqReceived );
#endif

    // Pack in known bytes
    BitStream.Open( CONN_PACKET_IDENTIFIER, CONN_PACKET_GAME_DATA );

    // Pack in session number
    BitStream.WriteU32( (u32)GetSessionID(), 8 );

    // Pack in new sequence number
    BitStream.WriteS32( m_LastSeqSent, 24 );

    // Pack in new acknowledge number
    BitStream.WriteS32( m_LastSeqReceived, 24 );

    // Pack in ack bits
    m_AckBits.Write( BitStream );

    ASSERT( m_pUpdateMgr );

    // Tell other managers to pack in their data
    WriteManagerData( m_Packet[P], BitStream );

    //x_DebugMsg( "(L:%d,U:%d)\n", m_Packet[P].NLife, m_Packet[P].NUpdates );

    ASSERT( m_pUpdateMgr );

    // Done with the bitstream, so close it
    BitStream.Close();

    // Encrypt Bitstream
    EncryptBitStream( BitStream );

    // Send the packet
    BitStream.Send( *m_pLocalSocket, m_RemoteAddress );

    if( SHOW_PACKET_INFO )
    {
        x_DebugMsg("SEND PACKET(%1d) %1d\n",m_Packet[P].Seq,BitStream.GetNBytesUsed());
    }

    if( m_DoLogging )
    {
        if( m_LogOutgoing == NULL )
        {
            xfs  XFS( "To(%s)From(%s).bin",
                      m_RemoteAddress.GetStrIP(),
                      m_pLocalSocket->GetStrIP() );
            m_LogOutgoing = x_fopen( XFS, "wb" );
        }

        if( m_LogOutgoing )
        {
            s32 NBytes = BitStream.GetNBytesUsed();
            x_fwrite( &NBytes, 4, 1, m_LogOutgoing );
            x_fwrite( BitStream.GetDataPtr(), NBytes, 1, m_LogOutgoing );
        }
    }

    // Verbosity.
    #ifdef VERBOSE_NETWORK
    {
        x_DebugMsg( "==>> SEND %d <", m_LastSeqSent );
        m_AckBits.Print();
        x_DebugMsg( "> HighAck %d\n", m_LastSeqReceived );
    }
    #endif

    //x_printf("(%2d,%4d) ",P,BitStream.GetNBytesUsed());
    m_PacketsSent++;
    return TRUE;
}

//==============================================================================

void conn_mgr::ProcessHeartbeatResponse( netstream& BitStream )
{
    // Look up which heartbeat it was
    s32     Index;
    s32     State;

#if defined(ENABLE_DEBUG_TICK)
    m_DebugTicker.ReceiveUpdate(m_RemoteAddress, BitStream);
#endif

    BitStream.ReadS32(Index);
    BitStream.ReadS32(State);

    #ifdef VERBOSE_NETWORK
    x_DebugMsg( "<--- RECV HBRSP %d\n", Index );
    #endif

    // If index is out of range it is probably from an old mission
    if( Index > m_HeartbeatsSent )
    {
        LOG_WARNING("conn_mgr::ProcessHeartbeatResponse","Ignored, out of range. Index:%d, Expected highest:%d",Index, m_HeartbeatsSent );
        return;
    }

    // Decide if it's too far in the past
    ASSERT(Index <= m_HeartbeatsSent);
    if( (m_HeartbeatsSent - Index) > CONN_NUM_HEARTBEATS )
    {
        LOG_WARNING("conn_mgr::ProcessHeartbeatResponse","Ignored, Too old. Index:%d, Highest Sent:%d",Index, m_HeartbeatsSent );
        return;
    }

    // Read current state
    if( m_IsServer )
    {
        m_ClientState = (client_state)State;
    }
    else
    {
        m_ServerState = (server_state)State;
        if( BitStream.ReadFlag() )
        {

            if( GameMgr.GameInProgress() )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_ADVANCE_LEVEL );
                GameMgr.EndGame();
            }
            GameMgr.AcceptFinalScore( BitStream );
            LOG_WARNING( "conn_mgr::ProcessHeartbeatResponse(score)",
                         "Client:%d -- Received final score -- ClientState:%s -- ServerState:%s",
                         m_ClientIndex, GetStateName(m_ClientState), GetStateName(m_ServerState) );
        }
    }
    // Confirm that the connection is still alive
    ResetTimeouts();

    // Look up time heartbeat traveled
    //s32 Time = (s32)(tgl.LogicTimeMs - m_HeartbeatSentTime[Index%CONN_NUM_HEARTBEATS]);
    f32 Time = x_TicksToMs( x_GetTime() - (xtick)m_HeartbeatSentTime[Index%CONN_NUM_HEARTBEATS] );

#if defined(TRACK_HEARTBEATS)
    LOG_MESSAGE( "conn_mgr::ProcessHeartbeatResponse",
                 "Client:%d -- Received:%d -- Ping:%d",
                 m_ClientIndex, Index, (s32)Time );
#endif

    // Store ping time
    m_HeartbeatPing[Index%CONN_NUM_HEARTBEATS] = Time;
    m_LastPing = Time;

    // Update ping
    s32 N = MIN(CONN_NUM_HEARTBEATS,m_HeartbeatsSent);
    m_AveragePing = 0;
    for( s32 i=0; i<N; i++ )
    {
        m_AveragePing += m_HeartbeatPing[ (Index + (CONN_NUM_HEARTBEATS-i))%CONN_NUM_HEARTBEATS ];
    }
    m_AveragePing *= (1.0f/N);

    #ifdef VERBOSE_NETWORK
    x_DebugMsg( "  Ping %g\n", m_AveragePing );
    #endif

    // Display ping buffer
    if( 0 )
    {
        x_printf("PINGS ");
        for( s32 j=0; j<CONN_NUM_HEARTBEATS; j++ )
        {
            x_printf("%5.1f",m_HeartbeatPing[j]);
        }
        x_printf("\n");
    }

    UpdateShipInterval();

    //if( m_AveragePing > 10000 )
    //    m_AveragePing = (f32)Time;
    //else
    //    m_AveragePing = (m_AveragePing*0.90f + (f32)Time*0.10f);

    //m_AveragePing = (f32)Time;
}

//==============================================================================

void conn_mgr::ProcessHeartbeatRequest(netstream& BitStream)
{
    s32 Index;
    s32 State;

#if defined(ENABLE_DEBUG_TICK)
    m_DebugTicker.ReceiveUpdate(m_RemoteAddress,BitStream);
#endif

    BitStream.ReadS32( Index );
    BitStream.ReadS32( State );

#if defined(TRACK_HEARTBEATS)
    LOG_MESSAGE( "conn_mgr::ProcessHeartbeatRequest",
                 "Client:%d -- Received:%d",
                 m_ClientIndex, Index );
#endif

    if( m_IsServer )
    {
        m_ClientState = (client_state)State;
    }
    else
    {
        m_ServerState = (server_state)State;

        if( BitStream.ReadFlag() )
        {   
            if( GameMgr.GameInProgress() )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_ADVANCE_LEVEL );
                GameMgr.EndGame();
            }
            GameMgr.AcceptFinalScore( BitStream );
            LOG_WARNING( "conn_mgr::ProcessHeartbeatRequest(score)",
                         "Client:%d -- Received final score -- ClientState:%s -- ServerState:%s",
                         m_ClientIndex, GetStateName(m_ClientState), GetStateName(m_ServerState) );
        }
    }

    ResetTimeouts();

    if(     (Index > m_LastHeartbeatRequest)
        ||  ((Index < 0) && (m_LastHeartbeatRequest > 0)) )
        m_LastHeartbeatRequest = Index;

    netstream Response;

    Response.Open(CONN_PACKET_IDENTIFIER,CONN_PACKET_HB_RESPONSE);
#if defined(ENABLE_DEBUG_TICK)
    m_DebugTicker.ProvideUpdate(Response);
#endif
    Response.WriteS32( Index );
    if( IsServer() )
    {
        Response.WriteS32( m_ServerState );

        if( Response.WriteFlag( (m_ServerState == STATE_SERVER_COOLDOWN) && 
                                (m_ClientState != STATE_CLIENT_COOLDOWN) ) )
        {
            LOG_WARNING( "conn_mgr::ProcessHeartbeatRequest(score)",
                         "Client:%d -- Sent final score -- ClientState:%s -- ServerState:%s",
                         m_ClientIndex, GetStateName(m_ClientState), GetStateName(m_ServerState) );

            GameMgr.ProvideFinalScore( Response, m_ClientIndex );
        }
    }
    else
    {
        Response.WriteS32( m_ClientState );
    }


    // Send the packet
    Response.Close();

    // Encrypt Bitstream
    EncryptBitStream( Response );

    Response.Send( *m_pLocalSocket, m_RemoteAddress );

    #ifdef VERBOSE_NETWORK
    x_DebugMsg( "---> SEND HBRSP %d  State %d(%s)\n",
                Index, g_SM.CurrentState, s_StateName[ g_SM.CurrentState ] );
    #endif

}

//==============================================================================

void conn_mgr::ProcessGameData(netstream& BitStream)
{

    //
    // This is a known packet type
    //
    m_PacketsReceived++;
    //x_DebugMsg("NReceived %1d\n",m_NPacketsReceived );
    // Read packet header
    s32         pkSeqNum;
    s32         pkHighestAck;
    ack_bits    pkAckBits;
    u32         SessionID;

    BitStream.ReadU32( SessionID, 8 );
    BitStream.ReadS32( pkSeqNum, 24 );
    BitStream.ReadS32( pkHighestAck, 24 );
    pkAckBits.Read( BitStream );

    g_PacketSeq = pkSeqNum;

    // Verbosity.
#if defined(TRACK_ACKS)
    LOG_MESSAGE( "conn_mgr::ProcessGameData", "Client:%d -- PacketSeq:%d, HighestAck:%d", m_ClientIndex, pkSeqNum, pkHighestAck );
#endif

#ifdef VERBOSE_NETWORK
    {
        x_DebugMsg( "<<== RECV %d <", pkSeqNum );
        pkAckBits.Print();
        x_DebugMsg( "> HighAck %d\n", pkHighestAck );
    }
#endif

    //x_DebugMsg("RECV---- SES(%1d) SEQ(%1d)(%1d,%1d) TYPE(%08X) PING(%1.2f)\n",SessionID,pkSeqNum,m_LastSeqReceived,m_LastSeqReceived+128,KnownBytes,GetPing());

    if( SHOW_PACKET_INFO )
    {
        x_DebugMsg("RECV PACKET(%1d) %1d\n",pkSeqNum,BitStream.GetNBytesUsed());
    }

    //x_DebugMsg("HANDLE PACKET: %5d %5d ",pkSeqNum,pkHighestAck,pkAckBits);
    //PrintBits(pkAckBits,16);

    // Check if session number is wrong
    if( SessionID != (u32)GetSessionID() )
    {
        // bad packet
        m_PacketsUnusable++;
        LOG_ERROR( "conn_mgr::ProcessGameData(error)",
                   "Client:%d -- PacketSeq:%d -- Session:%d -- Expected:%d", 
                   m_ClientIndex, pkSeqNum, SessionID, GetSessionID() );
        return;
    }

    // Check if sequence number is outside the known window
    if( ((s32)pkSeqNum <= m_LastSeqReceived) ||
        ((s32)pkSeqNum  > m_LastSeqReceived+128) )
    {
        // bad packet
        m_PacketsUnusable++;
        LOG_ERROR( "conn_mgr::ProcessGameData(error)",
                   "Client:%d -- SeqNum:%d -- ValidRange:(%d...%d)",
                    m_ClientIndex, pkSeqNum, m_LastSeqReceived+1, m_LastSeqReceived+128 );
        x_DebugMsg("packet dropped for bad seq number %1d (%1d,%1d)\n",pkSeqNum,m_LastSeqReceived+1,m_LastSeqReceived+128);
        return;
    }

    //
    // Store that we have received a new packet
    //
    {
        // Shift up ack bits so all packets between new and last have zeros
        {
            s32 ShiftAmt = pkSeqNum - m_LastSeqReceived;
            if( ShiftAmt >= 128 )
                LOG_ERROR( "conn_mgr::ProcessGameData(holy crap!)",
                           "ShiftAmt: %d - BOGUS!!!!!",
                            ShiftAmt );
        }
        m_AckBits.ShiftLeft( pkSeqNum - m_LastSeqReceived );

        // Mark this packet as being received
        m_AckBits.Set( 0 );

        // Remember we have received up to this point
        m_LastSeqReceived = pkSeqNum;
    }

    // Confirm that the connection is still alive
    ResetTimeouts();

    //
    // React to packet's ack bits
    //
    if( (s32)pkHighestAck != -1 )
    {
        for( s32 i=m_LastAckReceived+1; i<=(s32)pkHighestAck; i++ )
        {
            s32 Shift = (pkHighestAck-i);
            xbool Ack;

            if( Shift > 60 )
            {
                x_DebugMsg( "*** Big shift for pkt %d: %d\n", i, Shift );
                LOG_WARNING( "conn_mgr::ProcessGameData(ackbits)",
                    "Client:%d -- PacketSeq:%d -- Shift:%d -- last ack:%d",
                             m_ClientIndex, i, Shift, pkHighestAck );
            }

            if( Shift > ACKBITS )
            {
                x_DebugMsg( "****************** FATAL shift for pkt %d: %d\n", i, Shift );
                LOG_ERROR( "conn_mgr::ProcessGameData(ackbits)",
                           "Client:%d -- PacketSeq:%d -- Shift:%d -- DISCONNECTED",
                           m_ClientIndex, i, Shift );
                m_IsConnected = FALSE;

                // There was an error in the packet but we were sure it was for us anyway
                return;
            }

            if( Shift > ACKBITS )
                Ack = FALSE;
            else
                Ack = pkAckBits.Bit( Shift );

            HandlePacketNotify( i, Ack );
        }

        // Remember highest ack
        m_LastAckReceived = pkHighestAck;
    }

    // Tell other managers to dissect packet
    ProcessManagerData( BitStream );
}

//==============================================================================

void conn_mgr::ResetTimeouts( void )
{
    m_ConfirmAliveTimeout = CONFIRM_ALIVE_TIMEOUT;
}

//==============================================================================

void conn_mgr::SendHeartbeat( void )
{
    m_HeartbeatsSent++;
    m_HeartbeatInterval = HEARTBEAT_INTERVAL;

    // If the connection has not received any data for a second, then we start to increment
    // the calculated ping time. This will throttle the connection to the client should no
    // data be getting received. The ping time will be set to the actual average ping when
    // the next heartbeat does get received.
    if (m_ConfirmAliveTimeout < (CONFIRM_ALIVE_TIMEOUT - 1.0f))
    {
        LOG_WARNING( "conn_mgr::SendHeartbeat",
                     "Client:%d -- No packets received, bumping up ping time to %2.02fms",
                     m_ClientIndex, m_AveragePing );
        m_AveragePing += 50.0f;
    }

    m_HeartbeatSentTime[ m_HeartbeatsSent%CONN_NUM_HEARTBEATS ] = (s64)x_GetTime();//tgl.LogicTimeMs;

#if defined(TRACK_HEARTBEATS)
    LOG_MESSAGE( "conn_mgr::SendHeartbeat", "Client:%d -- Sending request %d", m_ClientIndex, m_HeartbeatsSent );
#endif

    netstream BitStream;
    BitStream.Open(CONN_PACKET_IDENTIFIER,CONN_PACKET_HB_REQUEST);

#if defined(ENABLE_DEBUG_TICK)
    m_DebugTicker.ProvideUpdate( BitStream );
#endif

    BitStream.WriteS32( m_HeartbeatsSent );
    if (IsServer())
    {
        BitStream.WriteS32( m_ServerState );
        if( BitStream.WriteFlag( (m_ServerState == STATE_SERVER_COOLDOWN) && 
                                 (m_ClientState != STATE_CLIENT_COOLDOWN) ) )
        {
            LOG_WARNING( "conn_mgr::SendHeartbeat(score)",
                         "Client:%d -- Sent final score -- ClientState:%s -- ServerState:%s",
                         m_ClientIndex, GetStateName(m_ClientState), GetStateName(m_ServerState) );

            GameMgr.ProvideFinalScore( BitStream, m_ClientIndex );
        }
    }
    else
    {
        BitStream.WriteS32( m_ClientState );
    }

    // Done with the bistream, so close it
    BitStream.Close();

    // Encrypt Bitstream
    EncryptBitStream( BitStream );

    // Send the packet
    BitStream.Send(*m_pLocalSocket,m_RemoteAddress);

    {
        //char HBDst[32];
        //m_DstAddress.GetStrIP(HBDst);
        //x_DebugMsg("Heartbeat sent to %s\n",HBDst);
    }
}

//==============================================================================

f32 conn_mgr::GetPing( void )
{
    return m_AveragePing;
}

//==============================================================================

client_state conn_mgr::GetClientState( void )
{
    return m_ClientState;
}

//==============================================================================

server_state conn_mgr::GetServerState( void )
{
    return m_ServerState;
}

//==============================================================================

void conn_mgr::SetClientState( client_state State)
{
    ASSERT(!m_IsServer);
    // If there is a state transition, we zero the heartbeat timer so we
    // can get the state change to the other party as soon as possible.
    // Note: Only the client can change it's state. The server can request
    // state changes through game manager control packets.
    if (m_ClientState != State)
    {
        m_HeartbeatInterval = 0.0f;
    }

    m_ClientState = State;
}

//==============================================================================

void conn_mgr::SetServerState( server_state State)
{
    ASSERT(m_IsServer);
    if (m_ServerState != State)
    {
        m_HeartbeatInterval = 0.0f;
    }
    m_ServerState = State;
}

//==============================================================================

void conn_mgr::WriteManagerData( conn_packet& Packet, netstream& BitStream )
{
    s32 Cursor;

//  s32 NGameManagerBits = 0;
//  s32 NUpdateBits      = 0;
//  s32 NVoiceBits       = 0;

    Packet.NUpdates     = 0;
    Packet.NPains       = 0;
    Packet.GMDirtyBits  = 0x00;
    Packet.GMScoreBits  = 0x00;
    Packet.GMPlayerBits = 0x00;

    if( !m_IsConnected )
        return;

    Cursor = BitStream.GetCursor();

    // This is temporary until we get the correct mechanism
    // for sending voice over the network.
#if defined(TARGET_XBOX)
    {
        // Write port # to be used for voice data
        BitStream.WriteS16( m_VoiceSocket.GetPort() );

        netstream BS;
        // The first two bytes will be 0 in the control stream. This tells the xbox
        // packet send layer that there are no encrypted blocks of data within this
        // VDP packet. The provide update function will deal with padding out this
        // control data when necessary.
        BS.WriteS16(2);
        if( m_pVoiceProxy )
        {
            m_pVoiceProxy->ProvideUpdate( BS );
        }
        else
        {
            g_VoiceMgr.ProvideUpdate( BS );
        }

        if( m_RemoteVoiceAddress.IsEmpty()==FALSE )
        {
            m_VoiceSocket.Send( m_RemoteVoiceAddress, BS );
        }
    }
#else
    if( m_pVoiceProxy )
    {
        m_pVoiceProxy->ProvideUpdate( BitStream );
    }
    else
    {
        g_VoiceMgr.ProvideUpdate( BitStream );
    }
#endif

    m_Stats.BitsVoiceMgr += BitStream.GetCursor() - Cursor;

#if !defined(X_RETAIL) || defined(X_QA)
    if (m_Stats.MaxBitsVoiceMgr < (BitStream.GetCursor() - Cursor) )
        m_Stats.MaxBitsVoiceMgr = BitStream.GetCursor() - Cursor;
#endif

    Cursor = BitStream.GetCursor();

    // Deal with game manager.
    if( IsServer() )
    {
        m_pGMMgr->ProvideUpdate( Packet, BitStream );
        m_Stats.BitsGameMgr += BitStream.GetCursor() - Cursor;

#if !defined(X_RETAIL) || defined(X_QA)
        if (m_Stats.MaxBitsGameMgr < BitStream.GetCursor() - Cursor) 
            m_Stats.MaxBitsGameMgr = BitStream.GetCursor() - Cursor;
#endif
        Cursor = BitStream.GetCursor();
    }

    // Deal with updates.
    {
        ASSERT( m_pUpdateMgr );

        m_pUpdateMgr->ProvideUpdates( Packet, BitStream, 100 );
        m_Stats.Updates       += Packet.NUpdates;
        m_Stats.BitsUpdateMgr += BitStream.GetCursor() - Cursor;
#if !defined(X_RETAIL) || defined(X_QA)
        m_Stats.BitsPlayerUpdates += Packet.BitsPlayerUpdates;
        m_Stats.BitsGhostUpdates += Packet.BitsGhostUpdates;
        m_Stats.BitsOtherUpdates += Packet.BitsOtherUpdates;

        //update stat max
        if (m_Stats.MaxBitsPlayerUpdates < Packet.BitsPlayerUpdates) 
            m_Stats.MaxBitsPlayerUpdates = Packet.BitsPlayerUpdates;
        if (m_Stats.MaxBitsGhostUpdates < Packet.BitsGhostUpdates)
            m_Stats.MaxBitsGhostUpdates = Packet.BitsGhostUpdates;
        if (m_Stats.MaxBitsOtherUpdates < Packet.BitsOtherUpdates) 
            m_Stats.MaxBitsOtherUpdates = Packet.BitsOtherUpdates;

        if (m_Stats.MaxBitsUpdateMgr < BitStream.GetCursor() - Cursor) 
            m_Stats.MaxBitsUpdateMgr = BitStream.GetCursor() - Cursor;
        if (m_Stats.MaxUpdates < Packet.NUpdates) 
            m_Stats.MaxUpdates = Packet.NUpdates;
#endif
        Cursor = BitStream.GetCursor();
    }

    // Pain queue.
    if( !IsServer() )
    {
        m_pPainQueue->ProvidePain( Packet, BitStream, CONN_PAINS_PER_PACKET );
        m_Stats.Pains         += Packet.NPains;
        m_Stats.BitsPainQueue += BitStream.GetCursor() - Cursor;

#if !defined(X_RETAIL) || defined(X_QA)
        if (m_Stats.MaxBitsPainQueue < BitStream.GetCursor() - Cursor) 
            m_Stats.MaxBitsPainQueue = BitStream.GetCursor() - Cursor;
        if (m_Stats.MaxPains < Packet.NPains) 
            m_Stats.MaxPains = Packet.NPains;
#endif

        Cursor = BitStream.GetCursor();
    }

    // Messages.
    if( IsServer() )
    {
        MsgMgr.ProvideMsg( Packet.Seq, BitStream, m_ClientIndex );
    }

    m_Stats.Packets   += 1;
    m_Stats.BitsTotal += BitStream.GetCursor();

#if !defined(X_RETAIL) || defined(X_QA)
    if (m_Stats.MaxBitsTotal < BitStream.GetCursor()) 
        m_Stats.MaxBitsTotal = BitStream.GetCursor();
#endif

//  m_Stats[m_StatsIndex].UpdatesSent += Packet.NUpdates;
//  m_Stats[m_StatsIndex].BytesSent   += BitStream.GetNBytesUsed();
//  m_Stats[m_StatsIndex].PacketsSent += 1;

    //LOG_MESSAGE("conn_mgr::WriteManagerData","V:%d, GM:%d, MM:%d Update:%d, Total:%d",NVoiceBits,NGameManagerBits,NMoveBits,NUpdateBits,BitStream.GetCursor());
}

//==============================================================================

void conn_mgr::ProcessManagerData( netstream& BitStream )
{
    if( !m_IsConnected )
        return;

#if defined(TARGET_XBOX)
    {
        netstream   BS;
        xbool       HasData;
        net_address Remote;
        s16         HeaderLength;
        s16         VoicePort;

        BitStream.ReadS16( VoicePort );
        m_RemoteVoiceAddress.Setup( m_RemoteAddress.GetIP(), VoicePort );

        HasData = m_VoiceSocket.Receive( Remote, BS );
        while( HasData )
        {
        
            if( Remote==m_RemoteVoiceAddress )
            {
                BS.ReadS16( HeaderLength );
                if( IsServer() )
                {
                    ASSERT( m_pVoiceProxy );
                    m_pVoiceProxy->AcceptUpdate( BS );
                }
                else
                {
                    ASSERT( m_pVoiceProxy == NULL );
                    g_VoiceMgr.AcceptUpdate( BS );
                }
            }
            HasData = m_VoiceSocket.Receive( Remote, BS );
        }
    }
#else
    if( IsServer() )
    {
        ASSERT( m_pVoiceProxy );
        m_pVoiceProxy->AcceptUpdate( BitStream );
    }
    else
    {
        ASSERT( m_pVoiceProxy == NULL );
        g_VoiceMgr.AcceptUpdate( BitStream );
    }
#endif

    if( !IsServer() )
    {
        m_pGMMgr->AcceptUpdate( BitStream );
    }

    m_pUpdateMgr->AcceptUpdates( BitStream );

    if( IsServer() )
    {
        m_pPainQueue->AcceptPain( BitStream );
    }

    // Messages.
    if( !IsServer() )
    {
        MsgClient.AcceptMsgs( BitStream );  
    }
}

//==============================================================================
/*
void conn_mgr::AdvanceStats( void )
{
    // Move to next stats
    m_StatsIndex = (m_StatsIndex+1)%CONN_NUM_STATS;

    // Clear stats
    m_Stats[m_StatsIndex].LifeSent    = 0;
    m_Stats[m_StatsIndex].UpdatesSent = 0;
    m_Stats[m_StatsIndex].MovesSent   = 0;
    m_Stats[m_StatsIndex].BytesSent   = 0;
    m_Stats[m_StatsIndex].PacketsSent = 0;
}
*/
//==============================================================================
/*
void conn_mgr::DumpStats( X_FILE* fp )
{
    conn_stats S;
    x_memset(&S,0,sizeof(conn_stats));

    // Sum up all collected stats
    for( s32 i=0; i<CONN_NUM_STATS; i++ )
    {
        S.BytesSent += m_Stats[i].BytesSent;
        S.LifeSent  += m_Stats[i].LifeSent;
        S.PacketsSent += m_Stats[i].PacketsSent;
        S.UpdatesSent += m_Stats[i].UpdatesSent;
    }

    if( fp == NULL )
    {
        x_DebugMsg("Connmanager stats:\n");
        x_DebugMsg("Packets   %5d\n",S.PacketsSent);
        x_DebugMsg("BytesSent %5d  BSPP %5d\n",S.BytesSent,(S.PacketsSent)?(S.BytesSent/S.PacketsSent):(0));
        x_DebugMsg("LifeSent  %5d\n",S.LifeSent);
        x_DebugMsg("Updates   %5d\n",S.UpdatesSent);
    }
    else
    {
        x_fprintf(fp,"Connmanager stats:\n");
        x_fprintf(fp,"Packets   %5d\n",S.PacketsSent);
        x_fprintf(fp,"BytesSent %5d  BSPP %5d\n",S.BytesSent,(S.PacketsSent)?(S.BytesSent/S.PacketsSent):(0));
        x_fprintf(fp,"LifeSent  %5d\n",S.LifeSent);
        x_fprintf(fp,"Updates   %5d\n",S.UpdatesSent);
    }
}
*/
//==============================================================================

void conn_mgr::Send(netstream& BitStream)
{
    m_pLocalSocket->Send(m_RemoteAddress,BitStream);
}

//==============================================================================

void conn_mgr::SetSessionID( s32 ID )
{
    s32 LastSession;

    LastSession = GetSessionID();
    ClearConnection();
    m_LoginSessionID = (ID & 0xff);
    LOG_WARNING( "conn_mgr::SetSessionID", "Client:%d, ********* SESSION ID: %d", m_ClientIndex, GetSessionID() );
    if( (GetSessionID() != (LastSession+1)) && (LastSession != -1) )
    {
        LOG_WARNING( "conn_mgr::SetSessionID", "Session ID changed but not as expected. Old:%d, New:%d", LastSession, GetSessionID() );
    }

}

//==============================================================================

s32 conn_mgr::GetSessionID( void )
{
    return m_LoginSessionID;
}

//==============================================================================

f32 conn_mgr::GetShipInterval(void)
{
    return m_PacketShipInterval;
}

//==============================================================================

void conn_mgr::UpdateShipInterval(void)
{
    f32 interval;

    
    interval = (m_LastPing / 2.0f) / 1000.0f;

    if (interval < MIN_SHIP_INTERVAL)
        interval = MIN_SHIP_INTERVAL;
    if (interval > MAX_SHIP_INTERVAL)
        interval = MAX_SHIP_INTERVAL;

    if ( interval < m_PacketShipInterval)
    {
        m_PacketShipInterval -= (SHIP_INTERVAL_INCREMENT/3);
    }
    if ( interval > m_PacketShipInterval)
    {
        m_PacketShipInterval += (SHIP_INTERVAL_INCREMENT/3);
    }

    if (m_LastPing > 600.0f)
    {
        m_PacketShipInterval = MIN_SHIP_INTERVAL;
    }
}

//==============================================================================

void conn_mgr::Update(f32 DeltaTime)
{
    if (!m_IsConnected || m_pLocalSocket->IsEmpty())
    {
        m_IsConnected = FALSE;
        return;
    }

    // If we got a very high delta time, then this would be due to either
    // debugging or level loading, so we just reset all the timeouts just
    // to be safe. DeltaTimes should never be greater than 100ms.
    // But that would all depend on game frame rate.


    if (DeltaTime > 0.5f)
    {
        ResetTimeouts();
        DeltaTime = 0.0f;
    }

#if defined(ENABLE_DEBUG_TICK)
    m_DebugTicker.Update(DeltaTime);
#endif

    m_HeartbeatInterval -= DeltaTime;
    if (m_HeartbeatInterval < 0.0f)
    {
        SendHeartbeat();
    }

    m_ConfirmAliveTimeout -= DeltaTime;
    if ( m_ConfirmAliveTimeout < 0.0f)
    {
        m_IsConnected = FALSE;
        LOG_WARNING( "conn_mgr::Update",
                     "Client:%d -- ConfirmAlive timeout -- DISCONNECTED",
                     m_ClientIndex );
        return;
    }

    UpdateShipInterval();

    // Stat processing.
    m_StatsClock += DeltaTime;
    if( m_StatsClock > 3.0f )
    {
        if( m_IsServer )
        {
            LOG_MESSAGE( "conn_mgr::StatsPerSec", 
                "Client:%d - Packets:%d - Bytes:%d - Updates(%d):%d - Voice:%d - Game:%d", 
                m_ClientIndex, 
                (s32)( m_Stats.Packets           / m_StatsClock),
                (s32)((m_Stats.BitsTotal>>3)     / m_StatsClock),
                (s32)( m_Stats.Updates           / m_StatsClock),
                (s32)((m_Stats.BitsUpdateMgr>>3) / m_StatsClock),
                (s32)((m_Stats.BitsVoiceMgr>>3)  / m_StatsClock),
                (s32)((m_Stats.BitsGameMgr >>3)  / m_StatsClock) );
        }
        else
        {
            LOG_MESSAGE( "conn_mgr::StatsPerSec", 
                "Client:%d - Packets:%d - Bytes:%d - Updates(%d):%d - Pains(%d):%d - Voice:%d", 
                m_ClientIndex, 
                (s32)( m_Stats.Packets           / m_StatsClock),
                (s32)((m_Stats.BitsTotal>>3)     / m_StatsClock),
                (s32)( m_Stats.Updates           / m_StatsClock),
                (s32)((m_Stats.BitsUpdateMgr>>3) / m_StatsClock),
                (s32)( m_Stats.Pains             / m_StatsClock),
                (s32)((m_Stats.BitsPainQueue>>3) / m_StatsClock),
                (s32)((m_Stats.BitsVoiceMgr>>3)  / m_StatsClock) );
        }

#if !defined(X_RETAIL) || defined(X_QA)
        //backup stats for onscreen reporting
        m_StatsDebug.Packets                = m_Stats.Packets;
        m_StatsDebug.Updates                = m_Stats.Updates;
        m_StatsDebug.Pains                  = m_Stats.Pains;
        m_StatsDebug.BitsUpdateMgr          = m_Stats.BitsUpdateMgr;
        m_StatsDebug.BitsPainQueue          = m_Stats.BitsPainQueue;
        m_StatsDebug.BitsVoiceMgr           = m_Stats.BitsVoiceMgr;
        m_StatsDebug.BitsGameMgr            = m_Stats.BitsGameMgr;
        m_StatsDebug.BitsTotal              = m_Stats.BitsTotal;
        m_StatsDebug.BitsPlayerUpdates      = m_Stats.BitsPlayerUpdates;
        m_StatsDebug.BitsGhostUpdates       = m_Stats.BitsGhostUpdates;
        m_StatsDebug.BitsOtherUpdates       = m_Stats.BitsOtherUpdates;
        m_StatsDebug.MaxBitsPlayerUpdates   = m_Stats.MaxBitsPlayerUpdates;
        m_StatsDebug.MaxBitsGhostUpdates    = m_Stats.MaxBitsGhostUpdates;
        m_StatsDebug.MaxBitsOtherUpdates    = m_Stats.MaxBitsOtherUpdates;
        m_StatsDebug.MaxBitsUpdateMgr       = m_Stats.MaxBitsUpdateMgr;
        m_StatsDebug.MaxBitsPainQueue       = m_Stats.MaxBitsPainQueue;
        m_StatsDebug.MaxBitsVoiceMgr        = m_Stats.MaxBitsVoiceMgr;
        m_StatsDebug.MaxBitsGameMgr         = m_Stats.MaxBitsGameMgr;
        m_StatsDebug.MaxBitsTotal           = m_Stats.MaxBitsTotal;
        m_StatsDebug.MaxUpdates             = m_Stats.MaxUpdates;
        m_StatsDebug.MaxPains               = m_Stats.MaxPains;
        m_StatsClockDebug                   = m_StatsClock;
#endif    

        m_StatsClock = 0.0f;
        x_memset( &m_Stats, 0, sizeof(m_Stats) );
    }
}

//------------------------------------------------------------------------------

void conn_mgr::QueueVoice(const byte* pData, s32 Length)
{
    (void)pData;
    (void)Length;
    ASSERT(FALSE);
    /*
    if (Length)
    {
        LOG_MESSAGE("conn_mgr::QueueVoice","Queued %d bytes to client %d",Length, m_ClientIndex);
    }
    m_VoiceFifo.Insert(pData,Length);
    */
}

//==============================================================================

void conn_mgr::SetEncryption( xbool bEnableEncryption )
{
    random Random;
    s32 Seed;

    m_bEnableEncryption = bEnableEncryption;
    LOG_MESSAGE("conn_mgr::SetEncryption","Encryption has been %s", bEnableEncryption?"enabled":"disabled" );

    if( bEnableEncryption )
    {
        // Encryption key based on session id
        Seed = GetSessionID();

    }
    else
    {
        // Encryption key based on server version
        Seed = SERVER_VERSION;
    }
    m_EncryptionKey[0] = (Random.rand() << 16) | Random.rand();
    m_EncryptionKey[1] = (Random.rand() << 16) | Random.rand();
    m_EncryptionKey[2] = (Random.rand() << 16) | Random.rand();
    m_EncryptionKey[3] = (Random.rand() << 16) | Random.rand();
    LOG_MESSAGE( "conn_mgr::SetEncryption", "Seed:0x%08x, Encryption key set to 0x%08x:%08x:%08x:%08x", Seed, 
                            m_EncryptionKey[0], m_EncryptionKey[1], m_EncryptionKey[2], m_EncryptionKey[3] );
}
