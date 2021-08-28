// EditorLayerView.cpp : implementation file
//

#include "StdAfx.h"
#include "EditorLayerView.h"
#include "EditorPaletteDoc.h"
#include "WorldEditor.hpp"
#include "EditorFrame.h"
#include "EditorDoc.h"
#include "EditorView.h"
#include "resource.h"
#include "transaction_mgr.hpp"
#include "Auxiliary\MiscUtils\guid.hpp"
#include "..\Editor\Project.hpp"
#include "..\Apps\WorldEditor\WorldEditor.hpp"
#include "ZoneMgr/ZoneMgr.hpp"

#include "Objects\Portal.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================

#define IMG_LIST_LAYER_UNLOADED     0
#define IMG_LIST_OBJECT             2
#define IMG_LIST_BLUEPRINT          4
#define IMG_LIST_LAYER_LOADED       6
#define IMG_LIST_FOLDER             8
#define IMG_LIST_RESOURCE           10
#define IMG_LIST_ZONE               12
#define IMG_LIST_PORTAL             14
#define IMG_LIST_ZONELINK_FOLDER    18

//=========================================================================

extern user_settings            g_SaveTrackUserSettings;
extern user_settings            g_LoadUpdateUserSettings;

// Ascending date sorting function
class CompareNamesForSort : public x_compare_functor<const xstring&>
{
public:
    s32 operator()( const xstring& A, const xstring& B )
    {
        CString strItem1 = A;
        CString strItem2 = B;
        strItem1.MakeUpper();
        strItem2.MakeUpper();
        if( strItem1 < strItem2 )   return( -1 );
        if( strItem1 > strItem2 )   return(  1 );
        else return(  0 );
    }
};

//=========================================================================

class CompareObjectsForSort : public x_compare_functor<const editor_object_ref&>
{
public:
    s32 operator()( const editor_object_ref& A, const editor_object_ref& B )
    {
        CString strItem1 = A.LayerPath;
        CString strItem2 = B.LayerPath;
        strItem1.MakeUpper();
        strItem2.MakeUpper();
        if( strItem1 < strItem2 )   return( -1 );
        if( strItem1 > strItem2 )   return(  1 );
        else return(  0 );
    }
};

//=========================================================================

class CompareBlueprintsForSort : public x_compare_functor<const editor_blueprint_ref&>
{
public:
    s32 operator()( const editor_blueprint_ref& A, const editor_blueprint_ref& B )
    {
        CString strItem1 = A.LayerPath;
        CString strItem2 = B.LayerPath;

        strItem1.MakeUpper();
        strItem2.MakeUpper();
        if( strItem1 < strItem2 )   return( -1 );
        if( strItem1 > strItem2 )   return(  1 );
        else return(  0 );
    }
};

//=========================================================================
// CEditorLayerView
//=========================================================================

IMPLEMENT_DYNCREATE(CEditorLayerView, CPaletteView)

CEditorLayerView::CEditorLayerView()
{
    m_bInternalSelect = FALSE;
    m_SelectionType = -1;
    m_SelectionItem = NULL;
    m_Guid = NULL;

}

//=========================================================================

CEditorLayerView::~CEditorLayerView()
{
	m_imageList.DeleteImageList();
}


//=========================================================================

BEGIN_MESSAGE_MAP(CEditorLayerView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorLayerView)
	ON_WM_CREATE()
	ON_WM_SIZE()
    ON_NOTIFY (TVN_ENDLABELEDIT, IDR_LAYER_LIST, OnEndLabelEdit   )
    ON_COMMAND(ID_LVTB_NEW_LAYER, OnLvtbAddLayer)
    ON_COMMAND(ID_LVTB_NEW_FOLDER, OnLvtbAddFolder)
    ON_COMMAND(ID_LVTB_NEW_RES, OnLvtbAddResource)
    ON_COMMAND(ID_LVTB_NEW_ZONE, OnLvtbAddZone)
    ON_COMMAND(ID_LVTB_NEW_PORTAL, OnLvtbAddPortal)
    ON_COMMAND(ID_LVTB_DELETE_LAYER, OnLvtbDeleteLayer)
    ON_COMMAND(ID_LVTB_RENAME_LAYER, OnLvtbRenameLayer)
    ON_COMMAND(ID_LVTB_SET_ACTIVE_LAYER, OnLvtbSetAsActiveLayer)
    ON_COMMAND(ID_LVTB_LIGHT_LAYER, OnLvtbLightLayer)
    ON_COMMAND(ID_LVTB_SELECT_LAYER, OnLvtbSelectLayer)
    ON_COMMAND(ID_LVTB_LOAD_LAYER, OnLvtbLoadLayer)
    ON_COMMAND(ID_LVTB_LOAD_ALL_LAYERS, OnLvtbLoadAllLayers)
    ON_COMMAND(ID_LVTB_CHECKOUT_LAYER, OnLvtbCheckOutLayer)
	ON_NOTIFY (TVN_SELCHANGED, IDR_LAYER_LIST, OnSelchangedTree)
    ON_NOTIFY (NM_RCLICK, IDR_LAYER_LIST, OnNMRclick )
    ON_NOTIFY (NM_DBLCLK, IDR_LAYER_LIST, OnNMDblClick )
    ON_COMMAND(ID_LVRM_HIDE_THIS, OnLvrmHideThis)
    ON_COMMAND(ID_LVRM_SHOW_THIS, OnLvrmShowThis)
    ON_COMMAND(ID_LVRM_THAW_THIS, OnLvrmThawThis)
    ON_COMMAND(ID_LVRM_FREEZE_THIS, OnLvrmFreezeThis)
    ON_COMMAND(ID_LVRM_THAWALL_THIS, OnLvrmThawAllThis)
    ON_COMMAND(ID_LVRM_FREEZEALL_THIS, OnLvrmFreezeAllThis)
    ON_COMMAND(ID_LVRM_HIDE_ALL, OnLvrmHideAll)
    ON_COMMAND(ID_LVRM_SHOW_ALL, OnLvrmShowAll)
    ON_COMMAND(ID_LVRM_THAW_ALL, OnLvrmThawAll)
    ON_COMMAND(ID_LVRM_FREEZE_ALL, OnLvrmFreezeAll)
    ON_COMMAND(ID_LVRM_SELECT_ALL, OnLvrmSelectAll)
    ON_COMMAND(ID_LVRM_HIDE_ANIM_THIS, OnLvrmHideThisObjectAnimation)
    ON_COMMAND(ID_LVRM_HIDE_RIGID_THIS, OnLvrmHideThisObjectRigid)
    ON_COMMAND(ID_LVRM_SHOW_ANIM_THIS, OnLvrmShowThisObjectAnimation)
    ON_COMMAND(ID_LVRM_SHOW_RIGID_THIS, OnLvrmShowThisObjectRigid)
    ON_COMMAND(ID_LVRM_THAW_ANIM_THIS, OnLvrmThawThisObjectAnimation)
    ON_COMMAND(ID_LVRM_THAW_RIGID_THIS, OnLvrmThawThisObjectRigid)
    ON_COMMAND(ID_LVRM_FREEZE_ANIM_THIS, OnLvrmFreezeThisObjectAnimation)
    ON_COMMAND(ID_LVRM_FREEZE_RIGID_THIS, OnLvrmFreezeThisObjectRigid)
    ON_COMMAND(ID_LVRM_SELECTALL_THIS, OnLvrmSelectAllThisObject)
    ON_COMMAND(ID_LVRM_SELECTALL_RIGID_THIS, OnLvrmSelectAllThisObjectRigid)
    ON_COMMAND(ID_LVRM_SELECTALL_ANIM_THIS, OnLvrmSelectAllThisObjectAnimation)
    ON_COMMAND(ID_LVRM_HIDEALL_THIS, OnLvrmHideAllThis)
    ON_COMMAND(ID_LVRM_SHOWALL_THIS, OnLvrmShowAllThis)

    ON_UPDATE_COMMAND_UI(ID_LVTB_NEW_LAYER, OnUpdateLvtbAddLayer)
    ON_UPDATE_COMMAND_UI(ID_LVTB_NEW_FOLDER, OnUpdateLvtbAddFolder)
    ON_UPDATE_COMMAND_UI(ID_LVTB_NEW_RES, OnUpdateLvtbAddResource)
    ON_UPDATE_COMMAND_UI(ID_LVTB_NEW_ZONE, OnUpdateLvtbAddZone)
    ON_UPDATE_COMMAND_UI(ID_LVTB_NEW_PORTAL, OnUpdateLvtbAddPortal)
    ON_UPDATE_COMMAND_UI(ID_LVTB_DELETE_LAYER, OnUpdateLvtbDeleteLayer)
    ON_UPDATE_COMMAND_UI(ID_LVTB_RENAME_LAYER, OnUpdateLvtbRenameLayer)
    ON_UPDATE_COMMAND_UI(ID_LVTB_SET_ACTIVE_LAYER, OnUpdateLvtbSetAsActiveLayer)
    ON_UPDATE_COMMAND_UI(ID_LVTB_LIGHT_LAYER, OnUpdateLvtbLightLayer)
    ON_UPDATE_COMMAND_UI(ID_LVTB_SELECT_LAYER, OnUpdateLvtbSelectLayer)
    ON_UPDATE_COMMAND_UI(ID_LVTB_LOAD_LAYER, OnUpdateLvtbLoadLayer)
    ON_UPDATE_COMMAND_UI(ID_LVTB_LOAD_ALL_LAYERS, OnUpdateLvtbLoadAllLayers)
    ON_UPDATE_COMMAND_UI(ID_LVTB_CHECKOUT_LAYER, OnUpdateLvtbCheckOutLayer)
	//}}AFX_MSG_MAP
    ON_COMMAND_RANGE(IDR_LEVEL_LIGHT_DIR, IDR_LEVEL_LIGHT_RAYCAST, OnLvtbLightLayerType)
END_MESSAGE_MAP()

//=========================================================================
// CEditorLayerView drawing
//=========================================================================

void CEditorLayerView::OnDraw(CDC* pDC)
{
}

//=========================================================================
// CEditorLayerView diagnostics
//=========================================================================

#ifdef _DEBUG
void CEditorLayerView::AssertValid() const
{
	CPaletteView::AssertValid();
}

//=========================================================================

void CEditorLayerView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================
// CEditorLayerView message handlers
//=========================================================================

int CEditorLayerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_LAYER_VIEW_BAR;
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_tcLayer.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                          TVS_EDITLABELS | TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDR_LAYER_LIST))
    {
        return -1;	      
    }

	// Create the image list used by the tree control.
	if (!m_imageList.Create (IDB_LAYERLIST_ICONS, 16, 1, RGB(0,255,0)))
		return -1;
	
	// Set the image list for the tree control.
	m_tcLayer.SetImageList(&m_imageList, TVSIL_NORMAL);

	return 0;
}

//=========================================================================

void CEditorLayerView::LoadLayers()
{
    m_tcLayer.SetRedraw( 0 );

    m_tcLayer.DeleteAllItems();
    m_lstObjectGuidStorage.Clear();

    xarray<xstring> ListLayers;
    g_WorldEditor.GetLayerNames(ListLayers);

    x_qsort( &ListLayers[0], ListLayers.GetCount(), CompareNamesForSort() );

    for (int i = 0; i < ListLayers.GetCount(); i++)
	{
        CString strItem = ListLayers.GetAt(i);
        HTREEITEM hRoot = NULL;
        BOOL bLoaded   = g_WorldEditor.IsLayerLoaded(strItem);

        if (bLoaded)
        {
            hRoot = m_tcLayer.InsertItem( strItem, IMG_LIST_LAYER_LOADED, IMG_LIST_LAYER_LOADED+1, TVI_ROOT, TVI_SORT );
        }
        else
        {
            hRoot = m_tcLayer.InsertItem( strItem, IMG_LIST_LAYER_UNLOADED, IMG_LIST_LAYER_UNLOADED+1, TVI_ROOT, TVI_SORT );
        }
    
        SetLayerDirtyColor(strItem, hRoot);

        //add zones
        xarray<xstring> ListZones;
        g_WorldEditor.GetZoneListForLayer(strItem, ListZones);
        for (int k = 0; k<ListZones.GetCount(); k++)
        {
            HTREEITEM hZone = m_tcLayer.InsertItem( ListZones.GetAt(k), IMG_LIST_ZONE, IMG_LIST_ZONE+1, hRoot, TVI_SORT );
            m_tcLayer.SetItemData(hZone, 0);
            m_tcLayer.SetItemColor(hZone,RGB(0,0,0));
        }

        m_tcLayer.SetItemData(hRoot, 0);

        CString strLastLayerPath;
        HTREEITEM hParent = NULL;

        //add objects
        xarray<editor_object_ref> ListObjects;
        if (g_WorldEditor.GetObjectsInLayer(strItem,ListObjects))
        {
            x_qsort( &ListObjects[0], ListObjects.GetCount(), CompareObjectsForSort() );

            //now add all sorted objects as fast as we can
            for (int j = 0; j < ListObjects.GetCount(); j++)
	        {
                editor_object_ref ObjectRef = ListObjects.GetAt(j);
                CString strLayerPath(ObjectRef.LayerPath);
                if (hParent && (strLastLayerPath == strLayerPath))
                {
                    //hey, last item is good to go, just use it
                    AddObjectToTree(hParent, ObjectRef.Guid);
                }
                else
                {
                    HTREEITEM hItem = AddObjectToLayer(strItem, strLayerPath, ObjectRef.Guid);
                    if (hItem) 
                        hParent = m_tcLayer.GetParentItem(hItem);
                    else
                        hParent = NULL;
                }
                strLastLayerPath = strLayerPath;
            }
        }

        //add blueprints
        xarray<editor_blueprint_ref> ListBlueprints;
        if (g_WorldEditor.GetBlueprintsInLayer(strItem,ListBlueprints))
        {
            x_qsort( &ListBlueprints[0], ListBlueprints.GetCount(), CompareBlueprintsForSort() );

            //now add all sorted blueprints as fast as we can
            for (int j = 0; j < ListBlueprints.GetCount(); j++)
	        {
                editor_blueprint_ref BPRef = ListBlueprints.GetAt(j);
                CString strLayerPath(BPRef.LayerPath);
                if (hParent && (strLastLayerPath == strLayerPath))
                {
                    //hey, last item is good to go, just use it
                    AddBlueprintToTree(hParent, BPRef.Guid);
                }
                else
                {
                    HTREEITEM hItem = AddBlueprintToLayer(strItem, strLayerPath, BPRef.Guid);
                    if (hItem) 
                        hParent = m_tcLayer.GetParentItem(hItem);
                    else
                        hParent = NULL;
                }
                strLastLayerPath = strLayerPath;
            }
        }

        //add resources
        xarray<xstring> ListResources;
        if (g_WorldEditor.GetResourcesInLayer(strItem,ListResources))
        {
            for (int j = 0; j < ListResources.GetCount(); j++)
	        {
                xstring xstrData = ListResources.GetAt(j);
                VERIFY(AddResourceToLayer(strItem, "\\.Res\\", CString((const char*)xstrData)) != NULL);
            }
        }
    }

    m_tcLayer.SetRedraw( 1 );
    m_tcLayer.RedrawWindow();
}

