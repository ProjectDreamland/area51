//==============================================================================
//
//  PlayerNet.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//#ifndef mtraub
//#define X_SUPPRESS_LOGS
//#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#include "..\NetworkMgr\NetworkMgr.hpp"
#include "Player.hpp"
#include "x_bitstream.hpp"

//==============================================================================
//  TYPES
//==============================================================================

//==============================================================================
//  STORAGE
//==============================================================================

//==============================================================================
//  FUNCTIONS
//==============================================================================

void player::net_Activate( void )
{
    // Call base class.
    actor::net_Activate();

    // Record as local player?
    if( m_NetModeBits & CONTROL_LOCAL )
        SetLocalPlayer( g_NetworkMgr.GetLocalSlot( m_NetSlot ) );

    LOG_MESSAGE( "player::net_Activate",
                 "Addr:%08X - LocalSlot:%d - NetSlot:%d - Status:%s on %s",
                 this, m_LocalSlot, m_NetSlot,
                 (m_NetModeBits & CONTROL_LOCAL) ? "LOCAL"  : "REMOTE",
                 (m_NetModeBits & ON_SERVER    ) ? "SERVER" : "CLIENT" );
}

//==============================================================================

void player::net_Logic( f32 /*DeltaTime*/ )
{
    if( m_WayPointTimeOut < 15 )
        m_WayPointTimeOut++;
    else
        m_WayPointFlags = 0;

    ASSERT( m_NetModeBits & CONTROL_LOCAL );  // Must be locally controlled.
    ASSERT( (actor::m_Net.LifeSeq & 0x01) == IsDead() );
}

//==============================================================================

xbool player::net_EquipWeapon2( inven_item WeaponItem )
{
    if( m_bIsMutated || (m_PrevWeaponItem == INVEN_NULL) )
    {
        m_PrevWeaponItem = WeaponItem;
    }

    xbool Result = actor::net_EquipWeapon2( WeaponItem );

    LoadAimAssistTweaks();
    LoadAimAssistTweakHandles();

    return( Result );
}

//==============================================================================
