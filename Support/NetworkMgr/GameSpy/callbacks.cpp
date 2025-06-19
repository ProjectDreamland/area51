//==============================================================================
//
// Callbacks.cpp - GameSpy callback functions. These will be used when match
// making data is received.
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

#if !defined(TARGET_PS2) && !defined(TARGET_PC)
#error This should only be included for PS2 gamespy support.
#endif

#if !defined(bwatson)
//#define X_SUPPRESS_LOGS
#endif

#include "x_files.hpp"
#include "NetworkMgr/GameMgr.hpp"
#include "e_Network.hpp"
#include "Configuration/GameConfig.hpp"

#include "NetworkMgr/GameSpy/Callbacks.hpp"
#include "NetworkMgr/MatchMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"

#ifdef GSI_UNICODE
    #define _T(a) L##a
    #define _tprintf wprintf
#else
    #define _T(a) a
    #define _tprintf x_DebugMsg
#endif

#if defined(X_LOGGING)
const char* GetName( NegotiateState State )
{
    switch( State )
    {
    case ns_initsent:       return "ns_initsent";           break;
    case ns_initack:        return "ns_initack";            break;
    case ns_connectping:    return "ns_connectping";        break;
    case ns_finished:       return "ns_finished";           break;
    case ns_canceled:       return "ns_canceled";           break;
    default:                return "<unknown>";             break;
    }
}

const char* GetName( NegotiateResult Result )
{
    switch( Result )
    {
    case nr_success:            return "nr_success";            break;
    case nr_deadbeatpartner:    return "nr_deadbeatpartner";    break;
    case nr_inittimeout:        return "nr_inittimeout";        break;
    case nr_unknownerror:       return "nr_unknownerror";       break;
    default:                    return "<unknown>";             break;
    }
}
#endif

extern s32 g_ServerVersion;
//==============================================================================

void gamespy_serverkey(int keyid, qr2_buffer_t outbuf, void *userdata)
{
    match_mgr& MatchMgr = *(match_mgr*)userdata;

    ASSERT( MatchMgr.IsBrowserLocked() );
    switch( keyid )
    {
        //--------------------------------------------------
    case HOSTNAME_KEY:
        LOG_MESSAGE("gamespy_serverkey","Name:%s", (const char*)xstring(g_ActiveConfig.GetServerName()));
        qr2_buffer_add(outbuf, (const char*)xstring(g_ActiveConfig.GetServerName()));
        break;
        //--------------------------------------------------
    case GAMEVER_KEY:
        LOG_MESSAGE("gamespy_serverkey","Version:%d", g_ServerVersion );
        qr2_buffer_add_int(outbuf, g_ServerVersion );
        break;
        //--------------------------------------------------
    case HOSTPORT_KEY:
        LOG_MESSAGE("gamespy_serverkey","HostPort:%d", MatchMgr.m_pSocket->GetPort());
        qr2_buffer_add_int(outbuf, MatchMgr.m_pSocket->GetPort() );
        break;
        //--------------------------------------------------
    case MAPNAME_KEY:
        LOG_MESSAGE("gamespy_serverkey","MapName:%s", (const char*)xstring(g_ActiveConfig.GetLevelName()));
        qr2_buffer_add(outbuf, (const char*)xstring(g_ActiveConfig.GetLevelName()));
        break;
        //--------------------------------------------------
    case LEVEL_KEY:
        LOG_MESSAGE("gamespy_serverkey","Level:%d", g_ActiveConfig.GetLevelID());
        qr2_buffer_add_int( outbuf, g_ActiveConfig.GetLevelID() );
        break;
        //--------------------------------------------------
    case GAMETYPE_KEY:
        LOG_MESSAGE("gamespy_serverkey","GameType:%s", (const char*)xstring(g_ActiveConfig.GetGameType()));
        qr2_buffer_add(outbuf, (const char*)xstring(g_ActiveConfig.GetGameType()));
        break;
    case GAMETYPE_ID_KEY:
        LOG_MESSAGE("gamespy_serverkey","GameTypeId:%d", g_ActiveConfig.GetGameTypeID() );
        qr2_buffer_add_int(outbuf, g_ActiveConfig.GetGameTypeID() );
        break;
        //--------------------------------------------------
    case NUMPLAYERS_KEY:
        LOG_MESSAGE("gamespy_serverkey","NumPlayers:%d", g_ActiveConfig.GetPlayerCount());
        qr2_buffer_add_int(outbuf, g_ActiveConfig.GetPlayerCount());
        break;
        //--------------------------------------------------
    case NUMTEAMS_KEY:
        LOG_MESSAGE("gamespy_serverkey","NumTeams:%d", 1);
        qr2_buffer_add_int(outbuf, 1 );
        break;
        //--------------------------------------------------
    case MAXPLAYERS_KEY:
        LOG_MESSAGE("gamespy_serverkey","MaxPlayers:%d", g_ActiveConfig.GetMaxPlayerCount());
        qr2_buffer_add_int(outbuf, g_ActiveConfig.GetMaxPlayerCount());
        break;
        //--------------------------------------------------
    case MUTATION_MODE_KEY:
        LOG_MESSAGE("gamespy_serverkey","MutationMode:%d", g_ActiveConfig.GetMutationMode());
        qr2_buffer_add_int(outbuf, g_ActiveConfig.GetMutationMode());
        break;
        //--------------------------------------------------
    case PASSWORD_SET_KEY:
        LOG_MESSAGE("gamespy_serverkey","PasswordEnabled:%d", g_ActiveConfig.IsPasswordEnabled());
        qr2_buffer_add_int(outbuf, g_ActiveConfig.IsPasswordEnabled());
        break;
        //--------------------------------------------------
    case VOICE_KEY:
        LOG_MESSAGE("gamespy_serverkey","Voice:%d", g_ActiveConfig.IsVoiceEnabled());
        qr2_buffer_add_int(outbuf, g_ActiveConfig.IsVoiceEnabled());
        break;        
        //--------------------------------------------------
    case GAMEMODE_KEY:
        qr2_buffer_add(outbuf, (const char*)xstring(g_ActiveConfig.GetShortGameType()));
        break;
        //--------------------------------------------------
    case TEAMPLAY_KEY:
        qr2_buffer_add_int(outbuf, 1);
        break;
        //--------------------------------------------------
    case FRAGLIMIT_KEY:
        qr2_buffer_add_int(outbuf, g_ActiveConfig.GetFragLimit() );
        break;
        //--------------------------------------------------
    case TIMELIMIT_KEY:
        qr2_buffer_add_int(outbuf, g_ActiveConfig.GetGameTime() );
        break;
        //--------------------------------------------------
    case FLAGS_KEY:
        qr2_buffer_add_int(outbuf, g_ActiveConfig.GetFlags() );
        break;
        //--------------------------------------------------
    case AVGPING_KEY:
        {
            s32 Ping = (s32)g_NetworkMgr.GetAveragePing();
            if( Ping == 0 )
            {
                Ping = 100;
            }
            qr2_buffer_add_int(outbuf, Ping );
        }
        break;
        //--------------------------------------------------
    case PLAYERS_KEY:
        {
            s32                 i;
            const game_score&   Score = GameMgr.GetScore();
            xstring             Players;
            xbool               NeedSeperator;

            NeedSeperator = FALSE;
            for( i=0; i<NET_MAX_PLAYERS; i++ )
            {
                if( Score.Player[i].IsConnected )
                {
                    if( NeedSeperator )
                    {
                        Players += '|';
                    }
                    Players += Score.Player[i].NName;
                    NeedSeperator = TRUE;
                }
            }
            qr2_buffer_add(outbuf, (const char*)Players );
        }
        break;
        //--------------------------------------------------
    default:
        qr2_buffer_add(outbuf, _T(""));
    }
    
    GSI_UNUSED(userdata);
}

