#ifndef DISABLE_LEGACY_CODE
//==============================================================================
//
//  GhostNet.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//#if !defined(mtraub)
//#define X_SUPPRESS_LOGS
//#endif

//==============================================================================
//	DEBUG DEFINES AND MACROS
//==============================================================================

// Comment or uncomment to log update details.
//#define LOG_UPDATE_DETAILS

// If X_DEBUG is defined, then force LOG_UPDATE_DETAILS to be defined.
#if defined(mtraub) && defined(X_DEBUG) && !defined(LOG_UPDATE_DETAILS)
#define LOG_UPDATE_DETAILS
#endif

// If logging update details, then define a helper macro.
#ifdef LOG_UPDATE_DETAILS
#define LOG(c) Msg[C++] = c
#else
#define LOG(c) ((void)0)
#endif    

//==============================================================================
//	INCLUDES
//==============================================================================

#include "NetworkMgr/NetworkMgr.hpp"
#include "Ghost.hpp"
#include "x_bitstream.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

const s32 g_FramesToBlendPos         = 3;
const s32 g_FramesToBlendOrientation = 3;

//==============================================================================
//  UPDATE FUNCTIONS
//==============================================================================

update::update( void )
{
    DirtyBits = 0;
}

//==============================================================================

void update::Read( const bitstream& BS )
{
    // Read update information from the bitstream into the update record.

    DirtyBits = 0x00000000;

    #ifdef LOG_UPDATE_DETAILS
    // Variables for update logging.
    char   Msg[64];
    s32    C = 0;
    x_memset( Msg, 0, 64 );
    #endif

    LOG('[');
    LOG('0' + Slot / 10);
    LOG('0' + Slot % 10);
    LOG(']');

    if( BS.ReadFlag() )
    {
        DirtyBits |= netobj::DEACTIVATE_BIT;
        LOG('D');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= netobj::ACTIVATE_BIT;
        LOG('A');

    //  TO DO - Read skin information.

    //  BS.ReadRangedS32( Jump   , 0, 7 );
    //  BS.ReadRangedS32( Melee  , 0, 7 );
    //  BS.ReadRangedS32( Reload , 0, 7 );
    //  BS.ReadRangedS32( Fire   , 0, 7 );
    //  BS.ReadRangedS32( Toss   , 0, 7 );
    //  BS.ReadRangedS32( Respawn, 0, 7 );
    //  BS.ReadRangedS32( LifeSeq, 0, 7 );
    }

    /*
    if( BS.ReadFlag() )
    {
        DeltaData  = TRUE;
        DirtyBits |= DELTA_DATA_BIT;
        BS.ReadS32( pDelta[0] );
        BS.ReadS32( pDelta[1] );
        BS.ReadS32( pDelta[2] );
        BS.ReadS32( pDelta[3] );
        BS.ReadS32( pDelta[4] );
        BS.ReadS32( pDelta[5] );
        LOG('D');
    }
    */

    if( BS.ReadFlag() )
    {
        DirtyBits |= POSITION_BIT;
        BS.ReadVector( Position );
        LOG('P');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= ORIENTATION_BIT;
        BS.ReadF32( Pitch );
        BS.ReadF32( Yaw   );
        LOG('O');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= CROUCH_BIT;
        BS.ReadFlag( Crouch );
        LOG(Crouch ? 'c' : 'C');
    }

    /*
    if( BS.ReadFlag() )
    {
        DirtyBits |= JUMP_BIT;
        BS.ReadRangedS32( Jump, 0, 7 );
        LOG('J');
        LOG('0' + Jump);
    }
    */

    if( BS.ReadFlag() )
    {
        DirtyBits |= HEALTH_BIT;
        BS.ReadRangedS32( Health,  0, 500 );
        BS.ReadRangedS32( LifeSeq, 0,   7 );
        LOG('H');
        LOG('(');
        LOG('0' + (Health / 100));
        LOG('0' + (Health % 100) / 10);
        LOG('0' + (Health %  10));
        LOG(')');
    } 

    if( BS.ReadFlag() )
    {
        BS.ReadRangedS32( LifeSeq, 0, 7 );
        DirtyBits |= LIFE_BIT;
        LOG('L');
        LOG('0' + LifeSeq);
    }

    /*
    if( BS.ReadFlag() )
    {
        DirtyBits |= RESPAWN_BIT;
        BS.ReadRangedS32( Respawn, 0, 7 );
        LOG('S');
        LOG('0' + Respawn);
    }
    */

    if( BS.ReadFlag() )
    {
        DirtyBits |= TEAM_BITS_BIT;
        BS.ReadU32( TeamBits );
        LOG('t');
        if( TeamBits == 0x00000001 ) LOG('0'); else 
        if( TeamBits == 0x00000002 ) LOG('1'); else
                                     LOG('?');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= WEAPON_BIT;
        BS.ReadS32( Weapon );
        LOG('W');
        LOG('(');
        LOG('0' + Weapon / 10);
        LOG('0' + Weapon % 10);
        LOG(')');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= RELOAD_BIT;
    //  BS.ReadRangedS32( Reload, 0, 7 );
        LOG('R');
    //  LOG('0' + Reload);
    }

    /*
    AMMO_BIT
    */

    if( BS.ReadFlag() )
    {
        DirtyBits |= FIRE_BIT;
    //  BS.ReadRangedS32( Fire, 0, 7 );
    //  BS.ReadVector   ( FirePos    );
    //  BS.ReadVector   ( FireVel    );
    //  BS.ReadF32      ( FireScalar );
        LOG('F');
    //  LOG('0' + Fire);
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= FIRE_ALT_BIT;
    }

    /*
    FIRE_BEGIN_BIT
    FIRE_END_BIT
    */

    if( BS.ReadFlag() )
    {
        DirtyBits |= TOSS_BIT;
    //  BS.ReadRangedS32( Toss, 0, 7 );
    //  BS.ReadVector   ( TossPos    );
    //  BS.ReadVector   ( TossVel    );
    //  BS.ReadF32      ( TossScalar );
        LOG('T');
    //  LOG('0' + Toss);
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= MELEE_BIT;
    //  BS.ReadRangedS32( Melee, 0, 7 );
        LOG('M');
    //  LOG('0' + Melee);
    }

    if( BS.ReadFlag() )
    {
        LOG('{');

        for( s32 i = 0; i < 8; i++ )
        {
            if( BS.ReadFlag() )
            {
                s32 Angle;
                DirtyBits |= (PAIN_0_BIT << i);
                BS.ReadS32( RecentPain[i].Amount );
                BS.ReadS32( RecentPain[i].Type   );
                BS.ReadRangedS32( Angle, 0, 255  );
                RecentPain[i].Yaw = (radian)(Angle * (R_360/255));
                LOG('0' + i);
            }
        }

        LOG('}');
    }

    #ifdef LOG_UPDATE_DETAILS
    // Most updates include only position and orientation.
    // Only display updates which include more than that.
    if( DirtyBits & ~(POSITION_BIT | ORIENTATION_BIT) )
        LOG_MESSAGE( "update::Read", "Data:%s", Msg );
    #endif        
    
    /*
    if( DeltaData )
    {
        LOG_MESSAGE( "update::Read( DeltaData )", 
                     "... DeltaData:%d,%d,%d,%d,%d,%d",
                     pDelta[0], pDelta[1], 
                     pDelta[2], pDelta[3], 
                     pDelta[4], pDelta[5] );
    }
    */
}

