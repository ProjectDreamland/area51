#define X_SUPPRESS_LOGS

#include "x_files.hpp"
#include "x_time.hpp"
#include "x_threads.hpp"

#include "e_Network.hpp"
#include "Network/ps2/netdefs.hpp"
#include "ps2/IopManager.hpp"
#include "netiop.hpp"
#include "e_memcard.hpp"

#include <eekernel.h>
#include <libgraph.h>
#include <sifrpc.h>


#ifdef TARGET_DEV
//#define INET_CONFIG_FILE "-no_decode" "\0" "host:C:/projects/t2/xcore/entropy/ps2/Modules/devnetcnf/main.cnf"
#define NETCNF_ICON_FILE "icon=host:"XCORE_PATH"/3rdparty/ps2/Sony/IOP/Modules/devnetcnf/sys_net.ico" "\0" "iconsys=host:"XCORE_PATH"/3rdparty/ps2/Sony/IOP/Modules/devnetcnf/icon.sys"
#else
#define NETCNF_ICON_FILE "icon=cdrom:\\MODULES\\CDNETCNF\\SYS_NET.ICO;1" "\0" "iconsys=cdrom:\\AREA51\\MODULES\\CDNETCNF\\ICON.SYS;1"
#endif
//-----------------------------------------------------------------------------
// LOCAL DEFINES
//-----------------------------------------------------------------------------
#define VALID_PACKET_SIZE (MAX_PACKET_SIZE+4)

//-----------------------------------------------------------------------------
// LOCAL STRUCTURES AND CLASSES
//-----------------------------------------------------------------------------
enum
{
    PORTSTAT_OK=0,
    PORTSTAT_CLOSED,
};

struct monitored_port
{
    monitored_port *        m_pNext;       // Ptr to next monitored port
    net_socket*             m_pSocket;
    xmesgq*                 m_pqPending;    // List of pending read requests on this port
    s32                     m_Status;
};

struct net_vars
{
    interface_info          m_Info[MAX_NET_DEVICES];// Information about the current interface
    s32                     m_InterfaceCount;
    xmesgq*                 m_pqSendPending;        // Queue for send requests
    xmesgq*                 m_pqAvailable;          // Queue for available packet blocks (shared between send/receive)
    xthread*                m_pSendThread;          // id of update thread
    xthread*                m_pRecvThread;          // id of kicker that will force update thread to run
    xbool                   m_BindBusy;             // Set if we're trying to bind a port (packets will be dropped when set)
    monitored_port  *       m_pPortList;           // List of ports opened
    s32                     m_MemcardHandle;
    net_sendrecv_request *  m_pRequests;      // Ptr to base memory block allocated for requests
    xmutex                  m_LockMutex;
};

//-----------------------------------------------------------------------------
// LOCAL VARIABLES
//-----------------------------------------------------------------------------
net_vars g_Net;

static sceSifQueueData  s_RecvQueueData;
static sceSifServeData  s_RecvServerData;
static sceSifClientData s_SendClientData;
static s32              s_PacketsGoingOut;

//-----------------------------------------------------------------------------
// LOCAL FUNCTIONS
//-----------------------------------------------------------------------------
static monitored_port * FindPort(const net_socket& Socket, monitored_port **pPrev = NULL);
static monitored_port * FindPort(s32 Port, monitored_port **pPrev = NULL);
void                    net_PeriodicSend(void);
void                    net_PeriodicRecv(void);
net_sendrecv_request *  AcquireBuffer(void);


static struct s_ReceiveBuffer
{
    net_sendrecv_request Request;
    char padding[32];
} s_ReceiveBuffer PS2_ALIGNMENT(64);


