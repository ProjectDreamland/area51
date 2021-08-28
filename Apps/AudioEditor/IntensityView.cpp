//=========================================================================
// CINTENSITYVIEW.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdafx.h"
#include "IntensityView.h"
#include "SoundDoc.h"
#include "Resource.h"
#include "IntensityDialog.h"

#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\WinControls\FileSearch.h"
#include "..\Editor\Project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CIntensityView, CView)

//=========================================================================
// MESSAGE PUMP
//=========================================================================

BEGIN_MESSAGE_MAP(CIntensityView, CView)
	//{{AFX_MSG_MAP(CIntensityView)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCHITTEST()
    ON_COMMAND          ( IDM_POPUP_MENU_DELETE,            OnDelDir                )
    ON_COMMAND          ( IDM_POPUP_MENU_DESC_TOINTENSITY,  OnDescToIntensity       )
    ON_COMMAND          ( IDM_POPUP_MENU_NEW_INTENSITY,     OnNewIntensity          )
    
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntensityView drawing


//=========================================================================
// SORTING FUNCTIONS
//=========================================================================

static
s32 DelCompare( const void* pItem1, const void* pItem2 )
{
    tree_item* pTreeItem1 = (tree_item*)pItem1;
    tree_item* pTreeItem2 = (tree_item*)pItem2;

    if( pTreeItem1->m_Data > pTreeItem2->m_Data )
        return 1;
    else if( pTreeItem1->m_Data == pTreeItem2->m_Data )
        return 0;
    else
        return -1;
}

// This is the only way we can update the tree item since the window is not in focus.
static
void UpdateLabelCallBack( void* pObject, const char* pLabel )
{
    ((CIntensityView*)pObject)->UpdateTreeItemLabel( pLabel );
}


#ifdef _DEBUG


//=========================================================================

void CIntensityView::AssertValid() const
{
	CView::AssertValid();
}

//=========================================================================

void CIntensityView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

CIntensityView::CIntensityView()
{
    m_CurrentItemHit    = NULL;
    m_PrevItemHit       = NULL;
    m_RightMouseMenu    = FALSE;
    m_PackageType       = FALSE;
    m_EditLock          = 0;
    m_hItemSelList.SetGrowAmount( 10 );

    g_DescAdded.Clear();
    g_DescAdded.SetGrowAmount( 8 );
}

//=========================================================================

CIntensityView::~CIntensityView()
{
}

//=========================================================================

CSoundDoc* CIntensityView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSoundDoc)));
	return (CSoundDoc*)m_pDocument;
}

//=========================================================================

int CIntensityView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{

    if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

    CWnd::ModifyStyle( NULL, TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_EDITLABELS | TVS_SHOWSELALWAYS, 0 );

    // Create the image list used by the tree control.
	if (!m_imageList.Create (IDB_AUDIO_TREE_DRIVE_ICONS, 16, 1, RGB(0,255,0)))
		return -1;
	
	// Set the image list for the tree control.
	GetTreeCtrl().SetImageList(&m_imageList, TVSIL_NORMAL);
    
    // We should clear everything form the tree view before we call to the load the project.
    GetTreeCtrl().DeleteAllItems();
    
	return 0;
}

//=========================================================================

void CIntensityView::OnPaint() 
{
	// Helps to reduce screen flicker.
	CPaintDC dc(this);
	CRect rectClient;
	GetClientRect(&rectClient);
	CXTMemDC memDC(&dc, rectClient, xtAfxData.clrWindow);
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );

}

//=========================================================================

BOOL CIntensityView::OnEraseBkgnd(CDC* pDC) 
{
	// Helps to reduce screen flicker.
	UNUSED_ALWAYS(pDC);
	return TRUE;
}

//=========================================================================

void CIntensityView::OnOpen ( void )
{
    // We should clear everything form the tree view before we call to the load the project.
    GetTreeCtrl().DeleteAllItems();
}

//=========================================================================

void CIntensityView::OnClose ( void )
{
    GetTreeCtrl().DeleteAllItems();
    GetDocument()->m_AudioEditor.DeleteAll();
}

//=========================================================================

