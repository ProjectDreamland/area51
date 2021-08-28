// CBaseFrame.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "Editor.h"
#include "BaseFrame.h"
#include "BaseDocument.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBaseFrame

IMPLEMENT_DYNCREATE(CBaseFrame, CXTMDIChildWnd)

CBaseFrame::CBaseFrame()
{
    m_pBaseDoc = NULL;
}

CBaseFrame::~CBaseFrame()
{
}


BEGIN_MESSAGE_MAP(CBaseFrame, CXTMDIChildWnd)
	//{{AFX_MSG_MAP(CBaseFrame)
	ON_WM_MDIACTIVATE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseFrame message handlers
BOOL CBaseFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	if( !CXTMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.style |= WS_CLIPCHILDREN;
	
	cs.style = WS_CHILD | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
		| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	return TRUE;
}

void CBaseFrame::ActivateFrame(int nCmdShow)
{
    nCmdShow = SW_SHOWMAXIMIZED;
	CXTMDIChildWnd::ActivateFrame(nCmdShow);
    if( m_pBaseDoc == NULL )
    {
        m_pBaseDoc = (CBaseDocument*)GetActiveDocument();
    }
}

int CBaseFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CXTMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    /*
    // Create the ToolBar
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	
    // Create the Status Bar
	if( !m_wndStatusBar.Create(this) )
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar     .EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
	EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
	DockControlBar      ( &m_wndToolBar );
*/
	return 0;
}


void CBaseFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CXTMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	
    if( GetBaseDocument() )
    {
        GetBaseDocument()->SetDocumentActive( bActivate );
    }
}