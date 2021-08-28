//==============================================================================
// AUDIODIRECTORYVIEW.CPP
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "stdafx.h"
#include "AudioDirectoryView.h"
#include "SoundDoc.h"
#include "Resource.h"
#include "..\WinControls\FileSearch.h"
#include "..\WorldEditor\Resource.h"
#include "..\Editor\Project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



IMPLEMENT_DYNCREATE(CAudioDirectoryView, CView)

//=========================================================================
// MESSAGE PUMP
//=========================================================================
BEGIN_MESSAGE_MAP(CAudioDirectoryView, CView)
	//{{AFX_MSG_MAP(CAudioDirectoryView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_COMMAND(IDM_POPUP_ADD_ELEMENT,       OnAddElement)
	ON_COMMAND(IDM_POPUP_CREATE_TO_DESC,    OnCreateDescriptor)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//=========================================================================

CAudioDirectoryView::CAudioDirectoryView() :
m_bUsePrev(FALSE)
{
    m_bInit = FALSE;
    m_hItemFirstSel = NULL;
    m_hItemSelList.SetGrowAmount( 10 );
}

//=========================================================================

CAudioDirectoryView::~CAudioDirectoryView()
{
    if (m_bInit)
	    m_imageList.DeleteImageList();
}

//=========================================================================

void CAudioDirectoryView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//=========================================================================

#ifdef _DEBUG
void CAudioDirectoryView::AssertValid() const
{
	CXTTreeView::AssertValid();
}

//=========================================================================

void CAudioDirectoryView::Dump(CDumpContext& dc) const
{
	CXTTreeView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

void CAudioDirectoryView::OnInitialUpdate() 
{
	CXTTreeView::OnInitialUpdate();
	
    Refresh();
	// TODO: Add your specialized code here and/or call the base class
	
}

//=========================================================================

void CAudioDirectoryView::OnPaint() 
{
	// Helps to reduce screen flicker.
	CPaintDC dc(this);
	CRect rectClient;
	GetClientRect(&rectClient);
	CXTMemDC memDC(&dc, rectClient, xtAfxData.clrWindow);
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );
}

//=========================================================================

BOOL CAudioDirectoryView::OnEraseBkgnd(CDC* pDC) 
{
	// Helps to reduce screen flicker.
	UNUSED_ALWAYS(pDC);
	return TRUE;
}

//=========================================================================

int CAudioDirectoryView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;


    CWnd::ModifyStyle( NULL, TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_EDITLABELS | TVS_SHOWSELALWAYS, 0 );

    CString strPath(CString(g_Settings.GetSourcePath()) + "\\Audio");

    GetDocument()->m_AudioSourcePath = strPath;
    BuildTreeFromPath(strPath, "*.aif", "");
    //BuildTreeFromPath( strPath, "*.wav", "");
    Refresh();
    
    return 0;
}

//=========================================================================

void CAudioDirectoryView::OnSize(UINT nType, int cx, int cy) 
{
	CXTTreeView::OnSize(nType, cx, cy);
//	CRect rect(0,0,cx,cy );
//   GetTreeCtrl().MoveWindow(&rect);	
}

//=========================================================================

CSoundDoc* CAudioDirectoryView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSoundDoc)));
	return (CSoundDoc*)m_pDocument;
}

//=========================================================================

void CAudioDirectoryView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
    (void)pSender;
    (void)lHint;
    (void)pHint;

    if( lHint == SET_AUDIO_SOURCE_PATH )
    {
        ClearTree();
        BuildTreeFromPath( GetDocument()->m_AudioSourcePath, "*.aif", "");
        //BuildTreeFromPath( GetDocument()->m_AudioSourcePath, "*.wav", "");
        Refresh();
    }
    else if( lHint == ADD_AUDIO_ELEMENT )
    {
        OnAddElement();
    }
    else if( lHint == SAMPLE_TO_DESCRIPTOR )
    {
        OnCreateDescriptor();
    }
}

//=========================================================================

void CAudioDirectoryView::OnCreateDescriptor ( void )
{
    s32 SelCount = m_hItemSelList.GetCount();
    
    for( s32 i = 0; i < SelCount; i++)
    {
        GetTreeCtrl().SetItemState( m_hItemSelList[i] ,  TVIS_SELECTED , TVIS_SELECTED );
        CString strPath = ItemToPath( m_hItemSelList[i] );

        // Make sure that we only get a aif sample.
        if (!strPath.IsEmpty() && !IsFolder(strPath))
        {
            int iIndex = strPath.ReverseFind('.');
            CString strExtention = strPath.Right(strPath.GetLength()-iIndex-1);
            if (strExtention.CompareNoCase("aif")==0)
            {
                GetDocument()->CreateDescFromSample( strPath ); 
            }
        }
    }
}