void CIntensityView::OnNewIntensity( void )
{
    HTREEITEM htItem = TVI_ROOT;
    CSoundDoc* pDoc = GetDocument();
    
    HTREEITEM AddedItem = GetTreeCtrl().InsertItem(_T("Intensity 0"), 0, 1, htItem);
    GetTreeCtrl().Expand(htItem, TVE_EXPAND);

    xhandle Package = pDoc->m_AudioEditor.m_PackageSelected;    
    s32 Index       = pDoc->m_AudioEditor.NewIntensity();

    // The type.
    DWORD Data = Index;
    Data |= FOLDER;
    
    // Put the full pathname as the data of the item we just inserted.
    GetTreeCtrl().SetItemData( AddedItem, Data );

    if( m_CurrentItemHit )
        GetTreeCtrl().SetItemState( m_CurrentItemHit, 0, TVIS_SELECTED );

    GetTreeCtrl().SelectItem( AddedItem );

    m_CurrentItemHit = AddedItem;
    pDoc->m_AudioEditor.m_pDesc( Package ).m_IntensitySelected = Index;
}

//=========================================================================

void CIntensityView::OnDescToIntensity( void )
{
    x_try;

    CIntensityDialog Dialog;
    CSoundDoc* pDoc = GetDocument();
    
    Dialog.InsertDescpritors( pDoc->m_AudioEditor );

    if( Dialog.DoModal() == IDOK )
    {
        xhandle Package     = pDoc->m_AudioEditor.m_PackageSelected;
        s32 IntensitySel    = pDoc->m_AudioEditor.m_pDesc( Package ).m_IntensitySelected;

        if( IntensitySel == -1 )
            x_throw( "Please select an intensity item." );

        editor_intensity& Intensity  = pDoc->m_AudioEditor.m_pDesc( Package ).m_pIntensity[ IntensitySel ];
        pDoc->m_AudioEditor.m_pDesc( Package ).SetChanged( TRUE );

        for( s32 i = 0; i < g_DescAdded.GetCount(); i ++ )
        {
            // Add the string to the intensity.
            Intensity.m_pDescriptors.Append( g_DescAdded[i] );
            
            // Add the item in the tree view.
            HTREEITEM AddedItem = GetTreeCtrl().InsertItem( g_DescAdded[i], 4, 5, m_CurrentItemHit );
            GetTreeCtrl().Expand(m_CurrentItemHit, TVE_EXPAND);

            s32 Index = Intensity.m_pDescriptors.GetCount() - 1;

            // The type.
            DWORD Data = Index;
            Data |= DESCRIPTOR;
    
            // Put the full pathname as the data of the item we just inserted.
            GetTreeCtrl().SetItemData( AddedItem, Data );
        }

        GetTreeCtrl().SelectItem( m_CurrentItemHit );
    }

    x_catch_display;
}

//=========================================================================

