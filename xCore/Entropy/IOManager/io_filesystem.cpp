#include "io_filesystem.hpp"
#include "io_mgr.hpp"
#include "x_files.hpp"
#include "e_virtual.hpp"
#include "device_dvd\io_device_dvd.hpp"
#include "x_log.hpp"

//==============================================================================

#ifdef TARGET_XBOX
#   include "xbox/xbox_private.hpp"
#endif

#define USE_VM_ALLOC

//==============================================================================
// defines.

#if defined(TARGET_GCN) && defined(TARGET_DEV)
#define ENABLE_PASSTHROUGH  (1)
#elif defined( TARGET_XBOX ) && defined(X_DEBUG)
#define ENABLE_PASSTHROUGH  (1)
#elif defined(TARGET_PS2) && defined(TARGET_DEV)
#define ENABLE_PASSTHROUGH  (1)
#elif defined(TARGET_PC)
#define ENABLE_PASSTHROUGH  (1)
#else
#define ENABLE_PASSTHROUGH  (0)
#endif

// CJ: DEBUG_DVD    #undef ENABLE_PASSTHROUGH
// CJ: DEBUG_DVD    #define ENABLE_PASSTHROUGH 0

//#define IO_FS_OPEN_SUCCESS      "io_fs::Open(success)"
//#define IO_FS_OPEN_FAILURE      "io_fs::Open(failure)"
#define IO_FS_PASS_OPEN_SUCCESS "io_fs::Open(pass success)"
#define IO_FS_PASS_OPEN_FAILURE "io_fs::Open(pass failure)"
//#define IO_FS_CLOSE             "io_fs::Close"
//#define DEBUG_IO

//==============================================================================
// Local variables.

static open_fn*     old_Open            = NULL;     // old filesystem functions
static close_fn*    old_Close           = NULL;
static read_fn*     old_Read            = NULL;
static write_fn*    old_Write           = NULL;
static seek_fn*     old_Seek            = NULL;
static tell_fn*     old_Tell            = NULL;
static flush_fn*    old_Flush           = NULL;
static eof_fn*      old_EOF             = NULL;
static length_fn*   old_Length          = NULL;

#ifdef IO_FS_LOGGING_ENABLED
static xstring*     4              = NULL;     // Pointer to log
#endif

static xbool        s_Initialized       = FALSE;
static s32          s_MountedCount      = 0;    

//==============================================================================

io_fs g_IOFSMgr;

//==============================================================================
// Helper functions.

void io_fs::DumpFileSystem( s32 Index )
{
    char filename[255];
    x_sprintf( filename, "dfs_list.%03d", Index );
    dfs_DumpFileListing( m_DFS[Index].pHeader, filename );
}

//==============================================================================

static void io_clean_path( char* pClean, const char* pFilename )
{
    char* pOrig = pClean;

    // Skip over leading "."
    if( *pFilename == '.' )
    {
        pFilename++;
    }

    // Skip any leading \ or /
    while( *pFilename == '\\' || *pFilename == '/' )
        pFilename++;

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

    *pClean = 0;

    x_strtoupper( pOrig );
}

//==============================================================================

static s32 io_read_sync( io_device_file* pFile, void* Buffer, s32 Offset, s32 Length )
{
    io_request      Request;
    io_open_file    OpenFile;

    OpenFile.pDeviceFile       = pFile;
    OpenFile.Offset            = 0;
    OpenFile.Length            = pFile->Length;
    OpenFile.Position          = 0;
    OpenFile.Mode              = 0; // TODO: Implement file mode.
    OpenFile.pNext             = NULL;
    OpenFile.bEnableChecksum   = FALSE;

    // Set the request
    Request.SetRequest( &OpenFile, Buffer, Offset, Length, io_request::MEDIUM_PRIORITY, FALSE, 0, 0, io_request::READ_OP );   

    // Queue it up
    g_IoMgr.QueueRequest( &Request );

    // Wait for read to finish.
    while( Request.GetStatus() < io_request::COMPLETED )
    {
#ifdef TARGET_XBOX
        Sleep( 1 );
#endif // TARGET_XBOX
    }

    // Success?
    if( Request.GetStatus() == io_request::COMPLETED )
        return Length;
    else
        return 0;
}

//==============================================================================
// X-files hooks

static X_FILE* io_open( const char* pFileName, const char* pMode )
{
    return (X_FILE*)g_IOFSMgr.Open( pFileName, pMode );
}

//==============================================================================

void io_close( X_FILE* pFile )
{
    if( ((io_open_file*)pFile)->PassThrough )
    {
        ASSERT( old_Close );
        old_Close( ((io_open_file*)pFile)->PassThrough );
        g_IOFSMgr.ReleaseFile( (io_open_file*)pFile );
    }
    else
    {
        g_IOFSMgr.Close( (io_open_file*)pFile );
    }
}

//==============================================================================

static s32 io_read( X_FILE* pFile, byte* pBuffer, s32 Bytes )
{
    if( ((io_open_file*)pFile)->PassThrough )
    {
        ASSERT( old_Read );
        return old_Read( ((io_open_file*)pFile)->PassThrough, pBuffer, Bytes );
    }
    else
    {
        return g_IOFSMgr.Read( (io_open_file*)pFile, pBuffer, Bytes );
    }
}

//==============================================================================

static s32 io_write( X_FILE* pFile, const byte* pBuffer, s32 Bytes )
{
    if( ((io_open_file*)pFile)->PassThrough )
    {
        ASSERT( old_Write );
        return old_Write( ((io_open_file*)pFile)->PassThrough, pBuffer, Bytes );
    }
    else
    {
        return g_IOFSMgr.Write( (io_open_file*)pFile, pBuffer, Bytes );
    }
}

//==============================================================================

