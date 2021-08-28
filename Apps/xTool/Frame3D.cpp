// FrameL3D.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "Frame3D.h"
#include "ViewBase.h"
#include "ViewGL.h"
#include "xToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFrame3D

CFrame3D::CFrame3D()
{
    m_pViewChannels = NULL;
    m_pView3D       = NULL;
}

CFrame3D::~CFrame3D()
{
}


BEGIN_MESSAGE_MAP(CFrame3D, CFrameBase)
	//{{AFX_MSG_MAP(CFrameLog)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFrame3D message handlers

int CFrame3D::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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
    m_Splitter.CreateView( 0, 0, RUNTIME_CLASS(CViewBase), CSize(200,0), &cc );
    m_Splitter.CreateView( 0, 1, RUNTIME_CLASS(CViewGL  ), CSize(200,0), &cc );

    m_pViewChannels = (CViewBase*)m_Splitter.GetPane( 0, 0 );
    m_pView3D       = (CViewGL*)  m_Splitter.GetPane( 0, 1 );
    ASSERT( m_pViewChannels );
    ASSERT( m_pView3D       );

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

void CFrame3D::OnDestroy() 
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
