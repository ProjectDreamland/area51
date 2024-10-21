//==============================================================================
//
// DVD Device layer for XBOX
//
//==============================================================================
#include "x_target.hpp"
#if !defined(TARGET_XBOX)
#error This is not for this target platform. Check dependancy rules.
#endif

#include "..\io_mgr.hpp"
#include "..\io_filesystem.hpp"

#include "x_memory.hpp"
#include "x_math.hpp"
#include "io_device_dvd.hpp"

#include "xbox/xbox_private.hpp"
#include "entropy.hpp"

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
#define CDROM_BUFFER_ALIGN  (2048)
#define CDROM_OFFSET_ALIGN  (2048)
#define CDROM_LENGTH_ALIGN  (2048)

#define USE_UTILITY_DRIVE 1

static char           s_CdromCache[ CDROM_CACHE_SIZE ] GCN_ALIGNMENT(CDROM_BUFFER_ALIGN);
static io_device_file s_CdromFiles[ CDROM_NUM_FILES ];

extern void OnIOErrorCallback( void );

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

//=========================================================================
//  Xbox IO death routine
//=========================================================================

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

    // Pre-pend the prefix.
    x_strcpy( pClean, m_Prefix );

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
    m_Prefix[0]=0;
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

static xbool CacheExcluded( const char* pFilename )
{
    if( x_strstr( pFilename,"PRELOAD." )) return FALSE; // Make legal screen faster
    if( x_strstr( pFilename,"BOOT."    )) return FALSE; // Make shader loading faster

    return TRUE;
}

//==============================================================================

static const char* CopyTo_UtilityPartition( const char* pCleanedPath,const char* pUtilityPath )
{
    const char* Result = pCleanedPath;
#ifdef X_DEBUG
    xtimer Time;
    Time.Reset();
    Time.Start();
#endif
    if( CopyFile( pCleanedPath,pUtilityPath,TRUE ))
        Result = pUtilityPath;
#ifdef X_DEBUG
    f32 TimeTaken = Time.StopMs();
    x_DebugMsg( "Caching of %s to utility partition took %.4f milliseconds\n",pCleanedPath,TimeTaken );
#endif
    return Result;
}

//==============================================================================

static const char* CacheFileToUtility( const char* pCleanedPath,const char* pUtilityPath )
{
    s32 Result = !CacheExcluded( pCleanedPath );
    if( Result )
    {
        // If destination doesn't exist then always copy **********************

        void* hDst = CreateFile( pUtilityPath,GENERIC_READ,0,0,OPEN_EXISTING,0,0 );
        if( hDst == INVALID_HANDLE_VALUE )
            return CopyTo_UtilityPartition( pCleanedPath,pUtilityPath );

        // Open source file and get file information **************************

        HANDLE  hSrc  = CreateFile( pCleanedPath,GENERIC_READ,0,0,OPEN_EXISTING,0,0 );
        ASSERT( hSrc != INVALID_HANDLE_VALUE );

        BY_HANDLE_FILE_INFORMATION Sin;
        BY_HANDLE_FILE_INFORMATION Din;

        GetFileInformationByHandle( hSrc,&Sin );
        GetFileInformationByHandle( hDst,&Din );

        CloseHandle( hSrc );
        CloseHandle( hDst );

        // Do file info comparisons *******************************************

        if( Sin.ftLastWriteTime.dwHighDateTime == Din.ftLastWriteTime.dwHighDateTime )
            return pUtilityPath;
        if( Sin.ftLastWriteTime.dwLowDateTime == Din.ftLastWriteTime.dwLowDateTime )
            return pUtilityPath;
        if( Sin.nFileSizeHigh == Din.nFileSizeHigh )
            return pUtilityPath;
        if( Sin.nFileSizeLow == Din.nFileSizeLow )
            return pUtilityPath;

        return CopyTo_UtilityPartition( pCleanedPath,pUtilityPath );
    }
    return pCleanedPath;
}
 
//==============================================================================
 
