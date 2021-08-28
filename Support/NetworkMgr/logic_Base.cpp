//==============================================================================
//
//  logic_Base.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
//    INCLUDES
//==============================================================================

#include "logic_Base.hpp"
#include "NetworkMgr.hpp"
#include "Objects/Player.hpp"               // For class actor/player.
#include "CollisionMgr\CollisionMgr.hpp"

#include "MsgMgr.hpp"

//==============================================================================
//    FUNCTIONS
//==============================================================================

logic_base::logic_base( void )
{
}

//==============================================================================

logic_base::~logic_base( void )
{
}

//==============================================================================

f32 logic_base::GetGameProgress( void )
{
    return( -1.0f );
}

//==============================================================================

void logic_base::PlayerDied( s32 Victim, s32 Killer, s32 PainType )
{
    (void)Victim;
    (void)Killer;
    (void)PainType;
}

//==============================================================================

void logic_base::RequestSpawn( s32 PlayerIndex, xbool Immediate )
{
    (void)PlayerIndex;
    (void)Immediate;
}

//==============================================================================

u32 logic_base::GetTeamBits( s32 PlayerIndex )
{
    return( 1 << PlayerIndex );
}

//==============================================================================

s32 logic_base::BitsToTeam( u32 TeamBits )
{
    if( TeamBits == 0x00000000 )
    {
        return( -1 );
    }

    s32 Team = 0;
    while( (TeamBits) && ((TeamBits & 0x01) == 0) )
    {
        TeamBits >>= 1;
        Team      += 1;
    }

    return( Team );
}

//==============================================================================

u32 logic_base::GetScoreFields( void )
{
    u32 Fields = SCORE_POINTS |
                 SCORE_KILLS  |
                 SCORE_DEATHS;
    
    for( s32 i = 0; i < 32; i++ )
    {
        if( ( GameMgr.m_Score.Player[i].IsInGame) &&
            (!GameMgr.m_Score.Player[i].IsBot) )
        {
            if( GameMgr.m_Score.Player[i].Votes > 0 )
            {
                Fields |= SCORE_VOTES;
            }

            if( GameMgr.m_Score.Player[i].TKs > 0 )
            {
                Fields |= SCORE_TKS;
            }

            if( GameMgr.m_Score.Player[i].Flags > 0 )
            {
                Fields |= SCORE_FLAGS;
            }
        }
    }

    return( Fields );
}

//==============================================================================

void logic_base::FlagTouched( s32 PlayerIndex, s32 FlagNetSlot )
{
    (void)PlayerIndex;
    (void)FlagNetSlot;
}

//==============================================================================

void logic_base::FlagTimedOut( s32 FlagNetSlot )
{
    (void)FlagNetSlot;
}

//==============================================================================

void logic_base::CircuitChanged( s32 Circuit, u32 TeamBits )
{
    GameMgr.SetCircuitBits( Circuit, TeamBits );
}

//==============================================================================

void logic_base::SetPlayerSpawnInfo( const vector3& Position, 
                                           radian   Pitch, 
                                           radian   Yaw, 
                                           s32      Zone, 
                                     const guid&    Guid )
{ 
    (void)Position; 
    (void)Pitch; 
    (void)Yaw; 
    (void)Zone; 
    (void)Guid; 
}

//==============================================================================

void logic_base::Initialize( void )
{
}

//==============================================================================

void logic_base::Connect( s32 PlayerIndex )
{
    (void)PlayerIndex;
}

//==============================================================================

void logic_base::EnterGame( s32 PlayerIndex )
{
    (void)PlayerIndex;
}

//==============================================================================

void logic_base::ExitGame( s32 PlayerIndex )
{
    (void)PlayerIndex;
}

//==============================================================================

void logic_base::Disconnect( s32 PlayerIndex )
{
    (void)PlayerIndex;
}

//==============================================================================

void logic_base::ChangeTeam( s32 PlayerIndex )
{
    (void)PlayerIndex;
}

//==============================================================================

void logic_base::AdvanceTime( f32 DeltaTime )
{
    (void)DeltaTime;
}

//==============================================================================

void logic_base::BeginGame( void )
{
    GameMgr.SetScoreDisplay( SCORE_DISPLAY_NONE );
}

//==============================================================================