//==============================================================================

void gamespy_playerkey(int keyid, int index, qr2_buffer_t outbuf, void *userdata)
{
    match_mgr& MatchMgr = *(match_mgr*)userdata;
    const game_score& Score = GameMgr.GetScore();
    ASSERT( MatchMgr.IsBrowserLocked() );
    (void)MatchMgr;

    //check for valid index
    if( index >= Score.NPlayers )
    {
        LOG_WARNING("gamespy_playerkey","IGNORED: Index:%d, nPlayers:%d", index, Score.NPlayers);
        qr2_buffer_add(outbuf, _T(""));
        return;
    }

    s32 PlayerIndex;
    s32 Count;

    Count = index;
    for( PlayerIndex=0; PlayerIndex<NET_MAX_PLAYERS; PlayerIndex++ )
    {
        if( Score.Player[PlayerIndex].IsConnected )
        {
            if( Count==0 )
            {
                break;
            }
            Count--;
        }
    }

    const player_score& Player = Score.Player[PlayerIndex];

    switch( keyid )
    {
        //--------------------------------------------------
    case PLAYER__KEY:
        LOG_MESSAGE("gamespy_playerkey","Index:%d, Name:%s, Ingame:%d", PlayerIndex, Player.NName,Player.IsInGame);
        qr2_buffer_add( outbuf, Player.NName );
        break;
        //--------------------------------------------------
    case SCORE__KEY:
        LOG_MESSAGE("gamespy_playerkey","Index:%d, Score:%d", PlayerIndex, Player.Score);
        qr2_buffer_add_int( outbuf, Player.Score );
        break;
        //--------------------------------------------------
    case DEATHS__KEY:
        LOG_MESSAGE("gamespy_playerkey","Index:%d, Deaths:%d", PlayerIndex, Player.Deaths);
        qr2_buffer_add_int( outbuf, Player.Deaths );
        break;
        //--------------------------------------------------
    case PING__KEY:
        {
            s32 Ping;

            Ping = (s32)g_NetworkMgr.GetClientPing( g_NetworkMgr.GetClientIndex( PlayerIndex ) );
            LOG_MESSAGE("gamespy_playerkey","Index:%d, Ping:%d", PlayerIndex, Ping);
            qr2_buffer_add_int( outbuf, Ping );
        }
        break;
        //--------------------------------------------------
    case TEAM__KEY:
        LOG_MESSAGE("gamespy_playerkey","Index:%d, Team:%d", PlayerIndex, Player.Team);
        qr2_buffer_add_int( outbuf, Player.Team );
        break;
        //--------------------------------------------------
    default:
        LOG_WARNING("gamespy_playerkey","Index:%d, UNKNOWN KEY:%d", PlayerIndex, keyid);
        qr2_buffer_add( outbuf, _T(""));
        break;      
    }
    
    GSI_UNUSED(userdata);
}

//==============================================================================

void gamespy_teamkey(int keyid, int index, qr2_buffer_t outbuf, void *userdata)
{
    match_mgr& MatchMgr = *(match_mgr*)userdata;
    const game_score& Score = GameMgr.GetScore();
    ASSERT( MatchMgr.IsBrowserLocked() );
    (void)MatchMgr;

    //check for valid index
    if( Score.IsTeamBased )
    {
        if( index > 1 )
        {
            LOG_WARNING("gamespy_teamkey","Team index:%d, ignored", index);
            qr2_buffer_add(outbuf, _T(""));
            return;
        }
    }
    else if( index > 0 )
    {
        LOG_WARNING("gamespy_teamkey","Team index:%d, ignored", index);
        qr2_buffer_add(outbuf, _T(""));
        return;
    }

    const team_score& Team = Score.Team[index];

    switch( keyid )
    {
        //--------------------------------------------------
    case TEAM_T_KEY:
        LOG_MESSAGE("gamespy_teamkey","TEAM_T_KEY: Index:%d, Name:%s", index, Team.NName);
        qr2_buffer_add(outbuf, Team.NName );
        break;
        //--------------------------------------------------
    case SCORE_T_KEY:
        LOG_MESSAGE("gamespy_teamkey","SCORE_T_KEY: Index:%d, Score:%d", index, Team.Score);
        qr2_buffer_add_int(outbuf, Team.Score );
        break;
        //--------------------------------------------------
    default:
        LOG_WARNING("gamespy_teamkey","SCORE_T_KEY: Unknown key %d", keyid);
        qr2_buffer_add(outbuf, _T(""));
        break;      
    }
    
    GSI_UNUSED(userdata);
}   

