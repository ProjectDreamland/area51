//==============================================================================
//
//  ActorNet.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
//	DEBUG DEFINES AND MACROS
//==============================================================================

// Comment or uncomment to log update details.
//#define LOG_UPDATE_DETAILS

// If X_DEBUG is defined, then force LOG_UPDATE_DETAILS to be defined.
#if defined(mtraub) && defined(X_DEBUG) && !defined(LOG_UPDATE_DETAILS)
//#define LOG_UPDATE_DETAILS
#endif

// If logging update details, then define a helper macro.
#ifdef LOG_UPDATE_DETAILS
#define LOG(c) Msg[C++] = c
#else
#define LOG(c) ((void)0)
#endif    

#define PAIN_FEEDBACK_FORCE 1.5f

//==============================================================================
//	INCLUDES
//==============================================================================

#include "Actor.hpp"
#include "x_bitstream.hpp"
#include "Objects/Event.hpp"
#include "Objects\PlayerLoco.hpp"
#include "Characters\Grunt\GruntLoco.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/GameMgr.hpp"
#include "NetworkMgr/PainQueue.hpp"
#include "Inventory/Inventory2.hpp"
#include "Characters\ActorEffects.hpp"
#include "Objects\Player.hpp"

//==============================================================================

static s32 s_NetEffects[] = 
{ 
    actor_effects::FX_FLAME,
    actor_effects::FX_SHOCK,
//  actor_effects::FX_CLOAK,
//  actor_effects::FX_DECLOAK,
};

//==============================================================================
//  UPDATE FUNCTIONS
//==============================================================================

update::update( void )
{
#ifdef X_DEBUG
    // SB: 2/24/05
    // Clear to invalid value for debug to catch data not setup
    s32 Size = sizeof( update );
    ASSERT( ( Size & 3 ) == 0 );    // Must be 4 byte aligned
    Size /= 4;
    u32* pThis = (u32*)this;
    for( s32 i = 0; i < Size; i++ )
    {
        pThis[i] = 0xFEEDC0DE;
    }
#endif    

    DirtyBits = 0x00000000;
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

        // Read skin and voice actor.
        BS.ReadRangedS32( Skin,       0, SKIN_COUNT );
        BS.ReadRangedS32( VoiceActor, 0, 3          );
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= POSITION_BIT;
        
        bbox BBox( g_ObjMgr.GetSafeBBox() );
        BS.ReadRangedF32( Position.GetX(), 16, BBox.Min.GetX(), BBox.Max.GetX() );
        BS.ReadRangedF32( Position.GetY(), 16, BBox.Min.GetY(), BBox.Max.GetY() );
        BS.ReadRangedF32( Position.GetZ(), 16, BBox.Min.GetZ(), BBox.Max.GetZ() );

        LOG('P');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= ORIENTATION_BIT;
        if( BS.ReadFlag() )
        {
            xbool Negative = BS.ReadFlag();
            BS.ReadRangedF32( Pitch, 10, R_0,  R_90 );
            BS.ReadRangedF32( Yaw  , 12, R_0, R_360 );
            if( Negative )
                Pitch = -Pitch;

            TargetNetSlot = -1;
        }
        else
        {
            s32 OffsetX;
            s32 OffsetY; 
            s32 OffsetZ; 

            BS.ReadRangedS32( TargetNetSlot, 0, 31 );

            BS.ReadRangedS32( OffsetX, 0, 511 );
            BS.ReadRangedS32( OffsetY, 0, 511 );
            BS.ReadRangedS32( OffsetZ, 0, 511 );              

            AimOffset.GetX() = (f32)(OffsetX - 256);
            AimOffset.GetY() = (f32)(OffsetY);
            AimOffset.GetZ() = (f32)(OffsetZ - 256);
        }
        LOG('O');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= AIR_CROUCH_BIT;

        BS.ReadFlag( Crouch );
        BS.ReadFlag( Airborn );

        LOG(Crouch ? 'c' : 'C');

        LOG('J');
        LOG('0' + Airborn);
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= CONTAGION_OFF_BIT;
        LOG('~');
        LOG('-');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= HEALTH_BIT;
        BS.ReadF32( IncHealth );
        BS.ReadF32( DecHealth );
        #ifdef LOG_UPDATE_DETAILS
        s32 Health = (s32)(IncHealth - DecHealth);
        #endif
        LOG('H');
        LOG('(');
        LOG('0' + (Health / 100));
        LOG('0' + (Health % 100) / 10);
        LOG('0' + (Health %  10));
        LOG(')');
    } 

    // Always read the LifeSeq.
    BS.ReadRangedS32( LifeSeq, 0, 7 );

    // Read corpse death pain?
    if( LifeSeq & 0x01 )	// Odd = Death
    {
        CorpseDeathPain.Read( BS );
    }
    
    if( BS.ReadFlag() )
    {
        DirtyBits |= LIFE_BIT;

        if( (LifeSeq & 0x01) == 0 )
        {
            // Spawn information.
            BS.ReadU32      ( WeaponBits, INVEN_WEAPON_LAST+1 );
            BS.ReadRangedS32( AmmoSMP      , 0, 750 );
            BS.ReadRangedS32( AmmoShotgun  , 0, 120 );
            BS.ReadRangedS32( AmmoSniper   , 0,  30 );
            BS.ReadRangedS32( AmmoEagle    , 0, 120 );
            BS.ReadRangedS32( AmmoMSN      , 0,   3 );
            BS.ReadRangedS32( AmmoGrensFrag, 0,   7 );
            BS.ReadRangedS32( AmmoGrensJB  , 0,   7 );
        }

        LOG('L');
        LOG('0' + LifeSeq);

        LOG('w');
        LOG('(');
        for( s32 i = 0; i < 10; i++ )
            if( WeaponBits & (1<<i) )
                LOG('0' + i);
        LOG(')');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= TEAM_BITS_BIT;
        BS.ReadU32( TeamBits );
        LOG('t');
        if( TeamBits == 0x00000001 ) LOG('0'); else 
        if( TeamBits == 0x00000002 ) LOG('1'); else
                                     LOG('*');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= LEAN_BIT;

        if( BS.ReadFlag() )     Lean = 0.0f;
        else                    BS.ReadRangedF32( Lean, 5, -1.0f, 1.0f );
        
        LOG('l');
        if( Lean >  0 ) LOG('\\');
        if( Lean == 0 ) LOG('|');
        if( Lean <  0 ) LOG('/');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= MUTATE_BIT;
        BS.ReadFlag( Mutated );
        LOG('M');
        LOG('0' + Mutated);
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= WEAPON_BIT;
        BS.ReadRangedS32( Weapon, INVEN_NULL, INVEN_WEAPON_LAST );
        LOG('W');
        LOG('0' + Weapon % 10);
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= RELOAD_BIT;
        LOG('R');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= CURRENT_AMMO_BIT;
        BS.ReadRangedS32( CurrentAmmo, 0, 1000 );
        LOG('(');
        LOG('0' + CurrentAmmo / 10);
        LOG('0' + CurrentAmmo % 10);
        LOG(')');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= FIRE_BIT;
        BS.ReadRangedS32( CurrentFireMode, 0, 1 );
        BS.ReadRangedS32( CurrentFireWeapon, INVEN_WEAPON_FIRST, INVEN_WEAPON_LAST );
        LOG('F');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= FIRE_BEGIN_BIT;
        BS.ReadRangedS32( CurrentFireWeapon, INVEN_WEAPON_FIRST, INVEN_WEAPON_LAST );
        BS.ReadRangedS32( CurrentFireSeq, 0, 7 );
        LOG('/');
        LOG('F');
        LOG('\\');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= FIRE_END_BIT;
        BS.ReadRangedS32( CurrentFireSeq, 0, 7 );
        
        LOG('\\');
        LOG('F');
        LOG('/');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= FLASHLIGHT_BIT;
        BS.ReadFlag( Flashlight );
        LOG('f');
        LOG(Flashlight?'+':'-');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= TOSS_BIT;
        
        BS.ReadRangedS32( CurrentFragGrenades,  0, 7 );
        BS.ReadRangedS32( CurrentJBeanGrenades, 0, 7 );
        LOG('T');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= MELEE_BIT;
        LOG('M');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= DROP_ITEM_BIT;
        LOG('d');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= PLAY_ANIM_BIT;
        BS.ReadS32      ( iAnim, 9 );
        BS.ReadRangedF32( AnimBlendTime, 8, 0.0f, 5.0f );
        BS.ReadU32      ( AnimFlags );
        BS.ReadRangedF32( AnimPlayTime, 8, 0.0f, 5.0f );
        LOG('a');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= WANT_NEW_TEAM_BIT;
        BS.ReadRangedS32( DesiredTeam, 0, 1 );
        LOG('#');
        LOG('0'+DesiredTeam);
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= MUTATION_FLAGS_BIT;
        BS.ReadFlag( ForceHuman );
        BS.ReadFlag( ForceMutant );
        BS.ReadRangedS32( MutagenBurnMode, actor::MBM_FORCED, actor::MBM_AT_WILL );
        if( ForceHuman )
        {
            LOG('<');  LOG('H');  LOG('>');
        }
        if( ForceMutant )
        {
            LOG('<');  LOG('M');  LOG('>');
        }
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= WAYPOINT_BIT;
        WayPointFlags = 0;
        if( BS.ReadFlag() )
        {
            WayPointFlags |= WAYPOINT_TELEPORT;
        }
        if( BS.ReadFlag() )
        {
            WayPointFlags |= WAYPOINT_TELEPORT_FX;
            BS.ReadVector( WayPoint[0] );
            BS.ReadVector( WayPoint[1] );
        }
        if( BS.ReadFlag() )
        {
            WayPointFlags |= WAYPOINT_JUMP_PAD_FX;
            BS.ReadVector( WayPoint[0] );
        }
        LOG('@');
        LOG('0' + WayPointFlags);
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= EFFECTS_BIT;
        BS.ReadU32( EffectsFlags, (sizeof(s_NetEffects) >> 2) );
        LOG('E');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= WANT_SPAWN_BIT;
        LOG('S');
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= VOTE_FLAGS_BIT;
        BS.ReadFlag( VoteCanCast );
        BS.ReadU32 ( VoteCanStartKick );
        BS.ReadFlag( VoteCanStartMap );
        LOG('v');
        LOG('0' + VoteCanCast);
        LOG('0' + !!(VoteCanStartKick));
        LOG('0' + VoteCanStartMap);
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= VOTE_ACTION_BIT;
        BS.ReadRangedS32( VoteAction, 0, 3 );
        switch( VoteAction )
        {
        case 0:                                                 break;
        case 1:  BS.ReadRangedS32     ( VoteArgument, -1, +1 ); break;
        case 2:  BS.ReadRangedS32     ( VoteArgument,  0, 31 ); break;
        case 3:  BS.ReadVariableLenS32( VoteArgument );         break;
        default: ASSERT( FALSE );
        }
        LOG('V');
        LOG('0' + VoteAction);
    }

    if( BS.ReadFlag() )
    {
        DirtyBits |= CONTAGION_ON_BIT;
        BS.ReadRangedS32( ContagionOrigin, 0, 31 );
        LOG('~');
        LOG('+');
        LOG('0' + ContagionOrigin);
    }

