#include "iopnet.h"
#include "iopmain.h"
#include "iopnet_threads.h"
#include "iopnet_config.h"
#include "iopmqueue.h"
#include "iopthreadpri.h"
#include "sifrpc.h"
#include "inet/inetctl.h"
#include "inet/netdev.h"

void NetSendThread(void);
void NetReadThread(void);
void NetConfigHelperThread(void);

void NetEventHandler(s32 id,s32 type);

sceInetCtlEventHandlers_t eventhandler=
{
    NULL,NULL,
    NetEventHandler,
    0,
};

//-----------------------------------------------------------------------------
void NetInitThreads(void)
{
	u8 MacAddress[16];
	s32 Result,Count,i;

	sceInetCtlSetAutoMode(0);
	sceInetCtlDownInterface(0);
    sceInetCtlRegisterEventHandler(&eventhandler);
    mq_Create(&g_Net.m_qConfigHelperRequests,2,"Configuration Requests");
    g_Net.m_SendThread = iop_CreateThread(NetSendThread,NET_SEND_PRIORITY,3072,"NetSendThread");
    g_Net.m_ReadThread = iop_CreateThread(NetReadThread,NET_RECV_PRIORITY,3072,"NetReadThread");
    g_Net.m_HelperThread = iop_CreateThread(NetConfigHelperThread,0,3072,"Configuration helper");
    g_Net.m_InterfaceCount = -1;

    Count = sceInetGetInterfaceList(g_Net.m_DeviceIdTable, MAX_NET_DEVICES);
	g_Net.m_SystemId = 0;

    for (i=0;i<Count;i++)
    {
		memset(MacAddress,0,sizeof(MacAddress));

		Result = sceInetInterfaceControl(g_Net.m_DeviceIdTable[i], sceInetCC_GetHWaddr,&MacAddress,sizeof(MacAddress));
		// We collapse the 6 byte mac address to a 4 byte value. Since the top 3 bytes define
		// vendor id, this should never change since it should always be Sony. However, it might
		// so we should take it in to account.
		g_Net.m_SystemId |= ((MacAddress[2]^MacAddress[0])<<24)|
						  ((MacAddress[3]^MacAddress[1])<<16)|
						  (MacAddress[4]<<8)|
						  MacAddress[5];
		iop_DebugMsg("Interface %d, mac address 0x%08x,result=%d\n",i,g_Net.m_SystemId,Result);
	}
}

void NetKillThreads(void)
{
    iop_DestroyThread(g_Net.m_SendThread);
    iop_DestroyThread(g_Net.m_ReadThread);
    iop_DestroyThread(g_Net.m_HelperThread);
}
s32 QueueLength;
s32 MaxSendQueueLength=0;
s32 Count=0;
s32 txdropped;
s32 rxdropped;

//-----------------------------------------------------------------------------
void* NetSendCallback(u32 command, void *data, s32 size)
{
    struct sceInetAddress   InetAddress;
    s32                     InetPort;
    net_sendrecv_request*   pRequest;
    static s32              SimpleReturn;
    sceInetInfo_t           Info;

    (void)command;
    (void)size;
    pRequest = (net_sendrecv_request*)data;

    ASSERT(command == INEV_NET_SEND_CMD);
    ASSERT(data);
    ASSERT((u32)size <= sizeof(net_sendrecv_request));

    SimpleReturn = 0;
    InetPort = pRequest->m_Header.Port;
    IntToInetAddress(pRequest->m_Header.Address,&InetAddress);
    // As a workaround to the buffer overflow network layer lockup, we monitor the size
    // of the outgoing queue for this port and if it's above a threshold, let's just drop
    // the current packet.
    sceInetControl(pRequest->m_Header.Socket,sceINETC_CODE_GET_INFO,&Info,sizeof(Info));
    QueueLength = Info.send_queue_length;

    if (QueueLength > MaxSendQueueLength)
        MaxSendQueueLength = QueueLength;


    SimpleReturn = sceInetSendTo(pRequest->m_Header.Socket,pRequest->m_Data,
                        pRequest->m_Header.Length,&SimpleReturn,
                        &InetAddress,InetPort,-1);
    if (SimpleReturn != pRequest->m_Header.Length)
    {
        iop_DebugMsg("NetSendCallback: Unexpected result %d from sceInetSendTo\n",SimpleReturn);
    }
    return (void*)&SimpleReturn;
}

sceSifQueueData SendQueueData;
sceSifServeData SendServerData;

void NetSendThread(void)
{
    void *pBuffer;

    // This is the maximum size of data that *should* be sent via the rpc call
    pBuffer = iop_Malloc(sizeof(net_sendrecv_request));
    ASSERT(pBuffer);

    sceSifSetRpcQueue (&SendQueueData, GetThreadId ());
    sceSifRegisterRpc (&SendServerData, INEV_NET_SEND_DEV, NetSendCallback, pBuffer, NULL, NULL, &SendQueueData);
    
    sceSifRpcLoop (&SendQueueData);
}