static s32 io_seek( X_FILE* pFile, s32 Offset, s32 Origin )
{
    if( ((io_open_file*)pFile)->PassThrough )
    {
        ASSERT( old_Seek );
        return old_Seek( ((io_open_file*)pFile)->PassThrough, Offset, Origin );
    }
    else
    {
        // (s32) cast is temporary warning fix
        s32 Position = (s32)((io_open_file*)pFile)->Position;
        s32 Length   = ((io_open_file*)pFile)->Length;
        s32 Result   = 0;

        switch( Origin )
        {
            case X_SEEK_SET: 
                Position = Offset;   
                break;

            case X_SEEK_CUR: 
                Position += Offset; 
                break;

            case X_SEEK_END: 
                Position = Length + Offset;
                break;

            default:
                ASSERT( 0 );
                Result = -1;
                break;
        }

        if( Position < 0 )
        {
            Result = -1;
        }
        else
        {
            ((io_open_file*)pFile)->Position = Position;
        }

        return Result;
    }
}

//==============================================================================

static s32 io_tell( X_FILE* pFile )
{
    if( ((io_open_file*)pFile)->PassThrough )
    {
        ASSERT( old_Tell );
        return old_Tell( ((io_open_file*)pFile)->PassThrough );
    }
    else
    {
        // (s32) cast is temporary warning fix
        return (s32)((io_open_file*)pFile)->Position;
    }
}

//==============================================================================

static s32 io_flush( X_FILE* pFile )
{
    if( ((io_open_file*)pFile)->PassThrough )
    {
        ASSERT( old_Flush );
        return old_Flush( ((io_open_file*)pFile)->PassThrough );
    }
    else
    {
        return 0;
    }
}

//==============================================================================

static xbool io_eof( X_FILE* pFile )
{
    if( ((io_open_file*)pFile)->PassThrough )
    {
        ASSERT( old_EOF );
        return old_EOF( ((io_open_file*)pFile)->PassThrough );
    }
    else
    {
        return ((io_open_file*)pFile)->Position >= ((io_open_file*)pFile)->Length;    
    }
}

//==============================================================================

static s32 io_length( X_FILE* pFile )
{
    if( ((io_open_file*)pFile)->PassThrough )
    {
        ASSERT( old_Length );
        return old_Length( ((io_open_file*)pFile)->PassThrough );
    }
    else
    {
        return ((io_open_file*)pFile)->Length;
    }
}

//==============================================================================
// Class functions.

io_fs::io_fs( void )
{
    m_CurrentDFS      = -1;
    m_CurrentDFSIndex = 0;
    m_Retries         = 10;
    m_LogFlags        = 0;
}

//==============================================================================

io_fs::~io_fs( void )
{
}

//==============================================================================

xbool io_fs::Init( void )
{
    io_open_file* pOpenFile;
    s32           i;

    // Can't do this multiple times...
    ASSERT( !s_Initialized );

    // Its MINE! MINE! MINE!
    m_Mutex.Enter();

    // Initialize the free list.
    m_FreeFiles = m_Files;
    
    // Set up the file handles
    for( i=0, pOpenFile=m_Files ; i<MAX_FILES ; i++, pOpenFile++ )
    {
        // Nuke it.
        x_memset( pOpenFile, 0, sizeof(io_open_file) );

        // Build the list...
        pOpenFile->pNext = pOpenFile+1;
    }

    // Back up and terminate the list
    (--pOpenFile)->pNext = NULL;

    // Initialize the caches...
    for( i=0 ; i<NUM_CACHES ; i++ )
    {
        m_Caches[ i ].Init();
    }

    // Read old IOHooks
    x_GetFileIOHooks(  old_Open,
                       old_Close,
                       old_Read,
                       old_Write,
                       old_Seek,
                       old_Tell,
                       old_Flush,
                       old_EOF,
                       old_Length );

#if defined(TARGET_GCN) || defined(TARGET_PS2) || defined(TARGET_XBOX) || ( defined(TARGET_PC) && !defined(X_EDITOR) )
    // Set new IOHooks
    x_SetFileIOHooks(  io_open,
                       io_close,
                       io_read,
                       io_write,
                       io_seek,
                       io_tell,
                       io_flush,
                       io_eof,
                       io_length );
#endif


    // Set Initial Capacity of m_DFS now to reduce memory fragmentation
    m_DFS.SetCapacity( 16 );

    // All good now!
    s_Initialized = TRUE;

    // Ok, you can have it now!
    m_Mutex.Exit();

    return s_Initialized;
}

//==============================================================================

void io_fs::Kill( void )
{
    ASSERT( s_Initialized );

    // Unmount all the currently mounted file systems
    while( m_DFS.GetCount() > 0 )
    {
        UnmountFileSystem( m_DFS[0].PathName );
    }

    // Snag that bad boy!
    m_Mutex.Enter();

    // Restore old IOHooks
    x_SetFileIOHooks(  old_Open,
                       old_Close,
                       old_Read,
                       old_Write,
                       old_Seek,
                       old_Tell,
                       old_Flush,
                       old_EOF,
                       old_Length );
    
    // Nuke the caches.
    for( s32 i=0 ; i<NUM_CACHES ; i++ )
    {
        m_Caches[ i ].Kill();
    }

    // Let it go.
    m_Mutex.Exit();
    
    // Clear initialized
    s_Initialized = FALSE;
}

//==============================================================================

