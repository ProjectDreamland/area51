//==============================================================================
//
//  logic_Infect.cpp
//
//==============================================================================
/*

    TO DO:

    - Team damage considerations.  
    - Messages and maybe some audio.
    - Score display.
    - What about teams?  Don't want to use a team based score display.  Yet
      would like to support team speak for the headset.  Maybe we use a team
      based display while in game, but switch it to non team for the final
      score display.
    - Must behave reasonably when there are less than 3 players in the game.
  
*/
//==============================================================================

//==============================================================================
//	INCLUDES
//==============================================================================

#include "logic_Infect.hpp"
#include "NetObjMgr.hpp"
#include "NetworkMgr.hpp"
#include "MsgMgr.hpp"
#include "PainMgr/PainTypes.hpp"

#include "Objects/Actor/Actor.hpp"
#include "Objects/Player.hpp"   // - For spawn_point.

//==============================================================================
//  TYPES
//==============================================================================

enum
{
    PLAYER_SCORE_SURVIVAL   =  1,
    PLAYER_SCORE_DEFENSE    =  3,
    PLAYER_SCORE_INFECTION  =  3,
    PLAYER_SCORE_LAST_MAN   =  5,

    PLAYER_SCORE_KILL_ALLY  =  0,
    PLAYER_SCORE_KILL_SELF  = -1,
};

//==============================================================================
//	FUNCTIONS
//==============================================================================

logic_infect::logic_infect( void )
{
    m_State       =  0;
    m_LastRandom  = -1;
    m_PatientZero = -1;
}

//==============================================================================

logic_infect::~logic_infect( void )
{
}

//==============================================================================

void logic_infect::BeginGame( void )
{
    if( g_NetworkMgr.IsClient() )
        return;

    GameMgr.SetScoreDisplay( SCORE_DISPLAY_RANKED );

    LOG_MESSAGE( "logic_infect::BeginGame", "" );

    // Only one mutation mode is acceptable for Infection.  So just force it.
    GameMgr.m_MutationMode = MUTATE_HUMAN_VS_MUTANT;

    m_State     = 0;    // PreGame.
    m_NInfected = 0;
    m_NNormal   = 0;

    for( s32 i = 0; i < 32; i++ )
    {
        m_RespawnDelay[i] = 0.0f;
        m_Alive       [i] = FALSE;
        m_Infected    [i] = FALSE;

        if( GameMgr.m_Score.Player[i].IsInGame )
            m_NNormal++;
    }

    // Update the player highlight.
    GameMgr.m_Score.HighLightMask = 0x00000000;
    GameMgr.m_DirtyBits          |= game_mgr::DIRTY_HIGHLIGHT;
}

//==============================================================================

void logic_infect::RequestSpawn( s32 PlayerIndex, xbool Immediate )
{
    if( g_NetworkMgr.IsClient() )
        return;
  
	if( !m_Alive[PlayerIndex] && 
        ( (m_RespawnDelay[PlayerIndex] == 0.0f) || Immediate ) )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
        ASSERT( pActor );

        m_Alive[PlayerIndex] = TRUE;

        // Find a spawn point.  HACK HACK
        {
            spawn_point* pSpawnPoint = GameMgr.SelectSpawnPoint( 0 );
            ASSERT( pSpawnPoint );
            MoveToSpawnPt( PlayerIndex, *pSpawnPoint );
        }

        SetTeam          ( PlayerIndex, m_Infected[PlayerIndex] ? 1 : 0 );

        pActor->OnSpawn();

        pActor->AddItemToInventory2( INVEN_WEAPON_MUTATION       );
        pActor->AddItemToInventory2( INVEN_WEAPON_DESERT_EAGLE   );
        pActor->AddAmmoToInventory2( INVEN_AMMO_DESERT_EAGLE, 40 );
        pActor->AddAmmoToInventory2( INVEN_GRENADE_FRAG,       1 );

        pActor->ReloadAllWeapons();

        pActor->net_EquipWeapon2( INVEN_WEAPON_DESERT_EAGLE );

        SetMutationRights( PlayerIndex );

        LOG_MESSAGE( "logic_infect::RequestSpawn", 
                     "SPAWN - Player:%d - Team:%d", 
                     PlayerIndex, m_Infected[PlayerIndex] );
    }
}

//==============================================================================

