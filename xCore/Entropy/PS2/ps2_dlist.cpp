//=========================================================================
//
// PS2_DLIST.CPP
//
//=========================================================================

#include "x_debug.hpp"
#include "x_string.hpp"
#include "x_threads.hpp"
#include "ps2_dlist.hpp"
#include "ps2_vifgif.hpp"
#include "ps2_dma.hpp"
#include "x_log.hpp"
#include <eeregs.h>
#include <libgraph.h>

//=========================================================================
// HANDY LOGGING MACROS
//=========================================================================

#ifdef ENABLE_MFIFO_LOG

#define MFIFO_CALLBACK_LOG(action_)     DLIST.AddToLog( action_ )
#define MFIFO_FN_LOG(action_)           {                                           \
                                            s32 DisableRet = DI();                  \
                                            AddToLog( action_ );                    \
                                            if( DisableRet )                        \
                                                EI();                               \
                                        }

#else // ENABLE_MFIFO_LOG

#define MFIFO_CALLBACK_LOG(action_)
#define MFIFO_FN_LOG(action_)

#endif // else ENABLE_MFIFO_LOG

#ifdef dstewart
#define DLIST_ASSERT( x )               if( !(x) )              \
                                        {                       \
                                            BREAK;              \
                                        }
#else
#define DLIST_ASSERT( x )               ASSERT( x )
#endif

//=========================================================================
// INTERNAL TYPES
//=========================================================================

struct signal_packet
{
    dmatag  DMA;
    giftag  GIF;
    u64     Texflush;
    u64     TexflushAddr;
    u64     Signal;
    u64     SignalAddr;
    u128    Pad[16];
};

//=========================================================================
// GLOBALS
//=========================================================================
dlist DLIST;

//=========================================================================
// DLIST IMPLEMENTATION
//=========================================================================

int VSyncCallback( int )
{
    DLIST.m_nVSyncsSincePresent++;

    // If the GPU is waiting for a vsync to present itself to the world, then
    // do that now...the "nVSyncsSincePresent" will lock us to 30 fps. Yes,
    // 60 fps would be better, but switching between 30/60 is worse than a
    // solid 30, because the switch becomes very noticeable.
    if( (DLIST.m_nVSyncsSincePresent>=2) &&
        (DLIST.m_GPUState == dlist::GPU_STATE_WAIT_VSYNC) )
    {
        // present this frame to the world!
        extern void eng_PresentFrame( s32 Frame );
        eng_PresentFrame( DLIST.m_GPUFrame & 1 );
        DLIST.m_nVSyncsSincePresent = 0;

        // let the GPU know that this frame has been presented to the world
        // and reset the SIGNAL register
        DLIST.m_GPUState = dlist::GPU_STATE_PRESENTED;
        DLIST.ResetSignal();
        MFIFO_CALLBACK_LOG( dlist::LOG_VSYNC_FRAME_PRESENTED );
    }
    else if( (DLIST.m_nVSyncsSincePresent > 240) && DLIST.m_TimeOutEnabled )
    {
        // If we've gotten too many vsyncs without a present, then we've
        // probably locked up the gpu or something really bad has occured.
        // Handle that case now...
        DLIST.m_TimedOut = TRUE;
        iSignalSema( DLIST.m_GPUSema );
        MFIFO_CALLBACK_LOG( dlist::LOG_VSYNC_TIMED_OUT );
    }

    ExitHandler();
    return 1;
}

//=========================================================================

