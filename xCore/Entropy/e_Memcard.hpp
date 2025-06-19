#ifndef MEMCARD_HPP
#define MEMCARD_HPP

#include "x_types.hpp"
#include "x_threads.hpp"
//-----------------------------------------------------------------------------

enum memcard_error 
{                               // gcn ps2 xbx  pc
    MEMCARD_END_OF_LIST = -1,   // --- --- --- ---
    MEMCARD_SUCCESS,            //  x   x           gcn::CARD_RESULT_READY       - Ready to start the next operation.
    MEMCARD_IN_PROGRESS,        //  x   x                                        - Operation is in progress.
    MEMCARD_FATAL_ERROR,        //  x   x           gcn::CARD_RESULT_FATAL_ERROR - Error due to program design (e.g., parameter range error, etc.)
    MEMCARD_BUSY,               //  x   x           gcn::CARD_RESULT_BUSY        - Busy
    MEMCARD_NOT_A_MEMCARD,      //  x   x           gcn::CARD_RESULT_WRONGDEVICE - A device is detected, but it is not a memory card. 
    MEMCARD_NO_CARD,            //  x   x           gcn::CARD_RESULT_NOCARD      - Memory card is not detected (or not mounted yet).
    MEMCARD_WORN_OUT,           //  x               gcn::CARD_RESULT_IOERROR     - Memory card has reached limit of useable life. 
    MEMCARD_WRONG_REGION,       //  x               gcn::CARD_RESULT_ENCODING    - Character set encoding is mismatched.     
    MEMCARD_DAMAGED,            //  x   x           gcn::CARD_RESULT_BROKEN      - File system is broken.
    MEMCARD_FILE_NOT_FOUND,     //  x   x           gcn::CARD_RESULT_NOFILE      - Specified file was not found.
    MEMCARD_FILE_ALREADY_EXISTS,//  x   x           gcn::CARD_RESULT_EXIST       - The filename about to be created/renamed already exists. 
    MEMCARD_NO_FILES_AVAILABLE, //  x   x           gcn::CARD_RESULT_NOENT       - No more free directory entries. 
    MEMCARD_NOT_ENOUGH_SPACE,   //  x   x           gcn::CARD_RESULT_INSSPACE    - Insufficient free space in data blocks. 
    MEMCARD_ACCESS_DENIED,      //  x   x           gcn::CARD_RESULT_NOPERM      - No file access permission. 
    MEMCARD_PAST_END_OF_FILE,   //  x   x           gcn::CARD_RESULT_LIMIT       - Tried to read/write over the file size limit. 
    MEMCARD_FILENAME_TOO_LONG,  //  x   x           gcn::CARD_RESULT_NAMETOOLONG - The filename about to be created/renamed is too long. 
    MEMCARD_IO_CANCELED,        //  x   x           gcn::CARD_RESULT_CANCELED    - The read/write operation is canceled.
    MEMCARD_INCOMPATIBLE,       //  x   x           extension                    - non-8k sector sized card was found. 
    MEMCARD_CARD_CHANGED,       //      x                                        - the memory card was changed
    MEMCARD_UNFORMATTED,        //      x                                        - the memory card is unformatted
    MEMCARD_FULL,               //      x                                        - the memory card is FULL (different to not enough space)
};

