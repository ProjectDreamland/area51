//==============================================================================
//
//  logic_Campaign.cpp
//
//==============================================================================

//==============================================================================
//	INCLUDES
//==============================================================================

#include "logic_Campaign.hpp"
#include "Objects/Actor/Actor.hpp"  // For actor class
#include "Objects/Render/PostEffectMgr.hpp"
#include "StateMgr\StateMgr.hpp"

//==============================================================================
//	FUNCTIONS
//==============================================================================

logic_campaign::logic_campaign( void ) :
    m_Alive                 ( TRUE              ),
    m_bSpawnInfoSet         ( FALSE             ),
    m_PlayerSpawnPosition   ( 0.0f, 0.0f, 0.0f  ),
    m_PlayerSpawnPitch      ( 0.0f              ),
    m_PlayerSpawnYaw        ( 0.0f              ),
    m_PlayerSpawnZone       ( -1                )
{
}

//==============================================================================

logic_campaign::~logic_campaign( void )
{
}

//==============================================================================

void logic_campaign::Activate( void )
{
    // Do nothing.  This function prevents the base class function (which does 
    // do stuff which is possibly campaign unfriendly) from getting called.
}

//==============================================================================

void logic_campaign::PlayerDied( s32 Victim, s32 Killer, s32 PainType )
{
    (void)Killer;
    (void)PainType;

    // Kill the player and reset the respawn timer.
    if( (Victim == 0) && m_Alive )
    {
        m_Alive         = FALSE;
        m_RespawnDelay  = 4.0f;
        g_PostEffectMgr.StartScreenFade( xcolor(0,0,0,255), m_RespawnDelay );
    }
}

//==============================================================================

void logic_campaign::RequestSpawn( s32 PlayerIndex, xbool /*Immediate*/ )
{
    (void)PlayerIndex;
    if( !m_Alive && (m_RespawnDelay == 0.0f) )
    {
        g_ActiveConfig.SetExitReason( GAME_EXIT_RELOAD_CHECKPOINT );
/* rmb - old way
        player* pPlayer = (player*)NetObjMgr.GetObjFromSlot( PlayerIndex );
        ASSERT( pPlayer );

        m_Alive = TRUE;
        pPlayer->OnSpawn();

        // restore the player to the last saved position
        g_SaveMgr.RestoreGame( TRUE );

        // Setup the weapon objects when spawning from a game restore
        pPlayer->CreateAllWeaponObjects();
        pPlayer->SetNextWeapon2( pPlayer->GetCurrentWeapon2() );

        g_PostEffectMgr.StartScreenFade( xcolor(0,0,0,0), 3.0f );
*/
    }
}

//==============================================================================

void logic_campaign::AdvanceTime( f32 DeltaTime )
{
    if( m_RespawnDelay > 0.0f )
    {
        m_RespawnDelay -= DeltaTime;
    }
    else
    {
        m_RespawnDelay = 0.0f;

        if ( !g_StateMgr.IsPaused() )
        {
            RequestSpawn( 0 );  // Force respawn!
        }
    }
}

//==============================================================================

void logic_campaign::SetPlayerSpawnInfo( const vector3& Position, 
                                               radian   Pitch, 
                                               radian   Yaw, 
                                               s32      Zone, 
                                         const guid&    Guid )
{
    m_bSpawnInfoSet         = TRUE;
    m_PlayerSpawnPosition   = Position;
    m_PlayerSpawnPitch      = Pitch;
    m_PlayerSpawnYaw        = Yaw;
    m_PlayerSpawnZone       = Zone;
    m_PlayerSpawnGuid       = Guid;
}

//==============================================================================

void logic_campaign::BeginGame( void )
{
    player* pPlayer = NULL;

    logic_base::BeginGame();

    GameMgr.SetScoreDisplay( SCORE_DISPLAY_NONE );

    //
    // Create the player
    //
    if( m_bSpawnInfoSet )
    {
        g_ObjMgr.CreateObject( "Player", m_PlayerSpawnGuid );
        pPlayer = (player*)g_ObjMgr.GetObjectByGuid( m_PlayerSpawnGuid );

        ASSERTS( pPlayer, "Unable to create player in logic_campaign::BeginGame()" );

        if( pPlayer )
        {
            // Setting position in orientation
            matrix4 L2W;
            L2W.Identity();
            L2W.RotateY( m_PlayerSpawnYaw );
            L2W.Translate( m_PlayerSpawnPosition );
            pPlayer->OnTransform( L2W );

            // set this player up
            pPlayer->SetPitch( m_PlayerSpawnPitch    );
            pPlayer->SetZone1( m_PlayerSpawnZone     );
            pPlayer->SetZone2( 0 );
            pPlayer->InitZoneTracking();
        }
    }
    else
    {
        // Should always have spawn info setup by here.
        LOG_ERROR( "logic_campagin::BeginGame", "Spawn info not set." );
        LOG_ERROR( "logic_campagin::BeginGame", "Attempting to use a spawn point." );

        // Let's try some trickery to get a player anyway.
        spawn_point* pSpawnPoint = GameMgr.SelectSpawnPoint(1);
        if( pSpawnPoint )
        {
            m_PlayerSpawnGuid = guid_New();
            g_ObjMgr.CreateObject( "Player", m_PlayerSpawnGuid );
            pPlayer = (player*)g_ObjMgr.GetObjectByGuid( m_PlayerSpawnGuid );

            ASSERTS( pPlayer, "Unable to create player in logic_campaign::BeginGame()" );

            if( pPlayer )
            {
                //  Lookup spawn point info
                vector3 Position;
                radian3 Rotation;
                u16     Zone1;
                u16     Zone2;
                pSpawnPoint->GetSpawnInfo( pPlayer->GetGuid(), Position, Rotation, Zone1, Zone2 );

                // Set this player up.
                pPlayer->Teleport( Position );
                pPlayer->SetPitch( Rotation.Pitch );
                pPlayer->SetYaw  ( Rotation.Yaw   ); 
                pPlayer->SetZone1( Zone1 );
                pPlayer->SetZone2( Zone2 );
                pPlayer->InitZoneTracking();
            }
        }
    }

    pPlayer->OnSpawn();

    // turn off shadows for the player--they're only needed for split-screen
    pPlayer->SetAttrBits( pPlayer->GetAttrBits() & ~object::ATTR_CAST_SHADOWS );

    m_Alive = TRUE;
}

//==============================================================================
