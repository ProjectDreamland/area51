//==============================================================================
//
//  NetPlayer.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "NetPlayer.hpp"
#include "x_bitstream.hpp"
#include "GameMgr.hpp"
#include "NetworkMgr.hpp"

#include "x_log.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  STORAGE
//==============================================================================

const s32 g_FramesToBlendPos          = 6;
const s32 g_FramesToBlendOrientation  = 6;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void net_player::Activate( void )
{
    m_Actual.Pos( 0, 0, 0 );
    m_Actual.Pitch      = R_0;
    m_Actual.Yaw        = R_0;

    m_Actual            = m_Actual;

    m_Weapon            =  -1;
    m_Health            = 100;
    m_Pain              =   0;
    m_PainType          =  -1;
    m_PainDirBits       =   0;
    m_PainOrigin        =  -1;
    m_Alive             = TRUE;
                      
    m_Crouch            = FALSE;
    m_Jump              = 0;
    m_Reload            = 0;

    m_Fire              =  0;
    m_FireWeaponIndex   = -1;

    for( s32 i=0 ; i<4 ; i++ )
    {
        m_AmmoClip   [i] =  0;
        m_AmmoReserve[i] =  0;
    }

    m_Toss              = 0;
    m_Respawn           = 0;
    m_LifeSeq           = 0;

    m_DeltaData[0]      = 0;  // 1st weapon
    m_DeltaData[1]      = 0;  // 2nd weapon
    m_DeltaData[2]      = 0;  // 3rd weapon
    m_DeltaData[3]      = 0;  // 4th weapon
    m_DeltaData[4]      = 0;  // grenades
    m_DeltaData[5]      = 0;  // health

    m_Active            = TRUE;
    m_DirtyBits         = ACTIVATE_BIT;
                    
    m_pMoveMgr          = g_NetworkMgr.GetMoveMgr( m_Slot );

    m_MuteBits          = 0x00;
    m_ChangingTeam      = FALSE;

    if( g_NetworkMgr.IsServer() )
    {
        if( m_Slot < g_NetworkMgr.GetLocalPlayerCount() )
            m_Mode = LOCAL_ON_SERVER;
        else
            m_Mode = GHOST_ON_SERVER;
    }
    else
    {
        if( m_pMoveMgr )
            m_Mode = LOCAL_ON_CLIENT;
        else
            m_Mode = GHOST_ON_CLIENT;
    }

    m_BlendPosX .Init(  0.01f, 3.0f, g_FramesToBlendPos        , FALSE );
    m_BlendPosY .Init(  0.01f, 3.0f, g_FramesToBlendPos        , FALSE );
    m_BlendPosZ .Init(  0.01f, 3.0f, g_FramesToBlendPos        , FALSE );
    m_BlendPitch.Init( 0.001f,   PI, g_FramesToBlendOrientation, TRUE  );
    m_BlendYaw  .Init( 0.001f,   PI, g_FramesToBlendOrientation, TRUE  );

    m_LocalStats.armHits[0] = 0;
    m_LocalStats.armHits[1] = 0;
    m_LocalStats.groinHits  = 0;
    m_LocalStats.headHits   = 0;
    m_LocalStats.hits       = 0;
    m_LocalStats.hitsTaken  = 0;
    m_LocalStats.legHits[0] = 0;
    m_LocalStats.legHits[1] = 0;
    m_LocalStats.shotsFired = 0;
    m_LocalStats.torsoHits  = 0;

    CreateGameObj();

    LOG_MESSAGE( "net_player::Activate", "Slot:%d", m_Slot );
}

//==============================================================================

void net_player::Deactivate( void )
{
    DestroyGameObj();

    m_Active    = FALSE;
    m_DirtyBits = DEACTIVATE_BIT;

    LOG_MESSAGE( "net_player::Deactivate", "Slot:%d", m_Slot );
}

//==============================================================================

#define GET(n,g,b)          \
    if( Buffer.g != n )     \
    {                       \
        n = Buffer.g;       \
        m_DirtyBits |= b;   \
    }

#define ADD(n,g,b)          \
    if( Buffer.g != 0 )     \
    {                       \
        n += Buffer.g;      \
        m_DirtyBits |= b;   \
    }