//==============================================================================

void gamespy_keylist(qr2_key_type keytype, qr2_keybuffer_t keybuffer, void *userdata)
{
    match_mgr& MatchMgr = *(match_mgr*)userdata;
    LOG_MESSAGE("gamespy_keylist","Request for keylist %d", keytype);
    ASSERT( MatchMgr.IsBrowserLocked() );
    (void)MatchMgr;

    //need to add all the keys we support
    switch( keytype )
    {
        //--------------------------------------------------
    case key_server:
        qr2_keybuffer_add(keybuffer, HOSTNAME_KEY);
        qr2_keybuffer_add(keybuffer, GAMEVER_KEY);
        qr2_keybuffer_add(keybuffer, HOSTPORT_KEY);
        qr2_keybuffer_add(keybuffer, MAPNAME_KEY);
        qr2_keybuffer_add(keybuffer, GAMETYPE_KEY);
        qr2_keybuffer_add(keybuffer, GAMETYPE_ID_KEY);
        qr2_keybuffer_add(keybuffer, NUMPLAYERS_KEY);
        qr2_keybuffer_add(keybuffer, NUMTEAMS_KEY);
        qr2_keybuffer_add(keybuffer, MAXPLAYERS_KEY);
        qr2_keybuffer_add(keybuffer, GAMEMODE_KEY);
        qr2_keybuffer_add(keybuffer, TEAMPLAY_KEY);
        qr2_keybuffer_add(keybuffer, FRAGLIMIT_KEY);
        qr2_keybuffer_add(keybuffer, TIMELIMIT_KEY);
        qr2_keybuffer_add(keybuffer, FLAGS_KEY);
        qr2_keybuffer_add(keybuffer, LEVEL_KEY);
        qr2_keybuffer_add(keybuffer, PLAYERS_KEY);
        qr2_keybuffer_add(keybuffer, AVGPING_KEY);
        qr2_keybuffer_add(keybuffer, MUTATION_MODE_KEY);
        qr2_keybuffer_add(keybuffer, PASSWORD_SET_KEY);
        qr2_keybuffer_add(keybuffer, VOICE_KEY);
        break;
        //--------------------------------------------------
    case key_player:
        qr2_keybuffer_add(keybuffer, PLAYER__KEY);
        qr2_keybuffer_add(keybuffer, SCORE__KEY);
        qr2_keybuffer_add(keybuffer, DEATHS__KEY);
        qr2_keybuffer_add(keybuffer, PING__KEY);
        qr2_keybuffer_add(keybuffer, TEAM__KEY);
        break;
        //--------------------------------------------------
    case key_team:
        qr2_keybuffer_add(keybuffer, TEAM_T_KEY);
        qr2_keybuffer_add(keybuffer, SCORE_T_KEY);
        break;
    }
    
    GSI_UNUSED(userdata);
}

//==============================================================================

int gamespy_count(qr2_key_type keytype, void *userdata)
{
    match_mgr& MatchMgr = *(match_mgr*)userdata;
    const game_score& Score = GameMgr.GetScore();
    ASSERT( MatchMgr.IsBrowserLocked() );
    (void)MatchMgr;

    switch( keytype )
    {
    case key_player:
        LOG_MESSAGE("gamespy_count","key_player: %d", Score.NPlayers );
        return Score.NPlayers;
        break;
    case key_team:
        if( Score.IsTeamBased )
        {
            LOG_MESSAGE("gamespy_count","key_team: 2");
            return 2;
        }
        else
        {
            LOG_MESSAGE("gamespy_count","key_team: 0");
            return 0;
        }
        break;
    default:
        LOG_MESSAGE("gamespy_count","Unknown key %d", keytype);
        return 0;
    }
        
    GSI_UNUSED(userdata);
}

//==============================================================================

void gamespy_adderror( qr2_error_t error, gsi_char *errmsg, void *userdata )
{
    match_mgr& MatchMgr = *(match_mgr*)userdata;
    LOG_ERROR("gamespy_adderror","Server registration returned: 0x%08x, %s", error, errmsg );
    (void)MatchMgr;
    if( (error==e_qrnochallengeerror)||(error==e_qrconnerror)||(error==e_qrdnserror) )
    {
        g_ActiveConfig.SetExitReason( GAME_EXIT_NETWORK_DOWN );
    }

    GSI_UNUSED(errmsg);
    GSI_UNUSED(error);
    GSI_UNUSED(userdata);
}

//==============================================================================
void gamespy_error( GPConnection* pConnection, GPErrorArg* pError, void* pParam )
{
    (void)pConnection;
    (void)pParam;
    LOG_ERROR("gamespy_error","Presence library returned error: %d, %s", pError->errorCode, pError->errorString );
    if( pError && (pError->result==GP_NETWORK_ERROR) && (pError->fatal == GP_FATAL) )
    {
        g_ActiveConfig.SetExitReason( GAME_EXIT_NETWORK_DOWN );
    }
}