/*==============================================================================

================================================================================
============================ Error Codes by Function ===========================
================================================================================

//-----------------------------------------------------------------------------

AsyncMount:                    

    Error code                  gcn ps2 xbx  pc
    --------------------------- --- --- --- ---

    MEMCARD_IN_PROGRESS          x   x
    MEMCARD_SUCCESS              x   x
    MEMCARD_FATAL_ERROR          x   x
    MEMCARD_NOT_A_MEMCARD        x   
    MEMCARD_NO_CARD              x   x
    MEMCARD_BUSY                 x   x
    MEMCARD_WORN_OUT             x
    MEMCARD_DAMAGED              x   x          - This error code indicates the card was succesfully mounted on the GCN.
    MEMCARD_WRONG_REGION         x              - This error code indicates the card was succesfully mounted on the GCN.
    MEMCARD_INCOMPATIBLE         x
    MEMCARD_CARD_CHANGED             x
    MEMCARD_UNFORMATTED              x

//-----------------------------------------------------------------------------

AsyncUnmount:

    Error code                  gcn ps2 xbx  pc
    --------------------------- --- --- --- ---

    MEMCARD_IN_PROGRESS          x   x
    MEMCARD_SUCCESS              x   x
    MEMCARD_FATAL_ERROR          x   x
    MEMCARD_NO_CARD              x   x
    MEMCARD_BUSY                 x

//-----------------------------------------------------------------------------

AsyncFormat:

    Error code                  gcn ps2 xbx  pc
    --------------------------- --- --- --- ---

    MEMCARD_IN_PROGRESS          x   x
    MEMCARD_SUCCESS              x   x
    MEMCARD_FATAL_ERROR          x   x
    MEMCARD_NO_CARD              x   x
    MEMCARD_BUSY                 x   x
    MEMCARD_WORN_OUT             x
 
//-----------------------------------------------------------------------------

AsyncWriteFile:
   
    Error code                  gcn ps2 xbx  pc
    --------------------------- --- --- --- ---

    MEMCARD_IN_PROGRESS          x   x
    MEMCARD_SUCCESS              x   x
    MEMCARD_FATAL_ERROR          x   x
    MEMCARD_NO_CARD              x   x
    MEMCARD_BUSY                 x   x
    MEMCARD_WORN_OUT             x
    MEMCARD_FILE_ALREADY_EXISTS  x   x  
    MEMCARD_NO_FILES_AVAILABLE   x   x
    MEMCARD_NOT_ENOUGH_SPACE     x   x   
    MEMCARD_FILENAME_TOO_LONG    x   x
    MEMCARD_FILE_NOT_FOUND       x   x
    MEMCARD_ACCESS_DENIED        x   x
    MEMCARD_PAST_END_OF_FILE     x   
    MEMCARD_IO_CANCELED          x   

//-----------------------------------------------------------------------------

AsyncReadFile:

    Error code                  gcn ps2 xbx  pc
    --------------------------- --- --- --- ---

    MEMCARD_IN_PROGRESS          x   x
    MEMCARD_SUCCESS              x   x
    MEMCARD_FATAL_ERROR          x   x
    MEMCARD_NO_CARD              x   x
    MEMCARD_BUSY                 x   x
    MEMCARD_FILE_NOT_FOUND       x   x
    MEMCARD_ACCESS_DENIED        x   x
    MEMCARD_DAMAGED              x
    MEMCARD_PAST_END_OF_FILE     x
    MEMCARD_IO_CANCELED          x

//-----------------------------------------------------------------------------

AsyncGetFileLength:

    Error code                  gcn ps2 xbx  pc
    --------------------------- --- --- --- ---

    MEMCARD_IN_PROGRESS          x   x
    MEMCARD_SUCCESS              x   x
    MEMCARD_FATAL_ERROR          x   x
    MEMCARD_NO_CARD              x   x
    MEMCARD_BUSY                 x   x
    MEMCARD_FILE_NOT_FOUND       x   x
    MEMCARD_ACCESS_DENIED        x
    MEMCARD_DAMAGED              x
    MEMCARD_IO_CANCELED          x

//-----------------------------------------------------------------------------
      
AsyncReadFileList: 
    
    Error code                  gcn ps2 xbx  pc
    --------------------------- --- --- --- ---
    MEMCARD_IN_PROGRESS          x   x
    MEMCARD_SUCCESS              x   x
    MEMCARD_FATAL_ERROR          x   x
    MEMCARD_NO_CARD              x   x
    MEMCARD_FILE_NOT_FOUND       x   x
    MEMCARD_ACCESS_DENIED        x
    MEMCARD_BUSY                 x   x
    MEMCARD_DAMAGED                  x

//-----------------------------------------------------------------------------

AsyncPurgeFileList:    
    
    Error code                  gcn ps2 xbx  pc
    --------------------------- --- --- --- ---
      
    NONE

//-----------------------------------------------------------------------------

AsyncDeleteFile:   

    Error code                  gcn ps2 xbx  pc
    --------------------------- --- --- --- ---
    MEMCARD_IN_PROGRESS          x   x
    MEMCARD_SUCCESS              x   x
    MEMCARD_FATAL_ERROR          x   x
    MEMCARD_NO_CARD              x   x
    MEMCARD_FILE_NOT_FOUND       x   x
    MEMCARD_ACCESS_DENIED        x
    MEMCARD_BUSY                 x   x
    MEMCARD_WORN_OUT             x

=============================================================================*/

//-----------------------------------------------------------------------------

#ifdef TARGET_XBOX
#define MEMCARD_FILENAME_LENGTH (42)
#else
#define MEMCARD_FILENAME_LENGTH (32)
#endif

