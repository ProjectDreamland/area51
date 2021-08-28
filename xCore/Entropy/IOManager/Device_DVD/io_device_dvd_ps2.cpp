//==============================================================================
//
// DVD Device layer for PlayStation 2 and XBOX with caching
//
//==============================================================================
#include "x_target.hpp"
#if !defined(TARGET_PS2) && !defined(TARGET_XBOX)
#error This is not for this target platform. Check dependancy rules.
#endif

#include "x_files.hpp"
#include "io_device_dvd.hpp"
#include "io_device_cache.hpp"


#define CDROM_CACHE_SIZE    (32*1024)
#define CDROM_NUM_FILES     (32)                        
#define CDROM_INFO_SIZE     (0)
#define CDROM_CACHE         ((void*)s_CdromCache)
#define CDROM_FILES         ((void*)s_CdromFiles)
#define CDROM_BUFFER_ALIGN  (64)
#define CDROM_OFFSET_ALIGN  (2048)
#define CDROM_LENGTH_ALIGN  (0)

static char             s_CdromCache[ CDROM_CACHE_SIZE ] PS2_ALIGNMENT(CDROM_BUFFER_ALIGN);
static io_device_file   s_CdromFiles[ CDROM_NUM_FILES ];

//==============================================================================
//
// Device definitions.
//
//==============================================================================

io_device::device_data s_DeviceData =
{
    "PS2 DVD",          // Name
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

static void IOCallback( s32 Result, void* pFileInfo )
{
    (void)pFileInfo;

    // We are in the callback
    g_IODeviceDVD.EnterCallback();

    // Success?
    if( Result > 0 )
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
    xbool AddPostFix;
    // Gotta fit.
    ASSERT( x_strlen(pFilename) + x_strlen(m_Prefix) < IO_DEVICE_FILENAME_LIMIT );

    AddPostFix = TRUE;
    // If hard disk drive is specified, do NOT prepend the prefix.
    if( x_strncmp( pFilename, "HDD:", 4 ) != 0 )
    {
        // Pre-pend the prefix.
        x_strcpy( pClean, m_Prefix );
    
        // Move to end of string.
        pClean += x_strlen( pClean );
    }
    else
    {
        AddPostFix = FALSE;
        *pClean = 0;
    }

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
            *pClean++ = x_toupper(*pFilename++);
        }
    }

    // Terminate it.
    if( AddPostFix )
    {
#if defined(TARGET_DVD)
        *pClean++=';';
        *pClean++='1';
#endif
    }
    *pClean = 0;
}

//==============================================================================
void io_device_dvd::Init( void )
{
    // Base class initialization
    io_device::Init();
    g_IODeviceCache.Init();

    // Set default path so current apps will function without modification
#if defined(TARGET_DEV)
    x_strcpy(m_Prefix,"host:");
#else
    x_strcpy(m_Prefix,"cdrom0:\\");
#endif
}

//==============================================================================
void io_device_dvd::Kill( void )
{
    g_IODeviceCache.Kill();
    io_device::Kill();
}

//==============================================================================

io_device::device_data* io_device_dvd::GetDeviceData( void )
{
    return &s_DeviceData;
}

//==============================================================================

xbool io_device_dvd::PhysicalOpen( const char* pFilename, io_device_file* pFile, io_device::open_flags OpenFlags )
{
    char            CleanFile[256];
    cache_open_mode Mode = CACHE_OPEN_READ;

    // Clean up the filename!
    CleanFilename( CleanFile, pFilename);

    // Writing? Default is Read...
    if( OpenFlags & WRITE )
    {
        // Set mode to write, set length to 0!
        Mode          = CACHE_OPEN_WRITE;
        pFile->Length = 0;
    }

    // Now we open the file using the iop file manager
    pFile->pHardwareData = (void*)g_IODeviceCache.Open( CleanFile, Mode, (u32&)pFile->Length );
    if (pFile->pHardwareData ==0)
        return FALSE;

    return TRUE;
}

//==============================================================================

void io_device_dvd::PhysicalClose( io_device_file* pFile )
{
    g_IODeviceCache.Close((s32)pFile->pHardwareData);
}

//==============================================================================

xbool io_device_dvd::PhysicalRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 )
{
    s32 ReadLength;

    // Log the read.
    LogPhysRead( pFile, Length, Offset );    

    ReadLength = g_IODeviceCache.Read( (s32)pFile->pHardwareData, pBuffer, Length, Offset );
    ASSERT( ReadLength == Length );
    IOCallback( (ReadLength==Length), pFile->pHardwareData );

    return ReadLength == Length;
}

//==============================================================================

xbool io_device_dvd::PhysicalWrite( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 )
{
    s32 WriteLength;

    // Log the write.
    LogPhysWrite( pFile, Length, Offset );

    WriteLength = g_IODeviceCache.Write( (s32)pFile->pHardwareData, pBuffer, Length, Offset );
    IOCallback((WriteLength==Length),pFile->pHardwareData);

    return WriteLength == Length;
}