void logic_infect::PlayerDied( s32 Victim, s32 Killer, s32 PainType )
{
    ASSERT( g_NetworkMgr.IsServer() );

    // The player could already be dead if killed on the server and a client
    // at approximately the same time since the client kill has latency.

    if( !m_Alive[Victim] )
        return;

    m_Alive       [Victim] = FALSE;
    m_RespawnDelay[Victim] = 3.0f;

    if( PainType == SERVER_MAINTENANCE )
        return;

    //
    // Messages and score.
    //
    MsgMgr.PlayerDied( Victim, Killer, PainType );

    logic_base::NonTeamScoreMsgPrep();

    // Death by environmental hazard or fall?  Or suicide?
    if( (Killer == -1) || (Killer == Victim) )
    {
        GameMgr.m_Score.Player[Victim].Score += PLAYER_SCORE_KILL_SELF;

        if( PLAYER_SCORE_KILL_SELF )
            MsgMgr.Message( MSG_SUICIDE_PENALTY, Victim, PLAYER_SCORE_KILL_SELF );

        LOG_MESSAGE( "logic_infect::PlayerDied",
                        "SUICIDE - Victim:%d", Victim );
    }
    else
    { 
        // Infection?
        if( (Killer != Victim) &&
            (m_Infected[Killer] == TRUE ) &&
            (m_Infected[Victim] == FALSE) )
        {
            GameMgr.m_Score.Player[Killer].Kills += 1;            
            GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_INFECTION;
            GameMgr.m_ScoreBits |= (1 << Killer);
            m_Infected[Victim] = TRUE;
            m_NInfected += 1;
            m_NNormal   -= 1;

            // We have a fresh infection.  Clear the unanswered kills counter.
            m_NMutantDeaths = 0;    

            // Message for the victim.
            if( m_NNormal > 0 )
                MsgMgr.Message( (msg_id)(MSG_INFECTED + x_irand(0,2)), Victim );
            
            // Messages for everybody else, only applicable if there are 5 or 
            // less remaining human players.
            if( IN_RANGE( 1, m_NNormal, 5 ) )
            {
                for( s32 i = 0; i < 32; i++ )
                {
                    // Already gave the victim a message, so skip him.
                    if( Victim == i )
                        continue;

                    if( !GameMgr.m_Score.Player[i].IsInGame )
                        continue;

                    if( (m_NNormal == 1) && !m_Infected[i] )
                        MsgMgr.Message( MSG_LAST_SURVIVOR, i );
                    else
                        MsgMgr.Message( (msg_id)(MSG_HUMANS_REMAIN_1 + m_NNormal - 1), i );
                }
            }

            // All "survivors" get a bonus point.
            for( s32 i = 0; i < 32; i++ )
            {
                if( !GameMgr.m_Score.Player[i].IsInGame )
                    continue;

                if( !m_Infected[i] )
                {
                    GameMgr.m_ScoreBits |= (1 << i);

                    if( m_NNormal == 1 )
                    {
                        GameMgr.m_Score.Player[i].Score += PLAYER_SCORE_LAST_MAN;
                        MsgMgr.Message( MSG_LAST_SURVIVOR_BONUS, i, PLAYER_SCORE_LAST_MAN );                        
                    }
                    else
                    {
                        GameMgr.m_Score.Player[i].Score += PLAYER_SCORE_SURVIVAL;
                        MsgMgr.Message( MSG_SURVIVAL_BONUS, i, PLAYER_SCORE_SURVIVAL );
                    }
                }
            }

            // Clear all of the idle timers.
            for( s32 i = 0; i < 32; i++ )
            {
                actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
                if( pActor )
                {
                    pActor->ClearInactiveTime();
                }
            }

            LOG_MESSAGE( "logic_infect::PlayerDied",
                         "INFECTION - Victim:%d - Killer:%d",
                         Victim, Killer );

            // Update the player highlight.
            GameMgr.m_Score.HighLightMask |= (1 << Victim);
            GameMgr.m_DirtyBits           |= game_mgr::DIRTY_HIGHLIGHT;
        }

        // Self defense?
        else
        if( (Killer != Victim) &&
            (m_Infected[Killer] == FALSE) &&
            (m_Infected[Victim] == TRUE ) )
        {
            GameMgr.m_Score.Player[Killer].Kills += 1;
            GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_DEFENSE;            
            GameMgr.m_ScoreBits |= (1 << Killer);

            m_NMutantDeaths += 1;

            LOG_MESSAGE( "logic_infect::PlayerDied",
                         "HUMAN DEFENSIVE KILL - Victim:%d - Killer:%d",
                         Victim, Killer );
        }

        // "Team kill"?
        else
        if( (Killer != Victim) &&
            (m_Infected[Killer] == m_Infected[Victim]) )
        {
            GameMgr.m_Score.Player[Killer].TKs   += 1;
            GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_ALLY;
            GameMgr.m_ScoreBits |= (1 << Killer);

            if( PLAYER_SCORE_KILL_ALLY )
                MsgMgr.Message( MSG_TEAM_KILL_PENALTY, Killer, PLAYER_SCORE_KILL_ALLY );

            LOG_MESSAGE( "logic_infect::PlayerDied",
                         "TEAM KILL - Victim:%d - Killer:%d",
                         Victim, Killer );
        }
    }

    // Victim gets a death.
    GameMgr.m_Score.Player[Victim].Deaths += 1;
    GameMgr.m_ScoreBits |= (1 << Victim);

    // More messages.

    logic_base::NonTeamScoreMsgExec( (Killer != Victim) ? Killer : -1 );
}

