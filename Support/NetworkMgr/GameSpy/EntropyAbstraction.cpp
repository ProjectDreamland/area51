//=========================================================================
//
//  EntropyAbstraction.cpp
//
//=========================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//=========================================================================
//
//  Abstraction of inevitable Entropy networking code to GameSpy. This is our
// implementation of the nonport.c file.
//=========================================================================
//
// NOTE:  Entropy network layer holds IP and PORT # in host endian order. The
// GameSpy code holds it in network endian order. Oh boy! How I wish we could
// go back in time and just kill whomever came up with all that little endian
// crap.
//

#if !defined(bwatson)
#define X_SUPPRESS_LOGS
#endif

#include "x_files.hpp"
#include "x_threads.hpp"
#include "e_network.hpp"
#include "EntropyAbstraction.h"

#ifdef TARGET_PS2

//==============================================================================
void abstract_ResolveHostname(const char* Hostname, s32 port, struct sockaddr_in* addr)
{
    (void)Hostname;
    (void)port;
    (void)addr;
}

//==============================================================================
s32 abstract_SendTo ( SOCKET s, const char* pBuffer, s32 Length, s32 Flags, struct sockaddr* To, s32 ToLength)
{
    net_socket* pSocket = (net_socket*)s;
    net_address Remote(htonl(To->s_addr), htons(To->s_port));

    if( (Remote.GetIP() == 0) || (Remote.GetPort()==0) )
    {
        LOG_WARNING("abstract_SendTo","UDP packet attempted to go to invalid address:%s", Remote.GetStrAddress() );
        return 0;
    }
    if( pSocket->IsEmpty() )
    {
        LOG_WARNING("abstract_SendTo","Packet was trying to be sent when the local socket was empty." );
        return 0;
    }
    (void)Flags;
    (void)ToLength;

    LOG_MESSAGE("abstract_SendTo","UDP packet size:%d sent to:%s", Length, Remote.GetStrAddress() );
    while( Length )
    {
        s32 Size = MIN( Length, MAX_PACKET_SIZE );
        xbool Result = pSocket->Send( Remote, pBuffer, Size );
        if( Result==FALSE )
        {
            return FALSE;
        }
        pBuffer += Size;
        Length  -= Size;
    }
    return TRUE;
}

//==============================================================================
SOCKET abstract_Socket ( s32 af, s32 type, s32 protocol )
{
    net_socket* pSocket;
    
    (void)af;
    (void)type;
    (void)protocol;
    pSocket = new net_socket;
    ASSERT(pSocket);
    if( type == SOCK_DGRAM)
    {
        pSocket->Bind();
    }
    else
    {
        pSocket->Bind(-1,NET_FLAGS_TCP);
    }
    return (SOCKET)pSocket;
}

//==============================================================================
void abstract_CloseSocket ( SOCKET s )
{
    net_socket* pSocket = (net_socket*)s;

    LOG_MESSAGE("abstract_CloseSocket","Socket closed address:%s",pSocket->GetStrAddress());
    pSocket->Close();
    delete pSocket;
}

//=============================================================
s32 abstract_RecvFrom ( SOCKET s, char* pBuffer, s32 Length, s32 Flags, struct sockaddr* From, s32* FromLength)
{
    net_socket* pSocket = (net_socket*)s;
    net_address Remote(htonl(From->s_addr), htons(From->s_port));
    s32 AmountRead;

    (void)Flags;
    (void)FromLength;

    AmountRead = Length;
    xbool ok = pSocket->Receive( Remote, pBuffer, AmountRead );
    if( ok )
    {
        if( AmountRead > Length )
        {
            LOG_WARNING("abstract_RecvFrom","Got data larger than our buffer, BufferSize:%d, DataSize:%d",Length,AmountRead);
        }
        LOG_MESSAGE("abstract_RecvFrom","TCP packet size:%d received through %s", AmountRead, Remote.GetStrAddress());
        From->s_addr = ntohl(Remote.GetIP());
        From->s_port = ntohs(Remote.GetPort());
        return AmountRead;
    }

    return 0;
}

