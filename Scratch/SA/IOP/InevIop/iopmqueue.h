#ifndef IOPMQUEUE_H
#define IOPMQUEUE_H

#include "ioptypes.h"

#define MQ_NOBLOCK              (0)         // Sending or receiving will not block
#define MQ_BLOCK                (1<<0)      // Sending or receiving will block
#define MQ_JAM                  (1<<1)      // Message must be sent to the start of the queue
#define MAX_DEFAULT_MESSAGES    16           // Number of messages held internally within the structure. 
// Any more than MAX_DEFAULT_MESSAGES this will require extra memory to be allocated to store
// the messages. Less than this number of entries will always guarantee thread safe operation 
// even if x_memory is not thread safe. This is also done to limit the amount of memory fragmentation
// caused by creating a message queue. The value given to MAX_DEFAULT_MESSAGES, to save space, should
// be no bigger than the size of a memory header block (divided by sizeof(s32)). In the default case of
// 8 entries, a saving will be seen if the memory header is greater than 32 bytes.

typedef struct s_message_queue
{
    s16 m_Head;
    s16 m_Tail;
    s16 m_ValidEntries;
    s16 m_MaxEntries;
    char *m_pName;
    s32 *m_Queue;
    s32 m_SendSema;
    s32 m_RecvSema;
    s32 m_LockSema;
    s32 m_QueueBuffer[MAX_DEFAULT_MESSAGES];
} iop_message_queue;

void    mq_Create(iop_message_queue *pQueue,s32 nEntries,char *pName);
void    mq_Destroy(iop_message_queue *pQueue);
xbool   mq_IsEmpty(iop_message_queue *pQueue);
xbool   mq_IsFull(iop_message_queue *pQueue);
s32     mq_Send(iop_message_queue *pQueue,void *message,s32 flags);
void    *mq_Recv(iop_message_queue *pQueue,s32 flags);

#endif