//=========================================================================

guid CEditorLayerView::GetObjectGuidFromHandle( xhandle xh )
{
    s32 i = m_lstObjectGuidStorage.GetIndexByHandle(xh);
    return m_lstObjectGuidStorage[i];
}

//=========================================================================

xhandle CEditorLayerView::AddObjectGuidToStorageArray( guid Guid )
{
    guid& GuidToAdd = m_lstObjectGuidStorage.Add();
    GuidToAdd = Guid;
    return FindHandleForObjectGuid( Guid );
}

//=========================================================================

xhandle CEditorLayerView::FindHandleForObjectGuid( guid Guid )
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

guid CEditorLayerView::GetBlueprintGuidFromHandle( xhandle xh )
{
    s32 i = m_lstBlueprintGuidStorage.GetIndexByHandle(xh);
    return m_lstBlueprintGuidStorage[i];
}

//=========================================================================

xhandle CEditorLayerView::AddBlueprintGuidToStorageArray( guid Guid )
{
    guid& GuidToAdd = m_lstBlueprintGuidStorage.Add();
    GuidToAdd = Guid;
    return FindHandleForBlueprintGuid( Guid );
}

//=========================================================================

xhandle CEditorLayerView::FindHandleForBlueprintGuid( guid Guid )
{
    xhandle hHandle;
    hHandle.Handle = -1;
    for( s32 i=0; m_lstBlueprintGuidStorage.GetCount(); i++ )
    {
        guid& GuidInList = m_lstBlueprintGuidStorage[i];

        if( GuidInList == Guid ) 
        {
            hHandle = m_lstBlueprintGuidStorage.GetHandleByIndex( i );
            break;
        }
    }

    return hHandle;
}

//=========================================================================

void CEditorLayerView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

//=========================================================================

void CEditorLayerView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);

    CSize size = SizeToolBar(cx, cy);
    m_tcLayer.MoveWindow(0,size.cy,cx,cy-size.cy);
}

//=========================================================================

