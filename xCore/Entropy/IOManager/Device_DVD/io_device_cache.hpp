//==============================================================================
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
//==============================================================================
//
// DVD caching to a background device.
//
//==============================================================================
#if !defined(IO_DEVICE_CACHE_HPP)
#define IO_DEVICE_CACHE_HPP
#include "x_target.hpp"
#if !defined(TARGET_PS2) && !defined(TARGET_XBOX)
#error This is not for this target platform. Check dependancy rules.
#endif

#include "x_types.hpp"

// Uncomment to actually enable HDD driver loading
//#define ENABLE_HDD_SUPPORT
// Uncomment this to enable cd -> hdd file caching as well as HDD file I/O
//#define ENABLE_CACHING

//
// We're being a bit naughty here. We're stealing a bit from the handle returned by
// the system open call to flag whether or not this file is currently on the caching
// device. We may need to move the bit being used if the target platform has a problem
// with this one. There are asserts all over the place to make sure this will be
// detected if it ever happens.
//
#define CACHED_FLAG         ( 1 << 30 )

//
//
//
enum cache_status
{
    CACHE_INIT,
    CACHE_READY,
    CACHE_NOT_PRESENT,
    CACHE_NOT_FORMATTED,
    CACHE_FULL,
    CACHE_BUSY,
    CACHE_ERROR,
};

#define MAX_CACHE_REQUESTS 4
//
// A cache_pending struct is used to allow us to queue up files to be cached to HDD. This
// will allow us to open many files without stalling the system waiting on a cache write to
// complete. However, if we run out of cache_pending requests, the background thread will
// finish caching the current file before continuing.
//
enum pending_status
{
    CACHE_COPY_IDLE,             // This cache entry is not in use
    CACHE_COPY_WAITING,
    CACHE_COPY_PROCESSING,
};

struct cache_pending
{
    pending_status  m_Status;
    xtimer          m_StartTime;
    s32             m_SourceHandle;
    s32             m_SourceLength;
    s32             m_DestHandle;
    s32             m_DestLength;
};

enum cache_request_type
{
    CACHE_REQUEST_READ,
    CACHE_REQUEST_WRITE,
    CACHE_REQUEST_OPEN,
    CACHE_REQUEST_CLOSE,
};

enum cache_open_mode
{
    CACHE_OPEN_READ,
    CACHE_OPEN_WRITE,
};

class cache_request
{
public:
    cache_request( void ) : m_qReply(1)
    {
    };

    ~cache_request( void )
    {
    };
    //
    // Even though this really should be a struct since it is a depository for simple data, I'm using
    // accessor functions so that I can, later on, add asserts to make sure unions are not being used
    // for multiple purposes.
    //
    void                SetType     ( cache_request_type Type )     { m_Type = Type;                        }
    void                SetMode     ( cache_open_mode mode )        { m_Mode = mode;                        }
    void                SetFilename ( const char* pFilename )       { x_strcpy(m_Filename, pFilename);      }
    void                SetHandle   ( s32 Handle )                  { m_Handle = Handle;                    }
    void                SetLength   ( s32 Length )                  { m_Length = Length;                    }
    void                SetDataPtr  ( void* pData )                 { m_pData = pData;                      }
    void                SetOffset   ( s32 Offset )                  { m_Offset = Offset;                    }
    void                SetResult   ( s32 Result )                  { m_Result = Result;                    }

    cache_request_type  GetType     ( void )                        { return m_Type;                        }
    cache_open_mode     GetMode     ( void )                        { return m_Mode;                        }
    const char*         GetFilename ( void )                        { return m_Filename;                    }
    void*               GetDataPtr  ( void )                        { return m_pData;                       }
    s32                 GetOffset   ( void )                        { return m_Offset;                      }
    s32                 GetHandle   ( void )                        { return m_Handle;                      }
    s32                 GetLength   ( void )                        { return m_Length;                      }
    s32                 GetResult   ( void )                        { return m_Result;                      }


    s32                 WaitDone    ( void )                        { return (s32)m_qReply.Recv( MQ_BLOCK );    }
    void                RequestDone ( s32 Result )                  { m_qReply.Send( (void*)Result, MQ_BLOCK ); }
private:
    cache_request_type  m_Type;

    union
    {
        struct
        {
            cache_open_mode m_Mode;
            char            m_Filename[128];
        };

        struct
        {
            s32         m_Result;
            void*       m_pData;
            s32         m_Offset;
            s32         m_Length;
            s32         m_Handle;
        };
    };
    xmesgq              m_qReply;
};

class cache_handler
{
public:
    cache_handler    ( void )
#if defined(ENABLE_CACHING)
      : m_qRequests(MAX_CACHE_REQUESTS)
#endif
      {
      }
      void                  Init                ( void );
      void                  Kill                ( void );
      s32                   Open                ( const char* pFilename, cache_open_mode Mode,u32& Length);
      void                  Close               ( s32 Handle );
      s32                   Read                ( s32 Handle, void* Addr, s32 Length, s32 Offset );
      s32                   Write               ( s32 Handle, void* Addr, s32 Length, s32 Offset );
      cache_status          GetStatus           ( void )                    { return m_Status; };
      xbool                 CacheIsReady        ( void );

private:
    //
    // These functions are machine dependant.
    //
    cache_status            SystemInit          ( void );
    s32                     SystemOpen          ( const char* pFilename, cache_open_mode Mode );
    void                    SystemClose         ( s32 Handle );
    s32                     SystemRead          ( s32 Handle, s32 Offset, void* Data, s32 Length );
    s32                     SystemWrite         ( s32 Handle, s32 Offset, void* Data, s32 Length );
    s32                     SystemGetLength     ( s32 Handle );

    //
    // Internal functions
    //
    void                    ProcessReadRequest  ( cache_request* pRequest );
    void                    ProcessOpenRequest  ( cache_request* pRequest );
    void                    ProcessWriteRequest ( cache_request* pRequest );
    void                    ProcessCloseRequest ( cache_request* pRequest );
    cache_status            m_Status;
    xbool                   m_CacheDeviceIsPresent;
#if defined(ENABLE_CACHING)
    void                    CacheThread         ( void );
    void                    UpdateCache         ( void );
    void                    InvalidateCache     ( void );
    s32                     m_ModuleHandle;
    s32                     m_CacheOwner;
    s32                     m_CacheOffset;
    s32                     m_CacheFileLength;
    xbool                   m_CacheDirty;
    xbool                   m_CopyingFile;
    s32                     m_CopyEntryIndex;
    xthread*                m_pCacheThread;
    xmesgq                  m_qRequests;
    cache_pending           m_CopyQueue[ MAX_CACHE_REQUESTS ];

    byte                    m_CacheBuffer[32768] PS2_ALIGNMENT(64);

    friend void             s_StartCacheThread  ( s32, char** );
#endif
};

extern cache_handler g_IODeviceCache;

#endif