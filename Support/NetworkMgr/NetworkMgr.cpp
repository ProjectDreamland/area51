//==============================================================================
//
//  NetworkMgr.cpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../Apps/GameApp/Main.hpp"
#include "x_files.hpp"
#include "Entropy.hpp"
#include "NetworkMgr.hpp"
#include "ServerVersion.hpp"
#include "NetLimits.hpp"

#include "GameMgr.hpp"
#include "GameState.hpp"
#include "Configuration/GameConfig.hpp"
#include "GameServer.hpp"
#include "GameClient.hpp"
#include "Voice/VoiceMgr.hpp"
#include "GameLib/LevelLoader.hpp"
#include "StateMgr/StateMgr.hpp"
#include "StateMgr/MapList.hpp"

#if defined(TARGET_PS2)
#include "e_Audio.hpp"
#include "ps2/iopmanager.hpp"
#include "IOManager/io_mgr.hpp"
#include "Audio/audio_hardware.hpp"
#include "e_Memcard.hpp"
#include "MemCardMgr/MemCardMgr.hpp"
#endif

//==============================================================================
//  DEFINITIONS
//==============================================================================

network_mgr g_NetworkMgr;
s32 g_ServerVersion = SERVER_VERSION;
//==============================================================================
//  TYPES
//==============================================================================



//==============================================================================
//  FUNCTIONS
//==============================================================================

void network_mgr::LoadMissionComplete( void )
{
    ASSERT( g_StateMgr.IsBackgroundThreadRunning() == FALSE );
    if( m_pServer )
    {
        m_pServer->LoadComplete();
    }

    if( m_pClient )
    {
        m_pClient->LoadComplete();
    }
}

//==============================================================================

network_mgr::network_mgr( void )
{
    m_LocalSocket.Clear();
    m_IsOnline  = FALSE;
    m_pServer   = NULL;
    m_pClient   = NULL;

    g_PendingConfig.Invalidate();
    g_ActiveConfig.Invalidate();
    //
    // Set the server information to sane values. These *SHOULD* be overwritten
    // by the game configuration functions.
    //
    g_PendingConfig.SetEliminationMode( FALSE );
    g_PendingConfig.SetScoreLimit( -1 );
    g_PendingConfig.SetGameTime( 900 );
    g_PendingConfig.SetSystemID( 0 );
    g_PendingConfig.SetLevelID( 0 );
    g_PendingConfig.SetMaxPlayerCount( 16 );
    g_PendingConfig.SetVersion( g_ServerVersion );
    g_PendingConfig.SetFirePercent( 75 );
    g_PendingConfig.SetVotePercent( 75 );
    g_PendingConfig.SetVoiceEnabled( TRUE );
    g_PendingConfig.SetMutationMode( MUTATE_CHANGE_AT_WILL );
    g_PendingConfig.SetGameType( xwstring("") );
    g_PendingConfig.SetPassword( "" );
    g_PendingConfig.SetServerName( g_StateMgr.GetActiveSettings().GetHostSettings().m_ServerName );
}

//==============================================================================
network_mgr::~network_mgr( void )
{
}

//==============================================================================
void network_mgr::Init( void )
{
#if defined(TARGET_XBOX)
    //klkl: need to initialize network matchmgr early so silent signin 
    //can happen before we enter the main menu
    net_Init();
#endif
    //net_Init();
}

//==============================================================================
void network_mgr::Kill( void )
{
#if defined(TARGET_XBOX)
    g_MatchMgr.Kill();
    net_Kill();
#endif
}