void CEditorLayerView::OnLvtbAddLayer()
{
    //first add the new layer
    CString strItemName;
    if (GetDocument()->GetFramePointer() &&
        GetDocument()->GetFramePointer()->GetEditorDoc())
    {
        xarray<xstring> List;
        g_WorldEditor.GetLayerNames(List);
        int nId = List.GetCount();
        strItemName.Format("[%d] Layer",nId);

        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Add Layer(%s)", (const char*)strItemName)));
        while (!g_WorldEditor.AddLayer(strItemName, TRUE))
        {
            //try appending a number to layer name
            strItemName.Format("[%d] Layer",++nId);
        }
        g_WorldEditor.MarkLayerLoaded(strItemName, TRUE);
        g_WorldEditor.CommitCurrentUndoEntry();
    }

    //now reload the frame
    if (GetDocument()->GetFramePointer())
    {
        GetDocument()->GetFramePointer()->OnLoadLayers();
        HTREEITEM hRoot = m_tcLayer.InsertItem( strItemName, IMG_LIST_LAYER_LOADED, IMG_LIST_LAYER_LOADED+1, TVI_ROOT, TVI_SORT );
        m_tcLayer.SetItemData(hRoot, 0);
        m_tcLayer.SetItemColor(hRoot,RGB(200,0,0));
        m_tcLayer.EnsureVisible( hRoot );
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbAddFolder()
{
    CString strItemName;
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    if (hItem)
    {
        strItemName = "[1] New Folder";
        int nId = 1;      
        while (DoesChildExist(hItem, strItemName))
        {
            //try appending a number to folder name
            strItemName.Format("[%d] New Folder",++nId);
        }

        HTREEITEM hFolder = m_tcLayer.InsertItem( strItemName, IMG_LIST_FOLDER, IMG_LIST_FOLDER+1, hItem, TVI_SORT );
        m_tcLayer.SetItemData(hFolder, 0);
        m_tcLayer.SetItemColor(hFolder,RGB(0,0,0));
        m_tcLayer.EnsureVisible(hFolder);
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbAddZone()
{
    CString strItemName;
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    if (hItem)
    {
        strItemName = "[1] New Zone";
        int nId = 1;      
        BOOL bKeepTrying = TRUE;
        while (bKeepTrying)
        {
            while (DoesChildExist(hItem, strItemName))
            {
                //try appending a number to zone name
                strItemName.Format("[%d] New Zone",++nId);
            }

            if (!g_WorldEditor.DoesZoneExist(strItemName))
            {
                bKeepTrying = FALSE;
            }
            else
            {
                //try appending a number to zone name
                strItemName.Format("[%d] New Zone",++nId);
            }
        }

        CString strLayer = m_tcLayer.GetItemText(hItem);
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Add Zone(%s)", (const char*)strItemName)));    
        if (g_WorldEditor.CreateZone(strItemName, strLayer))
        {
            HTREEITEM hZone = m_tcLayer.InsertItem( strItemName, IMG_LIST_ZONE, IMG_LIST_ZONE+1, hItem, TVI_SORT );
            m_tcLayer.SetItemData(hZone, g_WorldEditor.GetZoneId(strItemName));
            m_tcLayer.SetItemColor(hZone, RGB(0,0,0));
            m_tcLayer.EnsureVisible(hZone);
            g_WorldEditor.MarkLayerDirty(m_tcLayer.GetItemText(hItem));
//            m_tcLayer.SetItemColor(hItem,RGB(200,0,0));
            g_WorldEditor.CommitCurrentUndoEntry();
        }
        else
        {
            ::AfxMessageBox(xfs("The zone \"%s\" could not be created in layer \"%s\"", (const char*)strItemName, (const char*)strLayer));
            g_WorldEditor.ClearUndoList();
            g_WorldEditor.ClearUndoEntry();
        }
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbAddPortal()
{
    CString strItemName;
    HTREEITEM hItem = FindLayer( g_WorldEditor.GetGlobalLayer() );
    if (hItem)
    {
        strItemName = "[1] Portal";
        int nId = 1;      
        while (DoesChildExist(hItem, strItemName))
        {
            //try appending a number to zone name
            strItemName.Format("[%d] Portal",++nId);
        }
    
        guid PortalGuid = g_WorldEditor.CreateObject(
            strItemName, zone_portal::GetObjectType().GetTypeName(), 
            GetDocument()->GetFramePointer()->GetEditorView()->GetFocusPos(),
            g_WorldEditor.GetGlobalLayer(), "\\");
        if (PortalGuid.Guid != 0)
        {
            HTREEITEM hPortal = m_tcLayer.InsertItem( strItemName, IMG_LIST_PORTAL, IMG_LIST_PORTAL+1, hItem, TVI_SORT );
            xhandle xh = AddObjectGuidToStorageArray( PortalGuid );
            m_tcLayer.SetItemData(hPortal, xh);
            m_tcLayer.SetItemColor(hPortal,RGB(0,0,0));
            m_tcLayer.EnsureVisible(hPortal);  
            
            GetDocument()->GetFramePointer()->GetEditorView()->EnterMovementMode();
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
            GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
        }
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbAddResource()
{
    //first add the new layer
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    CString strLayer = m_tcLayer.GetItemText( hItem );

    if (hItem)
    {
	    CFileDialog dlgBrowse(	TRUE, "", "", 0, (_T("All Files (*.*)|*.*||")));

        CString strPath = g_Settings.GetReleasePath();
        dlgBrowse.m_ofn.lpstrInitialDir = strPath;
        
	    if (dlgBrowse.DoModal() == IDOK)
	    {
		    CString strItemName = dlgBrowse.GetFileName( );

            if (g_WorldEditor.AddResourceToLayer(strItemName, strLayer, TRUE))
            {
                AddResourceToLayer(strLayer, "\\.Res\\", strItemName);
            }
        }
    }
}

//=========================================================================

void CEditorLayerView::SelectChildren(HTREEITEM hParent)
{
	HTREEITEM hItem = m_tcLayer.GetNextItem(hParent, TVGN_CHILD);
	while (hItem)
	{
        int nImage = -1;
        int nSelectedImage = -1;
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            if (nImage == IMG_LIST_OBJECT || nImage == IMG_LIST_PORTAL)
            {
                xhandle xh = m_tcLayer.GetItemData(hItem);
                guid& ChildGuidFound = GetObjectGuidFromHandle(xh);
                g_WorldEditor.SelectObject(ChildGuidFound,FALSE,TRUE);
            }
            else if (nImage == IMG_LIST_BLUEPRINT)
            {
                xhandle xh = m_tcLayer.GetItemData(hItem);
                guid& ChildGuidFound = GetBlueprintGuidFromHandle(xh);
                editor_blueprint_ref BlueprintReference;
                if (g_WorldEditor.GetBlueprintByGuid(ChildGuidFound, BlueprintReference))
                {
                    g_WorldEditor.SelectBlueprintObjects(BlueprintReference,TRUE);
                }
            }
        }

		SelectChildren(hItem);
		hItem = m_tcLayer.GetNextSiblingItem(hItem);
	}
}

//=========================================================================

BOOL CEditorLayerView::GetChildBounds( HTREEITEM hParent, bbox& Bounds )
{
    Bounds.Clear();
    BOOL bInit = FALSE;
	HTREEITEM hItem = m_tcLayer.GetNextItem(hParent, TVGN_CHILD);
	while (hItem)
	{
        int nImage = -1;
        int nSelectedImage = -1;
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            if (nImage == IMG_LIST_OBJECT)
            {
                xhandle xh = m_tcLayer.GetItemData(hItem);
                guid& ChildGuidFound = GetObjectGuidFromHandle(xh);
                bbox objBounds = g_WorldEditor.GetObjectsBoundingBox(ChildGuidFound);
                if( !bInit && (objBounds.Min != objBounds.Max) )
                {
                    Bounds = objBounds;
                    bInit = TRUE;
                }
                else
                {
                    Bounds.AddVerts(&(objBounds.Min),1);
                    Bounds.AddVerts(&(objBounds.Max),1);
                }
            }
            else if (nImage == IMG_LIST_BLUEPRINT)
            {
                xhandle xh = m_tcLayer.GetItemData(hItem);
                guid& ChildGuidFound = GetBlueprintGuidFromHandle(xh);
                editor_blueprint_ref BlueprintReference;
                if (g_WorldEditor.GetBlueprintByGuid( ChildGuidFound, BlueprintReference ))
                {
                    bbox BPBounds = g_WorldEditor.GetBlueprintsBoundingBox(BlueprintReference);
                    if (!bInit)
                    {
                        Bounds = BPBounds;
                        bInit = TRUE;
                    }
                    else
                    {
                        Bounds.AddVerts(&(BPBounds.Min),1);
                        Bounds.AddVerts(&(BPBounds.Max),1);
                    }
                }
            }
        }

        bbox childrensBounds;
        if (GetChildBounds(hItem, childrensBounds))
        {
            if (!bInit)
            {
                Bounds = childrensBounds;
                bInit = TRUE;
            }
            else
            {
                Bounds.AddVerts(&(childrensBounds.Min),1);
                Bounds.AddVerts(&(childrensBounds.Max),1);
            }
        }

		hItem = m_tcLayer.GetNextSiblingItem(hItem);
	}
    return bInit;
}

//=========================================================================

void CEditorLayerView::SelectObject(guid ItemGuid, BOOL bAndExpand)
{
    if (ItemGuid.Guid != 0)
    {
        SelectTreeItem(FindObject(TVI_ROOT, ItemGuid), bAndExpand);
    }
}

//=========================================================================

void CEditorLayerView::SelectBlueprint(guid ItemGuid, BOOL bAndExpand)
{
    if (ItemGuid.Guid != 0)
    {
        SelectTreeItem(FindBlueprint(TVI_ROOT, ItemGuid), bAndExpand);
    }
}

//=========================================================================

void CEditorLayerView::SelectTreeItem(HTREEITEM hItem, BOOL bAndExpand)
{
    if (hItem)
    {
        if (bAndExpand)
        {
            m_bInternalSelect = TRUE;
            m_tcLayer.Select( hItem, TVGN_CARET );
        }
        else
        {
            HTREEITEM hParent = m_tcLayer.GetParentItem( hItem );
            if (hParent)
            {
                if (IsItemChainExpanded(hItem))
                {
                    //parent is expanded, so select item
                    m_bInternalSelect = TRUE;
                    m_tcLayer.SelectItem( hItem );
                }
                else
                {
                    //parent collapsed, so select first visible parent
                    HTREEITEM hNextParent = m_tcLayer.GetParentItem( hParent );
                    while (hNextParent)
                    {
                        if (IsItemChainExpanded(hParent))
                        {
                            break;
                        }
                        hParent = hNextParent;
                        hNextParent = m_tcLayer.GetParentItem( hParent );
                    }
                    m_bInternalSelect = TRUE;
                    m_tcLayer.SelectItem( hParent );
                }
            }
        }
    }
}

//=========================================================================

BOOL CEditorLayerView::IsItemChainExpanded(HTREEITEM hItem)
{
    if (hItem)
    {
        HTREEITEM hParent = m_tcLayer.GetParentItem( hItem );
        while (hParent)
        {
            if (!(m_tcLayer.GetItemState(hParent, TVIS_EXPANDED) & TVIS_EXPANDED))
            {
                //one of the parents was not expanded
                return FALSE;
            }
            hParent = m_tcLayer.GetParentItem( hParent );
        }
        return TRUE;
    }
    return FALSE;
}

//=========================================================================

HTREEITEM CEditorLayerView::FindLayer(CString strLayer)
{
	HTREEITEM hItem = m_tcLayer.GetNextItem(TVI_ROOT, TVGN_CHILD);
	while (hItem)
	{
        int nImage = -1;
        int nSelectedImage = -1;
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            if (nImage == IMG_LIST_LAYER_LOADED || nImage == IMG_LIST_LAYER_UNLOADED)
            {
                if (strLayer.CompareNoCase(m_tcLayer.GetItemText(hItem))==0)
                {
                    return hItem;
                }
            }
        }
		hItem = m_tcLayer.GetNextSiblingItem(hItem);
	}
    return NULL;
}

//=========================================================================

HTREEITEM CEditorLayerView::FindObject(HTREEITEM hParent, guid ItemGuid)
{
	HTREEITEM hItem = m_tcLayer.GetNextItem(hParent, TVGN_CHILD);
	while (hItem)
	{
        int nImage = -1;
        int nSelectedImage = -1;
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            if (nImage == IMG_LIST_OBJECT || nImage == IMG_LIST_PORTAL )
            {
                xhandle xh = m_tcLayer.GetItemData(hItem);
                guid& ChildGuidFound = GetObjectGuidFromHandle(xh);
                if (ItemGuid == ChildGuidFound)
                {
                    return hItem;
                }
            }
        }

		HTREEITEM hFound = FindObject(hItem, ItemGuid);
        if (hFound) return hFound;

		hItem = m_tcLayer.GetNextSiblingItem(hItem);
	}
    return NULL;
}

//=========================================================================

HTREEITEM CEditorLayerView::FindResource(HTREEITEM hParent, CString strRes)
{
	HTREEITEM hItem = m_tcLayer.GetNextItem(hParent, TVGN_CHILD);
	while (hItem)
	{
        int nImage = -1;
        int nSelectedImage = -1;
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            if (nImage == IMG_LIST_RESOURCE)
            {
                CString strText = m_tcLayer.GetItemText(hItem);
                if (strText.CompareNoCase(strRes) == 0)
                {
                    return hItem;
                }
            }
        }

		HTREEITEM hFound = FindResource(hItem, strRes);
        if (hFound) return hFound;

		hItem = m_tcLayer.GetNextSiblingItem(hItem);
	}
    return NULL;
}

//=========================================================================

HTREEITEM CEditorLayerView::FindBlueprint(HTREEITEM hParent, guid ItemGuid)
{
	HTREEITEM hItem = m_tcLayer.GetNextItem(hParent, TVGN_CHILD);
	while (hItem)
	{
        int nImage = -1;
        int nSelectedImage = -1;
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            if (nImage == IMG_LIST_BLUEPRINT)
            {
                xhandle xh = m_tcLayer.GetItemData(hItem);
                guid& ChildGuidFound = GetBlueprintGuidFromHandle(xh);
                if (ItemGuid == ChildGuidFound)
                {
                    return hItem;
                }
            }
        }

		HTREEITEM hFound = FindBlueprint(hItem, ItemGuid);
        if (hFound) return hFound;

		hItem = m_tcLayer.GetNextSiblingItem(hItem);
	}
    return NULL;
}

//=========================================================================

BOOL CEditorLayerView::GetItemPath(HTREEITEM hItem, CString& strPath, CString& strLayer, BOOL bIgnoreSelected)
{
    //Create the full string of the tree item
    BOOL bIgnore = bIgnoreSelected;
    HTREEITEM hParent = hItem;
    while (hParent)
    {
        CString strItem = m_tcLayer.GetItemText(hParent);
        int nLength = strItem.GetLength();
        ASSERT(nLength);
        hParent = m_tcLayer.GetParentItem(hParent);

        if (bIgnore)
        {
            bIgnore = FALSE;
        }
        else
        {
            if (hParent)
            {
                if (strItem.GetAt(nLength-1) == _T('\\'))
                    strPath = strItem + strPath;
                else
                {
                    if (strPath.GetLength())
                        strPath = strItem + _T('\\') + strPath;
                    else
                        strPath = strItem;
                }
            }
            strLayer = strItem; //last item
        }
    }

    if (!strPath.IsEmpty())
    {
        strPath = "\\" + strPath;
    }
    strPath += "\\";

    return TRUE;
}

//=========================================================================

void CEditorLayerView::SaveChildPaths(HTREEITEM hParent)
{
	HTREEITEM hItem = m_tcLayer.GetNextItem(hParent, TVGN_CHILD);
	while (hItem)
	{
        int nImage = -1;
        int nSelectedImage = -1;
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            CString strPath, strLayer;
            VERIFY(GetItemPath(hItem, strPath, strLayer, TRUE));
            SetLayerDirtyColor(strLayer);

            if (nImage == IMG_LIST_OBJECT)
            {
                xhandle xh = m_tcLayer.GetItemData(hItem);
                guid& ChildGuidFound = GetObjectGuidFromHandle(xh);

                //do the change
                g_WorldEditor.SetObjectLayerPath(ChildGuidFound, strLayer, strPath);
            }
            else if (nImage == IMG_LIST_BLUEPRINT)
            {
                xhandle xh = m_tcLayer.GetItemData(hItem);
                guid& ChildGuidFound = GetBlueprintGuidFromHandle(xh);

                //do the change
                g_WorldEditor.SetBlueprintLayerPath(ChildGuidFound, strLayer, strPath);
            }
        }

		SaveChildPaths(hItem);
		hItem = m_tcLayer.GetNextSiblingItem(hItem);
	}
}

//=========================================================================

BOOL CEditorLayerView::DoesSiblingExist(HTREEITEM hItem, CString strName)
{
    HTREEITEM hNextItem = m_tcLayer.GetNextSiblingItem(hItem);
    while (hNextItem)
    {
        if (strName.CompareNoCase(m_tcLayer.GetItemText(hNextItem)) == 0)
        {
            //found a match
            return TRUE;
        }
        hNextItem = m_tcLayer.GetNextSiblingItem(hNextItem);
    }
    hNextItem = m_tcLayer.GetPrevSiblingItem(hItem);
    while (hNextItem)
    {
        if (strName.CompareNoCase(m_tcLayer.GetItemText(hNextItem)) == 0)
        {
            //found a match
            return TRUE;
        }
        hNextItem = m_tcLayer.GetPrevSiblingItem(hNextItem);
    }
    return FALSE;
}

//=========================================================================

BOOL CEditorLayerView::DoesChildExist(HTREEITEM hParent, CString strName)
{
	HTREEITEM hItem = m_tcLayer.GetNextItem(hParent, TVGN_CHILD);
	while (hItem)
	{
        if (strName.CompareNoCase(m_tcLayer.GetItemText(hItem))==0)
        {
            //found one
            return TRUE;
        }
        
		hItem = m_tcLayer.GetNextSiblingItem(hItem);
	}
    return FALSE;
}

//=========================================================================

void CEditorLayerView::OnLvtbDeleteLayer()
{
    DeleteSelected();
}

//=========================================================================

void CEditorLayerView::DeleteSelected()
{
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    if (IsLayerSelected())
    {
        CString strItem = m_tcLayer.GetItemText( hItem );
        if (::AfxMessageBox("Are you sure you want to delete this layer and all objects and blueprints within it?",MB_YESNO) == IDYES)
        {
            g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Remove Layer(%s)", (const char*)strItem)));
            g_WorldEditor.RemoveLayer(strItem);
            g_WorldEditor.CommitCurrentUndoEntry();

            //now reload the frame
            if (GetDocument()->GetFramePointer())
            {
                GetDocument()->GetFramePointer()->OnLoadLayers();
                m_tcLayer.DeleteItem( hItem );
            }
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        }
    }
    else if (IsFolderSelected())
    {
        CString strItem = m_tcLayer.GetItemText( hItem );
        if (::AfxMessageBox("Are you sure you want to delete this folder and all objects and blueprints within it?",MB_YESNO) == IDYES)
        {
            g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Delete Layer Folder(%s)", (const char*)strItem)));
            g_WorldEditor.ClearSelectedObjectList();
            //select each object within this list
            SelectChildren(hItem);
            //now delete those selected objects
            xarray<editor_item_descript> lstItems;
	        g_WorldEditor.DeleteSelectedObjects(lstItems);
            g_WorldEditor.CommitCurrentUndoEntry();
            LoadLayers();
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        }
    }
    else if (IsResourceSelected())
    {
        CString strResource = m_tcLayer.GetItemText( hItem );
        CString strPath, strLayer;
        VERIFY(GetItemPath(hItem, strPath, strLayer));

        if (g_WorldEditor.RemoveResourceFromLayer(strResource, strLayer))
        {
            RemoveResourceFromLayer(strLayer, strPath, strResource);
        }
    }
    else if (IsZoneSelected())
    {
        CString strItem = m_tcLayer.GetItemText( hItem );
        if (::AfxMessageBox("Are you sure you want to delete this zone and all objects and blueprints within it?",MB_YESNO) == IDYES)
        {
            g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Remove Zone(%s)", (const char*)strItem)));
            //remove zone
            g_WorldEditor.DeleteZone(strItem);
            //select each object within this list
            g_WorldEditor.ClearSelectedObjectList();
            SelectChildren(hItem);
            //now delete those selected objects
            xarray<editor_item_descript> lstItems;
	        g_WorldEditor.DeleteSelectedObjects(lstItems);
            g_WorldEditor.CommitCurrentUndoEntry();

            HTREEITEM hParent = m_tcLayer.GetParentItem( hItem );
            g_WorldEditor.MarkLayerDirty(m_tcLayer.GetItemText(hParent));
//            m_tcLayer.SetItemColor(hParent,RGB(200,0,0));

            //now reload the frame
            if (GetDocument()->GetFramePointer())
            {
                GetDocument()->GetFramePointer()->OnLoadLayers();
                m_tcLayer.DeleteItem( hItem );
            }
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        }
    }
    else if (IsPortalSelected())
    {
        CString strItem = m_tcLayer.GetItemText( hItem );
        if (::AfxMessageBox(xfs("Are you sure you want to delete this portal[%s] and all objects and blueprints within it?", strItem),MB_YESNO) == IDYES)
        {
            g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Delete Portal(%s)", (const char*)strItem)));
            g_WorldEditor.ClearSelectedObjectList();
            //select each object within this list
            SelectChildren(hItem);
            //now delete those selected objects
            xarray<editor_item_descript> lstItems;
	        g_WorldEditor.DeleteSelectedObjects(lstItems);

            xhandle xh = m_tcLayer.GetItemData(hItem);
            guid& PortalGuidFound = GetObjectGuidFromHandle(xh);
            g_WorldEditor.DeleteObject(PortalGuidFound);

            g_WorldEditor.CommitCurrentUndoEntry();
            LoadLayers();
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        }
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbSetAsActiveLayer()
{
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    if (hItem)
    {
        CString strPath, strLayer;
        VERIFY(GetItemPath(hItem, strPath, strLayer));

        if (GetDocument()->GetFramePointer())
        {
            g_WorldEditor.SetActiveLayer(strLayer, strPath);
            GetDocument()->GetFramePointer()->OnLoadLayers();
        }
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbLightLayer()
{
//    CXTCoolMenu CoolMenu;
//    CoolMenu.HookWindow( this );

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

void CEditorLayerView::OnLvtbLightLayerType(UINT nId)
{
    CWaitCursor wc;

    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    CString strItem = m_tcLayer.GetItemText( hItem );
    if (GetDocument()->GetFramePointer())
    {
        switch(nId)
        {
        case IDR_LEVEL_LIGHT_DIR:
            g_WorldEditor.ComputeLightLayer(strItem, 1);
            break;
        case IDR_LEVEL_LIGHT_NORMAL:
            g_WorldEditor.ComputeLightLayer(strItem, 2);
            break;
        case IDR_LEVEL_LIGHT_DYNAMIC:
            g_WorldEditor.ComputeLightLayer(strItem, 3);
            break;
        case IDR_LEVEL_LIGHT_WHITE:
            g_WorldEditor.ComputeLightLayer(strItem, 4);
            break;
        case IDR_LEVEL_LIGHT_RAYCAST:
            g_WorldEditor.ComputeLightLayer(strItem, 5);
            break;
        case IDR_LEVEL_LIGHT_ZONE:
            g_WorldEditor.ComputeLightLayer(strItem, 6);
            break;
        }

        GetDocument()->GetFramePointer()->GetEditorView()->RedrawWindow();
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbSelectLayer()
{
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    if (IsLayerSelected())
    {
        CString strItem = m_tcLayer.GetItemText( hItem );
        if (GetDocument()->GetFramePointer())
        {
            g_WorldEditor.SelectAllItemsInLayer(strItem);
        }
    }
    else if (IsFolderSelected() || IsZoneSelected() || IsPortalSelected())
    {
        g_WorldEditor.ClearSelectedObjectList();
        SelectChildren(hItem);
    }
    
    if( g_WorldEditor.GetSelectedCount() )
    {
        GetDocument()->GetFramePointer()->GetEditorView()->SetFocusPos(g_WorldEditor.GetMinPositionForSelected());
    }

    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
}

//=========================================================================

void CEditorLayerView::OnLvtbRenameLayer()
{
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    CEdit* pEdit = m_tcLayer.EditLabel( hItem );
    ASSERT(pEdit);
}

//=========================================================================

void CEditorLayerView::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	*pResult = 0;

    if(!IsLayerEditable())
    {
        ::AfxMessageBox("Can't rename in a layer that is not checked out.");
        return;
    }

	if (pTVDispInfo->item.pszText)
	{
		CString strNewName(pTVDispInfo->item.pszText);

        if (strNewName.FindOneOf("*:?\\/.<>|\"()") != -1)
        {
            ::AfxMessageBox("That name contained invalid characters!"); 
        }
        else
        {
            int nImage = -1;
            int nSelectedImage = -1;
            if (m_tcLayer.GetItemImage( pTVDispInfo->item.hItem, nImage, nSelectedImage ))
            {
                switch(nImage)
                {
                case IMG_LIST_LAYER_LOADED: 
                case IMG_LIST_LAYER_UNLOADED: 
                    {
                        if (g_WorldEditor.DoesLayerExist(strNewName))
                        {
                            //layer already exists, invalid
                            ::AfxMessageBox("A layer of that name already exists!"); 
                            return;
                        }
                        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Rename Layer(%s to %s)",m_tcLayer.GetItemText( pTVDispInfo->item.hItem ), (const char*)strNewName)));
                        g_WorldEditor.RenameLayer(m_tcLayer.GetItemText( pTVDispInfo->item.hItem ),strNewName);
                        g_WorldEditor.CommitCurrentUndoEntry();

                        m_tcLayer.SetItemText( pTVDispInfo->item.hItem, strNewName ); 
                        SetLayerDirtyColor(strNewName);
                    }
                    break;
                case IMG_LIST_ZONE:
                    {
                        if (DoesSiblingExist(pTVDispInfo->item.hItem, strNewName))
                        {
                            //folder already exists, invalid
                            ::AfxMessageBox("A Virtual Folder of that name already exists!"); 
                            return;
                        }
                        else if (g_WorldEditor.DoesZoneExist(strNewName))
                        {
                            //folder already exists, invalid
                            ::AfxMessageBox("A Zone of that name already exists!"); 
                            return;
                        }

                        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Rename Zone(%s to %s)", 
                                            m_tcLayer.GetItemText( pTVDispInfo->item.hItem ), (const char*)strNewName)));
                        //do the change
                        if (g_WorldEditor.RenameZone(m_tcLayer.GetItemText( pTVDispInfo->item.hItem ), strNewName))
                        {
                            m_tcLayer.SetItemText( pTVDispInfo->item.hItem, strNewName ); 
                            //save the states
                            SaveChildPaths(pTVDispInfo->item.hItem);
                        }
                        g_WorldEditor.CommitCurrentUndoEntry();

                        //mark dirty
                        HTREEITEM hParent = m_tcLayer.GetParentItem( pTVDispInfo->item.hItem );
                        g_WorldEditor.MarkLayerDirty(m_tcLayer.GetItemText(hParent));
//                        m_tcLayer.SetItemColor(hParent,RGB(200,0,0));

                        //reset the active layer
                        g_WorldEditor.SetActiveLayer(g_WorldEditor.GetActiveLayer(), "\\");
                        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
 
                    }                           
                    break;
                case IMG_LIST_PORTAL:
                    {
                        if (DoesSiblingExist(pTVDispInfo->item.hItem, strNewName))
                        {
                            ::AfxMessageBox("A Sibling of that name already exists!"); 
                            return;
                        }

                        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Rename Portal(%s to %s)", 
                                            m_tcLayer.GetItemText( pTVDispInfo->item.hItem ), (const char*)strNewName)));

                        xhandle xh = m_tcLayer.GetItemData(pTVDispInfo->item.hItem);
                        guid& ChildGuidFound = GetObjectGuidFromHandle(xh);

                        if (g_WorldEditor.RenamePortal(ChildGuidFound, strNewName))
                        {
                            //do the change
                            m_tcLayer.SetItemText( pTVDispInfo->item.hItem, strNewName ); 
                            //save the states
                            SaveChildPaths(pTVDispInfo->item.hItem);
                            g_WorldEditor.CommitCurrentUndoEntry();

                            //reset the active layer
                            g_WorldEditor.SetActiveLayer(g_WorldEditor.GetActiveLayer(), "\\");
                            GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                        }
                        else
                        {
                            g_WorldEditor.ClearUndoList();
                            g_WorldEditor.ClearUndoEntry();
                            ::AfxMessageBox("That name is too long!"); 
                            return;
                        }
                    }   
                    break;
                case IMG_LIST_OBJECT:
                    {
                        xhandle xh = m_tcLayer.GetItemData(pTVDispInfo->item.hItem);
                        guid& ChildGuidFound = GetObjectGuidFromHandle(xh);
                        object* pObject = g_ObjMgr.GetObjectByGuid(ChildGuidFound) ;
                        if( pObject )
                        {
                            pObject->SetName(strNewName);                     
                            //do the change
                            m_tcLayer.SetItemText( pTVDispInfo->item.hItem, strNewName ); 
                            //save the states
                            SaveChildPaths(pTVDispInfo->item.hItem);
                            GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                        }
                    }
 
                    break;
                case IMG_LIST_ZONELINK_FOLDER:
                case IMG_LIST_FOLDER:
                    {
                        if (DoesSiblingExist(pTVDispInfo->item.hItem, strNewName))
                        {
                            //folder already exists, invalid
                            ::AfxMessageBox("A Virtual Folder of that name already exists!"); 
                            return;
                        }

                        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Rename Folder(%s to %s)", 
                                            m_tcLayer.GetItemText( pTVDispInfo->item.hItem ), (const char*)strNewName)));
                        //do the change
                        m_tcLayer.SetItemText( pTVDispInfo->item.hItem, strNewName ); 
                        //save the states
                        SaveChildPaths(pTVDispInfo->item.hItem);
                        g_WorldEditor.CommitCurrentUndoEntry();

                        SetFolderImageForItem(pTVDispInfo->item.hItem);

                        //reset the active layer
                        g_WorldEditor.SetActiveLayer(g_WorldEditor.GetActiveLayer(), "\\");
                    }                           
                    break;
                }
            }
        }
	    
        //now reload the frame
        if (GetDocument()->GetFramePointer())
        {
            GetDocument()->GetFramePointer()->OnLoadLayers();
        }
	}
}

