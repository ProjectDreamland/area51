//=========================================================================
//
// TASKFARM_DISPATCHER.CPP 
//
//=========================================================================
#include "TaskFarm.hpp"

//=========================================================================

void TaskFarmThread_Thread( s32 argc, char** argv )
{
    task_farm_thread* pProcessor = (task_farm_thread*)argv;

    pProcessor->Execute();

    return;
}

//=========================================================================

void TaskFarmDispatcher_Thread( s32 argc, char** argv )
{
    task_farm_dispatcher* pDispatcher = (task_farm_dispatcher*)argv;

    pDispatcher->Execute();

    return;
}

//=========================================================================

void task_farm_dispatcher::ServiceRequestQueue( void )
{
    m_State = task_farm_dispatcher::STATE_PROCESSING_TASK_REQUEST;

    //TASK_FARM_LOG("ServiceRequestQueue Start\n");
    while( 1 )
    {
        if( m_RequestList.IsEmpty() )
        {
            //TASK_FARM_LOG("ServiceRequestQueue Queue is EMPTY\n");
            break;
        }

        // Get a task from the queue
        task_farm_link* pLink = (task_farm_link*)m_RequestList.GetNext();
        task_farm_request* pTask = (task_farm_request*)pLink->GetOwner();


        // Look for an idle thread
        s32 i;
        s32 iBestThread=0;
        s32 iLowestNMsgs=m_ProcessorThread[iBestThread].m_pMsgQ->ValidEntries();
        for( i=1; i<m_nProcessorThreads; i++ )
        {
            s32 NMsgs = m_ProcessorThread[i].m_pMsgQ->ValidEntries();
            if( NMsgs < iLowestNMsgs )
            {
                iBestThread = i;
                iLowestNMsgs = NMsgs;
            }
        }

        // If all threads have too many msgs then go back to sleep until
        // someone is finished.
        if( iLowestNMsgs >= 4 )
            break;

        // Hand task to thread with fewest number of msgs
        {
            pTask->RemoveFromList();
           
            //TASK_FARM_LOG("Dispatcher task sent %08X to thread %d.\n",(u32)pTask,iBestThread);

            pTask->m_State = task_farm_request::STATE_POSTED_TO_THREAD;
            pTask->m_iThread = iBestThread;

            xtick BlockOnThreadQStart = x_GetTime();
            m_Stats.m_nTasksDispatched++;
            m_ProcessorThread[iBestThread].m_pMsgQ->Send( pTask, MQ_BLOCK );
            m_Stats.m_TimeBlockingOnThreadMsgQ += x_GetTime() - BlockOnThreadQStart;
        }
    }
    //TASK_FARM_LOG("ServiceRequestQueue Finish\n");
}

//=========================================================================

void task_farm_dispatcher::QueueRequestedTask( task_farm_request* pTask )
{
    m_Stats.m_nTasksReceived++;

    //TASK_FARM_LOG("Dispatcher added task request %08X to req queue.\n",(u32)pTask);

    pTask->AddToTailOfList( m_RequestList );
}

//=========================================================================

void task_farm_dispatcher::QueueCompletedTask( task_farm_request* pTask )
{
    m_State = task_farm_dispatcher::STATE_PROCESSING_COMPLETED_TASK;

    m_Stats.m_nTasksCompleted++;

    //TASK_FARM_LOG("TaskComplete %08X %2d %8.3fms %6.2fms\n",
    //    (u32)pTask,pTask->m_iThread,x_TicksToMs(pTask->m_IdleTime),x_TicksToMs(pTask->m_ExecutionTime));

    //TASK_FARM_LOG("Dispatcher received finished task %08X from thread %d, %8fms\n",(u32)pTask,pTask->m_iThread,x_TicksToMs(pTask->m_ExecutionTime));

    pTask->AddToTailOfList( m_CompletedList );
}

//=========================================================================

void task_farm_dispatcher::ProcessMsg( task_farm_request* pTask )
{
    // Watch for tasks being reported back from processor threads
    if( pTask->m_State == task_farm_request::STATE_COMPLETED )
    {
        QueueCompletedTask( pTask );
    }
    else
    // Watch for new request being posted
    if( pTask->m_State == task_farm_request::STATE_POSTED_TO_DISPATCHER )
    {
        QueueRequestedTask( pTask );
    }
    else
    // Task is in an invalid state
    {
        ASSERTS(FALSE,"Task is in an invalid state");
    }
}

//=========================================================================

