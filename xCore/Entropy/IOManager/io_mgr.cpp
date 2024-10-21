#ifdef TARGET_GCN // TODO: RMB - Remove this - need pc version
#include <dolphin.h>
#endif
#include "x_types.hpp"
#include "x_memory.hpp"
#include "x_threads.hpp"
#include "x_array.hpp"
#include "x_string.hpp"
#include "x_time.hpp"
#include "x_context.hpp"
#include "io_mgr.hpp"
#include "io_filesystem.hpp"
#include "device_dvd\io_device_dvd.hpp"
#include "device_net\io_device_net.hpp"
#if defined(TARGET_XBOX)
#include "Implementation/x_log_private.hpp"
#endif

//==============================================================================
// Local variables

static xbool s_Initialized      = FALSE;
static xbool s_DispatcherActive = FALSE;

//==============================================================================
// Global variables

io_mgr g_IoMgr;

//==============================================================================

void io_dispatcher( void )
{
    // Error check.
    ASSERT( s_Initialized );    
    ASSERT( !s_DispatcherActive );

    // Set semaphore
    s_DispatcherActive = TRUE;

    while( x_GetCurrentThread()->IsActive() )
    {
        io_device_file* pFile           = NULL;
        io_request*     pCurr           = NULL;
        io_request*     pRequest        = NULL;
        io_device*      pDevice         = NULL;
        xbool           bServiceQueue   = TRUE;
        xbool           bServiceRequest = TRUE;       

        // Wait on a request.
        pRequest = (io_request*)GET_DISPATCHER_MQ().Recv( MQ_BLOCK );
        if( x_GetCurrentThread()->IsActive()==FALSE )
        {
            ASSERT( pRequest==NULL );
            break;
        }

        // Error check.
        ASSERT( pRequest );
        ASSERT( pRequest->m_pOpenFile );
        ASSERT( pRequest->m_pOpenFile->pDeviceFile );
        ASSERT( pRequest->m_pOpenFile->pDeviceFile->pDevice );
        
        // Run-time protection if the file gets closed underneath the IO manager...
        if( pRequest && pRequest->m_pOpenFile && pRequest->m_pOpenFile->pDeviceFile && pRequest->m_pOpenFile->pDeviceFile->pDevice )
        {
            pFile   = pRequest->m_pOpenFile->pDeviceFile;
            pDevice = pRequest->m_pOpenFile->pDeviceFile->pDevice;
            
            // Error check
            ASSERT( VALID_DEVICE_FILE( pFile ) );
            
            // Aquire device semaphore.
            pDevice->m_Semaphore.Recv( MQ_BLOCK );

            // Need to be queued?
            if( pRequest->m_Status == io_request::QUEUED )
            {
                // Status is pending now.
                pRequest->m_Status = io_request::PENDING;

                // Bump the files reference count
                pFile->ReferenceCount++;

                // Now don't need to service the request
                bServiceRequest = FALSE;

                // Better not be in a list...
                ASSERT( pRequest->m_pNext == NULL );
                ASSERT( pRequest->m_pPrev == NULL );

                // Set sequence.
                pRequest->m_Sequence = pDevice->m_Sequence++;

                // Find the insertion point.
                pCurr = pDevice->m_RequestQueue.m_pNext;
                while( pRequest->m_Priority >= pCurr->m_Priority )
                    pCurr = pCurr->m_pNext;

                // Insert it into the list.
                pRequest->m_pNext       = pCurr;
                pRequest->m_pPrev       = pCurr->m_pPrev;
                pCurr->m_pPrev->m_pNext = pRequest;
                pCurr->m_pPrev          = pRequest;

                // Bump the request count.
                pDevice->m_RequestCount++;

                // Check for single entry in queue
                if( pDevice->m_RequestCount != 1 )
                {
                    bServiceQueue = FALSE;
                }
            }

            // Service the devices current io_request.
            if( bServiceRequest )
                pDevice->ServiceDeviceCurrentRequest();

            // Service the devices io_request queue.
            if( bServiceQueue )
                pDevice->ServiceDeviceQueue();
        
            // Release device semaphore.
            pDevice->m_Semaphore.Send( NULL, MQ_BLOCK );
        }
    }
}

//==============================================================================

io_mgr::io_mgr( void ) : m_DispatcherMQ(MAX_DEFAULT_MESSAGES)
{
}

//==============================================================================

io_mgr::~io_mgr( void )
{
}

//==============================================================================

s32 io_mgr::Init( void )
{
    // Error check.
    ASSERT( s_Initialized == FALSE );

    // Initialise some important xbox stuff
    //   (a hook is needed before DeviceCDROMOpen to launch a thread)
    // Set up the dvd
#if defined(ENABLE_NETFS)
    m_Devices[ IO_DEVICE_DVD ] = &g_IODeviceNET;
#else
    m_Devices[ IO_DEVICE_DVD ] = &g_IODeviceDVD;
#endif
    m_Devices[ IO_DEVICE_DVD ]->Init();

    // It's inited.
    s_Initialized = TRUE;

    // Create the io_mgr thread.
    m_pThread = new xthread( io_dispatcher, "io_mgr dispatcher", 8192, 3 );

    // Initialize file system
    g_IOFSMgr.Init();
#if defined(ENABLE_NETFS) && defined(TARGET_XBOX) && defined(X_LOGGING)
    g_LogControl.Enable = TRUE;
#endif

    // It's all good!
    return TRUE;
}

