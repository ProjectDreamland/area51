//==============================================================================
//
//  Socket.cpp
//
//==============================================================================

#include "socket.hpp"

//==============================================================================

u32  ip_address::Any            = INADDR_ANY;
u32  ip_address::Loopback       = INADDR_LOOPBACK;
u32  ip_address::Broadcast      = INADDR_BROADCAST;
u32  socket_client::NumReferences = 0;

//==============================================================================
//  socket_client
//==============================================================================

socket_client::socket_client()
{
    if( !NumReferences++ )
    {
        WSADATA WSAData;
        s32   Result = WSAStartup(MAKEWORD(1,0),&WSAData);
        ASSERT( Result==0 );
        //if(Result)
        //    throw Str+"WSAStartup failed: "+toString(Result);
    }
}

//==============================================================================

socket_client::~socket_client()
{
    if(!--NumReferences)
    {
        s32   Result = WSACleanup();
        ASSERT( Result==0 );
        //if(Result)
        //    throw Str+"WSACleanup failed: "+toString(Result);
    }
}

//==============================================================================
//  ip_address
//==============================================================================

void ip_address::GetSocketAddr(struct sockaddr_in* pResult) const
{
    pResult->sin_family = AF_INET;
    pResult->sin_addr.S_un.S_addr = htonl(Address);
    pResult->sin_port = htons(Port);
}

//==============================================================================

xstring ip_address::Describe(void)
{
    return xstring( xfs("%02X.%02X.%02X.%02X:%d",
        (Address>>24) & 0xFF,
        (Address>>16) & 0xFF,
        (Address>> 8) & 0xFF,
        (Address>> 0) & 0xFF,
        Port
        ));
}

//==============================================================================
//  data_socket
//==============================================================================

data_socket::data_socket(u32 InSocket)
{
    Socket = InSocket;

    if( Socket != INVALID_SOCKET )
    {
        u32  NoBlocking = 1;

        if(::ioctlsocket((SOCKET)Socket,FIONBIO,&NoBlocking) == SOCKET_ERROR)
        {
            ASSERT(FALSE);
            //throw Str+"Failed to make socket non-blocking: "+toString((s32)WSAGetLastError());
        }
    }
}

//==============================================================================

data_socket::~data_socket()
{
    if( Socket != INVALID_SOCKET )
    {
        shutdown((SOCKET)Socket,SD_SEND);
        closesocket((SOCKET)Socket);
        Socket = INVALID_SOCKET;
    }
}

//==============================================================================

data_socket* data_socket::Connect(ip_address Address)
{
    socket_client  socket_client;
    SOCKET      Socket;

    // Create the socket.

    Socket = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(Socket == INVALID_SOCKET)
        throw Str+"Couldn't create socket for listen: "+toString((s32)WSAGetLastError());

    // Bind the socket.

    sockaddr_in LocalSockAddr;
    ip_address(ip_address::Any,0).GetSocketAddr(&LocalSockAddr);
    if(::bind(Socket,(sockaddr*)&LocalSockAddr,sizeof(sockaddr_in)) == SOCKET_ERROR)
        throw Str+"Couldn't bind socket for listen: "+toString((s32)WSAGetLastError());

    // Connect to the remote socket.

    sockaddr_in RemoteSockAddr;
    Address.GetSocketAddr(&RemoteSockAddr);
    if(::connect(Socket,(sockaddr*)&RemoteSockAddr,sizeof(sockaddr_in)) == SOCKET_ERROR)
        throw Str+"Couldn't connect socket: "+toString((s32)WSAGetLastError());

    return new data_socket((u32)Socket);
}

//==============================================================================

ip_address data_socket::GetRemoteAddress( void )
{
    sockaddr_in RemoteSockAddr;
    s32       RemoteSockAddrLength = sizeof(sockaddr_in);
    if(getpeername((SOCKET)Socket,(sockaddr*)&RemoteSockAddr,(int*)&RemoteSockAddrLength) == SOCKET_ERROR)
        throw Str+"getpeername failed: "+toString((s32)WSAGetLastError());

    return ip_address(ntohl(RemoteSockAddr.sin_addr.S_un.S_addr),ntohs(RemoteSockAddr.sin_port));
}

//==============================================================================

bool data_socket::Receive(void* Data,u32 DataLength)
{
    s32   Result = ::recv((SOCKET)Socket,(char*)Data,(int)DataLength,0);

    if(Result == SOCKET_ERROR)
        throw Str+"recv failed: "+toString((s32)WSAGetLastError());
    else
        return Result == DataLength;
}

//==============================================================================

u32 data_socket::PartialReceive(void* Data,u32 DataLength)
{
    s32   Result = ::recv((SOCKET)Socket,(char*)Data,(int)DataLength,0);

    if(Result == SOCKET_ERROR)
        throw Str+"recv failed: "+toString((s32)WSAGetLastError());
    else
        return Result;
}