void CIntensityView::OnDelDir ( void )
{    
    xarray< tree_item > DelList;
    s32 i               = 0;
    s32 HighestItem     = 10000;
    HTREEITEM PrevItem  = NULL;
    CSoundDoc* pDoc = GetDocument();

    // Clear the copy array so it isn't refrencing a deleted tree item.
    m_hItemCopyList.Clear();

    DelList.SetCapacity( m_hItemSelList.GetCount() );

    if( m_hItemSelList.GetCount() == 0 )
        return;

    // The pointer to my tree control.
    CTreeCtrl& TreeCtrl = GetTreeCtrl();   
    TreeCtrl.SetRedraw( FALSE );

    // We want to delete the item in an incrementing order, meaning they should be deleted
    // element -> descriptor -> package -> folder
    // if they don't get deleted this way then we can seriously corrupt our audio editor resource.

    for( i = 0; i < m_hItemSelList.GetCount(); i++ )
    {
        tree_item tmpTreeItem;
        tmpTreeItem.m_Data = TreeCtrl.GetItemData( m_hItemSelList[i] );
        tmpTreeItem.m_hItem = m_hItemSelList[i];
        DelList.Append( tmpTreeItem );
    }

    x_qsort( &DelList[0], DelList.GetCount(), sizeof(tree_item), DelCompare );
    
    // Since the element has highest bit in the bit field we want to start from the bottom of the list
    // and then work out way up.
    for( i = DelList.GetCount()-1; i >= 0; i-- )
    {

        DWORD Data = DelList[i].m_Data;
        HTREEITEM hItem = DelList[i].m_hItem;

        RECT Rect;
        TreeCtrl.GetItemRect( hItem, &Rect, FALSE );
        if( Rect.top < HighestItem )
        {
            HighestItem = Rect.top;
            PrevItem    = TreeCtrl.GetPrevSiblingItem( hItem );
            
            // If there where to previous sibling then just go to the parent.
            if( !PrevItem )
            {
                PrevItem    = TreeCtrl.GetParentItem( hItem );
                
                // The only time we will step into this is if we have deleted the root.
                if( !PrevItem )
                {
                    HTREEITEM hRootPrevItem = TreeCtrl.GetNextItem( PrevItem, TVGN_PREVIOUS );
                    HTREEITEM hRootNextItem = TreeCtrl.GetNextItem( PrevItem, TVGN_NEXT );

                    if( hRootPrevItem )
                        PrevItem = hRootPrevItem;
                    else if( hRootNextItem )
                        PrevItem = hRootNextItem;
                    else
                        PrevItem = NULL;
                }   
            }
        }

        if( Data & FOLDER )
        {
            xhandle PackageSelected = pDoc->m_AudioEditor.m_PackageSelected;
            
            // Set the index of the descriptor we just deleted.
            s32 Index = (Data & ~FOLDER);

            pDoc->m_AudioEditor.DeleteIntensity( Index );
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_IntensitySelected = -1;
            TreeCtrl.DeleteItem( hItem);
        
            // Go through all the items next of the descriptor and change there index.
            HTREEITEM FirstItem =  TreeCtrl.GetRootItem( );
            HTREEITEM NextItem = FirstItem;//TreeCtrl.GetNextItem( FirstItem, TVGN_NEXT );
            while( NextItem )
            {

                DWORD IntensityData = TreeCtrl.GetItemData( NextItem );
                
                DWORD NextItemData = IntensityData;
                
                if( Index <= (s32)(IntensityData & ~FOLDER) )
                {
                    NextItemData = (IntensityData & ~FOLDER) - 1;
                    NextItemData |= FOLDER;
                }


                //DWORD NextItemData = Index;
                //NextItemData |= FOLDER;

                TreeCtrl.SetItemData( NextItem, NextItemData );

                NextItem = TreeCtrl.GetNextItem( NextItem, TVGN_NEXT );
                //Index++;
            }
            pDoc->m_pPropEditor->Refresh();
        }
        else if( Data & DESCRIPTOR )
        {

            // Get the parent descriptors index.
            HTREEITEM Parent = TreeCtrl.GetParentItem( hItem );
            DWORD ParentData = TreeCtrl.GetItemData( Parent );
        
            // Set the index of the descriptor we just deleted.
            s32 ParentIndex = (ParentData & ~FOLDER);
            s32 Index = (Data & ~DESCRIPTOR);

            pDoc->m_AudioEditor.DeleteIntensityDesc( ParentIndex, Index );
            TreeCtrl.DeleteItem( hItem);
            
            // Go through all the items next of the descriptor and change there index.
            HTREEITEM FirstChildItem =  TreeCtrl.GetChildItem( Parent );
            HTREEITEM NextItem = FirstChildItem;//TreeCtrl.GetNextItem( FirstChildItem, TVGN_NEXT );
            
            while( NextItem )
            {
                DWORD DescData = TreeCtrl.GetItemData( NextItem );
                
                DWORD NextItemData = DescData;
                
                if( Index <= (s32)(DescData & ~DESCRIPTOR) )
                {
                    NextItemData = (DescData & ~DESCRIPTOR) - 1;
                    NextItemData |= DESCRIPTOR;
                }

                TreeCtrl.SetItemData( NextItem, NextItemData );

                NextItem = TreeCtrl.GetNextItem( NextItem, TVGN_NEXT );
                //Index++;
            }
            pDoc->m_pPropEditor->Refresh();
        }
                
        
    }

    TreeCtrl.SetRedraw( TRUE );
    m_hItemSelList.Clear();
    
    if( PrevItem )
    {
        TreeCtrl.SelectItem( PrevItem );
    
        // Reset the current item.
        m_hItemSelList.Append( GetTreeCtrl().GetSelectedItem() );
        m_CurrentItemHit = PrevItem;
    }
}

