//==============================================================================
//
// Generic network device layer
//
//==============================================================================
#include "x_target.hpp"

#include "x_files.hpp"
#include "..\io_mgr.hpp"
#include "..\io_filesystem.hpp"
#include "io_device_net.hpp"
#include "x_log.hpp"
#include "Entropy.hpp"
#include "e_Network.hpp"


#define CDROM_CACHE_SIZE    (32*1024)
#define CDROM_NUM_FILES     (16)                        
#define CDROM_INFO_SIZE     (32)
#define CDROM_CACHE         ((void*)s_CdromCache)
#define CDROM_FILES         ((void*)s_CdromFiles)
#define CDROM_BUFFER_ALIGN  (64)
#define CDROM_OFFSET_ALIGN  (2048)
#define CDROM_LENGTH_ALIGN  (0)

static char           s_CdromCache[ CDROM_CACHE_SIZE ] PS2_ALIGNMENT(CDROM_BUFFER_ALIGN);
static io_device_file s_CdromFiles[ CDROM_NUM_FILES ];

#define MAX_CONCURRENT_REQUESTS (32)
//==============================================================================
//
// Device definitions.
//
//==============================================================================

io_device::device_data s_DeviceData_NET =
{
    "NETFS",            // Name
        TRUE,               // IsSupported
        TRUE,               // IsReadable
        FALSE,              // IsWriteable
        CDROM_CACHE_SIZE,   // CacheSize
        CDROM_BUFFER_ALIGN, // BufferAlign
        CDROM_OFFSET_ALIGN, // OffsetAlign
        CDROM_LENGTH_ALIGN, // LengthAlign
        CDROM_NUM_FILES,    // NumFiles
        CDROM_INFO_SIZE,    // InfoSize
        CDROM_CACHE,        // pCache    
        CDROM_FILES         // pFilesBuffer
};
//#define LOG_PHYSICAL_READ "io_device_net::PhysicalRead(read)"
//#define LOG_PHYSICAL_SEEK "io_device_net::PhysicalRead(seek)"

//==============================================================================

io_device_net g_IODeviceNET;

//==============================================================================

io_device_net::io_device_net( void )
{
    m_pLastFile  = NULL;
    m_LastOffset = -1;
    m_LastLength = 0;
    m_nSeeks     = 0;           
}

//==============================================================================

io_device_net::~io_device_net( void )
{
}