#ifdef TARGET_PC
// Root directory for the memory card
#define PC_MEMCARD_ROOT_DIR "C:\\GameData\\A51\\Release\\PC\\SAVES\\"
#endif

typedef void card_status_update_fn( s32 CardID );

struct mc_file_info
{
    s32     Index;
    s32     Length;
    char    FileName[MEMCARD_FILENAME_LENGTH];
    u64     CreationDate;
    u64     ModifiedDate;
};

//=============================================================================

class memcard_mgr 
{

friend class memcard_hardware;

//-----------------------------------------------------------------------------

public:

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// NOTE: All member functions that are prefixed with "Async" are
//       asynchronus functions.  Call GetAsyncState to determine
//       the status of the operation.

                    memcard_mgr             ( void );
                   ~memcard_mgr             ( void );
void                Init                    ( void );
void                Kill                    ( void );
void                Engage                  ( card_status_update_fn* pCallback );
void                Disengage               ( void );
void                Update                  ( f32 DeltaTime );
xbool               IsEngaged               ( void )                        { return m_bIsEngaged; }

// Synchronus card operations.

s32                 GetMaxCards             ( void );
xbool               IsCardConnected         ( s32 CardID );
xbool               IsSpaceAvailable        ( u32 nBytes );
u32                 GetConnectionBits       ( void );

// Synchronus get file list (valid after successful AsyncReadFileList).
// The format of this returned string has changed. It now contains one
// xstring per directory followed by the files within it.
//
// For example: "Dir0\0File0\0File1\0File2\0".

s32                 GetFileList             ( xarray<mc_file_info>& FileList );

// Synchronus get file length (valid after successful AsyncGetFileLength).

s32                 GetFileLength           ( void );
s32                 GetFilePosition         ( void );

// Async operations.

memcard_error       GetAsyncState           ( void );
void                AsyncMount              ( s32 CardID );
void                AsyncUnmount            ( void );
void                AsyncFormat             ( void );
void                AsyncReadFileList       ( void ); 
void                AsyncPurgeFileList      ( void );
void                AsyncReadFile           ( const char* pFileName,       void* const pBuffer, s32 nBytes );
void                AsyncWriteFile          ( const char* pFileName, const void* const pBuffer, s32 nBytes );
void                AsyncDeleteFile         ( const char* pFileName );
void                AsyncGetFileLength      ( const char* pFileName );
void                AsyncRead               ( const char* pFileName,       void* const pBuffer, s32 FileOffset, s32 nBytesToRead );
void                AsyncWrite              ( const char* pFileName, const void* const pBuffer, s32 FileOffset, s32 nBytesToRead );
void                AsyncCreateFile         ( const char* pFileName, s32 nBytes );
void                AsyncFileExists         ( const char* pFileName );
void                AsyncCreateDirectory    ( const char* pDirName );
void                AsyncSetDirectory       ( const char* pDirName );
void                AsyncDeleteDirectory    ( const char* pDirName );

void                SetIconDisplayName      ( const char* pName );
f32                 GetProgress             ( void );


//-----------------------------------------------------------------------------

private:

void                ResetSystem                 ( void );

//-----------------------------------------------------------------------------

private:

xbool                   m_bIsEngaged;
card_status_update_fn*  m_pCardStatusUpdate;
u32                     m_CurrConnectionBits;
u32                     m_PrevConnectionBits;
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#define NO_CARD             (-1)
#define VALID_CARDID( n )   ((n >= 0) && (n < GetMaxCards()))

enum memcard_op 
{
    MEMCARD_OP_IDLE = 0,
    MEMCARD_OP_MOUNT,
    MEMCARD_OP_UNMOUNT,
    MEMCARD_OP_READ_FILE,
    MEMCARD_OP_WRITE_FILE,
    MEMCARD_OP_DELETE_FILE,
    MEMCARD_OP_FORMAT,
    MEMCARD_OP_REPAIR,
    MEMCARD_OP_READ_FILE_LIST,
    MEMCARD_OP_PURGE_FILE_LIST,
    MEMCARD_OP_GET_FILE_LENGTH,
    MEMCARD_OP_READ,
    MEMCARD_OP_WRITE,
    MEMCARD_OP_CREATE_FILE,
    MEMCARD_OP_CREATE_DIR,
    MEMCARD_OP_SET_DIR,
    MEMCARD_OP_DELETE_DIR,
};

//-----------------------------------------------------------------------------

class memcard_hardware
{

friend void  MemcardDispatcher( void );

public:

