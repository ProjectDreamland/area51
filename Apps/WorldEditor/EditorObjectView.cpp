// EditorObjectView.cpp : implementation file
//

#include "StdAfx.h"
#include "EditorObjectView.h"
#include "EditorPaletteDoc.h"
#include "EditorFrame.h"
#include "EditorView.h"
#include "EditorDoc.h"
#include "ResourceBrowserDlg.h"
#include "resource.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\EDRscDesc\RscDesc.hpp"
#include "..\WinControls\FileSearch.h"
#include "..\Editor\Project.hpp"
#include "..\Editor\resource.h"
#include "WorldEditor.hpp"
#include "objects\PlaySurface.hpp"
#include "objects\PropSurface.hpp"
#include "objects\AnimSurface.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//=========================================================================
// CEditorObjectView
//=========================================================================

IMPLEMENT_DYNCREATE(CEditorObjectView, CPaletteView)

CEditorObjectView::CEditorObjectView() 
{
    m_bCanAdd = FALSE;
}

//=========================================================================

CEditorObjectView::~CEditorObjectView()
{
}

//=========================================================================


BEGIN_MESSAGE_MAP(CEditorObjectView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorObjectView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_OBJECTS, OnSelchangeList)
	ON_COMMAND(ID_OVTB_ADD_PLAYSURFACE, OnOvtbAddPlaysurface)
	ON_COMMAND(ID_OVTB_ADD_PROPSURFACE, OnOvtbAddPropsurface)
	ON_COMMAND(ID_OVTB_ADD_ANIMSURFACE, OnOvtbAddAnimsurface)
	ON_COMMAND(ID_OVTB_REFRESH, OnOvtbRefresh)
	ON_COMMAND(ID_OVTB_UPDATE_GEOMS_FROM_SEL, OnOvtbUpdateGeomsFromSel)
	ON_COMMAND(ID_OVTB_UPDATE_RIGIDINST_FROM_SEL, OnOvtbUpdateRigidInst)
	ON_COMMAND(ID_OVTB_SELECT_BASED_ON_RIGID, OnOvtbSelectBasedOnRigidInst)
    ON_UPDATE_COMMAND_UI(ID_OVTB_ADD_PLAYSURFACE, OnUpdateOvtbAddPlaysurface)
	ON_UPDATE_COMMAND_UI(ID_OVTB_ADD_PROPSURFACE, OnUpdateOvtbAddPropsurface)
	ON_UPDATE_COMMAND_UI(ID_OVTB_ADD_ANIMSURFACE, OnUpdateOvtbAddAnimsurface)
	ON_UPDATE_COMMAND_UI(ID_OVTB_REFRESH, OnUpdateOvtbRefresh)
    ON_UPDATE_COMMAND_UI(ID_OVTB_UPDATE_GEOMS_FROM_SEL, OnUpdateOvtbUpdateGeomsFromSel)
    ON_UPDATE_COMMAND_UI(ID_OVTB_UPDATE_RIGIDINST_FROM_SEL, OnUpdateOvtbUpdateRigidInst)    
    ON_UPDATE_COMMAND_UI(ID_OVTB_SELECT_BASED_ON_RIGID, OnUpdateOvtbSelectBasedOnRigidInst)    
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//=========================================================================
// CEditorObjectView drawing
//=========================================================================

void CEditorObjectView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//=========================================================================
// CEditorObjectView diagnostics
//=========================================================================

#ifdef _DEBUG
void CEditorObjectView::AssertValid() const
{
	CPaletteView::AssertValid();
}

//=========================================================================

void CEditorObjectView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================
// CEditorObjectView message handlers
//=========================================================================

int CEditorObjectView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_OBJECT_PALETTE;
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_rscTree.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                         TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDC_TREE_OBJECTS))
    {
        ASSERT(FALSE);
        return -1;
    }

    if (!m_wndPreview.Create(_T("STATIC"),"PreviewWnd", WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, IDC_STATIC))
    {
        ASSERT(FALSE);
        return -1;
    }

    if (!m_stPlaceholder.Create("",WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_ICON,
                         CRect(0,0,0,0),this,IDC_STATIC))
    {
        ASSERT(FALSE);
		return -1;      // fail to create
    }

	// Create the image list used by the tree control.
	if (!m_imageList.Create (IDB_LAYERLIST_ICONS, 16, 1, RGB(0,255,0)))
		return -1;
	
	// Set the image list for the tree control.
	m_rscTree.SetImageList(&m_imageList, TVSIL_NORMAL);

    m_strType = "RigidGeom";
    LoadList();

	return 0;
}

