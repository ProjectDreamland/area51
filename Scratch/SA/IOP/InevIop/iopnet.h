#ifndef __IOPNET_H
#define __IOPNET_H

#include "ioptypes.h"
#include "ps2/iop/netdefs.hpp"
#include "iopmqueue.h"
#include "inet/inet.h"
#include <inet/inetctl.h>

#ifdef  X_DEBUG
#define NET_DEBUG
#endif

#define NET_BUFFER_SIZE     1024
#define NET_MAX_REQUESTS    8
#define ENV_WORKSPACE_SIZE  2048

typedef struct s_interface_info
{
    u32 Address;
    u32 Netmask;
    u32 Broadcast;
    u32 Nameserver;
} interface_info;

typedef struct s_port_info
{
    struct s_port_info *m_pNext;
    u32 m_Address;
    u16 m_Port;
    u32 m_Socket;
} port_info;

typedef struct s_IopNetVars
{
    net_sendrecv_request ReadRequest;
    interface_info      m_InterfaceInfo[MAX_NET_DEVICES];
    s32                 m_DeviceIdTable[MAX_NET_DEVICES];
	s32					m_LastLoadedDeviceId;
    s32                 m_InterfaceCount;
    net_connect_status  m_ConnectStatus;
    s32                 m_SendThread;
    s32                 m_ReadThread;
    s32                 m_LastAttachStatus;
    s32                 m_NewAttachStatus;
    port_info           *m_pPortList;
    sceNetCnfEnv_t      m_LoadedEnvironment;
    byte                m_EnvironmentWorkspace[ENV_WORKSPACE_SIZE];
    s32                 m_ConfigStatus;
    byte                m_ConfigPath[64];
    s32                 m_ConfigIndex;
    s32                 m_ActivateStatus;
    s32                 m_ActivateOn;
    iop_message_queue   m_qConfigHelperRequests;
    s32                 m_HelperThread;
	s32					m_SystemId;
} t_IopNetVars;


void    *net_Dispatch(u32 Command,void *Data,s32 Size);
s32     InetAddressToInt(struct sceInetAddress *pAddr);
void    IntToInetAddress(s32 ipaddr,struct sceInetAddress *pAddr);

extern t_IopNetVars g_Net;

#endif // __IOPNET_H
