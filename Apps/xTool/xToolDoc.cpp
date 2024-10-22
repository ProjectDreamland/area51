// xToolDoc.cpp : implementation of the CxToolDoc class
//

#include "stdafx.h"
#include "xTool.h"

#include "xToolDoc.h"
#include "LogData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

log_packet::log_packet( CxToolDoc* pDoc, char* pData, s32 nBytes, xbool BigEndian )
{
    m_pDocument = pDoc;
    m_pStart    = pData;
    m_pData     = pData;
    m_nBytes    = nBytes;
    m_Size      = nBytes;
    m_BigEndian = BigEndian;
}

log_packet::~log_packet( void )
{
    delete m_pStart;
}

void log_packet::Align( void )
{
    while( m_nBytes & 3 )
    {
        m_pData++;
        m_nBytes--;
    }
}

log_packet& operator >> ( log_packet& Packet, f32& v )
{
    ASSERT( Packet.m_nBytes >= 4 );

    if( Packet.m_BigEndian )
    {
        s32 t = ENDIAN_SWAP_32(*(s32*)Packet.m_pData);
        v = *(f32*)&t;
    }
    else
        v = *(f32*)Packet.m_pData;
    Packet.m_pData += 4; Packet.m_nBytes -= 4;

    return Packet;
}

log_packet& operator >> ( log_packet& Packet, s32& v )
{
    ASSERT( Packet.m_nBytes >= 4 );

    if( Packet.m_BigEndian )
        v = ENDIAN_SWAP_32(*(s32*)Packet.m_pData);
    else
        v = *(s32*)Packet.m_pData;
    Packet.m_pData += 4; Packet.m_nBytes -= 4;

    return Packet;
}

log_packet& operator >> ( log_packet& Packet, u32& v )
{
    ASSERT( Packet.m_nBytes >= 4 );

    if( Packet.m_BigEndian )
        v = ENDIAN_SWAP_32(*(u32*)Packet.m_pData);
    else
        v = *(u32*)Packet.m_pData;
    Packet.m_pData += 4; Packet.m_nBytes -= 4;

    return Packet;
}

log_packet& operator >> ( log_packet& Packet, s8& v )
{
    ASSERT( Packet.m_nBytes >= 1 );

    v = *(s8*)Packet.m_pData;
    Packet.m_pData += 1; Packet.m_nBytes -= 1;

    return Packet;
}

log_packet& operator >> ( log_packet& Packet, u8& v )
{
    ASSERT( Packet.m_nBytes >= 1 );

    v = *(u8*)Packet.m_pData;
    Packet.m_pData += 1; Packet.m_nBytes -= 1;

    return Packet;
}

log_packet& operator >> ( log_packet& Packet, xtick& v )
{
    ASSERT( Packet.m_nBytes >= 8 );

    if( Packet.m_BigEndian )
        v = ENDIAN_SWAP_64(*(xtick*)Packet.m_pData);
    else
        v = *(xtick*)Packet.m_pData;
    Packet.m_pData += 8; Packet.m_nBytes -= 8;

    return Packet;
}

log_packet& operator >> ( log_packet& Packet, const char*& pString )
{
    pString = Packet.m_pData;
    while( (Packet.m_nBytes > 0) && (*Packet.m_pData) )
    {
        Packet.m_pData++;
        Packet.m_nBytes--;
    }
    Packet.m_pData++;
    Packet.m_nBytes--;

    ASSERT( Packet.m_nBytes >= 0 );

    return Packet;
}

/////////////////////////////////////////////////////////////////////////////
// ConnectionThread

void DisplayConnectionError( void )
{
#if 0 // Enable this for debugging only
    LPVOID lpMsgBuf;
    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
    );
    // Process any inserts in lpMsgBuf.
    // ...
    // Display the string.
    ::MessageBox( NULL, (LPCTSTR)lpMsgBuf, CResString(IDS_ERROR), MB_OK | MB_ICONINFORMATION );
    // Free the buffer.
    LocalFree( lpMsgBuf );
#endif
}

/////////////////////////////////////////////////////////////////////////////

VOID WINAPI HeaderReadComplete( DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap );
VOID WINAPI BodyReadComplete  ( DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap );

/////////////////////////////////////////////////////////////////////////////

VOID WINAPI HeaderReadComplete( DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap )
{
    // Get the connection object
    debug_connection*   pConnection = (debug_connection*)lpOverLap;
    ASSERT( pConnection );

    // Check for errors?
    if( (dwErr == 0) && (cbBytesRead != 0) )
    {
        // Check if we got all the data, continue the read if not
        pConnection->m_BytesLeftToRead -= cbBytesRead;
        pConnection->m_pRead           += cbBytesRead;
        if( pConnection->m_BytesLeftToRead > 0 )
        {
            // Continue the read
            ZeroMemory( &pConnection->m_Overlapped, sizeof(pConnection->m_Overlapped) );
            BOOL Success = ReadFileEx( pConnection->m_hPipe, pConnection->m_pRead, pConnection->m_BytesLeftToRead, (LPOVERLAPPED)pConnection, HeaderReadComplete );
            if( !Success )
            {
                // Drop the connection
                pConnection->m_IsConnected = false;

                // Display the error
                DisplayConnectionError();
            }
        }
        else
        {
            // ByteSwap header
            if( pConnection->m_pDoc->IsBigEndian() )
                pConnection->m_PacketHeader.ByteSwap();

            // Resize connection buffer to read message into
            if( pConnection->m_PacketHeader.Length > pConnection->m_PacketDataLength )
            {
                // Save new length and reallocate the buffer
                pConnection->m_PacketDataLength = pConnection->m_PacketHeader.Length;
                pConnection->m_pPacketData = realloc( pConnection->m_pPacketData, pConnection->m_PacketDataLength );
            }
            ASSERT( pConnection->m_pPacketData );

            // Issue body read
            ZeroMemory( &pConnection->m_Overlapped, sizeof(pConnection->m_Overlapped) );
            pConnection->m_BytesLeftToRead = pConnection->m_PacketHeader.Length;
            pConnection->m_pRead           = (char*)pConnection->m_pPacketData;
            BOOL Success = ReadFileEx( pConnection->m_hPipe, pConnection->m_pRead, pConnection->m_BytesLeftToRead, (LPOVERLAPPED)pConnection, BodyReadComplete );
            if( !Success )
            {
                // Drop the connection
                pConnection->m_IsConnected = false;

                // Display the error
                DisplayConnectionError();
            }
        }
    }
    else
    {
        // Drop connection after an error
        pConnection->m_IsConnected = false;
    }
}

/////////////////////////////////////////////////////////////////////////////

