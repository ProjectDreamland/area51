// EditorFrame.cpp : implementation of the CEditorFrame class
//

#include "StdAfx.h"

#include "EditorFrame.h"
#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\WinControls\StringEntryDlg.h"
#include "..\WinControls\ListBoxDlg.h"
#include "..\WinControls\FileSearch.h"
#include "..\Editor\Resource.h"
#include "..\Editor\Project.hpp"
#include "GenericDialog\GenericDialog.hpp"
#include "EditorPaletteDoc.h"
#include "EditorView.h"
#include "EditorDoc.h"
#include "Resource.h"
#include "ResourceBrowserDlg.h"

#include "EditorObjectView.h"
#include "EditorBlueprintView.h"
#include "EditorAIView.h"
#include "EditorLayerView.h"
#include "EditorSoundView.h"
#include "EditorSettingsView.h"
#include "EditorGlobalView.h"
#include "EditorDebuggerView.h"
#include "EditorWatchView.h"
#include "EditorDecalView.h"
#include "EditorTriggerView.h"

#include "transaction_mgr.hpp"
#include "TransactionStackDlg.h"

#include <IO.H>
#include <Shlwapi.h>

#include "Obj_mgr\obj_mgr.hpp"

#include "Objects\PlaySurface.hpp"
#include "Objects\PropSurface.hpp"
#include "Objects\AnimSurface.hpp"
#include "Objects\Portal.hpp"

#include "AudioMgr\AudioMgr.hpp"

#include "DialogTextEntry.h"

#include "..\Editor\MainFrm.h"
#include "Gamelib\DebugCheats.hpp"

#include "Objects\Player.hpp"   // Added to access the g_MPTweaks hack.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================
// CEditorFrame
//=========================================================================

IMPLEMENT_DYNCREATE(CEditorFrame, CBaseFrame)

BEGIN_MESSAGE_MAP(CEditorFrame, CBaseFrame)
	//{{AFX_MSG_MAP(CEditorFrame)
	ON_WM_CREATE()
    ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBOBOX_ACTIVE_LAYER, OnSelchangeActiveLayerCombo)
	ON_MESSAGE( WM_USER_MSG_UPDATE_REQUIRED, OnUpdateRequired )
	ON_MESSAGE( WM_USER_MSG_GUID_SELECT_FOR_PROPERTY, OnGuidSelectorRequest )
    ON_MESSAGE( WM_USER_MSG_GUID_HIGHLIGHT_REQUEST, OnGuidHighlightRequest )
    ON_COMMAND(IDR_CREATE_OBJECT_FROM_SELECTED, OnCreateFromSelected)
    ON_COMMAND_RANGE(IDR_CREATE_OBJECT_GENERIC_FIRST, IDR_CREATE_OBJECT_GENERIC_LAST, OnCreateGenericObject)
    ON_UPDATE_COMMAND_UI_RANGE( IDR_CREATE_OBJECT_GENERIC_FIRST, IDR_CREATE_OBJECT_GENERIC_LAST, OnUpdateCreateGenericObject )
    ON_COMMAND(ID_WETB_SHOWSELECTIONBOUNDS, OnShowSelectionBounds)
    ON_COMMAND(ID_WETB_UNDO, OnWetbUndo)
    ON_COMMAND(ID_WETB_REDO, OnWetbRedo)
    ON_COMMAND(ID_WETB_TRANS_STACK, OnWetbTransStack)
	ON_COMMAND(ID_WETB_SHOWAXIS, OnWetbShowaxis)
	ON_COMMAND(ID_WETB_SHOWGRID, OnWetbShowgrid)
	ON_COMMAND(ID_WETB_SHOWSPACD, OnWetbShowspacd)
	ON_COMMAND(ID_WETB_CREATE_OBJECT, OnWetbCreateObject)
	ON_COMMAND(ID_WETB_DELETE_SELECTED, OnWetbDeleteSelected)
	ON_COMMAND(ID_WETB_SNAPTOGRID, OnWetbSnaptogrid)
    ON_COMMAND(ID_WETB_ADDWITHRAY, OnWetbSnapAddWithRay)
    ON_COMMAND(ID_WETB_GRIDTOOBJECT, OnWetbGridToObject)
	ON_COMMAND(ID_WETB_MOVEMODE, OnWetbMoveMode)
    ON_COMMAND(ID_WETB_COPY_OBJECTS, OnWetbCopyObjects)
    ON_COMMAND(ID_WETB_MOVE_OBJECTS_TO_ACTIVE_LAYER, OnWetbMoveObjectsToActiveLayer)
    ON_COMMAND(ID_WETB_RUNGAME, OnWetbRunGame)
    ON_COMMAND(ID_WETB_PAUSEGAME, OnWetbPauseGame)
    ON_COMMAND(ID_WETB_STEPGAME, OnWetbStepGame)
    ON_COMMAND(ID_WETB_SAVEGAME, OnWetbSaveGame)
    ON_COMMAND(ID_WETB_LOADGAME, OnWetbLoadGame)
    ON_COMMAND(ID_WETB_TOGGLE_FPV, OnWetbToggleFPV)
    ON_COMMAND(ID_WETB_FOCUS_CAMERA, OnWetbFocusCam)
    ON_COMMAND(ID_WETB_RENDER_ICONS, OnWetbRenderIcons)
    ON_COMMAND(ID_WETB_LIGHT_LEVEL, OnWetbLightLevel)
    ON_COMMAND(ID_WETB_RENDER_SPEC, OnWetbRenderPreview)
    ON_COMMAND(ID_WETB_PORTAL_WALK, OnWetbPortalWalk)
    ON_COMMAND(ID_WETB_SOUND_DEBUG_STATS, OnWetbSoundDebugStats )    
    ON_COMMAND(ID_WETB_SOUND_PRINT_PACKAGE_DATA, OnWetbSoundPrintPackage )
    ON_COMMAND(ID_WETB_SOUND_PROPAGATE, OnWetbSoundPropagateLevel)
    ON_COMMAND(ID_WETB_REFRESH_RSC, OnWetbRefreshRsc)
    ON_COMMAND(ID_WETB_CAMERA_HISTORY_PREV, OnWetbCameraHistoryPrev)
    ON_COMMAND(ID_WETB_CAMERA_HISTORY_NEXT, OnWetbCameraHistoryNext)
    ON_COMMAND(ID_WETB_CAMERA_FAVORITES, OnWetbCameraFavorites)
    ON_COMMAND(ID_WETB_GOTO_PLAYER, OnWetbGotoPlayer)
    ON_UPDATE_COMMAND_UI(ID_WETB_PORTAL_WALK, OnUpdateWetbPortalWalk)
    ON_UPDATE_COMMAND_UI(ID_WETB_SOUND_DEBUG_STATS, OnUpdateWetbSoundDebugStats)
    ON_UPDATE_COMMAND_UI(ID_WETB_LIGHT_LEVEL, OnUpdateWetbLightLevel)
    ON_UPDATE_COMMAND_UI(ID_WETB_SOUND_PROPAGATE, OnUpdateWetbSoundPropagateLevel)
    ON_UPDATE_COMMAND_UI(ID_WETB_FOCUS_CAMERA, OnUpdateWetbFocusCam)
    ON_UPDATE_COMMAND_UI(ID_WETB_RENDER_ICONS, OnUpdateWetbRenderIcons)
    ON_UPDATE_COMMAND_UI(ID_WETB_UNDO, OnUpdateWetbUndo)
    ON_UPDATE_COMMAND_UI(ID_WETB_REDO, OnUpdateWetbRedo)
    ON_UPDATE_COMMAND_UI(ID_WETB_TRANS_STACK, OnUpdateWetbTransStack)
	ON_UPDATE_COMMAND_UI(ID_WETB_SHOWAXIS, OnUpdateWetbShowaxis)
	ON_UPDATE_COMMAND_UI(ID_WETB_SHOWGRID, OnUpdateWetbShowgrid)
	ON_UPDATE_COMMAND_UI(ID_WETB_SHOWSPACD, OnUpdateWetbShowspacd)
	ON_UPDATE_COMMAND_UI(ID_WETB_CREATE_OBJECT, OnUpdateWetbCreateObject)
	ON_UPDATE_COMMAND_UI(ID_WETB_DELETE_SELECTED, OnUpdateWetbDeleteSelected)
    ON_UPDATE_COMMAND_UI(ID_WETB_SHOWSELECTIONBOUNDS, OnUpdateShowSelectionBounds)
	ON_UPDATE_COMMAND_UI(ID_WETB_SNAPTOGRID, OnUpdateWetbSnaptogrid)
    ON_UPDATE_COMMAND_UI(ID_WETB_ADDWITHRAY, OnUpdateWetbSnapAddWithRay)
    ON_UPDATE_COMMAND_UI(ID_WETB_GRIDTOOBJECT, OnUpdateWetbGridToObject)
	ON_UPDATE_COMMAND_UI(ID_WETB_MOVEMODE, OnUpdateWetbMoveMode)
    ON_UPDATE_COMMAND_UI(ID_WETB_COPY_OBJECTS, OnUpdateWetbCopyObjects)
    ON_UPDATE_COMMAND_UI(ID_WETB_MOVE_OBJECTS_TO_ACTIVE_LAYER, OnUpdateWetbMoveObjectsToActiveLayer)
    ON_UPDATE_COMMAND_UI(ID_WETB_RUNGAME, OnUpdateWetbRunGame) 
    ON_UPDATE_COMMAND_UI(ID_WETB_PAUSEGAME, OnUpdateWetbPauseGame) 
    ON_UPDATE_COMMAND_UI(ID_WETB_STEPGAME, OnUpdateWetbStepGame) 
    ON_UPDATE_COMMAND_UI(ID_WETB_SAVEGAME, OnUpdateWetbSaveGame)  
    ON_UPDATE_COMMAND_UI(ID_WETB_LOADGAME, OnUpdateWetbLoadGame) 
    ON_UPDATE_COMMAND_UI(ID_WETB_TOGGLE_FPV, OnUpdateWetbToggleFPV)
    ON_UPDATE_COMMAND_UI(ID_WETB_RENDER_SPEC, OnUpdateWetbRenderPreview)
    ON_UPDATE_COMMAND_UI(ID_WETB_REFRESH_RSC, OnUpdateWetbRefreshRsc)
    ON_UPDATE_COMMAND_UI(ID_WETB_CAMERA_HISTORY_PREV, OnUpdateWetbCameraHistoryPrev)
    ON_UPDATE_COMMAND_UI(ID_WETB_CAMERA_HISTORY_NEXT, OnUpdateWetbCameraHistoryNext)
    ON_UPDATE_COMMAND_UI(ID_WETB_CAMERA_FAVORITES, OnUpdateWetbCameraFavorites)
    ON_UPDATE_COMMAND_UI(ID_BPTB_CREATE_BLUEPRINT, OnUpdateBptbCreateBlueprint)
    ON_UPDATE_COMMAND_UI(ID_BPTB_CREATE_ANCHOR, OnUpdateBptbCreateAnchor)
    ON_UPDATE_COMMAND_UI(ID_BPTB_ADD_BLUEPRINT_AS_OBJECTS, OnUpdateBptbAddBlueprintAsObjects)
    ON_UPDATE_COMMAND_UI(ID_BPTB_ADD_BLUEPRINT, OnUpdateBptbAddBlueprint)
    ON_UPDATE_COMMAND_UI(ID_BPTB_SHATTER_BLUEPRINT, OnUpdateBptbShatterBlueprint)
    ON_UPDATE_COMMAND_UI(ID_BPTB_SHATTER_FOR_EDIT, OnUpdateBptbShatterBlueprintForEdit)   
    ON_UPDATE_COMMAND_UI(ID_BPTB_REFRESH, OnUpdateBptbRefresh)       
	ON_UPDATE_COMMAND_UI(ID_OVTB_ADD_PLAYSURFACE, OnUpdateOvtbAddPlaysurface)
	ON_UPDATE_COMMAND_UI(ID_OVTB_ADD_PROPSURFACE, OnUpdateOvtbAddPropsurface)
	ON_UPDATE_COMMAND_UI(ID_OVTB_ADD_ANIMSURFACE, OnUpdateOvtbAddAnimsurface)
	ON_UPDATE_COMMAND_UI(ID_OVTB_REFRESH, OnUpdateOvtbRefresh)
    ON_UPDATE_COMMAND_UI(ID_OVTB_UPDATE_GEOMS_FROM_SEL, OnUpdateOvtbUpdateGeomsFromSel)
    ON_UPDATE_COMMAND_UI(ID_OVTB_UPDATE_RIGIDINST_FROM_SEL, OnUpdateOvtbUpdateRigidInst)
    ON_UPDATE_COMMAND_UI(ID_DVTB_REFRESH, OnUpdateDvtbRefresh)
    ON_UPDATE_COMMAND_UI(ID_DVTB_PAINT_MODE, OnUpdateDvtbPaintMode)
	ON_WM_CLOSE()
	ON_WM_MDIACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CREATE_PLAYER, OnUpdateAIButtonCreatePlayer)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CREATE_NAV_NODE, OnUpdateAIButtonCreateNavNode)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CONNECT_VISIBLE, OnUpdateButtonConnectVisible)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CONNECT_SELECTED, OnUpdateButtonConnectSelected)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CONNECT_MODE, OnUpdateButtonConnectMode)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CHECK_ALL_NODES, OnUpdateButtonCheckAllNodes)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CHAIN_NODES, OnUpdateButtonChainNodes)
    ON_MESSAGE(CBRN_XT_DROPDOWN, OnToolBarDropDown)
    ON_COMMAND(ID_CFM_ADD, OnCameraFavoriteAdd)
    ON_COMMAND_RANGE(ID_CFM_FAVORITE_1,ID_CFM_FAVORITE_10, OnCameraFavorite)
    ON_COMMAND(ID_CFM_MORE, OnCameraFavoriteMore)

    ON_UPDATE_COMMAND_UI(ID_SELECTION_SELECTALL, OnUpdateSelectionSelectAll)
    ON_COMMAND(ID_SELECTION_SELECTALL, OnSelectionSelectAll)

    ON_UPDATE_COMMAND_UI(ID_DEBUG_INVULNERABLE, OnUpdateDebugInvulnerable)
    ON_UPDATE_COMMAND_UI(ID_DEBUG_INFINITEAMMO, OnUpdateDebugInfiniteAmmo)
    ON_COMMAND(ID_DEBUG_INVULNERABLE, OnDebugInvulnerable)
    ON_COMMAND(ID_DEBUG_INFINITEAMMO, OnDebugInfiniteAmmo)

	//}}AFX_MSG_MAP
    ON_COMMAND_RANGE(IDR_LEVEL_LIGHT_DIR, IDR_LEVEL_LIGHT_RAYCAST, OnWetbLightLevelType)
