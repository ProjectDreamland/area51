#include "..\x_files.hpp"
#include "..\x_threads.hpp"
#include "x_threads_private.hpp"

//
// Define this to enable profile code. You will also need to make sure the timer routines in x_timer.cpp are
// using the non-zero timer versions. See notes there.
//
#if defined(TARGET_PS2) && defined(X_DEBUG) && (defined(bwatson) || defined(sskelton) )
//#define ENABLE_PROFILE
#define USE_SN_PROFILER
#endif


#ifdef TARGET_PC
#include <windows.h>
#endif

#ifdef TARGET_XBOX
#include <Xtl.h>
#endif

#ifdef TARGET_PS2
#include <eekernel.h>
#endif

#ifdef TARGET_GCN
#include <dolphin/os.h>
#endif

#ifdef ENABLE_PROFILE
#include <sifdev.h>
#include <libsn.h>
#include "../entropy/ps2/iopmanager.hpp"
#define PROFILE_BUFFER_LENGTH	X_KILOBYTE(16)			// 16K profile buffer
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
#define MAX_TRACKED_THREADS     16

#ifdef DEBUG_THREADS
static void s_WatchdogThread(void);
// Need to declare these here so we can be sure of the construction order
// on the PC.
static xmutex*      s_MutexBuffer[X_MAX_MUTEXES];
xarray<xmutex*>     xmutex::m_MasterMutexList(s_MutexBuffer,0,X_MAX_MUTEXES);

static xsema*       s_SemaphoreBuffer[X_MAX_SEMAPHORES];
xarray<xsema*>      xsema::m_MasterSemaphoreList(s_SemaphoreBuffer,0,X_MAX_SEMAPHORES);

static xthread*     s_ThreadBuffer[X_MAX_THREADS];
xarray<xthread*>    xthread::m_MasterThreadList( s_ThreadBuffer,0,X_MAX_THREADS );
#endif

#define IDLE_UTILIZATION_HISTORY 10

struct thread_vars
{
    xthreadlist			m_RunList;
    s32					m_ThreadTicks;
    xmutex				m_Lock;
    s32					m_TopThreadId;
    s32                 m_InterruptCount;
    xthread*			m_pAppMain;
    s64                 m_IdleTicks;
    volatile xthread*           m_pActiveThread;
    volatile xthread_private    m_ActiveThreadId;
    struct
    {
        xthread*        pThread;
        s32             Last[IDLE_UTILIZATION_HISTORY];
        s32             Average;
        s32             Max;
    } m_Idle;