sceSifClientData        ReadClientData;
//-----------------------------------------------------------------------------
void NetReadThread(void)
{
    port_info               *pPort;
    volatile s32            Status;
    s32                     PollDelay;
    s32                     PollTimeout;
    s32                     PacketAcquired;
    s32                     Length;
    s32                     Result;
    s32                     i;
    s32                     Changed;
    struct sceInetAddress   InetAddress;
    s32                     InetPort;
	s32						LinkActive;
	u32						address;
	s32						flags;

    while (1) 
    {
	    Status = sceSifBindRpc (&ReadClientData, INEV_NET_RECV_DEV, 0);
        ASSERTS(Status>=0,"NetReadThread: sceSifBindRpc failed");

	    if (ReadClientData.serve != 0) break;
        DelayThread(1000);
    }

    PollDelay   = 500;
    PollTimeout = 0;
    while (1)
    {
        //
        // Wait for a short time. This delay will be reduced if there is a packet
        // available. This will mean we should get packets pretty quickly from the
        // network thread if we need to. The delay will be increased to 5ms if
        // we have not received a packet for 10ms at the higher delay speed.
        //
        DelayThread(PollDelay);
        //
        // Send network interface information if it ever changes
        //
        Status = sceInetGetInterfaceList(g_Net.m_DeviceIdTable, MAX_NET_DEVICES);
        Changed = g_Net.m_InterfaceCount != Status;

        g_Net.m_InterfaceCount = Status;

        for (i=0;i<MAX_NET_DEVICES;i++)
        {
            if (i<Status)
            {
	            Result = sceInetInterfaceControl(g_Net.m_DeviceIdTable[i], sceInetCC_GetAddress, &InetAddress, sizeof(struct sceInetAddress));
				ASSERT(Result==0);
				address = (u32)InetAddressToInt(&InetAddress);

				Result = sceInetInterfaceControl(g_Net.m_DeviceIdTable[i], sceInetCC_GetFlags, &flags, sizeof(flags));
				if ( (flags & (sceInetDevF_Up|sceInetDevF_Running))!=(sceInetDevF_Up|sceInetDevF_Running))
				{
					address=0;
				}

				if (address)
				{
					static s32 lastactive;
					LinkActive = sceInetInterfaceControl(g_Net.m_DeviceIdTable[i], sceInetNDCC_GET_LINK_STATUS, NULL,0);
					if (LinkActive != lastactive)
					{
						iop_DebugMsg("[%d] Link status changed from %d to %d\n",g_Net.m_DeviceIdTable[i],lastactive,LinkActive);
					}

					lastactive = LinkActive;
					if (LinkActive < -1)
					{
						// Another, non ethernet interface returned an error. So we assume it was up anyway. Other
						// code chunks should detect the connection loss.
						LinkActive = 1;
					}
				}
				else
				{
					LinkActive = 0;
				}

				if (LinkActive == 0)
				{
					address = 0;
				}

                if (g_Net.m_InterfaceInfo[i].Address != address)
                {
					if (LinkActive != 1)
					{
						if (g_Net.m_InterfaceInfo[i].Address != 0)
						{
							iop_DebugMsg("Interface %d was disconnected from the network\n",i);
							Changed = TRUE;
			                memset(&g_Net.m_InterfaceInfo[i],0,sizeof(g_Net.m_InterfaceInfo[i]));
						}
					}
					else
					{
						iop_DebugMsg("Interface %d was connected to the network\n",i);
						g_Net.m_InterfaceInfo[i].Address = address;
	                    Changed=TRUE;

						Result = sceInetInterfaceControl(g_Net.m_DeviceIdTable[i], sceInetCC_GetNetmask, &InetAddress, sizeof(struct sceInetAddress));
						g_Net.m_InterfaceInfo[i].Netmask   = InetAddressToInt(&InetAddress);

						Result = sceInetGetNameServers(&InetAddress,1);
						g_Net.m_InterfaceInfo[i].Nameserver   = InetAddressToInt(&InetAddress);
						g_Net.m_InterfaceInfo[i].Broadcast = (g_Net.m_InterfaceInfo[i].Address & g_Net.m_InterfaceInfo[i].Netmask) | ~ g_Net.m_InterfaceInfo[i].Netmask;
					}
                }
            }
            else
            {
                memset(&g_Net.m_InterfaceInfo[i],0,sizeof(g_Net.m_InterfaceInfo[i]));
            }
        }

        if (Changed)
        {
            sceSifCallRpc (&ReadClientData,  
                    INEV_NET_NEWIFC_CMD,0,
                    &g_Net.m_InterfaceInfo,sizeof(g_Net.m_InterfaceInfo),
                    &Result,4,
                    NULL,NULL);
        }
 
        //
        // Now, go through all active ports and see if there is a packet available on it.
        //
        do
        {
            PacketAcquired=FALSE;
            pPort = g_Net.m_pPortList;
            while (pPort)
            {
                InetPort = pPort->m_Port;
                g_Net.ReadRequest.m_Header.Length = sceInetRecvFrom(pPort->m_Socket,
                                                            g_Net.ReadRequest.m_Data,
                                                            MAX_PACKET_SIZE,
                                                            NULL,
                                                            &InetAddress,
                                                            &InetPort,0);
                if (g_Net.ReadRequest.m_Header.Length>0)
                {
                    PacketAcquired=TRUE;
                    g_Net.ReadRequest.m_Header.Address  = InetAddressToInt(&InetAddress);
                    g_Net.ReadRequest.m_Header.Port     = InetPort;
                    g_Net.ReadRequest.m_Header.LocalPort= pPort->m_Port;
                    g_Net.ReadRequest.m_Header.Status   = 0;

                    Length = sizeof(net_sendrecv_header)+g_Net.ReadRequest.m_Header.Length;

                    Length = (Length+15)&~15;
                    ASSERT(((u32)(&g_Net.ReadRequest) & 15)==0);

                    FlushDcache();
                    sceSifCallRpc (&ReadClientData,  
                                    INEV_NET_RECV_CMD,0,
                                    &g_Net.ReadRequest,Length,
                                    &Result,4,
                                    NULL,NULL);
                    PollDelay = 1000;
                    PollTimeout = 20;
                }
                else
                {
                    if (g_Net.ReadRequest.m_Header.Length != sceINETE_TIMEOUT)
                    {
                        iop_DebugMsg("NetReadThread: Unexpected error %d returned from sceInetRecvFrom\n",g_Net.ReadRequest.m_Header.Length);
                    }
                }
                pPort = pPort->m_pNext;
				ASSERT(pPort != g_Net.m_pPortList);
            }
        } while (PacketAcquired);

        if (PollTimeout)
        {
            PollTimeout--;
         
        }
        else
        {
            PollDelay = 5000;
        }

    }
}

