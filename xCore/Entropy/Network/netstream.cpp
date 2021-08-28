//=========================================================================
//
//  netstream.CPP
//
//=========================================================================

#include "Network/netstream.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_color.hpp"
#include "x_string.hpp"

#ifndef LITTLE_ENDIAN
#error netstream.cpp currently relies on little endian
#endif

//=========================================================================
netstream::netstream( void )
{
    Init();
}

//=========================================================================

netstream::~netstream( void )
{
}

//=========================================================================
void netstream::Init(void)
{
    bitstream::Init( m_Buffer, sizeof(m_Buffer) );
}


//=============================================================================
// Bitstream network helper functions
//=============================================================================
xbool   netstream::Receive( net_socket&  Socket,
                            net_address& Remote)
{
    s32 NBytes = GetNBytes();

    return Socket.Receive( Remote, GetDataPtr(), NBytes );
}

//=============================================================================
void    netstream::Send (       net_socket&  Socket, 
                          const net_address& Remote)
{
    Socket.Send( Remote, GetDataPtr(), GetNBytesUsed() );
}

//=============================================================================
xbool   netstream::Validate( void )
{
    u16     Checksum;
    u16     CalculatedChecksum;
    byte*   pData;
    s32     Length;

    SetCursor(0);

    ReadU16( Checksum );

    pData  = GetDataPtr() + 2;
    Length = GetNBytes() - 2;

    if( Length <= 0 )
    {
        return FALSE;
    }

    CalculatedChecksum = (u16)x_chksum(pData,Length);
    if( CalculatedChecksum != Checksum )
    {
        SetCursor(0);
        return FALSE;
    }

    return TRUE;
}
//=========================================================================
void netstream::Open( s32 Identifier, s32 PacketType )
{
    Clear();
    WriteU16(0);                                    // Make space for checksum
    WriteU16(Identifier);                           // Packet header
    WriteU16(PacketType);                           // Type of packet
}

//=========================================================================
void netstream::Close( void )
{
    // We need to make sure we pad out the length of the bitstream to 8 byte alignment for
    // the encryption routines.

    while( GetNBytesUsed() & 0x07 )
    {
        WriteU32( 0, 8 );
    }

    s32 Cursor      = GetCursor();
    s32 Checksum    = x_chksum(GetDataPtr()+2,GetNBytesUsed()-2);

    SetCursor (0);
    WriteU16  ((u16)Checksum);
    SetCursor (Cursor);
    // Is this where we would do the diffie hellman encryption?

}

