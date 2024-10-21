//==============================================================================
//
//  x_mqueue_private.hpp
//
//==============================================================================

#ifndef _X_MQUEUE_HPP
#define _X_MQUEUE_HPP


//==============================================================================
// MESSAGE QUEUE
//==============================================================================
#define MQ_NOBLOCK              X_TH_NOBLOCK	// Operation will not block if needed, just returns false instead
#define MQ_BLOCK                X_TH_BLOCK		// Operation will block if resource is not available
#define MQ_JAM                  X_TH_JAM	    // Callee will have higher priority to acquire resource than others
#define X_MAX_MESSAGE_QUEUES    32
#define MAX_DEFAULT_MESSAGES    16           // Number of messages held internally within the structure. 

// Any more than MAX_DEFAULT_MESSAGES this will require extra memory to be allocated to store
// the messages. Less than this number of entries will always guarantee thread safe operation 
// even if x_memory is not thread safe. This is also done to limit the amount of memory fragmentation
// caused by creating a message queue. The value given to MAX_DEFAULT_MESSAGES, to save space, should
// be no bigger than the size of a memory header block (divided by sizeof(s32)). In the default case of
// 8 entries, a saving will be seen if the memory header is greater than 32 bytes.
//
// CAUTION:
// When you do a Recv(MQ_NOBLOCK), you must be aware it will return NULL if there was no message
// available. You must make sure you do not send any NULL messages to the message queue so you
// can ascertain whether or not the Recv(MQ_NOBLOCK) failed.

class xmesgq
{
public:
                        xmesgq          (s32 Entries);
                       ~xmesgq          (void);
            void*       Recv            (s32 Flags);
            xbool       Send            (void* pMessage, s32 Flags);
            s32         ValidEntries    (void);
            xbool       IsFull          (void);
            xbool       IsEmpty         (void);
            void        Clear           (void);
protected:
            xbool       m_Initialized;
            s16         m_Head;
            s16         m_Tail;
            s16         m_ValidEntries;
            s16         m_MaxEntries;
            s32*        m_pQueue;
			xthreadlist	m_WaitingForRecv;
			xthreadlist m_WaitingForSend;
            s32         m_QueueBuffer[MAX_DEFAULT_MESSAGES];
private:
                        xmesgq      ( void );
protected:
#ifdef DEBUG_THREADS
static      xarray<xmesgq*> m_MasterMessageList;
#endif
};

#endif