//==============================================================================
static void FormServerInfo( SBServer server, s32 Index, server_info& Response, const net_address& MyPublic )
{
    xbool       HasPrivateAddress;
    net_address Private;
    net_address Public;

    x_memset( &Response, 0, sizeof(Response) );

    HasPrivateAddress = SBServerHasPrivateAddress(server);

    Public   = net_address( SBServerGetPublicAddress(server), SBServerGetPublicQueryPort(server) );
    Private  = net_address( SBServerGetPrivateAddress(server), SBServerGetPrivateQueryPort(server) );

    Response.ID = Index;
    Response.External = Public;

    if( HasPrivateAddress && (Public.GetIP() == MyPublic.GetIP()) )
    {
        Response.ConnectionType     = CONNECT_DIRECT;
        Response.Remote             = Private;
        Response.External           = Private;
    }
    else
    {
        if( SBServerDirectConnect( server ) )
        {
            Response.ConnectionType  = CONNECT_DIRECT;
        }
        else
        {
            Response.ConnectionType  = CONNECT_NAT;
        }
        Response.Remote              = Public;
    }

    Response.GameTypeID             = SBServerGetIntValue(server, _T("gametypeid"), 0);
    Response.Version                = SBServerGetIntValue(server, _T("gamever"),    0);
    Response.MaxPlayers             = SBServerGetIntValue(server, _T("maxplayers"), 0);
    Response.Players                = SBServerGetIntValue(server, _T("numplayers"), 0);
    Response.Level                  = SBServerGetIntValue(server, _T("level"),      0);
    Response.FragLimit              = SBServerGetIntValue(server, _T("fraglimit"),  0);
    Response.GameTime               = SBServerGetIntValue(server, _T("timelimit"),  0);
    Response.Flags                  = SBServerGetIntValue( server, _T("svrflags"),  0);
    Response.PingDelay              = SBServerGetPing(server);
    if( Response.PingDelay == 0 )
    {
        Response.PingDelay          = SBServerGetIntValue(server, _T("avgping"),    0);
    }

    Response.MutationMode           = (mutation_mode)SBServerGetIntValue(server, _T("mutation"),  0);
    Response.PasswordEnabled        = SBServerGetIntValue(server, _T("passworden"),  0);    
    Response.VoiceEnabled           = SBServerGetIntValue(server, _T("voice"),  0);

    x_mstrncpy(Response.GameType,     SBServerGetStringValue(server,  _T("gametype"), _T("")),  sizeof(Response.GameType)/sizeof(xwchar));
    x_mstrncpy(Response.Name,         SBServerGetStringValue(server,  _T("hostname"), _T("")),  sizeof(Response.Name)/sizeof(xwchar));
    x_mstrncpy(Response.MissionName,  SBServerGetStringValue(server,  _T("mapname"),  _T("")),  sizeof(Response.MissionName)/sizeof(xwchar));
    x_mstrncpy(Response.ShortGameType,SBServerGetStringValue(server,  _T("gamemode"), _T("")),  sizeof(Response.ShortGameType)/sizeof(xwchar));
}

//==============================================================================
void gamespy_server_query(ServerBrowser sb, SBCallbackReason reason, SBServer server, void* pInstance)
{
    match_mgr& MatchMgr = *(match_mgr*)pInstance;
    server_info         Response;
    s32                 i;
    const char*         pPlayerNames;
    net_address         MyPublicAddress;
    s32                 Index;
    ASSERT( MatchMgr.IsBrowserLocked() );

    MatchMgr.m_StateTimeout = 2.0f;

    Index = -1;
    if( (reason==sbc_serveradded) || (reason==sbc_serverupdated) || (reason==sbc_serverupdatefailed) )
    {
        for( i=0; i<ServerBrowserCount(sb); i++ )
        {
            if( ServerBrowserGetServer(sb, i)==server )
            {
                Index = i;
                break;
            }
        }

        ASSERT( Index >= 0 );
    }

    MyPublicAddress = net_address( ntohl(ServerBrowserGetMyPublicIPAddr(g_MatchMgr.m_pBrowser)), g_MatchMgr.m_pSocket->GetPort() );
    int updatePercent = 0;

    switch( reason )
    {
    case sbc_serveradded:
        //determine our update percent
        if( ServerBrowserCount(sb) > 0 )
            updatePercent = (ServerBrowserCount(sb) - ServerBrowserPendingQueryCount(sb)) * 100 / ServerBrowserCount(sb);

        FormServerInfo( server, Index, Response, MyPublicAddress );

        pPlayerNames = SBServerGetStringValue(server,  _T("players"), _T("<none>"));

        if( SBServerHasBasicKeys(server) )
        {
            ///***NOTE*** We should be rejecting incorrect game versions at this point.
            // Form a fake internal data structure to pass back to the match manager logic
            pPlayerNames = SBServerGetStringValue(server,  _T("players"), _T("<none>"));

            LOG_MESSAGE("gamespy_server_query", "Firewall Server Added[BASIC KEYS]: %s:%d [%d] - %s - %d/%d (%d%%) - %s\n", SBServerGetPublicAddress(server), SBServerGetPublicQueryPort(server), SBServerGetPing(server),
                SBServerGetStringValue(server, _T("hostname"), _T("")), SBServerGetIntValue(server, _T("numplayers"), 0), SBServerGetIntValue(server, _T("maxplayers"), 0), updatePercent, pPlayerNames);
            //MatchMgr.QueueLookup( Response );
            MatchMgr.AppendServer(Response);
        }
        else
        {
            pPlayerNames = SBServerGetStringValue(server,  _T("players"), _T("<none>"));

            LOG_MESSAGE("gamespy_server_query","Firewall Server Added[NO KEYS]: %s:%d [%d] - %s - %d/%d (%d%%) - %s\n", SBServerGetPublicAddress(server), SBServerGetPublicQueryPort(server), SBServerGetPing(server),
            SBServerGetStringValue(server, _T("hostname"), _T("")), SBServerGetIntValue(server, _T("numplayers"), 0), SBServerGetIntValue(server, _T("maxplayers"), 0), updatePercent, pPlayerNames);
            //ServerBrowser
        }

        break;
    case sbc_serverupdated:
        //determine our update percent
        if( ServerBrowserCount(sb) > 0 )
            updatePercent = (ServerBrowserCount(sb) - ServerBrowserPendingQueryCount(sb)) * 100 / ServerBrowserCount(sb);

        ///***NOTE*** We should be rejecting incorrect game versions at this point.
        // Form a fake internal data structure to pass back to the match manager logic
        x_memset( &Response, 0, sizeof(Response) );
        FormServerInfo( server, Index, Response, MyPublicAddress );

        pPlayerNames = SBServerGetStringValue(server,  _T("players"), _T("<none>"));

        LOG_MESSAGE("gamespy_server_query","sbc_serverupdated: Server:%s, Players:%d, Version:%d, Ping:%2.02f, playernames:%s", (const char*)xstring(Response.Name), Response.Players, Response.Version, Response.PingDelay, pPlayerNames );
        MatchMgr.AppendServer(Response);

        LOG_MESSAGE("gamespy_server_query","Server Updated: %s:%d [%d] - %s - %d/%d (%d%%)", SBServerGetPublicAddress(server), SBServerGetPublicQueryPort(server), SBServerGetPing(server),
            SBServerGetStringValue(server, _T("hostname"), _T("")), SBServerGetIntValue(server, _T("numplayers"), 0), SBServerGetIntValue(server, _T("maxplayers"), 0), updatePercent);
        break;
    case sbc_serverupdatefailed:
        LOG_ERROR("gamespy_server_query","Update Failed: %s:%d", SBServerGetPublicAddress(server), SBServerGetPublicQueryPort(server));
        break;
    case sbc_updatecomplete:
        LOG_MESSAGE("gamespy_server_query","Server Browser Update Complete"); 
        MatchMgr.SetConnectStatus( MATCH_CONN_IDLE );
        break;
    case sbc_queryerror:
        LOG_ERROR( "gamespy_server_query", "Query error %s", ServerBrowserListQueryError(sb) );
        break;
    default:
        LOG_MESSAGE( "gamespy_server_query", "Unknown response %d", reason );
        break;
    }
}

