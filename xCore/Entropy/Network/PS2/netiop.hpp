#ifndef __NETIOP_HPP
#define __NETIOP_HPP

#include "x_types.hpp"
#include "x_threads.hpp"

#include <sifrpc.h>

#define MAX_PENDING_IOP_REQUESTS 16

typedef struct s_iop_request
{
    s32             Id;
    s32             Flags;
    void*           pToData;
    s32             ToLength;
    void*           pFromData;
    s32             FromLength;
    xmesgq*         pqReply;
} iop_request;

#define IOPREQ_ASYNC        (0)             // Operation is asynchronous. Default mode.
#define IOPREQ_SYNC         (1<<0)          // Operation is synchronous. i.e. will wait until complete
#define IOPREQ_JAM          (1<<3)          // Push request to head of send queue

// Initialize the iop subsystem. Pass in the maximum request size we will deal with
void        netiop_Init                (s32 IopBufferSize=2048,s32 MaxRequestsPending = MAX_PENDING_IOP_REQUESTS);
// Shutdown the system. Should never be called.
void        netiop_Kill                (void);
// Returns an iop_request structure to be used with the long version of SendRequest
xbool       netiop_SendSyncRequest     (s32 requestid);
//
// pIn, and InLength refer to data coming BACK from the iop
// pOut and OutLength refer to data going TO the iop
xbool       netiop_SendAsyncRequest    (s32 requestid,void *pToIop,s32 ToLength,void *pFromIop,s32 FromLength,xmesgq* pqReply);
xbool       netiop_SendSyncRequest     (s32 requestid,void *pToIop,s32 ToLength,void *pFromIop,s32 FromLength);
void        netiop_FreeRequest         (iop_request *pRequest);
struct iop_net_vars
{
    s32                 m_IopBufferSize;
    byte*               m_pIopInBuffer;
    byte*               m_pIopOutBuffer;
    sceSifClientData    m_ClientData PS2_ALIGNMENT(64);
    iop_request*        m_RequestBuffer;
    xmesgq*             m_pqPending;
    xmesgq*             m_pqAvailable;
    xthread*            m_pUpdateThread;
    xthread*            m_pIdleThread;
    s32                 m_CpuUtilization;
    s32                 m_ThreadTicks;
    s32                 m_InetHandle;
    s32                 m_InetLogHandle;
    s32                 m_NetCnfHandle;
    s32                 m_NetCnfIfHandle;
    s32                 m_NetCnfIfThread;

    s32                 m_InetCtlHandle;
    s32                 m_InevNetHandle;
    s32                 m_PPPHandle;
    s32                 m_PPPoEHandle;
    s32                 m_SmapHandle;
    s32                 m_USBHandle;
};

extern iop_net_vars g_NetIopMgr;
#endif