END_MESSAGE_MAP()

//=========================================================================
// CEditorFrame construction/destruction
//=========================================================================


CEditorFrame::CEditorFrame() :
m_pWorldEditView(NULL),
m_pPaletteDoc(NULL),
m_pPropertyEditorDoc(NULL),
m_pSettingsEditorDoc(NULL),
m_hPreviousMenu(0)

{
    for( object_desc* pNode = object_desc::GetFirstType(); 
                      pNode; 
                      pNode = object_desc::GetNextType( pNode )  )
    {
        if( pNode->UseInEditor() )
        {
            CString strData;
            strData.Format("%s\\%s",pNode->GetEditorGroupName(),pNode->GetTypeName());

            BOOL bAdded = FALSE;
            for (int i = 0; i < m_lstGenericCreateMenuItems.GetCount(); i++)
            {
                POSITION pos = m_lstGenericCreateMenuItems.FindIndex(i);
                CString strComp = m_lstGenericCreateMenuItems.GetAt(pos);
                if (strComp.CompareNoCase(strData) > 0)
                {
                    bAdded = TRUE;
                    m_lstGenericCreateMenuItems.InsertBefore(pos, strData);
                    m_lstGenericCreateNodeData.InsertBefore(m_lstGenericCreateNodeData.FindIndex(i), pNode);
                    break;
                }
            }
            
            if (!bAdded)
            {
                m_lstGenericCreateMenuItems.AddTail(strData);
                m_lstGenericCreateNodeData.AddTail(pNode);
            }
        }
    }
}

//=========================================================================

CEditorFrame::~CEditorFrame()
{
}

//=========================================================================

void CEditorFrame::SetProject(CString strProject) 
{ 
    m_strCurrentLevel = strProject;
    if (!m_strCurrentLevel.IsEmpty())
    {
        SetWindowText( m_strCurrentLevel );	
    }
    else
    {
        SetWindowText("WorldEditor");
    }
}

//=========================================================================

BOOL CEditorFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	if( !CBaseFrame::PreCreateWindow(cs) )
		return FALSE;

	cs.style |= WS_CLIPCHILDREN;
	
	cs.style = WS_CHILD | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
		| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	return TRUE;
}

//=========================================================================

void CEditorFrame::ActivateFrame(int nCmdShow)
{
    nCmdShow = SW_SHOWMAXIMIZED;
    TRACE("Activate Main World Editor\n");
    
    RefreshAll();

    if (GetPropertyEditorDoc())
    {
        GetPropertyEditorDoc()->SetInterface(g_WorldEditor);
    }
    if (GetEditorDoc())
    {
        GetEditorDoc()->InitSettingsInterface();
    }

	CBaseFrame::ActivateFrame(nCmdShow);
}

//=========================================================================
// CEditorFrame diagnostics
//=========================================================================

#ifdef _DEBUG
void CEditorFrame::AssertValid() const
{
	CBaseFrame::AssertValid();
}

//=========================================================================

void CEditorFrame::Dump(CDumpContext& dc) const
{
	CBaseFrame::Dump(dc);
}

#endif //_DEBUG

//=========================================================================
// CEditorFrame message handlers
//=========================================================================

//=========================================================================

CWnd* CEditorFrame::FindViewFromTab( CXTTabCtrlBar& Bar, CRuntimeClass *pViewClass )
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

//=========================================================================

int CEditorFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBaseFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

    // Create the ToolBar
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT, 
        WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBar.LoadToolBar(IDR_WORLDEDITOR))
    {
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
    }
	// Turn on toolbar customization.
	//m_wndToolBar.SetCustomBar(TRUE);

    // Set DropDowns on ToolBar
    m_wndToolBar.AddDropDownButton( ID_WETB_CAMERA_FAVORITES );

	if (!m_wndComboBox.Create( WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|CBS_SORT|WS_CLIPCHILDREN,
		CRect(0,0,200,150), &m_wndToolBar, IDC_COMBOBOX_ACTIVE_LAYER ))
	{
		TRACE0("Failed to create flat toolbar.\n");
		return -1;      // fail to create
	}
    m_wndToolBar.InsertControl(&m_wndComboBox);
	// Autosize the toolbar.
	m_wndToolBar.AutoSizeToolbar();

	// Create the property bar.
	if( !m_wndProperty.Create(this, IDW_PROPERTY_BAR, _T("Property"),
		CSize(350, 150), CBRS_LEFT, CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_CLIENT_STATIC)) //|(AFX_IDW_TOOLBAR + 6) ))
	{
		TRACE0("Failed to create property dock window\n");
		return -1;		// fail to create
	}

	// Create the workspace bar.
	if( !m_wndWrkspBar.Create(this, IDW_WORKSPBAR, _T("Workspace"),
		CSize(350, 150), CBRS_LEFT, CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_CLIENT_STATIC)) //|(AFX_IDW_TOOLBAR + 7) ))
	{
		TRACE0("Failed to create workspace dock window\n");
		return -1;		// fail to create
	}
    m_wndWrkspBar.EnableToolTips( TRUE );

	m_wndToolBar.EnableDockingEx(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM, CBRS_XT_ALL_FLAT);
	m_wndWrkspBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	m_wndProperty.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);

    m_wndWrkspBar.ModifyXTBarStyle(CBRS_XT_CLOSEBTN, 0);
    m_wndProperty.ModifyXTBarStyle(CBRS_XT_CLOSEBTN, 0);

	DockControlBar( &m_wndToolBar,AFX_IDW_DOCKBAR_TOP );
	DockControlBar(&m_wndWrkspBar,AFX_IDW_DOCKBAR_LEFT);
	DockControlBar(&m_wndProperty,AFX_IDW_DOCKBAR_LEFT);
	DockControlBarLeftOf(&m_wndWrkspBar, &m_wndProperty);

	// TODO: Add your own view and documents to the workspace window.
    m_pPaletteDoc = new CEditorPaletteDoc();
    if (m_pPaletteDoc) 
    {
        m_pPaletteDoc->SetTabParent(&m_wndWrkspBar);
        m_pPaletteDoc->SetFramePointer(this);

// MUCH OLDER
//        VERIFY(m_wndWrkspBar.AddView(_T(""), RUNTIME_CLASS(CEditorSoundView), m_pPaletteDoc ));
//        VERIFY(m_wndWrkspBar.AddView(_T(""), RUNTIME_CLASS(CEditorGlobalView), m_pPaletteDoc ));

// OLD STLYE
//    	VERIFY(m_wndWrkspBar.AddView(_T(""), RUNTIME_CLASS(CEditorLayerView), m_pPaletteDoc));
//    	VERIFY(m_wndWrkspBar.AddView(_T(""), RUNTIME_CLASS(CEditorObjectView), m_pPaletteDoc));
//        VERIFY(m_wndWrkspBar.AddView(_T(""), RUNTIME_CLASS(CEditorBlueprintView), m_pPaletteDoc ));
//        VERIFY(m_wndWrkspBar.AddView(_T(""), RUNTIME_CLASS(CEditorAIView), m_pPaletteDoc ));
//        VERIFY(m_wndWrkspBar.AddView(_T(""), RUNTIME_CLASS(CEditorSettingsView), m_pPaletteDoc ));

        CFrameWnd* pFrameWnd = NULL;
        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CEditorLayerView), m_pPaletteDoc);
//        m_wndWrkspBar.AddControl(_T("Layer View"), pFrameWnd);
        m_wndWrkspBar.AddControl(_T(""), pFrameWnd);

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CEditorObjectView), m_pPaletteDoc);
//        m_wndWrkspBar.AddControl(_T("Geom Browser"), pFrameWnd);
        m_wndWrkspBar.AddControl(_T(""), pFrameWnd);

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CEditorBlueprintView), m_pPaletteDoc);
//        m_wndWrkspBar.AddControl(_T("Blueprint Browser"), pFrameWnd);
        m_wndWrkspBar.AddControl(_T(""), pFrameWnd);

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CEditorDecalView), m_pPaletteDoc);
//        m_wndWrkspBar.AddControl(_T("Decals"), pFrameWnd);
        m_wndWrkspBar.AddControl(_T(""), pFrameWnd);

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CEditorSettingsView), m_pPaletteDoc);
//        m_wndWrkspBar.AddControl(_T("Managers"), pFrameWnd);
        m_wndWrkspBar.AddControl(_T(""), pFrameWnd);

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CEditorGlobalView), m_pPaletteDoc);
//        m_wndWrkspBar.AddControl(_T("Globals"), pFrameWnd);
        m_wndWrkspBar.AddControl(_T(""), pFrameWnd);

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CEditorDebuggerView), m_pPaletteDoc);
//        m_wndWrkspBar.AddControl(_T("Debugger"), pFrameWnd);
        m_wndWrkspBar.AddControl(_T(""), pFrameWnd);

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CEditorWatchView), m_pPaletteDoc);
//        m_wndWrkspBar.AddControl(_T("Watch"), pFrameWnd);
        m_wndWrkspBar.AddControl(_T(""), pFrameWnd);

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
            RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CEditorTriggerView), m_pPaletteDoc);
