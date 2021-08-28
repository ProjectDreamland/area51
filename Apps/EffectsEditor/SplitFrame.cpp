// SplitFrame.cpp : implementation file
//

#include "stdafx.h"
#include "PartEd.h"
#include "ParticleView3D.h"
#include "FXEditorView3D.h"
#include "SplitFrame.h"

#include <AFXPRIV.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSplitFrame

IMPLEMENT_DYNCREATE(CSplitFrame, CFrameWnd)

CSplitFrame::CSplitFrame()
{
    m_Focus = FALSE;
}

CSplitFrame::~CSplitFrame()
{
}


BEGIN_MESSAGE_MAP(CSplitFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CSplitFrame)
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


#ifdef _DEBUG
void CSplitFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CSplitFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CSplitFrame message handlers

BOOL CSplitFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
/******************************************************************************************/
/********************* NEW VIEWPORT CAMERA TEST *******************************************
#ifdef jversluis
	CFXEditorView3D*    pView = NULL;

	// Create a context.
	CCreateContext context;
	pContext = &context;

	// Assign custom view.
	pContext->m_pNewViewClass = RUNTIME_CLASS(CFXEditorView3D);

	// Create the view.
	pView = (CFXEditorView3D*)CreateView( pContext, AFX_IDW_PANE_FIRST );
	if( pView == NULL )
		return FALSE;

	// Notify the view.
	pView->SendMessage( WM_INITIALUPDATE );
	SetActiveView( pView, FALSE );
#else
/******************************************************************************************/
	CParticleView3D*    pView = NULL;

	// Create a context.
	CCreateContext context;
	pContext = &context;

	// Assign custom view.
	pContext->m_pNewViewClass = RUNTIME_CLASS(CParticleView3D);

	// Create the view.
	pView = (CParticleView3D*)CreateView( pContext, AFX_IDW_PANE_FIRST );
	if( pView == NULL )
		return FALSE;

	// Notify the view.
	pView->SendMessage( WM_INITIALUPDATE );
	SetActiveView( pView, FALSE );
//#endif

	return TRUE;
}	

int CSplitFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Create the KeyBar
	if ( !m_ViewportToolbar.Create(this, CBRS_ALIGN_TOP, IDW_VIEWPORT_TOOLBAR) )
	{
		TRACE0("Failed to create Viewport Toobar\n");
		return -1;      // fail to create
	}

/*********************************************************
********** Taking this out to put in my own toolbar ******
**********************************************************
	if (!m_wndToolBar.CreateEx( this, TBSTYLE_FLAT,
                                WS_CHILD |
                                WS_VISIBLE |
                                CBRS_TOP |
                                //CBRS_GRIPPER |
                                CBRS_TOOLTIPS |
                                CBRS_FLYBY |
                                CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar( IDR_TOOLBAR3D ))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    m_wndToolBar.EnableContextMenus( FALSE );
**********************************************************/

//	m_wndToolBar.EnableDocking( CBRS_ALIGN_TOP ); //CBRS_ALIGN_ANY );
//	EnableDocking( CBRS_ALIGN_TOP );
//	DockControlBar( &m_wndToolBar );
	return 0;
}

BOOL CSplitFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
    cs.style = WS_CHILD|WS_VISIBLE;

	return CFrameWnd::PreCreateWindow(cs);
}

void CSplitFrame::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CFrameWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

//    CRect r =lpncsp->rgrc[0];
//    r.DeflateRect( 2, 2 );
//    lpncsp->rgrc[0] = r;
}

void CSplitFrame::OnNcPaint() 
{
	// Do not call CFrameWnd::OnNcPaint() for painting messages

/*
    CWindowDC dc( this );

    CRect r;
    GetWindowRect( &r );
    r.OffsetRect( -r.left, -r.top );

    CRect rClient;
    GetClientRect( &rClient );
    rClient.OffsetRect( 2, 2 );

    dc.ExcludeClipRect( &rClient );

    if( !m_Focus )
        dc.FillSolidRect( &r, RGB(255,255,0) );
    else
        dc.FillSolidRect( &r, RGB(0,0,0) );
*/

    CFrameWnd::OnNcPaint();
}