//==============================================================================
void gamespy_indirect_server_query(ServerBrowser sb, SBCallbackReason reason, SBServer server, void* pInstance)
{
    match_mgr& MatchMgr = *(match_mgr*)pInstance;
    net_address         MyPublicAddress;
    server_info&        Config = g_PendingConfig.GetConfig();


    (void)sb;
    (void)reason;
    ASSERT( MatchMgr.IsBrowserLocked() );

    if( (reason==sbc_serverupdated) || (reason==sbc_serveradded) )
    {
        MyPublicAddress = net_address( ntohl(ServerBrowserGetMyPublicIPAddr(g_MatchMgr.m_pBrowser)), g_MatchMgr.m_pSocket->GetPort() );
        FormServerInfo( server, 0, Config, MyPublicAddress );
        LOG_MESSAGE( "gamespy_indirect_server_query", "Lookup response received. Internal address:%s, External Address:%s", Config.Remote.GetStrAddress(), Config.External.GetStrAddress() );
        if( MatchMgr.GetConnectStatus() == MATCH_CONN_ACQUIRING_SERVERS )
        {
            if( Config.Remote.IsEmpty() )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_SESSION_ENDED );
                MatchMgr.SetConnectStatus( MATCH_CONN_DISCONNECTED );
            }
            else
            {
                MatchMgr.SetConnectStatus( MATCH_CONN_CONNECTING );
            }
        }
    }
    else
    {
        if( MatchMgr.GetConnectStatus() == MATCH_CONN_ACQUIRING_SERVERS )
        {
            g_ActiveConfig.SetExitReason( GAME_EXIT_SESSION_ENDED );
            MatchMgr.SetConnectStatus( MATCH_CONN_DISCONNECTED );
        }
    }
}

//==============================================================================
void gamespy_extended_info_query(ServerBrowser sb, SBCallbackReason reason, SBServer pServer, void* pInstance)
{
    s32 nTeams;
    s32 nPlayers;

    (void)pInstance;
    (void)sb;
    (void)reason;
    ASSERT( g_MatchMgr.IsBrowserLocked() );

    if( (reason==sbc_serveradded) || (reason==sbc_serverupdated) )
    {
        game_score& Score = g_MatchMgr.m_ExtendedServerInfo.Score;
        if( SBServerHasFullKeys( pServer ) )
        {
            s32 i;
            // Now fill in temporary gamedata structure.
            x_memset( &g_MatchMgr.m_ExtendedServerInfo, 0, sizeof(g_MatchMgr.m_ExtendedServerInfo) );
            nPlayers                = SBServerGetIntValue(pServer, _T("numplayers"), 0);
            nTeams                  = SBServerGetIntValue(pServer, _T("numteams"), 1);
            Score.ScoreFieldMask    = SCORE_POINTS;
            Score.NPlayers          = nPlayers;
            Score.IsTeamBased       = (nTeams>1);

            x_mstrncpy(Score.MissionName,  SBServerGetStringValue(pServer,  _T("mapname"),  _T("<no mapname>")),   sizeof(Score.MissionName)/sizeof(xwchar));

            x_strcpy( Score.Team[0].NName, SBServerGetTeamStringValue(pServer, 0, _T("team"), _T("<unknown>") ) );
            x_mstrcpy( Score.Team[0].Name, Score.Team[0].NName );
            Score.Team[0].Score = SBServerGetTeamIntValue(pServer, 0, _T("score"), 0 );
            LOG_MESSAGE( "gamespy_extended_info_query", "Team 1 Name:%s, Score:%d", Score.Team[0].NName, Score.Team[0].Score );

            if( Score.IsTeamBased )
            {
                x_strcpy( Score.Team[1].NName, SBServerGetTeamStringValue(pServer, 1, _T("team"), _T("<unknown>") ) );
                x_mstrcpy( Score.Team[1].Name, Score.Team[1].NName );
                Score.Team[1].Score = SBServerGetTeamIntValue(pServer, 1, _T("score"), 0 );
                LOG_MESSAGE( "gamespy_extended_info_query", "Team 2 Name:%s, Score:%d", Score.Team[1].NName, Score.Team[1].Score );
            }

            for( i=0; i<Score.NPlayers; i++ )
            {
                x_strncpy( Score.Player[i].NName, SBServerGetPlayerStringValue( pServer, i, _T("player"),_T("") ), sizeof(Score.Player[i].NName) );
                x_mstrcpy( Score.Player[i].Name, Score.Player[i].NName );
                Score.Player[i].IsInGame = (x_strlen( Score.Player[i].NName )>0);
                Score.Player[i].Score    = SBServerGetPlayerIntValue( pServer, i, _T("score"), 0 );
                Score.Player[i].Team     = SBServerGetPlayerIntValue( pServer, i, _T("team"), 0 );
                Score.Player[i].Deaths   = SBServerGetPlayerIntValue( pServer, i, _T("deaths"), 0 );

                LOG_MESSAGE( "gamespy_extended_info_query", "Player:%d, Name:%s, Team:%d, Score:%d, Deaths:%d", i, 
                                                            Score.Player[i].NName, Score.Player[i].Team,
                                                            Score.Player[i].Score, Score.Player[i].Deaths );

            }
            g_MatchMgr.SetState( MATCH_IDLE );
        }
    }
}