xbool io_fs::MountFileSystemRAM( const char* pPathName, void* pHeaderData, void* pRawData  )
{
    dfs_header* pHeader = NULL;
    xbool       bSuccess = FALSE;
    char        pCleanFilename[X_MAX_PATH];

    CONTEXT( "io_fs::MountFileSystemRAM" );
    MEMORY_OWNER( "io_fs::MountFileSystemRAM()" );

    // Snag that bad boy!
    m_Mutex.Enter();

    // Clean the filename.
    ASSERT( pPathName );
    io_clean_path( pCleanFilename, pPathName );

    pHeader = dfs_InitHeaderFromRawPtr(pHeaderData,0);
    if( pHeader )
    {
        s32 FileIndex;

        // Woot!
        bSuccess = TRUE;

        // Get the index.
        FileIndex = m_DFS.GetCount();

        // Add one to the list.
        m_DFS.Append();

        // Set the DFS up.
        m_DFS[FileIndex].PathName       = pPathName;
        m_DFS[FileIndex].RamAddress     = (char*)pRawData;
        m_DFS[FileIndex].SearchPriority = 0;
        m_DFS[FileIndex].pHeader        = pHeader;
        m_DFS[FileIndex].FindIndex      = -1;

        // There can only be one ;)
        ASSERT( pHeader->nSubFiles == 1 );

        // Another one is mounted...
        s_MountedCount++;

        // Let FileSystem search in latest mounted .dfs
        m_CurrentDFS = -1;
    }

    // Release the mutex!
    m_Mutex.Exit();

    // Tell the world.
    return bSuccess;
}

//==============================================================================

xbool io_fs::MountFileSystem( const char* pPathName, s32 SearchPriority )
{
    io_device_file* pFile;
    dfs_header*     pHeader = NULL;
    xbool           bSuccess = FALSE;
    char            pCleanFilename[X_MAX_PATH];

    CONTEXT( "io_fs::MountFileSystem" );
    MEMORY_OWNER( "io_fs::MountFileSystem()" );

    // Snag that bad boy!
    m_Mutex.Enter();

    // Clean the filename.
    ASSERT( pPathName );
    io_clean_path( pCleanFilename, pPathName );

    // Open the filesystem header file.
    pFile = g_IoMgr.OpenDeviceFile( xfs("%s.DFS", pCleanFilename), IO_DEVICE_DVD, io_device::READ );
    
    // Success?
    if( pFile )
    {
        // Allocate space for the header file.
#ifdef USE_VM_ALLOC
        pHeader = (dfs_header*)vm_Alloc( pFile->Length );
#else
        pHeader = (dfs_header*)x_malloc( pFile->Length );
#endif
        // Read the file system header
        if( io_read_sync( pFile, pHeader, 0, pFile->Length ) )
        {
            pHeader = dfs_InitHeaderFromRawPtr(pHeader,pFile->Length);
            if( pHeader )
            {
                bSuccess = TRUE;
#ifdef bhapgood
                {
                    static s32 I=0;
                    //dfs_DumpFileListing( pHeader, xfs("T:\\DFS_DUMP%03d.txt",I));
                    I++;
                }
#endif
            }
        }

        // Did we fail?
        if( !bSuccess )
        {
            // Free ram and nuke the pointer.
#ifdef USE_VM_ALLOC
            vm_Free( pHeader );
#else
            x_free( pHeader );
#endif
            pHeader = NULL;
        }

        // Close file and nuke it.
        g_IoMgr.CloseDeviceFile( pFile );
        pFile = NULL;
    }

    // Header file loaded successfully?
    if( bSuccess )
    {
        s32 FileIndex;
        s32 j;

        // Get the index.
        FileIndex = m_DFS.GetCount();

        // Add one to the list.
        m_DFS.Append();

        // Set the DFS up.
        m_DFS[FileIndex].PathName       = pPathName;
        m_DFS[FileIndex].RamAddress     = NULL;
        m_DFS[FileIndex].SearchPriority = SearchPriority;
        m_DFS[FileIndex].pHeader        = pHeader;
        m_DFS[FileIndex].FindIndex      = -1;

        // Make room for sub-files.
        m_DFS[FileIndex].DeviceFiles.SetCapacity( pHeader->nSubFiles );

        // Lets open up the sub-files now.
        for( j=0 ; (j<pHeader->nSubFiles) && bSuccess ; j++ )
        {
            // Open the sub-file.
            pFile = g_IoMgr.OpenDeviceFile( xfs("%s.%03d", pCleanFilename, j), IO_DEVICE_DVD, io_device::READ );

            // Only if it was found!
            if( pFile )
            {
                // Mark it as a DFS file.
                pFile->pHeader      = pHeader;
                pFile->SubFileIndex = j;

                // Put it in the list.
                m_DFS[FileIndex].DeviceFiles.Append() = pFile;
            }
            else
            {
                // Oops!
                bSuccess = FALSE;
            }
        }

        // Failure to open one?
        if( !bSuccess )
        {
            // For each opened sub-file...
            for( j=0 ; j<m_DFS[FileIndex].DeviceFiles.GetCount() ; j++ )
            {
                // Close the bad boy...
                g_IoMgr.CloseDeviceFile( m_DFS[FileIndex].DeviceFiles[j] );
            }

            // Clean it up (the following m_DFS.Delete does NOT call a destructor!)
            m_DFS[FileIndex].DeviceFiles.Clear();

            // Free the header file now.
#ifdef USE_VM_ALLOC
            vm_Free( m_DFS[FileIndex].pHeader );
#else
            x_free( m_DFS[FileIndex].pHeader );
#endif
            // Now nuke it from the list.
            m_DFS.Delete( FileIndex );
        }
    }

    // Bump filesystem count if successful.
    if( bSuccess )
    {
        s_MountedCount++;

        // Let FileSystem search in latest mounted .dfs
        m_CurrentDFS = -1;

        //DumpFileSystem( m_DFS.GetCount() - 1 );
    }

    // Release the mutex!
    m_Mutex.Exit();

    
    // Tell the world.
    return bSuccess;
}

//==============================================================================

