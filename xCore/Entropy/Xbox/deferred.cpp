///////////////////////////////////////////////////////////////////////////////
// INCLUDES 
///////////////////////////////////////////////////////////////////////////////

#include "deferred_pipeline.hpp"
#include "x_threads.hpp"



///////////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////////

extern IDirect3DDevice8 * g_pd3dInternal;

#if SWITCH_USE_DEFERRED_RENDERING
static IDeferred3DDevice8 s_D3DDeferred;
IDeferred3DDevice8      * g_pd3dDevice = &s_D3DDeferred;
#endif

push_mgr g_Cmds;



///////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF SEMAPHORE MEMBER CLASS
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

semaphore::semaphore( s32 Count )
{
    m_pHandles[0]=CreateSemaphore( NULL,Count-1,Count-1,NULL );
}

///////////////////////////////////////////////////////////////////////////////

semaphore::~semaphore( void )
{
    void*& hEvent = m_pHandles[0];
    if( hEvent )
    {
        CloseHandle( hEvent );
        hEvent = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool semaphore::release( s32 Count )
{
    return !!ReleaseSemaphore( HANDLE( m_pHandles[0] ),Count,NULL );
}

///////////////////////////////////////////////////////////////////////////////

void semaphore::acquire( void )
{
    WaitForSingleObject( HANDLE( m_pHandles[0] ),INFINITE );
}



///////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF EVENT MEMBER CLASS
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

event::event( bool bManualReset )
{
    m_pHandles[0]=CreateEvent( NULL,bManualReset,FALSE,NULL );
}

///////////////////////////////////////////////////////////////////////////////

event::~event( void )
{
    void*& hEvent = m_pHandles[0];
    if( hEvent )
    {
        CloseHandle( hEvent );
        hEvent = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

void event::acquire( void )
{
    WaitForSingleObject( HANDLE( m_pHandles[0] ),INFINITE );
}

///////////////////////////////////////////////////////////////////////////////

void event::signal( void )
{
    SetEvent( HANDLE( m_pHandles[0] ));
}



///////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF PUSH MANAGER INTERFACES
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

push_mgr::push_mgr( void ): Flare(0), SemCritical(1), SemGPU(MAX_CMD_STREAM)
{
    new xthread( GpuFeed,"GPU Feed",MAX_CMD_STACK,2 );

    m_pStream     = new queue<u8,MAX_CMD_BUFFER>[MAX_CMD_STREAM];
    m_StreamIndex = 0;
    BytesUsed     = 0;
    Lock          = 0;
    Base          = 0;
    Mark          = 0;
    APID          = 0;
}

///////////////////////////////////////////////////////////////////////////////

void push_mgr::flip( void )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 159,88,130,255 ),"Vsync" );
#endif

    // present previous frame .............................................

    #if (defined X_DEBUG) && (defined bhapgood) && 1
    {
        static u32 Counter = 0;
        if( (Counter%240)==239 )
            x_DebugMsg( "%8u: UPDATE produced %08X bytes\n",Counter,back().count() );
        Counter++;
    }
    #endif

    eng_SyncVBlank();

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif

    g_pd3dInternal->Present( 0,0,0,0 );

    // toggle front/back streams ..............................................

    SemCritical.acquire(); m_StreamIndex++;
    SemCritical.release();

    // fire off a flare to start gpu running ..................................

    #if SWITCH_MULTITHREADED
    Flare.signal();
    #else
    kick();
    #endif

    // flush the new pending stream ...........................................

    SemGPU.acquire();
    back( ).flush( );
    SemGPU.release();
}

///////////////////////////////////////////////////////////////////////////////

void push_mgr::kick( void )
{
#ifdef X_DEBUG
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 159,88,130,255 ),"Kicking DList" );
#endif

    SemGPU.acquire();
    {
        queue<u8,MAX_CMD_BUFFER>& Stream = front();

        // kick alloc buffer queues ////////////////////////////////////////////

        s32 Ceiling = Stream.count();
        if( Ceiling )
        {
            #ifdef X_DEBUG
            xtimer Timer; Timer.Start();
            D3D__SingleStepPusher = 0;
            #endif

            // execute all packets ********************************************

            Stream.flush();

            for( s32 i=0;Stream.count()<Ceiling;i++ )
            {
                // allocate over first packet .................................

                BytesUsed     = * (s32*)Stream.alloc( 4 );
                void* pPacket = Stream.alloc( BytesUsed );

                // call D3D API ...............................................

                __asm
                {
                    push esi
                    push edi
                    push ebx
                         mov  ebx,esp              // Preserve stack pointer
                         mov  esi,[pPacket]        // Get data pointer
                         lodsd                     // Get param size
                         mov  ecx,eax              // Save it
                         lodsd                     // Get API
                         test ecx,ecx              // Args?
                         je   sk                   // Nope, skip
                              lea    edx,[ecx*4]   //   Multiply by 4
                              sub    esp,edx       //   Allocate params
                              mov    edi,esp       //   Prepare copy
                              rep    movsd         //   Wheeeee!
                    sk:  call eax                  // Run API
                         cmp  esp,ebx              // Is stack ok?
                         je   sa                   // Yep, continue
                              int    3             //   Oh crap
                sa: pop  ebx
                    pop  edi
                    pop  esi
                }
            }
            g_pd3dInternal->BlockUntilIdle();

            // debug logging **************************************************

            #if (defined X_DEBUG) && (defined bhapgood) && 1
            {
                D3D__SingleStepPusher = 0;

                static u32 Counter = 0;
                if( (Counter%240)==239 )
                {
                    f32 Time = Timer.StopMs()/240.0f;
                    x_DebugMsg( "%8u: RENDER consumed %08X bytes in %fms.\n",Counter,Ceiling,Time );
                }
                Counter++;
            }
            #endif
        }
    }
    SemGPU.release();

#ifdef X_DEBUG
     D3DPERF_EndEvent();
#endif
}



///////////////////////////////////////////////////////////////////////////////
// RENDER THREAD IMPLEMENTATION - SITS AROUND FOREVER; NEVER DIES; IMMORTAL !!!
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

void push_mgr::GpuFeed( void )
{
    for(;;)
    {
        g_Cmds.Flare.acquire();
        g_Cmds.kick();
    }
}