//==============================================================================
void abstract_Sleep ( s32 msec )
{
    x_DelayThread(msec);
}

//==============================================================================
s32 abstract_SetBlocking ( SOCKET s, xbool IsBlocking )
{
    net_socket* pSocket = (net_socket*)s;

    pSocket->SetBlocking(IsBlocking);
    return IsBlocking;
}


//==============================================================================
struct hostent* abstract_GetLocalHost ( void )
{
    interface_info  Info;
    static in_addr  LocalAddr;
    static char*    NamePtr[]={(char*)&LocalAddr, NULL };

    static hostent  HostEnt = {    "Local PS2",
        NULL,
        1,
        1,
        NamePtr};


    net_GetInterfaceInfo(-1,Info);

    LocalAddr.s_addr = ntohl(Info.Address);
    
    return &HostEnt;
}

//==============================================================================
u32 abstract_Resolve ( const char* pHostname )
{
    u32 Addr = net_ResolveName( pHostname );
    if( Addr==0 )
    {
        return (u32)-1;
    }
    return (u32)htonl(Addr);
    
}

//==============================================================================
char* abstract_ResolveIP ( struct in_addr in )
{
    static net_address addr;
    
    addr = net_address(ntohl(in.s_addr),0);

    return (char*)addr.GetStrIP();
}

//==============================================================================
struct hostent* abstract_GetHostByName ( const char* pHostname )
{
    static struct hostent   h;
    static char*            pName[2];
    static s32              Resolved;

    Resolved = htonl(net_ResolveName( pHostname ));
    if( Resolved == 0 )
    {
        return NULL;
    }
    pName[0] = (char*)&Resolved;
    pName[1] = NULL;

    h.h_addr_list = pName;

    return &h;
}

//==============================================================================
s32 abstract_Bind ( SOCKET s, const struct sockaddr* addr, s32 namelen )
{
    net_socket* pSocket = (net_socket*)s;

    if( (u32)pSocket->GetPort() != ntohs(addr->s_port) )
    {
        pSocket->Close();
        pSocket->Bind( ntohs(addr->s_port), pSocket->GetFlags() );
    }
    (void)namelen;
    return 0;

}


//==============================================================================
s32 abstract_GetSockName ( SOCKET s, struct sockaddr* name, s32* length )
{
    (void)s;
    (void)name;
    (void)length;
    return 0;
}

//==============================================================================
// Note: Receive on a tcp socket returns > 0, if data is present, SOCKET_ERROR (with
// error WSAEWOULDBLOCK) if there is no data and 0 if the socket was forcibly closed.
s32 abstract_Recv ( SOCKET s, char* pBuffer, s32 Length, s32 Flags )
{
    net_socket* pSocket=(net_socket*)s;
    (void)Flags;
    s32 AmountRead;

    // Check to see if we're still connected
    interface_info Info;

    net_GetInterfaceInfo(-1,Info);
    if( Info.Address == 0 )
    {
        return 0;
    }

    AmountRead = Length;
    s32 ok = pSocket->Receive( pBuffer, AmountRead );
    if( ok )
    {
        if( AmountRead>Length)
        {
            LOG_WARNING("abstract_Recv","Got a packet too big for buffer. BufferSize:%d, PacketSize:%d", Length,AmountRead);
        }

        LOG_MESSAGE("abstract_Recv","TCP packet size:%d, received from %s", AmountRead, pSocket->GetStrAddress() );
        return AmountRead;
    }
    if( pSocket->IsClosed() )
    {
        return 0;
    }
    return -1;
}

//==============================================================================
s32 abstract_Send ( SOCKET s, const char* pBuffer, s32 Length, s32 Flags )
{
    net_socket* pSocket=(net_socket*)s;
    s32 SentLength=0;

    (void)Flags;

    // Check to see if we're still connected
    interface_info Info;

    net_GetInterfaceInfo(-1,Info);
    if( (Info.Address == 0) || (pSocket->IsEmpty()) )
    {
        return -1;
    }
    //
    LOG_MESSAGE("abstract_Send","TCP packet size:%d, sent through %s",Length, pSocket->GetStrAddress());
    while( Length )
    {
        s32 Size = MIN( Length, MAX_PACKET_SIZE );
        xbool Result = pSocket->Send( pBuffer, Size );
        if( Result==FALSE )
        {
            return -1;
        }
        pBuffer     += Size;
        Length      -= Size;
        SentLength  += Size;
    }
    return SentLength;
}

