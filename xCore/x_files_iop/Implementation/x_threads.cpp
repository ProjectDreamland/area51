#include "..\x_files.hpp"
#include "..\x_threads.hpp"

//
// Define this to enable profile code. You will also need to make sure the timer routines in x_timer.cpp are
// using the non-zero timer versions. See notes there.
//
#if defined(TARGET_PS2_DEV) && defined(bwatson)
//#define ENABLE_PROFILE
#define USE_SN_PROFILER
#endif


#ifdef TARGET_PC
#include <windows.h>
#endif

#ifdef TARGET_PS2
	#ifdef TARGET_PS2_IOP
		extern "C"
		{
			#include <intrman.h>
			#include <thread.h>
		}
	#else
		#include <eekernel.h>
	#endif
#endif

#ifdef ENABLE_PROFILE
#include <sifdev.h>
#include <libsn.h>
s32 iop_LoadModule (char *module_name,char *pArg=NULL,s32 arglength=0,xbool AllowFail=FALSE);
#define PROFILE_BUFFER_LENGTH	(16*1024)			// 16K profile buffer
#define PROFILE_FILENAME		"profile.dat"
#define PROFILE_INTERVAL		2*16				// 16 ticks per ms
#define PROFILE_PC_ADDRESS		0x80078230
struct profile_sample
{
	u32	ProgramCounter;
};


#ifndef USE_SN_PROFILER
static void s_ProfileThread(void);
#endif
#endif

#define MAX_SYSTEM_THREAD_ID 128

#ifdef DEBUG_THREADS
static void s_WatchdogThread(void);
// Need to declare these here so we can be sure of the construction order
// on the PC.
static xmutex*      s_MutexBuffer[X_MAX_MUTEXES];
xarray<xmutex*>     xmutex::m_MasterMutexList(s_MutexBuffer,X_MAX_MUTEXES);

static xsema*       s_SemaphoreBuffer[X_MAX_SEMAPHORES];
xarray<xsema*>      xsema::m_MasterSemaphoreList(s_SemaphoreBuffer,X_MAX_SEMAPHORES);

static xthread*     s_ThreadBuffer[X_MAX_THREADS];
xarray<xthread*>    xthread::m_MasterThreadList( s_ThreadBuffer,X_MAX_THREADS );
#endif


struct thread_vars
{
    xthreadlist			m_RunList;
    s32					m_ThreadTicks;
    xmutex				m_Lock;
    s32					m_TopThreadId;
    s32					m_AllDeadSema;
    xthread*			m_pAppMain;
#if !defined(TARGET_PS2_IOP)
	xthread*			m_SysThreadToStruct[MAX_SYSTEM_THREAD_ID];
#endif
#ifdef DEBUG_THREADS
	xthread*			m_pWatchdogThread;
#endif
#ifdef ENABLE_PROFILE
	xthread*			m_pProfileThread;
	profile_sample*		m_pProfileBuffer;
	s32					m_ProfileLength;
	s32					m_ProfileFileHandle;
	xbool				m_ProfileEnabled;
	s32					m_ProfileInterval;
#endif
};

static thread_vars* s_pThreadVars;
static xbool       s_Initialized = FALSE;
        
//-----------------------------------------------------------------------------
// Global startup function
void x_InitThreads(void)
{
    ASSERT(!s_Initialized);
    ASSERT(!s_pThreadVars);


    s_pThreadVars = new thread_vars;
    ASSERT(s_pThreadVars);
 
    s_pThreadVars->m_ThreadTicks  = 1000;
    s_pThreadVars->m_TopThreadId  = 1;

    s_Initialized = TRUE;
    s_pThreadVars->m_pAppMain = new xthread(2048);

#ifdef ENABLE_PROFILE
	s_pThreadVars->m_ProfileLength  = PROFILE_BUFFER_LENGTH;
	s_pThreadVars->m_pProfileBuffer = (profile_sample*)x_malloc(PROFILE_BUFFER_LENGTH);
	ASSERT(s_pThreadVars->m_pProfileBuffer);
#ifdef USE_SN_PROFILER
	iop_LoadModule ("snprofil.irx");
	snProfInit(_4KHZ,s_pThreadVars->m_pProfileBuffer,s_pThreadVars->m_ProfileLength);
	xprof_Disable();
#else
	s_pThreadVars->m_ProfileFileHandle = sceOpen("host0:"PROFILE_FILENAME,SCE_WRONLY|SCE_CREAT|SCE_TRUNC);
	ASSERT(s_pThreadVars->m_ProfileFileHandle >=0);
	s_pThreadVars->m_pProfileThread = new xthread(s_ProfileThread,"Profiler Thread",8192,1);
	ASSERT(s_pThreadVars->m_pProfileThread);
#endif
#endif
#ifdef DEBUG_THREADS
	s_pThreadVars->m_pWatchdogThread = new xthread(s_WatchdogThread,"Watchdog timer",8192,10);
#endif

}