void net_player::Logic( void )
{
    buffer  Buffer;
    s32     i;

    switch( m_Mode )
    {
    case LOCAL_ON_SERVER:
        {
            // Locally controlled server players were updated during the game
            // logic.  These changes are authoritative.  Move data from the game
            // object into the net object.  All changes must be flagged with 
            // dirty bits in order to cause the data to be sent via updates to
            // client ghosts.

            // Prepare the buffer for READING.
            Buffer.ReadBits = POSITION_BIT    
                            | ORIENTATION_BIT 
                            | HEALTH_BIT
                            | ALIVE_BIT       
                            | WEAPON_BIT 
                            | JUMP_BIT      
                            | MELEE_BIT
                            | RELOAD_BIT      
                            | CROUCH_BIT      
                            | FIRE_BIT 
                            | TOSS_BIT
                            | RESPAWN_BIT
                            | DELTA_DATA_BIT;

            // Prepare the buffer for WRITING.
            Buffer.WriteBits         = PAIN_BIT;
            Buffer.ClientPain        = m_Pain;
            Buffer.ClientPainType    = m_PainType;
            Buffer.ClientPainDirBits = m_PainDirBits;
            Buffer.ClientPainOrigin  = m_PainOrigin;
            m_Pain                   = 0;   // Clear the client pain.
//          m_PainDirBits            = 0;   // Clear the client pain dirbits

            // Read values from game object into Buffer.
//            ExchangeValues( Buffer );

            // Now, the net object must receive all these values.  For any that
            // are different, set a dirty bit.

            if( (m_Actual.Pos - Buffer.Position).LengthSquared() != 0.0f )
            {
                m_Actual.Pos = Buffer.Position;
                m_DirtyBits |= POSITION_BIT;
            }

            GET( m_Actual.Pitch , Pitch         , ORIENTATION_BIT  );
            GET( m_Actual.Yaw   , Yaw           , ORIENTATION_BIT  );
            GET( m_Health       , Health        , HEALTH_BIT       );  
            GET( m_LifeSeq      , LifeSeq       , HEALTH_BIT       );  
            GET( m_Alive        , Alive         , ALIVE_BIT        );
            GET( m_Weapon       , Weapon        , WEAPON_BIT       );
            GET( m_Jump         , Jump          , JUMP_BIT         );
            GET( m_Melee        , Melee         , MELEE_BIT        );
            GET( m_Reload       , Reload        , RELOAD_BIT       );
            GET( m_Crouch       , Crouch        , CROUCH_BIT       );
            GET( m_Fire         , Fire          , FIRE_BIT         ); 
            GET( m_Toss         , Toss          , TOSS_BIT         ); 
            GET( m_Respawn      , Respawn       , RESPAWN_BIT      ); 
            ADD( m_DeltaData[5] , DeltaData[5]  , DELTA_DATA_BIT   );

            m_FirePos           = Buffer.FirePos;
            m_FireVel           = Buffer.FireVel;
            m_FireStrength      = Buffer.FireStrength;
            m_FireWeaponIndex   = Buffer.FireWeaponIndex;
            m_TossPos           = Buffer.TossPos;
            m_TossVel           = Buffer.TossVel;
            m_TossStrength      = Buffer.TossStrength;

            // Since this player is local on the server, there is no need for
            // blending.  So, just copy the Actual values to the Render.
            m_Render = m_Actual;

            // Did the host player want to change teams?
            if( m_ChangingTeam )
            {
                GameMgr.ChangeTeam( m_Slot );
            }

            //
            // Logging...
            //

            LOG_MESSAGE( "net_player::Logic",
                         "Slot:%d - Mode:LOCAL ON SERVER - DirtyBits:%08X - Health:%d", 
                         m_Slot, m_DirtyBits, m_Health );

            if( Buffer.ClientPain > 0 )
            {
                LOG_MESSAGE( "net_player::Logic( Pain )",
                             "Slot:%d - Mode:LOCAL ON SERVER - WritePain:%d",
                             m_Slot, Buffer.ClientPain );  
            }
        }
        break;

    case LOCAL_ON_CLIENT:
        {
            // Locally controlled client players were updated during the game
            // logic.  The "player controlled" changes are essentially 
            // authoritative, although the server can override later if needed.
            //
            // Other data is server controlled.  This includes the health, alive 
            // status, and ammo additions.
            //
            // We need to move locally controlled data from the game object to
            // the net object, and server controlled data from the net to the 
            // game.

            // Prepare the buffer for READING.
            Buffer.ReadBits = POSITION_BIT    
                            | ORIENTATION_BIT 
                            | WEAPON_BIT      
                            | JUMP_BIT      
                            | MELEE_BIT
                            | RELOAD_BIT      
                            | CROUCH_BIT      
                            | FIRE_BIT   
                            | TOSS_BIT   
                            | RESPAWN_BIT   
                            | PAIN_BIT
                            | AMMO_BIT;

            // Prepare the buffer for WRITING.
            Buffer.WriteBits = HEALTH_BIT 
                             | ALIVE_BIT
                             | DELTA_DATA_BIT;  // Ammo, health pickups.
            Buffer.Health       = m_Health;
            Buffer.PainDirBits  = m_PainDirBits;
            Buffer.LifeSeq      = m_LifeSeq;
            Buffer.Alive        = m_Alive;
            Buffer.DeltaData[0] = m_DeltaData[0];  m_DeltaData[0] = 0;
            Buffer.DeltaData[1] = m_DeltaData[1];  m_DeltaData[1] = 0;
            Buffer.DeltaData[2] = m_DeltaData[2];  m_DeltaData[2] = 0;
            Buffer.DeltaData[3] = m_DeltaData[3];  m_DeltaData[3] = 0;
            Buffer.DeltaData[4] = m_DeltaData[4];  m_DeltaData[4] = 0;
            Buffer.DeltaData[5] = m_DeltaData[5];  m_DeltaData[5] = 0;

            // Exchange values from game object into the buffer.
//            ExchangeValues( Buffer );

            // Copy appropriate data into the net object.
            m_Actual.Pos        = Buffer.Position;
            m_Actual.Pitch      = Buffer.Pitch;
            m_Actual.Yaw        = Buffer.Yaw;
            m_Weapon            = Buffer.Weapon;
            m_Jump              = Buffer.Jump;
            m_Melee             = Buffer.Melee;
            m_Reload            = Buffer.Reload;
            m_Crouch            = Buffer.Crouch;
            m_Fire              = Buffer.Fire;
            m_Toss              = Buffer.Toss;
            m_Respawn           = Buffer.Respawn;

            m_FirePos           = Buffer.FirePos;
            m_FireVel           = Buffer.FireVel;
            m_FireStrength      = Buffer.FireStrength;
            m_FireWeaponIndex   = Buffer.FireWeaponIndex;
            m_TossPos           = Buffer.TossPos;
            m_TossVel           = Buffer.TossVel;
            m_TossStrength      = Buffer.TossStrength;

            for( i=0 ; i<4 ; i++ )
            {
                m_AmmoClip   [i] = Buffer.AmmoClip   [i];
                m_AmmoReserve[i] = Buffer.AmmoReserve[i];
            }

            // Locally controlled clients send moves to the server.  Prepare a
            // move structure now.  This includes only data which is controlled
            // by the player.

            move Move;

            Move.Slot               = m_Slot;
            Move.Team               = -1;   // No change in team.
            Move.Position           = m_Actual.Pos;
            Move.Pitch              = m_Actual.Pitch;
            Move.Yaw                = m_Actual.Yaw;
            Move.Jump               = m_Jump;
            Move.Melee              = m_Melee;
            Move.Reload             = m_Reload;
            Move.Crouch             = m_Crouch;
            Move.Weapon             = m_Weapon;
            Move.Fire               = m_Fire;
            Move.Toss               = m_Toss;
            Move.Respawn            = m_Respawn;

            Move.FirePos            = m_FirePos;
            Move.FireVel            = m_FireVel;
            Move.FireStrength       = m_FireStrength;
            Move.FireWeaponIndex    = m_FireWeaponIndex;
            Move.TossPos            = m_TossPos;
            Move.TossVel            = m_TossVel;
            Move.TossStrength       = m_TossStrength;

            for( i=0 ; i<4 ; i++ )
            {
                Move.AmmoClip   [i] = m_AmmoClip   [i];
                Move.AmmoReserve[i] = m_AmmoReserve[i];
            }

            if( Move.Weapon ==  0 )
                Move.Weapon  = -1;

            for( i = 0; i < 32; i++ )
            {
                Move.Pain[i].Pain       = Buffer.Pain[i].Pain;
                Move.Pain[i].DirBits    = Buffer.Pain[i].DirBits;
                // TO DO - Pain Type!
            }

            if( m_ChangingTeam )
            {
                LOG_MESSAGE( "net_player::Logic",
                             "Slot:%d - DesiredTeam:%d", 
                             m_DesiredTeam );
                Move.Team = m_DesiredTeam;
            }

            ASSERT( m_pMoveMgr );
            m_pMoveMgr->AddMove( Move );

            // Although the local player is never rendered, we will go ahead and
            // keep the render data up to date just in case...
            m_Render = m_Actual;

            //
            // Logging...
            //

            LOG_MESSAGE( "net_player::Logic",
                         "Slot:%d - Mode:LOCAL ON CLIENT - Health:%d", m_Slot, m_Health );

            if( Buffer.DeltaData[0] | 
                Buffer.DeltaData[1] | 
                Buffer.DeltaData[2] | 
                Buffer.DeltaData[3] | 
                Buffer.DeltaData[4] )
            {
                LOG_MESSAGE( "net_player::Logic( Ammo )", 
                             "... AddAmmo:%d,%d,%d,%d - AddGrenades:%d",
                             Buffer.DeltaData[0], Buffer.DeltaData[1], 
                             Buffer.DeltaData[2], Buffer.DeltaData[3], 
                             Buffer.DeltaData[4] );
            }

            if( Buffer.DeltaData[5] )
            {
                LOG_MESSAGE( "net_player::Logic( Health )", 
                             "... AddHealth:%d",
                             Buffer.DeltaData[5] );
            }

            for( i = 0; i < 32; i++ )
            {
                if( Move.Pain[i].Pain )
                {
                    LOG_WARNING( "net_player::Logic( Pain )", 
                                 "Slot:%d - Mode:LOCAL ON CLIENT - Victim:%d - Pain:%d",
                                 m_Slot, i, Move.Pain[i].Pain );
                }
            }
        }
        break;

    case GHOST_ON_SERVER:
        {
            // Ghosts on the server receive moves from their corresponding
            // locally controlled client.  The moves are applied in the 
            // ApplyMove() function.  The moves are applied directly to the net
            // object and dirty bits are set accordingly.  In here, we must move
            // values from the net object to the game object.
            //
            // Except!  Health and alive are NOT controlled by moves.  These 
            // values are controlled on the server.  So, these values must go 
            // from the game object to the net object.
            //
            // And!  The game ghost may bump into ammo pickups and thus have 
            // additional ammo to report.
            // 
            // Furthermore!  We need to know if the ghost fired his weapon.

            ASSERT( m_pMoveMgr );

            //
            // Apply the moves.
            //

            /* Attempt to apply the moves smoothly over time...
            s32 Moves = m_pMoveMgr->GetCount();
            s32 Apply = (Moves-1) / 2;
            if( Moves > 0 )
            {
                Apply = MAX( 1, Apply );
                for( s32 i = 0; i < Apply; i++ )
                {
                    move Move;
                    m_pMoveMgr->GetMove( Move );
                    ApplyMove( Move );
                }
            }
            */

            // Apply ALL moves!
            s32 Moves = m_pMoveMgr->GetCount();
            s32 Apply = Moves;
            while( m_pMoveMgr->GetCount() > 0 )
            {
                move Move;
                m_pMoveMgr->GetMove( Move );
                ApplyMove( Move );
            }

            // First, run the blend logic to smooth out the impact of incoming 
            // moves.
            BlendLogicPos();
            BlendLogicOrientation();

            // Prepare the buffer for READING.
            Buffer.ReadBits  = HEALTH_BIT 
                             | ALIVE_BIT
                             | DELTA_DATA_BIT;

            // Prepare the buffer for WRITING.
            Buffer.WriteBits = POSITION_BIT    
                             | ORIENTATION_BIT 
                             | WEAPON_BIT      
                             | JUMP_BIT      
                             | MELEE_BIT
                             | RELOAD_BIT      
                             | CROUCH_BIT      
                             | FIRE_BIT   
                             | TOSS_BIT
                             | RESPAWN_BIT
                             | PAIN_BIT
                             | AMMO_BIT;
            Buffer.Position         = m_Render.Pos;
            Buffer.Pitch            = m_Render.Pitch; 
            Buffer.Yaw              = m_Render.Yaw;
            Buffer.Weapon           = m_Weapon;
            Buffer.Jump             = m_Jump;
            Buffer.Melee            = m_Melee;
            Buffer.Reload           = m_Reload;
            Buffer.Crouch           = m_Crouch;
            Buffer.Fire             = m_Fire;
            Buffer.Toss             = m_Toss;
            Buffer.Respawn          = m_Respawn;
            
            Buffer.FirePos          = m_FirePos;
            Buffer.FireVel          = m_FireVel;
            Buffer.FireStrength     = m_FireStrength;
            Buffer.FireWeaponIndex  = m_FireWeaponIndex;
            Buffer.TossPos          = m_TossPos;
            Buffer.TossVel          = m_TossVel;
            Buffer.TossStrength     = m_TossStrength;

            for( i=0 ; i<4 ; i++ )
            {
                Buffer.AmmoClip   [i] = m_AmmoClip   [i];
                Buffer.AmmoReserve[i] = m_AmmoReserve[i];
            }

            Buffer.ClientPain        = m_Pain;
            Buffer.ClientPainType    = m_PainType;
            Buffer.ClientPainDirBits = m_PainDirBits;
            Buffer.ClientPainOrigin  = m_PainOrigin;
            m_Pain                   = 0;   // Clear the client pain.
//          m_PainDirBits            = 0;   // Clear the client pain dirbits

            // Exchange values from game object into the buffer.
//            ExchangeValues( Buffer );

            // Copy appropriate data into the net object.
            GET( m_Health       , Health        , HEALTH_BIT   ); 
            GET( m_LifeSeq      , LifeSeq       , HEALTH_BIT   ); 
            GET( m_Alive        , Alive         , ALIVE_BIT    );
            ADD( m_DeltaData[0] , DeltaData[0]  , DELTA_DATA_BIT );
            ADD( m_DeltaData[1] , DeltaData[1]  , DELTA_DATA_BIT );
            ADD( m_DeltaData[2] , DeltaData[2]  , DELTA_DATA_BIT );
            ADD( m_DeltaData[3] , DeltaData[3]  , DELTA_DATA_BIT );
            ADD( m_DeltaData[4] , DeltaData[4]  , DELTA_DATA_BIT );
            ADD( m_DeltaData[5] , DeltaData[5]  , DELTA_DATA_BIT );
            m_PainDirBits = Buffer.PainDirBits;

            //
            // Logging...
            //

            LOG_MESSAGE( "net_player::Logic",
                         "Slot:%d - Mode:GHOST ON SERVER - DirtyBits:%08X - Moves:%d of %d - Health:%d", 
                         m_Slot, m_DirtyBits, Apply, Moves, m_Health );

            if( m_DirtyBits & DELTA_DATA_BIT )
            {
                LOG_MESSAGE( "net_player::Logic( Ammo )", "... AddAmmo:%d,%d,%d,%d,%d",
                             m_DeltaData[0], m_DeltaData[1], 
                             m_DeltaData[2], m_DeltaData[3], 
                             m_DeltaData[4] );
            }

            if( Buffer.ClientPain > 0 )
            {
                LOG_MESSAGE( "net_player::Logic( Pain )", 
                             "Slot:%d - Mode:LOCAL ON CLIENT - WritePain:%d",
                             m_Slot, Buffer.ClientPain );
            }
        }
        break;

    case GHOST_ON_CLIENT:
        {
            // Ghosts on the client are completely driven by data from the net
            // object.  So, we are going to move data from the net object into
            // the game object (via the Buffer).  However, we need to know if 
            // the game object has fired his weapon.

            // Since we are a ghost, we are using blending, so update it.
            BlendLogicPos();
            BlendLogicOrientation();

            // Prepare the buffer for READING.
            Buffer.ReadBits  = 0x00;

            // Prepare the buffer for WRITING.
            Buffer.WriteBits = POSITION_BIT
                             | ORIENTATION_BIT
                             | HEALTH_BIT
                             | ALIVE_BIT
                             | WEAPON_BIT
                             | JUMP_BIT
                             | MELEE_BIT
                             | RELOAD_BIT
                             | CROUCH_BIT
                             | FIRE_BIT
                             | TOSS_BIT
                             | RESPAWN_BIT
                             | DELTA_DATA_BIT;
            Buffer.Position         = m_Render.Pos;
            Buffer.Pitch            = m_Render.Pitch;
            Buffer.Yaw              = m_Render.Yaw;
            Buffer.Health           = m_Health;
            Buffer.LifeSeq          = m_LifeSeq;
            Buffer.Alive            = m_Alive;
            Buffer.Weapon           = m_Weapon;
            Buffer.Jump             = m_Jump;
            Buffer.Melee            = m_Melee;
            Buffer.Reload           = m_Reload;
            Buffer.Crouch           = m_Crouch;
            Buffer.Fire             = m_Fire;
            Buffer.Toss             = m_Toss;
            Buffer.Respawn          = m_Respawn;
                                         
            Buffer.FirePos          = m_FirePos;
            Buffer.FireVel          = m_FireVel;
            Buffer.FireStrength     = m_FireStrength;
            Buffer.FireWeaponIndex  = m_FireWeaponIndex;
            Buffer.TossPos          = m_TossPos;
            Buffer.TossVel          = m_TossVel;
            Buffer.TossStrength     = m_TossStrength;

            Buffer.DeltaData[5] = m_DeltaData[5];  m_DeltaData[5] = 0;

            // Exchange values from game object into the buffer.
//            ExchangeValues( Buffer );

            // Logging...
            LOG_MESSAGE( "net_player::Logic",
                         "Slot:%d - Mode:GHOST ON CLIENT - Health:%d", m_Slot, m_Health );
        }
        break;

    default:
        ASSERT( FALSE );
        break;
    }
}

