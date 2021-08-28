#include <eekernel.h>
#include <sif.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <libcdvd.h>
#include "x_files.hpp"
#include "iop.hpp"
#include "netdefs.hpp"
#include "x_threads.hpp"

// Define this to enable usb ethernet device support
//#ifdef TARGET_PS2_DEV
#define ENABLE_USB_NETWORK
//#endif

#define ENABLE_POWEROFF_PROCESSING

#include <libsn.h>

#define IOP_BUFFER_SIZE 2048             // Buffer size in bytes

#define INEV_IOP_RECV_DEV  0xbeef0001      // Our device #
#define INEV_IOP_SEND_DEV  0xbeef0002      // Our device #


typedef struct s_iop_init
{
    s32 IopBufferSize;
} iop_init;

typedef struct s_iop_vars
{
    s32                 m_IopBufferSize;
    byte*               m_pIopInBuffer;
    byte*               m_pIopOutBuffer;
    sceSifClientData    m_ClientData;
    iop_request*        m_RequestBuffer;
    xmesgq*             m_pqPending;
    xmesgq*             m_pqAvailable;
    xthread*            m_pUpdateThread;
    xthread*            m_pIdleThread;
    xthread*            m_pPoweroffThread;
    s32                 m_CpuUtilization;
    s32                 m_ThreadTicks;
} iop_vars;

iop_vars *g_pIopVars=NULL;
xbool inevloaded=FALSE;
xbool s_PCMCIA_Enabled = FALSE;

#ifdef TARGET_PS2_DEV
//#define INET_CONFIG_FILE "-no_decode" "\0" "host:C:/projects/t2/xcore/entropy/ps2/Modules/devnetcnf/main.cnf"
#define NETCNF_ICON_FILE "icon=host:c:/projects/t2/xcore/entropy/ps2/modules/devnetcnf/sys_net.ico" "\0" "iconsys=host:c:/projects/t2/xcore/entropy/ps2/modules/devnetcnf/icon.sys"
#define INET_CONFIG_FILE "-verbose" "\0" "-no_auto"
#define SPDUART_CONFIG_PARAMS "dial=host:c:/projects/t2/xcore/entropy/ps2/modules/devnetcnf/dial_spd.cnf"
#else
#define NETCNF_ICON_FILE "icon=cdrom:\\MODULES\\CDNETCNF\\SYS_NET.ICO;1" "\0" "iconsys=cdrom:\\MODULES\\CDNETCNF\\ICON.SYS;1"
#define INET_CONFIG_FILE "-no_auto"
#define SPDUART_CONFIG_PARAMS "dial=cdrom:\\MODULES\\CDNETCNF\\DIAL_SPD.CNF;1"
#endif

#define INET_CONFIG_PARAMS "mem=96KB"
void iop_PeriodicUpdate(void);
void iop_IdleThread(void);
//-----------------------------------------------------------------------------
char *KernelError(s32 status)
{
    switch (status)
    {
    case -200:
        return "KE_LINKERR";
        break;
    case -201:
        return "KE_ILLEGAL_OBJECT";
        break;
    case -202:
        return "KE_UNKNOWN_MODULE";
        break;
    case -203:
        return "KE_NOFILE";
        break;
    case -204:
        return "KE_FILEERR";
        break;
    case -205:
        return "KE_MEMINUSE";
        break;
	case -400:
		return "KE_NO_MEMORY";
		break;
    default:
        break;
    }
    return "<unknown>";
}

//-----------------------------------------------------------------------------
s32 iop_LoadModule (char *module_name,char *pArg=NULL,s32 arglength=0,xbool AllowFail=FALSE)
{
    // Get prefix for fileio
    char Buff[64];
    s32     status;
    s32     freebefore=0;

    if (inevloaded)
        freebefore = iop_GetFreeMemory();

#ifdef TARGET_PS2_DEV
	x_sprintf(Buff,"host:%s/entropy/ps2/modules/%s",XCORE_PATH,module_name);
#else
	x_sprintf(Buff,"cdrom0:\\MODULES\\%s;1",module_name);
	x_strtoupper(&Buff[6]);
#endif

    while (1)
    {
		status = sceSifLoadModule(Buff,arglength,pArg);
        if (status >=0)
            break;
	    x_DebugMsg("ERROR %s: Can't load module %s[path=%s]\n",KernelError(status),module_name,Buff);
        if (AllowFail)
        {
            status = -1;
            break;
        }
        asm("break 0x01");

    }
    if (inevloaded)
        x_DebugMsg("%d bytes free after loading module %s, id=%d, %d bytes used\n",iop_GetFreeMemory(),module_name,status,freebefore-iop_GetFreeMemory());
    return status;
}

