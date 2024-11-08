//==============================================================================
//
//  logic_Tag.cpp
//
//==============================================================================
/*

    TO DO:

    - Scoring?
    - Team damage considerations.  
    - Messages and maybe some audio.
    - Score display.
  
*/
//==============================================================================

//==============================================================================
//    INCLUDES
//==============================================================================

#include "logic_Tag.hpp"
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
    PLAYER_SCORE_KILL_ENEMY =  1,
    PLAYER_SCORE_KILL_ALLY  = -1,
    PLAYER_SCORE_KILL_SELF  = -1,
};

//==============================================================================
//    FUNCTIONS
//==============================================================================

logic_tag::logic_tag( void )
{
}

//==============================================================================

logic_tag::~logic_tag( void )
{
}

//==============================================================================

void logic_tag::BeginGame( void )
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
        case MUTATE_MAN_HUNT:
        case MUTATE_MUTANT_HUNT:
            break;

        // These are NOT acceptable.  Must alter the mode.
        case MUTATE_HUMAN_VS_MUTANT:
            GameMgr.m_MutationMode = MUTATE_CHANGE_AT_WILL;
            break;
    }

    for( s32 i = 0; i < 32; i++ )
    {
        m_RespawnDelay[i] = 0.0f;
        m_Alive       [i] = FALSE;
    }

    // Nobody is IT.  Yet.
    m_It      = -1;
    m_ItTimer =  15.0f;

    // Update the player highlight.
    GameMgr.m_Score.HighLightMask = 0x00000000;
    GameMgr.m_DirtyBits          |= game_mgr::DIRTY_HIGHLIGHT;
}

//==============================================================================

void logic_tag::RequestSpawn( s32 PlayerIndex, xbool Immediate )
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

        pActor->net_SetTeamBits( 0x00000001 );

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

void logic_tag::PlayerDied( s32 Victim, s32 Killer, s32 PainType )
{
    ASSERT( g_NetworkMgr.IsServer() );

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

        // Death by environmental hazard or fall?
        if( Killer == -1 )
        {
            GameMgr.m_Score.Player[Victim].Score += PLAYER_SCORE_KILL_SELF;

            m_It      = -1;
            m_ItTimer =  5.0f;

            actor* pVictim = (actor*)NetObjMgr.GetObjFromSlot( Victim );
            ASSERT( pVictim );
            pVictim->net_SetTeamBits( 0x00000001 );

            GameMgr.m_Score.HighLightMask = 0;
            GameMgr.m_DirtyBits          |= game_mgr::DIRTY_HIGHLIGHT;

            LOG_MESSAGE( "logic_tag", "================ IT:-1(<null>)" );
        }
        else
        { 
            // Player killed IT?
            if( (Killer != m_It) && (Victim == m_It) )
            {
                GameMgr.m_Score.Player[Killer].Kills += 1;
                GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_ENEMY;

                m_It = Killer;

                actor* pKiller = (actor*)NetObjMgr.GetObjFromSlot( Killer );
                actor* pVictim = (actor*)NetObjMgr.GetObjFromSlot( Victim );
                ASSERT( pKiller );
                ASSERT( pVictim );
                pKiller->net_SetTeamBits( 0x00000002 );
                pVictim->net_SetTeamBits( 0x00000001 );

                GameMgr.m_Score.HighLightMask = (1 << m_It);
                GameMgr.m_DirtyBits          |= game_mgr::DIRTY_HIGHLIGHT;

                LOG_MESSAGE( "logic_tag", "================ IT:%d(%s)",
                             m_It, GameMgr.m_Score.Player[m_It].NName );
            }

            // IT killed a player?
            else
            if( (Killer == m_It) && (Victim != m_It) )
            {
                GameMgr.m_Score.Player[Killer].Kills += 1;
                GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_ENEMY;
            }

            // One player killed another?
            else
            if( (Killer != Victim) && (Killer != m_It) && (Victim != m_It) )
            {
                GameMgr.m_Score.Player[Killer].TKs   += 1;
                GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_ALLY;
            }

            // Player killed himself?
            else
            if( (Killer == Victim) && (Victim != m_It) )
            {
                GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_SELF;
            }

            // IT killed himself?
            else
            if( (Killer == Victim) && (Victim == m_It) )
            {
                GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_SELF;

                m_It      = -1;
                m_ItTimer =  5.0f;

                actor* pVictim = (actor*)NetObjMgr.GetObjFromSlot( Victim );
                ASSERT( pVictim );
                pVictim->net_SetTeamBits( 0x00000001 );

                GameMgr.m_Score.HighLightMask = 0;
                GameMgr.m_DirtyBits          |= game_mgr::DIRTY_HIGHLIGHT;

                LOG_MESSAGE( "logic_tag", "================ IT:-1(<null>)" );
            }

            // That should cover all the bases...  Anything left?
            else
            {
                ASSERT( FALSE );
            }

            GameMgr.m_ScoreBits |= (1 << Killer);
        }

        // Victim gets a death.
        GameMgr.m_Score.Player[Victim].Deaths += 1;

        GameMgr.m_ScoreBits |= (1 << Victim);
    }
}

//==============================================================================

void logic_tag::AdvanceTime( f32 DeltaTime )
{
    ASSERT( g_NetworkMgr.IsServer() );

    // Are we "IT-less"?
    if( (m_It == -1) && (GameMgr.m_Score.NPlayers > 0) )
    {
        if( m_ItTimer <= 0.0f )
        {
            // Pick a new IT at random.
            s32 Candidate = x_irand( 0, 31 );
            if( GameMgr.m_Score.Player[Candidate].IsInGame )
            {
                m_It = Candidate;
                actor* pIt = (actor*)NetObjMgr.GetObjFromSlot( m_It );
                ASSERT( pIt );

                pIt->net_SetTeamBits( 0x00000002 );

                // MESSAGE

                GameMgr.m_Score.HighLightMask = (1 << m_It);
                GameMgr.m_DirtyBits          |= game_mgr::DIRTY_HIGHLIGHT;

                LOG_MESSAGE( "logic_tag", "================ IT:%d(%s)",
                             m_It, GameMgr.m_Score.Player[m_It].NName );
            }
        }
        else
        {
            m_ItTimer -= DeltaTime;
        }
    }

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

void logic_tag::Connect( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    GameMgr.m_Score.Player[PlayerIndex].Team = PlayerIndex;

    // Dirty bits.
    GameMgr.m_PlayerBits |= (1 << PlayerIndex);
}

//==============================================================================

xbool logic_tag::IsTeamBased( void )
{
    return( FALSE );
}

//==============================================================================

void logic_tag::Disconnect( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    // Dirty bits.
    GameMgr.m_PlayerBits |= (1 << PlayerIndex);
}

//==============================================================================

void logic_tag::EnterGame( s32 PlayerIndex )
{
    m_RespawnDelay[PlayerIndex] = 0.0f;
    m_Alive       [PlayerIndex] = FALSE;
}

//==============================================================================

void logic_tag::ExitGame( s32 PlayerIndex )
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

u32 logic_tag::GetTeamBits( s32 PlayerIndex )
{
    if( PlayerIndex == m_It )
        return( 0x00000002 );
    else
        return( 0x00000001 );
}

//==============================================================================
