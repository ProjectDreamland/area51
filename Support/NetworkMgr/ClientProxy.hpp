//=========================================================================
//
//  ClientProxy.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//=========================================================================
//
//  Server side proxy for a client.
//
//=========================================================================

#ifndef CLIENTPROXY_HPP
#define CLIENTPROXY_HPP

//=========================================================================

#include "x_files.hpp"
#include "e_Network.hpp"
#include "UpdateMgr.hpp"
#include "PainQueue.hpp"
#include "GMMgr.hpp"
#include "Voice/VoiceProxy.hpp"
//=========================================================================

class client_proxy
{
public:
                            client_proxy        ( void );
                           ~client_proxy        ( void );

        void                Init                ( net_socket*           pLocalSocket,
                                                  const net_address&    Remote,      
                                                  s32                   ClientIndex, 
                                                  const byte*           pUniqueId,   
                                                  const byte*           pTicket,
                                                  const char*           pEncryptionKey,
                                                  xbool                 EnableEncryption );

      //void                Init                ( s32 PlayerCount, const byte* UniqueId, const char* Ticket );
        void                Kill                ( void );
        void                Update              ( f32 DeltaTime, server_state ServerState);
        void                Clear               ( void );
        xbool               ProvideUpdate       ( void );
        xbool               ReceivePacket       ( netstream& BitStream );
        xbool               IsConnected         ( void )                    { return m_IsConnected; }
        void                Disconnect          ( exit_reason Reason );
  const net_address&        GetAddress          ( void );
        client_state        GetState            ( void )                    { return m_State;       }
        xbool               IsInGame            ( void );
        xbool               HasMapLoaded        ( void );
        void                QueueVoice          ( const byte* pBuffer, s32 Length );
  const byte*               GetUniqueId         ( void );
  const char*               GetTicket           ( void )                    { return m_Ticket;      }
        xbool               KickPlayer          ( s32 PlayerId );
        void                AddPlayer           ( s32 Index, const char* pName, u64 Identifier);
        s32                 GetPlayerSlot       ( s32 Index );
        s32                 GetPlayerCount      ( void )                    { return m_LocalPlayerCount; }
        s32                 ReadFromVoiceFifo   ( byte* pBuffer, s32 MaxLength );
        void                WriteToVoiceFifo    ( const byte* pBuffer, s32 Length );


private:
        exit_reason         GetExitReason       ( void );               
        void                SetExitReason       ( exit_reason Reason ); 
        void                EnterState          ( client_state NewState );
        void                UpdateState         ( f32 DeltaTime );
        void                ExitState           ( client_state OldState );
        void                SetState            ( client_state NewState );
        void                ProcessMissionRequest( netstream& BitStream );
        void                ProcessVerifyRequest( netstream& BitStream );   
        void                ProcessEndMission   ( netstream& BitStream );
        void                DumpSecurityResponse( netstream& BitStream );

        xbool               m_IsConnected;                              // Does the proxy think it's connected?
        s32                 m_ClientIndex;                              // Client index of this proxy
        exit_reason         m_ExitReason;                               // This is the mirror of the client's exit reason
        client_state        m_State;                                    // State of the remote client
        s32                 m_LocalPlayerCount;                         // Number of local players
        f32                 m_PacketSendDelay;                          // Time until next packet goes?
        conn_mgr            m_ConnMgr;
        update_mgr          m_UpdateMgr;
        pain_queue          m_PainQueue;
        gm_mgr              m_GMMgr;
        voice_proxy         m_VoiceProxy;
        byte                m_UniqueId[NET_MAX_ID_LENGTH];
        char                m_Ticket[128];
        s32                 m_LocalPlayerSlot[NET_MAX_PER_CLIENT];
        char                m_PlayerNames[NET_MAX_PER_CLIENT][NET_NAME_LENGTH];
        u64                 m_PlayerIdentifier[NET_MAX_PER_CLIENT];

//      object::id          PlayerObjID[2];
    //s32                 PlayerIndex[2];    // Index in GameMgr.
    //f32                 PacketShipDelay;
    //f32                 HeartbeatDelay;

    friend class game_server;

#if defined( ENABLE_DEBUG_MENU )
#if !defined(X_RETAIL) || defined(X_QA)
    friend class debug_menu_page_multiplayer;
#endif
#endif
};

//=========================================================================
#endif
//=========================================================================