//==============================================================================
void network_mgr::SetOnline( xbool IsOnline )
#if defined ( TARGET_PC )
{
}
#endif
#if defined ( TARGET_XBOX )
{
    if( m_IsOnline && !IsOnline )
    {
        if( !m_LocalSocket.IsEmpty() )
        {
            m_LocalSocket.Close();
        }
#if defined(TARGET_PS2)
        g_MatchMgr.Kill();
        net_Kill();
#else
        g_MatchMgr.SetState( MATCH_IDLE );
#endif
    }

#if defined(TARGET_PS2)
    if( m_IsOnline != IsOnline )
    {
#if !defined(TARGET_DEV)
        g_RscMgr.Unload( "DX_FrontEnd.audiopkg"    );
        g_RscMgr.Unload( "SFX_FrontEnd.audiopkg"   );
        g_RscMgr.Unload( "MUSIC_FrontEnd.audiopkg" );

        g_AudioMgr.UnloadAllPackages();
        g_AudioMgr.Kill();
        g_LevelLoader.UnmountDefaultFilesystems();
        
        // get current state of memcards
        g_UIMemCardMgr.Repoll( this, &network_mgr::OnRepollCB );
        g_UIMemCardMgr.Update(0.001f);
        while( !g_UIMemCardMgr.IsActionDone() )
        {
            // wait for memcard to finish polling
            x_DelayThread(1);
        }
        
        g_MemcardHardware.Kill();
        g_IoMgr.Kill();
        input_Kill();
        g_IopManager.Reboot(TRUE, FALSE, "DNAS300.IMG");
        input_Init();
        g_MemcardHardware.Init();
        
        // immediately remount the card to clear an unwanted card changed error code
        g_UIMemCardMgr.RebootCheck();
        g_UIMemCardMgr.Update(0.001f);
        while( !g_UIMemCardMgr.IsActionDone() )
        {
            // wait for remount to complete
            x_DelayThread(1);
        }
#endif
    }

    if( !m_IsOnline && IsOnline )
    {
        net_Init();
        //net_Kill();
        //net_Init();
#if defined(TARGET_XBOX)
        interface_info Info;
        net_GetInterfaceInfo( -1, Info );
        if( Info.NeedsServicing )
        {
            net_Kill();
            net_Init();
        }
#endif
    }

    // The audio packages have to be loaded after the network is initialized as that
    // will push more data off to the EE side. This is where we can define the memory
    // sizes for campaign versus multiplayer
    //
    if( m_IsOnline != IsOnline )
    {
#if !defined(TARGET_DEV)
        g_IoMgr.Init();
        g_LevelLoader.MountDefaultFilesystems();

        g_AudioMgr.Init( 5512*1024 );
        g_RscMgr.Load( PRELOAD_FILE("DX_FrontEnd.audiopkg"    ) );
        g_RscMgr.Load( PRELOAD_FILE("SFX_FrontEnd.audiopkg"   ) );
        g_RscMgr.Load( PRELOAD_FILE("MUSIC_FrontEnd.audiopkg" ) );
        // initialize audio volumes from global settings
        global_settings& Settings = g_StateMgr.GetActiveSettings();
        Settings.CommitAudio();
#endif
    }
#endif

    net_SetVersionKey( g_ServerVersion );
    m_IsOnline = IsOnline;
}
#endif
//==============================================================================
void network_mgr::Update( f32 DeltaTime )
{
    xbool           Used = FALSE;
    net_address     Remote;
    netstream       BitStream;
    exit_reason     ExitReason;
#if defined(TARGET_PS2)
    extern xbool g_RebootPending;

    if( g_RebootPending )
    {
        scePrintf("<<<POWEROFF SILENCED>>>\n");
        g_StateMgr.Reboot( REBOOT_HALT );
    }
#endif

    if( DeltaTime > 0.5f )
    {
        LOG_WARNING("network_mgr::Update","Long time between networkmgr updates. %2.02fsec", DeltaTime );
    }
    
    // Are we paused in a split screen game?
    xbool bIsSplitScreen       = ( GetLocalPlayerCount() > 1 );
    xbool bIsPaused            = g_StateMgr.IsPaused();    
    xbool bIsSplitScreenPaused = bIsSplitScreen && bIsPaused;
    
    // Run game manager logic?
    if( ( bIsSplitScreenPaused == FALSE ) && ( GameMgr.GameInProgress() ) )
    {
        GameMgr.Logic( DeltaTime );
        NetObjMgr.Logic( DeltaTime );
    }

    g_VoiceMgr.Update( DeltaTime );
//  SmokeMgr.Update( DeltaTime );

#if defined(TARGET_XBOX)
    // PS2 has it's matchmgr in a seperate thread. There is no good reason for
    // doing this on xbox and it may actually add some additional complexity
    // that we just simply don't need.
    g_MatchMgr.Update( DeltaTime );
#endif
    s32 MaxCount;

    MaxCount = 0;
    ExitReason = g_ActiveConfig.GetExitReason();

    if( m_IsOnline && (m_LocalSocket.IsEmpty()==FALSE) )
	#if defined ( TARGET_PC )
    {
    }
    #endif
    #if defined ( TARGET_XBOX )
    {
        // Verify that the network connection didn't just get yanked from
        // beneath us (cable disconnect, dropped etc).
        interface_info Info;

        net_GetInterfaceInfo( -1, Info );
        if( (Info.Address == 0) || (Info.IsAvailable==FALSE) )
        {
            //ClearConnection();
            g_ActiveConfig.SetExitReason( GAME_EXIT_NETWORK_DOWN );
        }

        // We need to check here to see if the network was yanked from beneath us. This can only happen when something else
        // sets network down or duplicate login. We close the local socket which, in turn, will cascade through the various
        // state machines and cause them to shutdown. For example, the connmgr will notice the socket is closed then disconnect
        // himself which will cause the server or client to shut itself down.
        if( (m_LocalSocket.IsEmpty()==FALSE) && ( (ExitReason==GAME_EXIT_NETWORK_DOWN)||(ExitReason==GAME_EXIT_DUPLICATE_LOGIN) ) )
        {
            m_LocalSocket.Close();
        }
        else
        {
            xtimer t;
            t.Start();

            while( m_LocalSocket.Receive(Remote,BitStream) )
            {
                //LOG_MESSAGE( "network_mgr::Update", "Packet received from %s, length %d", Remote.GetStrAddress(), BitStream.GetNBytes() );
                Used = FALSE;
                if( m_pServer )
                {
                    ASSERT( m_pClient==NULL );
                    Used = m_pServer->ReceivePacket( Remote, BitStream );
                }

                if( m_pClient )
                {
                    ASSERT( m_pServer==NULL );
                    Used = m_pClient->ReceivePacket( Remote, BitStream );
                }

                // If the client or server didn't consume it, give it to the match manager.
                if( !Used )
                {
                    g_MatchMgr.ReceivePacket( Remote, BitStream );
                }

                MaxCount++;
                if( MaxCount > 20 )
                {
                    LOG_WARNING( "network_mgr::Update", "Jumped out of receive loop. MaxCount:%d, Time:%2.02fms", MaxCount, t.ReadMs() );
                    break;
                }
            }
        }
    }
#endif
    ExitReason = GAME_EXIT_CONTINUE;
    // Update the client or server.
    if( m_pServer )
    {
        ASSERT( m_pClient==NULL );
        
        if( (m_pServer->GetState() == STATE_SERVER_INGAME) &&
            (GameMgr.GameInProgress() == FALSE) )
        {
            if( GameMgr.GetGameType() == GAME_CAMPAIGN )
            {
                // force exit condition
                const map_entry* pEntry = g_MapList.GetByIndex(g_StateMgr.GetLevelIndex());

                // check if this was the last map
                if( pEntry->GetMapID() == 1130 )
                {
                    // game is complete
                    g_ActiveConfig.SetExitReason( GAME_EXIT_GAME_COMPLETE );
                }
                else
                {
                    // advance to the next level
                    g_ActiveConfig.SetExitReason( GAME_EXIT_ADVANCE_LEVEL );
                }
            }
            else
            {
                // advance to the next level
                g_ActiveConfig.SetExitReason( GAME_EXIT_ADVANCE_LEVEL );
            }
        }
        ExitReason = m_pServer->Update( DeltaTime );
    }

    if( m_pClient )
    {
        ASSERT(m_pServer == NULL);
        ExitReason = m_pClient->Update( DeltaTime );
    }

    if( (g_ActiveConfig.GetExitReason() == GAME_EXIT_CONTINUE) && (ExitReason != GAME_EXIT_CONTINUE) )
    {
        g_ActiveConfig.SetExitReason( ExitReason );
    }

    LOG_FLUSH();
}
//==============================================================================