//==============================================================================
s32 abstract_Connect ( SOCKET s, const struct sockaddr* addr, s32 namelen )
{
    xbool ok;
    net_socket*     pSocket=(net_socket*)s;
    net_address     Remote(htonl(addr->s_addr), htons(addr->s_port));
    xtimer          t;
    (void)namelen;

    if( pSocket->IsConnected() )
    {
        return 0;
    }

    t.Start();

    ok = pSocket->Connect( Remote, -1, 5.0f );
    t.Stop();
    LOG_MESSAGE( "abstract_Connect", "TCP Connect to:%s, through:%s. status:%d, time:%2.02fms", Remote.GetStrAddress(), pSocket->GetStrAddress(), ok, t.ReadMs() );
    if( !ok )
    {
        return -1;
    }
    return 0;
}

//==============================================================================
xbool abstract_CanSend( SOCKET s )
{
    net_socket* pSocket=(net_socket*)s;

    return pSocket->CanSend();
    
}

//==============================================================================
xbool abstract_CanReceive( SOCKET s )
{
    net_socket* pSocket=(net_socket*)s;

    return pSocket->CanReceive();
    
}

//==============================================================================
s32 abstract_Shutdown( SOCKET s, s32 How )
{
    net_socket* pSocket=(net_socket*)s;
    (void)How;
    (void)pSocket;
    return 0;
}

//==============================================================================
s32 abstract_Listen( SOCKET s, s32 MaxConnect )
{
    net_socket* pSocket=(net_socket*)s;
    (void)MaxConnect;
    (void)pSocket;
    ASSERT(FALSE);
    return 0;
}

//==============================================================================
s32 abstract_Accept( SOCKET s, struct sockaddr* Addr, s32 namelen )
{
    net_socket* pSocket=(net_socket*)s;
    (void)Addr;
    (void)namelen;
    (void)pSocket;
    ASSERT(FALSE);
    return 0;
}

//==============================================================================
#if defined(X_ASSERT)
void abstract_Assert( const char* pMessage, const char* pFile, s32 Line )
{
    RTFHandler( pFile, Line, pMessage, NULL );
    BREAK;
}
#endif

//==============================================================================
u32 abstract_GetTime( void )
{
    return (u32)(x_GetTime() / x_GetTicksPerMs());
}
//==============================================================================
extern "C" 
{

void* gsimalloc( size_t size )
{
    void* pData;
    pData = x_malloc( size );
    if( pData )
    {
        x_memset(pData,0,size);
    }
    return pData;
}

//==============================================================================
extern "C" void gsifree( void* ptr )
{
    x_free( ptr );
}

//==============================================================================
extern "C" void* gsirealloc( void* ptr, size_t size )
{
    return x_realloc( ptr, size );
}

//==============================================================================
u16 htons(u16 n)
{
    return BIG_ENDIAN_16(n);
}

//==============================================================================
u16 ntohs(u16 n)
{
    return BIG_ENDIAN_16(n);
}

//==============================================================================
u32 htonl(u32 n)
{
    return BIG_ENDIAN_32(n);
}

//==============================================================================
u32 ntohl(u32 n)
{
    return BIG_ENDIAN_32(n);
}

}//End Extern "C"

//==============================================================================

s32 GOAGetLastError( SOCKET s )
{
    interface_info Info;
    net_GetInterfaceInfo(-1,Info);
    net_socket* pSocket=(net_socket*)s;

    if( pSocket->IsClosed() )
    {
        return ECONNRESET;
    }

    if( (Info.Address == 0) || (pSocket->IsEmpty()) )
    {
        return ENOTCONN;
    }

    return EWOULDBLOCK;
}
#endif // TARGET_PS2

