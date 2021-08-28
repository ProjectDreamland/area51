// EDRscDesc_Frame.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "EDRscDesc_Frame.h"

#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "../WorldEditor/EditorDoc.h"

extern user_settings    g_SaveTrackUserSettings;
extern user_settings    g_LoadUpdateUserSettings;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(EDRscDesc_Frame, CBaseFrame)

BEGIN_MESSAGE_MAP(EDRscDesc_Frame, CBaseFrame)
	//{{AFX_MSG_MAP(EDRscDesc_Frame)
	ON_WM_CREATE()
    ON_WM_DESTROY()
	ON_COMMAND(ID_RSC_ADD_RES_DESC, OnRscAddResDesc)
    ON_COMMAND_RANGE( ID_EDRSCDESC_ITEM1, ID_EDRSCDESC_ITEM24, OnPopupAddRscDesc )
	ON_COMMAND(ID_RSC_BUILD, OnRscBuild)
	ON_COMMAND(ID_RSC_BUILD_STOP, OnRscBuildStop)
	ON_COMMAND(ID_SAVE_ACTIVE, OnSaveActive)
	ON_COMMAND(ID_EDIT_RESCDESC, OnEditRescdesc)
    ON_UPDATE_COMMAND_UI(ID_EDIT_RESCDESC, OnEditRescdescUpdate)
	ON_COMMAND(ID_CHECKOUT_RESCDESC, OnCheckoutRescdesc)
    ON_UPDATE_COMMAND_UI(ID_CHECKOUT_RESCDESC, OnCheckoutRescdescUpdate)
	ON_COMMAND(ID_COMPILE_NINTENDO, OnCompileNintendo)
	ON_COMMAND(ID_COMPILE_PS2, OnCompilePs2)
	ON_COMMAND(ID_COMPILE_XBOX, OnCompileXbox)
	ON_COMMAND(ID_VERBOSE_MODE, OnVerboseMode)
    ON_COMMAND(ID_COLOR_MIPS, OnColorMips)
	ON_COMMAND(ID_COMPILE_PC, OnCompilePC)
    ON_UPDATE_COMMAND_UI(ID_COMPILE_NINTENDO, OnCompileNintendoUpdate)
    ON_UPDATE_COMMAND_UI(ID_COMPILE_PS2, OnCompilePs2Update)
    ON_UPDATE_COMMAND_UI(ID_COMPILE_XBOX, OnCompileXboxUpdate)
    ON_UPDATE_COMMAND_UI(ID_VERBOSE_MODE, OnVerboseModeUpdate)
    ON_UPDATE_COMMAND_UI(ID_COLOR_MIPS, OnColorMipsUpdate)
    ON_UPDATE_COMMAND_UI(ID_COMPILE_PC, OnCompilePCUpdate)
	ON_COMMAND(ID_CLEAN_RESOURCE, OnCleanResource)
	ON_COMMAND(ID_REFRESH_VIEWS, OnRefreshViews)
	ON_COMMAND(ID_DELETE_RSCDESC, OnDeleteRscdesc)
	ON_COMMAND(ID_REBUILD_ALL, OnRebuildAll)
    ON_COMMAND(ID_SCAN_RESOURCES, OnScanResources)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//=========================================================================

EDRscDesc_Frame::EDRscDesc_Frame()
{
    m_Init = FALSE;
}

//=========================================================================

EDRscDesc_Frame::~EDRscDesc_Frame()
{
}

//=========================================================================

BOOL EDRscDesc_Frame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	if( !CBaseFrame::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

//=========================================================================

void EDRscDesc_Frame::ActivateFrame(int nCmdShow)
{
    nCmdShow = SW_SHOWMAXIMIZED;
	CBaseFrame::ActivateFrame(nCmdShow);

    //
    // Connect the event editor with the property editor
    //
    if( m_Init == FALSE )
    {
        m_Init = TRUE;
	    // TODO: Add your own view and documents to the workspace window.
        EDRscDesc_Doc& Doc = *((EDRscDesc_Doc*)GetActiveDocument());
        m_pDoc               = &Doc;
        Doc.FrameInit( m_pPropEditor );
    }
    UpdateButtons();
}

//=========================================================================

int EDRscDesc_Frame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBaseFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the property bar.
	if( !m_TabCtrl.Create(this, IDR_RSC_DESC_PROPERTY_TAB, _T("Properties"),
		CSize(350, 150), CBRS_LEFT, CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_CLIENT_STATIC)) //(AFX_IDW_TOOLBAR + 6) ))
	{
		TRACE0("Failed to create property dock window\n");
		return -1;		// fail to create
	}

	if (!m_BuildToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_BuildToolBar.LoadToolBar(IDW_RSCDESC_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    m_BuildToolBar.SetWindowText( "Build" );
//    m_BuildToolBar.SetCustomBar(TRUE);

	if (!m_EditToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(0,0,0,0),
        AFX_IDW_CONTROLBAR_FIRST+1) ||
		!m_EditToolBar.LoadToolBar(IDR_RSCDESC_EDIT_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
    m_EditToolBar.SetWindowText( "Edit" );
//    m_EditToolBar.SetCustomBar(TRUE);

    m_TabCtrl.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
    m_BuildToolBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
    m_EditToolBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
    EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);

    DockControlBar(&m_TabCtrl,AFX_IDW_DOCKBAR_LEFT);
    DockControlBar(&m_BuildToolBar );
    DockControlBarLeftOf(&m_EditToolBar, &m_BuildToolBar);

	// Associate the image list with the tab control bar.
    m_pPropEditor = new CPropertyEditorDoc;
    if( m_pPropEditor == NULL )
        x_throw( "Out of memory" );

// OLD WAY
//	m_TabCtrl.AddView(_T("Properties"), RUNTIME_CLASS(CPropertyEditorView), m_pPropEditor );

    CFrameWnd* pFrameWnd = NULL;
    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CPropertyEditorView), m_pPropEditor);
    m_TabCtrl.AddControl(_T("Properties"), pFrameWnd);
    m_pPropEditor->GetView()->LoadColumnState( "BarState - Resource Project" );

    m_TabCtrl.ModifyXTBarStyle(CBRS_XT_CLOSEBTN, 0);

    // Restore control bar postion.
    LoadBarState(_T("BarState - EdRscDesc"));

    return 0;
}

