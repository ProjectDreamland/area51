// FrameMemory.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "FrameMemory.h"
#include "ViewMemoryLog.h"
#include "ViewMemoryGraph.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFrameMemory

CFrameMemory::CFrameMemory()
{
}

CFrameMemory::~CFrameMemory()
{
}


BEGIN_MESSAGE_MAP(CFrameMemory, CFrameBase)
	//{{AFX_MSG_MAP(CFrameMemory)
	ON_WM_CREATE()
	ON_COMMAND(ID_MEM_VIEW_MALLOC, OnMemViewMalloc)
	ON_COMMAND(ID_MEM_VIEW_FREE, OnMemViewFree)
	ON_COMMAND(ID_MEM_VIEW_REALLOC, OnMemViewRealloc)
	ON_UPDATE_COMMAND_UI(ID_MEM_VIEW_MALLOC, OnUpdateMemViewMalloc)
	ON_UPDATE_COMMAND_UI(ID_MEM_VIEW_FREE, OnUpdateMemViewFree)
	ON_UPDATE_COMMAND_UI(ID_MEM_VIEW_REALLOC, OnUpdateMemViewRealloc)
	ON_WM_ENABLE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFrameMemory message handlers

int CFrameMemory::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the splitter window
    if( !m_Splitter.CreateStatic( this, 2, 1, WS_CHILD|WS_VISIBLE, AFX_IDW_PANE_FIRST ) )
    {
		TRACE0( "Failed to create splitter.\n" );
		return -1;
    }

	// Create the splitter window
	if( !m_Splitter2.CreateStatic( &m_Splitter, 1, 2, WS_CHILD|WS_VISIBLE, m_Splitter.IdFromRowCol( 0, 0 ) ) )
	{
		TRACE0( "Failed to create splitter.\n" );
		return -1;
	}

    m_Splitter.SetRowInfo( 0, 500, 0 );

    // Make a context for creation of the views
    CCreateContext* pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;
    ASSERT( pContext && pContext->m_pCurrentDoc );
    CCreateContext cc;
    cc.m_pCurrentDoc     = pContext->m_pCurrentDoc;
    cc.m_pCurrentFrame   = NULL;
    cc.m_pLastView       = NULL;
    cc.m_pNewDocTemplate = NULL;
    cc.m_pNewViewClass   = NULL;

    // Create Log & Graph views
    m_Splitter2.CreateView( 0, 0, RUNTIME_CLASS(CViewBase       ), CSize(200,600), &cc );
    m_Splitter2.CreateView( 0, 1, RUNTIME_CLASS(CViewMemoryLog  ), CSize(200,600), &cc );
    m_Splitter .CreateView( 1, 0, RUNTIME_CLASS(CViewMemoryGraph), CSize(100,400), &cc );


// NOTE: This code is left to demonstrate how to create a single view in one of these windows
/*
    // Make a context for creation of the views
    CCreateContext* pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;
    ASSERT( pContext && pContext->m_pCurrentDoc );
    CCreateContext cc;
    cc.m_pCurrentDoc     = pContext->m_pCurrentDoc;
    cc.m_pCurrentFrame   = this;
    cc.m_pLastView       = NULL;
    cc.m_pNewDocTemplate = NULL;
    cc.m_pNewViewClass   = RUNTIME_CLASS( CViewMemoryLog );

    // Create view
    m_pViewLog = (CViewMemoryLog*)CreateView( &cc );
    m_pViewLog->ShowWindow( SW_SHOW );
    SetActiveView( m_pViewLog );
*/

    // Create Toolbar
    m_ToolBarID = g_NextToolBarID++;
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(0,0,0,0), m_ToolBarID ) ||
		!m_wndToolBar.LoadToolBar(IDR_PANE_MEMORY))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
    m_wndToolBar.SetWindowText( CResString(IDS_FRAME_MEMORY_TOOLBAR) );
    m_wndToolBar.EnableContextMenus( FALSE );

/*
    // Create Ruler Bar
    m_RulerBarID = g_NextToolBarID++;
    if( !m_wndRulerBar.CreateEx( this, 0, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|CBRS_BOTTOM|CBRS_SIZE_DYNAMIC, CRect(0,0,0,0), m_RulerBarID ) )
    {
		TRACE0("Failed to create ruler bar\n");
		return -1;      // fail to create
    }
    m_wndRulerBar.SetWindowText( CResString(IDS_FRAME_MEMORY_TIMEBAR) );
    m_wndRulerBar.EnableContextMenus( FALSE );
    m_wndRulerBar.SetUnits( CRuler::UNITS_SECONDS );
*/

    // Setup check buttons for displaying malloc/free/realloc
    int iMalloc = m_wndToolBar.CommandToIndex( ID_MEM_VIEW_MALLOC );
    m_wndToolBar.SetButtonStyle( iMalloc++, TBBS_CHECKBOX );
    m_wndToolBar.SetButtonStyle( iMalloc++, TBBS_CHECKBOX );
    m_wndToolBar.SetButtonStyle( iMalloc  , TBBS_CHECKBOX );
    m_wndToolBar.CheckButton( ID_MEM_VIEW_MALLOC , TRUE );
    m_wndToolBar.CheckButton( ID_MEM_VIEW_FREE   , TRUE );
    m_wndToolBar.CheckButton( ID_MEM_VIEW_REALLOC, TRUE );

    // Setup Toolbar view buttons
    int iButton = m_wndToolBar.CommandToIndex( ID_MEM_VIEW_LIST );
    if( iButton != -1 )
    {
        m_wndToolBar.SetButtonStyle( iButton  , TBBS_CHECKGROUP );
        m_wndToolBar.SetButtonStyle( iButton+1, TBBS_CHECKGROUP );
        m_wndToolBar.CheckButton( ID_MEM_VIEW_LIST, TRUE );
    }

	return 0;
}

void CFrameMemory::OnMemViewMalloc() 
{
	// TODO: Add your command handler code here
}

void CFrameMemory::OnMemViewFree() 
{
	// TODO: Add your command handler code here
}

void CFrameMemory::OnMemViewRealloc() 
{
	// TODO: Add your command handler code here
}

/*
void CFrameMemory::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
    // Paint a grey rectangle
    CRect rClient;
	GetClientRect( &rClient );
    rClient.top += 24;
    CRect rRuler = rClient;
    CRect rFill  = rClient;
    rRuler.bottom = rRuler.top + 18;
    rFill .top    = rFill .top + 18;
    
    m_Ruler.DrawRuler( &dc, rRuler, 0, 0.000001f );

    dc.FillSolidRect( &rFill, RGB(m_Shade,m_Shade,m_Shade) );
}
*/


void CFrameMemory::OnUpdateMemViewMalloc(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
}

void CFrameMemory::OnUpdateMemViewFree(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
}

void CFrameMemory::OnUpdateMemViewRealloc(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
}

void CFrameMemory::OnEnable(BOOL bEnable) 
{
	CFrameBase::OnEnable(bEnable);

    if( bEnable )
    {
        CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
        ASSERT( pMainFrame );
        pMainFrame->ReplaceMenu( IDR_MENU_MEMORY );
    }
}