//=========================================================================

void CIntensityView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    CXTMenu menu;
    menu.CreatePopupMenu();
    
    if( (GetDocument()->m_AudioEditor.m_PackageSelected) != -1 && (m_PackageType) )
    {
        if( m_IntensitySelected )
        {
            menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_DESC_TOINTENSITY,           "Add descriptor"    );
        }    

        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_NEW_INTENSITY,    "New Intensity"         );
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_DELETE,           "Delete"                );
    }

    m_RightMouseMenu = TRUE;

    ClientToScreen(&point);
    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
	
	CXTTreeView::OnRButtonDown(nFlags, point);
}

//=========================================================================

void CIntensityView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// We don't want the focus changing if the right mouse button was hit.
    if( m_RightMouseMenu && m_CurrentItemHit )
    {
        m_RightMouseMenu = FALSE;
//        GetTreeCtrl().SelectItem( NULL );
//        GetTreeCtrl().SetItemState( m_CurrentItemHit ,  TVIS_SELECTED , TVIS_SELECTED );
    }

    CXTTreeView::OnRButtonUp(nFlags, point);
}

//=========================================================================

void CIntensityView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CSoundDoc* Doc =  GetDocument();    

	// Shift key is down
	if( nFlags & MK_SHIFT )
	{
        // Are there any items selected by control.
        if( m_hItemSelList.GetCount() > 0 )
        {
            // Use the head of the list as the first item selected.
            m_hItemFirstSel = m_hItemSelList[ 0 ];
        }

		UINT flag;
		HTREEITEM hItem = GetTreeCtrl().HitTest( point, &flag );

		// Initialize the reference item if this is the first shift selection
		if( !m_hItemFirstSel )
			m_hItemFirstSel = GetTreeCtrl().GetSelectedItem();

		// Select new item
		if( GetTreeCtrl().GetSelectedItem() == hItem )
			GetTreeCtrl().SelectItem( NULL );			// to prevent edit

        CXTTreeView::OnLButtonDown(nFlags, point);

		if( m_hItemFirstSel )
		{
            ClearSelection();
            m_hItemSelList.Clear();
            
            AddItemsToList( m_hItemFirstSel, hItem, m_hItemSelList );

            SelectControlItem( m_hItemSelList );

            // Clear all the sample view.
            Doc->ClearAllSamples();
            
            Doc->m_pPropEditor->Refresh();

			return;
		}
	}
	else if( nFlags & MK_CONTROL )
	{

        UINT flag;
        HTREEITEM hItem = GetTreeCtrl().HitTest( point, &flag );
        HTREEITEM hSelItem = GetTreeCtrl().GetSelectedItem();

		// Select new item
		if( hSelItem == hItem )
			GetTreeCtrl().SelectItem( NULL );			// to prevent edit

        // Since its the first one then we want to insert the item was selected before we hit ctrl.
        if( m_hItemSelList.GetCount() == 0 && hSelItem )
        {
            m_hItemSelList.Append( hSelItem );
        }

        for( s32 i = 0; i < m_hItemSelList.GetCount(); i++ )
        {
            // If we click on the same item then just deselect it.
			if( m_hItemSelList[i] == hItem )
            {
                CXTTreeView::OnLButtonDown(nFlags, point);
                ClearSelection();
                
				m_hItemSelList.Delete( i );
                SelectControlItem( m_hItemSelList );

                // Clear all the sample view.
                Doc->ClearAllSamples();

                return;

            }
        }

        m_hItemSelList.Append( hItem );
        CXTTreeView::OnLButtonDown(nFlags, point);

        SelectControlItem( m_hItemSelList );
        
        // Clear all the sample view.
        Doc->ClearAllSamples();

        Doc->m_pPropEditor->Refresh();

        return;

	}
	else
	{
		// Normal - remove all selection and let default 
		// handler do the rest
		ClearSelection();
        m_hItemSelList.Clear();
        
        CXTTreeView::OnLButtonDown(nFlags, point);
        
        HTREEITEM hSelItem = GetTreeCtrl().GetSelectedItem();

        if( hSelItem )
            m_hItemSelList.Append( hSelItem );
        else
            return;

		m_hItemFirstSel = NULL;
        Doc->m_AudioEditor.SetMultiSelect( FALSE );
	}


    HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
    if( !hItem )
        return;

    // To prevent edit in the tree view.
    if( m_CurrentItemHit == hItem )
    {
        GetTreeCtrl().SelectItem( NULL );
        GetTreeCtrl().SetItemState( hItem ,  TVIS_SELECTED , TVIS_SELECTED );
    }

    // What item was previously hit.
    m_PrevItemHit = m_CurrentItemHit;
    m_CurrentItemHit = hItem;

    // Get the item's data and in the doc set that as the current selected item data.
    DWORD Data = GetTreeCtrl().GetItemData( hItem );
    Doc->m_SelectedItemData = Data;

    if( Data & FOLDER )
    {
        xhandle PackageSelected = Doc->m_AudioEditor.m_PackageSelected;

        // Don't do anything and change the selection type.
        if( (Doc->m_AudioEditor.m_pDesc.GetCount()) && (PackageSelected != -1) )
        {
            m_IntensitySelected = TRUE;
            Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected = 0;
            Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected |=  INTENSITY_SELECTED;

            Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_IntensitySelected = ( Data & ~FOLDER );

            Doc->m_pPropEditor->Refresh();
        
            s32 IntensitySelected = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_IntensitySelected;
        
            s32 Level = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pIntensity[ IntensitySelected ].m_Level;
            GetTreeCtrl().SetItemText( m_CurrentItemHit, xfs( "Intensity %d", Level) );
        }        
    }
    else if( Data & DESCRIPTOR )
    {
        m_IntensitySelected = TRUE;
    }
}

