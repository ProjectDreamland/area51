//==============================================================================
//
//  GameMgr.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
/*

    TO DO:

    - Get the default team names from the string manager.
    - Support "disconnect/reconnect, keep scores" feature from AADS.
    - Activate the player upon creation?
    - Deal with problematic names:
        - Profanity.
        - All whitespace.
        - Too "narrow".  For example ".".
    - Bots?
    - How to handle team change requests?
        - In MOH:RS, when GameMgr received an update with a new team for a 
          player, the GameMgr forced the player to change teams.
    - Should be prepared to deal with team changes without a respawn.  (Tag.)
    - Clock support!
    - Score limit support!
    - Buffer overrun on player name!
    - Do we need to send over the game type in init?  [No.]
        - Is it sent over as part of the map response?  [Yes.]
        - How does pGameLogic get set on a client?  [Before map load starts.]
    - Careful about split screen voting.
    - Auto kick if disconnect during vote to kick.
    - Don't use xwstring in RegisterName.
    - Limitation: TeamBits can never be, for example, 0x00000007.  This could
      prevent some "creative" uses of the TeamBits.

*/
//==============================================================================

#if !defined(mtraub) && !defined(mreed)
//#define X_SUPPRESS_LOGS
#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#include "GameMgr.hpp"
#include "NetObjMgr.hpp"
#include "GameServer.hpp"
#include "NetworkMgr.hpp"
#include "MsgMgr.hpp"
#include "StateMgr\MapList.hpp"
#include "Configuration/GameConfig.hpp"
#include "StringMgr\StringMgr.hpp"

#include "NetworkMgr\Voice\VoiceMgr.hpp"

#include "Objects/Player.hpp"               // HACKOMOTRON
#include "TemplateMgr\TemplateMgr.hpp"      // HACKOMOTRON
#include "Objects\Actor\Actor.hpp"
#include "Objects\SpawnPoint.hpp"
#include "Objects\CapPoint.hpp"

#include "logic_Campaign.hpp"
#include "logic_DM.hpp"
#include "logic_TDM.hpp"
#include "logic_CTF.hpp"
#include "logic_Tag.hpp"
#include "logic_Infect.hpp"
#include "logic_CNH.hpp"

#include "x_log.hpp"
#include "x_bitstream.hpp"

#include "../../Apps/GameApp/Config.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

logic_base    	s_LogicBase;
logic_campaign	s_LogicCampaign;
logic_dm        s_LogicDM;
logic_tdm       s_LogicTDM;
logic_ctf       s_LogicCTF;
logic_tag       s_LogicTag;
logic_infect    s_LogicInfect;
logic_cnh       s_LogicCNH;

s32             s_PlayerCursor = 0;

//==============================================================================

static xwchar       s_TeamName[2][NET_NAME_LENGTH];
static xwchar       s_VoteMsg[2][64];

//==============================================================================
#ifdef mtraub

static xbool        s_UseFakeScore = FALSE;
static s32          s_FakeCount    = 16;
static xbool        s_FakeTeams    = TRUE;
static game_score   s_FakeScore;
void                InitFakeScore( void );

#endif
//==============================================================================

game_mgr    GameMgr;
xbool       game_mgr::m_Initialized = FALSE;

logic_base* pGameLogic = &s_LogicTDM;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void Localize( xwchar* pString, xwchar* pSource )
{
    while( *pSource )
    {   
        if( *pSource == '\\' )
        {
            pSource++;

            const xwchar* pInsert = NULL;
            xwchar        L       = pSource[0];
            xwchar        R       = pSource[1];

            if( (L == 'A') && (R == 'L') )    pInsert = g_StringTableMgr( "ui", "IDS_TEAM_ALPHA"   );
            if( (L == 'O') && (R == 'M') )    pInsert = g_StringTableMgr( "ui", "IDS_TEAM_OMEGA"   );
            if( (L == 'H') && (R == 'U') )    pInsert = g_StringTableMgr( "ui", "IDS_HUMANS"       );
            if( (L == 'M') && (R == 'U') )    pInsert = g_StringTableMgr( "ui", "IDS_MUTANTS"      );
            if( (L == 'V') && (R == 'K') )    pInsert = g_StringTableMgr( "ui", "IDS_VOTE_TO_KICK" );
            if( (L == 'V') && (R == 'M') )    pInsert = g_StringTableMgr( "ui", "IDS_VOTE_FOR_MAP" );
            if( (L == 'I') && (R == 'N') )    pInsert = g_StringTableMgr( "ui", "IDS_INACTIVE"     );
            if( (L == 'T') && (R == 'K') )    pInsert = g_StringTableMgr( "ui", "IDS_TEAMKILLER"   );

            if( pInsert )
            {
                pSource += 2;
                x_wstrcpy( pString, pInsert );
                pString += x_wstrlen( pString );
            }
            else
            {
                ASSERT( FALSE );
            }            
        }
        else
        {
            // Copy over a single character.
            *pString++ = *pSource++;
        }
    }

    *pString = '\0';
}

//==============================================================================
//==============================================================================

void game_score::ComputeRanks( void )
{
    s32 i, j;

    for( i = 0; i < 32; i++ ) 
        Player[i].Rank = Player[i].IsInGame;

    for( i = 0; i < 32; i++ ) 
    {
        if( Player[i].IsInGame )
        {
            for( j = 0; j < 32; j++ ) 
            {
                if( Player[j].IsInGame )
                {
                    if( Player[i].Score > Player[j].Score )
                    {
                        Player[j].Rank++;
                    }
                }
            }
        }
    }
}

//==============================================================================
//==============================================================================

game_mgr::game_mgr( void )
{
    Init();
    // Need to set to false after the init so the next valid call to init does
    // not assert.
    m_Initialized = FALSE;
}

//==============================================================================

game_mgr::~game_mgr( void )
{
    Kill();
}

//==============================================================================

void game_mgr::Init( void )
{
    s32 i;

    ASSERT( !m_Initialized );
    m_Initialized = TRUE;
    m_DebugRender = FALSE;

    m_Score.NConnected  = 0;
    m_Score.NPlayers    = 0;
    m_Score.IsGameOver  = FALSE;    // No previous game to be "over".
    m_Score.IsTeamBased = FALSE;

    for( i = 0; i < NET_MAX_PLAYERS; i++ )
    {
        m_Score.Player[i].ClearAll();
    }

    for( i = 0; i < 2; i++ )
    {
        m_Score.Team[i].ClearAll();
    }

    m_ZoneMinimum = 0;
    for( i = 0; i < 255; i++ )
    {
        m_Zone[i].Minimum =  0;
        m_Zone[i].Maximum = 32;
    }

    // Clear all dirty bits.
    m_DirtyBits         = 0x00;
    m_ScoreBits         = 0x00;
    m_PlayerBits        = 0x00;

    // Clock.
    m_Clock             =  0.0f;  
    m_ClockUpdate       =  0.0f;
    m_ClockMode         =  0;
    m_ClockLimit        =  0;

    // Default values.
    m_MaxPlayers        = 16;
    m_ScoreLimit        =  0;       // Unlimited score.
    m_TeamDamageFactor  =  0.75f;
    m_LastIntensity     =  0;
    m_VoteKick          = -1;
    m_VoteMap           = -1;
    m_VoteInProgress    = FALSE;
    m_VotePercent       = 75;
    m_UpdateVoteRights  = TRUE;
    m_MapScaling        = TRUE;
    m_ScoreDisplay      = SCORE_DISPLAY_NONE;

#ifdef mtraub
    InitFakeScore();
#endif
}

//==============================================================================

void game_mgr::Kill( void )
{
    ASSERT( m_Initialized );
    m_Initialized = FALSE;
}

//==============================================================================

void game_mgr::SetGameType( game_type GameType )
{
    ASSERT(  m_Initialized );
    ASSERT( !m_GameInProgress );
//  ASSERT( g_NetworkMgr.IsServer() );  // Clients should not be in here. TO DO

    // Set the game type.
    m_GameType = GameType;

    // HACK - We are only allowing the client GameMgr to know the game type in
    // order to fix a design flaw with the circuits.
    if( g_NetworkMgr.IsClient() )
        GameType = GAME_MP;

    switch( GameType )
    {
		default:            // fall through   
        case GAME_MP:       pGameLogic = &s_LogicBase;      break;
        case GAME_CAMPAIGN: pGameLogic = &s_LogicCampaign;  break;
        case GAME_DM:       pGameLogic = &s_LogicDM;        break;
        case GAME_TDM:      pGameLogic = &s_LogicTDM;       break;
        case GAME_CTF:      pGameLogic = &s_LogicCTF;       break;
        case GAME_TAG:      pGameLogic = &s_LogicTag;       break;
        case GAME_INF:      pGameLogic = &s_LogicInfect;    break;
        case GAME_CNH:      pGameLogic = &s_LogicCNH;       break;
    }

    ASSERT( pGameLogic );

    // This gives the new game logic an opportunity to make some changes if
    // needed.  Make this call BEFORE changing local variables (such as
    // IsTeamBased).  If we changes from a non team based game type to a team
    // based game type, then we need to make sure that the players are all
    // properly assigned to a team.
    pGameLogic->Activate();

    m_Score.IsTeamBased    = pGameLogic->IsTeamBased();
    m_Score.ScoreFieldMask = pGameLogic->GetScoreFields();
}

//==============================================================================

game_type game_mgr::GetGameType( void )
{
    return( m_GameType );
}

//==============================================================================

void game_mgr::SetMutationMode( mutation_mode MutationMode )
{
    ASSERT( !m_GameInProgress );
    m_MutationMode = MutationMode;
}

//==============================================================================

mutation_mode game_mgr::GetMutationMode( void )
{
    return( m_MutationMode );
}

//==============================================================================

xbool game_mgr::IsGameOnline( void )
{
    xbool IsOnline = (g_NetworkMgr.GetLocalPlayerCount() == 1) &&
                     (GameMgr.GetGameType() != GAME_CAMPAIGN);

    return( IsOnline );
}

//==============================================================================

xbool game_mgr::IsGameMultiplayer( void )
{
    return( m_GameType != GAME_CAMPAIGN );
}

//==============================================================================

void game_mgr::DebugRender( void )
{
    ASSERT( m_Initialized );
}

//==============================================================================

void game_mgr::Logic( f32 DeltaTime )
{
    ASSERT( m_Initialized );

    if( m_Score.IsGameOver )
        return;

    if( !m_GameInProgress )
        return;

    //
    // Move in the 4th dimension.
    //

    m_PlayTime += DeltaTime;

    f32 OldClock = m_Clock;
    if( m_ClockMode == -1 )     m_Clock -= DeltaTime;
    if( m_ClockMode ==  1 )     m_Clock += DeltaTime;

    pGameLogic->AdvanceTime( DeltaTime );
    if( m_Score.IsGameOver )
        return;
    
    // Update the score fields.
    ScoreDisplayLogic();

    // Take care of zone logic that runs on clients and servers.
    ZoneLogicAll( DeltaTime );

    #ifdef mtraub
    if( (s_FakeScore.IsTeamBased != s_FakeTeams) ||
        (s_FakeScore.NPlayers    != s_FakeCount) )
    {
        InitFakeScore();
        m_DirtyBits  = DIRTY_ALL;
        m_ScoreBits  = 0xFFFFFFFF;
        m_PlayerBits = 0xFFFFFFFF;
    }        
    #endif

    // End of the line for clients.
    if( g_NetworkMgr.IsClient() )
        return;

    // Time out messages.
    if( (m_ClockMode == -1) && (m_GameType != GAME_CAMPAIGN) )
    {
        if( !m_Minutes5  && IN_RANGE( m_Clock, 5*60+1, OldClock ) )     { m_Minutes5  = TRUE; MsgMgr.Message( MSG_FIVE_MINUTES,    0 ); }
        if( !m_Minutes2  && IN_RANGE( m_Clock, 2*60+1, OldClock ) )     { m_Minutes2  = TRUE; MsgMgr.Message( MSG_TWO_MINUTES,     0 ); }
        if( !m_Seconds60 && IN_RANGE( m_Clock,   60+1, OldClock ) )     { m_Seconds60 = TRUE; MsgMgr.Message( MSG_SIXTY_SECONDS,   0 ); }
        if( !m_Seconds30 && IN_RANGE( m_Clock,   30+1, OldClock ) )     { m_Seconds30 = TRUE; MsgMgr.Message( MSG_THIRTY_SECONDS,  0 ); }
        if( !m_Seconds10 && IN_RANGE( m_Clock,   10+1, OldClock ) )     { m_Seconds10 = TRUE; MsgMgr.Message( MSG_TEN_SECONDS,     0 ); }
        if( !m_Seconds05 && IN_RANGE( m_Clock,    5+1, OldClock ) )     { m_Seconds05 = TRUE; MsgMgr.Message( MSG_FIVE_FOUR_THREE, 0 ); }
    }

    if( (m_ClockMode == 1) && (m_GameType != GAME_CAMPAIGN) )
    {
        if( !m_Minutes5  && IN_RANGE( OldClock, 5*60+1, m_Clock ) )     { m_Minutes5  = TRUE; MsgMgr.Message( MSG_FIVE_MINUTES,    0 ); }
        if( !m_Minutes2  && IN_RANGE( OldClock, 2*60+1, m_Clock ) )     { m_Minutes2  = TRUE; MsgMgr.Message( MSG_TWO_MINUTES,     0 ); }
        if( !m_Seconds60 && IN_RANGE( OldClock,   60+1, m_Clock ) )     { m_Seconds60 = TRUE; MsgMgr.Message( MSG_SIXTY_SECONDS,   0 ); }
        if( !m_Seconds30 && IN_RANGE( OldClock,   30+1, m_Clock ) )     { m_Seconds30 = TRUE; MsgMgr.Message( MSG_THIRTY_SECONDS,  0 ); }
        if( !m_Seconds10 && IN_RANGE( OldClock,   10+1, m_Clock ) )     { m_Seconds10 = TRUE; MsgMgr.Message( MSG_TEN_SECONDS,     0 ); }
        if( !m_Seconds05 && IN_RANGE( OldClock,    5+1, m_Clock ) )     { m_Seconds05 = TRUE; MsgMgr.Message( MSG_FIVE_FOUR_THREE, 0 ); }
    }

    // Utility work.
    if( m_GameType != GAME_CAMPAIGN )
    {
        for( s32 i = 0; i < 32; i++ )
        {
            if( m_Score.Player[i].IsInGame )
            {
                m_Score.Player[i].TeamTime += DeltaTime;

                if( m_Utility[i].GreetingTimer > 0.0f )
                {
                    m_Utility[i].GreetingTimer -= DeltaTime;
                    if( m_Utility[i].GreetingTimer <= 0.0f )
                    {
                        // We want to send this message to the CLIENT.  But, we
                        // only want to send it ONE TIME.  We need to send it to
                        // (a) the first player in a split screen game, and to 
                        // (b) clients.  Note that GetLocalSlot returns 0,1,2,3
                        // for successive split screen players, and -1 for all
                        // clients.  So, we should only send the message if
                        // GetLocalSlot returns a 0 (first player in split) or
                        // a -1 (client).  Said differently, if the return value 
                        // is less than or equal to 0.
                        if( g_NetworkMgr.GetLocalSlot( i ) <= 0 )
                            MsgMgr.Message( msg_id( MSG_WELCOME_01 + x_irand( 0, 6 ) ),
                                            m_Score.Player[i].ClientIndex );

                        m_Utility[i].GreetingTimer = 0.0f;
                    }
                }
            }
        }
    }

    // Voting logic.
    if( m_VoteInProgress )
    {
        m_VoteTimer -= DeltaTime;

        // Did we just cross the 5 second boundary?
        if( (m_VoteTimer <= 5.0f) && ((m_VoteTimer + DeltaTime) > 5.0f) )
        {
            LOG_MESSAGE( "game_mgr::Logic", "Vote timed out." );

            // Time is up, check the results.
            s32 Votes = m_VotesYes + m_VotesNo;
            VoteResult( (Votes != 0) && 
                        (((m_VotesYes * 100) / (Votes)) >= m_VotePercent) );
        }

        if( m_VoteTimer <= 0.0f )
        {
            ExecuteVote();
        }
    }
    UpdateVoteRights();

    // Run zone logic that only runs on the server.
    ZoneLogicServer( DeltaTime );

    // Update music levels for players.
    pGameLogic->UpdateMusic();

    // Time for a clock update?
    if( m_ClockMode )
    {
        m_ClockUpdate -= DeltaTime;
        if( m_ClockUpdate < 0.0f )
        {
            m_ClockUpdate = 30.0f;
            m_DirtyBits  |= DIRTY_CLOCK;
        }   
    }

    // Game over because of a score limit?
    if( m_ScoreLimit > 0 )
    {
        if( m_Score.IsTeamBased )
        {
            if( (m_Score.Team[0].Score >= m_ScoreLimit) || 
                (m_Score.Team[1].Score >= m_ScoreLimit) )
            {
                EndGame( TRUE );
                return;
            }
        }
        else
        {
            for( s32 i = 0; i < 32; i++ )
            {
                if( (m_Score.Player[i].IsInGame) &&
                    (m_Score.Player[i].Score >= m_ScoreLimit) )
                {
                    EndGame( TRUE );
                    return;
                }
            }
        }
    }
    
    // Game over because of time limit?

    if( (m_ClockMode == -1) && (m_Clock <= 0.0f) && (m_GameType != GAME_CAMPAIGN) )
    {
        EndGame( TRUE );
        return;
    }

    if( (m_ClockMode == 1) && (m_Clock >= m_ClockLimit) && (m_GameType != GAME_CAMPAIGN) )
    {
        EndGame( TRUE );
        return;
    }

    m_Score.ScoreFieldMask = pGameLogic->GetScoreFields();

    #ifdef mtraub
    // Add a test bot?
    static s32 AddBot = 0;
    if( (m_GameType != GAME_CAMPAIGN) && (g_NetworkMgr.IsServer()) )
    {
        while( AddBot > 0 )
        {
            s32 Index = -1;
            Connect( Index, -1, -1, xwstring("Bot"), SKIN_BOT_THETA, TRUE );
            EnterGame( Index );
            AddBot -= 1;
        }
    }

    /*
    // Game over because the monkeys have killed themselves enough?
    if( m_Score.IsTeamBased )
    {
        if( (m_Score.Team[0].Score <= -50) || 
            (m_Score.Team[1].Score <= -50) )
        {
            EndGame();
            return;
        }
    }
    else
    {
        for( s32 i = 0; i < 32; i++ )
        {
            if( (m_Score.Player[i].IsInGame) &&
                (m_Score.Player[i].Score <= -50) )
            {
                EndGame();
                return;
            }
        }
    }
    */
    #endif
}

