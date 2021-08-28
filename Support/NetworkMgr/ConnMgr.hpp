//==============================================================================
//
//  ConnMgr.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
//
// Revision History:
//  5/31/03 BW - Initial revision of conn manager code from T:AA
//
//==============================================================================

#ifndef CONNMANAGER_HPP
#define CONNMANAGER_HPP

#include "x_files.hpp"
#include "e_Network.hpp"
#include "Network/netstream.hpp"
#include "GameState.hpp"
#include "Configuration/GameConfig.hpp"

#if !defined(TARGET_XBOX)
#define ENABLE_ENCRYPTION
#endif

#include "DebugTicker.hpp"

#define DEBUG_TICK_SOCKET   24576

//==============================================================================
// MANAGERS
//==============================================================================

class update_mgr;
class pain_queue;
class gm_mgr;
class voice_proxy;

//==============================================================================
// The conn manager deals with packets coming in from a client or going out
// to a client. It will pass the packet off to the master server manager should
// it not be a conn manager handled packet (or if the checksum fails). The first
// two bytes are a CRC based checksum followed by 2 byte CONN_PACKET_IDENTIFIER, 
// then the request type. The checksum is performed on the remainder
// of the packet, since we should know whether or not the header is valid.
//
// Notes:
// Time within the conn manager is dealt with in xticks which is a 64 bit quantity
// which should 'time out' in a few millenia instead of a couple of days (servers
// may be running for a lot longer than 2 or 3 days).
// An xtick is of the same duration as with the x_timer class, regardless of
// whether or not the underlying game system delta time is based off xticks.
//==============================================================================

enum conn_packet_type
{
            CONN_PACKET_FIRST          = 0,
            CONN_PACKET_LOGIN_REQUEST   ,
            CONN_PACKET_LOGIN_RESPONSE  ,        
            CONN_PACKET_HB_REQUEST      ,
            CONN_PACKET_HB_RESPONSE     ,
            CONN_PACKET_GAME_DATA       ,
            CONN_PACKET_MISSION_REQUEST ,
            CONN_PACKET_MISSION_RESPONSE,
            CONN_PACKET_VERIFY_REQUEST  ,
            CONN_PACKET_VERIFY_RESPONSE ,
            CONN_PACKET_END_MISSION     ,
            CONN_PACKET_GAME_STAGE      ,
            CONN_PACKET_DISCONNECT      ,
            CONN_PACKET_INSYNC          ,
            CONN_PACKET_CS              ,
            CONN_PACKET_SECURITY_RESPONSE,
            CONN_PACKET_SECURITY_REQUEST,
            CONN_PACKET_LAST            ,
};

const   u16                 CONN_PACKET_IDENTIFIER  = 0xf00d;
const   s32                 CONN_NUM_HEARTBEATS     =   8;
const   s32                 CONN_NUM_PACKETS        = 128;
const   s32                 CONN_NUM_STATS          =  32;
const   s32                 CONN_LIVES_PER_PACKET   =   8;
const   s32                 CONN_PAINS_PER_PACKET   =   7;

const   s32                 PACKET_LOSS_HISTORY     =  16;

// Time before a client is declared dead (10 seconds of no packets)
const   f32                 CONFIRM_ALIVE_TIMEOUT   = 10.0f;
// Maximum ship interval of 250ms, minimum ship
// interval of 100ms.
const   f32                 MAX_SHIP_INTERVAL       = (1.0f /  4.0f);
const   f32                 MIN_SHIP_INTERVAL       = (1.0f / 10.0f);
// Default ship interval of 100ms
const   f32                 DEFAULT_SHIP_INTERVAL   = MIN_SHIP_INTERVAL;
const   f32                 HEARTBEAT_INTERVAL      = 0.5f;                            
const   f32                 SHIP_INTERVAL_INCREMENT = (1.0f / 100.0f);
// Threshold at which ship intervals will be modified
const   f32                 LOW_PING_THRESHOLD      = (290.0f);
const   f32                 HIGH_PING_THRESHOLD     = (310.0f);
const   f32                 LOGIN_PING_CUTOFF       = (600.0f);

