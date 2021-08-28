// EditorAIView.cpp : implementation file
//

#include "StdAfx.h"
#include "EditorAIView.h"
#include "EditorPaletteDoc.h"
#include "EditorFrame.h"
#include "EditorView.h"
#include "Resource.h"
#include "ai_editor.hpp"
#include "transaction_mgr.hpp"
#include "Objects\player.hpp"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "EditorLayerView.h"
#include "ConnectionAttributeDialog.h"

#include "nav_connection2_editor.hpp"
#include "nav_connection2_anchor.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEditorAIView* CEditorAIView::s_AIView = NULL;

/////////////////////////////////////////////////////////////////////////////
// CEditorAIView

IMPLEMENT_DYNCREATE(CEditorAIView, CPaletteView)

CEditorAIView::CEditorAIView() :
    m_CreateNavConnectionMode (false)
{
    s_AIView = this;

}

CEditorAIView::~CEditorAIView()
{ 
}


BEGIN_MESSAGE_MAP(CEditorAIView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorAIView)
	ON_WM_CREATE()
	ON_WM_SIZE()
    /*
	ON_COMMAND(ID_BUTTON_CREATE_NAV_NODE, OnButtonCreateNavNode)
	ON_COMMAND(ID_BUTTON_CREATE_PLAYER, OnButtonCreatePlayer)
	ON_COMMAND(ID_BUTTON_CHAIN_NODES, OnButtonChainNodes)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CHAIN_NODES, OnUpdateButtonChainNodes)
	ON_COMMAND(ID_BUTTON_CHECK_ALL_NODES, OnButtonCheckAllNodes)
	ON_COMMAND(ID_BUTTON_CONNECT_VISIBLE, OnButtonConnectVisible)
	ON_COMMAND(ID_BUTTON_CONNECT_MODE, OnButtonConnectMode)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CONNECT_MODE, OnUpdateButtonConnectMode)
	ON_COMMAND(ID_BUTTON_CONNECT_SELECTED, OnButtonConnectSelected)
	ON_COMMAND(ID_BUTTON_BATCH_FLAG, OnButtonBatchFlag)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CREATE_PLAYER, OnUpdateAIButtonCreatePlayer)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CREATE_NAV_NODE, OnUpdateAIButtonCreateNavNode)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CONNECT_VISIBLE, OnUpdateButtonConnectVisible)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CONNECT_SELECTED, OnUpdateButtonConnectSelected)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CONNECT_MODE, OnUpdateButtonConnectMode)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CHECK_ALL_NODES, OnUpdateButtonCheckAllNodes)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CHAIN_NODES, OnUpdateButtonChainNodes)
    */
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEditorAIView drawing

void CEditorAIView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CEditorAIView diagnostics

#ifdef _DEBUG
void CEditorAIView::AssertValid() const
{
	CPaletteView::AssertValid();
}

void CEditorAIView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEditorAIView message handlers

int CEditorAIView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_AIVIEW_FILTER;   
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_aiTree.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                           TVS_EDITLABELS | TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDC_AI_TREE_LIST))
    {
		TRACE0("Failed to create tree\n");
        return -1;	      
    }

	return 0;
}

void CEditorAIView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

void CEditorAIView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);

    CSize size = SizeToolBar(cx, cy);
    m_aiTree.MoveWindow(0,size.cy,cx,cy - size.cy);
}

void CEditorAIView::OnButtonCreateNavNode() 
{
    guid Connection;
    guid Anchor0;
    guid Anchor1;

    
    if( g_WorldEditor.CreateTemporaryObject( nav_connection2_editor::GetObjectType().GetTypeName() ) == false )
    {
        x_throw("CreateTemporaryObject failed for nav_connection2_editor!" );
        return;
    }
    Connection = g_WorldEditor.GetGuidOfLastPlacedTemp();


    //TODO - FIX ME - FIX ME - FIX ME -
    //DOH! AI doesn't support UNDO, CLEAR THE STACK
//    transaction_mgr::Transaction()->ClearStack();

    g_WorldEditor.SelectObject(Connection);
    //g_WorldEditor.SelectObject(Anchor0);
    //g_WorldEditor.SelectObject(Anchor1);

    GetDocument()->GetFramePointer()->GetPropertyEditorDoc()->Refresh();
    g_WorldEditor.MoveSelectedObjects(GetDocument()->GetFramePointer()->GetEditorView()->GetFocusPos());
    GetDocument()->GetFramePointer()->GetEditorView()->EnterPlacementMode();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
 
}

void CEditorAIView::OnButtonCreateNavConnection() 
{
	m_CreateNavConnectionMode = !m_CreateNavConnectionMode;
}

void CEditorAIView::OnButtonCreateForcedNavZone() 
{
}


