// ViewChannels.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "ViewChannels.h"
#include "VirtualListCtrl.h"
#include "xToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewChannels

IMPLEMENT_DYNCREATE(CViewChannels, CViewBase)

CViewChannels::CViewChannels()
{
    m_UpdateTimerID = 0;
}

CViewChannels::~CViewChannels()
{
}

/////////////////////////////////////////////////////////////////////////////

CListChannels* CViewChannels::GetList( void )
{
    return &m_wndList;
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CViewChannels, CViewBase)
	//{{AFX_MSG_MAP(CViewChannels)
	ON_WM_CREATE()
	ON_WM_SIZE()
    ON_NOTIFY(VLC_CHECK_CHANGED, AFX_IDW_PANE_FIRST, OnCheckChanged )
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewChannels drawing

void CViewChannels::OnDraw(CDC* pDC)
{
	CxToolDoc* pDoc = GetDocument();
}

/////////////////////////////////////////////////////////////////////////////
// CViewChannels diagnostics

#ifdef _DEBUG
void CViewChannels::AssertValid() const
{
	CViewBase::AssertValid();
}

void CViewChannels::Dump(CDumpContext& dc) const
{
	CViewBase::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewChannels message handlers

int CViewChannels::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

    ASSERT( m_pDocument );

    // Create the Log list control
    m_wndList.SetDocument( GetDocument() );
    if( !m_wndList.Create( WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_OWNERDATA, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST ) )
    {
        TRACE0( "Failed to create log window\n" );
        return -1;      // fail to create
    }

    // Create list columns
    m_wndList.InsertColumn( log_channel::field_Name  , CResString(IDS_VIEW_CHANNEL_COL_CHANNEL), LVCFMT_LEFT  , 200, log_channel::field_Name   );
    m_wndList.InsertColumn( log_channel::field_Thread, CResString(IDS_VIEW_CHANNEL_COL_THREAD ), LVCFMT_RIGHT , 100, log_channel::field_Thread );

    m_wndList.EnsureVisible( 0, FALSE );

    m_wndList.SetSortColumn( 0 );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CViewChannels::OnSize(UINT nType, int cx, int cy) 
{
	CViewBase::OnSize(nType, cx, cy);

    // Make the List control fill the client area
    if( m_wndList.GetSafeHwnd() )
    {
        CRect rc;
        GetClientRect( &rc );
//        m_wndList.SetRedraw( FALSE );
//        m_wndList.SetColumnWidth( 0, rc.Width() );
        m_wndList.MoveWindow( rc );
//        m_wndList.SetRedraw( TRUE );
//        m_wndList.RedrawWindow();
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewChannels::OnCheckChanged( NMHDR * pNotifyStruct, LRESULT * result )
{
    CxToolDoc*  pDoc            = GetDocument();
    ASSERT( pDoc );

    // Write the default disabled channels list to the registry
    pDoc->SaveDisabledChannels();

    // Filter the log with our changes
    pDoc->FilterLog();

    // Tell Document that we changed the log filter
    pDoc->UpdateAllViews( this, CxToolDoc::HINT_LOG_FILTER );
}

/////////////////////////////////////////////////////////////////////////////

void CViewChannels::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
    // If we don't have a timer then start one, this mechanism is used to limit
    // the number of updates reflected visually to 1 every 50ms
    if( m_UpdateTimerID == 0 )
    {
        m_UpdateTimerID = SetTimer( 1, 50, NULL );
        ASSERT( m_UpdateTimerID );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewChannels::OnTimer(UINT nIDEvent) 
{
    // Update timer message?
    if( nIDEvent == m_UpdateTimerID )
    {
        // Kill the timer
        VERIFY( KillTimer( m_UpdateTimerID ) );
        m_UpdateTimerID = 0;

        // Get document
        CxToolDoc* pDoc         = GetDocument();
        channel_array&  Channels    = pDoc->GetFilteredChannels();

        // Check if the channels need updating
        if( m_wndList.GetItemCount() != Channels.GetSize() )
        {
            // Determine if the last item has focus
            BOOL LastHasFocus = m_wndList.IsLastItemInFocus();

            // Disable redraw
            m_wndList.SetRedraw( FALSE );

            // Resize the list with no scrolling or unnecessary redraw
    	    m_wndList.SetItemCountEx( Channels.GetSize(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL );

            // Sort the list
            m_wndList.Sort();

            // Keep the last item visible if it was previously visible
            if( LastHasFocus && (Channels.GetSize() > 0) )
            {
                // Move focus to the last item
                int iItemFocus = m_wndList.GetItemCount()-1;
                m_wndList.SetFocusItem( iItemFocus );

                // Ensure the last item is visible
                m_wndList.EnsureVisible( iItemFocus, FALSE );
            }

            // Enable & force redraw
            m_wndList.SetRedraw( TRUE );
        }
        else
        {
            // Just redraw the list
            m_wndList.RedrawWindow();
        }
    }
	
	CViewBase::OnTimer(nIDEvent);
}