//-----------------------------------------------------------------------------
void sys_net_Init(void)
{
    net_sendrecv_request *pRequest;
    s32                 i;

    //
    // if already initialized, just bail right now.
    //
    g_Net.m_LockMutex.Enter();

    netiop_Init(IOP_BUFFER_SIZE);

    g_Net.m_pqAvailable = new xmesgq(MAX_PACKET_REQUESTS);
    g_Net.m_pqSendPending = new xmesgq( MAX_PACKET_REQUESTS);
    g_Net.m_pRequests      = (net_sendrecv_request *)x_malloc(sizeof(net_sendrecv_request)*MAX_PACKET_REQUESTS);
    g_Net.m_pPortList      = NULL;
    ASSERT(g_Net.m_pRequests);
    pRequest = g_Net.m_pRequests;
    for (i=0;i<MAX_PACKET_REQUESTS;i++)
    {
        g_Net.m_pqAvailable->Send(pRequest,MQ_BLOCK);
        pRequest++;
    }

    netiop_SendSyncRequest(NETCMD_INIT);
    x_memset(g_Net.m_Info,0,sizeof(g_Net.m_Info));

    g_Net.m_pSendThread = new xthread(net_PeriodicSend,"Net Periodic Send",8192,1);
    g_Net.m_pRecvThread = new xthread(net_PeriodicRecv,"Net Periodic Recv",8192,1);

    x_DebugMsg("Net init reported a system id of 0x%08x\n",net_GetSystemId());
  
    g_Net.m_LockMutex.Exit();
}

//-----------------------------------------------------------------------------
void sys_net_Kill(void)
{
    interface_info Info;

    net_ActivateConfig( FALSE );
    while( TRUE )
    {
        net_GetInterfaceInfo(-1,Info);
        if( Info.Address == 0 )
        {
            break;
        }
        x_DelayThread( 50 );
    }

    while( g_Net.m_pPortList )
    {
        x_DebugMsg("net_PS2Kill: WARNING: Port %d left open, closing\n",g_Net.m_pPortList->m_pSocket->GetPort());
        g_Net.m_pPortList->m_pSocket->Close();
    }
    g_Net.m_LockMutex.Enter();
    netiop_SendSyncRequest(NETCMD_KILL);
    delete g_Net.m_pSendThread;
    delete g_Net.m_pRecvThread;

    delete g_Net.m_pqAvailable;
    delete g_Net.m_pqSendPending;

    sceSifRemoveRpc(&s_RecvServerData,&s_RecvQueueData);
    sceSifRemoveRpcQueue(&s_RecvQueueData);

    x_free(g_Net.m_pRequests);
    netiop_Kill();
    g_Net.m_LockMutex.Exit();
}

//-----------------------------------------------------------------------------
xbool   net_IsInited    ( void )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
void    net_GetConnectStatus (connect_status &Status)
{
    net_connect_status     DialStatus;

    netiop_SendSyncRequest(NETCMD_GETCONNECTSTATUS,NULL,0,&DialStatus,sizeof(net_connect_status));
    Status.nRetries         = DialStatus.nRetries;
    Status.Status           = DialStatus.Status;
    Status.ConnectSpeed     = DialStatus.ConnectSpeed;
    Status.TimeoutRemaining = DialStatus.TimeoutRemaining;
    x_strcpy(Status.ErrorText,DialStatus.ErrorText);
}

//-----------------------------------------------------------------------------
void net_GetInterfaceInfo(s32 id,interface_info& info)
{
    xbool Found = FALSE;
    if (id==-1)
    {
        for( id=0; id<MAX_NET_DEVICES; id++ )
        {
            if( g_Net.m_Info[id].Address )
            {
                Found=TRUE;
                break;
            }
        }
        if( !Found )
        {
            id=0;
        }

    }

    info.Address        = BIG_ENDIAN_32(g_Net.m_Info[id].Address);
    info.Broadcast      = BIG_ENDIAN_32(g_Net.m_Info[id].Broadcast);
    info.Nameserver     = BIG_ENDIAN_32(g_Net.m_Info[id].Nameserver);
    info.Netmask        = BIG_ENDIAN_32(g_Net.m_Info[id].Netmask);
    info.IsAvailable    = g_Net.m_Info[id].IsAvailable;
    info.NeedsServicing = FALSE;
    //    netiop_SendSyncRequest(NETCMD_READCONFIG,&id,sizeof(s32),&info,sizeof(interface_info));
}

