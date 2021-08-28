#include <kernel.h>
#include "iopmqueue.h"
#include "iopmain.h"

iop_message_queue *s_WaitingOnQueue[10];

void mq_Create(iop_message_queue *pQueue,s32 nEntries,char *pName)
{
    struct SemaParam SemaInit;

    ASSERT( (nEntries > 0) && (nEntries < 32767) );
    ASSERT(pQueue);

    pQueue->m_Head = 0;
    pQueue->m_Tail = 0;
    pQueue->m_ValidEntries = 0;
    pQueue->m_MaxEntries = nEntries;
    pQueue->m_pName = pName;
    if (pQueue->m_MaxEntries > MAX_DEFAULT_MESSAGES)
    {
        pQueue->m_Queue = (s32 *)iop_Malloc(pQueue->m_MaxEntries * sizeof(s32));
    }
    else
    {
        pQueue->m_Queue = pQueue->m_QueueBuffer;
    }
    ASSERT(pQueue->m_Queue);

    SemaInit.initCount  = nEntries;              // Initially, we can send nEntries messages
    SemaInit.maxCount   = nEntries;
    SemaInit.attr       = AT_THFIFO;
    pQueue->m_SendSema  = CreateSema(&SemaInit);

    SemaInit.initCount  = 0;                     // Initially, we can't receive any entries
    pQueue->m_RecvSema  = CreateSema(&SemaInit);

    SemaInit.initCount  = 1;
    SemaInit.maxCount   = 1;
    pQueue->m_LockSema  = CreateSema(&SemaInit);
}

void mq_Destroy(iop_message_queue *pQueue)
{
    ASSERT(pQueue);
    DeleteSema(pQueue->m_RecvSema);
    DeleteSema(pQueue->m_SendSema);
    if (pQueue->m_MaxEntries > MAX_DEFAULT_MESSAGES)
        iop_Free(pQueue->m_Queue);
}

s32 mq_Send(iop_message_queue *pQueue,void *message,s32 flags)
{
    s32 status;
    ASSERT(pQueue);

    s_WaitingOnQueue[iop_GetThreadId()]=pQueue;

    if ((flags & MQ_BLOCK)==0)
    {
        status = PollSema(pQueue->m_SendSema);
        if (status == KE_SEMA_ZERO)
        {
            s_WaitingOnQueue[iop_GetThreadId()]=NULL;
            return FALSE;
        }
        ASSERT(status==0);
    }
    else
    {
        WaitSema(pQueue->m_SendSema);
    }
    // At this point, we know for certain there is at least
    // one entry available to send a message in to
    WaitSema(pQueue->m_LockSema);
    ASSERT(pQueue->m_ValidEntries != pQueue->m_MaxEntries);
    pQueue->m_ValidEntries++;

    if (flags & MQ_JAM)
    {
        pQueue->m_Head--;
        if (pQueue->m_Head < 0)
            pQueue->m_Head = pQueue->m_MaxEntries-1;
        pQueue->m_Queue[pQueue->m_Head] = (s32)message;
    }
    else
    {
        pQueue->m_Queue[pQueue->m_Tail] = (s32)message;
        pQueue->m_Tail++;
        if (pQueue->m_Tail >= pQueue->m_MaxEntries)
            pQueue->m_Tail = 0;
    }

    SignalSema(pQueue->m_LockSema);
    SignalSema(pQueue->m_RecvSema);        // The message queue now has a valid entry
    s_WaitingOnQueue[iop_GetThreadId()]=NULL;
    return TRUE;
}

void *mq_Recv(iop_message_queue *pQueue,s32 flags)
{
    s32 status;

    ASSERT(pQueue);
    s_WaitingOnQueue[iop_GetThreadId()]=pQueue;
    if (flags & MQ_BLOCK)
    {
        WaitSema(pQueue->m_RecvSema);
    }
    else
    {
        status = PollSema(pQueue->m_RecvSema);
        if (status == KE_SEMA_ZERO)
        {
            s_WaitingOnQueue[iop_GetThreadId()]=NULL;
            return NULL;
        }
        ASSERT(status==0);
    }
    // At this point, we know there is at least one message availble
    // to be received. BUT we cannot guarantee that we can get the first
    // message that was available since it *may* have been grabbed by
    // a higher priority thread. This is irrelevant though since the message
    // passing mechanism does not guarantee the order in which a message
    // will be received if multiple threads are waiting on that message.
    WaitSema(pQueue->m_LockSema);
    ASSERT(pQueue->m_ValidEntries);
    status = pQueue->m_Queue[pQueue->m_Head];
    pQueue->m_Head++;
    if (pQueue->m_Head >= pQueue->m_MaxEntries)
        pQueue->m_Head = 0;
    pQueue->m_ValidEntries--;
    SignalSema(pQueue->m_LockSema);

    SignalSema(pQueue->m_SendSema);     // The message queue has an entry available
    s_WaitingOnQueue[iop_GetThreadId()]=NULL;
    return (void *)status;
}

xbool mq_IsEmpty(iop_message_queue *pQueue)
{
    return (pQueue->m_ValidEntries==0);
}

xbool mq_IsFull(iop_message_queue *pQueue)
{
    return (pQueue->m_ValidEntries == pQueue->m_MaxEntries);
}