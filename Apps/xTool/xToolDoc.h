// xToolDoc.h : interface of the CxToolDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_XTOOLDOC_H__47C86445_594C_4D1B_81FF_057CD0137AC7__INCLUDED_)
#define AFX_XTOOLDOC_H__47C86445_594C_4D1B_81FF_057CD0137AC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

#include "LogData.h"
#include "HeapState.h"
#include "Pool.h"
#include "MapFile.h"

/////////////////////////////////////////////////////////////////////////////

class log_packet;

/////////////////////////////////////////////////////////////////////////////

class CxToolDoc : public CDocument
{
    friend UINT ConnectionThread( LPVOID pParam );

public:
    enum update_hint
    {
        HINT_NONE,

        // Log View hints
        HINT_NEW_LOG_DATA,
        HINT_LOG_FILTER,
        HINT_LOG_FIXED_FONT_CHANGED,
        HINT_LOG_REDRAW,

        // Memory View hints
        HINT_NEW_MEMORY_DATA,
        HINT_SELECT_MEMBLOCK,
        HINT_SYMBOLS_LOADED,
        HINT_REDRAW_MEMORY_GRAPH,
    };

protected: // create from serialization only
	CxToolDoc();
	DECLARE_DYNCREATE(CxToolDoc)

// Attributes
protected:

    // Connection Info
    debug_connection*   m_pConnection;
    CWinThread*         m_pConnectionThread;

    CTime               m_CaptureTime;
    platform            m_Platform;
    CString             m_ApplicationName;
    CString             m_PlatformName;
    double              m_TicksPerSecond;
    double              m_BaselineTicks;
    BOOL                m_BigEndian;
    u32                 m_LogSequence;

    // Map file
    CMapFile            m_MapFile;

    // Pools of log objects
    CPool<log_message>  m_PoolLogMessage;
    CPool<log_memory>   m_PoolLogMemory;

    // Log callstack data
    CPool<u32>          m_PoolCallStack;

    // Log data
    log_array           m_Log;
    log_array           m_FilteredLog;
    int                 m_LogSortField;
    BOOL                m_LogSortAscending;

    // Memory log data
    log_array           m_MemLog;
    log_array           m_MemMarks;
    log_array           m_FilteredMemLog;
    int                 m_MemLogSortField;
    BOOL                m_MemLogSortAscending;
    CHeapState          m_HeapState;
    u32                 m_MinAddress;           // TODO: Factor these out and into HeapState?
    u32                 m_MaxAddress;           // TODO: Factor these out and into HeapState?

    // Timer data
    xarray<xtick>       m_TimerStack;

    // Log Channel data
    CMapStringToPtr     m_ChannelsDisabled;
    channel_array       m_Channels;
    channel_array       m_FilteredChannels;
    int                 m_ChannelsSortField;
    BOOL                m_ChannelsSortAscending;

    // Log View flags
    BOOL                m_LogViewFixedFont;
    BOOL                m_LogViewMessages;
    BOOL                m_LogViewWarnings;
    BOOL                m_LogViewErrors;
    BOOL                m_LogViewRTFs;
    BOOL                m_LogViewMemory;

// Operations
public:
    void                ProcessPacketLog                ( log_packet& Packet );

protected:
    UINT                RunConnection                   ( debug_connection* pConnection );
    void                ProcessPacketLogMessage         ( log_packet& Packet );
    void                ProcessPacketLogMemory          ( log_packet& Packet );
    void                ProcessPacketLogApplicationName ( log_packet& Packet );
    void                ProcessPacketLogTimerPush       ( log_packet& Packet );
    void                ProcessPacketLogTimerPop        ( log_packet& Packet );

    void                CreateMemLog                    ( void );
    void                CreateHeapState                 ( void );

    void                AddEntryToFilteredLog           ( log_entry* pEntry );

public:

    // Query connection data
    double              GetBaselineTicks                ( void ) const { return m_BaselineTicks; }
    double              GetTicksPerSecond               ( void ) const { return m_TicksPerSecond; }
    BOOL                IsBigEndian                     ( void ) const { return m_BigEndian; }

    // External messages
    void                RebuildTitle                    ( void );
    void                ProcessPacket                   ( void );

    // CallStack access
    u32*                GetCallStackEntry               ( s32 Index ) { return &m_PoolCallStack[Index]; }

    // Log access
    log_array&          GetFilteredLog                  ( void ) { return m_FilteredLog; }
    void                SortLog                         ( int Field, BOOL Ascending );
    void                FilterLog                       ( void );
    log_message*        NewLogMessage                   ( void ) { return m_PoolLogMessage.New(); }
    log_memory*         NewLogMemory                    ( void ) { return m_PoolLogMemory.New(); }
    void                ClearLog                        ( void );

    // Memory log access
    log_array&          GetFilteredMemLog               ( void ) { return m_FilteredMemLog; }
    void                SortMemLog                      ( int Field, BOOL Ascending );
    void                FilterMemLog                    ( void );
    void                SetHeapAddresses                ( u32 HeapStart, u32 HeapEnd );
    CHeapState&         GetHeapState                    ( void ) { return m_HeapState; }
    log_memory*         GetLastEntryAtAddress           ( u32 Address );
    void                CreateHeapState                 ( double Ticks );

    // Channels access
    channel_array&      GetChannels                     ( void ) { return m_Channels; }
    channel_array&      GetFilteredChannels             ( void ) { return m_FilteredChannels; }
    void                SortChannels                    ( int Field, BOOL Ascending );
    void                FilterChannels                  ( void );
    void                SaveDisabledChannels            ( void );
    void                LoadDisabledChannels            ( void );
    void                HideSelectedEntryChannels       ( void );


    // Log view flag access
    void                SetLogViewFixedFont             ( BOOL State );
    BOOL                GetLogViewFixedFont             ( void ) { return m_LogViewFixedFont; }
    void                SetLogViewMessages              ( BOOL State );
    void                SetLogViewWarnings              ( BOOL State );
    void                SetLogViewErrors                ( BOOL State );
    void                SetLogViewRTFs                  ( BOOL State );
    void                SetLogViewMemory                ( BOOL State );
    BOOL                GetLogViewMessages              ( void ) { return m_LogViewMessages; }
    BOOL                GetLogViewWarnings              ( void ) { return m_LogViewWarnings; }
    BOOL                GetLogViewErrors                ( void ) { return m_LogViewErrors; }
    BOOL                GetLogViewRTFs                  ( void ) { return m_LogViewRTFs; }
    BOOL                GetLogViewMemory                ( void ) { return m_LogViewMemory; }

    // Export functions
    void                ExportFilteredLogToCSV          ( const char* pPathName );
    void                ExportMemLogToCSV               ( const char* pPathName, xbool OnlyActive );

    // Map file functions
    void                ImportMapFile                   ( const char* pPathName );
    const char*         AddressToSymbol                 ( u32 Address );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CxToolDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);

// Implementation
public:
	virtual ~CxToolDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CxToolDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XTOOLDOC_H__47C86445_594C_4D1B_81FF_057CD0137AC7__INCLUDED_)