//==============================================================================
void gamespy_connect( GPConnection*, GPConnectResponseArg * arg, void * param )
{
    match_mgr& MatchMgr = *(match_mgr*)param;
    ASSERT( MatchMgr.IsBrowserLocked() );

    LOG_MESSAGE( "gamespy_connect","Received connect result: %d, profile:%d", arg->result, arg->profile );
    if( arg->result == GP_NO_ERROR )
    {
        MatchMgr.SetAuthStatus( AUTH_STAT_CONNECTED );
        MatchMgr.m_PlayerIdentifier[0] = arg->profile;
    }
    else
    {
        MatchMgr.SetAuthStatus( AUTH_STAT_NO_ACCOUNT );
        MatchMgr.m_PlayerIdentifier[0] = 0;
    }
}

//==============================================================================
void gamespy_emailcheck( GPConnection*, GPIsValidEmailResponseArg* pArg, void* pThis )
{
    match_mgr& MatchMgr = *(match_mgr*)pThis;
    ASSERT( MatchMgr.IsBrowserLocked() );

    LOG_MESSAGE("gamespy_isvalid", "Result: %d",pArg->result );
    switch( pArg->result )
    {
    case GP_NO_ERROR:
        if( pArg->isValid == GP_VALID )
        {
            MatchMgr.SetAuthStatus( AUTH_STAT_CONNECTED );
        }
        else
        {
            MatchMgr.SetAuthStatus( AUTH_STAT_NO_ACCOUNT );
        }
        break;
    default:
        MatchMgr.SetAuthStatus( AUTH_STAT_CANNOT_CONNECT );
        ASSERT( FALSE );
        break;
    }

}

//==============================================================================
void gamespy_nat_negotiate( int cookie, void* userdata )
{
    match_mgr& MatchMgr = *(match_mgr*)userdata;
    ASSERT( MatchMgr.IsBrowserLocked() );

    ASSERT( MatchMgr.m_LocalIsServer );
    LOG_MESSAGE("gamespy_nat_negotiate","Received NAT negotiation cookie 0x%08x",cookie);
    NNBeginNegotiationWithSocket( (SOCKET)MatchMgr.m_pSocket, cookie, 0, gamespy_nat_progress, gamespy_nat_complete, userdata );
}

//==============================================================================
void    gamespy_nat_progress ( NegotiateState state, void *userdata )
{
    (void)state;
    (void)userdata;
#if defined(X_LOGGING)
    LOG_MESSAGE( "gamespy_nat_progress","Negotiation state: %s", GetName(state) );
#endif
}

//==============================================================================
void    gamespy_nat_complete ( NegotiateResult result, SOCKET gamesocket, struct sockaddr_in *remoteaddr, void *userdata )
{
    match_mgr& MatchMgr = *(match_mgr*)userdata;
    net_socket* pLocalSocket = (net_socket*)gamesocket;
    (void)pLocalSocket;
    if( result == nr_success )
    {
        net_address Remote( ntohl(remoteaddr->sin_addr.s_addr), ntohs(remoteaddr->sin_port) );
#if defined(X_LOGGING)		
        LOG_MESSAGE("gamespy_nat_complete","Result: %s, Local game socket:%s, Remote Socket:%s", GetName(result), pLocalSocket->GetStrAddress(), Remote.GetStrAddress() );
#endif
        if( MatchMgr.m_LocalIsServer == FALSE )
        {
            ASSERT( MatchMgr.GetState() == MATCH_NAT_NEGOTIATE );
            MatchMgr.SetConnectStatus( MATCH_CONN_CONNECTED );
            g_PendingConfig.SetRemoteAddress( Remote );
            g_PendingConfig.GetConfig().ConnectionType = CONNECT_DIRECT;
        }
    }
    else
    {
#if defined(X_LOGGING)		
        LOG_WARNING("gamespy_nat_complete","Result: %s", GetName(result) );
#endif		
        if( MatchMgr.m_LocalIsServer == FALSE )
        {
            ASSERT( MatchMgr.GetState() == MATCH_NAT_NEGOTIATE );
            g_ActiveConfig.SetExitReason( GAME_EXIT_CANNOT_CONNECT );
            MatchMgr.SetConnectStatus( MATCH_CONN_DISCONNECTED );
        }
    }
}

//==============================================================================
void gamespy_getinfo_response(GPConnection* pConnection, GPGetInfoResponseArg* pResponse, void* pArg)
{
    s32             Index;
    GPBuddyStatus   Status;
    net_address     Remote;

    x_strcpy( (char*)pArg, "<unknown>" );
    x_memset( &Status, 0, sizeof(Status) );

    if( pResponse->result == GP_NO_ERROR )
    {
        gpGetBuddyIndex( pConnection, pResponse->profile, &Index );
        if( Index != -1 )
        {
            gpGetBuddyStatus( pConnection, Index, &Status );
            Remote = net_address( ntohl(Status.ip), Status.port );
        }
        x_strcpy( (char*)pArg, pResponse->nick );
        LOG_MESSAGE( "gamespy_getinfo_response","GetInfo returned player index:%d, nickname:%s, profile:%d, Location:%s", Index, pResponse->nick, pResponse->profile, Remote.GetStrIP() );
    }
    else
    {
        LOG_ERROR( "gamespy_getinfo_response","GetInfo returned error code %d", pResponse->result );
    }
}

