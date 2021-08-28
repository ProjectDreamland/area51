#ifndef __AUDIOIOP_HPP
#define __AUDIOIOP_HPP

#include "x_types.hpp"
#include "x_threads.hpp"

#include <sifrpc.h>

#define INEV_IOP_DEV  0xbeef0001

#define INEV_AUD_UPDATE 0x00000001
#define MAX_PENDING_IOP_REQUESTS 16

enum {
    IOPCMD_BASE = 0x00000000,
    IOPCMD_INIT,
    IOPCMD_KILL,
    IOPCMD_UPDATE,
    IOPCMD_FREEMEM,
};


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
void        iop_Init                (s32 IopBufferSize=2048,s32 MaxRequestsPending = MAX_PENDING_IOP_REQUESTS);
// Shutdown the system. Should never be called.
void        iop_Kill                (void);
// Returns an iop_request structure to be used with the long version of SendRequest
void        iop_SendSyncRequest     (s32 requestid);
//
// pIn, and InLength refer to data coming BACK from the iop
// pOut and OutLength refer to data going TO the iop
void        iop_SendAsyncRequest    (s32 requestid,void *pToIop,s32 ToLength,void *pFromIop,s32 FromLength,xmesgq* pqReply);
void        iop_SendSyncRequest     (s32 requestid,void *pToIop,s32 ToLength,void *pFromIop,s32 FromLength);
void        iop_FreeRequest         (iop_request *pRequest);
s32         iop_LoadModule          (char *module_name,char *pArg=NULL,s32 arglength=0,xbool AllowFail=FALSE);
f32         iop_GetCpuUtilization   (void);
s32         iop_GetFreeMemory       (void);

#endif
