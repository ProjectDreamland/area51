//==============================================================================
//
//  GMMgr.hpp
//
//==============================================================================

#ifndef GMMGR_HPP
#define GMMGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  TYPES
//==============================================================================

class   bitstream;
struct  conn_packet;

//==============================================================================

class gm_mgr
{

private:

    xbool           m_Connected;
    u32             m_DirtyBits;
    u32             m_ScoreBits;
    u32             m_PlayerBits;

public:

            gm_mgr              ( void );
           ~gm_mgr              ( void );

    void    Init                ( void );
    void    Kill                ( void );

    xbool   IsConnected         ( void );
    void    PacketAck           ( conn_packet& Packet, xbool Arrived );

    void    ProvideUpdate       ( conn_packet& Packet, bitstream& BitStream );
    void    AcceptUpdate        ( bitstream& BitStream );
                                
    void    AddDirtyBits        ( u32 DirtyBits, u32 ScoreBits, u32 PlayerBits );
};

//==============================================================================
#endif // GMMGR_HPP
//==============================================================================