//-----------------------------------------------------------------------------
xbool   net_socket::Bind(s32 StartPort,s32 Flags)
{
    s32 port;
    s32 status[2];
    monitored_port *pPort;

    if( StartPort <= 0 )
    {
        StartPort = x_irand(16384,24576);
    }

    port = StartPort;

    m_Socket = BAD_SOCKET;
    if( Flags & NET_FLAGS_TCP )
    {
        interface_info Info;
        net_GetInterfaceInfo( -1, Info );
        m_Address.Setup( Info.Address, port );
        m_Flags = Flags;
        return TRUE;
    }

    g_Net.m_BindBusy = TRUE;
    while( TRUE )
    {
        status[0] = port;
        status[1] = Flags;
        if( netiop_SendSyncRequest(NETCMD_BIND,status,sizeof(status),status,sizeof(status))==FALSE )
        {
            return FALSE;
        }
        if (status[0] != -1)
            break;
        port++;

        if(port > StartPort + 256)
        {
            return FALSE;
        }
    }
    if( status[0]==BAD_SOCKET )
    {
        g_Net.m_BindBusy = FALSE;
        return FALSE;
    }

    pPort = (monitored_port *)x_malloc(sizeof(monitored_port));
    ASSERT(pPort);

    g_Net.m_LockMutex.Enter();
    pPort->m_pqPending  = new xmesgq(MAX_PACKET_REQUESTS);
    pPort->m_Status     = PORTSTAT_OK;
    pPort->m_pSocket    = this;
    pPort->m_pNext      = g_Net.m_pPortList;
    g_Net.m_pPortList   = pPort;
    m_Flags             = Flags;
    m_Socket            = status[0];
    m_Address.Setup( BIG_ENDIAN_32(status[1]),port );
    g_Net.m_LockMutex.Exit();

    //x_DebugMsg("Bound port %d, sock=%08x, ip=%08x\n",NewAddress.Port,NewAddress.Socket,NewAddress.IP);
    g_NetStats.AddressesBound++;
    // If we get a udp socket number of -1, this means
    // we didn't get anything bound properly.
    g_Net.m_BindBusy = FALSE;
//    x_printf("net_Bind: %d bytes free on iop\n",iop_GetFreeMemory());
    return( m_Socket != BAD_SOCKET);
}

//-----------------------------------------------------------------------------
void    net_socket::Close(void)
{
    u32 status;
    monitored_port *pPort,*pPrev;

    g_Net.m_LockMutex.Enter();
    // We may be making multiple calls so let's keep the request
    // block around for re-use.
    pPort = FindPort(*this,&pPrev);
    if (!pPort)
    {
        x_DebugMsg( "net_Unbind: Attempt to close port %d when not open\n", m_Address.GetPort() );
        g_Net.m_LockMutex.Exit();
        return;
    }

    if (pPrev)
    {
        pPrev->m_pNext = pPort->m_pNext;
    }
    else
    {
        g_Net.m_pPortList = pPort->m_pNext;
    }

    g_NetStats.AddressesBound--;

    s32 Count=0;

    while( TRUE )
    {
        void* pBuffer = pPort->m_pqPending->Recv( MQ_NOBLOCK );
        if( pBuffer==NULL )
            break;
        g_Net.m_pqAvailable->Send( pBuffer, MQ_NOBLOCK );
        Count++;
    }
    if( Count )
    {
        LOG_WARNING( "net_socket::Close", "Released %d unread buffers", Count );
    }
    delete pPort->m_pqPending;
    netiop_SendSyncRequest(NETCMD_UNBIND,&m_Socket,sizeof(s32),&status,sizeof(s32));
    m_Address.Clear();
    x_free(pPort);
    g_Net.m_LockMutex.Exit();
}