//=========================================================================

void CEditorLayerView::SetFolderImageForItem(HTREEITEM hItem)
{
    //check if this is a zone link folder or not?
    int nParentImage = -1;
    int nParentSelectedImage = -1;
    CString strItem = m_tcLayer.GetItemText(hItem);
    HTREEITEM hParent = m_tcLayer.GetParentItem(hItem);
    if ( hParent && m_tcLayer.GetItemImage( hParent, nParentImage, nParentSelectedImage ) &&
         (( nParentImage == IMG_LIST_LAYER_LOADED ) || ( nParentImage == IMG_LIST_LAYER_UNLOADED )) &&
         g_WorldEditor.DoesZoneExist(strItem) )
    {
        //parent is a layer, and this item name matches a zone name
        m_tcLayer.SetItemImage( hItem, IMG_LIST_ZONELINK_FOLDER, IMG_LIST_ZONELINK_FOLDER+1);
    }
    else
    {
        m_tcLayer.SetItemImage( hItem, IMG_LIST_FOLDER, IMG_LIST_FOLDER+1);
    }
}

//=========================================================================

BOOL CEditorLayerView::CanModify(BOOL bLayerChange)
{
    if (GetDocument()->GetFramePointer() &&
        GetDocument()->GetFramePointer()->GetEditorDoc())
    {
        if (IsLayerSelected())
        {
            HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
            if (hItem)
            {
                CString strItem = m_tcLayer.GetItemText( hItem );
                if ((strItem.CompareNoCase(g_WorldEditor.GetDefaultLayer( ))!=0) &&
                    (strItem.CompareNoCase(g_WorldEditor.GetGlobalLayer( ))!=0))
                {   
                    //not the default
                    if (bLayerChange)
                    {
                        if (IsLayerLoaded() &&                        
                            !g_WorldEditor.DoesLayerContainZones(strItem))
                        {
                            return TRUE;
                        }
                    }
                    else
                    {
                        return IsLayerLoaded();
                    }
                }
            }
        }
        else if (IsFolderSelected())
        {
            return TRUE;
        }
        else if (IsPortalSelected())
        {
            if (g_WorldEditor.CanAddRemovePortals())
            {
                return TRUE;
            }
        }
        else if (IsZoneSelected())
        {
            return IsLayerLoaded();
        }
    }

    return FALSE;
}

//=========================================================================

BOOL CEditorLayerView::IsLayerLoaded()
{
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    int nImage = -1;
    int nSelectedImage = -1;
    if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
    {
        if (nImage == IMG_LIST_ZONE)
        {
            //zone needs to check parent
            HTREEITEM hParent = m_tcLayer.GetParentItem(hItem);
            if (!(hParent && m_tcLayer.GetItemImage( hParent, nImage, nSelectedImage )))
            {
                return FALSE;
            }
        }        
        
        //can only modigy loaded layers
        if (nImage == IMG_LIST_LAYER_LOADED)
        {
            return TRUE;
        }
    }
    return FALSE;    
}

//=========================================================================

BOOL CEditorLayerView::IsLayerSelected(BOOL bIncludeDefault, BOOL bIncludeGlobal)
{
    BOOL bReturn = FALSE;
    if (GetDocument()->GetFramePointer() &&
        GetDocument()->GetFramePointer()->GetEditorDoc())
    {
        //hack to get type of item
        int nImage = -1;
        int nSelectedImage = -1;
        HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            if ((nImage == IMG_LIST_LAYER_LOADED) ||
                (nImage == IMG_LIST_LAYER_UNLOADED))
            {
                CString strText = m_tcLayer.GetItemText(hItem);
                if (!bIncludeDefault)
                {
                    //make sure this isn't the global layer
                    if (strText.CompareNoCase(g_WorldEditor.GetDefaultLayer( ))==0)
                    {
                        return FALSE;
                    }
                }
                if (!bIncludeGlobal)
                {
                    //make sure this isn't the global layer
                    if (strText.CompareNoCase(g_WorldEditor.GetGlobalLayer( ))==0)
                    {
                        return FALSE;
                    }
                }

                return TRUE;
            }
        }
    }

    return FALSE;
}

//=========================================================================

BOOL CEditorLayerView::IsLayerEditable()
{
    BOOL bReturn = FALSE;
    if (GetDocument()->GetFramePointer() &&
        GetDocument()->GetFramePointer()->GetEditorDoc())
    {
        HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
        if (hItem)
        {
            CString strPath, strLayer;
            VERIFY(GetItemPath(hItem, strPath, strLayer));
            bReturn = !g_WorldEditor.IsLayerReadonly(strLayer);
        }
    }

    return bReturn;
}

//=========================================================================

BOOL CEditorLayerView::IsZoneSelected()
{
    BOOL bReturn = FALSE;
    if (GetDocument()->GetFramePointer() &&
        GetDocument()->GetFramePointer()->GetEditorDoc())
    {
        //hack to get type of item
        int nImage = -1;
        int nSelectedImage = -1;
        HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            return (nImage == IMG_LIST_ZONE);
        }
    }

    return bReturn;
}

//=========================================================================

BOOL CEditorLayerView::IsPortalSelected()
{
    BOOL bReturn = FALSE;
    if (GetDocument()->GetFramePointer() &&
        GetDocument()->GetFramePointer()->GetEditorDoc())
    {
        //hack to get type of item
        int nImage = -1;
        int nSelectedImage = -1;
        HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            return (nImage == IMG_LIST_PORTAL);
        }
    }

    return bReturn;
}

//=========================================================================

