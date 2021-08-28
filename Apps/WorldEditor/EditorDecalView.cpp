// EditorDecalView.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "worldeditor.hpp"
#include "EditorDecalView.h"
#include "EditorFrame.h"
#include "EditorView.h"
#include "..\Apps\Editor\Project.hpp"
#include "..\WinControls\FileSearch.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\EDRscDesc\RscDesc.hpp"
#include "static_decal.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorDecalView

IMPLEMENT_DYNCREATE(CEditorDecalView, CPaletteView)

CEditorDecalView::CEditorDecalView()
{
    m_bCanAdd = FALSE;

    ForceDecalLoaderLink();
}

CEditorDecalView::~CEditorDecalView()
{
}


BEGIN_MESSAGE_MAP(CEditorDecalView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorDecalView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_OBJECTS, OnSelchangeList)
    ON_COMMAND(ID_DVTB_PAINT_MODE, OnDvtbPaintMode)
	ON_COMMAND(ID_DVTB_SELECT_MODE, OnDvtbSelectMode)
	ON_COMMAND(ID_DVTB_REFRESH, OnDvtbRefresh)
	ON_UPDATE_COMMAND_UI(ID_DVTB_PAINT_MODE, OnUpdateDvtbPaintMode)
	ON_UPDATE_COMMAND_UI(ID_DVTB_SELECT_MODE, OnUpdateDvtbSelectMode)
	ON_UPDATE_COMMAND_UI(ID_DVTB_REFRESH, OnUpdateDvtbRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorDecalView drawing

void CEditorDecalView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CEditorDecalView diagnostics

#ifdef _DEBUG
void CEditorDecalView::AssertValid() const
{
	CPaletteView::AssertValid();
}

void CEditorDecalView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

void CEditorDecalView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::DecalWindow");
        LoadList();
    }
}

//=========================================================================

void CEditorDecalView::LoadList( void )
{
    m_bCanAdd = FALSE;
    m_rscTree.DeleteAllItems();
    
    //iterate through the resources
    CString StrType( "decalpkg" );
	for (int i =0; i < g_RescDescMGR.GetRscDescCount(); i++)
    {
        rsc_desc_mgr::node &Node = g_RescDescMGR.GetRscDescIndex(i);
        CString strType(Node.pDesc->GetType());
        
        if (strType.CompareNoCase(StrType)==0)
        {
            // This is a decal resource. Add it to the tree.
            CString strName(Node.pDesc->GetName());
            CString strPath(Node.pDesc->GetPath());

            AddPathToTree(strPath, strName);
        }
    }
}

//=========================================================================

void CEditorDecalView::AddPathToTree(CString& strPath, CString& strName)
{
    // allocate space in our path array for this guy
    xhandle  hHandle      = HNULL;
    CString& strPathAdded = m_xaPaths.Add(hHandle);

    // initialize the added path as our path from the resource mgr,
    // and reformat it using backslashes
    strPathAdded = strPath;
    CFileSearch::FormatPath(strPath); //strPath now in form "c:\gamedata\a51\temp"
    strPath += "\\"; //ensure a final slash

    // loop through the resource directories and see what theme this
    // path is part of
    char    cThemePath[MAX_PATH];
    CString strThemePath;
    s32     j;
    for( j = g_Project.GetFirstResourceDir( cThemePath );
         j != -1;
         j = g_Project.GetNextResourceDir( j, cThemePath ) )
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

    // strip the theme path off of the path (so instead of "themes\decals\resources\"
    // we just get \decals).
    if (!strThemePath.IsEmpty())
    {
        s32 nThemeLen = strThemePath.GetLength();
        CFileSearch::FormatPath(strThemePath);

        CString strThemeName;
        strThemeName = strThemePath.Left(strThemePath.ReverseFind('\\'));
        strThemeName = strThemeName.Right(strThemeName.GetLength() - strThemeName.ReverseFind('\\') - 1);

        strPath = strThemeName + strPath.Right( strPath.GetLength() - nThemeLen);
    }

    ASSERT(hHandle != HNULL);

    // find the item's parent (adding levels if necesary)
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
            //need to insert this path into the tree
            xhandle    TreeItemHandle;
            tree_data& PathData = m_TreeItems.Add( TreeItemHandle );
            PathData.Type = tree_data::TYPE_FOLDER;
            PathData.Data = HNULL;

            hParent = m_rscTree.InsertItem(strCurrent, 0, 1, hParent);
            m_rscTree.SetItemData(hParent, TreeItemHandle);
        }

        nIndex = strPath.Find('\\');
    }

    // insert the decal package into the tree
    xhandle     PackageHandle;
    tree_data&  PackageData = m_TreeItems.Add( PackageHandle );
    HTREEITEM   hPackage    = m_rscTree.InsertItem(strName, 22, 23, hParent);
    PackageData.Type = tree_data::TYPE_PACKAGE;
    PackageData.Data = hHandle;
    m_rscTree.SetItemData(hPackage, PackageHandle);

    // insert the groups into the tree
    rhandle<decal_package> DecalPackageRsc;
    DecalPackageRsc.SetName( m_rscTree.GetItemText(hPackage) );
    decal_package* pDecalPkg = DecalPackageRsc.GetPointer();
    if ( pDecalPkg )
    {
        s32 iGroup;
        for ( iGroup = 0; iGroup < pDecalPkg->GetNGroups(); iGroup++ )
        {
            CString GroupText;
            GroupText.Format( "%02d: %s", iGroup, pDecalPkg->GetGroupName(iGroup) );

            xhandle    GroupHandle;
            tree_data& GroupData = m_TreeItems.Add( GroupHandle );
            HTREEITEM  hGroup    = m_rscTree.InsertItem( GroupText, 24, 25, hPackage );
            GroupData.Type = tree_data::TYPE_GROUP;
            GroupData.Data = iGroup;
            m_rscTree.SetItemData( hGroup, GroupHandle );

            // insert the decal definitions into the tree
            s32 iDecalDef;
            for ( iDecalDef = 0; iDecalDef < pDecalPkg->GetNDecalDefs(iGroup); iDecalDef++ )
            {
                decal_definition& DecalDef = pDecalPkg->GetDecalDef(iGroup, iDecalDef);

                CString DecalText;
                DecalText.Format( "%02d: %s", iDecalDef, DecalDef.m_Name );

                xhandle    DecalHandle;
                tree_data& DecalData = m_TreeItems.Add( DecalHandle );
                HTREEITEM  hDecal    = m_rscTree.InsertItem( DecalText, 26, 27, hGroup );
                DecalData.Type = tree_data::TYPE_DECALDEF;
                DecalData.Data = iDecalDef;
                m_rscTree.SetItemData( hDecal, DecalHandle );
            }
        }
    }
}