#ifdef LOG_UPDATE_DETAILS
    // Most updates include only position and orientation.
    // Only display updates which include more than that.
    if( DirtyBits & ~(POSITION_BIT | ORIENTATION_BIT) )
    {
        LOG_MESSAGE( "update::Read", "Data:%s", Msg );
        LOG_FLUSH();
    }
#endif        
}

//==============================================================================

void update::Write( bitstream& BS )
{
    // Write the update record to the bitstream.

    ASSERT( IN_RANGE( 0, Slot, 31 ) );
    ASSERT( !( (DirtyBits & netobj::ACTIVATE_BIT) && 
               (DirtyBits & netobj::DEACTIVATE_BIT) ) );

    #ifdef LOG_UPDATE_DETAILS
    // Variables for update logging.
    char   Msg[64];
    s32    C = 0;
    xbool  L = (DirtyBits & ~(POSITION_BIT | ORIENTATION_BIT));
    x_memset( Msg, 0, 64 );
    #endif

    // Tracking variables...
    xbool GotWeapon = FALSE;

    // Mask the rolling sequence numbers into range.
    LifeSeq &= 0x07;

    // Sending spawn information?
    xbool bSpawn = ( DirtyBits & LIFE_BIT ) && ( ( LifeSeq & 0x01 ) == 0 );

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

        // Write skin and voice actor.
        BS.WriteRangedS32( Skin,       0, SKIN_COUNT );
        BS.WriteRangedS32( VoiceActor, 0, 3          );

        ASSERT( DirtyBits & POSITION_BIT    );
        ASSERT( DirtyBits & ORIENTATION_BIT );
        ASSERT( DirtyBits & HEALTH_BIT      );
        ASSERT( DirtyBits & LIFE_BIT        );
        ASSERT( DirtyBits & TEAM_BITS_BIT   );
        ASSERT( DirtyBits & WEAPON_BIT      );
        ASSERT( DirtyBits & VOTE_FLAGS_BIT  );
    }

    if( BS.WriteFlag( DirtyBits & POSITION_BIT ) )
    {
        DirtyBits &= ~POSITION_BIT;
        
        bbox BBox( g_ObjMgr.GetSafeBBox() );
        BS.WriteRangedF32( Position.GetX(), 16, BBox.Min.GetX(), BBox.Max.GetX() );
        BS.WriteRangedF32( Position.GetY(), 16, BBox.Min.GetY(), BBox.Max.GetY() );
        BS.WriteRangedF32( Position.GetZ(), 16, BBox.Min.GetZ(), BBox.Max.GetZ() );

        LOG('P');
    }

    if( BS.WriteFlag( DirtyBits & ORIENTATION_BIT ) )
    {
        DirtyBits &= ~ORIENTATION_BIT;
        
        // If spawning or have no target, always write absolute pitch/yaw
        if( BS.WriteFlag( ( bSpawn ) || ( TargetNetSlot == -1 ) ) )
        {
            ASSERT( *(u32*)&Pitch != 0xFEEDC0DE );
            ASSERT( *(u32*)&Yaw   != 0xFEEDC0DE );
            
            Pitch = x_clamp( Pitch, -R_90, R_90 );
            Yaw   = x_ModAngle( Yaw );
            
            BS.WriteFlag( Pitch < R_0 );
            BS.WriteRangedF32( ABS(Pitch), 10, R_0,  R_90 );
            BS.WriteRangedF32( Yaw,        12, R_0, R_360 );
        }
        else 
        {
            BS.WriteRangedS32( TargetNetSlot, 0, 31 );

            // Convert the float values to more compact integers.
            s32 OffsetX = (s32)AimOffset.GetX() + 256;
            s32 OffsetY = (s32)AimOffset.GetY();
            s32 OffsetZ = (s32)AimOffset.GetZ() + 256;

            // Now clip any stray values that managed to make it through.
            OffsetX = MINMAX( 0, OffsetX, 511 );
            OffsetY = MINMAX( 0, OffsetY, 511 );
            OffsetZ = MINMAX( 0, OffsetZ, 511 );

            BS.WriteRangedS32( OffsetX, 0, 511 );
            BS.WriteRangedS32( OffsetY, 0, 511 );
            BS.WriteRangedS32( OffsetZ, 0, 511 );
        }

        LOG('O');
    }

    if( BS.WriteFlag( DirtyBits & AIR_CROUCH_BIT ) )
    {
        DirtyBits &= ~AIR_CROUCH_BIT;

        BS.WriteFlag( Crouch );
        BS.WriteFlag( Airborn );

        LOG(Crouch ? 'c' : 'C');

        LOG('J');
        LOG('0' + Airborn);
    }

    if( BS.WriteFlag( DirtyBits & CONTAGION_OFF_BIT ) )
    {
        DirtyBits &= ~CONTAGION_OFF_BIT;
        LOG('~');
        LOG('-');
    }

    if( BS.WriteFlag( DirtyBits & HEALTH_BIT ) )
    {
        DirtyBits &= ~HEALTH_BIT;
        BS.WriteF32( IncHealth );
        BS.WriteF32( DecHealth );
        #ifdef LOG_UPDATE_DETAILS
        s32 Health = (s32)(IncHealth - DecHealth);
        #endif
        LOG('H');
        LOG('(');
        LOG('0' + (Health / 100));
        LOG('0' + (Health % 100) / 10);
        LOG('0' + (Health %  10));
        LOG(')');
    } 

    // Always write the LifeSeq.
    BS.WriteRangedS32( LifeSeq, 0, 7 );

    // Write corpse death pain?
    if( LifeSeq & 0x01 )	// Odd = Death
    {
        CorpseDeathPain.Write( BS );
    }

    if( BS.WriteFlag( DirtyBits & LIFE_BIT ) )
    {
        DirtyBits &= ~LIFE_BIT;

        if( (LifeSeq & 0x01) == 0 )
        {
            // Spawn information.
            BS.WriteU32      ( WeaponBits, INVEN_WEAPON_LAST+1 );
            BS.WriteRangedS32( AmmoSMP      , 0, 750 );
            BS.WriteRangedS32( AmmoShotgun  , 0, 120 );
            BS.WriteRangedS32( AmmoSniper   , 0,  30 );
            BS.WriteRangedS32( AmmoEagle    , 0, 120 );
            BS.WriteRangedS32( AmmoMSN      , 0,   3 );
            BS.WriteRangedS32( AmmoGrensFrag, 0,   7 );
            BS.WriteRangedS32( AmmoGrensJB  , 0,   7 );
        }

        LOG('L');
        LOG('0' + LifeSeq);

        LOG('w');
        LOG('(');
        for( s32 i = 0; i < 10; i++ )
            if( WeaponBits & (1<<i) )
                LOG('0' + i);
        LOG(')');
    }

    if( BS.WriteFlag( DirtyBits & TEAM_BITS_BIT ) )
    {
        DirtyBits &= ~TEAM_BITS_BIT;
        BS.WriteU32( TeamBits );
        LOG('t');
        if( TeamBits == 0x00000001 ) LOG('0'); else
        if( TeamBits == 0x00000002 ) LOG('1'); else
                                     LOG('*');
    }

    if( BS.WriteFlag( DirtyBits & LEAN_BIT ) )
    {
        DirtyBits &= ~LEAN_BIT;
        if( BS.WriteFlag( Lean == 0.0f ) == FALSE )
        {
            BS.WriteRangedF32( Lean, 5, -1.0f, 1.0f );
        }

        LOG('l');
        if( Lean >  0 ) LOG('\\');
        if( Lean == 0 ) LOG('|');
        if( Lean <  0 ) LOG('/');
    }   

    if( BS.WriteFlag( DirtyBits & MUTATE_BIT ) )
    {
        DirtyBits &= ~MUTATE_BIT;
        BS.WriteFlag( Mutated );
        LOG('M');
        LOG('0' + Mutated);
    }

    if( BS.WriteFlag( DirtyBits & WEAPON_BIT ) )
    {
        DirtyBits &= ~WEAPON_BIT;
        BS.WriteRangedS32( Weapon, INVEN_NULL, INVEN_WEAPON_LAST );
        LOG('W');
        LOG('0' + Weapon % 10);
        GotWeapon = TRUE;
    }

    if( BS.WriteFlag( (DirtyBits & RELOAD_BIT) &&
                     !(DirtyBits & (FIRE_BIT | FIRE_BEGIN_BIT | FIRE_END_BIT)) ) )
    {
        DirtyBits &= ~RELOAD_BIT;
        LOG('R');
    }

    if( BS.WriteFlag( DirtyBits & CURRENT_AMMO_BIT ) )
    {
        ASSERT( GotWeapon );
        DirtyBits &= ~CURRENT_AMMO_BIT;
        BS.WriteRangedS32( CurrentAmmo, 0, 1000 );
        LOG('(');
        LOG('0' + CurrentAmmo / 10);
        LOG('0' + CurrentAmmo % 10);
        LOG(')');
    }

    if( BS.WriteFlag( DirtyBits & FIRE_BIT ) )
    {
        DirtyBits &= ~FIRE_BIT;
        BS.WriteRangedS32( CurrentFireMode, 0, 1 );
        BS.WriteRangedS32( CurrentFireWeapon, INVEN_WEAPON_FIRST, INVEN_WEAPON_LAST );
        LOG('F');
    }

    if( BS.WriteFlag( DirtyBits & FIRE_BEGIN_BIT ) )
    {
        DirtyBits &= ~FIRE_BEGIN_BIT;
        BS.WriteRangedS32( CurrentFireWeapon, INVEN_WEAPON_FIRST, INVEN_WEAPON_LAST );
        BS.WriteRangedS32( CurrentFireSeq, 0, 7 );
        
        LOG('/');
        LOG('F');
        LOG('\\');
    }

    if( BS.WriteFlag( DirtyBits & FIRE_END_BIT ) )
    {
        DirtyBits &= ~FIRE_END_BIT;
        BS.WriteRangedS32( CurrentFireSeq, 0, 7 );

        LOG('\\');
        LOG('F');
        LOG('/');
    }

    if( BS.WriteFlag( DirtyBits & FLASHLIGHT_BIT ) )
    {
        DirtyBits &= ~FLASHLIGHT_BIT;
        BS.WriteFlag( Flashlight );
        LOG(Flashlight?'+':'-');
    }

    if( BS.WriteFlag( DirtyBits & TOSS_BIT ) )
    {
        DirtyBits &= ~TOSS_BIT;
        BS.WriteRangedS32( CurrentFragGrenades,  0, 7 );
        BS.WriteRangedS32( CurrentJBeanGrenades, 0, 7 );
        LOG('T');
    }
    
    if( BS.WriteFlag( DirtyBits & MELEE_BIT ) )
    {
        DirtyBits &= ~MELEE_BIT;
        LOG('M');
    }

    if( BS.WriteFlag( DirtyBits & DROP_ITEM_BIT ) )
    {
        DirtyBits &= ~DROP_ITEM_BIT;
        LOG('d');
    }

    if( BS.WriteFlag( DirtyBits & PLAY_ANIM_BIT ) )
    {
        DirtyBits &= ~PLAY_ANIM_BIT;
        BS.WriteS32      ( iAnim, 9 );
        BS.WriteRangedF32( AnimBlendTime, 8, 0.0f, 5.0f );
        BS.WriteU32      ( AnimFlags );
        BS.WriteRangedF32( AnimPlayTime, 8, 0.0f, 5.0f );
        LOG('a');
    }

    if( BS.WriteFlag( DirtyBits & WANT_NEW_TEAM_BIT ) )
    {
        DirtyBits &= ~WANT_NEW_TEAM_BIT;
        BS.WriteRangedS32( DesiredTeam, 0, 1 );
        LOG('#');
        LOG('0'+DesiredTeam);
    }

    if( BS.WriteFlag( DirtyBits & MUTATION_FLAGS_BIT ) )
    {
        DirtyBits &= ~MUTATION_FLAGS_BIT;
        BS.WriteFlag( ForceHuman );
        BS.WriteFlag( ForceMutant );
        BS.WriteRangedS32( MutagenBurnMode, actor::MBM_FORCED, actor::MBM_AT_WILL );
        if( ForceHuman )
        {
            LOG('<');  LOG('H');  LOG('>');
        }
        if( ForceMutant )
        {
            LOG('<');  LOG('M');  LOG('>');
        }
    } 

    if( BS.WriteFlag( DirtyBits & WAYPOINT_BIT ) )
    {   
        DirtyBits &= ~WAYPOINT_BIT;
        if( BS.WriteFlag( WayPointFlags & WAYPOINT_TELEPORT ) )
        {
        }
        if( BS.WriteFlag( WayPointFlags & WAYPOINT_TELEPORT_FX ) )
        {
            BS.WriteVector( WayPoint[0] );
            BS.WriteVector( WayPoint[1] );
        }
        if( BS.WriteFlag( WayPointFlags & WAYPOINT_JUMP_PAD_FX ) )
        {
            BS.WriteVector( WayPoint[0] );
        }
        LOG('@');
        LOG('0' + WayPointFlags);
    }

    if( BS.WriteFlag( DirtyBits & EFFECTS_BIT ) )
    {
        DirtyBits &= ~EFFECTS_BIT;
        BS.WriteU32( EffectsFlags, (sizeof(s_NetEffects) >> 2) );
        LOG('E');
    }

    if( BS.WriteFlag( DirtyBits & WANT_SPAWN_BIT ) )
    {
        DirtyBits &= ~WANT_SPAWN_BIT;
        LOG('S');
    }

    if( BS.WriteFlag( DirtyBits & VOTE_FLAGS_BIT ) )
    {
        DirtyBits &= ~VOTE_FLAGS_BIT;
        BS.WriteFlag( VoteCanCast );
        BS.WriteU32 ( VoteCanStartKick );
        BS.WriteFlag( VoteCanStartMap );
        LOG('v');
        LOG('0' + VoteCanCast);
        LOG('0' + !!(VoteCanStartKick));
        LOG('0' + VoteCanStartMap);
    }

    if( BS.WriteFlag( DirtyBits & VOTE_ACTION_BIT ) )
    {
        DirtyBits &= ~VOTE_ACTION_BIT;    
        BS.WriteRangedS32( VoteAction, 0, 3 );
        switch( VoteAction )
        {
        case 0:                                                  break;
        case 1:  BS.WriteRangedS32     ( VoteArgument, -1, +1 ); break;
        case 2:  BS.WriteRangedS32     ( VoteArgument,  0, 31 ); break;
        case 3:  BS.WriteVariableLenS32( VoteArgument );         break;
        default: ASSERT( FALSE );
        }
        LOG('V');
        LOG('0' + VoteAction);
    }

    if( BS.WriteFlag( DirtyBits & CONTAGION_ON_BIT ) )
    {
        DirtyBits &= ~CONTAGION_ON_BIT;
        BS.WriteRangedS32( ContagionOrigin, 0, 31 );
        LOG('~');
        LOG('+');
        LOG('0' + ContagionOrigin);
    }

    #ifdef LOG_UPDATE_DETAILS
    // Most updates include only position and orientation.
    // Only display updates which include more than that.
    if( L )
    {
        LOG_MESSAGE( "update::Write", "Data:%s", Msg );
        LOG_FLUSH();
    }
    #endif        
}

