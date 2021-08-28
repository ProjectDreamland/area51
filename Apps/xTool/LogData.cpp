// LogData.cpp
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

log_entry::log_entry( void )
{
    m_Type      = type_entry;
    m_Sequence  = 0;
    m_pDoc      = NULL;
    m_ChannelID = -1;
    m_Flags     = 0;
}

log_entry::~log_entry( void )
{
}

xtool::log_severity log_entry::GetSeverity( void ) const
{
    return xtool::LOG_SEVERITY_MESSAGE;
}

const char* log_entry::GetChannel( void ) const
{
    return "";
}

int log_entry::GetChannelID( void ) const
{
    return m_ChannelID;
}

const char* log_entry::GetMessage( void ) const
{
    return "";
}

const char* log_entry::GetFile( void ) const
{
    return "";
}

int log_entry::GetLine( void ) const
{
    return 0;
}

void log_entry::Serialize( CArchive& ar )
{
    if( ar.IsStoring() )
    {
        ar << m_Type;
        ar << m_Sequence;
        ar << m_Ticks;
        ar << m_ThreadID;
        ar << m_ChannelID;
        ar << m_Flags;
    }
    else
    {
        ar >> m_Type;
        ar >> m_Sequence;
        ar >> m_Ticks;
        ar >> m_ThreadID;
        ar >> m_ChannelID;
        ar >> m_Flags;
    }
}

/////////////////////////////////////////////////////////////////////////////

log_message::log_message( void )
{
    m_Type      = type_message;
}

log_message::~log_message( void )
{
}

void log_message::Init( CxToolDoc*          pDoc,
                        u32                 Sequence,
                        double              Ticks,
                        u32                 ThreadID,
                        s32                 ChannelID,
                        xtool::log_severity Severity,
                        const char*         pMessage,
                        const char*         pFile,
                        s32                 Line )
{
    ASSERT( pDoc );

    m_pDoc          = pDoc;
    m_Sequence      = Sequence;
    m_Ticks         = Ticks;
    m_ThreadID      = ThreadID;
    m_ChannelID     = ChannelID;
    m_Severity      = Severity;
    m_Message       = pMessage;
    m_File          = pFile;
    m_Line          = Line;
}

xtool::log_severity log_message::GetSeverity( void ) const
{
    return (xtool::log_severity)m_Severity;
}

const char* log_message::GetChannel( void ) const
{
    return m_pDoc->GetChannels()[m_ChannelID]->GetName();
}

const char* log_message::GetMessage( void ) const
{
    return m_Message;
}

const char* log_message::GetFile( void ) const
{
    return m_File;
}

int log_message::GetLine( void ) const
{
    return m_Line;
}

void log_message::Serialize( CArchive& ar )
{
    log_entry::Serialize( ar );

    if( ar.IsStoring() )
    {
        ar << m_Severity;
        ar << m_Message;
        ar << m_File;
        ar << m_Line;
    }
    else
    {
        m_pDoc = (CxToolDoc*)ar.m_pDocument;
        ar >> m_Severity;
        ar >> m_Message;
        ar >> m_File;
        ar >> m_Line;
    }
}

/////////////////////////////////////////////////////////////////////////////

log_memory::log_memory( void )
{
    m_Type      = type_memory;
}

log_memory::~log_memory( void )
{
}

void log_memory::Init( CxToolDoc*   pDoc,
                       u32          Sequence,
                       double       Ticks,
                       u32          ThreadID,
                       s32          ChannelID,
                       s32          Operation,
                       u32          Address,
                       u32          OldAddress,
                       u32          Size,
                       const char*  pFile,
                       s32          Line,
                       s32          Mark,
                       s32          CallStackIndex )
{
    ASSERT( pDoc );

    m_pDoc              = pDoc;
    m_Sequence          = Sequence;
    m_Ticks             = Ticks;
    m_ThreadID          = ThreadID;
    m_ChannelID         = ChannelID;
    m_Operation         = Operation;
    m_Address           = Address;
    m_OldAddress        = OldAddress;
    m_Size              = Size;
    m_File              = pFile;
    m_Line              = Line;
    m_Mark              = Mark;
    m_CallStackIndex    = CallStackIndex;
    m_CurrentBytes      = 0;
}