xbool task_farm_dispatcher::ProcessMsgs( void )
{
    // This routine blocks on the first incoming message and then
    // loops through all messages and sends them to different queues
    // to be processed.  It returns FALSE if the shutdown message was
    // received.
    xtick BlockOnMsgQStartTime = x_GetTime();

    m_State = task_farm_dispatcher::STATE_WAITING_FOR_TASK;

    //TASK_FARM_LOG("Dispatcher waiting for msgs.\n");
    
    xbool bDoBlock = TRUE;

    while( 1 )
    {
        // Block for first msg, otherwise just peek at queue
        task_farm_request* pTask = NULL;
        
        if( bDoBlock )
        {
            pTask = (task_farm_request*)m_pMsgQ->Recv(MQ_BLOCK);
            m_Stats.m_TimeBlockingOnMsgQ += x_GetTime() - BlockOnMsgQStartTime;
            bDoBlock = FALSE;
        }
        else
        {
            pTask = (task_farm_request*)m_pMsgQ->Recv(MQ_NOBLOCK);
        }

        // Was msg queue empty?
        if( pTask==NULL )
            break;
        
        // Was this a 'shutdown' msg?
        if( (u32)pTask == 0x00000001 )
            return FALSE;

        // Was this a 'wakeup' msg?
        if( (u32)pTask == 0x00000002 )
            continue;

        // Process msg 
        ProcessMsg( pTask );
    }

    return TRUE;
}

//=========================================================================

void task_farm_dispatcher::Execute( void )
{
    HANDLE T = GetCurrentThread();
    SetThreadPriority(T,THREAD_PRIORITY_HIGHEST);
    s32 P = GetThreadPriority(T);
    //TASK_FARM_LOG("Dispatcher thread initialized at priority %d\n",P);

    m_Stats.m_TimeStarted = x_GetTime();

    while( 1 )
    {
        xbool bShutdown = !ProcessMsgs();

        if( bShutdown )
            break;

        if( m_bPaused==FALSE )
            ServiceRequestQueue();
    }

    m_Stats.m_TimeExisted = x_GetTime() - m_Stats.m_TimeStarted;

    m_State = task_farm_dispatcher::STATE_SHUTDOWN;
    return;
}

//=========================================================================

void task_farm_dispatcher::Init( s32 nThreads )
{
    ASSERT( (nThreads>0) && (nThreads<=16) );
    ASSERT( m_bInitialized==FALSE );
    m_bInitialized = TRUE;
    m_State = task_farm_dispatcher::STATE_INIT;
    m_iExecGroup = 0;
    m_bPaused = FALSE;

    x_memset(&m_Stats,0,sizeof(m_Stats));

    m_RequestList.Clear();
    m_CompletedList.Clear();

    HANDLE T = GetCurrentThread();
    //SetThreadPriority(T,THREAD_PRIORITY_HIGHEST);
    //SetThreadPriority(T,THREAD_PRIORITY_ABOVE_NORMAL);
    s32 P = GetThreadPriority(T);
    x_printf("Task Farm Dispatcher Initializing...\n");
    x_printf("- Creating %d worker threads.\n",nThreads);
    x_printf("- Main thread priority %d\n",P);
    x_printf("- Initialization finished.\n");

    m_nProcessorThreads = nThreads;

    // Initialize processor threads
    s32 i;
    for( i=0; i<m_nProcessorThreads; i++ )
    {
        task_farm_thread& TP = m_ProcessorThread[i];
        TP.Init();
        TP.m_iThread     = i;
        TP.m_pMsgQ       = new xmesgq(8);
        TP.m_pDispatcher = this;
        TP.m_pThread     = new xthread( TaskFarmThread_Thread, xfs("Processor%02d",i), 8192, 4, 1, (char**)(&TP) );
    }

    // Launch execute in new thread
    m_pMsgQ   = new xmesgq(128);
    m_pThread = new xthread( TaskFarmDispatcher_Thread, "Dispatcher", 8192, 5, 1, (char**)(this) );
}

//=========================================================================

