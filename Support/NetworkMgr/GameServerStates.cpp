//==============================================================================
//
//  GameServerStates.cpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================
// STATE MACHINE MANAGEMENT (SERVER SIDE)
//==============================================================================
//==============================================================================

#include "GameServer.hpp"
#include "ServerVersion.hpp"
#include "MatchMgr.hpp"
#include "MsgMgr.hpp"
#include "GameMgr.hpp"
#include "NetworkMgr.hpp"
#include "Voice/VoiceMgr.hpp"
#include "ClientProxy.hpp"
#include "StateMgr/StateMgr.hpp"

#include "x_log.hpp"

void game_server::SetState( server_state State )
{
    if( State != m_State )
    {
        LOG_MESSAGE( "game_server::SetState",
                     "State transition from %s to %s",
                     GetStateName(m_State), GetStateName(State) );
        LOG_FLUSH();
        ExitState ( m_State );
        EnterState( State   );
    }
    LOG_FLUSH();
}

//==============================================================================
//------------------------------------------------------------------------------
// This function is called when a state transition has been detected from the
// client. This performs all server-side initialization required on a state
// transition.
//==============================================================================
void game_server::EnterState( server_state NewState )
{
    s32 i;
    s32 PlayerIndex=0;
    m_StateTime = 0.0f;

    switch( NewState )
    {

    //-------------------------------------------------
    case STATE_SERVER_IDLE:
        break;

    //-------------------------------------------------
    case STATE_SERVER_INIT:

        // Prepare the profile list so they map to local players 
        g_StateMgr.SetupProfileListIndex();
        GameMgr.Init();

        // XBox requires the voice manager to be initialized differently because
        // Headset must be usable during the frontend menus for voice attachments.
        #ifndef TARGET_XBOX
        g_VoiceMgr.Init( TRUE, (m_MaxClients > 0) );
        #endif

        SetExitReason( GAME_EXIT_CONTINUE );

        // Must set the gametype before the local player connects. Even though it is being set later (that will take
        // precident if a game type changes).
        GameMgr.SetGameType( g_ActiveConfig.GetGameTypeID() );
        GameMgr.SetMaxPlayers( g_ActiveConfig.GetMaxPlayerCount() );
        GameMgr.SetMapScaling( g_ActiveConfig.IsMapScalingEnabled() );

        m_pClients = new client_proxy[m_MaxClients];
        for( i=0; i<m_MaxClients; i++ )
        {
            m_pClients[i].Clear();
        }

        for( i=0; i<m_LocalPlayerCount; i++ )
        {
            xwstring PlayerName;

#ifdef TARGET_XBOX
            if( g_NetworkMgr.IsOnline() )
            {
                PlayerName = xwstring(g_MatchMgr.GetActiveUserAccount().Name);
            }
            else
            {
                PlayerName = xwstring(g_StateMgr.GetProfileName(g_StateMgr.GetProfileListIndex(i)));
            }
#else            
            PlayerName = xwstring(g_StateMgr.GetProfileName(g_StateMgr.GetProfileListIndex(i)));
#endif

            // Create the server player.
            GameMgr.Connect( PlayerIndex, 
                             -1, 
                             g_MatchMgr.GetPlayerIdentifier(i),
                             PlayerName,
                             g_StateMgr.GetActiveProfile( g_StateMgr.GetProfileListIndex(i) ).GetAvatarID(),
                             FALSE );

/*
                             g_OnLinePlayerProfile.m_OnlineName,
                             g_OnLinePlayerProfile.m_Skin,
                             g_OnLinePlayerProfile.m_AlternateSkin );
*/

            m_LocalPlayerSlot[i] = PlayerIndex;
        }
//      g_VoipMgr.SetActive( !g_MatchMgr.GetActiveConfig().VoiceDisabled );
//      g_MatchMgr.GetActiveConfig().Players = m_LocalPlayerCount;
//      SetFriendlyFire( g_NetworkMgr.GetMatchMgr().GetActiveConfig().FriendFire );

        break;

    //-------------------------------------------------
    case STATE_SERVER_LOAD_MISSION:
        SetExitReason( GAME_EXIT_CONTINUE );
        //
        // Copy all necessary information from the active configuration to the gamemgr class
        //
        GameMgr.SetGameType( g_ActiveConfig.GetGameTypeID() );
        GameMgr.SetMutationMode( g_ActiveConfig.GetMutationMode() );
        GameMgr.SetMapScaling( g_ActiveConfig.IsMapScalingEnabled() );

        if( GameMgr.GetGameType() == GAME_CAMPAIGN )
        {
            GameMgr.SetClockMode( 0 );
        }
        else
        {
            if( g_ActiveConfig.GetGameTime() > 0 )
            {
                GameMgr.SetClockLimit( g_ActiveConfig.GetGameTime() );
                GameMgr.SetClockMode( -1 ); 
            }
            else
            {
                GameMgr.SetClockMode( 0 );
            }
        }

        GameMgr.SetTeamDamage( (f32)g_ActiveConfig.GetFirePercent() / 100.0f );

        GameMgr.SetVotePercent( g_ActiveConfig.GetVotePercent() );

        GameMgr.SetScoreLimit( g_ActiveConfig.GetScoreLimit() );
        GameMgr.SetMaxPlayers( g_ActiveConfig.GetMaxPlayerCount() );

        m_MissionLoaded = FALSE;
        break;

    //-------------------------------------------------
    case STATE_SERVER_SYNC:
        break;

    //-------------------------------------------------
    case STATE_SERVER_INGAME:
        g_MatchMgr.SetUserStatus( BUDDY_STATUS_INGAME );
        GameMgr.BeginGame();
        break;

    //-------------------------------------------------
    case STATE_SERVER_COOLDOWN:
        break;

    //-------------------------------------------------
    case STATE_SERVER_SHUTDOWN:
        // Every player must exit the game and disconnect.
        for( i=0; i<m_LocalPlayerCount; i++ )
        {
            const game_score& Score=GameMgr.GetScore();
            if( Score.Player[m_LocalPlayerSlot[i]].IsConnected )
            {
                if( Score.Player[m_LocalPlayerSlot[i]].IsInGame )
                {
                    GameMgr.ExitGame  ( m_LocalPlayerSlot[i] );
                }
                GameMgr.Disconnect( m_LocalPlayerSlot[i] );
                m_LocalPlayerSlot[i]=-1;
            }
        }
        // All remaining clients must disconnect.
        for( i=0; i<m_MaxClients; i++ )
        {
            if( m_pClients[i].IsConnected() )
            {
                m_pClients[i].Disconnect( GAME_EXIT_SERVER_SHUTDOWN );
            }
        }
        g_MatchMgr.SetUserStatus( BUDDY_STATUS_ONLINE );
        // And now we can kill the net object manager.
        // This must occur AFTER the clients are killed.
        NetObjMgr.Clear();
        break;

    //-------------------------------------------------
    case STATE_SERVER_KILL:
        delete[] m_pClients;
        m_pClients   = NULL;
        m_MaxClients = 0;

        #ifndef TARGET_XBOX
        g_VoiceMgr.Kill();
        #endif
        GameMgr.Kill();

        break;

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
void game_server::ExitState(server_state OldState)
{
    s32 i;

    switch(OldState)
    {
    //-------------------------------------------------
    case STATE_SERVER_IDLE:
        break;

    //-------------------------------------------------
    case STATE_SERVER_INIT:
        for( i = 0; i < m_LocalPlayerCount; i++ )
        {
            GameMgr.EnterGame( m_LocalPlayerSlot[i] );
        }
        // store the first local player slot for use later
        g_StateMgr.SetLocalPlayerSlot( m_LocalPlayerSlot[0] );

        // Now that the server has started up, we can update its game manager's
        // voice peripheral status directly.
        ASSERT( g_NetworkMgr.IsServer() == TRUE );
        GameMgr.SetVoiceAllowed( 0, !g_VoiceMgr.IsVoiceBanned()  );
        GameMgr.SetVoiceCapable( 0,  g_VoiceMgr.IsVoiceCapable() );
        break;

    //-------------------------------------------------
    case STATE_SERVER_LOAD_MISSION:
        break;

    //-------------------------------------------------
    case STATE_SERVER_SYNC:
        break;

    //-------------------------------------------------
    case STATE_SERVER_INGAME:
        // We know that we need to end the game when there is a connection loss, or
        // the player has decided to quit the game.

        switch( GetExitReason() )
        {
        case GAME_EXIT_ADVANCE_LEVEL:
        case GAME_EXIT_GAME_COMPLETE:
            break;
        case GAME_EXIT_RELOAD_CHECKPOINT:
        case GAME_EXIT_RELOAD_LEVEL:
        case GAME_EXIT_SERVER_SHUTDOWN:
        case GAME_EXIT_SERVER_BUSY:
        case GAME_EXIT_NETWORK_DOWN:
        case GAME_EXIT_DUPLICATE_LOGIN:
        case GAME_EXIT_PLAYER_QUIT:
        case GAME_EXIT_CONNECTION_LOST:
        case GAME_EXIT_CANNOT_CONNECT:
        case GAME_EXIT_PLAYER_DROPPED:
        case GAME_EXIT_FOLLOW_BUDDY:
            GameMgr.EndGame();
            break;
        default:
            ASSERT(FALSE);
            break;
        }
        ASSERT( GameMgr.GameInProgress() == FALSE );
        break;

    //-------------------------------------------------
    case STATE_SERVER_COOLDOWN:
        NetObjMgr.Clear();
        RelaxBanList();
        break;

    //-------------------------------------------------
    case STATE_SERVER_SHUTDOWN:
        RelaxBanList();
        break;

    //-------------------------------------------------
    case STATE_SERVER_KILL:
        break;

    //-------------------------------------------------
    default:
        ASSERT( FALSE );
        break;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
// This updates the current state.
//
//==============================================================================
void game_server::UpdateState(f32 DeltaTime)
{
    s32     i;
    xbool   AllShutdown;
    xbool   AllInSync;

    m_StateTime += DeltaTime;
    switch( m_State )
    {
    //-------------------------------------------------
    case STATE_SERVER_IDLE:
        break;

    //-------------------------------------------------
    case STATE_SERVER_INIT:
        SetState( STATE_SERVER_LOAD_MISSION );        
        break;

    //-------------------------------------------------
    case STATE_SERVER_LOAD_MISSION:
        //
        //*** State transition in to SYNC happens when the map loading operation 
        // is complete. This is done by network_mgr::LoadLevelComplete().
        //
        if( IsMissionLoaded() )
        {
            SetState( STATE_SERVER_SYNC );
        }

        //
        // If the exit reason changed, that means, typically, the map requested was
        // not found on the local machine.
        //
        if( GetExitReason() != GAME_EXIT_CONTINUE )
        {
            SetState( STATE_SERVER_SHUTDOWN );
        }
        break;

    //-------------------------------------------------
    case STATE_SERVER_SYNC:
        // Go through all clients and see if they are in the SYNC phase. This will
        // be more complex later.
        AllInSync = TRUE;
        for( i=0; i< m_MaxClients; i++ )
        {
            if( m_pClients[i].IsConnected() )
            {
                // Clients will 'be in game' if they have synced once with the server. This allows us
                // to kick off all clients at the same time.
                if( m_pClients[i].IsInGame() && (m_pClients[i].GetState() != STATE_CLIENT_SYNC) )
                {
                    AllInSync = FALSE;
                    break;
                }
            }
        }

        // We only stay in the sync phase for a maximum of 10 seconds.
        if( AllInSync || (m_StateTime > 10.0f) )
        {
            // This will inform all clients to jump in to the game.
            SetState(STATE_SERVER_INGAME);
        }
        break;
    //-------------------------------------------------
    case STATE_SERVER_INGAME:
        if( GameMgr.GameInProgress()==FALSE )
        {
            SetState( STATE_SERVER_COOLDOWN );
        }
        break;

    //-------------------------------------------------
    case STATE_SERVER_COOLDOWN:
        // Go through the clients, if they are all disconnected, then we
        // can exit this state.
        //
        // We don't need to 
        AllShutdown = TRUE;
        for( i=0; i<m_MaxClients; i++ )
        {
            client_state ClientState;

            if( m_pClients[i].IsConnected() )
            {
                ClientState = m_pClients[i].GetState();
                // If the client is in the ingame state, then it
                // has not shut down (the client CAN be in the loadmap state
                // which will be detected by the client)
                if( ClientState == STATE_CLIENT_INGAME )
                {
                    AllShutdown = FALSE;
                }
            }
        }

        if( AllShutdown )
        {
            SetState( STATE_SERVER_IDLE );
        }
        else
        {
            if( m_StateTime > 5.0f )
            {
                LOG_WARNING( "game_server::UpdateState", "Took too long for clients all to cooldown. Server forced." );
            }
        }
        break;

    //-------------------------------------------------
    case STATE_SERVER_SHUTDOWN:
        AllShutdown = TRUE;
        for( i=0; i<m_MaxClients; i++ )
        {
            if( m_pClients[i].IsConnected() )
            {
                AllShutdown = FALSE;
            }
        }

        if( AllShutdown || (m_StateTime > 0.5f) )
        {
            SetState(STATE_SERVER_IDLE);
        }

    //-------------------------------------------------
    case STATE_SERVER_KILL:
        SetState( STATE_SERVER_IDLE );
        break;

    //-------------------------------------------------
    default:
        ASSERT( FALSE );
        break;
    }
}

//------------------------------------------------------------------------------
exit_reason game_server::GetExitReason( void )
{
    return g_ActiveConfig.GetExitReason();
}

//------------------------------------------------------------------------------
void game_server::SetExitReason( exit_reason Reason )
{
    LOG_MESSAGE( "game_server::SetExitReason","Reason:%s", GetExitReasonName(Reason) );

    g_ActiveConfig.SetExitReason( Reason );
    if( GetState() == STATE_SERVER_INGAME )
    {
        SetState(STATE_SERVER_COOLDOWN);
    }
}

