//==============================================================================
//
//  logic_CTF.cpp
//
//==============================================================================
/*

NOTES:

  - Consider allowing the flags to resist being picked up by the same player
    that most recently dropped it for a handful of seconds.  Objective is to 
    prevent people from dropping and the picking back up again.  From that 
    player's point of view, perhaps the flag should render differently when
    it can't be taken.  (What about two players tossing the flag back and 
    forth?)
           
*/
//==============================================================================

//==============================================================================
//    INCLUDES
//==============================================================================

#include "logic_CTF.hpp"
#include "NetworkMgr.hpp"
#include "NetObjMgr.hpp"
#include "MsgMgr.hpp"
#include "PainMgr/PainTypes.hpp"

#include "Objects/Flag.hpp"
#include "Objects/FlagBase.hpp"
#include "Objects/GameProp.hpp"
#include "Objects/Player.hpp"               // For class actor/player.

//==============================================================================
//  TYPES
//==============================================================================

enum
{
    TEAM_SCORE_FLAG_CAP      =  1,

    PLAYER_SCORE_FLAG_CAP    = 10,
    PLAYER_SCORE_FLAG_TAKE   =  5,
    PLAYER_SCORE_FLAG_RETURN =  5,
    PLAYER_SCORE_FLAG_DEFEND =  5,

    PLAYER_SCORE_KILL_ENEMY  =  1,
    PLAYER_SCORE_KILL_ALLY   = -2,
    PLAYER_SCORE_KILL_SELF   = -1,
};

//==============================================================================
//    FUNCTIONS
//==============================================================================

logic_ctf::logic_ctf( void )
{
}

//==============================================================================

logic_ctf::~logic_ctf( void )
{
}

//==============================================================================

void logic_ctf::DropFlag(       s32      FlagIndex,
                                s32      PlayerIndex,
                          const vector3& Position,
                                radian   Yaw,
                                s32      Zone1,
                                s32      Zone2 )
{
    flag* pFlag = (flag*)NetObjMgr.GetObjFromSlot( m_FlagNetSlot[ FlagIndex ] );

    ASSERT( pFlag );
    ASSERT( IN_RANGE( 0, FlagIndex, 1 ) );

    // Update the player highlight.
    GameMgr.m_Score.HighLightMask &= ~(1 << m_FlagCarrier[ FlagIndex ]);
    GameMgr.m_DirtyBits           |= game_mgr::DIRTY_HIGHLIGHT;

    pFlag->SetTimeOut ( 15.0f, 0.0f );
    pFlag->SetYaw     ( Yaw );
    pFlag->SetPosition( Position, Zone1, Zone2 );

    if( PlayerIndex != -1 )
        pFlag->AddIgnore( PlayerIndex );

    m_FlagDropped[ FlagIndex ] = TRUE;
    m_FlagCarrier[ FlagIndex ] = -1;

    if( PlayerIndex != -1 )
    {
        MsgMgr.Message( MSG_P_DROPPED_FLAG_GOOD,   FlagIndex, PlayerIndex,   FlagIndex );
        MsgMgr.Message( MSG_P_DROPPED_FLAG_BAD , 1-FlagIndex, PlayerIndex,   FlagIndex );
    }

    pFlag->StartFall();
}

//==============================================================================

void logic_ctf::RequestSpawn( s32 PlayerIndex, xbool Immediate )
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

