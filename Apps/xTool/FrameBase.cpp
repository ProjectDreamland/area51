// FrameBase.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "FrameBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

int g_NextToolBarID = IDW_USER;

/////////////////////////////////////////////////////////////////////////////
// CFrameBase

CFrameBase::CFrameBase()
{
    m_pDoc  = NULL;
    m_Shade = 255;
}

CFrameBase::~CFrameBase()
{
}

void CFrameBase::PostNcDestroy( void )
{
}

IMPLEMENT_DYNCREATE(CFrameBase, CXTFrameWnd)

BEGIN_MESSAGE_MAP(CFrameBase, CXTFrameWnd)
	//{{AFX_MSG_MAP(CFrameBase)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFrameBase message handlers

void CFrameBase::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
    // Paint a grey rectangle
    CRect r;
	GetClientRect( &r );
    dc.FillSolidRect( &r, RGB(m_Shade,m_Shade,m_Shade) );
}

/////////////////////////////////////////////////////////////////////////////

BOOL CFrameBase::OnEraseBkgnd(CDC* pDC) 
{
    return 1;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CFrameBase::PreCreateWindow(CREATESTRUCT& cs) 
{
    cs.style |= WS_CLIPCHILDREN;

	return CXTFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////

int CFrameBase::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CXTFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    // Get the doc pointer out of the CreateContext
    CCreateContext* pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;
    ASSERT( pContext && pContext->m_pCurrentDoc );
    m_pDoc = (CxToolDoc*)pContext->m_pCurrentDoc;

	// Set style of window border
	DWORD dwAdd    = xtAfxData.bXPMode ? 0 : WS_EX_CLIENTEDGE;
	DWORD dwRemove = xtAfxData.bXPMode ? WS_EX_CLIENTEDGE : 0;
	ModifyStyleEx( dwRemove, dwAdd );
	
    InstallCoolMenus( IDR_MAINFRAME );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
