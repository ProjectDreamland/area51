#ifndef __IO_DEVICE_HPP__
#define __IO_DEVICE_HPP__

#include "x_types.hpp"
#include "x_threads.hpp"
#include "io_request.hpp"

//==============================================================================

#define VALID_DEVICE_INDEX( Id )    ((Id >= 0) && (Id < NUM_IO_DEVICES))
#define VALID_DEVICE_FILE( pFile )  ((pFile != NULL) && (pFile->pDevice!=NULL))
#define SUPPORTED_DEVICE( pDevice ) ( pDevice->m_IsSupported ) 
#define GET_DEVICE_FROM_INDEX( i )  (g_IoMgr.m_Devices[ i ])
#define GET_DISPATCHER_MQ()         g_IoMgr.m_DispatcherMQ

//==============================================================================

#define IO_DEVICE_FILENAME_LIMIT    (128)
#define IO_DEVICE_MAX_NAME_LENGTH   (32)
#define IO_DEVICE_MAX_PREFIX_LENGTH (128)

//==============================================================================

class io_device;

struct io_device_file
{
        	void*           pBuffer;
        	xbool           IsOpen;
        	void*           pHeader;
        	void*           Handle;
        	s32             Length;
        	s32             BufferValid;
            s32             SubFileIndex;
volatile	s32             ReferenceCount;
        	io_device*      pDevice;
        	void*           pHardwareData;
        	char            Filename[IO_DEVICE_FILENAME_LIMIT];
        	io_device_file* pNext;
};

//==============================================================================

class io_device
{

friend void  ProcessEndOfRequest         ( io_device* pDevice, s32 Status );
friend void  io_dispatcher               ( void );
friend class io_mgr;
friend class io_request;
friend class io_cache;

//------------------------------------------------------------------------------
//  Public structures

public:

enum open_flags
{
    READ   = (1<<0),
    WRITE  = (1<<1),
    APPEND = (1<<2),
};

struct device_data
{
            char            Name[IO_DEVICE_MAX_NAME_LENGTH];
            xbool           IsSupported;
            xbool           IsReadable;
            xbool           IsWriteable;
            s32             CacheSize;
            s32             BufferAlign;
            s32             OffsetAlign;
            s32             LengthAlign;
            s32             NumFiles;
            s32             HardwareDataSize;
            void*           pCache;
            void*           pFilesBuffer;
};

//------------------------------------------------------------------------------
//  Private data

protected:

            char            m_Name[IO_DEVICE_MAX_NAME_LENGTH];
            char            m_Prefix[IO_DEVICE_MAX_PREFIX_LENGTH];
            s32             m_DeviceIndex;
            xbool           m_IsSupported;
            xbool           m_IsReadable;
            xbool           m_IsWriteable;
            s32             m_CacheSize;
            s32             m_BufferAlign;
            s32             m_OffsetAlign;
            s32             m_LengthAlign;
            s32             m_NumFiles;
            s32             m_HardwareDataSize;
            void*           m_pCache;
            void*           m_pFilesBuffer;     // Buffer containing the file structures.
            io_device_file* m_pFreeFiles;
            xmesgq          m_Semaphore;
            s32             m_RequestCount;
            s32             m_Sequence;         // Request counter.
            io_request      m_RequestQueue;     // Linked list sentinel.
            io_request*     m_CurrentRequest;
volatile    s32             m_CallbackLevel;    // Is this device in a callback?

//------------------------------------------------------------------------------
//  Private functions

public:

                        io_device                   ( void );
virtual                ~io_device                   ( void );
virtual void            Init                        ( void );
virtual void            Kill                        ( void );
        void            EnterCallback               ( void ) { m_CallbackLevel++; }
        void            LeaveCallback               ( void ) { m_CallbackLevel--; }
        xbool           IsInCallback                ( void ) { return( m_CallbackLevel != 0 ); }

protected:

        io_device_file* OpenFile                    ( const char* pFileName, open_flags OpenFlags );
        void            CloseFile                   ( io_device_file* pFile );
        s32             GetFileDevice               ( io_device_file* pFile ) const;
        s32             GetDeviceQueueStatus        ( void ) const;
        void            ServiceDeviceQueue          ( void );
        void            ProcessReadRequest          ( io_request* pRequest );
        void            ProcessWriteRequest         ( io_request* pRequest );
        xbool           ProcessReadComplete         ( io_request* pRequest );
        xbool           ProcessWriteComplete        ( io_request* pRequest );
        void            ServiceDeviceCurrentRequest ( void );
        void            SetPathPrefix               ( const char *pPrefix );
        void            GetPathPrefix               ( char* pBuffer  );
virtual device_data*    GetDeviceData               ( void );
virtual void            CleanFilename               ( char* pClean, char* pFilename );
virtual xbool           PhysicalOpen                ( const char* pFilename, io_device_file* pFile, open_flags OpenFlags );
virtual xbool           PhysicalRead                ( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace );
virtual xbool           PhysicalWrite               ( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace );
virtual void            PhysicalClose               ( io_device_file* pFile );

};

#endif