#ifdef ENABLE_POWEROFF_PROCESSING
xsema   *pPoweroffSemaphore;

static void PoweroffCallback(void*)
{
    // we should use sema_Signal but we can't since it's in the interrupt context
    // so we just directly signal it.
    pPoweroffSemaphore->Release(X_TH_INTERRUPT|X_TH_NOBLOCK);
}

static void PoweroffThread(void)
{
    s32 stat=0;
    xsema PoweroffSemaphore(1,0);

    pPoweroffSemaphore = &PoweroffSemaphore;

    sceCdPOffCallback(PoweroffCallback,NULL);
    // Lock the mutex for now, the interrupt callback routine will release it

    while (1)
    {
        pPoweroffSemaphore->Acquire();
        while (sceDevctl("dev9x:",DDIOC_OFF,NULL,0,NULL,0) < 0)
        {
        }
        while (!sceCdPowerOff(&stat)||stat)
        {
        }
    }
}
#endif

//-----------------------------------------------------------------------------
void iop_Init(s32 IopBufferSize,s32 MaxRequestsPending /*= MAX_PENDING_IOP_REQUESTS*/)
{
    s32 result;
    iop_init init;
	s32 i;

	(void)MaxRequestsPending;
    ASSERT(!g_pIopVars);

    g_pIopVars = (iop_vars *)x_malloc(sizeof(iop_vars));
    ASSERT(g_pIopVars);

    x_memset(g_pIopVars,0,sizeof(iop_vars));

    //g_pIopVars->m_IdleThreadId = eng_CreateThread(iop_IdleThread,"Idle",4096,-THREAD_BASE_PRIORITY+1);

    init.IopBufferSize = IopBufferSize;
    g_pIopVars->m_IopBufferSize = IopBufferSize;
    sceSifInitRpc(0);
    sceSifInitIopHeap();
#if defined(TARGET_PS2_DEV) && defined(bwatson)
    // Allocate 6M from the top
    //sceSifAllocSysMemory(1,6*1048576-100*1024,NULL);
#endif
    // Load pre-requisite modules for the sound library
    iop_LoadModule("libsd.irx");
    // Pad management irx's
    iop_LoadModule("sio2man.irx");
    iop_LoadModule("padman.irx");
    // memory card irx's
    iop_LoadModule("mcman.irx");
    iop_LoadModule("mcserv.irx");

    // Load pre-requisite modules for the inet library
    iop_LoadModule("inet.irx",INET_CONFIG_PARAMS,sizeof(INET_CONFIG_PARAMS));
    iop_LoadModule("netcnf.irx",NETCNF_ICON_FILE,sizeof(NETCNF_ICON_FILE));
    iop_LoadModule("inetctl.irx",INET_CONFIG_FILE,sizeof(INET_CONFIG_FILE));
    // modem 63K.
    // Load our module
    iop_LoadModule("ineviop.irx",(char *)&init,sizeof(iop_init));
    // Now load all the network device modules
    iop_LoadModule("dev9.irx");
    iop_LoadModule("ppp.irx");
    iop_LoadModule("pppoe.irx");

    // There seems to be no way of knowing the error code returned from a module so
    // we have to allow the smap.irx load to fail
    s_PCMCIA_Enabled = (iop_LoadModule("smap.irx",NULL,0,TRUE) != -1);
    s_PCMCIA_Enabled|= (iop_LoadModule("spduart.irx",SPDUART_CONFIG_PARAMS,sizeof(SPDUART_CONFIG_PARAMS),TRUE) != -1);

#ifdef ENABLE_USB_NETWORK
    if (!s_PCMCIA_Enabled)
    {
        iop_LoadModule("usbd.irx");
        iop_LoadModule("an986.irx");
    }
    else
    {
        // Just make sure we try to open the file so that it'll get included
        // in the files.dat file list
        X_FILE *fp;

        fp = x_fopen(xfs("%s/entropy/ps2/modules/usbd.irx",XCORE_PATH),"rb");
        ASSERT(fp);
        x_fclose(fp);

        fp = x_fopen(xfs("%s/entropy/ps2/modules/an986.irx",XCORE_PATH),"rb");
        ASSERT(fp);
        x_fclose(fp);

    }
#endif

    while (1) 
    {
	    result = sceSifBindRpc (&g_pIopVars->m_ClientData, INEV_IOP_DEV, 0);
        ASSERTS(result>=0,"error: sceSifBindRpc failed");

	    if (g_pIopVars->m_ClientData.serve != 0) break;
    }

#ifdef TARGET_PS2_DEVKIT
        snDebugInit();
#endif

    g_pIopVars->m_pIopInBuffer = (byte *)x_malloc(IopBufferSize);
    ASSERT(g_pIopVars->m_pIopInBuffer);

    g_pIopVars->m_pIopOutBuffer = (byte *)x_malloc(IopBufferSize);
    ASSERT(g_pIopVars->m_pIopOutBuffer);

    g_pIopVars->m_RequestBuffer = (iop_request *)x_malloc(MaxRequestsPending * sizeof(iop_request));
    g_pIopVars->m_pqPending     = new xmesgq(MaxRequestsPending);
    g_pIopVars->m_pqAvailable   = new xmesgq(MaxRequestsPending);

    for (i=0;i<MaxRequestsPending;i++)
    {
        g_pIopVars->m_pqAvailable->Send(&g_pIopVars->m_RequestBuffer[i],MQ_BLOCK);
    }

    iop_SendAsyncRequest(IOPCMD_INIT,NULL,0,NULL,0,NULL);
    g_pIopVars->m_pUpdateThread = new xthread(iop_PeriodicUpdate,"Iop Periodic Update",8192,2);
    inevloaded = TRUE;
    x_DebugMsg("%d bytes free after loading modules\n",iop_GetFreeMemory());

#ifdef ENABLE_POWEROFF_PROCESSING
    g_pIopVars->m_pPoweroffThread = new xthread(PoweroffThread,"Power off Handler",1024,2);
#endif

}