//-----------------------------------------------------------------------------
void NetEventHandler(s32 id,s32 type)
{
    s32 status;
    s32 i;

    switch (type)
    {
    case sceINETCTL_IEV_Attach:
        status = ATTACH_STATUS_ATTACHED;
        iop_DebugMsg("[%d] Event ATTACH_STATUS_ATTACHED\n",id);
        break;
    case sceINETCTL_IEV_Detach:
        status = ATTACH_STATUS_DETACHED;
        iop_DebugMsg("[%d] Event ATTACH_STATUS_DETACHED\n",id);
        break;
    case sceINETCTL_IEV_Start:
        status = ATTACH_STATUS_STARTED;
        iop_DebugMsg("[%d] Event ATTACH_STATUS_STARTED\n",id);
        break;
    case sceINETCTL_IEV_Stop:
        status = ATTACH_STATUS_STOPPED;
        iop_DebugMsg("[%d] Event ATTACH_STATUS_STOPPED\n",id);
        break;
    case sceINETCTL_IEV_Error:
        status = ATTACH_STATUS_ERROR;
        iop_DebugMsg("[%d] Event ATTACH_STATUS_ERROR\n",id);
        break;
    case sceINETCTL_IEV_Conf:
        status = ATTACH_STATUS_CONFIGURED;
        iop_DebugMsg("[%d] Event ATTACH_STATUS_CONFIGURED\n",id);
        break;
    case sceINETCTL_IEV_NoConf:
        status = ATTACH_STATUS_NOCONFIG;
        iop_DebugMsg("[%d] Event ATTACH_STATUS_NOCONFIG\n",id);
        break;
    default:
        status = ATTACH_STATUS_UNKNOWN;
        iop_DebugMsg("[%d] Event ATTACH_STATUS_UNKNOWN\n",id);

    }

    for (i=0;i<MAX_NET_DEVICES;i++)
    {
        if (g_Net.m_DeviceIdTable[i] == id)
        {
            status |= (i<<24);
            break;
        }
    }
    g_Net.m_NewAttachStatus = status;
}

//-----------------------------------------------------------------------------
void NetConfigHelperThread(void)
{
    s32 RequestId;

    while(1)
    {
        RequestId = (s32)mq_Recv(&g_Net.m_qConfigHelperRequests,MQ_BLOCK);
        switch(RequestId)
        {
        case CR_NET_SET_CONFIG:
            ASSERT(g_Net.m_ConfigStatus == 1);
            g_Net.m_ConfigStatus = NetSetConfig(g_Net.m_ConfigPath,g_Net.m_ConfigIndex);
            ASSERT(g_Net.m_ConfigStatus != 1);
            break;
        case CR_NET_ACTIVATE_CONFIG:
            ASSERT(g_Net.m_ActivateStatus == 1);
            g_Net.m_ActivateStatus = NetActivateConfig(g_Net.m_ActivateOn);
            ASSERT(g_Net.m_ActivateStatus != 1);
            break;
        default:
            ASSERT(FALSE);
			break;
        }
    }
}


