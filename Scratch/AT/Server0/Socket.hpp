//==============================================================================
//
//  Socket.hpp
//
//==============================================================================

#ifndef SOCKET_HPP
#define SOCKET_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include <winsock2.h>
#include "x_files.hpp"

//==============================================================================

struct socket_client;
class  data_socket;
class  listen_socket;
struct ip_address;

//==============================================================================

struct socket_client
{
public:
    static u32 NumReferences;

    socket_client();
    ~socket_client();
};

//==============================================================================

struct ip_address
{
public:

    static u32   Any;
    static u32   Loopback;
    static u32   Broadcast;

    u32  Address;
    u16  Port;

    ip_address() {}

    ip_address(  u32 InAddress,  u16 InPort  )
    {
        Address = InAddress;
        Port = InPort;
    }

    void GetSocketAddr( struct sockaddr_in* pResult ) const;

    xstring Describe( void );
};

//==============================================================================

class data_socket
{
private:

    socket_client   SocketClient;
    u32             Socket;

public:

    data_socket( u32 InSocket );
    virtual ~data_socket();

    static data_socket* Connect( ip_address Address );

    ip_address GetRemoteAddress( void );

    xbool   Receive         ( void* pData,   u32 DataLength );
    u32     PartialReceive  ( void* pData,   u32 DataLength );
    void    Send            ( void* pData,   u32 DataLength );

    xbool IsReceiveDataQueued();
    xbool IsClosed();
};

//==============================================================================

class listen_socket
{
private:

    socket_client   SocketClient;
    u32             Socket;

public:

    listen_socket( ip_address Address );
    virtual ~listen_socket();

    ip_address GetLocalAddress( void );

    xbool HasPendingConnection( void );
    data_socket* AcceptConnection( void );
};

//==============================================================================
#endif
//==============================================================================

