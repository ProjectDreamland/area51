#include <eekernel.h>
#include <sif.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <libsn.h>
#include <libcdvd.h>
#include <netcnfif.h>

#include "x_files.hpp"
#include "netiop.hpp"
#include "netdefs.hpp"
#include "x_threads.hpp"
#include "ps2/iopmanager.hpp"

#ifdef TARGET_DEV
//#define INET_CONFIG_FILE "-no_decode" "\0" "host:C:/projects/t2/xcore/entropy/ps2/Modules/devnetcnf/main.cnf"
#else
#endif

#ifdef TARGET_DEV
//#define INET_CONFIG_FILE "-no_decode" "\0" "host:C:/projects/t2/xcore/entropy/ps2/Modules/devnetcnf/main.cnf"
#define NETCNF_ICON_FILE    "icon=host:"XCORE_PATH"/3rdparty/ps2/Sony/IOP/Modules/devnetcnf/sys_net.ico" "\0" "iconsys=host:"XCORE_PATH"/3rdparty/ps2/Sony/IOP/Modules/devnetcnf/icon.sys"
#define INET_CONFIG_FILE    "-verbose" "\0" "-no_auto"
#elif defined(DEMO_DISK_HARNESS)
#define NETCNF_ICON_FILE    "icon=cdrom:\\AREA51\\MODULES\\CDNETCNF\\SYS_NET.ICO;1" "\0" "iconsys=cdrom:\\AREA51\\MODULES\\CDNETCNF\\ICON.SYS;1"
#define INET_CONFIG_FILE    "-no_auto"
#else
#define NETCNF_ICON_FILE    "icon=cdrom:\\AREA51\\MODULES\\CDNETCNF\\SYS_NET.ICO;1" "\0" "iconsys=cdrom:\\AREA51\\MODULES\\CDNETCNF\\ICON.SYS;1"
#define INET_CONFIG_FILE    "-no_auto"
#endif

#define USB_IRX_PARAMS      "dev=11""\0""ed=24""\0""gtd=48""\0""ioreq=96""\0""hub=1"
#define INET_CONFIG_PARAMS  "mem=64KB""\0""debug=15"
#define INET_LOG_PARAMS     "host1:inet.log"
void netiop_PeriodicUpdate(void);

iop_net_vars g_NetIopMgr ;

//-----------------------------------------------------------------------------
void netiop_Init(s32 IopBufferSize,s32 MaxRequestsPending /*= MAX_PENDING_IOP_REQUESTS*/)
{
    s32 result;
	s32 i;

	(void)MaxRequestsPending;


    //g_IopMgr.m_IdleThreadId = eng_CreateThread(iop_IdleThread,"Idle",4096,-THREAD_BASE_PRIORITY+1);

    //sceSifInitRpc(0);

    // Load pre-requisite modules for the inet library
    g_NetIopMgr.m_InetHandle    = g_IopManager.LoadModule("inet.irx",INET_CONFIG_PARAMS,sizeof(INET_CONFIG_PARAMS));
//    iop_LoadModule("inetlog.irx",INET_LOG_PARAMS,sizeof(INET_LOG_PARAMS));
    g_NetIopMgr.m_NetCnfHandle  = g_IopManager.LoadModule("netcnf.irx",NETCNF_ICON_FILE,sizeof(NETCNF_ICON_FILE));
    g_NetIopMgr.m_NetCnfIfHandle= g_IopManager.LoadModule("netcnfif.irx"); 
    g_NetIopMgr.m_InetCtlHandle = g_IopManager.LoadModule("inetctl.irx",INET_CONFIG_FILE,sizeof(INET_CONFIG_FILE));
    // modem 63K.
    // Load our module
    g_NetIopMgr.m_InevNetHandle = g_IopManager.LoadModule("inevnet.irx");
    // Now load all the network device modules
    g_NetIopMgr.m_PPPHandle     = g_IopManager.LoadModule("ppp.irx");
    g_NetIopMgr.m_PPPoEHandle   = g_IopManager.LoadModule("pppoe.irx");
    g_NetIopMgr.m_USBHandle     = g_IopManager.LoadModule("usbd.irx",USB_IRX_PARAMS,sizeof(USB_IRX_PARAMS));

    // There seems to be no way of knowing the error code returned from a module so
    // we have to allow the smap.irx load to fail
    g_NetIopMgr.m_SmapHandle    = g_IopManager.LoadModule("smap.irx",NULL,0,TRUE);

    while (1) 
    {
	    result = sceSifBindRpc (&g_NetIopMgr.m_ClientData, INEV_NET_DEVICE, 0);
        ASSERTS(result>=0,"error: sceSifBindRpc failed");

	    if (g_NetIopMgr.m_ClientData.serve != 0) break;
    }

    g_NetIopMgr.m_pIopInBuffer = (byte *)x_malloc(IopBufferSize);
    ASSERT(g_NetIopMgr.m_pIopInBuffer);

    g_NetIopMgr.m_pIopOutBuffer = (byte *)x_malloc(IopBufferSize);
    ASSERT(g_NetIopMgr.m_pIopOutBuffer);

    g_NetIopMgr.m_RequestBuffer = (iop_request *)x_malloc(MaxRequestsPending * sizeof(iop_request));
    g_NetIopMgr.m_pqPending     = new xmesgq(MaxRequestsPending);
    g_NetIopMgr.m_pqAvailable   = new xmesgq(MaxRequestsPending);

    g_NetIopMgr.m_NetCnfIfThread= sceNetcnfifCreateMc2Thread( 120 );

    for (i=0;i<MaxRequestsPending;i++)
    {
        g_NetIopMgr.m_pqAvailable->Send(&g_NetIopMgr.m_RequestBuffer[i],MQ_BLOCK);
    }

    g_NetIopMgr.m_pUpdateThread = new xthread(netiop_PeriodicUpdate,"Iop Periodic Update",8192,2);

}