//==============================================================================

xbool game_mgr::IsZoneColored( s32 Zone )
{   
    if( m_Zone[Zone].Locked )
        return( TRUE );

    if( m_ZonePhase && m_Zone[Zone].Warned )
        return( TRUE );

    return( FALSE );
}

//==============================================================================

static f32    s_PulseTime = 2.0f;
static xcolor s_PulseColor( 31, 0, 0, 127 );

void game_mgr::ZoneLogicAll( f32 DeltaTime )
{
    if( !m_MapScaling )
        return;

    m_ZonePulse += DeltaTime;
    if( m_ZonePulse >= s_PulseTime )
    {
        m_ZonePulse = 0.0f;
        m_ZonePhase = 1 - m_ZonePhase;

        if( m_ZonePhase )
        {
            // If the local player is in a warned zone, kick off a sound.
            s32 Local = g_NetworkMgr.GetLocalPlayerSlot(0);
            object* pPlayer = (object*)NetObjMgr.GetObjFromSlot( Local );
            if( pPlayer )
            {
                s32 Z = pPlayer->GetZone1();
                if( m_Zone[Z].Warned )
                    MsgMgr.Message( MSG_EVACUATE_AREA, Local );
            }
        }
    }

    f32 Pulse  = m_ZonePulse / s_PulseTime;
    f32 Scalar = (1.0f - x_cos( Pulse * R_360 )) * 0.5f;

    m_ZoneColor   =      s_PulseColor;
    m_ZoneColor.A = (u8)(s_PulseColor.A * Scalar);
}

//==============================================================================

void game_mgr::ZoneLogicServer( f32 DeltaTime )
{
    if( !m_MapScaling )
        return;

    m_ZoneTimer -= DeltaTime;
    if( m_ZoneTimer <= 0.0f )
    {
        s32 Players = MAX( m_ZoneMinimum, m_Score.NPlayers );

        m_ZoneTimer = x_frand( 15.0f, 20.0f );

        for( s32 z = 0; z < 255; z++ )
        {
            if( m_Zone[z].Locked &&
                IN_RANGE( m_Zone[z].Minimum, Players, m_Zone[z].Maximum ) )
            {
                ASSERT( !m_Zone[z].Warned );
                m_Zone[z].Locked = FALSE;
                m_DirtyBits |= DIRTY_ZONE_STATE;
            }
            
            else if( m_Zone[z].Warned )
            {
                ASSERT( !m_Zone[z].Locked );
                m_Zone[z].Warned = FALSE;
                m_Zone[z].Locked = TRUE;
                m_DirtyBits |= DIRTY_ZONE_STATE;
            }
            
            // Zone was open.  (That is, NOT closed and NOT warned.)
            else if( !m_Zone[z].Locked && 
                     !IN_RANGE( m_Zone[z].Minimum, Players, m_Zone[z].Maximum ) )
            {
                m_Zone[z].Warned = TRUE;
                m_DirtyBits |= DIRTY_ZONE_STATE;
            }
        }
    }
}

//==============================================================================

void game_mgr::SetTeamDamage( f32 TeamDamage )
{
    m_TeamDamageFactor = TeamDamage;
}

//==============================================================================

f32 game_mgr::GetTeamDamage( void )
{
    return( m_TeamDamageFactor );
}

//==============================================================================

void game_mgr::SetMaxPlayers( s32 MaxPlayers )
{
    m_MaxPlayers = MaxPlayers; 
}

//==============================================================================

void game_mgr::SetVotePercent( s32 VotePercent )
{
    m_VotePercent = VotePercent;
}

//==============================================================================

void game_mgr::BeginGame( void )
{
    s32 i;

    ASSERT( m_Initialized );

    if( m_GameInProgress )
    {
        LOG_ERROR( "game_mgr::BeginGame", "Game already in progress." );
        ASSERT( FALSE );
        return;
    }

    LOG_MESSAGE( "game_mgr::BeginGame", "Players:%d", m_Score.NPlayers );

    // Restore the default team names.
    if( g_NetworkMgr.IsServer() )
    {
        if( m_MutationMode == MUTATE_HUMAN_VS_MUTANT )
        {
            // HUMANS
            s_TeamName[0][0] = '\\'; 
            s_TeamName[0][1] = 'H'; 
            s_TeamName[0][2] = 'U'; 
            s_TeamName[0][3] = 0;

            // MUTANTS
            s_TeamName[1][0] = '\\'; 
            s_TeamName[1][1] = 'M'; 
            s_TeamName[1][2] = 'U'; 
            s_TeamName[1][3] = 0;
        }
        else
        {
            // ALPHA
            s_TeamName[0][0] = '\\'; 
            s_TeamName[0][1] = 'A'; 
            s_TeamName[0][2] = 'L'; 
            s_TeamName[0][3] = 0;

            // OMEGA
            s_TeamName[1][0] = '\\'; 
            s_TeamName[1][1] = 'O'; 
            s_TeamName[1][2] = 'M'; 
            s_TeamName[1][3] = 0;
        }

        Localize( m_Score.Team[0].Name, s_TeamName[0] );
        Localize( m_Score.Team[1].Name, s_TeamName[1] );

        x_mstrcpy( m_Score.Team[0].NName, m_Score.Team[0].Name );
        x_mstrcpy( m_Score.Team[1].NName, m_Score.Team[1].Name );

        m_DirtyBits |= DIRTY_TEAM_NAMES;
    }

    // We are now LIVE.
    m_GameInProgress   = TRUE;
    m_Score.IsGameOver = FALSE;
    m_Minutes5         = FALSE; 
    m_Minutes2         = FALSE;
    m_Seconds60        = FALSE;
    m_Seconds30        = FALSE;
    m_Seconds10        = FALSE;
    m_Seconds05        = FALSE;
    m_ZonePulse        = 0.0f;
    m_ZonePhase        = 0;

    // Clear the local player stats.
    m_PlayTime = 0.0f;
    x_memset( &m_PlayerStats, 0, sizeof(m_PlayerStats) );

    // Ghastly HACK for MP in A51.
    // Ghastly HACK for MP in A51.
    g_MPTweaks.Active = (m_GameType != GAME_CAMPAIGN) || 
                        (g_ObjMgr.GetNumInstances( object::TYPE_MP_SETTINGS ));
    // Ghastly HACK for MP in A51.
    // Ghastly HACK for MP in A51.

    // End of the road for the client.
    if( g_NetworkMgr.IsClient() )
        return;

    // Reset the clock.
    if( m_ClockMode == -1 )
    {
        m_Clock = (f32)m_ClockLimit;
    }
    if( m_ClockMode == 1 )
    {
        m_Clock = 0.0f;
    }

    // Bring in the initial values for the circuits.
    for( i = 0; i < MAX_CIRCUITS; i++ )
    {
        m_CircuitBits[i] = mp_settings::GetTeamBits( i, m_GameType );
    }

    // Let the GameLogic in on the act.
    pGameLogic->BeginGame();

    // Set dirty bits.
    m_DirtyBits  = 0xFFFFFFFF;
    m_ScoreBits  = 0xFFFFFFFF;
    m_PlayerBits = 0xFFFFFFFF;

    // Clear all player scores.
    for( i = 0; i < NET_MAX_PLAYERS; i++ )
    {
        m_Score.Player[i].ClearScore();
    }

    // Clear all team scores.
    for( i = 0; i < 2; i++ )
    {
        m_Score.Team[i].ClearScore();
    }

    // Clear the highlight mask.
    m_Score.HighLightMask = 0x00000000;

    // Prepare the voting system.
    m_UpdateVoteRights = TRUE;
    m_VoteInProgress   = FALSE;
    m_VoteKick         = -1;
    m_VoteMap          = -1;

    // Prepare the zones.
    {
        s32 Players = m_MapScaling 
                    ? MAX( m_ZoneMinimum, m_Score.NPlayers )
                    : 32;

        m_ZoneTimer = x_frand( 5.0f, 10.0f );
        for( s32 z = 0; z < 255; z++ )
        {
            if( (Players >= m_Zone[z].Minimum) &&
                (Players <= m_Zone[z].Maximum) )
            {
                m_Zone[z].Warned = FALSE;
                m_Zone[z].Locked = FALSE;
            }
            else
            {
                m_Zone[z].Warned = FALSE;
                m_Zone[z].Locked = TRUE;
            }
        }
    }

    // Create all the players.
    for( i = 0; i < NET_MAX_PLAYERS; i++ )
    {
        if( m_Score.Player[i].IsInGame )
            CreatePlayer( i );
    }
}

//==============================================================================

void game_mgr::EndGame( xbool Complete )
{
    s32 i;

    if( !m_GameInProgress )
    {
        LOG_ERROR( "game_mgr::EndGame", "Game not in progress." );
        ASSERT( FALSE );
        return;
    }

    LOG_MESSAGE( "game_mgr::EndGame", "" );

    for( i = 0; i < NET_MAX_PLAYERS; i++ )
    {
        if( m_Score.Player[i].IsConnected )
        {
            if( m_Score.Player[i].IsInGame )
            {
                LOG_WARNING( "game_mgr::EndGame",
                             "Score:%02d - Deaths:%02d - Player[%d]:%s",
                             m_Score.Player[i].Score,
                             m_Score.Player[i].Deaths, i,
                             m_Score.Player[i].NName );
            }
            else
            {
                LOG_WARNING( "game_mgr::EndGame",
                             "Player[%d]:%s", i,
                             m_Score.Player[i].Name );
            }
        }
    }

    // Halt the game from the GameMgr's point of view.
    m_Score.ComputeRanks();
    m_Score.IsGameOver = TRUE;
    m_GameInProgress   = FALSE;

    // Preserve the final score.
    m_FinalScore = m_Score;

    // The polls (if any) are closed.
    ShutDownVote();

    if( g_NetworkMgr.IsServer() )
    {
        // TO DO - Nuke any bots.

        ASSERT( pGameLogic );
        pGameLogic->EndGame( Complete );
    }

    // Clients just kill all their score data.
    if( g_NetworkMgr.IsClient() )
    {
        for( i = 0; i < NET_MAX_PLAYERS; i++ )
        {
            m_Score.Player[i].ClearAll();
        }

        m_Score.Team[0].ClearAll();
        m_Score.Team[1].ClearAll();
    }

#ifdef AUDIO_ENABLE
    pGameLogic->UpdateMusic();
#endif
}

//==============================================================================

void game_mgr::UpdatePlayerStats( void )
{
    s32 Local = g_NetworkMgr.GetLocalPlayerSlot( 0 );

    if( Local != -1 )
    {
        ASSERT( IN_RANGE( 0, Local, 31 ) );

        const player_score& P = GetScore().Player[Local];

        m_PlayerStats.KillsAsHuman  = P.Kills;
        m_PlayerStats.KillsAsMutant = 0;
        m_PlayerStats.Deaths        = P.Deaths;
        m_PlayerStats.PlayTime      = (s32)((m_PlayTime+1) / 60);
        m_PlayerStats.Games         = P.Games;
        m_PlayerStats.Wins          = P.Wins;
        m_PlayerStats.Gold          = P.Gold;
        m_PlayerStats.Silver        = P.Silver;
        m_PlayerStats.Bronze        = P.Bronze;
        m_PlayerStats.VotesStarted  = P.Votes;

        LOG_MESSAGE( "game_mgr::UpdatePlayerStats", 
                     "Play:%d(%s) - PlayTime:%d - Games:%d", 
                     Local, P.NName, m_PlayerStats.PlayTime, P.Games );
    }
}

//==============================================================================

void game_mgr::AddKickStat( void )
{
    m_PlayerStats.Kicks = 1;
}

//==============================================================================

void game_mgr::SetClock( f32 Seconds )
{
    m_Clock       = Seconds;
    m_ClockUpdate = 0.0f;
    m_DirtyBits  |= DIRTY_CLOCK;
}

//==============================================================================

void game_mgr::SetClockLimit( s32 Seconds )
{
    ASSERT( Seconds > 0 );

    m_ClockLimit  = Seconds;
    m_ClockUpdate = 0.0f;
    m_DirtyBits  |= DIRTY_CLOCK_CONFIG;

    // Make sure current clock is valid.
    if( (m_ClockMode == -1) && (m_Clock > Seconds) )
    {
        // Example: Was counting down from 20min to 0 with clock at 15min.
        //          Then limit was set to 10min.  Change clock to 10min.
        m_Clock = (f32)Seconds;
    }
}

//==============================================================================

void game_mgr::SetClockMode( s32 Mode )
{
    ASSERT( IN_RANGE( -1, Mode, 1 ) );
    m_ClockMode   = Mode;
    m_ClockUpdate = 0.0f;
    m_DirtyBits  |= DIRTY_CLOCK_CONFIG;
}

//==============================================================================

f32 game_mgr::GetClock( void )
{
    return( m_Clock );
}

//==============================================================================

s32 game_mgr::GetClockLimit( void )
{
    return( m_ClockLimit );
}

//==============================================================================

s32 game_mgr::GetClockMode( void )
{
    return( m_ClockMode );
}

//==============================================================================

void game_mgr::SetScoreLimit( s32 ScoreLimit )
{
    m_ScoreLimit = ScoreLimit;
}

//==============================================================================