//-----------------------------------------------------------------------------
// Global shutdown function
void x_KillThreads(void)
{
    xthread* pThread;
    xthread* pCurrent;

    ASSERT(s_Initialized);

    pCurrent = x_GetCurrentThread();
    while (1)
    {
        pThread = s_pThreadVars->m_RunList.UnlinkFirst();
        if (!pThread)
            break;
        pThread->Link();
        delete pThread;
    }

    delete s_pThreadVars;
    s_pThreadVars = NULL;
    s_Initialized = FALSE;
}
xthread* pMainThread;

//-----------------------------------------------------------------------------
// NOTE: This is only used for the initial thread structure that is created for
// the main game application thread.
xthread::xthread(s32 StackSize)
{
	pMainThread = this;
    // Stack has to be allocated before we lock the thread handler system
    m_pStack        = x_malloc(StackSize);
    ASSERT(m_pStack);

    s_pThreadVars->m_Lock.Enter();
    m_ThreadId = s_pThreadVars->m_TopThreadId;

    ASSERTS(m_ThreadId==1,"Constructor is for the main thread only.");
    s_pThreadVars->m_TopThreadId++;

    m_Globals.NextOffset= 0;
    m_Globals.StringBuffer= (char*)m_pStack;
    m_Globals.BufferSize= (StackSize > 1024)?(StackSize-1024):(StackSize/2);
    m_pName             = "AppMain";
    m_Priority          = 0;
    m_BasePriority      = 0;
    m_pNext             = NULL;
    m_pPrev             = NULL;
    //
    // Insert this thread at the end of the list of threads. This will ensure the
    // thread list is in order of creation
    //
    s_pThreadVars->m_ThreadTicks = 10000;
    m_Initialized = TRUE;

    x_memset(m_pStack,m_ThreadId,StackSize);

#ifdef TARGET_PS2
    m_SysThreadId = GetThreadId();
	//
	// Used for a quick translate from system id to internal id
	//
#if !defined(TARGET_PS2_IOP)
	s_pThreadVars->m_SysThreadToStruct[m_SysThreadId % MAX_SYSTEM_THREAD_ID] = this;
#endif
    ASSERT(m_SysThreadId>=0);
    ChangeThreadPriority(m_SysThreadId,THREAD_BASE_PRIORITY);
    Link();

#elif defined(TARGET_PC)
    // NOTE: We need to make sure the thread does not start until the Link it to the run
    // list has been completed. Set it's initial priority to VERY low
    m_SysThreadId = (s32)GetCurrentThreadId();
	m_SysHandle = (s32)GetCurrentThread();
    ASSERT(m_SysThreadId);
	ASSERT(m_SysHandle);
    Link();
    SetThreadPriority((void *)m_SysHandle,THREAD_BASE_PRIORITY);
#else
#error "Need function to create a dummy thread for this platform"
#endif
    //
    // Last thing we do is add it to the runlist
    //
#ifdef DEBUG_THREADS
    m_MasterThreadList.Append(this);
#endif
    s_pThreadVars->m_Lock.Exit();
}

