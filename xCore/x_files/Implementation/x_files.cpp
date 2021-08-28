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
#include "../x_context.hpp"

//------------------------------------------------------------------------------

#if defined( TARGET_PC )
#include <windows.h>
// This is PC specific. This will force x_Init to be called
// prior to the constructors so constructors should be able to
// do memory allocations WITH the allocator initialized.
#pragma warning(push)

#pragma warning(disable:4073)
#pragma  init_seg(lib)
class pc_forcestartup
{
public:
    pc_forcestartup(void)
    {
        x_Init(0,NULL);
    }

    ~pc_forcestartup(void)
    {
        x_Kill();
    }
};

pc_forcestartup ForceStartup;

#pragma warning(pop)
#endif


//------------------------------------------------------------------------------

#if defined( TARGET_XBOX )
#   ifdef CONFIG_RETAIL
#       define D3DCOMPILE_PUREDEVICE 1
#   endif
#include <xtl.h>
// This is Xbox specific. This will force x_Init to be called
// prior to the constructors so constructors should be able to
// do memory allocations WITH the allocator initialized.
#pragma warning(push)

#pragma warning(disable:4073)
#pragma  init_seg(lib)
class xbox_forcestartup
{
public:
    xbox_forcestartup(void)
    {
        x_Init(0,NULL);
    }

    ~xbox_forcestartup(void)
    {
        x_Kill();
    }
};

xbox_forcestartup ForceStartup;

#pragma warning(pop)
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

static s32              s_Initialized = 0;

static char             s_StringBuffer[X_GLOBAL_STRING_BUFFER_SIZE];
x_thread_globals        s_DummyGlobals;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void x_Init( s32 argc, char** argv )
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
        x_ContextInit();
        //
        // Initialize the x_files within the current thread.
        //
        x_InitThreads(argc,argv);
    }
    s_Initialized++;

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
        x_ContextKill();
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
    if (pThread==NULL)
    {
        s_DummyGlobals.StringBuffer = s_StringBuffer;
        s_DummyGlobals.NextOffset   = 0;
        s_DummyGlobals.BufferSize   = sizeof(s_StringBuffer);
        return &s_DummyGlobals;
    }
    ASSERT(pThread);
    return pThread->GetGlobals();
}

//==============================================================================

s32 x_GetInitialized( void )
{
    return( s_Initialized );
}

//==============================================================================
