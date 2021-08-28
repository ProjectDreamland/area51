// ViewMemoryLog.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "ViewMemoryLog.h"
#include "xToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryLog

IMPLEMENT_DYNCREATE(CViewMemoryLog, CViewBase)

CViewMemoryLog::CViewMemoryLog()
{
    m_UpdateTimerID = 0;
    m_NewDataFlag   = FALSE;
}

CViewMemoryLog::~CViewMemoryLog()
{
}


BEGIN_MESSAGE_MAP(CViewMemoryLog, CViewBase)
	//{{AFX_MSG_MAP(CViewMemoryLog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryLog drawing

void CViewMemoryLog::OnDraw(CDC* pDC)
{
	CxToolDoc* pDoc = GetDocument();
}

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryLog diagnostics

#ifdef _DEBUG
void CViewMemoryLog::AssertValid() const
{
	CViewBase::AssertValid();
}

void CViewMemoryLog::Dump(CDumpContext& dc) const
{
	CViewBase::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryLog message handlers

int CViewMemoryLog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    ASSERT( m_pDocument );

    // Create the Log list control
    if( !m_wndList.Create( WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_OWNERDATA, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST ) )
    {
        TRACE0( "Failed to create log window\n" );
        return -1;      // fail to create
    }
    m_wndList.SetDocument( (CxToolDoc*)m_pDocument );

    // Create list columns
    m_wndList.InsertColumn( 0, CResString(IDS_VIEW_MEMORYLOG_COL_SEQ        ), LVCFMT_RIGHT,  64, 0 );
    m_wndList.InsertColumn( 1, CResString(IDS_VIEW_MEMORYLOG_COL_TIME       ), LVCFMT_RIGHT,  96, 1 );
    m_wndList.InsertColumn( 2, CResString(IDS_VIEW_MEMORYLOG_COL_THREAD     ), LVCFMT_RIGHT,  64, 2 );
    m_wndList.InsertColumn( 3, CResString(IDS_VIEW_MEMORYLOG_COL_TYPE       ), LVCFMT_RIGHT,  96, 3 );
    m_wndList.InsertColumn( 4, CResString(IDS_VIEW_MEMORYLOG_COL_ADDRESS    ), LVCFMT_RIGHT,  96, 4 );
    m_wndList.InsertColumn( 5, CResString(IDS_VIEW_MEMORYLOG_COL_SIZE       ), LVCFMT_RIGHT,  96, 5 );
    m_wndList.InsertColumn( 6, CResString(IDS_VIEW_MEMORYLOG_COL_CURRENT    ), LVCFMT_RIGHT,  96, 6 );
    m_wndList.InsertColumn( 7, CResString(IDS_VIEW_MEMORYLOG_COL_LINE       ), LVCFMT_RIGHT,  64, 7 );
    m_wndList.InsertColumn( 8, CResString(IDS_VIEW_MEMORYLOG_COL_FILE       ), LVCFMT_LEFT , 512, 8 );
    m_wndList.InsertColumn( 9, CResString(IDS_VIEW_MEMORYLOG_COL_CALLSTACK  ), LVCFMT_LEFT , 512, 9 );
	
    m_wndList.EnsureVisible( 0, FALSE );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryLog::OnSize(UINT nType, int cx, int cy) 
{
	CViewBase::OnSize(nType, cx, cy);

    // Make the Log control fill the client area
    if( m_wndList.GetSafeHwnd() )
    {
        CRect rc;
        GetClientRect( &rc );
        m_wndList.MoveWindow( rc );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryLog::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
    if( lHint == CxToolDoc::HINT_SELECT_MEMBLOCK )
    {
        SelectMemBlock( (log_memory*)pHint );
    }

    if( lHint == CxToolDoc::HINT_SYMBOLS_LOADED )
    {
        m_wndList.RedrawWindow();
    }

    if( (lHint == 0 ) ||
        (lHint == CxToolDoc::HINT_NEW_MEMORY_DATA) )
    {
        // If we don't have a timer then start one, this mechanism is used to limit
        // the number of updates reflected visually to 1 every 50ms
        if( m_UpdateTimerID == 0 )
        {
            m_UpdateTimerID = SetTimer( 1, 50, NULL );
            ASSERT( m_UpdateTimerID );
        }

        // Cause log to update and track last entry
        m_NewDataFlag = TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryLog::OnTimer(UINT nIDEvent) 
{
    // Update timer message?
    if( nIDEvent == m_UpdateTimerID )
    {
        // Kill the timer
        VERIFY( KillTimer( m_UpdateTimerID ) );
        m_UpdateTimerID = 0;

        // Get document
        CxToolDoc*  pDoc = GetDocument();
        log_array&      Log  = pDoc->GetFilteredMemLog();

        // Check if the log needs updating
        if( m_wndList.GetItemCount() != Log.GetSize() )
        {
            // Determine if the last item has focus
            BOOL LastHasFocus = m_wndList.IsLastItemInFocus();

            // Disable redraw
            m_wndList.SetRedraw( FALSE );

            // Resize the list with no scrolling or unnecessary redraw
            m_wndList.SetItemCountEx( Log.GetSize(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL );

            // Keep the last item visible if it was previously visible
            if( m_NewDataFlag && LastHasFocus && (m_wndList.GetItemCount() > 0) )
            {
                // Move focus to the last item
                int iItemFocus = m_wndList.GetItemCount()-1;
                m_wndList.SetFocusItem( iItemFocus );

                // Ensure the last item is visible
                m_wndList.EnsureVisible( iItemFocus, FALSE );
            }

            // Enable & force redraw
            m_wndList.SetRedraw( TRUE );
            m_wndList.Invalidate();

            m_NewDataFlag = FALSE;
        }
    }

	CViewBase::OnTimer(nIDEvent);
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryLog::SelectMemBlock( log_memory* pEntry )
{
    CxToolDoc*  pDoc        = GetDocument();
    ASSERT( pDoc );

    if( pEntry )
    {
        log_array& Log = pDoc->GetFilteredMemLog();
        int Index = Log.Find( pEntry );
        if( Index != -1 )
        {
            // If this is not currently selected
            if( m_wndList.GetFocusItem() != Index )
            {
                // Clear current selection
                m_wndList.SetFocusItem( Index );
                m_wndList.EnsureCentered( Index );
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
