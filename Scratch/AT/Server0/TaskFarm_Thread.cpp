//=========================================================================
//
// TASKFARM_THREAD.CPP
//
//=========================================================================
#include "TaskFarm.hpp"
//=========================================================================

//=========================================================================
// TASK_FARM_THREAD
//=========================================================================

void task_farm_thread::Init( void )
{
    m_State = task_farm_thread::STATE_INIT;
    m_pCurrentTask = NULL;

    // Clear the stats
    x_memset(&m_Stats,0,sizeof(m_Stats));
}

//=========================================================================

void task_farm_thread::Execute( void )
{
    // Collect global start time of thread
    m_Stats.m_TimeStarted = x_GetTime();

    // Setup and report thread priority
    {
        HANDLE T = GetCurrentThread();
        SetThreadPriority(T,THREAD_PRIORITY_ABOVE_NORMAL);
        s32 P = GetThreadPriority(T);
        //TASK_FARM_LOG("Thread %d initialized at priority %d.\n",m_iThread,P);
    }

    while( 1 )
    {
        //TASK_FARM_LOG("Thread %d waiting for task.\n",m_iThread);

        // Block for a new task
        m_State = task_farm_thread::STATE_IDLE;
        {
            // Begin timing for idle stats
            xtick IdleTimeStart = x_GetTime();

            // Block on Msg Queue waiting for a new task
            m_pCurrentTask = (task_farm_request*)m_pMsgQ->Recv(MQ_BLOCK);

            // If NULL Task then exit loop shut down thread
            if( m_pCurrentTask==NULL )
                break;

            // Peek at number of messages 'before' we pulled this one out
            s32 nMsgs = m_pMsgQ->ValidEntries() + 1;
            m_Stats.m_MaxMsgsInQueue = MAX( m_Stats.m_MaxMsgsInQueue, nMsgs );

            // Update Stats
            m_Stats.m_TimeIdle += x_GetTime() - IdleTimeStart;
            m_Stats.m_nTasksProcessed++;
            m_pCurrentTask->m_IdleTime = x_GetTime() - m_pCurrentTask->m_PostTime;

            // Log the new task received
            //TASK_FARM_LOG("[%2d] received [%08X] idle [%5.2f]\n",m_iThread,(u32)m_pCurrentTask,x_TicksToMs(x_GetTime() - IdleTimeStart));
        }


        // Execute the new task
        m_State = task_farm_thread::STATE_EXECUTING;
        {
            // Begin timing for execution stats
            xtick ExecTimeStart = x_GetTime();

            // Allow task to execute
            m_pCurrentTask->Execute();

            // Update Stats in thread and task
            xtick ExecTime = x_GetTime() - ExecTimeStart;
            m_Stats.m_TimeExecuting += ExecTime;
            m_pCurrentTask->m_ExecutionTime = ExecTime;
        }

        // Report completed task to dispatcher
        m_State = task_farm_thread::STATE_REPORTING;
        {
            // Begin timing for execution stats
            xtick ReportTimeStart = x_GetTime();

            //TASK_FARM_LOG("Thread %d reporting completed task %08X\n",m_iThread,(u32)m_pCurrentTask);
            m_pDispatcher->ReportCompletedTask( m_pCurrentTask );
            m_pCurrentTask = NULL;

            // Update Stats in thread and task
            m_Stats.m_TimeReporting += x_GetTime() - ReportTimeStart;
        }
    }

    // Shutdown thread
    m_State = task_farm_thread::STATE_SHUTDOWN;
    {
        // Compute total lifetime
        m_Stats.m_TimeExisted = x_GetTime() - m_Stats.m_TimeStarted;
    }
}

//=========================================================================
