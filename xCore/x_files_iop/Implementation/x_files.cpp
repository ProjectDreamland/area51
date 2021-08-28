//==============================================================================
//
//  x_files.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_FILES_HPP
#include "../x_files.hpp"
#endif

#ifndef X_FILES_PRIVATE_HPP
#include "x_files_private.hpp"
#endif

#ifndef X_STDIO_PRIVATE_HPP
#include "x_stdio_private.hpp"
#endif

#include "../x_threads.hpp"
//------------------------------------------------------------------------------

#if defined( TARGET_PC )
#include <windows.h>
#endif

//==============================================================================
//  DEFINITIONS
//==============================================================================

#ifdef X_DEBUG
#define X_THREAD_DEBUG
#endif

//==============================================================================
//  VARIABLES
//==============================================================================

static s32                s_Initialized = 0;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void x_Init( void )
{
    //
    // If this is the first time here, then initialize the global x_files 
    // subsystems.
    //

    if( s_Initialized == 0 )
    {
        x_IOInit();
        x_MemInit();
        x_TimeInit();
    }
    s_Initialized++;

    //
    // Initialize the x_files within the current thread.
    //
    x_InitThreads();
}

//==============================================================================

void x_Kill( void )
{
    //
    // If this is the last "session" of the x_files to go, then shut down 
    // everything else as well.
    //

    if( s_Initialized == 1 )
    {
        x_KillThreads();
        x_TimeKill();
        x_MemKill();
        x_IOKill();
    }
    s_Initialized--;

}

//==============================================================================

s32 x_GetThreadID( void )
{
    xthread* pThread;

    pThread = x_GetCurrentThread();
    ASSERT(pThread);
    return pThread->GetId();
}

//==============================================================================

x_thread_globals* x_GetThreadGlobals( void )
{
    xthread* pThread;

    pThread = x_GetCurrentThread();
    ASSERT(pThread);
    return pThread->GetGlobals();
}

//==============================================================================

s32 x_GetInitialized( void )
{
    return( s_Initialized );
}

//==============================================================================