void CEditorAIView::OnButtonGo() 
{
	// TODO: Add your command handler code here
    ai_editor::GetAIEditor()->CalcPath();
	
}

void CEditorAIView::OnButtonSet1() 
{
	// TODO: Add your command handler code here
    
    if(g_WorldEditor.GetSelectedCount() )
    {
        guid aGuid;
        aGuid = g_WorldEditor.GetSelectedObjectsByIndex( 0 );
        ai_editor::GetAIEditor()->SetNavTestStart(aGuid);
    }

	
}

void CEditorAIView::OnButtonSet2() 
{
	// TODO: Add your command handler code here
    if(g_WorldEditor.GetSelectedCount() )
    {
        guid aGuid;
        aGuid = g_WorldEditor.GetSelectedObjectsByIndex( 0 );
        ai_editor::GetAIEditor()->SetNavTestEnd(aGuid);
    }
}


void CEditorAIView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::AI");
    }
}

void CEditorAIView::OnButtonCreatePlayer() 
{
    //TODO - FIX ME - FIX ME - FIX ME -
    //DOH! AI doesn't support UNDO, CLEAR THE STACK
//    transaction_mgr::Transaction()->ClearStack();
    
    xbool playerExists = ai_editor::GetAIEditor()->DoesPlayerExist();
    
    guid aGuid = ai_editor::GetAIEditor()->CreatePlayer();

    if(!playerExists)
    {
        g_WorldEditor.AddObjectToActiveLayer( aGuid );
        GetDocument()->GetFramePointer()->AddObjectToActiveLayerView( aGuid );
    }

    g_WorldEditor.SelectObject(aGuid);
    GetDocument()->GetFramePointer()->GetEditorView()->EnterMovementMode();

    object* pObject = g_ObjMgr.GetObjectByGuid( aGuid );
   
}

void CEditorAIView::OnButtonChainNodes() 
{
	// TODO: Add your command handler code here

    //nav_node_editor::m_AutoChain = !(nav_node_editor::m_AutoChain);
    
	
}

void CEditorAIView::OnUpdateButtonChainNodes(CCmdUI* pCmdUI) 
{
    /*
	// TODO: Add your command update UI handler code here
    if( nav_node_editor::m_AutoChain )
    {
        pCmdUI->SetCheck( 1 );

    }
    else
    {
        pCmdUI->SetCheck( 0 );
    }
	*/
}

void CEditorAIView::OnButtonCheckAllNodes() 
{
    /*
	// TODO: Add your command handler code here
	ai_editor::GetAIEditor()->CheckAllNavConnections();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    */
}

void CEditorAIView::OnButtonConnectVisible() 
{
    /*
	// TODO: Add your command handler code here
    ai_editor::GetAIEditor()->ConnectAllNavNodes();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
	*/
}

void CEditorAIView::OnButtonConnectMode() 
{
    /*
	// TODO: Add your command handler code here
    ai_editor::GetAIEditor()->SetInConnectionMode( !ai_editor::GetAIEditor()->IsInConnectionMode() );
    ai_editor::GetAIEditor()->SetObjectSelected( 0 );
    */
}

void CEditorAIView::OnUpdateButtonConnectMode(CCmdUI* pCmdUI) 
{
    /*
	// TODO: Add your command update UI handler code here
    if ( ai_editor::GetAIEditor()->IsInConnectionMode() )
    {
        pCmdUI->SetCheck( 1 );
    }
    else
    {
        pCmdUI->SetCheck( 0 );
    }

	*/
}

void CEditorAIView::OnButtonConnectSelected() 
{
    /*
	// TODO: Add your command handler code here
    ai_editor::GetAIEditor()->ConnectAllSelectedNavNodes();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
	*/
}

void CEditorAIView::OnButtonBatchFlag() 
{
	// TODO: Add your command handler code here
	ConnectionAttributeDialog aDialog;
    aDialog.DoModal();
}

//=========================================================================

void CEditorAIView::OnUpdateAIButtonCreateNavNode(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateAIButtonCreateNavNode(pCmdUI);
    }
}

//=========================================================================

void CEditorAIView::OnUpdateAIButtonCreatePlayer(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateAIButtonCreatePlayer(pCmdUI);
    }
}

//=========================================================================

void CEditorAIView::OnUpdateButtonConnectVisible(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateButtonConnectVisible(pCmdUI);
    }
}

//=========================================================================

void CEditorAIView::OnUpdateButtonConnectSelected(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateButtonConnectSelected(pCmdUI);
    }
}

//=========================================================================

void CEditorAIView::OnUpdateButtonCheckAllNodes(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateButtonCheckAllNodes(pCmdUI);
    }
}