//=========================================================================

void CEditorObjectView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

//=========================================================================

void CEditorObjectView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);
    int nHt = cx/2;

	CSize size = m_wndToolBar.CalcLayout(LM_HORZ| LM_COMMIT,nHt);
	m_wndToolBar.MoveWindow(0,0,size.cx,size.cy);

    if (nHt > (cy/2)) nHt = cy/2;

    m_wndPreview.MoveWindow( size.cx, 0, cx - size.cx, nHt);

    if (size.cy > nHt) 
    {
        m_stPlaceholder.MoveWindow( size.cx, nHt, cx - size.cx, size.cy - nHt );
        nHt = size.cy;
    }
    else
    {
        m_stPlaceholder.MoveWindow( 0, size.cy, size.cx, nHt - size.cy );
    }

    m_rscTree.MoveWindow(0,nHt,cx,cy - nHt);
}

//=========================================================================

void CEditorObjectView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::Geoms");
        LoadList();
    }
    else
    {
        m_wndPreview.OnStopTimer();
    }
}

//=========================================================================

void CEditorObjectView::OnSelchangeList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
    xhandle hHandle = m_rscTree.GetItemData(pNMTreeView->itemNew.hItem);

    if (hHandle  != HNULL)
    {
        CWaitCursor wc;

        CString& strData = m_xaPaths(hHandle);
        CString strPath = strData + m_rscTree.GetItemText(pNMTreeView->itemNew.hItem);

        m_wndPreview.LoadGeom(m_strType, strPath);
        m_wndPreview.OnStartTimer();
        m_bCanAdd = TRUE;
    }
    else
    {
        m_wndPreview.ClearGeom();
        m_bCanAdd = FALSE;
    }

	*pResult = 0;
}

//=========================================================================

void CEditorObjectView::LoadList()
{
    m_bCanAdd = FALSE;
    m_rscTree.DeleteAllItems();
    
    //iterate through the resources
	for (int i =0; i < g_RescDescMGR.GetRscDescCount(); i++)
    {
        rsc_desc_mgr::node &Node = g_RescDescMGR.GetRscDescIndex(i);
        CString strType(Node.pDesc->GetType());
        
        if (strType.CompareNoCase(m_strType)==0)
        {
            CString strName(Node.pDesc->GetName());
            CString strPath(Node.pDesc->GetPath());

            AddPathToTree(strPath, strName);
        }
    }
}

//=========================================================================

void CEditorObjectView::AddPathToTree(CString strPath, CString strName)
{
    xhandle hHandle = HNULL;
    CString& strPathAdded = m_xaPaths.Add(hHandle);
    strPathAdded = strPath;
    CFileSearch::FormatPath(strPath); //strPath now in form "c:\gamedata\a51\temp"
    strPath += "\\"; //ensure a final slash

    char cThemePath[MAX_PATH];
    CString strThemePath;
    for( int j = g_Project.GetFirstResourceDir( cThemePath ); j != -1; j = g_Project.GetNextResourceDir( j, cThemePath ) )
    {
        if (strPath.Find(cThemePath) != -1)
        {
            //found base path
            CString strNewPath(cThemePath);
            if (strNewPath.GetLength() > strThemePath.GetLength())
            {
                //we want the longest match here
                strThemePath = strNewPath;
            }
        }
    }

    if (!strThemePath.IsEmpty())
    {
        int nThemeLen = strThemePath.GetLength();
        CFileSearch::FormatPath(strThemePath);

        CString strThemeName;
        strThemeName = strThemePath.Left(strThemePath.ReverseFind('\\'));
        strThemeName = strThemeName.Right(strThemeName.GetLength() - strThemeName.ReverseFind('\\') - 1);

        strPath = strThemeName + strPath.Right( strPath.GetLength() - nThemeLen);
    }

    ASSERT(hHandle != HNULL);

    int nIndex = strPath.Find('\\');
    HTREEITEM hParent = TVI_ROOT;
    while (nIndex!=-1)
    {
        CString strCurrent = strPath.Left(nIndex);
        strPath = strPath.Right ( strPath.GetLength() - nIndex - 1);

        //does strCurrent exist at this level?
        HTREEITEM hCurrent = DoesChildExist(strCurrent, hParent);
        if ( hCurrent )
        {
            hParent = hCurrent;
        }
        else
        {
            //need to insert this item
            hParent = m_rscTree.InsertItem(strCurrent, 0, 1, hParent);
            m_rscTree.SetItemData(hParent, HNULL);
        }

        nIndex = strPath.Find('\\');
    }

    HTREEITEM hItem = m_rscTree.InsertItem(strName, 2, 3, hParent);
    m_rscTree.SetItemData(hItem, hHandle);
}