xbool io_fs::UnmountFileSystem( const char* pPathName )
{
    xbool bSuccess = FALSE;
    s32   i;
    char  pCleanFilename[X_MAX_PATH];

    CONTEXT( "io_fs::UnmountFileSystem" );

    // Snag that bad boy!
    m_Mutex.Enter();

    // Clean the filename.
    ASSERT( pPathName );
    io_clean_path( pCleanFilename, pPathName );

    // Search thru all the disk file systems.
    for( i=0 ; i<m_DFS.GetCount() ; i++ )
    {
        // Match?
        if( x_stricmp( m_DFS[i].PathName, pPathName ) == 0 )
            break;
    }
    
    // Find it?
    if( i<m_DFS.GetCount() )
    {
        // This the current file?
        if( i == m_CurrentDFS )
        {
            m_CurrentDFS      = -1;
            m_CurrentDFSIndex = 0;
        }

        // For each opened sub-file...
        for( s32 j=0 ; j<m_DFS[i].DeviceFiles.GetCount() ; j++ )
        {
            io_device_file* pFile = m_DFS[i].DeviceFiles[j];
            ASSERT( pFile );

            // Close the bad boy...
            g_IoMgr.CloseDeviceFile( pFile );

            // Look for a file reference (should not have one).
            for( s32 k=0 ; k<MAX_FILES ; k++ )
            {
                ASSERT( m_Files[ k ].pDeviceFile != pFile );
            }
        }

        // Clean it up (the following m_DFS.Delete does NOT call a destructor!)
        m_DFS[i].DeviceFiles.Clear();

        // Free the header file now.
        if( m_DFS[i].RamAddress == NULL )
        {
#ifdef USE_VM_ALLOC
            vm_Free( m_DFS[i].pHeader );
#else
            x_free( m_DFS[i].pHeader );
#endif
        }
        // Now nuke it from the list.
        m_DFS[i].PathName.Clear();
        m_DFS[i].PathName.FreeExtra();
        m_DFS.Delete( i );

        // One less file mounted...
        s_MountedCount--;
        bSuccess = TRUE;
    }

    // Release it!
    m_Mutex.Exit();

    // Tell the world.
    return bSuccess;
}

//==============================================================================

io_open_file* io_fs::AcquireFile( void )
{
    io_open_file* pResult;

    // Snag that bad boy!
    m_Mutex.Enter();

    // Error check.
#ifndef bhapgood
    ASSERT( m_FreeFiles );
#endif

    // Take one out of the free list.
    pResult = m_FreeFiles;
    
    // Any free files left?
    if( m_FreeFiles )
    {
        // Walk the list...
        m_FreeFiles = m_FreeFiles->pNext;
    }

    // Release it!
    m_Mutex.Exit();

    // Tell the world.
    return( pResult );
}

//==============================================================================

void io_fs::ReleaseFile( io_open_file* pFile )
{
    // Error check.
    ASSERT( pFile >= &m_Files[ 0 ]         );
    ASSERT( pFile  < &m_Files[ MAX_FILES ] );

    // Snag that bad boy!
    m_Mutex.Enter();

    // Put it in the free list.
    if( (pFile >= &m_Files[ 0 ]) && (pFile < &m_Files[ MAX_FILES ]) )
    {
        // Nuke it.
        x_memset( pFile, 0, sizeof( io_open_file ) );

        // Put it in the free list...
        pFile->pNext = m_FreeFiles;
        m_FreeFiles  = pFile;
    }

    // Release it!
    m_Mutex.Exit();
}

//==============================================================================

void io_fs::SetRetries( s32 Retries )
{
    ASSERT( Retries > 0 );

    // Snag that bad boy!
    m_Mutex.Enter();

    // Set the retries.
    m_Retries = Retries;

    // Release it!
    m_Mutex.Exit();
}

//==============================================================================

void io_fs::InvalidateCaches( void )
{
    io_cache* pCache;
    s32       i;

    // Find this threads cache.
    for( i=0, pCache=m_Caches ; i<NUM_CACHES ; i++, pCache++ )
    {
        // Invalidate the cache.
        pCache->Invalidate();
    }
}

//==============================================================================

io_cache* io_fs::AcquireCache( io_open_file* pOpenFile )
{
    s32       ThreadID = x_GetThreadID();
    io_cache* pResult  = NULL;
    io_cache* pCache;
    s32       i;

    // Find this threads cache.
    for( i=0, pCache=m_Caches ; (i<NUM_CACHES && pResult==NULL) ; i++, pCache++ )
    {
        // Threads match?
        if( pCache->GetThreadID() == ThreadID )
            pResult = pCache;
    }

    // Not in the list?
    if( !pResult )
    {
        s64 SmallTicks = (s64)(((u64)(-1)) >> 1);

        // Find LRU cache then...
        for( i=0, pCache=m_Caches ; (i<NUM_CACHES && pResult==NULL) ; i++, pCache++ )
        {
            if( pCache->GetTicks() < SmallTicks )
            {
                // New smaller ticks!
                SmallTicks = pCache->GetTicks();

                // This is the one so far...
                pResult = pCache;
            }
        }
    }

    // Aquire the the cache.
    ASSERT( pResult );
    pResult->GetSemaphore()->Acquire();

    // Set ticks.
    pResult->SetTicks( x_GetTime() );

    // If different threads OR files, then invalidate the cache.
    if( (pResult->GetThreadID() != ThreadID) || x_strcmp( pResult->Filename, pOpenFile->pDeviceFile->Filename ) ) 
        pResult->Invalidate();

    // Set the last thread ID.
    pResult->SetThreadID( ThreadID );

    // Set the filename
    x_strcpy( pResult->Filename, pOpenFile->pDeviceFile->Filename );

    // Tell the world...
    return( pResult );
}

//==============================================================================

void io_fs::ReleaseCache( io_cache* pCache )
{
    ASSERT( pCache );
    pCache->GetSemaphore()->Release();
}

//==============================================================================