void logic_ctf::PlayerDied( s32 Victim, s32 Killer, s32 PainType )
{
    ASSERT( g_NetworkMgr.IsServer() );

    (void)PainType;

    // TO DO - Check the logic in here.  Twice death?  Client?
    // TO DO - Check it in the other game types, too.

    // The player COULD ALREADY BE DEAD if killed on the server and a client
    // at approximately the same time since the client kill has latency.

    if( m_Alive[Victim] )
    {
        m_Alive       [Victim] = FALSE;
        m_RespawnDelay[Victim] = 3.0f;

        if( PainType != SERVER_MAINTENANCE )
        {
            //
            // Messages and score.
            //
            MsgMgr.PlayerDied( Victim, Killer, PainType );

            s32 VTeam = GameMgr.m_Score.Player[Victim].Team;

            if( (Killer == -1) || (Killer == Victim) )
            {
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
                    GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_KILL_ENEMY;
                    GameMgr.m_Score.Player[Killer].Kills += 1;
                }

                // Kill ally?
                if( (Killer != Victim) && (KTeam == VTeam) )
                {
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
        }
    }
    
    // If the player had the flag, drop it!

    s32 FlagIndex = -1;

    if( m_FlagCarrier[0] == Victim )  FlagIndex = 0;
    if( m_FlagCarrier[1] == Victim )  FlagIndex = 1;

    if( FlagIndex != -1 )
    {
        actor* pVictim = (actor*)NetObjMgr.GetObjFromSlot( Victim );
        ASSERT( pVictim );

        if( (Killer != -1) && 
            (GameMgr.m_Score.Player[ Victim ].Team !=
             GameMgr.m_Score.Player[ Killer ].Team) )
        {
            MsgMgr.Message( MSG_KILL_CARRIER_BONUS, Killer, PLAYER_SCORE_FLAG_DEFEND );
            GameMgr.m_Score.Player[Killer].Score += PLAYER_SCORE_FLAG_DEFEND;
            GameMgr.m_ScoreBits |= (1 << Killer);
        }

        DropFlag( FlagIndex,
                  Victim,
                  pVictim->GetPosition(), 
                  pVictim->GetYaw(), 
                  pVictim->GetZone1(), 
                  pVictim->GetZone2() );
    }
}

//==============================================================================

void logic_ctf::AdvanceTime( f32 DeltaTime )
{
    for( s32 i = 0; i < 32; i++ )
    {
        if( m_RespawnDelay[i] > 0.0f )
        {
            m_RespawnDelay[i] -= DeltaTime;
        }
        else
        {
            m_RespawnDelay[i] = 0.0f;
        }
    }
}

//==============================================================================

void logic_ctf::Connect( s32 PlayerIndex )
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

void logic_ctf::Disconnect( s32 PlayerIndex )
{
    if( g_NetworkMgr.IsClient() )
        return;

    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    s32 Team = GameMgr.m_Score.Player[PlayerIndex].Team;

    GameMgr.m_Score.Team[Team].Size--;

    // Dirty bits.
    GameMgr.m_DirtyBits  |= game_mgr::DIRTY_TEAM_SCORES;
    GameMgr.m_PlayerBits |= (1 << PlayerIndex);
}

//==============================================================================

void logic_ctf::EnterGame( s32 PlayerIndex )
{
    m_RespawnDelay[PlayerIndex] = 0.0f;
    m_Alive       [PlayerIndex] = FALSE;
}

//==============================================================================

void logic_ctf::ExitGame( s32 PlayerIndex )
{
    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );

    if( g_NetworkMgr.IsClient() )
        return;

    if( !GameMgr.m_GameInProgress )
        return;

    // If the player has the flag, drop it.

    s32 FlagIndex = -1;

    if( m_FlagCarrier[0] == PlayerIndex )  FlagIndex = 0;
    if( m_FlagCarrier[1] == PlayerIndex )  FlagIndex = 1;

    if( FlagIndex != -1 )
    {          
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
        ASSERT( pActor );

        DropFlag( FlagIndex,
                  PlayerIndex,
                  pActor->GetPosition(), 
                  pActor->GetYaw(), 
                  pActor->GetZone1(), 
                  pActor->GetZone2() );
    }

    // Kill the player, and reset everything as needed.

    if( m_Alive[PlayerIndex] )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
        ASSERT( pActor );
        pActor->OnDeath();
    }

    m_RespawnDelay[PlayerIndex] = 0.0f;
    m_Alive       [PlayerIndex] = FALSE;
}

//==============================================================================

xbool logic_ctf::IsTeamBased( void )
{
    return( TRUE );
}

//==============================================================================

u32 logic_ctf::GetTeamBits( s32 PlayerIndex )
{
    return( 1 << GameMgr.m_Score.Player[PlayerIndex].Team );
}

//==============================================================================

