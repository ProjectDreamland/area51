// FrameLog.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "FrameLog.h"
#include "ViewLog.h"
#include "ViewChannels.h"
#include "DlgFindLog.h"
#include "xToolDoc.h"
#include "ViewChannels.h"
#include "ViewLog.h"
#include "ListLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFrameLog

CFrameLog::CFrameLog()
{
    m_pViewChannels = NULL;
    m_pViewLog      = NULL;
}

CFrameLog::~CFrameLog()
{
}


BEGIN_MESSAGE_MAP(CFrameLog, CFrameBase)
	//{{AFX_MSG_MAP(CFrameLog)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_LOG_VIEW_FIXED, OnLogViewFixed)
	ON_UPDATE_COMMAND_UI(ID_LOG_VIEW_FIXED, OnUpdateLogViewFixed)
	ON_COMMAND(ID_LOG_VIEW_MESSAGE, OnLogViewMessage)
	ON_COMMAND(ID_LOG_VIEW_WARNING, OnLogViewWarning)
	ON_COMMAND(ID_LOG_VIEW_ERROR, OnLogViewError)
	ON_COMMAND(ID_LOG_VIEW_RTF, OnLogViewRtf)
	ON_UPDATE_COMMAND_UI(ID_LOG_VIEW_MESSAGE, OnUpdateLogViewMessage)
	ON_UPDATE_COMMAND_UI(ID_LOG_VIEW_WARNING, OnUpdateLogViewWarning)
	ON_UPDATE_COMMAND_UI(ID_LOG_VIEW_ERROR, OnUpdateLogViewError)
	ON_UPDATE_COMMAND_UI(ID_LOG_VIEW_RTF, OnUpdateLogViewRtf)
	ON_COMMAND(ID_LOG_VIEW_MEMORY, OnLogViewMemory)
	ON_UPDATE_COMMAND_UI(ID_LOG_VIEW_MEMORY, OnUpdateLogViewMemory)
	ON_COMMAND(ID_LOG_VIEW_SEARCH, OnLogViewSearch)
	ON_UPDATE_COMMAND_UI(ID_LOG_VIEW_SEARCH, OnUpdateLogViewSearch)
	ON_COMMAND(ID_POPUP_LOG_FIND, OnPopupLogFind)
	ON_COMMAND(ID_POPUP_LOG_FINDNEXT, OnPopupLogFindnext)
	ON_COMMAND(ID_POPUP_LOG_FINDPREVIOUS, OnPopupLogFindprevious)
	ON_COMMAND(ID_POPUP_LOG_GOTONEXTCHANNELHIGHLIGHT, OnPopupLogGotonextchannelhighlight)
	ON_COMMAND(ID_POPUP_LOG_GOTOPREVCHANNELHIGHLIGHT, OnPopupLogGotoprevchannelhighlight)
	ON_COMMAND(ID_POPUP_LOG_GOTONEXTERRORWARNING, OnPopupLogGotonexterrorwarning)
	ON_COMMAND(ID_POPUP_LOG_GOTOPREVIOUSERRORWARNING, OnPopupLogGotopreviouserrorwarning)
	ON_COMMAND(ID_POPUP_LOG_TOGGLEMARK, OnPopupLogTogglemark)
	ON_COMMAND(ID_POPUP_LOG_GOTONEXTMARK, OnPopupLogGotonextmark)
	ON_COMMAND(ID_POPUP_LOG_GOTOPREVMARK, OnPopupLogGotoprevmark)
    ON_COMMAND(ID_POPUP_LOG_HIDECHANNEL, OnPopupLogHidechannel)
    ON_COMMAND(ID_LOG_CLEAR, OnLogClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFrameLog message handlers

int CFrameLog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the splitter window
	if( !m_Splitter.CreateStatic( this, 1, 2, WS_CHILD|WS_VISIBLE, AFX_IDW_PANE_FIRST ) )
	{        
		TRACE0( "Failed to create splitter.\n" );
		return -1;
	}

    // Make a context for creation of the views
    CCreateContext* pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;
    ASSERT( pContext && pContext->m_pCurrentDoc );
    CCreateContext cc;
    cc.m_pCurrentDoc     = pContext->m_pCurrentDoc;
    cc.m_pCurrentFrame   = NULL;
    cc.m_pLastView       = NULL;
    cc.m_pNewDocTemplate = NULL;
    cc.m_pNewViewClass   = NULL;

    // Create Channel & Log views
    m_Splitter.CreateView( 0, 0, RUNTIME_CLASS(CViewChannels), CSize(200,0), &cc );
    m_Splitter.CreateView( 0, 1, RUNTIME_CLASS(CViewLog     ), CSize(200,0), &cc );

    m_pViewChannels = (CViewChannels*)m_Splitter.GetPane( 0, 0 );
    m_pViewLog      = (CViewLog*)     m_Splitter.GetPane( 0, 1 );
    ASSERT( m_pViewChannels );
    ASSERT( m_pViewLog      );

    // Create Toolbar
    m_ToolBarID = g_NextToolBarID++;
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(0,0,0,0), m_ToolBarID ) ||
		!m_wndToolBar.LoadToolBar(IDR_PANE_LOG))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    m_wndToolBar.SetWindowText( CResString(IDS_FRAME_LOG_TOOLBAR) );
    m_wndToolBar.EnableContextMenus( FALSE );

    m_bToolBarCold.LoadBitmap( IDB_LOG_TOOLBAR_COLD );
    m_ilToolBarCold.Create   ( 16, 16, ILC_COLOR24|ILC_MASK, 5, 1 );
    m_ilToolBarCold.Add      ( &m_bToolBarCold, RGB(255,0,255) );
    m_wndToolBar.SetImageList( m_ilToolBarCold, FALSE );

    m_bToolBarHot.LoadBitmap ( IDB_LOG_TOOLBAR_HOT );
    m_ilToolBarHot.Create    ( 16, 16, ILC_COLOR24|ILC_MASK, 5, 1 );
    m_ilToolBarHot.Add       ( &m_bToolBarHot, RGB(255,0,255) );
    m_wndToolBar.SetImageList( m_ilToolBarHot, TRUE );