xbool io_fs::CompareFile( const char* pPathName, io_device_file* &DeviceFile, u32 &Offset, u32 &Length, s32 SubFile, s32 Index, void* &pRAM )
{
    dfs_header* pHeader = m_DFS[SubFile].pHeader;
    dfs_file*   pEntry  = pHeader->pFiles + Index;
    const char* p1      = pPathName;
    char*       p2;

    // Compare the path...
    p2 = pHeader->pStrings + (u32)pEntry->PathNameOffset;
    while( *p2 )
    {
        if( *p1++ != *p2++ )
            return FALSE;
    }

    // Compare the filename (part 1)...
    p2 = pHeader->pStrings + (u32)pEntry->FileNameOffset1;
    while( *p2 )
    {
        if( *p1++ != *p2++ )
            return FALSE;
    }

    // Compare the filename (part 2)...
    p2 = pHeader->pStrings + (u32)pEntry->FileNameOffset2;
    while( *p2 )
    {
        if( *p1++ != *p2++ )
            return FALSE;
    }

    // Compare the extension...
    p2 = pHeader->pStrings + (u32)pEntry->ExtNameOffset;
    while( *p2 )
    {
        if( *p1++ != *p2++ )
            return FALSE;
    }

    // Make sure terminating 0 matches...
    if( *p1 != *p2 )
        return FALSE;

    // Set the offset and length.
    Offset = pEntry->DataOffset;
    Length = pEntry->Length;

    // Now figure out which device file.
    s32             nSubFiles = pHeader->nSubFiles;
    dfs_subfile*    pSubFile  = pHeader->pSubFileTable;
    s32             i         = 0;
    while( i<nSubFiles )
    {
        // This the right file?
        if( Offset < (pSubFile[i].Offset ) )
        {
            // Ram based dfs?
            if( m_DFS[SubFile].RamAddress )
            {
                // Sneaky, sneaky!
                DeviceFile = (io_device_file*)NULL;
                pRAM = m_DFS[SubFile].RamAddress;
            }
            else
            {
                // Get the device file from the table.
                DeviceFile = m_DFS[SubFile].DeviceFiles[ i ];
                pRAM = NULL;

                // Now make the offset relative to the start of the file.
                if( i>0 )
                    Offset -= pSubFile[i-1].Offset;
            }

            // Woot!
            return TRUE;
        }
        else
        {
            // Next!!!
            i++;
        }
    }

    // Should never get here...
    ASSERT( 0 );
    return FALSE;
}

//==============================================================================

xbool io_fs::SearchDFS( const char* pPathName, io_device_file* &DeviceFile, u32 &Offset, u32 &Length, s32 SubFile, s32 StartIndex, void* &pRAM )
{
    s32 nFiles  = m_DFS[SubFile].pHeader->nFiles;
    s32 iMinus  = StartIndex;
    s32 iPlus   = StartIndex+1;

    // Default.
    m_CurrentDFSIndex = 0;

    // If it is an empty dfs - dont search.
    if( nFiles == 0 )
        return FALSE;

    // Search thru 'em all!
    while( (iMinus >= 0) || (iPlus < nFiles) )
    {
        if( iMinus >= 0 )
        {
            // Does this one match?
            if( CompareFile( pPathName, DeviceFile, Offset, Length, SubFile, iMinus, pRAM ) )
            {
                m_CurrentDFSIndex = iMinus;
                return TRUE;
            }
            else
            {
                // Search lower next time.
                iMinus--;
            }
        }

        if( iPlus < nFiles )
        {
            // Does this one match?
            if( CompareFile( pPathName, DeviceFile, Offset, Length, SubFile, iPlus, pRAM ) )
            {
                m_CurrentDFSIndex = iMinus;
                return TRUE;
            }
            else
            {
                // Search higher next time.
                iPlus++;
            }
        }
    }

    // Ack...
    return FALSE;
}

//==============================================================================

xbool io_fs::FindFile( const char* pPathName, io_device_file* &DeviceFile, u32 &Offset, u32 &Length, void* &pRAM )
{
    xbool Result = FALSE;
    char  pCleanFilename[X_MAX_PATH];

    // Snag that bad boy!
    m_Mutex.Enter();

    // Default is failure.
    DeviceFile = NULL;
    Offset     = 0;
    Length     = 0;

    // Clean the filename.
    ASSERT( pPathName );
    io_clean_path( pCleanFilename, pPathName );

    // Current DFS valid?
    if( m_CurrentDFS != -1 )
    {
        // Look in the current disk file system...
        Result = SearchDFS( pCleanFilename, DeviceFile, Offset, Length, m_CurrentDFS, m_CurrentDFSIndex, pRAM );
    }

    // Don't look further if we found it...
    if( !Result )
    {
        s32 nFileSystems = m_DFS.GetCount();
        // TODO: Prioritize the search.
        for( s32 i=nFileSystems-1 ; (i>=0) && (!Result) ; i-- )
        {
            // Don't look in the one we have already searched.
            if( i != m_CurrentDFS )
            {
                // Is it there?
                Result = SearchDFS( pCleanFilename, DeviceFile, Offset, Length, i, 0, pRAM );
                if( Result )
                    m_CurrentDFS = i;
            }
        }
    }

    // Let it go!
    m_Mutex.Exit();

    // Tell the world.
    return Result;
}

//==============================================================================

void io_fs::EnableChecksum( void* pOpenFile, xbool bEnable )
{
    ((io_open_file*)pOpenFile)->bEnableChecksum = bEnable;
}

//==============================================================================