VOID WINAPI BodyReadComplete( DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap )
{
    // Get the connection object
    debug_connection*   pConnection = (debug_connection*)lpOverLap;
    ASSERT( pConnection );
    CxToolDoc*      pDoc        = pConnection->m_pDoc;
    ASSERT( pDoc );

    // Check for errors?
    if( (dwErr == 0) && (cbBytesRead != 0) && pConnection->m_IsConnected )
    {
        // Check if we got all the data, continue the read if not
        pConnection->m_BytesLeftToRead -= cbBytesRead;
        pConnection->m_pRead           += cbBytesRead;
        if( pConnection->m_BytesLeftToRead > 0 )
        {
            // Continue the read
            ZeroMemory( &pConnection->m_Overlapped, sizeof(pConnection->m_Overlapped) );
            BOOL Success = ReadFileEx( pConnection->m_hPipe, pConnection->m_pRead, pConnection->m_BytesLeftToRead, (LPOVERLAPPED)pConnection, BodyReadComplete );
            if( !Success )
            {
                // Drop the connection
                pConnection->m_IsConnected = false;

                // Display the error
                DisplayConnectionError();
            }
        }
        else
        {
/*
            // Check CRC on data
            u32 CRC =0;
            for( s32 i=0 ; i<pConnection->m_PacketHeader.Length ; i++ )
            {
                CRC = CRC ^ ((CRC >> 31) & 1);
                CRC <<= 1;
                CRC += ((u8*)pConnection->m_pPacketData)[i];
            }
            ASSERT( CRC == pConnection->m_PacketHeader.CRC );
*/
            // Create a packet and post a message to process it
            log_packet* pPacket = new log_packet( pDoc, (char*)pConnection->m_pPacketData, pConnection->m_PacketHeader.Length, pDoc->IsBigEndian() );
            ASSERT( pPacket );
            pConnection->m_pPacketData = NULL;
            pConnection->m_PacketDataLength = 0;
            AfxGetMainWnd()->PostMessage( AM_LOG_PACKET, (WPARAM)pPacket, 0 );

            // Issue new header read
            ZeroMemory( &pConnection->m_Overlapped, sizeof(pConnection->m_Overlapped) );
            pConnection->m_BytesLeftToRead = sizeof(pConnection->m_PacketHeader);
            pConnection->m_pRead           = (char*)&pConnection->m_PacketHeader;
            BOOL Success = ReadFileEx( pConnection->m_hPipe, &pConnection->m_PacketHeader, sizeof(pConnection->m_PacketHeader), (LPOVERLAPPED)pConnection, HeaderReadComplete );
            if( !Success )
            {
                // Drop the connection
                pConnection->m_IsConnected = false;

                // Display the error
                DisplayConnectionError();
            }
        }
    }
    else
    {
        // Drop connection after an error
        pConnection->m_IsConnected = false;
    }
}

/////////////////////////////////////////////////////////////////////////////

UINT ConnectionThread( LPVOID pParam )
{
    // Get connection and document pointers
    debug_connection* pConnection = (debug_connection*)pParam;
    ASSERT( pConnection );
    CxToolDoc* pDoc = pConnection->m_pDoc;
    ASSERT( pDoc );

    // Call the document function to run the connection
    return pDoc->RunConnection( pConnection );
}

/////////////////////////////////////////////////////////////////////////////