//        m_wndWrkspBar.AddControl(_T("Triggers"), pFrameWnd);
        m_wndWrkspBar.AddControl(_T(""), pFrameWnd);

        CXTTabCtrl& TabCtrl = m_wndWrkspBar.GetTabCtrl();
        for( s32 i=0 ; i<TabCtrl.GetItemCount() ; i++ )
        {
            TabCtrl.SetTabText( i, "" );
        }

        //layers is the default view
        m_wndWrkspBar.SetCaption("Workspace::Layers");
    }
    
	// Create the image list used with the tab control bar.
	if (!m_imageList.Create(IDB_IMGLIST_TAB, 16, 1, RGB(0x00,0xff,0x00)))
	{
		TRACE0("Failed to create image list.\n");
		return -1;
	}
	
	// Associate the image list with the tab control bar.
    m_wndWrkspBar.SetTabImageList(&m_imageList);

    m_wndWrkspBar.GetTabCtrl().SetMinTabWidth( 16 );
    m_wndWrkspBar.GetTabCtrl().SetPadding( CSize(2,4) );
    m_wndWrkspBar.SetActiveView(0);

    m_pPropertyEditorDoc = new CPropertyEditorDoc();
    m_pSettingsEditorDoc = new CPropertyEditorDoc();

// OLD WAY
//	m_wndProperty.AddView(_T("Settings"), RUNTIME_CLASS(CPropertyEditorView), m_pSettingsEditorDoc );
//	m_wndProperty.AddView(_T("Objects"), RUNTIME_CLASS(CPropertyEditorView), m_pPropertyEditorDoc );


    CFrameWnd* pFrameWnd = NULL;
    pFrameWnd = m_wndProperty.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CPropertyEditorView), m_pSettingsEditorDoc);
    m_wndProperty.AddControl(_T("Settings"), pFrameWnd);
    m_pSettingsEditorDoc->GetView()->LoadColumnState( "BarState - World Settings" );

    pFrameWnd = m_wndProperty.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CPropertyEditorView), m_pPropertyEditorDoc);
    m_wndProperty.AddControl(_T("Objects"), pFrameWnd);
    m_pPropertyEditorDoc->GetView()->LoadColumnState( "BarState - World Properties" );

    if (GetPropertyEditorDoc()) GetPropertyEditorDoc()->SetCommandHandler(this);
    if (GetSettingsEditorDoc()) GetSettingsEditorDoc()->SetCommandHandler(this);

    SetWindowText("WorldEditor");

    // Load control bar postion.
    LoadBarState(_T("BarState - World"));

	return 0;
}

//=========================================================================

void CEditorFrame::OnDestroy( )
{
    // Save control bar postion.
    SaveBarState(_T("BarState - World"));
    m_pSettingsEditorDoc->GetView()->SaveColumnState( "BarState - World Settings" );
    m_pPropertyEditorDoc->GetView()->SaveColumnState( "BarState - World Properties" );
}

//=========================================================================

void CEditorFrame::CreateTemporaryObject( const char* pObjectType )
{
    if (m_pWorldEditView && GetEditorDoc())
    {
	    g_WorldEditor.CreateTemporaryObject( pObjectType );
        m_pWorldEditView->EnterPlacementMode();
        if (GetPropertyEditorDoc()) GetPropertyEditorDoc()->Refresh();

        m_pWorldEditView->Invalidate();
    }


}

//=========================================================================

void CEditorFrame::OnSelchangeActiveLayerCombo()
{
    CString strActiveLayer;
    m_wndComboBox.GetWindowText(strActiveLayer);

    g_WorldEditor.SetActiveLayer(strActiveLayer, "\\");
    TRACE(xfs("Active Layer Switch= %s\n", (const char*)strActiveLayer));
}

//=========================================================================

void CEditorFrame::SetActiveLayer(CString strLayer)
{
    int iIndex = m_wndComboBox.SelectString( -1, strLayer);
    ASSERT(iIndex != CB_ERR);
}

//=========================================================================

void CEditorFrame::OnLoadLayers()
{
    m_wndComboBox.ResetContent();

    xarray<xstring> List;
    g_WorldEditor.GetLayerNames(List);
    for (int i = 0; i < List.GetCount(); i++)
	{
        if (g_WorldEditor.IsLayerLoaded(List.GetAt(i)))
        {
            m_wndComboBox.AddString(List.GetAt(i));
        }
    }

    if (List.GetCount() <= 0)
    {
        m_wndComboBox.AddString(g_WorldEditor.GetDefaultLayer( ));
        m_wndComboBox.AddString(g_WorldEditor.GetGlobalLayer( ));
    }

    //select active layer
    CString strText = g_WorldEditor.GetActiveLayer();
    strText += g_WorldEditor.GetActiveLayerPath();
    strText = strText.Left(strText.GetLength()-1);
    if (m_wndComboBox.SelectString(-1,strText)==CB_ERR)
    {
        //going to need to add this one
        m_wndComboBox.AddString(strText);
        VERIFY(m_wndComboBox.SelectString(-1,strText)!=CB_ERR);
    }
}

//=========================================================================

LRESULT CEditorFrame::OnUpdateRequired(WPARAM wParam, LPARAM lParam)
{
    if (m_pWorldEditView) 
    {
        m_pWorldEditView->SetViewDirty();
 	    return 1;
    }
    return 0;
}

//=========================================================================

LRESULT CEditorFrame::OnGuidSelectorRequest(WPARAM wParam, LPARAM lParam)
{
    g_WorldEditor.SetGuidSelectMode((const char*)wParam, TRUE);
    return 1;
}

//=========================================================================

LRESULT CEditorFrame::OnGuidHighlightRequest(WPARAM wParam, LPARAM lParam)
{
    if (m_pWorldEditView) 
    {
        guid GuidVal = 0;
        if (wParam)
        {
            GuidVal = guid_FromString((const char*)wParam);
        }
        m_pWorldEditView->SetGuidSelHightLight(GuidVal);
        m_pWorldEditView->SetViewDirty();
    }

    return 1;
}

//=========================================================================

CPropertyEditorDoc* CEditorFrame::GetPropertyEditorDoc()
{
    return m_pPropertyEditorDoc;
}

//=========================================================================

CPropertyEditorDoc* CEditorFrame::GetSettingsEditorDoc()
{
    return m_pSettingsEditorDoc;
}

//=========================================================================

CEditorDoc* CEditorFrame::GetEditorDoc()
{
    CEditorDoc* pDoc = NULL;
    if (m_pWorldEditView) 
    {
        pDoc = m_pWorldEditView->GetDocument();
    }
    return pDoc;
}

//=========================================================================

void CEditorFrame::RefreshAll()
{
    xtimer RefreshTimer;
    RefreshTimer.Start();  

    OnLoadLayers();
    CEditorLayerView* pView = (CEditorLayerView*)FindViewFromTab( m_wndWrkspBar, RUNTIME_CLASS(CEditorLayerView));
    if (pView)
    {
        pView->LoadLayers();
    }

    CEditorBlueprintView* pBPView = (CEditorBlueprintView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorBlueprintView));
    if (pBPView)
    {
        pBPView->BuildTreeFromProject();
    } 

    CEditorObjectView* pObjView = (CEditorObjectView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorObjectView));
    if (pObjView)
    {
        pObjView->LoadList();
    } 

    CEditorGlobalView* pGlobalView = (CEditorGlobalView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorGlobalView));
    if (pGlobalView)
    {
        pGlobalView->RefreshView();
    }  
    
    CEditorWatchView* pWatchView = (CEditorWatchView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorWatchView));
    if (pWatchView)
    {
        pWatchView->RefreshView();
    }  
    
    CEditorTriggerView* pTriggerView = (CEditorTriggerView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorTriggerView));
    if (pTriggerView)
    {
        pTriggerView->RefreshView();
    } 

    GetPropertyEditorDoc()->Refresh();
    GetSettingsEditorDoc()->Refresh();

    SetProject(g_Project.GetName());
    GetEditorView()->SetViewDirty();

    x_DebugMsg("--TIMECHECK-- EditorFrame::RefreshAll took %g sec\n",RefreshTimer.TripSec());
}

//=========================================================================

void CEditorFrame::RefreshWatchWindowIfActive()
{
    CEditorWatchView* pWatchView = (CEditorWatchView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorWatchView));
    if (pWatchView && pWatchView->IsActive())
    {
        pWatchView->RefreshView();
    }  
}

//=========================================================================
// Toolbar handlers
//=========================================================================

void CEditorFrame::OnWetbRenderPreview()
{
    m_pWorldEditView->m_bRenderPreview = !m_pWorldEditView->m_bRenderPreview;
    m_pWorldEditView->SetViewDirty();
    m_pWorldEditView->SetFocus();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbRenderPreview(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && g_Project.IsProjectOpen())
    {
        pCmdUI->Enable(TRUE);	
        pCmdUI->SetCheck(m_pWorldEditView->m_bRenderPreview);
    }
    else 
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbLightLevel()
{
	CXTMenu menu;
    menu.CreatePopupMenu();

    menu.AppendMenu(MF_STRING|MF_ENABLED, IDR_LEVEL_LIGHT_NORMAL,   "Light::Distance");
    menu.AppendMenu(MF_STRING|MF_ENABLED, IDR_LEVEL_LIGHT_DYNAMIC,  "Light::Character Lighting Only");
    menu.AppendMenu(MF_STRING|MF_ENABLED, IDR_LEVEL_LIGHT_ZONE,     "Light::Debug Zone Lighting");
    menu.AppendMenu(MF_STRING|MF_ENABLED, IDR_LEVEL_LIGHT_DIR,      "Light::Directional");
    menu.AppendMenu(MF_STRING|MF_ENABLED, IDR_LEVEL_LIGHT_WHITE,    "Light::FullBright");
    menu.AppendMenu(MF_STRING|MF_ENABLED, IDR_LEVEL_LIGHT_RAYCAST,  "Light::Raycast");

	CPoint point;
	GetCursorPos( &point );
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x, point.y, this);
}

//=========================================================================