//-----------------------------------------------------------------------------
xbool net_socket::Accept(net_address& Remote)
{
    s32 status[3];
    monitored_port *pPort;

    ASSERTS(m_Flags & NET_FLAGS_TCP, "Must be bound as a TCP socket");
    status[0]=-1;
    if( netiop_SendSyncRequest(NETCMD_ACCEPT,&m_Socket,sizeof(m_Socket),status,sizeof(status))==FALSE )
    {
        return FALSE;
    };

    if( status[0] == BAD_SOCKET )
        return FALSE;

    m_Socket      = status[0];
    Remote.SetIP        ( status[1] );
    Remote.SetPortNumber( status[2] );

    pPort = (monitored_port *)x_malloc(sizeof(monitored_port));
    ASSERT(pPort);

    pPort->m_pqPending = new xmesgq(MAX_PACKET_REQUESTS);

    pPort->m_pSocket  = this;
    pPort->m_Status   = 0;
    g_Net.m_LockMutex.Enter();
    pPort->m_pNext    = g_Net.m_pPortList;
    g_Net.m_pPortList = pPort;
    g_Net.m_LockMutex.Exit();

    //x_DebugMsg("Bound port %d, sock=%08x, ip=%08x\n",NewAddress.Port,NewAddress.Socket,NewAddress.IP);
    g_NetStats.AddressesBound++;
    // If we get a udp socket number of -1, this means
    // we didn't get anything bound properly.
//    x_printf("net_Bind: %d bytes free on iop\n",iop_GetFreeMemory());
    return TRUE;
}

//-----------------------------------------------------------------------------
xbool   net_socket::Connect(const net_address& Remote, s32 LocalPort, f32 Timeout)
{
    s32             status[4];
    monitored_port* pPort;

    (void)LocalPort;

    ASSERTS(m_Flags & NET_FLAGS_TCP, "Must be bound as a TCP socket");

    status[0] = LocalPort;
    status[1] = BIG_ENDIAN_32(Remote.GetIP());
    status[2] = Remote.GetPort();
    status[3] = (s32)(Timeout * 1000.0f);

    m_Address.Clear();
    m_Socket = BAD_SOCKET;

    if( netiop_SendSyncRequest(NETCMD_CONNECT,status,sizeof(status),status,sizeof(status))==FALSE )
    {
        return FALSE;
    }

    m_Socket = status[0];
    if( m_Socket != BAD_SOCKET )
    {
        pPort = (monitored_port *)x_malloc(sizeof(monitored_port));
        ASSERT(pPort);

        pPort->m_pqPending = new xmesgq(MAX_PACKET_REQUESTS);

        pPort->m_pSocket  = this;
        pPort->m_Status   = 0;
        m_Socket          = status[0];
        m_Address.Setup(BIG_ENDIAN_32(status[1]),status[2]);

        g_Net.m_LockMutex.Enter();
        pPort->m_pNext    = g_Net.m_pPortList;
        g_Net.m_pPortList = pPort;
        g_Net.m_LockMutex.Exit();

        g_NetStats.AddressesBound++;
        return TRUE;
    }
    //x_debugmsg("bound port %d, sock=%08x, ip=%08x\n",newaddress.port,newaddress.socket,NewAddress.IP);

    return( FALSE );
}

//-----------------------------------------------------------------------------
xbool   net_socket::Listen( s32 LocalPort, s32 MaxClients )
{
    s32 status[2];

    (void)LocalPort;
    ASSERTS(m_Flags & NET_FLAGS_TCP, "Must be bound as a TCP socket");

    g_Net.m_BindBusy = TRUE;
    while (1)
    {
        status[0] = m_Socket;
        status[1] = MaxClients;
        if( netiop_SendSyncRequest(NETCMD_LISTEN,&status,sizeof(status),status,sizeof(status))==FALSE )
        {
            return BAD_SOCKET;
        }
    }
    return (status[0]);
}

//-----------------------------------------------------------------------------
xbool net_socket::CanReceive( void )
{
    monitored_port* pPort;
    interface_info Info;

    net_GetInterfaceInfo(-1, Info );
    if( Info.Address == 0 )
    {
        return FALSE;
    }

    pPort = FindPort( *this );
    if( pPort==NULL )
    {
        return FALSE;
    }
    return ( !pPort->m_pqPending->IsEmpty() );
}

//-----------------------------------------------------------------------------
xbool net_socket::CanSend( void )
{
    interface_info Info;

    net_GetInterfaceInfo(-1, Info );
    if( Info.Address == 0 )
    {
        return FALSE;
    }
    return (g_Net.m_pqSendPending->IsFull() == FALSE);
}

