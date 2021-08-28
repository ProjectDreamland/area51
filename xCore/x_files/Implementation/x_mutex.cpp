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
xmutex::xmutex( void )
       :m_Semaphore( 1, 1 )
{
    m_Initialized = TRUE;
    m_EnterCount  = 0;
    m_pOwner      = NULL;
#ifdef DEBUG_THREADS
    m_MasterMutexList.Append( this );
#endif
}

//==============================================================================
//
//==============================================================================
xmutex::~xmutex( void )
{
    ASSERT(m_Initialized);
#ifdef DEBUG_THREADS
    m_MasterMutexList.Delete( m_MasterMutexList.Find(this) );
#endif
}

//==============================================================================
//
//==============================================================================
xbool xmutex::Enter( s32 Flags )
{
    xbool status;

    ASSERT(m_Initialized);
    if( m_pOwner!=x_GetCurrentThread() )
    {
        status = m_Semaphore.Acquire( Flags );
        if( !status )
            return FALSE;
        m_pOwner = x_GetCurrentThread();
    }
    m_EnterCount++;
    return TRUE;
}

//==============================================================================
//
//==============================================================================
xbool xmutex::Exit( s32 Flags )
{
    ASSERT(m_Initialized);
    ASSERT(m_EnterCount>0);
    m_EnterCount--;
    if( m_EnterCount==0 )
    {
        m_pOwner=NULL;
        m_Semaphore.Release( Flags );
    }
    return TRUE;
}

//==============================================================================
//
//==============================================================================
xsema::xsema( s32 count, s32 initial )
{
    m_Initialized       = TRUE;
    m_Count             = count;
    m_Available         = initial;

#ifdef DEBUG_THREADS
    m_MasterSemaphoreList.Append( this );
#endif
}

//==============================================================================
//
//==============================================================================
xsema::~xsema( void )
{
    ASSERT( m_Initialized );
#ifdef DEBUG_THREADS
    m_MasterSemaphoreList.Delete( m_MasterSemaphoreList.Find(this) );
#endif
}

//==============================================================================
//
//==============================================================================
xbool xsema::Acquire( s32 Flags )
{
    xthread* pThread;

    ASSERT(m_Initialized);
    x_BeginAtomic();
    s_EntersDone++;

    if( Flags & X_TH_BLOCK )
    {
        xthread *pThread;
        pThread = x_GetCurrentThread();

        while( m_Available==0 )
        {
            pThread->Unlink();
            pThread->Link( m_WaitingAcquire );
            pThread->Suspend( Flags, xthread::BLOCKED_ON_SEMAPHORE_ACQUIRE );
            if( (pThread->IsActive()==FALSE) && (m_Available==0) )
            {
                if( pThread->m_pOwningQueue==&m_WaitingAcquire )
                {
                    pThread->Unlink( m_WaitingAcquire );
                }
                else
                {
                    ASSERT( pThread->m_pOwningQueue == NULL );
                }
                pThread->Link();
                x_EndAtomic();
                return FALSE;
            }
            ASSERT( pThread->m_pOwningQueue==NULL );
            pThread->Link();
        }
    }
    else
    {
        if( m_Available==0 )
        {
            x_EndAtomic();
            return FALSE;
        }
    }

    m_Available--;

    pThread = m_WaitingRelease.GetHead();

    if( pThread )
    {
        s_ReleasesDone++;
        pThread->Unlink( m_WaitingRelease );
        x_EndAtomic();
        pThread->Resume( Flags );
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
    xthread* pThread;

    ASSERT(m_Initialized);
    x_BeginAtomic();
    s_ExitsDone++;

    if( Flags & X_TH_BLOCK )
    {
        xthread *pThread;
        pThread = x_GetCurrentThread();

        while( m_Available==m_Count )
        {
            pThread->Unlink();
            pThread->Link( m_WaitingRelease );
            pThread->Suspend( Flags, xthread::BLOCKED_ON_SEMAPHORE_RELEASE );
            if( (pThread->IsActive()==FALSE) && (m_Available==m_Count) )
            {
                if( pThread->m_pOwningQueue==&m_WaitingRelease )
                {
                    pThread->Unlink( m_WaitingRelease );
                }
                else
                {
                    ASSERT( pThread->m_pOwningQueue == NULL );
                }
                pThread->Link();
                x_EndAtomic();
                return FALSE;
            }
            ASSERT( pThread->m_pOwningQueue==NULL );
            pThread->Link();
        }
    }
    else
    {
        if( m_Available==m_Count )
        {
            x_EndAtomic();
            return FALSE;
        }
    }

    m_Available++;

    pThread = m_WaitingAcquire.GetHead();

    if( pThread )
    {
        pThread->Unlink( m_WaitingAcquire );
        s_AcquiresDone++;
        x_EndAtomic();
        pThread->Resume( Flags );
    }
    else
    {
        x_EndAtomic();
    }
    return TRUE;
}

