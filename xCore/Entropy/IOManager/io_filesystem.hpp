#ifndef __IO_FILESYSTEM_HPP__
#define __IO_FILESYSTEM_HPP__

#include "io_cache.hpp"
#include "x_string.hpp"
#include "io_dfs.hpp"

//------------------------------------------------------------------------------
// Public structs.

#define IO_RETAIN_FILENAME

struct io_device_file;

struct io_open_file
{
        s32                     Position;               // Current position within the file (logical).
        s32                     Offset;                 // Offset with the file.
        s32                     Length;                 // Size of the file in bytes.
        s32                     Mode;                   // File mode.
        io_device_file*         pDeviceFile;            // Pointer to the device file.
        void*                   pRAM;                   // Pointer to ram based dfs data.
        xbool                   bRead;                  // Flag.
        xbool                   bWrite;                 // Flag.
        xbool                   bAppend;                // Flag.
        xbool                   bEnableChecksum;        // Flag.
        X_FILE*                 PassThrough;            // Pass through file handle
        io_open_file*           pNext;                  // Link.
#ifdef IO_RETAIN_FILENAME
        char                    Filename[256];
#endif
};

//------------------------------------------------------------------------------

class io_fs
{

private:

//------------------------------------------------------------------------------
//  Private defines

#define NUM_CACHES      (1)
#define MAX_FILES       (16)

//------------------------------------------------------------------------------
// Private structs

private:

struct io_dfs_data
{
    xstring                 PathName;           // dfs pathname.
    char*                   RamAddress;
    s32                     SearchPriority;
    s32                     FindIndex;
    dfs_header*             pHeader;
    xarray<io_device_file*> DeviceFiles;
};

//------------------------------------------------------------------------------
//  Private data

private:

xarray<io_dfs_data> m_DFS;
io_cache            m_Caches[ NUM_CACHES ];
s32                 m_Retries;
io_open_file        m_Files[ MAX_FILES ];
io_open_file*       m_FreeFiles;
s32                 m_CurrentDFS;
s32                 m_CurrentDFSIndex;
s32                 m_LogFlags;
xmutex              m_Mutex;

//------------------------------------------------------------------------------
//  Private functions

private:

io_open_file*       AcquireFile         ( void );
void                ReleaseFile         ( io_open_file* pFile );
io_cache*           AcquireCache        ( io_open_file* pFile );
void                ReleaseCache        ( io_cache* pChache );
xbool               FindFile            ( const char* pPathName, io_device_file* &DeviceFile, u32 &Offset, u32 &Length, void* &pRAM );
xbool               SearchDFS           ( const char* pPathName, io_device_file* &DeviceFile, u32 &Offset, u32 &Length, s32 SubFile, s32 StartIndex, void* &pRAM );
xbool               CompareFile         ( const char* pPathName, io_device_file* &DeviceFile, u32 &Offset, u32 &Length, s32 SubFile, s32 Index, void* &pRAM );
     
//------------------------------------------------------------------------------
//  Public functions

public:

                    io_fs                   ( void );
                   ~io_fs                   ( void );
xbool               Init                    ( void );
void                Kill                    ( void );
xbool               MountFileSystem         ( const char* pPathName, s32 SearchPriority );
xbool               MountFileSystemRAM      ( const char* pPathName, void* pHeaderData, void* pRawData );
xbool               UnmountFileSystem       ( const char* pPathName );
void                EnableChecksum          ( void* pOpenFile, xbool bEnable );
io_open_file*       Open                    ( const char* pPathName, const char* pMode );
void                Close                   ( io_open_file* pFile );
s32                 Read                    ( io_open_file* pFile, byte* pBuffer, s32 Bytes );
s32                 Write                   ( io_open_file* pFile, const byte* pBuffer, s32 Bytes );
void                SetRetries              ( s32 Retries );
const char*         GetFileName             ( const X_FILE* pFile );
void                DumpFileSystem          ( s32 Index );
void                InvalidateCaches        ( void );
s32                 GetFileSystemIndex      ( const char* pPathName );
s32                 GetNFilesInFileSystem   ( s32 iFileSystem );
void                GetFileNameInFileSystem ( s32 iFileSystem, s32 iFile, char* pFileName );

//------------------------------------------------------------------------------
//  Buddy list
friend void io_close( X_FILE* pFile );

};

//------------------------------------------------------------------------------

extern io_fs g_IOFSMgr;

//------------------------------------------------------------------------------

#endif // __IO_FILESYSTEM_HPP__