s32 net_receive_count=0;
//-----------------------------------------------------------------------------
xbool   sys_net_Receive  ( net_socket&   Local,
                           net_address&  Remote,
                           void*         pBuffer,
                           s32&          BufferSize )
{
    net_sendrecv_request *pPacket;
    monitored_port *pPort;

    ASSERT(Local.m_Socket != BAD_SOCKET);
    ASSERT(Local.GetIP());
    ASSERT(Local.GetPort());

    net_receive_count++;

    pPort = FindPort(Local);

    if (pPort)
    {
        // Check to see if the port was closed beneath our feet.
        pPacket=(net_sendrecv_request *)pPort->m_pqPending->Recv(MQ_NOBLOCK);
        if (pPacket)
        {
#ifdef X_DEBUG
            u32 *pData;
            pData = (u32 *)pPacket->Data;
            LOG_MESSAGE("sys_net_Receive","Receive from 0x%08x:%d, Length:%d data=%08x:%08x",pPacket->Header.Address, pPacket->Header.Port, pPacket->Header.Length, *pData++,*pData++);
#endif
            if (pPacket->Header.Length > VALID_PACKET_SIZE)
            {
                x_DebugMsg("netmgr: Packet from port %d, too big, truncated\n",pPort->m_pSocket->GetPort());
                pPacket->Header.Length = VALID_PACKET_SIZE;
            }
            // Was the socket closed on us?
            if( pPacket->Header.Status < 0 )
            {
                BREAK;
            }
            BufferSize = pPacket->Header.Length;
            x_memcpy(pBuffer,pPacket->Data,BufferSize);
            Remote.Setup( BIG_ENDIAN_32(pPacket->Header.Address), pPacket->Header.Port );
            g_Net.m_pqAvailable->Send(pPacket,MQ_BLOCK);
            return TRUE;
        }
        else
        {
            if( pPort->m_Status & NET_FLAGS_CLOSED )
            {
                // This should only happen if we're on a tcp connection and it has been force closed.
                BufferSize = -1;
                return FALSE;
            }
            //LOG_WARNING("sys_net_Receive","Receive but no packet, address:%s",Remote.GetStrAddress());
            BufferSize=0;
            return FALSE;
        }
    }

    BufferSize = -1;
    return FALSE;
}

//-----------------------------------------------------------------------------
void    sys_net_Send  (         net_socket&   Local, 
                          const net_address&  Remote, 
                          const void*         pBuffer, 
                          s32                 BufferSize )
{
    net_sendrecv_request *pPacket;

    // Get a packet that we need to send data with
    pPacket = (net_sendrecv_request *)g_Net.m_pqAvailable->Recv(MQ_BLOCK);
    if( pPacket==NULL )
    {
        return;
    }

    ASSERT(BufferSize <= VALID_PACKET_SIZE);
    x_memcpy(pPacket->Data,pBuffer,BufferSize);
    // Set up the header to be sent to the IOP
    pPacket->Header.Address = BIG_ENDIAN_32(Remote.GetIP());
    pPacket->Header.Port    = Remote.GetPort();
    pPacket->Header.Socket  = Local.m_Socket;
    pPacket->Header.Length  = BufferSize;
#ifdef X_DEBUG
    u32 *pData;
    pData = (u32 *)pPacket->Data;
    LOG_MESSAGE("sys_net_Send","Send to 0x%08x:%d, Length:%d, data=%08x:%08x",pPacket->Header.Address,pPacket->Header.Port, pPacket->Header.Length, *pData++,*pData++);
#endif
    g_Net.m_pqSendPending->Send(pPacket,MQ_BLOCK);
}

//-----------------------------------------------------------------------------
s32 net_ResolveName( char const * pStr )
{
    u32 ipaddr;
    s32 Retries;
    interface_info Info;

    Retries = 15;

    while( Retries )
    {
        net_GetInterfaceInfo( -1, Info );
        if( Info.IsAvailable==FALSE )
        {
            return 0;
        }
        if( netiop_SendSyncRequest(NETCMD_RESOLVENAME,(void *)pStr,x_strlen(pStr)+1,&ipaddr,sizeof(s32))==FALSE )
        {
            return 0;
        }
        if( ipaddr != 0 )
            break;
        Retries--;
    }
    LOG_MESSAGE("net_ResolveName","Resolve name %s to 0x%08x",pStr,ipaddr);

    return BIG_ENDIAN_32(ipaddr);
}