//==============================================================================
// ACTOR NET FUNCTIONS
//==============================================================================

actor::net::net( void ) 
:
    Skin            ( SKIN_NULL ),
    iAnim           ( -1        ),
    AnimBlendTime   (  0.0f     ),
    AnimFlags       (  0        ),
    AnimPlayTime    (  0.0f     ),
    FireMode        (  0        ),
    FireSeq         (  0        ),
    DesiredTeam     (  0        ),
    LifeSeq         ( -1        ),
    pPainQueue      ( NULL      )
{
}

//==============================================================================
//	ACTOR FUNCTIONS
//==============================================================================

void actor::net_Activate( void )
{
    m_NetDirtyBits = ACTIVATE_BIT;
    m_NetModeBits  = 0;

    if( g_NetworkMgr.IsServer() )   
        m_NetModeBits |= ON_SERVER;
    else                            
        m_NetModeBits |= ON_CLIENT;

    if( g_NetworkMgr.IsLocal( m_NetSlot ) )
        m_NetModeBits |= CONTROL_LOCAL;
    else                            
        m_NetModeBits |= CONTROL_REMOTE;

    m_Net.pPainQueue = g_NetworkMgr.GetPainQueue();

    LOG_MESSAGE( "actor::net_Activate",
                 "Addr:%08X - NetSlot:%d - Status:%s on %s",
                 this, m_NetSlot,
                 (m_NetModeBits & CONTROL_LOCAL) ? "LOCAL"  : "REMOTE",
                 (m_NetModeBits & ON_SERVER    ) ? "SERVER" : "CLIENT" );
}

//==============================================================================

void actor::net_AcceptUpdate( const bitstream& BS )
{
    // Read update using utility structure.
    update Update;
    Update.Slot = m_NetSlot;
    Update.Read( BS );

    // Call virtual function to process.
    net_AcceptUpdate( Update );
}

//==============================================================================

