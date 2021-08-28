//=========================================================================
//
// ps2_dlist.hpp
//
//=========================================================================

#ifndef PS2_DLIST_H
#define PS2_DLIST_H

#include "x_types.hpp"
#include "x_debug.hpp"
#include "x_time.hpp"
#include <eekernel.h>

//=========================================================================
// DEFINES
//=========================================================================

// Enable this define to do pointer checking for the MFIFO to make sure
// we're not trying to write to memory that has already been flushed.
#define DLIST_USE_SMART_POINTER     0

// Enable this define to assert when the dlist is used from multiple threads.
#ifdef CONFIG_RETAIL
#define DLIST_ENABLE_THREAD_ASSERT  0
#else
#define DLIST_ENABLE_THREAD_ASSERT  1
#endif

// This define will turn out stats logging and any printfs going to the
// console output.
#if !defined(X_RETAIL) || defined(X_QA) || defined(CONFIG_PROFILE)
#define ENABLE_DLIST_STATS
#endif

// This define will enable all MFIFO events (interrupts, begin/end frame,
// etc.) into a circular buffer. When a problem happens you can look at
// the log to see the events that led up to the problem.
//#define ENABLE_MFIFO_LOG

// If you need to do a DMA reference to an area of memory, but don't want
// to force FlushCache's in the code, use this uncached memory bit.
#define PS2_UNCACHED_MEM    0x20000000

//=========================================================================
// CLASSES
//=========================================================================

class dlist
{
public:
    //=====================================================================
    // INIT/KILL ROUTINES
    //=====================================================================
            dlist           ( void );
            ~dlist          ( void );
    void    Init            ( s32 Size );
    void    Kill            ( void );

    //=====================================================================
    // ALLOCATION FUNCTIONS - note that the dlist must be active for these
    // function calls to be valid
    //=====================================================================
    void*   Alloc           ( s32 Size );
    void    Dealloc         ( s32 Size );
    void    Flush           ( void )        X_SECTION(render_deferred);
    s32     GetAvailable    ( void ) const;

    //=====================================================================
    // LOCKING AND DISABLING FUNCTIONS
    //
    ///Use the locking mechanism to block
    // out scratchpad access by the dlist. At this point, the allocation
    // functionality is temporarily disabled, but the dlist is still
    // considered to be active.
    //
    // Use the disable function to completely stop the MFIFO pipeline.
    // In a retail game, you should never be doing this, but for viewer
    // builds, you can use the functionality to stop the bus for doing
    // screen shots.
    //=====================================================================
    void    LockScratchpad      ( void );
    void    UnlockScratchpad    ( void );
    void    Disable             ( void );
    void    Enable              ( void );

    //=====================================================================
    // FRAME STATE FUNCTIONS
    //
    // Generally, you want to begin a frame, add some tasks to the frame
    // (at least one), and then end the frame. Tasks will cause additional
    // interrupts to occur, so they should be minimized, but they can also
    // be helpful for getting timing information
    //=====================================================================
    void    BeginFrame          ( void );
    void    EndFrame            ( void );
    xbool   InsideFrame         ( void ) const;
    void    BeginTask           ( const char* pTaskName );
    void    EndTask             ( void );
    xbool   InsideTask          ( void ) const;
    void    WaitForTasks        ( void );

    //=====================================================================
    // STATS COLLECTION
    // This is for gathering timing and data stats about the MFIFO. Use
    // it for getting a quick picture of how a scene is performing.
    //=====================================================================
    byte*   GetMFIFOStart       ( void ) const;
    s32     GetMFIFOSize        ( void ) const;
    
    #ifdef ENABLE_MFIFO_LOG
    void    DumpMFIFOLog        ( void );
    #endif // ENABLE_MFIFO_LOG

    #ifdef ENABLE_DLIST_STATS
    xtick       GetVBlankTime   ( void )     const;
    s32         GetPrevNTasks   ( void )     const;
    xtick       GetPrevTaskTime ( s32 Task ) const;
    const char* GetPrevTaskName ( s32 Task ) const;
    #endif // ENABLE_DLIST_STATS

    //=====================================================================
    // THREAD DEBUG FUNCTIONS
    // The display list should only ever be used from a single thread,
    // but sometimes loading screens or dialogs are rendering from a
    // background thread. This is safe to do as long as only one thread
    // is accessing the dlist at a time. To verify that, we'll add a
    // couple debug functions.
    //=====================================================================
    void        SetThreadID     ( s32 ThreadID );
    s32         GetThreadID     ( void ) const;

    //=====================================================================
    // PUBLIC DEBUG DATA
    // This is here just so that the smart pointer method of allocating
    // from the display list will work. See the comments near the smart
    // pointer template for more detail.
    //=====================================================================
    #if DLIST_USE_SMART_POINTER
    void*   m_pLastAlloc;
    #endif // DLIST_USE_SMART_POINTER

private:
    //=====================================================================
    // FRIEND FUNCTIONS
    // C-style interrupts and callbacks to work with the Sony hardware and
    // Sony libraries. These need to access DLIST data directly.
    //=====================================================================

