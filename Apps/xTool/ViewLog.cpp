// ViewLog.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "ViewLog.h"
#include "xToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewLog

IMPLEMENT_DYNCREATE(CViewLog, CViewBase)

CViewLog::CViewLog()
{
    m_UpdateTimerID = 0;
    m_NewDataFlag   = FALSE;
}

CViewLog::~CViewLog()
{
}

/////////////////////////////////////////////////////////////////////////////

CListLog* CViewLog::GetList( void )
{
    return &m_wndList;
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CViewLog, CViewBase)
	//{{AFX_MSG_MAP(CViewLog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
    ON_NOTIFY(VLC_CONTEXT_MENU, AFX_IDW_PANE_FIRST, OnListContextMenu )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewLog drawing

void CViewLog::OnDraw(CDC* pDC)
{
	CxToolDoc* pDoc = GetDocument();
}

/////////////////////////////////////////////////////////////////////////////
// CViewLog diagnostics

#ifdef _DEBUG
void CViewLog::AssertValid() const
{
	CViewBase::AssertValid();
}

void CViewLog::Dump(CDumpContext& dc) const
{
	CViewBase::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewLog message handlers

int CViewLog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

    ASSERT( m_pDocument );

    // Create the Log list control
    if( !m_wndList.Create( WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_OWNERDATA, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST ) )
//    if( !m_wndList.Create( WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_OWNERDATA|LVS_OWNERDRAWFIXED, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST ) )
    {
        TRACE0( "Failed to create log window\n" );
        return -1;      // fail to create
    }
    m_wndList.SetDocument( (CxToolDoc*)m_pDocument );

    // Get a copy of the default font
    LOGFONT LogFont;
    m_wndList.GetFont()->GetLogFont( &LogFont );
//    LogFont.lfQuality = NONANTIALIASED_QUALITY;
    m_DefaultFont.CreateFontIndirect( &LogFont );

    // Create the new fixed width font for the list
//    m_Font.CreatePointFont(100, "Terminal" );
//    m_Font.CreatePointFont(100, "Courier New" );
    m_Font.CreatePointFont(100, "Lucida Console" );

//    m_wndList.SetFont( &m_DefaultFont );

    // Create list columns
    m_wndList.InsertColumn( 0, CResString(IDS_VIEW_LOG_COL_SEQ    ), LVCFMT_RIGHT ,  64, 0 );
    m_wndList.InsertColumn( 1, CResString(IDS_VIEW_LOG_COL_TIME   ), LVCFMT_RIGHT ,  72, 1 );
    m_wndList.InsertColumn( 2, CResString(IDS_VIEW_LOG_COL_CHANNEL), LVCFMT_LEFT  , 128, 2 );
    m_wndList.InsertColumn( 3, CResString(IDS_VIEW_LOG_COL_THREAD ), LVCFMT_CENTER,  32, 3 );
    m_wndList.InsertColumn( 4, CResString(IDS_VIEW_LOG_COL_MESSAGE), LVCFMT_LEFT  , 400, 4 );
    m_wndList.InsertColumn( 5, CResString(IDS_VIEW_LOG_COL_LINE   ), LVCFMT_RIGHT ,  50, 5 );
    m_wndList.InsertColumn( 6, CResString(IDS_VIEW_LOG_COL_FILE   ), LVCFMT_LEFT  , 400, 6 );

    m_wndList.EnsureVisible( 0, FALSE );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CViewLog::OnSize(UINT nType, int cx, int cy) 
{
	CViewBase::OnSize(nType, cx, cy);

    // Make the List control fill the client area
    if( m_wndList.GetSafeHwnd() )
    {
        CRect rc;
        GetClientRect( &rc );
        m_wndList.MoveWindow( rc );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewLog::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
    CxToolDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    if( (lHint == 0) ||
        (lHint == CxToolDoc::HINT_NEW_LOG_DATA) ||
        (lHint == CxToolDoc::HINT_NEW_MEMORY_DATA ) || // TODO: Check for filtered memory data here
        (lHint == CxToolDoc::HINT_LOG_FILTER) ||
        (lHint == CxToolDoc::HINT_LOG_REDRAW) )
    {
        // If we don't have a timer then start one, this mechanism is used to limit
        // the number of updates reflected visually to 1 every 50ms
        if( m_UpdateTimerID == 0 )
        {
            m_UpdateTimerID = SetTimer( 1, 50, NULL );
            ASSERT( m_UpdateTimerID );
        }

        // If this is not a filter channels change then set the new data flag
        if( (lHint == CxToolDoc::HINT_NEW_LOG_DATA) || (lHint == 0) )
        {
            m_NewDataFlag = TRUE;
        }
    }

    // If the filter was changed then set the number of items and rebuild the selection set
    if( lHint == CxToolDoc::HINT_LOG_FILTER )
    {
        // Get document
        CxToolDoc*  pDoc = GetDocument();
        log_array&  Log  = pDoc->GetFilteredLog();

        // Resize the list with no scrolling or unnecessary redraw
        m_wndList.SetItemCountEx( Log.GetSize(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL );

        // Rebuild the selection set
        m_wndList.BuildSelectionSet();
    }

    if( lHint == CxToolDoc::HINT_LOG_FIXED_FONT_CHANGED )
    {
        if( pDoc->GetLogViewFixedFont() )
        {
            m_wndList.SetFont( &m_Font );
            CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
            pHeader->SetFont( &m_DefaultFont );
        }
        else
        {
            m_wndList.SetFont( &m_DefaultFont );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewLog::OnTimer(UINT nIDEvent)
{
    // Update timer message?
    if( nIDEvent == m_UpdateTimerID )
    {
        // Kill the timer
        VERIFY( KillTimer( m_UpdateTimerID ) );
        m_UpdateTimerID = 0;

        // Get document
        CxToolDoc*  pDoc = GetDocument();
        log_array&  Log  = pDoc->GetFilteredLog();

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
            if( LastHasFocus && (m_wndList.GetItemCount() > 0) )
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
        else
        {
            // No new data, just force a redraw
            m_wndList.RedrawWindow();

            // TODO: Clear m_NewData here?
        }
    }

	CViewBase::OnTimer(nIDEvent);
}

/////////////////////////////////////////////////////////////////////////////

void CViewLog::OnListContextMenu( NMHDR* pNotifyStruct, LRESULT* result )
{
    // Get the document
    CxToolDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    // We need cool menus
    CXTCoolMenu	CoolMenu;
    CoolMenu.HookWindow( this );
    
    // Load menu
	CXTMenu menu;
	VERIFY(menu.LoadMenu(IDR_POPUP_LOG_VIEW));

    // Get popup from loaded menu
	CXTMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

    // Get the frame that contains this view
    CFrameWnd* pFrame = GetParentFrame();

    // Get the cursor position in screen space
    CPoint pt;
    GetCursorPos( &pt );

    // Run the popup
	pPopup->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, pFrame );

/*
    // Write the default disabled channels list to the registry
    pDoc->SaveDisabledChannels();

    // Filter the log with our changes
    pDoc->FilterLog();

    // Tell Document that we changed the log filter
    pDoc->UpdateAllViews( this, CxToolDoc::HINT_LOG_FILTER );
*/
}

/////////////////////////////////////////////////////////////////////////////