s32 network_mgr::GetLocalPlayerSlot( s32 Index )
{
    if( m_pServer )
    {
        return m_pServer->GetLocalPlayerSlot( Index );
    }

    if( m_pClient )
    {
        return m_pClient->GetLocalPlayerSlot( Index );
    }

    ASSERT( FALSE );
    return 0;
}

//==============================================================================

s32 network_mgr::GetClientIndex( void )
{
    if( m_pServer )
    {
        return( -1 );
    }

    ASSERT( m_pClient );
    return( m_pClient->GetClientIndex() );
}

//==============================================================================

s32 network_mgr::GetClientIndex( s32 PlayerIndex )
{
    if( m_pServer )
    {
        return m_pServer->GetClientIndex( PlayerIndex );
    }

    ASSERT( m_pClient );

    if( IsLocal( PlayerIndex ) )
    {
        return( m_pClient->GetClientIndex() );
    }

    return( -1 );
}

//==============================================================================

s32 network_mgr::GetClientPlayerSlot( s32 ClientIndex, s32 PlayerIndex )
{
    ASSERT( m_pServer );
    return m_pServer->GetClientPlayerSlot( ClientIndex, PlayerIndex );
}

//==============================================================================

s32 network_mgr::GetClientPlayerCount( s32 ClientIndex )
{
    ASSERT( m_pServer );
    return m_pServer->GetClientPlayerCount( ClientIndex );
}