//=========================================================================

void CAudioDirectoryView::OnAddElement ( void )
{
    s32 SelCount = m_hItemSelList.GetCount();
    
    for( s32 i = 0; i < SelCount; i++)
    {
        GetTreeCtrl().SetItemState( m_hItemSelList[i] ,  TVIS_SELECTED , TVIS_SELECTED );
        CString strPath = ItemToPath( m_hItemSelList[i] );

        // Make sure that we only get a aif sample.
        if (!strPath.IsEmpty() && !IsFolder(strPath))
        {
            int iIndex = strPath.ReverseFind('.');
            CString strExtention = strPath.Right(strPath.GetLength()-iIndex-1);
            if ( strExtention.CompareNoCase("aif")==0 )
            {
                GetDocument()->OnNewAudioElement( strPath ); 
            }
        }
    }
}

//=========================================================================

void CAudioDirectoryView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    CXTMenu menu;
    menu.CreatePopupMenu();
    
    // Menu items.
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_ADD_ELEMENT,           "Add Element"    );    
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_CREATE_TO_DESC,        "Create To Desc (Ctrl + W)" );

    ClientToScreen(&point);
    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
	
	CXTTreeView::OnRButtonDown(nFlags, point);
}

//=========================================================================

void CAudioDirectoryView::BuildTreeFromPath(CString strRootPath, CString strWildcard, CString strForcedExt)
{
    if (!m_bInit)
    {
	    // Create the image list used by the tree control.
	    if (!m_imageList.Create (IDB_AUDIO_TREE_DRIVE_ICONS, 16, 1, RGB(0,255,0)))
        {
            ASSERT(FALSE);
		    return;
        }

	    // Set the image list for the tree control.
	    GetTreeCtrl().SetImageList(&m_imageList, TVSIL_NORMAL);
        m_bInit = TRUE;
    }

    if (!strRootPath.IsEmpty())
    {
        CFileSearch::FormatPath(strRootPath);

        tree_structure_info& tree_item = m_xaTreeStruct.Add();
        x_strcpy(tree_item.cPath,strRootPath);
        x_strcpy(tree_item.cWildcard,strWildcard);
        x_strcpy(tree_item.cForcedExt,strForcedExt);
    }
}

//=========================================================================

void CAudioDirectoryView::ClearTree()
{
    GetTreeCtrl().DeleteAllItems();
    m_xaTreeStruct.Clear();
}

//=========================================================================

void CAudioDirectoryView::Refresh()
{
    GetTreeCtrl().DeleteAllItems(); //clear gui

    xhandle hHandle;
    hHandle.Handle = HNULL;
    for( s32 i=0; i < m_xaTreeStruct.GetCount(); i++ )
    {
        tree_structure_info& tree_item = m_xaTreeStruct[i];
        hHandle = m_xaTreeStruct.GetHandleByIndex( i );

        RecursePath(hHandle, CString(tree_item.cPath), CString(tree_item.cWildcard), TVI_ROOT);
    }
}

//=========================================================================