//==============================================================================
void io_device_net::Init( void )
{
    const char          WaitChars[] = "-\\|/";
    s32                 count;
    interface_info      Info;
    fileserver_request  Request;
    fileserver_reply    Reply;
    xtimer              Timeout;
    s32                 Retries;
    s32                 i;

    SimpleDialog("Please wait.\n"
        "Initializing network layer.");
    net_Init();

#if defined(TARGET_PS2)
    char            Path[64];
    s32             ConfigIndex;
    net_config_list ConfigList;
    s32             status;
    xbool           Done;
    s32             error;

    net_BeginConfig();
    net_ActivateConfig(FALSE);
    // First, try to find ANY network configuration
    x_strcpy(Path,"mc0:BWNETCNF/BWNETCNF");
    status = net_GetConfigList(Path,&ConfigList);
    if ( (status < 0) || (ConfigList.Count <= 0) )
    {
        x_strcpy(Path,"mc1:BWNETCNF/BWNETCNF");
        status = net_GetConfigList(Path,&ConfigList);
        if ( (status < 0) || (ConfigList.Count <= 0) )
        {
            SimpleDialog("No network configurations present.",1.5f);
            ASSERT(FALSE);
        }
    }

    // Just use the first one.
    ConfigIndex = 0;
    status = net_SetConfiguration(Path,ConfigIndex);
    ASSERT(status >=0);
    Timeout.Reset();
    Timeout.Start();
    Done = FALSE;

    while (Timeout.ReadSec() < 30.0f)
    {
        error = 0;
        error = net_GetAttachStatus(error);
        if ( (error==ATTACH_STATUS_CONFIGURED) ||
            (error==ATTACH_STATUS_ATTACHED) )
        {
            net_ActivateConfig(TRUE);

            // Wait until DHCP assigns us an address
            while ( Timeout.ReadSec() < 30.0f )
            {
                SimpleDialog((const char*)xfs("Please wait...\n"
                    "Connecting to the network.\n"
                    "Timeout remaining: %d",30 - (s32)Timeout.ReadSec()) );
                net_GetInterfaceInfo(-1,Info);
                if (Info.Address)
                {
                    Done = TRUE;
                    break;
                }
            }
            ASSERT(Done);
            break;
        }
        else
        {
            // Invalid net config file - its fatal!
            ASSERT( 0 );
        }
    }
    net_EndConfig();

#endif

    count = 0;
    while(1)
    {
        count++;
        SimpleDialog(xfs("Please wait.\n"
            "Waiting for network to start.\n"
            "%c",WaitChars[(count>>3) & 3]),0.0f);
        net_GetInterfaceInfo(-1, Info);
        if( Info.Address )
        {
            break;
        }
    }

    m_LocalSocket.Bind(-1,NET_FLAGS_BROADCAST);

    Retries = 0;
    xbool Connected = FALSE;

    Lock();
    while( !Connected )
    {
        x_memset(&Request,0,sizeof(Request));
        Request.Header.Type = FSRV_REQ_FIND;

        // Send the request to up-to 8 ports. This will allow us to run multiple instances of the fileserver
        // for xbox devkits
        for( i = FILESERVER_PORT; i < FILESERVER_PORT+8; i++ )
        {
            m_LocalSocket.Send(net_address(-1,i),&Request,sizeof(Request));
        }
        Timeout.Reset();
        Timeout.Start();

        while( Timeout.ReadSec() < 1.0f )
        {
            s32     Size;
            xbool   Received;

            SimpleDialog(xfs("Please wait.\n"
                "Connecting to fileserver.\n"
                "Local Address is %s\n"
                "Attempt %d\n"
                "%c",net_address(Info.Address,0).GetStrIP(), Retries, WaitChars[(count>>3) & 3]),0.0f);
            count++;
            Received = m_LocalSocket.Receive(m_ServerAddress,&Request,Size);
            if( Received )
            {
                // Set up the server we want here.
                Connected = TRUE;
                break;
            }
        }
        Retries++;
    }

    SimpleDialog( xfs("Server connected.\n"
        "Address is %s.",m_ServerAddress.GetStrAddress()), 1.0f );

    x_memset(&Request,0,sizeof(Request));
    Request.Header.Type = FSRV_REQ_RESET;

    m_LocalSocket.Send(m_ServerAddress,&Request,sizeof(Request));
    WaitForReply(Reply);

    Unlock();
    // Base class initialization
    io_device::Init();

}

//==============================================================================
void io_device_net::Kill( void )
{
    // We should be killing the network here
    io_device::Kill();
}

//==============================================================================
void io_device_net::LogPhysRead( io_device_file* pFile, s32 Length, s32 Offset )
{
    (void)pFile;
    (void)Length;
    (void)Offset;

#ifdef LOG_PHYSICAL_SEEK
    // Same file?
    if( m_pLastFile != pFile )
    {
        LOG_MESSAGE( LOG_PHYSICAL_SEEK, "SEEK! Different File: %s", pFile->Filename );
        m_nSeeks++;
    }
    // Need to seek?
    else if( Offset != (m_LastOffset+m_LastLength) )
    {
        LOG_MESSAGE( LOG_PHYSICAL_SEEK, "SEEK! Prev Offset: %d, Len: %d, Curr Offset: %d, Len: %d, Delta %d", m_LastOffset, m_LastLength, Offset, Length, Offset-m_LastOffset );
        m_nSeeks++;
    }

    // Update 'em
    m_pLastFile  = pFile;
    m_LastOffset = Offset;
    m_LastLength = Length;
#endif // LOG_PHYSICAL_SEEK

#ifdef LOG_PHYSICAL_READ
    LOG_MESSAGE( LOG_PHYSICAL_READ, "READ! File: %s, Offset: %d, Length: %d", pFile->Filename, Offset, Length );
#endif // LOG_PHYSICAL_READ
}




//==============================================================================
static void ReadCallback( s32 Result, void* pFileInfo )
{
    (void)pFileInfo;

    // We are in the callback
    g_IODeviceNET.EnterCallback();

    // Success?
    if( Result >= 0 )
    {
        // Its all good!
        ProcessEndOfRequest( &g_IODeviceNET, io_request::COMPLETED );
    }
    else
    {
        // Ack failed!
        ProcessEndOfRequest( &g_IODeviceNET, io_request::FAILED );
    }

    // Done with callback
    g_IODeviceNET.LeaveCallback();
}

