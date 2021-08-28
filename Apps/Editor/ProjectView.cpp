// CProjectView.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "resource.h"
#include "ProjectView.h"
#include "TextEditor.h"
#include "HelpView.h"
#include "BugBase.h"
#include "TextEditorView.h"
#include ".\projectview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectView

IMPLEMENT_DYNCREATE(CProjectView, CView)

CProjectView::CProjectView()
{
}

CProjectView::~CProjectView()
{
}


BEGIN_MESSAGE_MAP(CProjectView, CView)
	//{{AFX_MSG_MAP(CProjectView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
    ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectView drawing

void CProjectView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CProjectView diagnostics

#ifdef _DEBUG
void CProjectView::AssertValid() const
{
	CView::AssertValid();
}

void CProjectView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CProjectView message handlers

int CProjectView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

    m_Frame.Create( NULL, "", WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this );
/*
	if( !m_TabCtrl.Create(this, IDW_PROJ_VIEWTAB_BAR, _T("Properties"),
		CSize(200, 150), CBRS_TOP, CBRS_XT_DEFAULT ))
	{
		TRACE0("Failed to create property dock window\n");
		return -1;		// fail to create
	}

    CFrameWnd* pFrameWnd = NULL;
    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CHelpView), GetDocument());
    m_TabCtrl.AddControl(_T("Help"), pFrameWnd);

    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CBugBase), GetDocument());
    m_TabCtrl.AddControl(_T("BugBase"), pFrameWnd);

    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CTextEditor), RUNTIME_CLASS(TextEditorView), GetDocument());
    m_TabCtrl.AddControl(_T("Documentation"), pFrameWnd);
*/
	return 0;
}

BOOL CProjectView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
//	return CView::OnEraseBkgnd(pDC);
}

void CProjectView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

// TODO: Add your message handler code here
    CRect R;
	GetClientRect( R );
//    m_TabCtrl.MoveWindow( R );

    m_Frame.MoveWindow( R );
}

CTextEditor& CProjectView::GetTextEditor()
{
//    CTextEditor* pTextEditor = (CTextEditor*)m_TabCtrl.GetView( 2 );
    CTextEditor* pTextEditor = &m_Frame.GetTextEditor();
    ASSERT_KINDOF(CTextEditor, pTextEditor);

    return *pTextEditor;
}

void CProjectView::OnDestroy()
{
    CView::OnDestroy();

    // TODO: Add your message handler code here
    //m_Frame.DestroyWindow();
}
