//=========================================================================
//
//  GameClient.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//=========================================================================

#ifndef GAMECLIENT_HPP
#define GAMECLIENT_HPP

//=========================================================================

#include "x_Types.hpp"
#include "e_Network.hpp"
#include "ConnMgr.hpp"
#include "UpdateMgr.hpp"
#include "PainQueue.hpp"
#include "GMMgr.hpp"

//=========================================================================

class game_client
{
public:
                            game_client             ( void );
                           ~game_client             ( void );


        void                Init                    ( net_socket& Local, const net_address& Remote, s32 LocalPlayerCount );
        void                Kill                    ( void );
        void                SetClientIndex          ( s32 ClientIndex );
        exit_reason         Update                  ( f32 DeltaTime );

        net_address         GetAddress              ( void );
        s32                 GetClientIndex          ( void );
        s32                 GetLocalPlayerCount     ( void );
        s32                 GetLocalPlayerSlot      ( s32 Player );
        pain_queue*         GetPainQueue            ( void );
        s32                 GetGameLevel            ( void )                    { return m_GameLevel;       }

//    object::id  GetPlayerObjID  ( s32 ID ) const { return( m_PlayerObjID[ID] ); };
        xbool               ReceivePacket           ( const net_address& Remote, netstream& BitStream );
        f32                 GetPing                 ( void );

        void                LoadComplete            ( void )                    { m_MissionLoaded = TRUE;             }
        void                SyncComplete            ( void )                    { m_InSync = TRUE;                    }

        xbool               IsLoggedIn              ( void )                    { return m_LoggedIn;                  }
        xbool               IsInSync                ( void )                    { return m_InSync;                    }
        xbool               IsConnected             ( void )                    { return m_Connected;                 }
        xbool               IsMissionLoaded         ( void )                    { return m_MissionLoaded;             }

        void                UnloadMission           ( void );

        xbool               Disconnect              ( void );

        void                ProcessHeartbeat        ( f32 DeltaSec );

//    void        BuildPingGraph      ( xbitmap &Bitmap ) { m_ConnManager.BuildPingGraph(Bitmap);};

        void                EndMission              ( void );
        void                SetState                ( client_state NewState );
        client_state        GetState                ( void )                    { return m_State;                       }
        server_state        GetServerState          ( void )                    { return m_ConnMgr.GetServerState();    }
        exit_reason         GetExitReason           ( void );
        void                SetExitReason           ( exit_reason Reason );

private:
        void                EnterState              ( client_state NewState );
        void                UpdateState             ( f32 DeltaTime );
        void                ExitState               ( client_state OldState );

        void                SendLoginRequest        ( void );
        void                SendMissionRequest      ( void );
        void                SendVerifyRequest       ( void );

        void                ProcessLoginResponse    ( netstream& BitStream );
        void                ProcessMissionResponse  ( netstream& BitStream );
        void                ProcessVerifyResponse   ( netstream& BitStream );
        void                ProcessMessage          ( netstream& BitStream );
        void                ProcessDisconnect       ( netstream& BitStream );
        void                ProcessSecurityRequest  ( netstream& BitStream );

        net_socket*         m_pLocal;
        net_address         m_RemoteServer;
        client_state        m_State;
        server_state        m_LastServerState;
        s32                 m_Retries;
        f32                 m_StateTime;

        s32                    m_ClientIndex;
                        
        xwchar              m_ServerTitle[32];

        xbool               m_Connected;
        xbool               m_MissionLoaded;
        xbool               m_InSync;
        xbool               m_LoggedIn;

        s32                 m_GameLevel;
        f32                 m_LastConnectCheck;
        f32                 m_PacketSendDelay;

        conn_mgr            m_ConnMgr;
        update_mgr          m_UpdateMgr;
        pain_queue          m_PainQueue;
        gm_mgr              m_GMMgr;

        s32                 m_LocalPlayerCount;
        s32                 m_LocalPlayerSlot[ NET_MAX_PER_CLIENT ];

//    game_event_manager  m_GameEventManager;

#if defined( ENABLE_DEBUG_MENU )
#if !defined(X_RETAIL) || defined(X_QA)
        friend class debug_menu_page_multiplayer;
#endif
#endif
};

const char* GetStateName( client_state State );

//=========================================================================
#endif
//=========================================================================