//=========================================================================

UINT CIntensityView::OnNcHitTest(CPoint point) 
{
#if _MSC_VER >= 1200 //MFC 6.0
	UINT uFlag=0;

	// Get the cursor location in client coordinates.
	CPoint pt = point;	
	ScreenToClient(&pt);

	// Get a pointer to the tooltip control.
	CToolTipCtrl* pCtrl = GetTreeCtrl().GetToolTips();

	// If we have a valid tooltip pointer and the cursor
	// is over a tree item, the bring the tooltip control
	// to the top of the Z-order.
	if (pCtrl && GetTreeCtrl().HitTest(pt, &uFlag)){
		pCtrl->SetWindowPos(&wndTop,0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE |SWP_NOMOVE);
	}
#endif //MFC 6.0
	
	return CXTTreeView::OnNcHitTest(point);
}

//=========================================================================

void CIntensityView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{

    switch( lHint )
    {
        case REFRESH_INTENSITY: Refresh();          break;
        case SORT_DESCENDING:   OnSortDescending(); break;
        default: break;
    }
}

//=========================================================================

void CIntensityView::OnSortDescending( void )
{
    CTreeCtrl& TreeCtrl     = GetTreeCtrl();
    HTREEITEM  hRootItem    = TreeCtrl.GetRootItem();
    HTREEITEM  hItem        = TreeCtrl.GetChildItem( hRootItem );
    
    TreeCtrl.SortChildren( NULL );
    while( hItem )
    {
        TreeCtrl.SortChildren( hItem );
        hItem = TreeCtrl.GetNextItem( hItem, TVGN_NEXT );
    }
}

//=========================================================================

void CIntensityView::ClearSelection( void )
{
    CSoundDoc* Doc =  GetDocument();
    Doc->m_AudioEditor.ClearMultiSelIndex();    

    for( s32 i = 0; i < m_hItemSelList.GetCount(); i++ )
            GetTreeCtrl().SetItemState( m_hItemSelList[i], 0, TVIS_SELECTED );
}

//=========================================================================

