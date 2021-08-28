// ChildFrm.cpp : implementation of the CMeshViewer_Frame class
//

#include "stdafx.h"
#include "resource.h"
#include "MeshViewer_Frame.h"
#include "MeshViewer_View.h"
#include "MeshWorkspaceView.h"
#include "MeshWorkspaceDoc.h"
#include "..\editor\project.hpp"
#include "..\WinControls\FileSearch.h"
#include "..\EDRscDesc\RSCDesc.hpp"
#include "CDERR.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//=========================================================================

CWnd* FindViewFromTab( CXTTabCtrlBar& Bar, CRuntimeClass *pViewClass )
{
    CWnd* pWnd=NULL;
    for( s32 i=0; pWnd=Bar.GetView(i); i++ )
    {
        if( pWnd->IsKindOf( pViewClass ) )
        {
            break;
        }
        else if( pWnd->IsKindOf( RUNTIME_CLASS(CFrameWnd) ) )
        {
            CFrameWnd* pFrame = (CFrameWnd*)pWnd;
            CView*     pView = pFrame->GetActiveView();

            if( pView->IsKindOf( pViewClass ) )
            {
                pWnd = pView;
                break;
            }
        }
    }

    return pWnd;
}

/////////////////////////////////////////////////////////////////////////////
// CMeshViewer_Frame

IMPLEMENT_DYNCREATE(CMeshViewer_Frame, CBaseFrame)

BEGIN_MESSAGE_MAP(CMeshViewer_Frame, CBaseFrame)
	//{{AFX_MSG_MAP(CMeshViewer_Frame)
	ON_WM_CREATE()
    ON_WM_DESTROY()
	ON_COMMAND(ID_ORBITCAM, OnOrbitcam)
	ON_COMMAND(ID_FREEFLY_CAMERA, OnFreeflyCamera)
	ON_COMMAND(ID_LOADMESH, OnLoadMesh)
	ON_COMMAND(ID_PLAY, OnPlayAnim)
	ON_COMMAND(ID_PAUSE, OnPauseAnim)
	ON_COMMAND(ID_MV_ADD_MATX_AS_SKINGEOM, OnMvAddMatxAsSkinGeom)
	ON_COMMAND(ID_MV_ADD_MATX_AS_RIGIDGEOM, OnMvAddMatxAsRigidGeom)
	ON_UPDATE_COMMAND_UI(ID_MV_ADD_MATX_AS_RIGIDGEOM, OnUpdateMvAddMatxAsRigidGeom)
	ON_UPDATE_COMMAND_UI(ID_MV_ADD_MATX_AS_SKINGEOM, OnUpdateMvAddMatxAsSkinGeom)
	ON_UPDATE_COMMAND_UI(ID_MV_OPEN_THEME, OnUpdateMvOpenTheme)
	ON_UPDATE_COMMAND_UI(ID_ORBITCAM, OnUpdateOrbitcam)
	ON_UPDATE_COMMAND_UI(ID_FREEFLY_CAMERA, OnUpdateFreeflyCamera)
	ON_COMMAND(ID_ANIM_IN_PLACE, OnAnimInPlace)
	ON_UPDATE_COMMAND_UI(ID_ANIM_IN_PLACE, OnUpdateAnimInPlace)
	ON_COMMAND(ID_MESHV_RENDER_SKELETON, OnRenderSkeleton)
	ON_UPDATE_COMMAND_UI(ID_MESHV_RENDER_SKELETON, OnUpdateRenderSkeleton)
	ON_COMMAND(ID_MESHV_TAKE_TO_BIND, OnTakeToBind)
	ON_UPDATE_COMMAND_UI(ID_MESHV_TAKE_TO_BIND, OnUpdateTakeToBind)
	ON_COMMAND(ID_MESHV_RENDER_BONE_LAVELS, OnRenderBoneLavels)
	ON_UPDATE_COMMAND_UI(ID_MESHV_RENDER_BONE_LAVELS, OnUpdateRenderBoneLavels)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//==============================================================================
// CMeshViewer_Frame construction/destruction
//==============================================================================

CMeshViewer_Frame::CMeshViewer_Frame()
{
    m_pDoc          = NULL;
}

//==============================================================================

CMeshViewer_Frame::~CMeshViewer_Frame()
{
}

//==============================================================================

BOOL CMeshViewer_Frame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	if( !CBaseFrame::PreCreateWindow(cs) )
		return FALSE;
	
//	cs.style = WS_CHILD | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
//		| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
//
//	cs.style |= WS_CLIPCHILDREN;

	return TRUE;
}

//==============================================================================

void CMeshViewer_Frame::ActivateFrame(int nCmdShow)
{
    nCmdShow = SW_SHOWMAXIMIZED;
	CBaseFrame::ActivateFrame(nCmdShow);

    if( m_pDoc == NULL )
    {
        m_pDoc = (CMeshViewer_Doc*)GetActiveDocument();
    }
}

//==============================================================================
// CMeshViewer_Frame diagnostics
//==============================================================================