//==============================================================================

void update::Write( bitstream& BS )
{
    // Write the update record to the bitstream.

    ASSERT( IN_RANGE( 0, Slot, 31 ) );

    #ifdef LOG_UPDATE_DETAILS
    // Variables for update logging.
    char   Msg[64];
    s32    C = 0;
    x_memset( Msg, 0, 64 );
    #endif

//  xbool  DeltaData = FALSE;

    // Mask the rolling sequence numbers into range.
    JumpSeq   &= 0x07;
    MeleeSeq  &= 0x07;
    ReloadSeq &= 0x07;
    FireSeq   &= 0x07;
    TossSeq   &= 0x07;
    LifeSeq   &= 0x07;

    LOG('[');
    LOG('0' + Slot / 10);
    LOG('0' + Slot % 10);
    LOG(']');

    if( BS.WriteFlag( DirtyBits & netobj::DEACTIVATE_BIT ) )
    {
        DirtyBits &= ~netobj::DEACTIVATE_BIT;
        LOG('D');
    }

    if( BS.WriteFlag( DirtyBits & netobj::ACTIVATE_BIT ) )
    {
        DirtyBits &= ~netobj::ACTIVATE_BIT;
        LOG('A');

    //  TO DO - Write skin information.

    //  BS.WriteRangedS32( Jump   , 0, 7 );
    //  BS.WriteRangedS32( Melee  , 0, 7 );
    //  BS.WriteRangedS32( Reload , 0, 7 );
    //  BS.WriteRangedS32( Fire   , 0, 7 );
    //  BS.WriteRangedS32( Toss   , 0, 7 );
    //  BS.WriteRangedS32( Respawn, 0, 7 );
    //  BS.WriteRangedS32( LifeSeq   , 0, 7 );

        ASSERT( DirtyBits & POSITION_BIT    );
        ASSERT( DirtyBits & ORIENTATION_BIT );
        ASSERT( DirtyBits & HEALTH_BIT      );
        ASSERT( DirtyBits & LIFE_BIT        );
        ASSERT( DirtyBits & TEAM_BITS_BIT   );
        ASSERT( DirtyBits & WEAPON_BIT      );
    }

    /*
    if( BS.WriteFlag( DirtyBits & DELTA_DATA_BIT ) )
    {
        DeltaData  = TRUE;
        DirtyBits &= ~DELTA_DATA_BIT;
        BS.WriteS32( pDelta[0] );
        BS.WriteS32( pDelta[1] );
        BS.WriteS32( pDelta[2] );
        BS.WriteS32( pDelta[3] );
        BS.WriteS32( pDelta[4] );
        BS.WriteS32( pDelta[5] );
        LOG('D');
    }
    */

    if( BS.WriteFlag( DirtyBits & POSITION_BIT ) )
    {
        DirtyBits &= ~POSITION_BIT;
        BS.WriteVector( Position );
        LOG('P');
    }

    if( BS.WriteFlag( DirtyBits & ORIENTATION_BIT ) )
    {
        DirtyBits &= ~ORIENTATION_BIT;
        BS.WriteF32( Pitch );
        BS.WriteF32( Yaw   );
        LOG('O');
    }

    if( BS.WriteFlag( DirtyBits & CROUCH_BIT ) )
    {
        DirtyBits &= ~CROUCH_BIT;
        BS.WriteFlag( Crouch );
        LOG(Crouch ? 'c' : 'C');
    }

    /*
    if( BS.WriteFlag( DirtyBits & JUMP_BIT ) )
    {
        DirtyBits &= ~JUMP_BIT;
        BS.WriteRangedS32( Jump, 0, 7 );
        LOG('J');
        LOG('0' + Jump);
    }
    */

    if( BS.WriteFlag( DirtyBits & HEALTH_BIT ) )
    {
        DirtyBits &= ~HEALTH_BIT;
        BS.WriteRangedS32( Health,  0, 500 );
        BS.WriteRangedS32( LifeSeq, 0,   7 );
        LOG('H');
        LOG('(');
        LOG('0' + (Health / 100));
        LOG('0' + (Health % 100) / 10);
        LOG('0' + (Health %  10));
        LOG(')');
    } 

    if( BS.WriteFlag( DirtyBits & LIFE_BIT ) )
    {
        DirtyBits &= ~LIFE_BIT;
        BS.WriteRangedS32( LifeSeq, 0, 7 );
        LOG('L');
        LOG('0' + LifeSeq);
    }

    /*
    if( BS.WriteFlag( DirtyBits & RESPAWN_BIT ) )
    {
        DirtyBits &= ~RESPAWN_BIT;
        BS.WriteRangedS32( Respawn, 0, 7 );
        LOG('S');
        LOG('0' + Respawn);
    }
    */

    if( BS.WriteFlag( DirtyBits & TEAM_BITS_BIT ) )
    {
        DirtyBits &= ~TEAM_BITS_BIT;
        BS.WriteU32( TeamBits );
        LOG('t');
        if( TeamBits == 0x00000001 ) LOG('0'); else
        if( TeamBits == 0x00000002 ) LOG('1'); else
                                     LOG('?');
    }

    if( BS.WriteFlag( DirtyBits & WEAPON_BIT ) )
    {
        DirtyBits &= ~WEAPON_BIT;
        BS.WriteS32( Weapon );
        LOG('W');
        LOG('(');
        LOG('0' + Weapon / 10);
        LOG('0' + Weapon % 10);
        LOG(')');
    }

    if( BS.WriteFlag( DirtyBits & RELOAD_BIT ) )
    {
        DirtyBits &= ~RELOAD_BIT;
    //  BS.WriteRangedS32( ReloadSeq, 0, 7 );
        LOG('R');
    //  LOG('0' + Reload);
    }

    /*
    AMMO_BIT
    */

    if( BS.WriteFlag( DirtyBits & FIRE_BIT ) )
    {
        DirtyBits &= ~FIRE_BIT;
    //  BS.WriteRangedS32( FireSeq, 0, 7 );
    //  BS.WriteVector   ( FirePos    );
    //  BS.WriteVector   ( FireVel    );
    //  BS.WriteF32      ( FireScalar );
        LOG('F');
    //  LOG('0' + Fire);
    }

    if( BS.WriteFlag( DirtyBits & FIRE_ALT_BIT ) )
    {
        DirtyBits &= ~FIRE_ALT_BIT;
        LOG('f');
    }

    /*
    FIRE_BEGIN_BIT
    FIRE_END_BIT
    */

    if( BS.WriteFlag( DirtyBits & TOSS_BIT ) )
    {
        DirtyBits &= ~TOSS_BIT;
    //  BS.WriteRangedS32( TossSeq, 0, 7 );
    //  BS.WriteVector   ( TossPos    );
    //  BS.WriteVector   ( TossVel    );
    //  BS.WriteF32      ( TossScalar );
        LOG('T');
    //  LOG('0' + Toss);
    }
    
    if( BS.WriteFlag( DirtyBits & MELEE_BIT ) )
    {
        DirtyBits &= ~MELEE_BIT;
    //  BS.WriteRangedS32( MeleeSeq, 0, 7 );
        LOG('M');
    //  LOG('0' + Melee);
    }

    if( BS.WriteFlag( DirtyBits & PAIN_ANY_BIT ) )
    {
        LOG('{');

        for( s32 i = 0; i < 8; i++ )
        {
            if( BS.WriteFlag( DirtyBits & (PAIN_0_BIT << i) ) )
            {
                s32 Angle = (s32)(RecentPain[i].Yaw * (255/R_360));
                DirtyBits &= ~(PAIN_0_BIT << i);
                BS.WriteS32( RecentPain[i].Amount );
                BS.WriteS32( RecentPain[i].Type   );
                BS.WriteRangedS32( Angle, 0, 255 );
                LOG('0' + i);
            }
        }

        LOG('}');
    }

    #ifdef LOG_UPDATE_DETAILS
    // Most updates include only position and orientation.
    // Only display updates which include more than that.
    if( C > 6 )
        LOG_MESSAGE( "update::Write", "Data:%s", Msg );
    #endif        

    /*
    if( DeltaData )
    {
        LOG_MESSAGE( "update::Write( DeltaData )", 
                     "... DeltaData:%d,%d,%d,%d,%d,%d",
                     pDelta[0], pDelta[1], 
                     pDelta[2], pDelta[3], 
                     pDelta[4], pDelta[5] );
    }
    */
}

