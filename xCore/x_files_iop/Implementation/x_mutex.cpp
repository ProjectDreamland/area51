#include "..\x_types.hpp"
#include "..\x_debug.hpp"
#include "..\x_threads.hpp"

//==============================================================================
// This file contains our base level atomic synchronisation primitives. All other
// expanded synchronisation methods (such as message queues or potentially semaphores)
// use these functions to lock a mutex. They are guaranteed to be atomic.
//
// Since we can disable interrupts for short periods of time, it makes sense to
// wrap the target platform signalling with our own since, unless we need to block,
// we will eliminate the need for a system call. If we need to block, we don't
// really care about the time it takes anyway. This will help make it very efficient
// to use message queues for general queuing.
s32 s_AcquiresDone=0;
s32 s_ReleasesDone=0;
s32 s_EntersDone=0;
s32 s_ExitsDone=0;

//==============================================================================
//--- Mutexes are implemented as single entry semaphores. The code optimizations
// for using mutexes in a special case are minimal and not worth it.
//==============================================================================
xmutex::xmutex(void)
       :m_Semaphore(1,1)
{
    m_Initialized = TRUE;
#ifdef DEBUG_THREADS
    m_MasterMutexList.Append(this);
#endif
}

//==============================================================================
//
//==============================================================================
xmutex::~xmutex(void)
{
    ASSERT(m_Initialized);
#ifdef DEBUG_THREADS
    m_MasterMutexList.Delete(m_MasterMutexList.Find(this));
#endif
}

//==============================================================================
//
//==============================================================================
xbool xmutex::Enter(s32 Flags)
{
    ASSERT(m_Initialized);
    return m_Semaphore.Acquire(Flags);
}

//==============================================================================
//
//==============================================================================
xbool xmutex::Exit(s32 Flags)
{
	ASSERT(m_Initialized);
	return m_Semaphore.Release(Flags);
}

//==============================================================================
//
//==============================================================================
xsema::xsema(s32 count,s32 initial)
{
    m_Initialized       = TRUE;
    m_Count             = count;
    m_Available         = initial;

#ifdef DEBUG_THREADS
    m_MasterSemaphoreList.Append(this);
#endif
}

//==============================================================================
//
//==============================================================================
xsema::~xsema(void)
{
    ASSERT(m_Initialized);
#ifdef DEBUG_THREADS
    m_MasterSemaphoreList.Delete(m_MasterSemaphoreList.Find(this));
#endif
}

//==============================================================================
//
//==============================================================================
xbool xsema::Acquire(s32 Flags)
{
    xthread* pThread;

    ASSERT(m_Initialized);
    x_BeginAtomic();
    s_EntersDone++;
	if (Flags & X_TH_BLOCK)
	{
        while (m_Available == 0)
        {
			xthread *pThread;
			pThread = x_GetCurrentThread();
			pThread->Unlink();
			pThread->Link(m_WaitingAcquire);
			x_EndAtomic();
            pThread->Suspend(Flags,xthread::BLOCKED_ON_SEMAPHORE_ACQUIRE);
			x_BeginAtomic();
		}

    }
    else
    {
		if (m_Available == 0)
        {
			x_EndAtomic();
			return FALSE;
		}
    }

    m_Available--;
    pThread = m_WaitingRelease.UnlinkFirst();
    if (pThread)
    {
        pThread->Link();
        s_ReleasesDone++;
        x_EndAtomic();
        pThread->Resume(Flags);
    }
    else
    {
        x_EndAtomic();
    }
	return TRUE;

}

//==============================================================================
//
//==============================================================================
xbool xsema::Release(s32 Flags)
{
    xthread *pThread;

    ASSERT(m_Initialized);
    x_BeginAtomic();
    s_ExitsDone++;

	if (Flags & X_TH_BLOCK)
	{
        while (m_Available == m_Count)
        {
			xthread *pThread;
			pThread = x_GetCurrentThread();
			pThread->Unlink();
			pThread->Link(m_WaitingRelease);
			x_EndAtomic();
            pThread->Suspend(Flags,xthread::BLOCKED_ON_SEMAPHORE_RELEASE);
			x_BeginAtomic();
		}
    }
    else
    {
        if (m_Available == m_Count)
		{
			x_EndAtomic();
			return FALSE;
		}
    }

    m_Available++;

    pThread = m_WaitingAcquire.UnlinkFirst();

    if (pThread)
    {
        s_AcquiresDone++;
        pThread->Link();
        x_EndAtomic();
        pThread->Resume(Flags);
    }
    else
    {
        x_EndAtomic();
    }
	return TRUE;
}