//==============================================================================

void data_socket::Send(void* Data,u32 DataLength)
{
    u32 BytesSent = 0;

    while(BytesSent < DataLength)
    {
        s32   Result = ::send((SOCKET)Socket,(char*)Data,(int)DataLength,0);

        if(Result == SOCKET_ERROR)
        {
            s32   LastError = WSAGetLastError();
            if(LastError == WSAEWOULDBLOCK)
                Sleep(10);
            else
                throw Str+"send failed: "+toString(LastError);
        }
        else
            BytesSent += Result;
    };
}

//==============================================================================

xbool data_socket::IsReceiveDataQueued( void )
{
    char    TempBuf[256];
    s32     NumBytes;

    NumBytes = ::recv((SOCKET)Socket,TempBuf,1,MSG_PEEK);

    if(!NumBytes || (NumBytes == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK))
        return FALSE;
    else
        return TRUE;
}

//==============================================================================

xbool DisconnectError(s32 Error)
{
    if(Error == WSAECONNRESET || Error == WSAECONNABORTED || Error == WSAETIMEDOUT)
        return TRUE;
    else
        return FALSE;
}
//==============================================================================

xbool data_socket::IsClosed( void )
{
    fd_set  SocketSet;
    TIMEVAL SelectTime = {0, 0};

    FD_ZERO(&SocketSet);
    FD_SET((SOCKET)Socket,&SocketSet);

    s32   Result = ::select(0,&SocketSet,NULL,NULL,&SelectTime);

    if(Result == SOCKET_ERROR)
    {
        if(DisconnectError(WSAGetLastError()))
            return TRUE;
        else
            throw Str+"select failed: "+toString((s32)WSAGetLastError());
    }

    if(Result)
    {
        char    TempBuf[256];
        s32   NumBytes;
        NumBytes = ::recv((SOCKET)Socket,TempBuf,1,MSG_PEEK);
        if(!NumBytes || (NumBytes == SOCKET_ERROR && disconnectError(WSAGetLastError())))
            return TRUE;
    }

    return FALSE;
}

//==============================================================================
// listen_socket
//==============================================================================

listen_socket::listen_socket(const ip_address& Address)
{
    // Create the socket.

    Socket = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(!Socket)
        throw Str+"Couldn't create socket for listen.  GetLastError returned: "+toString((s32)WSAGetLastError());

    // Bind the socket.

    sockaddr_in SockAddr;
    Address.getSockAddr(&SockAddr);
    if(::bind((SOCKET)Socket,(sockaddr*)&SockAddr,sizeof(sockaddr_in)) == SOCKET_ERROR)
        throw Str+"Couldn't bind socket for listen.  GetLastError returned: "+toString((s32)WSAGetLastError());

    // Start listening on the socket.

    if(::listen((SOCKET)Socket,SOMAXCONN) == SOCKET_ERROR)
        throw Str+"Couldn't listen.  GetLastError returned: "+toString((s32)WSAGetLastError());
}

//==============================================================================

listen_socket::~listen_socket()
{
    if( Socket != INVALID_SOCKET )
    {
        closesocket((SOCKET)Socket);
        Socket = INVALID_SOCKET;
    }
}

//==============================================================================

ip_address listen_socket::GetLocalAddress( void )
{
    sockaddr_in LocalSockAddr;
    s32       LocalSockAddrLength = sizeof(sockaddr_in);
    if(getsockname((SOCKET)Socket,(sockaddr*)&LocalSockAddr,(int*)&LocalSockAddrLength) == SOCKET_ERROR)
        throw Str+"getsockname failed: "+toString((s32)WSAGetLastError());

    return ip_address(ntohl(LocalSockAddr.sin_addr.S_un.S_addr),ntohs(LocalSockAddr.sin_port));
}

//==============================================================================

xbool listen_socket::HasPendingConnection( void )
{
    fd_set  SocketSet;
    TIMEVAL SelectTime = {0, 0};

    FD_ZERO(&SocketSet);
    FD_SET((SOCKET)Socket,&SocketSet);

    s32   Result = ::select(0,&SocketSet,NULL,NULL,&SelectTime);

    if(Result == SOCKET_ERROR)
        throw Str+"select failed: "+toString((s32)WSAGetLastError());

    if(Result)
        return TRUE;
    else
        return FALSE;
}

//==============================================================================

data_socket* listen_socket::AcceptConnection( void )
{
    sockaddr_in AcceptedAddress;
    s32       AcceptedAddressSize = sizeof(sockaddr_in);
    SOCKET      AcceptedSocket = ::accept((SOCKET)Socket,(sockaddr*)&AcceptedAddress,(int*)&AcceptedAddressSize);

    if(AcceptedSocket != INVALID_SOCKET)
        return new data_socket((u32)AcceptedSocket);
    else
        return NULL;
}

//==============================================================================