//==============================================================================
void gamespy_buddy_request( GPConnection* pConnection, GPRecvBuddyRequestArg* pRequest,   void* pInstance )
{
    match_mgr& MatchMgr = *(match_mgr*)pInstance;
    char                ProfileName[64];
    GPResult            Status;
    buddy_info          Buddy;

    x_memset( &Buddy, 0, sizeof(Buddy) );
    ASSERT( MatchMgr.IsBrowserLocked() );

    MatchMgr.UnlockBrowser();
    
    Status = gpGetInfo( pConnection, pRequest->profile, GP_CHECK_CACHE, GP_BLOCKING, (GPCallback)gamespy_getinfo_response, ProfileName);
    MatchMgr.LockBrowser();

    Buddy.Identifier = pRequest->profile;
    x_strcpy( Buddy.Name, ProfileName );

    s32 BuddyIndex = MatchMgr.m_Buddies.Find( Buddy );
    if( BuddyIndex == -1 )
    {
        // Set a flag saying that this dude has sent buddy request to us.
        BuddyIndex = MatchMgr.m_Buddies.GetCount();
        Buddy.Flags = USER_REQUEST_RECEIVED;
        MatchMgr.m_Buddies.Append( Buddy );
        LOG_MESSAGE("gamespy_buddy_request","New buddy request received from id#%d, (%s).", pRequest->profile, ProfileName);
    }
    else
    {
        buddy_info& CurrentBuddy = MatchMgr.m_Buddies[BuddyIndex];
        // If we already have a request pending for this buddy, and we are not automatically ignoring them, just automatically accept their request
        if( CurrentBuddy.Flags & USER_REQUEST_IGNORED )
        {
            LOG_MESSAGE("gamespy_buddy_request","Auto declined add request received from id#%d, (%s).", pRequest->profile, ProfileName);
            // Automatically deny the request if we're ignoring that person
            gpDenyBuddyRequest( pConnection, pRequest->profile );
        }
        else
        {
            // If this buddy is already in our list, and is not being ignored, then we automatically accept their request as we will be in the process
            // of negotiating both being each others buddies.

            if( (CurrentBuddy.Status == BUDDY_STATUS_ADD_PENDING) || (gpIsBuddy( &MatchMgr.m_Presence, Buddy.Identifier )) )
            {
                LOG_MESSAGE("gamespy_buddy_request","Add request automatically accepted from id#%d, (%s).", pRequest->profile, ProfileName);
                gpAuthBuddyRequest( pConnection, pRequest->profile );
            }
            else
            {
                LOG_MESSAGE("gamespy_buddy_request","Ignored add request received when already a buddy id#%d, (%s).", pRequest->profile, ProfileName);
            }
        }
    }
}

//==============================================================================
void gamespy_buddy_status ( GPConnection* pConnection, GPRecvBuddyStatusArg* pStatus, void* pInstance )
{
    match_mgr& MatchMgr = *(match_mgr*)pInstance;
    GPBuddyStatus       BuddyStatus;
    char                ProfileName[64];
    s32                 Status;

    ASSERT( MatchMgr.IsBrowserLocked() );
    //
    // Since this is going to be an extended further request, then we unlock the browser and wait for
    // the request to complete before locking it again and returning.
    //
    MatchMgr.UnlockBrowser();
    Status = gpGetInfo( pConnection, pStatus->profile, GP_CHECK_CACHE, GP_BLOCKING, (GPCallback)gamespy_getinfo_response, ProfileName);
    if( Status != GP_NO_ERROR )
    {
        LOG_ERROR( "gamespy_buddy_status", "Returned error %d", Status );
        MatchMgr.LockBrowser();
        return;
    }

    Status = gpGetBuddyStatus( pConnection, pStatus->index, &BuddyStatus);
    if( Status != GP_NO_ERROR )
    {
        LOG_ERROR( "gamespy_buddy_status", "Returned error %d", Status );
        MatchMgr.LockBrowser();
        return;
    }

#if defined(X_DEBUG)
    const char*         pResult;

    switch( BuddyStatus.status )
    {
    case GP_OFFLINE:        pResult = "OFFLINE";    break;
    case GP_ONLINE:         pResult = "ONLINE";     break;
    case GP_PLAYING:        pResult = "PLAYING";    break;
    case GP_STAGING:        pResult = "STAGING";    break;
    case GP_CHATTING:       pResult = "CHATTING";   break;
    case GP_AWAY:           pResult = "AWAY";       break;
    default:                pResult = "<unknown>";  break;
    }
    LOG_MESSAGE( "gamespy_buddy_status", "Status update for %s: %s, Location:%s", ProfileName, pResult, BuddyStatus.locationString );
#endif
    MatchMgr.LockBrowser();
    
    // Now, update the status of this buddy within the list of buddies available.
    buddy_info* pBuddy=NULL;
    s32         i;

    for( i=0; i< MatchMgr.GetBuddyCount(); i++ )
    {
        buddy_info& Info = MatchMgr.GetBuddy(i);
        if( Info.Identifier == (u64)pStatus->profile )
        {
            pBuddy = &Info;
            break;
        }
    }
    if( pBuddy == NULL )
    {
        pBuddy = &MatchMgr.m_Buddies.Append();
        x_memset( pBuddy, 0, sizeof(buddy_info) );
    }

    x_strcpy( pBuddy->Name, ProfileName );
    x_strcpy( pBuddy->UniqueNick, "<unknown>" );
    const char* pInfoString;
    xwchar*     pString;
    char        TempString[64];

    pInfoString = BuddyStatus.locationString;
    pString = pBuddy->Location;

    // Parse out the gametype and mapname
    while( TRUE )
    {
        if( ( *pInfoString=='/' ) || (*pInfoString==0x0) )
        {
            break;
        }
        *pString++=*pInfoString++;
    }
    *pString=0x0;

    if( *pInfoString=='/' )
    {
        pInfoString++;
    }

    // Parse out the server address
    char* pAddrString;
    pBuddy->Remote.Clear();

    pAddrString = TempString;
    while( TRUE )
    {
        if( ( *pInfoString=='/' ) || (*pInfoString==0x0) )
        {
            break;
        }
        *pAddrString++=*pInfoString++;
    }
    *pAddrString=0x0;

    if( *pInfoString=='/' )
    {
        pInfoString++;
    }

    if( x_strlen( TempString ) > 0 )
    {
        pBuddy->Remote.SetStrAddress( TempString );
    }

    pAddrString = TempString;
    while( TRUE )
    {
        if( ( *pInfoString=='/' ) || (*pInfoString==0x0) )
        {
            break;
        }
        *pAddrString++=*pInfoString++;
    }
    *pAddrString=0x0;

    if( *pInfoString=='/' )
    {
        pInfoString++;
    }

    s32 Flags = 0;
    while( x_isdigit(*pInfoString) )
    {
        Flags = Flags*10+(*pInfoString-'0');
        pInfoString++;
    }

    Flags                       &=  (USER_VOICE_ENABLED|USER_HAS_PASSWORD);
    pBuddy->Flags               &= ~(USER_VOICE_ENABLED|USER_HAS_PASSWORD|USER_REQUEST_RECEIVED|USER_REQUEST_SENT);
    pBuddy->Flags               |= Flags;
    pBuddy->Identifier           = pStatus->profile;

    switch( BuddyStatus.status )
    {
    case GP_OFFLINE:    pBuddy->Status = BUDDY_STATUS_OFFLINE;  break;
    case GP_ONLINE:     pBuddy->Status = BUDDY_STATUS_ONLINE;   break;
    case GP_PLAYING:    pBuddy->Status = BUDDY_STATUS_INGAME;   break;
    default:            pBuddy->Status = BUDDY_STATUS_OFFLINE;  break;
    }

    if( pBuddy->Status != BUDDY_STATUS_INGAME )
    {
        pBuddy->Flags &= ~(USER_HAS_INVITE|USER_SENT_INVITE);
    }

}

