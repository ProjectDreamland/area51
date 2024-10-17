//==============================================================================
//
// DVD Device layer for Gamecube
//
//==============================================================================
#include "x_target.hpp"
#if !defined(TARGET_GCN)
#error This is not for this target platform. Check dependancy rules.
#endif

#include "..\io_mgr.hpp"
#include "..\io_filesystem.hpp"
#include "x_memory.hpp"
#include "x_math.hpp"
#include <dolphin.h>
#include <libsn.h>
#include "io_device_dvd.hpp"
#include <dolphin.h>
#include <libsn.h>

//==============================================================================
//
// CDROM Defines, Buffers, Etc
//
//==============================================================================

#define CDROM_CACHE_SIZE    (32*1024)
#define CDROM_NUM_FILES     (32)                        
#define CDROM_INFO_SIZE     (sizeof(DVDFileInfo))
#define CDROM_CACHE         ((void*)s_CdromCache)
#define CDROM_FILES         ((void*)s_CdromFiles)
#define CDROM_BUFFER_ALIGN  (32)
#define CDROM_OFFSET_ALIGN  (4)
#define CDROM_LENGTH_ALIGN  (32)

static char           s_CdromCache[ CDROM_CACHE_SIZE ] GCN_ALIGNMENT(CDROM_BUFFER_ALIGN);
static io_device_file s_CdromFiles[ CDROM_NUM_FILES ];

//==============================================================================
//
// Device definition.
//
//==============================================================================

io_device::device_data s_DeviceData =
{
    "GameCube DVD",     // Name
    TRUE,               // IsSupported
    TRUE,               // IsReadable
    FALSE,              // IsWriteable
    CDROM_CACHE_SIZE,   // CacheSize
    CDROM_BUFFER_ALIGN, // BufferAlign
    CDROM_OFFSET_ALIGN, // OffsetAlign
    CDROM_LENGTH_ALIGN, // LengthAlign
    CDROM_NUM_FILES,    // NumFiles
    CDROM_INFO_SIZE,    // InfoSize
    CDROM_CACHE,        // pCache    
    CDROM_FILES         // pFilesBuffer
};

//==============================================================================

static void ReadCallback( s32 Result, DVDFileInfo* pFileInfo )
{
    (void)pFileInfo;

    // We are in the callback
    g_IODeviceDVD.EnterCallback();

    // Success?
    if( Result >= 0 )
    {
        // Its all good!
        ProcessEndOfRequest( &g_IODeviceDVD, io_request::COMPLETED );
    }
    else
    {
        // Ack failed!
        ProcessEndOfRequest( &g_IODeviceDVD, io_request::FAILED );
    }

    // Done with callback
    g_IODeviceDVD.LeaveCallback();
}

//==============================================================================

void io_device_dvd::CleanFilename( char* pClean, const char* pFilename )
{
    // Gotta fit.
    ASSERT( x_strlen(pFilename) + x_strlen(m_Prefix) < IO_DEVICE_FILENAME_LIMIT );

    // Pre-pend the prefix.
    x_strcpy( pClean, m_Prefix );
    
    // Move to end of string.
    pClean += x_strlen( pClean );

    // Now clean it.
    while( *pFilename )
    {
        if( (*pFilename == '\\') || (*pFilename == '/') )
        {
            *pClean++ = '/';
            pFilename++;

            while( *pFilename && ((*pFilename == '\\') || (*pFilename == '/')) )
                pFilename++;
        }
        else
        {
            *pClean++ = *pFilename++;
        }
    }

    // Terminate it.
    *pClean = 0;
}

//==============================================================================
void io_device_dvd::Init(void)
{
    io_device::Init();
}

//==============================================================================
void io_device_dvd::Kill(void)
{
    io_device::Kill();
}

//==============================================================================

io_device::device_data* io_device_dvd::GetDeviceData( void )
{
    return &s_DeviceData;
}

#ifdef TARGET_DEV

//==============================================================================
//==============================================================================
//============================== Devkit Functions ==============================
//==============================================================================
//==============================================================================

