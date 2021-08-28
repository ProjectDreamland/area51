// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "PartEd.h"

#include "MainFrm.h"

#include "PartEdDoc.h"
#include "Properties.h"

#include "ParticleView3D.h"
#include "FXEditorView3D.h"
#include "SplitFrame.h"
#include "resource.h"
#include "MouseMode.hpp"

#include "x_context.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

CMainFrame*     g_pMainFrame    = NULL;
CProperties*    g_pProperties   = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CXTFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CXTFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
    ON_MESSAGE( WM_USER_MSG_KEYBAR_TIMECHANGE, OnTimeChange )
    ON_MESSAGE( WM_USER_MSG_KEYBAR_KEYS_CHANGED, OnKeysChange )
    ON_MESSAGE( WM_USER_MSG_KEYBAR_ANIMMODECHANGE, OnAnimModeChange )
    ON_MESSAGE( WM_USER_MSG_PROPERTIES_PROPERTYCHANGED, OnProperties_Property_Changed )
	ON_COMMAND(ID_MAIN_TOOLBAR_UNDO, OnMainToolbarUndo)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_UNDO, OnUpdateMainToolbarUndo)
	ON_COMMAND(ID_MAIN_TOOLBAR_REDO, OnMainToolbarRedo)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_REDO, OnUpdateMainToolbarRedo)
	ON_COMMAND(ID_MAIN_TOOLBAR_SELECT, OnMainToolbarSelect)
	ON_COMMAND(ID_MAIN_TOOLBAR_MOVE, OnMainToolbarMove)
	ON_COMMAND(ID_MAIN_TOOLBAR_ROTATE, OnMainToolbarRotate)
	ON_COMMAND(ID_MAIN_TOOLBAR_SCALE, OnMainToolbarScale)
	ON_COMMAND(ID_MAIN_TOOLBAR_AXIS_X, OnMainToolbarAxisX)
	ON_COMMAND(ID_MAIN_TOOLBAR_AXIS_Y, OnMainToolbarAxisY)
	ON_COMMAND(ID_MAIN_TOOLBAR_AXIS_Z, OnMainToolbarAxisZ)
	ON_COMMAND(ID_MAIN_TOOLBAR_ALIGN, OnMainToolbarAlign)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_SELECT, OnUpdateMainToolbarSelect)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_MOVE, OnUpdateMainToolbarMove)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_ROTATE, OnUpdateMainToolbarRotate)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_SCALE, OnUpdateMainToolbarScale)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_AXIS_X, OnUpdateMainToolbarAxisX)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_AXIS_Y, OnUpdateMainToolbarAxisY)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_AXIS_Z, OnUpdateMainToolbarAxisZ)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLBAR_ALIGN, OnUpdateMainToolbarAlign)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_SELECT_NONE, OnEditSelectNone)
	ON_COMMAND(ID_EDIT_SELECT_INVERT, OnEditSelectInvert)
	ON_COMMAND(ID_VIEW_VIEWPORT_MAXIMIZE, OnViewViewportMaximize)
	ON_COMMAND(ID_TIME_GOTO_FIRST_FRAME, OnTimeGotoFirstFrame)
	ON_COMMAND(ID_TIME_GOTO_LAST_FRAME, OnTimeGotoLastFrame)
	ON_COMMAND(ID_TIME_GOTO_PREVIOUS_FRAME, OnTimeGotoPreviousFrame)
	ON_COMMAND(ID_TIME_GOTO_NEXT_FRAME, OnTimeGotoNextFrame)
	ON_COMMAND(ID_TIME_PLAY, OnTimePlay)
	ON_COMMAND(ID_TIME_ZOOM_TIME_EXTENTS, OnTimeZoomTimeExtents)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIEWPORT_MAXIMIZE, OnUpdateViewViewportMaximize)
	//}}AFX_MSG_MAP
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
    RECT Window;
    ::GetWindowRect( ::GetDesktopWindow(), &Window );
    d3deng_SetWindowHandle( GetSafeHwnd() );
    d3deng_SetParentHandle( GetSafeHwnd() );
    d3deng_SetResolution( Window.right - Window.left, Window.bottom - Window.top );

    // Init pointers for KeyBar keyframe info
    m_pKeySets          = NULL;
    m_pKeyFilters       = NULL;
    m_ManipulateMode    = MANIPULATE_MOVE;

    g_pMainFrame        = this;
    g_pProperties       = &m_wndProperties;

    __MemDebug.Go();
}