BOOL CEditorLayerView::IsFolderSelected()
{
    if (GetDocument()->GetFramePointer() &&
        GetDocument()->GetFramePointer()->GetEditorDoc())
    {
        //hack to get type of item
        int nImage = -1;
        int nSelectedImage = -1;
        HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            if ((nImage == IMG_LIST_FOLDER) || (nImage == IMG_LIST_ZONELINK_FOLDER))
            {
                //if item data is not zero this is a non-mod folder
                DWORD dwData = m_tcLayer.GetItemData( hItem );
                if (dwData == 0)
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

//=========================================================================

BOOL CEditorLayerView::IsResourceSelected()
{
    if (GetDocument()->GetFramePointer() &&
        GetDocument()->GetFramePointer()->GetEditorDoc())
    {
        //hack to get type of item
        int nImage = -1;
        int nSelectedImage = -1;
        HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
        if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
        {
            if (nImage == IMG_LIST_RESOURCE)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

//=========================================================================

HTREEITEM CEditorLayerView::AddObjectToLayer(CString strLayer, CString strPath, guid ObjectGuid)
{
    HTREEITEM hNextItem;
    HTREEITEM hLayerItem = m_tcLayer.GetRootItem( );

    while (hLayerItem != NULL)
    {
        hNextItem = m_tcLayer.GetNextSiblingItem(hLayerItem);
        if (strLayer.CompareNoCase(m_tcLayer.GetItemText(hLayerItem)) == 0)
        {
            //Found it, add object!
            //do the layer path parsing
            ASSERT(strPath.GetAt(0) == '\\');
            CString strSubPath = strPath.Right(strPath.GetLength()-1);
            int iFind = strSubPath.Find('\\',0);
            HTREEITEM hParent = hLayerItem;
            while (iFind != -1)
            {
                //now parse the layer path
                CString strFolder = strSubPath.Left(iFind);
                ASSERT(!strFolder.IsEmpty());

                //now need to determine if this folder exists already
                BOOL bMatchingFolder = FALSE;
                if (m_tcLayer.ItemHasChildren(hParent))
                {
                    HTREEITEM hChildItem = m_tcLayer.GetChildItem(hParent);
                    while (hChildItem != NULL)
                    {
                        hNextItem = m_tcLayer.GetNextItem(hChildItem, TVGN_NEXT);

                        int nImage = -1;
                        int nSelectedImage = -1;
                        if (m_tcLayer.GetItemImage( hChildItem, nImage, nSelectedImage ))
                        {
                            if ((nImage == IMG_LIST_FOLDER) || //object
                                (nImage == IMG_LIST_ZONELINK_FOLDER ) ||
                                (nImage == IMG_LIST_ZONE) || 
                                (nImage == IMG_LIST_PORTAL))
                            {
                                if (strFolder.CompareNoCase(m_tcLayer.GetItemText(hChildItem)) == 0)
                                {
                                    bMatchingFolder = TRUE;
                                    hParent = hChildItem;
                                    break;
                                }
                            }
                        }
                        hChildItem = hNextItem;
                    }
                }

                if (!bMatchingFolder)
                {
                    hParent = m_tcLayer.InsertItem( strFolder, IMG_LIST_FOLDER, IMG_LIST_FOLDER+1, hParent, TVI_SORT );
                    SetFolderImageForItem(hParent);
                    m_tcLayer.SetItemData(hParent, 0);
                    m_tcLayer.SetItemColor(hParent,RGB(0,0,0));
                }

                strSubPath = strSubPath.Right(strSubPath.GetLength() - iFind - 1);
                iFind = strSubPath.Find('\\',0);
            }

            return AddObjectToTree(hParent, ObjectGuid);
        }
        
        hLayerItem = hNextItem;
    } 

    TRACE("ERROR CEditorLayerView::AddObjectToLayer... Could not find layer, doing full reload\n");
    LoadLayers();
    return NULL;
}

//=========================================================================

HTREEITEM CEditorLayerView::AddObjectToTree(HTREEITEM hParent, guid ObjGuid)
{
    HTREEITEM hItem;
    xstring xstrItem;
    g_WorldEditor.GetDisplayNameForObject( ObjGuid, xstrItem );
    CString strName(xstrItem);

    if( x_stricmp( g_WorldEditor.GetObjectTypeName(ObjGuid), zone_portal::GetObjectType().GetTypeName()) == 0 )
    {
        hItem = m_tcLayer.InsertItem( strName, IMG_LIST_PORTAL, IMG_LIST_PORTAL+1, hParent, TVI_SORT );
    }
    else
    {
        hItem = m_tcLayer.InsertItem( strName, IMG_LIST_OBJECT, IMG_LIST_OBJECT+1, hParent, TVI_SORT );
    }

    xhandle xh = AddObjectGuidToStorageArray( ObjGuid );
    m_tcLayer.SetItemData(hItem, xh);
    m_tcLayer.SetItemColor(hItem,RGB(0,0,0));

    return hItem;
}

//=========================================================================

BOOL CEditorLayerView::RemoveObjectFromLayer(CString strLayer, CString strPath, guid ObjectGuid)
{
    HTREEITEM hNextItem;
    HTREEITEM hLayerItem = m_tcLayer.GetRootItem( );

    while (hLayerItem != NULL)
    {
        hNextItem = m_tcLayer.GetNextSiblingItem(hLayerItem);
        if (strLayer.CompareNoCase(m_tcLayer.GetItemText(hLayerItem)) == 0)
        {
            //Found layer, now find the object!
            HTREEITEM hItemToRemove = FindObject(hLayerItem, ObjectGuid);
            if (hItemToRemove)
            {
                m_tcLayer.DeleteItem(hItemToRemove);
                return TRUE;
            }
            TRACE("ERROR CEditorLayerView::RemoveObjectFromLayer... Could not find object, doing full reload\n");
            LoadLayers();
            return FALSE;
        }
        hLayerItem = hNextItem;
    }

    TRACE("ERROR CEditorLayerView::RemoveObjectFromLayer... Could not find layer, doing full reload\n");
    LoadLayers();
    return FALSE;
}

//=========================================================================

HTREEITEM CEditorLayerView::AddResourceToLayer(CString strLayer, CString strPath, CString strRes)
{
    HTREEITEM hNextItem;
    HTREEITEM hLayerItem = m_tcLayer.GetRootItem( );

    while (hLayerItem != NULL)
    {
        hNextItem = m_tcLayer.GetNextSiblingItem(hLayerItem);
        if (strLayer.CompareNoCase(m_tcLayer.GetItemText(hLayerItem)) == 0)
        {
            //do the layer path parsing
            ASSERT(strPath.GetAt(0) == '\\');
            CString strSubPath = strPath.Right(strPath.GetLength()-1);
            int iFind = strSubPath.Find('\\',0);
            HTREEITEM hParent = hLayerItem;
            while (iFind != -1)
            {
                //now parse the layer path
                CString strFolder = strSubPath.Left(iFind);
                ASSERT(!strFolder.IsEmpty());

                //now need to determine if this folder exists already
                BOOL bMatchingFolder = FALSE;
                if (m_tcLayer.ItemHasChildren(hParent))
                {
                    HTREEITEM hChildItem = m_tcLayer.GetChildItem(hParent);
                    while (hChildItem != NULL)
                    {
                        hNextItem = m_tcLayer.GetNextItem(hChildItem, TVGN_NEXT);

                        int nImage = -1;
                        int nSelectedImage = -1;
                        if (m_tcLayer.GetItemImage( hChildItem, nImage, nSelectedImage ))
                        {
                            if ((nImage == IMG_LIST_FOLDER) ||
                                (nImage == IMG_LIST_ZONELINK_FOLDER )) //object
                            {
                                if (strFolder.CompareNoCase(m_tcLayer.GetItemText(hChildItem)) == 0)
                                {
                                    bMatchingFolder = TRUE;
                                    hParent = hChildItem;
                                    break;
                                }
                            }
                        }
                        hChildItem = hNextItem;
                   }
                }

                if (!bMatchingFolder)
                {
                    hParent = m_tcLayer.InsertItem( strFolder, IMG_LIST_FOLDER, IMG_LIST_FOLDER+1, hParent, TVI_SORT );
                    SetFolderImageForItem(hParent);
                    m_tcLayer.SetItemData(hParent, 1);
                    m_tcLayer.SetItemColor(hParent,RGB(0,0,0));
                }

                strSubPath = strSubPath.Right(strSubPath.GetLength() - iFind - 1);
                iFind = strSubPath.Find('\\',0);
            }

            HTREEITEM hItem = m_tcLayer.InsertItem( strRes, IMG_LIST_RESOURCE, IMG_LIST_RESOURCE+1, hParent, TVI_SORT );

            m_tcLayer.SetItemData(hItem, 0);
            m_tcLayer.SetItemColor(hItem,RGB(0,0,0));

            return hItem;
        }
        hLayerItem = hNextItem;
    }

    TRACE("ERROR CEditorLayerView::AddResourceToLayer... Could not find layer, doing full reload\n");
    LoadLayers();
    return NULL;
}

//=========================================================================

BOOL CEditorLayerView::RemoveResourceFromLayer(CString strLayer, CString strPath, CString strRes)
{
    HTREEITEM hNextItem;
    HTREEITEM hLayerItem = m_tcLayer.GetRootItem( );

    while (hLayerItem != NULL)
    {
        hNextItem = m_tcLayer.GetNextSiblingItem(hLayerItem);
        if (strLayer.CompareNoCase(m_tcLayer.GetItemText(hLayerItem)) == 0)
        {
            //Found layer, now find the object!
            HTREEITEM hItemToRemove = FindResource(hLayerItem, strRes);
            if (hItemToRemove)
            {
                m_tcLayer.DeleteItem(hItemToRemove);
                return TRUE;
            }
            TRACE("ERROR CEditorLayerView::RemoveResourceFromLayer... Could not find resource, doing full reload\n");
            LoadLayers();
            return FALSE;
        }
        hLayerItem = hNextItem;
    }

    TRACE("ERROR CEditorLayerView::RemoveResourceFromLayer... Could not find layer, doing full reload\n");
    LoadLayers();
    return FALSE;
}

//=========================================================================

HTREEITEM CEditorLayerView::AddBlueprintToLayer(CString strLayer, CString strPath, guid BlueprintGuid)
{
    HTREEITEM hNextItem;
    HTREEITEM hLayerItem = m_tcLayer.GetRootItem( );

    while (hLayerItem != NULL)
    {
        hNextItem = m_tcLayer.GetNextSiblingItem(hLayerItem);
        if (strLayer.CompareNoCase(m_tcLayer.GetItemText(hLayerItem)) == 0)
        {
            //Found it, add blueprint!

            //do the layer path parsing
            ASSERT(strPath.GetAt(0) == '\\');
            CString strSubPath = strPath.Right(strPath.GetLength()-1);
            int iFind = strSubPath.Find('\\',0);
            HTREEITEM hParent = hLayerItem;
            while (iFind != -1)
            {
                //now parse the layer path
                CString strFolder = strSubPath.Left(iFind);
                ASSERT(!strFolder.IsEmpty());

                //now need to determine if this folder exists already
                BOOL bMatchingFolder = FALSE;
                if (m_tcLayer.ItemHasChildren(hParent))
                {
                    HTREEITEM hChildItem = m_tcLayer.GetChildItem(hParent);
                    while (hChildItem != NULL)
                    {
                        hNextItem = m_tcLayer.GetNextItem(hChildItem, TVGN_NEXT);

                        int nImage = -1;
                        int nSelectedImage = -1;
                        if (m_tcLayer.GetItemImage( hChildItem, nImage, nSelectedImage ))
                        {
                            if ((nImage == IMG_LIST_FOLDER) || //object
                                (nImage == IMG_LIST_ZONELINK_FOLDER ) ||
                                (nImage == IMG_LIST_ZONE) || 
                                (nImage == IMG_LIST_PORTAL))
                            {
                                if (strFolder.CompareNoCase(m_tcLayer.GetItemText(hChildItem)) == 0)
                                {
                                    bMatchingFolder = TRUE;
                                    hParent = hChildItem;
                                    break;
                                }
                            }
                        }
                        hChildItem = hNextItem;
                    }
                }

                if (!bMatchingFolder)
                {
                    hParent = m_tcLayer.InsertItem( strFolder, IMG_LIST_FOLDER, IMG_LIST_FOLDER+1, hParent, TVI_SORT );
                    SetFolderImageForItem(hParent);
                    m_tcLayer.SetItemData(hParent, 0);
                    m_tcLayer.SetItemColor(hParent,RGB(0,0,0));
                }

                strSubPath = strSubPath.Right(strSubPath.GetLength() - iFind - 1);
                iFind = strSubPath.Find('\\',0);
            }

            return AddBlueprintToTree(hParent, BlueprintGuid);
        }
        hLayerItem = hNextItem;
    }

    TRACE("ERROR CEditorLayerView::AddBlueprintToLayer... Could not find layer, doing full reload\n");
    LoadLayers();
    return NULL;
}


//=========================================================================

HTREEITEM CEditorLayerView::AddBlueprintToTree(HTREEITEM hParent, guid BPGuid)
{
    xstring xstrItem;
    g_WorldEditor.GetDisplayNameForBlueprint( BPGuid, xstrItem );
    //for (int s =0; s<xstrItem.GetLength(); s++) strName += xstrItem.GetAt(s);
    CString strName(xstrItem);

    HTREEITEM hItem = m_tcLayer.InsertItem( strName, IMG_LIST_BLUEPRINT, IMG_LIST_BLUEPRINT+1, hParent, TVI_SORT );

    xhandle xh = AddBlueprintGuidToStorageArray( BPGuid );
    m_tcLayer.SetItemData(hItem, xh);
    m_tcLayer.SetItemColor(hItem,RGB(0,0,0));

    return hItem;
}

//=========================================================================

BOOL CEditorLayerView::RemoveBlueprintFromLayer(CString strLayer, CString strPath, guid BlueprintGuid)
{
    HTREEITEM hNextItem;
    HTREEITEM hLayerItem = m_tcLayer.GetRootItem( );

    while (hLayerItem != NULL)
    {
        hNextItem = m_tcLayer.GetNextSiblingItem(hLayerItem);
        if (strLayer.CompareNoCase(m_tcLayer.GetItemText(hLayerItem)) == 0)
        {
            //Found layer, now find the blueprint!
            HTREEITEM hItemToRemove = FindBlueprint(hLayerItem, BlueprintGuid);
            if (hItemToRemove)
            {
                m_tcLayer.DeleteItem(hItemToRemove);
                return TRUE;
            }
            TRACE("ERROR CEditorLayerView::RemoveBlueprintFromLayer... Could not find blueprint, doing full reload\n");
            LoadLayers();
            return FALSE;
        }
        hLayerItem = hNextItem;
    }

    TRACE("ERROR CEditorLayerView::RemoveBlueprintFromLayer... Could not find layer, doing full reload\n");
    LoadLayers();
    return FALSE;
}

//=========================================================================

void CEditorLayerView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::Layers");
    }
}

//=========================================================================

void CEditorLayerView::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
    //cancel out for internal selection change
    if (m_bInternalSelect)
    {
        m_bInternalSelect = FALSE;
    	*pResult = 0;
        return;
    }

    //reset render volume
    bbox Zone;
    xcolor Color;
    Zone.Clear();

    xhandle hHandle = m_tcLayer.GetItemData(pNMTreeView->itemNew.hItem);
    g_WorldEditor.UnSelectZone();

    if (hHandle  != HNULL)
    {
        //hack to get type of item
        int nImage = -1;
        int nSelectedImage = -1;
        if (m_tcLayer.GetItemImage( pNMTreeView->itemNew.hItem, nImage, nSelectedImage ))
        {
            switch(nImage)
            {
            case IMG_LIST_LAYER_LOADED: 
            case IMG_LIST_LAYER_UNLOADED: 
            case IMG_LIST_FOLDER: 
            case IMG_LIST_RESOURCE: 
                break;
            case IMG_LIST_ZONELINK_FOLDER:
                {
                    bbox Bounds;
                    if (GetChildBounds(pNMTreeView->itemNew.hItem, Bounds))
                    {
                        Zone = Bounds;
                        Color = xcolor(90,200,180,70);
                    }
                }
                break;
            case IMG_LIST_ZONE: 
                {
                    //display zone propeties but on if we are in standard mode
                    if (GetDocument()->GetFramePointer()->GetEditorView()->IsStandardMode())
                    {
                        g_WorldEditor.ClearSelectedObjectList();
                        g_WorldEditor.SelectZone(m_tcLayer.GetItemText(pNMTreeView->itemNew.hItem));
                    }
                    bbox Bounds;
                    if (GetChildBounds(pNMTreeView->itemNew.hItem, Bounds))
                    {
                        Zone = Bounds;
                        Color = xcolor(0,255,0,70);
                    }
                    GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                }
                break;
            case IMG_LIST_PORTAL: 
                {
                    bbox Bounds;
                    if (GetChildBounds(pNMTreeView->itemNew.hItem, Bounds))
                    {
                        Zone = Bounds;
                        Color = xcolor(255,255,0,70);
                    }
                }
                //fallthrough 
                //...
            case IMG_LIST_OBJECT: //object
                {
                    //must cancel move mode
                    if (GetDocument()->GetFramePointer()->GetEditorView()->IsMovementMode())
                    {
                        GetDocument()->GetFramePointer()->GetEditorView()->CancelMovementMode();
                    }
                    
                    guid& ObjGuid = GetObjectGuidFromHandle(hHandle);
                    if (ObjGuid.Guid != 0)
                    {
                        if(!g_WorldEditor.IsGuidSelectMode())
                        {
                        
                            g_WorldEditor.SelectObject(ObjGuid, (( GetKeyState( VK_CONTROL ) & ~1 ) == 0));  
                            GetDocument()->GetFramePointer()->GetEditorView()->SetFocusPos(g_WorldEditor.GetMinPositionForSelected());
                        }

                    }
                    GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
                    GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                }
                break;
            case IMG_LIST_BLUEPRINT: //blueprint
                {
                    //must cancel move mode
                    if (GetDocument()->GetFramePointer()->GetEditorView()->IsMovementMode())
                    {
                        GetDocument()->GetFramePointer()->GetEditorView()->CancelMovementMode();
                    }

                    guid& BPGuid = GetBlueprintGuidFromHandle(hHandle);
                    if (BPGuid.Guid != 0)
                    {
                        if(!g_WorldEditor.IsGuidSelectMode())
                        {
                            editor_blueprint_ref BlueprintReference;
                            if (g_WorldEditor.GetBlueprintByGuid(BPGuid, BlueprintReference))
                            {
                                if ((( GetKeyState( VK_CONTROL ) & ~1 ) == 0))
                                {
                                    //control not down so clear selection list
                                    g_WorldEditor.ClearSelectedObjectList();
                                }
                                g_WorldEditor.SelectBlueprintObjects(BlueprintReference,TRUE);
                                GetDocument()->GetFramePointer()->GetEditorView()->SetFocusPos(g_WorldEditor.GetMinPositionForSelected());
                            }
                        }
                    }
                    GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
                    GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                }
                break;
            default:
                ASSERT(FALSE);
                break;
            }
        }
    }

    GetDocument()->GetFramePointer()->GetEditorView()->SetHighlightVolume(Zone, Color);
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();

	*pResult = 0;
}

//=========================================================================

BOOL CEditorLayerView::SetLayerDirtyColor(CString strLayer, HTREEITEM hLayerItem)
{
    HTREEITEM hNextItem;

    if (hLayerItem == NULL)
    {
        hLayerItem = m_tcLayer.GetRootItem( );
    }

    while (hLayerItem != NULL)
    {
        hNextItem = m_tcLayer.GetNextSiblingItem(hLayerItem);
        if (strLayer.CompareNoCase(m_tcLayer.GetItemText(hLayerItem)) == 0)
        {
            //Found it, change color
            BOOL bReadOnly = g_WorldEditor.IsLayerReadonly(strLayer);
            BOOL bDirty    = g_WorldEditor.IsLayerDirty(strLayer);
            BOOL bLoaded   = g_WorldEditor.IsLayerLoaded(strLayer);

            //set layer color
            if (bDirty && bReadOnly)
            {
                m_tcLayer.SetItemColor(hLayerItem,RGB(255,128,64));
            }
            else if (bReadOnly)
            {
                m_tcLayer.SetItemColor(hLayerItem,RGB(100,100,150));
            }   
            else if (bDirty)
            {
                m_tcLayer.SetItemColor(hLayerItem,RGB(200,0,0));
            }       
            else if (!bLoaded)
            {
                m_tcLayer.SetItemColor(hLayerItem,RGB(180,180,180));
            }
            else
            {
                m_tcLayer.SetItemColor(hLayerItem,RGB(0,0,0));
            }
            
            return TRUE;
        }
        hLayerItem = hNextItem;
    }

    TRACE("ERROR CEditorLayerView::SetLayerDirtyColor... Could not find layer\n");
    return FALSE;
}

//=========================================================================

void CEditorLayerView::SetAllLayersClean()
{
    HTREEITEM hLayerItem = m_tcLayer.GetRootItem( );

    while (hLayerItem != NULL)
    {       
        CString strItem = m_tcLayer.GetItemText(hLayerItem);
        SetLayerDirtyColor(strItem,hLayerItem);
        hLayerItem = m_tcLayer.GetNextSiblingItem(hLayerItem);
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbLoadLayer() 
{
    HTREEITEM hItem = m_tcLayer.GetSelectedItem();

    //no loading/unloading default layer
    CString strLayer = m_tcLayer.GetItemText(hItem);
    if (!strLayer.IsEmpty())
    {
        g_WorldEditor.ClearSelectedObjectList();
        if ((strLayer.CompareNoCase(g_WorldEditor.GetDefaultLayer())!=0) &&
            (strLayer.CompareNoCase(g_WorldEditor.GetGlobalLayer())!=0))
        {
            m_bInternalSelect = TRUE;
            int nImage = -1;
            int nSelectedImage = -1;
            if (m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage ))
            {
                switch(nImage)
                {
                case IMG_LIST_LAYER_LOADED: 
                    if (::AfxMessageBox("Are you sure you want to unload this layer, any unsaved data will be lost and this action can not be undone.",MB_YESNO) == IDYES )
                    {
                        if (GetDocument()->GetFramePointer()->GetEditorDoc()->UnloadLayerFile(strLayer))
                        {
                            g_SaveTrackUserSettings.UnloadedLayers.Append(strLayer.GetString());
                            //change image
                            m_tcLayer.SetItemImage(hItem, IMG_LIST_LAYER_UNLOADED, IMG_LIST_LAYER_UNLOADED+1);
                            LoadLayers();
                            GetDocument()->GetFramePointer()->OnLoadLayers();
                        }
                    }
                    break;
                case IMG_LIST_LAYER_UNLOADED: 
                    if (GetDocument()->GetFramePointer()->GetEditorDoc()->LoadLayerFile(strLayer))
                    {
                        int nIndex;
                        nIndex = g_SaveTrackUserSettings.UnloadedLayers.Find(strLayer.GetString());
                        if(nIndex != -1)
                            g_SaveTrackUserSettings.UnloadedLayers.Delete(nIndex);

                        //change image
                        m_tcLayer.SetItemImage(hItem, IMG_LIST_LAYER_LOADED, IMG_LIST_LAYER_LOADED+1);
                        LoadLayers();
                        GetDocument()->GetFramePointer()->OnLoadLayers();
                    }
                    break;
                }
                GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
                GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
            }
        }
        g_WorldEditor.PostInitialize();
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbCheckOutLayer()
{
    CWaitCursor wc;

    HTREEITEM hItem = m_tcLayer.GetSelectedItem();
    CString strLayer = m_tcLayer.GetItemText(hItem);
    if (!strLayer.IsEmpty())
    {
        if (g_WorldEditor.IsLayerDirty(strLayer))
        {
            if (::AfxMessageBox("This layer is dirty, checking it out will revert any changes. Are you sure you want to continue?", MB_YESNO) == IDNO )
            {
                return;
            }
        }

        //unload layer
        GetDocument()->GetFramePointer()->GetEditorDoc()->UnloadLayerFile(strLayer);

        xstring LayerPath = xfs("%s%s.layer", g_Project.GetWorkingPath(), (const char*)strLayer);
        //sync to perforce
        xstring P4Sync = xfs("p4 sync \"%s\"", (const char*)LayerPath );
        const char* strP4Sync = P4Sync;            
        system( strP4Sync );

        //open file for editting
        xstring P4Edit = xfs("p4 edit \"%s\"", (const char*)LayerPath );
        const char* strP4Edit = P4Edit;            
        system( strP4Edit );

        CFileStatus status;
        if( CFile::GetStatus( LayerPath, status ) )   // static function
        {
            if (!(status.m_attribute & CFile::readOnly))
            {
                g_WorldEditor.MarkLayerReadonly(strLayer, FALSE);
            }
        }

        if (g_WorldEditor.IsLayerReadonly(strLayer))
        {
            ::AfxMessageBox(xfs("Layer (%s) could not be checked out of perforce", (const char*)strLayer));
        }

        //reload layer
        GetDocument()->GetFramePointer()->GetEditorDoc()->LoadLayerFile(strLayer);
       
        //refresh
        LoadLayers();
        GetDocument()->GetFramePointer()->OnLoadLayers();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvtbLoadAllLayers() 
{
    g_WorldEditor.ClearSelectedObjectList();

    if (::AfxMessageBox("Are you sure you want to load all layers. This could take a while.",MB_YESNO) == IDYES )
    {
        xarray<xstring> ListLayers;
        g_WorldEditor.GetLayerNames(ListLayers);

        for (int i = 0; i < ListLayers.GetCount(); i++)
	    {
            CString strItem = ListLayers.GetAt(i);
            if (!g_WorldEditor.IsLayerLoaded(strItem))
            {
                //not loaded, so load this one
                VERIFY(GetDocument()->GetFramePointer()->GetEditorDoc()->LoadLayerFile(strItem));
            }
        }
        g_SaveTrackUserSettings.UnloadedLayers.Clear();

       
        GetDocument()->GetFramePointer()->RefreshAll();
        GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
        g_WorldEditor.PostInitialize();
    }
}

//=========================================================================
//=========================================================================
//=========================================================================

void CEditorLayerView::OnUpdateLvtbAddLayer(CCmdUI* pCmdUI) 
{
    CEditorView* pEditorView = GetDocument()->GetFramePointer()->GetEditorView();
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();

    pCmdUI->Enable( g_Project.IsProjectOpen() && 
                    pEditorView && pEditorView->IsStandardMode() && 
                    pEditorDoc && !pEditorDoc->IsGameRunning());	
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbAddFolder(CCmdUI* pCmdUI) 
{
    CEditorView* pEditorView = GetDocument()->GetFramePointer()->GetEditorView();
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();

    if (g_Project.IsProjectOpen() && 
        pEditorView && pEditorView->IsStandardMode() && 
        pEditorDoc && !pEditorDoc->IsGameRunning())
    {
        pCmdUI->Enable(CanModify() && !IsPortalSelected() && IsLayerEditable());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbAddResource(CCmdUI* pCmdUI) 
{
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();

    if (g_Project.IsProjectOpen() && 
        pEditorDoc && !pEditorDoc->IsGameRunning())
    {
        pCmdUI->Enable(IsLayerSelected() && IsLayerLoaded() && IsLayerEditable());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbAddZone(CCmdUI* pCmdUI) 
{
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();

    if (g_Project.IsProjectOpen() && 
        pEditorDoc && !pEditorDoc->IsGameRunning())
    {
        pCmdUI->Enable(IsLayerSelected(FALSE/*do not include default*/,FALSE/*do not include global*/) && 
            IsLayerLoaded() &&
            IsLayerEditable());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbAddPortal(CCmdUI* pCmdUI) 
{
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();
    CEditorView* pEditorView = GetDocument()->GetFramePointer()->GetEditorView();

    pCmdUI->Enable(g_Project.IsProjectOpen() && 
                   pEditorView && pEditorView->IsStandardMode() && 
                   pEditorDoc && !pEditorDoc->IsGameRunning() &&
                   !g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetGlobalLayer()) &&
                   g_WorldEditor.CanAddRemovePortals());	
    pCmdUI->SetCheck(FALSE);		
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbDeleteLayer(CCmdUI* pCmdUI) 
{
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();
    CEditorView* pEditorView = GetDocument()->GetFramePointer()->GetEditorView();

    if (g_Project.IsProjectOpen() && 
        pEditorView && pEditorView->IsStandardMode() &&
        pEditorDoc && !pEditorDoc->IsGameRunning())
    {
        pCmdUI->Enable((CanModify(TRUE) || IsResourceSelected()) && 
            IsLayerEditable());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbRenameLayer(CCmdUI* pCmdUI) 
{
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();
    CEditorView* pEditorView = GetDocument()->GetFramePointer()->GetEditorView();

    if (g_Project.IsProjectOpen()  && 
        pEditorView && pEditorView->IsStandardMode() &&
        pEditorDoc && !pEditorDoc->IsGameRunning())
    {
        pCmdUI->Enable(CanModify(TRUE) && IsLayerEditable());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbSetAsActiveLayer(CCmdUI* pCmdUI) 
{
    if (g_Project.IsProjectOpen())
    {
        pCmdUI->Enable(    (( IsLayerSelected() && IsLayerLoaded() )|| 
                              IsFolderSelected() || 
                              IsPortalSelected() ||
                            ( IsZoneSelected() && IsLayerLoaded() )) &&
                              IsLayerEditable() 
                       );	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbLightLayer(CCmdUI* pCmdUI) 
{
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();

    if (g_Project.IsProjectOpen() &&
        pEditorDoc && !pEditorDoc->IsGameRunning())
    {
        pCmdUI->Enable(IsLayerSelected() && IsLayerLoaded());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbSelectLayer(CCmdUI* pCmdUI) 
{
    CEditorView* pEditorView = GetDocument()->GetFramePointer()->GetEditorView();

    if (g_Project.IsProjectOpen() && pEditorView && pEditorView->IsStandardMode())
    {
        pCmdUI->Enable(    (( IsLayerSelected() && IsLayerLoaded() )|| 
                              IsFolderSelected() || 
                              IsPortalSelected() ||
                            ( IsZoneSelected() && IsLayerLoaded() )) &&
                              IsLayerEditable() 
                       );	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbLoadLayer(CCmdUI* pCmdUI) 
{
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();
    CEditorView* pEditorView = GetDocument()->GetFramePointer()->GetEditorView();

    if (g_Project.IsProjectOpen() && 
        pEditorView && pEditorView->IsStandardMode() &&
        pEditorDoc && !pEditorDoc->IsGameRunning())
    {
        pCmdUI->Enable(IsLayerSelected(FALSE,FALSE));	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnUpdateLvtbLoadAllLayers(CCmdUI* pCmdUI) 
{
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();
    CEditorView* pEditorView = GetDocument()->GetFramePointer()->GetEditorView();

    if (g_Project.IsProjectOpen() && 
        pEditorView && pEditorView->IsStandardMode() &&
        pEditorDoc && !pEditorDoc->IsGameRunning())
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

void CEditorLayerView::OnUpdateLvtbCheckOutLayer(CCmdUI* pCmdUI) 
{
    CEditorDoc*  pEditorDoc = GetDocument()->GetFramePointer()->GetEditorDoc();
    CEditorView* pEditorView = GetDocument()->GetFramePointer()->GetEditorView();

    if (g_Project.IsProjectOpen() && 
        pEditorView && pEditorView->IsStandardMode() &&
        pEditorDoc && !pEditorDoc->IsGameRunning())
    {
        pCmdUI->Enable(IsLayerSelected(TRUE,FALSE) && IsLayerLoaded() && !IsLayerEditable());	
    }
    else
    {
        pCmdUI->Enable(FALSE);	
    }
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorLayerView::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
    // We need cool menus
//    CXTCoolMenu	CoolMenu;
//    CoolMenu.HookWindow( this );

    // Get the selected item & find the object from that
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    if( hItem )
    {
        // Start out with the NULL object
        object* pObject = NULL;

        SetMenuSelectionItem(hItem);

        // Get the object pointer
        xhandle xh = m_tcLayer.GetItemData( hItem );
        if( xh )
        {
            guid Guid = GetObjectGuidFromHandle( xh );
            pObject = g_ObjMgr.GetObjectByGuid( Guid );
            SetMenuSelectionGuid(Guid);
        }

        // Get the Image of the tree view item, we will use this to determine the type of the item
        int nItemImage = -1;
        int nItemSelectedImage = -1;
        m_tcLayer.GetItemImage( hItem, nItemImage, nItemSelectedImage );

        // Create the menu
        CXTMenu Menu;
        Menu.CreatePopupMenu();

        SetMenuSelectionType( nItemImage);

        // Populate the menu based on object type
        switch( nItemImage )
        {
        case IMG_LIST_LAYER_LOADED: 
            
            Menu.AppendMenu( MF_DISABLED | MF_STRING   , ID_WERM_HIDE_THIS, _T("Layer") );
            Menu.AppendMenu( MF_DISABLED | MF_SEPARATOR, 1, _T("") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_HIDEALL_THIS, _T("Hide all objects in layer") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SHOWALL_THIS, _T("Show all objects in layer") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_FREEZEALL_THIS, _T("Freeze all objects in layer") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_THAWALL_THIS, _T("Thaw all objects in layer") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SELECTALL_THIS, _T("Select all objects in layer") );
            break;
        case IMG_LIST_LAYER_UNLOADED: 
            Menu.AppendMenu( MF_DISABLED | MF_STRING   , ID_WERM_HIDE_THIS, _T("Layer (not loaded)") );
            break;
        case IMG_LIST_FOLDER: 
            Menu.AppendMenu( MF_DISABLED | MF_STRING   , ID_WERM_HIDE_THIS, _T("Folder") );
            Menu.AppendMenu( MF_DISABLED | MF_SEPARATOR, 1, _T("") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_HIDEALL_THIS, _T("Hide all objects in folder") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SHOWALL_THIS, _T("Show all objects in folder") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_FREEZEALL_THIS, _T("Freeze all objects in folder") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_THAWALL_THIS, _T("Thaw all objects in folder") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING    , ID_LVRM_SELECTALL_THIS, _T("Select all objects in folder") );

            break;
        case IMG_LIST_RESOURCE: 
            Menu.AppendMenu( MF_DISABLED | MF_STRING   , 1, _T("Resource") );
            break;
        case IMG_LIST_ZONELINK_FOLDER:
            Menu.AppendMenu( MF_DISABLED | MF_STRING   , ID_WERM_HIDE_THIS, _T("Zonelink Folder") );
            Menu.AppendMenu( MF_DISABLED | MF_SEPARATOR, 1, _T("") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   ,  ID_LVRM_HIDEALL_THIS, _T("Hide all objects in zonelink folder") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SHOWALL_THIS, _T("Show all objects in zonelink folder") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_FREEZEALL_THIS, _T("Freeze all objects in zonelink folder") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_THAWALL_THIS, _T("Thaw all objects in zonelink folder") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING    , ID_LVRM_SELECTALL_THIS, _T("Select all objects in zonelink folder") );
            break;
        case IMG_LIST_ZONE: 
            Menu.AppendMenu( MF_DISABLED | MF_STRING   , ID_WERM_HIDE_THIS, _T("Zone") );
            Menu.AppendMenu( MF_DISABLED | MF_SEPARATOR, 1, _T("") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_HIDEALL_THIS, _T("Hide all objects in zone") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SHOWALL_THIS, _T("Show all objects in zone") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_FREEZEALL_THIS, _T("Freeze all objects in zone") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_THAWALL_THIS, _T ("Thaw all objects in zone") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SELECTALL_THIS, _T("Select all all objects in zone") );

            break;
        case IMG_LIST_PORTAL: 
            Menu.AppendMenu( MF_DISABLED | MF_STRING   , ID_WERM_HIDE_THIS, _T("Portal") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SELECTALL_THIS, _T("Select all objects in Portal") );
            break;
        case IMG_LIST_BLUEPRINT:
            Menu.AppendMenu( MF_DISABLED | MF_STRING   , ID_WERM_HIDE_THIS, _T("Blueprint") );
            Menu.AppendMenu( MF_DISABLED | MF_SEPARATOR, 1, "" );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   ,  ID_LVRM_HIDE_ALL, xfs("Hide all blueprints") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   ,  ID_LVRM_SHOW_ALL, xfs("Show all blueprints") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   ,  ID_LVRM_FREEZE_ALL, xfs("Freeze all blueprints") );
            Menu.AppendMenu( MF_ENABLED | MF_STRING   ,  ID_LVRM_THAW_ALL, xfs("Thaw all blueprints") );
            break;
        case IMG_LIST_OBJECT:
            {
                object::type ObjectType = pObject ? pObject->GetType() : object::TYPE_NULL;
                switch( ObjectType )
                {
                case object::TYPE_NULL:
                    Menu.AppendMenu( MF_DISABLED | MF_STRING   , 1, _T("Unknown object") );
                    break;
                default:
                    {
                        xstring ObjectType = pObject->GetTypeDesc().GetTypeName();

                        Menu.AppendMenu( MF_DISABLED | MF_STRING   , ID_WERM_HIDE_THIS, ObjectType );
                        Menu.AppendMenu( MF_DISABLED | MF_SEPARATOR, 1, "" );
                        Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_HIDE_THIS, "Hide" );
                        Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SHOW_THIS, "Show" );
                        Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_FREEZE_THIS, "Freeze" );
                        Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_THAW_THIS, "Thaw" );
                        Menu.AppendMenu( MF_DISABLED | MF_SEPARATOR, 1, "" );
                        if(pObject->GetAnimGroupPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_HIDE_ANIM_THIS, xfs("Hide all %s objects by Animation", (const char*)ObjectType) );
                        if(pObject->GetGeomPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_HIDE_RIGID_THIS, xfs("Hide all %s objects by Rigid Geom", (const char*)ObjectType) );

                        if(pObject->GetAnimGroupPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SHOW_ANIM_THIS, xfs("Show all %s objects by Animation", (const char*)ObjectType) );
                        if(pObject->GetGeomPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SHOW_RIGID_THIS, xfs("Show all %s objects by Ridgid Geom", (const char*)ObjectType) );

                        if(pObject->GetAnimGroupPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_FREEZE_ANIM_THIS, xfs("Freeze all %s objects by Animation", (const char*)ObjectType) );
                        if(pObject->GetGeomPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_FREEZE_RIGID_THIS, xfs("Freeze all %s objects by Ridgid Geom", (const char*)ObjectType) );

                        if(pObject->GetAnimGroupPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_THAW_ANIM_THIS, xfs("Thaw all %s objects Animation", (const char*)ObjectType) );
                        if(pObject->GetGeomPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_THAW_RIGID_THIS, xfs("Thaw all %s objects Ridgid Geom", (const char*)ObjectType) );

                        if(pObject->GetAnimGroupPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SELECTALL_ANIM_THIS, xfs("Select all %s objects by Animation", (const char*)ObjectType) );
                        if(pObject->GetGeomPtr())
                            Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SELECTALL_RIGID_THIS, xfs("Select all %s objects by Ridgid Geom", (const char*)ObjectType) );
                    }
                    break;
                }
            }
            break;
        default:
            Menu.AppendMenu( MF_DISABLED | MF_STRING, ID_WERM_HIDE_THIS, _T("Unkown Object") );
            break;
        }

        // Add some general options
        Menu.AppendMenu( MF_DISABLED | MF_SEPARATOR, 1, _T("") );
        Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_HIDE_ALL, _T("Hide all objects") );
        Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_SHOW_ALL, _T("Show all objects") );
        Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_FREEZE_ALL, _T("Freeze all objects") );
        Menu.AppendMenu( MF_ENABLED | MF_STRING   , ID_LVRM_THAW_ALL, _T("Thaw all objects") );
        Menu.AppendMenu( MF_ENABLED | MF_STRING   ,ID_LVRM_SELECT_ALL, _T("Select all objects") );

        // Display and run the menu
        CPoint p;
        ::GetCursorPos( &p );
        Menu.TrackPopupMenu( TPM_LEFTALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON, p.x, p.y, GetParentFrame() );

        // Cleanup
        Menu.DestroyMenu();
    }

    // Handled
    *pResult = 0;
}

//=========================================================================

void CEditorLayerView::OnNMDblClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    // Get the selected item & find the object from that
    HTREEITEM hItem = m_tcLayer.GetSelectedItem( );
    if( hItem )
    {
        // Get the xhandle from the items data
        xhandle xh = m_tcLayer.GetItemData( hItem );

        // Hack to get type of item
        int nImage = -1;
        int nSelectedImage = -1;
        m_tcLayer.GetItemImage( hItem, nImage, nSelectedImage );

        if( xh != HNULL )
        {
            // Get the Image of the tree view item, we will use this to determine the type of the item
            int nItemImage = -1;
            int nItemSelectedImage = -1;
            m_tcLayer.GetItemImage( hItem, nItemImage, nItemSelectedImage );

            switch( nItemImage )
            {
            case IMG_LIST_BLUEPRINT:
                {
                    guid& BPGuid = GetBlueprintGuidFromHandle( xh );
                    if(g_WorldEditor.IsGuidSelectMode())
                    {
                        g_WorldEditor.TreeDoGuidSelect(BPGuid);
                        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();

                    }
                    else if (BPGuid.Guid != 0 && !g_WorldEditor.IsGuidSelectMode())
                    {
                        editor_blueprint_ref BlueprintReference;
                        if (g_WorldEditor.GetBlueprintByGuid(BPGuid, BlueprintReference))
                        {
                            // Move the camera focus and give the 3d view input focus
                            GetDocument()->GetFramePointer()->GetEditorView()->FocusCameraWithUndoAndNoRotation( BlueprintReference.GetAnchorPosition() );
                            GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
                        }
                    }
                }
                break;

            case IMG_LIST_OBJECT:
            case IMG_LIST_PORTAL:
                {
                    guid Guid = GetObjectGuidFromHandle( xh );
                    object* pObject = g_ObjMgr.GetObjectByGuid( Guid );
                    if( pObject && !g_WorldEditor.IsGuidSelectMode())
                    {
                        vector3 Pos;

                        // Get the position to focus on
                        switch( pObject->GetType() )
                        {
                        case object::TYPE_PLAY_SURFACE:
                            Pos = pObject->GetBBox().GetCenter();
                            break;
                        default:
                            Pos = pObject->GetPosition();
                            break;
                        }   
                        if(!g_WorldEditor.IsGuidSelectMode())
                        {
              
                            // Move the camera focus and give the 3d view input focus
                            GetDocument()->GetFramePointer()->GetEditorView()->FocusCameraWithUndoAndNoRotation( Pos );
                            GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
                        }
                    }
                    else if(g_WorldEditor.IsGuidSelectMode())
                    {
                        g_WorldEditor.TreeDoGuidSelect(Guid);
                        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                    }
                }
                break;

                case IMG_LIST_LAYER_LOADED:
                case IMG_LIST_ZONE:
                case IMG_LIST_ZONELINK_FOLDER:
                case IMG_LIST_FOLDER:
                {
                    bbox Bounds;
                    if( GetChildBounds( hItem, Bounds ) )
                    {
                        // Move the camera focus and give the 3d view input focus
                        GetDocument()->GetFramePointer()->GetEditorView()->FocusCameraWithUndoAndNoRotation( Bounds.GetCenter() );
                        GetDocument()->GetFramePointer()->GetEditorView()->SetFocus();
                    }
                }
                break;
            }
        }
    }

    // Processed - this stops the default expand / collapse on double click behaviour
    *pResult = 1;
}

//=========================================================================

void CEditorLayerView::TrackOpenTreeFolders( void )
{
    HTREEITEM hCurrent, Root;
    HTREEITEM hNext;
    CString FolderPath;

    SCROLLINFO ScrollBar;
    ZeroMemory(&ScrollBar, sizeof(SCROLLINFO));
    ScrollBar.cbSize = sizeof(SCROLLINFO);
    g_SaveTrackUserSettings.ScrollBarPos = 0;
    if(m_tcLayer.GetScrollInfo(SB_VERT, &ScrollBar, SIF_ALL))
    {
        g_SaveTrackUserSettings.ScrollBarPos = ScrollBar.nPos;
    }

    Root = hCurrent = m_tcLayer.GetRootItem();

    while(hCurrent)
    {    
        //Is it expanded 
        if(m_tcLayer.GetItemState(hCurrent, TVIS_EXPANDED) & TVIS_EXPANDED)
        {
             FolderPath += m_tcLayer.GetItemText(hCurrent) + '\\';
 
            //go down one level

            hNext = m_tcLayer.GetChildItem(hCurrent);
            if(hNext)
                hCurrent = hNext;
            else
            {
                g_SaveTrackUserSettings.TreeView.Append(FolderPath.GetString());
                FolderPath.Empty();
//                hCurrent = Root;
//                hCurrent = m_tcLayer.GetNextSiblingItem(Root);
                hCurrent = m_tcLayer.GetParentItem(hCurrent);
                hCurrent = m_tcLayer.GetNextSiblingItem(hCurrent);
            }
        }
        else
        {
            hNext = m_tcLayer.GetNextSiblingItem(hCurrent);
            if(hNext)
                hCurrent = hNext;
            else
            {
                g_SaveTrackUserSettings.TreeView.Append(FolderPath.GetString());
                FolderPath.Empty();
                hCurrent = m_tcLayer.GetParentItem(hCurrent);
                hCurrent = m_tcLayer.GetNextSiblingItem(hCurrent);
            }
        } 
    }
}

//=========================================================================

void CEditorLayerView::UpdateTreeFolders( void )
{
    HTREEITEM hCurrent, Root;
    xarray<CString>Folders;

    Root = hCurrent = m_tcLayer.GetRootItem();

    int ElementCount = g_LoadUpdateUserSettings.TreeView.GetCount();

    for( int i = 0, FolderCount = 0, cc = 0; i < ElementCount; i++)
    {
        //Count the open folders in this element
        CString StrFolderPath;
        CString StrCurrentFolder;
        int FolderBegin = 0, FolderEnd = 0;

        StrFolderPath.SetString(g_LoadUpdateUserSettings.TreeView.GetAt(i));
        while(FolderEnd < StrFolderPath.GetLength())
        {
            if(StrFolderPath.GetAt(FolderEnd) == '\\')
            {
                StrCurrentFolder = StrFolderPath.Mid(FolderBegin, FolderEnd - FolderBegin);
                Folders.Append(StrCurrentFolder);
                FolderCount++;
                StrCurrentFolder.Empty();
                FolderBegin = FolderEnd + 1;
            }
            FolderEnd++;
        }

        hCurrent = Root;
 
        for( int bb = 0; bb < FolderCount; bb++)
        {
            hCurrent = m_tcLayer.FindItem(Folders.GetAt( bb),FALSE,TRUE,TRUE,hCurrent);
            if(hCurrent)
            {
                if(m_tcLayer.ItemHasChildren(hCurrent))
                {
                    m_tcLayer.Expand(hCurrent, TVE_EXPAND);

                }
                else
                {
                    g_LoadUpdateUserSettings.ScrollBarPos = 0;  //do not attempt to update scroll bar
                    //This path doesn't exist
                    break;
                }
            }
            else
            {
                //This path doesn't exist
                g_LoadUpdateUserSettings.ScrollBarPos = 0;
                break;
            }
        }

    }
    SCROLLINFO ScrollBar;
    ZeroMemory(&ScrollBar, sizeof(SCROLLINFO));
    ScrollBar.cbSize = sizeof(SCROLLINFO);
    if(m_tcLayer.GetScrollInfo(SB_VERT, &ScrollBar, SIF_ALL))
    {
        ScrollBar.nPos = g_LoadUpdateUserSettings.ScrollBarPos;
    }
    m_tcLayer.SetScrollInfo(SB_VERT, &ScrollBar,1);
}

//=========================================================================

void CEditorLayerView::SetMenuSelectionType( int type)
{
    m_SelectionType = type;
}

//=========================================================================

int CEditorLayerView::GetMenuSelectionType( void )
{
    return m_SelectionType;
}

//=========================================================================

void CEditorLayerView::SetMenuSelectionItem ( HTREEITEM hItem)
{
    m_SelectionItem = hItem; 
}

//=========================================================================

HTREEITEM CEditorLayerView::GetMenuSelectionItem ( void )
{
    return m_SelectionItem;
}

//=========================================================================

void CEditorLayerView::SetMenuSelectionGuid ( guid Guid )
{
    m_Guid = Guid;
}
guid CEditorLayerView::GetMenuSelectionGuid ( void )
{
   return m_Guid;
}

//=========================================================================

void CEditorLayerView::CollectAllObjectsInFolder( xarray<guid>& ObjList)
{
    xhandle xh = NULL;
    BOOL    SkipDirectory = TRUE;
    ObjList.Clear();
    CString TextType;


    HTREEITEM hItem = GetMenuSelectionItem();
 
    //Get first Item
    hItem = m_tcLayer.GetChildItem(hItem);

    //skip any sub directories at the top
    while( SkipDirectory)
    {
        if( m_tcLayer.ItemHasChildren(hItem) )
        {
            hItem = m_tcLayer.GetNextItem( hItem );
            if(!hItem)
                SkipDirectory = FALSE;
        }
        else
            SkipDirectory = FALSE;
    }

    //Try to grab objects

    while(hItem)
    {
        xh = m_tcLayer.GetItemData(hItem);
        TextType = m_tcLayer.GetItemText(hItem);
        guid &ChildGuidFound = GetObjectGuidFromHandle(xh);      

        if( TextType.Find(".bpx", 0) != -1)
        {
            ChildGuidFound = GetBlueprintGuidFromHandle(xh);
            editor_blueprint_ref BlueprintReference;
            if (g_WorldEditor.GetBlueprintByGuid( ChildGuidFound, BlueprintReference ))
            {
                int BlueObjCount = BlueprintReference.ObjectsInBlueprint.GetCount();
                if(BlueObjCount)
                {
                    for(int ii = 0; ii < BlueObjCount; ii++ )
                        ObjList.Append(BlueprintReference.ObjectsInBlueprint.GetAt(ii));
                    ObjList.Append(BlueprintReference.Anchor);
                }
                
            }
        }
        else
        {
            ObjList.Append(ChildGuidFound);
        }
 

        hItem = m_tcLayer.GetNextItem(hItem);
        if(m_tcLayer.ItemHasChildren(hItem))
            break;
    }
}

//=========================================================================

void CEditorLayerView::CollectObjectList( xarray<guid>& ObjList)
{
    xhandle xh = NULL;
    CString TextType;
    ObjList.Clear();


    HTREEITEM hItem = GetMenuSelectionItem();

    if( m_tcLayer.ItemHasChildren(hItem) )
    {
        return;
    }

    xh = m_tcLayer.GetItemData(hItem);
    TextType = m_tcLayer.GetItemText(hItem);
    guid &ChildGuidFound = GetObjectGuidFromHandle(xh);      

    if( TextType.Find(".bpx", 0) != -1)
    {
        ChildGuidFound = GetBlueprintGuidFromHandle(xh);
        editor_blueprint_ref BlueprintReference;
        if (g_WorldEditor.GetBlueprintByGuid( ChildGuidFound, BlueprintReference ))
        {
            int BlueObjCount = BlueprintReference.ObjectsInBlueprint.GetCount();
            if(BlueObjCount)
            {
                for(int ii = 0; ii < BlueObjCount; ii++ )
                    ObjList.Append(BlueprintReference.ObjectsInBlueprint.GetAt(ii));
                ObjList.Append(BlueprintReference.Anchor);
            }

        }
    }
    else
    {
        ObjList.Append(ChildGuidFound);
    }
}

//=========================================================================
//=========================================================================
//  Right Button menu command handlers
//=========================================================================
//=========================================================================

void CEditorLayerView::OnLvrmThawThis()
{
    xarray<guid> ObjList;
    int ObjCount;
    CollectObjectList(ObjList);
    ObjCount = ObjList.GetCount();
    if(ObjList.GetCount())
    {
        for(int ii = 0; ii < ObjCount; ii++)
        {
                object* pObject = g_ObjMgr.GetObjectByGuid( GetMenuSelectionGuid());
                pObject->SetSelectable( TRUE );
        }
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmFreezeThis()
{
    xarray<guid> ObjList;
    int ObjCount;
    CollectObjectList(ObjList);
    ObjCount = ObjList.GetCount();
    if(ObjList.GetCount())
    {
        for(int ii = 0; ii < ObjCount; ii++)
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( GetMenuSelectionGuid());
            pObject->SetSelectable( FALSE );
        }
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmThawAllThis()
{
    switch(GetMenuSelectionType())
    {
    case IMG_LIST_FOLDER:
    case IMG_LIST_ZONE:
    case IMG_LIST_PORTAL:
    case IMG_LIST_ZONELINK_FOLDER:
        {
            xarray <guid> ObjList;
            CollectAllObjectsInFolder( ObjList );
            if(ObjList.GetCount())
            {
                g_WorldEditor.MakeSelectUnSelectObjectsByFolder(ObjList, TRUE);
                GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
            }
        }
        break;
    case IMG_LIST_LAYER_LOADED:
        {
            CString TextType;
            HTREEITEM hItem = GetMenuSelectionItem();
            TextType = m_tcLayer.GetItemText(hItem);
            g_WorldEditor.MakeSelectUnSelectObjectsByLayer(TextType, TRUE);
            GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        }
    break;
    }

}

//=========================================================================

void CEditorLayerView::OnLvrmFreezeAllThis()
{
    switch(GetMenuSelectionType())
    {
    case IMG_LIST_FOLDER:
    case IMG_LIST_ZONE:
    case IMG_LIST_PORTAL:
    case IMG_LIST_ZONELINK_FOLDER:
        {
            xarray <guid> ObjList;
            CollectAllObjectsInFolder( ObjList );
            if(ObjList.GetCount())
            {
                g_WorldEditor.MakeSelectUnSelectObjectsByFolder(ObjList, FALSE);
                GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
            }
        }
        break;
    case IMG_LIST_LAYER_LOADED:
        {
            CString TextType;
            HTREEITEM hItem = GetMenuSelectionItem();
            TextType = m_tcLayer.GetItemText(hItem);
            g_WorldEditor.MakeSelectUnSelectObjectsByLayer(TextType, FALSE);
            GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        }
        break;

    }

}
//=========================================================================

void CEditorLayerView::OnLvrmHideThis()
{
    xarray<guid> ObjList;
    int ObjCount;
    CollectObjectList(ObjList);
    ObjCount = ObjList.GetCount();
    if(ObjList.GetCount())
    {
        for(int ii = 0; ii < ObjCount; ii++)
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( GetMenuSelectionGuid());
            pObject->SetHidden( TRUE );
        }
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}
//=========================================================================

void CEditorLayerView::OnLvrmShowThis()
{
    xarray<guid> ObjList;
    int ObjCount;
    CollectObjectList(ObjList);
    ObjCount = ObjList.GetCount();
    if(ObjCount)
    {
        for(int ii = 0; ii < ObjCount; ii++)
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( GetMenuSelectionGuid());
            pObject->SetHidden( FALSE );
        }
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }


}

//=========================================================================


void CEditorLayerView::OnLvrmHideAllThis()
{
    switch(GetMenuSelectionType())
    {
    case IMG_LIST_FOLDER:
    case IMG_LIST_ZONE:
    case IMG_LIST_PORTAL:
    case IMG_LIST_ZONELINK_FOLDER:
        {
            xarray <guid> ObjList;
            CollectAllObjectsInFolder( ObjList );
            if(ObjList.GetCount())
            {
                g_WorldEditor.MakeHiddenUnHiddenObjectsByFolder(ObjList, TRUE);
                GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
            }
        }
        break;
    case IMG_LIST_LAYER_LOADED:
        {
            CString TextType;
            HTREEITEM hItem = GetMenuSelectionItem();
            TextType = m_tcLayer.GetItemText(hItem);
            g_WorldEditor.MakeHiddenUnHiddenObjectsByLayer(TextType, TRUE);
            GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();

        }
        break;

    }
}

//=========================================================================

void CEditorLayerView::OnLvrmShowAllThis()
{
    switch(GetMenuSelectionType())
    {
    case IMG_LIST_FOLDER:
    case IMG_LIST_ZONE:
    case IMG_LIST_PORTAL:
    case IMG_LIST_ZONELINK_FOLDER:
        {
            xarray <guid> ObjList;
            CollectAllObjectsInFolder( ObjList );
            if(ObjList.GetCount())
            {
                g_WorldEditor.MakeHiddenUnHiddenObjectsByFolder(ObjList, FALSE);
                GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
                GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
            }
        }
        break;
    case IMG_LIST_LAYER_LOADED:
        {
            CString TextType;
            HTREEITEM hItem = GetMenuSelectionItem();
            TextType = m_tcLayer.GetItemText(hItem);
            g_WorldEditor.MakeHiddenUnHiddenObjectsByLayer(TextType, FALSE);
            GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
            GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
        }
        break;

    }
}

//=========================================================================

void CEditorLayerView::OnLvrmSelectAllThisObject()
{
    switch(GetMenuSelectionType())
    {
        case IMG_LIST_OBJECT:
            {
                //default to selecting rigid geom from this menu selection 
                g_WorldEditor.SelectObjectsByRidgedGeom( GetMenuSelectionGuid());
                GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
            }
        break;

        case IMG_LIST_FOLDER:
        case IMG_LIST_ZONE:
        case IMG_LIST_PORTAL:
        case IMG_LIST_ZONELINK_FOLDER:
            {
                xarray <guid> ObjList;
                CollectAllObjectsInFolder( ObjList );
                if(ObjList.GetCount())
                {
                    g_WorldEditor.SelectObjectsByFolder(ObjList);
                    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
                }
                break;
            }
        case IMG_LIST_LAYER_LOADED:
            {
                CString TextType;
                HTREEITEM hItem = GetMenuSelectionItem();
                TextType = m_tcLayer.GetItemText(hItem);
                g_WorldEditor.SelectAllItemsInLayer(TextType);
                GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();

            }
            break;

    }
}

//=========================================================================

void CEditorLayerView::OnLvrmHideAll()
{
    if(GetMenuSelectionType() == IMG_LIST_BLUEPRINT)
    {
        xarray <guid> ObjList;
        g_WorldEditor.GetAllObjectsInAllBluePrints(ObjList);
        s32 ObjCount = ObjList.GetCount();
        if(ObjCount)
        {
            for(int ii = 0; ii < ObjCount; ii++)
            {
                object* pObject = g_ObjMgr.GetObjectByGuid( ObjList.GetAt(ii));
                pObject->SetHidden( TRUE );
            }
        }
    }
    else
    {
        g_WorldEditor.MakeHiddenUnHiddenAllObjects(TRUE);
    }
    GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorLayerView::OnLvrmShowAll()
{
    if(GetMenuSelectionType() == IMG_LIST_BLUEPRINT)
    {
        xarray <guid> ObjList;
        g_WorldEditor.GetAllObjectsInAllBluePrints(ObjList);
        s32 ObjCount = ObjList.GetCount();
        if(ObjCount)
        {
            for(int ii = 0; ii < ObjCount; ii++)
            {
                object* pObject = g_ObjMgr.GetObjectByGuid( ObjList.GetAt(ii));
                pObject->SetHidden( FALSE );
            }
        }
    }
    else
    {
        g_WorldEditor.MakeHiddenUnHiddenAllObjects(FALSE);
    }
    GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorLayerView::OnLvrmThawAll()
{
    if(GetMenuSelectionType() == IMG_LIST_BLUEPRINT)
    {
        xarray <guid> ObjList;
        g_WorldEditor.GetAllObjectsInAllBluePrints(ObjList);
        s32 ObjCount = ObjList.GetCount();
        if(ObjCount)
        {
            for(int ii = 0; ii < ObjCount; ii++)
            {
                object* pObject = g_ObjMgr.GetObjectByGuid( ObjList.GetAt(ii));
                pObject->SetSelectable( TRUE );
            }
        }
    }
    else
    {
        g_WorldEditor.MakeSelectUnSelectAllObjects(TRUE);
    }
    GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorLayerView::OnLvrmFreezeAll()
{
    if(GetMenuSelectionType() == IMG_LIST_BLUEPRINT)
    {
        xarray <guid> ObjList;
        g_WorldEditor.GetAllObjectsInAllBluePrints(ObjList);
        s32 ObjCount = ObjList.GetCount();
        if(ObjCount)
        {
            for(int ii = 0; ii < ObjCount; ii++)
            {
                object* pObject = g_ObjMgr.GetObjectByGuid( ObjList.GetAt(ii));
                pObject->SetSelectable( FALSE);
            }
        }
    }
    else
    {
        g_WorldEditor.MakeSelectUnSelectAllObjects(FALSE);
    }
    GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorLayerView::OnLvrmSelectAll()
{
    if(GetMenuSelectionType() == IMG_LIST_BLUEPRINT)
    {
        xarray <guid> ObjList;
        g_WorldEditor.GetAllObjectsInAllBluePrints(ObjList);
        s32 ObjCount = ObjList.GetCount();
        if(ObjCount)
        {
            g_WorldEditor.SelectObjectsByFolder(ObjList);
        }
    }
    else
    {
        g_WorldEditor.SelectObjectsAll();
    }
    GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
    GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
}

//=========================================================================

void CEditorLayerView::OnLvrmSelectAllThisObjectRigid()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.SelectObjectsByRidgedGeom( GetMenuSelectionGuid());
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmSelectAllThisObjectAnimation()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.SelectObjectsByAnimation(GetMenuSelectionGuid());
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================


void CEditorLayerView::OnLvrmHideThisObjectAnimation()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.ShowHideObjectsByAnimation(GetMenuSelectionGuid(), TRUE);
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmHideThisObjectRigid()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.ShowHideObjectsByRidgedGeom(GetMenuSelectionGuid(), TRUE);
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmShowThisObjectAnimation()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.ShowHideObjectsByAnimation(GetMenuSelectionGuid(), FALSE);
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmShowThisObjectRigid()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.ShowHideObjectsByRidgedGeom(GetMenuSelectionGuid(), FALSE);
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmThawThisObjectAnimation()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.ThawFreezeObjectsByAnimation(GetMenuSelectionGuid(), TRUE);
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmThawThisObjectRigid()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.ThawFreezeObjectsByRidgedGeom( GetMenuSelectionGuid(), TRUE);
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmFreezeThisObjectAnimation()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.ThawFreezeObjectsByAnimation(GetMenuSelectionGuid(), FALSE);
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

void CEditorLayerView::OnLvrmFreezeThisObjectRigid()
{
    if(GetMenuSelectionType() == IMG_LIST_OBJECT)
    {
        g_WorldEditor.ThawFreezeObjectsByRidgedGeom( GetMenuSelectionGuid(), FALSE);
        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }
}

//=========================================================================

BOOL CEditorLayerView::DoesZoneExist(CString strLayer, CString strZone)
{
    HTREEITEM hLayerItem = m_tcLayer.GetRootItem( );

    while (hLayerItem != NULL)
    {
        hLayerItem = m_tcLayer.GetNextSiblingItem(hLayerItem);
        if (strLayer.CompareNoCase(m_tcLayer.GetItemText(hLayerItem)) == 0)
        {
            //Found the layer, does it have this zone
            if (m_tcLayer.ItemHasChildren(hLayerItem))
            {
                HTREEITEM hChildItem = m_tcLayer.GetChildItem(hLayerItem);
                while (hChildItem != NULL)
                {
                    int nImage = -1;
                    int nSelectedImage = -1;
                    if (m_tcLayer.GetItemImage( hChildItem, nImage, nSelectedImage ))
                    {
                        if (nImage == IMG_LIST_ZONE)
                        {
                            if (strZone.CompareNoCase(m_tcLayer.GetItemText(hChildItem)) == 0)
                            {
                                //found the zone
                                return TRUE;
                            }
                        }
                    }
                    hChildItem = m_tcLayer.GetNextItem(hChildItem, TVGN_NEXT);
                }
            }
        }
    }
    return FALSE;
}

