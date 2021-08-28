//==============================================================================
//
//  x_threads.hpp
//
//==============================================================================

#ifndef _X_THREADS_HPP
#define _X_THREADS_HPP


#ifdef TARGET_PS2_DEV
//#define DEBUG_THREADS
#endif
//==============================================================================
//  
//  This file provides basic, cross platform multithreading capability. It requires
//  only four system dependant functions. The host system needs to provide the 
//  following function sets:
//
//		x_BeginAtomic()
//			- Locks out all context switching. Between the BeginAtomic and EndAtomic
//			  code blocks, we can be assured that no context switch in this process
//			  tree will occur.
//		x_EndAtomic()
//			- Re-enables context switching.
//		xthread::Suspend()
//			-  Suspends the current thread. Moves it from the system running queue
//			   to the system suspended queue. These queues are independent from those
//			   maintained internally.
//		xthread::Resume()
//			-  Sets a thread back to a running state. The system can reschedule this
//			   thread as it sees fit.
//
//  Each thread can also have an additional set of information associated with it.
//  Things such as text format buffers
//==============================================================================

#include "x_types.hpp"
#include "x_array.hpp"

#include "implementation/x_files_private.hpp"

#define X_MAX_THREADS           16
#define X_MAX_SEMAPHORES        16
#define X_MAX_MUTEXES           16
#define X_MAX_MESSAGE_QUEUES    32

#define THREAD_BASE_PRIORITY    64
#define MAX_DEFAULT_MESSAGES    16           // Number of messages held internally within the structure. 

#define X_TH_NOBLOCK		   (0<<0)
#define X_TH_BLOCK			   (1<<0)
#define X_TH_JAM			   (1<<1)
#define X_TH_INTERRUPT		   (1<<7)

class xthread;

//==============================================================================
// THREAD LISTS
// These are just essentially internally to keep track of current threads. There
// is a master RunList which contains all threads ready to execute. Each semaphore
// has two lists, one of tasks waiting to acquire the semaphore and another for
// tasks waiting to release the semaphore
//==============================================================================
// Note: Each thread can exist in multiple queues. We keep one for actual running
// and the other for internal tracking (for example, runlist and global thread list
class xthreadlist
{

public:
                        xthreadlist     (void);
                       ~xthreadlist     (void);
            void        Link            (xthread* pThread);
            void        Unlink          (xthread* pThread);
            xthread*    Find            (s32 Id);
            xthread*    UnlinkFirst     (void);
protected:
            xthread*                    m_pHead;
            xthread*                    m_pTail;
};

//==============================================================================
// THREAD CLASS
//==============================================================================
typedef void entry_fn( void );

class xthread
{
public:
    enum state {
            RUNNING,
            KILLING,
            SUSPENDED,
            LIMBO,
            BLOCKED_ON_MESSAGE_SEND,
            BLOCKED_ON_MESSAGE_RECV,
            BLOCKED_ON_SEMAPHORE_ACQUIRE,
            BLOCKED_ON_SEMAPHORE_RELEASE,
            BLOCKED_ON_MUTEX,
            BLOCKED_ON_UNKNOWN,
    };

                        xthread             (entry_fn* pEntryPoint, const char* pName,s32 StackSize, s32 Priority);
                        xthread             (s32 StackSize);
                       ~xthread             (void);
            void        Delay               (s32 milliseconds);

            void        Suspend             (s32 Flags, state State);
            void        Resume              (s32 Flags);

            void        SetPriority         (s32 Priority);
            s32         GetPriority         (void);
            void        Unlink              (xthreadlist& List);				// Remove from specific list    
            void        Unlink              (void);								// Remove from run list
            void        Link                (xthreadlist& List);				// Link to specific list
            void        Link                (void);								// Link to runlist
            s32         GetId               (void)          { return m_ThreadId;            };
            s32         GetSystemId         (void)          { return m_SysThreadId;         };