//==============================================================================

s32 network_mgr::GetLocalSlot( s32 NetSlot )
{
    s32 i;

    if( m_pServer )
    {
        for( i = 0; i < m_LocalPlayerCount; i++ )
        {
            if( m_pServer->GetLocalPlayerSlot( i ) == NetSlot )
            {
                return( i );
            }
        }
    }

    if( m_pClient )
    {
        for( i = 0; i < m_LocalPlayerCount; i++ )
        {
            if( m_pClient->GetLocalPlayerSlot( i ) == NetSlot )
            {
                return( i );
            }
        }
    }

    return( -1 );
}

//==============================================================================

xbool network_mgr::IsLocal( s32 NetSlot )
{
    s32 i;

    if( m_pServer )
    {
        for( i = 0; i < m_LocalPlayerCount; i++ )
        {
            if( m_pServer->GetLocalPlayerSlot( i ) == NetSlot )
            {
                return( TRUE );
            }
        }
    }

    if( m_pClient )
    {
        for( i = 0; i < m_LocalPlayerCount; i++ )
        {
            if( m_pClient->GetLocalPlayerSlot( i ) == NetSlot )
            {
                return( TRUE );
            }
        }
    }

    return( FALSE );
}

//==============================================================================
void network_mgr::BeginLogin( void )
{
    m_pClient = new game_client;
    m_pClient->Init( m_LocalSocket, g_ActiveConfig.GetRemoteAddress(), 1 );
}

//==============================================================================

void network_mgr::BecomeServer( void )
#if defined ( TARGET_PC )
{
}
#endif
#if defined ( TARGET_XBOX )
{
    LOG_APP_NAME( "SERVER" );

    ASSERT( m_pServer == NULL );
    ASSERT( m_pClient == NULL );

    game_config::Commit();
    SetLocalPlayerCount( g_ActiveConfig.GetPlayerCount() );
    
    m_pServer = new game_server;

    // A little hack here. We can tell whether or not we should have any client
    // instances depending on whether or not we have a local network socket since
    // a 'local' game will not have a network socket. This is to make sure that
    // the server instance takes up less space when in an offline game. This will
    // minimize offline memory footprint.
    xbool IsOnline = (g_ActiveConfig.GetPlayerCount() == 1) &&
                     (g_ActiveConfig.GetGameTypeID()  != GAME_CAMPAIGN);


    if( m_LocalSocket.IsEmpty() || (IsOnline == FALSE) )
    {
        m_pServer->Init( m_LocalSocket, g_ActiveConfig.GetPlayerCount(), 0 );
    }
    else
    {
        m_pServer->Init( m_LocalSocket, g_ActiveConfig.GetPlayerCount(), NET_MAX_LOCAL_CLIENTS );
        g_MatchMgr.SetState( MATCH_BECOME_SERVER );
    }

    g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
}
#endif
//==============================================================================
//
void network_mgr::ReenterGame( void )
{
    if( m_pServer )
    {
        ASSERT( m_pClient == NULL );
        m_pServer->SetState( STATE_SERVER_LOAD_MISSION );
    }
    if( m_pClient )
    {
        ASSERT( m_pServer == NULL );
        m_pClient->SetState( STATE_CLIENT_REQUEST_MISSION );
    }
}

