#include "iopmain.h"
#include "ioptypes.h"
#include "iopnet.h"
#include "iopnet_port.h"

#include <inet/inet.h>
#include <inet/inetctl.h>
#include <inet/netdev.h>

//-----------------------------------------------------------------------------
s32 NetBind(s32 port,s32 *buffer)
{
    struct sceInetParam param;
    s32 udpid;
    s32 ret;
    port_info *pPort;
    s32 i;

    /* create UDP socket */
    memset(&param, 0, sizeof(struct sceInetParam));
    param.type = sceINETT_DGRAM;
    param.local_port = port;
    param.remote_port = sceINETP_ANY;
    udpid = sceInetCreate(&param);
    if(udpid <= 0)
    {
        iop_DebugMsg("sceInetCreate(port %d) failed.\n",port);
        buffer[0] = -1;
        buffer[1] = 0;
        return -1;
    }

    ret = sceInetOpen(udpid,-1);
    if (ret != sceINETE_OK)
    {
		iop_DebugMsg( "sceInetOpen() failed %d.\n", ret );
        buffer[0] = -1;
        buffer[1] = 0;
		return -1;
    }

    for (i=0;i<g_Net.m_InterfaceCount;i++)
    {
		if (g_Net.m_InterfaceInfo[i].Address)
            break;
    }
	if (i==g_Net.m_InterfaceCount)
	{
		i=0;
	}
    buffer[0] = udpid;
    buffer[1] = g_Net.m_InterfaceInfo[i].Address;

    pPort = iop_Malloc(sizeof(port_info));
    ASSERT(pPort);
    iop_DumpMemory();
    pPort->m_Address = buffer[1];
    pPort->m_Port = port;
    pPort->m_Socket = udpid;
    pPort->m_pNext = g_Net.m_pPortList;
    g_Net.m_pPortList = pPort;
    iop_DebugMsg("Created port %d, %d bytes free, largest free %d.\n",port,iop_MemFree(),iop_LargestFree());
    return udpid;
}

//-----------------------------------------------------------------------------
void NetUnbind(u32 socket)
{
    port_info *pPort,*pPrev;

    pPrev = NULL;

    pPort = g_Net.m_pPortList;

    while (pPort)
    {
        if (pPort->m_Socket == socket)
        {
            break;
        }
        pPrev = pPort;
        pPort = pPort->m_pNext;
		// Try to find an infinite loop
		ASSERT(pPort != g_Net.m_pPortList);
    }

    if (pPort)
    {
        if (pPrev)
        {
            pPrev->m_pNext = pPort->m_pNext;
        }
        else
        {
            g_Net.m_pPortList = pPort->m_pNext;
        }

        sceInetClose(pPort->m_Socket,-1);
        iop_Free(pPort);
    }
    else
    {
        iop_DebugMsg("NetUnbind: Attempt to close a non existent socket %d\n",socket);
    }
}