//-----------------------------------------------------------------------------
void net_ResolveIP( u32 IP, char* pStr )
{
    netiop_SendSyncRequest(NETCMD_RESOLVEIP,&IP,sizeof(u32),pStr,MAX_NET_NAME_LENGTH);
    LOG_MESSAGE("net_ResolveIP","Resolve ID %08x to %s",IP,pStr);
}


//-----------------------------------------------------------------------------
//------------- Utility functions, just internal
//-----------------------------------------------------------------------------
static monitored_port *FindPort(s32 port,monitored_port **pPrev)
{
    monitored_port *pPort,*pPrevPort;

    pPort=g_Net.m_pPortList;
    pPrevPort = NULL;

    while( pPort )
    {
        if( pPort->m_pSocket->GetPort() == port )
        {
            if( pPrev )
            {
                *pPrev = pPrevPort;
            }
            return pPort;
        }
        pPrevPort = pPort;
        pPort = pPort->m_pNext;
    }
    return NULL;
}

//-----------------------------------------------------------------------------
static monitored_port *FindPort(const net_socket& Socket,monitored_port **pPrev)
{
    return FindPort(Socket.GetPort(),pPrev);
}

//-----------------------------------------------------------------------------
net_sendrecv_request *AcquireBuffer(void)
{
    net_sendrecv_request *pRequest;
    monitored_port       *pPort,*pLongestPort;
    s32                   nRequests;
    s32                   status;
    s32                   Retries;

    Retries = 30;
    while( Retries )
    {
        pRequest = (net_sendrecv_request *)g_Net.m_pqAvailable->Recv(MQ_NOBLOCK);
        pLongestPort = NULL;
        if (pRequest)
            return pRequest;
        x_DelayThread(2);
        Retries--;
    }

    pPort = g_Net.m_pPortList;
    nRequests = 0;

    while( TRUE )
    {
        if( pPort )
        {
            if( pPort->m_pqPending->ValidEntries() >= nRequests )
            {
                nRequests = pPort->m_pqPending->ValidEntries();
                pLongestPort = pPort;
            }
        }
        else
        {
            break;
        }
        pPort = pPort->m_pNext;
    }

    if( pLongestPort && nRequests )
    {
        // We're going to steal from a port queue. We continue stealing until
        // the number of entries in the port queue fall below the threshold for
        // a block purge
        nRequests = MIN( nRequests, 8 );
        while( nRequests )
        {
            pRequest = (net_sendrecv_request *)pLongestPort->m_pqPending->Recv(MQ_NOBLOCK);
            ASSERT(pRequest);

            nRequests--;

            status = g_Net.m_pqAvailable->Send(pRequest,MQ_NOBLOCK);
            ASSERT(status);
        }
        pRequest = (net_sendrecv_request *)g_Net.m_pqAvailable->Recv(MQ_BLOCK);
        ASSERT(pRequest);
        x_DebugMsg("netlib: Ran out of buffers, purged %d requests from port %d\n",nRequests,pLongestPort->m_pSocket->GetPort());
    }
    else
    {
        //
        // We stole from the send queue. This *SHOULD* never happen since the send queue should always be
        // relatively short.
        //
        pRequest = (net_sendrecv_request *)g_Net.m_pqSendPending->Recv(MQ_NOBLOCK);
        ASSERT(pRequest);
        x_DebugMsg("netlib: Ran out of buffers, stole from send queue, addr=%08x,port=%d\n",pRequest->Header.Address,pRequest->Header.Port);
    }

    return pRequest;
}
//-----------------------------------------------------------------------------
//------------- Threads, just internal
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void *net_RecvCallback(u32 command,void *data,s32 size)
{
    net_sendrecv_request *pRequest;
    net_sendrecv_request *pPacket;
    monitored_port       *pPort;
    xbool                status;
    s32                  i;

    (void)command;
    (void)size;

    // If we issue this command, it's a interface_info packet coming in
    if( command == INEV_NET_NEWIFC_CMD )
    {
        interface_info *pInfo;

        pInfo = (interface_info *)data;
        for( i=0; i<MAX_NET_DEVICES; i++ )
        {
            g_Net.m_Info[i] = pInfo[i];
        }
        return data;
    }

    ASSERT(command == INEV_NET_RECV_CMD);
    pPacket = (net_sendrecv_request *)data;

    pPort = FindPort( pPacket->Header.LocalPort );
    if( pPort )
    {
        if( pPacket->Header.Status )
        {
            pPort->m_Status = pPacket->Header.Status;
        }
        else
        {
            pRequest = AcquireBuffer();
            ASSERT(pRequest);
            LOG_MESSAGE( "net_RecvCallback", "Length:%d, Dest:%d", pPacket->Header.Length, pPacket->Header.LocalPort );
            x_memset(pRequest, x_rand(), sizeof(net_sendrecv_request) );
            x_memcpy(pRequest,pPacket,sizeof(net_sendrecv_header)+pPacket->Header.Length);
            status = pPort->m_pqPending->Send( pRequest,MQ_NOBLOCK );
            ASSERT(status);
        }
    }
    else
    {
        LOG_WARNING( "net_RecvCallback", "Packet received for unknown port, Target:%d", pPacket->Header.LocalPort );
    }
#ifdef bwatson
    x_memset(data, 0xcf, sizeof(net_sendrecv_request)) ;
#endif
    return data;

}