//==============================================================================
//	GHOST FUNCTIONS
//==============================================================================

void ghost::net_Activate( void )
{
    s32 i;

    actor::InitInventory();  // HACKOMOTRON - Tried to do it the "right" way.  Didn't work.

    m_NetDirtyBits = ACTIVATE_BIT;
    m_NetModeBits  = 0;

    for( i = 0; i < NDELTA; i++ )
        m_NetDelta[i] = 0;

    if( g_NetworkMgr.IsServer() )   
        m_NetModeBits |= ON_SERVER;
    else                            
        m_NetModeBits |= ON_CLIENT;

    if( g_NetworkMgr.IsLocal( m_NetSlot ) )
        m_NetModeBits |= CONTROL_LOCAL;
    else                            
        m_NetModeBits |= CONTROL_REMOTE;

    m_Net.pMoveMgr = g_NetworkMgr.GetMoveMgr( m_NetSlot );

    m_Net.BlendPosX .Init( 1.000f, 500.0f, g_FramesToBlendPos        , FALSE );
    m_Net.BlendPosY .Init( 1.000f, 500.0f, g_FramesToBlendPos        , FALSE );
    m_Net.BlendPosZ .Init( 1.000f, 500.0f, g_FramesToBlendPos        , FALSE );
    m_Net.BlendPitch.Init( 0.001f,     PI, g_FramesToBlendOrientation, TRUE  );
    m_Net.BlendYaw  .Init( 0.001f,     PI, g_FramesToBlendOrientation, TRUE  );

    m_Net.FireSeq = 0;
    m_Net.PainSeq = 0;

    for( i = 0; i < 8; i++ )
    {
        m_Net.RecentPain[0].Amount = 0;
        m_Net.RecentPain[0].Type   = 0;
        m_Net.RecentPain[0].Yaw    = R_0;
    }

    LOG_MESSAGE( "ghost::net_Activate",
                 "Addr:%08X - NetSlot:%d - Status:%s on %s",
                 this, m_NetSlot,
                 (m_NetModeBits & CONTROL_LOCAL) ? "LOCAL"  : "REMOTE",
                 (m_NetModeBits & ON_SERVER    ) ? "SERVER" : "CLIENT" );
}