//==============================================================================

void logic_infect::AdvanceTime( f32 DeltaTime )
{
    ASSERT( g_NetworkMgr.IsServer() );

    if( (GameMgr.m_Score.NPlayers <= 1) && (m_State != 0) )
        EnterPreGame();

    // There are 3 states in Infection:
    //  0 - PreGame
    //  1 - Paranoia 
    //  2 - InGame

    switch( m_State )
    {
    case  0:    LogicPreGame ( DeltaTime );     break;
    case  1:    LogicParanoia( DeltaTime );     break;
    case  2:    LogicInGame  ( DeltaTime );     break;
    default:    ASSERT       ( FALSE     );     break;
    }

    // Respawn delay processing.
    for( s32 i = 0; i < 32; i++ )
    {
        if( !m_Alive[i] )
        {
            if( m_RespawnDelay[i] > 0.0f )
            {
                m_RespawnDelay[i] -= DeltaTime;
            }
            else
            {
                m_RespawnDelay[i] = 0.0f;
                if( GameMgr.m_Score.Player[i].IsInGame )
                    RequestSpawn( i );  // Force respawn!
            }
        }
    }   

    // Sanity check.
    {
        s32 I = 0;
        s32 N = 0;
        for( s32 i = 0; i < 32; i++ )
        {
            if( GameMgr.m_Score.Player[i].IsInGame )
            {
                if( m_Infected[i] )     I++;
                else                    N++;
            }
        }

        ASSERT( m_NInfected == I );
        ASSERT( m_NNormal   == N );
    }
}

//==============================================================================

void logic_infect::EnterPreGame( void )
{
    m_NInfected = 0;
    m_NNormal   = 0;
    m_Timer     = x_frand( 5.0f, 10.0f );

    GameMgr.m_Score.HighLightMask  = 0;
    GameMgr.m_DirtyBits           |= game_mgr::DIRTY_HIGHLIGHT;

    LOG_MESSAGE( "logic_infect::EnterPreGame", "" );

    for( s32 i = 0; i < 32; i++ )
    {
        m_Infected[i] = FALSE;
        if( GameMgr.m_Score.Player[i].IsInGame )
        {
            SetTeam( i, 0 );
            SetMutationRights( i );

            m_NNormal++;
        }
    }

    // It's official: PreGame!
    m_State = 0;    
}

//==============================================================================

void logic_infect::LogicPreGame( f32 DeltaTime )
{
    ASSERT( m_State     == 0 );
    ASSERT( m_NInfected == 0 );

    // Need multiple players to move forward.
    if( GameMgr.m_Score.NPlayers <= 1 )
    {
        m_Timer = MAX( m_Timer, 5.0f );     // Wait at least 5 seconds...
        return;
    }

    // Advance the countdown timer.
    m_Timer -= DeltaTime;

    // If the timer expired, Paranoia!
    if( m_Timer <= 0.0f )
        EnterParanoia();
}

//==============================================================================

void logic_infect::EnterParanoia( void )
{
    // Select Patient Zero.
    do
    {
        m_PatientZero = x_irand( 0, 31 );
    }
    while( (!GameMgr.m_Score.Player[m_PatientZero].IsInGame) ||
           (m_PatientZero == m_LastRandom) );

    // MESSAGES
    //  To the first mutant: "You are Patient Zero."
    //  To all other players, random messages such as:
    //      "<player> is looking a little green."
    //      "Is <player> sweating?"
    //      "Do you trust <player>?"

    for( s32 i = 0; i < 32; i++ )
    {
        if( !GameMgr.m_Score.Player[i].IsInGame )
            continue;

        if( i == m_PatientZero )
        {
            MsgMgr.Message( MSG_YOU_ARE_PATIENT_ZERO, i );
        }
        else
        {
            s32 Accuse = i;

            if( x_irand(0,3) == 0 )
            {
                do
                {
                    Accuse = x_irand( 0, 31 );
                }
                while( (!GameMgr.m_Score.Player[Accuse].IsInGame) || (Accuse == i) );
            }
            else
            {
                Accuse = m_PatientZero;
            }

            MsgMgr.Message( msg_id( MSG_PARANOIA_01 + x_irand(0,10) ), i, Accuse );
        }
    }

    LOG_MESSAGE( "logic_infect::AdvanceTime",
                 "PREGAME to PARANOIA -- FirstMutant:%d(%s)", 
                 m_PatientZero, GameMgr.m_Score.Player[m_PatientZero].NName );

    // And now we let the players stew with these messages.
    m_Timer = x_frand( 7.5f, 12.5f );

    // It's official: Paranoia!
    m_State = 1;    
}

