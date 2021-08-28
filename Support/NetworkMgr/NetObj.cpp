//==============================================================================
//
//  NetObj.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "NetObjMgr.hpp"
#include "x_debug.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

netobj::netobj( void )
{   
    m_NetType      = TYPE_NULL;
    m_NetSlot      = -1;            // Invalid value!
    m_OwningClient = -2;            // Invalid value!  (-1 is the server.)
    m_NetTeamBits  = 0x00000000;
    m_NetDirtyBits = 0x00000000;
}

//==============================================================================

netobj::~netobj( void )
{
}

//==============================================================================

void netobj::net_Logic( f32 /*DeltaTime*/ )
{
}

//==============================================================================

u32 netobj::net_GetClearDirty( void )
{
    u32 Bits = m_NetDirtyBits;
    m_NetDirtyBits = 0x00000000;
    return( Bits );
}

//==============================================================================

u32 netobj::net_GetDirtyBits( void ) const
{
    return( m_NetDirtyBits );
}

//==============================================================================

s32 netobj::net_GetSlot( void ) const
{
    return( m_NetSlot );
}

//==============================================================================

s32 netobj::net_GetOwningClient( void ) const
{
    return( m_OwningClient );
}

//==============================================================================

u32 netobj::net_GetTeamBits( void ) const
{
    return( m_NetTeamBits );
}

//==============================================================================

netobj::type netobj::net_GetType( void ) const
{
    return( m_NetType );
}

//==============================================================================

const char* netobj::net_GetTypeName( void ) const
{
    return( NetObjMgr.GetTypeInfo( m_NetType ).pTypeName );
}

//==============================================================================

s32 netobj::net_GetGameSlot( void ) const
{
    return( object::GetSlot() );
}

//==============================================================================

guid netobj::net_GetGameGuid( void ) const
{
    return( object::GetGuid() );
}

//==============================================================================

u32 netobj::net_GetUpdateMask( s32 TargetClient ) const
{
    if( TargetClient == m_OwningClient )
        return( 0x00000000 );
    else
        return( 0xFFFFFFFF );
}

//==============================================================================

void netobj::net_SetSlot( s32 Slot )
{
    m_NetSlot = Slot;
}

//==============================================================================

void netobj::net_SetOwningClient( s32 OwningClient )
{
    m_OwningClient = OwningClient;
}

//==============================================================================

void netobj::net_SetTeamBits( u32 TeamBits )
{
    m_NetTeamBits = TeamBits;
}

//==============================================================================

void netobj::net_Activate( void )
{
    m_NetDirtyBits |= ACTIVATE_BIT;
}

//==============================================================================

void netobj::net_Deactivate( void )
{
    m_NetDirtyBits |= DEACTIVATE_BIT;
}

//==============================================================================

void netobj::net_AcceptUpdate( const bitstream& /*BitStream*/ )
{
}

//==============================================================================

void netobj::net_ProvideUpdate( bitstream& /*BitStream*/, 
                                u32&       /*DirtyBits*/ ) 
{
}

//==============================================================================

void netobj::net_ApplyNetPain( net_pain& /*NetPain*/ )
{
}

//==============================================================================

void netobj::net_DebugRender( void ) const
{
}

//==============================================================================