//==============================================================================
void gamespy_buddy_invite ( GPConnection* pConnection, GPRecvGameInviteArg* pInvite, void* pInstance )
{
    match_mgr& MatchMgr = *(match_mgr*)pInstance;

    (void)pConnection;
    ASSERT( MatchMgr.IsBrowserLocked() );


    s32         i;

    for( i=0; i< MatchMgr.GetBuddyCount(); i++ )
    {
        buddy_info& Info = MatchMgr.GetBuddy(i);
        if( Info.Identifier == (u64)pInvite->profile )
        {
            if( (Info.Flags & USER_INVITE_IGNORED)==0 )
            {
                Info.Flags = Info.Flags | USER_HAS_INVITE;
                LOG_MESSAGE("gamespy_buddy_invite","Game invite received from %s, game with product id:%d.", Info.Name, pInvite->productID);
            }
            break;
        }
    }
}

//==============================================================================
void gamespy_motd_complete( GPConnection* pConnection, GPProfileSearchResponseArg* pInvite, void* pInstance )
{
    match_mgr& MatchMgr = *(match_mgr*)pInstance;
    (void)pConnection;
    (void)pInvite;
    (void)pInstance;
    
    MatchMgr.m_MessageOfTheDayReceived = TRUE;
}

//==============================================================================
void gamespy_public_address( unsigned int ip, unsigned short port, void* userdata )
{
    match_mgr& MatchMgr = *(match_mgr*)userdata;
    server_info&        Active = g_ActiveConfig.GetConfig();
    server_info&        Pending = g_PendingConfig.GetConfig();

    ASSERT( MatchMgr.IsBrowserLocked() );

    Active.External.Setup( ntohl( ip ), port );
    Pending.External.Setup( ntohl( ip ), port );
    LOG_MESSAGE( "gamespy_public_address", "Server public address:%s, private address:%s", Active.External.GetStrAddress(), MatchMgr.m_pSocket->GetStrAddress() );

    // This will force our local user to be updated with the correct address of his server.
    if( MatchMgr.m_pSocket->GetAddress() != Active.External )
    {
        MatchMgr.m_UserStatus = BUDDY_STATUS_OFFLINE;
    }
}

//==============================================================================
void gamespy_security_complete( GPConnection* pConnection, GPProfileSearchResponseArg* pResponse, void* pInstance )
{
    match_mgr& MatchMgr = *(match_mgr*)pInstance;
    s32 Length;

    (void)pConnection;
    (void)pResponse;

    LOG_MESSAGE( "gamespy_security_complete", "Security username callback hit. Matches:%d, Result:%d", pResponse->numMatches, pResponse->result );

    // No more responses, we're done.
    if( pResponse->numMatches == 0 )
    {
        MatchMgr.m_SecurityChallengeReceived = TRUE;
    }
    else
    {
        //
        // This is where we may decide to add other security challenges in a longer negotiation phase.
        //
        ASSERT( MatchMgr.m_pSecurityChallenge == NULL );
        Length = x_strlen( pResponse->matches->firstname ) + x_strlen( pResponse->matches->lastname ) + 1;
        MatchMgr.m_pSecurityChallenge = (char*)x_malloc( Length );
        ASSERT( MatchMgr.m_pSecurityChallenge );
        x_strcpy( MatchMgr.m_pSecurityChallenge, pResponse->matches->firstname );
        x_strcat( MatchMgr.m_pSecurityChallenge, pResponse->matches->lastname );
        MatchMgr.m_SecurityChallengeReceived = TRUE;
    }
}