xtool::log_severity log_memory::GetSeverity( void ) const
{
    return xtool::LOG_SEVERITY_MESSAGE;
}

const char* log_memory::GetChannel( void ) const
{
    return m_pDoc->GetChannels()[m_ChannelID]->GetName();
}

const char* log_memory::GetMessage( void ) const
{
#define NUM_STRINGS 2
    static CString  Strings[NUM_STRINGS];
    static int      Index = 0;

    CString&        String = Strings[Index];
    Index = (Index+1)%NUM_STRINGS;

    switch( m_Operation )
    {
    case xtool::LOG_MEMORY_MALLOC:
        String.Format( "malloc( %d ); = 0x%08X, 0x%08X", m_Size, m_Address, m_Size );
        break;
    case xtool::LOG_MEMORY_REALLOC:
        String.Format( "realloc( 0x%08X, %d ); = 0x%08X, 0x%08X", m_OldAddress, m_Size, m_Address, m_Size );
        break;
    case xtool::LOG_MEMORY_FREE:
        String.Format( "free( 0x%08X );", m_Address );
        break;
    default:
        ASSERT( 0 );
    }

    return String;
}

const char* log_memory::GetFile( void ) const
{
    return m_File;
}

int log_memory::GetLine( void ) const
{
    return m_Line;
}

void log_memory::Serialize( CArchive& ar )
{
    log_entry::Serialize( ar );

    if( ar.IsStoring() )
    {
        ar << m_Operation;
        ar << m_Address;
        ar << m_OldAddress;
        ar << m_Size;
        ar << m_CurrentBytes;
        ar << m_File;
        ar << m_Line;
        ar << m_Mark;
        ar << m_CallStackIndex;
    }
    else
    {
        m_pDoc = (CxToolDoc*)ar.m_pDocument;
        ar >> m_Operation;
        ar >> m_Address;
        ar >> m_OldAddress;
        ar >> m_Size;
        ar >> m_CurrentBytes;
        ar >> m_File;
        ar >> m_Line;
        ar >> m_Mark;
        ar >> m_CallStackIndex;
    }
}

/////////////////////////////////////////////////////////////////////////////

log_channel::log_channel( void )
{
    m_pDoc      = NULL;
    m_Flags     = flag_checked;
}

log_channel::~log_channel( void )
{
}

void log_channel::Init( CxToolDoc* pDoc, const char* pName, u32 ThreadID )
{
    ASSERT( pDoc );

    m_pDoc      = pDoc;
    m_Name      = pName;
    m_ThreadID  = ThreadID;
}

const char* log_channel::GetName( void ) const
{
    return m_Name;
}

u32 log_channel::GetThreadID( void ) const
{
    return m_ThreadID;
}

void log_channel::Serialize( CArchive& ar )
{
    if( ar.IsStoring() )
    {
        ar << m_Name;
        ar << m_ThreadID;
        ar << m_Flags;
    }
    else
    {
        m_pDoc = (CxToolDoc*)ar.m_pDocument;
        ar >> m_Name;
        ar >> m_ThreadID;
        ar >> m_Flags;
        m_Flags = 4; // TODO: Remove this
    }
}

/////////////////////////////////////////////////////////////////////////////

channel_array::channel_array( void )
{
}

channel_array::~channel_array( void )
{
}

int channel_array::GetSize( void )
{
//    CSingleLock Lock( &m_Mutex );

//    Lock.Lock();
//    ASSERT( Lock.IsLocked() );

    int s = m_Array.GetCount();

    return s;
}

int channel_array::Add( log_channel* pChannel )
{
    int i = m_Array.GetCount();
    m_Array.Append( pChannel );
    return i;
}

log_channel** channel_array::GetData( void )
{
    log_channel** pData = (log_channel**)m_Array.GetPtr();
    return pData;
}

int channel_array::Find( log_channel* pChannel )
{
    for( int i=0 ; i<m_Array.GetCount() ; i++ )
    {
        if( (log_channel*)m_Array[i] == pChannel )
        {
            return i;
        }
    }
    
    return -1;
}

log_channel* channel_array::operator[]( int Index )
{
    log_channel* pChannel = (log_channel*)m_Array[Index];

    return pChannel;
}

