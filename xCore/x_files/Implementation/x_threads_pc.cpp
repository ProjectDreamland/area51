#include "..\x_types.hpp"
#if !defined(TARGET_PC)
#error "This is only for the PC target platform. Please check build exclusion rules"
#endif

#include "..\x_threads.hpp"
#include "x_threads_private.hpp"
#include <windows.h>
//
// Define this to enable profile code. You will also need to make sure the timer routines in x_timer.cpp are
// using the non-zero timer versions. See notes there.
//
CRITICAL_SECTION s_Critical;
xbool s_Critical_Initialized=FALSE;
//-----------------------------------------------------------------------------
void sys_thread_Delay(s32 milliseconds)
{
    Sleep(milliseconds);
}

//-----------------------------------------------------------------------------
//--- Returns current active thread id
xthread_private  sys_thread_GetId(void)
{
    xthread_private id;

    id.ThreadId = GetCurrentThreadId();
    id.Handle   = (HANDLE)-1;
    return id;
}

//-----------------------------------------------------------------------------
//--- Sets thread priority to an abosolute priority.
// -THREAD_BASE_PRIORITY (0) is idle, THREAD_BASE_PRIORITY (64) is the default
// and 2*THREAD_BASE_PRIORITY (127) is the highest
// PC thread priorities use 0 as normal, -ve for lower, +ve for higher
void sys_thread_SetPriority(xthread_private& Private, s32 AbsolutePriority)
{
    SetThreadPriority(Private.Handle,AbsolutePriority-THREAD_BASE_PRIORITY);
}

//-----------------------------------------------------------------------------
void sys_thread_Suspend(xthread_private& Private, s32 Flags)
{
    (void)Flags;
    ASSERTS((Flags & X_TH_INTERRUPT)==0,"Cannot sleep in an interrupt context");
    VERIFY(WaitForSingleObject(Private.SuspendSemaphore,INFINITE)==WAIT_OBJECT_0);
}

//-----------------------------------------------------------------------------
void sys_thread_Resume(xthread_private& Private, s32 Flags)
{
    (void)Flags;
//    VERIFY(ReleaseSemaphore(Private.SuspendSemaphore,1,NULL));
    ReleaseSemaphore(Private.SuspendSemaphore,1,NULL);
}

//-----------------------------------------------------------------------------
xthread_private sys_thread_Create(x_thread_boot_fn* pEntry, void* pParam, void* pStack, s32 StackSize, s32 InitialPriority)
{
    (void)InitialPriority ;

    xthread_private Private;
	DWORD tid;

    Private.SuspendSemaphore = CreateSemaphore(NULL,0,128,NULL);
    ASSERT(Private.SuspendSemaphore);

    if (pEntry)
    {
        Private.Handle = CreateThread(
                            NULL                           ,
                            StackSize                      ,
                            (LPTHREAD_START_ROUTINE)pEntry,
                            pParam                         ,
                            CREATE_SUSPENDED               ,
                            &tid);
	    Private.ThreadId = tid;
    }
    else
    {
        Private.Handle   = (HANDLE)-1;
        Private.ThreadId = GetCurrentThreadId();
    }
    ASSERT(Private.ThreadId);
	ASSERT(Private.Handle);
    return Private;
}

//-----------------------------------------------------------------------------
void sys_thread_Start(xthread_private& Private, void* pArg)
{
    (void)pArg;
    ResumeThread(Private.Handle);
}

//-----------------------------------------------------------------------------
void sys_thread_Destroy(xthread_private& Private)
{
    CloseHandle(Private.Handle);
    CloseHandle(Private.SuspendSemaphore);
}

//-----------------------------------------------------------------------------
void sys_thread_Lock(void)
{
    if (!s_Critical_Initialized)
    {
        InitializeCriticalSection(&s_Critical);
        s_Critical_Initialized = TRUE;
    }
    EnterCriticalSection(&s_Critical);
}

//-----------------------------------------------------------------------------
void sys_thread_Unlock(void)
{
    ASSERT(s_Critical_Initialized);
    LeaveCriticalSection(&s_Critical);
}

//-----------------------------------------------------------------------------
void sys_thread_Exit(s32 ReturnCode)
{
    ExitThread(ReturnCode);
}