#undef ADD
#undef GET

//==============================================================================

#define SET(n,g,b)  \
    if( n != Move.g )       \
    {                       \
        n = Move.g;         \
        m_DirtyBits |= b;   \
    }    

#define INC(n,g,b)  \
    if( Move.g )            \
    {                       \
        n += 1;             \
        m_DirtyBits |= b;   \
    }    

void net_player::ApplyMove( const move& Move )
{
    s32 i;

    // We have received an authoritative move from a client.

    ASSERT( m_Mode == GHOST_ON_SERVER );

    LOG_MESSAGE( "net_player::ApplyMove",
                 "Slot:%d - Seq:%d - Fire:%d - Toss:%d - Pain[0]:%d - Pain[1]:%d",
                 m_Slot, Move.Seq, Move.Fire, Move.Toss, Move.Pain[0].Pain, Move.Pain[1].Pain );

    if( (m_Actual.Pos - Move.Position).LengthSquared() != 0.0f )
    {
        m_Actual.Pos = Move.Position;
        m_DirtyBits |= POSITION_BIT;
    }

    SET( m_Actual.Pitch     , Pitch         , ORIENTATION_BIT   );
    SET( m_Actual.Yaw       , Yaw           , ORIENTATION_BIT   );
    SET( m_Weapon           , Weapon        , WEAPON_BIT        );
    SET( m_Jump             , Jump          , JUMP_BIT          );
    SET( m_Melee            , Melee         , MELEE_BIT         );
    SET( m_Reload           , Reload        , RELOAD_BIT        );
    SET( m_Crouch           , Crouch        , CROUCH_BIT        );
    SET( m_Fire             , Fire          , FIRE_BIT          );
    SET( m_Toss             , Toss          , TOSS_BIT          );
    SET( m_Respawn          , Respawn       , RESPAWN_BIT       );

    m_FirePos           = Move.FirePos;
    m_FireVel           = Move.FireVel;
    m_FireStrength      = Move.FireStrength;
    m_FireWeaponIndex   = Move.FireWeaponIndex;
    m_TossPos           = Move.TossPos;
    m_TossVel           = Move.TossVel;
    m_TossStrength      = Move.TossStrength;

    for( i=0 ; i<4 ; i++ )
    {
        m_AmmoClip   [i] = Move.AmmoClip   [i];
        m_AmmoReserve[i] = Move.AmmoReserve[i];
    }

    // Apply client reported pain.
    for( i = 0; i < 32; i++ )
    {
        if( Move.Pain[i].Pain )
        {
            net_player* pNetPlayer = (net_player*)NetObjMgr.GetObjFromSlot( i );
            if( pNetPlayer )
                pNetPlayer->ApplyPain( Move.Pain[i].Pain, Move.Pain[i].Type, Move.Pain[i].DirBits, m_Slot );
        }
    }

    // Did the client want to change teams?
    if( GameMgr.IsTDM() && (Move.Team != -1) )
    {
        s32 Team = GameMgr.GetScore().Player[m_Slot].Team;

        if( Team != Move.Team )
        {
//            ForceDeath();
            GameMgr.ChangeTeam( m_Slot, FALSE );
        }
    }

    // TO DO - Check contraints.  Hacker tracker v2.3!

    BlendUpdatePos();
    BlendUpdateOrientation();
}

