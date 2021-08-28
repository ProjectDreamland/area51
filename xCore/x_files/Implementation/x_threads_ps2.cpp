#include "..\x_types.hpp"
#if !defined(TARGET_PS2)
#error "This is only for the PS2 target platform. Please check build exclusion rules"
#endif

#include "..\x_threads.hpp"
#include "x_threads_private.hpp"
#include "sntty.h"

//
// Define this to enable profile code. You will also need to make sure the timer routines in x_timer.cpp are
// using the non-zero timer versions. See notes there.
//

//-----------------------------------------------------------------------------
static u64    s_DelayCallback( s32 id, u64 Scheduled, u64 Actual, void *arg, void* pc )
{
    (void)id;
    (void)Scheduled;
    (void)Actual;
    (void)pc;
    ThreadParam Status;

    x_memset(&Status,0,sizeof(Status));
    iReferThreadStatus((s32)arg,&Status);
    if( (Status.status != THS_DORMANT) && (Status.status != 0) )
    {
        iWakeupThread((s32)arg);
    }
    ExitHandler();
    return 0;
}

//-----------------------------------------------------------------------------
#if defined(TARGET_DEV)
static void    s_DelayCallback(s32 id, u16 count,void *arg)
{
    (void)id;
    (void)count;
    ThreadParam Status;

    iReferThreadStatus((s32)arg,&Status);
    if( (Status.status != THS_DORMANT) && (Status.status != 0) )
    {
        iWakeupThread((s32)arg);
    }
    ExitHandler();

}
#endif

//-----------------------------------------------------------------------------
void sys_thread_Delay(s32 milliseconds)
{
    // This is such a hack it makes me feel very dirty! The SetTimerAlarm mis-behaves on a proview build
    // for some reason so we try to use the original method of delaying a thread if we're running under
    // proview.
    s32 Status;

#if defined(TARGET_DEV)
    static s32 s_ProviewPresent = -1;
    if( s_ProviewPresent == -1 )
    {
        s_ProviewPresent = (snputs("")>=0);
    }
    if( s_ProviewPresent )
    {
        Status = SetAlarm(16*milliseconds,s_DelayCallback,(void*)sys_thread_GetId().ThreadId);
    }
    else
#endif
    {
        Status = SetTimerAlarm( TimerUSec2BusClock(milliseconds / 1000, (milliseconds % 1000)*1000 ), s_DelayCallback, (void*)sys_thread_GetId().ThreadId);
    }

    if( Status >= 0 )
    {
        SleepThread();
    }
}

//-----------------------------------------------------------------------------
//--- Returns current active thread id
xthread_private  sys_thread_GetId(void)
{
    xthread_private id;

    id.ThreadId = GetThreadId();
    return id;
}

//-----------------------------------------------------------------------------
//--- Sets thread priority to an abosolute priority.
// -THREAD_BASE_PRIORITY (0) is idle, THREAD_BASE_PRIORITY (64) is the default
// and 2*THREAD_BASE_PRIORITY (127) is the highest
// PS2 thread priorities use (1) as the highest, (127) as the lowest
void sys_thread_SetPriority(xthread_private& Private, s32 AbsolutePriority)
{
    ChangeThreadPriority(Private.ThreadId,127-AbsolutePriority);

}

//-----------------------------------------------------------------------------
void sys_thread_Suspend(xthread_private& Private, s32 Flags)
{
    (void)Private;
    (void)Flags;
    ASSERTS((Flags & X_TH_INTERRUPT)==0,"Cannot sleep in an interrupt context");
    SleepThread();
}

//-----------------------------------------------------------------------------
void sys_thread_Resume(xthread_private& Private, s32 Flags)
{
    if (Flags & X_TH_INTERRUPT)
    {
        ThreadParam Status;
        x_memset(&Status,0,sizeof(Status));
        iReferThreadStatus(Private.ThreadId,&Status);
        if( (Status.status != THS_DORMANT) && (Status.status != 0) )
        {
            iWakeupThread(Private.ThreadId);
        }
    }
    else
    {
        ThreadParam Status;
        x_memset(&Status,0,sizeof(Status));
        ReferThreadStatus(Private.ThreadId,&Status);
        if( (Status.status != THS_DORMANT) && (Status.status != 0) )
        {
            WakeupThread(Private.ThreadId);
        }
    }
}

//-----------------------------------------------------------------------------
xthread_private sys_thread_Create(x_thread_boot_fn* pEntry, void* pParam, void* pStack, s32 StackSize,s32 InitialPriority)
{
    struct ThreadParam Param;
    xthread_private Private;

    (void)pParam;

    if (pEntry)
    {
        Param.entry             = pEntry;
        Param.stack             = (u8*)pStack;
        Param.stackSize         = StackSize;
        Param.initPriority      = 127-InitialPriority;
        Param.gpReg             = &_gp;

        // We need the 'Param' field of m_Startup to be at the beginning of m_Startup so
        // we can caculate the offset to the beginning of the xthread structure properly
        Private.ThreadId        = CreateThread(&Param);
    }
    else
    {
        Private.ThreadId = GetThreadId();
    }
    ASSERT(Private.ThreadId>=0);
    return Private;
}

//-----------------------------------------------------------------------------
void sys_thread_Start(xthread_private& Private, void* pArg)
{
    StartThread(Private.ThreadId, pArg);
}

//-----------------------------------------------------------------------------
void sys_thread_Destroy(xthread_private& Private)
{
    TerminateThread(Private.ThreadId);
    DeleteThread(Private.ThreadId);
}

volatile s32 s_InterruptCount=0;
#if defined(X_DEBUG)
s32 s_InterruptOwner = -1;
#endif
//-----------------------------------------------------------------------------
void sys_thread_Lock(void)
{
    xbool WasEnabled;

    WasEnabled = DI();
    if( WasEnabled )
    {
#if defined(X_DEBUG)
        if( s_InterruptOwner != -1 )
        {
            BREAK;
        }
        s_InterruptOwner = GetThreadId();
        if( s_InterruptCount != 0 )
        {
            BREAK;
        }
#endif
    }
#if defined(X_DEBUG)
    if( s_InterruptOwner != GetThreadId() )
    {
        BREAK;
    }
#endif
    s_InterruptCount++;
}

//-----------------------------------------------------------------------------
void sys_thread_Unlock(void)
{
#if defined(X_DEBUG)
    if( s_InterruptOwner != GetThreadId() )
    {
        BREAK;
    }
#endif
    s_InterruptCount--;
    if (s_InterruptCount==0)
    {
#if defined(X_DEBUG)
        s_InterruptOwner = -1;
#endif
        EI();
    }
}

//-----------------------------------------------------------------------------
void sys_thread_Exit(s32 ReturnCode)
{
    (void)ReturnCode;
    ExitThread();
}