void CAudioDirectoryView::RecursePath(xhandle hData, CString strPath, CString strWildcard, HTREEITEM hRoot)
{
    CFileSearch::FormatPath(strPath);
    CString strName;
    
    if (m_bUsePrev && (hRoot == TVI_ROOT)) //only for root directories
    {
        strName = strPath.Left(strPath.ReverseFind('\\'));
        strName = strName.Right(strName.GetLength() - strName.ReverseFind('\\') - 1);
    }
    else
    {
        strName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
    }
    
    HTREEITEM hItem         = NULL;
    HTREEITEM hFirstItem    = NULL;
    if( hRoot == TVI_ROOT )
        hFirstItem          = GetTreeCtrl().GetNextItem( hRoot, TVGN_ROOT );
    else
        hFirstItem          = GetTreeCtrl().GetChildItem( hRoot );

    while( hFirstItem )
    {
        CString String = GetTreeCtrl().GetItemText( hFirstItem );
        if( String == strName )
        {
            hItem = hFirstItem;
            break;
        }
        hFirstItem =  GetTreeCtrl().GetNextItem( hFirstItem, TVGN_NEXT );
    }

    if( !hItem )
    {
        HTREEITEM hNewItem = GetTreeCtrl().InsertItem( strName, 0, 1, hRoot);
        GetTreeCtrl().SetItemData(hNewItem, hData);
        hItem = hNewItem;
    }

    CFileSearch fSearch;
    fSearch.GetDirs(strPath);
    CStringList &lstDirs = fSearch.Dirs();
    CString strNextDir = fSearch.GetNextDir(TRUE);
    while (!strNextDir.IsEmpty())
    {   
        //add subdirs
        CString strNewPath = strPath + "\\" + strNextDir;
        RecursePath(hData, strNewPath, strWildcard, hItem);
        strNextDir = fSearch.GetNextDir(TRUE);
    }

    //add files
    fSearch.ClearDirs();
    fSearch.AddDirPath(strPath);
    fSearch.GetFiles(strWildcard);
    CString strNextFile = fSearch.GetNextFile(TRUE);
    while (!strNextFile.IsEmpty())
    {   
        CString strFileName = strNextFile.Right(strNextFile.GetLength() - strNextFile.ReverseFind('\\') - 1);
        HTREEITEM hFileItem = GetTreeCtrl().InsertItem( strFileName, 2, 3, hItem);
        GetTreeCtrl().SetItemData(hFileItem, hData);

        strNextFile = fSearch.GetNextFile(TRUE);
    }
    
    GetTreeCtrl().SortChildren( hItem );
}

//=========================================================================

CString CAudioDirectoryView::ItemToPath(HTREEITEM hItem)
{
    CString strPath;

    if (hItem)
    {
        //Create the full string of the tree item
        HTREEITEM hParent = hItem;
        while (hParent)
        {
            CString strItem = GetTreeCtrl().GetItemText(hParent);
            int nLength = strItem.GetLength();
            ASSERT(nLength);
            hParent = GetTreeCtrl().GetParentItem(hParent);

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
        }

        //Add the root folder if there is one
        xhandle hData = (xhandle)GetTreeCtrl().GetItemData(hItem);
        if (hData != HNULL)
        {
            s32 index = m_xaTreeStruct.GetIndexByHandle(hData);
            ASSERT(index<m_xaTreeStruct.GetCount());
            tree_structure_info& tree_item = m_xaTreeStruct[index];

            strPath = CString(tree_item.cPath) + _T('\\') + strPath;
        }

        CFileSearch::FormatPath(strPath);
    }

    return strPath;
}

//=========================================================================

BOOL CAudioDirectoryView::IsFolder(CString strPath)
{
    CFileStatus rStatus;
    if( CFile::GetStatus( strPath, rStatus ) )
    {
        if (!(rStatus.m_attribute & CFile::directory))
        {
            return FALSE;
        }
    }

    return TRUE;
}

//=========================================================================

void CAudioDirectoryView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	GetDocument()->SetRecentAudioSampleName( "" );
    if( nFlags & MK_SHIFT )
	{
        // Are there any items selected by control.
        if( m_hItemSelList.GetCount() > 0 )
        {
            // Use the head of the list as the first item selected.
            m_hItemFirstSel = m_hItemSelList[ 0 ];
        }

		// Shift key is down
		UINT flag;
		HTREEITEM hItem = GetTreeCtrl().HitTest( point, &flag );

		// Initialize the reference item if this is the first shift selection
		if( !m_hItemFirstSel )
			m_hItemFirstSel = GetTreeCtrl().GetSelectedItem();

		// Select new item
		if( GetTreeCtrl().GetSelectedItem() == hItem )
			GetTreeCtrl().SelectItem( NULL );			// to prevent edit
		//CTreeCtrl::OnLButtonDown(nFlags, point);
        CXTTreeView::OnLButtonDown(nFlags, point);

		if( m_hItemFirstSel )
		{

            ClearSelection();
            m_hItemSelList.Clear();
            
            AddItemsToList( m_hItemFirstSel, hItem, m_hItemSelList );

            SelectControlItem( m_hItemSelList );

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
        if( m_hItemSelList.GetCount() == 0 )
        {
            m_hItemSelList.Append( hSelItem );
        }

        for( s32 i = 0; i < m_hItemSelList.GetCount(); i++ )
        {
            if( m_hItemSelList[i] == hItem )
            {
                CXTTreeView::OnLButtonDown(nFlags, point);
                ClearSelection();
                m_hItemSelList.Delete( i );
                
                //GetTreeCtrl().SelectItem( NULL );

                SelectControlItem( m_hItemSelList );
                return;

            }
        }

        m_hItemSelList.Append( hItem );
        CXTTreeView::OnLButtonDown(nFlags, point);

        SelectControlItem( m_hItemSelList );

        return;

	}
	else
	{
		// Normal - remove all selection and let default 
		// handler do the rest
		ClearSelection();
        m_hItemSelList.Clear();
        UINT flag;
        m_hItemSelList.Append( GetTreeCtrl().HitTest( point, &flag ) );
		m_hItemFirstSel = NULL;
	}

	CXTTreeView::OnLButtonDown(nFlags, point);

    HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
    if( !hItem )
        return;

    // Get the path to the sample.
    CString strPath = ItemToPath( hItem );
    
    // Make sure what ever we clicked on has a aif extension.
    int iIndex = strPath.ReverseFind('.');
    CString strExtention = strPath.Right(strPath.GetLength()-iIndex-1);
    if ( strExtention.CompareNoCase("aif")==0 )
        GetDocument()->SetRecentAudioSampleName( strPath );

    GetTreeCtrl().SelectItem( NULL );
    GetTreeCtrl().SetItemState( hItem ,  TVIS_SELECTED , TVIS_SELECTED );
}