int GsInterruptHandler( int )
{
    // Disable all interrupts (It is possible to lose interrupts if we
    // don't do this. Read the technote dated 12/28/99 "Handling GS -> EE
    // Interrupts")
    *GS_IMR = GS_IMR_SIGMSK_M    |
              GS_IMR_FINISHMSK_M |
              GS_IMR_HSMSK_M     |
              GS_IMR_VSMSK_M     |
              GS_IMR_EDWMSK_M    | (0x3<<13);

    xbool ResetSignal = FALSE;

    // Was this caused by a signal event?
    if( ((tGS_CSR*)GS_CSR)->SIGNAL )
    {
        DLIST_ASSERT( (DLIST.m_GPUState != dlist::GPU_STATE_STOPPED) );

        s32 CurrTask = DLIST.m_GPUTaskID;

        // It is possible to get multiple interrupts for the same event,
        // and we need to be able to handle the case. (grr...stupid hardware)
        xbool HandleSignal = TRUE;
        if( DLIST.m_GPUState == dlist::GPU_STATE_WAIT_VSYNC )
            HandleSignal = FALSE;
        else if( (u32)DLIST.m_TaskList[CurrTask].Task != ((tGS_SIGLBLID*)GS_SIGLBLID)->SIGID )
            HandleSignal = FALSE;

        // now make a decision based on the signal we got
        if( HandleSignal )
        {
            // mark down the time of this signal
            DLIST.m_GPUTaskID = (DLIST.m_GPUTaskID + 1) % dlist::MAX_DLIST_TASKS;
            DLIST.m_TaskList[CurrTask].GPUTime = x_GetTime();
            DLIST_ASSERT( DLIST.m_CPUFrame >= DLIST.m_GPUFrame );
            DLIST_ASSERT( DLIST.m_GPUFrame == DLIST.m_TaskList[CurrTask].Frame );

            switch( DLIST.m_TaskList[CurrTask].Signal )
            {
            case dlist::SIGNAL_TASK:
                // Reset the signal so we can move on to the next task
                DLIST_ASSERT( ((tGS_SIGLBLID*)GS_SIGLBLID)->SIGID == (u32)DLIST.m_TaskList[CurrTask].Task );
                ResetSignal = TRUE;
                MFIFO_CALLBACK_LOG( dlist::LOG_GS_TASK_FINISHED );
                break;

            case dlist::SIGNAL_END_FRAME:
                // Don't reset the signal. We need to wait for the vsync to
                // do that for us. But go ahead and put the GPU in that state.
                // We can also signal the semaphore, letting the CPU know that
                // we are finished with this data, so it is safe to start
                // clobbering the data we were using.
                DLIST_ASSERT( ((tGS_SIGLBLID*)GS_SIGLBLID)->SIGID == 0x7ffffffe );
#ifdef ENABLE_DLIST_STATS
                DLIST.GatherMFIFOStats();
#endif
                iSignalSema( DLIST.m_GPUSema );
                DLIST.m_GPUState = dlist::GPU_STATE_WAIT_VSYNC;
                MFIFO_CALLBACK_LOG( dlist::LOG_GS_FRAME_FINISHED );
                break;

            case dlist::SIGNAL_RESTART:
                // We have returned from presenting a frame to the world after
                // a vsync, it is now safe to restart the GPU.
                DLIST_ASSERT( ((tGS_SIGLBLID*)GS_SIGLBLID)->SIGID == 0x7fffffff );
                DLIST.m_GPUFrame++;
                ResetSignal = TRUE;
                DLIST.m_GPUState = dlist::GPU_STATE_RUNNING;
                MFIFO_CALLBACK_LOG( dlist::LOG_GS_RESTART );
                break;
            }
        }
    }
    else
    {
        *GS_IMR = //GS_IMR_SIGMSK_M    |
                  GS_IMR_FINISHMSK_M |
                  GS_IMR_HSMSK_M     |
                  GS_IMR_VSMSK_M     |
                  GS_IMR_EDWMSK_M    | (0x3<<13);
    }

    // re-enable the signal interrupt if necessary
    if( ResetSignal )
    {
        DLIST.ResetSignal();
    }

    ExitHandler();
    return 1;
}

//=========================================================================

dlist::dlist( void ) :
    m_MFIFOAddr                 ( NULL ),
    m_MFIFOSize                 ( 0 ),
    m_SpadBuffer                ( 0 ),
    m_CurrAddr                  ( NULL ),
    m_BackupMADR                ( 0 ),
    m_Disabled                  ( TRUE ),
    m_Active                    ( FALSE ),
    m_InsideTask                ( FALSE ),
    m_SpadLocked                ( FALSE ),
    #ifdef ENABLE_DLIST_STATS
    m_VBlankTime                ( 0 ),
    #endif // ENABLE_DLIST_STATS
    m_nVSyncsSincePresent       ( 0 ),
    m_TimeOutEnabled            ( FALSE ),
    m_TimedOut                  ( FALSE ),
    m_GsHandlerId               ( -1 ),
    m_GPUSema                   ( -1 ),
    m_CPUFrame                  ( 0 ),
    m_GPUFrame                  ( 0 ),
    m_CPUTaskID                 ( 0 ),
    m_GPUTaskID                 ( 0 ),
    m_CPUState                  ( CPU_STATE_LOGIC ),
    m_GPUState                  ( GPU_STATE_STOPPED ),
    m_nTasks                    ( 0 ),
    m_ActiveThreadID            ( -1 )

    #ifdef ENABLE_MFIFO_LOG
    , m_MFIFOLogIndex           ( 0 )
    #endif
{
    // zero everything out
    m_StartAddr[0] = NULL;
    m_StartAddr[1] = NULL;
    m_EndAddr[0]   = NULL;
    m_EndAddr[1]   = NULL;
    
    #ifdef ENABLE_DLIST_STATS
    m_StatInfo.nTasksRendered = 0;
    m_StatInfo.StartTask      = 0;
    #endif // ENABLE_DLIST_STATS
}

//=========================================================================

dlist::~dlist( void )
{
    Kill();
}

//=========================================================================

// DW 9/7/04 - Biscuit's comment nicked from Meridian code base.
// BW 4/28/03 - We don't yet have the notion of aligned allocations within
// the 'standardized' x_memory call spec. We are using a direct reference
// to the Doug Lea allocator so we can get a properly aligned block of memory.
// This actually turns out to be the cleanest way to allow us to specify
// the size of this block of memory. Going through the link file approach
// turned out to be terribly nasty and totally broke the x-files/entropy 
// split paradigm (x-files is NOT dependent on Entropy for any data but
// Entropy is dependent on the facilities provided by x-files).
// x_memory.hpp allocation will need to include the idea of 'fixed address'
// and 'fixed alignment' allocations. This will be needed by other platforms
// than just the PS2.

extern "C" void* dlmemalign(int,size_t,size_t);
extern "C" void  dlfree(int, void*);