f32 game_mgr::GetGameProgress( void )
{
    s32 i;
    
    ASSERT( pGameLogic );
    f32 Progress = pGameLogic->GetGameProgress();

    if( Progress >= -0.1f )
        return( Progress );

    if( m_ScoreLimit > 0 )
    {
        if( m_Score.IsTeamBased )
        {
            for( i = 0; i < 2; i++ )
            {
                f32 V = (f32)m_Score.Team[i].Score / (f32)m_ScoreLimit;
                Progress = MAX( Progress, V );
            }
        }
        else
        {
            for( i = 0; i < 32; i++ )
            {
                if( m_Score.Player[i].IsInGame )
                {
                    f32 V = (f32)m_Score.Player[i].Score / (f32)m_ScoreLimit;
                    Progress = MAX( Progress, V );
                }
            }
        }
    }

    if( m_ClockMode == -1 )
    {
        Progress = MAX( Progress, 1.0f - (m_Clock / m_ClockLimit) );
    }

    if( m_ClockMode == 1 )
    {
        Progress = MAX( Progress, (m_Clock / m_ClockLimit) );
    }

    Progress = x_clamp( Progress, 0.0f, 1.0f );

    return( Progress );
}

//==============================================================================

xbool game_mgr::RoomForPlayers( s32 Players )
{
    return( (m_Score.NConnected + Players) <= m_MaxPlayers );
}

//==============================================================================

void game_mgr::Connect(       s32&    PlayerIndex,
                              s32     ClientIndex,
                              u64     Identifier,
                        const xwchar* pName,
                              s32     Skin,
                              xbool   Bot )
{
    ASSERT( m_Initialized );

    PlayerIndex = -1;

    if( g_NetworkMgr.IsClient() )
        return;

    // TO DO - If we support bots of any kind, may need to nuke a bot here.

    // Bot and already have maximum players?
    if( Bot && (m_Score.NConnected == m_MaxPlayers) )
    {
        ASSERT( FALSE );
        return;
    }

    // Can we let this player this player in?
    if( m_Score.NConnected == m_MaxPlayers )
    {
        char NName[NET_NAME_LENGTH];
        x_mstrcpy( NName, pName );
        LOG_ERROR( "game_mgr::Connect", 
                   "Result:Failed, server full - Name:%s", 
                   NName );
        return;
    }

    // Find available position for the new player.

    if( m_GameType == GAME_CAMPAIGN )
    {
        ASSERT( !m_Score.Player[0].IsConnected );
        s_PlayerCursor = 0;
    }

    if( (m_Score.NConnected == 0) && (!m_GameInProgress) )
        s_PlayerCursor = 0;

    while( PlayerIndex == -1 )
    {
        if( !m_Score.Player[s_PlayerCursor].IsConnected )
            PlayerIndex = s_PlayerCursor;

        s_PlayerCursor = (s_PlayerCursor+1) % 32;
    }
    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    player_score& Player = m_Score.Player[PlayerIndex];

    Player.ClearAll();  // TO DO - Rather than clear all, support reconnect.

    // Name game.
    {
        // TO DO - Deal with problematic names.
        ASSERT( pName );
        RegisterName( PlayerIndex, pName );
    }

    m_Options[PlayerIndex].Skin = Skin;
    LOG_MESSAGE( "game_mgr::Connect", "Skin:%i", m_Options[PlayerIndex].Skin );

    Player.IsConnected = TRUE;
    Player.ClientIndex = ClientIndex;
    Player.Identifier  = Identifier;
    Player.IsBot       = Bot;
    m_Score.NConnected++;

    // Let the specific game logic do its thing.  
    // ATTENTION:  This function call MUST set m_Score.Player[?].Team and take
    //             care of m_Score.Team[?].Size.
    ASSERT( pGameLogic );
    pGameLogic->Connect( PlayerIndex );

    // Muting.
    {
        // On XBox this will lookup the mutelist from the XBox Live service
		#if defined ( TARGET_PC )
        {
        }
        #endif
        #if defined ( TARGET_XBOX )
        xbool IsMuted = g_MatchMgr.IsPlayerMuted( Identifier );

        g_VoiceMgr.SetLocallyMuted( PlayerIndex, IsMuted );
        #endif
        /*
        if( IsMuted == TRUE )
            LOG_MESSAGE( "game_mgr::Connect", "Locally muting player %d", PlayerIndex );
        else
            LOG_MESSAGE( "game_mgr::Connect", "Locally un-muting player %d", PlayerIndex );
        */
    }

    // Utility work.
    m_Utility[ PlayerIndex ].GreetingTimer = 1.5f;

    // Announcement for the players on the server.
    if( m_GameInProgress && 
        (!g_NetworkMgr.IsLocal( PlayerIndex )) )
    {
        MsgConnect( PlayerIndex );
    }

    // Set dirty bits.
    m_ScoreBits  |= (1 << PlayerIndex);
    m_PlayerBits |= (1 << PlayerIndex);

    LOG_MESSAGE( "game_mgr::Connect", 
                 "Player:%d - Team:%d - Name:%s, Identifier:%d",
                 Player.Team,
                 PlayerIndex, 
                 Player.NName,
                 Player.Identifier);
}

//==============================================================================

void game_mgr::RegisterName( s32 PlayerIndex, const xwchar* pName )
{
    s32      i;
    s32      Version = 0;
    xwstring Name    = pName;
    xwstring Ext     = "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    ASSERT( Ext.GetLength() > NET_NAME_LENGTH );

    if( Name.GetLength() > NET_NAME_LENGTH-1 )
        Name = Name.Left( NET_NAME_LENGTH-1 );

    // Look for a name collision.
    for( i = 0; i < NET_MAX_PLAYERS; i++ )
    {
        if( (m_Score.Player[i].IsConnected) &&
            (Name == m_Score.Player[i].Name) )
            break;
    }

    // Well?
    if( i < NET_MAX_PLAYERS )
    {
        // We had a name collision.  Crap.  "Adjust" the name.

        if( Name.GetLength() > NET_NAME_LENGTH-3 )
            Name = Name.Left( NET_NAME_LENGTH-3 );

        Name += ".?";

        s32 Edit = Name.GetLength() - 1;

        i = 0;

        do
        {
            Name[Edit] = Ext[Version];
            if( (m_Score.Player[i].IsConnected) &&
                (Name == m_Score.Player[i].Name) )
            {
                Version++;
                i = 0;
            }
            else
            {
                i++;
            }
        } while( i < NET_MAX_PLAYERS );
    }

    // Ship it!
    x_wstrcpy( m_Score.Player[PlayerIndex].Name,  (const xwchar*)Name );
    x_mstrcpy( m_Score.Player[PlayerIndex].NName, (const xwchar*)Name );
}

//==============================================================================

void game_mgr::MsgConnect( s32 PlayerIndex )
{
    s32 Local = g_NetworkMgr.GetLocalPlayerSlot( 0 );

    ASSERT( IN_RANGE( 0, Local      , 31 ) );
    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    if( !m_GameInProgress )
        return;

    if( !m_Score.Player[Local].IsInGame )
        return;

    // TEXT                 AUDIO                           IF
    // "Rico connected."    "Reinforcements incoming."      will be ally
    // "Rico connected."    <nothing>                       otherwise

    if( m_Score.IsTeamBased )
    {
        s32 T = m_Score.Player[Local].Team;
        if( m_Score.Team[T].Size < m_Score.Team[1-T].Size )
        {
            MsgMgr.Message( MSG_P_CONNECTED_ALLY, 0, PlayerIndex );
            return;
        }
    }

    MsgMgr.Message( MSG_P_CONNECTED, 0, PlayerIndex );
}

//==============================================================================

void game_mgr::MsgEnterGame( s32 PlayerIndex )
{
    s32 Local = g_NetworkMgr.GetLocalPlayerSlot( 0 );

    if( !m_GameInProgress )
        return;

    if( !m_Score.Player[Local].IsInGame )
        return;

    // TEXT                        AUDIO                       IF
    // "Rico joined team YOURS."   "Reinforcements arrived."   team ally
    // "Rico joined team OTHER."   "Resistance increased."     team enemy 50%
    // "Rico joined team OTHER."   "Enemy forces increased."   team enemy 50%
    // "Rico entered game."        "New challenger arrived."   non-team 50%
    // "Rico entered game."        "New enemy arrived."        non-team 50%

    if( m_Score.IsTeamBased )
    {
        if( m_Score.Player[Local].Team == m_Score.Player[PlayerIndex].Team )
        {
            MsgMgr.Message( MSG_P_JOINED_TEAM_ALLY, 0, PlayerIndex, m_Score.Player[PlayerIndex].Team );
        }
        else
        {
            MsgMgr.Message( msg_id( MSG_P_JOINED_TEAM_ENEMY_1 + x_irand(0,1) ), 0, PlayerIndex, m_Score.Player[PlayerIndex].Team );
        }
    }
    else
    {
        MsgMgr.Message( msg_id( MSG_P_ENTERED_GAME_1 + x_irand(0,1) ), 0, PlayerIndex );
    }
}

//==============================================================================

void game_mgr::MsgDisconnect( s32 PlayerIndex )
{
    s32 Local = g_NetworkMgr.GetLocalPlayerSlot( 0 );

    if( !m_GameInProgress )
        return;

    if( !m_Score.Player[Local].IsInGame )
        return;

    MsgMgr.Message( MSG_P_DISCONNECTED, 0, PlayerIndex ); 
}

//==============================================================================

void game_mgr::GetSkinValues( s32 Skin, s32& Mesh, s32& Texture )
{
    Mesh    = Skin >> 2;
    Texture = Skin & 0x03;
}

//==============================================================================

const char* game_mgr::GetSkinAudioPrefix( s32 Skin, s32 VoiceActor, xbool bMutated )
{
    // Mutated overrides everything
    if( bMutated )
    {
        switch( VoiceActor & 1 )
        {
        case 0: return "MutatedA";
        case 1: return "MutatedB";
        }
    }
    else
    {
        // Lookup character and # of voice types
        switch( Skin )
        {
            default:
                ASSERTS( 0, "You need to add the new skin type -> audio mapping here!" );
                
            case SKIN_HAZMAT_0:
            case SKIN_HAZMAT_1:
            case SKIN_HAZMAT_2:
            case SKIN_HAZMAT_3:
                switch( VoiceActor & 3 )
                {
                case 0: return "HazmatA";
                case 1: return "HazmatB";
                case 2: return "HazmatC";
                case 3: return "HazmatD";
                }
                break;
                
            case SKIN_SPECFOR_0:
            case SKIN_SPECFOR_1:
            case SKIN_SPECFOR_2:
            case SKIN_SPECFOR_3:
                switch( VoiceActor & 3 )
                {
                case 0: return "SecurityA";
                case 1: return "SecurityB";
                case 2: return "SecurityC";
                case 3: return "SecurityD";
                }
                break;
                
            case SKIN_TECH_0:
            case SKIN_TECH_1:
            case SKIN_TECH_2:
            case SKIN_TECH_3:
                switch( VoiceActor & 3 )
                {
                case 0: return "SecurityA";
                case 1: return "SecurityB";
                case 2: return "SecurityC";
                case 3: return "SecurityD";
                }
                break;
                
            case SKIN_GREY_0:
            case SKIN_GREY_1:
            case SKIN_GREY_2:
            case SKIN_GREY_3:
                return "GrayA";
                
            case SKIN_BOT_GRUNT:
            case SKIN_BOT_THETA:
            case SKIN_BOT_LEAPER:
            case SKIN_BOT_BLACKOP:
            case SKIN_BOT_MITE:
            case SKIN_BOT_GRAY:
            case SKIN_BOT_ALIEN:
                return "GrayA";
        }    
    }

    // Default    
    return "HazmatA";
}

//==============================================================================

const char* game_mgr::GetSkinBlueprint( s32 Skin )
{
    switch( Skin )
    {
        default:
            ASSERT( FALSE );

        case SKIN_BOT_THETA:
            return "C:\\GameData\\A51\\Source\\Themes\\AI_Test\\Blueprint\\AITest_Theta.bpx";

        // "AITest_DustMite.bpx"
        // "AITest_Gray.bpx"
        // "AITest_Grunt_HazMat_SHT.bpx"
        // "AITest_Grunt_HazMat_SMP.bpx"
        // "AITest_Grunt_HazMat_Unarmed.bpx"
        // "AITest_Grunt_Scientist_Unarmed.bpx"
        // "AITest_Grunt_Spec4_SHT.bpx"
        // "AITest_Grunt_Spec4_SMP.bpx"
        // "AITest_Grunt_Spec4_Unarmed.bpx"
        // "AITest_Leaper.bpx"
        // "AITest_MIL_BlackOps_Leader_MasonCannon.bpx"
        // "AITest_MIL_BlackOps_Leader_SHT.bpx"
        // "AITest_MIL_BlackOps_Leader_SMP.bpx"
        // "AITest_MIL_BlackOps_MasonCannon.bpx"
        // "AITest_MIL_BlackOps_SHT.bpx"
        // "AITest_MIL_BlackOps_SMP.bpx"
        // "AITest_MIL_BlackOps_SNI.bpx"
        // "AITest_Theta.bpx"
        // "AITest_UberTheta.bpx"

    }
}

//==============================================================================

void game_mgr::Disconnect( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    ASSERT( m_Initialized );
    ASSERT( PlayerIndex >=  0 );
    ASSERT( PlayerIndex <  NET_MAX_PLAYERS );
    ASSERT(  m_Score.Player[PlayerIndex].IsConnected );
    ASSERT( !m_Score.Player[PlayerIndex].IsInGame    );
    ASSERT( m_Score.NConnected );

    player_score& Player = m_Score.Player[PlayerIndex];

    // Let the specific game logic do its thing.
    pGameLogic->Disconnect( PlayerIndex );

    // Announcement for the players on the server.
    if( m_GameInProgress && 
        (m_GameType != GAME_CAMPAIGN) &&
        (!g_NetworkMgr.IsLocal( PlayerIndex )) )
    {
        MsgDisconnect( PlayerIndex );
    }
    
    m_Score.NConnected--;

    // Set dirty bits.
    m_ScoreBits  |= (1 << PlayerIndex);
    m_PlayerBits |= (1 << PlayerIndex);

    LOG_MESSAGE( "game_mgr::Disconnect",
                 "Slot:%d - Name:%s", 
                 PlayerIndex, Player.NName );

    Player.ClearAll();  // TO DO - Support reconnect.
}

//==============================================================================

void game_mgr::CreatePlayer( s32 PlayerIndex )
{
    LOG_MESSAGE( "game_mgr::CreatePlayer", "Slot:%d", PlayerIndex );

    ASSERT( m_Initialized );
//  ASSERT( m_GameInProgress ); // HACKOMOTRON 3000 - Bring this back later.

    actor* pActor = NULL;

    NetObjMgr.ReserveSlot( PlayerIndex );

    //
    // Is this for the campaign player?
    //

    if( m_GameType == GAME_CAMPAIGN )
    {
        ASSERT( PlayerIndex == 0 );
        ASSERT( g_NetworkMgr.IsLocal( 0 ) );

        // The campaign logic must personally create the player in order to 
        // force it to have a predefined guid.  This happens in BeginGame which
        // occurs before now.  Thus, the player should already be there.  If
        // he is there, then "bind" to him and be done.

        g_ObjMgr.SelectByAttribute( object::ATTR_ALL, object::TYPE_PLAYER );
        select_slot_iterator SlotIter;

        for( SlotIter.Begin(); !SlotIter.AtEnd(); SlotIter.Next() )
        {
            pActor = (actor*)SlotIter.Get();
            ASSERT( pActor );

            // At this point, the player code will magically bind to the 
            // existing player.  So nothing more to do.  For now.
        }
        SlotIter.End();

        if( pActor )
        {                   
            LOG_MESSAGE( "game_mgr::CreatePlayer", 
                         "PLAYER - Used campaign logic player." );
        }
        else
        {
            // If, for some reason, we DON'T have a player, then something is
            // amiss.  However, in order to facilitate development, we will go 
            // ahead and create a player here and now.
            LOG_ERROR( "game_mgr::CreatePlayer", 
                       "PLAYER - Could NOT find campaign player.  Created new player." );
            guid GUID = g_ObjMgr.CreateObject( "Player" );
            pActor = (actor*)g_ObjMgr.GetObjectByGuid( GUID );
        }
    }

    //
    // Is this for a LOCAL MP player?
    //
    if( (m_GameType != GAME_CAMPAIGN) && 
        (g_NetworkMgr.IsLocal( PlayerIndex )) &&
        (!m_Score.Player[PlayerIndex].IsBot) )
    {
        LOG_MESSAGE( "game_mgr::CreatePlayer", 
                     "PLAYER - Created local MP player. Skin: %i", m_Options[PlayerIndex].Skin );
        guid GUID = g_ObjMgr.CreateObject( "Player" );
        pActor = (actor*)g_ObjMgr.GetObjectByGuid( GUID );

        // Set the player's skin.
        pActor->net_SetSkin( m_Options[PlayerIndex].Skin );
    }

    //
    // Is this for a REMOTE MP player?  (That is, a GHOST.)
    //
    if( (m_GameType != GAME_CAMPAIGN) && 
        (!g_NetworkMgr.IsLocal( PlayerIndex )) &&
        (!m_Score.Player[PlayerIndex].IsBot) )
    {
        LOG_MESSAGE( "game_mgr::CreatePlayer", 
                     "GHOST - Created ghost.  Skin:%i", m_Options[PlayerIndex].Skin );
        guid GUID = g_ObjMgr.CreateObject( "NetGhost" );
        pActor = (actor*)g_ObjMgr.GetObjectByGuid( GUID );

        // Set the ghost's skin.
        // Could be ghost of a player or a bot.
        pActor->net_SetSkin( m_Options[PlayerIndex].Skin );
    }

    //
    // Is this for a MP bot?
    //
    if( (g_NetworkMgr.IsServer()) &&
        (m_Score.Player[PlayerIndex].IsBot) )
    {
        LOG_MESSAGE( "game_mgr::CreatePlayer", "BOT - Created NPC bot." );

        // Must use a blueprint for creating a bot character.
        const char* pBlueprint = GetSkinBlueprint( m_Options[PlayerIndex].Skin );
        guid GUID = g_TemplateMgr.CreateSingleTemplate( pBlueprint, 
                                vector3(0,0,0), radian3(R_0,R_0,R_0), 0, 0 );
        pActor = (actor*)g_ObjMgr.GetObjectByGuid( GUID );
    }

//  pActor->net_SetSkin( m_Options[PlayerIndex].Skin );
    pActor->InitInventory();

    NetObjMgr.AddObject( pActor->AsNetObj() );

    if( g_NetworkMgr.IsClient() )
        return;

    // Set the player's team bits.
    pActor->net_SetTeamBits( pGameLogic->GetTeamBits( PlayerIndex ) );
                   
    // Spawn!
    pGameLogic->RequestSpawn( PlayerIndex );

    // Set his voting privileges.
    m_UpdateVoteRights = TRUE;
}