    friend  int VSyncCallback       ( int );
    friend  int GsInterruptHandler  ( int );

    //=====================================================================
    // PRIVATE TYPES AND ENUMERATIONS
    //=====================================================================

    // (tasks+end+vsync+safety buffer) * (running, building, last run for stats)
    enum { MAX_TASKS_PER_FRAME = 80 };
    enum { MAX_DLIST_TASKS     = ((MAX_TASKS_PER_FRAME+3)*3) };

    enum cpu_state
    {
        CPU_STATE_LOGIC = 0,
        CPU_STATE_RENDER,
    };

    enum gpu_state
    {
        GPU_STATE_STOPPED = 0,
        GPU_STATE_RUNNING,
        GPU_STATE_WAIT_VSYNC,
        GPU_STATE_PRESENTED,
    };

    enum signal_type
    {
        SIGNAL_TASK = 0,
        SIGNAL_END_FRAME,
        SIGNAL_RESTART,
    };

    #ifdef ENABLE_MFIFO_LOG
    enum { MAX_LOG_ENTRIES = 256 };

    enum log_action
    {
        LOG_CPU_BEGIN_LOGIC = 0,
        LOG_CPU_BEGIN_RENDER,

        LOG_VSYNC_TIMED_OUT,
        LOG_VSYNC_FRAME_PRESENTED,

        LOG_GS_TASK_FINISHED,
        LOG_GS_FRAME_FINISHED,
        LOG_GS_RESTART,
    };

    struct log_entry
    {
        log_action  Action;
        s32         CPUFrame;
        s32         GPUFrame;
        s32         CPUTask;
        s32         GPUTask;
        cpu_state   CPUState;
        gpu_state   GPUState;
        xtick       Time;
    };
    #endif // ENABLE_MFIFO_LOG

    struct task
    {
        char        TaskName[32];
        signal_type Signal;
        s32         Frame;
        s32         Task;
        xtick       CPUTime;
        xtick       GPUTime;
        u32         DMAStartAddr;
    };

    #ifdef ENABLE_DLIST_STATS
    struct stat_info
    {
        s32     StartTask;
        s32     nTasksRendered;
    };
    #endif

    //=====================================================================
    // HELPER ROUTINES
    //=====================================================================
    void ResetSignal        ( void );
    void HandleMFIFOTimeout ( void );

    #ifdef ENABLE_MFIFO_LOG
    void    AddToLog        ( log_action Action );
    #endif // ENABLE_MFIFO_LOG

    #ifdef ENABLE_DLIST_STATS
    void    GatherMFIFOStats    ( void );
    #endif // ENABLE_DLIST_STATS

    //=====================================================================
    // PRIVATE DATA
    //=====================================================================
    
    // large MFIFO buffer data in main memory
    byte*   m_MFIFOAddr;
    s32     m_MFIFOSize;

    // scratchpad data for MFIFO
    s32     m_SpadBuffer;
    byte*   m_StartAddr[2];
    byte*   m_EndAddr[2];
    byte*   m_CurrAddr;
    u32     m_BackupMADR;

    // state management data
    xbool   m_Disabled;
    xbool   m_Active;
    xbool   m_InsideTask;
    xbool   m_SpadLocked;

    // stats
    #ifdef ENABLE_DLIST_STATS
    xtick       m_VBlankTime;
    stat_info   m_StatInfo;
    #endif // ENABLE_DLIST_STATS

    // system call data
    s32             m_nVSyncsSincePresent;
    volatile xbool  m_TimeOutEnabled;
    xbool           m_TimedOut;
    s32             m_GsHandlerId;
    s32             m_GPUSema;

    // frame data
    s32             m_CPUFrame;
    volatile s32    m_GPUFrame;
    s32             m_CPUTaskID;
    s32             m_GPUTaskID;
    cpu_state       m_CPUState;
    gpu_state       m_GPUState;
    s32             m_nTasks;
    task            m_TaskList[MAX_DLIST_TASKS];

    // thread data
    s32             m_ActiveThreadID;

    // logging data
    #ifdef ENABLE_MFIFO_LOG
    volatile log_entry  m_MFIFOLog[MAX_LOG_ENTRIES];
    volatile s32        m_MFIFOLogIndex;
    #endif // ENABLE_MFIFO_LOG
};

//=========================================================================
// The global DLIST
//=========================================================================

extern dlist DLIST;