//  m_wndToolBar.EnableCustomization();
//	m_wndToolBar.AutoSizeToolbar();

    // Configure the toolbar buttons
    m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_LOG_VIEW_FIXED  ), TBBS_CHECKBOX );
    m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_LOG_VIEW_MESSAGE), TBBS_CHECKBOX );
    m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_LOG_VIEW_WARNING), TBBS_CHECKBOX );
    m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_LOG_VIEW_ERROR  ), TBBS_CHECKBOX );
    m_wndToolBar.SetButtonStyle( m_wndToolBar.CommandToIndex( ID_LOG_VIEW_RTF    ), TBBS_CHECKBOX );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnDestroy() 
{
	CFrameBase::OnDestroy();
/*
    // Destroy & delete the splitter panes
    if( ::IsWindow( m_Splitter ) )
    {
        CWnd* pPane1 = m_Splitter.GetPane(0,0);
        CWnd* pPane2 = m_Splitter.GetPane(0,1);
        pPane1->DestroyWindow();
        pPane2->DestroyWindow();
        delete pPane1;
        delete pPane2;
    }
*/
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnLogViewFixed() 
{
    ASSERT( m_pDoc );
    m_pDoc->SetLogViewFixedFont( !m_pDoc->GetLogViewFixedFont() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnUpdateLogViewFixed(CCmdUI* pCmdUI) 
{
    ASSERT( m_pDoc );
    pCmdUI->SetCheck( m_pDoc->GetLogViewFixedFont() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnLogViewMessage() 
{
    ASSERT( m_pDoc );
    m_pDoc->SetLogViewMessages( !m_pDoc->GetLogViewMessages() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnLogViewWarning() 
{
    ASSERT( m_pDoc );
    m_pDoc->SetLogViewWarnings( !m_pDoc->GetLogViewWarnings() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnLogViewError() 
{
    ASSERT( m_pDoc );
    m_pDoc->SetLogViewErrors( !m_pDoc->GetLogViewErrors() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnLogViewRtf() 
{
    ASSERT( m_pDoc );
    m_pDoc->SetLogViewRTFs( !m_pDoc->GetLogViewRTFs() );
}


/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnLogViewMemory() 
{
    ASSERT( m_pDoc );
    m_pDoc->SetLogViewMemory( !m_pDoc->GetLogViewMemory() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnLogViewSearch() 
{
    // Get active view
    CView* pActiveView = GetActiveView();

    // Load recent list
    CRecentList RecentList;
    RecentList.LoadRegistry( _T("Recent Find List") );

    // Search in the channel view
    if( pActiveView && pActiveView->IsKindOf( RUNTIME_CLASS( CViewChannels ) ) )
    {
        CViewChannels* pView = (CViewChannels*)pActiveView;
        CListChannels* pList = pView->GetList();

        // Execute dialog
        CDlgFindLog Dialog( _T("Find in channels"), pList );
        Dialog.SetRecentList( RecentList );
        Dialog.DoModal();
        pList->SetFocus();
    }

    // Search in the log view by default
    else
    {
        CViewLog*   pView = (CViewLog*)m_Splitter.GetPane(0, 1);
        ASSERT( pView->IsKindOf( RUNTIME_CLASS( CViewLog ) ) );
        CListLog*   pList = pView->GetList();

        // Execute dialog
        CDlgFindLog Dialog( _T("Find in log"), pList );
        Dialog.SetRecentList( RecentList );
        Dialog.DoModal();
        pList->SetFocus();
    }

    // Save recent list
    RecentList.SaveRegistry( _T("Recent Find List") );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnUpdateLogViewMessage(CCmdUI* pCmdUI) 
{
    ASSERT( m_pDoc );
    pCmdUI->SetCheck( m_pDoc->GetLogViewMessages() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnUpdateLogViewWarning(CCmdUI* pCmdUI) 
{
    ASSERT( m_pDoc );
    pCmdUI->SetCheck( m_pDoc->GetLogViewWarnings() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnUpdateLogViewError(CCmdUI* pCmdUI) 
{
    ASSERT( m_pDoc );
    pCmdUI->SetCheck( m_pDoc->GetLogViewErrors() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnUpdateLogViewRtf(CCmdUI* pCmdUI) 
{
    ASSERT( m_pDoc );
    pCmdUI->SetCheck( m_pDoc->GetLogViewRTFs() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnUpdateLogViewMemory(CCmdUI* pCmdUI) 
{
    ASSERT( m_pDoc );
    pCmdUI->SetCheck( m_pDoc->GetLogViewMemory() );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnUpdateLogViewSearch(CCmdUI* pCmdUI) 
{
    ASSERT( m_pDoc );
}

/////////////////////////////////////////////////////////////////////////////

/*
void CFrameLog::OnLogExportCsv() 
{
    CxToolDoc* pDoc = GetDocument();
	ASSERT( pDoc );

    CString     Path    = pDoc->GetTitle();

    // Strip extension from Path
    char Drive[_MAX_DRIVE];
    char Dir  [_MAX_DIR  ];
    char File [_MAX_FNAME];
    _splitpath( Path, Drive, Dir, File, NULL );
    Path = Drive;
    Path += Dir;
    Path += File;

    CFileDialog FileDialog( FALSE, _T(".csv"), Path, OFN_ENABLESIZING|OFN_OVERWRITEPROMPT, _T("Excel CSV File (*.csv)|*.csv||"), this );
    FileDialog.m_ofn.lpstrTitle = _T("Export log to CSV");
    if( FileDialog.DoModal() == IDOK )
    {
        CString Path = FileDialog.GetPathName();

        pDoc->ExportFilteredLogToCSV( Path );
    }
}
*/

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogFind() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->OnFind();
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogFindnext() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->OnFindNext();
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogFindprevious() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->OnFindPrevious();
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogGotonextchannelhighlight() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->GotoNextChannel( 1 );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogGotoprevchannelhighlight() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->GotoNextChannel( -1 );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogGotonexterrorwarning() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->GotoNextError( 1 );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogGotopreviouserrorwarning() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->GotoNextError( -1 );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogTogglemark() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->ToggleMarkSelected();
    m_pViewLog->GetList()->RedrawWindow();
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogGotonextmark() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->GotoNextMark( 1 );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogGotoprevmark() 
{
    ASSERT( m_pViewLog );
	m_pViewLog->GetList()->GotoNextMark( -1 );
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnPopupLogHidechannel() 
{
    ASSERT( m_pDoc );
    m_pDoc->HideSelectedEntryChannels();
}

/////////////////////////////////////////////////////////////////////////////

void CFrameLog::OnLogClear()
{
    ASSERT( m_pDoc );
    m_pDoc->ClearLog();
}

/////////////////////////////////////////////////////////////////////////////