void net_PeriodicRecv(void)
{
    xthread* pThread = x_GetCurrentThread();

    ASSERT( pThread );
    sceSifSetRpcQueue( &s_RecvQueueData, GetThreadId() );
    sceSifRegisterRpc( &s_RecvServerData, INEV_NET_RECV_DEV, net_RecvCallback, (void*)(0x20000000|(s32)&s_ReceiveBuffer), NULL, NULL, &s_RecvQueueData );

    while( pThread->IsActive() )
    {
        sceSifServeData *pServerData;

        while( TRUE )
        {
            pServerData = sceSifGetNextRequest( &s_RecvQueueData );
            if( pServerData == NULL )
            {
                break;
            }
            sceSifExecRequest( pServerData );
        }
        SleepThread();
    }
}
//-----------------------------------------------------------------------------
void net_PeriodicSend(void)
{
    net_sendrecv_request *pRequest;
    s32 result;
    s32 Count;
    xthread* pThread = x_GetCurrentThread();

    ASSERT( pThread );

    while( TRUE ) 
    {
        result = sceSifBindRpc ( &s_SendClientData, INEV_NET_SEND_DEV, 0 );
        ASSERTS(result>=0,"error: sceSifBindRpc for PeriodicSend failed");

        if( s_SendClientData.serve != 0 ) break;
    }
    x_DebugMsg("Send init complete\n");

    Count = 0;
    while( pThread->IsActive() )
    {
        pRequest = (net_sendrecv_request *)g_Net.m_pqSendPending->Recv(MQ_BLOCK);
        // This can return NULL if we're needing to abort networking
        if( pThread->IsActive()==FALSE )
        {
            break;
        }
        ASSERT(pRequest);
        Count++;

#ifdef X_DEBUG
        u32 *pData;

        pData = (u32 *)pRequest->Data;
        LOG_MESSAGE("net_PeriodicSend","To:%08x:%d, Length:%d, data:%08x:%08x",pRequest->Header.Address,pRequest->Header.Port, pRequest->Header.Length, *pData++,*pData++);
#endif
        sceSifCallRpc (&s_SendClientData,  
                INEV_NET_SEND_CMD,0,
                pRequest,sizeof(net_sendrecv_header)+pRequest->Header.Length,
                &pRequest->Header,sizeof(net_sendrecv_header),
                NULL,NULL);
        g_Net.m_pqAvailable->Send(pRequest,MQ_BLOCK);
    }
    s_PacketsGoingOut = Count;
}
static s32 CheckCard( const char* pPath )
{
    s32             CardId;
    memcard_error   Status;

    if( x_memcmp(pPath,"mc",2)!=0 )
    {
        return NET_ERR_OK;
    }
    CardId = pPath[2]-'0';
    Status = g_MemcardHardware.GetCardStatus( CardId );

    switch( Status )
    {
    case MEMCARD_SUCCESS:
    case MEMCARD_CARD_CHANGED:
        break;
    case MEMCARD_NO_CARD:
        return NET_ERR_NO_CARD;
        break;
    case MEMCARD_UNFORMATTED:
        return NET_ERR_CARD_NOT_FORMATTED;
        break;
    case MEMCARD_NOT_A_MEMCARD:
        return NET_ERR_INVALID_CARD;
        break;
    default:
        ASSERT(FALSE);
        break;
    }
    return NET_ERR_OK;
}
//-----------------------------------------------------------------------------
s32 net_GetConfigList(const char *pPath,net_config_list *pConfigList)
{
    s32 error;

    error = 0;
    // Run through initial memory card checks to see if we can read from the memcard. Return an error
    // accordingly.
    if( g_NetIopMgr.m_SmapHandle<=0 )
    {
        return NET_ERR_NO_HARDWARE;
    }
    
    error = CheckCard( pPath );
    if( error != 0 )
    {
        return error;
    }
    netiop_SendSyncRequest(NETCMD_GETCONFIGLIST,(void *)pPath,x_strlen(pPath)+1,pConfigList,sizeof(net_config_list));
//    netiop_SendSyncRequest(NETCMD_GETCONFIGLIST,(void *)pPath,x_strlen(pPath)+1,pConfigList,sizeof(net_config_list));
    if (pConfigList->Count < 0)
    {
        error = pConfigList->Count; 
        pConfigList->Count = 0;
    }
    return error;
}

