//==============================================================================
//
//  x_threads_private.hpp
//
// Contains all the prototypes for machine specific functions needed to start up
// a threading system.
//
//==============================================================================

#ifndef _X_THREADS_PRIVATE_HPP
#define _X_THREADS_PRIVATE_HPP

typedef void x_thread_boot_fn(void*);

#if defined(TARGET_PC)
#include <windows.h>
#endif

#if defined TARGET_XBOX
#   ifdef CONFIG_RETAIL
#       define D3DCOMPILE_PUREDEVICE 1
#   endif
#   include <xtl.h>
#endif

#if defined(TARGET_GCN)
#include <os.h>
#endif

struct xthread_private
{
            s32                 ThreadId;

#ifdef TARGET_GCN
            OSThread            TaskControlBlock;
#endif

#if (defined TARGET_PC)||(defined TARGET_XBOX)
            HANDLE              Handle;
            HANDLE              SuspendSemaphore;
            s32                 SemaphoreCount;
#endif
            xbool operator==    (const xthread_private& right) { return ThreadId == right.ThreadId;};
            xbool operator!=    (const xthread_private& right) { return ThreadId != right.ThreadId;};

};

// Thread creation
xthread_private sys_thread_Create           (x_thread_boot_fn* pEntry, void* pParam, void* pStack, s32 StackSize,s32 InitialPriority);
void            sys_thread_Start            (xthread_private& Private, void* pArg);
void            sys_thread_Destroy          (xthread_private& Private);
void            sys_thread_Exit             (s32 ExitCode);

// Thread execution control
void            sys_thread_Delay            (s32 Milliseconds);
void            sys_thread_Suspend          (xthread_private& Private, s32 Flags);
void            sys_thread_Resume           (xthread_private& Private, s32 Flags);

// Thread priority control
xthread_private sys_thread_GetId   (void);
void            sys_thread_SetPriority      (xthread_private& Private, s32 AbsolutePriority);

// System context switch control
void            sys_thread_Lock             (void);
void            sys_thread_Unlock           (void);


#endif
