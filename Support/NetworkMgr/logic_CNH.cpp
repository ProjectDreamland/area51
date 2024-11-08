//==============================================================================
//
//  logic_CNH.cpp
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

#include "logic_CNH.hpp"
#include "NetObjMgr.hpp"
#include "NetworkMgr.hpp"
#include "MsgMgr.hpp"
#include "PainMgr/PainTypes.hpp"

#include "Objects/CapPoint.hpp"
#include "Objects/GameProp.hpp"
#include "Objects/Player.hpp"               // For class actor/player.

//==============================================================================
//  TYPES
//==============================================================================

enum
{
    PLAYER_SCORE_KILL_ENEMY =  1,
    PLAYER_SCORE_KILL_ALLY  = -2,
    PLAYER_SCORE_KILL_SELF  = -1,
    PLAYER_SCORE_CAP_BONUS  =  1,
    PLAYER_SCORE_KILL_BONUS =  1,

    TEAM_SCORE_KILL_ENEMY   =  0,
    TEAM_SCORE_KILL_ALLY    =  0,
    TEAM_SCORE_KILL_SELF    =  0,
};

//==============================================================================
//    FUNCTIONS
//==============================================================================

logic_cnh::logic_cnh( void )
{
}

//==============================================================================

logic_cnh::~logic_cnh( void )
{
}

//==============================================================================

void logic_cnh::BeginGame( void )
{
    if( g_NetworkMgr.IsClient() )
        return;

    GameMgr.SetScoreDisplay( SCORE_DISPLAY_CNH );

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

    for( s32 i = 0; i < 16; i++ )
    {
        m_PreviousTeam[i] = -1;
    }

    // Create the cap points.

    g_ObjMgr.SelectByAttribute( object::ATTR_ALL, object::TYPE_GAME_PROP );
    select_slot_iterator SlotIter;

    for( SlotIter.Begin(); !SlotIter.AtEnd(); SlotIter.Next() )
    {
        game_prop* pProp = (game_prop*)SlotIter.Get();
        ASSERT( pProp );

        if( pProp->GetKind() == game_prop::CAPTURE_POINT )
        {
            cap_point* pCP = (cap_point*)NetObjMgr.CreateObject( netobj::TYPE_CAP_POINT );
            pCP->SetElevation( pProp->GetElevation() );
            pCP->SetRadius   ( pProp->GetRadius() );
            pCP->SetCircuit  ( pProp->GetCircuit().GetCircuit() );
            pCP->SetL2W      ( pProp->GetL2W() );
            pCP->SetZone1    ( pProp->GetZone1() );
            pCP->SetZone2    ( pProp->GetZone2() );
            pCP->Init();

            ASSERTS( pProp->GetCircuit().GetCircuit() > 3, "CNH points must be in a circuit > 3!" );

            // So that maps like Two to Tango which start with CNH points in 
            // team possession generate the correct messages.
            m_PreviousTeam[ pProp->GetCircuit().GetCircuit() ] = pProp->GetCircuit().GetTeamBits() - 1;
            ASSERT( IN_RANGE( -1, m_PreviousTeam[ pProp->GetCircuit().GetCircuit() ], 1 ) ); 

            NetObjMgr.AddObject( pCP );
        }
    }
    SlotIter.End();
}

//==============================================================================

void logic_cnh::CircuitChanged( s32 Circuit, u32 TeamBits )
{
    s32 NewTeam = BitsToTeam( TeamBits );

    // Capture a point not previously held?
    if( (NewTeam != -1) &&
        (m_PreviousTeam[Circuit] == -1) )
    {
        ASSERT( IN_RANGE( 0, NewTeam, 1 ) );

        // For the capturing team...
        MsgMgr.Message( MSG_ALLIED_CAPTURE_SECTOR, NewTeam );

        // For the other team...
        MsgMgr.Message( MSG_ENEMY_CAPTURE_SECTOR, 1-NewTeam );

        // Bonus points!
        CaptureBonus( Circuit, NewTeam, TRUE );
    }

    // Neutralization?
    if( (NewTeam == -1) )
    {
        ASSERT( IN_RANGE( 0, m_PreviousTeam[Circuit], 1 ) );

        // For the previously owning team...
        MsgMgr.Message( MSG_SECTOR_ATTACKED, m_PreviousTeam[Circuit] );

        // For the other team...
        MsgMgr.Message( MSG_SECTOR_NEUTRALIZED, 1-m_PreviousTeam[Circuit] );
    }

    // Retake point after neutralization?
    if( (NewTeam != -1) &&
        (m_PreviousTeam[Circuit] != -1) &&
        (m_PreviousTeam[Circuit] == NewTeam) )
    {
        // For the capturing team...
        MsgMgr.Message( MSG_SECTOR_SECURE, NewTeam );

        // For the other team...
        // Nothing.

        // Bonus points!
        CaptureBonus( Circuit, NewTeam, FALSE );
    }

    // Change of ownership after neutralization?
    if( (NewTeam != -1) &&
        (m_PreviousTeam[Circuit] != -1) &&
        (m_PreviousTeam[Circuit] != NewTeam) )
    {
        ASSERT( IN_RANGE( 0, NewTeam, 1 ) );

        // For the capturing team...
        MsgMgr.Message( MSG_SECTOR_CAPTURED, NewTeam );

        // For the other team...
        MsgMgr.Message( MSG_SECTOR_LOST, 1-NewTeam );

        // Bonus points!
        CaptureBonus( Circuit, NewTeam, TRUE );
    }

    // Record the new owner.
    if( NewTeam != -1 )
        m_PreviousTeam[Circuit] = NewTeam;
      
    // Run base logic.
    logic_base::CircuitChanged( Circuit, TeamBits );
}

