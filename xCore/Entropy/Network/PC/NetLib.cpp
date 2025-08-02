//==============================================================================
//
//  NETLIB.CPP
//
//==============================================================================
#include "x_files.hpp"

#if !defined(TARGET_PC)
#error This should not be compiled for this target platform. PC only.
#endif

// Auto include WinSock libs in a .NET build
#if _MSC_VER >= 1300
#pragma comment( lib, "ws2_32.lib" )
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include "e_Network.hpp"

//==============================================================================

static s32 s_STAT_NPacketsSent;
static s32 s_STAT_NPacketsReceived;
static s32 s_STAT_NBytesSent;
static s32 s_STAT_NBytesReceived;
static s32 s_STAT_NAddressesBound;
extern xtimer NET_SendTime;
extern xtimer NET_ReceiveTime;

static xbool s_Inited = FALSE;

//==============================================================================

void    sys_net_Init( void )
{
    ASSERT( !s_Inited );
    s_Inited = TRUE;

    s_STAT_NPacketsSent     = 0;
    s_STAT_NPacketsReceived = 0;
    s_STAT_NBytesSent       = 0;
    s_STAT_NBytesReceived   = 0;
    s_STAT_NAddressesBound  = 0;

    WSADATA wsadata;

    // initialize the appropriate sockets
    WSAStartup( 0x0101, &wsadata );
}

//==============================================================================

void    sys_net_Kill( void )
{
    ASSERT( s_Inited );
    s_Inited = FALSE;

    WSACleanup();
}

//==============================================================================

xbool   net_IsInited    ( void )
{
    return s_Inited;
}

//==============================================================================