UINT CxToolDoc::RunConnection( debug_connection* pConnection )
{
    BOOL                        Success;
    DWORD                       BytesRead;
    xtool::connect_message      Connect;
    HANDLE                      hPipe;

    // Get pipe handle
    hPipe = pConnection->m_hPipe;
    ASSERT( hPipe );

    // Read the connect message from the client
    Success = ReadFile( hPipe, &Connect, sizeof(Connect), &BytesRead, NULL );
    if( !Success || (BytesRead != sizeof(Connect)) ||
        ((strncmp(Connect.Magic, "XDBG", 4) != 0) && (strncmp(Connect.Magic, "GBDX", 4) != 0)) )
    {
        // Connection failed
        ::MessageBox( NULL, CResString(IDS_ERROR_BAD_COMMS_HEADER), CResString(IDS_CONNECTION_ERROR), MB_ICONSTOP );
        return 1;
    }

    // Set connected flag
    pConnection->m_IsConnected = true;

    // Set endian flag
    m_BigEndian = ( strncmp(Connect.Magic, "GBDX", 4) == 0 );

    // Byte swap the rest of the data if necessary
    if( IsBigEndian() )
        Connect.ByteSwap();

    // Set platform name and endian flag for connection
    switch( Connect.Platform )
    {
    case PLATFORM_PC:
        m_PlatformName = _T("PC");
        break;
    case PLATFORM_GCN:
        m_PlatformName = _T("GCN");
        break;
    case PLATFORM_PS2:
        m_PlatformName = _T("PS2");
        break;
    case PLATFORM_XBOX:
        m_PlatformName = _T("XBOX");
        break;
    default:
        ASSERTS( 0, "Bad platform designation" );
        m_PlatformName = _T("<unknown>");
        break;
    }

    // Read remaining connect info
    m_Platform        = (platform)Connect.Platform;
    m_ApplicationName = Connect.ApplicationName;
    m_TicksPerSecond  = (double)Connect.TicksPerSecond;
    m_BaselineTicks   = (double)Connect.BaselineTicks;

    // Set heap range if available
    if( (Connect.HeapStart != 0) || (Connect.HeapEnd != 0) )
    {
        m_HeapState.SetMinAddress( Connect.HeapStart );
        m_HeapState.SetMaxAddress( Connect.HeapEnd   );
    }

    // Change the document name to reflect the new information received
    AfxGetMainWnd()->PostMessage( AM_REBUILD_DOC_TITLE, (WPARAM)this, 0 );

    // Issue read for message header
    ZeroMemory( &pConnection->m_Overlapped, sizeof(pConnection->m_Overlapped) );
    pConnection->m_BytesLeftToRead = sizeof(pConnection->m_PacketHeader);
    pConnection->m_pRead           = (char*)&pConnection->m_PacketHeader;
    Success = ReadFileEx( hPipe, &pConnection->m_PacketHeader, sizeof(pConnection->m_PacketHeader), (LPOVERLAPPED)pConnection, HeaderReadComplete );
    if( !Success )
    {
        // Drop the connection
        pConnection->m_IsConnected = false;

        // Display the error message
        DisplayConnectionError();
    }
    else
    {
        // Loop until connection broken
        while( pConnection->m_IsConnected )
        {
            // Wait for exit event or completion routine to be queued
            DWORD dwWait = WaitForSingleObjectEx( pConnection->m_hExitEvent, INFINITE, TRUE );

            // Test outcome of wait
            switch( dwWait )
            {
            case WAIT_IO_COMPLETION:
                // IO completed - nothing to do since the Callbacks do it all
                break;
            case WAIT_OBJECT_0:
                // Exit event signaled - drop the connection
                pConnection->m_IsConnected = false;
                break;
            default:
                // Error of some kind - drop the connection
                pConnection->m_IsConnected = false;
                break;
            }
        }
    }

    // Close the Pipe
    DisconnectNamedPipe( hPipe );
    CloseHandle( hPipe );
    pConnection->m_hPipe = NULL;

    // Tell main thread we have exited
    SetEvent( pConnection->m_hExitedEvent );

    // Exit thread
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CxToolDoc

IMPLEMENT_DYNCREATE(CxToolDoc, CDocument);

BEGIN_MESSAGE_MAP(CxToolDoc, CDocument)
	//{{AFX_MSG_MAP(CxToolDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CxToolDoc construction/destruction

CxToolDoc::CxToolDoc()
{
    m_TicksPerSecond        = 1;
    m_BigEndian             = false;
    m_pConnection           = NULL;
    m_pConnectionThread     = NULL;
    m_LogSequence           = 1;
    m_LogSortField          = -1;
    m_LogSortAscending      = TRUE;
    m_MemLogSortField       = -1;
    m_MemLogSortAscending   = TRUE;
    m_ChannelsSortField     = -1;
    m_ChannelsSortAscending = TRUE;
    m_LogViewFixedFont      = FALSE;
    m_LogViewMessages       = TRUE;
    m_LogViewWarnings       = TRUE;
    m_LogViewErrors         = TRUE;
    m_LogViewRTFs           = TRUE;
    m_LogViewMemory         = FALSE;
    m_MinAddress            = U32_MAX;
    m_MaxAddress            = U32_MIN;
}                         

/////////////////////////////////////////////////////////////////////////////

CxToolDoc::~CxToolDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

BOOL CxToolDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

    // Record this as a valid document
    g_Documents.Add( (void*)this );

    // Read the default disabled channels list from the registry
    LoadDisabledChannels();

    // Pull next incoming connection off the queue and spawn a thread to process it
    debug_connection* pConnection = g_ConnectionList.GetNext();
    if( pConnection )
    {
        // Make sure the connection has a pipe associated with it
        ASSERT( pConnection->m_hPipe );

        // Set the document pointer in the connection to this and keep a record of the connection
        pConnection->m_pDoc = this;
        m_pConnection = pConnection;

        // Create events for the thread exiting
        pConnection->m_hExitEvent   = CreateEvent( NULL, TRUE, FALSE, NULL );
        pConnection->m_hExitedEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

        // Only start the thread if the events were created
        if( pConnection->m_hExitEvent && pConnection->m_hExitedEvent )
        {
            // Start the thread to handle the connection
            m_pConnectionThread = AfxBeginThread( ConnectionThread, pConnection );
        }
        else
        {
            // Event could not be created
            ::MessageBox( NULL, CResString(IDS_ERROR_EXIT_EVENTS), CResString(IDS_ERROR), MB_ICONSTOP );
        }
    }

    // Set time of document creation and use that as the document name
    m_CaptureTime = CTime::GetCurrentTime();
//    CString TimeString = m_CaptureTime.Format( _T("%Y-%m-%d %I.%M %p") );
    CString TimeString = m_CaptureTime.Format( _T("%H.%M") );

    // Create Document name from status and time string combination
    CString DocumentName;
    if( pConnection )
        DocumentName = TimeString;
    else
        DocumentName = _T("No Client - ") + TimeString;

    // Set the document title
    SetTitle( DocumentName );

    // Done
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CxToolDoc serialization

#define VERSION_DOCUMENT 5

void CxToolDoc::Serialize(CArchive& ar)
{
    CDocument::Serialize( ar );

	if (ar.IsStoring())
	{
        ar.SetStoreParams( 2053, 2*1024*1024 );

        ar << VERSION_DOCUMENT;

        ar << m_CaptureTime;
        ar << m_ApplicationName;
        ar << m_PlatformName;
        ar << m_TicksPerSecond;
        ar << m_BaselineTicks;
        ar << m_BigEndian;
        ar << m_HeapState.GetMinAddress();
        ar << m_HeapState.GetMaxAddress();

        m_Log.Serialize( ar, this );
        ar << m_LogSortField;
        ar << m_LogSortAscending;

        m_Channels.Serialize( ar, this );
        ar << m_ChannelsSortField;
        ar << m_ChannelsSortAscending;

        m_PoolCallStack.Serialize( ar );
        m_MapFile.Serialize( ar );
	}
	else
	{
        ar.SetLoadParams( 2*1024*1024 );

        int Version = 0;
        ar >> Version;

        if( Version == VERSION_DOCUMENT )
        {
            ar >> m_CaptureTime;
            ar >> m_ApplicationName;
            ar >> m_PlatformName;
            ar >> m_TicksPerSecond;
            ar >> m_BaselineTicks;
            ar >> m_BigEndian;
            ar >> m_MinAddress;
            ar >> m_MaxAddress;

            m_Log.Serialize( ar, this );
            ar >> m_LogSortField;
            ar >> m_LogSortAscending;

            m_Channels.Serialize( ar, this );
            ar >> m_ChannelsSortField;
            ar >> m_ChannelsSortAscending;

            m_PoolCallStack.Serialize( ar );
            m_MapFile.Serialize( ar );

            FilterLog();
            FilterChannels();
            CreateMemLog();
            FilterMemLog();
            CreateHeapState();
        }
        else
        {
            // Bad file version
            AfxThrowArchiveException( CArchiveException::badSchema, ar.GetFile()->GetFileName() );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CxToolDoc diagnostics

#ifdef _DEBUG
void CxToolDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CxToolDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CxToolDoc commands
/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::RebuildTitle( void )
{
    // Set new title
//    CString Title = m_ApplicationName + _T("(") + m_PlatformName + _T(") ") + m_CaptureTime.Format( _T("%Y-%m-%d %I.%M %p") );
    CString Title = m_ApplicationName + m_CaptureTime.Format( _T(" %H.%M") );
    SetTitle( Title );

    // Clear and pathname that is cached
    m_strPathName.Empty();
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::ProcessPacketLog( log_packet& Packet )
{
    bool    PacketLog       = false;
    bool    PacketMemory    = false;

    // Save current number of filtered log entries for comparison after processing
    int nLogEntries = m_FilteredLog.GetSize();

    // Loop through all the tokens in the packet
    while( !Packet.IsEmpty() )
    {
        u32 LogType;
        Packet >> LogType;

        // Call appropriate processing function
        switch( LogType )
        {
        case xtool::LOG_TYPE_MESSAGE:
            PacketLog = true;
            ProcessPacketLogMessage( Packet );
            break;

        case xtool::LOG_TYPE_MEMORY:
            PacketMemory = true;
            ProcessPacketLogMemory( Packet );
            break;

        case xtool::LOG_TYPE_APPLICATION_NAME:
            PacketMemory = true;
            ProcessPacketLogApplicationName( Packet );
            break;

        case xtool::LOG_TYPE_TIMER_PUSH:
            ProcessPacketLogTimerPush( Packet );
            break;

        case xtool::LOG_TYPE_TIMER_POP:
            ProcessPacketLogTimerPop( Packet );
            break;

        default:
            ASSERT( 0 );
            break;
        }

        // Align to the next token in the packet
        Packet.Align();
    }

    // TODO: Limit the number of log entries

    // Force views to update by sending messages through the main window
    if( PacketLog && (nLogEntries != m_FilteredLog.GetSize()) )
        AfxGetMainWnd()->PostMessage( AM_UPDATE_DOC_VIEWS, (WPARAM)m_pConnection->m_pDoc, CxToolDoc::HINT_NEW_LOG_DATA );
    if( PacketMemory )
        AfxGetMainWnd()->PostMessage( AM_UPDATE_DOC_VIEWS, (WPARAM)m_pConnection->m_pDoc, CxToolDoc::HINT_NEW_MEMORY_DATA );
}

/////////////////////////////////////////////////////////////////////////////
// IMPORTANT: This function is called from the pipe connection thread

void CxToolDoc::ProcessPacketLogMessage( log_packet& Packet )
{
    xtick       Ticks;
    u32         ThreadID;
    s32         Line;
    u8          Severity;
    const char* pChannelName;
    const char* pMessage;
    const char* pFile;

    // Parse the packet
    Packet >> Ticks;
    Packet >> ThreadID;
    Packet >> Line;
    Packet >> Severity;
    Packet >> pChannelName;
    Packet >> pMessage;
    Packet >> pFile;

    // Find channel or add it into the array
    s32 iChannel;
    for( iChannel=0 ; iChannel<m_Channels.GetSize() ; iChannel++ )
    {
        if( (strcmp( m_Channels[iChannel]->GetName(), (const char*)pChannelName ) == 0) &&
            (m_Channels[iChannel]->GetThreadID() == ThreadID) )
            break;
    }
    if( iChannel == m_Channels.GetSize() )
    {
        log_channel* pChannel = new log_channel;
        ASSERT( pChannel );
        pChannel->Init( this, (const char*)pChannelName, ThreadID );

        // Should we init this channel as unchecked
        void* Ptr;
        if( m_ChannelsDisabled.Lookup( (const char*)pChannelName, Ptr ) )
        {
            pChannel->SetFlags( 0, log_channel::flag_checked );
        }

        m_Channels.Add( pChannel );
        m_FilteredChannels.Add( pChannel );
    }

    // Create the log message and add it into the log
    log_message* pLog = m_PoolLogMessage.New(); //new log_message;
    ASSERT( pLog );
    pLog->Init( this, m_LogSequence++, (double)Ticks, ThreadID, iChannel, (xtool::log_severity)Severity, (const char*)pMessage, (const char*)pFile, Line );
    m_Log.Add( pLog );

    // Add into filtered log if channel enabled
    AddEntryToFilteredLog( pLog );

    // Mark the document as modified
    //SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////
// IMPORTANT: This function is called from the pipe connection thread

void CxToolDoc::ProcessPacketLogMemory( log_packet& Packet )
{
    s32         i;
    u32         Operation;
    xtick       Ticks;
    u32         ThreadID;
    u32         Address = 0;
    u32         OldAddress = 0;
    u32         Size = 0;
    s32         Line = 0;
    s32         CallStackDepth;
    s32         CallStackIndex;
    const char* pChannelName = "<memory>";
    const char* pFile;

    // Parse the packet
    Packet >> Operation;
    Packet >> Ticks;
    Packet >> ThreadID;

    switch( Operation )
    {
    case xtool::LOG_MEMORY_MALLOC:
        Packet >> Address;
        Packet >> Size;
        Packet >> Line;
        break;
    case xtool::LOG_MEMORY_REALLOC:
        Packet >> Address;
        Packet >> OldAddress;
        Packet >> Size;
        Packet >> Line;
        break;
    case xtool::LOG_MEMORY_FREE:
        Packet >> Address;
        Packet >> Line;
        break;
    case xtool::LOG_MEMORY_MARK:
        break;
    default:
        ASSERT( 0 );
    }

    // Read CallStack
    Packet >> CallStackDepth;
    u32* pCallStack = m_PoolCallStack.NewArray( CallStackDepth+1, CallStackIndex );
    ASSERT( pCallStack );
    for( i=0 ; i<CallStackDepth ; i++ )
        Packet >> pCallStack[i];
    pCallStack[CallStackDepth] = 0;

    // Read FileName or MarkName
    Packet >> pFile;

    // Find channel or add it into the array
    s32 iChannel;
    for( iChannel=0 ; iChannel<m_Channels.GetSize() ; iChannel++ )
    {
        if( (strcmp( m_Channels[iChannel]->GetName(), (const char*)pChannelName ) == 0) &&
            (m_Channels[iChannel]->GetThreadID() == ThreadID) )
            break;
    }
    if( iChannel == m_Channels.GetSize() )
    {
        log_channel* pChannel = new log_channel;
        ASSERT( pChannel );
        pChannel->Init( this, (const char*)pChannelName, ThreadID );
        m_Channels.Add( pChannel );
        m_FilteredChannels.Add( pChannel );
    }

    // Create the log entry and add it into the log
    log_memory* pLog = m_PoolLogMemory.New(); //new log_memory;
    ASSERT( pLog );
    pLog->Init( this, m_LogSequence++, (double)Ticks, ThreadID, iChannel, Operation, Address, OldAddress, Size, (const char*)pFile, Line, m_MemMarks.GetSize()-1, CallStackIndex );
    m_Log.Add( pLog );

    // Add into filtered log if channel enabled
    AddEntryToFilteredLog( pLog );

    // Add into mark log if it's a mark
    if( Operation == xtool::LOG_MEMORY_MARK )
    {
        m_MemMarks.Add( pLog );
    }

    // Add into memory log
    m_MemLog.Add( pLog );
    m_FilteredMemLog.Add( pLog );

    // Apply to current memory map
    m_HeapState.ApplyOperation( pLog, TRUE );

    // Update Heap ends
    m_MinAddress = MIN( m_MinAddress, m_HeapState.GetMinAddress() );
    m_MaxAddress = MAX( m_MaxAddress, m_HeapState.GetMaxAddress() );

    // Mark the document as modified
    //SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////
// IMPORTANT: This function is called from the pipe connection thread

void CxToolDoc::ProcessPacketLogApplicationName( log_packet& Packet )
{
    const char* pName;

    // Parse the packet
    Packet >> pName;

    // Save the name and send a request to rename the window
    m_ApplicationName = pName;
    AfxGetMainWnd()->PostMessage( AM_REBUILD_DOC_TITLE, (WPARAM)this, 0 );

    // Mark the document as modified
    //SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////
// IMPORTANT: This function is called from the pipe connection thread

void CxToolDoc::ProcessPacketLogTimerPush( log_packet& Packet )
{
    xtick       Ticks;
    u32         ThreadID;

    // Parse the packet
    Packet >> Ticks;
    Packet >> ThreadID;

    m_TimerStack.Append( Ticks );
}

/////////////////////////////////////////////////////////////////////////////
// IMPORTANT: This function is called from the pipe connection thread

void CxToolDoc::ProcessPacketLogTimerPop( log_packet& Packet )
{
    xtick       Ticks;
    u32         ThreadID;
    f32         WarningTimeLimit;
    const char* pChannelName;
    const char* pMessage;

    // Parse the packet
    Packet >> Ticks;
    Packet >> ThreadID;
    Packet >> WarningTimeLimit;
    Packet >> pChannelName;
    Packet >> pMessage;

    // Find channel or add it into the array
    s32 iChannel;
    for( iChannel=0 ; iChannel<m_Channels.GetSize() ; iChannel++ )
    {
        if( (strcmp( m_Channels[iChannel]->GetName(), (const char*)pChannelName ) == 0) &&
            (m_Channels[iChannel]->GetThreadID() == ThreadID) )
            break;
    }
    if( iChannel == m_Channels.GetSize() )
    {
        log_channel* pChannel = new log_channel;
        ASSERT( pChannel );
        pChannel->Init( this, (const char*)pChannelName, ThreadID );

        // Should we init this channel as unchecked
        void* Ptr;
        if( m_ChannelsDisabled.Lookup( (const char*)pChannelName, Ptr ) )
        {
            pChannel->SetFlags( 0, log_channel::flag_checked );
        }

        m_Channels.Add( pChannel );
        m_FilteredChannels.Add( pChannel );
    }

    // Create the log message
    log_message* pLog = m_PoolLogMessage.New(); //new log_message;
    ASSERT( pLog );

    // Check for timer stack underflow
    if( m_TimerStack.GetCount() > 0 )
    {
        xbool Overflow = (m_TimerStack.GetCount() > 100);
        xtick Interval = Ticks - m_TimerStack[m_TimerStack.GetCount()-1];
        m_TimerStack.SetCount( m_TimerStack.GetCount()-1 );

        xtool::log_severity Severity = xtool::LOG_SEVERITY_MESSAGE;
        if( (Interval / GetTicksPerSecond()) >= (WarningTimeLimit / 1000.0f) )
            Severity = xtool::LOG_SEVERITY_WARNING;
        if( Overflow )
            Severity = xtool::LOG_SEVERITY_ERROR;

        pLog->Init( this, m_LogSequence++, (double)Ticks, ThreadID, iChannel, xtool::LOG_SEVERITY_MESSAGE, pMessage, "", 0 );
    }
    else
    {
        pLog->Init( this, m_LogSequence++, (double)Ticks, ThreadID, iChannel, xtool::LOG_SEVERITY_ERROR, xfs( "Timer Stack Underflow - %s", pMessage ), "", 0 );
    }

    // Add into log
    m_Log.Add( pLog );

    // Add into filtered log if channel enabled
    AddEntryToFilteredLog( pLog );

    // Mark the document as modified
    //SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////

static int  s_LogSortField     = 0;
static BOOL s_LogSortAscending = TRUE;

int _cdecl fnCompareLog( const void* p1, const void* p2 )
{
    int SortCode = 0;

    ASSERT( p1 );
    ASSERT( p2 );

    log_entry* pLog1 = *(log_entry**)p1;
    log_entry* pLog2 = *(log_entry**)p2;

    ASSERT( pLog1 );
    ASSERT( pLog2 );

    switch( s_LogSortField )
    {
    case 0:
        SortCode = pLog1->GetSequence() - pLog2->GetSequence();
        break;
    case 1:
        {
            double t = pLog1->GetTicks() - pLog2->GetTicks();
            if( t == 0.0 )
                SortCode = 0;
            else if( t > 0.0 )
                SortCode = 1;
            else
                SortCode = -1;
        }
        break;
    case 2:
        SortCode = strcmp( pLog1->GetChannel(), pLog2->GetChannel() );
        break;
    case 3:
        SortCode = pLog1->GetThreadID() - pLog2->GetThreadID();
        break;
    case 4:
        SortCode = strcmp( pLog1->GetMessage(), pLog2->GetMessage() );
        break;
    case 5:
        SortCode = pLog1->GetLine() - pLog2->GetLine();
        break;
    case 6:
        SortCode = strcmp( pLog1->GetFile(), pLog2->GetFile() );
        break;
    }

    if( !s_LogSortAscending )
        SortCode = -SortCode;

    return SortCode;
}

/////////////////////////////////////////////////////////////////////////////

extern void hsort(char* base,int nel,int width,compare_fn compare);

void CxToolDoc::SortLog( int Field, BOOL Ascending )
{
    // Sort the filtered log
    s_LogSortField     = Field;
    s_LogSortAscending = Ascending;
    hsort( (char*)m_FilteredLog.GetData(), m_FilteredLog.GetSize(), 4, fnCompareLog );
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::AddEntryToFilteredLog( log_entry* pEntry )
{
        int                 ChannelID   = pEntry->GetChannelID();
        xtool::log_severity Severity    = pEntry->GetSeverity();

        BOOL    ChannelFiltered     = (ChannelID != -1) && !m_Channels[ChannelID]->GetFlags( log_channel::flag_checked );
        BOOL    SeverityFiltered    = ( ((Severity == xtool::LOG_SEVERITY_MESSAGE) && !GetLogViewMessages()) || 
                                        ((Severity == xtool::LOG_SEVERITY_WARNING) && !GetLogViewWarnings()) || 
                                        ((Severity == xtool::LOG_SEVERITY_ERROR  ) && !GetLogViewErrors  ()) || 
                                        ((Severity == xtool::LOG_SEVERITY_ASSERT ) && !GetLogViewRTFs    ()) );
        BOOL    IsMemory            = pEntry->GetType() == log_entry::type_memory;
        BOOL    MemoryFiltered      = IsMemory && !GetLogViewMemory();

        if( !ChannelFiltered && !SeverityFiltered && !MemoryFiltered )
        {
            m_FilteredLog.Add( pEntry );
        }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::FilterLog( void )
{
    m_FilteredLog.m_Array.SetCount( 0 );

    for( int i=0 ; i<m_Log.m_Array.GetCount() ; i++ )
    {
        log_entry* pEntry = (log_entry*)m_Log.m_Array[i];
        AddEntryToFilteredLog( pEntry );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::ClearLog( void )
{
    m_Log.Clear();
    m_FilteredLog.Clear();

    UpdateAllViews( NULL, HINT_LOG_FILTER );
}

/////////////////////////////////////////////////////////////////////////////

static int  s_ChannelsSortField     = 1;
static BOOL s_ChannelsSortAscending = TRUE;

int _cdecl fnCompareChannels( const void* p1, const void* p2 )
{
    int SortCode = 0;

    ASSERT( p1 );
    ASSERT( p2 );

    log_channel* pChannel1 = *(log_channel**)p1;
    log_channel* pChannel2 = *(log_channel**)p2;

    ASSERT( pChannel1 );
    ASSERT( pChannel2 );

    switch( s_ChannelsSortField )
    {
    case 0:
        SortCode = strcmp( pChannel1->GetName(), pChannel2->GetName() );
        break;
    case 1:
        SortCode = pChannel1->GetThreadID() - pChannel2->GetThreadID();
        break;
    }

    if( !s_ChannelsSortAscending )
        SortCode = -SortCode;

    return SortCode;
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SortChannels( int Field, BOOL Ascending )
{
    // Sort the filtered channels
    s_ChannelsSortField     = Field;
    s_ChannelsSortAscending = Ascending;
    qsort( m_FilteredChannels.GetData(), m_FilteredChannels.GetSize(), sizeof(log_entry*), fnCompareChannels );
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::FilterChannels( void )
{
    m_FilteredChannels.m_Array = m_Channels.m_Array;
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SaveDisabledChannels( void )
{
    // Make sure we're up to date with the registry in case another document saved
    LoadDisabledChannels();

    // Add currently disabled channels into the map and remove currently enabled channels
    for( int i=0 ; i<m_Channels.GetSize() ; i++ )
    {
        // If channel is not checked add it into the map
        if( m_Channels[i]->GetFlags( log_channel::flag_checked ) )
        {
            m_ChannelsDisabled.RemoveKey( m_Channels[i]->GetName() );
        }
        else
        {
            m_ChannelsDisabled.SetAt( m_Channels[i]->GetName(), 0 );
        }
    }

    // Write the map to the registry
    CXTRegistryManager regManager;
    POSITION Pos = m_ChannelsDisabled.GetStartPosition();
    int Count = 0;
    while( Pos )
    {
        CString Key;
        void*   Ptr;
        m_ChannelsDisabled.GetNextAssoc( Pos, Key, Ptr );

        CString Entry;
        Entry.Format( "%d", Count );
        regManager.WriteProfileString( _T("DisabledChannels"), Entry, Key );

        Count++;
    }

	regManager.WriteProfileInt( _T("DisabledChannels"), _T("Count"), Count );
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::LoadDisabledChannels( void )
{
    // Load from registry into the default unchecked map
    CXTRegistryManager regManager;
	int Count  = regManager.GetProfileInt( _T("DisabledChannels"), _T("Count"), 0);
    for( int i=0 ; i<Count ; i++ )
    {
        CString Entry;
        Entry.Format( "%d", i );
        CString Channel = regManager.GetProfileString( _T("DisabledChannels"), Entry, _T("") );
        m_ChannelsDisabled.SetAt( Channel, 0 );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::CreateMemLog( void )
{
    m_MemLog.m_Array.SetCount( 0 );

    for( int i=0 ; i<m_Log.m_Array.GetCount() ; i++ )
    {
        log_entry* pEntry = (log_entry*)m_Log.m_Array[i];
        if( pEntry->GetType() == log_entry::type_memory )
        {
            m_MemLog.m_Array.Append( pEntry );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::CreateHeapState( void )
{
    m_HeapState.Clear();
    m_HeapState.SetMinAddress( m_MinAddress );
    m_HeapState.SetMaxAddress( m_MaxAddress );

    if( m_MemLog.m_Array.GetCount() > 0 )
    {
        log_memory** pEntries = (log_memory**)&m_MemLog.m_Array[0];
        s32          nEntries = m_MemLog.m_Array.GetCount();
        m_HeapState.ApplyOperations( pEntries, nEntries );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::CreateHeapState( double Ticks )
{
    m_HeapState.Clear();
    m_HeapState.SetMinAddress( m_MinAddress );
    m_HeapState.SetMaxAddress( m_MaxAddress );

    log_memory** pEntries = (log_memory**)&m_MemLog.m_Array[0];
    int          nEntries = m_MemLog.m_Array.GetCount();
    int          nToApply = 0;

    // TODO: Binary search to find the last entry to apply
    for( nToApply=0 ; nToApply<nEntries ; nToApply++ )
    {
        if( pEntries[nToApply]->GetTicks() > Ticks )
            break;
    }

    // Update the heap state
    m_HeapState.ApplyOperations( pEntries, nToApply );
}

/////////////////////////////////////////////////////////////////////////////

static int  s_MemLogSortField     = 0;
static BOOL s_MemLogSortAscending = TRUE;

int _cdecl fnCompareMemLog( const void* p1, const void* p2 )
{
    int SortCode = 0;

    ASSERT( p1 );
    ASSERT( p2 );

    log_memory* pLog1 = *(log_memory**)p1;
    log_memory* pLog2 = *(log_memory**)p2;

    ASSERT( pLog1 );
    ASSERT( pLog2 );

    switch( s_MemLogSortField )
    {
    case 0:
        SortCode = pLog1->GetSequence() - pLog2->GetSequence();
        break;
    case 1:
        {
            double t = pLog1->GetTicks() - pLog2->GetTicks();
            if( t == 0.0 )
                SortCode = 0;
            else if( t > 0.0 )
                SortCode = 1;
            else
                SortCode = -1;
        }
        break;
    case 2:
        SortCode = pLog1->GetThreadID() - pLog2->GetThreadID();
        break;
    case 3:
        SortCode = pLog1->GetOperation() - pLog2->GetOperation();
        break;
    case 4:
        SortCode = pLog1->GetAddress() - pLog2->GetAddress();
        break;
    case 5:
        SortCode = pLog1->GetSize() - pLog2->GetSize();
        break;
    case 6:
        SortCode = pLog1->GetCurrentBytes() - pLog2->GetCurrentBytes();
        break;
    case 7:
        SortCode = pLog1->GetLine() - pLog2->GetLine();
        break;
    case 8:
        SortCode = strcmp( pLog1->GetFile(), pLog2->GetFile() );
        break;
    }

    if( !s_MemLogSortAscending )
        SortCode = -SortCode;

    return SortCode;
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SortMemLog( int Field, BOOL Ascending )
{
    // Sort the filtered log
    s_MemLogSortField     = Field;
    s_MemLogSortAscending = Ascending;
    qsort( m_FilteredMemLog.GetData(), m_FilteredMemLog.GetSize(), sizeof(log_memory*), fnCompareMemLog );
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::FilterMemLog( void )
{
    m_FilteredMemLog.m_Array = m_MemLog.m_Array;
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SetHeapAddresses( u32 HeapStart, u32 HeapEnd )
{
    m_HeapState.SetMinAddress( HeapStart );
    m_HeapState.SetMaxAddress( HeapEnd );
}

/////////////////////////////////////////////////////////////////////////////

log_memory* CxToolDoc::GetLastEntryAtAddress( u32 Address )
{
    for( s32 i=m_MemLog.GetSize()-1 ; i>=0 ; i-- )
    {
        // Get entry and start and end addresses of its block
        log_memory* pEntry = (log_memory*)m_MemLog[i];
        u32 Start = pEntry->GetAddress() - MEMORY_HEADER_SIZE;
        u32 End   = pEntry->GetAddress() + ((pEntry->GetSize() + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1));

        // Does the address fall within this entrys range?
        if( (Address >= Start) && (Address < End) )
            return pEntry;
    }

    // Not found
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::OnCloseDocument() 
{
    int i;

    bool Removed = false;
    for( i=0 ; i<g_Documents.GetSize() ; i++ )
    {
        if( g_Documents[i] == (void*)this )
        {
            g_Documents.RemoveAt( i );
            Removed = true;
            break;
        }
    }

	// Terminate any connection thread associated with this document
    if( m_pConnection && m_pConnectionThread )
    {
        // Set the event to signal the thread to die
        SetEvent( m_pConnection->m_hExitEvent );

        // Wait for the thread to terminate
        WaitForSingleObject( m_pConnection->m_hExitedEvent, INFINITE );

        // Close the thread events
        CloseHandle( m_pConnection->m_hExitEvent );
        CloseHandle( m_pConnection->m_hExitedEvent );
    }

    // Delete the connection
    if( m_pConnection )
        delete m_pConnection;

    // Clear the log data
    for( i=0 ; i<m_Channels.GetSize() ; i++ )
    {
        delete m_Channels[i];
    }

    // Call base class
	CDocument::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CxToolDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	CFileException fe;
	CFile* pFile = NULL;
	pFile = GetFile(lpszPathName, CFile::modeCreate |
		CFile::modeReadWrite | CFile::shareExclusive, &fe);

	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}

	CArchive saveArchive(pFile, CArchive::store | CArchive::bNoFlushOnDelete, 2*1024*1024);
	saveArchive.m_pDocument = this;
	saveArchive.m_bForceFlat = FALSE;
	TRY
	{
		CWaitCursor wait;
		Serialize(saveArchive);     // save me
		saveArchive.Close();
		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		}
		END_TRY
#define DELETE_EXCEPTION(e) do { e->Delete(); } while (0)
		DELETE_EXCEPTION(e);
		return FALSE;
	}
	END_CATCH_ALL

	SetModifiedFlag(FALSE);     // back to unmodified

	return TRUE;        // success
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SetLogViewFixedFont( BOOL State )
{
    m_LogViewFixedFont = State;
    UpdateAllViews( NULL, HINT_LOG_FIXED_FONT_CHANGED );
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SetLogViewMessages( BOOL State )
{
    if( m_LogViewMessages != State )
    {
        m_LogViewMessages = State;
        FilterLog();
        UpdateAllViews( NULL, HINT_LOG_FILTER );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SetLogViewWarnings( BOOL State )
{
    if( m_LogViewWarnings != State )
    {
        m_LogViewWarnings = State;
        FilterLog();
        UpdateAllViews( NULL, HINT_LOG_FILTER );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SetLogViewErrors( BOOL State )
{
    if( m_LogViewErrors != State )
    {
        m_LogViewErrors = State;
        FilterLog();
        UpdateAllViews( NULL, HINT_LOG_FILTER );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SetLogViewRTFs( BOOL State )
{
    if( m_LogViewRTFs != State )
    {
        m_LogViewRTFs = State;
        FilterLog();
        UpdateAllViews( NULL, HINT_LOG_FILTER );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::SetLogViewMemory( BOOL State )
{
    if( m_LogViewMemory != State )
    {
        m_LogViewMemory = State;
        FilterLog();
        UpdateAllViews( NULL, HINT_LOG_FILTER );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::HideSelectedEntryChannels( void )
{
    s32         nEntries = m_FilteredLog.GetSize();
    log_entry** pEntries = m_FilteredLog.GetData();

    // Iterate over the entries and clear the check on all the channels
    for( s32 i=0 ; i<nEntries ; i++ )
    {
        if( pEntries[i]->GetFlags( log_entry::flag_selected ) )
        {
            s32 ChannelID = pEntries[i]->GetChannelID();
            m_Channels[ChannelID]->SetFlags( 0, log_channel::flag_checked );
        }
    }

    // Write the default disabled channels list to the registry
    SaveDisabledChannels();

    // Filter the log with the new channel restrictions
    FilterLog();

    // Force all views to redraw with a hint
    UpdateAllViews( NULL, HINT_LOG_FILTER );
}

/////////////////////////////////////////////////////////////////////////////

static void QuoteQuotes( CString& s )
{
    int i = s.Find( '"' );
    while( i >= 0 )
    {
        s.Insert( i+1, '"' );
        i = s.Find( '"', i+2 );
    }
}

void CxToolDoc::ExportFilteredLogToCSV( const char* pPathName )
{
    xbool       Append  = FALSE;
    log_array&  Log     = GetFilteredLog();
    CString     s;
    xstring     Output( (s32)32*1024 );

    Output += "Seq,Time,Channel,Thread,Message,Line,File\n";

    for( s32 i=0 ; i<Log.GetSize() ; i++ )
    {
        log_entry* pEntry = Log[i];
        ASSERT( pEntry );

        s.Format( _T("%d"), pEntry->GetSequence() );
        Output += s;
        Output += ",";
        s.Format( _T("%.5f"), (pEntry->GetTicks() - GetBaselineTicks()) / GetTicksPerSecond() );
        Output += s;
        Output += ",";
        Output += "\"";
        s = pEntry->GetChannel();
        QuoteQuotes( s );
        Output += s;
        Output += "\"";
        Output += ",";
        s.Format( _T("%d"), pEntry->GetThreadID() );
        Output += s;
        Output += ",";
        Output += "\"";
        s = pEntry->GetMessage();
        QuoteQuotes( s );
        Output += s;
        Output += "\"";
        Output += ",";
        s.Format( _T("%d"), pEntry->GetLine() );
        Output += s;
        Output += ",";
        s = pEntry->GetFile();
        Output += s;
        Output += "\n";

        if( Output.GetLength() > (4*1024) )
        {
            Output.SaveFile( pPathName, Append );
            Output.Clear();
            Append = TRUE;
        }

        // Every 100 entries check for windows messages
        if( (i % 100) == 0 )
        {
            // Keep windows ticking over
            MSG     msg;
            BOOL    bRet;
            while( (bRet = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) != 0 )
            {
                if (bRet == -1)
                {
                    // handle the error and possibly exit
                }
                else
                {
                    TranslateMessage( &msg );
                    DispatchMessage ( &msg );
                }
            }
        }
    }

    // Save final piece of output
    Output.SaveFile( pPathName, Append );
}

void CxToolDoc::ExportMemLogToCSV( const char* pPathName, xbool OnlyActive )
{
    xbool       Append  = FALSE;
    log_array&  Log     = GetFilteredMemLog();
    CString     s;
    xstring     Output( (s32)32*1024 );

    Output += "Seq,Time,Thread,Operation,Address,Size,Line,File,Callstack\n";

    for( s32 i=0 ; i<Log.GetSize() ; i++ )
    {
        log_memory* pEntry = (log_memory*)Log[i];
        ASSERT( pEntry );

        // Export all entries or just the active ones?
        if( !OnlyActive || (pEntry->GetFlags( log_entry::flag_memory_active )) )
        {
            s.Format( _T("%d"), pEntry->GetSequence() );
            Output += s;
            Output += ",";

            s.Format( _T("%.5f"), (pEntry->GetTicks() - GetBaselineTicks()) / GetTicksPerSecond() );
            Output += s;
            Output += ",";

            s.Format( _T("%d"), pEntry->GetThreadID() );
            Output += s;
            Output += ",";

            switch( pEntry->GetOperation() )
            {
            case xtool::LOG_MEMORY_MALLOC:
                s = _T("malloc");
                break;
            case xtool::LOG_MEMORY_REALLOC:
                s.Format( _T("realloc 0x%08X"), pEntry->GetOldAddress() );
                break;
            case xtool::LOG_MEMORY_FREE:
                s = _T("free");
                break;
            case xtool::LOG_MEMORY_MARK:
                s = _T("mark");
                break;
            default:
                ASSERT( 0 );
                break;
            }
            Output += s;
            Output += ",\"";

            s.Format( _T("%08X"), pEntry->GetAddress() );
            Output += s;
            Output += "\",";

            s.Format( _T("%d"), pEntry->GetSize() );
            Output += s;
            Output += ",";

            s.Format( _T("%d"), pEntry->GetLine() );
            Output += s;
            Output += ",\"";

            s = pEntry->GetFile();
            Output += s;
            Output += "\",\"";

            {
                s32 Index = pEntry->GetCallStackIndex();
                u32* pCallStack = GetCallStackEntry( Index );
                s.Empty();
                while( *pCallStack != 0 )
                {
                    CString t;
                    const char* pSymbol = AddressToSymbol( *pCallStack );

                    if( pSymbol )
                    {
                        t = pSymbol;
                        int Index = t.Find( "(" );
                        if( Index != -1 )
                            t = t.Left( Index );
                    }
                    else
                    {
                        t.Format( "%08x", *pCallStack );
                    }

                    if( !s.IsEmpty() )
                        s += " - ";

                    s += t;

                    pCallStack++;
                }
            }

            Output += s;
            Output += "\"\n";

            if( Output.GetLength() > (4*1024) )
            {
                Output.SaveFile( pPathName, Append );
                Output.Clear();
                Append = TRUE;
            }

            // Every 100 entries check for windows messages
            if( (i % 100) == 0 )
            {
                // Keep windows ticking over
                MSG     msg;
                BOOL    bRet;
                while( (bRet = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) != 0 )
                {
                    if (bRet == -1)
                    {
                        // handle the error and possibly exit
                    }
                    else
                    {
                        TranslateMessage( &msg );
                        DispatchMessage ( &msg );
                    }
                }
            }
        }
    }

    // Save final piece of output
    Output.SaveFile( pPathName, Append );
}

/////////////////////////////////////////////////////////////////////////////

void CxToolDoc::ImportMapFile( const char* pPathName )
{
    if( !m_MapFile.Init( pPathName ) )
    {
        ::MessageBox( 0, _T("Error loading map file."), _T("Error"), MB_ICONSTOP );
    }
}

/////////////////////////////////////////////////////////////////////////////

const char* CxToolDoc::AddressToSymbol( u32 Address )
{
    return m_MapFile.AddressToSymbol( Address );
}

/////////////////////////////////////////////////////////////////////////////

BOOL CxToolDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
    // Record this as a valid document
    g_Documents.Add( (void*)this );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CxToolDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
	// Save the document data to a file
	// lpszPathName = path name where to save document file
	// if lpszPathName is NULL then the user will be prompted (SaveAs)
	// note: lpszPathName can be different than 'm_strPathName'
	// if 'bReplace' is TRUE will change file name if successful (SaveAs)
	// if 'bReplace' is FALSE will not change path name (SaveCopyAs)
{
	CString newName = lpszPathName;
	if (newName.IsEmpty())
	{
		CDocTemplate* pTemplate = GetDocTemplate();
		ASSERT(pTemplate != NULL);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty())
		{
			newName = m_strTitle;
			// check for dubious filename
			int iBad = newName.FindOneOf(_T("#%;/\\"));
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);

			// append the default suffix if there is one
			CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
			  !strExt.IsEmpty())
			{
				ASSERT(strExt[0] == '.');
				newName += strExt;
			}
		}

		if (!AfxGetApp()->DoPromptFileName(newName,
		  bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
		  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate))
			return FALSE;       // don't even attempt to save
	}

	CWaitCursor wait;

	if (!OnSaveDocument(newName))
	{
		if (lpszPathName == NULL)
		{
			// be sure to delete the file
			TRY
			{
				CFile::Remove(newName);
			}
			CATCH_ALL(e)
			{
				TRACE0("Warning: failed to delete file after failed SaveAs.\n");
				DELETE_EXCEPTION(e);
			}
			END_CATCH_ALL
		}
		return FALSE;
	}

	// reset the title and change the document name
	if (bReplace)
		SetPathName(newName);

	return TRUE;        // success
}

/////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_RETAIL
void xtool::packet_header::ByteSwap() 
{
}

/////////////////////////////////////////////////////////////////////////////

void xtool::connect_message::ByteSwap() 
{
}
#endif