    xthread*            m_ThreadList[MAX_TRACKED_THREADS];

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

static char         s_ThreadVars[sizeof(thread_vars)];
static thread_vars* s_pThreadVars = NULL;
static xbool        s_Initialized = FALSE;
#ifdef TARGET_PC
static char         s_IdleStack[32*1024];
#else
static char         s_IdleStack[1*1024];
#endif
static char         s_IdleThread[sizeof(xthread)];

#define X_THR_START_SENTINAL (0x4afb0001)
void x_thread_Root(void* pInit);

extern "C" void main(s32 argc,char** argv);
static xthread* FindThread(void);
static void x_IdleLoop(void);
        
void CreateThreadVars( void );
void CreateIdleThread( void );

//-----------------------------------------------------------------------------
// Global startup function
void x_InitThreads(s32 argc, char** argv)
{
    (void)argc;
    (void)argv;
    ASSERT(!s_Initialized);
    ASSERT(!s_pThreadVars);

    // IMPORTANT!!!!!!!!
    // Call thread_vars creation function. We jump through these hoops here because of the new logging system
    // x_InitThreads needs to complete with no memory allocations, hence we can't "new" the thread_vars
    // structure it must be new'd in place from a preallocated static buffer. To "new" in place we must
    // #undef new which is why the CreateThreadVars function is at the end of this file.
    CreateThreadVars();

    s_pThreadVars->m_ThreadTicks  = 1000;
    s_pThreadVars->m_TopThreadId  = 1;
    s_pThreadVars->m_InterruptCount = 0;

    s_Initialized = TRUE;

    // IMPORTANT!!!!!!!!
    // Call idle thread creation function. We jump through these hoops here because of the new logging system
    // x_InitThreads needs to complete with no memory allocations, hence we can't "new" the thread_vars
    // structure it must be new'd in place from a preallocated static buffer. To "new" in place we must
    // #undef new which is why the CreateMainThread function is at the end of this file.
    CreateIdleThread();

#ifdef ENABLE_PROFILE
	s_pThreadVars->m_ProfileLength  = PROFILE_BUFFER_LENGTH;
	s_pThreadVars->m_pProfileBuffer = (profile_sample*)x_malloc(PROFILE_BUFFER_LENGTH);
	ASSERT(s_pThreadVars->m_pProfileBuffer);
#ifdef USE_SN_PROFILER
	g_IopManager.LoadModule("snprofil.irx");
	snProfInit(_4KHZ,s_pThreadVars->m_pProfileBuffer,s_pThreadVars->m_ProfileLength);
	//xprof_Disable();
#else
	s_pThreadVars->m_ProfileFileHandle = sceOpen("host0:"PROFILE_FILENAME,SCE_WRONLY|SCE_CREAT|SCE_TRUNC);
	ASSERT(s_pThreadVars->m_ProfileFileHandle >=0);
	s_pThreadVars->m_pProfileThread = new xthread(s_ProfileThread,"Profiler Thread",8192,1);
	ASSERT(s_pThreadVars->m_pProfileThread);
#endif
#endif

#ifdef DEBUG_THREADS
	s_pThreadVars->m_pWatchdogThread = new xthread(s_WatchdogThread,"Watchdog timer",8192,5);
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
        pThread = s_pThreadVars->m_RunList.GetHead();
        if (!pThread)
            break;
        if( (pThread != (xthread*)s_IdleThread) )
            delete pThread;
        else
            pThread->Unlink();
    }

    if( s_pThreadVars != (thread_vars*)s_ThreadVars )
        delete s_pThreadVars;

    s_pThreadVars = NULL;
    s_Initialized = FALSE;
}
//-----------------------------------------------------------------------------
// NOTE: This is only used for the initial thread structure that is created for
// the main game entry point. It will be used to set the 'default' thread to
// idle priority and also allocate some stack space for that thread that will only
// end up being used for text format buffers since the stack is intrinsically set
// by the system startup.

xthread::xthread(s32 StackSize, const char* pName)
{
#ifdef TARGET_PC
    StackSize = MAX( StackSize, 32768 );
#endif

    // Stack has to be allocated before we lock the thread handler system
    m_pStack        = x_malloc(StackSize);
    ASSERT(m_pStack);

    m_ThreadId = s_pThreadVars->m_TopThreadId;

    //ASSERTS(m_ThreadId==1,"Constructor is for the main thread only.");
    s_pThreadVars->m_TopThreadId++;

    m_Globals.NextOffset= 0;
    m_Globals.StringBuffer= (char*)m_pStack;
    m_Globals.BufferSize= (StackSize > X_KILOBYTE(1))?(StackSize-X_KILOBYTE(1)):(StackSize/2);
    m_pName             = pName;
    m_Priority          = 0;
    m_BasePriority      = 0;
    m_pNext             = NULL;
    m_pPrev             = NULL;
    m_pOwningQueue      = NULL;
    m_NeedToTerminate   = FALSE;
    m_Status            = RUNNING;
    //
    // Insert this thread at the end of the list of threads. This will ensure the
    // thread list is in order of creation
    //
    s_pThreadVars->m_ThreadTicks = 10000;
    m_Initialized = TRUE;

    x_memset(m_pStack,m_ThreadId,StackSize);

    m_System = sys_thread_Create(NULL,NULL,NULL,0,THREAD_BASE_PRIORITY);
    Link();
    x_SetCurrentThread(this);
    sys_thread_SetPriority(m_System,THREAD_BASE_PRIORITY);

    //
    // Last thing we do is add it to the runlist
    //
    s32 i;
    for( i=0; i<MAX_TRACKED_THREADS; i++ )
    {
        if( s_pThreadVars->m_ThreadList[i]==NULL )
        {
            s_pThreadVars->m_ThreadList[i]=this;
            break;
        }
    }
#ifdef DEBUG_THREADS
    m_MasterThreadList.Append(this);
#endif
}