//-----------------------------------------------------------------------------
void iop_Kill(void)
{
    ASSERT(g_pIopVars);

    //delete g_pIopVars->m_IdleThread;
    delete g_pIopVars->m_pUpdateThread;
    delete g_pIopVars->m_pPoweroffThread;

    delete g_pIopVars->m_pqAvailable;
    delete g_pIopVars->m_pqPending;

    x_free(g_pIopVars->m_pIopInBuffer);        
    x_free(g_pIopVars->m_pIopOutBuffer);
    x_free(g_pIopVars->m_RequestBuffer);
    x_free(g_pIopVars);
    g_pIopVars = NULL;
}

//-----------------------------------------------------------------------------
void iop_IdleThread(void)
{
    xtimer idletimer;
    s64 idlecount=1;
    s64 counter=0;

    // We disable ints right now so we can get a really accurate measure
    // of the maximum counter value reached when no other activity is going
    // on. This is used later to calculate out the actual processor utilization.
    DI();
    idlecount=0;
    idletimer.Reset();
    idletimer.Start();
    while (idletimer.ReadMs() < 100.0f)
    {
        idlecount++;
    }
    EI();

    while (1)
    {
        idletimer.Reset();
        idletimer.Start();
        while (idletimer.ReadMs() < 100.0f)
        {
            counter++;
        }

        idletimer.Stop();
#ifdef TARGET_PS2_DEV
        x_CheckThreads(FALSE);
#endif
#if 0
        if ( (counter>idlecount) && (idletimer.ReadMs() < 105.0f) )
        {
            idlecount = counter;
        }
#endif
        g_pIopVars->m_CpuUtilization = 100-((counter * 100)/idlecount);
        counter=0;
    }
    ASSERT(FALSE);
}

iop_request *g_ActiveIopRequest;