//-----------------------------------------------------------------------------
// Thread constructor (adds it to the master list and determines correct id #
xthread::xthread    (entry_fn *pEntry,const char *pName,s32 StackSize,s32 Priority)
{
    // Stack has to be allocated before we lock the thread handler system
    m_pStack        = x_malloc(StackSize);
    ASSERT(m_pStack);

    s_pThreadVars->m_Lock.Enter();
    m_ThreadId = s_pThreadVars->m_TopThreadId;

    s_pThreadVars->m_TopThreadId++;
    m_Globals.BufferSize= (StackSize > 1024)?(StackSize-1024):(StackSize/2);
    m_Globals.StringBuffer=(char*)m_pStack;
    m_Globals.NextOffset= 0;
    m_pName             = pName;
    m_Priority          = Priority;
    m_BasePriority      = Priority;
    m_pNext             = NULL;
    m_pPrev             = NULL;
    //
    // Insert this thread at the end of the list of threads. This will ensure the
    // thread list is in order of creation
    //
    s_pThreadVars->m_ThreadTicks = 10000;
    m_Initialized = TRUE;

    x_memset(m_pStack,m_ThreadId,StackSize);

#ifdef TARGET_PS2
    struct ThreadParam tp;

    tp.entry        = pEntry;
    tp.stackSize    = StackSize;
    tp.initPriority = THREAD_BASE_PRIORITY-Priority;

	#if defined(TARGET_PS2_IOP)
		tp.attr			= TH_C;
		tp.option		= 0;
	#else
		tp.stack        = (u8 *)m_pStack;
		tp.gpReg = &_gp;
	#endif


    m_SysThreadId = CreateThread(&tp);
#if !defined(TARGET_PS2_IOP)
	ASSERT(s_pThreadVars->m_SysThreadToStruct[m_SysThreadId % MAX_SYSTEM_THREAD_ID]==0);
	s_pThreadVars->m_SysThreadToStruct[m_SysThreadId % MAX_SYSTEM_THREAD_ID] = this;
#endif
    Link();
    ASSERT(m_SysThreadId>=0);
    StartThread(m_SysThreadId, NULL);
#elif defined(TARGET_PC)
	DWORD tid;
    // NOTE: We need to make sure the thread does not start until the Link it to the run
    // list has been completed. Set it's initial priority to VERY low.
    // Note that PC threads return a handle but also have an associated thread id which will
    // be used when trying to "find" the current running thread.
    m_SysHandle = (s32)CreateThread(NULL,StackSize,(LPTHREAD_START_ROUTINE)pEntry,NULL,0,&tid);
	m_SysThreadId = tid;
    ASSERT(m_SysThreadId);
	ASSERT(m_SysHandle);
    Link();
    SetThreadPriority((void *)m_SysHandle,Priority);
#else
#error "Need function to create a thread for this target platform"
#endif
    //
    // Last thing we do is add it to the runlist
    //
#ifdef DEBUG_THREADS
    m_MasterThreadList.Append(this);
#endif
    s_pThreadVars->m_Lock.Exit();
}

//-----------------------------------------------------------------------------
// Thread destructor
xthread::~xthread(void)
{
    ASSERT(m_Initialized);
    m_Initialized = FALSE;
    s_pThreadVars->m_Lock.Enter();

    if (m_Status == RUNNING)
    {
        Unlink();
    }

    // Did we ever resize the text format buffer?
    if (m_Globals.StringBuffer != m_pStack)
    {
        x_free(m_Globals.StringBuffer);
    }

#if defined(TARGET_PS2)
#if !defined(TARGET_PS2_IOP)
	ASSERT(s_pThreadVars->m_SysThreadToStruct[m_SysThreadId % MAX_SYSTEM_THREAD_ID] == this);
	s_pThreadVars->m_SysThreadToStruct[m_SysThreadId % MAX_SYSTEM_THREAD_ID] = NULL;
#endif
    TerminateThread(m_SysThreadId);
    DeleteThread(m_SysThreadId);

#elif defined(TARGET_PC) && !defined(TARGET_PC_MFC)
    CloseHandle((void*)m_SysHandle);
#elif defined(TARGET_PC_MFC)
#elif
#error "Need function to destroy a thread for this target platform"
#endif
    x_free(m_pStack);
#ifdef DEBUG_THREADS
    m_MasterThreadList.Delete(m_MasterThreadList.Find(this));
#endif
    s_pThreadVars->m_Lock.Exit();

}

//-----------------------------------------------------------------------------
void xthread::Suspend(s32 Flags, state Status)
{
    m_Status = Status;

#if defined(TARGET_PS2)
	if (Flags & X_TH_INTERRUPT)
	{
        ASSERTS(FALSE,"Cannot sleep a thread in interrupt context");
#ifdef TARGET_PS2_IOP
		ASSERT(FALSE);
#else
		iSuspendThread(m_SysThreadId);
#endif
	}
	else
	{
		SleepThread();
	}
#elif defined(TARGET_PC)
    (void)Flags;
    SuspendThread((void*)m_SysHandle);
#else
#error "Need a Suspend call for this target platform"
#endif
}