xthread::xthread( void )
{
}

void xthread::InitIdle( void* pUserStack, s32 StackSize )
{
    // Stack has to be allocated before we lock the thread handler system
    m_pStack        = pUserStack; //x_malloc(StackSize);
    ASSERT(m_pStack);

    m_ThreadId = s_pThreadVars->m_TopThreadId;

    //ASSERTS(m_ThreadId==1,"Constructor is for the main thread only.");
    s_pThreadVars->m_TopThreadId++;

    m_Globals.NextOffset= 0;
    m_Globals.StringBuffer= (char*)m_pStack;
    m_Globals.BufferSize= (StackSize > X_KILOBYTE(1))?(StackSize-X_KILOBYTE(1)):(StackSize/2);
    m_pName             = "Idle";
    m_Priority          = 0;
    m_BasePriority      = 0;
    m_pNext             = NULL;
    m_pPrev             = NULL;
    m_pOwningQueue      = NULL;
    m_NeedToTerminate   = FALSE;
    m_Status            = RUNNING;
    //
    // Insert this thread at the end of the list of threads. This will ensure the
    // thread list is in order of creation
    //
    s_pThreadVars->m_ThreadTicks = 10000;
    m_Initialized = TRUE;

    x_memset(m_pStack,m_ThreadId,StackSize);

    m_System = sys_thread_Create(NULL,NULL,NULL,0,THREAD_BASE_PRIORITY);
    Link();
    x_SetCurrentThread(this);
    sys_thread_SetPriority(m_System,THREAD_BASE_PRIORITY);

    //
    // Last thing we do is add it to the runlist
    //
    s32 i;
    for( i=0; i<MAX_TRACKED_THREADS; i++ )
    {
        if( s_pThreadVars->m_ThreadList[i]==NULL )
        {
            s_pThreadVars->m_ThreadList[i]=this;
            break;
        }
    }
#ifdef DEBUG_THREADS
    m_MasterThreadList.Append( this );
#endif
}

