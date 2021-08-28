#include "audio_ram_mgr.hpp"
#include "x_threads.hpp"

#ifdef TARGET_GCN
#include <dolphin.h>
#endif

// Status bit-field defines.

#define USED_BIT                (1<<31)
#define HEAP_NUMBER_BITS        ((1<<30) + (1<<29))
#define HEAP_NUMBER_SHIFT       (29)
#define USER_SIZE_MASK          (~(USED_BIT+HEAP_NUMBER_BITS))

// Mcb macros.

#define GET_IS_USED( mcb )      ((mcb->status & USED_BIT) != 0)
#define GET_IS_FREE( mcb )      ((mcb->status & USED_BIT) == 0 )
#define GET_HEAP_NUMBER( mcb )  ((mcb->status & HEAP_NUMBER_BITS) >> HEAP_NUMBER_SHIFT)
#define GET_USER_SIZE( mcb )    (mcb->status & USER_SIZE_MASK)

// 16 megabytes (2^24)
#define MAX_BUCKETS             (24)           

//------------------------------------------------------------------------------

struct double_link
{
        struct mcb*         pPrev;
        struct mcb*         pNext;
};                          
                            
struct quad_link            
{                           
        double_link         Global;
        double_link         Local;
};                          
                            
struct mcb                  
{                           
        quad_link                   Link;         // 16 bytes
        void*                       pAddress;     // 4 bytes
        s32                         Size;         // 4 bytes
        u32                         Status;       // 4 bytes - Top 3 bits are status, bottom 24 bits are user size.
        audio_ram_mgr::reference*   pReference;   // 4 bytes - Pointer to reference for this memory. 
};

struct heap
{
        void*                               m_pHeapBottom;
        void*                               m_pHeapTop;
        s32                                 m_AlignToPower;
        s32                                 m_AlignUpMask;
        s32                                 m_AlignUpDelta;
        s32                                 m_HeapIndex;
        s32                                 m_HeapIndexBits;
        s32                                 m_NumMcbs;
        audio_ram_mgr::allocation_method    m_AllocationMethod;
        quad_link                           m_pGlobalList;
        quad_link                           m_pFreeList[ MAX_BUCKETS ];
        quad_link                           m_pUsedList[ MAX_BUCKETS ];
        quad_link                           m_pFreeMcbs;
        void*                               m_pMcbBuffer;
};

//------------------------------------------------------------------------------

static xbool             s_Initialized      = FALSE;
static xbool             s_DispatcherActive = FALSE;
static xthread*          s_pThread          = 0;
static s32               s_Sequence         = 0;
static s32               s_RequestCount     = 0;
static audio_io_request* s_CurrentRequest   = NULL;
static audio_io_request  s_RequestQueue;
static xmesgq            s_DispatcherMQ(1);
static xmesgq            s_QueueSemaphore(1);
// TODO: Implement handle based heap manager.
//static heap              s_Heaps[ audio_ram_mgr::NUM_HEAPS ];

// GCN Specific variables.
#ifdef TARGET_GCN
static ARQRequest s_ARQ_Request;
#endif

//------------------------------------------------------------------------------

audio_ram_mgr g_AudioRamMgr;

//------------------------------------------------------------------------------

void AudioRequestDispatcher( void );
void AudioServiceQueue( void );
void AudioServiceCurrentRequest( void );

//------------------------------------------------------------------------------

#ifdef TARGET_GCN
static void gcn_AudioIoRequestCallback( u32 p )
{
    s32         MessageStatus;
    ARQRequest* pRequest = (ARQRequest*)p;

    // Nuke warning.
    (void)pRequest;
    
    // Wake up the dispatcher.
    MessageStatus = s_DispatcherMQ.Send( (void*)NULL, MQ_NOBLOCK );
    ASSERT( MessageStatus );
}
#endif

//------------------------------------------------------------------------------

void AudioRequestDispatcher( void )
{
    // Error check.
    ASSERT( !s_DispatcherActive );

    // Set flag
    s_DispatcherActive = TRUE;

    while( 1 )
    {
        // Wait on a dispatcher message.
        s_DispatcherMQ.Recv( MQ_BLOCK );
        
        // Aquire queue semaphore.
        s_QueueSemaphore.Recv( MQ_BLOCK );

        // Service the current audio_io_request.
        AudioServiceCurrentRequest();

        // Service the audio_io_request queue.
        AudioServiceQueue();
        
        // Release queue semaphore.
        s_QueueSemaphore.Send( NULL, MQ_BLOCK );
    }
}

//------------------------------------------------------------------------------

