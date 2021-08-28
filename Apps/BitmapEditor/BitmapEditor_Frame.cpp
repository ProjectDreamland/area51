// BitmapEditor.cpp : implementation file
//

#include "stdafx.h"
#include "BitmapEditor_Frame.h"
#include "BitmapEditor_View.h"
#include "BitmapEditor_Doc.h"
#include "resource.h"


#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitmapEditor_Frame

IMPLEMENT_DYNCREATE(CBitmapEditor_Frame, CBaseFrame)

CBitmapEditor_Frame::CBitmapEditor_Frame()
{
    m_Init = FALSE;
}

CBitmapEditor_Frame::~CBitmapEditor_Frame()
{
}


BEGIN_MESSAGE_MAP(CBitmapEditor_Frame, CBaseFrame)
	//{{AFX_MSG_MAP(CBitmapEditor_Frame)
	ON_WM_CREATE()
    ON_WM_DESTROY()
	ON_COMMAND(ID_SAVE_DESC, OnSaveDesc)
	ON_COMMAND(ID_OPEN, OnOpen)
	ON_COMMAND(ID_NEW_DESC, OnNewDesc)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBitmapEditor_Frame message handlers

void CBitmapEditor_Frame::ActivateFrame(int nCmdShow) 
{
	// TODO: Add your specialized code here and/or call the base class
    nCmdShow = SW_SHOWMAXIMIZED;
	CBaseFrame::ActivateFrame(nCmdShow);
	
    //
    // Connect the event editor with the property editor
    //
    if( m_Init == FALSE )
    {
        m_Init = TRUE;
	    // TODO: Add your own view and documents to the workspace window.
        CBitmapEditor_Doc& Doc = GetDoc();
        m_pDoc               = &Doc;
        Doc.m_pPropEditor    = m_pPropEditor;
        Doc.m_pPropEditor->SetInterface( Doc.m_BitmapEd );
        Doc.m_pFrame = this;
    }
}

int CBitmapEditor_Frame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBaseFrame::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Create the property bar.
	if( !m_TabCtrl.Create(this, IDW_PROPERTY_BAR, _T("Properties"),
		CSize(200, 150), CBRS_LEFT, CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_CLIENT_STATIC)) //(AFX_IDW_TOOLBAR + 6) ))
	{
		TRACE0("Failed to create property dock window\n");
		return -1;		// fail to create
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_GENERAL_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_TabCtrl.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	m_wndToolBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);

	EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);

	DockControlBar(&m_TabCtrl,AFX_IDW_DOCKBAR_LEFT);
	DockControlBar(&m_wndToolBar );

	// Associate the image list with the tab control bar.
    m_pPropEditor = new CPropertyEditorDoc;
    if( m_pPropEditor == NULL )
        x_throw( "Out of memory" );

    // OLD WAY
	// m_TabCtrl.AddView(_T("Properties"), RUNTIME_CLASS(CPropertyEditorView), m_pPropEditor );

    CFrameWnd* pFrameWnd = NULL;
    pFrameWnd = m_TabCtrl.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CPropertyEditorView), m_pPropEditor);
    m_TabCtrl.AddControl(_T("Properties"), pFrameWnd);

    // Load control bar postion.
    LoadBarState(_T("BarState - Bitmap"));

	return 0;
}

//=========================================================================

void CBitmapEditor_Frame::OnDestroy( )
{
    // Save control bar postion.
    SaveBarState(_T("BarState - Bitmap"));
}

void CBitmapEditor_Frame::OnSaveDesc() 
{
	// TODO: Add your command handler code here
	m_pDoc->Save();
}

void CBitmapEditor_Frame::OnOpen() 
{
	// TODO: Add your command handler code here
	m_pDoc->Open();
}

void CBitmapEditor_Frame::OnNewDesc() 
{
	// TODO: Add your command handler code here
	m_pDoc->New();
}

void CBitmapEditor_Frame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	if( m_pDoc->CloseApp() )
    {
	    CBaseFrame::OnClose();
    }
}