void dlist::Init( s32 Size )
{
    // Kill off any old display lists
    Kill();

    // Verify size is a power of 2 for MFIFO
    ASSERT( ((Size-1) & Size) == 0 );

    // allocate space for the mfifo in main memory
    m_MFIFOAddr = (byte*)dlmemalign( 0, Size, Size );
    LOG_MALLOC( m_MFIFOAddr, Size, "c:\\Projects\\A51\\xCore\\Entropy\\Ps2\\ps2_dlist.cpp", 277 );
    m_MFIFOSize = Size;

    // Set up the scratchpad buffers...we will have two 4k buffers
    // that we switch between, but we'll mark the end address at
    // 4k - 1 qword so that we have extra space to store a dummy
    // CNT dma tag. This is to help solve one of the hardware
    // limitations that Sony was so kind to stick us with.
    m_StartAddr[0] = (byte*)0x70000000;
    m_EndAddr[0]   = (byte*)0x70000ff0;
    m_StartAddr[1] = (byte*)0x70001000;
    m_EndAddr[1]   = (byte*)0x70001ff0;
    m_SpadBuffer   = 0;
    m_CurrAddr     = NULL;

    // start out in a disabled state
    m_Disabled   = TRUE;
    m_Active     = FALSE;
    m_InsideTask = FALSE;
    m_SpadLocked = FALSE;

    // reset it so that we're starting off in an "idle" state
    m_CPUState  = CPU_STATE_LOGIC;
    m_GPUState  = GPU_STATE_STOPPED;
    m_CPUTaskID = 0;
    m_GPUTaskID = 0;
    m_nTasks    = 0;
    ASSERT( m_CPUFrame == m_GPUFrame );
    #ifdef ENABLE_DLIST_STATS
    m_StatInfo.nTasksRendered = 0;
    m_StatInfo.StartTask      = 0;
    #endif // ENABLE_DLIST_STATS

    // Install the vsync interrupt handler
    sceGsSyncVCallback( VSyncCallback );
    m_nVSyncsSincePresent = 0;

    // Install the GS interrupt handler (which handles SIGNAL event)
    m_GsHandlerId = AddIntcHandler( INTC_GS, GsInterruptHandler, 0 );
    EnableIntc( INTC_GS );

    // Install the gpu semaphore
    SemaParam Param;
    Param.initCount = 0;
    Param.maxCount  = 2;
    Param.option    = 0;
    m_GPUSema = CreateSema( &Param );
    ASSERT( m_GPUSema != -1 );

    // and assume we haven't timed out...yet...
    m_nVSyncsSincePresent = 0;
    m_TimedOut            = FALSE;
}

//=========================================================================

void dlist::Kill( void )
{
    // free space that the mfifo might've malloced
    if( m_MFIFOAddr != NULL )
    {
        dlfree( 0, m_MFIFOAddr );
        LOG_FREE( m_MFIFOAddr, "c:\\Projects\\A51\\xCore\\Entropy\\Ps2\\ps2_dlist.cpp", 337 );
    }
    m_MFIFOAddr  = NULL;
    m_MFIFOSize  = 0;
    m_Disabled   = TRUE;
    m_Active     = FALSE;
    m_InsideTask = FALSE;
    m_SpadLocked = FALSE;

    // Clear the scratchpad buffers
    m_StartAddr[0] = NULL;
    m_EndAddr[0]   = NULL;
    m_StartAddr[1] = NULL;
    m_EndAddr[1]   = NULL;
    m_SpadBuffer   = 0;
    m_CurrAddr     = NULL;

    // Reset the state data
    m_CPUFrame  = 0,
    m_GPUFrame  = 0,
    m_CPUTaskID = 0,
    m_GPUTaskID = 0,
    m_CPUState  = CPU_STATE_LOGIC,
    m_GPUState  = GPU_STATE_STOPPED,
    m_nTasks    = 0;

    // Turn off all GS interrupts
    *GS_IMR = GS_IMR_SIGMSK_M    |
              GS_IMR_FINISHMSK_M |
              GS_IMR_HSMSK_M     |
              GS_IMR_VSMSK_M     |
              GS_IMR_EDWMSK_M    | (0x3<<13);

    // if we've installed callbacks or interrupt handlers, remove those now
    if( m_GsHandlerId != -1 )
    {
        RemoveIntcHandler( INTC_GS, m_GsHandlerId );
        sceGsSyncVCallback( NULL );
    }
    m_GsHandlerId = -1;

    // and remove the gpu semaphore
    if( m_GPUSema != -1 )
    {
        DeleteSema( m_GPUSema );
    }
    m_GPUSema = -1;
}

//=========================================================================