#undef INC
#undef SET

//==============================================================================

void net_player::ApplyPain( s32 Pain, s32 Type, s32 DirBits, s32 Origin )
{
    LOG_MESSAGE( "net_player::ApplyPain", "Slot:%d - Pain:%d", GetSlot(), Pain );

    ASSERT( Pain > 0 );

    m_Pain         += Pain;
    m_PainType      = Type;
    m_PainDirBits   = DirBits;
    m_PainOrigin    = Origin;
}

//==============================================================================

void net_player::OnProvideUpdate(       bitstream& BS, 
                                        u32&       DirtyBits, 
                                        s32        Client,
                                  const delta*     pDelta )
{
    // Anything to do?
    if( !DirtyBits )
        return;

    char   Msg[64];
    s32    C = 0;
    xbool  DeltaData = FALSE;
    x_memset( Msg, 0, 64 );

    if( BS.WriteFlag( DirtyBits & DEACTIVATE_BIT ) )
    {
        Msg[C++] = 'D';
        DirtyBits &= ~DEACTIVATE_BIT;
    }

    if( BS.WriteFlag( DirtyBits & ACTIVATE_BIT ) )
    {
        Msg[C++] = 'A';
        DirtyBits &= ~ACTIVATE_BIT;

        BS.WriteRangedS32( m_PrimarySkin,   0, 29 );
        BS.WriteRangedS32( m_AlternateSkin, 0, 29 );

        BS.WriteRangedS32( m_Jump    & 0x07, 0, 7 );
        BS.WriteRangedS32( m_Melee   & 0x07, 0, 7 );
        BS.WriteRangedS32( m_Reload  & 0x07, 0, 7 );
        BS.WriteRangedS32( m_Fire    & 0x07, 0, 7 );
        BS.WriteRangedS32( m_Toss    & 0x07, 0, 7 );
        BS.WriteRangedS32( m_Respawn & 0x07, 0, 7 );
        BS.WriteRangedS32( m_LifeSeq & 0x07, 0, 7 );

        DirtyBits |= POSITION_BIT;
        DirtyBits |= ORIENTATION_BIT;
        DirtyBits |= WEAPON_BIT;
        DirtyBits |= HEALTH_BIT;
        DirtyBits |= ALIVE_BIT;
    }

    if( BS.WriteFlag( DirtyBits & POSITION_BIT ) )
    {
        Msg[C++] = 'P';
        DirtyBits &= ~POSITION_BIT;
        BS.WriteVector( m_Actual.Pos );
    }

    if( BS.WriteFlag( DirtyBits & ORIENTATION_BIT ) )
    {
        Msg[C++] = 'O';
        DirtyBits &= ~ORIENTATION_BIT;
        BS.WriteF32( m_Actual.Pitch );
        BS.WriteF32( m_Actual.Yaw   );
    }

    if( BS.WriteFlag( DirtyBits & WEAPON_BIT ) )
    {
        Msg[C++] = 'W';
        DirtyBits &= ~WEAPON_BIT;
        BS.WriteS32( m_Weapon );
    }

    if( BS.WriteFlag( DirtyBits & HEALTH_BIT ) )
    {
        Msg[C++] = 'H';
        DirtyBits &= ~HEALTH_BIT;
        BS.WriteRangedS32( m_Health,         0, 100 );
        BS.WriteRangedS32( m_LifeSeq & 0x07, 0,   7 );
        BS.WriteRangedS32( m_PainDirBits,    0,  15 );
    }

    if( BS.WriteFlag( DirtyBits & ALIVE_BIT ) )
    {
        Msg[C++] = 'L';
        DirtyBits &= ~ALIVE_BIT;
        BS.WriteFlag( m_Alive );
    }

    if( BS.WriteFlag( DirtyBits & JUMP_BIT ) )
    {
        Msg[C++] = 'J';
        DirtyBits &= ~JUMP_BIT;
        BS.WriteRangedS32( m_Jump & 0x07, 0, 7 );
    }

    if( BS.WriteFlag( DirtyBits & MELEE_BIT ) )
    {
        Msg[C++] = 'M';
        DirtyBits &= ~MELEE_BIT;
        BS.WriteRangedS32( m_Melee & 0x07, 0, 7 );
    }

    if( BS.WriteFlag( DirtyBits & RELOAD_BIT ) )
    {
        Msg[C++] = 'R';
        DirtyBits &= ~RELOAD_BIT;
        BS.WriteRangedS32( m_Reload & 0x07, 0, 7 );
    }

    if( BS.WriteFlag( DirtyBits & CROUCH_BIT ) )
    {
        Msg[C++] = m_Crouch ? 'c' : 'C';
        DirtyBits &= ~CROUCH_BIT;
        BS.WriteFlag( m_Crouch );
    }

    if( BS.WriteFlag( DirtyBits & FIRE_BIT ) )
    {
        Msg[C++] = 'F';
        DirtyBits &= ~FIRE_BIT;
        BS.WriteRangedS32( m_Fire & 0x07, 0, 7 );
        BS.WriteVector( m_FirePos      );
        BS.WriteVector( m_FireVel      );
        BS.WriteF32   ( m_FireStrength );
    }

    if( BS.WriteFlag( DirtyBits & TOSS_BIT ) )
    {
        Msg[C++] = 'T';
        DirtyBits &= ~TOSS_BIT;
        BS.WriteRangedS32( m_Toss & 0x07, 0, 7 );
        BS.WriteVector( m_TossPos      );
        BS.WriteVector( m_TossVel      );
        BS.WriteF32   ( m_TossStrength );
    }

    if( BS.WriteFlag( DirtyBits & RESPAWN_BIT ) )
    {
        Msg[C++] = 'S';
        DirtyBits &= ~RESPAWN_BIT;
        BS.WriteRangedS32( m_Respawn & 0x07, 0, 7 );
    }

    if( BS.WriteFlag( DirtyBits & DELTA_DATA_BIT ) )
    {
        DeltaData = TRUE;
        Msg[C++] = 'D';
        DirtyBits &= ~DELTA_DATA_BIT;
        BS.WriteS32( pDelta[0] );
        BS.WriteS32( pDelta[1] );
        BS.WriteS32( pDelta[2] );
        BS.WriteS32( pDelta[3] );
        BS.WriteS32( pDelta[4] );
        BS.WriteS32( pDelta[5] );
    }

    LOG_MESSAGE( "net_player::OnProvideUpdate", 
                 "Client:%d - Slot:%d - Data:%s - Health:%d - LifeSeq:%d - Respawn:%d", 
                 Client, m_Slot, Msg, m_Health, m_LifeSeq, m_Respawn );

    if( DeltaData )
    {
        LOG_MESSAGE( "net_player::OnProvideUpdate( DeltaData )", 
                     "... DeltaData:%d,%d,%d,%d,%d,%d",
                     pDelta[0], pDelta[1], 
                     pDelta[2], pDelta[3], 
                     pDelta[4], pDelta[5] );
    }
}

