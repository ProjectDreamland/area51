//==============================================================================
//
//  NetAddress.hpp
//
//==============================================================================

#ifndef NETADDRESS_HPP
#define NETADDRESS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"
#include "x_time.hpp"

//==============================================================================
//  DEFINITIONS
//==============================================================================

class netbitstream;

//==============================================================================
//  TYPES
//==============================================================================

//
// NOTE:
//  m_IP and m_Port are in MACHINE ENDIAN format. This means that an IP address
// of 1.2.3.4 is (in hex) 0x01020304. This IS NOT the same as network endian.
//
class net_address
{
public:

                        net_address         ( void );
                        net_address         (       s32    IP,    s32 Port );
                        net_address         ( const char* pIPStr, s32 Port );

        void            Clear               ( void );
        xbool           IsEmpty             ( void ) const;
        xbool           IsValid             ( void ) const;

        void            Setup               ( s32 IP, s32 Port ); 
        void            SetIP               ( s32 IP );
        void            SetPortNumber       ( s32 Port );
        void            SetStrIP            ( const char* pIPStr   );
        void            SetStrAddress       ( const char* pAddrStr );

        s32             GetIP               ( void ) const;
        s32             GetPort             ( void ) const;
        const char*     GetStrIP            ( void ) const;
        const char*     GetStrAddress       ( void ) const;

        xbool           operator ==         ( const net_address& A ) const;
        xbool           operator !=         ( const net_address& A ) const;

private:    

        void            UpdateStrings       ( void );

        s32             m_IP;
        s32             m_Port;
        char            m_IPStr  [ 16 ];  // "123.456.789.012"
        char            m_AddrStr[ 22 ];  // "123.456.789.012:12345"
};

//==============================================================================
#endif // NETADDRESS_HPP
//==============================================================================