#ifdef _DEBUG
void CMeshViewer_Frame::AssertValid() const
{
	CBaseFrame::AssertValid();
}

void CMeshViewer_Frame::Dump(CDumpContext& dc) const
{
	CBaseFrame::Dump(dc);
}

#endif //_DEBUG

//==============================================================================
// CMeshViewer_Frame message handlers
//==============================================================================

int CMeshViewer_Frame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBaseFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

    // Create the ToolBar
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MESH_VIEWER))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	
	// Create the workspace bar.
	if( !m_wndWrkspBar.Create(this, IDW_WORKSPBAR, _T("Workspace"),
		CSize(200, 150), CBRS_LEFT, CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_CLIENT_STATIC)) //(AFX_IDW_TOOLBAR + 11) ))
	{
		TRACE0("Failed to create workspace dock window\n");
		return -1;		// fail to create
	}

	m_wndWrkspBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	m_wndToolBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
    
    EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
	DockControlBar( &m_wndToolBar );
	DockControlBar(&m_wndWrkspBar,AFX_IDW_DOCKBAR_LEFT);

    CMeshWorkspaceDoc *pMVDoc = new CMeshWorkspaceDoc();
    if (pMVDoc)
    {
        pMVDoc->SetTabParent(&m_wndWrkspBar);
        pMVDoc->SetFramePointer(this);

        // OLD WAY
        // VERIFY(m_wndWrkspBar.AddView(_T("Workspace"), RUNTIME_CLASS(CMeshWorkspaceView), pMVDoc));

        CFrameWnd* pFrameWnd = NULL;
        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CMeshWorkspaceView), pMVDoc);
        m_wndWrkspBar.AddControl(_T("Workspace"), pFrameWnd);

    }

    // Load control bar postion.
    LoadBarState(_T("BarState - Mesh"));

    return 0;
}

//=========================================================================

void CMeshViewer_Frame::OnDestroy( )
{
    // Save control bar postion.
    SaveBarState(_T("BarState - Mesh"));
}

//==============================================================================

void CMeshViewer_Frame::OnOrbitcam() 
{
	// TODO: Add your command handler code here
    m_pDoc->CameraOrbitMode();
}

//==============================================================================

void CMeshViewer_Frame::OnFreeflyCamera() 
{
	// TODO: Add your command handler code here
	m_pDoc->CameraFreeFlyMode();
}

//==============================================================================

void CMeshViewer_Frame::OnLoadMesh() 
{
	// TODO: Add your command handler code here
    m_pDoc->LoadMesh();
}

//==============================================================================

void CMeshViewer_Frame::OnPlayAnim() 
{
	// TODO: Add your command handler code here
	m_pDoc->PlayAnimation();	
}

//==============================================================================

void CMeshViewer_Frame::OnPauseAnim() 
{
	// TODO: Add your command handler code here
    m_pDoc->PauseAnimation();		
}

//==============================================================================

void CMeshViewer_Frame::OnUpdateMvAddMatxAsRigidGeom(CCmdUI* pCmdUI) 
{
    BOOL bEnable = FALSE;

    CMeshWorkspaceView* pView = (CMeshWorkspaceView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CMeshWorkspaceView));
    if (pView)
    {
        if (pView->CanAddToTheme())
        {
            bEnable = TRUE;
        }
    }
    pCmdUI->Enable(bEnable);
}

//==============================================================================

void CMeshViewer_Frame::OnUpdateMvAddMatxAsSkinGeom(CCmdUI* pCmdUI) 
{
    BOOL bEnable = FALSE;
    CMeshWorkspaceView* pView = (CMeshWorkspaceView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CMeshWorkspaceView));
    if (pView)
    {
        if (pView->CanAddToTheme())
        {
            bEnable = TRUE;
        }
    }
    pCmdUI->Enable(bEnable);
}

//==============================================================================

void CMeshViewer_Frame::OnUpdateMvOpenTheme(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
}

//==============================================================================

void CMeshViewer_Frame::OnUpdateOrbitcam(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if (m_pDoc)
	    pCmdUI->SetCheck(m_pDoc->IsOrbitMode());
}

//==============================================================================

void CMeshViewer_Frame::OnUpdateFreeflyCamera(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if (m_pDoc)
	    pCmdUI->SetCheck(!m_pDoc->IsOrbitMode());
}

//==============================================================================

void CMeshViewer_Frame::OnMvAddMatxAsRigidGeom() 
{
    OnMvAddMatxAsRes("rigidgeom", TRUE);
}

//==============================================================================

void CMeshViewer_Frame::OnMvAddMatxAsSkinGeom() 
{
    OnMvAddMatxAsRes("skingeom", FALSE);
}

//==============================================================================

