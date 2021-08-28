//=========================================================================
//
// TASKFARM.HPP
//
//=========================================================================
#include "TaskFarm.hpp"
#include "x_files.hpp"
#include "x_threads.hpp"
//=========================================================================
// Know what task each thread is working on - done
// Handle thread timeout
// Test with variable time-length tasks - done
// See why we aren't getting both pipelines working ????
//=========================================================================


//=========================================================================

task_farm_dispatcher g_TaskFarmDispatcher;

//=========================================================================

void TaskFarmThread_Thread( s32 argc, char** argv );
void TaskFarmDispatcher_Thread( s32 argc, char** argv );


//=========================================================================
// TASK_FARM_REQUEST
//=========================================================================

task_farm_request::task_farm_request() 
{ 
    m_State=STATE_UNINITIALIZED;
    m_PostTime      = 0;
    m_IdleTime      = 0;
    m_ExecutionTime = 0;
    m_iThread       = -1;
    m_Link.SetOwner(this);
    m_iExecGroup    = -1;
}

//=========================================================================

task_farm_request::~task_farm_request() 
{
    ASSERT( (m_State != STATE_POSTED_TO_DISPATCHER) &&
            (m_State != STATE_POSTED_TO_THREAD) &&
            (m_State != STATE_EXECUTING) );
}

//=========================================================================

void task_farm_request::AddToTailOfList( task_farm_link& Chain ) 
{ 
    m_Link.InsertAtTail( Chain ); 
}

//=========================================================================

void task_farm_request::RemoveFromList( void )              
{ 
    m_Link.Remove(); 
}

//=========================================================================
