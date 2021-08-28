// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "xTool.h"

#include "MainFrm.h"
#include "xToolDoc.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CXTMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CXTMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_WINDOW_CLOSE_ALL, OnWindowCloseAll)
	//}}AFX_MSG_MAP
    ON_MESSAGE(AM_REBUILD_DOC_TITLE, OnRebuildDocTitle)
    ON_MESSAGE(AM_LOG_PACKET       , OnLogPacket      )
    ON_MESSAGE(AM_UPDATE_DOC_VIEWS , OnUpdateDocViews )
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    // Enable/Disable XP GUI Mode
    xtAfxData.bXPMode = TRUE;

    // Enable/Disable Menu Shadows
    xtAfxData.bMenuShadows = TRUE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CXTMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndMenuBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndMenuBar.LoadMenuBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

    // Dock bars
	m_wndMenuBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
	m_wndToolBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
	EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
	DockControlBar(&m_wndMenuBar);
	DockControlBar(&m_wndToolBar);

	// Remove this line if you don't want cool menus.
	InstallCoolMenus(IDR_MAINFRAME);

    m_Tabs.Install( this, TCS_HOTTRACK );
    m_Tabs.SetBorderGap( 0 );

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CXTMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

	// Helps to reduce screen flicker.
	cs.lpszClass = AfxRegisterWndClass(0, NULL, NULL,
		AfxGetApp()->LoadIcon(IDR_MAINFRAME));

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CXTMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CXTMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnClose() 
{
	// Save control bar postion.
	SaveBarState(_T("Control Bar State"));

	// Save frame window size and position.
	m_wndPosition.SaveWindowPos(this);

	CXTMDIFrameWnd::OnClose();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::ShowWindowEx(int nCmdShow)
{
	ASSERT_VALID(this);

	// Restore control bar postion.
	LoadBarState(_T("Control Bar State"));

	// Restore frame window size and position.
	m_wndPosition.LoadWindowPos(this);
	nCmdShow = m_wndPosition.showCmd;

	return ShowWindow(nCmdShow);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::IsValidDocument( CxToolDoc* pDoc )
{
    for( int i=0 ; i<g_Documents.GetSize() ; i++ )
    {
        if( g_Documents[i] == (void*)pDoc )
        {
            return TRUE;
        }
    }

    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnRebuildDocTitle( WPARAM wParam, LPARAM lParam )
{
    if( wParam )
    {
        CxToolDoc* pDoc = (CxToolDoc*)wParam;

        // Check that the document pointer is still valid
        if( IsValidDocument( pDoc ) )
            pDoc->RebuildTitle();
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnLogPacket( WPARAM wParam, LPARAM lParam )
{
    if( wParam )
    {
        log_packet* pPacket = (log_packet*)wParam;
        CxToolDoc* pDoc = pPacket->GetDocument();

        // Check that the document pointer is still valid
        if( IsValidDocument( pDoc ) )
            pDoc->ProcessPacketLog( *pPacket );

        delete pPacket;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnUpdateDocViews( WPARAM wParam, LPARAM lParam )
{
    if( wParam )
    {
        CxToolDoc* pDoc = (CxToolDoc*)wParam;

        // Check that the document pointer is still valid
        if( IsValidDocument( pDoc ) )
            pDoc->UpdateAllViews( NULL, lParam );
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::ReplaceMenu( UINT n_IDResource )
{
    return;

    CXTMenuBar* pMenuBar = GetMenuBar();
    ASSERT( pMenuBar );
//    pMenuBar->LoadMenuBar( n_IDResource );

    CMenu* pMenu = pMenuBar->GetMenu();

    // Look for "File" menu.
    int pos = FindMenuItem( pMenu, "&File" );
    if( pos == -1 )
        return;

    CMenu* pSubMenu = pMenu->GetSubMenu( pos );
    pos = FindMenuItem( pSubMenu, "&Open...\tCtrl+O" );
    if( pos > -1 )
        pSubMenu->InsertMenu( pos + 1, MF_BYPOSITION, ID_LOG_EXPORT_CSV, "New Menu Item" );
}

/////////////////////////////////////////////////////////////////////////////

// FindMenuItem() will find a menu item string from the specified
// popup menu and returns its position (0-based) in the specified 
// popup menu. It returns -1 if no such menu item string is found.
int CMainFrame::FindMenuItem(CMenu* Menu, LPCTSTR MenuString)
{
   ASSERT(Menu);
   ASSERT(::IsMenu(Menu->GetSafeHmenu()));

   int count = Menu->GetMenuItemCount();
   for (int i = 0; i < count; i++)
   {
      CString str;
      if (Menu->GetMenuString(i, str, MF_BYPOSITION) &&
         (strcmp(str, MenuString) == 0))
         return i;
   }

   return -1;
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowCloseAll() 
{
/*
    CObArray    Docs;
    CObArray    Frames;

    CWinApp*     pApp    = AfxGetApp();
    CDocManager* pDocMgr = pApp->m_pDocManager;

    // For all document templates
    for( POSITION TmplPos = pDocMgr->GetFirstDocTemplatePosition() ; TmplPos ; )
    {
        CDocTemplate *pTmpl = pDocMgr->GetNextDocTemplate(TmplPos);
        ASSERT_VALID(pTmpl);    

        // For All open documents of this document template type.
        for( POSITION Pos = pTmpl->GetFirstDocPosition() ; Pos ; )
        {
            CDocument *pDoc = pTmpl->GetNextDoc( Pos );
            Docs.Add( pDoc );

            // get me a view and from it the MDI child frame
            POSITION    ViewPos = pDoc->GetFirstViewPosition();
            CView*      pView   =  pDoc->GetNextView(ViewPos);
            CFrameWnd*  pFrame  = pView->GetParentFrame();
            ASSERT_VALID( pFrame );
            Frames.Add( pFrame );
        }
    }

    // Close all the documents
    for( s32 i=0 ; i<Docs.GetSize() ; i++ )
    {
        ((CDocument*)Docs[i])->OnCloseDocument();
    }
    for( s32 i=0 ; i<Frames.GetSize() ; i++ )
    {
        ((CChildFrame*)Frames[i])->CloseWindow();
    }
*/

    // Get the app pointer
    CWinApp* pApp = AfxGetApp();

	// attempt to save all documents
//	if( pApp->SaveAllModified() )
    {
    	// Close all documents first
	    pApp->CloseAllDocuments(FALSE);
    }
}

/////////////////////////////////////////////////////////////////////////////
