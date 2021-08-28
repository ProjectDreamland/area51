//=========================================================================
//
// TASKFARM.HPP
//
//=========================================================================
#ifndef TASKFARM_HPP
//=========================================================================
#include "x_files.hpp"
#include "x_threads.hpp"
#include "CommandLine.hpp"
//=========================================================================

class task_farm_link;
class task_farm_request;
class task_farm_thread;
class task_farm_dispatcher;

//=========================================================================

class task_farm_link
{
public:
    task_farm_link();
    ~task_farm_link();

    xbool IsEmpty   ( void );
    void  Clear     ( void );
    void  SetOwner  ( void* apOwner );
    void* GetOwner  ( void );

    task_farm_link* GetNext( void );
    task_farm_link* GetPrev( void );

    void InsertAtHead( task_farm_link& Link );
    void InsertAtTail( task_farm_link& Link );
    void Remove( void );

    task_farm_link* PopHead( void );
    task_farm_link* PopTail( void );

private:

    void*       pOwner;
    task_farm_link* pNext;
    task_farm_link* pPrev;
};

//=========================================================================

class task_farm_request
{
public:
                    task_farm_request();
    virtual         ~task_farm_request();
    virtual void    Execute( void )=0;

    xbool           IsCompleted     ( void ) { return m_State==STATE_COMPLETED; }
    xtick           GetExecutionTime( void ) { return m_ExecutionTime; }

private:
    
    void            AddToTailOfList    ( task_farm_link& Chain );
    void            RemoveFromList     ( void );

    enum thread_task_state 
    {
        STATE_UNINITIALIZED=0,
        STATE_POSTED_TO_DISPATCHER,
        STATE_POSTED_TO_THREAD,
        STATE_EXECUTING,
        STATE_COMPLETED
    };

    task_farm_link      m_Link;
    s32                 m_iThread;
    thread_task_state   m_State;
    xtick               m_PostTime;
    xtick               m_IdleTime;
    xtick               m_ExecutionTime;
    s32                 m_iExecGroup;

    friend task_farm_dispatcher;
    friend task_farm_thread;
};

//=========================================================================

class task_farm_thread
{
public:

    task_farm_thread() {};
    ~task_farm_thread() {};

    void Init       ( void );
    void Kill       ( void ){};
    void Execute    ( void );

private:

    // Initialized by dispatcher
    s32                     m_iThread;
    xmesgq*                 m_pMsgQ;
    task_farm_dispatcher*   m_pDispatcher;
    xthread*                m_pThread;
    task_farm_request*      m_pCurrentTask;

    // Stats collected at the thread level
    struct stats
    {
        s32     m_nTasksProcessed;
        s32     m_MaxMsgsInQueue;
        xtick   m_TimeStarted;
        xtick   m_TimeExisted;
        xtick   m_TimeIdle;
        xtick   m_TimeExecuting;
        xtick   m_TimeReporting;
    };
    stats                   m_Stats;

    // State of thread
    enum task_farm_thread_state
    {
        STATE_INIT=0,
        STATE_IDLE,
        STATE_EXECUTING,
        STATE_REPORTING,
        STATE_SHUTDOWN,
    };
    task_farm_thread_state m_State;

    friend task_farm_dispatcher;
    friend void TaskFarmThread_Thread( s32 argc, char** argv );
};

//=========================================================================

class task_farm_dispatcher
{
public:

    task_farm_dispatcher() {m_bInitialized=0;};
    ~task_farm_dispatcher() {};

    void Init                       ( s32 nThreads );
    void Kill                       ( void );

    // Provides a new task to the dispatcher
    void PostTask                   ( task_farm_request* pTask );

    // Blocks until all outstanding tasks are completed
    void FlushTasks                 ( void );

    void Pause                      ( void );
    void Unpause                    ( void );

    // Execution Groups
    //s32  BeginExecGroup             ( void );
    //void FlushExecGroup             ( s32 iGroup );

private:

    void    Execute                 ( void );
    void    ReportCompletedTask     ( task_farm_request* pTask );
    xbool   ProcessMsgs             ( void );
    void    ProcessMsg              ( task_farm_request* pTask );
    void    QueueRequestedTask      ( task_farm_request* pTask );
    void    QueueCompletedTask      ( task_farm_request* pTask );
    void    ServiceRequestQueue     ( void );

private:

    // Processor threads
    #define MAX_TASK_FARM_THREADS   8
    s32                m_nProcessorThreads;
    task_farm_thread   m_ProcessorThread[MAX_TASK_FARM_THREADS];

    // Message Queue
    xmesgq*     m_pMsgQ;
    xthread*    m_pThread;

    // Chains
    task_farm_link  m_RequestList;
    task_farm_link  m_CompletedList;

    // State info
    xbool       m_bInitialized;
    xbool       m_bPaused;

    // Stats collected at the dispatcher level
    struct stats
    {
        s32     m_nTasksReceived;
        s32     m_nTasksDispatched;
        s32     m_nTasksCompleted;
        xtick   m_TimeStarted;
        xtick   m_TimeExisted;
        xtick   m_TimeBlockingOnMsgQ;
        xtick   m_TimeBlockingOnThreadMsgQ;
        xtick   m_TimeFlushing;
    };
    stats                   m_Stats;

    enum task_farm_dispatcher_state
    {
        STATE_INIT=0,
        STATE_WAITING_FOR_TASK,
        STATE_PROCESSING_TASK_REQUEST,
        STATE_PROCESSING_COMPLETED_TASK,
        STATE_SHUTDOWN,
        STATE_KILL,
    };
    task_farm_dispatcher_state m_State;

    // Execution group
    s32         m_iExecGroup;

    friend task_farm_thread;
    friend void TaskFarmDispatcher_Thread( s32 argc, char** argv );

};

//=========================================================================

#include "TaskFarm_Inline.hpp"

//=========================================================================

extern task_farm_dispatcher g_TaskFarmDispatcher;

//=========================================================================
#endif //TASKFARM_HPP
//=========================================================================