//==============================================================================

void ghost::net_AcceptUpdate( const bitstream& BS )
{
    update Update;

    Update.Slot = m_NetSlot;
    Update.Read( BS );

    u32& DirtyBits = Update.DirtyBits;

    // Don't need to worry about ACTIVATE_BIT here.  By the time we get here,
    // the activation has been converted into other dirty bits.

    // Don't need to worry about DEACTIVATE_BIT here.  That is handled by the
    // client's update manager.

    /*
    DELTA_DATA_BIT
    */

    if( DirtyBits & POSITION_BIT )
    {
        m_Net.Actual.Position = Update.Position;
        m_Net.BlendPosX.SetTarget( m_Net.Actual.Position.GetX() );
        m_Net.BlendPosY.SetTarget( m_Net.Actual.Position.GetY() );
        m_Net.BlendPosZ.SetTarget( m_Net.Actual.Position.GetZ() );
    }

    if( DirtyBits & ORIENTATION_BIT )
    {
        m_Net.Actual.Pitch = Update.Pitch;
        m_Net.Actual.Yaw   = Update.Yaw;  
        m_Net.BlendPitch.SetTarget( m_Net.Actual.Pitch );
        m_Net.BlendYaw  .SetTarget( m_Net.Actual.Yaw   );
    }

    /*
    CROUCH_BIT
    JUMP_BIT
    */
    
    if( DirtyBits & HEALTH_BIT )
    {
        // Only allow health to DECREASE via an update.
        if( m_Health > Update.Health )
        {
            m_Health = (f32)Update.Health;
        }
    }

    if( DirtyBits & LIFE_BIT )
    {
        xbool UpdateSaysDead = Update.LifeSeq & 0x01;

        // Spawn?
        if( !UpdateSaysDead && m_Dead )
        {
            ASSERT( DirtyBits & POSITION_BIT );
            ASSERT( DirtyBits & ORIENTATION_BIT );

            OnSpawn();
        }

        // Die?
        if( UpdateSaysDead && !m_Dead )
        {
            OnDeath();
        }

        // Sanity?
        if( Update.LifeSeq != (m_Net.LifeSeq & 0x07) )
        {
            LOG_WARNING( "ghost::net_AcceptUpdate",
                         "Unexpected value for LifeSeq." );
        }

        m_Net.LifeSeq = Update.LifeSeq;
    }

    /*
    SPAWN_BIT
    */

    if( DirtyBits & TEAM_BITS_BIT )
    {
        net_SetTeamBits( Update.TeamBits );
    }

    if( DirtyBits & WEAPON_BIT )
    {
        net_EquipWeapon( (inventory_item::inv_type)Update.Weapon );
    }

    /*
    RELOAD_BIT
    AMMO_BIT
    */

    if( DirtyBits & FIRE_BIT )
    {
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
        {
            pWeapon->FireGhostPrimary();
        }
    }

    /*
    FIRE_ALT_BIT
    FIRE_BEGIN_BIT
    FIRE_END_BIT
    */

    if( DirtyBits & TOSS_BIT )
    {
        x_DebugMsg( "TOSS\n" );
        /*
        weapon_event Event;
        Event.Type        = event::EVENT_WEAPON;
        Event.Pos         = GetEyesPosition();
        Event.WeaponState = new_weapon::EVENT_FIRE;

        OnEvent( Event );
        */
        // TO DO -- Need to add a very simplified version of OnEvent to
        // class ghost to fire the weapon.
    }

    if( DirtyBits & MELEE_BIT )
    {
        x_DebugMsg( "MELEE\n" );

        // TO DO -- Get the loco to play a melee animation.
    }

    /*
    PAIN_ANY_BIT
    */
}