xbool CIntensityView::AddItemsToList(HTREEITEM hItemFrom, HTREEITEM hItemTo, xarray<HTREEITEM>& ItemList)
{
	HTREEITEM hItem = GetTreeCtrl().GetRootItem();

	// Clear selection upto the first item
	while ( (hItem) && (hItem != hItemFrom) && (hItem != hItemTo) )
	{
		hItem = GetTreeCtrl().GetNextVisibleItem( hItem );
		GetTreeCtrl().SetItemState( hItem, 0, TVIS_SELECTED );
	}

	if ( !hItem )
		return FALSE;	// Item is not visible

	GetTreeCtrl().SelectItem( hItemTo );

	// Rearrange hItemFrom and hItemTo so that hItemFirst is at top
	if( hItem == hItemTo )
	{
		hItemTo   = hItemFrom;
		hItemFrom = hItem;
	}


	// Go through remaining visible items
	BOOL bSelect = TRUE;
	while ( hItem )
	{
		// Select or remove selection depending on whether item
		// is still within the range.
        if( bSelect )
            ItemList.Append( hItem );

		// Do we need to start removing items from selection
		if( hItem == hItemTo ) 
			bSelect = FALSE;

		hItem = GetTreeCtrl().GetNextVisibleItem( hItem );
	}

	return TRUE;
}

//=========================================================================