void channel_array::GetSelectionSet( CSelectionSet& SelectionSet )
{
    SelectionSet.Clear();

    log_channel** pData = (log_channel**)m_Array.GetPtr();
    int Count = m_Array.GetCount();
    int Start = Count;
    for( int i=0 ; i<Count ; i++ )
    {
        if( pData[i]->GetFlags() & log_channel::flag_selected )
        {
            if( Start == Count )
                Start = i;
        }
        else
        {
            if( Start != Count )
            {
                SelectionSet.Select( Start, i-1 );
                Start = Count;
            }
        }
    }
    if( Start != Count )
    {
        SelectionSet.Select( Start, Count-1 );
    }
}

void channel_array::Serialize( CArchive& ar, CxToolDoc* pDoc )
{
	if( ar.IsStoring() )
	{
        int Count = m_Array.GetCount();
        ar << Count;
        for( int i=0 ; i<Count ; i++ )
        {
            m_Array[i]->Serialize( ar );
        }
    }
	else
	{
        int Count;
        ar >> Count;
        m_Array.SetCount( 0 );
        m_Array.SetCapacity( Count );
        for( int i=0 ; i<Count ; i++ )
        {
            log_channel* pChannel = new log_channel;
            ASSERT( pChannel );
            pChannel->Serialize( ar );
            m_Array.Append( pChannel );
        }
	}
}

/////////////////////////////////////////////////////////////////////////////

log_array::log_array( void )
{
}

log_array::~log_array( void )
{
}

void log_array::Clear( void )
{
    m_Array.Clear();
}

int log_array::GetSize( void )
{
    int s = m_Array.GetCount();
    return s;
}

int log_array::Add( log_entry* pEntry )
{
    int i = m_Array.GetCount();
    m_Array.Append( pEntry );
    return i;
}

log_entry** log_array::GetData( void )
{
    log_entry** pData = (log_entry**)m_Array.GetPtr();
    return pData;
}

int log_array::Find( log_entry* pLog )
{
    for( int i=0 ; i<m_Array.GetCount() ; i++ )
    {
        if( (log_entry*)m_Array[i] == pLog )
        {
            return i;
        }
    }

    return -1;
}

log_entry* log_array::operator[]( int Index )
{
    log_entry* pEntry = (log_entry*)m_Array[Index];
    return pEntry;
}

void log_array::GetSelectionSet( CSelectionSet& SelectionSet )
{
    SelectionSet.Clear();

    log_entry** pData = (log_entry**)m_Array.GetPtr();
    int Count = m_Array.GetCount();
    int Start = Count;
    for( int i=0 ; i<Count ; i++ )
    {
        if( pData[i]->GetFlags() & log_entry::flag_selected )
        {
            if( Start == Count )
                Start = i;
        }
        else
        {
            if( Start != Count )
            {
                SelectionSet.Select( Start, i-1 );
                Start = Count;
            }
        }
    }
    if( Start != Count )
    {
        SelectionSet.Select( Start, Count-1 );
    }
}

void log_array::Serialize( CArchive& ar, CxToolDoc* pDoc )
{
	if( ar.IsStoring() )
	{
        int Count = m_Array.GetCount();
        ar << Count;
        for( int i=0 ; i<Count ; i++ )
        {
            log_entry* pEntry = m_Array[i];

            // Write the type
            int Type = pEntry->GetType();
            ar << Type;

            // Serialize the entry
            pEntry->Serialize( ar );
        }
    }
	else
	{
        int Count;
        ar >> Count;
        m_Array.SetCount( 0 );
        m_Array.SetCapacity( Count );
        for( int i=0 ; i<Count ; i++ )
        {
            int         Type;
            log_entry*  pEntry;

            // Read the type and create the entry based on that
            ar >> Type;
            switch( Type )
            {
            case log_entry::type_entry:
                ASSERT( 0 );
                pEntry = new log_entry;
                break;
            case log_entry::type_message:
                pEntry = pDoc->NewLogMessage();
                break;
            case log_entry::type_memory:
                pEntry = pDoc->NewLogMemory();
                break;
            default:
                ASSERTS( 0, "Invalid log entry type" );
            }

            // Make sure we now have an entry
            ASSERT( pEntry );

            // Serialize and add to array
            pEntry->Serialize( ar );
            m_Array.Append( pEntry );
        }
	}
}

/////////////////////////////////////////////////////////////////////////////