io_open_file* io_fs::Open( const char* pPathName, const char* pMode )
{
    io_open_file* pOpenFile       = NULL;
    xbool         bRead           = FALSE;
    xbool         bWrite          = FALSE;
    xbool         bAppend         = FALSE;
    io_device*    pDevice         = NULL;
    s32           Device          = IO_DEVICE_DVD;
    xbool         bIsDVD          = TRUE;
    xbool         bOpenDeviceFile = TRUE;
    s32           OpenFlags       = 0;

    if( !s_Initialized )
        return NULL;

    // Acquire the mutex.
    m_Mutex.Enter();

    // Pick through the Mode characters.
    const char* pModeLocal = pMode;
    ASSERT( pModeLocal );
    while( *pModeLocal )
    {
        if( (*pModeLocal == 'r') || (*pModeLocal == 'R') ) { bRead   = TRUE; OpenFlags |= io_device::READ; }
        if( (*pModeLocal == 'w') || (*pModeLocal == 'W') ) { bWrite  = TRUE; OpenFlags |= io_device::WRITE; }
        if( (*pModeLocal == 'a') || (*pModeLocal == 'A') ) { bAppend = TRUE; OpenFlags |= io_device::APPEND; }
        ++pModeLocal;
    }

    // Currently we don't support Read+Write or Append
    ASSERTS( !bAppend,        "Unsupported File Open APPEND mode specified" );
 //   ASSERTS( bRead == !bWrite, "Unsupported File Open READ/WRITE mode combination specified!" ); 

    // TODO: Put in device name indentification/stripping from filenames.
    pDevice = GET_DEVICE_FROM_INDEX( Device );
    bIsDVD = (Device == IO_DEVICE_DVD); 

#ifdef DEBUG_IO
    x_DebugMsg( "FS Open: '%s'\n", pPathName );
#endif
        
    if( bIsDVD && s_MountedCount && bRead )
    {
        u32             Offset;
        u32             Length;
        io_device_file* pDeviceFile;
        void*           pRAM = NULL;

        // File found?
        if( FindFile( pPathName, pDeviceFile, Offset, Length, pRAM ) )
        {
            // Create a new openfile structure
            pOpenFile = AcquireFile();
            ASSERTS( pOpenFile, "Open: Out of file handles" );

            if( pOpenFile )
            {
                // Set up the open file
                pOpenFile->bRead             = bRead;
                pOpenFile->bWrite            = bWrite;
                pOpenFile->bAppend           = bAppend;
                pOpenFile->PassThrough       = NULL;
                pOpenFile->pDeviceFile       = pDeviceFile;
                pOpenFile->pRAM              = pRAM;
                pOpenFile->Offset            = Offset;
                pOpenFile->Length            = Length;
                pOpenFile->Position          = 0;
                pOpenFile->Mode              = OpenFlags; 
                pOpenFile->pNext             = NULL;
                pOpenFile->bEnableChecksum   = TRUE;

#ifdef IO_RETAIN_FILENAME
                // Save the filename...
                x_strncpy( pOpenFile->Filename, pPathName, 256 );
#endif
            }
        }
    }

    // Properly deal with passthru
#if !ENABLE_PASSTHROUGH
#ifndef X_RETAIL
    bOpenDeviceFile = (bRead || bWrite);
#else
    bOpenDeviceFile = bRead && (!bWrite);
#endif
#endif
    if( x_strncmp("HDD:", pPathName, 4)==0 )
    {
        bOpenDeviceFile = TRUE;
    }

#if ENABLE_PASSTHROUGH
    // File was not found? Try opening the file on the actual device.
    if( !pOpenFile && bOpenDeviceFile )
    {
        io_device_file* pDeviceFile;

#if !defined(TARGET_DVD) && defined(TARGET_PS2)
        char temp[256];
        g_IoMgr.GetDevicePathPrefix( temp, Device );
        g_IoMgr.SetDevicePathPrefix( "host:", Device );
#endif

#if !defined(TARGET_DVD) && defined(TARGET_XBOX)
        char temp[256];
        g_IoMgr.GetDevicePathPrefix( temp, Device );
        g_IoMgr.SetDevicePathPrefix( "", Device );
#endif

        // Open the file.
        pDeviceFile = g_IoMgr.OpenDeviceFile( pPathName, Device, (io_device::open_flags)OpenFlags );
        if( pDeviceFile )
        {
#ifdef IO_FS_PASS_OPEN_SUCCESS
            LOG_MESSAGE( IO_FS_PASS_OPEN_SUCCESS, "SUCCESS! Filename: %s (0x%08x)", pPathName, pOpenFile );
#endif // IO_FS_PASS_OPEN_SUCCESS

            pOpenFile = AcquireFile();
            ASSERTS( pOpenFile, "Open: Out of file handles" );
            if( pOpenFile )
            {
                // Set up the open file
                pOpenFile->bRead             = bRead;
                pOpenFile->bWrite            = bWrite;
                pOpenFile->bAppend           = bAppend;
                pOpenFile->PassThrough       = NULL;
                pOpenFile->pDeviceFile       = pDeviceFile;
                pOpenFile->pRAM              = NULL;
                pOpenFile->Offset            = 0;
                pOpenFile->Length            = pDeviceFile->Length;
                pOpenFile->Position          = 0;
                pOpenFile->Mode              = OpenFlags;
                pOpenFile->pNext             = NULL;
                pOpenFile->bEnableChecksum   = FALSE;

#ifdef IO_RETAIN_FILENAME
                // Save the filename...
                x_strncpy( pOpenFile->Filename, pPathName, 256 );
#endif // IO_RETAIN_FILENAME
            }
            else
            {
                // Close the file on the device...
                g_IoMgr.CloseDeviceFile( pDeviceFile );
            }
        }
        else
        {
#ifdef IO_FS_PASS_OPEN_FAILURE
            LOG_MESSAGE( IO_FS_PASS_OPEN_FAILURE, "FAILURE! Filename: %s (0x%08x)", pPathName, pOpenFile );
#endif // IO_FS_PASS_OPEN_SUCCESS
        }

#if !defined(TARGET_DVD) && ( defined(TARGET_PS2) || defined(TARGET_XBOX) )
        g_IoMgr.SetDevicePathPrefix( temp, Device );
#endif
    }
#endif //ENABLE_PASSTHROUGH
    if( pOpenFile )
    {
#ifdef IO_FS_OPEN_SUCCESS
        LOG_MESSAGE( IO_FS_OPEN_SUCCESS, "Filename: %s (0x%08x)", pPathName, pOpenFile );
#endif // IO_FS_OPEN_SUCCESS
    }
    else
    {
#ifdef IO_FS_OPEN_FAILURE
        LOG_MESSAGE( IO_FS_OPEN_FAILURE, "Filename: %s", pPathName );
#endif // IO_FS_OPEN_FAILURE
    }

#ifdef DEBUG_IO
    x_DebugMsg( "   Open File: %08x\n", pOpenFile );
#endif

    // Release it.
    m_Mutex.Exit();

    // Return the file handle
    return pOpenFile;
}