xbool io_device_dvd::PhysicalOpen( const char* pFilename, io_device_file* pFile )
{
    if( IsDevLink() )
    {
        // Open the file
        pFile->Handle=(void*)(PCopen( (char *)pFilename, 0, 0 ) + 1);

        if( pFile->Handle )
        {
            pFile->Length = PClseek((int)pFile->Handle-1,0,X_SEEK_END);
            PClseek((int)pFile->Handle-1,0,X_SEEK_SET);
            return TRUE;
        }
        else
        {
            // Oops!
            return FALSE;
        }
    }
    else
    {
        // Clean the filename.
        char CleanFile[IO_DEVICE_FILENAME_LIMIT];
        CleanFilename( CleanFile, (char *)pFilename );

        // Open it.
        if( DVDOpen( (char *)CleanFile, (DVDFileInfo*)pFile->pHardwareData ) )
        {
            // Set the handle.
            pFile->Handle = pFile->pHardwareData;

            // Get the length of the file.
            pFile->Length = DVDGetLength( (DVDFileInfo*)pFile->pHardwareData );

            // It's all good!
            return TRUE;
        }
        else
        {
            // Oops!
            return FALSE;
        }
    }
}

//==============================================================================

xbool io_device_dvd::PhysicalRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    xbool Success;
    (void)AddressSpace;

    // Just to make for certain we are NOT trying to transfer directly to vm space.
    ASSERTS( ((u32)pFile->pBuffer >> 24) >= 0x80,"Attempt to do a device read direct to virtual memory");

    // Log the read.
    LogPhysRead( pFile, Length, Offset );    

    if( IsDevLink() )
    {
        Success = TRUE;
        PClseek( (int)pFile->Handle-1, Offset, 0 );
        if( Length != PCread( (int)pFile->Handle-1, (char*)pBuffer, Length ) )
        {
            Success = FALSE;
        }
        else
        {
            Success = TRUE;
        }
        ReadCallback( 1, (DVDFileInfo*)pFile->pHardwareData );
    }
    else
    {
        Success = DVDReadAsync( (DVDFileInfo*)pFile->pHardwareData, pBuffer, Length, Offset, ReadCallback );
    }

    return Success;
}

//==============================================================================

void io_device_dvd::PhysicalClose ( io_device_file* pFile )
{
    if( IsDevLink() )
    {
        PCclose( (int)pFile->Handle-1 );
    }
    else
    {
        DVDClose( (DVDFileInfo*)pFile->pHardwareData );
    }
}

#endif // TARGET_DEV

#ifdef TARGET_DVD

//==============================================================================
//==============================================================================
//================================ DVD Functions ===============================
//==============================================================================
//==============================================================================

xbool io_device_dvd::PhysicalOpen( const char* pFilename, io_device_file* pFile )
{
    // Clean the filename.
    char CleanFile[IO_DEVICE_FILENAME_LIMIT];
    CleanFilename( CleanFile, (char *)pFilename );

    // Open the file on the dvd.
    if( DVDOpen( (char *)CleanFile, (DVDFileInfo*)pFile->pHardwareData ) )
    {
        // Set the handle.
        pFile->Handle = pFile->pHardwareData;

        // Get the length of the file.
        pFile->Length = DVDGetLength( (DVDFileInfo*)pFile->pHardwareData );

        // Woot!
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//==============================================================================

xbool io_device_dvd::PhysicalRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    (void)AddressSpace;

    // Just to make for certain we are NOT trying to transfer directly to vm space.
    ASSERTS( ((u32)pFile->pBuffer >> 24) == 0x80,"Attempt to do a device read direct to virtual memory");

    // Log the read.
    LogPhysRead( pFile, Length, Offset );    

    // Do the async read.
    xbool Success = DVDReadAsync( (DVDFileInfo*)pFile->pHardwareData, pBuffer, Length, Offset, ReadCallback );

    // Tell the world.
    return Success;
}

//==============================================================================

void io_device_dvd::PhysicalClose ( io_device_file* pFile )
{
    // Close the file on the DVD.
    DVDClose( (DVDFileInfo*)pFile->pHardwareData );
}

#endif // TARGET_DVD