//=========================================================================

void EDRscDesc_Frame::OnDestroy() 
{
    // Save control bar postion.
    SaveBarState(_T("BarState - EdRscDesc"));
    m_pPropEditor->GetView()->SaveColumnState( "BarState - Resource Project" );
}

//=========================================================================

void EDRscDesc_Frame::OnRscAddResDesc()
{
	// TODO: Add your command handler code here
    x_try;

	CPoint pt;
    RECT   Rect;

    s32 iItem = m_BuildToolBar.GetHotItem();
    m_BuildToolBar.GetItemRect( iItem, &Rect );

    pt.y = Rect.bottom-8;
    pt.x = Rect.left-16;
    m_BuildToolBar.ClientToScreen( &pt );


    CXTMenu MainTPMMenu;

    MainTPMMenu.CreatePopupMenu();

    //
    // Lets all all the types
    //
    xarray<xstring> Types;
    m_pDoc->GetTypeList( Types );

    for( s32 i=0; i<Types.GetCount(); i++ )
    {
        MainTPMMenu.AppendMenu(MF_STRING | MF_ENABLED, ID_EDRSCDESC_ITEM1+i, (const char*)Types[i] );
    }

    MainTPMMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                                pt.x, pt.y, this, NULL);

    x_catch_display;
}

//=========================================================================

void EDRscDesc_Frame::OnPopupAddRscDesc( UINT nID )
{
    xarray<xstring> Types;

    x_try;

    nID -= ID_EDRSCDESC_ITEM1;
    m_pDoc->GetTypeList( Types );
    m_pDoc->AddRscDesc ( Types[(s32)nID] );
    x_catch_display;
}

//=========================================================================

void EDRscDesc_Frame::OnRscBuild()
{
	// TODO: Add your command handler code here
	m_pDoc->Build();
}

//=========================================================================

void EDRscDesc_Frame::OnRscBuildStop()
{
	// TODO: Add your command handler code here
    m_pDoc->StopBuild();
}

//=========================================================================

void EDRscDesc_Frame::OnSaveActive()
{
	// TODO: Add your command handler code here
	m_pDoc->SaveActive();
}

//=========================================================================

void EDRscDesc_Frame::OnEditRescdesc()
{
	// TODO: Add your command handler code here
	m_pDoc->StartStopEdit();
}

//=========================================================================

void EDRscDesc_Frame::OnEditRescdescUpdate( CCmdUI* pCmdUI )
{
    if( m_Init )
    {
        pCmdUI->SetCheck(m_pDoc->IsSelectedBeenEditedLocal() );
    }
    else
    {
        pCmdUI->SetCheck( FALSE );
    }
}

//=========================================================================

void EDRscDesc_Frame::OnCheckoutRescdesc()
{
	m_pDoc->CheckOutSelected();
}

//=========================================================================

void EDRscDesc_Frame::OnCheckoutRescdescUpdate( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( TRUE );
    pCmdUI->SetCheck( FALSE );
}

//=========================================================================

void EDRscDesc_Frame::OnCompileNintendoUpdate( CCmdUI* pCmdUI )
{
    if( m_Init )
    {
        UpdateButtons();
        pCmdUI->SetCheck(m_pDoc->IsCompileNintendo() );
        g_SaveTrackUserSettings.CompileGCN = m_pDoc->IsCompileNintendo();
    }
    else
    {
        pCmdUI->SetCheck( FALSE );
        g_SaveTrackUserSettings.CompileGCN = FALSE;
    }
}

//=========================================================================

void EDRscDesc_Frame::OnCompilePs2Update( CCmdUI* pCmdUI )
{
    if( m_Init )
    {
        UpdateButtons();
        pCmdUI->SetCheck(m_pDoc->IsCompilePS2() );
        g_SaveTrackUserSettings.CompilePS2 = m_pDoc->IsCompilePS2();
    }
    else
    {
        pCmdUI->SetCheck( FALSE );
        g_SaveTrackUserSettings.CompilePS2 = FALSE;
    }
}