//==============================================================================
void io_device_net::CleanFilename( char* pClean, const char* pFilename )
{
    // Gotta fit.
    ASSERT( x_strlen(pFilename) + x_strlen(m_Prefix) < IO_DEVICE_FILENAME_LIMIT );

    x_strcpy( pClean, m_Prefix );

    // Move to end of string.
    pClean += x_strlen( pClean );

    // Now clean it.
    while( *pFilename )
    {
        if( (*pFilename == '\\') || (*pFilename == '/') )
        {
            *pClean++ = '\\';
            pFilename++;

            //            while( *pFilename && ((*pFilename == '\\') || (*pFilename == '/')) )
            //                pFilename++;
        }
        else
        {
            *pClean++ = x_toupper(*pFilename++);
        }
    }

    *pClean = 0;
}

//==============================================================================

io_device::device_data* io_device_net::GetDeviceData( void )
{
    return &s_DeviceData_NET;
}

//==============================================================================
xbool io_device_net::PhysicalOpen( const char* pFilename, io_device_file* pFile, io_device::open_flags OpenFlags  )
{
    char                ModeFlags[8];
    char                Filename[256];

    x_strcpy( ModeFlags, "b" );
    if( OpenFlags & io_device::READ )
    {
        x_strcat( ModeFlags, "r" );
    }

    if( OpenFlags & io_device::WRITE )
    {
        x_strcat( ModeFlags, "w" );
    }

    CleanFilename( Filename, pFilename);

    pFile->Handle = SystemOpen( Filename, ModeFlags, pFile->Length );
    return (pFile->Handle != NULL);
}

//==============================================================================

void io_device_net::PhysicalClose( io_device_file* pFile )
{
    SystemClose( pFile->Handle );
}

//==============================================================================

xbool io_device_net::PhysicalRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    s32 ReadLength;
    (void)AddressSpace;

    LogPhysRead( pFile, Length, Offset );    
    ReadLength = SystemRead( pFile->Handle, pBuffer, Offset, Length );
    ReadCallback( (ReadLength==Length), pFile->pHardwareData );
    return ReadLength==Length;
}

//==============================================================================

