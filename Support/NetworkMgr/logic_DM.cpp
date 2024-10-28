//==============================================================================
//
//  logic_DM.cpp
//
//==============================================================================

//==============================================================================
//    INCLUDES
//==============================================================================

#include "logic_DM.hpp"
#include "NetObjMgr.hpp"
#include "NetworkMgr.hpp"
#include "PainMgr/PainTypes.hpp"

#include "Objects/Actor/Actor.hpp"
#include "Objects/Player.hpp"   // - For spawn_point

#include "MsgMgr.hpp"

//==============================================================================
//  TYPES
//==============================================================================

enum
{
    PLAYER_SCORE_KILL_ENEMY =  1,
    PLAYER_SCORE_KILL_SELF  = -1,
};

//==============================================================================
//    FUNCTIONS
//==============================================================================

logic_dm::logic_dm( void )
{
}

//==============================================================================

logic_dm::~logic_dm( void )
{
}

//==============================================================================

void logic_dm::BeginGame( void )
{
    if( g_NetworkMgr.IsClient() )
        return;

    GameMgr.SetScoreDisplay( SCORE_DISPLAY_RANKED );

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
            break;

        // These are NOT acceptable.  Must alter the mode.
        case MUTATE_HUMAN_VS_MUTANT:
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

void logic_dm::RequestSpawn( s32 PlayerIndex, xbool Immediate )
{
    if( g_NetworkMgr.IsClient() )
        return;

    u32 TeamBits = GetTeamBits( PlayerIndex );

    if( !m_Alive[PlayerIndex] && 
        ( (m_RespawnDelay[PlayerIndex] == 0.0f) || Immediate ) )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );

        m_Alive[PlayerIndex] = TRUE;

        // Find a spawn point.  HACK HACK
        {
            spawn_point* pSpawnPoint = GameMgr.SelectSpawnPoint( TeamBits );
            ASSERT( pSpawnPoint );
            MoveToSpawnPt( PlayerIndex, *pSpawnPoint );
        }
        
        pActor->OnSpawn();
        
        pActor->ClearInventory2();
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

void logic_dm::PlayerDied( s32 Victim, s32 Killer, s32 PainType )
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

    // Messages.

    MsgMgr.PlayerDied( Victim, Killer, PainType );

    logic_base::NonTeamScoreMsgPrep();

    // Scoring.

    if( (Killer == -1) || (Killer == Victim) )
    {
        GameMgr.m_Score.Player[Victim].Score += PLAYER_SCORE_KILL_SELF;
        if( PLAYER_SCORE_KILL_SELF )
            MsgMgr.Message( MSG_SUICIDE_PENALTY, Victim, PLAYER_SCORE_KILL_SELF );
    }
    else if( Killer != Victim )
    {
        GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_ENEMY;
        GameMgr.m_Score.Player[Killer].Kills += 1;
        GameMgr.m_ScoreBits |= (1 << Killer);
    }

    GameMgr.m_Score.Player[Victim].Deaths += 1;
    GameMgr.m_ScoreBits |= (1 << Victim);

    // More messages.

    logic_base::NonTeamScoreMsgExec( Killer != Victim ? Killer : -1 );
}

//==============================================================================

void logic_dm::AdvanceTime( f32 DeltaTime )
{
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

void logic_dm::Connect( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    GameMgr.m_Score.Player[PlayerIndex].Team = PlayerIndex;

    // Dirty bits.
    GameMgr.m_PlayerBits |= (1 << PlayerIndex);
}

//==============================================================================

void logic_dm::EnterGame( s32 PlayerIndex )
{
    m_RespawnDelay[PlayerIndex] = 0.0f;
    m_Alive       [PlayerIndex] = FALSE;
}

//==============================================================================

void logic_dm::ExitGame( s32 PlayerIndex )
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