//=========================================================================

HTREEITEM CEditorObjectView::DoesChildExist(CString strCurrent, HTREEITEM hParent)
{
    HTREEITEM hNextItem;
    HTREEITEM hChildItem;
    
    if (hParent == TVI_ROOT)
    {
        hChildItem = m_rscTree.GetRootItem();
        if (!hChildItem)
        {
            return NULL;
        }
    }
    else
    {
        if (m_rscTree.ItemHasChildren(hParent))
        {   
            hChildItem = m_rscTree.GetChildItem(hParent);
        }
        else
        {
            return NULL;
        }
    }

    while (hChildItem != NULL)
    {
        hNextItem = m_rscTree.GetNextSiblingItem(hChildItem);
        if (strCurrent.CompareNoCase(m_rscTree.GetItemText(hChildItem)) == 0)
        {
            //Found it!
            return hChildItem;
        }
        hChildItem = hNextItem;
    }

    return NULL;
}

//=========================================================================

void CEditorObjectView::OnOvtbAddPlaysurface()
{
    HTREEITEM hItem = m_rscTree.GetSelectedItem();
    if (hItem)
    {
        g_WorldEditor.CreateTemporaryObject( play_surface::GetObjectType().GetTypeName() );
        g_WorldEditor.SetTempObjectExternal("RenderInst\\File", m_rscTree.GetItemText(hItem));
        g_WorldEditor.MoveTemporaryObjects(GetDocument()->GetFramePointer()->GetEditorView()->GetFocusPos());

        GetDocument()->GetFramePointer()->GetPropertyEditorDoc()->Refresh();

        GetDocument()->GetFramePointer()->GetEditorView()->EnterPlacementMode();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
    }
}

//=========================================================================

void CEditorObjectView::OnOvtbAddPropsurface()
{
    HTREEITEM hItem = m_rscTree.GetSelectedItem();
    if (hItem)
    {
        g_WorldEditor.CreateTemporaryObject( prop_surface::GetObjectType().GetTypeName() );
        g_WorldEditor.SetTempObjectExternal("RenderInst\\File", m_rscTree.GetItemText(hItem));
        g_WorldEditor.MoveTemporaryObjects(GetDocument()->GetFramePointer()->GetEditorView()->GetFocusPos());

        GetDocument()->GetFramePointer()->GetPropertyEditorDoc()->Refresh();

        GetDocument()->GetFramePointer()->GetEditorView()->EnterPlacementMode();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
    }
}

//=========================================================================

void CEditorObjectView::OnOvtbAddAnimsurface()
{
    HTREEITEM hItem = m_rscTree.GetSelectedItem();
    if (hItem)
    {
        g_WorldEditor.CreateTemporaryObject( anim_surface::GetObjectType().GetTypeName() );
        g_WorldEditor.SetTempObjectExternal("RenderInst\\File", m_rscTree.GetItemText(hItem));
        g_WorldEditor.MoveTemporaryObjects(GetDocument()->GetFramePointer()->GetEditorView()->GetFocusPos());

        GetDocument()->GetFramePointer()->GetPropertyEditorDoc()->Refresh();

        GetDocument()->GetFramePointer()->GetEditorView()->EnterPlacementMode();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
    }
}

//=========================================================================

void CEditorObjectView::OnOvtbRefresh()
{
    m_wndPreview.ClearGeom();
    LoadList();
}

//=========================================================================

BOOL CEditorObjectView::CanAdd()
{
    return m_bCanAdd;
}

//=========================================================================