void dlist::Flush( void )
{
    DLIST_THREAD_SANITY_CHECK();
    ASSERT( m_Active && !m_SpadLocked );
    ASSERT( DLIST.m_CPUState == dlist::CPU_STATE_RENDER );

    // how much data needs to be flushed?
    s32 Size = m_CurrAddr - m_StartAddr[m_SpadBuffer];
    if( Size == 0 )
    {
        // nothing to flush
        return;
    }

    // we need to make sure that the last packet is a continue - there is a hardware
    // limitation that will cause problems if the last tag was a REF, REFE, or REFS
    dmatag* pCNT = (dmatag*)m_CurrAddr;
    pCNT->SetCont( 0 );
    Size       += 16;
    ASSERT( Size <= 4096 );

    // wait for the previous scratchpad dma to complete
    while( *D8_CHCR & (1<<8) )
    {
        // intentionally empty loop
    }

    // Wait for enough space in the MFIFO to flush this scratchpad data into
    // NOTE: I'm not sure why I should have to wait for an extra 16k, but
    // for some reason the display list is not stalling like it should in,
    // all cases, which gives us some nice random dlist crashes. The extra
    // 16k seems to solve the issue.
    // SCAAAARY!!!!
    u32 nBytesAvailable = 0;
    while( nBytesAvailable <= (u32)(Size/*+16*1024*/) )
    {
        // grab the current VIF tag address
        u32 TADR;
        while( (TADR = *D1_TADR) == 0 )
        {
            // intentionally empty loop
        }

        // and grab the fromSPR machine address
        u32 MADR = *D8_MADR;

        // If the tag address is the same as the spad from address, that
        // means the MFIFO is totally caught up to where we are, otherwise
        // we can calculate how much is left, taking into account the
        // circular nature of the MFIFO.
        if( TADR == MADR )
        {
            nBytesAvailable = (u32)m_MFIFOSize;
        }
        else
        {
            nBytesAvailable = (TADR + (u32)m_MFIFOSize - MADR) & ((u32)m_MFIFOSize-0x10);
        }
    }

    // start the new scratchpad transfer
    *D8_SADR = (u32)m_StartAddr[m_SpadBuffer];
    *D8_QWC  = (Size>>4);
    *D8_CHCR = (1<<8);
    asm __volatile__ ("sync.l" );
    asm __volatile__ ("sync.p" );

    // toggle spad buffers
    m_SpadBuffer ^= 1;
    m_CurrAddr = m_StartAddr[m_SpadBuffer];

    // clear out the last alloc pointer for debugging purposes
#if DLIST_USE_SMART_POINTER
    m_pLastAlloc = NULL;
#endif
}

//=========================================================================

void dlist::LockScratchpad( void )
{
    if( m_Active )
    {
        DLIST_THREAD_SANITY_CHECK();

        // Flush out the current scratchpad contents
        Flush();

        // And wait for that last scratchpad dma to complete
        while( *D8_CHCR & (1<<8) )
        {
            // intentionally empty loop
        }

        // back up the MADR address for the fromSPR channel so we can restore
        // it (just in case some yahoo clobbers it for us while messing with
        // scratchpad)
        m_BackupMADR = *D8_MADR;

        // And we are locked now
        m_SpadLocked = TRUE;
    }
}

//=========================================================================

void dlist::UnlockScratchpad( void )
{
    if( m_Active )
    {
        DLIST_THREAD_SANITY_CHECK();

        // Now it is safe to use scratchpad again. So start it up!
        m_SpadBuffer = 0;
        m_CurrAddr   = m_StartAddr[m_SpadBuffer];

        // Reset the MADR address in case some yahoo messed with it while we
        // were locked.
        *D8_MADR = m_BackupMADR;

        // and we're unlocked
        m_SpadLocked = FALSE;
    }
}

//=========================================================================

void dlist::Disable( void )
{
    DLIST_THREAD_SANITY_CHECK();
    ASSERT( !m_Active );
    ASSERT( !m_Disabled );

    m_Disabled = TRUE;

    // Wait until both frames are idle before proceeding
    while( m_GPUFrame < m_CPUFrame )
    {
        // intentionally empty loop
    }

    // wait for the previous scratchpad dma to complete
    while( *D8_CHCR & (1<<8) )
    {
        // intentionally empty loop
    }

    // now halt the VIF1 DMA
    *D1_CHCR = (1<<0) |     // From memory
               (1<<2) |     // Chain mode
               (1<<6);

    // disable the mfifo
    ((tD_CTRL*)D_CTRL)->MFD  = 0;

    // disable the SIGNAL interrupt
    *GS_IMR = GS_IMR_SIGMSK_M    |
              GS_IMR_FINISHMSK_M |
              GS_IMR_HSMSK_M     |
              GS_IMR_VSMSK_M     |
              GS_IMR_EDWMSK_M    | (0x3<<13);
}

//=========================================================================

