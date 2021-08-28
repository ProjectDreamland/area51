//=========================================================================
//
// MAIN.CPP
//
//=========================================================================
#include "TaskFarm.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================


//=========================================================================

class thread_task_msg : public task_farm_request
{
public:

    thread_task_msg( const char* pStr )
    {
        x_strcpy(Buffer,pStr);
    }

    ~thread_task_msg()
    {}

    virtual void Execute( void )
    {
        x_printf("MSG: %s\n",Buffer);
    }

private:
    char Buffer[256];
};

//=========================================================================

class thread_task_loop : public task_farm_request
{
public:

    thread_task_loop()
    {
    }

    ~thread_task_loop()
    {}

    virtual void Execute( void )
    {
        //x_printf("Running...thread_task_loop::Execute(void)\n");

        // 0,1,1,2,3,5,8
        for( s32 j=0; j<1000; j++ )
        {
            V = 0;
            random Rand;
            for( s32 i=1; i<5000; i++ )
            {
                V += Rand.irand(0,1000);
            }
        }
    }

private:
    s32 V;
};

//=========================================================================

class thread_task_tiny : public task_farm_request
{
public:

    thread_task_tiny()
    {V=0;}

    ~thread_task_tiny()
    {}

    virtual void Execute( void )
    {
        V++;
    }

private:
    s32 V;
};

//=========================================================================

class thread_task_diff_time : public task_farm_request
{
public:

    thread_task_diff_time()
    {DelayTime=0;}

    ~thread_task_diff_time()
    {}

    virtual void Execute( void )
    {
        xtimer Timer;
        Timer.Start();
        while( Timer.ReadMs() < DelayTime ) {}
    }

public:
    f32 DelayTime;
};

//=========================================================================

void Test( void )
{
    g_TaskFarmDispatcher.Init(4);

    xtimer Timer;
    Timer.Start();
/*
    if( 0 )
    {
        thread_task_msg Msg1("HELLO");
        thread_task_msg Msg2("HOW ARE YOU");
        thread_task_msg Msg3("WHAT'S UP!!!");

        g_TaskFarmDispatcher.PostTask( &Msg1 );
        g_TaskFarmDispatcher.PostTask( &Msg2 );
        g_TaskFarmDispatcher.PostTask( &Msg3 );
    }
*/
/*
    //Dispatch a number of tasks
    const s32 NUM_TESTS = 16;
    thread_task_loop Test[NUM_TESTS];
    f32 TaskExecutionTime=0;
    {
        s32 i;

        // Kickoff tasks
        for( i=0; i<NUM_TESTS; i++ )
            g_TaskFarmDispatcher.PostTask( &Test[i] );

        // Wait for tasks to complete
        g_TaskFarmDispatcher.FlushTasks();

        // Collect execution times
        for( i=0; i<NUM_TESTS; i++ )
            TaskExecutionTime += (f32)x_TicksToSec(Test[i].GetExecutionTime());
    }
*/
/*
    //Dispatch a number of tasks
    const s32 NUM_TESTS = 1000;
    thread_task_tiny Test[NUM_TESTS];
    f32 TaskExecutionTime=0;
    {
        s32 i;

        // Kickoff tasks
        for( i=0; i<NUM_TESTS; i++ )
            g_TaskFarmDispatcher.PostTask( &Test[i] );

        // Wait for tasks to complete
        g_TaskFarmDispatcher.FlushTasks();

        // Collect execution times
        for( i=0; i<NUM_TESTS; i++ )
            TaskExecutionTime += (f32)x_TicksToSec(Test[i].m_ExecutionTime);
    }
*/

    // Dispatch a number of tasks
    const s32 NUM_TESTS = 1000;
    thread_task_diff_time Test[NUM_TESTS];
    f32 TaskExecutionTime=0;
    {
        s32 i;
        random Rand;

        // Build test times
        {
            f32 ExpectedExecutionTime = 1000.0f;
            f32 TotalTime = 0;
            for( i=0; i<NUM_TESTS; i++ )
            {
                Test[i].DelayTime = Rand.frand(0.0001f,1.0f);
                TotalTime += Test[i].DelayTime;
            }
            for( i=0; i<NUM_TESTS; i++ )
            {
                Test[i].DelayTime = (ExpectedExecutionTime)*(Test[i].DelayTime/TotalTime);
            }
        }

        //for( s32 T=0; T<320; T++ )
        {
            // Kickoff tasks
            //g_TaskFarmDispatcher.Pause();
            for( i=0; i<NUM_TESTS; i++ )
            {
                g_TaskFarmDispatcher.PostTask( &Test[i] );
                //if( i==10 )break;
            }
            //g_TaskFarmDispatcher.Unpause();

            // Wait for tasks to complete
            g_TaskFarmDispatcher.FlushTasks();
        }

        // Collect execution times
        for( i=0; i<NUM_TESTS; i++ )
            TaskExecutionTime += (f32)x_TicksToSec(Test[i].GetExecutionTime());
    }

    Timer.Stop();
    f32 TotalTime = Timer.ReadSec();
    x_printf("TOTAL TIME:   %8.3f\n",TotalTime);
    x_printf("EXEC  TIME:   %8.3f\n",TaskExecutionTime);
    x_printf("SPEEDUP:      %8.3f\n",(TaskExecutionTime/TotalTime));
    x_printf("OVERH TIME:   %8.3f\n",TotalTime-TaskExecutionTime);

    g_TaskFarmDispatcher.Kill();

    x_printf("FINISHED\n");
    BREAK;

}

//=========================================================================

void PrintHelp( void )
{
    x_printf( "parameters unknown.\n");
}

//=========================================================================

void ExecuteScript( command_line& CommandLine )
{
    xbool           bBitmapLoaded = FALSE;
    const char*     pFileName     = NULL;
    s32             iOptLevel     = 0;
    xbool           bIsXbox       = FALSE;
    s32             Count         = 0;

    x_try;

    //
    // Parse all the options
    //
    for( s32 i=0; i<CommandLine.GetNumOptions(); i++ )
    {
        // Get option name and string
        xstring OptName   = CommandLine.GetOptionName( i );
        xstring OptString = CommandLine.GetOptionString( i );
        xbool   bOption   = FALSE;

        if( OptName == xstring( "S" ))
        {
            x_printf("Server\n");
            x_DebugMsg("Server\n");
            iOptLevel = 3;
            continue;
        }
    }

    x_catch_begin;
    #ifdef X_EXCEPTIONS
        x_printf( "Error: %s\n", xExceptionGetErrorString() );
    #endif
    x_catch_end;
}

//=========================================================================

void main( s32 argc, char* argv[] )
{
    x_Init(argc,argv);
    x_try;

    // save out the exe timestamp for doing dependancy checks
    command_line CommandLine;
    
    // Specify all the options
    CommandLine.AddOptionDef( "S" );

    // Parse the command line
    if( CommandLine.Parse( argc, argv ) )
    {
        PrintHelp();
        return;
    }
    
    // Do the script
    ExecuteScript( CommandLine );

    Test();
    //Test2();

    x_catch_begin;
    #ifdef X_EXCEPTIONS
        x_printf( "Error: %s\n", xExceptionGetErrorString() );
    #endif
    x_catch_end;

    x_Kill();
}
