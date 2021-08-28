 // EditorDebuggerView.cpp

#include "StdAfx.h"
#include "EditorDebuggerView.h"
#include "EditorPaletteDoc.h"
#include "EditorFrame.h"
#include "EditorDoc.h"
#include "EditorView.h"
#include "Resource.h"
#include "Auxiliary\MiscUtils\Guid.hpp"
#include "worldeditor.hpp"
#include "..\Editor\Project.hpp"
#include "..\Support\Globals\Global_Variables_Manager.hpp"
#include "..\WinControls\ListBoxDlg.h"
#include "Obj_Mgr\Obj_Mgr.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CEditorDebuggerView

IMPLEMENT_DYNCREATE(CEditorDebuggerView, CPaletteView)

BEGIN_MESSAGE_MAP(CEditorDebuggerView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorDebuggerView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_GDTB_GUID_LOOKUP, OnGdtbGuidLookup)
	ON_UPDATE_COMMAND_UI(ID_GDTB_GUID_LOOKUP, OnUpdateGdtbGuidLookup)
	ON_COMMAND(ID_GDTB_GLOBAL_GUID_LOOKUP, OnGdtbGlobalGuidLookup)
	ON_UPDATE_COMMAND_UI(ID_GDTB_GLOBAL_GUID_LOOKUP, OnUpdateGdtbGlobalGuidLookup)
	ON_COMMAND(ID_GDTB_CLEAR_INFO, OnGdtbClearInfo)
	ON_UPDATE_COMMAND_UI(ID_GDTB_CLEAR_INFO, OnUpdateGdtbClearInfo)
	ON_NOTIFY (TVN_SELCHANGED, IDR_DEBUGGER_VIEW, OnSelchangedTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorDebuggerView diagnostics

#ifdef _DEBUG
void CEditorDebuggerView::AssertValid() const
{
	CPaletteView::AssertValid();
}

void CEditorDebuggerView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

CEditorDebuggerView::CEditorDebuggerView()
{
    m_QueryType = DQT_NULL;
}

//=========================================================================

CEditorDebuggerView::~CEditorDebuggerView()
{
}

//=========================================================================

void CEditorDebuggerView::OnDraw(CDC* pDC)
{
}

//=========================================================================

int CEditorDebuggerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_GUID_DEBUGGER;
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_tcDebugger.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                          TVS_EDITLABELS | TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDR_DEBUGGER_VIEW))
    {
        return -1;	      
    }

	// Create the image list used by the tree control.
	if (!m_imageList.Create (IDB_LAYERLIST_ICONS, 16, 1, RGB(0,255,0)))
		return -1;
	
	// Set the image list for the tree control.
	m_tcDebugger.SetImageList(&m_imageList, TVSIL_NORMAL);

	return 0;
}

//=========================================================================

void CEditorDebuggerView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

//=========================================================================

void CEditorDebuggerView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);

    CSize size = SizeToolBar(cx, cy);
    m_tcDebugger.MoveWindow(0,size.cy,cx,cy-size.cy);
}

//=========================================================================

void CEditorDebuggerView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::Debugger");
        RefreshView();
    }
}

//=========================================================================

void CEditorDebuggerView::OnGdtbGuidLookup() 
{
    g_WorldEditor.ReverseGuidLookup();
    m_QueryType = DQT_GUID;
    xstring xstrDebug;
    g_WorldEditor.GetDisplayNameForObject(g_WorldEditor.GetLookupGuid(), xstrDebug);
    m_strDebugItem = xstrDebug;
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    RefreshView();
}

//=========================================================================

void CEditorDebuggerView::OnUpdateGdtbGuidLookup(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen() && g_WorldEditor.CanPerformReverseLookup());
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorDebuggerView::OnGdtbClearInfo() 
{
    m_QueryType = DQT_NULL;
    g_WorldEditor.ClearGuidLookupList();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    RefreshView();
}

//=========================================================================