//==============================================================================

void logic_infect::LogicParanoia( f32 DeltaTime )
{
    ASSERT( m_State     == 1 );
    ASSERT( m_NInfected == 0 );
    ASSERT( GameMgr.m_Score.NPlayers > 1 );

    if( m_PatientZero == -1 )
    {
        // Well, crap.  It looks like Patient Zero dropped.  Try again.
        EnterParanoia();
    }

    // Advance the countdown timer.
    m_Timer -= DeltaTime;

    // If the timer expired, InGame!
    if( m_Timer <= 0.0f )
        EnterInGame();
}

//==============================================================================

void logic_infect::EnterInGame( void )
{
    ASSERT( m_NInfected   ==  0 );
    ASSERT( m_PatientZero != -1 );
    ASSERT( GameMgr.m_Score.Player[m_PatientZero].IsInGame );
    ASSERT( GameMgr.m_Score.NPlayers > 1 );

    LOG_MESSAGE( "logic_infect::AdvanceTime",
                 "PARANOIA to INGAME -- FirstMutant:%d(%s)", 
                 m_PatientZero, GameMgr.m_Score.Player[m_PatientZero].NName );

    m_LastRandom = m_PatientZero;
    m_Infected[ m_PatientZero ] = TRUE;

    SetTeam          ( m_PatientZero, 1 );
    SetMutationRights( m_PatientZero );

    // MESSAGES
    MsgMgr.Message( MSG_PLAYER_INFECTED, m_PatientZero, m_PatientZero );

    GameMgr.m_ScoreBits           |= (1 << m_PatientZero);
    GameMgr.m_Score.HighLightMask  = (1 << m_PatientZero);
    GameMgr.m_DirtyBits           |= game_mgr::DIRTY_HIGHLIGHT;
    
    m_Timer         = 7.5f;   // This is a cool down timer.
    m_NInfected    += 1;
    m_NNormal      -= 1;
    m_NMutantDeaths = 0;

    // It's official: InGame!
    m_State = 2;    
}

//==============================================================================

void logic_infect::LogicInGame( f32 DeltaTime )
{
    ASSERT( m_State == 2 );

    // Ran out of mutants?  PreGame immediately!
    if( m_NInfected == 0 )
    {
        EnterPreGame();
        return;
    }

    if( m_NNormal > 0 )
    {
        xbool WantNewMutant = FALSE;

        // Are the mutants getting worked over?
        if( (m_NMutantDeaths >= (m_NInfected * 5)) && 
            (m_NInfected < m_NNormal) )
        {
            WantNewMutant = TRUE;
        }

        // Are all of the mutants just goofing off?
        xbool GoofOff = TRUE;
        if( !WantNewMutant )
        {
            for( s32 i = 0; i < 32; i++ )
            {
                if( (GameMgr.m_Score.Player[i].IsInGame) && (m_Infected[i]) )
                {
                    actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
                    ASSERT( pActor );
                    if( pActor && (pActor->GetInactiveTime() < 60.0f) )
                    {
                        GoofOff = FALSE;
                        break;
                    }
                }
            }
            if( GoofOff )
                WantNewMutant = TRUE;
        }

        // Are all of the humans "goofing" off?  (That is, are they all hiding?)
        GoofOff = TRUE;
        if( !WantNewMutant )
        {
            for( s32 i = 0; i < 32; i++ )
            {
                if( (GameMgr.m_Score.Player[i].IsInGame) && (!m_Infected[i]) )
                {
                    actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
                    ASSERT( pActor );
                    if( pActor && (pActor->GetInactiveTime() < 60.0f) )
                    {
                        GoofOff = FALSE;
                        break;
                    }
                }
            }
            if( GoofOff )
                WantNewMutant = TRUE;
        }

        // So... Want a new mutant or not?
        if( WantNewMutant  )
        {
            // Mutate some player at random.

            s32 NewMutant;
            do
            {
                NewMutant = x_irand( 0, 31 );
            }
            while( (!GameMgr.m_Score.Player[NewMutant].IsInGame) ||
                   (m_Infected[NewMutant]) );

            SetTeam          ( NewMutant, 1 );
            SetMutationRights( NewMutant );

            m_Infected[NewMutant] = TRUE;
            m_NInfected    += 1;
            m_NNormal      -= 1;
            m_NMutantDeaths = 0;

            MsgMgr.Message( (msg_id)(MSG_INFECTED + x_irand(0,2)), NewMutant );

            // Messages for everybody else, only applicable if there are 5 or 
            // less remaining human players.
            if( IN_RANGE( 1, m_NNormal, 5 ) )
            {
                for( s32 i = 0; i < 32; i++ )
                {
                    // Already gave the new mutant a message, so skip him.
                    if( NewMutant == i )
                        continue;

                    if( !GameMgr.m_Score.Player[i].IsInGame )
                        continue;

                    if( (m_NNormal == 1) && !m_Infected[i] )
                        MsgMgr.Message( MSG_LAST_SURVIVOR, i );
                    else
                        MsgMgr.Message( (msg_id)(MSG_HUMANS_REMAIN_1 + m_NNormal - 1), i );
                }
            }

            GameMgr.m_Score.HighLightMask |= (1 << NewMutant);
            GameMgr.m_DirtyBits           |= game_mgr::DIRTY_HIGHLIGHT;

            // Clear all of the idle timers.
            for( s32 i = 0; i < 32; i++ )
            {
                actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
                if( pActor )
                {
                    pActor->ClearInactiveTime();
                }
            }
        }
    }

    // Still got some pesky humans?  Keep playing.
    if( m_NNormal > 0 )
        return;

    // Once there are no more humans, advance the timer to cool down.
    m_Timer -= DeltaTime;

    // If the timer expired, PreGame!
    if( m_Timer <= 0.0f )
        EnterPreGame();
}