void AudioServiceCurrentRequest( void )
{
    audio_io_request* pRequest;

    // Get the queues current request.
    pRequest = s_CurrentRequest;

    // No longer has a current request!
    s_CurrentRequest = NULL;

    // Only if current request is valid...
    if( pRequest )
    {
        audio_io_request* pPrev;
        audio_io_request* pNext;

        // get links
        pPrev = pRequest->m_pPrev;
        pNext = pRequest->m_pNext;

        // Remove request from list.
        pPrev->m_pNext = pNext;
        pNext->m_pPrev = pPrev;

        // Unlink it.
        pRequest->m_pPrev = NULL;
        pRequest->m_pNext = NULL;

        // Its completed now!
        pRequest->m_Status = audio_io_request::COMPLETED;

        // Adjust request count.
        s_RequestCount--;
        ASSERT( s_RequestCount >= 0 );
    
        // User callback requested?
        if( pRequest->m_pCallback )
        {
            // Make the user callback.
            (*pRequest->m_pCallback)( pRequest ); 
        }
    }
}

//------------------------------------------------------------------------------

void AudioServiceQueue( void )
{
    audio_io_request* pRequest;

    // Get first request.
    pRequest = s_RequestQueue.m_pNext;
    ASSERT( pRequest );

    // Check for empty queue.
    if( pRequest != &s_RequestQueue )
    {
        s32 Length;
        u32 ARAM;
        u32 MRAM;

        // Error check request.
        ASSERT( (pRequest->m_Status == audio_io_request::PENDING) || (pRequest->m_Status == audio_io_request::IN_PROGRESS) );

        // First time?
        if( pRequest->m_Status == audio_io_request::PENDING )
        {
            // Mark it as in progress.
            pRequest->m_Status = audio_io_request::IN_PROGRESS;
        }

        // Set current request
        s_CurrentRequest = pRequest;

        //================================
        // Begin hardware specific stuff
        //================================

        Length = pRequest->m_Length;
        ASSERT( (Length & 31) == 0 );

        // TODO: Put in handle-based stuff for ARAM.

        // Read based on device type.
        switch( pRequest->m_Type )
        {
            // Copy from main ram to audio ram?
            case audio_io_request::UPLOAD:
            {
                ARAM = (u32)pRequest->m_Destination;
                MRAM = (u32)pRequest->m_Source;
                ASSERT( (MRAM & 3) == 0 );

#ifdef TARGET_GCN
                // Force d-cache writeback.
                DCStoreRange( (void *)MRAM, Length );

                // Make the request.
                ARQPostRequest( &s_ARQ_Request, 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH, (u32)MRAM, ARAM, Length, gcn_AudioIoRequestCallback );
#endif
                break;
            }

            // Copy from audio ram to main ram?
            case audio_io_request::DOWNLOAD:
            {
                ARAM = (u32)pRequest->m_Source;
                MRAM = (u32)pRequest->m_Destination;
                ASSERT( (MRAM & 3) == 0 );

#ifdef TARGET_GCN
                // Invalidate the d-cache.
                DCInvalidateRange( (void*)MRAM, Length );
   
                // Make the request.
                ARQPostRequest( &s_ARQ_Request, 0, ARQ_TYPE_ARAM_TO_MRAM, ARQ_PRIORITY_HIGH, ARAM, (u32)MRAM, Length, gcn_AudioIoRequestCallback );
#endif
                break;
            }

            // Copy from audio ram to audio ram?
            case audio_io_request::COPY:
            {
                // TODO: Put in copy support.
                ASSERT( 0 );
            }

            default:
            {
                ASSERT( 0 );
            }
        }

        //================================
        // End hardware specific stuff
        //================================
    }
}

//------------------------------------------------------------------------------

audio_ram_mgr::audio_ram_mgr( void )
{
}

//------------------------------------------------------------------------------

audio_ram_mgr::~audio_ram_mgr( void )
{
}

//------------------------------------------------------------------------------

void audio_ram_mgr::Init( void )
{
    // Error check.
    ASSERT( !s_Initialized );

    // Create the dispatcher thread.
    s_pThread = new xthread( AudioRequestDispatcher, "audio dispatcher", 8192, 3 );

    // Release the queue semaphore.
    s_QueueSemaphore.Send( (void*)NULL, MQ_NOBLOCK );
    
    // Empty the request queue.
    s_RequestQueue.m_pPrev = &s_RequestQueue;
    s_RequestQueue.m_pNext = &s_RequestQueue;

    // Head/Tail is now the LOWEST priority!
    s_RequestQueue.m_Priority = audio_io_request::NUM_PRIORITIES;

    // Its intialized now.
    s_Initialized = TRUE;
}

//------------------------------------------------------------------------------