//==============================================================================

void net_player::OnAcceptUpdate( const bitstream& BS )
{
    char    Msg[64];
    s32     C = 0;
    xbool   DeltaData = FALSE;
    x_memset( Msg, 0, 64 );

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'D';
        Deactivate();
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'A';

        BS.ReadRangedS32( m_PrimarySkin,   0, 29 );
        BS.ReadRangedS32( m_AlternateSkin, 0, 29 );

        Activate();

        BS.ReadRangedS32( m_Jump   , 0, 7 );
        BS.ReadRangedS32( m_Melee  , 0, 7 );
        BS.ReadRangedS32( m_Reload , 0, 7 );
        BS.ReadRangedS32( m_Fire   , 0, 7 );
        BS.ReadRangedS32( m_Toss   , 0, 7 );
        BS.ReadRangedS32( m_Respawn, 0, 7 );
        BS.ReadRangedS32( m_LifeSeq, 0, 7 );
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'P';
        BS.ReadVector( m_Actual.Pos );
        BlendUpdatePos();
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'O';
        BS.ReadF32( m_Actual.Pitch );
        BS.ReadF32( m_Actual.Yaw   );
        BlendUpdateOrientation();
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'W';
        BS.ReadS32( m_Weapon );
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'H';
        BS.ReadRangedS32( m_Health,      0, 100 );
        BS.ReadRangedS32( m_LifeSeq,     0,   7 );
        BS.ReadRangedS32( m_PainDirBits, 0,  15 );
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'L';
        BS.ReadFlag( m_Alive );
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'J';
        BS.ReadRangedS32( m_Jump, 0, 7 );
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'M';
        BS.ReadRangedS32( m_Melee, 0, 7 );
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'R';
        BS.ReadRangedS32( m_Reload, 0, 7 );
    }

    if( BS.ReadFlag() )
    {
        BS.ReadFlag( m_Crouch );
        Msg[C++] = m_Crouch ? 'c' : 'C';
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'F';
        BS.ReadRangedS32( m_Fire, 0, 7 );
        BS.ReadVector( m_FirePos      );
        BS.ReadVector( m_FireVel      );
        BS.ReadF32   ( m_FireStrength );
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'T';
        BS.ReadRangedS32( m_Toss, 0, 7 );
        BS.ReadVector( m_TossPos      );
        BS.ReadVector( m_TossVel      );
        BS.ReadF32   ( m_TossStrength );
    }

    if( BS.ReadFlag() )
    {
        Msg[C++] = 'S';
        BS.ReadRangedS32( m_Respawn, 0, 7 );
    }

    if( BS.ReadFlag() )
    {
        s32 V;
        DeltaData = TRUE;
        Msg[C++] = 'D';
        BS.ReadS32( V );    m_DeltaData[0] += V;
        BS.ReadS32( V );    m_DeltaData[1] += V;
        BS.ReadS32( V );    m_DeltaData[2] += V;
        BS.ReadS32( V );    m_DeltaData[3] += V;
        BS.ReadS32( V );    m_DeltaData[4] += V;
        BS.ReadS32( V );    m_DeltaData[5] += V;
    }

    LOG_MESSAGE( "net_player::OnAcceptUpdate", 
                 "Slot:%d - Data:%s - Health:%d - LifeSeq:%d - Respawn:%d", 
                 m_Slot, Msg, m_Health, m_LifeSeq, m_Respawn );

    if( DeltaData )
    {
        LOG_MESSAGE( "net_player::OnAcceptUpdate( DeltaData )", 
                     "... DeltaData:%d,%d,%d,%d,%d,%d",
                     m_DeltaData[0], m_DeltaData[1], 
                     m_DeltaData[2], m_DeltaData[3], 
                     m_DeltaData[4], m_DeltaData[5] );
    }
}