void dlist::Enable( void )
{
    DLIST_THREAD_SANITY_CHECK();
    ASSERT( !m_Active );
    ASSERT( m_Disabled );
    m_Disabled = FALSE;

    // wait for the VIF from any previous transfers (such as startup
    // screen, exception handler, or whatever)
    while( *D1_CHCR & (1<<8) )
    {
        // intentionally empty loop
    }

    // Initialize the vif1 channel to point at our mfifo
    *D1_TADR = (u32)m_MFIFOAddr;
    *D8_MADR = (u32)m_MFIFOAddr;
    *D8_QWC  = 0;

    // Setup the ring buffer
    *D_RBSR  = m_MFIFOSize - 0x10;  // MFIFO mask
    *D_RBOR  = (u32)m_MFIFOAddr;    // base address of mfifo

    // Reset it such that we are starting off in an "idle" state
    m_CPUState  = CPU_STATE_LOGIC;
    m_GPUState  = GPU_STATE_STOPPED;
    m_CPUTaskID = 0;
    m_GPUTaskID = 0;
    m_nTasks    = 0;
    m_GPUFrame  = m_CPUFrame;
    #ifdef ENABLE_DLIST_STATS
    m_StatInfo.nTasksRendered = 0;
    m_StatInfo.StartTask      = 0;
    s32 i;
    for( i = 0; i < MAX_DLIST_TASKS; i++ )
        m_TaskList[i].Signal = SIGNAL_RESTART;
    #endif // ENABLE_DLIST_STATS

    // Set up the masks for our interrupt handlers
    // (Mask all but the SIGNAL interrupt)
    *GS_IMR = GS_IMR_FINISHMSK_M |
              GS_IMR_HSMSK_M     |
              GS_IMR_VSMSK_M     |
              GS_IMR_EDWMSK_M    | (0x3<<13);

    // Enable the SIGNAL event
    ((tGS_CSR*)GS_CSR)->SIGNAL = 1;

    // turn on the MFIFO using VIF1 as the FIFO drain channel
    ((tD_CTRL*)D_CTRL)->MFD  = 2;

    // start DMA transfer (we may need to set reset the tag address thanks
    // to a possible extra CNT tag at the end of the previous frame)
    u32 StartAddr = *D8_MADR;
    *D8_QWC  = 0;
    *D1_TADR = StartAddr;
    *D1_QWC  = 0;
    *D1_CHCR = (1<<0) |     // From memory
               (1<<2) |     // Chain mode
               (1<<6) |     // Tag transfer enabled
               (1<<8);      // Start the dma
    asm __volatile__ ("sync.l");
    asm __volatile__ ("sync.p");

    // Make sure the semaphores are at a "zero" count initially
    SemaParam SemaParam;
    ReferSemaStatus( m_GPUSema, &SemaParam );
    while( SemaParam.currentCount > 0 )
    {
        PollSema( m_GPUSema );
        ReferSemaStatus( m_GPUSema, &SemaParam );
    }

    // And initialize the semaphore to a positive count. This allows for
    // the cpu to get ahead of the gpu some, but at some point they have
    // to sync up.
    SignalSema( m_GPUSema );
}

//=========================================================================

void dlist::BeginFrame( void )
{
    DLIST_THREAD_SANITY_CHECK();
    ASSERT( !m_Active );
    ASSERT( !m_Disabled );
    ASSERT( !m_InsideTask );
    ASSERT( m_CPUState == CPU_STATE_LOGIC );

    // Flush the cache to make sure anything that was recently loaded or
    // initialized is written out.
    FlushCache( WRITEBACK_DCACHE );

    // Reset the scratchpad pointers
    m_SpadBuffer = 0;
    m_CurrAddr   = m_StartAddr[m_SpadBuffer];

    ///////////////////////////////////////////////////////////////////////
    // CRITICAL SECTION BEGIN
    s32 DIRet = DI();   // disable interrupts

    // if the gpu is currently stopped, then start it up
    if( m_GPUState == GPU_STATE_STOPPED )
    {
        ResetSignal();
        m_GPUState = GPU_STATE_RUNNING;
    }

    // move the CPU from LOGIC to RENDER
    m_CPUState = CPU_STATE_RENDER;
    m_nTasks   = 0;
    MFIFO_FN_LOG( LOG_CPU_BEGIN_RENDER );

    // re-enable interrupts
    if( DIRet )
        EI();
    // END CRITICAL SECTION
    ///////////////////////////////////////////////////////////////////////

    // Now we are active
    m_Active = TRUE;
}

//=========================================================================