void CEditorObjectView::OnOvtbUpdateGeomsFromSel()
{
    if (g_WorldEditor.GetSelectedCount() == 1)
    {
        guid ObjGuid = g_WorldEditor.GetSelectedObjectsByIndex( 0 );
        xarray<guid> lstGuids;
        g_WorldEditor.GetListOfMatchingGeoms(ObjGuid, lstGuids);
        if (lstGuids.GetCount() > 0)
        {
            if (::AfxMessageBox(xfs("%d similar %s(s) were found, are you sure you want to update these?",
                                    lstGuids.GetCount(),g_WorldEditor.GetObjectTypeName(ObjGuid)), MB_YESNO) == IDYES)
            {
                g_WorldEditor.UpdateListOfGeoms(ObjGuid, lstGuids);
                GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
            }
        }
        else
        {
            ::AfxMessageBox("No similar Geoms could be found.");
        }
    }
}

//=========================================================================

void CEditorObjectView::OnOvtbUpdateRigidInst()
{
    if (g_WorldEditor.GetSelectedCount() == 1)
    {
        guid ObjGuid = g_WorldEditor.GetSelectedObjectsByIndex( 0 );
        xarray<guid> lstGuids;
        lstGuids.Append(ObjGuid); //must include initial guid
        g_WorldEditor.GetListOfMatchingGeoms(ObjGuid, lstGuids);
        if (::AfxMessageBox(xfs("%d %s(s) were found, are you sure you want to change the rigid instance these objects point to?",
                                lstGuids.GetCount(),g_WorldEditor.GetObjectTypeName(ObjGuid)), MB_YESNO) == IDYES)
        {
            CResourceBrowserDlg dlg;
            dlg.SetType("RigidGeom");
            if (dlg.DoModal() == IDOK)
            {
                g_WorldEditor.UpdateRigidInstances(lstGuids, dlg.GetName());
                GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
            }
        }
    }
}

//=========================================================================

void CEditorObjectView::OnOvtbSelectBasedOnRigidInst()
{
    if (g_WorldEditor.GetSelectedCount() == 1)
    {
        CWaitCursor wc;
        guid ObjGuid = g_WorldEditor.GetSelectedObjectsByIndex( 0 );
        s32 nCount = g_WorldEditor.SelectMatchingGeomObjects(ObjGuid);

        if (nCount == -1)
        {
            ::AfxMessageBox("The object you had selected did not have a RenderInst\\File property, so no objects could be selected.");
        }
        else if (nCount == 0)
        {
            ::AfxMessageBox("The object you had selected was non-editable and no editable objects matched the render inst.");
        }

        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
    }
}

//=========================================================================

void CEditorObjectView::OnUpdateOvtbAddPlaysurface(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateOvtbAddPlaysurface(pCmdUI);
    }
}

//=========================================================================

void CEditorObjectView::OnUpdateOvtbAddPropsurface(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateOvtbAddPropsurface(pCmdUI);
    }
}

//=========================================================================

void CEditorObjectView::OnUpdateOvtbAddAnimsurface(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateOvtbAddAnimsurface(pCmdUI);
    }
}

//=========================================================================

void CEditorObjectView::OnUpdateOvtbUpdateRigidInst(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateOvtbUpdateRigidInst(pCmdUI);
    }
}

//=========================================================================

void CEditorObjectView::OnUpdateOvtbUpdateGeomsFromSel(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateOvtbUpdateGeomsFromSel(pCmdUI);
    }
}

//=========================================================================

void CEditorObjectView::OnUpdateOvtbRefresh(CCmdUI* pCmdUI)
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateOvtbRefresh(pCmdUI);
    }
}

//=========================================================================

void CEditorObjectView::OnUpdateOvtbSelectBasedOnRigidInst(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;
    if (g_Project.IsProjectOpen() &&                                //project open
        GetDocument() && GetDocument()->GetFramePointer() && 
        GetDocument()->GetFramePointer()->GetEditorView()->IsStandardMode() &&
        !GetDocument()->GetFramePointer()->GetEditorDoc()->IsGameRunning() )   
    {
        if (g_WorldEditor.GetSelectedCount() == 1)
        {
            bEnable = TRUE;	
        }
    }

    pCmdUI->Enable(bEnable);	
    pCmdUI->SetCheck(FALSE);	
}