//-----------------------------------------------------------------------------
s32 net_GetAttachStatus(s32 &InterfaceId)
{
    s32 status;

    netiop_SendSyncRequest(NETCMD_GET_ATTACH_STATUS,NULL,0,&status,sizeof(status));
    InterfaceId = (status >> 24);
    return status & (0x00ffffff);
}

//-----------------------------------------------------------------------------
s32  net_SetConfiguration(const char *pPath,s32 configindex)
{
    char Buffer[128];
    s32 status;
    xtimer t;

    status = CheckCard( pPath );
    if( status != 0 )
    { 
        return status;
    }
    x_memcpy(Buffer,&configindex,4);
    x_strcpy(Buffer+4,pPath);
    netiop_SendSyncRequest(NETCMD_SETCONFIGURATION,Buffer,sizeof(Buffer),&status,sizeof(status));

    t.Reset();
    t.Start();

    while( TRUE )
    {
        if( netiop_SendSyncRequest(NETCMD_GET_SETCONFIG_STATUS,NULL,0,&status,sizeof(status))==FALSE )
        {
            return -1;
        }
        if( status!=1 )
            break;
        x_DelayThread(2);
        if( t.ReadMs() > 8000 )
        {
            ASSERT(FALSE);
            status = -1;
            break;
        }
    }
    return status;
}

//-----------------------------------------------------------------------------
void    net_ActivateConfig(xbool on)
{
    xtimer t;
    s32 status;

    netiop_SendSyncRequest(NETCMD_ACTIVATECONFIG,&on,sizeof(on),NULL,0);
    t.Reset();
    t.Start();
    while(1)
    {
        if( netiop_SendSyncRequest(NETCMD_GET_ACTIVATE_CONFIG_STATUS,NULL,0,&status,sizeof(status))==FALSE )
        {
            return;
        }
        if (status != 1)
            break;
        x_DelayThread(2);
        if( t.ReadMs() > 10000 )
        {
            ASSERT(FALSE);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
s32 net_GetSystemId(void)
{
    s32 sysid;

    netiop_SendSyncRequest(NETCMD_GET_SYSID,NULL,0,&sysid,sizeof(sysid));
    return sysid;
}

//-----------------------------------------------------------------------------
void net_BeginConfig(void)
{
    // Make sure we have the memory card IRX modules loaded
    //g_Net.m_MemcardHandle = g_IopManager.LoadModule("mcman.irx");
}

void net_EndConfig(void)
{
    //g_IopManager.UnloadModule(g_Net.m_MemcardHandle);
}