//-----------------------------------------------------------------------------
// Thread constructor (adds it to the master list and determines correct id #
void xthread::Init    (x_thread_entry_fn *pEntry,const char *pName,s32 StackSize,s32 Priority,s32 argc, char** argv)
{
    // Stack has to be allocated before we lock the thread handler system
#if defined(TARGET_XBOX) || defined(TARGET_PC)
    // For xbox and pc, the stack is automatically sized by the system so we only allocate enough space for
    // the string buffers
#ifdef TARGET_PC
    u32 StringSize          = X_KILOBYTE(32); // PC get's a bigger string buffer
#else
    u32 StringSize          = X_KILOBYTE(4);
#endif
    m_pStack                = x_malloc(StringSize);
    m_Globals.BufferSize    = StringSize;
    m_Globals.StringBuffer  = (char*)m_pStack;
    x_memset(m_pStack,m_ThreadId,StringSize);
#else
    m_pStack                = x_malloc(StackSize);
    m_Globals.BufferSize    = (StackSize > X_KILOBYTE(1))?(StackSize-X_KILOBYTE(1)):(StackSize/2);
    m_Globals.StringBuffer  = (char*)m_pStack;
    x_memset(m_pStack,m_ThreadId,StackSize);
#endif
    ASSERT(m_pStack);

    s_pThreadVars->m_Lock.Enter();
    m_ThreadId              = s_pThreadVars->m_TopThreadId;

    s_pThreadVars->m_TopThreadId++;
    m_Globals.NextOffset    = 0;
    m_pName                 = pName;
    m_Priority              = Priority;
    m_BasePriority          = Priority;
    m_pNext                 = NULL;
    m_pPrev                 = NULL;
    m_pOwningQueue          = NULL;
    m_Status                = RUNNING;
    m_NeedToTerminate       = FALSE;
    //
    // Insert this thread at the end of the list of threads. This will ensure the
    // thread list is in order of creation
    //
    s_pThreadVars->m_ThreadTicks = 10000;
    m_Initialized           = TRUE;

    
    // Prep for starting thread
    m_Startup.Sentinal      = X_THR_START_SENTINAL;
    m_Startup.argc          = argc;
    m_Startup.argv          = argv;
    m_Startup.pEntry        = pEntry;

    // Now kick off the thread 'bootstrap' helper function
    m_System = sys_thread_Create(x_thread_Root,this,m_pStack,StackSize,THREAD_BASE_PRIORITY+Priority);
    Link();
    sys_thread_Start(m_System,this);
    x_SetCurrentThread(FindThread());

    //
    // Last thing we do is add it to the runlist
    //
    s32 i;
    for( i=0; i<MAX_TRACKED_THREADS; i++ )
    {
        if( s_pThreadVars->m_ThreadList[i]==NULL )
        {
            s_pThreadVars->m_ThreadList[i]=this;
            break;
        }
    }
#ifdef DEBUG_THREADS
    m_MasterThreadList.Append( this );
#endif
    s_pThreadVars->m_Lock.Exit();
}

//-----------------------------------------------------------------------------
// Even though we are using a static data area to 'start' up a thread, this will have locked access since the
// access to the thread vars is currently locked.

void x_thread_Root(void* pInit)
{
    xthread* pThread=(xthread*)pInit;
    
    x_SetCurrentThread( pThread );
    ASSERTS( pThread->m_Startup.Sentinal == X_THR_START_SENTINAL, "x_thread_Root: Invalid xthread structure passed" );
    pThread->m_Startup.pEntry( pThread->m_Startup.argc, pThread->m_Startup.argv );
    pThread->Kill();
}

//-----------------------------------------------------------------------------
// Thread destructor
xthread::~xthread(void)
{
    ASSERT(m_Initialized);
    m_Initialized = FALSE;
    s_pThreadVars->m_Lock.Enter();

    // Did we ever resize the text format buffer?
    if (m_Globals.StringBuffer != m_pStack)
    {
        x_free(m_Globals.StringBuffer);
    }

    xtimer CloseTimeout;

    if( m_Status != TERMINATED )
    {
        m_NeedToTerminate = TRUE;
        CloseTimeout.Start();

        // Give the thread a little breathing time while needed to terminate so it
        // can gracefully deal with some conditions.
        if( this != x_GetCurrentThread() )
        {
            while( m_Status != TERMINATED )
            {
                if( CloseTimeout.ReadMs() > 200.0f )
                {
                    break;
                }
                s_pThreadVars->m_Lock.Exit();
                sys_thread_Delay(1);
                s_pThreadVars->m_Lock.Enter();
            }
        }

        //
        // Now wake it up from it's slumber to terminate it if needed
        //
        while( m_Status != TERMINATED )
        {
            s_pThreadVars->m_Lock.Exit();

            sys_thread_Delay(1);
            Resume(0);
            s_pThreadVars->m_Lock.Enter();

            if( CloseTimeout.ReadMs() > 1500.0f )
            {
                #if defined(bwatson)  
                ASSERTS( FALSE, "Thread did not gracefully exit when requested." );
                #endif                
                
                break;
            }
        }
        CloseTimeout.Stop();
    }

//  LOG_MESSAGE( "xthread::~xthread","Thread '%s' Shutdown took %2.02fms",m_pName,CloseTimeout.ReadMs());
    m_Status = TERMINATED;
    sys_thread_Destroy( m_System );

    if( m_pOwningQueue )
    {
        Unlink( *m_pOwningQueue );
    }

    if( m_pStack != s_IdleStack )
    {
        x_free(m_pStack);
    }

    s32 i;
    for( i=0; i<MAX_TRACKED_THREADS; i++ )
    {
        if( s_pThreadVars->m_ThreadList[i]==this )
        {
            s_pThreadVars->m_ThreadList[i]=NULL;
            break;
        }
    }
#ifdef DEBUG_THREADS
    m_MasterThreadList.Delete( m_MasterThreadList.Find(this) );
#endif
    s_pThreadVars->m_Lock.Exit();

}