void CEditorDebuggerView::OnUpdateGdtbClearInfo(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen());
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorDebuggerView::OnGdtbGlobalGuidLookup() 
{
    CListBoxDlg dlg;
    dlg.SetDisplayText("Which Global do you want to perform a lookup upon?");

    xharray<var_mngr::global_def> hGlobalArray;
    g_VarMgr.GetGlobalsList(hGlobalArray);
    //we should now have all the current globals...
    for ( int i = 0; i < hGlobalArray.GetCount(); i++ )
    {
        var_mngr::global_def& Def = hGlobalArray[i];
        CString strGlobal(Def.Name.Get());
        dlg.AddString(strGlobal);
    }

    if (dlg.DoModal() == IDOK)
    {
        m_QueryType = DQT_GLOBAL;
        m_strDebugItem = dlg.GetSelection();
        g_WorldEditor.ReverseGlobalLookup(m_strDebugItem);
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        RefreshView();
    }
}

//=========================================================================

void CEditorDebuggerView::OnUpdateGdtbGlobalGuidLookup(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen());
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorDebuggerView::RefreshView()
{
    m_tcDebugger.DeleteAllItems();
    m_lstObjectGuidStorage.Clear();

    if (!g_Project.IsProjectOpen())
        return;

    if (m_QueryType == DQT_NULL)
        return;

    if (m_strDebugItem.IsEmpty())
        return;
    
    HTREEITEM hParent = NULL;
    HTREEITEM hChildren = NULL;
    HTREEITEM hAffecters = NULL;
    
    if (m_QueryType == DQT_GUID)
    {
        hParent= m_tcDebugger.InsertItem( m_strDebugItem, 2, 3, TVI_ROOT );

        //also worry about children for this type
        object* pObject = g_ObjMgr.GetObjectByGuid(g_WorldEditor.GetLookupGuid());
        if (pObject)
        {
            xhandle xh = AddObjectGuidToStorageArray( pObject->GetGuid() );
            m_tcDebugger.SetItemData(hParent, xh);

            prop_enum List;
            pObject->OnEnumProp( List );
            for( s32 i = 0; i < List.GetCount(); i++)
            {
                prop_enum::node& enData = List[i];

                if ((enData.GetType() & PROP_TYPE_BASIC_MASK) == PROP_TYPE_EXTERNAL )
                {
                    //its an external
                    if (enData.GetEnumCount() >= 2)
                    {
                        if ( (x_strcmp("global_all", xstring(enData.GetEnumType(1)) ) == 0) ||
                             (x_strcmp("global_guid", xstring(enData.GetEnumType(1)) ) == 0) )
                        {
                            //its a global
                            char       Temp[MAX_PATH];
                            prop_query pq;
                            pq.RQueryExternal( enData.GetName(), Temp ); 
                            if (pObject->OnProperty( pq ) )
                            {
                                if (!hChildren)
                                {
                                    hChildren = m_tcDebugger.InsertItem( "Things that I Affect", 8, 9, hParent );
                                    m_tcDebugger.SetItemData(hChildren, HNULL);
                                }

                                HTREEITEM hItem = m_tcDebugger.InsertItem( xfs("GLOBAL::%s",Temp), 20, 21, hChildren, TVI_SORT );
                                m_tcDebugger.SetItemData(hItem, HNULL);            
                            }
                        }
                    }
                }
                else if ((enData.GetType() & PROP_TYPE_BASIC_MASK) == PROP_TYPE_GUID )
                {
                    //its a guid
                    guid Guid;
                    prop_query pq;
                    pq.RQueryGUID( enData.GetName(), Guid ); 
                    if (pObject->OnProperty( pq ) )
                    {
                        if ( ( Guid != 0 ) && ( Guid != pObject->GetGuid() ) )
                        {
                            if (!hChildren)
                            {
                                hChildren = m_tcDebugger.InsertItem( "Things that I Affect", 8, 9, hParent );
                                m_tcDebugger.SetItemData(hChildren, HNULL);
                            }

                            xstring xstrDebug;
                            g_WorldEditor.GetDisplayNameForObject(Guid, xstrDebug);
                            HTREEITEM hItem = m_tcDebugger.InsertItem( xstrDebug, 2, 3, hChildren, TVI_SORT );
                            xhandle xh = AddObjectGuidToStorageArray( Guid );
                            m_tcDebugger.SetItemData(hItem, xh);
                        }
                    }
                }
            }
        }
        else
        {
            m_tcDebugger.SetItemData(hParent, HNULL);
        }
    }
    else if (m_QueryType == DQT_GLOBAL)
    {
        hParent= m_tcDebugger.InsertItem( m_strDebugItem, 10, 11, TVI_ROOT, TVI_SORT );
        xhandle xh = HNULL;
        xhandle tempHandle = HNULL;
        if( g_VarMgr.GetGuidHandle(m_strDebugItem,&tempHandle) )
        {
            xh = AddObjectGuidToStorageArray( g_VarMgr.GetGuid(tempHandle) );
        }         
        m_tcDebugger.SetItemData(hParent, xh);
    }

    xarray<guid>& GuidList = g_WorldEditor.GetGuidLookupList();
    if (GuidList.GetCount() > 0)
    {
        hAffecters = m_tcDebugger.InsertItem( "Things that Affect me", 8, 9, hParent );
        m_tcDebugger.SetItemData(hAffecters, HNULL);

        for (s32 i=0; i < GuidList.GetCount(); i++)
        {
            guid Guid = GuidList.GetAt(i);
            xstring xstrDebug;
            g_WorldEditor.GetDisplayNameForObject(Guid, xstrDebug);
            HTREEITEM hNewItem = m_tcDebugger.InsertItem( xstrDebug, 2, 3, hAffecters, TVI_SORT );
            xhandle xh = AddObjectGuidToStorageArray( Guid );
            m_tcDebugger.SetItemData(hNewItem, xh);
        }
    }

    m_tcDebugger.Expand(hParent, TVE_EXPAND);
    m_tcDebugger.Expand(hChildren, TVE_EXPAND);
    m_tcDebugger.Expand(hAffecters, TVE_EXPAND);
}

