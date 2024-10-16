//==============================================================================
//
//  logic_TDM.cpp
//
//==============================================================================
/*

    TO DO:

    - Need to add team indications to spawn points.
    - Team damage considerations.  
  
*/
//==============================================================================

//==============================================================================
//    INCLUDES
//==============================================================================

#include "logic_tdm.hpp"
#include "NetObjMgr.hpp"
#include "NetworkMgr.hpp"
#include "MsgMgr.hpp"
#include "PainMgr/PainTypes.hpp"

#include "Objects/Actor/Actor.hpp"
#include "Objects/Player.hpp"   // - For spawn_point

//==============================================================================
//  TYPES
//==============================================================================

enum
{
    PLAYER_SCORE_KILL_ENEMY =  1,
    PLAYER_SCORE_KILL_ALLY  = -2,
    PLAYER_SCORE_KILL_SELF  = -1,

    TEAM_SCORE_KILL_ENEMY   =  1,
    TEAM_SCORE_KILL_ALLY    = -1,
    TEAM_SCORE_KILL_SELF    = -1,
};

//==============================================================================
//    FUNCTIONS
//==============================================================================

logic_tdm::logic_tdm( void )
{
}

//==============================================================================

logic_tdm::~logic_tdm( void )
{
}

//==============================================================================

void logic_tdm::BeginGame( void )
{
    if( g_NetworkMgr.IsClient() )
        return;

    GameMgr.SetScoreDisplay( SCORE_DISPLAY_TEAM );

    // Validate the mutation mode.  Normally the mutation mode is set in the UI
    // before the game starts.  The UI filters the modes to prevent a bad
    // selected.  However, players can vote to change the game type, so we must
    // revalidate the mutation mode and alter it if it is unacceptable.

    switch( GameMgr.m_MutationMode )
    {
        // These are acceptable.
        case MUTATE_CHANGE_AT_WILL:
        case MUTATE_HUMAN_LOCK:
        case MUTATE_MUTANT_LOCK:
        case MUTATE_HUMAN_VS_MUTANT:
            break;

        // These are NOT acceptable.  Must alter the mode.
        case MUTATE_MAN_HUNT:
        case MUTATE_MUTANT_HUNT:
            GameMgr.m_MutationMode = MUTATE_CHANGE_AT_WILL;
            break;
    }

    for( s32 i = 0; i < 32; i++ )
    {
        m_RespawnDelay[i] = 0.0f;
        m_Alive       [i] = FALSE;
    }
}

//==============================================================================

void logic_tdm::RequestSpawn( s32 PlayerIndex, xbool Immediate )
{
    if( g_NetworkMgr.IsClient() )
        return;
  
    u32 TeamBits = GetTeamBits( PlayerIndex );

    if( !m_Alive[PlayerIndex] && 
        ( (m_RespawnDelay[PlayerIndex] == 0.0f) || Immediate ) )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
        ASSERT( pActor );

        m_Alive[PlayerIndex] = TRUE;

        // Find a spawn point.  HACK HACK
        {
            spawn_point* pSpawnPoint = GameMgr.SelectSpawnPoint( TeamBits );
            ASSERT( pSpawnPoint );
            MoveToSpawnPt( PlayerIndex, *pSpawnPoint );
        }

        pActor->OnSpawn();

        pActor->AddItemToInventory2( INVEN_WEAPON_MUTATION       );
        pActor->AddItemToInventory2( INVEN_WEAPON_DESERT_EAGLE   );
        pActor->AddAmmoToInventory2( INVEN_AMMO_DESERT_EAGLE, 40 );
        pActor->AddAmmoToInventory2( INVEN_GRENADE_FRAG,       1 );

        pActor->ReloadAllWeapons();

        pActor->net_EquipWeapon2( INVEN_WEAPON_DESERT_EAGLE );
        
        pGameLogic->SetMutationRights( PlayerIndex );
    }
}

//==============================================================================