void actor::net_AcceptUpdate( const update& Update )
{
    u32 DirtyBits = Update.DirtyBits;

    //
    // LifeSeq checking.  Only worry if: 
    //  (a) LifeSeq values do not match,
    //  (b) the LifeSeq is not being updated, and
    //  (c) the object is not being activated with this update.
    //
    if(  (Update.LifeSeq != m_Net.LifeSeq) && 
        !(DirtyBits & LIFE_BIT) &&
        !(DirtyBits & ACTIVATE_BIT) )
    {
        // When the server receives updates from a client, the client could
        // be "out of date" on his LifeSeq if he doesn't yet know that he has
        // just died or just respawned.  
        //
        // Alternately, when the client receives updates from the server, the
        // client could be recently dead of a local death.
        //
        // Allow these cases to go by without a warning.

        if( (g_NetworkMgr.IsServer() && (((Update.LifeSeq+1) & 0x07) != m_Net .LifeSeq)) ||
            (g_NetworkMgr.IsClient() && (((m_Net .LifeSeq+1) & 0x07) != Update.LifeSeq)) )
        {
            LOG_WARNING( "actor::net_AcceptUpdate",
                         "Slot:%d - Unexpected LifeSeq value - Update:%d - Actor:%d",
                         m_NetSlot, Update.LifeSeq, m_Net.LifeSeq );
        }

        // Regardless of the warnings...  The LifeSeq values do not match, AND
        // there is no LIFE or ACTIVATE bit in the lineup.  There is no use
        // applying this update.  Bail!

        return;
    }

    // Setup character and skin?
    if( DirtyBits & ACTIVATE_BIT )
    {
        // Initialize skin?
        if( actor::m_Net.Skin != Update.Skin )
        {
            if( IN_RANGE( SKIN_BEGIN_PLAYERS, Update.Skin, SKIN_END_PLAYERS ) )
            {
                net_SetSkin( Update.Skin );
            }
            else
            {
                // Record skin.
                m_Net.Skin = Update.Skin;

                if( m_LocoAllocated )
                {
                    delete m_pLoco;
                    m_pLoco = NULL;
                }

                if( m_Net.Skin == SKIN_BOT_GRUNT )
                    m_pLoco = new grunt_loco();

                if( m_Net.Skin == SKIN_BOT_THETA )
                    ASSERT( FALSE );
                //  m_pLoco = new mutant_tank_loco();
                
                m_LocoAllocated = TRUE;
                ASSERTS( m_pLoco, "Must have a loco to proceed!" );

                // Clear animation and skin geom so properties initialize correctly.
                m_hAnimGroup.SetName    ( "NULL" );
                m_SkinInst.SetUpSkinGeom( "NULL" );

                // Save info that blue-print will trash.
                u32         AttrBits    = GetAttrBits();
                matrix4     L2W         = GetL2W();
                u16         Zone1       = GetZone1();
                u16         Zone2       = GetZone2();
                factions    Factions    = GetFaction();
                u32         FriendFlags = GetFriendFlags();

                // Load properties from blueprint.
                const char* pBlueprint = GameMgr.GetSkinBlueprint( m_Net.Skin );
                g_TemplateMgr.ApplyTemplateToObject( pBlueprint, GetGuid() );

                // Restore info that blue-print has trashed.
                SetAttrBits   ( AttrBits );
                OnTransform   ( L2W );
                SetZone1      ( Zone1 );
                SetZone2      ( Zone2 );
                SetFaction    ( Factions );
                SetFriendFlags( FriendFlags );
            }
        }
        
        // Initialize voice actor?
        if( m_PreferedVoiceActor != Update.VoiceActor )
        {
            net_SetVoiceActor( Update.VoiceActor );
        }
    }

    // Don't need to worry about DEACTIVATE_BIT here.  That is handled by the
    // client's update manager.

    // The POSITION_BIT and ORIENTATION_BIT are handled special by net_ghost
    // since that class wants to do blending.  Players and characters will never
    // accept/react to *incoming* position and orientation data.  Thus, we don't
    // need to do anything in here with those bits and that data.

    if( DirtyBits & AIR_CROUCH_BIT )
    {
        m_bIsCrouching  = Update.Crouch;
        m_bIsAirborn    = Update.Airborn;
        m_NetDirtyBits |= AIR_CROUCH_BIT;
    }

    if( DirtyBits & CONTAGION_OFF_BIT )
    {
        m_ContagionTimer = 0.0f;
        m_NetDirtyBits  |= CONTAGION_OFF_BIT;
    }

    if( DirtyBits & HEALTH_BIT )
    {
        ASSERT( g_NetworkMgr.IsClient() );

        f32 OldHealth = m_Health.GetHealth();
        f32 NewHealth = Update.IncHealth - Update.DecHealth;

        m_Health.SetInc( Update.IncHealth );
        m_Health.SetDec( Update.DecHealth );

        // Should we show player basic pain
        if( OldHealth > NewHealth )
        {
            // If actor is the player, then do the general pain.
            if( GetType() == object::TYPE_PLAYER )
                ((player*)this)->DoBasicPainFeedback( PAIN_FEEDBACK_FORCE );
        }
    }

    if( DirtyBits & LIFE_BIT )
    {    
        xbool DoSpawn = FALSE;
        xbool DoDeath = FALSE;

        //
        // Analyze the situation and decide what to do.
        //

        #define EVEN(a)   ( ((a) & 0x01) == 0 )
        #define  ODD(a)   ( ((a) & 0x01) == 1 )

        if( DirtyBits & ACTIVATE_BIT )
        {
            LOG_MESSAGE( "actor::net_AcceptUpdate", 
                         "[%02d] ACTIVATE - Update.LifeSeq:%d - m_Net.LifeSeq:%d - m_bDead:%d",
                         m_NetSlot, Update.LifeSeq, m_Net.LifeSeq, m_bDead );
        }

        if( IsDead() && EVEN(Update.LifeSeq) )
        {
            DoSpawn = TRUE;
        }
        else
        if( IsAlive() && ODD(Update.LifeSeq) )
        {
            DoDeath = TRUE;
        }
        else
        if( IsAlive() && 
            EVEN(Update.LifeSeq) && 
            EVEN(m_Net.LifeSeq) &&
            (m_Net.LifeSeq != Update.LifeSeq) )
        {
            // Special case.  Caused by a team change.
            // Example: LifeSeq = 2, Update.LifeSeq = 4.
            // Actions: DoDeath then DoSpawn.
            DoDeath = DoSpawn = TRUE;
        }
        else
        {
            // Something is fishy.
            CLOG_WARNING( m_Net.LifeSeq != Update.LifeSeq,
                          "actor::net_AcceptUpdate",
                          "Slot:%d - Unexpected LifeSeq update - Update:%d - Actor:%d",
                          m_NetSlot, Update.LifeSeq, m_Net.LifeSeq );
        }

        #undef EVEN
        #undef  ODD

        //
        // Now DO IT!
        //

        if( DoDeath )
        {
            // Grab corpse pain?
    		ASSERT( (Update.LifeSeq & 0x01) || (DoSpawn) ); // Odd = Death
            
            // Is death pain present? 
            // (it might not be due to packet loss missing life sequence #'s etc)
            if( Update.LifeSeq & 0x01 )     // Odd = death
            {
                ASSERT( *(u32*)&Update.CorpseDeathPain != 0xFEEDC0DE );
	            m_CorpseDeathPain = Update.CorpseDeathPain;
            }
            else
            {
                // Just use suicide pain for the ragdoll.
                m_CorpseDeathPain.Clear();
            }        
                        
            // If actor is the player, then do the general pain response.
            if( GetType() == object::TYPE_PLAYER )
                ((player*)this)->DoBasicPainFeedback( PAIN_FEEDBACK_FORCE );

            OnDeath();
        }

        if( DoSpawn )
        {
#ifdef X_ASSERT         
            // Make sure transform is setup.
            ASSERT( DirtyBits & POSITION_BIT );
            ASSERT( DirtyBits & ORIENTATION_BIT );
            u32* pI = (u32*)&Update.Position;
            ASSERT( pI[0] != 0xFEEDC0DE );
            ASSERT( pI[1] != 0xFEEDC0DE );
            ASSERT( pI[2] != 0xFEEDC0DE );
            ASSERT( *(u32*)&Update.Pitch != 0xFEEDC0DE );
            ASSERT( *(u32*)&Update.Yaw   != 0xFEEDC0DE );
#endif

            // Grab transform.
            vector3 Position = Update.Position;
            radian  Pitch    = Update.Pitch;
            radian  Yaw      = Update.Yaw;
            
            // TO DO - More data.
            Teleport( Position, Pitch, Yaw, FALSE, FALSE );

            // TO DO - Set all of the stuff from the TO DO above.
            OnSpawn();

            // Loadout.
            ASSERT( Update.WeaponBits != 0xFEEDC0DE );
            m_Inventory2.Clear();
            for( s32 i = INVEN_WEAPON_FIRST; i <= INVEN_WEAPON_LAST; i++ )
            {
                if( Update.WeaponBits & (1<<i) )
                {
                    m_Inventory2.AddAmount( (inven_item)i, 1.0f );
                }
            }

            ASSERT( *(u32*)&Update.AmmoSMP       != 0xFEEDC0DE );
            ASSERT( *(u32*)&Update.AmmoShotgun   != 0xFEEDC0DE );
            ASSERT( *(u32*)&Update.AmmoSniper    != 0xFEEDC0DE );
            ASSERT( *(u32*)&Update.AmmoEagle     != 0xFEEDC0DE );
            ASSERT( *(u32*)&Update.AmmoMSN       != 0xFEEDC0DE );
            ASSERT( *(u32*)&Update.AmmoGrensFrag != 0xFEEDC0DE );
            ASSERT( *(u32*)&Update.AmmoGrensJB   != 0xFEEDC0DE );

            AddAmmoToInventory2( INVEN_AMMO_SMP,          Update.AmmoSMP       );
            AddAmmoToInventory2( INVEN_AMMO_SHOTGUN,      Update.AmmoShotgun   );
            AddAmmoToInventory2( INVEN_AMMO_SNIPER_RIFLE, Update.AmmoSniper    );
            AddAmmoToInventory2( INVEN_AMMO_DESERT_EAGLE, Update.AmmoEagle     );
            AddAmmoToInventory2( INVEN_AMMO_MESON,        Update.AmmoMSN       );
            AddAmmoToInventory2( INVEN_GRENADE_FRAG,      Update.AmmoGrensFrag );
            AddAmmoToInventory2( INVEN_GRENADE_JBEAN,     Update.AmmoGrensJB   );

            ReloadAllWeapons();

            ASSERT( *(u32*)&Update.Weapon != 0xFEEDC0DE );
            net_EquipWeapon2( (inven_item)Update.Weapon );
        }

        // And finally...
        ASSERT( (Update.LifeSeq & 0x01) == (m_Net.LifeSeq & 0x01) );
        ASSERT( *(u32*)&Update.LifeSeq != 0xFEEDC0DE );
        m_Net.LifeSeq = Update.LifeSeq;
    }

    if( DirtyBits & TEAM_BITS_BIT )
    {
        net_SetTeamBits( Update.TeamBits );  // May need this ABOVE the LIFE.
    }

    if( DirtyBits & MUTATE_BIT )
    {
        m_bIsMutated = Update.Mutated;
        m_NetDirtyBits |= MUTATE_BIT;
    }

    if( DirtyBits & WEAPON_BIT )
    {
        net_EquipWeapon2( (inven_item)Update.Weapon );
    }

    if( DirtyBits & RELOAD_BIT )
    {
        // Play reload anim.
        net_Reload();
    }

    if( DirtyBits & CURRENT_AMMO_BIT )
    {   
        ASSERT( DirtyBits & WEAPON_BIT );

        s32 CurrentAmmo = Update.CurrentAmmo;

        {
            new_weapon* pWeapon = GetWeaponPtr( m_CurrentWeaponItem );
            if( pWeapon )
            {
                pWeapon->ClearAmmo( new_weapon::AMMO_PRIMARY );
                pWeapon->AddAmmoToWeapon( CurrentAmmo, 0 );
            }

            m_Inventory2.SetAmount( m_Inventory2.WeaponToAmmo( m_CurrentWeaponItem ), (f32)CurrentAmmo );
        }
    }

    if( DirtyBits & FIRE_BIT )
    {
        new_weapon* pWeapon = GetWeaponPtr( (inven_item)Update.CurrentFireWeapon );
        if( pWeapon )
        {
            m_FireState = 1;

            switch( Update.CurrentFireMode )
            {
            case net::FIRE_PRIMARY:

                // Play shoot anim on 3rd person avatar
                net_FirePrimary();
                break;

            case net::FIRE_SECONDARY:
                // Play shoot anim on 3rd person avatar
                net_FireSecondary();
                break;
            }
        }
    }

    if( DirtyBits & FIRE_BEGIN_BIT )
    {
        m_FireState = 2;

        // Play fire anim on 3rd person avatar only if this is the next new fire sequence
        // (a "begin" packet may arrive after the "end" packet for the same fire sequence!)
        s32 NextFireSeq = ( m_Net.FireSeq + 1 ) & 7;
        if( Update.CurrentFireSeq == NextFireSeq )
        {
            net_BeginFirePrimary();
            
            // If this is the server, then we need to update the internal sequence count
            // so that other clients get the correct sequence # and start to fire
            if( g_NetworkMgr.IsServer() )
            {
                m_Net.FireSeq = Update.CurrentFireSeq;
            }
        }            
    }

    if( DirtyBits & FIRE_END_BIT )
    {
        m_FireState = 0;

        // Flag primary fire should be stopped after next shot
        // (this makes sure a bullet is fired if the begin and end bits are in the same net packet)
        m_bEndPrimaryFire = TRUE;
        
        // Update fire sequence so begin fire can be matched up correctly
        m_Net.FireSeq = Update.CurrentFireSeq;
    }

    if( DirtyBits & FLASHLIGHT_BIT )
    {
        m_NetDirtyBits |= FLASHLIGHT_BIT;
            
        if( Update.Flashlight )
        {
            GetActorEffects( TRUE )->InitEffect( actor_effects::FX_FLASHLIGHT, this );
        }
        else
        {
            if( m_pEffects )
                m_pEffects->KillEffect( actor_effects::FX_FLASHLIGHT );
        }
    }

    if( DirtyBits & TOSS_BIT )
    {
        // Play toss grenade anim on 3rd person avatar
        net_Grenade();
        
        /*
        if( Update.CurrentFragGrenades != ((s32)m_Inventory2.GetAmount( INVEN_GRENADE_FRAG )) )
        {
            ASSERT( FALSE );
        }
        */

        // Update grenade inventory    
        m_Inventory2.SetAmount( INVEN_GRENADE_FRAG,  (f32)Update.CurrentFragGrenades  );
        m_Inventory2.SetAmount( INVEN_GRENADE_JBEAN, (f32)Update.CurrentJBeanGrenades );
    }
    
    if( DirtyBits & MELEE_BIT )
    {
        // Play melee on 3rd person avatar
        net_Melee();
    }

    if( DirtyBits & DROP_ITEM_BIT )
    {
        if( g_NetworkMgr.IsServer() )
            pGameLogic->DropFlag( m_NetSlot );
    }

    if( DirtyBits & WANT_NEW_TEAM_BIT )
    {
        if( Update.DesiredTeam != GameMgr.GetScore().Player[m_NetSlot].Team )
            GameMgr.ChangeTeam( m_NetSlot, FALSE );
    }

    if( DirtyBits & MUTATION_FLAGS_BIT )
    {
        xbool CanToggle = TRUE;

        if( Update.ForceHuman )
        {
            ForceMutationChange( FALSE );
            CanToggle = FALSE;
        }

        if( Update.ForceMutant )
        {
            ForceMutationChange( TRUE );
            CanToggle = FALSE;
        }

        SetCanToggleMutation( CanToggle );
        SetMutagenBurnMode( (mutagen_burn_mode)Update.MutagenBurnMode );
    }

    if( DirtyBits & WAYPOINT_BIT )
    {   
        m_NetDirtyBits   |= WAYPOINT_BIT;
        m_WayPointFlags   = Update.WayPointFlags;
        m_WayPointTimeOut = 0;
        if( m_WayPointFlags & WAYPOINT_TELEPORT_FX )
        {
            m_WayPoint[0] = Update.WayPoint[0];
            m_WayPoint[1] = Update.WayPoint[1];
        }
        else
        if( m_WayPointFlags & WAYPOINT_JUMP_PAD_FX )
        {
            m_WayPoint[0] = Update.WayPoint[0];
        }
    }

    if( DirtyBits & EFFECTS_BIT )
    {
        for( s32 i = 0; i < (sizeof(s_NetEffects) >> 2); i++ )
        {
            if( Update.EffectsFlags & (1 << i) )
            {
                GetActorEffects(TRUE)->InitEffect( (actor_effects::effect_type)s_NetEffects[i], this );
            }
        }
    }

    if( DirtyBits & WANT_SPAWN_BIT )
    {
        m_bWantToSpawn = TRUE;
    }

    if( Update.DirtyBits & PLAY_ANIM_BIT )
    {
        // Ready?
        if( (m_Net.Skin != SKIN_NULL) && (m_pLoco) )
        {
            // Play animation.
            m_pLoco->PlayAnim( Update.iAnim,            // Animation index
                               Update.AnimBlendTime,    // Blend time
                               Update.AnimFlags |       // Flags
                               loco::ANIM_FLAG_INTERRUPT_BLEND |
                               loco::ANIM_FLAG_TURN_OFF_RANDOM_SELECTION |
                               loco::ANIM_FLAG_RESTART_IF_SAME_ANIM,
                               Update.AnimPlayTime );   // PlayTime

            // Update current pain event ID so AI hurts other players.
            m_CurrentPainEventID = pain_event::CurrentEventID++;
            if( pain_event::CurrentEventID >= S32_MAX )
            {
                pain_event::CurrentEventID = 0;
            }
        }
    }

    if( Update.DirtyBits & VOTE_FLAGS_BIT )
    {
        m_VoteCanCast       = Update.VoteCanCast;
        m_VoteCanStartKick  = Update.VoteCanStartKick;
        m_VoteCanStartMap   = Update.VoteCanStartMap;
    }

    if( Update.DirtyBits & VOTE_ACTION_BIT )
    {
        switch( Update.VoteAction )
        {
        case 0:                                                           break;
        case 1:  GameMgr.CastVote     ( m_NetSlot, Update.VoteArgument ); break;
        case 2:  GameMgr.StartVoteKick( Update.VoteArgument, m_NetSlot ); break;
        case 3:  GameMgr.StartVoteMap ( Update.VoteArgument, m_NetSlot ); break;
        default: ASSERT( FALSE ); break;
        }
    }

    if( Update.DirtyBits & CONTAGION_ON_BIT )
    {
        InitContagion( Update.ContagionOrigin );
    }
}