//==============================================================================

void ghost::net_ProvideUpdate(       bitstream& BS, 
                                     u32&       DirtyBits, 
                                     s32        Client, 
                               const delta*     pDelta )
{
	(void)Client;
	(void)pDelta;

    update Update;
    Update.Slot = m_NetSlot;

    //
    // Preprocessing...
    //

    // If the ACTIVATE_BIT is set, then we must include several other bits of 
    // data.
    if( DirtyBits & ACTIVATE_BIT )
    {
        DirtyBits |= POSITION_BIT;
        DirtyBits |= ORIENTATION_BIT;
        DirtyBits |= HEALTH_BIT;
        DirtyBits |= LIFE_BIT;
        DirtyBits |= TEAM_BITS_BIT;
        DirtyBits |= WEAPON_BIT;
    }

    // If the LIFE_BIT is set, then we must include the position and
    // orientation data.
    if( DirtyBits & LIFE_BIT )
    {
        DirtyBits |= POSITION_BIT;
        DirtyBits |= ORIENTATION_BIT;
    }

    //
    // Let's rock!  For every dirty bit, move the appropriate data into the
    // update structure.  Also, set the corresponding dirty bit in the update.
    //

    if( DirtyBits & ACTIVATE_BIT )
    {
        // Pass the bit along to the update.
        Update.DirtyBits |= ACTIVATE_BIT;
    }

    // Don't need to worry about DEACTIVATE_BIT.  If the object has been 
    // deactivated, then we would never be here inside of a ghost function.

    /*
    DELTA_DATA_BIT
    */

    if( DirtyBits & POSITION_BIT )
    {
        Update.Position   = GetPosition();
    	Update.DirtyBits |= POSITION_BIT;
    }

    if( DirtyBits & ORIENTATION_BIT )
    {
        Update.Pitch      = GetPitch();
        Update.Yaw        = GetYaw();
	    Update.DirtyBits |= ORIENTATION_BIT;
    }

    /*
    CROUCH_BIT
    JUMP_BIT
    */
    
    if( DirtyBits & HEALTH_BIT )
    {
        Update.Health     = (s32)m_Health;
        Update.DirtyBits |= HEALTH_BIT;
    }

    if( DirtyBits & LIFE_BIT )
    {
        ASSERT( (m_Net.LifeSeq&0x01) == m_Dead );
        ASSERT( DirtyBits & POSITION_BIT );
        ASSERT( DirtyBits & ORIENTATION_BIT );

        Update.LifeSeq    = m_Net.LifeSeq;
        Update.DirtyBits |= LIFE_BIT;
    }

    /*
    SPAWN_BIT
    */

    if( DirtyBits & TEAM_BITS_BIT )
    {
        Update.TeamBits   = m_NetTeamBits;
        Update.DirtyBits |= TEAM_BITS_BIT;
    }

    if( DirtyBits & WEAPON_BIT )
    {
        Update.Weapon     = GetCurrentWeaponType();
        Update.DirtyBits |= WEAPON_BIT;
    }

    if( DirtyBits & RELOAD_BIT )
    {
        Update.DirtyBits |= RELOAD_BIT;
    }

    /*
    AMMO_BIT
    */

    if( DirtyBits & FIRE_BIT )
    {
        Update.DirtyBits |= FIRE_BIT;
    }

    if( DirtyBits & FIRE_ALT_BIT )
    {
        Update.DirtyBits |= FIRE_ALT_BIT;
    }

    if( DirtyBits & FIRE_BEGIN_BIT )
    {
        Update.DirtyBits |= FIRE_BEGIN_BIT;
    }

    if( DirtyBits & FIRE_END_BIT )
    {
        Update.DirtyBits |= FIRE_END_BIT;
    }

    if( DirtyBits & TOSS_BIT )
    {
        Update.DirtyBits |= TOSS_BIT;
    }

    if( DirtyBits & MELEE_BIT )
    {
        Update.DirtyBits |= MELEE_BIT;
    }

    if( DirtyBits & PAIN_ANY_BIT )
    {        
        for( s32 i = 0; i < 8; i++ )
        {
            Update.RecentPain[i] = m_Net.RecentPain[i];
        }
        Update.DirtyBits |= (DirtyBits & PAIN_ANY_BIT);
    }

    //=================================
    // Write the update.

    if( Update.DirtyBits )
    {
        Update.Write( BS );
	    DirtyBits = Update.DirtyBits;
    }
}