void logic_ctf::FlagTouched( s32 PlayerIndex, s32 FlagNetSlot )
{
    if( g_NetworkMgr.IsClient() )
        return;

    flag*       pFlag       = (flag *)NetObjMgr.GetObjFromSlot( FlagNetSlot );
    actor*      pActor      = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );

    if( pActor->IsDead() )
        return;

    u32         FlagBits    = pFlag ->net_GetTeamBits();
    u32         PlayerBits  = pActor->net_GetTeamBits();

    s32         FlagTeam    = BitsToTeam( FlagBits );
    s32         PlayerTeam  = BitsToTeam( PlayerBits );

    ASSERT( pFlag  );
    ASSERT( pActor );

    ASSERT( IN_RANGE( 0, PlayerIndex, 31 ) );
    ASSERT( IN_RANGE( 0, FlagTeam,     1 ) );
    ASSERT( IN_RANGE( 0, PlayerTeam,   1 ) );

    ASSERT( m_FlagSecure[FlagTeam] || m_FlagDropped[FlagTeam] );

    // Captured the flag?
    if( (FlagTeam == PlayerTeam) &&
        (m_FlagSecure [   FlagTeam ]) &&
        (m_FlagCarrier[ 1-FlagTeam ] == PlayerIndex) )
    {
        // Play a captured effect on the touched flag.
        pFlag->PlayEffect( FLAG_CAPTURE );
        
        // Take the flag off the player's back and send it back to the base.

        // We aren't really interested in the flag that was touched.  Instead,
        // we are going to work with the flag the player was carrying.  So,
        // to make life a little easier, change the variables from "the flag
        // that was touched" to "the flag that was captured".  Since we have
        // changed these variables, we MUST RETURN DIRECTLY FROM THIS BLOCK.

        FlagTeam = 1 - FlagTeam;
        pFlag    = (flag*)NetObjMgr.GetObjFromSlot( m_FlagNetSlot[ FlagTeam ] );

        ASSERT( pFlag );

        // Update the player highlight.
        GameMgr.m_Score.HighLightMask &= ~(1 << m_FlagCarrier[ FlagTeam ]);
        GameMgr.m_DirtyBits           |= game_mgr::DIRTY_HIGHLIGHT;

        pFlag->PlayEffect( FLAG_TELEPORT );

        pFlag->ClearTimeOut( );
        pFlag->SetYaw      ( m_FlagYaw  [ FlagTeam ] );
        pFlag->SetPosition ( m_FlagBase [ FlagTeam ], 
                             m_FlagZones[ FlagTeam ] >> 16,
                             m_FlagZones[ FlagTeam ]  & 0x0000FFFF );
        pFlag->InitCloth();
                           
        m_FlagSecure [ FlagTeam ] = TRUE;
        m_FlagDropped[ FlagTeam ] = FALSE;
        m_FlagCarrier[ FlagTeam ] = -1;

        // Update waypoint.

        // Score.
//      GameMgr.ScoreForPlayer( PlayerID, SCORE_CAPTURE_FLAG_PLAYER, MSG_SCORE_CTF_CAP_FLAG );
        GameMgr.m_Score.Team  [ PlayerTeam  ].Score += TEAM_SCORE_FLAG_CAP;
        GameMgr.m_Score.Player[ PlayerIndex ].Score += PLAYER_SCORE_FLAG_CAP;
        GameMgr.m_Score.Player[ PlayerIndex ].Flags += 1;
        GameMgr.m_DirtyBits |= game_mgr::DIRTY_TEAM_SCORES;
        GameMgr.m_ScoreBits |= (1 << PlayerIndex);

        // Messages.
        MsgMgr.Message( MSG_P_CAPTURED_FLAG_BAD ,   FlagTeam, PlayerIndex, FlagTeam );
        MsgMgr.Message( MSG_P_CAPTURED_FLAG_GOOD, 1-FlagTeam, PlayerIndex, FlagTeam );
        MsgMgr.Message( MSG_CAPTURE_FLAG_BONUS, PlayerIndex, PLAYER_SCORE_FLAG_CAP );
    //  MsgMgr.Message( MSG_CTF_TEAM_SCORES, 0, ARG_TEAM, PlayerTeam );
    //  MsgMgr.Message( MSG_CTF_TEAM_CAP,    PlayerTeam );
    //  MsgMgr.Message( MSG_CTF_ENEMY_CAP, 1-PlayerTeam );

        // Chatter?

        // DO NOT REMOVE THIS RETURN STATEMENT!  
        // READ COMMENTS AT THE TOP OF THIS CODE BLOCK.
        return;
    }

    // Returned a dropped flag?
    if( (FlagTeam == PlayerTeam) && (m_FlagDropped[ FlagTeam ]) )
    {
        pFlag->ClearTimeOut( );
        pFlag->SetYaw      ( m_FlagYaw  [ FlagTeam ] );
        pFlag->SetPosition ( m_FlagBase [ FlagTeam ], 
                             m_FlagZones[ FlagTeam ] >> 16,
                             m_FlagZones[ FlagTeam ]  & 0x0000FFFF );
        pFlag->InitCloth();

        m_FlagSecure [ FlagTeam ] = TRUE;
        m_FlagDropped[ FlagTeam ] = FALSE;
        m_FlagCarrier[ FlagTeam ] = -1;

        // Update the way point.
        
        // Give points for the return.
    //  GameMgr.ScoreForPlayer( PlayerID, SCORE_RETURN_FLAG, MSG_SCORE_CTF_RETURN_FLAG );
        MsgMgr.Message( MSG_P_RETURNED_FLAG_GOOD,   FlagTeam, PlayerIndex,   FlagTeam );
        MsgMgr.Message( MSG_P_RETURNED_FLAG_BAD , 1-FlagTeam, PlayerIndex,   FlagTeam );
        MsgMgr.Message( MSG_RETURN_FLAG_BONUS, PlayerIndex, PLAYER_SCORE_FLAG_RETURN );            

        GameMgr.m_Score.Player[ PlayerIndex ].Score += PLAYER_SCORE_FLAG_RETURN;
        GameMgr.m_ScoreBits |= (1 << PlayerIndex);

        pFlag->PlayEffect( FLAG_TELEPORT );

    //  FlagReturned( FlagTeam );

        return;
    }

    // Take a dropped flag or from a base?
    if( FlagTeam != PlayerTeam )
    {
        // Messages.
        for( s32 i = 0; i < 32; i++ )
        {
            if( !GameMgr.m_Score.Player[i].IsInGame || GameMgr.m_Score.Player[i].IsBot )
                continue;

            if( PlayerIndex == i )
            {
                MsgMgr.Message( MSG_YOU_HAVE_ENEMY_FLAG, i );
                continue;
            }
            
            if( GameMgr.m_Score.Player[i].Team == FlagTeam )
            {
                MsgMgr.Message( m_FlagSecure[FlagTeam] 
                                    ? MSG_P_TOOK_FLAG_BAD 
                                    : MSG_P_TOOK_FLAG_BAD_2, 
                                i, PlayerIndex, FlagTeam );        
            }
            else
            {
                MsgMgr.Message( m_FlagSecure[FlagTeam] 
                                    ? MSG_P_TOOK_FLAG_GOOD 
                                    : MSG_P_TOOK_FLAG_GOOD_2, 
                                i, PlayerIndex, FlagTeam );        
            }
        }           

        if( m_FlagSecure[ FlagTeam ] )
        {
            // Flag was secure, give points to the player.
            // TO DO 
            //  - Points
            //  - Message
            //  - Chatter?

            GameMgr.m_Score.Player[ PlayerIndex ].Score += PLAYER_SCORE_FLAG_TAKE;
            GameMgr.m_ScoreBits |= (1 << PlayerIndex);

            MsgMgr.Message( MSG_TAKE_FLAG_BONUS, PlayerIndex, PLAYER_SCORE_FLAG_TAKE );            
        }

        // Attach the flag to the player.
        pFlag->AttachToPlayer( PlayerIndex );
        pFlag->ClearTimeOut  ( );

        // Assign the flag to the player.
        m_FlagSecure [ FlagTeam ] = FALSE;
        m_FlagDropped[ FlagTeam ] = FALSE;
        m_FlagCarrier[ FlagTeam ] = PlayerIndex;

        // TO DO - pActor->AttachFlag( ? );
        // TO DO - pActor->SetFlagCount( 1 );

        // Update the way point.

        // Update the player highlight.
        GameMgr.m_Score.HighLightMask |= (1 << PlayerIndex);
        GameMgr.m_DirtyBits           |= game_mgr::DIRTY_HIGHLIGHT;

        // Dirty bits.
    //  GameMgr.m_DirtyBits |= game_mgr::DIRTY_TEAM_SCORES;

        return;
    }
}