//==============================================================================

s32 io_mgr::Kill( void )
{
    s32 i;

    // Error check.
    ASSERT( s_Initialized );

    // Destroy the io_mgr thread.
    delete m_pThread;

    // Shut down the file system
    g_IOFSMgr.Kill();

    // For each device...
    for( i=0 ; i<NUM_IO_DEVICES ; i++ )
    {
        // Kill each device.
        m_Devices[ i ]->Kill();
    }

    // Clear flag
    s_Initialized = FALSE;
    s_DispatcherActive = FALSE;

    // ok its all good.
    // Destroy some xbox stuff
    //   (a hook is needed before DeviceCDROMClose to kill a thread)
    return 1;
}

//==============================================================================

s32 io_mgr::QueueRequest( io_request* pRequest )
{
    io_device_file* pFile;
    io_device*      pDevice;

    // Error check.
    ASSERT( s_Initialized );

    // Error check request's "data".
    ASSERT( pRequest );

    ASSERT( pRequest->m_Priority >= io_request::HIGH_PRIORITY );
    ASSERT( pRequest->m_Priority <= io_request::LOW_PRIORITY );
	// BW - A buffer address of 0 is actually valid when the destination
	// is audio ram for the PS2.
    ASSERT( pRequest->m_pBuffer || pRequest->m_Destination);
    ASSERT( pRequest->m_Offset >= 0 );
    ASSERT( pRequest->m_Length >= 0 );

    // Coerce.
    pFile = pRequest->m_pOpenFile->pDeviceFile;
    ASSERT( VALID_DEVICE_FILE( pFile ) );

    // Get device.
    pDevice = pFile->pDevice;
    ASSERT( pDevice );

    // Error check.
    ASSERT( pFile->IsOpen );
    ASSERT( pFile->pBuffer );

    // Special checks if its a read operation.
    if( pRequest->m_Operation == io_request::READ_OP )
    {
        ASSERT( pRequest->m_Offset < pFile->Length );
        ASSERT( (pRequest->m_Offset + pRequest->m_Length) <= pFile->Length );
    }

    // Mark it as queued.
    pRequest->m_Status = io_request::QUEUED;

    // Get the queue time.
#if !defined(X_RETAIL)
    pRequest->m_QueueTick = x_GetTime();
#endif

    // Queue the request.
    s32 MessageStatus = GET_DISPATCHER_MQ().Send( pRequest, MQ_NOBLOCK );
    ASSERT( MessageStatus );
    (void)MessageStatus;

    // Tell the world...
    return( 1 );
}

//==============================================================================

io_device_file* io_mgr::OpenDeviceFile( const char* pFileName, s32 Device, io_device::open_flags OpenFlags )
{
    io_device* pDevice = GET_DEVICE_FROM_INDEX( Device );

    // Error check
    ASSERT( s_Initialized );
    ASSERT( pFileName );
    ASSERT( pDevice );

    // Open the file
    return pDevice->OpenFile( pFileName, OpenFlags );
}

//==============================================================================

void io_mgr::CloseDeviceFile( io_device_file* pFile )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( VALID_DEVICE_FILE( pFile ) );
    ASSERT( pFile->IsOpen );
    
    // Close it
    pFile->pDevice->CloseFile( pFile );
}

//==============================================================================

s32 io_mgr::GetDeviceQueueStatus( s32 Device ) const
{
    io_device* pDevice;

    if( !s_Initialized )
        return 0;

    // Error check.
    ASSERT( s_Initialized );
    ASSERT( VALID_DEVICE_INDEX( Device ) );

    // Get device.
    pDevice = GET_DEVICE_FROM_INDEX( Device );
    ASSERT( pDevice );

    // Tell the world
    return pDevice->GetDeviceQueueStatus();
}

//==============================================================================

void io_mgr::SetDevicePathPrefix( const char* pPrefix, s32 Device )
{
    io_device* pDevice;

    // Error check.
    ASSERT( pPrefix );
    ASSERT( s_Initialized );
    ASSERT( VALID_DEVICE_INDEX( Device ) );

    // Get device.
    pDevice = GET_DEVICE_FROM_INDEX( Device );
    ASSERT( pDevice );

    // Tell the world
    pDevice->SetPathPrefix( pPrefix );
}

//==============================================================================

void io_mgr::GetDevicePathPrefix( char* pBuffer, s32 Device )
{
    io_device* pDevice;

    // Error check.
    ASSERT( pBuffer );
    ASSERT( s_Initialized );
    ASSERT( VALID_DEVICE_INDEX( Device ) );

    // Get device.
    pDevice = GET_DEVICE_FROM_INDEX( Device );
    ASSERT( pDevice );

    // Tell the world
    pDevice->GetPathPrefix( pBuffer );
}