//-----------------------------------------------------------------------------
void xthread::Resume(s32 Flags)
{
    m_Status = RUNNING;
#if defined(TARGET_PS2)
	if (Flags & X_TH_INTERRUPT)
	{
		iWakeupThread(m_SysThreadId);
	}
	else
	{
		WakeupThread(m_SysThreadId);
	}
#elif defined(TARGET_PC)
    ResumeThread((void*)m_SysHandle);
#else
#error "Need a Resume call for this target platform"
#endif
}

//-----------------------------------------------------------------------------
// Link this thread to the runlist
//-----------------------------------------------------------------------------
void    xthread::Link(void)
{
    m_Status = RUNNING;
    s_pThreadVars->m_RunList.Link(this);
}

//-----------------------------------------------------------------------------
// Unlink this thread from the runlist
//-----------------------------------------------------------------------------
void    xthread::Unlink(void)
{
    s_pThreadVars->m_RunList.Unlink(this);
    m_Status = LIMBO;
}

//-----------------------------------------------------------------------------
// Link this thread to a specific thread list
//-----------------------------------------------------------------------------
void    xthread::Link(xthreadlist &List)
{
    List.Link(this);
}

//-----------------------------------------------------------------------------
// Unlink this thread from a specific thread list
//-----------------------------------------------------------------------------
void    xthread::Unlink(xthreadlist &List)
{
    List.Unlink(this);
}