//==============================================================================

void game_mgr::EnterGame( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    ASSERT( m_Initialized );
    ASSERT( PlayerIndex >=  0 );
    ASSERT( PlayerIndex <  32 );
    ASSERT(  m_Score.Player[PlayerIndex].IsConnected );
    ASSERT( !m_Score.Player[PlayerIndex].IsInGame );

    player_score& Player = m_Score.Player[PlayerIndex];

    Player.Score  = 0;      // TO DO - Support reconnect.
    Player.Kills  = 0;
    Player.Deaths = 0;
    Player.TKs    = 0;
    Player.Flags  = 0;
    Player.Votes  = 0;

    m_Score.NPlayers++;

    // Let the specific game logic do its thing.  
    ASSERT( pGameLogic );
    pGameLogic->EnterGame( PlayerIndex );

    Player.IsInGame = TRUE;

    // If a game is already running, create the player or bot object.
    if( m_GameInProgress )
    {
        CreatePlayer( PlayerIndex );
    }

    // Announcement for the players on the server.
    if( !g_NetworkMgr.IsLocal( PlayerIndex ) )
    {
        MsgEnterGame( PlayerIndex );
    }

    // Set dirty bits.
    m_ScoreBits  |= (1 << PlayerIndex);
    m_PlayerBits |= (1 << PlayerIndex);
    m_UpdateVoteRights = TRUE;

    LOG_MESSAGE( "game_mgr::EnterGame",
                 "Slot:%d - Name:%s",
                 PlayerIndex, Player.NName );
}

//==============================================================================

void game_mgr::ExitGame( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    player_score& Player = m_Score.Player[PlayerIndex];

    ASSERT( m_Initialized );
    ASSERT( PlayerIndex >=  0 );
    ASSERT( PlayerIndex <  32 );
    ASSERT( Player.IsConnected );

    if( Player.IsInGame )
    {
        // Let the specific game logic do its thing.  
        ASSERT( pGameLogic );
        pGameLogic->ExitGame( PlayerIndex );

        Player.IsInGame = FALSE;

        // TO DO - Preserve these values for possible reconnect.
        Player.Score    = 0;
        Player.TKs      = 0;
        Player.Deaths   = 0;
        Player.Flags    = 0;
        Player.Votes    = 0;

        m_Score.NPlayers--;

//??    ObjMgr.UnbindOriginID( m_PlayerID[PlayerIndex] );

        // Set dirty bits.
        m_ScoreBits  |= (1 << PlayerIndex);
        m_PlayerBits |= (1 << PlayerIndex);
        m_UpdateVoteRights = TRUE;

        // Destroy the player object if it exists.
        if( m_GameInProgress )
        {
            NetObjMgr.DestroyObject( PlayerIndex );
        }

        // Cancel kick vote if this player was the object of the vote.
        if( m_VoteKick == PlayerIndex )
        {
            g_NetworkMgr.KickPlayer( m_VoteKick );
            ShutDownVote();
        }

        LOG_MESSAGE( "game_mgr::ExitGame",
                     "Slot:%d - Name:%s",
                     PlayerIndex, Player.NName );
    }
    else
    {
        LOG_WARNING( "game_mgr::ExitGame",
                     "PLAYER WAS NOT 'IN GAME' - Slot:%d - Name:%s",
                     PlayerIndex, Player.NName );
    }
}

//==============================================================================

const game_score& game_mgr::GetScore( void )
{
    #ifdef mtraub
    if( s_UseFakeScore )
        return( s_FakeScore );
    #endif

    if( m_Score.IsGameOver )
        return( m_FinalScore );
    else
        return( m_Score );
}

//==============================================================================

void game_mgr::ProvideFinalScore( bitstream& BS, s32 ClientIndex )
{
    ASSERT( !m_GameInProgress );

    s32 i;
    u32 Bits = 0x00;
    u32 Mask = m_FinalScore.ScoreFieldMask;

    BS.WriteRangedU32( Mask, 0, SCORE_ALL );

    for( i = 0; i < 32; i++ )
    {
        player_score& P = m_FinalScore.Player[i];

        if( BS.WriteFlag( (P.IsConnected) &&
                          (P.IsInGame) ) )
        {
            if( Mask & SCORE_POINTS )    BS.WriteVariableLenS32( P.Score  );
            if( Mask & SCORE_KILLS  )    BS.WriteVariableLenS32( P.Kills  );
            if( Mask & SCORE_DEATHS )    BS.WriteVariableLenS32( P.Deaths );
            if( Mask & SCORE_TKS    )    BS.WriteVariableLenS32( P.TKs    );
            if( Mask & SCORE_FLAGS  )    BS.WriteVariableLenS32( P.Flags  );
            if( Mask & SCORE_VOTES  )    BS.WriteVariableLenS32( P.Votes  );

            if( BS.WriteFlag( ClientIndex == P.ClientIndex ) )
            {
                BS.WriteFlag( P.Games  );   
                BS.WriteFlag( P.Wins   );    
                BS.WriteFlag( P.Gold   );
                BS.WriteFlag( P.Silver );
                BS.WriteFlag( P.Bronze );
            }

            Bits |= (1 << i);

            if( ClientIndex == P.ClientIndex )
            {
                LOG_WARNING( "game_mgr::ProvideFinalScore",
                             "Score:%02d - Kills:%02d - Deaths:%02d - TKs:%02d - "
                             "Flags:%02d - Votes:%02d - Player[%d]:%s - Games:%d",
                             P.Score,
                             P.Kills,
                             P.Deaths, 
                             P.TKs, 
                             P.Flags, 
                             P.Votes, 
                             i,
                             P.NName,
                             P.Games );
            }
            else
            {
                LOG_WARNING( "game_mgr::ProvideFinalScore",
                             "Score:%02d - Kills:%02d - Deaths:%02d - TKs:%02d - "
                             "Flags:%02d - Votes:%02d - Player[%d]:%s",
                             P.Score,
                             P.Kills,
                             P.Deaths, 
                             P.TKs, 
                             P.Flags, 
                             P.Votes, 
                             i,
                             P.NName );
            }
        }
    }

    if( BS.WriteFlag( m_FinalScore.IsTeamBased ) )
    {
        BS.WriteS32( m_FinalScore.Team[0].Score );
        BS.WriteS32( m_FinalScore.Team[1].Score );
    }

    LOG_MESSAGE( "game_mgr::ProvideFinalScore", "Players:0x%08X", Bits );

    ASSERT( !BS.IsFull() );
}

//==============================================================================

void game_mgr::AcceptFinalScore( bitstream& BS )
{
    ASSERT( !m_GameInProgress );

    s32 i;
    u32 Bits = 0x00;
    u32 Mask = 0x00;

    BS.ReadRangedU32( Mask, 0, SCORE_ALL );
    m_FinalScore.ScoreFieldMask = Mask;

    m_FinalScore.NConnected     = 0;
    m_FinalScore.NPlayers       = 0;

    for( i = 0; i < 32; i++ )
    {
        player_score& P = m_FinalScore.Player[i];

        if( BS.ReadFlag() )
        {
            if( !P.IsInGame )
            {
                LOG_ERROR( "game_mgr::AcceptFinalScore( error )",
                           "RECEIVED FINAL SCORE FOR PLAYER(%d) NOT IN GAME", i );
            }
            else
            {
                m_FinalScore.NConnected++;
                m_FinalScore.NPlayers++;
            }

            if( Mask & SCORE_POINTS )    BS.ReadVariableLenS32( P.Score  );
            if( Mask & SCORE_KILLS  )    BS.ReadVariableLenS32( P.Kills  );
            if( Mask & SCORE_DEATHS )    BS.ReadVariableLenS32( P.Deaths );
            if( Mask & SCORE_TKS    )    BS.ReadVariableLenS32( P.TKs    );
            if( Mask & SCORE_FLAGS  )    BS.ReadVariableLenS32( P.Flags  );
            if( Mask & SCORE_VOTES  )    BS.ReadVariableLenS32( P.Votes  );

            if( BS.ReadFlag() )
            {
                BS.ReadFlag( P.Games  );   
                BS.ReadFlag( P.Wins   );    
                BS.ReadFlag( P.Gold   );
                BS.ReadFlag( P.Silver );
                BS.ReadFlag( P.Bronze );

                LOG_WARNING( "game_mgr::AcceptFinalScore",
                             "Score:%02d - Kills:%02d - Deaths:%02d - TKs:%02d - "
                             "Flags:%02d - Votes:%02d - Player[%d]:%s - Games:%d",
                             P.Score,
                             P.Kills,
                             P.Deaths, 
                             P.TKs, 
                             P.Flags, 
                             P.Votes, 
                             i,
                             P.NName,
                             P.Games );

                UpdatePlayerStats();
            }
            else
            {
                LOG_WARNING( "game_mgr::AcceptFinalScore",
                             "Score:%02d - Kills:%02d - Deaths:%02d - TKs:%02d - "
                             "Flags:%02d - Votes:%02d - Player[%d]:%s",
                             P.Score,
                             P.Kills,
                             P.Deaths, 
                             P.TKs, 
                             P.Flags, 
                             P.Votes, 
                             i,
                             P.NName );
            }

            Bits |= (1 << i);
        }
    }

    if( BS.ReadFlag() )
    {
        BS.ReadS32( m_FinalScore.Team[0].Score );
        BS.ReadS32( m_FinalScore.Team[1].Score );
    }

    m_FinalScore.ComputeRanks();

    LOG_MESSAGE( "game_mgr::AcceptFinalScore", "Players:0x%08X", Bits );
}

//==============================================================================

void game_mgr::GetClearDirty( u32& DirtyBits, u32& ScoreBits, u32& PlayerBits )
{
    ASSERT( m_GameInProgress );

    DirtyBits  = m_DirtyBits;
    ScoreBits  = m_ScoreBits;
    PlayerBits = m_PlayerBits;

    m_DirtyBits  = 0x00;
    m_ScoreBits  = 0x00;
    m_PlayerBits = 0x00;

    // TODO - Make sure this works in split screen and with 0 clients.

    if( ScoreBits )
        m_Score.ComputeRanks();
}

//==============================================================================

