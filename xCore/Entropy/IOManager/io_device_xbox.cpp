//==============================================================================
//
// CDROM Defines, Buffers, Etc
//
//==============================================================================

#define CDROM_CACHE_SIZE    (2048*16)
#define CDROM_NUM_FILES     (16)                        // TODO: CJG - Increase this to accomodate multiple CDFS's
#define CDROM_INFO_SIZE     (0)
#define CDROM_CACHE         ((void*)s_CdromCache)
#define CDROM_FILES         ((void*)s_CdromFiles)
#define CDROM_BUFFER_ALIGN  (2048)
#define CDROM_OFFSET_ALIGN  (2048)
#define CDROM_LENGTH_ALIGN  (2048)

static char           s_CdromCache[ CDROM_CACHE_SIZE ] GCN_ALIGNMENT(CDROM_BUFFER_ALIGN);
static io_device_file s_CdromFiles[ CDROM_NUM_FILES ];
static xthread      * s_IoProcessor;

#include "xtl.h"

//==============================================================================
//
// Device definitions.
//
//==============================================================================

io_device::device_data s_DeviceData[ io_mgr::NUM_DEVICES ] = {
//   Name               Id IsSupported IsReadable IsWriteable         CacheSize         BufferAlign         OffsetAlign         LengthAlign         NumFiles         InfoSize          pCache    pFilesBuffer
//======= ================ =========== ========== =========== ================= =================== =================== =================== ================ ================ =============== ===============
{ "CDROM",   io_mgr::CDROM,       TRUE,      TRUE,      FALSE, CDROM_CACHE_SIZE, CDROM_BUFFER_ALIGN, CDROM_OFFSET_ALIGN, CDROM_LENGTH_ALIGN, CDROM_NUM_FILES, CDROM_INFO_SIZE,    CDROM_CACHE,    CDROM_FILES },
};



//==============================================================================


static void xbox_ConvertPath( char* pClean, const char* pFilename )
{
    x_strcpy(pClean,"D:\\GameData\\");
    pClean+=x_strlen(pClean);

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
}

//==============================================================================

struct OverLap : public OVERLAPPED
{
    s32       Type;
    void    * pBuff;
    HANDLE    hFile;
    u32       nSize;
};

//==============================================================================

#define REQ_NO_PENDING 0
#define REQ_QUEUE_READ 1
#define REQ_PENDING    2 // or more
#define MAX_REQUESTS_PENDING 8

xmesgq  s_PendingRequests(MAX_REQUESTS_PENDING);
xmesgq  s_AvailableRequests(MAX_REQUESTS_PENDING);

OverLap s_Requests[MAX_REQUESTS_PENDING];
volatile xbool s_RequestInProgress = TRUE;
//==============================================================================

static void CALLBACK FileIOCompletionRoutine(
  DWORD dwErrorCode,                // completion code
  DWORD dwNumberOfBytesTransfered,  // number of bytes transferred
  LPOVERLAPPED lpOverlapped         // I/O information buffer
)
{   (void)dwNumberOfBytesTransfered;
    (void)lpOverlapped;

    // We are in the callback
    s_CallbackActive++;

    ASSERT(s_RequestInProgress);
    s_RequestInProgress = FALSE;
    // Success?
    if( !dwErrorCode )
    {
        // Its all good!
        ProcessEndOfRequest( io_mgr::CDROM, io_request::COMPLETED );
    }
    else
    {
        // Ack failed!
        ProcessEndOfRequest( io_mgr::CDROM, io_request::FAILED );
    }


    // Done with callback
    s_CallbackActive--;
}

//==============================================================================

static void IoProcessor( void )
{   //
    //  Process I/O requests
    //
    for( ;; )
    {
        OverLap* pRequest;
        xbool    ok;

        pRequest = (OverLap*)s_PendingRequests.Recv(MQ_BLOCK);
        switch( pRequest->Type )
        {
            case REQ_QUEUE_READ:
                //
                //  Start read
                //
                s_RequestInProgress = TRUE;
                ok=ReadFileEx(
                    pRequest->hFile ,
                    pRequest->pBuff ,
                    pRequest->nSize ,
                    pRequest  ,
                    FileIOCompletionRoutine
                );
                if (!ok)
                {
                    x_DebugMsg("IO Request submit failed, error %d\n",GetLastError());
                }
                ASSERT(ok);
                // Suspend this thread until the callback gets a chance to run
                SleepEx(INFINITE,TRUE);
                ASSERT(!s_RequestInProgress);
                // Kill allocated memory
                VERIFY(s_AvailableRequests.Send(pRequest,MQ_NOBLOCK));
                break;

            default:
                ASSERT(FALSE);
                break;
        }
    }
}

//==============================================================================
//
//  todo: this should be DeviceCDROMInit
//

void InitXboxIoMgr( void )
{
    {   //
        //  Start I/O processor
        //
        s32 i;
        for (i=0;i<MAX_REQUESTS_PENDING;i++)
        {
            s_AvailableRequests.Send(&s_Requests[i],MQ_BLOCK);
        }
        s_IoProcessor = new xthread( IoProcessor,"Io Processor",256,2 );
    }
}

//==============================================================================
//
//  todo: this should be DeviceCDROMKill
//

void KillXboxIoMgr( void )
{
    {
        delete s_IoProcessor;
        s_IoProcessor = NULL;
    }
}

//==============================================================================

// These are the asynchronous commands. For background loading.

static xbool DeviceCDROMOpen( const char* pFilename, io_device_file* pFile )
{
    char CleanFile[256];
    xbox_ConvertPath(CleanFile,pFilename);

    HANDLE hFile=CreateFile(
        CleanFile                        ,    // lpFileName
        GENERIC_READ                     ,    // dwDesiredAccess
        FILE_SHARE_READ                  ,    // dwShareMode
        NULL                             ,    // lpSecurityAttributes
        OPEN_EXISTING                    ,    // dwCreationDisposition
        FILE_ATTRIBUTE_NORMAL
            |   FILE_FLAG_OVERLAPPED
            |   FILE_FLAG_NO_BUFFERING   ,    // dwFlagsAndAttributes
        NULL                                  // hTemplateFile
    );

    if(hFile==INVALID_HANDLE_VALUE){
        return FALSE;
    }

    //
    //  Start I/O thread
    //

    DWORD dwFileSize=GetFileSize(hFile,NULL);
    pFile->Handle=(void*)hFile;
    pFile->Length=dwFileSize;
    return TRUE;
}

//==============================================================================

static void DeviceCDROMClose( io_device_file* pFile )
{
    HANDLE hFile=HANDLE(pFile->Handle);
    ASSERTS(hFile,"Bad handle value!");
    pFile->Handle=NULL;
    CloseHandle(hFile);
}

//==============================================================================

static xbool DeviceCDROMRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    (void)AddressSpace;

    ASSERTS(pFile->Handle,"Bad handle value");

    //
    //  Create new overlap structure
    //


    OverLap*pNode=(OverLap*)s_AvailableRequests.Recv(MQ_NOBLOCK);

    if( !pNode )
    {
        return FALSE;
    }

    //
    //  Add overlap to pending list
    //

    pNode->Offset       = Offset;           // Initialise the OVERLAPPED part of the structure
    pNode->OffsetHigh   = 0;
    pNode->hEvent       = 0;

    pNode->Type         = REQ_QUEUE_READ;
    pNode->hFile        = HANDLE( pFile->Handle );
    pNode->pBuff        = pBuffer;
    pNode->nSize        = Length;

    VERIFY(s_PendingRequests.Send(pNode,MQ_NOBLOCK));

    return TRUE;
}
