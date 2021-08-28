//==============================================================================
//
// DVD Device layer for PC
//
//==============================================================================
#include "x_target.hpp"
#if !defined(TARGET_PC)
#error This is not for this target platform. Check dependancy rules.
#endif

#include "..\io_mgr.hpp"
#include "..\io_filesystem.hpp"
#include "x_memory.hpp"
#include "x_math.hpp"
#include "io_device_dvd.hpp"
#include <stdio.h>

//==============================================================================
//
// CDROM Defines, Buffers, Etc
//
//==============================================================================

#define CDROM_CACHE_SIZE    (32*1024)
#define CDROM_NUM_FILES     (32)                        // TODO: CJG - Increase this to accomodate multiple CDFS's
#define CDROM_INFO_SIZE     (32)
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
    "PC DVD",           // Name
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

static void ReadCallback( s32 Result, void* pFileInfo )
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

    // If hard disk drive is specified, do NOT prepend the prefix.
    if( strnicmp( pFilename, "C:", 2 ) != 0 )
    {
        // Pre-pend the prefix.
        x_strcpy( pClean, m_Prefix );
    }
    else
    {
        *pClean = 0;
    }
    
    // Move to end of string.
    pClean += x_strlen( pClean );

    // Now clean it.
    while( *pFilename )
    {
        if( (*pFilename == '\\') || (*pFilename == '/') )
        {
            *pClean++ = '\\';
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
void io_device_dvd::Init( void )
{
    // Base class initialization
    io_device::Init();

    // Set default path so current apps will function without modification
    x_strcpy( m_Prefix, "C:\\GAMEDATA\\A51\\RELEASE\\PC\\DVD\\" );
}

//==============================================================================
void io_device_dvd::Kill( void )
{
    io_device::Kill();
}

//==============================================================================

io_device::device_data* io_device_dvd::GetDeviceData( void )
{
    return &s_DeviceData;
}


//==============================================================================
//==============================================================================
//================================ DVD Functions ===============================
//==============================================================================
//==============================================================================

xbool io_device_dvd::PhysicalOpen( const char* pFilename, io_device_file* pFile, open_flags OpenFlags )
{
    (void)OpenFlags;
    // Clean the filename.
    char CleanFile[IO_DEVICE_FILENAME_LIMIT];
    CleanFilename( CleanFile, (char *)pFilename );

    pFile->Handle = fopen(CleanFile,"rb");
    // Open the file on the dvd.
    if( pFile->Handle )
    {
        // Get the length of the file.
        fseek( (FILE*)pFile->Handle, 0, SEEK_END );
        pFile->Length = ftell( (FILE*)pFile->Handle );
        fseek( (FILE*)pFile->Handle,0,SEEK_SET );
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
    s32 ReadLength;
	(void)AddressSpace;

    // Log the read.
    LogPhysRead( pFile, Length, Offset );    

    fseek( (FILE*)pFile->Handle, Offset, SEEK_SET );
    ReadLength = fread(pBuffer,1,Length,(FILE*)pFile->Handle);
    ReadCallback((ReadLength==Length),pFile->pHardwareData);

    // Tell the world.
    return ReadLength == Length;
}

//==============================================================================

xbool io_device_dvd::PhysicalWrite( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    (void)pFile;
    (void)pBuffer;
    (void)Length;
    (void)Offset;
    (void)AddressSpace;
    ASSERT( 0 );
    return 0;
}

//==============================================================================

void io_device_dvd::PhysicalClose ( io_device_file* pFile )
{
    fclose((FILE*)pFile->Handle);
    // Close the file on the DVD.
}