void game_mgr::AcceptUpdate( const bitstream& BS )
{
    ASSERT( g_NetworkMgr.IsClient() );
    ASSERT( m_Initialized );

    //
    // There is one bit which indicates if there is any data at all.
    //
    if( !BS.ReadFlag() )
        return;

    s32 i;
    u32 DBits = 0;
    u32 SBits = 0;
    u32 PBits = 0;

    //
    // Try to read the DIRTY_INIT data.
    //
    if( BS.ReadFlag() )
    {
        DBits |= DIRTY_INIT;

    //  HACK FOR CIRCUITS - Do not set the game type to GAME_MP.
    //  m_GameType = GAME_MP;   // Essentially "not GAME_CAMPAIGN" for client.
    //  HACK FOR CIRCUITS
        pGameLogic = &s_LogicBase;

    //  BS.ReadString  ( Mission Name );   // TO DO

        BS.ReadFlag( m_Score.IsTeamBased );
        BS.ReadRangedF32( m_TeamDamageFactor, 10, 0.0f, 1.0f );

    //  BS.ReadRangedS32( m_Score.ScoreFieldMask, 0, SCORE_ALL );
        // Want to send the ScoreFieldMask here, but if the client misses this 
        // packet, he won't be able to read any score updates.  Current solution
        // is to resent this value in front of every score update.  Proper 
        // solution would require keeping the bit dirty until the client ACKs 
        // it, but this would require some special coding.
    }

    //
    // Try to read the DIRTY_CLOCK data.
    //
    if( BS.ReadFlag() )
    {
        DBits |= DIRTY_CLOCK;
        BS.ReadRangedF32( m_Clock, 16, 0.0f, 4095.0f );  // over an hour 
    }

    //
    // Try to read the DIRTY_CLOCK_CONFIG data.
    //
    if( BS.ReadFlag() )
    {
        DBits |= DIRTY_CLOCK_CONFIG;

        BS.ReadRangedS32( m_ClockMode, -1,    1 );
        BS.ReadRangedS32( m_ClockLimit, 0, 4095 );
    }

    //
    // Try to read DIRTY_VOTE_MODE data.
    //
    if( BS.ReadFlag() )
    {
        DBits |= DIRTY_VOTE_MODE;
        if( BS.ReadFlag( m_VoteInProgress ) )
        {
            BS.ReadWString  ( s_VoteMsg[0], NET_NAME_LENGTH );
            BS.ReadWString  ( s_VoteMsg[1], NET_NAME_LENGTH );
            BS.ReadRangedS32( m_VotePercent, 0, 100 );
            Localize( m_VoteMsg[0], s_VoteMsg[0] );
            Localize( m_VoteMsg[1], s_VoteMsg[1] );
            LOG_MESSAGE( "game_mgr::AcceptUpdate",
                         "Vote initiated: %s / %s with %d%%",
                         (const char*)xstring(m_VoteMsg[0]),
                         (const char*)xstring(m_VoteMsg[1]),
                         m_VotePercent );
        }
        else
        {
            LOG_MESSAGE( "game_mgr::AcceptUpdate", "Voting deactivated." );
        }
    }

    //
    // Try to read DIRTY_VOTE_UPDATE data.
    //
    if( BS.ReadFlag() )
    {
        DBits |= DIRTY_VOTE_UPDATE;
        if( BS.ReadFlag( m_VoteInProgress ) )
        {
            BS.ReadRangedS32( m_VotesYes    , 0, 32 );
            BS.ReadRangedS32( m_VotesNo     , 0, 32 );
            BS.ReadRangedS32( m_VotesMissing, 0, 32 );
            LOG_MESSAGE( "game_mgr::AcceptUpdate", 
                         "Vote status - Yes:%d - No:%d - Missing:%d",
                         m_VotesYes, m_VotesNo, m_VotesMissing );
        }
    }

    //
    // Try to read DIRTY_CIRCUIT_BITS data.
    //
    if( BS.ReadFlag() )
    {
        DBits |= DIRTY_CIRCUIT_BITS;
        for( s32 c = 0; c < MAX_CIRCUITS; c++ )
        {
            s32 Value;
            BS.ReadRangedS32( Value, 0, 3 );            
            if( Value == 3 )    m_CircuitBits[c] = 0xFFFFFFFF;
            else                m_CircuitBits[c] = Value;
        }
    }

    //
    // Try to read DIRTY_TEAM_NAMES data.
    //
    if( BS.ReadFlag() )
    {
        BS.ReadWString( s_TeamName[0], NET_NAME_LENGTH );
        BS.ReadWString( s_TeamName[1], NET_NAME_LENGTH );

        Localize( m_Score.Team[0].Name, s_TeamName[0] );
        Localize( m_Score.Team[1].Name, s_TeamName[1] );

        x_mstrcpy( m_Score.Team[0].NName, m_Score.Team[0].Name );
        x_mstrcpy( m_Score.Team[1].NName, m_Score.Team[1].Name );
    }

    //
    // Try to read DIRTY_TEAM_SCORES data.
    //
    if( BS.ReadFlag() )
    {
        for( i = 0; i < 2; i++ )
        {
            s32 Score;

            BS.ReadRangedS32( m_Score.Team[i].Size, 0, 32 );

            if     ( BS.ReadFlag() )  BS.ReadRangedS32( Score, -255,   255 );
            else if( BS.ReadFlag() )  BS.ReadRangedS32( Score,  256,  5000 );
            else                      BS.ReadS32      ( Score );    

            m_Score.Team[i].Score = Score;
        }
    }

    //
    // Try to read DIRTY_HIGHLIGHT data.
    //
    if( BS.ReadFlag() )
    {
        BS.ReadU32( m_Score.HighLightMask );
    }

    //
    // Try to read data for each player which was marked as dirty.
    //

    // If there is ANY player data...
    if( BS.ReadFlag() )
    {
        // Iterate through all player slots.
        for( i = 0; i < 32; i++ )
        {
            player_score& Player = m_Score.Player[i];

            xbool OldConnected = Player.IsConnected;
            xbool OldInGame    = Player.IsInGame;

            // If player 'i' had any data...
            if( BS.ReadFlag() )
            {
                PBits |= (1 << i);  // Used only for logging.

                // If the player is connected...
                if( (Player.IsConnected = BS.ReadFlag()) )
                {
                    // Accept his name.
                    BS.ReadWString( Player.Name );
                    x_mstrcpy( Player.NName, Player.Name );

                    // Accept his unique identifier.
                    BS.ReadU64( Player.Identifier );

                    // If player is actively in the game...
                    if( (Player.IsInGame = BS.ReadFlag()) )
                    {
                        BS.ReadRangedS32( m_Score.Player[i].Team, 0, 31 );
                        // Bot?
                        // Other player characteristics?
                        // Only non-local players get added to the recent players list.
                        if( !g_NetworkMgr.IsLocal(i) )
                        {
                            g_MatchMgr.AddRecentPlayer( m_Score.Player[i] );
                        }
                    }
                    
                    // Muting.
                    if( (OldConnected == FALSE) && (Player.IsConnected == TRUE) )
                    {
                        // On XBox this will lookup the mutelist from the XBox Live service
						#if defined ( TARGET_PC )
                        {
                        }
                        #endif
                        #if defined ( TARGET_XBOX )
                        xbool IsMuted = g_MatchMgr.IsPlayerMuted( Player.Identifier );

                        g_VoiceMgr.SetLocallyMuted( i, IsMuted );
                        #endif
                        /*
                        if( IsMuted == TRUE )
                            LOG_MESSAGE( "game_mgr::AcceptUpdate", "Locally muting player %d", i );
                        else
                            LOG_MESSAGE( "game_mgr::AcceptUpdate", "Locally un-muting player %d", i );
                        */
                    }

                    /*
                    LOG_MESSAGE( "game_mgr::AcceptUpdate",
                                 "Player:%d - InGame:%d - Team:%d",
                                 i, Player.IsInGame, Player.Team );
                    */
                }
                else
                {
                    Player.IsInGame = FALSE;
                }
            }

            // Locally generated messages...
            if( m_GameInProgress && (i != g_NetworkMgr.GetLocalPlayerSlot(0)) )
            {   
                if( Player.IsConnected && !OldConnected )
                {
                    ASSERT( Player.Name );
                    MsgConnect( i );
                }
                if( !Player.IsConnected && OldConnected )
                {
                    ASSERT( Player.Name );
                    MsgDisconnect( i );                    
                }
                if( Player.IsInGame && !OldInGame )
                {
                    ASSERT( Player.Name );
                    MsgEnterGame( i );
                }
            }
        }

        m_Score.NConnected = 0;
        m_Score.NPlayers   = 0;
        for( s32 i = 0; i < 32; i++ )
        {
            if( m_Score.Player[i].IsConnected )     m_Score.NConnected++;
            if( m_Score.Player[i].IsInGame    )     m_Score.NPlayers++;
        }
    }

    BS.ReadMarker();

    //
    // Try to read scores for each player which had dirty scores.
    //
    if( BS.ReadFlag() )
    {
        BS.ReadRangedU32( m_Score.ScoreFieldMask, 0, SCORE_ALL );

        for( i = 0; i < 32; i++ )
        {
            player_score& P = m_Score.Player[i];

            if( BS.ReadFlag() )
            {
                SBits |= (1 << i);

                if( m_Score.ScoreFieldMask & SCORE_POINTS )
                {
                if     ( BS.ReadFlag() )  BS.ReadRangedS32( P.Score , -255,  255 );
                else if( BS.ReadFlag() )  BS.ReadRangedS32( P.Score ,  256, 5000 );
                else                      BS.ReadS32      ( P.Score );
                }

                if( m_Score.ScoreFieldMask & SCORE_KILLS )
                {
                if     ( BS.ReadFlag() )  BS.ReadRangedS32( P.Kills ,    0,   63 );
                else if( BS.ReadFlag() )  BS.ReadRangedS32( P.Kills ,   64, 1000 );
                else                      BS.ReadS32      ( P.Kills );
                }

                if( m_Score.ScoreFieldMask & SCORE_DEATHS )
                {
                if     ( BS.ReadFlag() )  BS.ReadRangedS32( P.Deaths,    0,   63 );
                else if( BS.ReadFlag() )  BS.ReadRangedS32( P.Deaths,   64, 1000 );
                else                      BS.ReadS32      ( P.Deaths );
                }

                if( m_Score.ScoreFieldMask & SCORE_TKS )
                {
                if     ( BS.ReadFlag() )  BS.ReadRangedS32( P.TKs   ,    0,   15 );
                else if( BS.ReadFlag() )  BS.ReadRangedS32( P.TKs   ,   15, 1000 );
                else                      BS.ReadS32      ( P.TKs   );
                }

                if( m_Score.ScoreFieldMask & SCORE_FLAGS )
                {
                if     ( BS.ReadFlag() )  BS.ReadRangedS32( P.Flags ,    0,   15 );
                else if( BS.ReadFlag() )  BS.ReadRangedS32( P.Flags ,   15, 1000 );
                else                      BS.ReadS32      ( P.Flags );
                }

                if( m_Score.ScoreFieldMask & SCORE_VOTES )
                {
                if     ( BS.ReadFlag() )  BS.ReadRangedS32( P.Votes ,    0,   15 );
                else if( BS.ReadFlag() )  BS.ReadRangedS32( P.Votes ,   15, 1000 );
                else                      BS.ReadS32      ( P.Votes );
                }

                // Music.
                {
                    BS.ReadRangedS32( P.MusicLevel, 0, 15 );
                }

                // Player stats.
                if( BS.ReadFlag() )
                {
                    BS.ReadFlag( P.Games  );   
                    BS.ReadFlag( P.Wins   );    
                    BS.ReadFlag( P.Gold   );
                    BS.ReadFlag( P.Silver );
                    BS.ReadFlag( P.Bronze );
                }

                // Voice peripheral status.
                {
                    BS.ReadFlag( m_Score.Player[i].IsVoiceAllowed );
                    BS.ReadFlag( m_Score.Player[i].IsVoiceCapable );
                }
            }
        }

        m_Score.ComputeRanks();
    }

    BS.ReadMarker();

    //
    // Try to read dirty zone state information.
    //
    if( BS.ReadFlag() )
    {
        for( s32 z = 0; z < 255; z++ )
        {
            xbool Flag;
            BS.ReadFlag( Flag );    m_Zone[z].Warned = Flag;
            BS.ReadFlag( Flag );    m_Zone[z].Locked = Flag;
        }
    }

    BS.ReadMarker();

    //
    // Try to read the score display mode.
    //
    if( BS.ReadFlag() )
    {
        s32 Mode;
        BS.ReadRangedS32( Mode, 0, SCORE_DISPLAY_MAX-1 );
        m_ScoreDisplay = (score_display)Mode;
    }

    BS.ReadMarker();

    /*
    LOG_MESSAGE( "game_mgr::AcceptUpdate",
                 "DirtyBits:%08X - ScoreBits:%08X - PlayerBits:%08X",
                 DBits, SBits, PBits );
    */
}

//==============================================================================

void game_mgr::ProvideUpdate( bitstream& BS, 
                              s32        ClientIndex,
                              u32&       DirtyBits, 
                              u32&       ScoreBits, 
                              u32&       PlayerBits )
{
    s32 i;

    ASSERT( g_NetworkMgr.IsServer() );
    ASSERT( m_Initialized );
    ASSERT( m_GameInProgress );

    u32 DBits = DirtyBits;
    u32 SBits = ScoreBits;
    u32 PBits = PlayerBits;

    // Send a single bit indicating the presence of ANY data.
    if( !BS.WriteFlag( DirtyBits | ScoreBits | PlayerBits ) )
        return;

    const game_score& Score = GetScore();

    // See if all bits are set.  This indicates the "first time" update 
    // situation, and all possible data should be sent.
    if( (DirtyBits & ScoreBits & PlayerBits) == 0xFFFFFFFF )
    {
        DirtyBits  = DIRTY_ALL;
        ScoreBits  = 0x00000000;
        PlayerBits = 0x00000000;

        for( i = 0; i < 32; i++ )
        {
            if( Score.Player[i].IsConnected )
            {
                ScoreBits  |= (1 << i); 
                PlayerBits |= (1 << i);
            }
        }

        DBits = DirtyBits; 
        SBits = ScoreBits; 
        PBits = PlayerBits; 
    }   

    //
    // Try to pack up the DIRTY_INIT data.
    //

    if( BS.OpenSection( DirtyBits & DIRTY_INIT ) )
    {
    //  BS.WriteString  ( Mission Name );   // TO DO

        BS.WriteFlag( Score.IsTeamBased );
        BS.WriteRangedF32( m_TeamDamageFactor, 10, 0.0f, 1.0f );

    //  BS.WriteRangedS32( Score.ScoreFieldMask, 0, SCORE_ALL );
        // Want to send the ScoreFieldMask here, but if the client misses this 
        // packet, he won't be able to read any score updates.  Current solution
        // is to resend this value in front of every score update.  Proper 
        // solution would require keeping the bit dirty until the client ACKs 
        // it, but this would require some special coding.

        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_INIT;
    }

    //
    // Try to pack up the DIRTY_CLOCK data.
    //
    if( BS.OpenSection( (DirtyBits & DIRTY_CLOCK) ) )
    {
        BS.WriteRangedF32( m_Clock, 16, 0.0f, 4095.0f );
        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_CLOCK;
    }

    //
    // Try to pack up the DIRTY_CLOCK_CONFIG data.
    //
    if( BS.OpenSection( (DirtyBits & DIRTY_CLOCK_CONFIG) ) )
    {
        BS.WriteRangedS32( m_ClockMode, -1,    1 );
        BS.WriteRangedS32( m_ClockLimit, 0, 4095 );
        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_CLOCK_CONFIG;
    }

    //
    // Try to pack up DIRTY_VOTE_MODE data.
    //
    if( BS.OpenSection( (DirtyBits & DIRTY_VOTE_MODE) ) )
    {
        if( BS.WriteFlag( m_VoteInProgress ) )
        {
            BS.WriteWString  ( s_VoteMsg[0] );
            BS.WriteWString  ( s_VoteMsg[1] );
            BS.WriteRangedS32( m_VotePercent, 0, 100 );
        }
        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_VOTE_MODE;
    }

    //
    // Try to pack up DIRTY_VOTE_UPDATE data.
    //
    if( BS.OpenSection( (DirtyBits & DIRTY_VOTE_UPDATE) ) )
    {
        if( BS.WriteFlag( m_VoteInProgress ) )
        {
            BS.WriteRangedS32( m_VotesYes    , 0, 32 );
            BS.WriteRangedS32( m_VotesNo     , 0, 32 );
            BS.WriteRangedS32( m_VotesMissing, 0, 32 );
        }
        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_VOTE_UPDATE;
    }

    //
    // Try to pack up DIRTY_CIRCUIT_BITS data.
    //
    if( BS.OpenSection( (DirtyBits & DIRTY_CIRCUIT_BITS) ) )
    {
        for( s32 c = 0; c < MAX_CIRCUITS; c++ )
        {
            s32 Value;
            if( m_CircuitBits[c] == 0xFFFFFFFF)     Value = 3;
            else                                    Value = m_CircuitBits[c];
            BS.WriteRangedS32( Value, 0, 3 );
        }
        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_CIRCUIT_BITS;
    }  

    //
    // Try to pack up the DIRTY_TEAM_NAMES data.
    //
    if( BS.OpenSection( DirtyBits & DIRTY_TEAM_NAMES ) )
    {
    //  ASSERT( Score.IsTeamBased );

        BS.WriteWString( s_TeamName[0] );
        BS.WriteWString( s_TeamName[1] );

        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_TEAM_NAMES;
    }

    //
    // Try to pack up the DIRTY_TEAM_SCORES data.
    //
    if( BS.OpenSection( DirtyBits & DIRTY_TEAM_SCORES ) )
    {          
        for( s32 i = 0; i < 2; i++ )
        {
            BS.WriteRangedS32( Score.Team[i].Size, 0, 32 );

            s32 Value = Score.Team[i].Score;
            if     ( BS.WriteFlag( IN_RANGE( -255, Value,  255 ) ) )  BS.WriteRangedS32( Value, -255,  255 );
            else if( BS.WriteFlag( IN_RANGE(  256, Value, 5000 ) ) )  BS.WriteRangedS32( Value,  256, 5000 );   
            else                                                      BS.WriteS32      ( Value );
        }

        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_TEAM_SCORES;
    }

    //
    // Try to pack up the DIRTY_HIGHLIGHT data.
    //
    if( BS.OpenSection( DirtyBits & DIRTY_HIGHLIGHT ) )
    {
        BS.WriteU32( Score.HighLightMask );

        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_HIGHLIGHT;
    }

    //
    // Try to pack up data for each player which is marked as dirty.
    //

    // If there are ANY player bits...
    if( BS.WriteFlag( PlayerBits ) )
    {
        s32 Limit = 4;

        // Iterate through all player slots.
        for( i = 0; i < 32; i++ )
        {
            // If bits is set for player 'i' and the section opens...
            if( BS.OpenSection( (PlayerBits & (1 << i)) && (Limit > 0) ) )
            {
                // If player is connected...
                if( BS.WriteFlag( Score.Player[i].IsConnected ) )
                {
                    Limit--;

                    // Send over his name.
                    BS.WriteWString( Score.Player[i].Name );

                    // Send unique identifier.
                    BS.WriteU64( Score.Player[i].Identifier );

                    // Send 'machine'?
                    // Player attributes like 'skin' go with net object.

                    // If player is actively in the game...
                    if( BS.WriteFlag( Score.Player[i].IsInGame ) )
                    {
                        BS.WriteRangedS32( Score.Player[i].Team, 0, 31 );
                    //  BS.WriteFlag( Score.Player[i].IsBot );
                    //  BS.WriteFlag( Score.Player[i].IsMale );
                    }

                    /*
                    LOG_MESSAGE( "game_mgr::ProvideUpdate",
                                 "Player:%d - Team:%d",
                                 i, Score.Player[i].Team );
                    */
                }
                // If the section closes, then clear the bit.
                if( BS.CloseSection() )  PlayerBits &= ~(1 << i);
            }
        }
    }

    BS.WriteMarker();
  
    //
    // Try to pack up scores for each player with dirty scores.
    //
    if( BS.WriteFlag( ScoreBits ) )
    {
        BS.WriteRangedS32( Score.ScoreFieldMask, 0, SCORE_ALL );

        s32 Limit = 4;

        for( i = 0; i < 32; i++ )
        {
            if(     (ScoreBits & (1 << i))
                && !(    (Score.Player[i].IsConnected)
                      && (Score.Player[i].IsInGame)) )
            {
                LOG_WARNING( "game_mgr::ProvideUpdate",
                             "Unexpected score dirty bit for player %d.",
                             i );
                BS.WriteFlag( FALSE );
                ScoreBits &= ~(1 << i);
                continue;
            }

            const player_score& P = Score.Player[i];

            if( BS.OpenSection( (ScoreBits & (1 << i)) && (Limit > 0) ) )
            {
                Limit--;

                if( Score.ScoreFieldMask & SCORE_POINTS )
                {
                if     ( BS.WriteFlag( IN_RANGE( -255, P.Score ,  255 ) ) )  BS.WriteRangedS32( P.Score , -255,  255 );
                else if( BS.WriteFlag( IN_RANGE(  256, P.Score , 5000 ) ) )  BS.WriteRangedS32( P.Score ,  256, 5000 );
                else                                                         BS.WriteS32      ( P.Score );
                }

                if( Score.ScoreFieldMask & SCORE_KILLS )
                {
                if     ( BS.WriteFlag( IN_RANGE(    0, P.Kills ,   63 ) ) )  BS.WriteRangedS32( P.Kills ,    0,   63 );
                else if( BS.WriteFlag( IN_RANGE(   64, P.Kills , 1000 ) ) )  BS.WriteRangedS32( P.Kills ,   64, 1000 );
                else                                                         BS.WriteS32      ( P.Kills );
                }

                if( Score.ScoreFieldMask & SCORE_DEATHS )
                {
                if     ( BS.WriteFlag( IN_RANGE(    0, P.Deaths,   63 ) ) )  BS.WriteRangedS32( P.Deaths,    0,   63 );
                else if( BS.WriteFlag( IN_RANGE(   64, P.Deaths, 1000 ) ) )  BS.WriteRangedS32( P.Deaths,   64, 1000 );
                else                                                         BS.WriteS32      ( P.Deaths );
                }

                if( Score.ScoreFieldMask & SCORE_TKS )
                {
                if     ( BS.WriteFlag( IN_RANGE(    0, P.TKs   ,   15 ) ) )  BS.WriteRangedS32( P.TKs   ,    0,   15 );
                else if( BS.WriteFlag( IN_RANGE(   15, P.TKs   , 1000 ) ) )  BS.WriteRangedS32( P.TKs   ,   15, 1000 );
                else                                                         BS.WriteS32      ( P.TKs   );
                }

                if( Score.ScoreFieldMask & SCORE_FLAGS )
                {
                if     ( BS.WriteFlag( IN_RANGE(    0, P.Flags ,   15 ) ) )  BS.WriteRangedS32( P.Flags ,    0,   15 );
                else if( BS.WriteFlag( IN_RANGE(   15, P.Flags , 1000 ) ) )  BS.WriteRangedS32( P.Flags ,   15, 1000 );
                else                                                         BS.WriteS32      ( P.Flags );
                }

                if( Score.ScoreFieldMask & SCORE_VOTES )
                {
                if     ( BS.WriteFlag( IN_RANGE(    0, P.Votes ,   15 ) ) )  BS.WriteRangedS32( P.Votes ,    0,   15 );
                else if( BS.WriteFlag( IN_RANGE(   15, P.Votes , 1000 ) ) )  BS.WriteRangedS32( P.Votes ,   15, 1000 );
                else                                                         BS.WriteS32      ( P.Votes );
                }

                // Music.
                {
                    BS.WriteRangedS32( P.MusicLevel, 0, 15 );
                }

                // Stats data.
                if( BS.WriteFlag( ClientIndex == P.ClientIndex ) )
                {
                    BS.WriteFlag( P.Games  );   
                    BS.WriteFlag( P.Wins   );    
                    BS.WriteFlag( P.Gold   );
                    BS.WriteFlag( P.Silver );
                    BS.WriteFlag( P.Bronze );
                }

                // Voice peripheral status.
                {
                    BS.WriteFlag( P.IsVoiceAllowed );
                    BS.WriteFlag( P.IsVoiceCapable );
                }

                if( BS.CloseSection() )  ScoreBits &= ~(1 << i);
            }
        }
    }

    BS.WriteMarker();

    //
    // Try to pack up zone state information.
    //
    if( BS.OpenSection( DirtyBits & DIRTY_ZONE_STATE ) )
    {
        for( s32 z = 0; z < 255; z++ )
        {
            BS.WriteFlag( m_Zone[z].Warned );
            BS.WriteFlag( m_Zone[z].Locked );
        }

        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_ZONE_STATE;
    }

    BS.WriteMarker();
    
    //
    // Try to pack up the score display mode.
    //
    if( BS.OpenSection( DirtyBits & DIRTY_SCORE_DISPLAY ) )
    {
        BS.WriteRangedS32( m_ScoreDisplay, 0, SCORE_DISPLAY_MAX-1 );

        if( BS.CloseSection() )  DirtyBits &= ~DIRTY_SCORE_DISPLAY;
    }

    BS.WriteMarker();

    /*
    if( (DBits ^ DirtyBits) | (SBits ^ ScoreBits) | (PBits ^ PlayerBits) )
    {   
        LOG_MESSAGE( "game_mgr::ProvideUpdate",
                     "DirtyBits:%08X - ScoreBits:%08X - PlayerBits:%08X",
                     DBits ^ DirtyBits, 
                     SBits ^ ScoreBits, 
                     PBits ^ PlayerBits );
    }
    */
}

