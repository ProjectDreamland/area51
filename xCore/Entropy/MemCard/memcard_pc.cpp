#include "entropy.hpp"
#include "..\entropy\e_Memcard.hpp"
#include "winerror.h"

//==============================================================================
//=================================== Locals ===================================
//==============================================================================

static xbool s_Initialized      = FALSE; // FLAG: Has the Init function been called?
static xbool s_InitHardware     = TRUE;  // FLAG: Initialize hardware inside the Init funtcion?
static xbool s_DispatcherActive = FALSE; // FLAG: Is the dispatcher active? (prevents re-entry).

//==============================================================================
//=================== Hardware Specific Defines, Enums, etc ====================
//==============================================================================

// Root directory for the memory card
#define PC_MEMCARD_ROOT_DIR "C:\\GameData\\A51\\Release\\PC\\MEMCARD\\"

// Maximum number of memory cards the system supports.
#define MAX_MEMCARDS (2) 

// Size of the message queue.
#define MAX_MESSAGES (6) 

// Maximum number of files (nintendo limit).
#define MEMCARD_MAX_FILES (127)

// Valid messages for the Dispatcher.
enum memcard_message 
{
    MSG_INITIATE = 0,
    MSG_PROCESS,
    MSG_COMPLETE,
    MSG_ERROR,
};

// Make DAMN sure that error_extensions do not 
// collide with the systems native error codes.
// On GCN, all error codes are <= 0.
enum error_extensions 
{
    EXT_CARD_RESULT_INCOMPATIBLE = 1,
    EXT_CARD_RESULT_REMOVED,
};

enum memcard_file_flags 
{
    MEMCARD_FILE_EXISTS =       (1<<0),
    MEMCARD_FILE_NOPERMISSION = (1<<1),
};

// Error mapping structure.
struct error_map 
{
    memcard_error AbstractError;
    s32           HardwareError;
};

struct pc_save_file
{
    s32     Checksum;
    char    Comment[128];
    char    Preferences[128];
    u8        Icon[2048];
};

static pc_save_file* s_pFile = NULL; 

static xbool CheckError( s32 HardwareError, xbool bProcessOnNoError = FALSE );

//==============================================================================
//========================== Hardware Specific Data ============================
//==============================================================================

static xarray<s32> s_IgnoreList;

static u32* s_FileFlags;

static xarray<mc_file_info> s_FileInfo;

//==============================================================================
//============================= Hardware Functions =============================
//==============================================================================

//------------------------------------------------------------------------------

static void ClearIgnoreList( void )
{
}

//==============================================================================
//================================= Dispatcher =================================
//==============================================================================

//------------------------------------------------------------------------------

void MemcardDispatcher( void )
{
    // Error check.
    ASSERT( !s_DispatcherActive );

    // Set semaphore.
    s_DispatcherActive = TRUE;

    // Do this a whole lot....
    while( 1 )
    {   
        // So lonely...waiting by the phone for someone to call!!!!
        memcard_message Message = (memcard_message)((s32)g_MemcardHardware.m_pDispatcherMQ->Recv( MQ_BLOCK ));

        // Someone called! OMG! What should I do?
        switch( Message )
        {
            case MSG_INITIATE:
                // We are in progress now!
                g_MemcardHardware.SetState( MEMCARD_IN_PROGRESS );

                // Lets get started! (0 means start the process)
                g_MemcardHardware.SetSubState( 0 );
                
                // Ok process the bad boy.
                g_MemcardHardware.Process();
                break;

            case MSG_PROCESS:
                // Next state.
                g_MemcardHardware.SetSubState( g_MemcardHardware.GetSubState() + 1 );

                // Ok process the bad boy.
                g_MemcardHardware.Process();
                break;

            case MSG_COMPLETE:
                // Woot! All good!
                g_MemcardHardware.SetState( MEMCARD_SUCCESS );
                g_MemcardHardware.SetOperation( MEMCARD_OP_IDLE );
                break;

            case MSG_ERROR:
                // Opps...something bad.
                g_MemcardHardware.SetState( MEMCARD_FATAL_ERROR );
                g_MemcardHardware.SetOperation( MEMCARD_OP_IDLE );
                break;

            default:
                // Should never get here.
                ASSERT( 0 );
        }
    }
}