xbool    net_socket::Bind( s32 StartPort,s32 Flags )
{
    ASSERT( s_Inited );

    // Clear address in case we need to exit early

    struct sockaddr_in addr;
    SOCKET sd_dg;

    // create an address and bind it
    x_memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_family      = PF_INET;
    addr.sin_port        = htons(StartPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // create a socket
    sd_dg = socket( PF_INET, SOCK_DGRAM, 0 );

    // attempt to bind to the port
    while( bind( sd_dg, (struct sockaddr *)&addr, sizeof(addr) ) == SOCKET_ERROR )
    {
        // increment port if that port is assigned
        if( WSAGetLastError() == WSAEADDRINUSE )
        {
            StartPort++;
            addr.sin_port = htons( StartPort );
        }
        // if some other error, nothing we can do...abort
        else
        {
            return FALSE;
            //StartPort = net_NOSLOT;
            //break;
        }
    }

    // set this socket to non-blocking, so we can poll it
    u_long  dwNoBlock = !(Flags & NET_FLAGS_BLOCKING);
    ioctlsocket( sd_dg, FIONBIO, &dwNoBlock );

    // fill out the slot structure
    // first, determine local IP address
    char tbuf[128];
    hostent* pHe;
    struct in_addr in_IP;

    gethostname( tbuf, 127 );
    pHe = gethostbyname( tbuf );

    in_IP = *(struct in_addr *)(pHe->h_addr);
    m_Address.Setup( ntohl(in_IP.s_addr), StartPort );
    m_Socket = (u32)sd_dg;

    if(Flags & NET_FLAGS_BROADCAST)
    {
        u_long dwBroadcast;
        dwBroadcast = TRUE;
        setsockopt(m_Socket,SOL_SOCKET,SO_BROADCAST,(char *)&dwBroadcast,sizeof(u_long));
    }

    s_STAT_NAddressesBound++;
    return TRUE;
}

//==============================================================================

void    net_socket::Close      ( void )
{
    ASSERT( s_Inited );

    s_STAT_NAddressesBound--;

    closesocket( m_Socket );
    m_Socket = BAD_SOCKET;
}

//==============================================================================

xbool   sys_net_Receive  ( net_socket&   Local,
                           net_address&  Remote,
                           void*         pBuffer,
                           s32&          BufferSize )
{
    s32   RetSize;

    ASSERT( s_Inited );

    struct sockaddr_in sockfrom;
    int addrsize = sizeof(sockaddr_in);

    // receive any incoming packet
    RetSize = recvfrom( Local.m_Socket, 
                        (char*)pBuffer, 
                        BufferSize, 
                        0, 
                        (struct sockaddr *)&sockfrom, 
                        &addrsize );

    // if a packet was received
    if ( RetSize > 0 )
    {
        // fill out the "From" with the appropriate information
        Remote.Setup( ntohl(sockfrom.sin_addr.s_addr), ntohs(sockfrom.sin_port) );
        ASSERT( RetSize <= BufferSize );
        BufferSize = RetSize;

        s_STAT_NPacketsReceived++;
        s_STAT_NBytesReceived += BufferSize;
        return TRUE;
    }
    else
    {
        s32 LastError = WSAGetLastError();
        // We could use WSAECONNRESET to signify the socket was closed by the
        // target side. This *should* only apply to TCP sockets but it also
        // seems to apply in Windows for UDP sockets if sent to the same machine.
        if ( (LastError != WSAEWOULDBLOCK) && (LastError != WSAECONNRESET) )
        {
            x_DebugMsg("RecvFrom returned an error %d\n",LastError);
        }
        BufferSize = 0;
    }

    // No packet received
    return FALSE;
}

//==============================================================================

void    sys_net_Send  (         net_socket&   Local, 
                          const net_address&  Remote, 
                          const void*         pBuffer, 
                          s32                 BufferSize )
{
    s32 status;
    ASSERT( s_Inited );

    s_STAT_NPacketsSent++;
    s_STAT_NBytesSent += BufferSize;

    struct sockaddr_in sockto;

    // address your package and stick a stamp on it :-)
    x_memset(&sockto,0,sizeof(struct sockaddr_in));
    sockto.sin_family       = PF_INET;
    sockto.sin_port         = htons(Remote.GetPort());
    sockto.sin_addr.s_addr  = htonl( Remote.GetIP() );
    status = sendto( Local.m_Socket, (const char*)pBuffer, BufferSize, 0, (struct sockaddr*)&sockto, sizeof(sockto) );
    if (status<=0)
    {
        x_DebugMsg("SendTo returned an error code %d\n",WSAGetLastError());
    }
}


//==============================================================================


void net_GetInterfaceInfo(s32 id,interface_info &info)
{
    INTERFACE_INFO InterfaceList[8];
    unsigned long nBytesReturned;
    s32 status;
    SOCKET socket;
    s32 nInterfaces;

    info.Address    = 0;
    info.Broadcast  = 0;
    info.Nameserver = 0;
    info.Netmask    = 0;
    info.IsAvailable= TRUE;

    socket = WSASocket(AF_INET,SOCK_DGRAM,0,0,0,0);
    if (socket == SOCKET_ERROR)
    {
        return;
    }
    status = WSAIoctl(socket,SIO_GET_INTERFACE_LIST,0,0,&InterfaceList,sizeof(InterfaceList),&nBytesReturned,0,0);
    closesocket(socket);
    if (status == SOCKET_ERROR)
    {
        return;
    }
    
    nInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

    if (id<0)
    {
        for (id=0;id<nInterfaces;id++)
        {
            if ( (InterfaceList[id].iiFlags & IFF_LOOPBACK)==0 )
            {
                break;
            }
        }
        // 
        // if we can't find any appropriate interfaces, let's just use the
        // loopback if it's present.
        //
        if (id==nInterfaces)
        {
            id=0;
        }
    }
    info.Address    = ntohl(InterfaceList[id].iiAddress.AddressIn.sin_addr.S_un.S_addr);
    info.Netmask    = ntohl(InterfaceList[id].iiNetmask.AddressIn.sin_addr.S_un.S_addr);
    info.Broadcast  = (info.Address & info.Netmask) | ~info.Netmask;
}

s32     net_ResolveName( const char* pStr )
{
    LPHOSTENT hostent;
    struct in_addr in_addrIP;

    hostent = gethostbyname(pStr);

    if ( hostent == NULL )
        return 0;

    in_addrIP = *(struct in_addr *)(hostent->h_addr_list[0]);
    return in_addrIP.S_un.S_addr;
}

void    net_ResolveIP( u32 IP, char* pStr )
{      
    net_address Dummy(IP,0);
    x_strcpy( pStr, Dummy.GetStrIP() );
}

void    net_SetDialupInfo(char *pNumber,char *pUsername,char *pPassword)
{
}

void    net_StartDial       (s32 nRetries,s32 Timeout)
{
}

void    net_ActivateConfig(xbool on)
{
    (void)on;
}

s32 net_GetConfigList(const char *pPath,net_config_list *pConfigList)
{
    (void)pPath;
    x_memset(pConfigList,0,sizeof(net_config_list));
    return 0;
}

s32 net_SetConfiguration(const char *pPath,s32 configindex)
{
    (void)pPath;
    (void)configindex;
    return 0;
}

s32 net_GetAttachStatus(s32 &InterfaceId)
{
    InterfaceId = 0;
    return 0;
}

s32		net_GetSystemId		(void)
{
    return( x_rand() );
}

void net_BeginConfig(void)
{
}

void net_EndConfig(void)
{
}

void net_GetConnectStatus(connect_status &Status)
{
}

//-----------------------------------------------------------------------------
xbool net_socket::CanReceive( void )
{
	fd_set          fd;
	struct timeval  timeout;
	int             rcode;

	// setup the fd set
	FD_ZERO(&fd);
	FD_SET(m_Socket, &fd);

	// setup the timeout
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// do the actual select
	rcode = select(FD_SETSIZE, &fd, NULL, NULL, &timeout);
	if((rcode == SOCKET_ERROR) || (rcode == 0))
		return FALSE;

    return TRUE;
}

//-----------------------------------------------------------------------------
xbool net_socket::CanSend( void )
{
	fd_set          fd;
	struct timeval  timeout;
	int             rcode;

	// setup the fd set
	FD_ZERO(&fd);
	FD_SET(m_Socket, &fd);

	// setup the timeout
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// do the actual select
	rcode = select(FD_SETSIZE, NULL, &fd, NULL, &timeout);
	if((rcode == SOCKET_ERROR) || (rcode == 0))
		return FALSE;

    return TRUE;
}