//-----------------------------------------------------------------------------
xbool xthread::IsActive(void)
{
    if( m_NeedToTerminate || (m_Status==TERMINATED) || (m_Status==TERMINATING) )
    {
        return FALSE;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
void xthread::Kill(void)
{
    xbool CommitSuicide;

    ASSERT(m_pOwningQueue==&s_pThreadVars->m_RunList);
    CommitSuicide = ( x_GetCurrentThread()==this );
    x_BeginAtomic();
    Unlink();
    m_Status = TERMINATED;
    x_EndAtomic();
    if( CommitSuicide )
    {
        sys_thread_Exit(0);
    }
}

//-----------------------------------------------------------------------------
// Note:
// 1. The thread should currently be in the runlist.
// 2. It will be moved to the specified list
// 3. On resume, it should NOT be 'owned' by anyone
// 4. It will be placed back in the runlist.
void xthread::Suspend( s32 Flags, state Status )
{
    if( m_NeedToTerminate )
    {
        return;
    }
    m_Status = Status;

    x_EndAtomic();                          // Exit the atomic state
    sys_thread_Suspend( m_System, Flags );  // Actually suspend the thread
    x_BeginAtomic();                        // Enter the atomic state
    x_SetCurrentThread( this );
}

//-----------------------------------------------------------------------------
void xthread::Resume( s32 Flags )
{
    if( m_NeedToTerminate==FALSE )
    {
        m_Status = RUNNING;
    }

//    ASSERT( x_IsAtomic() == FALSE );
    sys_thread_Resume( m_System, Flags );
    x_SetCurrentThread( this );
}

//-----------------------------------------------------------------------------
// Link this thread to the runlist
//-----------------------------------------------------------------------------
void    xthread::Link( void )
{
    s_pThreadVars->m_RunList.Link( this );
}

//-----------------------------------------------------------------------------
// Unlink this thread from the runlist
//-----------------------------------------------------------------------------
void xthread::Unlink( void )
{
    s_pThreadVars->m_RunList.Unlink( this );
}

//-----------------------------------------------------------------------------
// Link this thread to a specific thread list
//-----------------------------------------------------------------------------
void    xthread::Link( xthreadlist &List )
{
    ASSERT( List.Find( GetSystemId() )==NULL );
    List.Link( this );
}

//-----------------------------------------------------------------------------
// Unlink this thread from a specific thread list
//-----------------------------------------------------------------------------
void xthread::Unlink( xthreadlist &List )
{
    ASSERT( List.Find( GetSystemId() )!=NULL );
    List.Unlink( this );
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
    s32      SystemId;

    pThread = m_pHead;

    while (pThread)
    {
        SystemId = pThread->GetSystemId();
        if (SystemId == id)
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
    ASSERT( Find(pThread->GetSystemId())==NULL );
    ASSERT( pThread->m_pPrev==NULL );
    ASSERT( pThread->m_pNext==NULL );
    ASSERT( pThread->m_pOwningQueue==NULL );

    if( m_pHead==NULL )
    {
        // The list is empty
        ASSERT( m_pTail==NULL );

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
    pThread->m_pOwningQueue = this;
}

//-----------------------------------------------------------------------------
// Unlink a thread from this list
//-----------------------------------------------------------------------------
void    xthreadlist::Unlink(xthread *pThread)
{
    ASSERT(pThread->m_pOwningQueue);
    ASSERT(m_pHead);
    ASSERT(m_pTail);

    // Make sure this thread is in the specified list
    ASSERT(Find(pThread->GetSystemId()));

    if( pThread->m_pPrev )
    {
        pThread->m_pPrev->m_pNext = pThread->m_pNext;
    }
    else
    {
        m_pHead = pThread->m_pNext;
    }

    if( pThread->m_pNext )
    {
        pThread->m_pNext->m_pPrev = pThread->m_pPrev;
    }
    else
    {
        m_pTail = pThread->m_pPrev;
    }

    pThread->m_pNext = NULL;
    pThread->m_pPrev = NULL;
    pThread->m_pOwningQueue = NULL;
}

//-----------------------------------------------------------------------------
// Unlink the first thread within this list
//-----------------------------------------------------------------------------
xthread* xthreadlist::GetHead(void)
{
    return m_pHead;
}


//-----------------------------------------------------------------------------
void xthread::Delay(s32 milliseconds)
{
    ASSERT(s_Initialized);
    if( milliseconds <=0 )
        return;

    if( IsActive()==FALSE )
    {
        return;
    }
    m_Status = BLOCKED_ON_TIMER;
    sys_thread_Delay(milliseconds);

    m_Status = RUNNING;
}

//-----------------------------------------------------------------------------
void x_DelayThread(s32 milliseconds)
{
    xthread* pThread;
    if( milliseconds <=0 )
        return;
    pThread = x_GetCurrentThread();
    pThread->Delay( milliseconds );
}

//-----------------------------------------------------------------------------
void x_StartMain         (x_thread_entry_fn* pEntry, s32 argc, char** argv)
{
    s_pThreadVars->m_pAppMain = new xthread( pEntry," Main Application", X_KILOBYTE(128), 0, argc, argv );
    x_IdleLoop();
}

//-----------------------------------------------------------------------------
xtick x_GetIdleTicks(void)
{
    if( s_pThreadVars->m_Idle.Max )
    {
        return s_pThreadVars->m_IdleTicks * x_GetTicksPerMs() * 100 / s_pThreadVars->m_Idle.Max;
    }
    return 0;
}

//-----------------------------------------------------------------------------
s32 x_GetMainThreadID( void )
{
    return s_pThreadVars->m_pAppMain->GetId();
}

//-----------------------------------------------------------------------------
// The idle loop doesn't really do much. It just sits and counts. When 100ms has
// passed, it will re-average the cpu utilization. That is actually quite hard
// to do since the processor may be 100% busy. In that case, it will not be
// very accurate!
static void x_IdleLoop(void)
{

    s32     ticks;
    s32     index;
    xtick   StartTicks;
#if defined( bwatson ) && defined( TARGET_DEV )
    xtick   CurrentTicks;
    s32     average;
#endif // defined( bwatson ) && defined( TARGET_DEV )
    xtick   ticks_for_100ms;
    xthread_private Pid;

    Pid = sys_thread_GetId();
    sys_thread_SetPriority(Pid,0);

    ticks           = 0;
    index           = 0;
    StartTicks      = x_GetTime();
    ticks_for_100ms = x_GetTicksPerMs() * 100;
    while( s_pThreadVars->m_pAppMain->IsActive() )
    {
#if defined( bwatson ) && defined( TARGET_DEV )
        ticks++;

        s_pThreadVars->m_IdleTicks++;
        CurrentTicks = x_GetTime() - StartTicks;
        if( CurrentTicks > 10 * ticks_for_100ms )
        {
            CurrentTicks = 10 * ticks_for_100ms;
            StartTicks = x_GetTime() - 10 * ticks_for_100ms;
        }

        if( CurrentTicks >= ticks_for_100ms )
        {
            // We update the idle time tracking every 100ms
            while( CurrentTicks >= ticks_for_100ms )
            {
                // If more than 200ms has passed, then this means the last tick
                // was totally busy so we zero all the array entries for the amount
                // of time that has passed.
                s_pThreadVars->m_Idle.Last[index] = ticks;
                if( ticks > s_pThreadVars->m_Idle.Max )
                {
                    s_pThreadVars->m_Idle.Max = ticks;
                }
                CurrentTicks -= ticks_for_100ms;
                StartTicks += ticks_for_100ms;
                index++;
                if( index >= IDLE_UTILIZATION_HISTORY )
                {
                    index = 0;
                }
                ticks = 0;
            }

            average=0;
            for( ticks=0; ticks<IDLE_UTILIZATION_HISTORY; ticks++ )
            {
                average += s_pThreadVars->m_Idle.Last[ticks];
            }

            s_pThreadVars->m_Idle.Average = average / IDLE_UTILIZATION_HISTORY;

            ticks=0;
        }
#endif // defined( bwatson ) && defined( TARGET_DEV )
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//------------- DEBUG FUNCTIONS
//-----------------------------------------------------------------------------
void x_CheckThreads(xbool ForceDump)
{
    (void)ForceDump;

    if( ForceDump )
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

static xthread* FindThread(void)
{
    xthread_private Current;
    xthread* pReturn;

    pReturn = (xthread*)s_pThreadVars->m_pActiveThread;
    Current = sys_thread_GetId();
    if( (xthread_private&)s_pThreadVars->m_ActiveThreadId != Current )
    {
        x_BeginAtomic();
        pReturn = s_pThreadVars->m_RunList.Find( Current.ThreadId );
        if( !pReturn )
            return NULL;
        (xthread_private&)s_pThreadVars->m_ActiveThreadId = Current;
        s_pThreadVars->m_pActiveThread = pReturn;
        x_EndAtomic();
    }
    return pReturn;
}

//-----------------------------------------------------------------------------
xthread* x_GetCurrentThread( void )
{
    //ASSERT(FindThread() == s_pThreadVars->m_pActiveThread);

    if( !s_pThreadVars )
        return NULL;

    return FindThread();

}
//-----------------------------------------------------------------------------
void x_SetCurrentThread( xthread* pThread )
{
    if( pThread==NULL )
    {
        pThread = FindThread();
    }
}

//-----------------------------------------------------------------------------
void x_WatchdogReset(void)
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
            BREAK;
		}
	}
}
#else

void xthread::DumpThreads(void)
{
}

#endif

//==============================================================================

#if defined( TARGET_PC )
static volatile LONG s_InterruptCount=0;
#else
static volatile s32 s_InterruptCount=0;
#endif

void x_BeginAtomic(void)
{
    sys_thread_Lock();

#if defined( TARGET_PC )
    InterlockedIncrement( &s_InterruptCount );
#else
    s_InterruptCount++;
#endif
}

//==============================================================================
xbool x_IsAtomic(void)
{
//#if defined( TARGET_PC )
//    sys_thread_Lock();
//#endif

    s32 InterruptCount = s_InterruptCount;

//#if defined( TARGET_PC )
//    sys_thread_Unlock();
//#endif

    return (InterruptCount > 0);
}

//==============================================================================
void x_EndAtomic(void)
{
    ASSERT(s_InterruptCount);

#if defined( TARGET_PC )
    InterlockedDecrement( &s_InterruptCount );
#else
    s_InterruptCount--;
#endif

    sys_thread_Unlock();
}

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

//=============================================================================
// Create thread vars structure. See comment near top of x_InitThreads for
// information on why this is here, etc.

#undef new

void CreateThreadVars( void )
{
    new((void*)&s_ThreadVars) thread_vars;
    s_pThreadVars = (thread_vars*)&s_ThreadVars;
    ASSERT(s_pThreadVars);
}

void CreateIdleThread( void )
{
    new((void*)&s_IdleThread) xthread;
    ((xthread*)s_IdleThread)->InitIdle( s_IdleStack, sizeof(s_IdleStack) );
    s_pThreadVars->m_pAppMain = (xthread*)s_IdleThread;
}

//=============================================================================