void logic_base::EndGame( xbool Complete )
{
    if( g_NetworkMgr.IsClient() )
        return;

    if( !Complete )
        return;

    // At this time, the game is over and the final score has been established.
    // We need to tweak some of the values in the final score for stats.
    ASSERT( !GameMgr.m_GameInProgress );

    game_score& Score = GameMgr.m_FinalScore;

    // Everybody gets a completed game.
    for( s32 i = 0; i < 32; i++ )
        if( Score.Player[i].IsInGame )
            Score.Player[i].Games = 1;

    // Award Gold/Silver/Bronze medals.
    {
        // We give out Gold/Silver/Bronze to the top 3 scoring players.  
        // But!  We need to be careful to prevent abuse.  Only give out medals
        // if there are at least 3 players who rank below 3rd.  For example:
        //  - Only 4 players in the game?  No medals.
        //  - 7 players in the game, but all have the same score?  No medals.
        
        s32 BelowThird = 0;

        for( s32 i = 0; i < 32; i++ )
        {
            if( (Score.Player[i].IsInGame) && 
                (Score.Player[i].Rank > 3) )
            {
                BelowThird += 1;
            }
        }

        if( BelowThird >= 3 )
        {
            for( s32 i = 0; i < 32; i++ )
            {
                if( Score.Player[i].IsInGame )
                {
                    if( Score.Player[i].Rank == 1 )  Score.Player[i].Gold   = 1;
                    if( Score.Player[i].Rank == 2 )  Score.Player[i].Silver = 1;
                    if( Score.Player[i].Rank == 3 )  Score.Player[i].Bronze = 1;
                }
            }
        }
    }

    // Award team based Wins.
    if( Score.IsTeamBased )
    {
        // In team games, each player on the winning team gets a "Win", but only
        // if he has been on the team for at least 4 minutes.

        s32 T = -1;
        if( Score.Team[0].Score > Score.Team[1].Score )  T = 0;
        if( Score.Team[1].Score > Score.Team[0].Score )  T = 1;

        if( T == -1 ) 
            return; // Tie game.

        for( s32 i = 0; i < 32; i++ )
        {
            if( (Score.Player[i].IsInGame) &&
                (Score.Player[i].Team == T) &&
                (Score.Player[i].TeamTime > 240.0f) )
            {
                Score.Player[i].Wins = 1;
            }
        }
    }
}

//==============================================================================

xbool logic_base::IsTeamBased( void )
{
    return( FALSE );
}

//==============================================================================

void logic_base::UpdateMusic( void )
{
    s32 MusicLevel = (s32)(GameMgr.GetGameProgress() * 5) + 1;
    if( MusicLevel > 5 )  MusicLevel = 5;

    for( s32 i = 0; i < NET_MAX_PLAYERS; i++ )
    {
        player_score& Player = GameMgr.m_Score.Player[i];
        if( Player.IsInGame )
        {
            if( MusicLevel != Player.MusicLevel )
            {
                Player.MusicLevel    = MusicLevel;
                GameMgr.m_ScoreBits |= (1 << i);
            }
        }
    }
}

//==============================================================================

void logic_base::Activate( void )
{
    if( !g_NetworkMgr.IsServer() )
        return;

    if( IsTeamBased() )
    {
        if( !GameMgr.m_Score.IsTeamBased )
            Teamify();
    }
    else
    {
        Unteamify();
    }
}

//==============================================================================

void logic_base::Teamify( void )
{
    if( !g_NetworkMgr.IsServer() )
        return;

    s32 T = 0;

    GameMgr.m_Score.Team[0].Size = 0;
    GameMgr.m_Score.Team[1].Size = 0;

    for( s32 i = 0; i < 32; i++ )
    {
        player_score& Player = GameMgr.m_Score.Player[i];
        if( Player.IsConnected )
        {
            Player.Team                   = T;
            GameMgr.m_Score.Team[T].Size += 1;
            T = 1-T;
        }
    }
}

//==============================================================================

void logic_base::Unteamify( void )
{
    if( !g_NetworkMgr.IsServer() )
        return;

    GameMgr.m_Score.Team[0].Size = 0;
    GameMgr.m_Score.Team[1].Size = 0;

    for( s32 i = 0; i < 32; i++ )
    {
        player_score& Player = GameMgr.m_Score.Player[i];
        if( Player.IsConnected )
        {
            Player.Team = i;
        }
    }
}

//==============================================================================

void logic_base::SetTeam( s32 PlayerIndex, s32 Team )
{
    GameMgr.m_Score.Player[PlayerIndex].Team = Team;

    actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
    if( pActor )
    {
        pActor->net_SetTeamBits( 1 << Team );
        GameMgr.m_PlayerBits |= (1 << PlayerIndex);
    }
}