                        memcard_hardware        ( void );
                       ~memcard_hardware        ( void );
void                    Init                    ( void );
void                    Kill                    ( void );
void                    AllocIOBuffer           ( void );
void                    FreeIOBuffer            ( void );
s32                     GetMaxCards             ( void );
xbool                   IsCardConnected         ( s32 CardID );

#ifdef TARGET_PS2                        
void                    ProcessHardwareCallback ( void );               
s32                     GetMemcardIRXHandle     ( void )                    { return m_MC2Handle;   }
void                    SetMemcardIRXHandle     ( s32 handle )              { m_MC2Handle = handle; }
void                    SetIconDisplayName      ( const char* pName );
#endif

#ifdef TARGET_PS2
void                    SetRootDir              ( void ){ x_strcpy( m_pRequestedDirName,"/*A51*" ); }
#endif

#ifdef TARGET_XBOX
void                    SetRootDir              ( void ){ m_pRequestedDirName[0]=0; }
#endif

#ifdef TARGET_PC
void                    SetRootDir              ( void ){ x_strcpy(m_pRequestedDirName, PC_MEMCARD_ROOT_DIR); }
#undef GetFreeSpace // thank you .NET for making a macro named GetFreeSpace.
#endif              // I mean, it's not like I'd ever want one of my own.

u32                     GetFreeSpace            ( void );

void                    SendMessage             ( s32 Message );
void                    Process                 ( void );
void                    ProcessMount            ( void );
void                    ProcessUnmount          ( void );
void                    ProcessReadFile         ( void );
void                    ProcessWriteFile        ( void );
void                    ProcessDeleteFile       ( void );
void                    ProcessFormat           ( void );
void                    ProcessRepair           ( void );
void                    ProcessReadFileList     ( void );
void                    ProcessPurgeFileList    ( void );
void                    ProcessGetFileLength    ( void );
void                    ProcessRead             ( void );
void                    ProcessWrite            ( void );
void                    ProcessCreateFile       ( void );
void                    ProcessCreateDir        ( void );
void                    ProcessSetDir           ( void );
void                    ProcessDeleteDir        ( void );
void                    InitiateOperation       ( void );
void                    SetOperation            ( memcard_op Operation );
inline memcard_op       GetOperation            ( void )                    { return m_Operation; }
inline void             SetRequestedCard        ( s32 CardID )              { m_RequestedCard = CardID; }
inline s32              GetMountedCard          ( void )                    { return m_MountedCard; }
inline void             SetMountedCard          ( s32 CardID )              { m_MountedCard = CardID; }
inline memcard_error    GetState                ( void )                    { return m_Error; }
       void             SetState                ( memcard_error NewState );
void                    SetIOParams             ( const char* FileName, byte* pBuffer, s32 Offset, s32 nBytes );
s32                     GetFileList             ( xarray<mc_file_info>& FileList );
s32                     GetFileCount            ( void )                    { return m_nFileCount;      }
mc_file_info*           GetFileInfo             ( s32 Index );
inline s32              GetFileLength           ( void )                    { return m_FileLength;      }
inline void             SetFileLength           ( s32 Length )              { m_FileLength = Length;    }
s32                     GetFilePosition         ( void )                    { return m_FilePosition;    }
void                    InvalidateFileList      ( void );
memcard_error           GetCardStatus           ( s32 CardId );

private:

inline void             SetSubState         ( s32 NewState )            { m_SubState = NewState; }
inline s32              GetSubState         ( void )                    { return m_SubState; }

private:

xmesgq*                 m_pDispatcherMQ;
xthread*                m_pThread;
volatile memcard_error  m_Error;
volatile memcard_op     m_Operation;
volatile s32            m_SubState;
s32                     m_RequestedCard;
char                    m_pRequestedFileName[32];
char                    m_pRequestedDirName [32];
byte*                   m_pRequestedBuffer;
s32                     m_nRequestedBytes;
s32                     m_RequestedOffset;
volatile s32            m_MountedCard;
volatile s32            m_FileLength;
volatile s32            m_FilePosition;
u32                     m_SectorSize;
volatile xbool          m_bIsFileListValid;
volatile s32            m_nFileCount;
#ifdef TARGET_PS2
s32                     m_MC2Handle;
#endif
};

extern memcard_mgr      g_MemcardMgr;
extern memcard_hardware g_MemcardHardware;

#endif // MEMCARD_HPP