//=========================================================================
// Display list pointer template making it a smart pointer. This will
// compile out 100% in retail, but will be extremely useful in finding
// display list crashes. The problem we're trying to find is when someone
// allocates a piece of display list, then allocates a second, then tries
// to write back into the first. Because of the double-buffer nature of
// scratchpad dlist allocations after installing MFIFO, we need to avoid
// this or we will see random hangs and crashes. Catch them all with the
// smart pointer, then turn off the define to get your performance back.
//=========================================================================

#if DLIST_USE_SMART_POINTER
template <class T> class dlist_ptr
{
private:
    T* m_Ptr;
public:
    explicit dlist_ptr( T* p ) : m_Ptr(p) {}
    ~dlist_ptr() {}
    T& operator*()           { ASSERT( (T*)DLIST.m_pLastAlloc==m_Ptr ); return *m_Ptr; }
    T& operator[](s32 Index) { ASSERT( (T*)DLIST.m_pLastAlloc==m_Ptr ); return m_Ptr[Index]; }
    T* operator->()          { ASSERT( (T*)DLIST.m_pLastAlloc==m_Ptr ); return m_Ptr; }
};
#endif

#if DLIST_USE_SMART_POINTER
#define DLPtr(VarName,TypeName)                 dlist_ptr<TypeName> VarName
#define DLPtrAlloc(VarName,TypeName)            dlist_ptr<TypeName> VarName( (TypeName*)DLIST.Alloc(sizeof(TypeName)) )
#define DLArrayAlloc(VarName,TypeName,ArrSize)  dlist_ptr<TypeName> VarName( (TypeName*)DLIST.Alloc(sizeof(TypeName)*ArrSize) )
#else
#define DLPtr(VarName,TypeName)                 TypeName* VarName
#define DLPtrAlloc(VarName,TypeName)            TypeName* VarName = (TypeName*)DLIST.Alloc(sizeof(TypeName))
#define DLArrayAlloc(VarName,TypeName,ArrSize)  TypeName* VarName = (TypeName*)DLIST.Alloc(sizeof(TypeName)*ArrSize)
#endif

//=========================================================================
// An ASSERT for verifying that dlist calls are only happening from within
// a single thread.
//=========================================================================

#if DLIST_ENABLE_THREAD_ASSERT
#include "x_threads.hpp"
#define DLIST_THREAD_SANITY_CHECK()     { ASSERT( (x_GetThreadID() == GetThreadID()) || (GetThreadID() == -1) ); }
#else
#define DLIST_THREAD_SANITY_CHECK()
#endif

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

inline void* dlist::Alloc( s32 Size )
{
    DLIST_THREAD_SANITY_CHECK();
    ASSERT( m_Active && !m_SpadLocked );
    ASSERT( (Size & 0xf) == 0 );

    // Flush if we will overflow scratchpad
    if( (m_CurrAddr+Size) > m_EndAddr[m_SpadBuffer] )
    {
        Flush();
    }

    // store out the last alloc
    #if DLIST_USE_SMART_POINTER
    m_pLastAlloc = m_CurrAddr;
    #endif // DLIST_USE_SMART_POINTER

    // Alloc some space from scratchpad
    byte* AllocAddr = m_CurrAddr;
    m_CurrAddr += Size;
    ASSERT( m_CurrAddr<=m_EndAddr[m_SpadBuffer] );
    
    return AllocAddr;
}

//=========================================================================

inline void dlist::Dealloc( s32 Size )
{
    DLIST_THREAD_SANITY_CHECK();
    ASSERT( m_InsideTask && !m_SpadLocked );
    ASSERT( (Size & 0xf) == 0 );

    // Dealloc the space
    m_CurrAddr -= Size;
    ASSERT( m_CurrAddr >= m_StartAddr[m_SpadBuffer] );
}

//=========================================================================

inline s32 dlist::GetAvailable( void ) const
{
    ASSERT( m_InsideTask && !m_SpadLocked );

    return (s32)(m_EndAddr[m_SpadBuffer]-m_CurrAddr);
}

//=========================================================================

inline xbool dlist::InsideFrame( void ) const
{
    return m_Active;
}

//=========================================================================

inline xbool dlist::InsideTask( void ) const
{
    return m_InsideTask;
}

//=========================================================================

inline byte* dlist::GetMFIFOStart( void ) const
{
    return m_MFIFOAddr;
}

//=========================================================================

inline s32 dlist::GetMFIFOSize( void ) const
{
    return m_MFIFOSize;
}

//=========================================================================

#ifdef ENABLE_DLIST_STATS
inline xtick dlist::GetVBlankTime( void ) const
{
    return m_VBlankTime;
}
#endif

//=========================================================================

inline void dlist::SetThreadID( s32 ThreadID )
{
    m_ActiveThreadID = ThreadID;
}

//=========================================================================

inline s32 dlist::GetThreadID( void ) const
{
    return m_ActiveThreadID;
}

//=========================================================================

#endif  //#ifndef PS2_DLIST_H