//==============================================================================
void network_mgr::Disconnect( void )
#if defined ( TARGET_PC )
{
}
#endif
#if defined ( TARGET_XBOX )
{

    if( m_pServer )
    {
        if( IsOnline() )
        {
            g_MatchMgr.SetState( MATCH_UNREGISTER_SERVER );
        }
        else
        {
            g_MatchMgr.SetState( MATCH_IDLE );
        }
        m_pServer->Kill();
        delete m_pServer;
        m_pServer = NULL;
    }
 
    if( m_pClient )
    {
        g_MatchMgr.SetState( MATCH_IDLE );
        m_pClient->Kill();
        delete m_pClient;
        m_pClient = NULL;
    }
}
#endif
//==============================================================================
void network_mgr::KickPlayer( s32 Index )
{
    ASSERT( m_pServer );
    m_pServer->KickPlayer( Index );
}

//==============================================================================

pain_queue* network_mgr::GetPainQueue( void )
{
    if( m_pClient )
    {
        return( m_pClient->GetPainQueue() );
    }

    return( NULL );
}

//==============================================================================
const char* network_mgr::GetStateName(void)
{
    if( m_pServer )
    {
        return ::GetStateName(m_pServer->GetState());
    }

    if( m_pClient )
    {
        return ::GetStateName(m_pClient->GetState());
    }

    return "<Undefined>";
}

//==============================================================================
xbool network_mgr::IsLoggedIn( void )
{
    ASSERT( m_pClient );

    return m_pClient->IsLoggedIn();
}

//==============================================================================
xbool network_mgr::IsRequestingMission( void )
{
    client_state State;

    // Only a client should call this.
    ASSERT( m_pClient );

    // The client state should only be request mission or load mission. If we get any sort
    // of disconnect message, then there is something wrong with the higher level connection
    // loss detection. Make sure it's catered for.
    State = m_pClient->GetState();
    if( m_pClient->IsConnected()==FALSE )
    {
        g_ActiveConfig.SetExitReason( GAME_EXIT_CONNECTION_LOST );
        return FALSE;
    }
    return (State == STATE_CLIENT_REQUEST_MISSION);
}

//==============================================================================
voice_proxy& network_mgr::GetVoiceProxy( s32 ClientIndex )
{
    ASSERT( m_pServer );
    return m_pServer->GetVoiceProxy( ClientIndex );
}

//==============================================================================
xbool network_mgr::IsServerABuddy( const char* pSearch )
{
    ASSERT( m_pServer );
    return m_pServer->IsServerABuddy( pSearch );
}

//==============================================================================
xbool network_mgr::HasPlayerBuddy( const char* pSearch )
{
    ASSERT( m_pServer );
    return m_pServer->HasPlayerBuddy( pSearch );
}

//==============================================================================
f32 network_mgr::GetClientPing( s32 ClientIndex )
{
    ASSERT( m_pServer );
    return m_pServer->GetClientPing( ClientIndex );
}

//==============================================================================
f32 network_mgr::GetAveragePing( void )
{
    ASSERT( m_pServer );
    return m_pServer->GetAveragePing();
}

//==============================================================================
#ifdef TARGET_PS2
void network_mgr::OnRepollCB( void )
{
    // do nothing!
}
#endif
//==============================================================================