//=========================================================================

guid CEditorDebuggerView::GetObjectGuidFromHandle( xhandle xh )
{
    s32 i = m_lstObjectGuidStorage.GetIndexByHandle(xh);
    return m_lstObjectGuidStorage[i];
}

//=========================================================================

xhandle CEditorDebuggerView::AddObjectGuidToStorageArray( guid Guid )
{
    guid& GuidToAdd = m_lstObjectGuidStorage.Add();
    GuidToAdd = Guid;
    return FindHandleForObjectGuid( Guid );
}

//=========================================================================

xhandle CEditorDebuggerView::FindHandleForObjectGuid( guid Guid )
{
    xhandle hHandle;
    hHandle.Handle = -1;
    for( s32 i=0; m_lstObjectGuidStorage.GetCount(); i++ )
    {
        guid& GuidInList = m_lstObjectGuidStorage[i];

        if( GuidInList == Guid ) 
        {
            hHandle = m_lstObjectGuidStorage.GetHandleByIndex( i );
            break;
        }
    }

    return hHandle;
}

//=========================================================================

void CEditorDebuggerView::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

    xhandle hHandle = m_tcDebugger.GetItemData(pNMTreeView->itemNew.hItem);

    if (hHandle  != HNULL)
    {
        guid& ObjGuid = GetObjectGuidFromHandle(hHandle);
        if (ObjGuid.Guid != 0)
        {
            g_WorldEditor.SelectObject(ObjGuid, TRUE);  
            GetDocument()->GetFramePointer()->GetEditorView()->SetFocusPos(g_WorldEditor.GetMinPositionForSelected());
        }
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
    }
}