//==============================================================================

xbool logic_infect::IsTeamBased( void )
{
    return( FALSE );
}

//==============================================================================

void logic_infect::Connect( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    GameMgr.m_Score.Player[PlayerIndex].Team = PlayerIndex;

    // Dirty bits.
    GameMgr.m_PlayerBits |= (1 << PlayerIndex);
}

//==============================================================================

void logic_infect::Disconnect( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    // Dirty bits.
    GameMgr.m_PlayerBits |= (1 << PlayerIndex);
}

//==============================================================================

void logic_infect::EnterGame( s32 PlayerIndex )
{
    LOG_MESSAGE( "logic_infect::EnterGame", "Player:%d", PlayerIndex );

    m_RespawnDelay[PlayerIndex] = 0.0f;
    m_Alive       [PlayerIndex] = FALSE;

    if( m_NInfected > m_NNormal )
    {
        // New player starts infected.
        m_Infected[PlayerIndex] = TRUE;
        m_NInfected++;
    }
    else
    {
        // New player starts as human.
        m_Infected[PlayerIndex] = FALSE;
        m_NNormal++;
        GameMgr.m_Score.HighLightMask &= ~(1 << PlayerIndex);
    }

    // Dirty bits.
    GameMgr.m_DirtyBits  |= game_mgr::DIRTY_HIGHLIGHT;
}

//==============================================================================

void logic_infect::ExitGame( s32 PlayerIndex )
{
    LOG_MESSAGE( "logic_infect::ExitGame", 
                 "Player:%d - Infected:%d", 
                 PlayerIndex, m_Infected[PlayerIndex] );

    if( GameMgr.m_GameInProgress && m_Alive[PlayerIndex] )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
        ASSERT( pActor );
        pActor->OnDeath();
    }

    if( m_Infected[PlayerIndex] )   m_NInfected--;
    else                            m_NNormal--;

    if( m_PatientZero == PlayerIndex )
        m_PatientZero = -1;

    if( PlayerIndex == m_LastRandom )
        m_LastRandom = -1;

    m_Infected    [PlayerIndex] = FALSE;
    m_RespawnDelay[PlayerIndex] = 0.0f;
    m_Alive       [PlayerIndex] = FALSE;

    GameMgr.m_Score.HighLightMask &= ~(1 << PlayerIndex);

    // Dirty bits.
    GameMgr.m_PlayerBits |= (1 << PlayerIndex);
    GameMgr.m_DirtyBits  |= game_mgr::DIRTY_HIGHLIGHT;
}

//==============================================================================

u32 logic_infect::GetTeamBits( s32 PlayerIndex )
{
    if( m_Infected[ PlayerIndex ] )
        return( 0x00000002 );
    else
        return( 0x00000001 );
}

//==============================================================================