//==============================================================================

#define ACKSLOTS    8
#define ACKBITS     (ACKSLOTS * 16)

//------------------------------------------------------------------------------

class ack_bits
{
private:
        u16                 Bits[ ACKSLOTS ];

public:
                            ack_bits            ( void );
        void                Clear               ( void );
        void                Print               ( void );
        void                ShiftLeft           ( s32 Shift );
        xbool               Bit                 ( s32 Position );
        void                Set                 ( s32 Position );
        void                Read                ( netstream& BS );
        void                Write               ( netstream& BS );
};

//==============================================================================

struct conn_packet
{
        // Packet info
        s32                 Seq;
        s16                 TargetClient;   // -1 for server, 0-31 otherwise

        // Update manager update info
        s16                 FirstUpdate;
        s16                 NUpdates;

        // Pain queue data
        s16                 PainID  [ CONN_PAINS_PER_PACKET ];
        s16                 NPains;

        // Game manager dirty bits
        u32                 GMDirtyBits;
        u32                 GMPlayerBits;
        u32                 GMScoreBits;

#if !defined(X_RETAIL) || defined(X_QA)
        s32                 BitsPlayerUpdates;
        s32                 BitsGhostUpdates;
        s32                 BitsOtherUpdates;
#endif
};

//==============================================================================

struct conn_stats
{
        s32                 Packets;
        s32                 Updates;
        s32                 Pains;
        s32                 BitsUpdateMgr;
        s32                 BitsPainQueue;
        s32                 BitsVoiceMgr;
        s32                 BitsGameMgr;
        s32                 BitsTotal;
#if !defined(X_RETAIL) || defined(X_QA)
        s32                 BitsPlayerUpdates;
        s32                 BitsGhostUpdates;
        s32                 BitsOtherUpdates;

        s32                 MaxBitsPlayerUpdates;
        s32                 MaxBitsGhostUpdates;
        s32                 MaxBitsOtherUpdates;
        s32                 MaxBitsUpdateMgr;
        s32                 MaxBitsPainQueue;
        s32                 MaxBitsVoiceMgr;
        s32                 MaxBitsGameMgr;
        s32                 MaxBitsTotal;
        s32                 MaxUpdates;
        s32                 MaxPains;
#endif
};

//==============================================================================

class conn_mgr
{


public:
                            conn_mgr                ( void );
                           ~conn_mgr                ( void );

        void                Init                    ( net_socket&           Local,
                                                      const net_address&    Remote,
                                                      update_mgr*           pUpdateMgr,
                                                      pain_queue*           pPainQueue,
                                                      gm_mgr*               pGMMgr,
                                                      voice_proxy*          pVoiceProxy,
                                                      s32                   ClientIndex,
                                                      xbool                 IsServer,
                                                      const char*           BFKey,
                                                      xbool                 bEnableEncryption );
                            
        void                Kill                    ( void );
        void                Update                  ( f32 DeltaTime );

        void                SetSessionID            ( s32 ID );
        s32                 GetSessionID            ( void );
                            
        void                ClearConnection         ( void );
        void                Disconnect              ( exit_reason Reason );
                            
        // returns TRUE if packet was parsed, false if not for the conn manager
        xbool               ReceivePacket           ( netstream& BitStream );
        xbool               SendPacket              ( void );
        void                AdvanceStats            ( void );
        void                DumpStats               ( X_FILE* pFile = NULL );
        xbool               DecryptBitStream        ( netstream& BitStream );
        void                EncryptBitStream        ( netstream& BitStream );

        xbool               IsConnected             ( void )        { return m_IsConnected; };
        xbool               IsServer                ( void )        { return m_IsServer;    };
                            
