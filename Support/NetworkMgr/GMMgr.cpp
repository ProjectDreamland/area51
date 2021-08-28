//=========================================================================
//
//  GMMgr.cpp
//
//=========================================================================

#include "x_files.hpp"
#include "GMMgr.hpp"
#include "ConnMgr.hpp"
#include "GameMgr.hpp"

//=========================================================================

gm_mgr::gm_mgr( void )
{
}

//=========================================================================

gm_mgr::~gm_mgr( void )
{
}

//=========================================================================

void gm_mgr::Init( void )
{
    m_Connected  = TRUE;

    m_DirtyBits  = 0xFFFFFFFF;
    m_ScoreBits  = 0xFFFFFFFF;
    m_PlayerBits = 0xFFFFFFFF;
}

//=========================================================================

void gm_mgr::Kill( void )
{
    m_Connected  = FALSE;
}

//=========================================================================

void gm_mgr::ProvideUpdate( conn_packet& Packet, bitstream& BitStream )
{
    if( !m_Connected ) 
        return;

    // Make backup copies of the dirty bits.
    u32 DirtyBits  = m_DirtyBits;
    u32 ScoreBits  = m_PlayerBits;
    u32 PlayerBits = m_PlayerBits;

    // Let the GameMgr send whatever data it can given the current
    // dirty bits.  This function call will clear bits which are sent.
    GameMgr.ProvideUpdate( BitStream,
                           Packet.TargetClient,
                           m_DirtyBits, 
                           m_ScoreBits, 
                           m_PlayerBits );

    // Now, save into the packet the bits which were written.  Simply XOR
    // the backup copies of the bit masks with the updated versions.
    Packet.GMDirtyBits   =  m_DirtyBits   ^  DirtyBits;
    Packet.GMScoreBits   =  m_ScoreBits   ^  ScoreBits;
    Packet.GMPlayerBits  =  m_PlayerBits  ^  PlayerBits;
}

//=========================================================================

void gm_mgr::AcceptUpdate( bitstream& BitStream )
{
    if( !m_Connected ) 
        return;

    GameMgr.AcceptUpdate( BitStream );
}

//=========================================================================

void gm_mgr::PacketAck( conn_packet& Packet, xbool Arrived )
{
    if( !m_Connected ) 
        return;

    if( !Arrived )
    {
        m_DirtyBits  |= Packet.GMDirtyBits; 
        m_ScoreBits  |= Packet.GMScoreBits;
        m_PlayerBits |= Packet.GMPlayerBits;
    }
}

//=========================================================================

xbool gm_mgr::IsConnected( void )
{
    return m_Connected;
}

//=========================================================================

void gm_mgr::AddDirtyBits( u32 DirtyBits, u32 ScoreBits, u32 PlayerBits )
{
    m_DirtyBits  |= DirtyBits;
    m_ScoreBits  |= ScoreBits;
    m_PlayerBits |= PlayerBits;
}

//=========================================================================