void audio_ram_mgr::Kill( void )
{
    // Error check.
    ASSERT( s_Initialized );

    // Destroy the thread.
    delete s_pThread;

    // No longer initialized.
    s_Initialized = FALSE;
}

//------------------------------------------------------------------------------

audio_ram_mgr::ram_handle audio_ram_mgr::NewHandle( heap_type HeapType, s32 Count )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)HeapType;
    (void)Count;

    return( (ram_handle)0 );
}

//------------------------------------------------------------------------------

void audio_ram_mgr::FreeHandle( ram_handle Handle )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)Handle;
}

//------------------------------------------------------------------------------

void* audio_ram_mgr::GetAddress( ram_handle Handle )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)Handle;

    return( NULL );
}

//------------------------------------------------------------------------------

audio_ram_mgr::heap_type audio_ram_mgr::GetRamType( ram_handle Handle )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)Handle;

    return( (heap_type)0);
}

//------------------------------------------------------------------------------

s32 audio_ram_mgr::GetBlockSize( ram_handle Handle )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)Handle;

    return( 0 );
}

//------------------------------------------------------------------------------

s32 audio_ram_mgr::GetUserSize( ram_handle Handle )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)Handle;

    return( 0 );
}

//------------------------------------------------------------------------------

audio_ram_mgr::reference* audio_ram_mgr::GetReference( ram_handle Handle )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)Handle;

    return( NULL );
}

//------------------------------------------------------------------------------

void audio_ram_mgr::SetReference( ram_handle Handle, reference* pReference )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)Handle;
    (void)pReference;
}

//------------------------------------------------------------------------------

s32 audio_ram_mgr::GetFreeMemory( heap_type HeapType )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)HeapType;

    return( 0 );
}

//------------------------------------------------------------------------------

s32 audio_ram_mgr::GetUsedMemory( heap_type HeapType )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)HeapType;

    return( 0 );
}

//------------------------------------------------------------------------------

s32 audio_ram_mgr::GetTotalMemory( heap_type HeapType )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)HeapType;

    return( 0 );
}

//------------------------------------------------------------------------------

s32 audio_ram_mgr::GetBigBlock( heap_type HeapType )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)HeapType;

    return( 0 );
}

//------------------------------------------------------------------------------

audio_ram_mgr::allocation_method audio_ram_mgr::GetAllocationMethod( heap_type HeapType )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)HeapType;

    return( (allocation_method)0 );
}

//------------------------------------------------------------------------------

s32 audio_ram_mgr::SetAllocationMethod( heap_type HeapType, allocation_method Method )
{
    // Error check.
    ASSERT( s_Initialized );

    (void)HeapType;
    (void)Method;

    return( 0 );
}

//------------------------------------------------------------------------------

void audio_ram_mgr::QueueRequest( audio_io_request* pRequest ) 
{
    audio_io_request* pCurr;

    // Error check.
    ASSERT( s_Initialized );

    // Error check request's "data".
    ASSERT( pRequest );
    ASSERT( pRequest->m_Priority >= audio_io_request::HIGH_PRIORITY );
    ASSERT( pRequest->m_Priority <= audio_io_request::LOW_PRIORITY );
    ASSERT( pRequest->m_Source );
    ASSERT( pRequest->m_Destination );
    ASSERT( pRequest->m_Length >= 0 );

    // Aquire queue semaphore
    s_QueueSemaphore.Recv( MQ_BLOCK );

    // Status is pending now.
    pRequest->m_Status = audio_io_request::PENDING;

    // Set sequence.
    pRequest->m_Sequence = s_Sequence++;

    // Find the insertion point.
    pCurr = s_RequestQueue.m_pNext;
    while( pRequest->m_Priority >= pCurr->m_Priority )
        pCurr = pCurr->m_pNext;

    // Insert it into the list.
    pRequest->m_pNext       = pCurr;
    pRequest->m_pPrev       = pCurr->m_pPrev;
    pCurr->m_pPrev->m_pNext = pRequest;
    pCurr->m_pPrev          = pRequest;

    // Bump the request count.
    s_RequestCount++;

    // Check for single entry in queue
    if( s_RequestCount == 1 )
    {
        // Wake up the dispatcher
        s_DispatcherMQ.Send( (void*)NULL, MQ_BLOCK );
    }

    // Release queue semaphore
    s_QueueSemaphore.Send( NULL, MQ_BLOCK );
}

//------------------------------------------------------------------------------

void audio_ram_mgr::CancelRequest( audio_io_request* pRequest )
{
    // Error check.
    ASSERT( s_Initialized );

    // TODO: Implement canceling an audio io request (if needed).

    (void)pRequest;
}