void CEditorFrame::OnWetbLightLevelType(UINT nId)
{
    CWaitCursor wc;

    switch(nId)
    {
    case IDR_LEVEL_LIGHT_DIR:
        g_WorldEditor.ComputeLightLayer("", 1);
        break;
    case IDR_LEVEL_LIGHT_NORMAL:
        g_WorldEditor.ComputeLightLayer("", 2);
        break;
    case IDR_LEVEL_LIGHT_DYNAMIC:
        g_WorldEditor.ComputeLightLayer("", 3);
        break;
    case IDR_LEVEL_LIGHT_WHITE:
        g_WorldEditor.ComputeLightLayer("", 4);
        break;
    case IDR_LEVEL_LIGHT_RAYCAST:
        if (::AfxMessageBox("Are you sure you want to Raycast?  This operation could take a significant amount of time.",MB_YESNO) == IDYES)
            g_WorldEditor.ComputeLightLayer("", 5);
        break;
    case IDR_LEVEL_LIGHT_ZONE:
        g_WorldEditor.ComputeLightLayer("", 6);
        break;
    }

    GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbLightLevel(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && g_Project.IsProjectOpen() && (!GetEditorDoc()->IsGameRunning()))
    {
        pCmdUI->Enable(TRUE);	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorFrame::OnUpdateWetbSoundPropagateLevel(CCmdUI* pCmdUI)
{
    if (m_pWorldEditView && g_Project.IsProjectOpen() && (!GetEditorDoc()->IsGameRunning()))
    {
        pCmdUI->Enable(TRUE);	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorFrame::OnWetbTransStack() 
{
    CTransactionStackDlg dlg;
    if (dlg.DoModal() == IDOK)
    {
        //must clear selections
        g_WorldEditor.ClearSelectedObjectList();

        RefreshAll();
    }
}

//=========================================================================

void CEditorFrame::OnUpdateWetbTransStack(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc() && !GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable((transaction_mgr::Transaction()->CanUndo() || 
                        transaction_mgr::Transaction()->CanRedo()));	
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnWetbUndo() 
{
    if (GetEditorView()->IsMovementMode())
    {
        GetEditorView()->CancelMovementMode();
    }
    
    ASSERT(!g_WorldEditor.InTransaction());
    int nUndone = transaction_mgr::Transaction()->Undo(1);
    ASSERT(nUndone == 1);

    RefreshAll();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbUndo(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc() && !GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable(transaction_mgr::Transaction()->CanUndo());
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnWetbRedo() 
{
    if (GetEditorView()->IsMovementMode())
    {
        GetEditorView()->CancelMovementMode();
    }

    ASSERT(!g_WorldEditor.InTransaction());
    int nRedone = transaction_mgr::Transaction()->Redo(1);
    ASSERT(nRedone == 1);

    RefreshAll();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbRedo(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc() && !GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable(transaction_mgr::Transaction()->CanRedo() && 
            (!GetEditorView()->IsMovementMode())); //not allowed while moving
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }

    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnUpdateWetbFocusCam(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen());	
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnWetbFocusCam() 
{
    m_pWorldEditView->FocusCamera();
}

//=========================================================================

void CEditorFrame::OnWetbShowgrid() 
{
    if (m_pWorldEditView)
    {
        m_pWorldEditView->m_bShowGrid = !m_pWorldEditView->m_bShowGrid;
        m_pWorldEditView->Invalidate();
        m_pWorldEditView->UpdateWindow();
    }
    if (m_pWorldEditView) m_pWorldEditView->SetFocus(); //set focus to view
}

//=========================================================================

void CEditorFrame::OnUpdateWetbShowgrid(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && g_Project.IsProjectOpen() )
    {
        pCmdUI->Enable(TRUE);	
        pCmdUI->SetCheck(m_pWorldEditView->m_bShowGrid);	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbShowspacd() 
{
    if (m_pWorldEditView)
    {
        m_pWorldEditView->m_bShowSpacD = !m_pWorldEditView->m_bShowSpacD;
        m_pWorldEditView->Invalidate();
        m_pWorldEditView->UpdateWindow();
    }
    if (m_pWorldEditView) m_pWorldEditView->SetFocus(); //set focus to view
}

//=========================================================================

void CEditorFrame::OnUpdateWetbShowspacd(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && g_Project.IsProjectOpen() )
    {
        pCmdUI->Enable(TRUE);	
        pCmdUI->SetCheck(m_pWorldEditView->m_bShowSpacD);	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbShowaxis() 
{
    if (m_pWorldEditView)
    {
	    m_pWorldEditView->m_bShowAxis = !m_pWorldEditView->m_bShowAxis;
        m_pWorldEditView->Invalidate();
        m_pWorldEditView->UpdateWindow();
    }
    if (m_pWorldEditView) m_pWorldEditView->SetFocus(); //set focus to view
}

//=========================================================================

void CEditorFrame::OnUpdateWetbShowaxis(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && g_Project.IsProjectOpen())
    {
        pCmdUI->Enable(TRUE);	
        pCmdUI->SetCheck(m_pWorldEditView->m_bShowAxis);	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnShowSelectionBounds() 
{
    if (m_pWorldEditView)
    {
	    m_pWorldEditView->m_bShowSelectionBounds = !m_pWorldEditView->m_bShowSelectionBounds;
        m_pWorldEditView->Invalidate();
        m_pWorldEditView->UpdateWindow();
    }
    if (m_pWorldEditView) m_pWorldEditView->SetFocus(); //set focus to view
}

//=========================================================================

void CEditorFrame::OnUpdateShowSelectionBounds(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && g_Project.IsProjectOpen())
    {
        pCmdUI->Enable(TRUE);	
        pCmdUI->SetCheck(m_pWorldEditView->m_bShowSelectionBounds);	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbSnaptogrid() 
{
    if (m_pWorldEditView)
    {
	    m_pWorldEditView->m_bGridSnap = !m_pWorldEditView->m_bGridSnap;
        m_pWorldEditView->Invalidate();
        m_pWorldEditView->UpdateWindow();
    }
    if (m_pWorldEditView) m_pWorldEditView->SetFocus(); //set focus to view
}

//=========================================================================

void CEditorFrame::OnUpdateWetbSnaptogrid(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && g_Project.IsProjectOpen())
    {
        pCmdUI->Enable(TRUE);	
        pCmdUI->SetCheck(m_pWorldEditView->m_bGridSnap);	
    }
    else     
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbSnapAddWithRay()
{
    if (m_pWorldEditView)
    {
	    m_pWorldEditView->m_bAddWithRaySnap = !m_pWorldEditView->m_bAddWithRaySnap;
        m_pWorldEditView->Invalidate();
        m_pWorldEditView->UpdateWindow();
    }
    if (m_pWorldEditView) m_pWorldEditView->SetFocus(); //set focus to view
}

//=========================================================================

void CEditorFrame::OnUpdateWetbSnapAddWithRay(CCmdUI* pCmdUI)
{
    if (m_pWorldEditView && g_Project.IsProjectOpen())
    {
        pCmdUI->Enable(TRUE);	
        pCmdUI->SetCheck(m_pWorldEditView->m_bAddWithRaySnap);	
    }
    else     
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbMoveMode() 
{
    if (m_pWorldEditView)
    {
        if (m_pWorldEditView->IsMovementMode())
        {
            g_WorldEditor.CheckSelectedForDuplicates();
	        m_pWorldEditView->CancelMovementMode();
        }
        else
        {
            //setup generic undo entry for moving of objects
            g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(
                xfs("Moving %d Object(s)",g_WorldEditor.GetSelectedCount())));

	        m_pWorldEditView->EnterMovementMode();
        }
    }
    if (m_pWorldEditView) m_pWorldEditView->SetFocus(); //set focus to view
}

//=========================================================================

void CEditorFrame::OnUpdateWetbMoveMode(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && (g_WorldEditor.GetSelectedCount() > 0) &&
        g_Project.IsProjectOpen() && g_WorldEditor.IsSelectedObjectsEditable() && 
        (!GetEditorDoc()->IsGameRunning()))
    {
        pCmdUI->Enable(m_pWorldEditView->IsStandardMode() || m_pWorldEditView->IsMovementMode());	
        pCmdUI->SetCheck(m_pWorldEditView->IsMovementMode());	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbCopyObjects() 
{
    if (m_pWorldEditView)
    {
        xarray<guid> lstItems;
        xarray<guid> lstBPRefs;
       
        //setup generic undo entry for copying of objects
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(
            xfs("Copying %d Object(s)",g_WorldEditor.GetSelectedCount())));

        //now do the copy
        g_WorldEditor.CopySelectedObjects(lstItems, lstBPRefs);
        //Add to layers

        for (int i=0; i<lstItems.GetCount(); i++)
        {
            guid& ObjGuid = lstItems.GetAt(i);
            AddObjectToActiveLayerView(ObjGuid);
        }
        for (i=0; i<lstBPRefs.GetCount(); i++)
        {
            guid& BPGuid = lstBPRefs.GetAt(i);
            AddBlueprintToActiveLayerView(BPGuid);
        }

	    m_pWorldEditView->EnterMovementMode();
        //for UNDO, must be done after entering movement mode, force UNDO commit after moving
        m_pWorldEditView->ForceMovementUndoRecording(); 
        m_pWorldEditView->SetViewDirty();
        m_pWorldEditView->SetFocus(); //set focus to view
    }
}

//=========================================================================

void CEditorFrame::OnUpdateWetbCopyObjects(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && (g_WorldEditor.GetSelectedCount() > 0) &&
        g_Project.IsProjectOpen() &&
        (!GetEditorDoc()->IsGameRunning()))
    {
        pCmdUI->Enable(m_pWorldEditView->IsStandardMode());	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorFrame::OnWetbMoveObjectsToActiveLayer() 
{
    if (m_pWorldEditView)
    {
        CString strMsg;
        strMsg.Format("Are you sure you want to move the %d selected objects into %s?",
            g_WorldEditor.GetSelectedCount(),g_WorldEditor.GetActiveLayer());
        if (::AfxMessageBox(strMsg,MB_YESNO) == IDYES)
        {
            xarray<editor_item_descript> lstItems;
            
            g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("LayerChange (%d Objects) to %s",
                                             g_WorldEditor.GetSelectedCount(), g_WorldEditor.GetActiveLayer())));
            g_WorldEditor.MoveSelectedObjectsToActiveLayer(lstItems);
            g_WorldEditor.CommitCurrentUndoEntry();

            for (int i=0; i<lstItems.GetCount(); i++)
            {
                //the layer is which layer to remove from
                editor_item_descript& ItemInfo = lstItems.GetAt(i);

                if (ItemInfo.IsInBlueprint)
                {
                    //blueprint
                    RemoveBlueprintFromLayerView(CString(ItemInfo.Layer),
                        CString(ItemInfo.LayerPath), ItemInfo.Guid);
                    AddBlueprintToActiveLayerView(ItemInfo.Guid);
                }
                else
                {
                    //objects
                    RemoveObjectFromLayerView(CString(ItemInfo.Layer),
                        CString(ItemInfo.LayerPath), ItemInfo.Guid);
                    AddObjectToActiveLayerView(ItemInfo.Guid);
                }
            }
            
            //make sure no zones picked up selection
            //added to avoid tree view doing a switch on us
            g_WorldEditor.UnSelectZone();
            g_WorldEditor.ClearSelectedObjectList();
            for (i=0; i<lstItems.GetCount(); i++)
            {
                editor_item_descript& ItemInfo = lstItems.GetAt(i);

                //reselect all items
                g_WorldEditor.SelectObject(ItemInfo.Guid,FALSE);
            }
            GetPropertyEditorDoc()->Refresh();
        }

        m_pWorldEditView->SetFocus(); //set focus to view
    }
}

//=========================================================================

void CEditorFrame::OnUpdateWetbMoveObjectsToActiveLayer(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && (g_WorldEditor.GetSelectedCount() > 0) &&
        g_Project.IsProjectOpen() && g_WorldEditor.IsSelectedObjectsEditable() &&
        (!GetEditorDoc()->IsGameRunning()))
    {
        pCmdUI->Enable(m_pWorldEditView->IsStandardMode());	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorFrame::OnWetbRunGame() 
{
    if (GetEditorDoc()->IsGameRunning())
    {      
        GetEditorDoc()->StopGame();
        m_pWorldEditView->SetViewDirty();       
    }
    else
    {
        // Only happen for non-auto-build
        if( g_WorldEditor.IsPerformingAutomatedBuild() == FALSE )
        {
            //add a portal check
            xarray <guid> PortalList;
            g_WorldEditor.GetListOfPortals(PortalList);
            if(PortalList.GetCount() == 0)
            {
                generic_dialog Dialog;
                Dialog.SetTitle( "ERROR! No Portal in Level" );
                Dialog.SetMessage( "Level will not export and will not run correctly without a Portal");
                Dialog.AppendButton( "Edit Level" );
                Dialog.AppendButton( "Ignore" );
                switch( Dialog.Execute() )
                {
                    // Edit Level?
                case 0:
                    {
                        return;
                        break;
                    }
                    // Ignore?
                case 1:
                    break;
                }
            }
            // Check for bad properties
            xarray<property_error> Errors;
            if( g_WorldEditor.ValidateObjectProperties( Errors, "RunGame" ) )
            {
                // Setup dialog
                generic_dialog Dialog;
                Dialog.SetTitle( "OBJECT PROPERTY ERROR! - Level will not export and may not run correctly!" );
                Dialog.AppendButton( "Edit Object" );
                Dialog.AppendButton( "Ignore" );
                Dialog.AppendButton( "Ignore All" );
                Dialog.AppendButton( "Cancel" );

                // Loop through all errors
                s32 bIgnoreAll = FALSE;
                for( s32 i = 0 ; ( i < Errors.GetCount() ) && ( bIgnoreAll == FALSE ) ; i++ )
                {
                    // Lookup error
                    property_error& Error = Errors[i];

                    // Set dialog error message
                    Error.m_ErrorMsg = "\n" + Error.m_ErrorMsg ;
                    Dialog.SetMessage( Error.m_ErrorMsg );
                    
                    // What does user want to do?
                    switch( Dialog.Execute() )
                    {
                        // Edit object?
                        case 0:
                        {
                            // Get object
                            object* pObject = g_ObjMgr.GetObjectByGuid( Error.m_ObjectGuid );
                            if( pObject )
                            {
                                // Is this object part of a blue-print?
                                editor_blueprint_ref* pBlueprintRef = NULL;
                                g_WorldEditor.GetBlueprintRefContainingObject2( pObject->GetGuid(), &pBlueprintRef );
                                if( pBlueprintRef )
                                {
                                    // Select all objects in blue-print
                                    g_WorldEditor.ClearSelectedObjectList();
                                    g_WorldEditor.SelectBlueprintObjects( *pBlueprintRef, TRUE );
                                }                
                                else
                                {
                                    // Select single object
                                    g_WorldEditor.ClearSelectedObjectList();
                                    g_WorldEditor.SelectObject( pObject->GetGuid(), TRUE );
                                }
                                
                                // Select layer
                                CString SelectedLayer;
                                if ( g_WorldEditor.SetCurrentObjectsLayerAsActive() )
                                {
                                    SelectedLayer = CString( "New Active Layer \"" ) + 
                                                    g_WorldEditor.GetActiveLayer() +
                                                    g_WorldEditor.GetActiveLayerPath() + "\"";
                                }

                                // Get view
                                CEditorView* pEditorView = GetEditorView();
                                ASSERT( pEditorView );

                                // Set camera to look at object
                                pEditorView->SetFocusPos( pObject->GetPosition() );
                                pEditorView->FocusCamera();
                                pEditorView->Invalidate( TRUE );
                                pEditorView->UpdateWindow();
                                pEditorView->SetMessage( SelectedLayer );

                                // Update properties
                                GetPropertyEditorDoc()->Refresh();
                            }
                            else
                            {
                                generic_dialog Dialog;
                                Dialog.SetTitle( "Couldn't locate the object!" );
                                xstring Msg;
                                Msg.Format( "GUID \"%08X:%08X\" could not be found", Error.m_ObjectGuid.GetHigh(), Error.m_ObjectGuid.GetLow() );
                                Dialog.SetMessage( (const char*)Msg );
                                Dialog.AppendButton( "OK" );
                                Dialog.Execute();
                            }
                        }
                        return;

                        // Ignore?
                        case 1:
                            break;

                        // Ignore All?
                        case 2:
                            bIgnoreAll = TRUE;
                            break;

                        // Cancel?
                        case 3:
                            return;
                    }
                }
            }
        }

        // Before actually running, we need to determine which player physics
        // to use: the normal campaign physics or the faster multiplayer 
        // version.  This decision is made based on the presence of an 
        // mp_settings object within the level.
        if( g_ObjMgr.GetNumInstances( object::TYPE_MP_SETTINGS ) > 0 )
        {
            g_MPTweaks.Active = TRUE;
        }
        else
        {
            g_MPTweaks.Active = FALSE;
        }

        // Run the game
        GetEditorDoc()->RunGame();
        GetEditorDoc()->PauseGame(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnUpdateWetbRunGame(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc() && GetEditorView()->IsStandardMode())
    {
        pCmdUI->Enable(g_Project.IsProjectOpen());	
        pCmdUI->SetCheck(GetEditorDoc()->IsGameRunning());	
    }
    else
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbRefreshRsc() 
{
    g_WorldEditor.RefreshResources();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbRefreshRsc(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(GetEditorDoc() && !GetEditorDoc()->IsGameRunning() && g_Project.IsProjectOpen());
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorFrame::OnWetbPauseGame() 
{
    if (GetEditorDoc()->IsGameRunning())
    {      
        GetEditorDoc()->PauseGame(!GetEditorDoc()->IsGamePaused());
    }
}

//=========================================================================

void CEditorFrame::OnUpdateWetbPauseGame(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc())
    {
        pCmdUI->Enable(GetEditorDoc()->IsGameRunning());	
        pCmdUI->SetCheck(GetEditorDoc()->IsGamePaused());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
        pCmdUI->SetCheck(FALSE);	
    }
}

//=========================================================================

void CEditorFrame::OnWetbStepGame() 
{
    if ( (GetEditorDoc()->IsGameRunning()) && (GetEditorDoc()->IsGamePaused()) )
    {      
        GetEditorDoc()->StepGame() ;
    }
}

//=========================================================================

void CEditorFrame::OnUpdateWetbStepGame(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(FALSE);	
    if (GetEditorDoc())
    {
        pCmdUI->Enable( (GetEditorDoc()->IsGameRunning()) && (GetEditorDoc()->IsGamePaused()) );
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
}

//=========================================================================

void CEditorFrame::OnWetbSaveGame() 
{
    if (GetEditorDoc()->IsGameRunning())
    {      
        GetEditorDoc()->SaveGame();
    }
}

//=========================================================================

void CEditorFrame::OnUpdateWetbSaveGame(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc())
    {
        pCmdUI->Enable(GetEditorDoc()->IsGameRunning());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnWetbLoadGame() 
{
    if (GetEditorDoc()->IsGameRunning())
    {      
        GetEditorDoc()->LoadGame();
    }
}

//=========================================================================

void CEditorFrame::OnUpdateWetbLoadGame(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc())
    {
        pCmdUI->Enable(GetEditorDoc()->IsGameRunning());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnWetbPortalWalk() 
{
    if (GetEditorDoc()->IsGameRunning())
    {      
        GetEditorView()->DoPortalWalk(!GetEditorView()->m_bDoPortalWalk);
    }
}

//=========================================================================

void CEditorFrame::OnWetbSoundDebugStats() 
{
    if (GetEditorDoc()->IsGameRunning())
    {   
        xarray<xstring> LoadedPackages;
        extern xarray<xstring> g_AuditionPackages;

        g_AudioMgr.GetLoadedPackages( LoadedPackages );
        if( g_AuditionPackages.GetCount() && LoadedPackages.GetCount() )
        {
            for( s32 i=0 ; i<g_AuditionPackages.GetCount() ; i++ )
            {
                s32 j;
                if( (j = LoadedPackages.Find( g_AuditionPackages[i] )) != -1 )
                {
                    LoadedPackages.Delete( j );
                }
            }
        }

        if( LoadedPackages.GetCount() )
        {
            x_DebugMsg( "********** Loaded Audio Packages **********\n" );
            x_DebugMsg( "\n" );
            s32 TotalRamFootprint = 0;
            for( s32 j=0 ; j<LoadedPackages.GetCount() ; j++ )
            {
                X_FILE* pPS2File = NULL;
                char PS2FilePath[256];

                x_try;

                x_sprintf( PS2FilePath, "%s\\PS2\\%s", g_Settings.GetReleasePath(), (const char*)LoadedPackages[j] );

                pPS2File = x_fopen( PS2FilePath, "rb" );

                if( pPS2File == NULL )
                    x_throw( xfs("Unable to open file [%s]", (const char*)LoadedPackages[j]) );

                package_identifier  PackageID;
                package_header      PackageHeader;
                s32                 MRAM = 0;
                s32                 ARAM = 0;
                s32                 i    = 0;

                // Read in the package identifier.
                x_fread( &PackageID, sizeof(package_identifier), 1, pPS2File );

                // Correct version?
                if( !x_strncmp( PackageID.VersionID, PS2_PACKAGE_VERSION, VERSION_ID_SIZE ) )
                {
                    // Correct platform?
                    if( !x_strncmp( PackageID.TargetID, PS2_TARGET_ID, TARGET_ID_SIZE ) )
                    {

                        // Now read in the header.
                        x_fread( &PackageHeader, sizeof(package_header), 1, pPS2File );

                        MRAM    += PackageHeader.StringTableFootprint;
                        MRAM    += PackageHeader.MusicDataFootprint;
                        MRAM    += PackageHeader.LipSyncTableFootprint;
                        MRAM    += PackageHeader.BreakPointTableFootprint;
                        MRAM    += PackageHeader.nIdentifiers * sizeof(descriptor_identifier);
                        MRAM    += PackageHeader.nDescriptors * sizeof(u32*);
                        MRAM    += PackageHeader.DescriptorFootprint;

                        // For each temperature...
                        for( i=0 ; i<NUM_TEMPERATURES ; i++ )
                        {
                            if( PackageHeader.nSampleIndices[ i ] )
                            {
                                // Allocate memory for sample header index table.
                                MRAM += (PackageHeader.nSampleIndices[ i ]+1) * sizeof(u16);
                            }
                        }

                        // Allocate memory for the hot and cold samples
                        if( PackageHeader.nSampleHeaders[ HOT ] )
                        {
                            MRAM +=  PackageHeader.nSampleHeaders[ HOT ] * PackageHeader.HeaderSizes[ HOT ];
                        }

                        if( PackageHeader.nSampleHeaders[ COLD ] )
                        {
                            MRAM += PackageHeader.nSampleHeaders[ COLD ] * PackageHeader.HeaderSizes[ COLD ];
                        }

                        ARAM    += PackageHeader.Aram;
                    }
                    else
                    {
                        x_throw( xfs("Incorrect audio package PLATFORM [%s]", (const char*)LoadedPackages[j]) );
                    }
                }
                else
                {
                    x_throw( xfs("Incorrect audio package VERSION [%s]", (const char*)LoadedPackages[j]) );
                }

                TotalRamFootprint += ARAM;

                x_DebugMsg( "%7d %s\n", ARAM, (const char*)LoadedPackages[j] );
                x_fclose( pPS2File );

                x_catch_begin;

                x_display_exception_msg( xfs("Could not load audiopkg for PS2 memory scan:\n%s",PS2FilePath) );

                if( pPS2File )
                    x_fclose( pPS2File );

                x_catch_end;
            }

            x_DebugMsg( "=======\n" );
            x_DebugMsg( "%7d Total [%dk]\n", TotalRamFootprint, (TotalRamFootprint + 1023) / 1024 );
            x_DebugMsg( "%7d FREE [%dk]\n", (5000*1024 - TotalRamFootprint), (5000*1024 - TotalRamFootprint + 1023) / 1024 );
        }
    }
}

//=========================================================================

void CEditorFrame::OnUpdateWetbSoundDebugStats(CCmdUI* pCmdUI)
{
    if (GetEditorDoc() && GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable(TRUE);
        pCmdUI->SetCheck(FALSE);	
    }
    else
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbSoundPrintPackage() 
{
    //
    // Open all the packages in the PS2 directory and display how much memory that are going to be taking
    // up on the PS2 side.
    //
    _finddata_t     c_file;
    long            hFile = _findfirst( xfs( "%s\\PS2\\*.audiopkg", g_Settings.GetReleasePath() ), &c_file );
    
    if( hFile == -1L )
        return;
    
    s32 TotalRamFootprint = 0;
    do
    {
        X_FILE* pPS2File = NULL;
        char PS2FilePath[256];
            
        x_try;

            x_sprintf( PS2FilePath, "%s\\PS2\\%s", g_Settings.GetReleasePath(), c_file.name );
    
            pPS2File = x_fopen( PS2FilePath, "rb" );
        
            if( pPS2File == NULL )
                x_throw( xfs("Unable to open file [%s]", c_file.name) );

            package_identifier  PackageID;
            package_header      PackageHeader;
            s32                 MRAM = 0;
            s32                 i    = 0;

            // Read in the package identifier.
            x_fread( &PackageID, sizeof(package_identifier), 1, pPS2File );

            // Correct version?
            if( !x_strncmp( PackageID.VersionID, PS2_PACKAGE_VERSION, VERSION_ID_SIZE ) )
            {
                // Correct platform?
                if( !x_strncmp( PackageID.TargetID, PS2_TARGET_ID, TARGET_ID_SIZE ) )
                {

                    // Now read in the header.
                    x_fread( &PackageHeader, sizeof(package_header), 1, pPS2File );
                
                    MRAM    += PackageHeader.StringTableFootprint;
                    MRAM    += PackageHeader.MusicDataFootprint;
                    MRAM    += PackageHeader.LipSyncTableFootprint;
                    MRAM    += PackageHeader.BreakPointTableFootprint;
                    MRAM    += PackageHeader.nIdentifiers * sizeof(descriptor_identifier);
                    MRAM    += PackageHeader.nDescriptors * sizeof(u32*);
                    MRAM    += PackageHeader.DescriptorFootprint;

                    // For each temperature...
                    for( i=0 ; i<NUM_TEMPERATURES ; i++ )
                    {
                        if( PackageHeader.nSampleIndices[ i ] )
                        {
                            // Allocate memory for sample header index table.
                            MRAM += (PackageHeader.nSampleIndices[ i ]+1) * sizeof(u16);
                        }
                    }

                    // Allocate memory for the hot and cold samples
                    if( PackageHeader.nSampleHeaders[ HOT ] )
                    {
                        MRAM +=  PackageHeader.nSampleHeaders[ HOT ] * PackageHeader.HeaderSizes[ HOT ];
                    }

                    if( PackageHeader.nSampleHeaders[ COLD ] )
                    {
                        MRAM += PackageHeader.nSampleHeaders[ COLD ] * PackageHeader.HeaderSizes[ COLD ];
                    }
                
                    MRAM    += PackageHeader.Aram;
                }
                else
                {
                    x_throw( xfs("Incorrect audio package PLATFORM [%s]", c_file.name) );
                }
            }
            else
            {
                x_throw( xfs("Incorrect audio package VERSION [%s]", c_file.name) );
            }

            TotalRamFootprint += MRAM;

            x_DebugMsg( (const char*)xfs( "Audio Package [%s]\t\t\tSize [%dk]\n", c_file.name, (MRAM/1024) ) );

            x_fclose( pPS2File );

        x_catch_begin;
        
        x_display_exception_msg( xfs("Could not load audiopkg for PS2 memory scan:\n%s",PS2FilePath) );

            if( pPS2File )
                x_fclose( pPS2File );

        x_catch_end;
    }
    while( _findnext( hFile, &c_file ) == 0 );

    x_DebugMsg( (const char*)xfs( "Total Audio Package Footprint [%dk]\n", (TotalRamFootprint/1024) ) );

    _findclose( hFile );
}

//=========================================================================

void CEditorFrame::OnWetbSoundPropagateLevel( void )
{
    g_WorldEditor.ComputeSoundPropagation();
}


//=========================================================================

void CEditorFrame::OnUpdateWetbRenderIcons(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc())
    {
        pCmdUI->Enable(TRUE);
        pCmdUI->SetCheck(GetEditorView()->m_bRenderIcons);	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
        pCmdUI->SetCheck(FALSE);	
    }
}


//=========================================================================

void CEditorFrame::OnWetbRenderIcons() 
{
    GetEditorView()->m_bRenderIcons = !GetEditorView()->m_bRenderIcons;
    GetEditorView()->RedrawWindow();
}


//=========================================================================

void CEditorFrame::OnUpdateWetbPortalWalk(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc())
    {
        pCmdUI->Enable(GetEditorDoc()->IsGameRunning());	
        pCmdUI->SetCheck(GetEditorView()->m_bDoPortalWalk);	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
        pCmdUI->SetCheck(FALSE);	
    }
}

//=========================================================================

void CEditorFrame::OnWetbToggleFPV() 
{
    GetEditorDoc()->SetFPV(!GetEditorDoc()->IsFPV());
}

//=========================================================================

void CEditorFrame::OnUpdateWetbToggleFPV(CCmdUI* pCmdUI) 
{
    if (GetEditorDoc() && GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable(TRUE);	
        pCmdUI->SetCheck(GetEditorDoc()->IsFPV());	
    }
    else
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbGridToObject() 
{
    if (m_pWorldEditView)
        m_pWorldEditView->MoveGridToSelectedObjects();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbGridToObject(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView &&
        m_pWorldEditView->IsStandardMode() &&
        g_Project.IsProjectOpen() &&
        (g_WorldEditor.GetSelectedCount() > 0))
    {
        pCmdUI->Enable(TRUE);	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

// FindMenuItem() will find a menu item string from the specified
// popup menu and returns its position (0-based) in the specified 
// popup menu. It returns -1 if no such menu item string is found.
int FindMenuItem(CXTMenu& Menu, LPCTSTR MenuString)
{
   ASSERT(Menu);
   ASSERT(::IsMenu(Menu.GetSafeHmenu()));

   int count = Menu.GetMenuItemCount();
   for (int i = 0; i < count; i++)
   {
      CString str;
      if (Menu.GetMenuString(i, str, MF_BYPOSITION) &&
         (stricmp(str, MenuString) == 0))
         return i;
   }

   return -1;
}

//=========================================================================

void CEditorFrame::OnCreateObject()
{
    if (m_pWorldEditView->IsPlacementMode())
    {
        m_pWorldEditView->CancelPlacementMode();
    }
    else if (m_pWorldEditView->IsStandardMode())
    {
        s32     Count = 0;
	    CXTMenu menu;
        CXTMenu SubmenuArray[64];
        s32     iSubMenu=0;
        menu.CreatePopupMenu();

        for (int i = 0; i < m_lstGenericCreateMenuItems.GetCount(); i++)
        {
            //parse data
            CString strRawData = m_lstGenericCreateMenuItems.GetAt(m_lstGenericCreateMenuItems.FindIndex(i));
            int index = strRawData.Find('\\');
            if (index == -1)
            {
                ASSERT(FALSE);
                return;
            }
            CString strGroup = strRawData.Left(index);
            CString strType  = strRawData.Right(strRawData.GetLength() - index -1);

            //add group if not already there
            int pos = FindMenuItem(menu, strGroup );
            if( pos == -1 )
            {   
                ASSERT( iSubMenu < 64 );
                CXTMenu& Submenu = SubmenuArray[iSubMenu];
                iSubMenu++;

                Submenu.CreatePopupMenu();
                menu.AppendMenu(MF_POPUP, (UINT) Submenu.m_hMenu, strGroup);
                pos = FindMenuItem(menu, strGroup );
                ASSERT( pos != -1 );
            }
    
            //add type item
            if( pos != -1 )
            {
                CMenu* submenu = menu.GetSubMenu(pos);
                submenu->AppendMenu(MF_STRING|MF_ENABLED, IDR_CREATE_OBJECT_GENERIC_FIRST+Count, strType );

                // Get ready for the nxt one
                Count++;
            }
            else
            {
                ASSERT(FALSE);
            }
        }

        if (g_WorldEditor.GetSelectedCount() == 1)
	        menu.AppendMenu(MF_STRING|MF_ENABLED, IDR_CREATE_OBJECT_FROM_SELECTED, "Create From Selected");

	    CPoint point;
	    GetCursorPos( &point );
	    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x, point.y, this);
    }
}

//=========================================================================

void CEditorFrame::OnWetbCreateObject() 
{
    OnCreateObject();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbCreateObject(CCmdUI* pCmdUI) 
{    
    if (m_pWorldEditView && 
        g_Project.IsProjectOpen() &&
        (!GetEditorDoc()->IsGameRunning()) &&
        !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetActiveLayer()) &&
        (m_pWorldEditView->IsPlacementMode() || m_pWorldEditView->IsStandardMode()))
    {
        pCmdUI->Enable(TRUE);	
        pCmdUI->SetCheck(m_pWorldEditView->IsPlacementMode());	
    }
    else 
    {
        pCmdUI->Enable(FALSE);	
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorFrame::OnWetbDeleteSelected() 
{
    //HACK check for portals in the list
    xarray<guid> lstObjs;
    xarray<guid> lstPortals;
    g_WorldEditor.GetSelectedList(lstObjs);
    for (int i =0; i < lstObjs.GetCount(); i++)
    {
        guid ObjGuid = lstObjs.GetAt(i);
        object* pObj = g_ObjMgr.GetObjectByGuid(ObjGuid);
        if (pObj && pObj->IsKindOf( zone_portal::GetRTTI() ) )
        {
            //append to portal list, we will delete these last
            lstPortals.Append(ObjGuid);
        }
    }

    xarray<editor_item_descript> lstItems;

    //setup generic undo entry for moving of objects
    g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(
        xfs("Deleting %d Object(s)",g_WorldEditor.GetSelectedCount())));

	g_WorldEditor.DeleteSelectedObjects(lstItems);
    for (i=0; i<lstItems.GetCount(); i++)
    {
        editor_item_descript Description = lstItems.GetAt(i);
        CString strName;
        for (int s =0; s<Description.Layer.GetLength(); s++) strName += Description.Layer.GetAt(s);
        if (Description.IsInBlueprint)
        {
            RemoveBlueprintFromLayerView(strName, CString(Description.LayerPath), Description.Guid);
        }
        else
        {
            RemoveObjectFromLayerView(strName, CString(Description.LayerPath), Description.Guid);
        }
    }

    //Commit the Undo
    g_WorldEditor.CommitCurrentUndoEntry();

    CEditorLayerView* pView = (CEditorLayerView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    if (pView)
    {
        for (int i =0; i < lstPortals.GetCount(); i++)
        {
            guid ObjGuid = lstPortals.GetAt(i);
            pView->SelectObject(ObjGuid, TRUE);
            pView->DeleteSelected();
        }
    }

    if (GetPropertyEditorDoc()) GetPropertyEditorDoc()->Refresh();
    if (m_pWorldEditView) m_pWorldEditView->SetFocus(); //set focus to view
    if (m_pWorldEditView) m_pWorldEditView->Invalidate();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbDeleteSelected(CCmdUI* pCmdUI) 
{
    BOOL bEnable = FALSE;
    if (GetEditorDoc() && 
        GetEditorView() && 
        g_Project.IsProjectOpen() &&
        GetEditorView()->IsWindowVisible() &&
        !GetEditorDoc()->IsGameRunning() && g_WorldEditor.IsSelectedObjectsEditable())
    {
        bEnable = (g_WorldEditor.GetSelectedCount()>0) &&
                   m_pWorldEditView->IsStandardMode();
    }
	pCmdUI->Enable(bEnable);
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnWetbCameraHistoryPrev()
{
    GetEditorView()->CameraUndo();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbCameraHistoryPrev(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;

    if (GetEditorDoc() && 
        GetEditorView() && 
        g_Project.IsProjectOpen() &&
        GetEditorView()->IsWindowVisible() &&
        !GetEditorDoc()->IsGameRunning() )
    {
        bEnable = GetEditorView()->CameraCanUndo();
    }

    pCmdUI->Enable( bEnable );
}

//=========================================================================

void CEditorFrame::OnWetbCameraHistoryNext()
{
    GetEditorView()->CameraRedo();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbCameraHistoryNext(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;

    if (GetEditorDoc() && 
        GetEditorView() && 
        g_Project.IsProjectOpen() &&
        GetEditorView()->IsWindowVisible() &&
        !GetEditorDoc()->IsGameRunning() )
    {
        bEnable = GetEditorView()->CameraCanRedo();
    }

    pCmdUI->Enable( bEnable );
}

//=========================================================================

void CEditorFrame::OnWetbCameraFavorites()
{
}

//=========================================================================

void CEditorFrame::OnWetbGotoPlayer()
{
    GetEditorView()->CameraGotoPlayer();
}

//=========================================================================

void CEditorFrame::OnUpdateWetbCameraFavorites(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;

    if (GetEditorDoc() && 
        GetEditorView() && 
        g_Project.IsProjectOpen() &&
        GetEditorView()->IsWindowVisible() )
    {
        // If game is not running, always enable
        if( !GetEditorDoc()->IsGameRunning() )
        {        
            bEnable = TRUE;
        }
        // If game is running and not in first person view, then enable        
        else if (!GetEditorDoc()->IsFPV() )
        {
            bEnable = TRUE;
        }
    }
    
    pCmdUI->Enable( bEnable );
}

//=========================================================================

void CEditorFrame::OnCreateFromSelected()
{
    if (m_pWorldEditView && GetEditorDoc())
    {
	    g_WorldEditor.CreateTemporaryObjectFromSel();
        g_WorldEditor.MoveTemporaryObjects(m_pWorldEditView->GetFocusPos());
        m_pWorldEditView->EnterPlacementMode();
        if (GetPropertyEditorDoc()) GetPropertyEditorDoc()->Refresh();
        m_pWorldEditView->SetViewDirty();
    }
    if (m_pWorldEditView) m_pWorldEditView->SetFocus(); //set focus to view
}

//=========================================================================

void CEditorFrame::OnUpdateCreateGenericObject(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnCreateGenericObject(UINT nID) 
{
    s32 nIndex = nID - IDR_CREATE_OBJECT_GENERIC_FIRST;  //zero based
    
    ASSERT(m_lstGenericCreateNodeData.GetCount() > nIndex);
    if( m_lstGenericCreateNodeData.GetCount() > nIndex )
    {
        object_desc* pNode = (object_desc*)m_lstGenericCreateNodeData.GetAt(m_lstGenericCreateNodeData.FindIndex(nIndex));
        ASSERT(pNode);
        CreateTemporaryObject( pNode->GetTypeName() );

        if( pNode->QuickResourceName() && pNode->QuickResourcePropertyName() )
        {
            //Show resource dialog
            CResourceBrowserDlg dlg;
            dlg.SetType( pNode->QuickResourceName() );

            if (dlg.DoModal() == IDOK)
            {
                //set the objects property
                CString strPath = dlg.GetPath();
                CString strName = dlg.GetName();
                CString strFullPath = strPath + strName;

                g_WorldEditor.SetTempObjectExternal( pNode->QuickResourcePropertyName(), strName);
            }
        }

        g_WorldEditor.MoveTemporaryObjects(m_pWorldEditView->GetFocusPos());

        GetPropertyEditorDoc()->Refresh();

        m_pWorldEditView->SetViewDirty();
        m_pWorldEditView->SetFocus(); //set focus to view
    }
}

//=========================================================================

guid CEditorFrame::CreateObject( const char* pObjectType, vector3& pos, BOOL bSelect)
{
    guid Guid(0);
    if (GetEditorDoc())
    {
        Guid = g_WorldEditor.CreateNewObjectAtPosition( pObjectType, pos, xbool(bSelect));

        if (Guid.Guid!=0)
        {
            AddObjectToActiveLayerView(Guid);

            if (GetPropertyEditorDoc()) GetPropertyEditorDoc()->Refresh();

            m_pWorldEditView->Invalidate();
        }
    }
    return Guid;
}

//=========================================================================
// Update messages for the blueprint view
//=========================================================================

void CEditorFrame::OnUpdateBptbCreateBlueprint(CCmdUI* pCmdUI)
{
    if (GetEditorDoc() && g_Project.IsProjectOpen() && 
        GetEditorView()->IsStandardMode() && !GetEditorDoc()->IsGameRunning() &&
        !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetActiveLayer()))
    {
        //if objects are selected and NOT in standard mode
        BOOL bEnable = ((g_WorldEditor.GetSelectedCount() > 0) &&
                        (m_pWorldEditView->IsStandardMode()));

	    pCmdUI->Enable(bEnable);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnUpdateBptbCreateAnchor(CCmdUI* pCmdUI) 
{
    if (m_pWorldEditView && g_Project.IsProjectOpen() && GetEditorView()->IsStandardMode() &&
        GetEditorDoc() && !GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable(TRUE);	
    }
    else 
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnUpdateBptbAddBlueprintAsObjects(CCmdUI* pCmdUI) 
{
    CEditorBlueprintView* pView = (CEditorBlueprintView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorBlueprintView));
    if (pView && g_Project.IsProjectOpen() && GetEditorView()->IsStandardMode() && 
        GetEditorDoc() && !GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable(pView->IsBlueprintAddable(this));	
    }
    else 
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnUpdateBptbShatterBlueprint(CCmdUI* pCmdUI)
{
    if (GetEditorDoc() && g_WorldEditor.IsBlueprintSelected() &&
        g_Project.IsProjectOpen() && GetEditorView()->IsStandardMode() && 
        !GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable(TRUE);	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnUpdateBptbRefresh(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(FALSE);	
    pCmdUI->Enable(g_Project.IsProjectOpen());	
}

//=========================================================================

void CEditorFrame::OnUpdateBptbShatterBlueprintForEdit(CCmdUI* pCmdUI)
{
    editor_blueprint_ref BPRef;
    if (GetEditorDoc() && g_WorldEditor.IsOneBlueprintSelected(BPRef) &&
        g_Project.IsProjectOpen() && GetEditorView()->IsStandardMode() &&
        !GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable(TRUE);	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnUpdateBptbAddBlueprint(CCmdUI* pCmdUI) 
{
    CEditorBlueprintView* pView = (CEditorBlueprintView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorBlueprintView));
    if (pView && g_Project.IsProjectOpen() && 
        (GetEditorView()->IsStandardMode() || GetEditorView()->IsBlueprintMode()) &&
        GetEditorDoc() && !GetEditorDoc()->IsGameRunning())
    {
        pCmdUI->Enable(pView->IsBlueprintAddable(this) || GetEditorView()->IsBlueprintMode());	
        pCmdUI->SetCheck(GetEditorView()->IsBlueprintMode());	
    }
    else 
    {
        pCmdUI->Enable(FALSE);	
    }
}

//=========================================================================
// Layer handlers
//=========================================================================

BOOL CEditorFrame::AddObjectToLayerView(CString strLayer, CString strLayerPath, guid ObjectGuid)
{
    CEditorLayerView* pView = (CEditorLayerView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    if (pView)
    {
        if (g_WorldEditor.IsGlobalObject(ObjectGuid))
        {
            return (pView->AddObjectToLayer(g_WorldEditor.GetGlobalLayer(), strLayerPath, ObjectGuid) != NULL);
        }
        else
        {
            return (pView->AddObjectToLayer(strLayer, strLayerPath, ObjectGuid) != NULL);
        }
    }
    return FALSE;
}

//=========================================================================

BOOL CEditorFrame::AddObjectToActiveLayerView(guid ObjectGuid)
{
    CEditorLayerView* pView = (CEditorLayerView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    if (pView)
    {
        CString strActiveLayer;
        m_wndComboBox.GetWindowText(strActiveLayer);
        if (g_WorldEditor.IsGlobalObject(ObjectGuid))
        {
            return (pView->AddObjectToLayer(g_WorldEditor.GetGlobalLayer(), "\\GlobalObjs\\", ObjectGuid) != NULL);
        }
        else
        {
            return (pView->AddObjectToLayer(g_WorldEditor.GetActiveLayer(), g_WorldEditor.GetActiveLayerPath(), ObjectGuid) != NULL);
        }
    }
    return FALSE;
}

//=========================================================================

BOOL CEditorFrame::RemoveObjectFromLayerView(CString strLayer, CString strLayerPath, guid ObjectGuid)
{
    CEditorLayerView* pView = (CEditorLayerView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    if (pView)
    {
        return pView->RemoveObjectFromLayer(strLayer, strLayerPath, ObjectGuid);
    }
    return FALSE;
}

//=========================================================================

BOOL CEditorFrame::AddBlueprintToLayerView(CString strLayer, CString strLayerPath, guid BlueprintGuid)
{
    CEditorLayerView* pView = (CEditorLayerView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    if (pView)
    {
        return (pView->AddBlueprintToLayer(strLayer, strLayerPath, BlueprintGuid) != NULL);
    }
    return FALSE;
}

//=========================================================================

BOOL CEditorFrame::AddBlueprintToActiveLayerView(guid BlueprintGuid)
{
    CEditorLayerView* pView = (CEditorLayerView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    if (pView)
    {
        return (pView->AddBlueprintToLayer(g_WorldEditor.GetActiveLayer(), g_WorldEditor.GetActiveLayerPath(), BlueprintGuid) != NULL);
    }
    return FALSE;
}

//=========================================================================

BOOL CEditorFrame::RemoveBlueprintFromLayerView(CString strLayer, CString strLayerPath, guid BlueprintGuid)
{
    CEditorLayerView* pView = (CEditorLayerView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
    if (pView)
    {
        return pView->RemoveBlueprintFromLayer(strLayer, strLayerPath, BlueprintGuid);
    }
    return FALSE;
}

//=========================================================================
// Update messages for the AI view
//=========================================================================

void CEditorFrame::OnUpdateAIButtonCreateNavNode(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(FALSE);	
    pCmdUI->Enable(
        g_Project.IsProjectOpen() && 
        !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetGlobalLayer()) &&
        GetEditorView()->IsStandardMode());	
}

//=========================================================================

void CEditorFrame::OnUpdateAIButtonCreatePlayer(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(FALSE);	
    pCmdUI->Enable(g_Project.IsProjectOpen() && 
        !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetActiveLayer()) &&
        GetEditorView()->IsStandardMode());	
}

//=========================================================================
// Update messages for the Object Palette view
//=========================================================================

void CEditorFrame::OnUpdateOvtbAddPlaysurface(CCmdUI* pCmdUI)
{
    CEditorObjectView* pView = (CEditorObjectView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorObjectView));
    pCmdUI->Enable( g_Project.IsProjectOpen() &&
                    !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetActiveLayer()) &&
                    pView && 
                    pView->CanAdd() &&
                    GetEditorDoc() && 
                    !GetEditorDoc()->IsGameRunning() &&
                    m_pWorldEditView && 
                    m_pWorldEditView->IsStandardMode());	
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorFrame::OnUpdateOvtbAddPropsurface(CCmdUI* pCmdUI)
{
    CEditorObjectView* pView = (CEditorObjectView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorObjectView));
    pCmdUI->Enable( g_Project.IsProjectOpen() &&
                    !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetActiveLayer()) &&
                    pView && 
                    pView->CanAdd() &&
                    GetEditorDoc() && 
                    !GetEditorDoc()->IsGameRunning() &&
                    m_pWorldEditView && 
                    m_pWorldEditView->IsStandardMode());	
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorFrame::OnUpdateOvtbAddAnimsurface(CCmdUI* pCmdUI)
{
    CEditorObjectView* pView = (CEditorObjectView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorObjectView));
    pCmdUI->Enable( g_Project.IsProjectOpen() &&
                    !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetActiveLayer()) &&
                    pView && 
                    pView->CanAdd() &&
                    GetEditorDoc() && 
                    !GetEditorDoc()->IsGameRunning() &&
                    m_pWorldEditView && 
                    m_pWorldEditView->IsStandardMode());	
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorFrame::OnUpdateOvtbUpdateRigidInst(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;
    if (g_Project.IsProjectOpen() &&                                //project open
        GetEditorDoc() && (!GetEditorDoc()->IsGameRunning()) &&     //game not running
        m_pWorldEditView && m_pWorldEditView->IsStandardMode())     //in standard mode
    {
        if (g_WorldEditor.GetSelectedCount() == 1)
        {
            guid ObjGuid = g_WorldEditor.GetSelectedObjectsByIndex( 0 );

            const char* pTypeName = g_WorldEditor.GetObjectTypeName( ObjGuid );
            if ( (x_stricmp( pTypeName, play_surface::GetObjectType().GetTypeName() ) == 0) ||
                 (x_stricmp( pTypeName, prop_surface::GetObjectType().GetTypeName() ) == 0) ||
                 (x_stricmp( pTypeName, anim_surface::GetObjectType().GetTypeName() ) == 0) )
            {
                bEnable = TRUE;	
            }
        }
    }

    pCmdUI->Enable(bEnable);	
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnUpdateOvtbUpdateGeomsFromSel(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;
    if (g_Project.IsProjectOpen() &&                                //project open
        GetEditorDoc() && (!GetEditorDoc()->IsGameRunning()) &&     //game not running
        m_pWorldEditView && m_pWorldEditView->IsStandardMode())     //in standard mode
    {
        if (g_WorldEditor.GetSelectedCount() == 1)
        {
            guid ObjGuid = g_WorldEditor.GetSelectedObjectsByIndex( 0 );

            const char* pTypeName = g_WorldEditor.GetObjectTypeName( ObjGuid );
            if ( (x_stricmp( pTypeName, play_surface::GetObjectType().GetTypeName() ) == 0) ||
                 (x_stricmp( pTypeName, prop_surface::GetObjectType().GetTypeName() ) == 0) ||
                 (x_stricmp( pTypeName, anim_surface::GetObjectType().GetTypeName() ) == 0) )
            {
                bEnable = TRUE;	
            }
        }
    }

    pCmdUI->Enable(bEnable);	
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorFrame::OnUpdateOvtbRefresh(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(FALSE);	
    pCmdUI->Enable(g_Project.IsProjectOpen());	
}

//=========================================================================
// Update messages for the Decal Palette view
//=========================================================================

void CEditorFrame::OnUpdateDvtbRefresh(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(FALSE);
    pCmdUI->Enable(g_Project.IsProjectOpen());
}

//=========================================================================

void CEditorFrame::OnUpdateDvtbPaintMode(CCmdUI* pCmdUI)
{
    CEditorDecalView* pView = (CEditorDecalView*)FindViewFromTab( m_wndWrkspBar,RUNTIME_CLASS(CEditorDecalView));
    pCmdUI->Enable( g_Project.IsProjectOpen() &&
                    !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetActiveLayer()) &&
                    pView && 
                    pView->CanAdd() &&
                    GetEditorDoc() && 
                    !GetEditorDoc()->IsGameRunning() &&
                    m_pWorldEditView && 
                    m_pWorldEditView->IsStandardMode());	
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorFrame::OnClose() 
{
	//commenting out so user can not close this window
	//CBaseFrame::OnClose();
}

//=========================================================================

void CEditorFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CBaseFrame::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	
    if( GetEditorDoc() )
    {
        GetEditorDoc()->SetDocumentActive( bActivate );
    }

    if (!m_strCurrentLevel.IsEmpty())
        SetWindowText(m_strCurrentLevel);	
    else
        SetWindowText("WorldEditor");
   
    CXTMenuBar* pMenuBar = CMainFrame::s_pMainFrame->GetMenuBar();
    if( pMenuBar )
    {
        if( bActivate ) //&& (m_MenuResource != 0) )
        {
            CMenu menu;
            menu.LoadMenu( IDR_MENU_WORLD );
            HMENU hMenu = menu.Detach();
            m_hPreviousMenu = pMenuBar->LoadMenu( hMenu, hMenu );
        }
        else if( !bActivate && (m_hPreviousMenu != 0) )
        {
            pMenuBar->LoadMenu( m_hPreviousMenu, NULL );
            m_hPreviousMenu = 0;
        }
    }

    GetEditorView()->SetViewDirty();   
}

//=========================================================================

void CEditorFrame::ReloadCurrentLevel()
{
    GetEditorDoc()->ReloadLevel(CString(g_Project.GetWorkingPath() ));
}

//=========================================================================

void CEditorFrame::ShowSettings()
{
    m_wndProperty.SetActiveView(0);
}

//=========================================================================

void CEditorFrame::ShowProperties()
{
    m_wndProperty.SetActiveView(1);
}

//=========================================================================

void CEditorFrame::OnUpdateButtonConnectVisible(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen() && 
        !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetGlobalLayer()) &&
        GetEditorView()->IsStandardMode());	
}

//=========================================================================

void CEditorFrame::OnUpdateButtonConnectSelected(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen() && 
        !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetGlobalLayer()) &&
        GetEditorView()->IsStandardMode());	
}

//=========================================================================

void CEditorFrame::OnUpdateButtonConnectMode(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen() && 
        !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetGlobalLayer()) &&
        GetEditorView()->IsStandardMode());
}

//=========================================================================

void CEditorFrame::OnUpdateButtonCheckAllNodes(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen() && 
        !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetGlobalLayer()) &&
        GetEditorView()->IsStandardMode());	
}

//=========================================================================

void CEditorFrame::OnUpdateButtonChainNodes(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen() && 
        !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetGlobalLayer()) &&
        GetEditorView()->IsStandardMode());
}

//=========================================================================

LRESULT CEditorFrame::OnToolBarDropDown(WPARAM wParam, LPARAM lParam)
{
    NMTOOLBAR* pNMTB = (NMTOOLBAR*)wParam;
    ASSERT( pNMTB != NULL );

    CRect* pRect = (CRect*)lParam;
    ASSERT( pRect != NULL );

    // Create the menu
    CXTMenu Menu;
    Menu.CreatePopupMenu();

    // Add the "Add Favorite option"
    Menu.AppendMenu( MF_STRING   , ID_CFM_ADD, _T("Add camera to favorites...") );
    Menu.AppendMenu( MF_SEPARATOR, 1, _T("") );

    // Get data
    CEditorView*            pView = GetEditorView();
    xarray<view_favorite>&  Favorites = pView->GetViewFavorites();

    // Add the favorites to the menu
    s32 Count = Favorites.GetCount();
    Count = MIN( Count, 10 );
    for( s32 i=0 ; i<Count ; i++ )
    {
        Menu.AppendMenu( MF_STRING   , ID_CFM_FAVORITE_1 + i , Favorites[i].m_Name );
    }

    if( Favorites.GetCount() > 10 )
    {
        Menu.AppendMenu( MF_SEPARATOR, 1, _T("") );
        Menu.AppendMenu( MF_DISABLED | MF_STRING   , ID_CFM_MORE, _T("More favorites...") );
    }

    // Display and run the menu
    CPoint p;
    p.x = pRect->left;
    p.y = pRect->bottom - 1;
    Menu.TrackPopupMenu( TPM_LEFTALIGN, p.x, p.y, this );

    // Cleanup
    Menu.DestroyMenu();

    return 0;
}

//=========================================================================

void CEditorFrame::OnCameraFavoriteAdd()
{
    // Get data
    CDialogTextEntry        Dialog;
    CEditorView*            pView = GetEditorView();
    xarray<view_favorite>&  Favorites = pView->GetViewFavorites();

    // Set the next favorite name
    Dialog.m_Caption = "Add Favorite Camera...";
    Dialog.m_Name.Format( "Favorite %d", Favorites.GetCount() + 1 );

    // Show the dialog to prompt for a name
    if( Dialog.DoModal() == IDOK )
    {
        // Add the favorite
        view_favorite Favorite;
        Favorite.m_Name     = Dialog.m_Name;
        Favorite.m_View     = pView->GetCameraView();
        Favorite.m_FocusPos = pView->GetFocusPos();
        Favorites.Insert( 0, Favorite );
    }
}

//=========================================================================

void CEditorFrame::OnCameraFavorite( UINT nID )
{
    // Get data
    CEditorView*            pView = GetEditorView();
    xarray<view_favorite>&  Favorites = pView->GetViewFavorites();

    // Get index into favorites array
    s32 Index = nID - ID_CFM_FAVORITE_1;

    // Jump to the favorite view
    if( (Index >= 0) && (Index < Favorites.GetCount()) )
    {
        // Move the camera to the favorite
        pView->SetCameraWithUndo( Favorites[Index].m_View, Favorites[Index].m_FocusPos );
    }
}

//=========================================================================

void CEditorFrame::OnCameraFavoriteMore()
{
}

//=========================================================================

void CEditorFrame::OnUpdateSelectionSelectAll(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( FALSE ); //g_Project.IsProjectOpen() );
//    pCmdUI->SetCheck( FALSE );
}

//=========================================================================

void CEditorFrame::OnSelectionSelectAll()
{
//    g_WorldEditor.SelectObjectsAll();
//    GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorFrame::OnUpdateDebugInvulnerable(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck( DEBUG_INVULNERABLE );
}

//=========================================================================

extern xbool g_UnlimitedAmmo;

void CEditorFrame::OnUpdateDebugInfiniteAmmo(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck( DEBUG_INFINITE_AMMO );
}

//=========================================================================

void CEditorFrame::OnDebugInvulnerable()
{
#ifndef CONFIG_RETAIL
    DEBUG_INVULNERABLE = !DEBUG_INVULNERABLE;
#endif
}

//=========================================================================

void CEditorFrame::OnDebugInfiniteAmmo()
{
#ifndef CONFIG_RETAIL
    DEBUG_INFINITE_AMMO = !DEBUG_INFINITE_AMMO;
#endif
}

//=========================================================================