xbool io_device_dvd::PhysicalOpen( const char* pFilename, io_device_file* pFile, io_device::open_flags OpenFlags )
{
    const char* pFileToUse;

    pFile->pHardwareData = NULL;

    // Clean the filename.
    char CleanFile[IO_DEVICE_FILENAME_LIMIT];
    char NewFile  [IO_DEVICE_FILENAME_LIMIT];
    CleanFilename( CleanFile, (char *)pFilename );
    strupr( CleanFile );

    DWORD dwCreationDisposition;
    DWORD dwDesiredAccess;
    DWORD dwShareMode;
    DWORD dwFlags;

    // Does file exist on utility drive
    if( USE_UTILITY_DRIVE )
    {
        ASSERT( OpenFlags==io_device::READ );
        x_strcpy( NewFile,CleanFile );
        NewFile[0]='Z';

        // Copy file
        pFileToUse = CacheFileToUtility( CleanFile,NewFile );

        dwCreationDisposition = OPEN_EXISTING;
        dwDesiredAccess       = GENERIC_READ;
        dwShareMode           = FILE_SHARE_READ;
        dwFlags               = FILE_ATTRIBUTE_NORMAL
                              | FILE_FLAG_OVERLAPPED
                              | FILE_FLAG_NO_BUFFERING;
    }
    else
    {
        if( OpenFlags==io_device::WRITE )
        {
            dwCreationDisposition = CREATE_ALWAYS;
            dwDesiredAccess = GENERIC_WRITE;
            dwShareMode = 0;
            dwFlags = 0;

            x_strcpy( NewFile,"Z:\\" );
            x_strcat( NewFile,CleanFile );

            pFileToUse = NewFile;
        }
        else
        {
            dwCreationDisposition = OPEN_EXISTING;
            dwDesiredAccess       = GENERIC_READ;
            dwShareMode           = FILE_SHARE_READ;
            dwFlags               = FILE_ATTRIBUTE_NORMAL
                                | FILE_FLAG_OVERLAPPED
                                | FILE_FLAG_NO_BUFFERING;
            pFileToUse = CleanFile;
        }
    }

    pFile->Handle = CreateFile(
        pFileToUse            , // lpFileName
        dwDesiredAccess       , // dwDesiredAccess
        dwShareMode           , // dwShareMode
        NULL                  , // lpSecurityAttributes
        dwCreationDisposition , // dwCreationDisposition
        dwFlags               , // dwFlagsAndAttributes
        NULL                    // hTemplateFile
    );

    if( pFile->Handle==INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    //
    //  Start I/O thread
    //

    DWORD dwFileSize=GetFileSize(pFile->Handle,NULL);
    pFile->Length=dwFileSize;
    return TRUE;
}

xbool s_InProgress=FALSE;
xbool s_InCallback=FALSE;
//==============================================================================
static void CALLBACK FileIOCompletionRoutine(
  DWORD dwErrorCode,                // completion code
  DWORD dwNumberOfBytesTransfered,  // number of bytes transferred
  LPOVERLAPPED lpOverlapped         // I/O information buffer
)
{   (void)dwNumberOfBytesTransfered;
    (void)lpOverlapped;

    s_InCallback=TRUE;
    ASSERT(s_InProgress);

    if( dwErrorCode )
    {
        OnIOErrorCallback();
    }

    ReadCallback(dwErrorCode == 0, NULL);
    s_InCallback=FALSE;
    s_InProgress=FALSE;
}

//==============================================================================
xbool io_device_dvd::PhysicalWrite( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    (void)pFile;
    (void)pBuffer;
    (void)Length;
    (void)Offset;
    (void)AddressSpace;

    // We are in the callback
    g_IODeviceDVD.EnterCallback();

    DWORD dwWritten;
    BOOL bResult = WriteFile( pFile->Handle,pBuffer,Length,&dwWritten,NULL );
    if ( bResult )
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

    return bResult;
}

//==============================================================================
xbool io_device_dvd::PhysicalRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    (void)AddressSpace;
    static OVERLAPPED request;
    xbool ok;

    // Log the read.
    LogPhysRead( pFile, Length, Offset );    

    if( s_InProgress) OnIOErrorCallback();

    ASSERT(!s_InProgress);
    s_InProgress = TRUE;

    x_memset( &request,0,sizeof( request ));
    request.Offset = Offset;

    ok=ReadFileEx(
        pFile->Handle,
        pBuffer,
        Length,
        &request,
        FileIOCompletionRoutine
    );
    if( !ok )
    {
        x_DebugMsg("IO Request submit failed, error %d\n",GetLastError());
        static void*           ErrBuffer = pBuffer;
        static u32             ErrCode   = GetLastError();
        static u32             ErrLen    = Length;
        static io_device_file* ErrFile   = pFile;
        static s32             ErrOffset = Offset;
        {
            OnIOErrorCallback();
        }
        return FALSE;
    }
    // Suspend this thread until the callback gets a chance to run
    SleepEx(INFINITE,TRUE);
	ASSERT(!s_InProgress);
    return TRUE;
}

//==============================================================================

void io_device_dvd::PhysicalClose ( io_device_file* pFile )
{
    HANDLE hFile=HANDLE(pFile->Handle);
    ASSERTS(hFile,"Bad handle value!");
    pFile->Handle=NULL;
    CloseHandle(pFile->Handle);
    pFile->Handle = NULL;
}