//==============================================================================

void io_fs::Close( io_open_file* pOpenFile )
{
    ASSERT( pOpenFile );

    // Ram based?
    if( pOpenFile && pOpenFile->pRAM )
    {
        // Snag it.
        m_Mutex.Enter();

        // Give up the file!
        ReleaseFile( pOpenFile );
    }
    else
    {
        ASSERT( pOpenFile->pDeviceFile );

        // Snag it.
        m_Mutex.Enter();

    #ifdef IO_FS_CLOSE
        LOG_MESSAGE( IO_FS_CLOSE, "Filename: %s (0x%08x)", pOpenFile->Filename, pOpenFile );
    #endif // IO_FS_CLOSE

    #ifdef DEBUG_IO
        x_DebugMsg( "FS Close: %08x\n", pOpenFile );
    #endif
        
        if( pOpenFile )
        {
            // Valid device file?
            if( pOpenFile->pDeviceFile )
            {
                // Only close it if its NOT a DFS file.
                if( !pOpenFile->pDeviceFile->pHeader )
                {
                    g_IoMgr.CloseDeviceFile( pOpenFile->pDeviceFile );
                }
            }
            else if( pOpenFile->pRAM == NULL )
            {
                ASSERTS( FALSE, "Close: NULL device file handle" );
            }

            // Give up the file!
            ReleaseFile( pOpenFile );
        }
        else
        {
            ASSERTS( FALSE, "Close: NULL file handle" );
        }
    }

    // Let it go.
    m_Mutex.Exit();
}


//==============================================================================

s32 g_IOFSReadBytesRequested=0;
s32 g_IOFSReadBytesRead=0;

s32 io_fs::Read( io_open_file* pOpenFile, byte* pBuffer, s32 Bytes )
{
    // Error check.
    ASSERT( pOpenFile );
    ASSERT( pBuffer );
    ASSERT( Bytes >= 0 );

#ifdef DEBUG_IO
    x_DebugMsg( "FS Read: %08x, Buffer: %08x, Bytes: %d\n", pOpenFile, pBuffer, Bytes );
#endif

    g_IOFSReadBytesRequested += Bytes;

    // Setup counters
    s32 BytesLeft    = Bytes;
    s32 BytesRead    = 0;
    s32 Position     = pOpenFile->Position;

    // RAM based?
    if( pOpenFile->pRAM )
    {
        // Make sure read does not pass end of file
        BytesLeft = MIN( BytesLeft, pOpenFile->Length-Position );

        // Gonna read 'em all.
        BytesRead = BytesLeft;

        // Copy the data.
        x_memcpy( pBuffer, ((char*)pOpenFile->pRAM) + pOpenFile->Offset + Position, BytesLeft );

        // Bump the position.
        Position += BytesRead;
    }
    else
    {
        // Is this a read from the file system file?
        xbool IsFileSystemFile = (pOpenFile->pDeviceFile->pHeader != NULL);

        // Make sure read does not pass end of file
        BytesLeft = MIN( BytesLeft, pOpenFile->Length-Position );

    #ifdef DEBUG_IO
        x_DebugMsg( "   Acquiring Cache\n" );
    #endif

        // Acquire a cache. 
        io_cache* pCache = AcquireCache( pOpenFile );

    #ifdef DEBUG_IO
        x_DebugMsg( "   Acquired!\n" );
    #endif

        // Loop until all bytes have been read
        while( BytesLeft > 0 )
        {
            s32 PhysicalByte = pOpenFile->Offset + Position;

            // Check if there is data in the cache, if so copy it, else fill cache.
            if( pCache->IsCacheValid() &&
                (PhysicalByte >= pCache->GetFirstByte()) &&
                (PhysicalByte <= (pCache->GetFirstByte() + pCache->GetBytesCached()-1)) )
            {
                CONTEXT("IOFS - ReadCacheHit");

                // Determine how many bytes we get from the cache
                s32 nBytes = MIN( BytesLeft, (s32)(pCache->GetFirstByte() + pCache->GetBytesCached() - PhysicalByte) );

                // Copy Data
                x_memcpy( (void*)pBuffer, &pCache->GetBuffer()[ PhysicalByte - pCache->GetFirstByte() ], nBytes );

                // Update Counters
                BytesRead += nBytes;
                BytesLeft -= nBytes;
                Position  += nBytes;
                pBuffer   += nBytes;
            }
            else
            {
                CONTEXT("IOFS - ReadCacheMiss");

                s32         SectorByte;
                s32         Offset;
                s32         SectorSize   = 32768;
                s32         ReadAttempts = 0;
                xbool       Success      = FALSE;
                io_request  Request;
                io_request* pRequest     = &Request;
                s32         BytesToCache;

                // Calculate how many bytes to read into the cache
                if( IsFileSystemFile )
                {
                    dfs_header* pHeader = (dfs_header*)pOpenFile->pDeviceFile->pHeader;
                    SectorSize = pHeader->SectorSize;
                    SectorSize = 32768;
                }

                SectorByte   = PhysicalByte - (PhysicalByte % SectorSize);
                Offset       = SectorByte;
                BytesToCache = MIN( pCache->GetCacheSize(), pOpenFile->pDeviceFile->Length - SectorByte );
                g_IOFSReadBytesRead += BytesToCache;

                // Allow for 10 retries...
                while( !Success )
                {
                    // Only try so many times before bailing...
                    if( ReadAttempts > m_Retries )
                    {
                        while( 1 )
                            x_DelayThread( 1 );
                        ASSERT( 0 );
                        goto Error;
                    }
                    
                    // Cache is now invalid...
                    pCache->Invalidate();

                    // Bump the read attempt.
                    ReadAttempts++;

                    // Set up the read request (use the request semaphore).
                    pRequest->SetRequest( pOpenFile, pCache->GetBuffer(), (s32)Offset, BytesToCache, io_request::MEDIUM_PRIORITY, TRUE, 0, 0, io_request::READ_OP ); 
                    
    #ifdef DEBUG_IO
                    x_DebugMsg( "   Requesting Read\n" );
    #endif
                    // Queue the read request.
                    g_IoMgr.QueueRequest( pRequest );

                    // Wait for read to finish...
                    pRequest->AcquireSemaphore();

    #ifdef DEBUG_IO
                    x_DebugMsg( "   Request complete!\n" );
    #endif

                    // Successful?
                    if( pRequest->GetStatus() == io_request::COMPLETED )
                    {
                        // All good!
                        Success = TRUE;

                        // Set cache bytes read.
                        pCache->SetFirstByte( SectorByte );
                        pCache->Validate( BytesToCache );
                    }
                    else
                    {
                        x_DelayThread( 1 );
                    }
                }
            }
        }

    Error:

        // Release the cache.
        ReleaseCache( pCache );
    }

    // Set new position back into open file structure
    pOpenFile->Position = Position;

    // Return number of bytes read
    return BytesRead;
}