//-----------------------------------------------------------------------------
void netiop_Kill(void)
{
    //delete g_IopMgr.m_IdleThread;
    delete g_NetIopMgr.m_pUpdateThread;

    delete g_NetIopMgr.m_pqAvailable;
    delete g_NetIopMgr.m_pqPending;

    x_free(g_NetIopMgr.m_pIopInBuffer);        
    x_free(g_NetIopMgr.m_pIopOutBuffer);
    x_free(g_NetIopMgr.m_RequestBuffer);


    // ??? Do we need to unbind the sifrpc if it's a client? Maybe not since there is no unbind call
    // and sifremoverpc uses a server structure.

    //
//    sceSifRemoveRpc(&g_NetIopMgr.m_ClientData);
    sceNetcnfifDeleteMc2Thread( g_NetIopMgr.m_NetCnfIfThread );

    if( g_NetIopMgr.m_SmapHandle ) g_IopManager.UnloadModule(g_NetIopMgr.m_SmapHandle);
    g_IopManager.UnloadModule(g_NetIopMgr.m_USBHandle);
    g_IopManager.UnloadModule(g_NetIopMgr.m_PPPoEHandle);
    g_IopManager.UnloadModule(g_NetIopMgr.m_PPPHandle);
    g_IopManager.UnloadModule(g_NetIopMgr.m_InevNetHandle);
    g_IopManager.UnloadModule(g_NetIopMgr.m_InetCtlHandle);
    g_IopManager.UnloadModule(g_NetIopMgr.m_NetCnfIfHandle);
    g_IopManager.UnloadModule(g_NetIopMgr.m_NetCnfHandle);
    g_IopManager.UnloadModule(g_NetIopMgr.m_InetHandle);

    g_NetIopMgr.m_SmapHandle    = 0;
    g_NetIopMgr.m_USBHandle     = 0;
    g_NetIopMgr.m_PPPoEHandle   = 0;
    g_NetIopMgr.m_PPPHandle     = 0;
    g_NetIopMgr.m_InevNetHandle = 0;
    g_NetIopMgr.m_NetCnfIfHandle= 0;
    g_NetIopMgr.m_InetCtlHandle = 0;
    g_NetIopMgr.m_NetCnfHandle  = 0;
    g_NetIopMgr.m_InetHandle    = 0;
}

iop_request *g_ActiveIopRequest;