// IOP update cycle
//-----------------------------------------------------------------------------
void iop_PeriodicUpdate(void)
{
    iop_request *pRequest;
    s32 fromlength,tolength;
    xtimer timer;
    f32 time;

    while (1)
    {
        pRequest = (iop_request *)g_pIopVars->m_pqPending->Recv(MQ_BLOCK);
        ASSERT(pRequest);

        tolength = (pRequest->ToLength + 15) & ~15; 
        fromlength = (pRequest->FromLength + 15) & ~15;
        ASSERT(fromlength);
        ASSERT(tolength);
        //
        if (pRequest->pToData && pRequest->ToLength)
            x_memcpy(g_pIopVars->m_pIopOutBuffer,pRequest->pToData,pRequest->ToLength);

        g_ActiveIopRequest = pRequest;
        timer.Reset();
        timer.Start();
        sceSifCallRpc (&g_pIopVars->m_ClientData,  
                        pRequest->Id,0,
                        g_pIopVars->m_pIopOutBuffer,tolength,
                        g_pIopVars->m_pIopInBuffer,fromlength,
                        NULL,NULL);
        FlushCache(0);
        timer.Stop();
        time = timer.ReadMs();
#if 0
#ifdef bwatson
        if (time > 16.0f)
        {
            x_DebugMsg("*** Request Delayed on IOP too long, id=%08x, time=%2.4f\n",pRequest->Id,time);
        }
#endif
#endif
        g_ActiveIopRequest = NULL;
        if ( pRequest->pFromData && pRequest->FromLength)
            x_memcpy(pRequest->pFromData,g_pIopVars->m_pIopInBuffer,pRequest->FromLength);
        pRequest->pqReply->Send(pRequest,MQ_BLOCK);
    }
    ASSERT(FALSE);
}

//-----------------------------------------------------------------------------
void        iop_SendSyncRequest(s32 requestid,void *pToIop,s32 ToLength,void *pFromIop,s32 FromLength)
{
    xmesgq qReply(1);

    void    *pMessage;

    iop_SendAsyncRequest(requestid,pToIop,ToLength,pFromIop,FromLength,&qReply);
    pMessage = qReply.Recv(MQ_BLOCK);
    ASSERT(pMessage);
    g_pIopVars->m_pqAvailable->Send(pMessage,MQ_BLOCK);
}

//
// If we send just an absolute minimal request, then we must do it synchronously. This will, for all intents,
// be used for init/kill and load requests
//

//-----------------------------------------------------------------------------
void        iop_SendSyncRequest(s32 requestid)
{
    iop_SendSyncRequest(requestid,NULL,0,NULL,0);

}

static s32 tempbuff[4];

//-----------------------------------------------------------------------------
void    iop_SendAsyncRequest(s32 requestid,void *pToIop,s32 ToLength,void *pFromIop,s32 FromLength,xmesgq* pqReply)
{
    iop_request *pRequest;

    pRequest = (iop_request *)g_pIopVars->m_pqAvailable->Recv(MQ_BLOCK);
    ASSERT(pRequest);

    if ( (pToIop==NULL) || (ToLength==0) )
    {
        pToIop = tempbuff;
        ToLength = sizeof(s32);
    }

    if ( (pFromIop==NULL) || (FromLength==0) )
    {
        pFromIop = tempbuff;
        FromLength = sizeof(s32);
    }

    if (pqReply==NULL)
    {
        pqReply = g_pIopVars->m_pqAvailable;
    }

    pRequest->Id = requestid;
    pRequest->pToData = pToIop;
    pRequest->ToLength = ToLength;
    pRequest->pFromData = pFromIop;
    pRequest->FromLength = FromLength;
    pRequest->pqReply = pqReply;

    g_pIopVars->m_pqPending->Send(pRequest,MQ_BLOCK);
}

//-----------------------------------------------------------------------------
void    iop_FreeRequest(iop_request *pRequest)
{
    g_pIopVars->m_pqAvailable->Send(pRequest,MQ_BLOCK);
}

//-----------------------------------------------------------------------------
f32         iop_GetCpuUtilization(void)
{
    return g_pIopVars->m_CpuUtilization;
}

//-----------------------------------------------------------------------------
s32         iop_GetFreeMemory(void)
{
    s32 memfree;

    iop_SendSyncRequest(IOPCMD_FREEMEM,NULL,0,&memfree,sizeof(memfree));
    return memfree;
}