//==============================================================================

void logic_ctf::FlagTimedOut( s32 FlagNetSlot )
{
    if( g_NetworkMgr.IsClient() )
        return;

    flag* pFlag    = (flag*)NetObjMgr.GetObjFromSlot( FlagNetSlot );
    u32   FlagBits = pFlag->net_GetTeamBits();
    s32   FlagTeam = BitsToTeam( FlagBits );

    pFlag->PlayEffect( FLAG_TELEPORT );

    ASSERT( pFlag );
    ASSERT( IN_RANGE( 0, FlagTeam, 1 ) );

    // Teleport the flag back to the base.

    pFlag->ClearTimeOut( );
    pFlag->SetYaw      ( m_FlagYaw [ FlagTeam ] );
    pFlag->SetPosition ( m_FlagBase[ FlagTeam ],
                         m_FlagZones[ FlagTeam ] >> 16,
                         m_FlagZones[ FlagTeam ]  & 0x0000FFFF );

    m_FlagSecure [ FlagTeam ] = TRUE;
    m_FlagDropped[ FlagTeam ] = FALSE;
    m_FlagCarrier[ FlagTeam ] = -1;

    // Update the way point.

    // Message.
    MsgMgr.Message( MSG_ALLIED_FLAG_RETURNED,  FlagTeam );
    MsgMgr.Message( MSG_ENEMY_FLAG_RETURNED, 1-FlagTeam );

    // Dirty bits.
}