        f32                 GetPing                 ( void );
        f32                 GetPacketLoss           ( void );
        f32                 GetShipInterval         ( void );
        net_address&        GetAddress              ( void )        { return m_RemoteAddress; };
        f32                 GetConnectTime          ( void )        { return (f32)x_TicksToSec( x_GetTime()-m_ConnectTime );  };
        void                ResetTimeouts           ( void );

        void                SetClientIndex          ( s32 Index )   { m_ClientIndex = Index; }
        s32                 GetClientIndex          ( void )        { return m_ClientIndex; }
        client_state        GetClientState          ( void );
        void                SetClientState          ( client_state State );
        server_state        GetServerState          ( void );
        void                SetServerState          ( server_state State );
        void                SetEncryption           ( xbool bEnableEncryption);
        void                Send                    ( netstream& BitStream );
        void                QueueVoice              ( const byte* pData, s32 Length );

//      void                BuildPingGraph          ( xbitmap &Bitmap );

#if !defined(X_RETAIL) || defined(X_QA)
    f32                 m_StatsClockDebug;
    conn_stats          m_StatsDebug;
#endif                            

private:
        net_socket*         m_pLocalSocket;
        net_address         m_RemoteAddress;
        s32                 m_ClientIndex;
        f32                 m_ConfirmAliveTimeout;
                            
        s32                 m_LoginSessionID;
                            
        s32                 m_LastSeqReceived;
        s32                 m_LastSeqSent;
        s32                 m_LastAckReceived;
        ack_bits            m_AckBits;
                            
        s32                 m_PacketsReceived;
        s32                 m_PacketsUnusable;
        s32                 m_PacketsSent;
        s32                 m_PacketsDropped;
                            
        xbool               m_IsConnected;
        xbool               m_IsInitialized;
        xbool               m_IsServer;
                            
        xtick               m_ConnectTime;
        s32                 m_HeartbeatsSent;
        xtick               m_HeartbeatSentTime[CONN_NUM_HEARTBEATS];
        f32                 m_HeartbeatInterval;
        f32                 m_HeartbeatPing[CONN_NUM_HEARTBEATS];
        s32                 m_LastHeartbeatRequest;
                            
        f32                 m_AveragePing;
        client_state        m_ClientState;
        server_state        m_ServerState;
                            
        conn_packet         m_Packet[CONN_NUM_PACKETS];
        s16                 m_PacketAck[PACKET_LOSS_HISTORY];
        s16                 m_PacketNak[PACKET_LOSS_HISTORY];
        xtimer              m_PacketLossTimer;
        s32                 m_PacketLossIndex;
        f32                 m_PacketShipInterval;
        f32                 m_LastPing;
                            
        update_mgr*         m_pUpdateMgr;
        pain_queue*         m_pPainQueue;
        gm_mgr*             m_pGMMgr;
        voice_proxy*        m_pVoiceProxy;
#if defined(TARGET_XBOX)
        net_socket          m_VoiceSocket;
        net_address         m_RemoteVoiceAddress;
#endif

//      s32                 m_StatsIndex;
        f32                 m_StatsClock;
        conn_stats          m_Stats;
        xbool               m_DoLogging;
        X_FILE*             m_LogOutgoing;
        X_FILE*             m_LogIncoming;

        debug_ticker        m_DebugTicker;

        void                HandlePacketNotify          ( s32 PacketID, xbool Received );
        void                WriteManagerData            ( conn_packet& Packet, netstream& BitStream );
        void                SendHeartbeat               ( void );

        void                ProcessGameData             ( netstream& BitStream );
        void                ProcessManagerData          ( netstream& BitStream );
        void                ProcessHeartbeatRequest     ( netstream& BitStream );
        void                ProcessHeartbeatResponse    ( netstream& BitStream );

        void                UpdateShipInterval          ( void );
        xbool               m_bEnableEncryption;
        u32                 m_EncryptionKey[4];
};

//==============================================================================
#endif
//==============================================================================

