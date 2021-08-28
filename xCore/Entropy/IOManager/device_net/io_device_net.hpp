#ifndef IO_DEVICE_NET_HPP
#define IO_DEVICE_NET_HPP

#include "..\io_device.hpp"
#include "..\io_filesystem.hpp"
#include "e_Network.hpp"

//-----------------------------------------------------------------------------
// Definitions used internally within the netfs client/server. This includes
// packet format and communication protocol tags
//-----------------------------------------------------------------------------
enum
{
    FSRV_TYPE_PASSTHRU  = 0,                // This file is passed through to the previous fs
    FSRV_TYPE_LOCAL,                        // This file has to be processed by our fs.
};

#define FILESERVER_PORT         32000
#define FILESERVER_TIMEOUT      1000          // Timeout in milliseconds
#define FILESERVER_RETRIES      3           // Number of retries before giving up
#define FILESERVER_CONCURRENT_REQUESTS  8
//
// The fileserver block size is LESS than the maximum packet size because we need to leave a little space for a header.
// Our header is ONLY 6 bytes but this will leave us a 24 byte space for future header expansion.
//
#define FILESERVER_BLOCK_SIZE   1000
#define FILESERVER_MAX_FILENAME 256
#define FILESERVER_MAX_MODE     10

//-----------------------------------------------------------------------------
// Fileserver requests
//-----------------------------------------------------------------------------
enum netfs_request
{
    FSRV_REQ_RESET   = 0x1000,
    FSRV_REQ_OPEN,
    FSRV_REQ_CLOSE,
    FSRV_REQ_READ,
    FSRV_REQ_WRITE,
    FSRV_REQ_FLUSH,
    FSRV_REQ_FIND,
};

struct fsrv_reset
{
};

struct fsrv_open
{
    char    Filename[FILESERVER_MAX_FILENAME];
    char    Mode[FILESERVER_MAX_MODE];
};

struct fsrv_close
{
    s32     Handle;
};

struct fsrv_read
{
    s32     Handle;
    s32     Length;
    s32     Position;
};

struct fsrv_write
{
    s32     Handle;
    s32     Length;
    s32     Position;
    byte    Data[FILESERVER_BLOCK_SIZE];
};

struct fsrv_find
{
    s32     Address;
    s32     Port;
};

struct fsrv_header
{
    s32             Type;
    s32             Sequence;
};

struct fileserver_request
{
    fsrv_header         Header;
    union
    {
        fsrv_reset      Reset;
        fsrv_open       Open;
        fsrv_close      Close;
        fsrv_read       Read;
        fsrv_write      Write;
        fsrv_find       Find;
    };
    u8  Padding[1024-sizeof(fsrv_write)-sizeof(fsrv_header)];
};

//-----------------------------------------------------------------------------
// Fileserver replies
//-----------------------------------------------------------------------------
enum
{
    FSRV_ERR_OK = 0,
    FSRV_ERR_NOTFOUND,
    FSRV_ERR_NOTOPEN,
    FSRV_ERR_READONLY,
    FSRV_ERR_BADCOMMAND,
};

struct fsrv_open_reply
{
    s32     Status;                     // Status MUST always be the first field
    s32     Handle;
    s32     Length;
};

struct fsrv_reply
{
    s32     Status;                     // Status MUST always be the first field
};

struct fsrv_read_reply
{
    s32     Status;                     // Status MUST always be the first field
    s32     Length;
    byte    Data[FILESERVER_BLOCK_SIZE];
};

struct fsrv_find_reply
{
    s32     Address;
    s32     Port;
};

struct fsrv_write_reply
{
    s32     Status;                     // Status MUST always be the first field
    s32     Length;
};

struct fileserver_reply
{
    struct
    {
        s32                 Sequence;
    } Header;
    union
    {
        fsrv_reply          Reset;
        fsrv_open_reply     Open;
        fsrv_reply          Close;
        fsrv_read_reply     Read;
        fsrv_write_reply    Write;
        fsrv_find_reply     Find;
    };
    u8  Padding[1024-sizeof(fsrv_read_reply)-sizeof(s32)];
};

class io_device_net : public io_device
{

private:

    io_device_file*         m_pLastFile;        // Last file read from (logging).
    s32                     m_LastOffset;       // Offset of last read (logging).
    s32                     m_LastLength;       // Length of last read (logging).
    s32                     m_nSeeks;           // Number of seeks (logging).    
    net_address             m_ServerAddress;
    net_socket              m_LocalSocket;
    xmutex                  m_LockMutex;

public:

    virtual                ~io_device_net               ( void );
    io_device_net           ( void );
    virtual void            Init                        ( void );
    virtual void            Kill                        ( void );

private:

    void                    LogPhysRead                 ( io_device_file* pFile, s32 Length, s32 Offset );
    virtual device_data*    GetDeviceData               ( void );
    virtual void            CleanFilename               ( char* pClean, const char* pFilename );

    virtual xbool           PhysicalOpen                ( const char* pFilename, io_device_file* pFile,io_device::open_flags );
    virtual xbool           PhysicalRead                ( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace );
    virtual xbool           PhysicalWrite               ( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace );
    virtual void            PhysicalClose               ( io_device_file* pFile );

    xbool                   WaitForReply                ( fileserver_reply& Reply, f32 Timeout=1.0f );
    void                    SimpleDialog                ( const char* pText, f32 Timeout=0.125f);
    void                    Lock                        ( void );
    void                    Unlock                      ( void );

    void*                   SystemOpen                  ( const char* pFilename, const char* Mode, s32& Length );
    s32                     SystemWrite                 ( void* Handle, const void* pBuffer, s32 Offset, s32 Length );
    s32                     SystemRead                  ( void* Handle, void* pBuffer, s32 Offset, s32 Length );
    void                    SystemClose                 ( void* Handle );

    friend void*            netfs_Open                  ( const char* pFilename, const char* pMode );
    friend s32              netfs_Write                 ( void* Handle, const void* pBuffer, s32 Offset, s32 Length );
    friend s32              netfs_Read                  ( void* Handle, void* pBuffer, s32 Offset, s32 Length );
    friend void             netfs_Close                 ( void* Handle );
};

extern io_device_net g_IODeviceNET;

#endif // IO_DEVICE_NET_HPP