//==============================================================================
//=============================== Class Functions ==============================
//==============================================================================

//------------------------------------------------------------------------------

memcard_hardware::memcard_hardware( void )
{
}

//------------------------------------------------------------------------------

memcard_hardware::~memcard_hardware( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::Init( void )
{
    // Error check.
    ASSERT( !s_Initialized );

    // Can only init the hardware once on the GameCube.
    if( s_InitHardware )
    {
        // Don't do it again...
        s_InitHardware = FALSE;
    }

    m_pDispatcherMQ      = new xmesgq(MAX_MESSAGES);
    m_Error              = MEMCARD_SUCCESS;
    m_Operation          = MEMCARD_OP_IDLE;
    m_RequestedCard      = NO_CARD;
    m_pRequestedBuffer   = NULL;
    m_nRequestedBytes    = 0;
    m_RequestedOffset    = 0;
    m_MountedCard        = NO_CARD;
    m_SectorSize         = 0;
    m_FileLength         = -1;

    // Don't ignore any errors.
    ClearIgnoreList();

    // Allocate file flag buffer
    s_FileFlags = (u32*)x_malloc( MEMCARD_MAX_FILES * sizeof(u32) );
    ASSERT( s_FileFlags );
    x_memset( s_FileFlags, 0, MEMCARD_MAX_FILES * sizeof(u32) );

    // Nuke the fileinfo.
    s_FileInfo.Clear();

    // TODO: Allocate the work area (currently static).

    // Create the memcard_hardware thread.
    m_pThread = new xthread( MemcardDispatcher, "memcard_mgr dispatcher", 8192, 1 );

    // Ok all good now!
    s_Initialized = TRUE;
}

//------------------------------------------------------------------------------

void memcard_hardware::Kill( void )
{
    // Error check.
    ASSERT( s_Initialized );

    // Nuke the thread.
    delete m_pThread;
    delete m_pDispatcherMQ;

    s_DispatcherActive = FALSE;

    // Free the file flag buffer.
    if( s_FileFlags )
    {
        x_free( s_FileFlags );
        s_FileFlags = NULL;
    }

    // Nuke the fileinfo.
    s_FileInfo.Clear();
    s_FileInfo.FreeExtra();

    // TODO: Free the work area (currently static).

    // All bad now...
    s_Initialized = FALSE;
}

//------------------------------------------------------------------------------

void memcard_hardware::SendMessage( s32 Message )
{
    // Error check.
    ASSERT( s_Initialized );

    // A message in a bottle...send it to the dispatcher.
    s32 MessageStatus = m_pDispatcherMQ->Send( (void*)Message, MQ_NOBLOCK );

    // Make sure it was sent!
    ASSERT( MessageStatus );

    // No nasty warnings in release.
    (void)MessageStatus;
}

//------------------------------------------------------------------------------

void memcard_hardware::SetIOParams( const char* FileName, byte* pBuffer, s32 Offset, s32 nBytes )
{
    // Error check.
    ASSERT( s_Initialized );

    ASSERT( FileName );
    x_strcpy( m_pRequestedFileName,FileName );

    m_pRequestedBuffer   = pBuffer;
    m_nRequestedBytes    = nBytes;
    m_RequestedOffset    = Offset;
}

//------------------------------------------------------------------------------

static u32 s_LastSize = 0;
static char* s_pIOBuffer = NULL;

void memcard_hardware::AllocIOBuffer( void )
{
    if( (s_pIOBuffer == NULL) || ( m_SectorSize != s_LastSize ) )
    {
        // Free the old one.
        FreeIOBuffer();

        // Allocate a new one.
        s_pIOBuffer = (char*)x_malloc( m_SectorSize );

        // Retain last size.
        s_LastSize = m_SectorSize;
    }    
}

//------------------------------------------------------------------------------

void memcard_hardware::FreeIOBuffer( void )
{
    if( s_pIOBuffer )
    {
        x_free( s_pIOBuffer );
        s_pIOBuffer = NULL;
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::SetOperation( memcard_op Operation )
{
    // Error check.
    ASSERT( s_Initialized );

    // Set the operation.
    m_Operation = Operation;
}

//------------------------------------------------------------------------------

void memcard_hardware::InitiateOperation( void )
{
    // Error check.
    ASSERT( s_Initialized );

    // Can't have 2 concurrent operations.
    ASSERT( GetState() != MEMCARD_IN_PROGRESS ); 

    // Don't ignore any errors.
    ClearIgnoreList();

    // Free the IO Buffer!
    FreeIOBuffer();

    // We have ignition...er or something like that...
    SendMessage( MSG_INITIATE );
}

//------------------------------------------------------------------------------

s32 memcard_hardware::GetFileList( xarray<mc_file_info>& FileList )
{
    FileList.Clear();
    
    if( m_bIsFileListValid )
    {
        FileList.SetCapacity( m_nFileCount );
        
        for( s32 i=0 ; i<m_nFileCount ; i++ )
        {
            FileList.Append() = s_FileInfo[i];
        }
        
        return m_nFileCount;
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::InvalidateFileList( void )
{
    // Invalid the file list.
    m_bIsFileListValid = FALSE;
    m_nFileCount       = 0;

    // Clear the file info.
    s_FileInfo.Clear();
}

//------------------------------------------------------------------------------

void memcard_hardware::Process( void )
{
    // Error check.
    ASSERT( s_Initialized );
    
    // Lets figure out what to do!
    switch( GetOperation() )
    {
    case MEMCARD_OP_MOUNT:
        ProcessMount();
        break;
        
    case MEMCARD_OP_UNMOUNT:
        ProcessUnmount();
        break;
        
    case MEMCARD_OP_READ_FILE:
        ProcessReadFile();
        break;
        
    case MEMCARD_OP_WRITE_FILE:
        ProcessWriteFile();
        break;
        
    case MEMCARD_OP_DELETE_FILE:
        ProcessDeleteFile();
        break;
        
    case MEMCARD_OP_FORMAT:
        ProcessFormat();
        break;
        
    case MEMCARD_OP_REPAIR:
        ProcessRepair();
        break;
        
    case MEMCARD_OP_READ_FILE_LIST:
        ProcessReadFileList();
        break;
        
    case MEMCARD_OP_PURGE_FILE_LIST:
        ProcessPurgeFileList();
        break;
        
    case MEMCARD_OP_GET_FILE_LENGTH:
        ProcessGetFileLength();
        break;

    case MEMCARD_OP_READ:
        ProcessRead();
        break;
    
    case MEMCARD_OP_WRITE:
        ProcessWrite();
        break;
    
    case MEMCARD_OP_CREATE_FILE:
        ProcessCreateFile();
        break;

    case MEMCARD_OP_CREATE_DIR:
        ProcessCreateDir();
        break;

    case MEMCARD_OP_SET_DIR:
        ProcessSetDir();
        break;

    case MEMCARD_OP_DELETE_DIR:
        ProcessDeleteDir();
        break;

    case MEMCARD_OP_IDLE:
    default:
        // Um, how did this happen?
        ASSERT( 0 );
        break;
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessCreateDir( void )
{
    CreateDirectory(
        xfs("C:\\GAMEDATA\\A51\\RELEASE\\PC\\MEMCARD\\%s",m_pRequestedDirName ),
        NULL
    );
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessSetDir( void )
{
    x_strcpy( m_pRequestedDirName, m_pRequestedFileName );
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessDeleteDir( void )
{
    ProcessReadFileList();
    s32 i,n = s_FileInfo.GetCount();
    for( i=0;i<n;i++ )
        DeleteFile(
            xfs("C:\\GAMEDATA\\A51\\RELEASE\\PC\\MEMCARD\\%s\\%s",
                m_pRequestedDirName,
                s_FileInfo[i].FileName
            )
        );
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessMount( void )
{
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessUnmount( void )
{
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessReadFile( void )
{
    HANDLE hFile = CreateFile(
        xfs("C:\\GAMEDATA\\A51\\RELEASE\\PC\\MEMCARD\\%s\\%s",
            m_pRequestedDirName,
            m_pRequestedFileName
        ),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if( hFile==INVALID_HANDLE_VALUE )
    {
        SendMessage( MSG_ERROR );
        return;
    }
    DWORD BytesRead;
    BOOL bReadResult = ReadFile(
        hFile,
        m_pRequestedBuffer,
        m_nRequestedBytes,
        &BytesRead,
        NULL
    );
    if( ! bReadResult )
    {
        SendMessage( MSG_ERROR );
        return;
    }
    SendMessage( MSG_COMPLETE );
    CloseHandle( hFile );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessWriteFile( void )
{
    HANDLE hFile = CreateFile(
        xfs("C:\\GAMEDATA\\A51\\RELEASE\\PC\\MEMCARD\\%s\\%s",
            m_pRequestedDirName,
            m_pRequestedFileName
        ),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        0,
        NULL
    );
    if( hFile==INVALID_HANDLE_VALUE )
    {
        SendMessage( MSG_ERROR );
        return;
    }
    DWORD BytesWritten;
    BOOL bWriteResult = WriteFile(
        hFile,
        m_pRequestedBuffer,
        m_nRequestedBytes,
        &BytesWritten,
        NULL
    );
    if( ! bWriteResult )
    {
        SendMessage( MSG_ERROR );
        return;
    }
    SendMessage( MSG_COMPLETE );
    CloseHandle( hFile );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessDeleteFile( void )
{
    DeleteFile(
        xfs("C:\\GAMEDATA\\A51\\RELEASE\\PC\\MEMCARD\\%s\\%s",
            m_pRequestedDirName,
            m_pRequestedFileName
        )
    );
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessFormat( void )
{
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessReadFileList( void )
{
    WIN32_FIND_DATA Fd;
    HANDLE hFind = FindFirstFile( m_pRequestedDirName,&Fd );
    if( hFind==INVALID_HANDLE_VALUE )
        SendMessage( MSG_ERROR );
    else
    {
        s_FileInfo.Clear();
        s32 Index = 0;
        do
        {   mc_file_info& Info = s_FileInfo.Append();
            x_strcpy( Info.FileName, Fd.cFileName );
            Info.Length = Fd.nFileSizeLow;
            Info.Index  = Index++;
        }
        while( FindNextFile( hFind,&Fd ));
        SendMessage( MSG_COMPLETE );
        FindClose( hFind );
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessPurgeFileList( void )
{
    // Nuke it.
    InvalidateFileList();
    
    // Done!
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessGetFileLength( void )
{
    HANDLE hFile = CreateFile(
        xfs("C:\\GAMEDATA\\A51\\RELEASE\\PC\\MEMCARD\\%s\\%s",
            m_pRequestedDirName,
            m_pRequestedFileName
        ),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if( hFile==INVALID_HANDLE_VALUE )
    {
        SendMessage( MSG_ERROR );
        return;
    }
    DWORD dwResult = GetFileSize( hFile,NULL );
    SendMessage( MSG_COMPLETE );
    SetFileLength( dwResult );
    CloseHandle( hFile );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessRepair( void )
{
    // do nothing!
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

s32 memcard_hardware::GetMaxCards( void )
{
    return 1;
}

//------------------------------------------------------------------------------

xbool memcard_hardware::IsCardConnected( s32 CardID )
{
    return TRUE;
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessCreateFile( void )
{
BREAK
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessWrite( void )
{
BREAK;
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessRead( void )
{
    BREAK;
}

//------------------------------------------------------------------------------

u32 memcard_hardware::GetFreeSpace( void )
{
    DWORD TotalNumberOfClusters;
    DWORD NumberOfFreeClusters;
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;

    GetDiskFreeSpace
    (
        "C:\\",
        &SectorsPerCluster,
        &BytesPerSector,
        &NumberOfFreeClusters,
        &TotalNumberOfClusters
    );
    return NumberOfFreeClusters*SectorsPerCluster*BytesPerSector;
}
