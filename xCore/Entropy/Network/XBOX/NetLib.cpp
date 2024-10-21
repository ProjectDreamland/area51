//==============================================================================
//
//  NETLIB.CPP
//
//==============================================================================
#include "x_files.hpp"

#if !defined(TARGET_XBOX)
#error This should not be compiled for this target platform. XBOX only.
#endif

#ifdef TARGET_XBOX
#   include "xbox\xbox_private.hpp"
#endif

#include "e_Network.hpp"

//==============================================================================

static  s32     s_STAT_NPacketsSent;
static  s32     s_STAT_NPacketsReceived;
static  s32     s_STAT_NBytesSent;
static  s32     s_STAT_NBytesReceived;
static  s32     s_STAT_NAddressesBound;
extern  xtimer  NET_SendTime;
extern  xtimer  NET_ReceiveTime;

static  xbool   s_Inited = FALSE;

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

    XNetStartupParams   xnsp;
    s32                 err;

    memset(&xnsp, 0, sizeof(xnsp));
    xnsp.cfgSizeOfStruct                = sizeof(XNetStartupParams);
    xnsp.cfgFlags                       = XNET_STARTUP_BYPASS_SECURITY;
    xnsp.cfgSockDefaultRecvBufsizeInK   = 32;
    xnsp.cfgSockDefaultSendBufsizeInK   = 32;
    xnsp.cfgKeyRegMax                   = 100;          // Too many? Memory use?
    err                  = XNetStartup(&xnsp);

    WSADATA wsadata;

    // initialize the appropriate sockets
    WSAStartup( 0x0101, &wsadata );
}

//==============================================================================