// IOP update cycle
//-----------------------------------------------------------------------------
void netiop_PeriodicUpdate(void)
{
    iop_request*    pRequest;
    s32             fromlength;
    s32             tolength;
    xtimer          timer;
    f32             time;
    xthread*        pThread;

    pThread =  x_GetCurrentThread();
    ASSERT( pThread );

    while( pThread->IsActive() )
    {
        pRequest = (iop_request *)g_NetIopMgr.m_pqPending->Recv(MQ_BLOCK);
        // This may return NULL but only when the thread is being killed.
        if( pThread->IsActive()==FALSE )
        {
            break;
        }
        ASSERT(pRequest);

        tolength = (pRequest->ToLength + 15) & ~15; 
        fromlength = (pRequest->FromLength + 15) & ~15;
        ASSERT(fromlength);
        ASSERT(tolength);
        //
        if( pRequest->pToData && pRequest->ToLength )
        {
            x_memcpy(g_NetIopMgr.m_pIopOutBuffer,pRequest->pToData,pRequest->ToLength);
        }

        g_ActiveIopRequest = pRequest;
        timer.Reset();
        timer.Start();
        sceSifCallRpc (&g_NetIopMgr.m_ClientData,  
                        pRequest->Id,0,
                        g_NetIopMgr.m_pIopOutBuffer,tolength,
                        g_NetIopMgr.m_pIopInBuffer,fromlength,
                        NULL,NULL);
        FlushCache(0);
        timer.Stop();
        time = timer.ReadMs();

        g_ActiveIopRequest = NULL;
        if( pRequest->pFromData && pRequest->FromLength )
        {
            x_memcpy(pRequest->pFromData,g_NetIopMgr.m_pIopInBuffer,pRequest->FromLength);
        }
        pRequest->pqReply->Send(pRequest,MQ_BLOCK);
    }
}

//-----------------------------------------------------------------------------
xbool netiop_SendSyncRequest(s32 requestid,void *pToIop,s32 ToLength,void *pFromIop,s32 FromLength)
{
    xmesgq qReply(1);

    void    *pMessage;
    xbool   WasSent;

    WasSent = netiop_SendAsyncRequest(requestid,pToIop,ToLength,pFromIop,FromLength,&qReply);
    if( !WasSent )
    {
        LOG_WARNING( "netiop_SendSyncRequest", "Request aborted due to thread shutdown." );
        return FALSE;
    }
    pMessage = qReply.Recv(MQ_BLOCK);
    if( pMessage )
    {
        g_NetIopMgr.m_pqAvailable->Send(pMessage,MQ_BLOCK);
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

//
// If we send just an absolute minimal request, then we must do it synchronously. This will, for all intents,
// be used for init/kill and load requests
//

//-----------------------------------------------------------------------------
xbool netiop_SendSyncRequest(s32 requestid)
{
    return netiop_SendSyncRequest(requestid,NULL,0,NULL,0);
}

static s32 tempbuff[4];

//-----------------------------------------------------------------------------
xbool netiop_SendAsyncRequest(s32 requestid,void *pToIop,s32 ToLength,void *pFromIop,s32 FromLength,xmesgq* pqReply)
{
    iop_request *pRequest;

    pRequest = (iop_request *)g_NetIopMgr.m_pqAvailable->Recv(MQ_BLOCK);
    if( pRequest==NULL )
    {
        LOG_WARNING( "netiop_SendAsyncRequest", "Request aborted due to thread shutdown." );
        return FALSE;
    }
    ASSERT(pRequest);

    if( (pToIop==NULL) || (ToLength==0) )
    {
        pToIop = tempbuff;
        ToLength = sizeof(s32);
    }

    if( (pFromIop==NULL) || (FromLength==0) )
    {
        pFromIop = tempbuff;
        FromLength = sizeof(s32);
    }

    if( pqReply==NULL )
    {
        pqReply = g_NetIopMgr.m_pqAvailable;
    }

    pRequest->Id            = requestid;
    pRequest->pToData       = pToIop;
    pRequest->ToLength      = ToLength;
    pRequest->pFromData     = pFromIop;
    pRequest->FromLength    = FromLength;
    pRequest->pqReply       = pqReply;

    g_NetIopMgr.m_pqPending->Send(pRequest,MQ_BLOCK);
    return TRUE;
}

//-----------------------------------------------------------------------------
void    netiop_FreeRequest(iop_request *pRequest)
{
    g_NetIopMgr.m_pqAvailable->Send(pRequest,MQ_BLOCK);
}