//==============================================================================

void game_mgr::ChangeTeam( s32 PlayerIndex, xbool ByAdmin /*= TRUE */ )
{
    s32 OldTeam;
    s32 NewTeam;

    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    // Only allowed on the server.
    if( g_NetworkMgr.IsClient() )
        return;

    // Only allowed in team based games.
    if( !m_Score.IsTeamBased )
        return;

    // Only allowed on connected players.
    if( !m_Score.Player[PlayerIndex].IsConnected )
        return;

    // Only allowed on players which are participating.
    if( !m_Score.Player[PlayerIndex].IsInGame )
        return;

    // Gather some basic information.
    OldTeam = m_Score.Player[PlayerIndex].Team;
    NewTeam = 1 - OldTeam;

    // Let's do it.
    m_Score.Player[PlayerIndex].Team = NewTeam;
    m_Score.Team[OldTeam].Size--;
    m_Score.Team[NewTeam].Size++;
    actor* pPlayer = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
    ASSERT( pPlayer );
    pPlayer->net_SetTeamBits( 1 << NewTeam );

    // Set dirty bits.
    m_DirtyBits  |= DIRTY_TEAM_SCORES;     // TO DO - Validate this.
    m_PlayerBits |= (1 << PlayerIndex);
    m_UpdateVoteRights = TRUE;

    AdminKill( PlayerIndex );
    if( ByAdmin )
    {
        pGameLogic->RequestSpawn( PlayerIndex, TRUE );
    }

    m_Score.Player[PlayerIndex].TeamTime = 0.0f;

    // Send message.
    if( ByAdmin )
        MsgMgr.Message( MSG_SERVER_TEAMCHANGED_PLAYER, 0, PlayerIndex, NewTeam );
    else
        MsgMgr.Message( MSG_PLAYER_SWITCHED_TEAM, 0, PlayerIndex, NewTeam );

    // Logging.
    LOG_MESSAGE( "game_mgr::ChangeTeam", 
                 "Player:%d - From:%d - To:%d - ByAdmin:%d",
                 PlayerIndex, OldTeam, NewTeam, ByAdmin );
}

//==============================================================================

void game_mgr::AdminKill( s32 PlayerIndex )
{
    // The game needs to make the indicated player dead for some reason.  (Maybe
    // the player is changing teams.)  This death needs to be penalty free.
    // There should not be a message, either.

    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    object* pObject = (object*)NetObjMgr.GetObjFromSlot( PlayerIndex );
    if( !pObject )
        return;

    if( (pObject->GetType() != object::TYPE_PLAYER) &&
        (pObject->GetType() != object::TYPE_NET_GHOST)  )
        return;

    // Lookup player and clear spawn neutral time to make sure pain gets applied
    // (fixes bug where host rapidly switches a players team from the team change menu)
    actor* pPlayer = (actor*)pObject;
    pPlayer->SetSpawnNeutralTime( 0.0f );

    pain Pain;
    Pain.Setup( "SERVER_MAINTENANCE", 0, pPlayer->GetPosition() );
    Pain.SetDirectHitGuid( pPlayer->GetGuid() );
    Pain.ApplyToObject( pPlayer );    
}

//==============================================================================

void game_mgr::PlayerSuicide( void )
{
    // The local player has decided to commit suicide.
    // This function is valid on both the server and clients.

    s32 PlayerIndex = g_NetworkMgr.GetLocalPlayerSlot( 0 );
    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    object* pObject = (object*)NetObjMgr.GetObjFromSlot( PlayerIndex );
    if( !pObject )
        return;

    if( pObject->GetType() != object::TYPE_PLAYER )
        return;

    player* pPlayer = (player*)pObject;

    pain Pain;
    Pain.Setup( "MANUAL_SUICIDE", 0, pPlayer->GetPosition() );
    Pain.SetDirectHitGuid( pPlayer->GetGuid() );
    Pain.ApplyToObject( pPlayer );    
}

//==============================================================================

void game_mgr::KickPlayer( s32 PlayerIndex, xbool ByAdmin )
{
    (void)ByAdmin;

    LOG_MESSAGE( "game_mgr::KickPlayer", 
                 "Slot:%d - ByAdmin:%s", 
                 PlayerIndex, ByAdmin ? "TRUE" : "FALSE" );

    if( g_NetworkMgr.IsClient() )
        return;

    if( !m_GameInProgress )
        return;

    g_NetworkMgr.KickPlayer( PlayerIndex );

/*
    // Message: Player kicked by admin.
    if( ByAdmin )
        MsgMgr.Message( MSG_ADMIN_PLAYER_KICKED_BY_ADMIN, 
                        0, ARG_INLINE, PlayerIndex );
*/
}

//==============================================================================

u32 game_mgr::GetCircuitBits( s32 Circuit )
{
    ASSERT( IN_RANGE( 0, Circuit, MAX_CIRCUITS ) );
    return( m_CircuitBits[ Circuit ] );
}

//==============================================================================

void game_mgr::SetCircuitBits( s32 Circuit, u32 Bits )
{
    ASSERT( IN_RANGE( 0, Circuit, MAX_CIRCUITS ) );
    m_CircuitBits[ Circuit ] = Bits;
    m_DirtyBits |= DIRTY_CIRCUIT_BITS;
}

//==============================================================================

void game_mgr::StartVoteKick( s32 PlayerIndex, s32 StarterIndex )
{
    // NOTES FROM PREVIOUS WORK
    //  - Can't kick player if there is a password.
    //  - Can only kick inactive players in non-team based games.
    //  - Can't kick Inevitable people.

    ASSERT( g_NetworkMgr.IsServer() );

    // Cannot kick players on the server.
    if( g_NetworkMgr.IsLocal( PlayerIndex ) )
        return;

    // Get a "time remaining" value.
    f32 TimeRemaining = 100.0f;     // Default value which is "safe".
    if( m_ClockMode == -1 ) TimeRemaining = m_Clock;
    if( m_ClockMode ==  1 ) TimeRemaining = m_ClockLimit - m_Clock;
                                    
    // Consider common reasons to disallow the vote.
    if( (!m_GameInProgress) ||                     // No game in progress
        (m_VoteInProgress) ||                      // Vote in progress
        (TimeRemaining < 60.0f) ||                 // Less than 1 minute to go
        (!m_Score.Player[PlayerIndex].IsInGame) || // Player isn't here
        (m_Score.Player[PlayerIndex].IsBot) ||     // Player is a bot
        (m_GameType == GAME_CAMPAIGN) )            // Campaign game
    {
        return;
    }

    // This vote seems to be viable.  Initialize a few variables.
    m_VotesYes     = 0;
    m_VotesNo      = 0;
    m_VotesMissing = 0;
    m_VoteCanCast  = 0x00000000;

    // Decide who can vote and who can't.
    for( s32 i = 0; i < 32; i++ )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
        if( (pActor) &&
            ( m_Score.Player[i].IsInGame) && 
            (!m_Score.Player[i].IsBot) &&
            (i != PlayerIndex) &&
            (i != StarterIndex) )
        {
            if( (!m_Score.IsTeamBased) || 
                (m_Score.Player[i].Team == m_Score.Player[PlayerIndex].Team) )
            {
                m_VotesMissing++;
                m_VoteCanCast |= (1 << i);
            }
        }
    }

    // No participants?  No vote!
    if( m_VotesMissing == 0 )
        return;

    // Everything is valid.  Trigger updates to voting rights.
    m_UpdateVoteRights = TRUE;

    // Set up all remaining variables.

    // Who can SEE the vote progress?
    m_VoteCanSee  = m_VoteCanCast;          // Everybody who can cast a vote.
    m_VoteCanSee |= (1 << PlayerIndex);     // Plus the vote target.
    if( StarterIndex != -1 )
        m_VoteCanSee |= (1 << StarterIndex);     // Plus the vote starter.

    m_VoteInProgress = TRUE;
    m_VoteHasPassed  = FALSE;
    m_VoteTimer      = 35.0f;           // Desired time (30) + extra 5.
    m_VoteKick       = PlayerIndex;
    m_DirtyBits     |= DIRTY_VOTE_MODE;
    m_DirtyBits     |= DIRTY_VOTE_UPDATE;

    m_Score.Player[StarterIndex].Votes += 1;
    m_ScoreBits |= (1 << StarterIndex);

    // Build the vote message.
    BuildVoteMessage();

    // Message all involved players that a vote has started.
    for( s32 i = 0; i < 32; i++ )
    {
        if( (m_Score.Player[i].IsInGame) && (m_VoteCanSee & (1<<i)) )
        {
            MsgMgr.Message( MSG_P_CALLED_VOTE, i, StarterIndex );
        }
    }
    
    // Message the player that is being voted against.
    MsgMgr.Message( MSG_WARNING_VOTE_TO_KICK, PlayerIndex );

    LOG_MESSAGE( "game_mgr::StartVoteKick",
                 "Kick:%d - Starter:%d - CanSee:%08X - CanCast:%08X",
                 PlayerIndex, StarterIndex, m_VoteCanSee, m_VoteCanCast );
}

//==============================================================================