xbool io_device_net::PhysicalWrite( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{

    s32 WriteLength;
    (void)AddressSpace;

    WriteLength = SystemWrite( pFile->Handle, pBuffer, Offset, Length );
    ReadCallback( (WriteLength==Length), pFile->pHardwareData );

    return WriteLength==Length;
}

//==============================================================================
void* io_device_net::SystemOpen( const char* pFilename, const char* Mode, s32& Length )
{
    fileserver_request  Request;
    fileserver_reply    Reply;

    Request.Header.Type = FSRV_REQ_OPEN;

    x_strcpy(Request.Open.Filename, pFilename );
    x_strcpy(Request.Open.Mode, Mode);

    Lock();

    g_IODeviceNET.m_LocalSocket.Send( g_IODeviceNET.m_ServerAddress, &Request, sizeof(Request.Open)+sizeof(Request.Header) );
    xbool Ok = WaitForReply(Reply);

    Unlock();

    if( !Ok || (Reply.Open.Status != 0) )
    {
        Length = 0;
        return NULL;
    }

    Length = Reply.Open.Length;
    return (void*)Reply.Open.Handle;
}


//==============================================================================

s32 io_device_net::SystemWrite( void* Handle, const void* pBuffer, s32 Offset, s32 Length )
{
    fileserver_request  Request;
    fileserver_reply    Reply;
    xbool               ok;
    byte*               pBuff = (byte*)pBuffer;
    s32                 SequenceNumber;
    s32                 BlockLength;
    s32                 Written;

    SequenceNumber = 0;     // Make this x_rand
    Written = 0;

    Lock();

    while( Length )
    {
        BlockLength = MIN( Length, (s32)sizeof(Request.Write.Data) );

        Request.Header.Type      = FSRV_REQ_WRITE;
        Request.Write.Handle     = (s32)Handle;
        Request.Header.Sequence  = SequenceNumber;
        Request.Write.Length     = BlockLength;
        Request.Write.Position   = Offset;
        x_memcpy( Request.Write.Data, pBuff, BlockLength );
        m_LocalSocket.Send( m_ServerAddress, &Request, sizeof(Request.Write)+sizeof(Request.Header) );
        ok = WaitForReply( Reply, 2.0f );
        if( ok && (SequenceNumber == Reply.Header.Sequence) )
        {
            SequenceNumber++;
            Offset += BlockLength;
            pBuff  += BlockLength;
            Length -= BlockLength;
            Written+= BlockLength;

        }
        else
        {
            x_DebugMsg( "io_device_net::SystemWrite - Had to retry write request.\n" );
        }
    }
    Unlock();

    return Written;
}

//==============================================================================
s32 io_device_net::SystemRead( void* Handle, void* pBuffer, s32 Offset, s32 Length )
{
    struct pending_read
    {
        s32     Sequence;
        s32     Offset;
        byte*   pBuffer;
        s32     Length;
    };

    fileserver_request  Request;
    fileserver_reply    Reply;
    xbool               ok;
    byte*               pBuff = (byte*)pBuffer;
    s32                 BlockLength;
    s32                 OutstandingRequests;
    pending_read        RequestTags[MAX_CONCURRENT_REQUESTS];
    xtimer              Timeout;
    s32                 SequenceNumber;
    s32                 i;
    s32                 RequestsToRetry;
    s32                 OriginalLength;

    Lock();
    SequenceNumber = 0;     // Make this x_rand

    x_memset(RequestTags,-1,sizeof(RequestTags));
    OriginalLength = Length;

    while( Length )
    {
        // Send up-to 32 requests in one go
        OutstandingRequests = 0;
        //
        // Send up-to 32 requests in one go. Each block will be 1000 bytes.
        //
        while( Length && (OutstandingRequests < MAX_CONCURRENT_REQUESTS) )
        {
            BlockLength = MIN( Length, (s32)sizeof(Reply.Read.Data) );

            RequestTags[OutstandingRequests].Sequence   = SequenceNumber++;
            RequestTags[OutstandingRequests].Length     = BlockLength;
            RequestTags[OutstandingRequests].Offset     = Offset;
            RequestTags[OutstandingRequests].pBuffer    = pBuff;

            Request.Header.Type     = FSRV_REQ_READ;
            Request.Read.Handle     = (s32)Handle;
            Request.Header.Sequence = RequestTags[OutstandingRequests].Sequence;
            Request.Read.Length     = RequestTags[OutstandingRequests].Length;
            Request.Read.Position   = RequestTags[OutstandingRequests].Offset;
            m_LocalSocket.Send( m_ServerAddress, &Request, sizeof(Request.Read)+sizeof(Request.Header) );

            OutstandingRequests++;
            Length   -= BlockLength;
            Offset   += BlockLength;
            pBuff    += BlockLength;
        }

        //
        // Wait for the responses from those requests. Retry if necessary.
        //
        while( OutstandingRequests > 0 )
        {
            while( OutstandingRequests > 0 )
            {
                ok = WaitForReply(Reply,0.25f);
                if( !ok )
                {
                    // We must have taken 0.25 seconds to get a packet back so lets just
                    // bail out now.
                    break;
                }

                for( i=0; i< MAX_CONCURRENT_REQUESTS; i++ )
                {
                    if( RequestTags[i].Sequence == Reply.Header.Sequence)
                    {
                        break;
                    }
                }

                // If we don't find the request in the list, this means we got a double response
                if( i!= MAX_CONCURRENT_REQUESTS )
                {
                    if( RequestTags[i].Length == Reply.Read.Length )
                    {
                        x_memcpy( RequestTags[i].pBuffer, Reply.Read.Data, RequestTags[i].Length );
                        RequestTags[i].Sequence = -1;
                        OutstandingRequests--;
                    }
                }
                else
                {
                    x_DebugMsg("io_device_net::PhysicalRead - Got duplicate response from server.\n");
                }
            }

            // WaitForReply deals with the timeout :)
            // If we don't get an ack within a half second, then we're going to start sending
            // the requests again. So we go through all the pending requests and send those
            // that have not been acked again. We DO NOT care about the order in which the
            // requests have been received (the sequence# takes care of that problem).
            RequestsToRetry = OutstandingRequests;
            i = 0;

            Timeout.Reset();
            Timeout.Start();
            while( RequestsToRetry )
            {
                // If we assert here, then we have lost count of the number of outstanding
                // requests.
                ASSERT( i != MAX_CONCURRENT_REQUESTS );
                if( RequestTags[i].Sequence != -1 )
                {
                    RequestsToRetry--;
                    RequestTags[i].Sequence = SequenceNumber++;
                    Request.Header.Type     = FSRV_REQ_READ;
                    Request.Read.Handle     = (s32)Handle;
                    Request.Header.Sequence = RequestTags[i].Sequence;
                    Request.Read.Length     = RequestTags[i].Length;
                    Request.Read.Position   = RequestTags[i].Offset;
                    x_DebugMsg("io_device_net::PhysicalRead - Retrying request sequence %d\n",RequestTags[i].Sequence);
                    m_LocalSocket.Send( m_ServerAddress, &Request, sizeof(Request.Read)+sizeof(Request.Header) );
                }
                i++;
            }
        }

        ASSERT( Length >= 0 );
    }

    Unlock();

    return OriginalLength;
}

//==============================================================================
void io_device_net::SystemClose( void* Handle )
{
    fileserver_request  Request;
    fileserver_reply    Reply;

    Lock();

    Request.Header.Type     = FSRV_REQ_CLOSE;
    Request.Close.Handle    = (s32)Handle;

    m_LocalSocket.Send( m_ServerAddress, &Request, sizeof(Request.Close)+sizeof(Request.Header) );
    WaitForReply(Reply);

    Unlock();
}

//==============================================================================
xbool io_device_net::WaitForReply( fileserver_reply& Reply, f32 Timeout )
{
    s32         TimeoutTicks;
    xbool       Received;
    net_address Remote;

    // 1 second timeout on all calls.
    TimeoutTicks = (s32)(Timeout * 1000.0f);
    while( TimeoutTicks )
    {
        s32 Length;

        Received = m_LocalSocket.Receive(Remote, &Reply, Length );
        if( Received && (Remote==m_ServerAddress) )
        {
            return TRUE;
        }
        x_DelayThread(1);
        TimeoutTicks--;
    }
    return FALSE;
}

//==============================================================================
void io_device_net::SimpleDialog( const char* pText, f32 Timeout)
{
    xtimer      t;
    const char* pString;
    s32         LineCount;
    s32         y;

    t.Start();

    LineCount = 1;

    pString = pText;
    while (*pString)
    {
        if (*pString=='\n')
        {
            LineCount++;
        }
        pString++;
    }


    do
    {
        char    TextBuffer[128];
        char*   pTextBuffer;

        pString = pText;
        y       = 10 - (LineCount/2);

        if( eng_Begin("Screen Clear") )
        {
            irect Rect(0,0,640,480);
            draw_Rect(Rect,XCOLOR_BLACK,FALSE);
            eng_End();
        }
        while (*pString)
        {
            pTextBuffer = TextBuffer;

            while ( (*pString != 0) && (*pString !='\n') )
            {
                *pTextBuffer++ = *pString++;
            }

            if (*pString == '\n')
            {
                pString++;
            }
            *pTextBuffer = 0x0;
            x_printfxy((40-x_strlen(TextBuffer))/2,y,TextBuffer);
            y++;
        }

        eng_PageFlip();
    } while ( t.ReadSec() < Timeout );
}

//==============================================================================
void io_device_net::Lock( void )
{
    m_LockMutex.Acquire();
}

//==============================================================================
void io_device_net::Unlock( void )
{
    m_LockMutex.Release();

}
#if defined(TARGET_XBOX) && defined(X_LOGGING)
//==============================================================================
//
//*** PLEASE NOTE *** These functions are only required for xtool.
//
//==============================================================================
void* netfs_Open( const char* pFilename, const char* pMode )
{
    void*   Handle;
    s32     Length;

    Handle = g_IODeviceNET.SystemOpen( pFilename, pMode, Length );
    return Handle;
}

//==============================================================================
s32 netfs_Write( void* Handle, const void* pBuffer, s32 Offset, s32 Length )
{
    return g_IODeviceNET.SystemWrite( Handle, pBuffer, Offset, Length );
}

//==============================================================================
s32 netfs_Read( void* Handle, void* pBuffer, s32 Offset, s32 Length )
{
    return g_IODeviceNET.SystemRead( Handle, pBuffer, Offset, Length );
}

//==============================================================================
void netfs_Close( void* Handle )
{
    g_IODeviceNET.SystemClose( Handle );
}

#endif
