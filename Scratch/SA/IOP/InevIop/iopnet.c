#include "ioptypes.h"
#include "iopmain.h"
#include "iopnet.h"
#include "iopnet_modem.h"
#include "iopnet_threads.h"
#include "iopnet_port.h"
#include "iopnet_config.h"

#include <stdio.h>

t_IopNetVars g_Net;

//-----------------------------------------------------------------------------
s32 InetAddressToInt(struct sceInetAddress *pAddr)
{
    return ((u8)pAddr->data[0]<<24)|((u8)pAddr->data[1]<<16)|
           ((u8)pAddr->data[2]<<8) |((u8)pAddr->data[3]);
}

//-----------------------------------------------------------------------------
void IntToInetAddress(s32 ipaddr,struct sceInetAddress *pAddr)
{
    memset(pAddr,0,sizeof(struct sceInetAddress));
    pAddr->data[0]=((ipaddr>>24)&0xff);
    pAddr->data[1]=((ipaddr>>16)&0xff);
    pAddr->data[2]=((ipaddr>> 8)&0xff);
    pAddr->data[3]=((ipaddr>> 0)&0xff);
}


//-----------------------------------------------------------------------------
void NetInit(void)
{
    iop_DebugMsg("Initializing net subsystem\n");
    memset(&g_Net,0,sizeof(g_Net));

    g_Net.m_ConnectStatus.Status = CONNECT_STATUS_NOTPRESENT;
    g_Net.m_LastAttachStatus = 0;

    NetInitThreads();

}

//-----------------------------------------------------------------------------
void *net_Dispatch(u32 Command,void *Data,s32 Size)
{
    s32 *pData;
    static s32 SimpleReturn;
    s32 ret;
    struct sceInetAddress InetAddress;
    net_sendrecv_request   *pRequest;

    (void)Size;
    pData = (s32 *)Data;

    switch(Command)
    {
//-------------------------------------------------
    case NETCMD_INIT:
        NetInit();
        break;
//-------------------------------------------------
    case NETCMD_READCONFIG:
        ret = *pData;
        if ( (ret < 0) || (ret >= MAX_NET_DEVICES) )
        {
            ret = 0;
        }
        return &g_Net.m_InterfaceInfo[ret];
        break;

//-------------------------------------------------
    case NETCMD_BIND:
        NetBind(pData[0],pData);
        return pData;
        break;

//-------------------------------------------------
    case NETCMD_UNBIND:
        NetUnbind(pData[0]);
        return NULL;
        break;
        
//-------------------------------------------------
    case NETCMD_KILL:
        NetKillThreads();
        break;

//-------------------------------------------------
    case NETCMD_SEND:
        ASSERT(FALSE);
#if 0
        pIn = (net_sendrecv_request *)pData;
        pRequest = mq_Recv(&g_Net.m_qAvailableRequests,MQ_NOBLOCK);
        if (!pRequest)
        {
            iop_DebugMsg("net: Packet dropped because no request, reads available=%d\n",g_Net.m_qPendingReads.m_ValidEntries);
            SimpleReturn = -1;
            return &SimpleReturn;
        }

        memcpy(&pRequest->m_Header,&pIn->m_Header,sizeof(pIn->m_Header));
        memcpy(pRequest->m_Data,pIn->m_Data,pRequest->m_Header.Length);

        ASSERT((pRequest->m_Header.Port > 0) && (pRequest->m_Header.Port < 65536));
        mq_Send(&g_Net.m_qPendingSends,pRequest,MQ_BLOCK);
#endif
        SimpleReturn=0;
        return &SimpleReturn;
        break;

//-------------------------------------------------
    case NETCMD_RESOLVEIP:
        IntToInetAddress(*pData,&InetAddress);
        pRequest = (net_sendrecv_request*)Data;
        ASSERT(pRequest);
        SimpleReturn = sceInetAddress2Name(0, pRequest->m_Data, MAX_NAME_LENGTH, &InetAddress, 0,0);
        if (SimpleReturn < 0)
        {
            // If it can't resolve the actual name, let's just put in a dotted notation
            // ip address
            sprintf(pRequest->m_Data,"%u.%u.%u.%u",InetAddress.data[3] & 0xff,
                                                     InetAddress.data[2] & 0xff,
                                                     InetAddress.data[1] & 0xff,
                                                     InetAddress.data[0] & 0xff);
        }
        return pRequest->m_Data;
        break;

//-------------------------------------------------
    case NETCMD_RESOLVENAME:
        SimpleReturn = sceInetName2Address(0, &InetAddress, (void *)pData, 500,3);
        if (SimpleReturn < 0)
        {
            SimpleReturn = 0;
        }
        else
        {
            SimpleReturn = InetAddressToInt(&InetAddress);
        }

        return &SimpleReturn;
        break;
//-------------------------------------------------
    case NETCMD_GETCONNECTSTATUS:
        return NetConnectStatus();
        break;

//-------------------------------------------------
    case NETCMD_GETCONFIGLIST:
        NetGetConfig((char *)pData,0,(net_config_list *)pData);
        return pData;
// These functions have been exploded a little. Since they
// take quite some time to start, let's just make sure we
// do them using the "helper" thread
//-------------------------------------------------
    case NETCMD_SETCONFIGURATION:
        ASSERT(g_Net.m_ConfigStatus != 1);
        g_Net.m_ConfigIndex = *pData++;
        strcpy(g_Net.m_ConfigPath,(char*)pData);
        g_Net.m_ConfigStatus = 1;
        mq_Send(&g_Net.m_qConfigHelperRequests,(void*)CR_NET_SET_CONFIG,MQ_NOBLOCK);
        return &g_Net.m_ConfigStatus;
//-------------------------------------------------
    case NETCMD_GET_SETCONFIG_STATUS:
        return &g_Net.m_ConfigStatus;
        break;

//-------------------------------------------------
    case NETCMD_ACTIVATECONFIG:
        // This would be set if busy
        ASSERT(g_Net.m_ActivateStatus != 1);
        g_Net.m_ActivateStatus  = 1;
        g_Net.m_ActivateOn = *pData;
        mq_Send(&g_Net.m_qConfigHelperRequests,(void*)CR_NET_ACTIVATE_CONFIG,MQ_NOBLOCK);
        return &SimpleReturn;
        break;
//-------------------------------------------------
    case NETCMD_GET_ACTIVATE_CONFIG_STATUS:
        return &g_Net.m_ActivateStatus;
//-------------------------------------------------
    case NETCMD_GET_ATTACH_STATUS:
        SimpleReturn = NetGetAttachStatus();
        return &SimpleReturn;
//-------------------------------------------------
	case NETCMD_GET_SYSID:
		return &g_Net.m_SystemId;
//-------------------------------------------------
    default:
        iop_DebugMsg("NetDispatch: Invalid net dispatch code 0x%08x\n",Command);
        break;
    }
    return &SimpleReturn;
}
