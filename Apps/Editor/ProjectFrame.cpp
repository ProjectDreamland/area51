// ProjectFrame.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "editor.h"
#include "ProjectFrame.h"
#include "resource.h"
#include "ProjectDoc.h"
#include "project.hpp"

#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectFrame

IMPLEMENT_DYNCREATE(CProjectFrame, CBaseFrame)


BEGIN_MESSAGE_MAP(CProjectFrame, CBaseFrame)
	//{{AFX_MSG_MAP(CProjectFrame)
	ON_WM_CREATE()
    ON_WM_DESTROY()
	ON_WM_MDIACTIVATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_INSERT_THEME, OnInsertTheme)
	ON_COMMAND(ID_PROJ_CREATE_NEW_THEME, OnProjCreateNewTheme)
	ON_COMMAND(ID_REMOVE_THEME, OnRemoveTheme)
	ON_UPDATE_COMMAND_UI(ID_PROJ_CREATE_NEW_THEME, OnUpdateProjCreateNewTheme)
	ON_UPDATE_COMMAND_UI(ID_INSERT_THEME, OnUpdateInsertTheme)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_THEME, OnUpdateRemoveTheme)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//=========================================================================

CProjectFrame::CProjectFrame()
{
    m_pDoc        = NULL;
    m_pProjectProp = NULL;
}

//=========================================================================

CProjectFrame::~CProjectFrame()
{
}

//=========================================================================

int CProjectFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBaseFrame::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    SetWindowText("Empty Project");

	EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);

	// Create the property bar.
	if( !m_TabCtrl.Create(this, IDW_PROJ_PROPERTY_BAR, _T("Properties"),
		CSize(200, 150), CBRS_LEFT, CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_CLIENT_STATIC)) //(AFX_IDW_TOOLBAR + 6) ))
	{
		TRACE0("Failed to create property dock window\n");
		return -1;		// fail to create
	}
	m_TabCtrl.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	DockControlBar(&m_TabCtrl,AFX_IDW_DOCKBAR_LEFT);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_PROJ_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
    m_wndToolBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
    DockControlBar(&m_wndToolBar );

	// Associate the image list with the tab control bar.
    m_pProjectProp  = new CPropertyEditorDoc;
    m_pSettingsProp = new CPropertyEditorDoc;
    if( m_pProjectProp == NULL || m_pSettingsProp == NULL )
    {
        if( m_pProjectProp ) delete []m_pProjectProp;
        if( m_pSettingsProp ) delete []m_pSettingsProp;
        x_throw( "Out of memory" );
    }

    // OLD WAY
    // m_TabCtrl.AddView(_T("Settings"), RUNTIME_CLASS(CPropertyEditorView),  m_pSettingsProp );
	// m_TabCtrl.AddView(_T("Project"),  RUNTIME_CLASS(CPropertyEditorView),  m_pProjectProp );


    CFrameWnd* pFrameWnd = NULL;
    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CPropertyEditorView), m_pSettingsProp);
    m_TabCtrl.AddControl(_T("Settings"), pFrameWnd);
    m_pSettingsProp->GetView()->LoadColumnState( "BarState - Settings" );

    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CPropertyEditorView), m_pProjectProp);
    m_TabCtrl.AddControl(_T("Project"), pFrameWnd);
    m_pProjectProp->GetView()->LoadColumnState( "BarState - Project" );

    m_TabCtrl.ModifyXTBarStyle(CBRS_XT_CLOSEBTN, 0);

	//m_TabCtrl.ModifyTabStyle(TCS_BOTTOM, 0);	

    // Load control bar postion.
    LoadBarState(_T("BarState - Project"));

	return 0;
}

//=========================================================================

void CProjectFrame::OnDestroy( )
{
    // Save control bar postion.
    SaveBarState(_T("BarState - Project"));
    m_pSettingsProp->GetView()->SaveColumnState( "BarState - Settings" );
    m_pProjectProp->GetView()->SaveColumnState( "BarState - Project" );
}

//=========================================================================

void CProjectFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CBaseFrame::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	
    SetProject( m_strCurrentProject );

    if( m_pDoc == NULL )
    {
        m_pDoc = (CProjectDoc*)GetActiveDocument();
        m_pDoc->InitFormFrame( m_pSettingsProp, m_pProjectProp );
    }
}

//=========================================================================

void CProjectFrame::SetProject(CString strProject) 
{ 
    m_strCurrentProject = strProject;
    if (!m_strCurrentProject.IsEmpty())
    {
        SetWindowText( m_strCurrentProject );	
    }
    else
    {
        SetWindowText("Empty Project");
    }
}

//=========================================================================

void CProjectFrame::OnClose() 
{
	// Can't close this window!!!!!!
	
	//CBaseFrame::OnClose();
}

//=========================================================================

void CProjectFrame::OnProjCreateNewTheme() 
{
    m_pDoc->CreateNewTheme();	
}

//=========================================================================

void CProjectFrame::OnUpdateProjCreateNewTheme(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(CanEdit());
}

//=========================================================================

void CProjectFrame::OnInsertTheme() 
{
	m_pDoc->InsertTheme();
}

//=========================================================================

void CProjectFrame::OnUpdateInsertTheme(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(CanEdit());
}

//=========================================================================

void CProjectFrame::OnRemoveTheme() 
{
	m_pDoc->RemoveTheme();
}

//=========================================================================

void CProjectFrame::OnUpdateRemoveTheme(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(CanEdit() && g_Project.GetThemeCount());
}

//=========================================================================

BOOL CProjectFrame::CanEdit()
{
    BOOL bEnable = FALSE;
    if (g_Project.IsProjectOpen())
    {
        CFileStatus status;

        char FullFileName[256];
        g_Project.GetFileName(FullFileName);
        if( CFile::GetStatus( FullFileName, status ) )   // static function
        {
            bEnable = !(status.m_attribute & CFile::readOnly);
        }    
    }

    return bEnable;
}