//-----------------------------------------------------------------------------
void    xthread::SetFormatBufferSize(s32 Size)
{
    (void)Size;
    if (m_Globals.StringBuffer != m_pStack)
    {
        x_free(m_Globals.StringBuffer);
    }

    m_Globals.StringBuffer = (char*)x_malloc(Size);
    m_Globals.BufferSize = Size;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
xthreadlist::xthreadlist(void)
{
    m_pHead = NULL;
    m_pTail = NULL;
}

//-----------------------------------------------------------------------------
xthreadlist::~xthreadlist(void)
{
}

//-----------------------------------------------------------------------------
// Search for a thread within this threadlist. The system supplied thread id is
// used as the search criteria. This is often used in a debug build to check that
// the thread you're attempting to remove from a list is in the list and one
// that you're attempting to insert has not already been inserted.
//-----------------------------------------------------------------------------
xthread* xthreadlist::Find(s32 id)
{
    xthread* pThread;

    pThread = m_pHead;

    while (pThread)
    {
        if (pThread->GetSystemId() == id)
            break;
        pThread = pThread->m_pNext;
    }
    return pThread;
}
//-----------------------------------------------------------------------------
// Link a thread in to this list at the end
//-----------------------------------------------------------------------------
void    xthreadlist::Link(xthread *pThread)
{
    // Make sure this thread isn't already in the list
    ASSERT(Find(pThread->GetSystemId())==NULL);
    ASSERT(pThread->m_pPrev==NULL);
    ASSERT(pThread->m_pNext==NULL);

    if (m_pHead==NULL)          
    {
        // The list is empty
        ASSERT(m_pTail==NULL);

        m_pHead          = pThread;
        m_pTail          = pThread;
        pThread->m_pPrev = NULL;
        pThread->m_pNext = NULL;
    }
    else
    {
        // List is not empty, add to the end
        m_pTail->m_pNext = pThread;
        pThread->m_pNext = NULL;
        pThread->m_pPrev = m_pTail;
        m_pTail          = pThread;
    }
}

//-----------------------------------------------------------------------------
// Unlink a thread from this list
//-----------------------------------------------------------------------------
void    xthreadlist::Unlink(xthread *pThread)
{
    ASSERT(m_pHead);
    ASSERT(m_pTail);

    // Make sure this thread is in the specified list
    ASSERT(Find(pThread->GetSystemId()));

    if (pThread->m_pPrev)
    {
        pThread->m_pPrev->m_pNext = pThread->m_pNext;
    }
    else
    {
        m_pHead = pThread->m_pNext;
    }

    if (pThread->m_pNext)
    {
        pThread->m_pNext->m_pPrev = pThread->m_pPrev;
    }
    else
    {
        m_pTail = pThread->m_pPrev;
    }

    pThread->m_pNext = NULL;
    pThread->m_pPrev = NULL;
}

//-----------------------------------------------------------------------------
// Unlink the first thread within this list
//-----------------------------------------------------------------------------
xthread* xthreadlist::UnlinkFirst(void)
{
    xthread* pThread;

    pThread = m_pHead;
    if (pThread)
        Unlink(m_pHead);

    return pThread;
}


#ifdef TARGET_PS2
//-----------------------------------------------------------------------------
void    DelayCallback(s32 id, u16 count,void *arg)
{
    (void)id;
    (void)count;
    iWakeupThread((s32)arg);
#ifndef TARGET_PS2_IOP
    ExitHandler();
#endif
}
#endif

//-----------------------------------------------------------------------------
void    x_DelayThread(s32 milliseconds)
{
    xtimer t;

    t.Reset();
    t.Start();

    ASSERT(s_Initialized);
    if (milliseconds <=0 )
        return;
#ifdef TARGET_PS2
#ifdef TARGET_PS2_IOP
	DelayThread(milliseconds*1000);
#else
    SetAlarm(16*milliseconds,DelayCallback,(void *)GetThreadId());
    SleepThread();
#endif
#elif defined(TARGET_PC)
    Sleep(milliseconds);
#else
#error "Need a function to delay a thread for this target platform"
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//------------- DEBUG FUNCTIONS
//-----------------------------------------------------------------------------
void x_CheckThreads(xbool ForceDump)
{
    (void)ForceDump;

    if (ForceDump)
    {
        x_DumpThreads();

    }
}

//-----------------------------------------------------------------------------
void xthread::DumpState(void)
{
}

//-----------------------------------------------------------------------------
// Dump the status of the entire threading system
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Global access function to dump entire thread state
//-----------------------------------------------------------------------------
void x_DumpThreads(void)
{
}

//-----------------------------------------------------------------------------
xthread*    x_GetCurrentThread(void)
{
    static s32      s_LastThreadId=-1;
    static xthread* pThread;
           s32      Current;
           xthread* pReturn;

#if defined(TARGET_PS2_IOP)
	x_BeginAtomic();
	Current = GetThreadId();
#elif defined(TARGET_PS2)
    Current = GetThreadId() % MAX_SYSTEM_THREAD_ID;
	return s_pThreadVars->m_SysThreadToStruct[Current];
#elif defined(TARGET_PC)
    x_BeginAtomic();
    Current = GetCurrentThreadId();
#else
#error "Need to have a GetThreadId for this target platform"
#endif

    if (s_LastThreadId != Current)
    {
        s_LastThreadId = Current;
        pThread = s_pThreadVars->m_RunList.Find(s_LastThreadId);
    }
    ASSERT(pThread);
    pReturn = pThread;
    x_EndAtomic();
    return pReturn;
}

//-----------------------------------------------------------------------------
void    x_WatchdogReset(void)
{
    ASSERT(s_Initialized);
    s_pThreadVars->m_ThreadTicks = 40;         // 2 second delay until the threadticks time out
}

#ifdef DEBUG_THREADS

void xthread::DumpThreads(void)
{
	xthread* pThread;
	s32		 count,i;
	struct ThreadParam threadparam;

	count = xthread::m_MasterThreadList.GetCount();
	for (i=0;i<count;i++)
	{
		pThread = xthread::m_MasterThreadList[i];
		ReferThreadStatus(pThread->GetSystemId(),&threadparam);
		x_DebugMsg("Thread %d (%s), pc=0x%08x\n",i,pThread->m_pName,threadparam.entry);
	}
}


static void	s_WatchdogThread(void)
{
	while(1)
	{
		x_DelayThread(100);

		s_pThreadVars->m_ThreadTicks--;
		if ( s_pThreadVars->m_ThreadTicks < 0)
		{
			x_DebugMsg("--------------- THREAD STALL -------------------\n");
			xthread::DumpThreads();
			s_pThreadVars->m_ThreadTicks = 40;
		}
	}
}
#else

void xthread::DumpThreads(void)
{
}

#endif
//==============================================================================
// 
// SYSTEM PROVIDED TASK SWITCHING LOCKING MECHANISM
//
//==============================================================================
xbool   IntsDisabled;
#if defined(TARGET_PC)
#include <windows.h>
CRITICAL_SECTION s_Critical;
xbool s_crit_initialized = FALSE;
//==============================================================================
void x_BeginAtomic(void)
{
    if (!s_crit_initialized)
    {
        InitializeCriticalSection( &s_Critical ); 
        s_crit_initialized=TRUE;
    }
    EnterCriticalSection(&s_Critical);
	IntsDisabled++;
}

//==============================================================================
void x_EndAtomic(void)
{
	ASSERT(IntsDisabled);
	IntsDisabled--;
    LeaveCriticalSection(&s_Critical);
}

xbool x_IsAtomic(void)
{
	return IntsDisabled!=0;
}
#elif defined(TARGET_PS2)
s32 s_DisabledCount = 0;
s32 s_OldState;
//==============================================================================
void x_BeginAtomic(void)
{
	s32 oldstate;

	if (s_DisabledCount==0)
	{
#ifdef TARGET_PS2_IOP
		CpuSuspendIntr(&oldstate);
#else
		DI();
#endif
		if (s_DisabledCount==0)
		{
			s_OldState = oldstate;
		}
	}
    s_DisabledCount++;
}

//==============================================================================
void x_EndAtomic(void)
{
    ASSERT(s_DisabledCount);
    s_DisabledCount--;
    if (s_DisabledCount==0)
    {
#ifdef TARGET_PS2_IOP
		CpuResumeIntr(s_OldState);
#else
        EI();
#endif
    }
}

xbool x_IsAtomic(void)
{
	return s_DisabledCount!=0;
}
#else
#error "Need a x_BeginAtomic() and a x_EndAtomic() function for this target platform"
#endif

#ifdef ENABLE_PROFILE
#ifndef USE_SN_PROFILER
void    s_ProfileDelayCallback(s32 id, u16 count,void *arg)
{
    (void)id;
    (void)count;
    iWakeupThread((s32)arg);
    ExitHandler();
}
//-----------------------------------------------------------------------------
static void s_ProfileThread(void)
{
	profile_sample*	pBuffer;
	s32				Count;
	s32				status;
	ThreadParam		info;
	s32				lastpc;


	s_pThreadVars->m_ProfileInterval = PROFILE_INTERVAL;				// 10ms profile interval
	s_pThreadVars->m_ProfileEnabled  = TRUE;

	while(1)
	{
		pBuffer = s_pThreadVars->m_pProfileBuffer;
		Count = s_pThreadVars->m_ProfileLength / sizeof(profile_sample);
		ASSERT(Count);
		ASSERT(pBuffer);
		ASSERT(s_pThreadVars->m_ProfileInterval);

		while (Count)
		{
		    SetAlarm(s_pThreadVars->m_ProfileInterval,s_ProfileDelayCallback,(void *)GetThreadId());
		    SleepThread();

			if (s_pThreadVars->m_ProfileEnabled)
			{
				status = ReferThreadStatus(s_pThreadVars->m_pAppMain->GetSystemId(),&info);
				lastpc = (s32)info.entry;
				pBuffer->ProgramCounter = ENDIAN_SWAP_32(lastpc);
				pBuffer++;
				Count--;
			}
		}
		sceWrite(s_pThreadVars->m_ProfileFileHandle,s_pThreadVars->m_pProfileBuffer,s_pThreadVars->m_ProfileLength);
	}

}
#endif

void xprof_Enable(void)
{
	s_pThreadVars->m_ProfileEnabled = TRUE;
#ifdef USE_SN_PROFILER
	snProfEnableInt();
#endif
}

void xprof_Disable(void)
{
	s_pThreadVars->m_ProfileEnabled = FALSE;
#ifdef USE_SN_PROFILER
	snProfDisableInt();
#endif
}

void xprof_SetSampleRate(s32 ms)
{
	s_pThreadVars->m_ProfileInterval = ms;
#ifdef USE_SN_PROFILER
//	snProfSetInterval(ms);
#endif
}

void xprof_Flush(void)
{
}
#else
void xprof_Enable(void)
{
}

void xprof_Disable(void)
{
}

void xprof_SetSampleRate(s32)
{
}

void xprof_Flush(void)
{
}

#endif