//==============================================================================

void net_player::GetClearDelta( delta* pDeltaData )
{
    for( s32 i = 0; i < 6; i++ )
    {
        pDeltaData [i] = m_DeltaData[i];
        m_DeltaData[i] = 0;
    }
}

//==============================================================================

void net_player::BlendUpdatePos( void )
{
    m_BlendPosX .SetTarget( m_Actual.Pos.X );
    m_BlendPosY .SetTarget( m_Actual.Pos.Y );
    m_BlendPosZ .SetTarget( m_Actual.Pos.Z );
}

//==============================================================================

void net_player::BlendUpdateOrientation( void )
{
    m_BlendPitch.SetTarget( m_Actual.Pitch );
    m_BlendYaw  .SetTarget( m_Actual.Yaw   );
}

//==============================================================================

void net_player::BlendLogicPos( void )
{
    m_Render.Pos.X = m_BlendPosX .BlendLogic();
    m_Render.Pos.Y = m_BlendPosY .BlendLogic();
    m_Render.Pos.Z = m_BlendPosZ .BlendLogic();
}

//==============================================================================

void net_player::BlendLogicOrientation( void )
{
    m_Render.Pitch = m_BlendPitch.BlendLogic();
    m_Render.Yaw   = m_BlendYaw  .BlendLogic();
}

