//==============================================================================
//
//  GameClientStates.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
//==============================================================================
// STATE MACHINE MANAGEMENT (CLIENT SIDE)
//==============================================================================
//==============================================================================

#include "x_Files.hpp"
#include "x_Color.hpp"
#include "e_Network.hpp"
#include "x_bitstream.hpp"
#include "GameServer.hpp"
#include "GameClient.hpp"
#include "GameMgr.hpp"
#include "MatchMgr.hpp"
#include "MsgMgr.hpp"
#include "x_log.hpp"
#include "ServerVersion.hpp"
#include "Voice/VoiceMgr.hpp"
#include "StateMgr/StateMgr.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
void game_client::SetState( client_state NewState )
{
    // Action to take when transitioning out of one state to another.
    if (NewState != m_State)
    {
        LOG_MESSAGE( "game_client::SetState", "Old:%s, New:%s, Server:%s", GetStateName(m_State), GetStateName(NewState), GetStateName(m_ConnMgr.GetServerState()) );
        LOG_FLUSH();

        ExitState ( m_State  );
        EnterState( NewState );
        m_StateTime = 0.0f;
        m_Retries   = 3;
    }

    // Tell the connection manager our state changed. This will
    // get mirrored on the server side client proxy.
    m_ConnMgr.SetClientState( m_State );
    LOG_FLUSH();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
void game_client::EnterState( client_state NewState )
{
    s32 i;

    switch (NewState)
    {
    //--------------------------------------------------------------------------
    case STATE_CLIENT_INIT:
        GameMgr.Init();
        for( i = 0; i < m_LocalPlayerCount; i++ )
        {
            m_LocalPlayerSlot[i] = -1;
        }

        //
        // Startup all the various managers
        //

        // XBox requires the voice manager to be initialized differently because
        // Headset must be usable during the frontend menus for voice attachments.
        #ifndef TARGET_XBOX
        g_VoiceMgr.Init( FALSE, TRUE );
        #endif

//      m_UpdateMgr.Init( m_ClientIndex );  // Don't know the ClientIndex yet!
        m_GMMgr.Init();
        m_ConnMgr.Init( *m_pLocal,
                        m_RemoteServer,
                        &m_UpdateMgr,
                        &m_PainQueue,
                        &m_GMMgr,
                        NULL,
                        m_ClientIndex,  // THE CLIENT INDEX IS NOT YET VALID!  WATSON!
                        FALSE,
                        "TESTKEY",
                        FALSE );

        SetExitReason( GAME_EXIT_CONTINUE );
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_LOGIN:
        SendLoginRequest();
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_REQUEST_MISSION:
        SendMissionRequest();
        break;

        //--------------------------------------------------------------------------
    case STATE_CLIENT_VERIFY_MISSION:
        SendVerifyRequest();
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_LOAD_MISSION:
        // This will also make sure the buddy status information is updated as to
        // which map is playing.
        g_MatchMgr.SetUserStatus( BUDDY_STATUS_INGAME );
        m_MissionLoaded = FALSE;
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_SYNC:
        m_InSync = FALSE;
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_INGAME:
        GameMgr.BeginGame();
        // store the first local player slot for use later
        g_StateMgr.SetLocalPlayerSlot( m_LocalPlayerSlot[0] );
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_COOLDOWN:
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_DISCONNECT:
        LOG_WARNING( "game_client::EnterState",
                     "Client: %d -- Reason: %s -- Disconnecting",
                     m_ConnMgr.GetClientIndex(), GetExitReasonName(GetExitReason()) );
        m_ConnMgr.Disconnect( GetExitReason() );
        //
        // This prevents the higher layer connection loss detection code from kicking in
        //
        m_Connected = FALSE;
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_KILL:
        m_ConnMgr.Kill();
        m_GMMgr.Kill();
        m_UpdateMgr.Kill();

        m_Connected = FALSE;

        #ifndef TARGET_XBOX
        g_VoiceMgr.Kill();
        #endif
        GameMgr.Kill();
        break;

    //--------------------------------------------------------------------------
    default:
        break;
    }

    m_State = NewState;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
void game_client::UpdateState(f32 DeltaTime)
{
    s32             i;
    server_state    ServerState = m_ConnMgr.GetServerState();
    (void)DeltaTime;

    switch(m_State)
    {
        //----------------------------------------------------------------------
    case STATE_CLIENT_IDLE:
        break;
        //----------------------------------------------------------------------
    case STATE_CLIENT_INIT:
        SetState( STATE_CLIENT_LOGIN );
        break;

        //----------------------------------------------------------------------
        // Start a login request. IsLoggedIn will be set by ReceivePacket when
        // we receive a login acknowledgement.
    case STATE_CLIENT_LOGIN:
        if( IsLoggedIn() )
        {
            SetState( STATE_CLIENT_REQUEST_MISSION );
        }
        else
        {
            if( GetExitReason() != GAME_EXIT_CONTINUE )
            {
                SetState( STATE_CLIENT_DISCONNECT );
            }

            if( m_StateTime > 2.0f )
            {
                m_Retries--;
                if( m_Retries > 0 )
                {
                    SendLoginRequest();
                    m_StateTime = 0.0f;
                }
                else
                {
                    // We have to abort now!
                    SetExitReason( GAME_EXIT_CONNECTION_LOST );
                    SetState( STATE_CLIENT_DISCONNECT );
                }
            }
        }
        break;


        //----------------------------------------------------------------------
    case STATE_CLIENT_REQUEST_MISSION:
        // We transition out of this state when a map response is received.
        if( m_StateTime > 2.0f )
        {
            m_Retries--;
            if( m_Retries >= 0 )
            {
                SendMissionRequest();
                m_StateTime = 0.0f;
            }
            else
            {
                SetExitReason( GAME_EXIT_CONNECTION_LOST );
                SetState( STATE_CLIENT_DISCONNECT );
            }
        }
        break;

        //----------------------------------------------------------------------
case STATE_CLIENT_VERIFY_MISSION:
        // We transition out of this state when a verify response is received.
        if( m_StateTime > 2.0f )
        {
            m_Retries--;
            if( m_Retries >= 0 )
            {
                SendVerifyRequest();
                m_StateTime = 0.0f;
            }
            else
            {
                if( GetExitReason()==GAME_EXIT_CONTINUE )
                {
                    SetExitReason( GAME_EXIT_CONNECTION_LOST );
                }
                SetState( STATE_CLIENT_DISCONNECT );
            }
        }
        break;

        //----------------------------------------------------------------------
    case STATE_CLIENT_LOAD_MISSION:
        if( IsMissionLoaded() )
        {
            SetState( STATE_CLIENT_VERIFY_MISSION );
        }
        else if( IsConnected()==FALSE )
        {
            if( GetExitReason()==GAME_EXIT_CONTINUE )
            {
                SetExitReason( GAME_EXIT_CONNECTION_LOST );
            }
            SetState( STATE_CLIENT_DISCONNECT );
        }

        break;

        //----------------------------------------------------------------------
    case STATE_CLIENT_SYNC:
        //**** SYNC STATE TRANSFER CHECK
        // We are in sync with the server if (1) the server is in-game, and
        // (2) objects exist in the world for all local players.
        {
            m_InSync = TRUE;
            for( i=0; i<m_LocalPlayerCount; i++ )
            {
                netobj* pPlayer = NetObjMgr.GetObjFromSlot( m_LocalPlayerSlot[i] );
                if( !pPlayer )
                {
                    m_InSync = FALSE;
                    break;
                }
            }
        }

        if( IsInSync() )
        {
            SetState( STATE_CLIENT_INGAME );
        }
        else if( IsConnected()==FALSE )
        {
            if( GetExitReason()==GAME_EXIT_CONTINUE )
            {
                SetExitReason( GAME_EXIT_CONNECTION_LOST );
            }
            SetState( STATE_CLIENT_DISCONNECT );
        }
        else if( ((ServerState != STATE_SERVER_INGAME) && (ServerState!=STATE_SERVER_SYNC)) && (ServerState!=STATE_SERVER_LOAD_MISSION) )
        {
            LOG_WARNING( "game_client::UpdateState", "Noticed that we need to reload. ServerState:%s", GetStateName(ServerState) );
            SetExitReason( GAME_EXIT_RELOAD_LEVEL );
            m_UpdateMgr.Reset();
            m_PainQueue.Reset();
            NetObjMgr.Clear();
            SetState( STATE_CLIENT_COOLDOWN );
        }
        break;

        //----------------------------------------------------------------------
    case STATE_CLIENT_INGAME:

        // If the server is no longer in-game, then we cool down before
        // we do anything else.
        // Note that the server will go from INGAME to IDLE when it notices all the clients are no longer in game. This
        // can happen if a client has to re-load a mission due to a mismatch in map id.
        if( IsConnected()==FALSE )
        {
            if( GetExitReason()==GAME_EXIT_CONTINUE )
            {
                SetExitReason( GAME_EXIT_CONNECTION_LOST );
            }
            SetState( STATE_CLIENT_COOLDOWN );
            return;
        }

        if( (ServerState!=STATE_SERVER_INGAME) && (ServerState!=STATE_SERVER_IDLE) )
        {
            LOG_MESSAGE("game_client::UpdateState","Server left INGAME state, new state %s",GetStateName(m_ConnMgr.GetServerState()));

            SetState( STATE_CLIENT_COOLDOWN );
        }

        break;
        //----------------------------------------------------------------------
    case STATE_CLIENT_COOLDOWN:
        // Wait until the server has decided where to go next.
        if( (m_LastServerState != m_ConnMgr.GetServerState()) || (IsConnected()==FALSE) )
        {
            LOG_MESSAGE("game_client::UpdateState","CLIENT_COOLDOWN, ServerState:%s",GetStateName(m_ConnMgr.GetServerState()));
            m_LastServerState = m_ConnMgr.GetServerState();
        }

        if( (ServerState==STATE_SERVER_LOAD_MISSION) || 
            (ServerState==STATE_SERVER_SYNC) )
        {
            SetState( STATE_CLIENT_REQUEST_MISSION );
            return;
        }

        // The server *may* actually still be in-game. This can only happen if the local player
        // quit the game. In that case, we just advance to the disconnect phase.

        if( (ServerState!=STATE_SERVER_COOLDOWN) && (ServerState!=STATE_SERVER_IDLE)  )
        {
            if( GetExitReason()==GAME_EXIT_CONTINUE )
            {
                SetExitReason( GAME_EXIT_CONNECTION_LOST );
            }
            SetState( STATE_CLIENT_DISCONNECT );
            return;
        }

        if( IsConnected()==FALSE )
        {
            if( GetExitReason()==GAME_EXIT_CONTINUE )
            {
                SetExitReason( GAME_EXIT_CONNECTION_LOST );
            }
            SetState( STATE_CLIENT_DISCONNECT );
        }
        break;
        //----------------------------------------------------------------------
    case STATE_CLIENT_DISCONNECT:
        if( !IsConnected() || (m_StateTime > 0.5f) )
        {
            SetState(STATE_CLIENT_KILL);
        }
        break;

        //----------------------------------------------------------------------
    case STATE_CLIENT_KILL:
        SetState( STATE_CLIENT_IDLE );
        break;

    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
void game_client::ExitState( client_state OldState )
{
    switch(OldState)
    {

    //--------------------------------------------------------------------------
    case STATE_CLIENT_IDLE:
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_INIT:
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_LOGIN:
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_LOAD_MISSION:
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_SYNC:
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_INGAME:
        m_ConnMgr.SetSessionID( m_ConnMgr.GetSessionID() + 1 );

        switch( GetExitReason() )
        {
        case GAME_EXIT_SERVER_SHUTDOWN:
        case GAME_EXIT_SERVER_BUSY:
        case GAME_EXIT_NETWORK_DOWN:
        case GAME_EXIT_DUPLICATE_LOGIN:
        case GAME_EXIT_PLAYER_QUIT:
        case GAME_EXIT_CONNECTION_LOST:
        case GAME_EXIT_PLAYER_DROPPED:
        case GAME_EXIT_PLAYER_KICKED:
        case GAME_EXIT_FOLLOW_BUDDY:
            GameMgr.EndGame();
            break;
        case GAME_EXIT_ADVANCE_LEVEL:
        case GAME_EXIT_RELOAD_CHECKPOINT:
        case GAME_EXIT_RELOAD_LEVEL:
        case GAME_EXIT_CONTINUE:
            break;
        default:
            ASSERT( FALSE );
            break;
        }

        m_UpdateMgr.Reset();
        m_PainQueue.Reset();
        NetObjMgr.Clear();
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_COOLDOWN:
        break;

    //--------------------------------------------------------------------------
    case STATE_CLIENT_DISCONNECT:
        g_MatchMgr.SetUserStatus( BUDDY_STATUS_ONLINE );
        break;

    default:
        break;
    }
}