//==============================================================================

s32 logic_base::GetTeam( s32 PlayerIndex )
{
    return( GameMgr.m_Score.Player[PlayerIndex].Team );
}

//==============================================================================

void logic_base::SetMutationRights( s32 PlayerIndex )
{
    actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
    ASSERT( pActor );

    switch( GameMgr.m_MutationMode )
    {
    default:
        ASSERT( FALSE );
        // fall through

    case MUTATE_CHANGE_AT_WILL:
        // Always start off as a human.
        pActor->ForceMutationChange ( FALSE );
        pActor->SetCanToggleMutation( TRUE  );
        pActor->SetMutagenBurnMode  ( actor::MBM_AT_WILL );
        break;

    case MUTATE_HUMAN_LOCK:
        pActor->ForceMutationChange ( FALSE );
        pActor->SetCanToggleMutation( FALSE );
        pActor->SetMutagenBurnMode  ( actor::MBM_AT_WILL  );
        break;

    case MUTATE_MUTANT_LOCK:
        pActor->ForceMutationChange ( TRUE  );
        pActor->SetCanToggleMutation( FALSE );
        pActor->SetMutagenBurnMode  ( actor::MBM_FORCED );
        break;

    case MUTATE_HUMAN_VS_MUTANT:
        pActor->ForceMutationChange ( (GameMgr.m_Score.Player[PlayerIndex].Team == 1) );
        pActor->SetCanToggleMutation( FALSE );
        pActor->SetMutagenBurnMode  ( actor::MBM_FORCED );
        break;

    case MUTATE_MAN_HUNT:
        pActor->ForceMutationChange ( (GameMgr.m_Score.Player[PlayerIndex].Team == 0) );
        pActor->SetCanToggleMutation( FALSE );
        pActor->SetMutagenBurnMode  ( actor::MBM_FORCED );
        break;

    case MUTATE_MUTANT_HUNT:
        pActor->ForceMutationChange ( (GameMgr.m_Score.Player[PlayerIndex].Team == 1) );
        pActor->SetCanToggleMutation( FALSE );
        pActor->SetMutagenBurnMode  ( actor::MBM_FORCED );
        break;
    }
}

//==============================================================================

void logic_base::DropFlag( s32 PlayerIndex )
{
    (void)PlayerIndex;
}

//==============================================================================

void logic_base::MoveToSpawnPt( s32 PlayerIndex, spawn_point& SpawnPt )
{
    // Lookup actor requesting the spawn
    actor*  pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
    ASSERT( pActor );

    //  Lookup spawn point info
    vector3 Position;
    radian3 Rotation;
    u16     Zone1;
    u16     Zone2;
    SpawnPt.GetSpawnInfo( pActor->GetGuid(), Position, Rotation, Zone1, Zone2 );

    // Teleport the actor
    pActor->Teleport( Position, FALSE );
    pActor->SetPitch( Rotation.Pitch );
    pActor->SetYaw  ( Rotation.Yaw   ); 
    pActor->SetZone1( Zone1 );
    pActor->SetZone2( Zone2 );
    pActor->InitZoneTracking();
}

//==============================================================================

void logic_base::NonTeamScoreMsgPrep( void )
{
    m_LeadBefore  = 0x00000000;
    m_NLeadBefore = 0;

    for( s32 i = 0; i < 32; i++ )
    {
        if( (GameMgr.m_Score.Player[i].IsInGame) && 
            (GameMgr.m_Score.Player[i].Rank == 1) &&
            (GameMgr.m_Score.Player[i].IsBot == FALSE) )
        {
            m_LeadBefore  |= (1 << i);
            m_NLeadBefore += 1;
        }
    }
}

//==============================================================================