//==============================================================================

s32 io_fs::Write( io_open_file* pOpenFile, const byte* pBuffer, s32 Bytes )
{
    io_request  Request;
    io_request* pRequest = &Request;
    s32         Result   = 0;

    // Setup counters
    s32 BytesLeft    = Bytes;
    s32 BytesWritten = 0;
    s32 Offset       = pOpenFile->Offset;
    s32 BlockSize;

    // Acquire a cache. 
    io_cache* pCache = AcquireCache( pOpenFile );

#ifdef DEBUG_IO
    x_DebugMsg( "FS Write: %08x, Buffer: %08x, Bytes: %d\n", pOpenFile, pBuffer, Bytes );
#endif
    
    while( BytesLeft )
    {
        // We can only write a maximum of the cache size.
        BlockSize = MIN( pCache->GetCacheSize(), BytesLeft );
        x_memcpy(pCache->GetBuffer(), pBuffer, BlockSize );
        // Set up the write request (use the request semaphore).
        pRequest->SetRequest( pOpenFile, pCache->GetBuffer(), Offset, BlockSize, io_request::MEDIUM_PRIORITY, TRUE, 0, 0, io_request::WRITE_OP ); 
#ifdef DEBUG_IO
        x_DebugMsg( "   Requesting Write\n" );
#endif

        // Queue the write request.
        g_IoMgr.QueueRequest( pRequest );

        // Wait for write to finish...
        pRequest->AcquireSemaphore();

#ifdef DEBUG_IO
        x_DebugMsg( "   Request complete!\n" );
#endif
        Offset      += BlockSize;
        BytesLeft   -= BlockSize;
        pBuffer     += BlockSize;
        BytesWritten+= BlockSize;
    }
    ReleaseCache( pCache );
    // Successful?
    if( pRequest->GetStatus() == io_request::COMPLETED )
    {
        pOpenFile->Offset += Bytes;
        if( pOpenFile->Offset > pOpenFile->Length )
        {
            pOpenFile->Length = pOpenFile->Offset;
        }
        // All good!
        Result = Bytes;
    }

    return Result;
}

//==============================================================================

const char* io_fs::GetFileName( const X_FILE* pFile )
{
    ASSERTS( s_Initialized, "Rob's file system is not initialized" );

    if (NULL == pFile)
        return "-No File Handle-";
#ifdef IO_RETAIN_FILENAME
    return ((io_open_file*)pFile)->Filename;
#else
    return "-No Filename Available-";
#endif
}

//==============================================================================

s32 io_fs::GetFileSystemIndex( const char* pPathName )
{
    s32   i;

    // Snag that bad boy!
    m_Mutex.Enter();

    // Clean the filename.
    ASSERT( pPathName );

    // Search thru all the disk file systems.
    for( i=0 ; i<m_DFS.GetCount() ; i++ )
    {
        // Match?
        if( x_stricmp( m_DFS[i].PathName, pPathName ) == 0 )
            break;
    }

    m_Mutex.Exit();

    if( i==m_DFS.GetCount() )
        return -1;

    return i;
}

//==============================================================================

s32 io_fs::GetNFilesInFileSystem( s32 iFileSystem )
{
    if( iFileSystem == -1 )
        return 0;

    ASSERT( (iFileSystem>=0) && (iFileSystem<m_DFS.GetCount()) );

    s32 N = m_DFS[iFileSystem].pHeader->nFiles;

    return N;
}

//==============================================================================

void io_fs::GetFileNameInFileSystem( s32 iFileSystem, s32 iFile, char* pFileName )
{
    ASSERT( pFileName );

    if( iFileSystem == -1 )
    {
        pFileName[0] = 0;
        return;
    }

    ASSERT( (iFileSystem>=0) && (iFileSystem<m_DFS.GetCount()) );
    ASSERT( (iFile>=0) && (iFile<m_DFS[iFileSystem].pHeader->nFiles) );

    dfs_BuildFileName( m_DFS[iFileSystem].pHeader, iFile, pFileName );
}