//==============================================================================

void actor::net_ProvideUpdate( bitstream& BS, u32& DirtyBits )
{
    update Update;
    Update.Slot = m_NetSlot;

    // Call virtual function to process.
    net_ProvideUpdate( Update, DirtyBits );

    // Finally, write the update.
    Update.Write( BS );
	DirtyBits = Update.DirtyBits;
}

//==============================================================================
// Given an update record, and a set of minimum required dirty bits...
// Transfer all appropriate data from the actor into the update record setting 
// and adding dirty bits accordingly.

void actor::net_ProvideUpdate( update& Update, u32& DirtyBits )
{
    ASSERT( m_NetModeBits != GHOST_ON_CLIENT );
    ASSERT( (m_Net.LifeSeq & 0x01) == IsDead() );

    //
    // Opportunistic logic.  
    // If this player/ghost is doing "stuff", then clear his "inactive timer".
    // Ordinarily, calls to ClearInactiveTime would be scattered all over the
    // code.  But thanks to the magic of dirty bits, we can catch it all right
    // here.
    //
    if( DirtyBits & ( MUTATE_BIT
                    | WEAPON_BIT
                    | RELOAD_BIT
                    | FIRE_BIT
                    | FIRE_BEGIN_BIT
                    | FLASHLIGHT_BIT
                    | TOSS_BIT
                    | MELEE_BIT ) )
    {
        ClearInactiveTime();
    }

    //
    // Preprocessing.
    //

    // LifeSeq operations:
    // If we are DEAD, then some dirty bits are not relevant.  Only keep the 
    // useful bits.
    if( m_Net.LifeSeq & 0x01 )
    {
        DirtyBits &= 0
                //      |    POSITION_BIT
                //      |    ORIENTATION_BIT
                //      |    AIR_CROUCH_BIT
                //      |    CONTAGION_OFF_BIT
                 
                //      |    HEALTH_BIT
                        |    LIFE_BIT
                        |    TEAM_BITS_BIT
                //      |    LEAN_BIT

                        |    MUTATE_BIT
                //      |    WEAPON_BIT
                //      |    RELOAD_BIT
                        |    CURRENT_AMMO_BIT

                //      |    FIRE_BIT
                //      |    FIRE_BEGIN_BIT
                        |    FIRE_END_BIT
                //      |    FLASHLIGHT_BIT

                //      |    TOSS_BIT
                //      |    MELEE_BIT
                //      |    DROP_ITEM_BIT
                //      |    PLAY_ANIM_BIT

                //      |    WANT_NEW_TEAM_BIT
                        |    MUTATION_FLAGS_BIT
                //      |    WAYPOINT_BIT
                //      |    EFFECTS_BIT

                        |    WANT_SPAWN_BIT
                        |    VOTE_FLAGS_BIT
                        |    VOTE_ACTION_BIT
                //      |    CONTAGION_ON_BIT

                        |    ACTIVATE_BIT
                        |    DEACTIVATE_BIT
        ;
    }

    // If we are spawning, then we need to force a few bits.
    if( (DirtyBits & LIFE_BIT) && ((m_Net.LifeSeq & 0x01) == 0) )
    {
        DirtyBits |= POSITION_BIT;
        DirtyBits |= ORIENTATION_BIT;
        DirtyBits |= WEAPON_BIT;
        DirtyBits |= AIR_CROUCH_BIT;        
    }

    // Activation processing:
    // On activation, we must force several other data bits.
    if( DirtyBits & ACTIVATE_BIT )
    {
        // Must set these bits so that the following code will place the 
        // corresponding data in the update record.
        DirtyBits |= POSITION_BIT;
        DirtyBits |= ORIENTATION_BIT;
        DirtyBits |= HEALTH_BIT;
        DirtyBits |= LIFE_BIT;
        DirtyBits |= TEAM_BITS_BIT;
        DirtyBits |= MUTATE_BIT;
        DirtyBits |= WEAPON_BIT;
        DirtyBits |= VOTE_FLAGS_BIT;        
        DirtyBits |= MUTATION_FLAGS_BIT;
        DirtyBits |= EFFECTS_BIT;
    }

    if( DirtyBits & WEAPON_BIT )
    {
        if( m_CurrentWeaponItem == INVEN_WEAPON_MESON_CANNON )
        {
            DirtyBits |= CURRENT_AMMO_BIT;
        }
    }

    // Current ammo preprocessing:
    // When we give a current ammo update, we must include the weapon.
    if( DirtyBits & CURRENT_AMMO_BIT )
    {
        DirtyBits |= WEAPON_BIT;

        ASSERT( IN_RANGE( INVEN_NULL, m_CurrentWeaponItem, INVEN_WEAPON_LAST ) );

        new_weapon* pWeapon = GetWeaponPtr( m_CurrentWeaponItem );
        if( pWeapon )
            Update.CurrentAmmo = pWeapon->GetAmmoAmount( new_weapon::AMMO_PRIMARY );
        else
            Update.CurrentAmmo = 0;
    }

    // 
    // End of preprocessing.
    //
    // Now, for each bit set, copy its data into the update record.
    //

    if( DirtyBits & ACTIVATE_BIT )
    {
        Update.Skin       = m_Net.Skin;
        Update.VoiceActor = m_PreferedVoiceActor;
        Update.DirtyBits |= ACTIVATE_BIT;
    }

    // Don't need to worry about DEACTIVATE_BIT.  If the object has been 
    // deactivated, then we would never be here inside of a ghost function.

    if( DirtyBits & POSITION_BIT )
    {
        Update.Position   = GetPosition();
    	Update.DirtyBits |= POSITION_BIT;
    }

    if( DirtyBits & ORIENTATION_BIT )
    {
        Update.TargetNetSlot = m_TargetNetSlot;
        Update.AimOffset     = m_AimOffset;

        Update.AimOffset.GetX() *= 0.5f;
        Update.AimOffset.GetZ() *= 0.5f;

        Update.Pitch         = GetPitch();
        Update.Yaw           = GetYaw(); 
	    Update.DirtyBits    |= ORIENTATION_BIT;
    }

    if( DirtyBits & AIR_CROUCH_BIT )
    {
        Update.Crouch     = m_bIsCrouching;
        Update.Airborn    = m_bIsAirborn;
	    Update.DirtyBits |= AIR_CROUCH_BIT;
    }

    if( DirtyBits & CONTAGION_OFF_BIT )
    {           
        Update.DirtyBits |= CONTAGION_OFF_BIT;
    }

    if( DirtyBits & HEALTH_BIT )
    {
        Update.IncHealth  = m_Health.GetInc();
        Update.DecHealth  = m_Health.GetDec();
        Update.DirtyBits |= HEALTH_BIT;
    }

    // Always include the LifeSeq value, even if the dirty bit isn't set.  Also,
    // we stick it in the middle just to muddy the waters a little in case of
    // prying eyes...
    Update.LifeSeq = m_Net.LifeSeq;

    // Record corpse pain (it's only written/read on death)
    Update.CorpseDeathPain = m_CorpseDeathPain;
    
    if( DirtyBits & LIFE_BIT )
    {
        // If this is for a spawn (that is, if the LifeSeq goes from an odd 
        // value to an even value), then we need to piggy back in the spawn 
        // parameters.

        if( (m_Net.LifeSeq & 0x01) == 0 )
        {
            Update.Position = GetPosition();
            Update.Pitch    = GetPitch();
            Update.Yaw      = GetYaw();

            // Loadout.
            Update.WeaponBits = 0x00000000;
            for( s32 i = INVEN_WEAPON_FIRST; i <= INVEN_WEAPON_LAST; i++ )
            {
                if( m_Inventory2.GetAmount( (inven_item)i ) > 0.0f )
                {
                    Update.WeaponBits |= (1<<i);
                }
            }
            Update.AmmoSMP       = (s32)m_Inventory2.GetAmount( INVEN_AMMO_SMP          );
            Update.AmmoShotgun   = (s32)m_Inventory2.GetAmount( INVEN_AMMO_SHOTGUN      );
            Update.AmmoSniper    = (s32)m_Inventory2.GetAmount( INVEN_AMMO_SNIPER_RIFLE );
            Update.AmmoEagle     = (s32)m_Inventory2.GetAmount( INVEN_AMMO_DESERT_EAGLE );
            Update.AmmoMSN       = (s32)m_Inventory2.GetAmount( INVEN_AMMO_MESON        );
            Update.AmmoGrensFrag = (s32)m_Inventory2.GetAmount( INVEN_GRENADE_FRAG      );
            Update.AmmoGrensJB   = (s32)m_Inventory2.GetAmount( INVEN_GRENADE_JBEAN     );

            Update.Weapon = m_CurrentWeaponItem;
        }

        Update.DirtyBits |= LIFE_BIT;
    }

    if( DirtyBits & TEAM_BITS_BIT )
    {
        Update.TeamBits   = m_NetTeamBits;
        Update.DirtyBits |= TEAM_BITS_BIT;
    }

    if( DirtyBits & LEAN_BIT )
    {
        Update.Lean = m_LeanAmount;
        Update.DirtyBits |= LEAN_BIT;
    }

    if( DirtyBits & MUTATE_BIT )
    {
        Update.DirtyBits |= MUTATE_BIT;
        Update.Mutated    = m_bIsMutated;
    }

    if( DirtyBits & WEAPON_BIT )
    {
        Update.Weapon     = m_CurrentWeaponItem;
        Update.DirtyBits |= WEAPON_BIT;
    }

    if( DirtyBits & RELOAD_BIT )
    {
        Update.DirtyBits |= RELOAD_BIT;
    }

    if( DirtyBits & CURRENT_AMMO_BIT )
    {           
        Update.DirtyBits |= CURRENT_AMMO_BIT;
    }

    if( DirtyBits & FIRE_BIT )
    {
        Update.CurrentFireMode   = m_Net.FireMode;
        Update.CurrentFireWeapon = m_CurrentWeaponItem;
        Update.DirtyBits |= FIRE_BIT;
    }

    if( DirtyBits & FIRE_BEGIN_BIT )
    {
        Update.CurrentFireWeapon = m_CurrentWeaponItem;
        Update.CurrentFireSeq    = m_Net.FireSeq;
        Update.DirtyBits |= FIRE_BEGIN_BIT;
    }

    if( DirtyBits & FIRE_END_BIT )
    {
        Update.CurrentFireSeq = m_Net.FireSeq;
        Update.DirtyBits |= FIRE_END_BIT;
    }

    if( DirtyBits & FLASHLIGHT_BIT )
    {
        Update.Flashlight = IsFlashlightOn();
        Update.DirtyBits |= FLASHLIGHT_BIT;
    }

    if( DirtyBits & TOSS_BIT )
    {
        Update.CurrentFragGrenades  = (s32)m_Inventory2.GetAmount( INVEN_GRENADE_FRAG  );
        Update.CurrentJBeanGrenades = (s32)m_Inventory2.GetAmount( INVEN_GRENADE_JBEAN );

        Update.DirtyBits |= TOSS_BIT;
    }

    if( DirtyBits & MELEE_BIT )
    {
        Update.DirtyBits |= MELEE_BIT;
    }

    if( DirtyBits & DROP_ITEM_BIT )
    {
        Update.DirtyBits |= DROP_ITEM_BIT;
    }

    if( DirtyBits & WANT_NEW_TEAM_BIT )
    {
        Update.DesiredTeam = m_Net.DesiredTeam;
        Update.DirtyBits  |= WANT_NEW_TEAM_BIT;
    }

    if( DirtyBits & MUTATION_FLAGS_BIT )
    {
        Update.ForceHuman      = FALSE;
        Update.ForceMutant     = FALSE;
        Update.MutagenBurnMode = m_MutagenBurnMode;

        if( !m_bCanToggleMutation )
        {
            if( m_bIsMutated )  Update.ForceMutant = TRUE;
            else                Update.ForceHuman  = TRUE;
        }

        Update.DirtyBits |= MUTATION_FLAGS_BIT;
    }

    if( DirtyBits & WAYPOINT_BIT )
    {
        Update.WayPointFlags = m_WayPointFlags;
        Update.DirtyBits    |= WAYPOINT_BIT;
        if( m_WayPointFlags & WAYPOINT_TELEPORT_FX )
        {
            Update.WayPoint[0] = m_WayPoint[0];
            Update.WayPoint[1] = m_WayPoint[1];
        }
        else
        if( m_WayPointFlags & WAYPOINT_JUMP_PAD_FX )
        {
            Update.WayPoint[0] = m_WayPoint[0];
        }
    }

    if( DirtyBits & EFFECTS_BIT )
    {
        Update.EffectsFlags = 0;
                
        if( GetActorEffects() )
        {
            for( s32 i = 0; i < (sizeof(s_NetEffects) >> 2); i++ )
            {
                if( m_pEffects->IsEffectOn( (actor_effects::effect_type)s_NetEffects[i] ) )
                Update.EffectsFlags |= (1 << i);
            }
        }
        Update.DirtyBits |= EFFECTS_BIT;
    }

    if( DirtyBits & WANT_SPAWN_BIT )
    {
        Update.WantToSpawn = TRUE;
        Update.DirtyBits  |= WANT_SPAWN_BIT;
    }

    if( DirtyBits & PLAY_ANIM_BIT )
    {
        Update.iAnim         = m_Net.iAnim;
        Update.AnimBlendTime = m_Net.AnimBlendTime;
        Update.AnimFlags     = m_Net.AnimFlags;
        Update.AnimPlayTime  = m_Net.AnimPlayTime;
        Update.DirtyBits    |= PLAY_ANIM_BIT;
    }

    if( DirtyBits & VOTE_FLAGS_BIT )
    {
        Update.VoteCanCast      = m_VoteCanCast;
        Update.VoteCanStartKick = m_VoteCanStartKick;
        Update.VoteCanStartMap  = m_VoteCanStartMap;
        Update.DirtyBits       |= VOTE_FLAGS_BIT;
    }

    if( DirtyBits & VOTE_ACTION_BIT )
    {
        ASSERT( IN_RANGE( 0, m_VoteAction, 3 ) );
        Update.VoteAction   = m_VoteAction;
        Update.VoteArgument = m_VoteArgument;
        Update.DirtyBits   |= VOTE_ACTION_BIT;
    }

    if( DirtyBits & CONTAGION_ON_BIT )
    {
        if( IsContagious() )
        {
            Update.ContagionOrigin = m_pMPContagion->Origin;
            Update.DirtyBits      |= CONTAGION_ON_BIT;
        }
    }   
}

