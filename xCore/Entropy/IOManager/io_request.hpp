#ifndef __IO_REQUEST_HPP__
#define __IO_REQUEST_HPP__

#include "x_types.hpp"
#include "x_threads.hpp"
#include "x_time.hpp"

//==============================================================================

struct io_device_file;
struct io_open_file;
class  io_device;

//==============================================================================

class io_request
{

friend void ProcessEndOfRequest         ( io_device* pDevice, s32 Status );
friend void io_dispatcher               ( void );

friend class io_mgr;
friend class io_device;
friend class audio_stream_mgr;

//------------------------------------------------------------------------------
//  Public enums, types

public:

        enum status
        {
            NOT_QUEUED = 0,
            QUEUED,
            PENDING,
            IN_PROGRESS,
            COMPLETED,
            FAILED,
        };

        enum priority
        {
            HIGH_PRIORITY = 0,
            MEDIUM_PRIORITY,     
            LOW_PRIORITY,
            NUM_PRIORITIES,
        };

        enum operation
        {
            READ_OP = 0,
            WRITE_OP,
        };

        typedef void callback_fn( io_request* pRequest );

//------------------------------------------------------------------------------
//  Private data

private:

            io_request*         m_pPrev;
            io_request*         m_pNext;
            io_open_file*       m_pOpenFile;
            priority            m_Priority;         // Request priority
            operation           m_Operation;        // Request operation
            void*               m_pBuffer;          // Memory address for requested data
            s32                 m_Offset;           // Offset into file to start the read
            s32                 m_Length;           // Number of bytes to read
            s32                 m_ChunkOffset;      // Current "chunks" offset into the file
            s32                 m_ChunkLength;      // Current "chunks" length
            callback_fn*        m_pCallback;
volatile    status              m_Status;
            s32                 m_HardwareStatus;
            s32                 m_Sequence;
            u32                 m_UserData;
            xbool               m_UseSema;
            xsema               m_Semaphore;
            u32                 m_Destination;      // Destination
            xtick               m_QueueTick;
            xtick               m_DispatchTick;
            xtick               m_CompleteTick;

//------------------------------------------------------------------------------
//  Public functions

public:

                            io_request      ( void );
                           ~io_request      ( void );
                            
        void                SetRequest      ( io_open_file*     pOpenFile, 
                                              void*             pBuffer,
                                              s32               Offset,
                                              s32               Length,
                                              priority          Priority,
                                              xbool             UseSema,
                                              u32               Destination,
                                              u32               UserData,
                                              operation         Operation, 
                                              callback_fn*      pCallback = NULL );
                                                                
inline  io_open_file*       GetFile         ( void )            { return m_pOpenFile; }
inline  void*               GetBuffer       ( void )            { return m_pBuffer; }
inline  s32                 GetOffset       ( void )            { return m_Offset; }
inline  s32                 GetChunkOffset  ( void )            { return m_ChunkOffset; }
inline  s32                 GetLength       ( void )            { return m_Length; }
inline  priority            GetPriority     ( void )            { return m_Priority; }
inline  callback_fn*        GetCallback     ( void )            { return m_pCallback; }
inline  status              GetStatus       ( void )            { return m_Status; }
inline  u32                 GetUserData     ( void )            { return m_UserData; }
inline  s32                 GetSequence     ( void )            { return m_Sequence; }
inline  xbool               GetUseSema      ( void )            { return m_UseSema; }
inline  void                AcquireSemaphore( void )            { m_Semaphore.Acquire(); }
inline  void                ReleaseSemaphore( void )            { m_Semaphore.Release(); }
                                                                
};

#endif
