#include "io_mgr.hpp"
#include "io_filesystem.hpp"
#include "x_memory.hpp"
#include "x_math.hpp"
#include "x_time.hpp"
#include "x_log.hpp"

#ifdef TARGET_GCN
#include <dolphin.h>
#include <libsn.h>
#include "Device_DVD\io_device_dvd.hpp"
#endif

//==============================================================================
//  Checksum Helper Class
//==============================================================================

u16 crc16Table[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

#define crc16ApplyByte( v, crc ) (u16)((crc << 8) ^  crc16Table[((crc >> 8) ^ (v)) & 255])

//==============================================================================

//#define LOG_REQUESTS
#if defined(TARGET_XBOX) && defined(X_DEBUG)
//#define USE_NET_FS
#endif
//==============================================================================

void ProcessEndOfRequest( s32 DeviceIndex, s32 Status );

//==============================================================================
//========================== Hardware Specific Stuff ===========================
//==============================================================================
//==============================================================================

void ProcessEndOfRequest( io_device* pDevice, s32 Status )
{
    io_request* pRequest;

    // Error check.
    ASSERT( pDevice );
    ASSERT( pDevice->IsInCallback() );

    // Get devices current request.
    pRequest = pDevice->m_CurrentRequest;
    ASSERT( pRequest );

    // Checksum enabled for this file?
    if( pRequest->m_pOpenFile->bEnableChecksum )
    {
        // Get the checksum from the dfs header.
        io_device_file* pDeviceFile = pRequest->m_pOpenFile->pDeviceFile;
        dfs_header*     pHeader     = (dfs_header*)pDeviceFile->pHeader;
        s32             Index       = (pRequest->m_Offset+pRequest->m_ChunkOffset) / 32768;

        // Only if checksums are available...
        if( pHeader->pChecksums )
        {
            // Adjust the index based on the sub files index offset.
            Index += pHeader->pSubFileTable[ pDeviceFile->SubFileIndex ].ChecksumIndex;
            u16 CheckSum2 = pHeader->pChecksums[ Index ];

            // Calculate the crc.
            u8* pData    = (u8*)pDeviceFile->pBuffer;
            s32 nBytes   = 32768;
            u16 CheckSum = 0;

            while( nBytes-- > 0 )
            {
                CheckSum = crc16ApplyByte( *pData, CheckSum );
                pData++;
            }

            (void)CheckSum2;
            ASSERT( CheckSum2 == CheckSum );

#ifdef TARGET_XBOX
            extern void OnIOErrorCallback( void );
            if( CheckSum2 != CheckSum )
            {
                OnIOErrorCallback();
            }
#endif
        }
    }

    // Set hardware status.
    pRequest->m_HardwareStatus = Status;

    // Wake up the dispatcher.
    s32 MessageStatus = GET_DISPATCHER_MQ().Send( (void*)pRequest, MQ_NOBLOCK );
    ASSERT( MessageStatus );
    (void)MessageStatus;
}

#if defined(LOG_REQUESTS)
static f32 s_MaxRequestTime = 0.0f;
#endif

//==============================================================================

xbool io_device::ProcessReadComplete( io_request* pRequest )
{
    io_device_file* pFile;
    xbool           RequestDone = FALSE;

    // Get the file handle.
    pFile = pRequest->m_pOpenFile->pDeviceFile;
    ASSERT( pFile );

    // Decide what to do based on the hardware status.
    switch( pRequest->m_HardwareStatus )
    {
        case io_request::COMPLETED:
        {
            void* pDest;
            void* pSrc;
            s32   Count;
            
            // Was an offset adjustment required for proper alignment?
            if( pRequest->m_ChunkOffset < 0 )
            {
                //
                // Had to adjust the initial chunk offset.
                //

                //
                // NOTE: This code can only be executed for the FIRST
                //       read in any request.  Afterwards, the offset
                //       will be properly aligned.
                //

                // Set source (bump to actual data in the buffer)
                pSrc = (void*)((u32)pFile->pBuffer - (u32)pRequest->m_ChunkOffset);
                
                // Set destination
                pDest = (void*)pRequest->m_pBuffer;

                // Set length
                pRequest->m_ChunkOffset = pRequest->m_ChunkLength + pRequest->m_ChunkOffset;
                Count = pRequest->m_ChunkLength + pRequest->m_ChunkOffset;
                if( Count > pRequest->m_Length )
                    Count = pRequest->m_Length;
            }
            else
            {
                //
                // No adjustment requried.
                //

                // Set source
                pSrc = (void*)pFile->pBuffer;
                
                // Set destination
                pDest = (void*)((u32)pRequest->m_pBuffer + (u32)pRequest->m_ChunkOffset);

                // Set length
                if( pRequest->m_ChunkOffset + pRequest->m_ChunkLength > pRequest->m_Length )
                {
                    Count = pRequest->m_Length - pRequest->m_ChunkOffset;
                }
                else
                {
                    Count = pRequest->m_ChunkLength;
                }

                // go to the next chunk...
                pRequest->m_ChunkOffset += Count;
            }

            // Copy the data from the internal buffers (if needed).
            if ( ( pDest != pSrc ) && (pRequest->m_Destination==0) )
                x_memcpy( pDest, pSrc, Count );

            // Error check
            ASSERT( pRequest->m_ChunkOffset >= 0 );

            // Read done?
            if( pRequest->m_ChunkOffset >= pRequest->m_Length )
            {
                // Set flag
                RequestDone = TRUE;

                // Set status
                pRequest->m_Status = io_request::COMPLETED;
            }
            break;
        }

        case io_request::FAILED:
        {
            // Set flag
            RequestDone = TRUE;

            // Set status
            pRequest->m_Status = io_request::FAILED;

            // TODO: Deal with failures and retries...
            break;
        }

        default:
        {
            ASSERT( 0 );
            break;
        }
    }        

    return RequestDone;
}

//==============================================================================

xbool io_device::ProcessWriteComplete( io_request* pRequest )
{
    xbool RequestDone = TRUE;

    // Decide what to do based on the hardware status.
    switch( pRequest->m_HardwareStatus )
    {
        case io_request::COMPLETED:
            pRequest->m_Status = io_request::COMPLETED;
            RequestDone = TRUE;
            break;

        case io_request::FAILED:
            pRequest->m_Status = io_request::FAILED;
            RequestDone = TRUE;
            break;

        default:
            ASSERT( 0 );
    }        

    return RequestDone;
}

//==============================================================================

void io_device::ServiceDeviceCurrentRequest( void )
{
    io_request* pRequest;

    // Get the devices current request.
    pRequest = m_CurrentRequest;

    // Device no longer has a current request.
    m_CurrentRequest = NULL;

#ifdef TARGET_GCN
    // This st00pid code fixes a problem with the optimizer on the GCN.
    f32 f = x_sqrt( 1.0f );       // DO NOT REMOVE!!!!
    (void)f;
#endif

    // Only if current request is valid...
    if( pRequest )
    {
        xbool bDone;

        // What kind of operation?
        switch( pRequest->m_Operation )
        {
            case io_request::READ_OP:   bDone = ProcessReadComplete ( pRequest );   break;
            case io_request::WRITE_OP:  bDone = ProcessWriteComplete( pRequest );   break;
            default:                    bDone = FALSE; ASSERT( 0 );
        }

        // Request done?
        if( bDone )
        {
            io_request* pPrev;
            io_request* pNext;

            // get links
            pPrev = pRequest->m_pPrev;
            pNext = pRequest->m_pNext;

            // Remove request from list.
            pPrev->m_pNext = pNext;
            pNext->m_pPrev = pPrev;

            // unlink it
            pRequest->m_pPrev = NULL;
            pRequest->m_pNext = NULL;

            // Adjust request count
            m_RequestCount--;
            ASSERT( m_RequestCount >= 0 );
        
#if !defined(X_RETAIL) && defined(LOG_REQUESTS)
            // Get the time now...
            pRequest->m_CompleteTick = x_GetTime();
            xtick DispatchTicks = pRequest->m_DispatchTick - pRequest->m_QueueTick;
            xtick ReadTicks     = pRequest->m_CompleteTick - pRequest->m_DispatchTick;
            f32   DispatchMS    = x_TicksToMs( DispatchTicks );
            f32   ReadMS        = x_TicksToMs( ReadTicks );
            f32   TotalMS       = DispatchMS + ReadMS;
            if( TotalMS > s_MaxRequestTime )
                s_MaxRequestTime = TotalMS;
            LOG_MESSAGE( "ServiceDeviceCurrentRequest", "Request Complete! Total: %07.3fms, Dispatch: %07.3fms, Read: %07.3fms, MAX: %08.3fms", TotalMS, DispatchMS, ReadMS, s_MaxRequestTime );
#endif

            // Adjust files reference count.
            io_device_file* pFile = pRequest->m_pOpenFile->pDeviceFile;
            pFile->ReferenceCount--;
            ASSERT( pFile->ReferenceCount >= 0 );

            // User callback requested?
            if( pRequest->m_pCallback )
            {
                // Make the user callback
                (*pRequest->m_pCallback)( pRequest ); 
            }

            // Is this request using the semaphore?
            if( pRequest->m_UseSema )
            {
                // Release the semaphore.
                pRequest->m_Semaphore.Release();
            }
        }
    }
}

//==============================================================================

void io_device::ProcessReadRequest( io_request* pRequest )
{
    io_device_file* pFile;
    s32             Adjust;

    // BW - On PS2, having a zero m_pBuffer is valid if the destination address 
    // is not the local memory space.
    ASSERT( pRequest->m_pBuffer || pRequest->m_Destination);

    // Coerce.
    pFile = pRequest->m_pOpenFile->pDeviceFile;
    ASSERT( VALID_DEVICE_FILE( pFile ) );

    // Error check file.
    ASSERT( pFile->pDevice->m_DeviceIndex == m_DeviceIndex );
    ASSERT( pFile->IsOpen );
    ASSERT( pRequest->m_Offset < pFile->Length );
    ASSERT( (pRequest->m_Offset + pRequest->m_Length) <= pFile->Length );
    ASSERT( pFile->pBuffer );

    // Set chunk length.
    pRequest->m_ChunkLength = m_CacheSize;

    // Any offset restrictions?
    if( m_OffsetAlign )
    {
        // Calculate any adjustment
        Adjust = (pRequest->m_Offset + pRequest->m_ChunkOffset) & (m_OffsetAlign-1);
    }
    else
    {
        // No need
        Adjust = 0;
    }

    // Need to adjust for offset alignment?
    if( Adjust )
    {
        //
        // NOTE: This code can only be executed for the FIRST
        //       read in any request.  Afterwards, the offset
        //       will be properly aligned.
        //

        // Adjust chunk offset for required alignment.
        pRequest->m_ChunkOffset -= Adjust;

        //
        // NOTE: pRequest->m_ChunkOffset will be negative!
        //
    }

    // Gonna read past the end of file?
    if( (pRequest->m_Offset + pRequest->m_ChunkOffset + pRequest->m_ChunkLength) > pFile->Length )
    {
        // Set chunk length.
        pRequest->m_ChunkLength = pFile->Length - (pRequest->m_ChunkOffset+pRequest->m_Offset);
    }

    // Error check.
    ASSERT( pRequest->m_ChunkLength <= m_CacheSize );
    ASSERT( pRequest->m_ChunkLength > 0 );

    // Set current request
    m_CurrentRequest = pRequest;

    // TODO: Put write support in...

    xbool Success;
    s32   Length = pRequest->m_ChunkLength;

    // any length alignment needed?
#ifdef TARGET_GCN
    if( m_LengthAlign && ((this==&g_IODeviceDVD) && !IsDevLink()) )
#else
    if( m_LengthAlign )
#endif
    {
        s32 Mask = m_LengthAlign-1;

        // Adjust 
        Length = ((pRequest->m_ChunkLength + Mask) & (~Mask));

        // Error check.
        ASSERT( Length > 0 );
        ASSERT( Length <= m_CacheSize );
    }

    //
    // Do the asynch read.
    //
    // BW - If we are transferring data to anything other than the main memory address space,
    // go ahead and transfer it to where it is supposed to be but bear in mind that the device
    // must be aware of the differences between memory spaces and deal with alignment that may
    // cause.
    //
    if( Length + pRequest->m_Offset > pFile->Length )
    {
        Length = pFile->Length - pRequest->m_Offset;
        ASSERT( Length > 0 );
    }

    if (pRequest->m_Destination==0)
    {
        Success = PhysicalRead( pFile, 
                                pFile->pBuffer, 
                                Length, 
                                pRequest->m_Offset+pRequest->m_ChunkOffset, 
                                pRequest->m_Destination );
    }
    else
    {
        Success = PhysicalRead( pFile, 
                                (void*)((s32)pRequest->m_pBuffer+pRequest->m_ChunkOffset), 
                                Length, 
                                pRequest->m_Offset+pRequest->m_ChunkOffset, 
                                pRequest->m_Destination );
    }

    // TODO: Handle Errors.
    ASSERT( Success );
}

//==============================================================================

void io_device::ProcessWriteRequest( io_request* pRequest )
{
    io_device_file* pFile;
    xbool           bSuccess;

    // Coerce.
    pFile = pRequest->m_pOpenFile->pDeviceFile;
    ASSERT( VALID_DEVICE_FILE( pFile ) );

    // Error check file.
    ASSERT( pFile->pDevice->m_DeviceIndex == m_DeviceIndex );
    ASSERT( pFile->IsOpen );

    // Set current request
    m_CurrentRequest = pRequest;

    // Do the write
    bSuccess = PhysicalWrite( pFile, pRequest->m_pBuffer, pRequest->m_Length, pRequest->m_Offset, 0 ); 

    // TODO: Handle Errors.
    ASSERT( bSuccess );
}

//==============================================================================

void io_device::ServiceDeviceQueue( void )
{
    io_request* pRequest;

    // Get first request.
    pRequest = m_RequestQueue.m_pNext;
    ASSERT( pRequest );

    // Check for empty queue.
    if( pRequest != &m_RequestQueue )
    {
        // Needs to be pending or in progress...
        ASSERT( (pRequest->m_Status == io_request::PENDING) || (pRequest->m_Status == io_request::IN_PROGRESS) );
        
        // First time?
        if( pRequest->m_Status == io_request::PENDING )
        {
#if !defined(X_RETAIL)
            // Set the dispatch time.
            pRequest->m_DispatchTick = x_GetTime();
#endif
            // Mark it as in progress.
            pRequest->m_Status = io_request::IN_PROGRESS;

            // Init chunk offset.
            pRequest->m_ChunkOffset = 0;
        }

        // Which operation requested?
        switch( pRequest->m_Operation )
        {
            case io_request::READ_OP:   ProcessReadRequest ( pRequest );    break;
            case io_request::WRITE_OP:  ProcessWriteRequest( pRequest );    break;
            default:                    ASSERT( 0 );
        }
    }
}

//==============================================================================

io_device::io_device( void ) : m_Semaphore(1)
{
    m_IsSupported = FALSE;
}

//==============================================================================

io_device::~io_device( void )
{
}

//==============================================================================

void io_device::Init()
{
    device_data* pData;

    MEMORY_OWNER( "io_device::Init()" );

    // Get pointer to data
    pData = GetDeviceData();

    // Only if it is supported
    if( pData->IsSupported )
    {
        io_device_file* pFile;
        s32             i;

        // copy name
        x_strncpy( m_Name, pData->Name, IO_DEVICE_MAX_NAME_LENGTH );
        m_Name[ IO_DEVICE_MAX_NAME_LENGTH-1 ] = 0;

        // fill in data structures
        m_IsSupported       = pData->IsSupported;
        m_IsReadable        = pData->IsReadable;
        m_IsWriteable       = pData->IsWriteable;
        m_CacheSize         = pData->CacheSize;
        m_BufferAlign       = pData->BufferAlign;
        m_OffsetAlign       = pData->OffsetAlign;
        m_LengthAlign       = pData->LengthAlign;
        m_NumFiles          = pData->NumFiles;
        m_HardwareDataSize  = pData->HardwareDataSize;
        m_pCache            = pData->pCache;
        m_pFilesBuffer      = pData->pFilesBuffer;
        m_CallbackLevel     = 0;
        m_Prefix[0]         = 0;
                            
        // set up the free list for the files
        for( i=0,pFile=(io_device_file*)m_pFilesBuffer ; i<m_NumFiles ; i++,pFile++ )
        {
            if( m_HardwareDataSize )
            {
                // Allocate device memory
                pFile->pHardwareData = x_malloc( m_HardwareDataSize );
            }

            // Put in list
            pFile->pNext = (pFile+1);
        }

        pFile--;
        // Terminate list
        pFile->pNext = NULL;

        // Init free list.
        m_pFreeFiles = (io_device_file*)m_pFilesBuffer;

        // Init
        m_RequestCount   = 0;
        m_Sequence       = 0;
        m_CurrentRequest = NULL;

        // Empty the queue
        m_RequestQueue.m_pNext    = &m_RequestQueue;
        m_RequestQueue.m_pPrev    = &m_RequestQueue;
        m_RequestQueue.m_Priority = io_request::NUM_PRIORITIES;

        // Release the semaphore.
        m_Semaphore.Send( NULL, MQ_BLOCK );
    }
    else
    {
        x_strncpy( m_Name, "UNSUPPORTED", IO_DEVICE_MAX_NAME_LENGTH );
        m_DeviceIndex          = -1;
        m_IsSupported          = FALSE;
        m_IsReadable           = FALSE;
        m_IsWriteable          = FALSE;
        m_CacheSize            = 0;
        m_BufferAlign          = 0;
        m_OffsetAlign          = 0;
        m_LengthAlign          = 0;
        m_NumFiles             = 0;
        m_pCache               = NULL;
        m_pFilesBuffer         = NULL;
        m_HardwareDataSize     = 0;
        m_pFreeFiles           = NULL;
        m_RequestCount         = 0;
        m_Sequence             = 0;
        m_RequestQueue.m_pNext = &m_RequestQueue;
        m_RequestQueue.m_pPrev = &m_RequestQueue;
        m_CurrentRequest       = NULL;
        m_CallbackLevel        = 0;
        m_Prefix[0]            = 0;
    }
}

//==============================================================================

void io_device::Kill( void )
{
    // Only if the device is supported
    if( m_IsSupported )
    {
        io_device_file* pFile;
        s32        i;

        m_Semaphore.Recv( MQ_BLOCK );
        // set up the free list for the files
        for( i=0,pFile=(io_device_file*)m_pFilesBuffer ; i<m_NumFiles ; i++,pFile++ )
        {
            if( m_HardwareDataSize )
            {
                // Free the device hardware data
                ASSERT( pFile->pHardwareData );
                x_free( pFile->pHardwareData );
            }
            pFile->pHardwareData = NULL;
        }
    }
}

//==============================================================================

io_device_file* io_device::OpenFile( const char* pFileName, open_flags OpenFlags )
{
    io_device_file* pFile;
    io_device*      pDevice = this;

    ASSERT( pDevice );
    ASSERT( SUPPORTED_DEVICE( pDevice ) );

    // Error check.
    ASSERT( pFileName );

    // Aquire device semaphore
    pDevice->m_Semaphore.Recv( MQ_BLOCK );

    // Get free file.
    pFile = m_pFreeFiles;

    // Is a file available?
    if( pFile )
    {
        // set the device
        pFile->pDevice = pDevice;

        // Attempt to open the file
        if( PhysicalOpen( (const char*)pFileName, pFile, OpenFlags ) )
        {
            // Set up the buffer (uses devices cache)
            pFile->pBuffer     = pDevice->m_pCache;
            pFile->BufferValid = 0;

            // Remove file from the free list.
            m_pFreeFiles = m_pFreeFiles->pNext; 

            // Its open.
            pFile->ReferenceCount = 0;
            pFile->IsOpen         = TRUE;
            pFile->pHeader        = NULL;
        }
        else
        {
            // Could not open the file.
            pFile = NULL;
        }
    }

    // Save filename
    if( pFile )
    {
        ASSERT( x_strlen(pFileName) < IO_DEVICE_FILENAME_LIMIT );
        x_strncpy( pFile->Filename, pFileName, IO_DEVICE_FILENAME_LIMIT );
    }

    // Release semaphore
    pDevice->m_Semaphore.Send( NULL, MQ_BLOCK );

    // Tell the world...
    return( pFile );
}

//==============================================================================

void io_device::CloseFile( io_device_file* pFile )
{
    io_device* pDevice = this;

    // Error check.
    ASSERT( VALID_DEVICE_FILE( pFile ) );
    ASSERT( pFile->pDevice );
    ASSERT( pDevice == pFile->pDevice );

    // Error check.
    ASSERT( pFile->IsOpen );

    // Aquire devices semaphore
    pDevice->m_Semaphore.Recv( MQ_BLOCK );

    // Physically close the file
    PhysicalClose( pFile );

    // The file CANNOT be referenced by any IO Requests.
    ASSERT( pFile->ReferenceCount == 0 );

    // Initialize
    pFile->Handle      = 0;
    pFile->Length      = 0;
    pFile->BufferValid = 0;
    pFile->IsOpen      = 0;
    pFile->pDevice     = NULL;

    // clear buffer
    pFile->pBuffer = 0;

    // Put in free list.
    pFile->pNext = m_pFreeFiles;
    m_pFreeFiles = pFile;

    // Release devices semaphore
    pDevice->m_Semaphore.Send( NULL, MQ_BLOCK );
}

//==============================================================================

s32 io_device::GetFileDevice ( io_device_file* pFile ) const
{
    // Error check
    ASSERT( VALID_DEVICE_FILE( pFile ) );
    ASSERT( pFile->pDevice );

    // tell the world
    return( pFile->pDevice->m_DeviceIndex );
}

//==============================================================================

s32 io_device::GetDeviceQueueStatus( void ) const
{
    // tell the world
    return( m_RequestCount );
}

//==============================================================================

void io_device::SetPathPrefix( const char *pPrefix )
{
    ASSERT( pPrefix );
    ASSERT( x_strlen( pPrefix ) < IO_DEVICE_MAX_PREFIX_LENGTH );

    // Aquire devices semaphore.
    m_Semaphore.Recv( MQ_BLOCK );
    
    // Copy the prefix
    x_strncpy( m_Prefix, pPrefix, IO_DEVICE_MAX_PREFIX_LENGTH );
    
    // Make sure its null terminated.
    m_Prefix[ IO_DEVICE_MAX_PREFIX_LENGTH-1 ] = 0;

    // Release devices semaphore.
    m_Semaphore.Send( NULL, MQ_BLOCK );
}
//==============================================================================

void io_device::GetPathPrefix( char *pBuffer )
{
    // Aquire devices semaphore.
    m_Semaphore.Recv( MQ_BLOCK );

    // Copy the prefix
    x_strncpy( pBuffer, m_Prefix, IO_DEVICE_MAX_PREFIX_LENGTH );

    // Make sure its null terminated.
    pBuffer[ IO_DEVICE_MAX_PREFIX_LENGTH-1 ] = 0;

    // Release devices semaphore.
    m_Semaphore.Send( NULL, MQ_BLOCK );
}

//==============================================================================

void io_device::CleanFilename( char* pClean, char* pFilename )
{
    (void)pClean;
    (void)pFilename;
    ASSERT( 0 );
}

//==============================================================================

io_device::device_data* io_device::GetDeviceData( void )
{
    ASSERT( 0 );
    return NULL;
}

//==============================================================================

xbool io_device::PhysicalOpen( const char* pFilename, io_device_file* pFile, open_flags OpenFlags )
{
    (void)pFilename;
    (void)pFile;
    (void)OpenFlags;
    ASSERT( 0 );
    return FALSE;
}

//==============================================================================

xbool io_device::PhysicalRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    (void)pFile;
    (void)pBuffer;
    (void)Length;
    (void)Offset;
    (void)AddressSpace;
    ASSERT( 0 );
    return FALSE;
}

//==============================================================================

xbool io_device::PhysicalWrite( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    (void)pFile;
    (void)pBuffer;
    (void)Length;
    (void)Offset;
    (void)AddressSpace;
    ASSERT( 0 );
    return FALSE;
}

//==============================================================================

void io_device::PhysicalClose ( io_device_file* pFile )
{
    (void)pFile;
    ASSERT( 0 );
}