void dlist::EndFrame( void )
{
    s32 i;

    DLIST_THREAD_SANITY_CHECK();
    ASSERT( m_Active );
    ASSERT( !m_Disabled );
    ASSERT( !m_InsideTask );
    ASSERT( m_CPUState == CPU_STATE_RENDER );

    // Let the task queue know about our "end frame" task
    xtick CurrTime = x_GetTime();
    x_strcpy( m_TaskList[m_CPUTaskID].TaskName, "End Frame" );
    m_TaskList[m_CPUTaskID].CPUTime      = CurrTime;
    m_TaskList[m_CPUTaskID].Signal       = SIGNAL_END_FRAME;
    m_TaskList[m_CPUTaskID].Frame        = m_CPUFrame;
    m_TaskList[m_CPUTaskID].Task         = 0x7ffffffe;
    m_TaskList[m_CPUTaskID].DMAStartAddr = 0;
    m_CPUTaskID = (m_CPUTaskID+1) % MAX_DLIST_TASKS;

    // Add a SIGNAL to mark the frame as being completed...
    DLPtrAlloc( pPacket1, signal_packet );
    pPacket1->DMA.SetCont( sizeof(signal_packet) - sizeof(dmatag) );
    pPacket1->DMA.PAD[0] = SCE_VIF1_SET_FLUSHA(0);
    pPacket1->DMA.PAD[1] = SCE_VIF1_SET_DIRECT( 3, 0 );
    pPacket1->GIF.BuildRegLoad( 2, TRUE );
    pPacket1->Texflush     = 0;
    pPacket1->TexflushAddr = SCE_GS_TEXFLUSH;
    pPacket1->Signal       = SCE_GS_SET_SIGNAL( 0x7ffffffe, 0xffffffff );
    pPacket1->SignalAddr   = SCE_GS_SIGNAL;
    for( i = 0; i < 16; i++ )
        pPacket1->Pad[i] = 0;

    // And let the task queue know about our "restart" task...
    // This will allow the GPU to carry on after we get the vsync interrupt
    x_strcpy( m_TaskList[m_CPUTaskID].TaskName, "Restart" );
    m_TaskList[m_CPUTaskID].CPUTime      = CurrTime;
    m_TaskList[m_CPUTaskID].Signal       = SIGNAL_RESTART;
    m_TaskList[m_CPUTaskID].Frame        = m_CPUFrame;
    m_TaskList[m_CPUTaskID].Task         = 0x7fffffff;
    m_TaskList[m_CPUTaskID].DMAStartAddr = 0;
    m_CPUTaskID = (m_CPUTaskID+1) % MAX_DLIST_TASKS;

    // And then add a SIGNAL to stop until vsync (Note that the GS does not
    // stop rendering when it gets one signal, but it does stop rendering if
    // if it gets TWO signals without the signal interrupt being enabled.
    DLPtrAlloc( pPacket2, signal_packet );
    pPacket2->DMA.SetCont( sizeof(signal_packet) - sizeof(dmatag) );
    pPacket2->DMA.PAD[0] = SCE_VIF1_SET_FLUSHA(0);
    pPacket2->DMA.PAD[1] = SCE_VIF1_SET_DIRECT( 3, 0 );
    pPacket2->GIF.BuildRegLoad( 2, TRUE );
    pPacket2->Texflush     = 0;
    pPacket2->TexflushAddr = SCE_GS_TEXFLUSH;
    pPacket2->Signal       = SCE_GS_SET_SIGNAL( 0x7fffffff, 0xffffffff );
    pPacket2->SignalAddr   = SCE_GS_SIGNAL;
    for( i = 0; i < 16; i++ )
        pPacket2->Pad[i] = 0;

    // Flush this dlist
    Flush();

    // wait for scratchpad to be fully empty
    while( *D8_CHCR & (1<<8) )
    {
        // intentionally empty loop
    }

    // We're no longer actively adding to the dlist
    m_Active = FALSE;

    // Sanity check
    #ifdef X_ASSERT
    SemaParam SemaParam;
    ReferSemaStatus( m_GPUSema, &SemaParam );
    ASSERT( SemaParam.currentCount <= 2 );
    #endif

    // now block until it is safe to go into a logic state (this should
    // make sure we don't clobber memory that the GPU might be reading
    // from
    #ifdef ENABLE_DLIST_STATS
    xtimer VBlankTimer;
    VBlankTimer.Reset();
    VBlankTimer.Start();
    #endif
    m_TimeOutEnabled = TRUE;
    WaitSema( m_GPUSema );
    m_TimeOutEnabled = FALSE;
    #ifdef ENABLE_DLIST_STATS
    m_VBlankTime = VBlankTimer.Stop();
    #endif
    if( m_TimedOut )
    {
        DLIST.HandleMFIFOTimeout();
        BREAK;
    }

    // debugging via vram-snap
    #ifdef dstewart
    static xbool StopForVramSnap = FALSE;
    if( StopForVramSnap )
    {
        Disable();
        BREAK;
        Enable();
    }
    #endif

    // Move the cpu to the next frame and put it into a LOGIC state
    m_CPUFrame++;
    m_CPUState = CPU_STATE_LOGIC;
    MFIFO_FN_LOG( LOG_CPU_BEGIN_LOGIC );
}

//=========================================================================

void dlist::BeginTask( const char* pTaskName )
{
    DLIST_THREAD_SANITY_CHECK();
    ASSERT( m_Active );
    ASSERT( !m_InsideTask );
    ASSERT( !m_Disabled );
    m_InsideTask = TRUE;

    // create a new task in the queue
    xtick CurrTime = x_GetTime();
    x_strncpy( m_TaskList[m_CPUTaskID].TaskName, pTaskName, 31 );
    m_TaskList[m_CPUTaskID].TaskName[31] = '\0';
    m_TaskList[m_CPUTaskID].CPUTime      = CurrTime;
    m_TaskList[m_CPUTaskID].Signal       = SIGNAL_TASK;
    m_TaskList[m_CPUTaskID].Frame        = m_CPUFrame;
    m_TaskList[m_CPUTaskID].Task         = m_nTasks;
    m_TaskList[m_CPUTaskID].DMAStartAddr = *D8_MADR;
    m_CPUTaskID = (m_CPUTaskID+1) % MAX_DLIST_TASKS;

    // fill in the MARK register for debugging purposes
    DLPtrAlloc( pDMA, dmatag );
    pDMA->SetCont( 0 );
    pDMA->PAD[0] = SCE_VIF1_SET_MARK( m_nTasks, 0 );

    // we now have one more task in the queue
    m_nTasks++;
    ASSERT( m_nTasks <= MAX_TASKS_PER_FRAME );
}

//=========================================================================

void dlist::EndTask( void )
{
    DLIST_THREAD_SANITY_CHECK();
    ASSERT( m_Active );
    ASSERT( m_InsideTask );
    ASSERT( !m_Disabled );

    // add the SIGNAL to show that its the end of this task
    s32 i;
    DLPtrAlloc( pPacket, signal_packet );
    pPacket->DMA.SetCont( sizeof(signal_packet) - sizeof(dmatag) );
    pPacket->DMA.PAD[0] = SCE_VIF1_SET_FLUSHA(0);
    pPacket->DMA.PAD[1] = SCE_VIF1_SET_DIRECT( 3, 0 );
    pPacket->GIF.BuildRegLoad( 2, TRUE );
    pPacket->Texflush     = 0;
    pPacket->TexflushAddr = SCE_GS_TEXFLUSH;
    pPacket->Signal       = SCE_GS_SET_SIGNAL( (m_nTasks-1), 0xffffffff );
    pPacket->SignalAddr   = SCE_GS_SIGNAL;
    for( i = 0; i < 16; i++ )
        pPacket->Pad[i] = 0;

    // Flush this data to make sure the gpu knows what it's getting into
    // before a new task is started.
    Flush();
    while( *D8_CHCR & (1<<8) )
    {
        // intentionally empty loop
    }

    m_InsideTask = FALSE;
}