//==============================================================================

void ghost::net_SetTeamBits( u32 TeamBits )
{
    LOG_MESSAGE( "ghost::net_SetTeamBits", "Player:%d - TeamBits:%08X", 
                 m_NetSlot, TeamBits );

    if( m_NetTeamBits != TeamBits )
    {
        m_NetDirtyBits |= TEAM_BITS_BIT;
        m_NetTeamBits   = TeamBits;
    }
}

//==============================================================================

guid ghost::net_GetGameGuid( void ) const
{
    return( object::GetGuid() );
}

//==============================================================================

s32 ghost::net_GetGameSlot( void ) const
{
    return( object::GetSlot() );
}

//==============================================================================

void ghost::net_Logic( void )
{
    switch( m_NetModeBits )
    {   
    case LOCAL_ON_SERVER:   ASSERT( FALSE );    break;
    case LOCAL_ON_CLIENT:   ASSERT( FALSE );    break;

    case GHOST_ON_SERVER:
        {
            // Remotely controlled player on the server.
            //  - Apply ALL moves received from the controlling client.
            //  - Run the blending.

            // Apply ALL moves!
            while( m_Net.pMoveMgr->GetCount() > 0 )
            {
                move Move;
                m_Net.pMoveMgr->GetMove( Move );
                net_ApplyMove( Move );
            }

            if( !m_Dead )
            {
                m_Net.Render.Position.Set( m_Net.BlendPosX.BlendLogic(),
                                           m_Net.BlendPosY.BlendLogic(),
                                           m_Net.BlendPosZ.BlendLogic() );
                m_Net.Render.Pitch = m_Net.BlendPitch.BlendLogic();
                m_Net.Render.Yaw   = m_Net.BlendYaw  .BlendLogic();

                Teleport( m_Net.Render.Position );
                SetPitch( m_Net.Render.Pitch    );
                SetYaw  ( m_Net.Render.Yaw      );
            }

            /*
            LOG_MESSAGE( "ghost::net_Logic",
                         "Slot:%d - Mode:GHOST ON SERVER - DirtyBits:%08X - Moves:%d", 
                         m_NetSlot, m_NetDirtyBits, m_Net.pMoveMgr->GetCount() );
            */
        }
        break;

    case GHOST_ON_CLIENT:
        {
            // Remotely controlled player on a client.
            //  - Run the blending.

            if( !m_Dead )
            {
                m_Net.Render.Position.Set( m_Net.BlendPosX.BlendLogic(),
                                           m_Net.BlendPosY.BlendLogic(),
                                           m_Net.BlendPosZ.BlendLogic() );
                m_Net.Render.Pitch = m_Net.BlendPitch.BlendLogic();
                m_Net.Render.Yaw   = m_Net.BlendYaw  .BlendLogic();

                Teleport( m_Net.Render.Position );
                SetPitch( m_Net.Render.Pitch    );
                SetYaw  ( m_Net.Render.Yaw      );
            }

            /*
            LOG_MESSAGE( "ghost::net_Logic",
                         "Slot:%d - Mode:GHOST ON CLIENT", m_NetSlot );
            */
        }
        break;
    }
}

