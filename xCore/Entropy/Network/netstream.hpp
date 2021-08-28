//=============================================================================
//
//  BitStream.hpp
//
//=============================================================================

#ifndef NETSTREAM_HPP
#define NETSTREAM_HPP

//=============================================================================
//  INCLUDES
//=============================================================================

#include "x_types.hpp"
#include "x_math.hpp"
#include "e_Network.hpp"
#include "x_bitstream.hpp"

//=============================================================================
//  TYPES
//=============================================================================

class netstream : public bitstream
{
public:
                        netstream(void);
                       ~netstream(void);

            void        Init                ( void );
            void        Kill                ( void );
            void        Open                ( s32 HeaderId, s32 Type ); 
            void        Close               ( void );
            void        Send                ( net_socket& Socket, const net_address& Remote );
            xbool       Receive             ( net_socket& Socket, net_address& Remote );
            xbool       Validate            ( void );

private:
            byte        m_Buffer[896];
};

//=============================================================================
#endif
//=============================================================================