void    sys_net_Kill( void )
{
    ASSERT( s_Inited );
    s_Inited = FALSE;

    // Do a cleanup call
    WSACleanup();
    XNetCleanup();
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

    if( StartPort <= 0 )
    {
        StartPort = x_irand( 8192, 16384 );
    }
    // create an address and bind it
    x_memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_family      = PF_INET;
    addr.sin_port        = htons(StartPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // create a socket
    if( Flags & NET_FLAGS_VDP )
    {
        sd_dg = socket( PF_INET, SOCK_DGRAM, IPPROTO_VDP );
    }
    else
    {
        sd_dg = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
    }

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
    u_long  dwNoBlock = TRUE;
    ioctlsocket( sd_dg, FIONBIO, &dwNoBlock );

    interface_info Info;
    net_GetInterfaceInfo(-1,Info);
    m_Address.SetIP( Info.Address );
    m_Address.SetPortNumber( StartPort ); //htons( StartPort );
    m_Socket = (u32)sd_dg;

    if( Flags & NET_FLAGS_BROADCAST )
    {
        u_long dwBroadcast;
        dwBroadcast = TRUE;
        setsockopt(m_Socket,SOL_SOCKET,SO_BROADCAST,(char *)&dwBroadcast,sizeof(u_long));
    }

    s_STAT_NAddressesBound++;
    return TRUE;
}

//==============================================================================
void net_socket::Close      ( void )
{
    ASSERT( s_Inited );

    s_STAT_NAddressesBound--;

    closesocket( m_Socket );
    m_Socket = BAD_SOCKET;
    m_Address.Clear();
}

//==============================================================================
xbool sys_net_Receive( net_socket&   Local,
                       net_address&  Remote,
                       void*         pBuffer,
                       s32&          BufferSize )
{
    s32   RetSize;

    ASSERT( s_Inited );

    struct sockaddr_in sockfrom;

    s32 addrsize = sizeof(sockaddr_in);

    // receive any incoming packet
    RetSize = recvfrom( Local.m_Socket, 
                        (char*)pBuffer, 
                        BufferSize, 
                        0, 
                        (struct sockaddr *)&sockfrom, 
                        &addrsize );

    // if a packet was received
    if( RetSize > 0 )
    {
        // fill out the "From" with the appropriate information
        Remote.SetIP( ntohl(sockfrom.sin_addr.s_addr) );
        Remote.SetPortNumber( ntohs(sockfrom.sin_port) );
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
        if( (LastError != WSAEWOULDBLOCK) && (LastError != WSAECONNRESET) )
        {
            x_DebugMsg("RecvFrom returned an error %d\n",LastError);
        }
        BufferSize = 0;
    }

    // No packet received
    return FALSE;
}

//==============================================================================
void sys_net_Send  ( net_socket&         Local, 
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
    sockto.sin_addr.s_addr  = htonl(Remote.GetIP());
    status = sendto( Local.m_Socket, (const char*)pBuffer, BufferSize, 0, (struct sockaddr*)&sockto, sizeof(sockto) );
    if( status<=0 )
    {
        x_DebugMsg("SendTo %s returned an error code %d\n",Remote.GetStrAddress(), WSAGetLastError());
    }
}


//==============================================================================
void net_GetInterfaceInfo(s32 id,interface_info &info)
{
    XNADDR  HostAddr;
    s32     Status;
    // Repeat while pending; OK to do other work in this loop
    x_memset( &info, 0, sizeof(info) );
    x_memset( &HostAddr, 0, sizeof(XNADDR) );

    Status = XNetGetTitleXnAddr( &HostAddr );
    info.IsAvailable    = (XNetGetEthernetLinkStatus()!=0);
    info.NeedsServicing = (Status & XNET_GET_XNADDR_TROUBLESHOOT )!=0;
    if( (Status == XNET_GET_XNADDR_PENDING) || (Status == XNET_GET_XNADDR_NONE) )
        return;
    info.Address        = ntohl(HostAddr.ina.s_addr);
    info.Broadcast      = -1;
    info.Nameserver     = 0;
    info.Netmask        = -1;
}

//==============================================================================
s32     net_ResolveName( const char* pStr )
{
    XNDNS* dns;
    s32 addr;

    if( (*pStr>='0') && (*pStr<='9') )
    {
        s32 i;
        addr = 0;
        for( i=0; i<4; i++ )
        {
            s32 val;
            val = 0;
            while( (*pStr >='0') && (*pStr<='9') )
            {
                val = val * 10 + *pStr-'0';
                pStr++;
            }
            addr = addr << 8 | val;
            if( *pStr != '.' )
                break;
            pStr++;
        }
    }
    else
    {
        XNetDnsLookup(pStr,NULL,&dns);

        if( dns->iStatus != 0 )
        {
            addr = 0;
        }
        else
        {
            addr = dns->aina[0].s_addr;
        }
        XNetDnsRelease(dns);
    }
        
    return addr;
}

//==============================================================================
void    net_ResolveIP( u32 IP, char* pStr )
{
    net_address Dummy(IP,0);
    x_strcpy( pStr, Dummy.GetStrIP() );
}

//==============================================================================
void    net_SetDialupInfo(char *pNumber,char *pUsername,char *pPassword)
{
}

//==============================================================================
void    net_StartDial       (s32 nRetries,s32 Timeout)
{
}

//==============================================================================
void    net_ActivateConfig(xbool on)
{
    (void)on;
}

//==============================================================================
s32 net_GetConfigList(const char *pPath,net_config_list *pConfigList)
{
    (void)pPath;
    x_memset(pConfigList,0,sizeof(net_config_list));
    return 0;
}

//==============================================================================
s32 net_SetConfiguration(const char *pPath,s32 configindex)
{
    (void)pPath;
    (void)configindex;
    return 0;
}

//==============================================================================
s32 net_GetAttachStatus(s32 &InterfaceId)
{
    InterfaceId = 0;
    return ATTACH_STATUS_ATTACHED;
}

//==============================================================================
s32 net_GetSystemId     (void)
{
    return( x_rand() );
}

//==============================================================================
void net_BeginConfig(void)
{
}

//==============================================================================
void net_EndConfig(void)
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