//==============================================================================

void logic_cnh::CaptureBonus( s32 Circuit, s32 Team, xbool Offense )
{
    // All players influencing the given point/circuit on the given team are
    // awarded a bonus point using either an offense or defense message.

    cap_point* pCP  = NULL;
    slot_id    Slot = g_ObjMgr.GetFirst( object::TYPE_CAP_POINT );
    
    while( Slot != SLOT_NULL )
    {
        pCP = (cap_point*)g_ObjMgr.GetObjectBySlot( Slot );

        if( pCP->GetCircuit() == Circuit )
        {
            for( s32 i = 0; i < 32; i++ )
            {
                if( (GameMgr.m_Score.Player[i].IsInGame) && 
                    (GameMgr.m_Score.Player[i].Team == Team) &&
                    (pCP->PlayerInfluence( i )) )
                {
                    GameMgr.m_Score.Player[i].Score += PLAYER_SCORE_CAP_BONUS;
                    GameMgr.m_ScoreBits |= (1 << i);
                    
                    if( Offense )
                        MsgMgr.Message( MSG_OFFENSE_BONUS, i, 1 );
                    else
                        MsgMgr.Message( MSG_DEFENSE_BONUS, i, 1 );
                }
            }

            break;
        }

        Slot = g_ObjMgr.GetNext( Slot );
    }
}

//==============================================================================

void logic_cnh::RequestSpawn( s32 PlayerIndex, xbool Immediate )
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

void logic_cnh::PlayerDied( s32 Victim, s32 Killer, s32 PainType )
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
                GameMgr.m_ScoreBits |= (1 << Killer);

                // Give the killer an extra point per capture point influence.
                cap_point* pCP  = NULL;
                slot_id    Slot = g_ObjMgr.GetFirst( object::TYPE_CAP_POINT );
                
                while( Slot != SLOT_NULL )
                {
                    pCP = (cap_point*)g_ObjMgr.GetObjectBySlot( Slot );
                    if( pCP && pCP->PlayerInfluence( Victim ) )
                    {
                        GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_BONUS;
                        GameMgr.m_ScoreBits |= (1 << Killer);

                        s32 C = pCP->GetCircuit();
                        if( m_PreviousTeam[C] == KTeam )
                            MsgMgr.Message( MSG_DEFENSE_BONUS, Killer, 1 );
                        else
                            MsgMgr.Message( MSG_OFFENSE_BONUS, Killer, 1 );
                    }

                    Slot = g_ObjMgr.GetNext( Slot );
                }
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

void logic_cnh::AdvanceTime( f32 DeltaTime )
{
    if( g_NetworkMgr.IsClient() )
        return;

    cap_point* pCP  = NULL;
    slot_id    Slot = g_ObjMgr.GetFirst( object::TYPE_CAP_POINT );
    
    while( Slot != SLOT_NULL )
    {
        pCP = (cap_point*)g_ObjMgr.GetObjectBySlot( Slot );
        if( pCP )
        {
            s32 Award = pCP->GetAward();
            if( Award < 0 )
            {
                GameMgr.m_Score.Team[0].Score -= Award;
                GameMgr.m_DirtyBits |= game_mgr::DIRTY_TEAM_SCORES;
            }
            if( Award > 0 )
            {
                GameMgr.m_Score.Team[1].Score += Award;
                GameMgr.m_DirtyBits |= game_mgr::DIRTY_TEAM_SCORES;
            }
        }

        Slot = g_ObjMgr.GetNext( Slot );
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

void logic_cnh::Connect( s32 PlayerIndex )
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

xbool logic_cnh::IsTeamBased( void )
{
    return( TRUE );
}

//==============================================================================

void logic_cnh::Disconnect( s32 PlayerIndex )
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

void logic_cnh::EnterGame( s32 PlayerIndex )
{
    m_RespawnDelay[PlayerIndex] = 0.0f;
    m_Alive       [PlayerIndex] = FALSE;
}

//==============================================================================

void logic_cnh::ExitGame( s32 PlayerIndex )
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

u32 logic_cnh::GetTeamBits( s32 PlayerIndex )
{
    return( 1 << GameMgr.m_Score.Player[PlayerIndex].Team );
}

//==============================================================================