            x_thread_globals* GetGlobals    (void)          { return &m_Globals;            };
            void        SetFormatBufferSize (s32 Size);

static		void        DumpThreads         (void);
            void        DumpState           (void);
            xthread*    m_pPrev;
            xthread*    m_pNext;
protected:
            s32         m_SysThreadId;
#ifdef TARGET_PC
			s32			m_SysHandle;
#endif
            xbool       m_Initialized;
            s32         m_ThreadId;
            s32         m_Priority;
            s32         m_BasePriority;
            x_thread_globals m_Globals;
const       char*       m_pName;
            void*       m_pStack;
            state       m_Status;

protected:
#ifdef DEBUG_THREADS
static      xarray<xthread*> m_MasterThreadList;
#endif
};

//==============================================================================
// THREAD FUNCTIONS MAINLY PERFORMED ON THE CURRENT EXECUTING THREAD
//==============================================================================
            void        x_InitThreads   (void);
            void        x_KillThreads   (void);

            void        x_DelayThread   (s32 milliseconds);
            s32         x_GetThreadId   (void);
            xthread*    x_GetCurrentThread(void);

// Checks thread system integrity. Should catch deadlocks
            void        x_CheckThreads  (xbool ForceDump);
            void        x_WatchdogReset (void);

//==============================================================================
// SEMAPHORES (defined in x_mutex.cpp)
//==============================================================================
// These provide the base level atomic synchronisation primitives. All higher 
// level thread synchronisation is performed using semaphores.

class xsema
{
public:
                        xsema           ( s32 count,s32 initial);
                       ~xsema           ( void );

		    xbool       Acquire         ( s32 Flags );
            xbool       Release         ( s32 Flags );

            void        Acquire         ( void )			{ Acquire(X_TH_BLOCK); };
			void 		Release			( void )			{ Release(X_TH_BLOCK); };

protected:
            xbool       m_Initialized;
            xthreadlist m_WaitingAcquire;
            xthreadlist m_WaitingRelease;
            s32         m_Count;
            s32         m_Available;
protected:
#ifdef DEBUG_THREADS
static      xarray<xsema*> m_MasterSemaphoreList;
#endif
};

//==============================================================================
// MUTEX
// Since the implementation of semaphores is significantly more optimal than
// using a system call, the additional overhead for implementing a mutex by 
// using a single entry semaphore is minimal. To keep code down, let's just make
// it a semaphore! Only difference is that mutexes should have been Entered()
// before they can be Exited()
//==============================================================================
// When a mutex is initially constructed, it is unlocked
class xmutex
{
public:
                        xmutex          ( void );
                       ~xmutex          ( void );

			xbool		Enter		    ( s32 Flags );
            xbool       Exit            ( s32 Flags );

			void        Enter           ( void )			{ Enter(X_TH_BLOCK); };
			void		Exit			( void )			{ Exit(X_TH_BLOCK);  };

            void        Acquire         ( void )			{ Enter(X_TH_BLOCK); };          // Same functionality as enter
            void        Release         ( void )			{ Exit(X_TH_BLOCK);  };          // Same functionality as exit

            xbool       Acquire         ( s32 Flags )       { return Enter(Flags);  };
            xbool       Release         ( s32 Flags )       { return Exit(Flags);  };

protected:
            xthreadlist m_Waiting;
            xbool       m_Initialized;
            xsema       m_Semaphore;
protected:
#ifdef DEBUG_THREADS
static      xarray<xmutex*> m_MasterMutexList;
#endif

};

//==============================================================================
// MESSAGE QUEUE
//==============================================================================
#define MQ_NOBLOCK              X_TH_NOBLOCK	// Operation will not block if needed, just returns false instead
#define MQ_BLOCK                X_TH_BLOCK		// Operation will block if resource is not available
#define MQ_JAM                  X_TH_JAM	    // Callee will have higher priority to acquire resource than others

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

			void		xprof_Enable				(void);
			void		xprof_Disable			(void);
			void		xprof_Flush				(void);
			void		xprof_SetSampleRate		(s32 ms);

            void        x_QueueDebugDump(void);
            void        x_DumpThreads(void);

//==============================================================================
// System specific functions to enter a mutually exclusive section that should not
// be interrupted. On the PS2 this is done by disabling interrupts, on other
// platforms, it can be done using a system mutex or lock of some kind.
//==============================================================================
            void        x_BeginAtomic(void);
            void        x_EndAtomic(void);
			xbool		x_IsAtomic(void);

#endif