//==============================================================================

void ghost::net_ApplyMove( const move& Move )
{
    ASSERT( m_NetModeBits == GHOST_ON_SERVER );

    /*
    LOG_MESSAGE( "ghost::net_ApplyMove",
                 "Slot:%d - Seq:%d",
                 m_NetSlot, Move.Seq );
    */

    if( m_Dead && Move.Respawn )
        m_WantToSpawn = TRUE;

    vector3 DeltaPos   = m_Net.Actual.Position - Move.Position;
    radian  DeltaPitch = m_Net.Actual.Pitch    - Move.Pitch;
    radian  DeltaYaw   = m_Net.Actual.Yaw      - Move.Yaw;  

    if( (m_Net.LifeSeq & 0x07) == Move.LifeSeq )
    {
        if( !IN_RANGE( -0.5f, DeltaPos.GetX(), 0.5f ) ||  // Half a centimeter.
            !IN_RANGE( -0.5f, DeltaPos.GetY(), 0.5f ) ||
            !IN_RANGE( -0.5f, DeltaPos.GetZ(), 0.5f ) ) 
        {
            m_NetDirtyBits |= POSITION_BIT;
            m_Net.Actual.Position = Move.Position;
            m_Net.BlendPosX.SetTarget( m_Net.Actual.Position.GetX() );
            m_Net.BlendPosY.SetTarget( m_Net.Actual.Position.GetY() );
            m_Net.BlendPosZ.SetTarget( m_Net.Actual.Position.GetZ() );
        }

        if( !IN_RANGE( -0.001f, DeltaPitch, 0.001f ) ||
            !IN_RANGE( -0.001f, DeltaYaw,   0.001f ) )
        {
            m_NetDirtyBits     |= ORIENTATION_BIT;
            m_Net.Actual.Pitch  = Move.Pitch;
            m_Net.Actual.Yaw    = Move.Yaw;       
            m_Net.BlendPitch.SetTarget( m_Net.Actual.Pitch );
            m_Net.BlendYaw  .SetTarget( m_Net.Actual.Yaw   );
        }  

        if( Move.Weapon != GetCurrentWeaponType() )
        {
            net_EquipWeapon( (inventory_item::inv_type)Move.Weapon );
            m_NetDirtyBits |= WEAPON_BIT;
        }

        if( Move.FireSeq != (m_Net.FireSeq & 0x07) )
        {
            new_weapon* pWeapon = GetCurrentWeaponPtr();
            if( pWeapon )
            {
                pWeapon->FireGhostPrimary();    // Fire weapon!
                m_Net.FireSeq   = Move.FireSeq; // MAYBE - m_Net.FireSeq++;
                m_NetDirtyBits |= FIRE_BIT;     // Dirty bit for other clients.
            }
        }
    }
    else
    {
        m_NetDirtyBits |= LIFE_BIT;

        /*
        LOG_MESSAGE( "ghost::net_ApplyMove",
                     "LifeSeq:%d - Move.LifeSeq:%d - Mismatch! - Movement not applied!",
                     m_Net.LifeSeq, Move.LifeSeq );
        */
    }

    for( s32 i = 0; i < 32; i++ )
    {
        if( Move.Pain[i].Amount > 0 )
        {
            LOG_MESSAGE( "ghost::net_ApplyMove",
                         "PAIN -- Source:%d - Victim:%d - Amount:%d",
                         m_NetSlot, i, Move.Pain[i].Amount );

            netobj* pNetObj = NetObjMgr.GetObjFromSlot( i );
            if( pNetObj )
            {                
                object* pObject = g_ObjMgr.GetObjectBySlot( pNetObj->net_GetGameSlot() );
                if( pObject && pObject->IsKindOf( ghost::GetRTTI() ) )
                {
                    ghost* pGhost = (ghost*)pObject;
                    pGhost->net_ApplyPain( m_NetSlot, 
                                           Move.Pain[i].Amount, 
                                           Move.Pain[i].Type );
                }
            }
        }
    }
}