//==============================================================================

xbool net_player::IsLocal( void )
{
    return( m_Mode & CONTROL_LOCAL );
}

//==============================================================================

xbool net_player::IsGhost( void )
{
    return( m_Mode & CONTROL_REMOTE );
}

//==============================================================================

void net_player::SetMute( s32 MouthIndex, xbool Mute )
{
    ASSERT( IN_RANGE( 0, MouthIndex, 31 ) );

    if( Mute )
        m_MuteBits |=  (1 << MouthIndex);
    else
        m_MuteBits &= ~(1 << MouthIndex);
}

//==============================================================================

xbool net_player::GetMute( s32 MouthIndex )
{
    return( m_MuteBits & (1 << MouthIndex) ? TRUE : FALSE );
}

//==============================================================================

void net_player::DesireTeamChange( void )
{
    if( GameMgr.IsTDM() )
    {
        m_ChangingTeam = TRUE;
        m_DesiredTeam  = 1 - GameMgr.GetScore().Player[ m_Slot ].Team;

        LOG_MESSAGE( "net_player::DesireTeamChange", 
                     "Player:%d - ToTeam:%d", m_Slot, m_DesiredTeam );
    }
    else
    {
        LOG_ERROR( "net_player::DesireTeamChange", "NOT IN TDM" );
    }
}

//==============================================================================

void net_player::SetSkins( s32 Primary, s32 Alternate )
{
    m_PrimarySkin   = Primary;
    m_AlternateSkin = Alternate;
}

//==============================================================================

void net_player::CreateGameObj(void)
{
}

void net_player::DestroyGameObj(void)
{
}

void net_player::DebugRender(void) const
{
}