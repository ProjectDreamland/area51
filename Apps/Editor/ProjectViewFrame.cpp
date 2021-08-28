// CProjectViewFrame.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "resource.h"
#include "ProjectViewFrame.h"
#include "TextEditor.h"
#include "HelpView.h"
#include "BugBase.h"
#include "TextEditorView.h"
#include "ProjectView.h"
#include ".\projectviewframe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectView

IMPLEMENT_DYNCREATE(CProjectViewFrame, CFrameWnd)

CProjectViewFrame::CProjectViewFrame()
{
}

CProjectViewFrame::~CProjectViewFrame()
{
}


BEGIN_MESSAGE_MAP(CProjectViewFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CProjectViewFrame)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
    ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectViewFrame drawing

void CProjectViewFrame::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CProjectView diagnostics

#ifdef _DEBUG
void CProjectViewFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CProjectViewFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CProjectView message handlers

int CProjectViewFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if( !m_TabCtrl.Create(this, IDW_PROJ_VIEWTAB_BAR, _T("Properties"),
		CSize(200, 150), CBRS_TOP, CBRS_XT_DEFAULT ))
	{
		TRACE0("Failed to create property dock window\n");
		return -1;		// fail to create
	}

    CDocument*  pDocument   = ((CProjectView*)GetParent())->GetDocument();
    CFrameWnd*  pFrameWnd   = NULL;

    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CHelpView), pDocument);
    m_TabCtrl.AddControl(_T("Help"), pFrameWnd);

    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CBugBase), pDocument);
    m_TabCtrl.AddControl(_T("BugBase"), pFrameWnd);

    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CTextEditor), RUNTIME_CLASS(TextEditorView), pDocument);
    m_TabCtrl.AddControl(_T("Documentation"), pFrameWnd);

	return 0;
}

BOOL CProjectViewFrame::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
//	return CView::OnEraseBkgnd(pDC);
}

void CProjectViewFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
    CRect R;
	GetClientRect( R );

    //m_TabCtrl.SetWindowRect( R );
    m_TabCtrl.MoveWindow( R );    
}

CTextEditor& CProjectViewFrame::GetTextEditor()
{
    CTextEditor* pTextEditor = (CTextEditor*)m_TabCtrl.GetView( 2 );
    ASSERT_KINDOF(CTextEditor, pTextEditor);

    return *pTextEditor;
}

void CProjectViewFrame::OnDestroy()
{
    CFrameWnd::OnDestroy();

    // TODO: Add your message handler code here
}

void CProjectViewFrame::PostNcDestroy()
{
    // TODO: Add your specialized code here and/or call the base class

    // Stop default processing which tries to delete the window
    //CFrameWnd::PostNcDestroy();
}
