#include "entropy.hpp"
#include "..\entropy\e_Memcard.hpp"
#include "winerror.h"

//==============================================================================
//=================================== Locals ===================================
//==============================================================================

static xbool s_Initialized      = FALSE; // FLAG: Has the Init function been called?
static xbool s_InitHardware     = TRUE;  // FLAG: Initialize hardware inside the Init funtcion?
static xbool s_DispatcherActive = FALSE; // FLAG: Is the dispatcher active? (prevents re-entry).
#define SWITCH_LABEL(x) case x: return #x;


//==============================================================================
//=================== Hardware Specific Defines, Enums, etc ====================
//==============================================================================

// Root directory for the memory card
#define PC_MEMCARD_ROOT_DIR "C:\\GameData\\A51\\Release\\PC\\MEMCARD\\"

// Maximum number of memory cards the system supports.
#define MAX_MEMCARDS 1 

// Size of the message queue.
#define MAX_MESSAGES 6 

// Maximum number of files (nintendo limit).
#define MEMCARD_MAX_FILES 127

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

struct xbox_save_file
{
    s32     Checksum;
	char	Comment[128];
    char    Preferences[128];
	u8		Icon[2048];
};

static xbox_save_file* s_pFile = NULL; 
static u32 s_ErrCode;

static xbool CheckError( s32 HardwareError, xbool bProcessOnNoError = FALSE );

static char s_SaveDir[384];

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

static void ProcessInternalError( void )
{
    memcard_error State;

    // Defined in WinError.H
    switch( s_ErrCode )
    {
        case ERROR_ACCESS_DENIED:
            State = MEMCARD_ACCESS_DENIED;
            break;

        case ERROR_PATH_NOT_FOUND:
        case ERROR_FILE_NOT_FOUND:
            State = MEMCARD_FILE_NOT_FOUND;
            break;

        case ERROR_DISK_FULL:
            State = MEMCARD_FULL;
            break;

        default:
            State = MEMCARD_FATAL_ERROR;
            break;
    }
    g_MemcardHardware.SetOperation( MEMCARD_OP_IDLE );
    g_MemcardHardware.SetState( State );
}

//------------------------------------------------------------------------------