//=========================================================================

void dlist::WaitForTasks( void )
{
    DLIST_THREAD_SANITY_CHECK();

    // wait for the gpu to be caught up to the cpu
    while( m_GPUFrame < m_CPUFrame )
    {
        x_DelayThread( 1 );
    }

    // wait for scratchpad to be idle
    while( *D8_CHCR & (1<<8) )
    {
        // intentionally empty loop
    }

    // wait for the VIF to finish
    while( *D1_TADR != *D8_MADR )
    {
        // intentionally empty loop
    }

    /*
    // start timing for the vblank
    xtimer VBlankTimer;
    VBlankTimer.Reset();
    VBlankTimer.Start();

    // Wait until both frames are idle before proceeding
    while( (m_Frames[m_CurrFrame].State   != STATE_IDLE) ||
    (m_Frames[m_CurrFrame^1].State != STATE_IDLE) )
    {
    // allow other threads to use this down-time
    x_DelayThread( 1 );

    // If we're taking too long, then there was some error that has
    // occured. Handle that error...
    if( VBlankTimer.ReadMs() > 1000.0f )
    {
    HandleMFIFOTimeout();
    }

    #ifdef ENABLE_DLIST_STATS
    m_VBlankTime = VBlankTimer.Stop();
    #endif
    */
}

//=========================================================================

#ifdef ENABLE_DLIST_STATS
s32 dlist::GetPrevNTasks( void ) const
{
    return m_StatInfo.nTasksRendered;
}
#endif // ENABLE_DLIST_STATS

//=========================================================================

#ifdef ENABLE_DLIST_STATS
xtick dlist::GetPrevTaskTime( s32 Task ) const
{
    const task* pTask = &m_TaskList[(m_StatInfo.StartTask+Task)%MAX_DLIST_TASKS];
    const task* pPrev = &m_TaskList[(m_StatInfo.StartTask+Task+MAX_DLIST_TASKS-1)%MAX_DLIST_TASKS];
    return (pTask->GPUTime - pPrev->GPUTime);
}
#endif // ENABLE_DLIST_STATS

//=========================================================================

#ifdef ENABLE_DLIST_STATS
const char* dlist::GetPrevTaskName( s32 Task ) const
{
    const task* pTask = &m_TaskList[(m_StatInfo.StartTask+Task)%MAX_DLIST_TASKS];
    return pTask->TaskName;
}
#endif // ENABLE_DLIST_STATS

//=========================================================================

void dlist::ResetSignal( void )
{
    // re-enable the SIGNAL so that the next frame can begin processing
    *GS_IMR = GS_IMR_SIGMSK_M    |
              GS_IMR_FINISHMSK_M |
              GS_IMR_HSMSK_M     |
              GS_IMR_VSMSK_M     |
              GS_IMR_EDWMSK_M    | (0x3<<13);
    ((tGS_CSR*)GS_CSR)->SIGNAL = 1;
    *GS_IMR = //GS_IMR_SIGMSK_M    |
              GS_IMR_FINISHMSK_M |
              GS_IMR_HSMSK_M     |
              GS_IMR_VSMSK_M     |
              GS_IMR_EDWMSK_M    | (0x3<<13);
}

//=========================================================================

void dlist::HandleMFIFOTimeout( void )
{
    #ifdef ENABLE_MFIFO_LOG
    DumpMFIFOLog();
    #endif

    /*
    // figure out which frame and task was running
    s32 Frame = 0;
    if( m_Frames[0].State == dlist::STATE_RUNNING )
    {
    Frame = 0;
    }
    else if( m_Frames[1].State == dlist::STATE_RUNNING )
    {
    Frame = 1;
    }
    else
    {
    ASSERT( FALSE );
    }

    s32 Task = m_Frames[Frame].CurrTask;
    (void)Task;

    // Tell the TTY which task timed out
    #ifdef ENABLE_DLIST_STATS
    scePrintf( "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
    scePrintf( "MFIFO has timed out!\n" );
    scePrintf( "Graphics task <%s> (%1d/%1d) did not return!\n",
    m_Frames[Frame].Tasks[Task],
    Task,
    m_Frames[Frame].NTasks );
    scePrintf( "Attempting to continue...\n" );
    #endif

    // If on a devkit, do a break so we can debug it
    #ifdef TARGET_DEV
    BREAK;
    #endif

    // If on a viewer build, do a bluescreen so that programmers
    // can be informed of what was going on.
    #if defined( CONFIG_VIEWER ) || defined( CONFIG_QA )
    extern void x_DebugSetCause( const char* pCause );
    x_DebugSetCause( xfs( "MFIFO timeout (%s)", m_Frames[Frame].Tasks[Task].TaskName ) );
    *(u32*)1 = 0;
    #endif

    // And on all builds, attempt to reset the hardware and continue
    // after debugging (or just attempt to let the problem resolve itself
    // on a retail build).
    extern void ps2_ResetHardware(void);
    ps2_ResetHardware();
    DLIST.Enable();
    x_DelayThread(10);
    */
}

