//==============================================================================
//
//  ClientProxyStates.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
// STATE MACHINE MANAGEMENT (SERVER SIDE CLIENT PROXY)
//==============================================================================
//==============================================================================

#include "ClientProxy.hpp"
#include "MatchMgr.hpp"
#include "GameMgr.hpp"
#include "x_log.hpp"
#include "GameServer.hpp"
#include "MsgMgr.hpp"
#include "NetworkMgr.hpp"
#include "Configuration/GameConfig.hpp"

//==============================================================================
//------------------------------------------------------------------------------
// This function is called when a state transition has been detected from the
// client. This performs all server-side initialization required on a state
// transition.
//==============================================================================
void client_proxy::EnterState(client_state NewState)
{
    s32                 i;

    switch( NewState )
    {
    //-------------------------------------------------
    case STATE_CLIENT_INIT:
        break;

    //-------------------------------------------------
    case STATE_CLIENT_LOAD_MISSION:
        SetExitReason( GAME_EXIT_CONTINUE );
        break;

    //-------------------------------------------------
    case STATE_CLIENT_SYNC:
        m_UpdateMgr.Reset();
        m_UpdateMgr.SyncActivates();
        break;

    //-------------------------------------------------
    case STATE_CLIENT_INGAME:
        break;

    //-------------------------------------------------
    case STATE_CLIENT_COOLDOWN:
        break;

    //-------------------------------------------------
    case STATE_CLIENT_DISCONNECT:
    case STATE_CLIENT_SHUTDOWN:
        for( i = 0; i< m_LocalPlayerCount; i++ )
        {
            if( m_LocalPlayerSlot[i] != -1 )
            {
                GameMgr.ExitGame  ( m_LocalPlayerSlot[i] );
                GameMgr.Disconnect( m_LocalPlayerSlot[i] );
                m_LocalPlayerSlot[i] = -1;
            }
        }

        g_ActiveConfig.SetPlayerCount( g_ActiveConfig.GetPlayerCount() - m_LocalPlayerCount );
        g_PendingConfig.SetPlayerCount( g_ActiveConfig.GetPlayerCount() );

        // We only change state if we were already registered. Otherwise,
        // the lack of registration would mean that timeouts in matchmgr
        // will deal with registration.
        g_MatchMgr.RefreshRegistration();

        if( m_ConnMgr.IsConnected() )
        {
            LOG_WARNING( "client_proxy::EnterState", 
                         "Client:%d -- State:%s -- Disconnecting!",
                         m_ConnMgr.GetClientIndex(), GetExitReasonName(GetExitReason()) );
            m_ConnMgr.Disconnect( GetExitReason() );
        }

        // Note: Killing the conn manager will kill the update manager.
        m_ConnMgr.Kill();
        break;

    //-------------------------------------------------
    default:
        break;
    }
    m_State = NewState;
}

//==============================================================================
//------------------------------------------------------------------------------
// This function is called when state transition has been detected from the client.
// This performs all server-side shutdown on a state transition
//==============================================================================
void client_proxy::ExitState(client_state OldState)
{
    switch( OldState )
    {
    case STATE_CLIENT_INIT:
        break;

    case STATE_CLIENT_SYNC:
        break;

    case STATE_CLIENT_INGAME:
        NetObjMgr.ClientExitGame( m_ClientIndex );
        m_ConnMgr.SetSessionID(m_ConnMgr.GetSessionID()+1);
        m_UpdateMgr.Reset();
        m_PainQueue.Reset();
        break;

    case STATE_CLIENT_COOLDOWN:
        break;

    default:
        break;
    }
}

//------------------------------------------------------------------------------
void client_proxy::SetState(client_state NewState)
{
    LOG_MESSAGE( "client_proxy::SetState",
                 "Client:%d -- OldState:%s -- NewState:%s",
                 m_ConnMgr.GetClientIndex(), 
                 GetStateName(m_State), GetStateName(NewState) );

    LOG_FLUSH();
    if( m_State != NewState )
    {
        ExitState(m_State);
        EnterState(NewState);
    }
    LOG_FLUSH();
}

//==============================================================================
//------------------------------------------------------------------------------
// This updates the current state.
//
//==============================================================================
void client_proxy::UpdateState(f32 DeltaTime)
{
    s32 i;
    const game_score&   Score = GameMgr.GetScore();

    (void)DeltaTime;

    switch (m_State)
    {
    //-----------------------------------------------------
    case STATE_CLIENT_SYNC:
        if( m_ConnMgr.GetServerState() == STATE_SERVER_INGAME )
        {
            for( i = 0; i< m_LocalPlayerCount; i++ )
            {
                if( m_LocalPlayerSlot[i] != -1 )
                {
                    if( !(Score.Player[ m_LocalPlayerSlot[i] ].IsInGame) )
                    {
                        GameMgr.EnterGame( m_LocalPlayerSlot[i] );

                        const game_score&   Score       = GameMgr.GetScore();
                        g_MatchMgr.AddRecentPlayer( Score.Player[ m_LocalPlayerSlot[i] ] );
                    }
                }
            }
        }
        break;

    //-----------------------------------------------------
    case STATE_CLIENT_INGAME:
        break;

    //-----------------------------------------------------
    case STATE_CLIENT_COOLDOWN:
        break;
    //-----------------------------------------------------
    case STATE_CLIENT_DISCONNECT:
        Kill();
        break;
    default:
        break;

    }
}