//=========================================================================

HTREEITEM CEditorDecalView::DoesChildExist(const CString& strCurrent, HTREEITEM hParent)
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

/////////////////////////////////////////////////////////////////////////////
// CEditorDecalView message handlers

void CEditorDecalView::OnSelchangeList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
    xhandle hHandle = m_rscTree.GetItemData(pNMTreeView->itemNew.hItem);
    if ( hHandle != HNULL )
    {
        tree_data& TreeData = m_TreeItems(hHandle);
        if ( TreeData.Type == tree_data::TYPE_DECALDEF )
        {
            m_bCanAdd = TRUE;
        }
        else
        {
            m_bCanAdd = FALSE;
        }
    }
    else
    {
        m_bCanAdd = FALSE;
    }

	*pResult = 0;
}

//=========================================================================

int CEditorDecalView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_DECAL_PALETTE;
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    if (!m_rscTree.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                         TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDC_TREE_OBJECTS))
    {
        ASSERT(FALSE);
        return -1;
    }
	
	// Create the image list used by the tree control.
	if (!m_imageList.Create (IDB_LAYERLIST_ICONS, 16, 1, RGB(0,255,0)))
		return -1;

	// Set the image list for the tree control.
	m_rscTree.SetImageList(&m_imageList, TVSIL_NORMAL);

    // repopulate the tree view
    LoadList();

	return 0;
}

//=========================================================================

void CEditorDecalView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);

	CSize size = SizeToolBar(cx, cy);//m_wndToolBar.CalcLayout(LM_HORZ| LM_COMMIT,cx);
	m_wndToolBar.MoveWindow(0,0,cx,size.cy);

    m_rscTree.MoveWindow(0, size.cy, cx, cy-size.cy);
}

//=========================================================================

void CEditorDecalView::OnDvtbPaintMode() 
{
    HTREEITEM hItem = m_rscTree.GetSelectedItem();
    if (hItem)
    {
        xhandle hData = m_rscTree.GetItemData( hItem );
        if ( (hData != HNULL) && (m_TreeItems(hData).Type == tree_data::TYPE_DECALDEF) )
        {
            HTREEITEM hGroup       = m_rscTree.GetParentItem( hItem );
            HTREEITEM hPackage     = m_rscTree.GetParentItem( hGroup );
            xhandle   hGroupData   = m_rscTree.GetItemData( hGroup );
            xhandle   hPackageData = m_rscTree.GetItemData( hPackage );
            ASSERT( (hGroupData != HNULL) && (m_TreeItems(hGroupData).Type == tree_data::TYPE_GROUP) );
            ASSERT( (hPackageData != HNULL) && (m_TreeItems(hPackageData).Type == tree_data::TYPE_PACKAGE) );

            g_WorldEditor.CreateTemporaryObject( static_decal::GetObjectType().GetTypeName() );
            g_WorldEditor.SetTempObjectExternal("StaticDecal\\DecalPackage", m_rscTree.GetItemText(hPackage));
            g_WorldEditor.SetTempObjectInt("StaticDecal\\GroupIndex", m_TreeItems(hGroupData).Data);
            g_WorldEditor.SetTempObjectInt("StaticDecal\\DecalIndex", m_TreeItems(hData).Data);
            GetDocument()->GetFramePointer()->GetPropertyEditorDoc()->Refresh();
            GetDocument()->GetFramePointer()->GetEditorView()->EnterDecalPlacementMode();
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
            GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
        }
    }
}

//=========================================================================

void CEditorDecalView::OnDvtbSelectMode() 
{
	// TODO: Add your command handler code here
	
}

//=========================================================================

void CEditorDecalView::OnDvtbRefresh() 
{
    LoadList();
}

//=========================================================================

void CEditorDecalView::OnUpdateDvtbPaintMode(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateDvtbPaintMode(pCmdUI);
    }
}

//=========================================================================

void CEditorDecalView::OnUpdateDvtbSelectMode(CCmdUI* pCmdUI) 
{
    //if ( GetDocument() && GetDocument()->GetFramePointer())
    //{
    //    GetDocument()->GetFramePointer()->OnUpdateDvtbSelectMode(pCmdUI);
    //}
}

//=========================================================================

void CEditorDecalView::OnUpdateDvtbRefresh(CCmdUI* pCmdUI) 
{
    if ( GetDocument() && GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnUpdateDvtbRefresh(pCmdUI);
    }
}