HTREEITEM CIntensityView::GetFirstSelectedItem()
{
	for ( HTREEITEM hItem = GetTreeCtrl().GetRootItem(); hItem != NULL; hItem = GetNextItem( hItem ) )
		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CIntensityView::GetNextSelectedItem( HTREEITEM hItem )
{
	for ( hItem = GetNextItem( hItem ); hItem != NULL; hItem = GetNextItem( hItem ) )
		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CIntensityView::GetPrevSelectedItem( HTREEITEM hItem )
{
	for ( hItem = GetPrevItem( hItem ); hItem!=NULL; hItem = GetPrevItem( hItem ) )
		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CIntensityView::GetNextItem( HTREEITEM hItem )
{
    HTREEITEM hNextItem;

    if( GetTreeCtrl().ItemHasChildren( hItem ) )
	{
		// return first child.
		return GetTreeCtrl().GetChildItem( hItem );
	}
    else
	{
        // return next sibling item
        // Go up the tree to find a parent's sibling if needed.
        while( (hNextItem = GetTreeCtrl().GetNextSiblingItem( hItem )) == NULL )
		{
			if( (hItem = GetTreeCtrl().GetParentItem( hItem ) ) == NULL )
					return NULL;
        }
    }
    return hNextItem;
}

//=========================================================================

void CIntensityView::SelectControlItem ( xarray<HTREEITEM>& ItemList )
{
    x_DebugMsg( "%d Items in the list\n", ItemList.GetCount() );

    CSoundDoc* Doc =  GetDocument();
    Doc->m_AudioEditor.SetMultiSelect( TRUE );

    s32 i = 0;

    ClearSelection();

    for( i = 0; i < ItemList.GetCount(); i++ )
    {
        GetTreeCtrl().SetItemState( ItemList[i] ,  TVIS_SELECTED , TVIS_SELECTED );
        DWORD Data = GetTreeCtrl().GetItemData( ItemList[i] );
                
        multi_sel Selection;

        if( Data & DESCRIPTOR )
        {
            // Get the package index.
            HTREEITEM hParent = GetTreeCtrl().GetNextItem( ItemList[i], TVGN_PARENT );
            DWORD ParentData = GetTreeCtrl().GetItemData( hParent );

            // Set the package and descriptor index.
            Selection.m_SelectedType   |= DESCRIPTOR_SELECTED;
            Selection.m_PackageHandle   = ( ParentData & ~PACKAGE);    
            Selection.m_DescriptorIndex = ( Data & ~DESCRIPTOR );
        }

        // Don't inset any selection indexes for a folder.
        if( !(Data & FOLDER) )
            Doc->m_AudioEditor.InsertMultiSelIndex( Selection );
    }
}

//=========================================================================

void CIntensityView::UpdateTreeItemLabel (  const char* pLabel )
{
    if( m_CurrentItemHit )
    {
        // Set the items text to the label of the descriptor.
        GetTreeCtrl().SetItemText( m_CurrentItemHit, xfs( "Intensity %s", pLabel) );
    }
}

//=========================================================================

void CIntensityView::OnTabActivate( BOOL bActivate )
{
    if( bActivate )
    {
        CSoundDoc* pDoc = GetDocument();

        pDoc->m_AudioEditor.ClearParamsMode();
        pDoc->m_pPropEditor->Refresh();

        xhandle PackageSelected = pDoc->m_AudioEditor.m_PackageSelected;
    
        pDoc->m_AudioEditor.SetUpdateLabelFn( this, UpdateLabelCallBack );

        if( PackageSelected != -1 )
        {
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_PrevSelected = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_Selected;
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected = 0;
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected |=  INTENSITY_SELECTED;
        
            m_PackageType = x_strlen(pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pType);
            pDoc->m_pPropEditor->Refresh();
        }
    
        m_IntensitySelected = FALSE;
    }
}

//=========================================================================

void CIntensityView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
    if( bActivate )
    {
        Refresh();

        CSoundDoc* pDoc = GetDocument();

        pDoc->m_AudioEditor.ClearParamsMode();
        pDoc->m_pPropEditor->Refresh();

        xhandle PackageSelected = pDoc->m_AudioEditor.m_PackageSelected;
        
        pDoc->m_AudioEditor.SetUpdateLabelFn( this, UpdateLabelCallBack );

        if( PackageSelected != -1 )
        {
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_PrevSelected = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_Selected;
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected = 0;
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected |=  INTENSITY_SELECTED;
            
            m_PackageType = x_strlen(pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pType);
            pDoc->m_pPropEditor->Refresh();
        }
        
        m_IntensitySelected = FALSE;
    }

	CXTTreeView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

//=========================================================================

void CIntensityView::Refresh ( void )
{
    CSoundDoc* pDoc = GetDocument();
    if( pDoc->m_AudioEditor.m_PackageSelected == -1 )
        return;

    CTreeCtrl& TreeCtrl = GetTreeCtrl();
    TreeCtrl.DeleteAllItems();
    
    xhandle Package = pDoc->m_AudioEditor.m_PackageSelected;
    if( !x_strlen(pDoc->m_AudioEditor.m_pDesc( Package ).m_pType) )
    {
        x_try;
            x_throw( "No Type set for this audio package, unable to setup view" );
        x_catch_display;

        return;
    }

    editor_audio_package& rPackage = pDoc->m_AudioEditor.m_pDesc( Package );

    m_CurrentItemHit    = NULL;
    m_PrevItemHit       = NULL;
    m_RightMouseMenu    = FALSE;
    m_EditLock          = 0;

    for( s32 i = 0; i < rPackage.m_pIntensity.GetCount(); i++ )
    {
        editor_intensity& rIntensity = rPackage.m_pIntensity[i];

        HTREEITEM htItem = TVI_ROOT;
        HTREEITEM AddedItem = GetTreeCtrl().InsertItem( xfs("Intensity %d", rIntensity.m_Level), 0, 1, htItem );
        GetTreeCtrl().Expand(htItem, TVE_EXPAND);

        s32 Index = i;

        // The type.
        DWORD Data = Index;
        Data |= FOLDER;
    
        // Put the full pathname as the data of the item we just inserted.
        GetTreeCtrl().SetItemData( AddedItem, Data );
        
        for( s32 j = 0; j < rIntensity.m_pDescriptors.GetCount(); j++ )
        {
            // Add the item in the tree view.
            HTREEITEM AddedDesc = GetTreeCtrl().InsertItem( rIntensity.m_pDescriptors[j], 4, 5, AddedItem);
            GetTreeCtrl().Expand(AddedDesc, TVE_EXPAND);

            s32 Index = j;

            // The type.
            DWORD Data = Index;
            Data |= DESCRIPTOR;
    
            // Put the full pathname as the data of the item we just inserted.
            GetTreeCtrl().SetItemData( AddedDesc, Data );
        }

    }
}

//=========================================================================