void game_mgr::StartVoteMap( s32 MapIndex, s32 StarterIndex )
{
    ASSERT( IN_RANGE( 0, StarterIndex, 31 ) );

    // Get a "time remaining" value.
    f32 TimeRemaining = 100.0f;     // Default value which is "safe".
    if( m_ClockMode == -1 ) TimeRemaining = m_Clock;
    if( m_ClockMode ==  1 ) TimeRemaining = m_ClockLimit - m_Clock;
                                    
    // Consider common reasons to disallow the vote.
    if( (!m_GameInProgress) ||                     // No game in progress
        (m_VoteInProgress) ||                      // Vote in progress
        (TimeRemaining < 60.0f) ||                 // Less than 1 minute to go
        (m_GameType == GAME_CAMPAIGN) )            // Campaign game
    {
        return;
    }

    // This vote seems to be viable.  Initialize a few variables.
    m_VotesYes     = 0;
    m_VotesNo      = 0;
    m_VotesMissing = 0;
    m_VoteCanCast  = 0x00000000;

    // Decide who can vote and who can't.
    for( s32 i = 0; i < 32; i++ )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
        if( (pActor) &&
            ( m_Score.Player[i].IsInGame) && 
            (!m_Score.Player[i].IsBot) &&
            (i != StarterIndex) )
        {
            m_VotesMissing++;
            m_VoteCanCast |= (1 << i);
        }
    }

    // No participants?  No vote!
    if( m_VotesMissing == 0 )
        return;

    // Everything is valid.  Trigger update to voting rights.
    m_UpdateVoteRights = TRUE;

    // Set up all remaining variables.

    // Who can SEE the vote progress?
    m_VoteCanSee  = m_VoteCanCast;          // Everybody who can cast a vote.
    if( StarterIndex != -1 )
        m_VoteCanSee |= (1 << StarterIndex);     // Plus the vote starter.
    
    // Message all involved players that a vote has started.
    for( s32 i = 0; i < 32; i++ )
    {
        if( (m_Score.Player[i].IsInGame) && (m_VoteCanSee & (1<<i)) )
        {
            MsgMgr.Message( MSG_P_CALLED_VOTE, i, StarterIndex );
        }
    }

    m_VoteInProgress = TRUE;
    m_VoteHasPassed  = FALSE;
    m_VoteTimer      = 35.0f;           // Desired time (30) + extra 5.
    m_VoteMap        = MapIndex;
    m_DirtyBits     |= DIRTY_VOTE_MODE;
    m_DirtyBits     |= DIRTY_VOTE_UPDATE;

    m_Score.Player[StarterIndex].Votes += 1;
    m_ScoreBits |= (1 << StarterIndex);

    // Build the vote message.
    BuildVoteMessage();

    LOG_MESSAGE( "game_mgr::StartVoteMap",
                 "Map:%d - Starter:%d - CanSee:%08X - CanCast:%08X",
                 MapIndex, StarterIndex, m_VoteCanSee, m_VoteCanCast );
}

//==============================================================================

void game_mgr::BuildVoteMessage( void )
{
    m_VoteMsg[0][0] = '\0';
    m_VoteMsg[1][0] = '\0';

    if( m_VoteInProgress )
    {
        if( m_VoteKick != -1 )
        {
            s_VoteMsg[0][0] = '\\'; 
            s_VoteMsg[0][1] = 'V'; 
            s_VoteMsg[0][2] = 'K'; 
            s_VoteMsg[0][3] = 0;

            x_wstrcpy( s_VoteMsg[1], m_Score.Player[m_VoteKick].Name );

            Localize( m_VoteMsg[0], s_VoteMsg[0] );
            Localize( m_VoteMsg[1], s_VoteMsg[1] );
        }

        if( m_VoteMap != -1 )
        {
            s32 MapID    = (m_VoteMap & 0x0000FFFF);
            s32 GameType = (m_VoteMap & 0xFFFF0000) >> 16;

            s_VoteMsg[0][0] = '\\'; 
            s_VoteMsg[0][1] = 'V'; 
            s_VoteMsg[0][2] = 'M'; 
            s_VoteMsg[0][3] = 0;

            if( GameType == m_GameType )
            {
                x_mstrcpy( s_VoteMsg[1], g_MapList.GetDisplayName( MapID ) );
            }
            else
            {
                const game_type_info* pGTI = g_MapList.GetGameTypeInfo( GameType );
                x_mstrcpy( s_VoteMsg[1], 
                           xfs( "%s - %s", (const char*)pGTI->ShortTypeName,
                                           g_MapList.GetDisplayName( MapID ) ) );
            }

            Localize( m_VoteMsg[0], s_VoteMsg[0] );
            Localize( m_VoteMsg[1], s_VoteMsg[1] );
        }

        LOG_MESSAGE( "game_mgr::BuildVoteMessage",
                     "%s / %s", 
                     (const char*)xstring(m_VoteMsg[0]),
                     (const char*)xstring(m_VoteMsg[1]) );
    }
}

//==============================================================================

void game_mgr::ExecuteVote( void )
{
    s32 Kick = m_VoteKick;  // Make backup copy.
    s32 Map  = m_VoteMap;   // Make backup copy.

    // Do the shut down BEFORE actually executing the vote.  Why?  Because
    // kicking the player will trigger the vote to terminate and this may have
    // undesired side effects.  And changing the map without first shutting
    // down the vote may also have side effects.  Of course, there may be no
    // side effects at all.  But no harm in playing it safe.

    ShutDownVote();         // This clears m_VoteKick and m_VoteMap.

    if( m_VoteHasPassed )
    {
        m_VoteHasPassed = FALSE;

        if( Kick != -1 )    
        {
            KickPlayer( Kick, FALSE );
            LOG_MESSAGE( "game_mgr::ExecuteVote", "Kick:%d(%s)", 
                         Kick, m_Score.Player[Kick].NName );
        }

        if( Map != -1 )
        {
            if( g_PendingConfig.GetGameTypeID() != (game_type)(Map >> 16) )
                g_PendingConfig.SetScoreLimit( -1 );

            g_PendingConfig.SetLevelID( (Map & 0x0000FFFF) );
            g_PendingConfig.SetGameTypeID( (game_type)(Map >> 16) );
            g_ActiveConfig.SetExitReason( GAME_EXIT_RELOAD_LEVEL );

            LOG_MESSAGE( "game_mgr::ExecuteVote", 
                         "Map:%d - GameType:%d", 
                         g_PendingConfig.GetLevelName(),
                         g_PendingConfig.GetGameTypeID() );
        }
    }
}

//==============================================================================

void game_mgr::VoteResult( xbool Passed )
{
    ASSERT( m_VoteInProgress );

    LOG_MESSAGE( "game_mgr::VoteResult", 
                 "%s - Yes:%d - No:%d - Missing:%d",
                 Passed ? "PASSED" : "FAILED", 
                 m_VotesYes, m_VotesNo, m_VotesMissing );

    for( s32 i = 0; i < 32; i++ )
    {
        if( (m_Score.Player[i].IsInGame) && (m_VoteCanSee & (1<<i)) )
        {
            if( Passed )
            {
                // If player[i] is being kicked, then "Judgement has passed",
                // otherwise "Vote passed."
                if( m_VoteKick == i )
                    MsgMgr.Message( MSG_JUDGEMENT_PASSED, i );
                else
                    MsgMgr.Message( MSG_VOTE_PASSED, i );
            }
            else
            {
                // MESSAGE - Vote failed to player[i].
                MsgMgr.Message( MSG_VOTE_FAILED, i );
            }
        }
    }

    m_VoteTimer        = 4.9f;          // Just under 5 seconds.
    m_VoteHasPassed    = Passed;        // Save the vote result.
    m_VoteCanCast      = 0x00000000;    // Nobody can cast votes.
    m_UpdateVoteRights = TRUE;          // 
}

//==============================================================================

void game_mgr::ShutDownVote( void )
{
    LOG_MESSAGE( "game_mgr::ShutDownVote", "" );

    if( m_VoteInProgress )
    {
        m_DirtyBits       |= DIRTY_VOTE_MODE;
        m_VoteKick         = -1;
        m_VoteMap          = -1;
        m_VoteInProgress   = FALSE;
        m_VoteCanSee       = 0x00000000;
        m_UpdateVoteRights = TRUE;
    }

    // This will build an empty vote message.
    // Technically not needed, but it is symmetric.
    BuildVoteMessage();
}

//==============================================================================

void game_mgr::CastVote( s32 PlayerIndex, s32 Vote )
{
    LOG_MESSAGE( "game_mgr::CastVote", 
                 "Player:%d(%s) - Vote:%d",
                 PlayerIndex, 
                 m_Score.Player[PlayerIndex].NName,
                 Vote );

    ASSERT( g_NetworkMgr.IsServer() );

    if( m_VoteInProgress && (m_VoteCanCast & (1<<PlayerIndex)) )
    {
        m_DirtyBits       |= DIRTY_VOTE_UPDATE;
        m_UpdateVoteRights = TRUE;

        if( Vote == +1 )  m_VotesYes++;
        if( Vote == -1 )  m_VotesNo ++;
        m_VotesMissing--;
        m_VoteCanCast &= ~(1<<PlayerIndex);

        s32 TotalVotes = m_VotesYes + m_VotesNo + m_VotesMissing;

    //  x_DebugMsg( "GAMEMGR: Yes=%d(%4.1f%%)  No=%d(%4.1f%%)  Missing=%d(%4.1f%%)\n", 
    //              m_VotesYes,     ((m_VotesYes     * 100) / (f32)TotalVotes),
    //              m_VotesNo,      ((m_VotesNo      * 100) / (f32)TotalVotes),
    //              m_VotesMissing, ((m_VotesMissing * 100) / (f32)TotalVotes) );

        if( (m_VoteTimer > 10.0f) && (TotalVotes > 0) )
        {
            // See if we have enough votes to pass.
            if( ((m_VotesYes * 100) / TotalVotes) >= m_VotePercent )
            {
                VoteResult( TRUE );
            }
            else
            // See if we have enough votes to fail.
            if( ((m_VotesNo * 100) / TotalVotes) > (100 - m_VotePercent) )
            {
                VoteResult( FALSE );
            }
        }
        else
        {
            if( m_VotesMissing == 0 )
            {
                VoteResult( FALSE );
            }
        }
    }
}

//==============================================================================

xbool game_mgr::GetVoteData( const xwchar*& pMessage1, 
                             const xwchar*& pMessage2, 
                                   s32&     Yes, 
                                   s32&     No, 
                                   s32&     Missing, 
                                   s32&     PercentNeeded )
{
    if( m_VoteInProgress )
    {
        pMessage1     = m_VoteMsg[0];
        pMessage2     = m_VoteMsg[1];
        Yes           = m_VotesYes;
        No            = m_VotesNo;
        Missing       = m_VotesMissing;
        PercentNeeded = m_VotePercent;
        return( TRUE );
    }
    else
    {
        return( FALSE );
    }
}

//==============================================================================

u32 game_mgr::GetVoteKickMask( s32 PlayerIndex )
{
    ASSERT( g_NetworkMgr.IsServer() );

    u32 Mask = 0x00000000;

    if( m_Score.IsTeamBased )
    {
        s32 Team = m_Score.Player[PlayerIndex].Team;
        if( m_Score.Team[Team].Size <= 2 )
            return( Mask );
    }
    else
    {
        if( m_Score.NPlayers <= 3 )
            return( Mask );
    }

    for( s32 i = 0; i < 32; i++ )
    {
        // Can't kick yourself.
        // Can't kick the server.
        // Can't kick someone who is not in the game.
        if( (i == PlayerIndex) || 
            (i == g_NetworkMgr.GetLocalPlayerSlot(0)) ||
            (m_Score.Player[i].IsInGame == FALSE) )
        {
            continue;
        }

        // TO DO - Don't allow player to kick bots.
        
        // Can't kick players on the other team.  (Its THEIR job.)
        if( m_Score.IsTeamBased && 
            (m_Score.Player[i].Team != m_Score.Player[PlayerIndex].Team) )
        {
            continue;
        }

        // Couldn't find a reason to eliminate 'i', so mask him in.
        Mask |= (1 << i);
    }

    return( Mask );
}

//==============================================================================

void game_mgr::UpdateVoteRights( void )
{
    if( !m_UpdateVoteRights )
        return;

    m_UpdateVoteRights = FALSE;

    LOG_MESSAGE( "game_mgr::UpdateVoteRights", "" );

    xbool EnoughTime = TRUE;

    if( (m_ClockMode == -1) && (                m_Clock  < 60.0f) )  EnoughTime = FALSE;
    if( (m_ClockMode == +1) && ((m_ClockLimit - m_Clock) < 60.0f) )  EnoughTime = FALSE;

    for( s32 i = 0; i < 32; i++ )
    {
        if( !m_Score.Player[i].IsInGame )
            continue;

        actor* pPlayer = (actor*)NetObjMgr.GetObjFromSlot( i );
        if( !pPlayer )
        {
            LOG_ERROR( "game_mgr::UpdateVoteRights",
                        "Could not acquire player %d (%s).", 
                        i, m_Score.Player[i].NName );
            continue;
        }

        if( (m_VotePercent == 0) || (!m_GameInProgress) )
        {
            // NOBODY can CAST  a vote.
            // NOBODY can START a vote.

            pPlayer->SetVoteCanCast     ( FALSE );
            pPlayer->SetVoteCanStartMap ( FALSE );
            pPlayer->SetVoteCanStartKick( 0x00000000 );
        }
        else
        if( m_VoteInProgress )
        {
            // SOME   can CAST  a vote.
            // NOBODY can START a vote.

            xbool CanCast = FALSE;
            if( m_VoteCanCast & (1 << i) )
                CanCast = TRUE;

            pPlayer->SetVoteCanCast     ( CanCast );
            pPlayer->SetVoteCanStartMap ( FALSE );
            pPlayer->SetVoteCanStartKick( 0x00000000 );
        }
        else
        if( !EnoughTime )
        {
            // NOBODY can CAST  a vote.
            // NOBODY can START a vote.

            pPlayer->SetVoteCanCast     ( FALSE );
            pPlayer->SetVoteCanStartMap ( FALSE );
            pPlayer->SetVoteCanStartKick( 0x00000000 );
        }
        else
        {
            // NOBODY can CAST  a vote.
            // ALL    can START a MAP  vote.
            // SOME   can START a KICK vote.

            pPlayer->SetVoteCanCast     ( FALSE );
            pPlayer->SetVoteCanStartMap ( (m_Score.NPlayers > 1) || g_NetworkMgr.IsClient() );
            pPlayer->SetVoteCanStartKick( GetVoteKickMask( i ) );
        }
    }
}

//==============================================================================

spawn_point* game_mgr::SelectSpawnPoint( u32 TeamBits )
{
    select_slot_iterator SlotIter;
    xarray<spawn_point*> SpawnPtList;
    SpawnPtList.SetCapacity( g_ObjMgr.GetNumInstances( object::TYPE_SPAWN_POINT ) );

    /*
    LOG_MESSAGE( "game_mgr::SelectSpawnPoint", "Spawn points: %d", 
                 g_ObjMgr.GetNumInstances( object::TYPE_SPAWN_POINT ) );
    */

    //
    // Make a list of spawn point candidates.  For team based games, filter out
    // "enemy team" spawn points.
    //

    xbool CheckBits = TRUE;

    attempt_recovery:

    g_ObjMgr.SelectByAttribute( object::ATTR_ALL, object::TYPE_SPAWN_POINT );
    for( SlotIter.Begin(); !SlotIter.AtEnd(); SlotIter.Next() )
    {
        spawn_point* pSpawnPoint = (spawn_point*)SlotIter.Get();
        ASSERT( pSpawnPoint );
        if( (m_Zone[ pSpawnPoint->GetZone1() ].Warned) || 
            (m_Zone[ pSpawnPoint->GetZone1() ].Locked) )
            continue;

        if( CheckBits )
        {   
            u32 SpawnPtBits = pSpawnPoint->GetCircuit().GetTeamBits();

            if( SpawnPtBits == 0 )
                continue;

            if( (TeamBits) && (m_Score.IsTeamBased) &&
                ((SpawnPtBits & TeamBits) == 0) )
                continue;
        }

        SpawnPtList.Append( pSpawnPoint );
    }
    SlotIter.End();

    // This is general purpose recovery code.  (Ignore team bits, try again.)
    if( (SpawnPtList.GetCount() == 0) && (CheckBits) ) 
    {
        LOG_ERROR( "game_mgr::SelectSpawnPoint", "NO SPAWN POINT AVAILABLE" );
        CheckBits = FALSE;
        goto attempt_recovery;
    }

    // This is worst case recovery code.  (Try FIRST spawn point.)
    if( SpawnPtList.GetCount() == 0 ) 
    {
        LOG_ERROR( "game_mgr::SelectSpawnPoint", "NO SPAWN POINT AVAILABLE" );
        
        g_ObjMgr.SelectByAttribute( object::ATTR_ALL, object::TYPE_SPAWN_POINT );
        for( SlotIter.Begin(); !SlotIter.AtEnd(); SlotIter.Next() )
        {
            spawn_point* pSpawnPoint = (spawn_point*)SlotIter.Get();
            ASSERT( pSpawnPoint );
            SpawnPtList.Append( pSpawnPoint );
        }
        SlotIter.End();
    }

    //
    // Make a list of pointers to all live players.
    //

    xarray<actor*> PlayerList;
    PlayerList.SetCapacity( g_ObjMgr.GetNumInstances( object::TYPE_PLAYER ) + 
                            g_ObjMgr.GetNumInstances( object::TYPE_NET_GHOST ) );

    g_ObjMgr.SelectByAttribute( object::ATTR_LIVING );
    for( SlotIter.Begin();  !SlotIter.AtEnd();  SlotIter.Next() )
    {
        object* pObject = (object*)SlotIter.Get();
        if( pObject && pObject->IsKindOf( actor::GetRTTI() ) )
        {
            actor* pActor = (actor*)pObject;
            ASSERT( pActor );
            if( pActor->IsAlive() )
                PlayerList.Append( pActor );
        }
    }
    SlotIter.End();

    //
    // For every spawn point, we need to know the shortest distance from it
    // to any living player.
    //

    s32 i, j;
    xarray<f32> DistList;
    DistList.SetCapacity( SpawnPtList.GetCount() );

    for( i = 0; i < SpawnPtList.GetCount(); ++i )
    {
        f32 Shortest = F32_MAX;
        for( j = 0; j < PlayerList.GetCount(); ++j )
        {   
            const f32 Dist = (SpawnPtList[i]->GetPosition() - 
                               PlayerList[j]->GetPosition()).LengthSquared();
            if( Dist < Shortest )
            {
                Shortest = Dist;
            }
        }
        DistList.Append( Shortest );
    }

    //
    // Find the spawn point with the greatest short-distance to a player.
    //
    s32 Index = 0;
    f32 Dist  = 0;
    for( i = 0; i < SpawnPtList.GetCount(); ++i )
    {
        if( DistList[i] > Dist )
        {   
            Index = i;
            Dist  = DistList[i];
        }
    }

    /*
    LOG_MESSAGE( "game_mgr::SelectSpawnPoint",
                 "Selected:%d", Index );
    */

    return( SpawnPtList[Index] );
}