//==============================================================================

u32 actor::net_GetUpdateMask( s32 TargetClient ) const
{
    u32 Mask = 0x00000000;

    if( m_NetModeBits & ON_SERVER )
    {
        if( TargetClient != m_OwningClient )
        {
            // Channel (A) update - Server player/ghost -to- client ghost.
            Mask =  Mask
                |  POSITION_BIT       
                |  ORIENTATION_BIT    
                |  AIR_CROUCH_BIT
                |  CONTAGION_OFF_BIT

                |  HEALTH_BIT         
                |  LIFE_BIT           
                |  TEAM_BITS_BIT      
                |  LEAN_BIT      

                |  MUTATE_BIT         
                |  WEAPON_BIT         
                |  RELOAD_BIT         
            //  |  CURRENT_AMMO_BIT         

                |  FIRE_BIT           
                |  FIRE_BEGIN_BIT     
                |  FIRE_END_BIT 
                |  FLASHLIGHT_BIT 

                |  TOSS_BIT           
                |  MELEE_BIT          
                |  DROP_ITEM_BIT      
                |  PLAY_ANIM_BIT      

            //  |  WANT_NEW_TEAM_BIT
                |  MUTATION_FLAGS_BIT
                |  WAYPOINT_BIT
                |  EFFECTS_BIT

            //  |  WANT_SPAWN_BIT
            //  |  VOTE_FLAGS_BIT
            //  |  VOTE_ACTION_BIT
                |  CONTAGION_ON_BIT

                |  ACTIVATE_BIT       
                |  DEACTIVATE_BIT     
                ;
        }
        else
        {
            // Channel (B) update - Server ghost -to- client PLAYER.
            Mask =  Mask
            //  |  POSITION_BIT       
            //  |  ORIENTATION_BIT    
            //  |  AIR_CROUCH_BIT         
            //  |  CONTAGION_OFF_BIT
                
                |  HEALTH_BIT         
                |  LIFE_BIT           
                |  TEAM_BITS_BIT      
            //  |  LEAN_BIT      

            //  |  MUTATE_BIT         
            //  |  WEAPON_BIT         
            //  |  RELOAD_BIT         
            //  |  CURRENT_AMMO_BIT         

            //  |  FIRE_BIT           
            //  |  FIRE_BEGIN_BIT     
            //  |  FIRE_END_BIT       
            //  |  FLASHLIGHT_BIT 

            //  |  TOSS_BIT           
            //  |  MELEE_BIT          
            //  |  DROP_ITEM_BIT      
            //  |  PLAY_ANIM_BIT      

            //  |  WANT_NEW_TEAM_BIT
                |  MUTATION_FLAGS_BIT
            //  |  WAYPOINT_BIT
            //  |  EFFECTS_BIT

            //  |  WANT_SPAWN_BIT           
                |  VOTE_FLAGS_BIT
            //  |  VOTE_ACTION_BIT
                |  CONTAGION_ON_BIT

                |  ACTIVATE_BIT       
                |  DEACTIVATE_BIT     
                ;
        }
    }
    else
    {
        if( m_NetModeBits & CONTROL_LOCAL )
        {
            // Channel (C) update - Client PLAYER -to- server ghost.
            Mask =  Mask
                |  POSITION_BIT       
                |  ORIENTATION_BIT    
                |  AIR_CROUCH_BIT         
                |  CONTAGION_OFF_BIT

            //  |  HEALTH_BIT         
            //  |  LIFE_BIT           
            //  |  TEAM_BITS_BIT      
                |  LEAN_BIT      

                |  MUTATE_BIT      
                |  WEAPON_BIT         
                |  RELOAD_BIT         
                |  CURRENT_AMMO_BIT

                |  FIRE_BIT           
                |  FIRE_BEGIN_BIT     
                |  FIRE_END_BIT       
                |  FLASHLIGHT_BIT 

                |  TOSS_BIT           
                |  MELEE_BIT          
                |  DROP_ITEM_BIT      
                |  PLAY_ANIM_BIT      

                |  WANT_NEW_TEAM_BIT
            //  |  MUTATION_FLAGS_BIT
                |  WAYPOINT_BIT
                |  EFFECTS_BIT

                |  WANT_SPAWN_BIT           
            //  |  VOTE_FLAGS_BIT
                |  VOTE_ACTION_BIT
            //  |  CONTAGION_ON_BIT

            //  |  ACTIVATE_BIT       
            //  |  DEACTIVATE_BIT     
                ;
        }
    }

    return( Mask );
}

