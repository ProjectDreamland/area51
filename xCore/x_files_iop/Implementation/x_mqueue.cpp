#include "..\x_types.hpp"
#include "..\x_memory.hpp"
#include "..\x_debug.hpp"
#include "..\x_threads.hpp"

#ifdef DEBUG_THREADS
static xmesgq*      s_MesgqBuffer[X_MAX_MESSAGE_QUEUES];
xarray<xmesgq*>     xmesgq::m_MasterMessageList(s_MesgqBuffer,X_MAX_MESSAGE_QUEUES);
#endif

//==============================================================================
xmesgq::xmesgq( s32 nEntries )
{
    ASSERT( (nEntries > 0) && (nEntries < 32767) );


    m_Head              = 0;
    m_Tail              = 0;
    m_ValidEntries      = 0;
    m_MaxEntries        = nEntries;

    if (m_MaxEntries > MAX_DEFAULT_MESSAGES)

    {
        m_pQueue = (s32 *)x_malloc(m_MaxEntries * sizeof(s32));
    }
    else
    {
        m_pQueue = m_QueueBuffer;
    }

    m_Initialized = TRUE;
    // The mutexes will automatically construct for
    // SendMutex, RecvMutex and LockMutex
#ifdef DEBUG_THREADS
    m_MasterMessageList.Append(this);
#endif

}

//==============================================================================
xmesgq::~xmesgq(void)
{
    if (m_MaxEntries > MAX_DEFAULT_MESSAGES)
        x_free(m_pQueue);
    m_Initialized = FALSE;
#ifdef DEBUG_THREADS
    m_MasterMessageList.Delete(m_MasterMessageList.Find(this));
#endif
}

//==============================================================================
xbool xmesgq::Send(void *message,s32 flags)
{
	xthread *pThread;

    ASSERT(m_Initialized);
    ASSERT(m_MaxEntries);


	x_BeginAtomic();

    if ((flags & MQ_BLOCK)==0)
    {
		// If in non-blocking mode, we check to see if the message
		// queue is full. If it is, just return otherwise send the
		// message.
		if (IsFull())
		{
			x_EndAtomic();
			return FALSE;
		}
    }
    else
    {
		while (IsFull())
		{
			pThread = x_GetCurrentThread();
			pThread->Unlink();
			pThread->Link(m_WaitingForRecv);
			x_EndAtomic();
            pThread->Suspend(flags,xthread::BLOCKED_ON_MESSAGE_RECV);
			x_BeginAtomic();

		}
    }
    // At this point, we know for certain there is at least
    // one entry available to send a message in to

    ASSERT(!IsFull());
    m_ValidEntries++;

    if (flags & MQ_JAM)
    {
        m_Head--;
        if (m_Head < 0)
            m_Head = m_MaxEntries-1;
        m_pQueue[m_Head] = (s32)message;
    }
    else
    {
        m_pQueue[m_Tail] = (s32)message;
        m_Tail++;
        if (m_Tail >= m_MaxEntries)
            m_Tail = 0;
    }

	// Do we have anything waiting on a send to complete?
    pThread = m_WaitingForSend.UnlinkFirst();
    if (pThread)
	{
        pThread->Link();
        x_EndAtomic();
        pThread->Resume(flags);
	}
	else
	{
		x_EndAtomic();
	}
    return TRUE;
}

//==============================================================================
void *xmesgq::Recv(s32 flags)
{
	xthread *pThread;
    s32 message;

    ASSERT(m_Initialized);

	x_BeginAtomic();

    if ((flags & MQ_BLOCK)==0)
    {
		// If in non-blocking mode, we check to see if the message
		// queue is full. If it is, just return otherwise send the
		// message.
		if (IsEmpty())
		{
			x_EndAtomic();
			return FALSE;
		}
    }
    else
    {
		while (IsEmpty())
		{
			pThread = x_GetCurrentThread();
			pThread->Unlink();
			pThread->Link(m_WaitingForSend);
			x_EndAtomic();
            pThread->Suspend(flags,xthread::BLOCKED_ON_MESSAGE_SEND);
			x_BeginAtomic();

		}
    }

    // At this point, we know there is at least one message available
    // to be received. BUT we cannot guarantee that we can get the first
    // message that was available since it *may* have been grabbed by
    // a higher priority thread. This is irrelevant though since the message
    // passing mechanism does not guarantee the order in which a message
    // will be received if multiple threads are waiting on that message queue.

    ASSERT(IsEmpty()==FALSE);

    m_ValidEntries--;
    message = m_pQueue[m_Head];
    m_Head++;
    if (m_Head >= m_MaxEntries)
        m_Head = 0;

	// Do we have anything waiting on a send to complete?
    pThread = m_WaitingForRecv.UnlinkFirst();
    if (pThread)
	{
        pThread->Link();
        x_EndAtomic();
        pThread->Resume(flags);
	}
	else
	{
		x_EndAtomic();
	}

    return (void *)message;
}

//==============================================================================
xbool xmesgq::IsEmpty(void)
{
    ASSERT(m_Initialized);
    return (m_ValidEntries==0);
}

//==============================================================================
xbool xmesgq::IsFull(void)
{
    ASSERT(m_Initialized);
    return (m_ValidEntries == m_MaxEntries);
}

//==============================================================================
s32 xmesgq::ValidEntries(void)
{
    ASSERT(m_Initialized);
    return m_ValidEntries;
}

//==============================================================================
void mq_DebugDump(void)
{
}