//==============================================================================

void game_mgr::SetZoneLimits( s32 Zone, s32 Min, s32 Max )
{
    m_Zone[Zone].Minimum = Min;
    m_Zone[Zone].Maximum = Max;
}

//==============================================================================

void game_mgr::SetZoneMinimum( s32 Minimum )
{
    m_ZoneMinimum = Minimum;
}

//==============================================================================

void game_mgr::SetMapScaling( xbool MapScaling )
{
    m_MapScaling = MapScaling;
}

//==============================================================================

xbool game_mgr::GetMapScaling( void )
{
    return( m_MapScaling );
}

//==============================================================================

void game_mgr::SetSpeaking( s32 PlayerIndex, xbool Speaking )
{
    if( PlayerIndex != -1 )
    {
        ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );
        m_Score.Player[ PlayerIndex ].Speaking = Speaking;
    }
}

//==============================================================================

void game_mgr::SetScoreDisplay( score_display ScoreDisplay )
{
    m_ScoreDisplay = ScoreDisplay;
    m_DirtyBits |= DIRTY_SCORE_DISPLAY;
}

//==============================================================================

void game_mgr::DebugSetSkin( s32 Skin )
{
    actor* pActor;

    if( !m_GameInProgress )
        return;

    for( s32 i = 0; i < 32; i++ )
    {
        if( !m_Score.Player[i].IsInGame )
            continue;

        pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
        if( !pActor )
            continue;

        pActor->net_SetSkin( Skin );
    }
}

//==============================================================================

void game_mgr::DebugSetVoiceActor( s32 VoiceActor )
{
    actor* pActor;

    if( !m_GameInProgress )
        return;

    for( s32 i = 0; i < 32; i++ )
    {
        if( !m_Score.Player[i].IsInGame )
            continue;

        pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
        if( !pActor )
            continue;

        pActor->net_SetVoiceActor( VoiceActor );
    }
}

//==============================================================================

void game_mgr::ScoreDisplayLogic( void )
{
    for( s32 LocalPlayerNum = 0; LocalPlayerNum < g_NetworkMgr.GetLocalPlayerCount(); LocalPlayerNum++ )
    {

        switch( GetScoreDisplay() )
        {
        //--------------------------------------------------------------------------
        case SCORE_DISPLAY_RANKED:
            {
                // Three options:
                //  - If tie scores (for 1st or with YOU), display only your own
                //    score (no ranks).
                //  - If YOU lead, show YOU on top and 2nd below.
                //  - If YOU do not lead, show lead on top and you below.

                s32 NFirst =  0;
                s32 NSame  =  0;
                s32 First  = -1;
                s32 Next   = 32;
                s32 NNext  =  0;
                s32 You    = g_NetworkMgr.GetLocalPlayerSlot( LocalPlayerNum );
                s32 Rank;

                ASSERT( m_Score.Player[You].IsInGame );

                if( !IN_RANGE( 0, You, 31 ) )
                {
                    ASSERT( FALSE );
                    continue;
                }

                Rank = m_Score.Player[You].Rank;

                for( s32 i = 0; i < 32; i++ )
                {
                    s32 R = m_Score.Player[i].Rank;
                    if( R == 1    )     NFirst++;
                    if( R == Rank )     NSame++;
                    if( R == 1    )     First = i;
                }

                s32 PNext = 0;

                for( s32 i = 0; i < 32; i++ )
                {
                    s32 R = m_Score.Player[i].Rank;
                    if( R > 1 )
                    {
                        if( R < Next )
                        {
                            Next  = R;
                            NNext = 1;
                            PNext = i;
                        }
                        else if( R == Next )                
                        {
                            NNext += 1;
                        }
                    }
                    else if( (i != You) && (R == 1) )
                    {
                        PNext = i;
                    }
                }

                // Nobody has a score over 0, OR we have two player split screen.
                if( (m_Score.Player[First].Score <= 0) || 
                    (g_NetworkMgr.GetLocalPlayerCount() == 2) )
                {
                    // Display:
                    //                      YourName 123    (green)
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You, 0, You, You,  1 );
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You, 0,   0,   0, -2 );
                }

                // Three or more players in split screen and somebody has a 
                // positive score.
                else if( g_NetworkMgr.GetLocalPlayerCount() >= 3 )
                {
                    // Display:
                    //                  Nth YourName 123    (green)
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You, Rank, You, You,  1 );
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You,    0,   0,   0, -2 );
                }

                // You are the only one playing.
                else if( (NFirst == 1) && (NNext == 0) )
                {
                    // Display:
                    //                      YourName 123    (green)
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You, 1, You, You,  1 );
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You, 0,   0,   0, -2 );
                }

                // You are in first and not the only one playing.
                else if( Rank == 1 )
                {
                    // Display:
                    //                  1st YourName 456    (green)
                    //                  2nd SomeBody 123    ( red )
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You,                          1,   You,   You,  1 );
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You, m_Score.Player[PNext].Rank, PNext, PNext,  2 );
                }

                // You are not in first.
                else if( Rank > 1 )
                {
                    // Display:
                    //                  1st SomeBody 456    ( red )
                    //                  3rd YourName 123    (green)
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You,    1, First, First, 1 );
                    MsgMgr.Message( MSG_SCORE_DISPLAY_RANKED, You, Rank,   You,   You, 2 );
                }

                else
                {
                    ASSERTS( FALSE, "Uncaught score display!" );
                }

                break;
            }

        //--------------------------------------------------------------------------
        case SCORE_DISPLAY_CNH:
            {
                s32 You = g_NetworkMgr.GetLocalPlayerSlot( LocalPlayerNum );
                ASSERT( m_Score.Player[You].IsInGame );

                if( !IN_RANGE( 0, You, 31 ) )
                {
                    ASSERT( FALSE );
                    continue;
                }

                // Get the number of cappoints each team has.
                s32 AlphaCaps = 0;
                s32 OmegaCaps = 0;

                g_ObjMgr.SelectByAttribute( object::ATTR_COLLIDABLE, object::TYPE_CAP_POINT );
                slot_id aID = g_ObjMgr.StartLoop();
                while( (aID != SLOT_NULL) )
                {
                    object* pObject = g_ObjMgr.GetObjectBySlot(aID);

                    cap_point& CapPoint = cap_point::GetSafeType( *pObject );

                    if( (CapPoint.net_GetTeamBits() & 0x00000001) )
                    {
                        AlphaCaps++;
                    }
                    else if( (CapPoint.net_GetTeamBits() & 0x00000002) )
                    {
                        OmegaCaps++;
                    }

                    aID = g_ObjMgr.GetNextResult( aID );
                }
                g_ObjMgr.EndLoop();

                if( m_Score.Player[You].Team == 0 )
                {
                    // Display:
                    //                  Alpha 1 123   (green)
                    //                  Omega 3 456   ( red )
                    MsgMgr.Message( MSG_SCORE_DISPLAY_CNH, You, 0, AlphaCaps, 0, 1 );
                    MsgMgr.Message( MSG_SCORE_DISPLAY_CNH, You, 1, OmegaCaps, 1, 2 );
                }
                else
                {
                    // Display:
                    //                  Omega 3 456   (green)
                    //                  Alpha 1 123   ( red )
                    MsgMgr.Message( MSG_SCORE_DISPLAY_CNH, You, 1, OmegaCaps, 1, 1 );
                    MsgMgr.Message( MSG_SCORE_DISPLAY_CNH, You, 0, AlphaCaps, 0, 2 );
                }
            }
            return;
            break;

        //--------------------------------------------------------------------------
        case SCORE_DISPLAY_TEAM:
            {
                s32 You = g_NetworkMgr.GetLocalPlayerSlot( LocalPlayerNum );
                ASSERT( m_Score.Player[You].IsInGame );

                if( !IN_RANGE( 0, You, 31 ) )
                {
                    ASSERT( FALSE );
                    continue;
                }

                if( m_Score.Player[You].Team == 0 )
                {
                    // Display:
                    //                  Alpha 123   (green)
                    //                  Omega 456   ( red )
                    MsgMgr.Message( MSG_SCORE_DISPLAY_TEAM, You, 0, 0, 0, 1 );
                    MsgMgr.Message( MSG_SCORE_DISPLAY_TEAM, You, 0, 1, 1, 2 );
                }
                else
                {
                    // Display:
                    //                  Omega 456   (green)
                    //                  Alpha 123   ( red )
                    MsgMgr.Message( MSG_SCORE_DISPLAY_TEAM, You, 0, 1, 1, 1 );
                    MsgMgr.Message( MSG_SCORE_DISPLAY_TEAM, You, 0, 0, 0, 2 );
                }

                return;
                break;
            }

        //--------------------------------------------------------------------------
        /*
        case SCORE_DISPLAY_CNH:
            {
                // Iterate through all cap point objects in the world.
                // Count number of cap points loyal to each team.
                // May need to add a function to the cap point to get this.
                // Must make sure this works on clients as well as server.

                s32 Count0; // Number of cap points for team Alpha.
                s32 Count1; // Number of cap points for team Omega.

                Count0 = 1;
                Count1 = 2;

                s32 You = g_NetworkMgr.GetLocalPlayerSlot( LocalPlayerNum );

                if( !IN_RANGE( 0, You, 31 ) )
                {
                    ASSERT( FALSE );
                    return;
                }

                if( GameMgr.GetScore().Player[You].Team == 0 )
                {
                    // Display:
                    //                  Alpha (1) 123   (green)
                    //                  Omega (3) 456   ( red )
                }
                else
                {
                    // Display:
                    //                  Omega (3) 456   (green)
                    //                  Alpha (1) 123   ( red )
                }

                return;
                break;
            }
        */
        //--------------------------------------------------------------------------
        case SCORE_DISPLAY_NONE:    
        case SCORE_DISPLAY_CUSTOM:  
            return;
            break;

        default:
            ASSERT( FALSE );
            break;  
        }
    }
}

//==============================================================================

void game_mgr::SetVoiceAllowed( s32 PlayerIndex, xbool IsVoiceAllowed )
{
    ASSERT( g_NetworkMgr.IsServer() == TRUE );
    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    player_score& PlayerScore = m_Score.Player[ PlayerIndex ];

    // Check if the voice status has changed.
    if( IsVoiceAllowed != PlayerScore.IsVoiceAllowed )
    {
        PlayerScore.IsVoiceAllowed = IsVoiceAllowed;

        // Signify that this player's score structure has been changed
        // and needs to be transmitted to all clients.
        m_ScoreBits |= (1 << PlayerIndex);
    }
}

//==============================================================================

void game_mgr::SetVoiceCapable( s32 PlayerIndex, xbool IsVoiceCapable )
{
    ASSERT( g_NetworkMgr.IsServer() == TRUE );
    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    player_score& PlayerScore = m_Score.Player[ PlayerIndex ];

    // Check if the voice status has changed.
    if( IsVoiceCapable != PlayerScore.IsVoiceCapable )
    {
        PlayerScore.IsVoiceCapable = IsVoiceCapable;

        // Signify that this player's score structure has been changed
        // and needs to be transmitted to all clients.
        m_ScoreBits |= (1 << PlayerIndex);
    }
}

//==============================================================================
#ifdef mtraub

void InitFakeScore( void )
{
    s_FakeScore.IsGameOver      = FALSE;
    s_FakeScore.IsTeamBased     = s_FakeTeams;
    s_FakeScore.ScoreFieldMask  = SCORE_POINTS |
                                  SCORE_KILLS  |
                                  SCORE_DEATHS |
                                  SCORE_VOTES  |
                                  SCORE_TKS    |
                                  SCORE_FLAGS;
    s_FakeScore.HighLightMask   = x_rand();
    s_FakeScore.NConnected      = s_FakeCount;
    s_FakeScore.NPlayers        = s_FakeCount;

    for( s32 i = 0; i < 2; i++ )
    {
        x_sprintf( s_FakeScore.Team[i].NName, "Team-%02d-123", i );
        x_mstrcpy( s_FakeScore.Team[i].Name, s_FakeScore.Team[i].NName );
        x_mstrcpy( s_TeamName[i], s_FakeScore.Team[i].NName );
        s_FakeScore.Team[i].Score = x_irand( 0, 1000 );
        s_FakeScore.Team[i].Size  = 0;
    }

    s32 Offset = x_irand( 0, 31 );
    for( s32 I = 0; I < 32; I++ )
    {
        s32 i = (I+Offset) & 31;

        if( i == 7 )
            x_sprintf( s_FakeScore.Player[i].NName, "WWWWWWWWWWWWWWW" );
        else
            x_sprintf( s_FakeScore.Player[i].NName, "Fake-%02d-%03d", i, x_irand(0,999) );
        x_mstrcpy( s_FakeScore.Player[i].Name, s_FakeScore.Player[i].NName );

        s_FakeScore.Player[i].IsConnected   = (I < s_FakeCount);
        s_FakeScore.Player[i].IsInGame      = (I < s_FakeCount);
        s_FakeScore.Player[i].IsBot         = FALSE;
        s_FakeScore.Player[i].Team          = s_FakeTeams ? x_irand(0,1) : i;
        s_FakeScore.Player[i].ClientIndex   = i-1;
        s_FakeScore.Player[i].Identifier    = x_rand();
        s_FakeScore.Player[i].MusicLevel    = 0;
        s_FakeScore.Player[i].Speaking      = FALSE;
        s_FakeScore.Player[i].IsVoiceAllowed= FALSE;
        s_FakeScore.Player[i].IsVoiceCapable= FALSE;
        s_FakeScore.Player[i].Score         = x_irand( 0, 1000 );
        s_FakeScore.Player[i].Kills         = x_irand( 0,  100 );
        s_FakeScore.Player[i].Deaths        = x_irand( 0,  100 );
        s_FakeScore.Player[i].TKs           = x_irand( 0,   10 );
        s_FakeScore.Player[i].Flags         = x_irand( 0,   10 );
        s_FakeScore.Player[i].Votes         = x_irand( 0,   10 );

        /*
        if( s_FakeTeams && 
            (s_FakeScore.Team[ s_FakeScore.Player[i].Team ].Size >= 16) )
        {
            s_FakeScore.Player[i].Team = 1 - s_FakeScore.Player[i].Team;
        }
        */
        if( s_FakeScore.Player[i].IsInGame )
            s_FakeScore.Team[ s_FakeScore.Player[i].Team ].Size++;        
    }

    /*
    if( s_FakeTeams )
    {
        ASSERT( s_FakeScore.Team[ 0 ].Size <= 16 );
        ASSERT( s_FakeScore.Team[ 1 ].Size <= 16 );
    }
    */
}

#endif
//==============================================================================