//==============================================================================

void actor::net_SetTeamBits( u32 TeamBits )
{
    LOG_MESSAGE( "actor::net_SetTeamBits", "Player:%d - TeamBits:%08X", 
                 m_NetSlot, TeamBits );

    if( m_NetTeamBits != TeamBits )
    {
        m_NetDirtyBits |= TEAM_BITS_BIT;
        m_NetTeamBits   = TeamBits;

        // Deal with the faction system.
        switch( TeamBits )
        {
        case 0x00000001:
            SetFaction( FACTION_TEAM_ONE );
            SetFriendFlags( FACTION_TEAM_ONE );
            break;

        case 0x00000002:
            SetFaction( FACTION_TEAM_TWO );
            SetFriendFlags( FACTION_TEAM_TWO );
            break;

        default:
            SetFaction( FACTION_DEATHMATCH );
            SetFriendFlags( 0 );
            break;
        }
    }
}

//==============================================================================

void actor::net_SetSkin( s32 Skin )
{
    s32 Mesh;
    s32 Texture;

    m_Net.Skin = Skin;

    GameMgr.GetSkinValues( Skin, Mesh, Texture );

    // Mesh 0 - VMeshMask =  1 (or   2 for mutated)   bit 0 (or bit 1)
    // Mesh 1 - VMeshMask =  4 (or   8 for mutated)   bit 2 (or bit 3)
    // Mesh 2 - VMeshMask = 16 (or  32 for mutated)   bit 4 (or bit 5)
    // Mesh 3 - VMeshMask = 64 (or 128 for mutated)   bit 6 (or bit 7)
    
    s32 MeshMask = 1;
    MeshMask <<= (Mesh * 2);
    if( GetAvatarMutationState() == AVATAR_MUTANT )
        MeshMask <<= 1;
    m_SkinInst.SetVMeshMask( MeshMask );

    // Texture 0 - VTextureMask = 0
    // Texture 1 - VTextureMask = 1
    // Texture 2 - VTextureMask = 2
    // Texture 3 - VTextureMask = 3
    // Note that the texture mask gets 4 bits per virtual mesh.  Thus, the value
    // for the texture mask is shifted up 4 bits per step in the mesh value.

    s32 TextureMask = Texture << (Mesh * 4);
    m_SkinInst.SetVirtualTexture( TextureMask );

    LOG_MESSAGE( "actor::net_SetSkin", 
                 "Skin:%d - Mesh:%d - Texture:%d -- MeshMask:%d - TextureMask:%d", 
                 Skin, Mesh, Texture, MeshMask, TextureMask );

    // Setup correct blood color (0 = Human, 1 = Grey, 2 = Blackops)
    switch( Skin )
    {
        default:
            m_BloodDecalGroup = 0;
            break;

        case SKIN_GREY_0:
        case SKIN_GREY_1:
        case SKIN_GREY_2:
        case SKIN_GREY_3:
        case SKIN_BOT_GRAY:
        case SKIN_BOT_ALIEN:
            m_BloodDecalGroup = 1;
            break;       
            
        case SKIN_BOT_BLACKOP:
            m_BloodDecalGroup = 2;
            break;       
    }

    // SB:                 
    // Select a random voice actor for now.
    // In theory this could be put in the player options
    net_SetVoiceActor( x_irand( 0, 3 ) );
}

//==============================================================================

void actor::net_SetVoiceActor( s32 VoiceActor )
{
    m_PreferedVoiceActor = VoiceActor;
    
    LOG_MESSAGE( "actor::net_SetVoiceActor", "VoiceActor:%d", VoiceActor );
}

//==============================================================================

void actor::net_DropWeapon( void )
{
    s32 i;
    inven_item DropWeapon;
    s32 CurrentAmmo = 0;
    
    CLOG_WARNING( m_CurrentWeaponItem == INVEN_NULL, 
                  "actor::net_DropWeapon", 
                  "CurrentWeapon:NULL - Slot:%d", m_NetSlot );
    
    ASSERT( IN_RANGE( INVEN_NULL, m_CurrentWeaponItem, INVEN_WEAPON_LAST ) );

    switch( m_CurrentWeaponItem )
    {
    case INVEN_WEAPON_DUAL_SHT:
        DropWeapon = INVEN_WEAPON_SHOTGUN;
        break;

    case INVEN_WEAPON_DUAL_SMP:
        DropWeapon = INVEN_WEAPON_SMP;
        break;

    case INVEN_WEAPON_DUAL_EAGLE:
        DropWeapon = INVEN_WEAPON_DESERT_EAGLE;
        break;

    case INVEN_WEAPON_MUTATION:
    case INVEN_WEAPON_SCANNER:
        DropWeapon = INVEN_NULL;
        break;

    default:
        DropWeapon = m_CurrentWeaponItem;
        break;
    }

    new_weapon* pWeapon = GetWeaponPtr( m_CurrentWeaponItem );
    if( pWeapon )
    {
        CurrentAmmo = pWeapon->GetAmmoAmount( new_weapon::AMMO_PRIMARY );
    }

    // Don't drop invalid weapons or empty meson cannons!
    if( ((DropWeapon != INVEN_WEAPON_MESON_CANNON) || (CurrentAmmo > 0)) &&
         (DropWeapon != INVEN_NULL) )
    {
        random r( x_rand() );
        vector3 Velocity = r.v3( -50.0f, 50.0f, 
                                  20.0f, 50.0f, 
                                 -50.0f, 50.0f );
        pickup::CreatePickup( GetGuid(), net_GetSlot(), DropWeapon, 1.0f, 30.0f, 
                              GetPosition() + vector3(0.0f, 100.0f, 0.0f), 
                              radian3(0.0f,r.frand(R_0,R_360),0.0f), 
                              Velocity, GetZone1(), GetZone2() );
    }
    if( m_CurrentWeaponItem )
    {
        net_EquipWeapon2( INVEN_NULL ); 
    }   

    // Frag Grenades.
    for( i = 0; i < m_Inventory2.GetAmount( INVEN_GRENADE_FRAG ); i++ )
    {
        if( (x_rand() % 100) > 50 )
        {
            random r( x_rand() );
            vector3 Velocity = r.v3( -50.0f, 50.0f, 
                                      20.0f, 50.0f, 
                                     -50.0f, 50.0f );
            pickup::CreatePickup( GetGuid(), net_GetSlot(), 
                                  INVEN_GRENADE_FRAG, 1.0f, 30.0f, 
                                  GetPosition() + vector3(0.0f, 100.0f, 0.0f), 
                                  radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, 
                                  GetZone1(), GetZone2() );
        }
    }
    m_Inventory2.SetAmount( INVEN_GRENADE_FRAG, 0.0f );

    // JB Grenades.
    for( i = 0; i < m_Inventory2.GetAmount( INVEN_GRENADE_JBEAN ); i++ )
    {
        if( (x_rand() % 100) > 50 )
        {
            random r( x_rand() );
            vector3 Velocity = r.v3( -50.0f, 50.0f, 20.0f, 50.0f, -50.0f, 50.0f );
            pickup::CreatePickup( GetGuid(), net_GetSlot(), 
                                  INVEN_GRENADE_JBEAN, 1.0f, 30.0f, 
                                  GetPosition() + vector3(0.0f, 100.0f, 0.0f),
                                  radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity,
                                  GetZone1(), GetZone2() );
        }
    }
    m_Inventory2.SetAmount( INVEN_GRENADE_JBEAN, 0.0f );
}

//==============================================================================

xbool actor::net_EquipWeapon2( inven_item WeaponItem )
{
    // Make sure loco anims are updated.
    loco* pLoco = GetLocoPointer();
    if( pLoco )
    {
        pLoco->SetWeapon( WeaponItem );
    }

    // Is this a controlled 1st person player on this machine?
    xbool bIsPlayer = ( GetType() == object::TYPE_PLAYER );
    
    // For non players (3rd person ghost, character) mutation weapon is never visible
    if( ( !bIsPlayer ) && ( WeaponItem == INVEN_WEAPON_MUTATION ) )
    {
        // Do not display a weapon
        WeaponItem = INVEN_NULL;
    }

    // Update?
    if( WeaponItem != m_CurrentWeaponItem )
    {
        new_weapon* pOldWeapon = GetCurrentWeaponPtr();
        new_weapon* pNewWeapon = GetWeaponPtr( WeaponItem );

        // First, deactivate the old weapon.
        if( pOldWeapon != NULL )
        {
            pOldWeapon->SetVisible( FALSE );
            pOldWeapon->ReleaseAudio();
        }

        // Default to NULL incase not found or clearing weapon
        m_CurrentWeaponItem = INVEN_NULL;

        // Now activate.
        if( pNewWeapon )
        {
            // Is this a controlled 1st person player?
            if( bIsPlayer )
            {
                // Setup for 1st person render
                pNewWeapon->InitWeapon( GetPosition(), new_weapon::RENDER_STATE_PLAYER, GetGuid() ); 
                pNewWeapon->SetVisible( TRUE );
            }
            else
            {
                // Setup for 3rd person render
                pNewWeapon->InitWeapon( GetPosition(), new_weapon::RENDER_STATE_NPC, GetGuid() ); 
                pNewWeapon->SetVisible( TRUE );
            }
            
            // Record as current item                            
            m_CurrentWeaponItem = WeaponItem;
        }

        // Position ready for rendering.
        MoveWeapon( TRUE );

        // I thought that this would have happened somewhere else, 
        // but it seems not.  So...
        #ifndef X_EDITOR
        m_NetDirtyBits |= WEAPON_BIT;  // NETWORK
        #endif // X_EDITOR 

        return( TRUE );
    }

    return( FALSE );
}