//=========================================================================

#ifdef ENABLE_MFIFO_LOG
void dlist::AddToLog( log_action Action )
{
    // just copy out the info
    m_MFIFOLog[m_MFIFOLogIndex].Action   = Action;
    m_MFIFOLog[m_MFIFOLogIndex].CPUFrame = m_CPUFrame;
    m_MFIFOLog[m_MFIFOLogIndex].GPUFrame = m_GPUFrame;
    m_MFIFOLog[m_MFIFOLogIndex].CPUTask  = m_CPUTaskID;
    m_MFIFOLog[m_MFIFOLogIndex].GPUTask  = m_GPUTaskID;
    m_MFIFOLog[m_MFIFOLogIndex].CPUState = m_CPUState;
    m_MFIFOLog[m_MFIFOLogIndex].GPUState = m_GPUState;
    m_MFIFOLog[m_MFIFOLogIndex].Time     = x_GetTime();
    m_MFIFOLogIndex++;
    if( m_MFIFOLogIndex == MAX_LOG_ENTRIES )
        m_MFIFOLogIndex = 0;
}
#endif // ENABLE_MFIFO_LOG

//=========================================================================

#ifdef ENABLE_MFIFO_LOG
void dlist::DumpMFIFOLog( void )
{
    s32 i;

    // open up a file to dump into
    X_FILE* fh = x_fopen( "C:\\DListLog.txt", "wb" );
    ASSERT( fh );

    // go through all the logs up to the index and dump them out
    for( i = 0; i < m_MFIFOLogIndex; i++ )
    {
        // dump out the action
        switch( m_MFIFOLog[i].Action )
        {
        case LOG_CPU_BEGIN_LOGIC:       x_fprintf( fh, "CPU_BEGIN_LOGIC, " );   break;
        case LOG_CPU_BEGIN_RENDER:      x_fprintf( fh, "CPU_BEGIN_RENDER, " );  break;
        case LOG_VSYNC_TIMED_OUT:       x_fprintf( fh, "VSYNC_TIMED_OUT, " );   break;
        case LOG_VSYNC_FRAME_PRESENTED: x_fprintf( fh, "VSYNC_PRESENTED, " );   break;
        case LOG_GS_TASK_FINISHED:      x_fprintf( fh, "GS_TASK_FINISHED, " );  break;
        case LOG_GS_FRAME_FINISHED:     x_fprintf( fh, "GS_FRAME_FINISHED, " ); break;
        case LOG_GS_RESTART:            x_fprintf( fh, "GS_RESTARTED, " );      break;
        }

        // dump out the CPU data
        x_fprintf( fh, "%d, ", m_MFIFOLog[i].CPUFrame );
        x_fprintf( fh, "%d, ", m_MFIFOLog[i].CPUTask );
        switch( m_MFIFOLog[i].CPUState )
        {
        case CPU_STATE_LOGIC:       x_fprintf( fh, "CPU_STATE_LOGIC, " );   break;
        case CPU_STATE_RENDER:      x_fprintf( fh, "CPU_STATE_RENDER, " );  break;
        }

        // dump out the GPU data
        x_fprintf( fh, "%d, ", m_MFIFOLog[i].GPUFrame );
        x_fprintf( fh, "%d, ", m_MFIFOLog[i].GPUTask );
        switch( m_MFIFOLog[i].GPUState )
        {
        case GPU_STATE_STOPPED:     x_fprintf( fh, "GPU_STATE_STOPPED, " );        break;
        case GPU_STATE_RUNNING:     x_fprintf( fh, "GPU_STATE_RUNNING, " );      break;
        case GPU_STATE_WAIT_VSYNC:  x_fprintf( fh, "GPU_STATE_WAIT_VSYNC, " );  break;
        case GPU_STATE_PRESENTED:   x_fprintf( fh, "GPU_STATE_PRESENTED, " );
        }

        // and finally, dump out the time stamp
        x_fprintf( fh, "%12.3f\n", x_TicksToMs( m_MFIFOLog[i].Time ) );
    }

    x_fclose( fh );
}
#endif // ENABLE_MFIFO_LOG

//=========================================================================

#ifdef ENABLE_DLIST_STATS
void dlist::GatherMFIFOStats( void )
{
    ASSERT( m_TaskList[m_GPUTaskID].Signal == SIGNAL_RESTART );
    
    // back up two tasks...this should be the last task rendered
    s32 Task = (m_GPUTaskID + MAX_DLIST_TASKS - 2) % MAX_DLIST_TASKS;

    // now find the prev restart...everything else should've been tasks
    m_StatInfo.nTasksRendered = 0;
    while( m_TaskList[Task].Signal != SIGNAL_RESTART )
    {
        ASSERT( m_TaskList[Task].Signal == SIGNAL_TASK );
        m_StatInfo.nTasksRendered++;
        Task = (Task + MAX_DLIST_TASKS - 1) % MAX_DLIST_TASKS;
    }

    // now the "first" task is the one right after the restart
    m_StatInfo.StartTask = (Task + 1) % MAX_DLIST_TASKS;
}
#endif // ENABLE_DLIST_STATS

//=========================================================================