void task_farm_dispatcher::Kill( void )
{
    s32 i;

    ASSERT( m_bInitialized );
    m_bInitialized = FALSE;

    // Wait until current work is completed.
    FlushTasks();

    TASK_FARM_LOG("------------------------------------------------------\n");
    TASK_FARM_LOG("task_farm_dispatcher::Kill\n");
    TASK_FARM_LOG("------------------------------------------------------\n");

    // Send processor threads Kill task
    {
        // Send thread exit message
        for( i=0; i<m_nProcessorThreads; i++ )
        {
            task_farm_thread& TP = m_ProcessorThread[i];
            TP.m_pMsgQ->Send( NULL, MQ_BLOCK );
        }

        // Wait until all threads have emptied their message queues
        s32 Count=100;
        while( Count-- )
        {
            for( i=0; i<m_nProcessorThreads; i++ )
            {
                task_farm_thread& TP = m_ProcessorThread[i];
                if( TP.m_State != task_farm_thread::STATE_SHUTDOWN )
                    break;
            }

            if( i==m_nProcessorThreads )
                break;

            x_DelayThread(1);
        }
        
        // Display thread stats
        {
            TASK_FARM_LOG("TID\tnTasks\tIdleT\tExecT\tRepT\tMaxMsg\n");
            for( i=0; i<m_nProcessorThreads; i++ )
            {
                task_farm_thread& TP = m_ProcessorThread[i];
                TASK_FARM_LOG("%02d\t%d\t%1.3f\t%1.3f\t%1.3f\t%d\n",
                    i,
                    TP.m_Stats.m_nTasksProcessed,
                    (f32)x_TicksToSec(TP.m_Stats.m_TimeIdle),
                    (f32)x_TicksToSec(TP.m_Stats.m_TimeExecuting),
                    (f32)x_TicksToSec(TP.m_Stats.m_TimeReporting),
                    TP.m_Stats.m_MaxMsgsInQueue
                    );
            }
        }
/*
        s32     m_nTasksProcessed;
        s32     m_MaxMsgsInQueue;
        xtick   m_TimeStarted;
        xtick   m_TimeExisted;
        xtick   m_TimeIdle;
        xtick   m_TimeExecuting;
        xtick   m_TimeReporting;
*/
        // Delete processor threads
        for( i=0; i<m_nProcessorThreads; i++ )
        {
            task_farm_thread& TP = m_ProcessorThread[i];
            delete TP.m_pThread;
            delete TP.m_pMsgQ;
            TP.Kill();
        }
    }

    // Kill dispatcher thread
    m_State = task_farm_dispatcher::STATE_KILL;
    m_pMsgQ->Send((void*)0x00000001,MQ_BLOCK);
    {
        s32 Count=100;
        while( Count-- )
        {
            if( m_State == task_farm_dispatcher::STATE_SHUTDOWN )
                break;
            x_DelayThread(1);
        }
    }

    // Display dispatcher stats
    {
        TASK_FARM_LOG("------------------------------------------------------\n");
        TASK_FARM_LOG("Dispatcher Tasks:\n");
        TASK_FARM_LOG("Tasks Received:      %d\n",m_Stats.m_nTasksReceived);
        TASK_FARM_LOG("Tasks Dispatched:    %d\n",m_Stats.m_nTasksDispatched);
        TASK_FARM_LOG("Tasks Completed:     %d\n",m_Stats.m_nTasksCompleted);

        f32 MsgQTime = (f32)x_TicksToSec(m_Stats.m_TimeBlockingOnMsgQ);
        f32 TMsgQTime = (f32)x_TicksToSec(m_Stats.m_TimeBlockingOnThreadMsgQ);
        f32 TotalTime = (f32)x_TicksToSec(m_Stats.m_TimeExisted);
        f32 MiscTime = TotalTime - MsgQTime - TMsgQTime;

        TASK_FARM_LOG("Dispatcher Timer:\n");
        TASK_FARM_LOG("Total:           %1.3f\n",TotalTime);
        TASK_FARM_LOG("BlockingOnMsgQ:  %1.3f\n",MsgQTime);
        TASK_FARM_LOG("BlockingOnTMsgQ: %1.3f\n",TMsgQTime);
        TASK_FARM_LOG("Misc:            %1.3f\n",MiscTime);
        TASK_FARM_LOG("Flushing:        %1.3f\n",(f32)x_TicksToSec(m_Stats.m_TimeFlushing));
        TASK_FARM_LOG("------------------------------------------------------\n");
    }

    // Deallocate the dispatcher thread
    delete m_pThread;
    delete m_pMsgQ;
    m_pThread = NULL;
    m_pMsgQ = NULL;

    // Unhook all requests from RequestQueue and CompletedQueue
    {
        while( m_RequestList.IsEmpty() == FALSE )
            m_RequestList.PopHead();

        while( m_CompletedList.IsEmpty() == FALSE )
            m_CompletedList.PopHead();
    }
}

//=========================================================================
/*
s32 task_farm_dispatcher::BeginExecGroup( void )
{
    m_iExecGroup++;
    return m_iExecGroup;
}
*/
//=========================================================================

void task_farm_dispatcher::PostTask( task_farm_request* pTask )
{
    ASSERT( pTask );
    pTask->m_State      = task_farm_request::STATE_POSTED_TO_DISPATCHER;
    pTask->m_iExecGroup = m_iExecGroup;
    pTask->m_PostTime   = x_GetTime();

    m_pMsgQ->Send(pTask,MQ_BLOCK);

    Sleep(0);
}

//=========================================================================

void task_farm_dispatcher::FlushTasks( void )
{
    xtick FlushStartTime = x_GetTime();

    while( m_Stats.m_nTasksReceived > m_Stats.m_nTasksCompleted )
    {
        x_DelayThread(1);
    }

    m_Stats.m_TimeFlushing += x_GetTime() - FlushStartTime;
}

//=========================================================================

void task_farm_dispatcher::ReportCompletedTask( task_farm_request* pTask )
{
    pTask->m_State = task_farm_request::STATE_COMPLETED;
    s32 C=50;
    while( C-- )
    {
        if( m_pMsgQ->Send(pTask,MQ_NOBLOCK) )
        {
            return;
        }
        x_DelayThread(1);
    }
    ASSERTS(FALSE,"Dispatcher cannot receive complete messages!");
}

//=========================================================================

void task_farm_dispatcher::Pause( void )
{
    m_bPaused = TRUE;
    TASK_FARM_LOG("---  PAUSED  ---\n");
    m_pMsgQ->Send((void*)0x00000002,MQ_BLOCK);
    Sleep(0);
}

//=========================================================================

void task_farm_dispatcher::Unpause( void )
{
    m_bPaused = FALSE;
    TASK_FARM_LOG("--- UNPAUSED ---\n");
    m_pMsgQ->Send((void*)0x00000002,MQ_BLOCK);
    Sleep(0);
}

//=========================================================================