void logic_base::NonTeamScoreMsgExec( s32 ScoringPlayer )
{
    // Make sure the score ranks are up to date.
    GameMgr.m_Score.ComputeRanks();

    u32 LeadAfter  = 0x00000000;
    s32 NLeadAfter = 0;

    for( s32 i = 0; i < 32; i++ )
    {
        if( (GameMgr.m_Score.Player[i].IsInGame) && 
            (GameMgr.m_Score.Player[i].Rank == 1) &&
            (GameMgr.m_Score.Player[i].IsBot == FALSE) )
        {
            LeadAfter  |= (1 << i);
            NLeadAfter += 1;
        }
    }

    //
    // Issue player specific "take the lead", "tied for lead", "lost the lead" 
    // messages.
    //

    if( g_NetworkMgr.GetLocalPlayerCount() <= 1 )
    {
        for( s32 i = 0; i < 32; i++ )
        {
            if( (GameMgr.m_Score.Player[i].IsInGame == FALSE) ||
                (GameMgr.m_Score.Player[i].IsBot) )
                continue;

            u32 Mask = (1 << i);

            // New single leader?
            //  - You are in the lead, "You take the lead."
            //  - You aren't in the lead, "Rico is in the lead."
            if( (m_NLeadBefore > 1) && (NLeadAfter == 1) )
            {
                if( GameMgr.m_Score.Player[i].Rank == 1 )
                {
                    MsgMgr.Message( MSG_YOU_TAKE_LEAD, i );
                }
                else
                {
                //  "Rico is in the lead."
                }
            }
                
            // New tie for the lead?
            //  - You were the single lead, but now "You are tied for the lead."
            if( (m_NLeadBefore == 1) && (NLeadAfter > 1) && (m_LeadBefore & Mask) )
            {
                MsgMgr.Message( MSG_YOU_TIE_LEAD, i );
            }

            // New tie for the lead?
            //  - You caught up to leader(s), and so "You are tied for the lead."
            if( (NLeadAfter > 1) && !(m_LeadBefore & Mask) && (LeadAfter & Mask) )
            {            
                MsgMgr.Message( MSG_YOU_TIE_LEAD, i );
            }

            // Old leader dethroned?
            if( (m_LeadBefore & Mask) && !(LeadAfter & Mask) && 
                (GameMgr.m_Score.Player[i].Score > 0) )
            {
                MsgMgr.Message( MSG_YOU_LOSE_LEAD, i );
            }
        }
    }

    //
    // Issue global "Rico is in the lead", "Rico is 3 points from victory"
    // messages.
    //

    if( ScoringPlayer != -1 )
    {  
        u32     Mask = (1 << ScoringPlayer);
        xbool   Skip = FALSE;   // Prevent sending two messages about leader.

        // If the player who scored is in the lead (tied or solo) and he is
        // within 5 points of winning, issue "Rico is 3 points from victory" to
        // everybody.

        s32 ScoreLimit = GameMgr.GetScoreLimit();
        s32 Score      = GameMgr.m_Score.Player[ScoringPlayer].Score;

        s32 Gap = (ScoreLimit - Score);

        if( (ScoreLimit > 0) && 
            (Mask & LeadAfter) && 
            (Gap <= 5) )
        {
            if( Gap == 1 )
                MsgMgr.Message( MSG_P_1_POINT, 0, ScoringPlayer ); 
            else if( Gap > 0 )
                MsgMgr.Message( MSG_P_N_POINTS, 0, ScoringPlayer, ScoreLimit - Score );

            Skip = TRUE;
        }

        // Issue audio "points remaining" messages to scoring player.
        if( (ScoreLimit > 0) && (Score > 5) && (Mask & LeadAfter) )
        {
            s32    Points = ScoreLimit - Score;
            msg_id Msg    = MSG_NULL;

            switch( Points )
            {
            case 50: Msg = MSG_POINTS_REMAINING_50; break;
            case 25: Msg = MSG_POINTS_REMAINING_25; break;
            case 10: Msg = MSG_POINTS_REMAINING_10; break;
            case  5: Msg = MSG_POINTS_REMAINING_05; break;
            case  4: Msg = MSG_POINTS_REMAINING_04; break;
            case  3: Msg = MSG_POINTS_REMAINING_03; break;
            case  2: Msg = MSG_POINTS_REMAINING_02; break;
            case  1: Msg = MSG_POINTS_REMAINING_01; break;
            default: break;
            }

            if( Msg != MSG_NULL )
                MsgMgr.Message( Msg, ScoringPlayer );
        }

        // If the player who scored is the single leader, issue a global "Rico 
        // is in the lead with 12 points" message, but only to a random 25% of 
        // the crowd.
        if( (!Skip) &&
            (Mask == LeadAfter) && 
            (GameMgr.m_Score.Player[ScoringPlayer].Score > 1) )
        {
            for( s32 i = 0; i < 32; i++ )
            {
                if( (GameMgr.m_Score.Player[i].IsInGame == FALSE) ||
                    (GameMgr.m_Score.Player[i].IsBot) ||
                    (x_irand(0,3)) )
                    continue;

                MsgMgr.Message( MSG_P_IN_LEAD, i, ScoringPlayer, ScoringPlayer );
            }
        }
    }
}

//==============================================================================