CMainFrame::~CMainFrame()
{
    // destroy texture manager
    delete fx_core::g_pTextureMgr;

    eng_Kill();
#ifdef cgalley
//    x_ContextSaveProfile( "c:\\profile.txt" );
#endif
	x_Kill();

    // Cleanup pointers for KeyBar keyframe info
    if( m_pKeySets )
    {
        delete[] m_pKeySets;
        m_pKeySets = NULL;
    }

    if( m_pKeyFilters )
    {
        delete[] m_pKeyFilters;
        m_pKeyFilters = NULL;
    }
}

bool    CMainFrame::InitKeyBar( void )
{
	// Setup KeyFilters
    int         nKeyFilters         = 5;
    m_pKeyFilters                   = new KeyFilter[ nKeyFilters ];

    m_pKeyFilters[0].m_Name         = "Position";
    m_pKeyFilters[0].m_Color        = RGB( 196, 0, 0);
    m_pKeyFilters[0].m_IsVisible    = true;

    m_pKeyFilters[1].m_Name         = "Rotation";
    m_pKeyFilters[1].m_Color        = RGB( 0, 196, 0);
    m_pKeyFilters[1].m_IsVisible    = true;

    m_pKeyFilters[2].m_Name         = "Scale";
    m_pKeyFilters[2].m_Color        = RGB( 0, 0, 196);
    m_pKeyFilters[2].m_IsVisible    = true;

    m_pKeyFilters[3].m_Name         = "Color";
    m_pKeyFilters[3].m_Color        = RGB( 196, 196, 0);
    m_pKeyFilters[3].m_IsVisible    = true;

    m_pKeyFilters[4].m_Name         = "Alpha";
    m_pKeyFilters[4].m_Color        = RGB( 128, 128, 128 );
    m_pKeyFilters[4].m_IsVisible    = true;

	// Set Keybar stuff
    m_KeyBar.SetTimeRange   ( 0, 60 );
    m_KeyBar.SetTime        ( 0 );
    m_KeyBar.SetKeySets     ( 0, NULL );
    m_KeyBar.SetKeyFilters  ( nKeyFilters, m_pKeyFilters );

    return true;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    lpCreateStruct->style |= WS_MAXIMIZE;

    if (CXTFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    // Enable/Disable XP GUI Mode
    xtAfxData.bXPMode = TRUE;

    // Enable/Disable Menu Shadows
    xtAfxData.bMenuShadows = TRUE;
	
    // Create the Menu Bar
	if (!m_wndMenuBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| /*CBRS_GRIPPER |*/ CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndMenuBar.LoadMenuBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}
    
	//  Create the toolbar
/*
	if( !m_wndToolBar.CreateEx( this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY )
        || !m_wndToolBar.LoadToolBar(IDR_MAINFRAME) )
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    int iButton = m_wndToolBar.CommandToIndex( ID_VIEW_4WAY );
    if( iButton != -1 )
    {
        m_wndToolBar.SetButtonStyle( iButton, TBBS_CHECKBOX );
        m_wndToolBar.CheckButton( ID_VIEW_4WAY, TRUE );
    }
*/
	if( !m_wndToolBar.CreateEx( this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | /*CBRS_GRIPPER |*/ CBRS_TOOLTIPS | CBRS_FLYBY )
        || !m_wndToolBar.LoadToolBar(IDR_MAIN_TOOLBAR) )
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    // Setup Toolbar Undo/Redo
//    m_wndToolBar.InitDropDownButton( ID_MAIN_TOOLBAR_UNDO, TRUE );
//    m_wndToolBar.InitDropDownButton( ID_MAIN_TOOLBAR_REDO, TRUE );

    // Setup Toolbar mouse mode
    int iButton = m_wndToolBar.CommandToIndex( ID_MAIN_TOOLBAR_SELECT );
    if( iButton != -1 )
    {
        m_wndToolBar.SetButtonStyle( iButton  , TBBS_CHECKGROUP );
        m_wndToolBar.SetButtonStyle( iButton+1, TBBS_CHECKGROUP );
        m_wndToolBar.SetButtonStyle( iButton+2, TBBS_CHECKGROUP );
        m_wndToolBar.SetButtonStyle( iButton+3, TBBS_CHECKGROUP );

        m_wndToolBar.CheckButton( ID_MAIN_TOOLBAR_MOVE, TRUE );
        m_ManipulateMode    = MANIPULATE_MOVE;
    }

    // Setup Toolbar axis enable
    iButton = m_wndToolBar.CommandToIndex( ID_MAIN_TOOLBAR_AXIS_X );
    if( iButton != -1 )
    {
        m_wndToolBar.SetButtonStyle( iButton  , TBBS_CHECKBOX );
        m_wndToolBar.SetButtonStyle( iButton+1, TBBS_CHECKBOX );
        m_wndToolBar.SetButtonStyle( iButton+2, TBBS_CHECKBOX );

        m_wndToolBar.CheckButton( ID_MAIN_TOOLBAR_AXIS_X, TRUE );
        m_wndToolBar.CheckButton( ID_MAIN_TOOLBAR_AXIS_Y, TRUE );
        m_wndToolBar.CheckButton( ID_MAIN_TOOLBAR_AXIS_Z, TRUE );
    }

    // Create the Property Editor
	if( !m_wndProperties.Create( this, IDW_PROPERTIES, _T("Properties"), CSize(256,256), CBRS_RIGHT ) )
	{
		TRACE0( "Failed to create properties dock window\n" );
		return -1;		// fail to create
	}

	//  Dock the toolbars
//	m_wndMenuBar   .EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
//	m_wndToolBar   .EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
	m_wndProperties.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
//    DockControlBar( &m_wndMenuBar    );
//    DockControlBar( &m_wndToolBar    );
    DockControlBar( &m_wndProperties );

	// Cool Menus
	InstallCoolMenus(IDR_MAINFRAME);

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
    int     Width, Height;
    RECT    Rect;

    m_pDoc = (CPartEdDoc*)pContext->m_pCurrentDoc;

    GetWindowRect( &Rect );

    Width = Rect.right;
    Height = Rect.bottom;

    // do it here to give the window time to be generated
    d3deng_SetWindowHandle( this->GetSafeHwnd() );
    eng_Init();

    // g_pTextureMgr = new texture_mgr;

/***********************************************************************************************************/
/***********************************************************************************************************/
/*** TEMPORARY: Keybar should be in OnCreate()...only here til Property Editor goes into the SplitterWnd ***/
/***********************************************************************************************************/
/***********************************************************************************************************/

	// Create the KeyBar
	if ( !m_KeyBar.Create(this, CBRS_ALIGN_BOTTOM, IDW_KEYBAR) )
	{
		TRACE0("Failed to create KeyBar\n");
		return -1;      // fail to create
	}

	// Initialize the KeyBar
    if( !InitKeyBar() )
    {
		TRACE0("Failed to create KeyBar\n");
		return -1;      // KeyBar initialization failed
    }

/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/
/***********************************************************************************************************/

    // split the remaining view
    m_Splitter.CreateStatic( this, 2, 2 );
    s32 ViewWidth = (Width - 192) / 2;
    s32 ViewHeight = (Height - 64) / 2;

    m_Splitter.CreateView( 0, 0, RUNTIME_CLASS(CSplitFrame), CSize(ViewWidth,ViewHeight), pContext );
    m_Splitter.CreateView( 0, 1, RUNTIME_CLASS(CSplitFrame), CSize(ViewWidth,ViewHeight), pContext );
    m_Splitter.CreateView( 1, 0, RUNTIME_CLASS(CSplitFrame), CSize(ViewWidth,ViewHeight), pContext );
    m_Splitter.CreateView( 1, 1, RUNTIME_CLASS(CSplitFrame), CSize(ViewWidth,ViewHeight), pContext );

/******************************************************************************************/
/********************* NEW VIEWPORT CAMERA TEST *******************************************/
#if 1 //def jversluis
    {
        CWnd* pFrame = m_Splitter.GetPane( 0, 0 );
        CFXEditorView3D* pView = (CFXEditorView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->AddView( pView );
    }
    {
        CWnd* pFrame = m_Splitter.GetPane( 0, 1 );
        CFXEditorView3D* pView = (CFXEditorView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->AddView( pView );
    }
    {
        CWnd* pFrame = m_Splitter.GetPane( 1, 0 );
        CFXEditorView3D* pView = (CFXEditorView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->AddView( pView );
    }
    {
        CWnd* pFrame = m_Splitter.GetPane( 1, 1 );
        CFXEditorView3D* pView = (CFXEditorView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->AddView( pView );
    }
#else
/******************************************************************************************/
    {
        CWnd* pFrame = m_Splitter.GetPane( 0, 0 );
        CParticleView3D* pView = (CParticleView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->AddView( pView );
 
        // Set Camera Position - Top
        pView->SetCamera_Perspective();
        pView->SetCameraPos( 500, 500, 500 );
    }
    {
        CWnd* pFrame = m_Splitter.GetPane( 0, 1 );
        CParticleView3D* pView = (CParticleView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->AddView( pView );

        // Set Camera Position - Front
        pView->SetCamera_Perspective();
        pView->SetCameraPos( 500, 500, 500 );
    }
    {
        CWnd* pFrame = m_Splitter.GetPane( 1, 0 );
        CParticleView3D* pView = (CParticleView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->AddView( pView );

        // Set Camera Position - Left
        pView->SetCamera_Perspective();
        pView->SetCameraPos( 500, 500, 500 );
    }
    {
        CWnd* pFrame = m_Splitter.GetPane( 1, 1 );
        CParticleView3D* pView = (CParticleView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->AddView( pView );

        // Set Camera Position - Perspective
        pView->SetCamera_Perspective();
        //pView->SetCameraPos( 0, 100, -500 );
        pView->SetCameraPos( 500, 500, 500 );
    }
#endif

    m_Splitter.MaximizeViewport();

    return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CXTFrameWnd::PreCreateWindow(cs) )
		return FALSE;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CXTFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CXTFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

//---------------------------------------------------------------------------

void CMainFrame::OnDestroy() 
{
	CXTFrameWnd::OnDestroy();
}

//---------------------------------------------------------------------------

void CMainFrame::OnClose() 
{
    {
        CWnd* pFrame = m_Splitter.GetPane( 0, 0 );
        CParticleView3D* pView = (CParticleView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->RemoveView( pView );
    }
    {
        CWnd* pFrame = m_Splitter.GetPane( 0, 1 );
        CParticleView3D* pView = (CParticleView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->RemoveView( pView );
    }
    {
        CWnd* pFrame = m_Splitter.GetPane( 1, 0 );
        CParticleView3D* pView = (CParticleView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->RemoveView( pView );
    }
    {
        CWnd* pFrame = m_Splitter.GetPane( 1, 1 );
        CParticleView3D* pView = (CParticleView3D*)pFrame->GetDlgItem( AFX_IDW_PANE_FIRST );
        m_pDoc->RemoveView( pView );
    }
	
	CXTFrameWnd::OnClose();
}

//---------------------------------------------------------------------------

afx_msg LRESULT CMainFrame::OnTimeChange(WPARAM, LPARAM)
{
    m_pDoc->UpdateAllViews( NULL );
    if( !m_KeyBar.IsPlayOn() )
    {
        m_pDoc->PopulatePropertyControl();
    }

    return 0;
}

//---------------------------------------------------------------------------

afx_msg LRESULT CMainFrame::OnKeysChange( WPARAM UserData, LPARAM NewData )
{
    KeySet* pKeySet = (KeySet*)NewData;
    fx_core::ctrl_linear* pCont = (fx_core::ctrl_linear*)UserData;

    if ( pKeySet->m_nKeys == pCont->GetKeyCount() )
    {
        for ( s32 i = 0; i < pKeySet->m_nKeys; i++ )
        {
            fx_core::key* pKey = pCont->GetKeyByIndex(i);
            pKey->SetKey( pKeySet->m_pKeys[i].m_Time, (f32*)pKeySet->m_pKeys[i].m_pData );
        }
    }
    else
    {
        s32 i;
        s32 Count = pCont->GetKeyCount();

        // delete all existing keys
        for ( i = 0; i < Count; i++ )
        {
            pCont->DeleteKeyByIndex(0);
        }

        ASSERT( pCont->GetKeyCount() == 0 );
    
        // replace all keys
        for ( i = 0; i < pKeySet->m_nKeys; i++ )
        {
            fx_core::key* pKey = new fx_core::key( pCont->GetNumFloats() );
            pKey->SetKey( pKeySet->m_pKeys[i].m_Time, (f32*)pKeySet->m_pKeys[i].m_pData );
            pCont->SetValue( pKeySet->m_pKeys[i].m_Time, pKey );
        }
    }    

    // m_pDoc->UpdateAllViews(NULL);

    return 0;
}

//---------------------------------------------------------------------------

afx_msg LRESULT CMainFrame::OnAnimModeChange( WPARAM ControlID, LPARAM IsOn )
{
    m_pDoc->SetAnimMode( (xbool)IsOn );
    return 0;
}

//---------------------------------------------------------------------------

afx_msg LRESULT CMainFrame::OnProperties_Property_Changed( WPARAM ControlID, LPARAM lParam )
{
    POSITION            Pos     = m_pDoc->GetFirstViewPosition();
    CParticleView3D*    pView   = (CParticleView3D*)m_pDoc->GetNextView( Pos );

    pView->UpdateKeyBar();

    return 0;    
}

//---------------------------------------------------------------------------

void CMainFrame::PopulatePropertyControl( void ) 
{
    m_pDoc->PopulatePropertyControl();
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarUndo() 
{
}

void CMainFrame::OnUpdateMainToolbarUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( FALSE );
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarRedo() 
{
}

void CMainFrame::OnUpdateMainToolbarRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( FALSE );
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarSelect() 
{
    m_ManipulateMode = MANIPULATE_SELECT;
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarMove() 
{
    m_ManipulateMode = MANIPULATE_MOVE;
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarRotate() 
{
    m_ManipulateMode = MANIPULATE_ROTATE;
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarScale() 
{
    m_ManipulateMode = MANIPULATE_SCALE;
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarAxisX() 
{
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarAxisY() 
{
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarAxisZ() 
{
}

//---------------------------------------------------------------------------

void CMainFrame::OnMainToolbarAlign() 
{
}

//---------------------------------------------------------------------------

void CMainFrame::OnUpdateMainToolbarSelect(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateMainToolbarMove(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateMainToolbarRotate(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateMainToolbarScale(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateMainToolbarAxisX(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateMainToolbarAxisY(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateMainToolbarAxisZ(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateMainToolbarAlign(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( FALSE );
}

//---------------------------------------------------------------------------

void CMainFrame::OnEditSelectAll() 
{
	m_pDoc->SelectAll();

    POSITION            Pos     = m_pDoc->GetFirstViewPosition();
    CParticleView3D*    pView   = (CParticleView3D*)m_pDoc->GetNextView( Pos );

    pView->UpdateKeyBar();
}

//---------------------------------------------------------------------------

void CMainFrame::OnEditSelectNone() 
{
	m_pDoc->SelectNone();

    POSITION            Pos     = m_pDoc->GetFirstViewPosition();
    CParticleView3D*    pView   = (CParticleView3D*)m_pDoc->GetNextView( Pos );

    pView->UpdateKeyBar();
}

//---------------------------------------------------------------------------

void CMainFrame::OnEditSelectInvert() 
{
	m_pDoc->SelectInvert();

    POSITION            Pos     = m_pDoc->GetFirstViewPosition();
    CParticleView3D*    pView   = (CParticleView3D*)m_pDoc->GetNextView( Pos );

    pView->UpdateKeyBar();
}

//---------------------------------------------------------------------------

void CMainFrame::OnViewViewportMaximize() 
{
    if( m_Splitter.IsViewportMaximized() )
        m_Splitter.UnmaximizeViewport();
    else
        m_Splitter.MaximizeViewport();
}

void CMainFrame::OnUpdateViewViewportMaximize(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck( m_Splitter.IsViewportMaximized() );
}

//---------------------------------------------------------------------------

void CMainFrame::OnTimeGotoFirstFrame() 
{
    if( m_KeyBar.IsPlayOn() )
    {
        m_KeyBar.StopPlayback();
    }

    m_KeyBar.SetTime( m_KeyBar.GetTimeRangeStart() );
}

//---------------------------------------------------------------------------

void CMainFrame::OnTimeGotoLastFrame() 
{
    if( m_KeyBar.IsPlayOn() )
    {
        m_KeyBar.StopPlayback();
    }

    m_KeyBar.SetTime( m_KeyBar.GetTimeRangeEnd() );
}

//---------------------------------------------------------------------------

void CMainFrame::OnTimeGotoPreviousFrame() 
{
    if( m_KeyBar.IsPlayOn() )
    {
        m_KeyBar.StopPlayback();
    }

    m_KeyBar.SetTime( (s32)m_KeyBar.GetTime() - 1 );
}

//---------------------------------------------------------------------------

void CMainFrame::OnTimeGotoNextFrame() 
{
    if( m_KeyBar.IsPlayOn() )
    {
        m_KeyBar.StopPlayback();
    }

    m_KeyBar.SetTime( (s32)m_KeyBar.GetTime() + 1 );
}

//---------------------------------------------------------------------------

void CMainFrame::OnTimePlay() 
{
    if( m_KeyBar.IsPlayOn() )     { m_KeyBar.StopPlayback();  }
    else                          { m_KeyBar.StartPlayback(); }
}

//---------------------------------------------------------------------------

void CMainFrame::OnTimeZoomTimeExtents() 
{
    if( m_pDoc->m_Effect.GetNumElements() == 0 )
    {
        // Default Range
        SetMinMaxTime( 0, 60 );
    }
    else
    {
        SetMinMaxTime( 0, m_pDoc->m_Effect.GetLifeSpan() );
    }
}

//---------------------------------------------------------------------------