void logic_tdm::PlayerDied( s32 Victim, s32 Killer, s32 PainType )
{
    ASSERT( g_NetworkMgr.IsServer() );

    (void)Killer;
    (void)PainType;

    // The player could already be dead if killed on the server and a client
    // at approximately the same time since the client kill has latency.

    if( m_Alive[Victim] )
    {
        m_Alive       [Victim] = FALSE;
        m_RespawnDelay[Victim] = 3.0f;

        if( PainType == SERVER_MAINTENANCE )
            return;

        //
        // Messages and score.
        //
        MsgMgr.PlayerDied( Victim, Killer, PainType );

        s32 VTeam = GameMgr.m_Score.Player[Victim].Team;

        if( (Killer == -1) || (Killer == Victim) )
        {
            GameMgr.m_Score.Team  [VTeam ].Score += TEAM_SCORE_KILL_SELF;
            GameMgr.m_Score.Player[Victim].Score += PLAYER_SCORE_KILL_SELF;
            if( PLAYER_SCORE_KILL_SELF )
                MsgMgr.Message( MSG_SUICIDE_PENALTY, Victim, PLAYER_SCORE_KILL_SELF );
        }
        else
        { 
            s32 KTeam = GameMgr.m_Score.Player[Killer].Team;

            // Kill enemy?
            if( (Killer != Victim) && (KTeam != VTeam) )
            {
                GameMgr.m_Score.Team  [KTeam ].Score += TEAM_SCORE_KILL_ENEMY;
                GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_ENEMY;  
                GameMgr.m_Score.Player[Killer].Kills += 1;
            }

            // Kill ally?
            if( (Killer != Victim) && (KTeam == VTeam) )
            {
                GameMgr.m_Score.Team  [KTeam ].Score += TEAM_SCORE_KILL_ALLY;
                GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_ALLY;
                GameMgr.m_Score.Player[Killer].TKs   += 1;
                if( PLAYER_SCORE_KILL_ALLY )
                    MsgMgr.Message( MSG_TEAM_KILL_PENALTY, Killer, PLAYER_SCORE_KILL_ALLY );
            }

            GameMgr.m_ScoreBits |= (1 << Killer);
        }

        // Victim gets a death.
        GameMgr.m_Score.Player[Victim].Deaths += 1;

        GameMgr.m_ScoreBits |= (1 << Victim);
        GameMgr.m_DirtyBits |= game_mgr::DIRTY_TEAM_SCORES;
    }
}

//==============================================================================

void logic_tdm::AdvanceTime( f32 DeltaTime )
{
    if( g_NetworkMgr.IsClient() )
        return;

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
}

//==============================================================================

void logic_tdm::Connect( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    s32         Team  = -1;
    game_score& Score = GameMgr.m_Score;

    // Need to choose a team for this player.
    if( Score.Team[0].Size <  Score.Team[1].Size )    Team = 0;
    if( Score.Team[0].Size >  Score.Team[1].Size )    Team = 1;
    if( Score.Team[0].Size == Score.Team[1].Size )
    {
        // TO DO - If bots enabled, consider the number of humans.
        Team = PlayerIndex & 0x01;
    }

    Score.Player[PlayerIndex].Team = Team;
    Score.Team[Team].Size++;

    // Dirty bits.
    GameMgr.m_PlayerBits |= (1 << PlayerIndex);
}

//==============================================================================

xbool logic_tdm::IsTeamBased( void )
{
    return( TRUE );
}

//==============================================================================

void logic_tdm::Disconnect( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    s32 Team = GameMgr.m_Score.Player[PlayerIndex].Team;

    GameMgr.m_Score.Team[Team].Size--;

    // Dirty bits.
    GameMgr.m_DirtyBits  |= game_mgr::DIRTY_TEAM_SCORES;
    GameMgr.m_PlayerBits |= (1 << PlayerIndex);
}

//==============================================================================

void logic_tdm::EnterGame( s32 PlayerIndex )
{
    m_RespawnDelay[PlayerIndex] = 0.0f;
    m_Alive       [PlayerIndex] = FALSE;
}

//==============================================================================

void logic_tdm::ExitGame( s32 PlayerIndex )
{
    if( GameMgr.m_GameInProgress && m_Alive[PlayerIndex] )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
        ASSERT( pActor );
        pActor->OnDeath();
    }

    m_RespawnDelay[PlayerIndex] = 0.0f;
    m_Alive       [PlayerIndex] = FALSE;
}

//==============================================================================

u32 logic_tdm::GetTeamBits( s32 PlayerIndex )
{
    return( 1 << GameMgr.m_Score.Player[PlayerIndex].Team );
}

//==============================================================================