void CMeshViewer_Frame::OnMvAddMatxAsRes(CString strType, BOOL bIncludeCollision) 
{
    s32 MaxBuffer = MAX_PATH*1024;
    char* pBuffer = NULL;

    x_try;

    CFileDialog FileOpen(TRUE, 
            _T("matx"), 
            _T(""), 
            OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER, 
            "MATX Files (*.matx)|*.matx||");

    CString strOpenPath(g_Settings.GetSourcePath());
    strOpenPath += "\\Art";
    FileOpen.m_ofn.lpstrInitialDir = strOpenPath;

    pBuffer = new char[ MaxBuffer ];
    if( pBuffer == NULL )
        x_throw( "Out of memory" );

    x_memset( pBuffer, 0, MaxBuffer );

    FileOpen.m_ofn.lpstrFile = pBuffer;
    FileOpen.m_ofn.nMaxFile  = MaxBuffer;

    if( FileOpen.DoModal() == IDOK )
    {
        CWaitCursor wc;
        CMeshWorkspaceView* pView = (CMeshWorkspaceView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CMeshWorkspaceView));
        if (pView)
        {
            int nCount = 0;
            POSITION pos = FileOpen.GetStartPosition();
            while (pos != NULL)
            {
                CString strMatxPath = FileOpen.GetNextPathName(pos);
                CString strMatxDir = strMatxPath.Left( strMatxPath.ReverseFind('\\'));
                CString strMatxFileName = strMatxPath.Right( strMatxPath.GetLength() - strMatxPath.ReverseFind('\\') -1);
                CString strResourceFileName = strMatxFileName.Left ( strMatxFileName.ReverseFind('.') );
                CString strCollisionFilePath = strMatxDir + "\\" + strResourceFileName + "_c.MATX";
                strResourceFileName += "." + strType;
                CString strResourceFilePath = pView->GetRscPath() + "\\" + strResourceFileName;

                rsc_desc& RscDesc = g_RescDescMGR.CreateRscDesc( strResourceFilePath );
                prop_query propQueryFile;
                propQueryFile.WQueryFileName( "ResDesc\\FileName", strMatxPath);
                if (RscDesc.OnProperty(propQueryFile))
                {
                    RscDesc.SetBeingEdited( TRUE );
                    if (bIncludeCollision && CFileSearch::DoesFileExist(strCollisionFilePath))
                    {
                        prop_query propQueryCol;
                        propQueryCol.WQueryFileName( "ResDesc\\Collision", strCollisionFilePath);
                        VERIFY(RscDesc.OnProperty(propQueryCol));
                    }
                    g_RescDescMGR.Save( RscDesc );
                }
                nCount++;
            }

            pView->RefreshThemeRsc();
        }
    }
    else
    {
        DWORD Hres = CommDlgExtendedError();
        switch( Hres )
        {
            case FNERR_BUFFERTOOSMALL:
                x_throw( "You have selected more files than the editor can handle at ones." );
            case FNERR_INVALIDFILENAME:
                x_throw( "Invalid file name. Ask programmers about this error" );
            case FNERR_SUBCLASSFAILURE: 
                x_throw( "Subclass failure. Ask programmers about this error" );
        }

    }

    x_catch_display;

    if( pBuffer )
        delete []pBuffer;
}

//==============================================================================

void CMeshViewer_Frame::RefreshThemeRsc()
{
    CMeshWorkspaceView* pView = (CMeshWorkspaceView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CMeshWorkspaceView));
    if (pView)
    {
        pView->RefreshThemeRsc();
    }
}

//==============================================================================

void CMeshViewer_Frame::OnAnimInPlace() 
{
	m_pDoc->m_bPlayInPlace = !m_pDoc->m_bPlayInPlace;	
}

//==============================================================================

void CMeshViewer_Frame::OnUpdateAnimInPlace(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if (m_pDoc)
	    pCmdUI->SetCheck(!m_pDoc->m_bPlayInPlace);	
}

//==============================================================================

void CMeshViewer_Frame::OnRenderSkeleton() 
{
	// TODO: Add your command handler code here
	m_pDoc->m_bRenderSkeleton = !m_pDoc->m_bRenderSkeleton;
}

//==============================================================================

void CMeshViewer_Frame::OnUpdateRenderSkeleton(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if (m_pDoc)
	    pCmdUI->SetCheck(m_pDoc->m_bRenderSkeleton);		
}

//==============================================================================

void CMeshViewer_Frame::OnTakeToBind() 
{
	// TODO: Add your command handler code here
    m_pDoc->m_bTakeToBindPose = !m_pDoc->m_bTakeToBindPose;	
}

//==============================================================================

void CMeshViewer_Frame::OnUpdateTakeToBind(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if (m_pDoc)
	    pCmdUI->SetCheck(m_pDoc->m_bTakeToBindPose);			
}

//==============================================================================

void CMeshViewer_Frame::OnRenderBoneLavels() 
{
	// TODO: Add your command handler code here
	m_pDoc->m_bRenderBoneLavels = !m_pDoc->m_bRenderBoneLavels;	
}

//==============================================================================

void CMeshViewer_Frame::OnUpdateRenderBoneLavels(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if (m_pDoc)
	    pCmdUI->SetCheck(m_pDoc->m_bRenderBoneLavels );				
}