//==============================================================================

void actor::net_ApplyNetPain( net_pain& NetPain )
{
    // Skip if already dead since we only want to call "actor::OnDeath" just once per death
    if( IsDead() )
        return;

    // If this pain kills the victim, then grab the corpse death pain so that
    // the ragdoll has the correct death force applied
    if( NetPain.Kill )
        m_CorpseDeathPain = NetPain.CorpseDeathPain;

    // Update health (will create ragdoll on kill)
    net_UpdateHealth( -NetPain.Damage );
    if( IsDead() )
    {
        pGameLogic->PlayerDied( m_NetSlot, NetPain.Origin, NetPain.PainType );
    }
    else
    {
        // If actor is the player, then do the general pain.
        if( GetType() == object::TYPE_PLAYER )
            ((player*)this)->DoBasicPainFeedback( PAIN_FEEDBACK_FORCE );

        // Special case: contagion infection!
        if( NetPain.PainType == 101 )
        {
            LOG_MESSAGE( "actor::net_ApplyNetPain", "InitContagion" );
            InitContagion( NetPain.Origin );
        }
    }

    // Make sure the player makes a sound!
    PlayPainSfx();
    /*
    LOG_MESSAGE( "actor::net_ApplyPain",
                 "Source:%d - Victim:%d - Amount:%d - Type:%d - Health:%d",
                 NetPain.Origin, m_NetSlot, 
                 NetPain.Damage, NetPain.PainType, 
                 (s32)m_Health.GetHealth() );
    */
}

//==============================================================================

void actor::net_UpdateHealth( f32 DeltaHealth )
{
    // Modify health value by the incoming delta.  It could be + or -.
    m_Health.Add( DeltaHealth );

    // Flag the change for additional networking traffic.
    m_NetDirtyBits |= HEALTH_BIT;

    // Dead?
    if( m_Health.GetHealth() <= 0.0f )
    {
        OnDeath(); // You are DEAD!
    }
}

//==============================================================================

void actor::net_ReportNetPain( s32   Victim,
                               s32   Origin,
                               s32   PainType,
                               s32   VictimLifeSeq,
                               xbool Kill,
                               f32   Damage )
{
    #ifndef X_EDITOR

    // Assert that this is a client machine.
    ASSERT( g_NetworkMgr.IsClient() );

    net_pain NetPain;

    NetPain.Victim          = (s16)Victim;
    NetPain.Origin          = (s16)Origin;
    NetPain.PainType        = (s16)PainType;
    NetPain.LifeSeq         = (s16)VictimLifeSeq;
    NetPain.Kill            = (s8 )Kill;
    NetPain.Damage          =      Damage;
    NetPain.CorpseDeathPain = m_CorpseDeathPain;
    
    m_Net.pPainQueue->AddPain( NetPain );

    #endif
}

//==============================================================================

void actor::net_WantPickup( pickup& Pickup )
{
    (void)Pickup;

    #ifndef X_EDITOR
    Pickup.PlayerRequest( m_NetSlot );
    #endif
}

//==============================================================================

void actor::net_RequestTeam( s32 NewTeam )
{
    (void)NewTeam;

    #ifndef X_EDITOR

    if( g_NetworkMgr.IsServer() )
    {
        if( NewTeam != GameMgr.GetScore().Player[m_NetSlot].Team )
            GameMgr.ChangeTeam( m_NetSlot, FALSE );
    }
    else
    {
        m_Net.DesiredTeam = NewTeam;
        m_NetDirtyBits   |= WANT_NEW_TEAM_BIT;
    }

    #endif
}

//==============================================================================

void actor::net_PlayAnim( s32 iAnim, f32 BlendTime, u32 Flags, f32 PlayTime )
{
    m_NetDirtyBits |= PLAY_ANIM_BIT;
    m_Net.iAnim        = iAnim;
    m_Net.AnimPlayTime = BlendTime;
    m_Net.AnimFlags    = Flags;
    m_Net.AnimPlayTime = PlayTime;
}

//==============================================================================
// Avatar animation/net playback functions (call on server or client)
//==============================================================================

void actor::net_Reload( void )
{
    // Using 3rd person avatar on this machine?
    if( UsingLoco() )
    {
        // Make sure primary fire is stopped
        net_EndFirePrimary();
    
        // Play upper body reload anim.
        ASSERT( m_pLoco );
        m_pLoco->PlayMaskedAnim( loco::ANIM_RELOAD,                 // AnimType
                                 loco::BONE_MASKS_TYPE_UPPER_BODY,  // MaskType
                                 0.1f,                              // BoneMaskBlendTime
                                 0 );                               // Flags
    }
    
    // Send to other machines
    m_NetDirtyBits |= RELOAD_BIT;
}

//==============================================================================

void actor::net_Melee( void )
{
    // Using 3rd person avatar on this machine?
    if ( UsingLoco() )
    {
        // Make sure primary fire is stopped
        net_EndFirePrimary();
    
        // Play upper body melee anim
        ASSERT( m_pLoco );
        m_pLoco->PlayMaskedAnim( loco::ANIM_MELEE,                  // AnimType 
                                 loco::BONE_MASKS_TYPE_UPPER_BODY,  // MaskType
                                 0.1f,                              // BoneMaskBlendTime
                                 0 );                               // Flags
    }

    // Send to other machines
    m_NetDirtyBits |= MELEE_BIT;
}

//==============================================================================

void actor::net_Grenade( void )
{
    // Using 3rd person avatar on this machine?
    if ( UsingLoco() )
    {
        ASSERT( m_pLoco );
        
        // Stop reload and melee
        m_pLoco->GetMaskController( 0 ).Clear();
    
        // Play additive grenade toss
        m_pLoco->PlayAdditiveAnim( loco::ANIM_GRENADE,                  // AnimType
                                   0.1f,                                // BlendInTime
                                   0.1f,                                // BlendOutTime
                                   actor::ANIM_FLAG_SHOOT_CONTROLLER ); // Flags
    }
        
    // Send to other machines
    m_NetDirtyBits |= TOSS_BIT;
}

//==============================================================================

void actor::net_FirePrimary( void )
{
    // Using 3rd person avatar on this machine?
    if ( UsingLoco() )
    {
        ASSERT( m_pLoco );
    
        // Stop reload and melee
        m_pLoco->GetMaskController( 0 ).Clear();
    
        // Play additive shoot
        m_pLoco->PlayAdditiveAnim( loco::ANIM_SHOOT,                        // AnimType
                                   0.1f,                                    // BlendInTime
                                   0.1f,                                    // BlendOutTime
                                   loco::ANIM_FLAG_RESTART_IF_SAME_ANIM |
                                   actor::ANIM_FLAG_SHOOT_CONTROLLER );      // Flags

        // Make sure anim starts at frame zero, matrices are forced to be 
        // re-computed, and the weapon position is updated so that rapid
        // firing is accurate for the pistol etc.
        m_pLoco->GetAdditiveController( actor::ANIM_FLAG_SHOOT_CONTROLLER ).SetFrame( 0.0f );
        m_pLoco->m_Player.DirtyCachedL2Ws();
        MoveWeapon( FALSE );
    }    
    
    // Send to other machines
    m_NetDirtyBits |= FIRE_BIT;
    m_Net.FireMode  = net::FIRE_PRIMARY;
}

//==============================================================================

void actor::net_FireSecondary( void )
{
    // Using 3rd person avatar on this machine?
    if ( UsingLoco() )
    {
        ASSERT( m_pLoco );
        
        // Stop reload and melee
        m_pLoco->GetMaskController( 0 ).Clear();
        
        // Play additive shoot
        m_pLoco->PlayAdditiveAnim( loco::ANIM_SHOOT_SECONDARY,              // AnimType
                                   0.1f,                                    // BlendInTime
                                   0.1f,                                    // BlendOutTime
                                   loco::ANIM_FLAG_RESTART_IF_SAME_ANIM |
                                   actor::ANIM_FLAG_SHOOT_CONTROLLER );      // Flags

        // Make sure anim starts at frame zero, matrices are forced to be 
        // re-computed, and the weapon position is updated so that rapid
        // firing is accurate for the pistol etc.
        m_pLoco->GetAdditiveController( actor::ANIM_FLAG_SHOOT_CONTROLLER ).SetFrame( 0.0f );
        m_pLoco->m_Player.DirtyCachedL2Ws();
        MoveWeapon( FALSE );
    }    

    // Send to other machines
    m_NetDirtyBits |= FIRE_BIT;
    m_Net.FireMode  = net::FIRE_SECONDARY;
}

//==============================================================================

void actor::net_BeginFirePrimary( void )
{
    // Using 3rd person avatar on this machine?
    if ( UsingLoco() )
    {
        ASSERT( m_pLoco );
        
        // Stop reload and melee
        m_pLoco->GetMaskController( 0 ).Clear();
    
        // Play additive shoot
        m_pLoco->PlayAdditiveAnim( loco::ANIM_SHOOT,                        // AnimType
                                   0.1f,                                    // BlendInTime
                                   0.1f,                                    // BlendOutTime
                                   loco::ANIM_FLAG_RESTART_IF_SAME_ANIM |
                                   actor::ANIM_FLAG_SHOOT_CONTROLLER );      // Flags
    }    

    // Begin looping audio for ghosts?
    if( GetType() != object::TYPE_PLAYER )
    {
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
            pWeapon->BeginPrimaryFire();
    }
    else
    {
        // Increment fire sequence for controlling player
        m_Net.FireSeq++;
        m_Net.FireSeq &= 7;
    }
    
    // Send to other machines
    m_NetDirtyBits |= FIRE_BEGIN_BIT;
    m_Net.FireMode  = net::FIRE_PRIMARY;
    
    // Flag fire has not happened yet
    m_bPrimaryFired   = FALSE;
    m_bEndPrimaryFire = FALSE;
}

//==============================================================================

void actor::net_EndFirePrimary( void )
{
    m_FireState = 0;
    
    // Using 3rd person avatar on this machine?
    if( UsingLoco() )
    {
        // Stop additive shoot.
        ASSERT( m_pLoco );
        m_pLoco->GetAdditiveController( actor::ANIM_FLAG_SHOOT_CONTROLLER ).Clear();
    }
    
    // End looping audio for ghosts?
    if( GetType() != object::TYPE_PLAYER )
    {
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
            pWeapon->EndPrimaryFire();
    }
    
    // Send to other machines.
    m_NetDirtyBits |= CURRENT_AMMO_BIT;
    m_NetDirtyBits |= FIRE_END_BIT;
    
    // Flag fire is complete.
    m_bPrimaryFired   = FALSE;
    m_bEndPrimaryFire = FALSE;
}

//==============================================================================