void MemcardDispatcher( void )
{
    // Error check.
    ASSERT( !s_DispatcherActive );

    // Set semaphore.
    s_DispatcherActive = TRUE;

    // Do this a whole lot....
    while( x_GetCurrentThread()->IsActive() )
    {   
        // So lonely...waiting by the phone for someone to call!!!!
        memcard_message Message = (memcard_message)((s32)g_MemcardHardware.m_pDispatcherMQ->Recv( MQ_BLOCK ));

        // Die if the thread was told it needed to kill itself
        if( x_GetCurrentThread()->IsActive()==FALSE )
        {
            break;
        }

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
                g_MemcardHardware.SetOperation( MEMCARD_OP_IDLE );
                g_MemcardHardware.SetState( MEMCARD_SUCCESS );
                break;

            case MSG_ERROR:
                // Opps...something bad.
                ProcessInternalError();
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

const char* GetStateName( memcard_op Operation )
{
    switch( Operation )
    {
        SWITCH_LABEL(MEMCARD_OP_IDLE);
        SWITCH_LABEL(MEMCARD_OP_MOUNT);
        SWITCH_LABEL(MEMCARD_OP_UNMOUNT);
        SWITCH_LABEL(MEMCARD_OP_READ_FILE);
        SWITCH_LABEL(MEMCARD_OP_WRITE_FILE);
        SWITCH_LABEL(MEMCARD_OP_DELETE_FILE);
        SWITCH_LABEL(MEMCARD_OP_FORMAT);
        SWITCH_LABEL(MEMCARD_OP_REPAIR);
        SWITCH_LABEL(MEMCARD_OP_READ_FILE_LIST);
        SWITCH_LABEL(MEMCARD_OP_PURGE_FILE_LIST);
        SWITCH_LABEL(MEMCARD_OP_GET_FILE_LENGTH);
        SWITCH_LABEL(MEMCARD_OP_READ);
        SWITCH_LABEL(MEMCARD_OP_WRITE);
        SWITCH_LABEL(MEMCARD_OP_CREATE_FILE);
        SWITCH_LABEL(MEMCARD_OP_CREATE_DIR);
        SWITCH_LABEL(MEMCARD_OP_SET_DIR);
        SWITCH_LABEL(MEMCARD_OP_DELETE_DIR);
    default: return "<unknown>";
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::SetOperation( memcard_op Operation )
{
    // Error check.
    ASSERT( s_Initialized );

    if( m_Operation != Operation )
    {
    //  LOG_MESSAGE( "memcard_hardware::SetOperation", "Operation set from %s to %s", GetStateName(m_Operation), GetStateName(Operation) );
    }
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

    m_Error = MEMCARD_IN_PROGRESS;

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
    x_strcpy( m_pRequestedDirName, m_pRequestedFileName );
    s_ErrCode = XCreateSaveGame(
        "U:\\",
        xwstring( m_pRequestedDirName ),
        OPEN_ALWAYS,
        XSAVEGAME_NOCOPY,
        s_SaveDir,
        384
    );
//  LOG_MESSAGE( "memcard_hardware::ProcessCreateDir", "Create directory:%s, result:%d", m_pRequestedDirName, ErrCode );
    u32 Result;
    if( s_ErrCode == ERROR_SUCCESS )
    	Result = MSG_COMPLETE;
    else
        Result = MSG_ERROR;
    SendMessage( Result );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessSetDir( void )
{
    x_strcpy( m_pRequestedDirName, m_pRequestedFileName );
    s_ErrCode = XCreateSaveGame(
        "U:\\",
        xwstring( m_pRequestedDirName ),
        OPEN_EXISTING,
        0,
        s_SaveDir,
        384
    );
    u32 Result;
//  LOG_MESSAGE( "memcard_hardware::ProcessSetDir", "Set Directory:%s, result:%d", m_pRequestedDirName, ErrCode );
    if( s_ErrCode == ERROR_SUCCESS )
    	Result = MSG_COMPLETE;
    else
        Result = MSG_ERROR;
    SendMessage( Result );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessDeleteDir( void )
{
    x_strcpy( m_pRequestedDirName, m_pRequestedFileName );

    s_ErrCode = XDeleteSaveGame( "U:\\",xwstring( m_pRequestedDirName ));
//  LOG_MESSAGE( "memcard_hardware::ProcessDeleteDir", "Delete Directory:%s, result:%d", m_pRequestedDirName, ErrCode );

    u32 Result;
    if( s_ErrCode == ERROR_SUCCESS )
    	Result = MSG_COMPLETE;
    else
        Result = MSG_ERROR;
    SendMessage( Result );
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
        xfs("%s%s",
            s_SaveDir,
            m_pRequestedFileName
        ),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if( hFile!=INVALID_HANDLE_VALUE )
    {
        u32 FileSize = GetFileSize( hFile,NULL );
        if( FileSize != m_nRequestedBytes )
        {
            s_ErrCode = -1;
            SendMessage( MSG_ERROR );
            CloseHandle( hFile );
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
    //  LOG_MESSAGE( "memcard_hardware::ProcessReadFile", "Read File:%s%s, Length:%d, Handle:%d, Result:%d", s_SaveDir, m_pRequestedFileName, m_nRequestedBytes, hFile, bReadResult );
        if( bReadResult && (BytesRead==m_nRequestedBytes) )
        {
            SendMessage( MSG_COMPLETE );
            CloseHandle( hFile );
            return;
        }
        CloseHandle( hFile );
    }
    s_ErrCode = -1;
    SendMessage( MSG_ERROR );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessWriteFile( void )
{
    HANDLE hFile = CreateFile(
        xfs("%s%s",
            s_SaveDir,
            m_pRequestedFileName
        ),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        0,
        NULL
    );

    if( hFile!=INVALID_HANDLE_VALUE )
    {
        DWORD BytesWritten;
        BOOL bWriteResult = WriteFile(
            hFile,
            m_pRequestedBuffer,
            m_nRequestedBytes,
            &BytesWritten,
            NULL
        );
    //  LOG_MESSAGE( "memcard_hardware::ProcessWriteFile", "Write File:%s%s, Length:%d, Handle:%d, Result:%d", s_SaveDir, m_pRequestedFileName, m_nRequestedBytes, hFile, bWriteResult );
        if( bWriteResult && (BytesWritten==m_nRequestedBytes) )
        {
            SendMessage( MSG_COMPLETE );
            CloseHandle( hFile );
            return;
        }
        CloseHandle( hFile );
    }
    s_ErrCode = -1;
    SendMessage( MSG_ERROR );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessDeleteFile( void )
{
BREAK;
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessFormat( void )
{
BREAK;
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessReadFileList( void )
{
    XGAME_FIND_DATA Fd;
    HANDLE hFind = XFindFirstSaveGame( "U:\\",&Fd );

    // start by clearing the current file list
    InvalidateFileList();

    // check for save games
    if( hFind!=INVALID_HANDLE_VALUE )
    {
        s32 Index = 0;
        do
        {   mc_file_info& Info = s_FileInfo.Append();
            xstring Filename( Fd.szSaveGameName );
            const char* pName;

            pName = (const char*)Filename;
            while( *pName=='/' )
            {
                pName++;
            }
            x_strcpy( Info.FileName, pName );
            Info.Length         = Fd.wfd.nFileSizeLow;
            Info.Index          = Index++;
            FileTimeToLocalFileTime( &Fd.wfd.ftCreationTime, (FILETIME*)&Info.CreationDate );
            FileTimeToLocalFileTime( &Fd.wfd.ftLastWriteTime,(FILETIME*)&Info.ModifiedDate );
        }
        while( XFindNextSaveGame( hFind,&Fd ));
        m_nFileCount = Index;
        XFindClose( hFind );
    }
    SendMessage( MSG_COMPLETE );
    m_bIsFileListValid = TRUE;
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
        xfs("%s%s",
            s_SaveDir,
            m_pRequestedFileName
        ),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        0,
        NULL
    );
    if( hFile!=INVALID_HANDLE_VALUE )
    {
        DWORD dwResult = GetFileSize( hFile,NULL );
        SendMessage( MSG_COMPLETE );
        SetFileLength( dwResult );
        CloseHandle( hFile );
        return;
    }
    s_ErrCode = -1;
    SendMessage( MSG_ERROR );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessRepair( void )
{
BREAK;
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
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes > 0 );
    ASSERT( m_RequestedOffset >= 0 );

    HANDLE hFile = CreateFile(
        xfs("%s%s",
            s_SaveDir,
            m_pRequestedFileName
        ),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        0,
        NULL
    );
    if( hFile!=INVALID_HANDLE_VALUE )
    {
        DWORD BytesWritten;
        SetFilePointer( hFile,m_RequestedOffset,NULL,FILE_BEGIN );
        BOOL bWriteResult = WriteFile(
            hFile,
            m_pRequestedBuffer,
            m_nRequestedBytes,
            &BytesWritten,
            NULL
        );
        if( bWriteResult )
        {
            SendMessage( MSG_COMPLETE );
            CloseHandle( hFile );
            return;
        }
        CloseHandle( hFile );
    }
    s_ErrCode = -1;
    SendMessage( MSG_ERROR );
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessRead( void )
{
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes > 0 );
    ASSERT( m_RequestedOffset >= 0 );

    HANDLE hFile = CreateFile(
        xfs("%s%s",
            s_SaveDir,
            m_pRequestedFileName
        ),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if( hFile!=INVALID_HANDLE_VALUE )
    {
        DWORD BytesRead;
        SetFilePointer( hFile,m_RequestedOffset,NULL,FILE_BEGIN );
        BOOL bReadResult = ReadFile(
            hFile,
            m_pRequestedBuffer,
            m_nRequestedBytes,
            &BytesRead,
            NULL
        );
        if( bReadResult )
        {
            SendMessage( MSG_COMPLETE );
            CloseHandle( hFile );
            return;
        }
        CloseHandle( hFile );
    }
    s_ErrCode = -1;
    SendMessage( MSG_ERROR );
}

//------------------------------------------------------------------------------

u32 memcard_hardware::GetFreeSpace( void )
{
    ULARGE_INTEGER FreeBytesAvailable;
    ULARGE_INTEGER TotalNumberOfBytes;
    ULARGE_INTEGER TotalNumberOfFreeBytes;

    GetDiskFreeSpaceEx
    (
        "U:\\",
        &FreeBytesAvailable,
        &TotalNumberOfBytes,
        &TotalNumberOfFreeBytes
    );
    if (FreeBytesAvailable.HighPart > 0)
        return S32_MAX;

    return MIN(S32_MAX,FreeBytesAvailable.LowPart);
}