//==============================================================================

void logic_ctf::BeginGame( void )
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

    #ifdef X_ASSERT
    u32 Sanity = 0;
    #endif

    // Create the flags on the flag bases.
    
    // Create waypoints.

    // Locate game_prop objects.
    // Create flags as appropriate.

    g_ObjMgr.SelectByAttribute( object::ATTR_ALL, object::TYPE_GAME_PROP );
    select_slot_iterator SlotIter;

    for( SlotIter.Begin(); !SlotIter.AtEnd(); SlotIter.Next() )
    {
        game_prop* pProp = (game_prop*)SlotIter.Get();
        ASSERT( pProp );

        if( pProp->GetKind() == game_prop::FLAG_BASE )
        {
            flag*      pFlag     = (flag*)     NetObjMgr.CreateObject( netobj::TYPE_FLAG );
            flag_base* pFlagBase = (flag_base*)NetObjMgr.CreateObject( netobj::TYPE_FLAG_BASE );
            s32        FlagTeam  = BitsToTeam( pProp->GetCircuit().GetTeamBits() );
            matrix4    L2W;

            ASSERT( pFlag );
            ASSERT( IN_RANGE( 0, FlagTeam, 1 ) );

            m_FlagBase [ FlagTeam ] = pProp->GetPosition();
            m_FlagYaw  [ FlagTeam ] = pProp->GetL2W().GetRotation().Yaw;
            m_FlagZones[ FlagTeam ] = (pProp->GetZone1() << 16) &
                                      (pProp->GetZone2()      );

            pFlag->ClearTimeOut( );
            pFlag->SetTeamBits ( 1 << FlagTeam );
            pFlag->SetYaw      ( m_FlagYaw [ FlagTeam ] );
            pFlag->SetPosition ( m_FlagBase[ FlagTeam ],
                                 m_FlagZones[ FlagTeam ] >> 16,
                                 m_FlagZones[ FlagTeam ]  & 0x0000FFFF );
            pFlag->InitCloth();
            NetObjMgr.AddObject( pFlag );

            L2W.Identity();
            L2W.SetRotation( radian3( R_0, m_FlagYaw[ FlagTeam ], R_0 ) );
            L2W.SetTranslation( m_FlagBase[ FlagTeam ] );
            pFlagBase->Init( pProp->GetCircuit().GetCircuit(), L2W );
            NetObjMgr.AddObject( pFlagBase );

            m_FlagSecure [ FlagTeam ] = TRUE;
            m_FlagDropped[ FlagTeam ] = FALSE;
            m_FlagCarrier[ FlagTeam ] = -1;
            m_FlagNetSlot[ FlagTeam ] = pFlag->net_GetSlot();

            #ifdef X_ASSERT
            Sanity |= (1 << FlagTeam);
            #endif
        }
    }
    SlotIter.End();

    ASSERT( Sanity == 0x0003 );

    // Update the player highlight.
    GameMgr.m_Score.HighLightMask = 0x00000000;
    GameMgr.m_DirtyBits          |= game_mgr::DIRTY_HIGHLIGHT;

    // Set the score limit.
//  GameMgr.SetScoreLimit( ? );
}

//==============================================================================

void logic_ctf::EndGame( void )
{
    // Detach flags from players.  
    // (Players should ASSERT that they have no flags when they are destroyed.)
    /*
    if( m_FlagCarrier[0] != -1 )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( m_FlagCarrier[0] );
        pActor->DetachFlag();
    }

    if( m_FlagCarrier[1] != -1 )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( m_FlagCarrier[1] );
        pActor->DetachFlag();
    }
    */
}

//==============================================================================

void logic_ctf::DropFlag( s32 PlayerIndex )
{
    // If the player had the flag, drop it!

    s32 FlagIndex = -1;

    if( m_FlagCarrier[0] == PlayerIndex )  FlagIndex = 0;
    if( m_FlagCarrier[1] == PlayerIndex )  FlagIndex = 1;

    if( FlagIndex != -1 )
    {
        actor* pPlayer = (actor*)NetObjMgr.GetObjFromSlot( PlayerIndex );
        ASSERT( pPlayer );

        DropFlag( FlagIndex, 
                  PlayerIndex,
                  pPlayer->GetPosition(), 
                  pPlayer->GetYaw(), 
                  pPlayer->GetZone1(), 
                  pPlayer->GetZone2() );
    }
}

//==============================================================================