//=========================================================================

void EDRscDesc_Frame::OnCompileXboxUpdate( CCmdUI* pCmdUI )
{
    if( m_Init )
    {
        UpdateButtons();
        pCmdUI->SetCheck(m_pDoc->IsCompileXBox() );
        g_SaveTrackUserSettings.CompileXBOX = m_pDoc->IsCompileXBox();
    }
    else
    {
        g_SaveTrackUserSettings.CompileXBOX = FALSE;
        pCmdUI->SetCheck( FALSE );
    }
}

//=========================================================================

void EDRscDesc_Frame::OnVerboseModeUpdate( CCmdUI* pCmdUI )
{
    if( m_Init )
    {
        pCmdUI->SetCheck(m_pDoc->IsVerboseMode() );
    }
    else
    {
        pCmdUI->SetCheck( FALSE );
    }
}

//=========================================================================

void EDRscDesc_Frame::OnColorMipsUpdate( CCmdUI* pCmdUI )
{
    if( m_Init ) pCmdUI->SetCheck(m_pDoc->IsColorMipsMode() );
    else          pCmdUI->SetCheck( FALSE );
}

//=========================================================================

void EDRscDesc_Frame::OnCompilePCUpdate( CCmdUI* pCmdUI )
{
    if( m_Init )
    {
        UpdateButtons();
        pCmdUI->SetCheck(m_pDoc->IsCompilePC() );
        g_SaveTrackUserSettings.CompilePC = m_pDoc->IsCompilePC();
    }
    else
    {
        g_SaveTrackUserSettings.CompilePC = TRUE;
        pCmdUI->SetCheck( TRUE );
    }
}

//=========================================================================

void EDRscDesc_Frame::OnCompileNintendo()
{
	// TODO: Add your command handler code here
    m_pDoc->ToggleCompileNintendo();
    g_SaveTrackUserSettings.CompileGCN = m_pDoc->IsCompileNintendo();
}

//=========================================================================

void EDRscDesc_Frame::OnCompilePs2()
{
	// TODO: Add your command handler code here
    m_pDoc->ToggleCompilePS2();
    g_SaveTrackUserSettings.CompilePS2 = m_pDoc->IsCompilePS2();
}

//=========================================================================

void EDRscDesc_Frame::OnCompileXbox()
{
	// TODO: Add your command handler code here
	m_pDoc->ToggleCompileXBox();
    g_SaveTrackUserSettings.CompileXBOX = m_pDoc->IsCompileXBox();
}

//=========================================================================

void EDRscDesc_Frame::OnVerboseMode()
{
	// TODO: Add your command handler code here
	m_pDoc->ToggleVerboseMode();
}

//=========================================================================

void EDRscDesc_Frame::OnColorMips()
{
	// TODO: Add your command handler code here
	m_pDoc->ToggleColorMipsMode();
}

//=========================================================================

void EDRscDesc_Frame::OnCompilePC()
{
	// TODO: Add your command handler code here
	m_pDoc->ToggleCompilePC();
    g_SaveTrackUserSettings.CompilePC = m_pDoc->IsCompilePC();
}

//=========================================================================

void EDRscDesc_Frame::OnCleanResource()
{
	// TODO: Add your command handler code here
    m_pDoc->CleanSelected();
}

//=========================================================================

void EDRscDesc_Frame::OnRefreshViews()
{
	// TODO: Add your command handler code here
    m_pDoc->Refresh();
}

//=========================================================================

void EDRscDesc_Frame::OnDeleteRscdesc()
{
	// TODO: Add your command handler code here
	m_pDoc->DeleteSelectedResource();
}

//=========================================================================

void EDRscDesc_Frame::OnRebuildAll()
{
	// TODO: Add your command handler code here
	m_pDoc->RebuildAll();
}

//=========================================================================

void EDRscDesc_Frame::OnScanResources()
{
    m_pDoc->ScanResources();
}

//=========================================================================

//=========================================================================

void EDRscDesc_Frame::UpdateButtons( void )
{
    if(!g_LoadUpdateUserSettings.UpdateCompileButtonsFlag)
    {
        if(g_LoadUpdateUserSettings.CompileGCN)
             m_pDoc->ForceCompileNintendo();
        else
            m_pDoc->ForceCompileNintendoOff();

        if(g_LoadUpdateUserSettings.CompilePS2)
            m_pDoc->ForceCompilePS2();
        else
            m_pDoc->ForceCompilePS2Off();

        if(g_LoadUpdateUserSettings.CompileXBOX)
            m_pDoc->ForceCompileXBox();
        else
            m_pDoc->ForceCompileXBoxOff();

        if(g_LoadUpdateUserSettings.CompilePC)
            m_pDoc->ForceCompilePC();
        else
            m_pDoc->ForceCompilePCOff();


        g_LoadUpdateUserSettings.UpdateCompileButtonsFlag = TRUE;
    }
}

//=========================================================================