//=========================================================================

void CAudioDirectoryView::ClearSelection( void )
{
	// This can be time consuming for very large trees 
	// and is called every time the user does a normal selection
	// If performance is an issue, it may be better to maintain 
	// a list of selected items
//	for ( HTREEITEM hItem= GetTreeCtrl().GetRootItem(); hItem != NULL; hItem = GetNextItem( hItem ) )
//		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
//			GetTreeCtrl().SetItemState( hItem, 0, TVIS_SELECTED );

    for( s32 i = 0; i < m_hItemSelList.GetCount(); i++ )
            GetTreeCtrl().SetItemState( m_hItemSelList[i], 0, TVIS_SELECTED );
}

//=========================================================================

xbool CAudioDirectoryView::AddItemsToList(HTREEITEM hItemFrom, HTREEITEM hItemTo, xarray<HTREEITEM>& ItemList)
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
		hItemTo = hItemFrom;
		hItemFrom = hItem;
	}


	// Go through remaining visible items
	BOOL bSelect = TRUE;
	while ( hItem )
	{
		// Select or remove selection depending on whether item
		// is still within the range.
		//GetTreeCtrl().SetItemState( hItem, bSelect ? TVIS_SELECTED : 0, TVIS_SELECTED );
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

HTREEITEM CAudioDirectoryView::GetFirstSelectedItem()
{
	for ( HTREEITEM hItem = GetTreeCtrl().GetRootItem(); hItem != NULL; hItem = GetNextItem( hItem ) )
		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CAudioDirectoryView::GetNextSelectedItem( HTREEITEM hItem )
{
	for ( hItem = GetNextItem( hItem ); hItem != NULL; hItem = GetNextItem( hItem ) )
		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CAudioDirectoryView::GetPrevSelectedItem( HTREEITEM hItem )
{
	for ( hItem = GetPrevItem( hItem ); hItem!=NULL; hItem = GetPrevItem( hItem ) )
		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CAudioDirectoryView::GetNextItem( HTREEITEM hItem )
{
        HTREEITEM       hti;

        if( GetTreeCtrl().ItemHasChildren( hItem ) )
                return GetTreeCtrl().GetChildItem( hItem );           // return first child
        else{
                // return next sibling item
                // Go up the tree to find a parent's sibling if needed.
                while( (hti = GetTreeCtrl().GetNextSiblingItem( hItem )) == NULL ){
                        if( (hItem = GetTreeCtrl().GetParentItem( hItem ) ) == NULL )
                                return NULL;
                }
        }
        return hti;
}

//=========================================================================

void CAudioDirectoryView::SelectControlItem ( xarray<HTREEITEM>& ItemList )
{
    x_DebugMsg( "%d Items in the list\n", ItemList.GetCount() );

	HTREEITEM hItem = GetTreeCtrl().GetRootItem();
    s32 i = 0;

    ClearSelection();

    for( i = 0; i < ItemList.GetCount(); i++ )
        GetTreeCtrl().SetItemState( ItemList[i] ,  TVIS_SELECTED , TVIS_SELECTED );
}

//=========================================================================