//=============================================================================

xbool ghost::net_EquipWeapon( inventory_item::inv_type WeaponType )
{
    if( (WeaponType < inventory_item::INV_FIRST_WEAPON) || 
        (WeaponType > inventory_item::INV_LAST_WEAPON) )
    {
        return( FALSE );
    }

    for( s32 i = 0; i < m_WeaponInventory.GetTotalPotentialCount(); i++ )
    {
        if ( m_WeaponInventory.GetCollapsedListPtr(i)->m_ItemCount > 0 )
        {
            guid WeaponGuid = m_WeaponInventory.GetCollapsedListPtr(i)->m_GuidList[0];
            ASSERT( WeaponGuid != NULL );
            
            if( WeaponGuid != m_Net.WeaponGuid )
            {
                new_weapon* pNewWeapon;
                new_weapon* pOldWeapon;

                // Activate the new weapon.

                pNewWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( WeaponGuid );
                ASSERT( pNewWeapon );

                if( pNewWeapon->GetInvType() == WeaponType )
                {
                    // But first, deactivate the old weapon.
                    if( m_Net.WeaponGuid != NULL )
                    {
                        pOldWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( m_Net.WeaponGuid );
                        ASSERT( pOldWeapon );
                        pOldWeapon->SetVisible( FALSE );
                        m_Net.WeaponGuid = NULL;
                    }

                    // Now activate.
                    pNewWeapon->InitWeapon( GetPosition(), new_weapon::RENDER_STATE_NPC, GetGuid() ); 
                    pNewWeapon->SetVisible( TRUE );
                    m_Net.WeaponGuid = WeaponGuid;

                    return( TRUE );
                }
            }
        }
    }

    return( FALSE );
}

//==============================================================================

void ghost::net_CausedPain( s32 , s32 , s32  )
{
    // Since ghosts don't report back to the server, there's nothing to record.
}

//==============================================================================
// Should/could this function simply turn into call to ghost::OnPain?  Note that
// there aren't enough parameters at present to completely create a full pain.
//
// Currently called when:
//  - Ghost on the server receives a move from his remote player which lists
//    pain to somebody else (namey THIS player/ghost).  net_ApplyMove()
//
// Two tasks:
//  - Adjust the health and set the HEALTH_BIT so the ghost's remote player (on
//    a client) will get the new, lower health value.
//  - Log the pain in the Recent Pain list and set a PAIN_n_BIT so the ghost's
//    remote player (on a client) will receive the pain data and can display
//    a "hit" on the HUD.

void ghost::net_ApplyPain( s32 Source, s32 Amount, s32 Type )
{
    pain    Pain;     
    netobj* pNetObj;

    if( m_Dead )
        return;

    pNetObj = NetObjMgr.GetObjFromSlot( Source );
    if( pNetObj )
    {
        Pain.Origin = pNetObj->net_GetGameGuid();
    }

    Pain.Type = (pain::type)Type;

    // Hurt the player.
    ASSERT( Amount >= 0 );
    AddHealth( (const f32)-Amount );
    if( m_Health <= 0.0f )
    {
        m_PainDeathEvent = Pain;
        OnDeath(); // You are DEAD!
    }

    // Play an impact anim.
    const eHitType HitType = GetHitType(Pain);
    PlayImpactAnim( Pain, HitType );

    // Audio?
    // Effects?

    m_Net.PainSeq = (m_Net.PainSeq + 1) & 0x07;

    m_NetDirtyBits |=  HEALTH_BIT;
    m_NetDirtyBits |= (PAIN_0_BIT << m_Net.PainSeq);

    m_Net.RecentPain[m_Net.PainSeq].Amount = Amount;
    m_Net.RecentPain[m_Net.PainSeq].Type   = Type;
    m_Net.RecentPain[m_Net.PainSeq].Yaw    = R_0;   // TO DO - Yaw.

    LOG_MESSAGE( "ghost::net_ApplyPain",
                 "Source:%d - Victim:%d - Amount:%d - Type:%d - Health:%d",
                 Source, m_NetSlot, Amount, Type, (s32)m_Health );
}

//==============================================================================
#endif
