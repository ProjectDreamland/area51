//=========================================================================
// CAUDIOEDITORVIEW.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdafx.h"
#include "AudioEditorView.h"
#include "SoundDoc.h"
#include "Resource.h"
#include "FaderDialog.h"

#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\WinControls\FileSearch.h"
#include "..\Editor\Project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



IMPLEMENT_DYNCREATE(CAudioEditorView, CView)

//=========================================================================
// MESSAGE PUMP
//=========================================================================

BEGIN_MESSAGE_MAP(CAudioEditorView, CView)
	//{{AFX_MSG_MAP(CAudioEditorView)
	ON_WM_CREATE        ()
	ON_WM_PAINT         ()
    ON_WM_KILLFOCUS     ()
	ON_WM_ERASEBKGND    ()
	ON_WM_RBUTTONDOWN   ()
    ON_WM_RBUTTONUP     ()
	ON_WM_LBUTTONDOWN   ()
	ON_WM_NCHITTEST     ()
	ON_WM_MOUSEMOVE     ()
	ON_WM_LBUTTONUP     ()
    ON_NOTIFY_REFLECT   ( TVN_BEGINLABELEDIT,               OnBeginlabeledit            )
    ON_NOTIFY_REFLECT   ( TVN_ENDLABELEDIT,                 OnEndlabeledit              )
    ON_NOTIFY_REFLECT   ( TVN_BEGINDRAG,                    OnBeginDrag                 )
    ON_COMMAND          ( IDM_POPUP_MENU_NEW_DIR,           OnNewDir                    )
    ON_COMMAND          ( IDM_POPUP_MENU_NEW_ROOT_DIR,      OnNewRootDir                )
    ON_COMMAND          ( IDM_POPUP_MENU_DELETE,            OnDelDir                    )
    ON_COMMAND          ( IDM_POPUP_MENU_NEW_PACKAGE,       OnNewAudioPackageEvent      )
    ON_COMMAND          ( IDM_POPUP_MENU_NEW_DESCRIPTOR,    OnNewAudioDescriptorEvent   )
    ON_COMMAND          ( ID_EDIT_COPY,                     OnCopyEvent                 )
    ON_COMMAND          ( ID_EDIT_PASTE,                    OnPaste                     )
    ON_COMMAND          ( IDM_POPUP_MENU_RENAME,            OnFileRename                )
    ON_COMMAND          ( IDM_POPUP_MENU_MOVE_TO,           OnMoveTo                    )
    ON_COMMAND          ( IDM_POPUP_MENU_MOVE_PASTE,        OnMovePaste                 )	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioEditorView drawing


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

// Sort the item in alphabetical order.
static int CALLBACK 
MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   // lParamSort contains a pointer to the tree control.
   // The lParam of an item is just its handle.
   CTreeCtrl* pmyTreeCtrl = (CTreeCtrl*) lParamSort;
   CString    strItem1 = pmyTreeCtrl->GetItemText((HTREEITEM) lParam1);
   CString    strItem2 = pmyTreeCtrl->GetItemText((HTREEITEM) lParam2);

   return strcmp(strItem1, strItem2);
}

// This is the only way we can update the tree item since the window is not in focus.
static
void UpdateLabelCallBack( void* pObject, const char* pLabel )
{
    ((CAudioEditorView*)pObject)->UpdateTreeItemLabel( pLabel );
}


#ifdef _DEBUG


//=========================================================================

void CAudioEditorView::AssertValid() const
{
	CView::AssertValid();
}

//=========================================================================

void CAudioEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

CAudioEditorView::CAudioEditorView()
{
    m_CurrentItemHit    = NULL;
    m_PrevItemHit       = NULL;
    m_pDragImage        = NULL;
    m_RightMouseMenu    = FALSE;
    m_bLDragging        = FALSE;
    m_CheckMultiSelectRelease   = FALSE;
    m_EditLock          = 0;
    m_hItemSelList.SetGrowAmount( 10 );
    SetDefaultCursor();
}

//=========================================================================

CAudioEditorView::~CAudioEditorView()
{
}

//=========================================================================

CSoundDoc* CAudioEditorView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSoundDoc)));
	return (CSoundDoc*)m_pDocument;
}

//=========================================================================

int CAudioEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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
    OnLoadProject();
    
	return 0;
}

//=========================================================================

void CAudioEditorView::OnLoadProject ( void )
{
    // We want to have the Fader List loaded before we load the packages.
    // Get the first resouce dir and go up a level to load the gobal fader list.
    char cFirstThemePath[MAX_PATH];
    g_Project.GetFirstResourceDir( cFirstThemePath );
    CString FilePath = cFirstThemePath;

    s32 ResourceIndex = FilePath.Find( "Resource" );
    FilePath.Delete( ResourceIndex, FilePath.GetLength() - ResourceIndex );
    FilePath += "FaderList.txt";

	if( CFileSearch::DoesFileExist( FilePath ) )
    {
        CFile File( (LPCTSTR)FilePath, CFile::modeRead );
        char tempFaderList[512];
	    s32 FileLen = (s32)File.GetLength();
        File.Read( tempFaderList, FileLen );
    
        // This will force the string to know how many character there are since each fader name is followed by a 0.
        g_String.Empty();
        while( FileLen )
        {
            FileLen--;
            g_String.Insert( 0, tempFaderList[FileLen] );
        }
        g_pFaderList = g_String.GetBuffer( g_String.GetLength() );
    }

    CFileSearch FileSearch;
    FileSearch.Recurse( FALSE );
    CSoundDoc* pDoc = GetDocument();
    
    // Go through all the themes.
    char cThemePath[MAX_PATH];
    for( int j = g_Project.GetFirstResourceDir( cThemePath ); j != -1; j = g_Project.GetNextResourceDir( j, cThemePath ) )
    {
        // Does the directory even exist.
        FileSearch.AddDirPath( cThemePath );
            
        // Are their any audio packages in here.
        if( !(FileSearch.GetFiles( "*.audiopkg" )) )
            continue;
        
        FileSearch.SetFilesIndexToHead();

		// Get all the audio package in the resource directory and load them in the tree ctrl
		// view.        
        CString  FullFileName = FileSearch.GetNextFile();
        while( FullFileName.GetLength() > 0 )
        {
            // Get the package handle.
			pDoc->m_AudioEditor.Load( FullFileName );
            xhandle Package = pDoc->m_AudioEditor.m_AddedPackage;

			OnLoadVirtualDir( pDoc->m_AudioEditor.m_pDesc( Package ).m_pVirtualDirectory, Package );
            FullFileName = FileSearch.GetNextFile();
        }
        FileSearch.ClearFiles();
        FileSearch.ClearDirs();
    }

    for( s32 i = 0; i < pDoc->m_AudioEditor.m_pDesc.GetCount(); i++ )
    {
        pDoc->m_AudioEditor.m_pDesc[i].SetChanged( FALSE );
    }
}

//=========================================================================

void CAudioEditorView::OnPaint() 
{
	// Helps to reduce screen flicker.
	CPaintDC dc(this);
	CRect rectClient;
	GetClientRect(&rectClient);
	CXTMemDC memDC(&dc, rectClient, xtAfxData.clrWindow);
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );

}

//=========================================================================

BOOL CAudioEditorView::OnEraseBkgnd(CDC* pDC) 
{
	// Helps to reduce screen flicker.
	UNUSED_ALWAYS(pDC);
	return TRUE;
}

//=========================================================================

void CAudioEditorView::OnOpen ( void )
{
    // We should clear everything form the tree view before we call to the load the project.
    GetTreeCtrl().DeleteAllItems();
    OnLoadProject();
}

//=========================================================================

void CAudioEditorView::OnClose ( void )
{
    GetTreeCtrl().DeleteAllItems();
    GetDocument()->m_AudioEditor.DeleteAll();
}

//=========================================================================

void CAudioEditorView::OnNewRootDir( void )
{
    HTREEITEM htItem = TVI_ROOT;
    
    HTREEITEM AddedItem = GetTreeCtrl().InsertItem(_T("New Root Dir"), 0, 1, htItem);
    GetTreeCtrl().Expand(htItem, TVE_EXPAND);

    // The type.
    DWORD Data = 0;
    Data |= FOLDER;
    
    // Put the full pathname as the data of the item we just inserted.
    GetTreeCtrl().SetItemData( AddedItem, Data );

}

//=========================================================================

void CAudioEditorView::OnNewDir( void )
{
    HTREEITEM htItem = m_CurrentItemHit;
    
    if( !htItem )
        htItem = TVI_ROOT;

    HTREEITEM AddedItem = GetTreeCtrl().InsertItem(_T("New Dir"), 0, 1, htItem);
    GetTreeCtrl().Expand(htItem, TVE_EXPAND);

    // The type.
    DWORD Data = 0;
    Data |= FOLDER;
    
    // Put the full pathname as the data of the item we just inserted.
    GetTreeCtrl().SetItemData( AddedItem, Data );

}

//=========================================================================

void CAudioEditorView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
    (void)pResult;
    TV_DISPINFO* pInfo = (TV_DISPINFO*)pNMHDR;

    // Unlock the edit mode.
    m_EditLock         = 0;
    
	// Set the new label text.
    HTREEITEM hItem = m_CurrentItemHit;
    DWORD     Data  = GetTreeCtrl().GetItemData( hItem);

    if( ( Data & DESCRIPTOR ) && ( pInfo->item.pszText ) )
    {
        // Set the descriptors label the name.
        xhandle PackageSelected = GetDocument()->m_AudioEditor.m_PackageSelected;
		s32 DescIndex = GetDocument()->m_AudioEditor.m_pDesc( PackageSelected ).m_DescriptorSelected;
		
		editor_descriptor& rDesc = GetDocument()->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ];
        x_strncpy( rDesc.m_Label, pInfo->item.pszText, 128 );

        GetTreeCtrl().SetItemText(hItem, pInfo->item.pszText);
    }
    else if( (Data & FOLDER) && ( pInfo->item.pszText ) )
    {
        GetTreeCtrl().SetItemText( hItem, pInfo->item.pszText);
        HTREEITEM hChildItem = GetTreeCtrl().GetNextItem( hItem, TVGN_CHILD );
        HTREEITEM hMainItem = hItem;
        HTREEITEM htmpItem = NULL;

        // Select the item.
        GetTreeCtrl().SelectItem( hMainItem );
    
        // Reset the current item.
        m_hItemSelList.Append( GetTreeCtrl().GetSelectedItem() );
        m_CurrentItemHit = hMainItem;

        // If we change the name of the folder then we have to update the virtual directory
		// string for all the package underneath the folder.
		while ( hChildItem )
        {
            DWORD ItemData  = 0;
            
            if( hChildItem  )
            {
                ItemData = GetTreeCtrl().GetItemData( hChildItem);
            
                if( ItemData & PACKAGE )
                {
                    xhandle PackageHandle = (ItemData & ~PACKAGE);
                    
                    
                    // Save the virtual directory path.
                    CString BackwardVirtualDir;
                    xstring VirDir;
                    HTREEITEM hParentItem =  GetTreeCtrl().GetNextItem( hChildItem, TVGN_PARENT );
                    while( hParentItem )
                    {
                        BackwardVirtualDir = GetTreeCtrl().GetItemText( hParentItem );
                        VirDir.Insert( 0, '\\' );
                        VirDir.Insert( 0, (LPCTSTR)BackwardVirtualDir );
                        hParentItem = GetTreeCtrl().GetNextItem( hParentItem, TVGN_PARENT );
                    }

                    // Save the path of the virtual dir again.
                    x_strncpy( GetDocument()->m_AudioEditor.m_pDesc( PackageHandle ).m_pVirtualDirectory, VirDir, 128);
                    GetDocument()->m_AudioEditor.m_pDesc( PackageHandle ).SetChanged( TRUE );

                    // We don't have to go any deeper so go the the paren't next item.
                    htmpItem = GetTreeCtrl().GetNextItem( hChildItem, TVGN_NEXT );
                }
                else
                {
                    htmpItem = GetTreeCtrl().GetNextItem( hChildItem, TVGN_CHILD );
                }
            }
            
            if( htmpItem == NULL )
            {
                // We can't go any deeper so go the the paren't next item.
                hChildItem = GetTreeCtrl().GetNextItem( hChildItem, TVGN_PARENT );

                if( hMainItem == hChildItem )
                    break;

                hChildItem = GetTreeCtrl().GetNextItem( hChildItem, TVGN_NEXT );

            }
            else
            {
                hChildItem = htmpItem;
            }
        }
    }
}
    

//=========================================================================

void CAudioEditorView::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
    (void)pNMHDR;
    (void)pResult;
    
    // Lock the edit mode.
    m_EditLock = 1;
}

//=========================================================================

void CAudioEditorView::OnDelDir ( void )
{    
    xarray< tree_item > DelList;
    xarray< tree_item > VirtualDelList;
    s32 i,j             = 0;
    s32 HighestItem     = 10000;
    HTREEITEM PrevItem  = NULL;

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
    
    // If we have a folder selected to delete we want to get all of its children.
    for( i = 0; i < DelList.GetCount(); i++ )
    {
        DWORD       Data  = DelList[i].m_Data;
        HTREEITEM   hItem = DelList[i].m_hItem;
        
        if( Data & FOLDER )
        {
            HTREEITEM hChild = TreeCtrl.GetChildItem( hItem );
            
            if( hChild )
                GetSubFolders( hChild, VirtualDelList );
        }
    }
            
    // Make sure that the same item doesn't get deleted twice.
    for( i = 0; i < DelList.GetCount(); i++ )
    {
        for( j = 0; j < VirtualDelList.GetCount(); j++ )
        {
            if( VirtualDelList[j].m_hItem == DelList[i].m_hItem )
                VirtualDelList.Delete( j );
        }
    }
    
    DelList += VirtualDelList;

    x_qsort( &DelList[0], DelList.GetCount(), sizeof(tree_item), DelCompare );
    
    // Since the element has highest bit in the bit field we want to start from the bottom of the list
    // and then work out way up.
    for( i = DelList.GetCount()-1; i >= 0; i-- )
    {

        DWORD       Data  = DelList[i].m_Data;
        HTREEITEM   hItem = DelList[i].m_hItem;
        
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

        CSoundDoc* Doc =  GetDocument();

        // Clear all the sample view.
        if (Doc->m_pCommandHandler)
            Doc->m_pCommandHandler->SendMessage(WM_USER_MSG_CLEAR_ALL_SOUND_FILE, NULL);

        if( Data & FOLDER )
        {
            TreeCtrl.DeleteItem( hItem );
        }
        else if( Data & PACKAGE )
        {
            s32 PackageSelected = (Data & ~PACKAGE);

            // Delete the package.
            Doc->m_AudioEditor.Delete( PackageSelected );

            Doc->m_pPropEditor->Refresh();
            TreeCtrl.DeleteItem( hItem );
        }
        else if( Data & DESCRIPTOR )
        {
            // Get the parent descriptors index.
            HTREEITEM Parent = TreeCtrl.GetParentItem( hItem );
            DWORD ParentData = TreeCtrl.GetItemData( Parent );
            xhandle PackageSelected = (ParentData & ~PACKAGE);
            
            Doc->m_AudioEditor.m_PackageSelected = PackageSelected;

            // Set the index of the descriptor we just deleted.
            s32 Index = (Data & ~DESCRIPTOR);
            if( !(Doc->m_AudioEditor.DeleteDescriptor( Index )) )
                continue;
        
            Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_DescriptorSelected = -1;
            TreeCtrl.DeleteItem( hItem );
            
            
            // Go through all the items next of the descriptor and change there index.
            HTREEITEM FirstChildItem    = TreeCtrl.GetChildItem( Parent );
            HTREEITEM NextItem          = FirstChildItem;
            
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

            Doc->m_pPropEditor->Refresh();
        }
        else if( Data & ELEMENT )
        {
            // Set the index of the element we just deleted.
            s32 Index = (Data & ~ELEMENT);

            // Get the parent descriptors index.
            HTREEITEM Parent = TreeCtrl.GetParentItem( hItem );
            DWORD ParentData = TreeCtrl.GetItemData( Parent );
            s32 DescIndex = (ParentData & ~DESCRIPTOR);
        
            // Get the package index.
            HTREEITEM hDescParent = TreeCtrl.GetNextItem( Parent, TVGN_PARENT );
            DWORD DescParentData = TreeCtrl.GetItemData( hDescParent );
            xhandle PackageSelected = (DescParentData & ~PACKAGE);
            
            Doc->m_AudioEditor.m_PackageSelected = PackageSelected;
            Doc->m_AudioEditor.DeleteElement( DescIndex, Index );
            Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_DescriptorSelected = -1;       
            Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_ElementSelected = -1;       

            TreeCtrl.DeleteItem( hItem );
            
            // Go through all the items next of the element and change there index.
            HTREEITEM FirstChildItem    = TreeCtrl.GetChildItem( Parent );
            HTREEITEM NextItem          = FirstChildItem;
            while( NextItem )
            {
                DWORD ElementData = TreeCtrl.GetItemData( NextItem );
                DWORD NextItemData = ElementData;
                
                if( Index <= (s32)(ElementData & ~ELEMENT) )
                {
                    NextItemData = (ElementData & ~ELEMENT) - 1;
                    NextItemData |= ELEMENT;
                }

                //DWORD NextItemData = Index;
                //NextItemData |= ELEMENT;

                TreeCtrl.SetItemData( NextItem, NextItemData);

                NextItem = TreeCtrl.GetNextItem( NextItem, TVGN_NEXT );
                //Index++;
            }

            Doc->m_pPropEditor->Refresh();
        }
    }

    TreeCtrl.SetRedraw( TRUE );
    m_hItemSelList.Clear();
    
    if( PrevItem )
    {
        TreeCtrl.SelectItem( PrevItem );
    
        // Reset the current item.
        m_hItemSelList.Clear();
        m_hItemSelList.Append( GetTreeCtrl().GetSelectedItem() );
        m_CurrentItemHit = PrevItem;
        
        SetSelectedItemData();
    }
}

//=========================================================================

void CAudioEditorView::GetSubFolders( HTREEITEM hItem, xarray< tree_item >& TreeList )
{
    CTreeCtrl& TreeCtrl = GetTreeCtrl();

    HTREEITEM NextItem      = TreeCtrl.GetNextItem( hItem, TVGN_NEXT );
    HTREEITEM ChildItem     = TreeCtrl.GetChildItem( hItem );

    // Recurse if there is a child item and its a folder or a package.
    if( ChildItem )
    {
        DWORD Data = TreeCtrl.GetItemData( ChildItem );

        if( (Data & FOLDER) || (Data & PACKAGE) )
        {
            GetSubFolders( ChildItem, TreeList );
        }
    }

    // Recurse if there is a sibiling item and its a folder or a package.
    if( NextItem )
    {
        DWORD Data = TreeCtrl.GetItemData( NextItem );

        if( (Data & FOLDER) || (Data & PACKAGE) )
        {
            GetSubFolders( NextItem, TreeList );    
        }
    }

    tree_item TreeItem;
    TreeItem.m_Data     = TreeCtrl.GetItemData( hItem );
    TreeItem.m_hItem    = hItem;

    CString String = TreeCtrl.GetItemText( hItem );
    
    // Does the item already exist in our array.
    for( s32 i = 0; i < TreeList.GetCount(); i++ )
    {
        if( TreeList[i].m_hItem == hItem )
            return;
    }
    
    // Add the item to the deletion list.
    TreeList.Append( TreeItem );    
}

//=========================================================================

void CAudioEditorView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    CXTMenu menu;
    menu.CreatePopupMenu();
    
    if( m_bLDragging )
        return;

    // We don't want to give the user an option to create any new things when they have multiple things selected.
    if( GetDocument()->m_AudioEditor.GetMultiSelect() )
    {
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_DELETE,           "Delete"                );
        menu.AppendMenu( MF_STRING|MF_ENABLED, ID_EDIT_COPY,                    "Copy"                  );
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_MOVE_TO,          "Move Pkg To"           );
    }
    else
    {
        // Menu items.
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_NEW_ROOT_DIR,     "New Root Directory"    );    
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_NEW_DIR,          "New Directory"         );    
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_RENAME,           "Rename"                );
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_DELETE,           "Delete"                );
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_NEW_PACKAGE,      "New Package"           );
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_NEW_DESCRIPTOR,   "New Descriptor"        );
        menu.AppendMenu( MF_STRING|MF_ENABLED, ID_EDIT_COPY,                    "Copy"                  );
        menu.AppendMenu( MF_STRING|MF_ENABLED, ID_EDIT_PASTE,                   "Paste"                 );
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_MOVE_TO,          "Move Pkg To"           );
        menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_MOVE_PASTE,       "Move Pkg Paste"        );
    }
        
    m_RightMouseMenu = TRUE;

    ClientToScreen(&point);
    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);
	
	CXTTreeView::OnRButtonDown(nFlags, point);
}

//=========================================================================

void CAudioEditorView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// We don't want the focus changing if the right mouse button was hit.
    if( m_RightMouseMenu )
    {
        m_RightMouseMenu = FALSE;
        GetTreeCtrl().SelectItem( NULL );
        GetTreeCtrl().SetItemState( m_CurrentItemHit ,  TVIS_SELECTED , TVIS_SELECTED );
    }

    CXTTreeView::OnRButtonUp(nFlags, point);
}

//=========================================================================

void CAudioEditorView::OnFileRename ( void )
{
    GetTreeCtrl().EditLabel( m_CurrentItemHit );
}

//=========================================================================

void CAudioEditorView::OnNewAudioPackageEvent( void )
{
    OnNewAudioPackage();
}

s32 CAudioEditorView::OnNewAudioPackage      ( void )
{
    x_try;

    HTREEITEM hItem = m_CurrentItemHit;
    if( !hItem )
        x_throw( "Please select a root directory in which to create a new package" );

    // Check how the parent is.
    DWORD ParentData = GetTreeCtrl().GetItemData( hItem );
    if( ((ParentData & PACKAGE) || (ParentData & DESCRIPTOR) || (ParentData & ELEMENT)) )
        x_throw( "You can't create a package underneath the selected type." );


    HTREEITEM AddedItem = GetTreeCtrl().InsertItem(_T("New Package"), 2, 3, hItem);
    m_NewestAddedPackage = AddedItem;
    GetTreeCtrl().Expand(hItem, TVE_EXPAND);

    CSoundDoc* Doc =  GetDocument();
       
    // Add a new package.
    Doc->m_AudioEditor.NewPackage();

    // The type.
    DWORD Data = Doc->m_AudioEditor.m_AddedPackage;
    Data |= PACKAGE;
    
    // Put the full pathname as the data of the item we just inserted.
    GetTreeCtrl().SetItemData( AddedItem, Data );

    s32 NewPackageIndex = Doc->m_AudioEditor.m_pDesc.GetIndexByHandle( Doc->m_AudioEditor.m_AddedPackage );
    GetTreeCtrl().SetItemText( AddedItem, Doc->m_AudioEditor.m_pDesc[ NewPackageIndex ].GetName() );

    // Save the virtual directory path.
    CString BackwardVirtualDir;
    xstring VirDir;
    while( hItem )
    {
        BackwardVirtualDir = GetTreeCtrl().GetItemText( hItem );
        VirDir.Insert( 0, '\\' );
        VirDir.Insert( 0, (LPCTSTR)BackwardVirtualDir );
        hItem = GetTreeCtrl().GetNextItem( hItem, TVGN_PARENT );
    }

    x_strncpy( Doc->m_AudioEditor.m_pDesc[ NewPackageIndex ].m_pVirtualDirectory, VirDir, 128);

    Doc->m_pPropEditor->Refresh();

    return Doc->m_AudioEditor.m_AddedPackage;

    x_catch_display;

    return -1;
}

//=========================================================================

void CAudioEditorView::OnNewAudioDescriptorEvent( void )
{
    OnNewAudioDescriptor();
}

s32 CAudioEditorView::OnNewAudioDescriptor   ( void )
{
    x_try;

    // Check who the parent is.
    HTREEITEM Parent =m_CurrentItemHit;
    DWORD ParentData = GetTreeCtrl().GetItemData( Parent );
    if( !(ParentData & PACKAGE) )
        x_throw( "You need a select a Package before you create a new Descriptor" );

    HTREEITEM AddedItem = GetTreeCtrl().InsertItem(_T("New Descriptor"), 4, 5, Parent);
    m_NewestAddedDesc = AddedItem;
    GetTreeCtrl().Expand(Parent, TVE_EXPAND);

    CSoundDoc* Doc =  GetDocument();

    Doc->m_AudioEditor.NewDescriptor();

    xhandle PackageSelected = Doc->m_AudioEditor.m_PackageSelected;
    s32     Index           = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList.GetCount() - 1;
    
    // The index and type.
    DWORD Data = Index;
    Data |= DESCRIPTOR;
    
    // Put the full pathname as the data of the item we just inserted.
    GetTreeCtrl().SetItemData( AddedItem, Data );
    
    // Set the items text to the label of the descriptor.
    GetTreeCtrl().SetItemText( AddedItem, Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ Index ].m_Label );

    Doc->m_pPropEditor->Refresh();
    
    return Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList.GetCount() - 1;

    x_catch_display;

    return -1;
}

//=========================================================================

void CAudioEditorView::OnCreateDescFromSample ( CString PathName )
{
    x_try;

    // Check who the parent is.
    HTREEITEM Parent = m_CurrentItemHit;
    DWORD ParentData = GetTreeCtrl().GetItemData( Parent );
    if( !(ParentData & PACKAGE) )
        x_throw( "You need a select a Package before you create a new Descriptor" );

    HTREEITEM AddedItem = GetTreeCtrl().InsertItem(_T("New Descriptor"), 4, 5, Parent);
    GetTreeCtrl().Expand(Parent, TVE_EXPAND);

    HTREEITEM ElementAddedItem = GetTreeCtrl().InsertItem(_T("New Element"), 6, 7, AddedItem );
    GetTreeCtrl().Expand(AddedItem, TVE_EXPAND);

    CSoundDoc* Doc =  GetDocument();

    Doc->m_AudioEditor.NewDescriptor();
    
    // Set the selection index for the package, desc and elements.
    xhandle PackageSelected = Doc->m_AudioEditor.m_PackageSelected;
    s32 DescIndex = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList.GetCount() - 1;
    s32 ElementIndex = Doc->m_AudioEditor.NewElement( DescIndex );
    
    x_strncpy( Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ].m_pSamplePathName, 
                (LPCTSTR)PathName, 128 );
    
    // Truncate the string so only the the sample name shows.
    s32 LastHashMark = PathName.ReverseFind( '\\' );
    LastHashMark++;
    PathName = PathName.Right( PathName.GetLength() - LastHashMark );    
    s32 ExtIndex = PathName.Find( '.' );
    PathName.Delete( ExtIndex, PathName.GetLength() - ExtIndex );

    // The index and type.
    DWORD ElementData = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[DescIndex].m_pElements.GetCount() - 1;
    ElementData |= ELEMENT;
    
    // Set the short sample name, this is going to be used as a label for the samples.
    x_strncpy( Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_Label, 
                (LPCTSTR)PathName, 128 );

    PathName = "f_" + PathName;
    x_strncpy( Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ].m_pSampleName, 
                (LPCTSTR)PathName, 128 );
    
    // Put the full pathname as the data of the item we just inserted.
    GetTreeCtrl().SetItemData( ElementAddedItem, ElementData );
    GetTreeCtrl().SetItemText( ElementAddedItem, (LPCTSTR)PathName );

    // The index and type.
    DWORD Data = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList.GetCount() - 1;
    Data |= DESCRIPTOR;
    
    // Put the full pathname as the data of the item we just inserted.
    GetTreeCtrl().SetItemData( AddedItem, Data );
    
    // Set the items text to the label of the descriptor.
    GetTreeCtrl().SetItemText( AddedItem, Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList.GetCount() - 1 ].m_Label );

    Doc->m_pPropEditor->Refresh();

    x_catch_display;
}

//=========================================================================

s32 CAudioEditorView::CreateElement( HTREEITEM Parent, HTREEITEM& rAddedItem )
{
    DWORD ParentData = GetTreeCtrl().GetItemData( Parent );
    if( !(ParentData & DESCRIPTOR) )
        x_throw( "You need a select a Descriptor before you create a new Element" );

    rAddedItem = GetTreeCtrl().InsertItem(_T("New Element"), 6, 7, Parent);
	m_NewestAddedElement = rAddedItem;
    GetTreeCtrl().Expand(Parent, TVE_EXPAND);

    CSoundDoc* Doc =  GetDocument();
    
    // Set the selection index for the package, desc and elements.
    xhandle PackageSelected = Doc->m_AudioEditor.m_PackageSelected;
    s32 DescIndex = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_DescriptorSelected;
    
    if( DescIndex == -1 )
        x_throw( "Invalid Descriptor selected" );

    s32 ElementIndex = Doc->m_AudioEditor.NewElement( DescIndex );
    
    // The index and type.
    DWORD Data = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[DescIndex].m_pElements.GetCount() - 1;
    Data |= ELEMENT;

    GetTreeCtrl().SetItemData( rAddedItem, Data );

    return ElementIndex;
}

//=========================================================================

s32 CAudioEditorView::OnNewAudioElement ( CString PathName )
{
    x_try;

    CSoundDoc*  Doc =  GetDocument();
    HTREEITEM   AddedItem;

    s32 ElementIndex = CreateElement( m_CurrentItemHit, AddedItem );

    // Check the parent.
    //HTREEITEM Parent = m_CurrentItemHit;
    // Set the selection index for the package, desc and elements.
    xhandle PackageSelected = Doc->m_AudioEditor.m_PackageSelected;
    s32 DescIndex = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_DescriptorSelected;
    
    x_strncpy( Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ].m_pSamplePathName, 
                (LPCTSTR)PathName, 128 );
    
    // Truncate the string so only the the sample name shows.
    s32 LastHashMark = PathName.ReverseFind( '\\' );
    LastHashMark++;
    PathName = PathName.Right( PathName.GetLength() - LastHashMark );    
    s32 ExtIndex = PathName.Find( '.' );
    PathName.Delete( ExtIndex, PathName.GetLength() - ExtIndex );

    
    PathName = "f_" + PathName;
    // Set the short sample name, this is going to be used as a label for the samples.
    x_strncpy( Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ].m_pSampleName, 
                (LPCTSTR)PathName, 128 );

    // Put the full pathname as the data of the item we just inserted.
    //GetTreeCtrl().SetItemData( AddedItem, Data );
    GetTreeCtrl().SetItemText( AddedItem, (LPCTSTR)PathName );
    
    SetSelectedItemData();
    Doc->m_pPropEditor->Refresh();
    
    return ElementIndex;

    x_catch_display;

    return -1;
}

//=========================================================================

void CAudioEditorView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CSoundDoc*  Doc     =  GetDocument();    
    CTreeCtrl& TreeCtrl = GetTreeCtrl();   
    
    Doc->m_AudioEditor.ClearParamsMode();

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

        //CXTTreeView::OnLButtonDown(nFlags, point);

		if( m_hItemFirstSel )
		{
            ClearSelection();
            m_hItemSelList.Clear();
            
            AddItemsToList( m_hItemFirstSel, hItem, m_hItemSelList );
		}

        CXTTreeView::OnLButtonDown(nFlags, point);

        if( m_hItemFirstSel )
        {
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
        if( m_hItemSelList.GetCount() == 0 )
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

                Doc->m_pPropEditor->Refresh();

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
        if( Doc->m_AudioEditor.GetMultiSelect() )
        {
            m_CheckMultiSelectRelease = TRUE;
            
            // Did we click on the selection.
            RECT    Rect;
            irect   iRect;
            s32     i = 0;
            UINT    Flags;
            HTREEITEM hItemHitTest = TreeCtrl.HitTest( point, &Flags );

            for( i = 0; i < m_hItemSelList.GetCount(); i++ )
            {
                if( hItemHitTest == m_hItemSelList[i] )
                {
                    // Prevent edit.
                    TreeCtrl.SetItemState( m_hItemSelList[i], 0, TVIS_SELECTED );
                    TreeCtrl.SetItemState( m_hItemSelList[i], TVIS_SELECTED, TVIS_SELECTED );
                }
                    
                TreeCtrl.GetItemRect( m_hItemSelList[i], &Rect, 0 );
                iRect.Set( Rect.left, Rect.top, Rect.right, Rect.bottom );
                
                if( iRect.PointInRect( point.x, point.y ) == TRUE )
                {
                    break;
                }
            }
            
            if( i < m_hItemSelList.GetCount() )
            {
                CXTTreeView::OnLButtonDown(nFlags, point);
            }

            return;
        }            

        // Normal - remove all selection and let default 
		// handler do the rest
		ClearSelection();
        m_hItemSelList.Clear();
        
        UINT flags;
        HTREEITEM hSelItem = TreeCtrl.HitTest( point, &flags );//GetTreeCtrl().GetSelectedItem();

        if( hSelItem && ((flags & TVHT_ONITEMLABEL)||(flags&TVHT_ONITEMICON)) )
            m_hItemSelList.Append( hSelItem );
        else
        {
            // Toggle the expanding or contracting of the item if its valid and we clicked on the button region.
            if( hSelItem )
            {
                if( flags & TVHT_ONITEMBUTTON )
                {
                    TreeCtrl.Expand( hSelItem, TVE_TOGGLE );
                }
            }

            Doc->m_pPropEditor->Refresh();
            return;
        }

		m_hItemFirstSel = NULL;
        Doc->m_AudioEditor.SetMultiSelect( FALSE );
	}


    UINT flags;
    HTREEITEM hItem = TreeCtrl.HitTest( point, &flags );//GetTreeCtrl().GetSelectedItem();
    if( !hItem && !((flags & TVHT_ONITEMLABEL)||(flags&TVHT_ONITEMICON)) )
        return;

    // To prevent edit in the tree view.
    if( m_CurrentItemHit == hItem )
    {
        TreeCtrl.SelectItem( NULL );
    }


    TreeCtrl.SetItemState( hItem ,  TVIS_SELECTED , TVIS_SELECTED );
    RECT Rect;
    TreeCtrl.GetItemRect( hItem, &Rect, FALSE );
    InvalidateRect( &Rect );

    // What item was previously hit.
    m_PrevItemHit = m_CurrentItemHit;
    m_CurrentItemHit = hItem;

#ifdef sansari
        if( m_CurrentItemHit )
        {
            CString String = TreeCtrl.GetItemText( m_CurrentItemHit );
            DWORD currentData = TreeCtrl.GetItemData( m_CurrentItemHit );
            x_DebugMsg( "Item hit: %s Data[%d]\n", String.GetBuffer( String.GetLength() ), (u32)currentData );
        }

        if( m_PrevItemHit )
        {
            CString String = TreeCtrl.GetItemText( m_PrevItemHit );
            DWORD prevData = TreeCtrl.GetItemData( m_PrevItemHit );
            x_DebugMsg( "Prev Item hit: %s Data[%d]\n", String.GetBuffer( String.GetLength() ), (u32)prevData );
        }
#endif

    SetSelectedItemData();


    CXTTreeView::OnLButtonDown(nFlags, point);
}

//=========================================================================

UINT CAudioEditorView::OnNcHitTest(CPoint point) 
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

void CAudioEditorView::RefrenceDescriptor( void )
{
    x_try;

    CSoundDoc* Doc =  GetDocument();

    DWORD RefDescData   = GetTreeCtrl().GetItemData( m_PrevItemHit );
    DWORD ElementData   = GetTreeCtrl().GetItemData( m_CurrentItemHit );

#ifdef sansari
        CString String = GetTreeCtrl().GetItemText( m_CurrentItemHit );
        //DWORD currentData = GetTreeCtrl().GetItemData( m_CurrentItemHit );

        x_DebugMsg( "Item hit: %s Data[%d]\n", String.GetBuffer( String.GetLength() ), (u32)ElementData );
        
        String = GetTreeCtrl().GetItemText( m_PrevItemHit );
        //DWORD prevData = GetTreeCtrl().GetItemData( m_PrevItemHit );
        x_DebugMsg( "Prev Item hit: %s Data[%d]\n", String.GetBuffer( String.GetLength() ), (u32)RefDescData );
#endif

    if( ( ElementData & ELEMENT ) && ( RefDescData & DESCRIPTOR ) )
    {
        // Make sure that we don't select the parent.
        HTREEITEM Parent = GetTreeCtrl().GetParentItem( m_CurrentItemHit );

        HTREEITEM ElementPackage    = GetTreeCtrl().GetParentItem( Parent );
        HTREEITEM DescPackage       = GetTreeCtrl().GetParentItem( m_PrevItemHit );

        if( ElementPackage != DescPackage )
            x_throw( "You can only reference descriptors in the same audio package." );

        if( Parent != m_PrevItemHit )
        {
            DWORD ParentData = GetTreeCtrl().GetItemData( Parent );
            s32 ParentDescIndex = ParentData & ~DESCRIPTOR;
            s32 RefDescIndex    = RefDescData & ~DESCRIPTOR;
            s32 ElementIndex    = ElementData & ~ELEMENT;
            
            // Refrence a descriptor with this element.
            Doc->m_AudioEditor.RefrenceDescriptor( ParentDescIndex, ElementIndex, RefDescIndex );

            GetTreeCtrl().SetItemText( m_CurrentItemHit, "RefrenceElement" );
        }
        else
        {
            x_throw( "You can't refrence the parent descriptor of an element." );
        }
    }

    Doc->m_pPropEditor->Refresh();

    x_catch_display;
}

//=========================================================================

void CAudioEditorView::ChangeElementSelection ( void )
{
    // Update the property editor since the selection has changed.
    if( GetDocument()->m_SoundFileCount )
    {
        xhandle PackageSelected = GetDocument()->m_AudioEditor.m_PackageSelected;
        GetDocument()->m_AudioEditor.m_pDesc( PackageSelected ).m_ElementSelected = GetDocument()->m_SampleSelected;
        GetDocument()->m_pPropEditor->Refresh();
    }
}

//=========================================================================

void CAudioEditorView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
    switch( lHint )
    {
        case NEW_AUDIO_PACKAGE:         OnNewAudioPackage();                                            break;
        case NEW_AUDIO_DESCRITPOR:      OnNewAudioDescriptor();                                         break;
        case NEW_AUDIO_ELEMENT:         OnNewAudioElement( GetDocument()->m_ElementPathName );          break;
        case REF_AUDIO_DESCRITPOR:      RefrenceDescriptor();                                           break;
        case ELEMENT_SEL_CHANGE:        ChangeElementSelection();                                       break;
        case CREATE_DESC_FROM_SAMPLE:   OnCreateDescFromSample( GetDocument()->m_ElementPathName );     break;
        case OPEN_AUDIO_PROJECT:        OnOpen();                                                       break;
        case CLOSE_AUDIO_PROJECT:       OnClose();                                                      break;
        case NEW_AUDIO_PROJECT:         OnClose();                                                      break;
        case SORT_DESCENDING:           OnSortDescending();                                             break;
        case AUDIO_EDITOR_COPY:         OnCopy();                                                       break;
        case AUDIO_EDITOR_PASTE:        OnPaste();                                                      break;
        default:                                                                                        break;
    }
}

//=========================================================================

void CAudioEditorView::UpdateTreeItemLabel (  const char* pLabel )
{
    if( m_CurrentItemHit )
    {
        // Check if the the item that we have selected a descriptor and if it label has changed.
        DWORD Data = GetTreeCtrl().GetItemData( m_CurrentItemHit );

        if( Data & PACKAGE )
        {
            // If we are not it a editlock
            if( m_EditLock == 0 )
            {
                // Set the items text to the label of the descriptor.
                GetTreeCtrl().SetItemText( m_CurrentItemHit, pLabel );
            }
        }
        else if( Data & DESCRIPTOR )
        {
            xhandle PackageSelected     = GetDocument()->m_AudioEditor.m_PackageSelected;
            s32 DescIndex               = (Data & ~DESCRIPTOR);
            editor_descriptor& rDesc    = GetDocument()->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ];

            // If we are not it a editlock
            if( m_EditLock == 0 )
            {
                // Set the items text to the label of the descriptor.
                GetTreeCtrl().SetItemText( m_CurrentItemHit, rDesc.m_Label );
            }
        }
    }
}

//=========================================================================

void CAudioEditorView::OnBuildPackage ( void )
{

}

//=========================================================================

void CAudioEditorView::OnReBuildPackage ( void )
{

}

//=========================================================================

void CAudioEditorView::OnLoadVirtualDir ( CString VirtualDir, xhandle PackageHandle )
{
    CSoundDoc* pDoc  = GetDocument();
    // The type.
    DWORD Data = 0;
    Data |= FOLDER;

    // Force the tree item to start from the root.
    HTREEITEM hItem = TVI_ROOT;
    
	xbool Child = TRUE;

    HTREEITEM hCurrentTreeItem = GetTreeCtrl().GetNextItem( TVI_ROOT, TVGN_ROOT );

	while( VirtualDir.GetLength() > 0 )
	{
		s32 DirNameIndex = VirtualDir.Find( '\\', 0 );
		CString DirName = VirtualDir.Left( DirNameIndex );
		
		if( Child )
			VirtualDir.Delete(0, DirNameIndex+1 );
		
		// Does the directory exist.
        if( (hCurrentTreeItem != NULL) )
        {
			CString CurrentDirName = GetTreeCtrl().GetItemText( hCurrentTreeItem );
			
			// Is it the same directory.
			if( DirName != CurrentDirName )
			{
				HTREEITEM hSeriesItem = hCurrentTreeItem;
                xbool AddItem = TRUE;
                xbool ItemFound = FALSE;
				
				// We have to search both above and below the current item we have just decended down to.
                // Going up.
				while ( hSeriesItem )
				{
					HTREEITEM htmpItem = GetTreeCtrl().GetNextItem( hSeriesItem, TVGN_NEXT );
					if( htmpItem )
                    {
					    CurrentDirName = GetTreeCtrl().GetItemText( htmpItem );
					    if( DirName == CurrentDirName )
					    {
						    
						    hCurrentTreeItem = htmpItem;
						    hItem            = hCurrentTreeItem;
						    hCurrentTreeItem = GetTreeCtrl().GetNextItem( hItem, TVGN_CHILD );

						    AddItem     = FALSE;
                            ItemFound   = TRUE;
                            Child       = TRUE;
						    break;
					    }
						
                        hSeriesItem = htmpItem;
                    }
					else
                    {
						break;
                    }
				}
            
				if( ItemFound == FALSE )
				{
					// Going down.
                    hSeriesItem = hCurrentTreeItem;
					while( hSeriesItem )
					{
						HTREEITEM htmpItem = GetTreeCtrl().GetNextItem( hSeriesItem, TVGN_PREVIOUS );
						if( htmpItem )
                        {
					        CurrentDirName = GetTreeCtrl().GetItemText( htmpItem );
					        if( DirName == CurrentDirName )
					        {
						        
						        hCurrentTreeItem = htmpItem;
						        hItem            = hCurrentTreeItem;
						        hCurrentTreeItem = GetTreeCtrl().GetNextItem( hItem, TVGN_CHILD );
						        AddItem = FALSE;
                                Child   = TRUE;
						        break;
					        }

							hSeriesItem = htmpItem;
                        }
						else
                        {
							break;
                        }
					}

				}
				if( AddItem )
				{
					Child = TRUE;
					hItem = GetTreeCtrl().InsertItem(DirName, 0, 1, hItem);
					GetTreeCtrl().SetItemData( hItem, Data);
					GetTreeCtrl().SetItemText( hItem, (LPCTSTR)DirName);
					hCurrentTreeItem = NULL;
				}

			}
			else
			{				
				
				Child = TRUE;
				hItem = hCurrentTreeItem;
				hCurrentTreeItem = GetTreeCtrl().GetNextItem( hItem, TVGN_CHILD );
			}
		}
        else
        {
			// Add the item.
			Child = TRUE;
			hItem = GetTreeCtrl().InsertItem(DirName, 0, 1, hItem);
			GetTreeCtrl().SetItemData( hItem, Data);	
            GetTreeCtrl().SetItemText( hItem, (LPCTSTR)DirName);
        }
	}

    OnLoadAudioPackage( PackageHandle, hItem );
}

//=========================================================================

void CAudioEditorView::OnLoadAudioPackage( xhandle PackageHandle, HTREEITEM ParentItem )
{
    // Insert the package.
    char PackagePathName[128];
    CSoundDoc* pDoc     = GetDocument();
    DWORD Data          = 0;
    s32 PackageIndex    = pDoc->m_AudioEditor.m_pDesc.GetIndexByHandle( PackageHandle );

    pDoc->m_AudioEditor.m_pDesc[ PackageIndex ].GetFullName( PackagePathName );
    CString PackageName = PackagePathName;
    
    // Find the closest back slash.
    s32 Delcount = PackageName.ReverseFind( '\\' );
   
    // Include the slash.
    Delcount++;
    
    // Just get the file name.
    PackageName = PackageName.Right( PackageName.GetLength() - Delcount );
    Data = PackageHandle;
    
    // Set the packages data.
    Data |= PACKAGE;
    HTREEITEM hItem = GetTreeCtrl().InsertItem((LPCTSTR)PackageName, 2, 3, ParentItem);
    GetTreeCtrl().SetItemData( hItem, Data);
    
    // Put the element and the descriptor in the directory.
    for( s32 i = 0; i < pDoc->m_AudioEditor.m_pDesc[ PackageIndex ].m_pDescriptorList.GetCount(); i++ )
    {
        Data = i;
        Data |= DESCRIPTOR;
        HTREEITEM Desc = GetTreeCtrl().InsertItem(pDoc->m_AudioEditor.m_pDesc[ PackageIndex ].m_pDescriptorList[i].m_Label, 4, 5, hItem);
        GetTreeCtrl().SetItemData( Desc, Data);
        
        for( s32 j = 0; j < pDoc->m_AudioEditor.m_pDesc[ PackageIndex ].m_pDescriptorList[i].m_pElements.GetCount(); j++ )
        {
            Data = j;
            Data |= ELEMENT;

            CString PathName = pDoc->m_AudioEditor.m_pDesc[ PackageIndex ].m_pDescriptorList[i].m_pElements[ j].m_pSampleName;
            
            // Append the "f_" if the elements don't already have it.
            // We can't use Find here because some of the footfalls have "f_" in the middle of the sample name.
            //if( (PathName[0] != 'f') && (PathName[1] != '_') )
                //PathName = "f_" + PathName;

            // Then this is a refrence.
            if( PathName.GetLength() == 0 )
                PathName = "RefrenceElement";

            HTREEITEM Element = GetTreeCtrl().InsertItem((LPCTSTR)PathName, 6, 7, Desc);
            GetTreeCtrl().SetItemData( Element, Data);
        }
    }

}

//=========================================================================

void CAudioEditorView::OnSortDescending( void )
{
    CTreeCtrl& TreeCtrl     = GetTreeCtrl();
    HTREEITEM  hRootItem    = TreeCtrl.GetRootItem();
    HTREEITEM  hItem        = hRootItem;

    TreeCtrl.SortChildren( hItem );

    while( hItem )
    {
        // Sort the current item and its children.
        TreeCtrl.SortChildren( hItem );
        HTREEITEM hChildItem = TreeCtrl.GetNextItem( hItem, TVGN_CHILD );
        
        if( hChildItem )
        {
            TreeCtrl.SortChildren( hChildItem );
            DWORD Data   = TreeCtrl.GetItemData( hChildItem );    
            CString Text = TreeCtrl.GetItemText( hChildItem );

            // If its a descriptor we know that its children doesn't have any children so we can just sort at this level.
            if( Data & DESCRIPTOR )
            {
                HTREEITEM hNextItem = TreeCtrl.GetNextItem( hChildItem, TVGN_NEXT );

                // Get all the next descriptors
                while( hNextItem )
                {
                    TreeCtrl.SortChildren( hNextItem );
                    hNextItem = TreeCtrl.GetNextItem( hNextItem, TVGN_NEXT );
                }
                
                // Keep going back till we find the next sibling.
                HTREEITEM hParent = TreeCtrl.GetNextItem( hChildItem, TVGN_PARENT );
                while( hParent )
                {
                    HTREEITEM hNextItem = TreeCtrl.GetNextItem( hParent, TVGN_NEXT );
                    if( hNextItem )
                    {
                        hParent = hNextItem;
                        break;
                    }
                    else
                    {
                        hParent      = TreeCtrl.GetNextItem( hParent, TVGN_PARENT );
                        CString Text = TreeCtrl.GetItemText( hParent );
                    }
                }
                hItem = hParent;
            }
            else
            {
                hItem = hChildItem;
            }
        }
        else
        {
            // Keep going back till we find the next sibling.
            HTREEITEM hParent = TreeCtrl.GetNextItem( hItem, TVGN_PARENT );
            while( hParent )
            {
                HTREEITEM hNextItem = TreeCtrl.GetNextItem( hParent, TVGN_NEXT );
                if( hNextItem )
                {
                    hParent = hNextItem;
                    break;
                }
                else
                {
                    hParent = TreeCtrl.GetNextItem( hParent, TVGN_PARENT );
                }
            }
            hItem = hParent;
        }
    }
}

//=========================================================================

void CAudioEditorView::ClearSelection( void )
{
    CSoundDoc* Doc =  GetDocument();
    GetTreeCtrl().SelectItem( NULL );
    Doc->m_AudioEditor.ClearMultiSelIndex();    

    for( s32 i = 0; i < m_hItemSelList.GetCount(); i++ )
            GetTreeCtrl().SetItemState( m_hItemSelList[i], 0, TVIS_SELECTED );
}

//=========================================================================

xbool CAudioEditorView::AddItemsToList(HTREEITEM hItemFrom, HTREEITEM hItemTo, xarray<HTREEITEM>& ItemList)
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

HTREEITEM CAudioEditorView::GetFirstSelectedItem()
{
	for ( HTREEITEM hItem = GetTreeCtrl().GetRootItem(); hItem != NULL; hItem = GetNextItem( hItem ) )
		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CAudioEditorView::GetNextSelectedItem( HTREEITEM hItem )
{
	for ( hItem = GetNextItem( hItem ); hItem != NULL; hItem = GetNextItem( hItem ) )
		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CAudioEditorView::GetPrevSelectedItem( HTREEITEM hItem )
{
	for ( hItem = GetPrevItem( hItem ); hItem!=NULL; hItem = GetPrevItem( hItem ) )
		if ( GetTreeCtrl().GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CAudioEditorView::GetNextItem( HTREEITEM hItem )
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

void CAudioEditorView::SelectControlItem ( xarray<HTREEITEM>& ItemList )
{
    x_DebugMsg( "%d Items in the list\n", ItemList.GetCount() );

    CSoundDoc* Doc      =  GetDocument();
    CTreeCtrl& TreeCtrl = GetTreeCtrl();
    Doc->m_AudioEditor.SetMultiSelect( TRUE );

    s32 i = 0;

    ClearSelection();

    for( i = 0; i < ItemList.GetCount(); i++ )
    {
        TreeCtrl.SetItemState( ItemList[i] ,  TVIS_SELECTED , TVIS_SELECTED );
        DWORD Data = TreeCtrl.GetItemData( ItemList[i] );
        
        RECT Rect;
        TreeCtrl.GetItemRect( ItemList[i], &Rect, 0 );
        InvalidateRect( &Rect );
                        
        multi_sel Selection;

        if( Data & PACKAGE )
        {
            // Get the package index.
            Selection.m_SelectedType |= PACKAGE_SELECTED;
            Selection.m_PackageHandle = ( Data & ~PACKAGE);    
        }
        else if( Data & DESCRIPTOR )
        {
            // Get the package index.
            HTREEITEM hParent = GetTreeCtrl().GetNextItem( ItemList[i], TVGN_PARENT );
            DWORD ParentData = GetTreeCtrl().GetItemData( hParent );

            // Set the package and descriptor index.
            Selection.m_SelectedType   |= DESCRIPTOR_SELECTED;
            Selection.m_PackageHandle   = ( ParentData & ~PACKAGE);    
            Selection.m_DescriptorIndex = ( Data & ~DESCRIPTOR );
        }
        else if( Data & ELEMENT )
        {

            // Get the parent descriptors index.
            HTREEITEM Parent = GetTreeCtrl().GetParentItem( ItemList[i] );
            DWORD ParentData = GetTreeCtrl().GetItemData( Parent );
            s32 ParentIndex  = (ParentData & ~DESCRIPTOR);
        
            // Get the package index.
            HTREEITEM hDescParent = GetTreeCtrl().GetNextItem( Parent, TVGN_PARENT );
            DWORD DescParentData  = GetTreeCtrl().GetItemData( hDescParent );

            // Set the package, descriptor and the element index.
            Selection.m_SelectedType   |= ELEMENT_SELECTED;
            Selection.m_PackageHandle   = ( DescParentData & ~PACKAGE);    
            Selection.m_DescriptorIndex = ( ParentData & ~DESCRIPTOR );
            Selection.m_ElementIndex    = ( Data & ~ELEMENT );
        }

        // Don't inset any selection indexes for a folder.
        if( !(Data & FOLDER) )
            Doc->m_AudioEditor.InsertMultiSelIndex( Selection );
    }
}

//=========================================================================

void CAudioEditorView::SetSelectedItemData( void )
{
    if( m_CurrentItemHit == NULL )
        return;
    
    CSoundDoc* Doc =  GetDocument();

    DWORD Data = GetTreeCtrl().GetItemData( m_CurrentItemHit );
    Doc->m_SelectedItemData = Data;

    if( Data & FOLDER )
    {
        // Clear all the sample view.
        Doc->ClearAllSamples();

        xhandle PackageSelected = Doc->m_AudioEditor.m_PackageSelected;

        // Don't do anything and change the selection type.
        if( (Doc->m_AudioEditor.m_pDesc.GetCount()) && (PackageSelected != -1) )
        {    
            Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected = 0;
            Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected |=  FOLDER_SELECTED;

            Doc->m_pPropEditor->Refresh();
        }
        
        //GetTreeCtrl().SortChildren( m_CurrentItemHit );
    }
    else if( Data & PACKAGE )
    {
        // Clear all the sample view.
        Doc->ClearAllSamples();

        Doc->m_AudioEditor.m_PackageSelected = ( Data & ~PACKAGE);
        xhandle PackageSelected = Doc->m_AudioEditor.m_PackageSelected;
        
        // Package is selected.
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected            = 0;
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected            |=  PACKAGE_SELECTED;
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_DescriptorSelected  = -1;

        GetTreeCtrl().SetItemText( m_CurrentItemHit, Doc->m_AudioEditor.m_pDesc( PackageSelected ).GetName() );

        Doc->m_pPropEditor->Refresh();

    }
    else if( Data & DESCRIPTOR )
    {
        // Get the package index.
        HTREEITEM hParent = GetTreeCtrl().GetNextItem( m_CurrentItemHit, TVGN_PARENT );
        DWORD ParentData = GetTreeCtrl().GetItemData( hParent );
        xhandle PackageSelected = (ParentData & ~PACKAGE);
        s32 DescIndex = (Data & ~DESCRIPTOR);

        // Clear all the sample view.
        Doc->ClearAllSamples();

        
        Doc->m_AudioEditor.m_PackageSelected = PackageSelected;

        // Get the index out of the item.
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_DescriptorSelected = DescIndex;
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected = 0;
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected |=  DESCRIPTOR_SELECTED;

        s32 i = 0;
		
		editor_descriptor& rDesc = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ];

        // Tell the doc to start opening all the samples attached with this descriptor.
        if (Doc->m_pCommandHandler)
        {
            // Walk through all the element.
            for( i = 0; i < rDesc.m_pElements.GetCount(); i++ )
            {
                f32 Offset = rDesc.m_pElements[i].m_Delta;

                if( rDesc.m_Type != EDITOR_COMPLEX )
                    Offset = 0.0f;
                
				// Does this element refrence a descriptor.
                if( rDesc.m_pElements[i].m_ReferenceDescIndex == -1 )
                    Doc->LoadSample( rDesc.m_pElements[i].m_pSamplePathName, Offset);
            }
        }

        // Set the items text to the label of the descriptor.
        GetTreeCtrl().SetItemText( m_CurrentItemHit, rDesc.m_Label );

        for( i = 0; i < rDesc.m_pElements.GetCount(); i++ )
        {
            // Is the elements parent name the same ? or has it changed.
            if( x_strcmp( rDesc.m_Label, rDesc.m_pElements[i].m_ParentDesc.m_Label ) )
            {
                x_strcpy( rDesc.m_pElements[i].m_ParentDesc.m_Label, rDesc.m_Label );
            }
        }

        Doc->m_pPropEditor->Refresh();

    }
    else if( Data & ELEMENT )
    {
        // Get the parent descriptors index.
        HTREEITEM Parent = GetTreeCtrl().GetParentItem( m_CurrentItemHit );
        DWORD ParentData = GetTreeCtrl().GetItemData( Parent );
        s32 ParentIndex = (ParentData & ~DESCRIPTOR);
        
        // Get the package index.
        HTREEITEM hDescParent = GetTreeCtrl().GetNextItem( Parent, TVGN_PARENT );
        DWORD DescParentData = GetTreeCtrl().GetItemData( hDescParent );
        xhandle PackageSelected = (DescParentData & ~PACKAGE);

        // Clear all the sample view.
        Doc->ClearAllSamples();
        
        Doc->m_AudioEditor.m_PackageSelected = PackageSelected;
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_DescriptorSelected = ParentIndex;       

        // Get the index out of the item.
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_ElementSelected = (Data & ~ELEMENT);
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected = 0;
        Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected |=  ELEMENT_SELECTED;

		editor_descriptor& rDesc = Doc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ ParentIndex ];

        for( s32 i = 0; i < rDesc.m_pElements.GetCount(); i++ )
        {
            // Does this element refrence a descriptor.
            if( rDesc.m_pElements[ i ].m_ReferenceDescIndex == -1  )
            {
                f32 Offset = rDesc.m_pElements[i].m_Delta;
                
                // Is this element part of a complex descriptor.
                if( rDesc.m_Type != EDITOR_COMPLEX )
                    Offset = 0.0f;
                
                // Load the sample.
                if( rDesc.m_pElements[i].m_ReferenceDescIndex == -1 )
                    Doc->LoadSample( rDesc.m_pElements[ i ].m_pSamplePathName, Offset );
            }
        }

        // Set which element is selected.
        Doc->m_SampleSelected = (Data & ~ELEMENT);
        
        editor_element& rElement = Doc->m_AudioEditor.m_pDesc[ PackageSelected ].m_pDescriptorList[ ParentIndex ].m_pElements[ Doc->m_SampleSelected ];
        
        // If its a reference then don't change the name of the item.
        if( rElement.m_ReferenceDescIndex == -1 )
            GetTreeCtrl().SetItemText( m_CurrentItemHit, rElement.m_pSampleName );

        Doc->m_pPropEditor->Refresh();
    }    
}

//=========================================================================

void CAudioEditorView::OnMoveTo( void )
{
    x_try;

    CSoundDoc* Doc =  GetDocument();
    m_hItemMoveList.Clear();

    for( s32 i = 0; i < m_hItemSelList.GetCount(); i++ )
    {
        DWORD Data      = GetTreeCtrl().GetItemData( m_hItemSelList[i] );
                
        if( !(Data & PACKAGE) )
            x_throw( "Please only select package type to perform this operation." );

    }
    
    m_hItemMoveList = m_hItemSelList;

    x_catch_begin;
    
    m_hItemMoveList.Clear();

    x_catch_end;
}

//=========================================================================

void CAudioEditorView::OnMovePaste( void )
{
    x_try;
        
    //if( m_hItemSelList.GetCount() > 1 )
        //x_throw( "Please select A folder to move the packages to." );
    
    if( !(GetTreeCtrl().GetItemData(m_CurrentItemHit) & FOLDER) )
        x_throw( "Please select a Folder to move the package to." );

    CSoundDoc* pDoc =  GetDocument();

    // Save the virtual directory path.
    CString BackwardVirtualDir;
    xstring VirDir;
    HTREEITEM hParentItem =  m_CurrentItemHit;//GetTreeCtrl().GetNextItem( hChildItem, TVGN_PARENT );
    while( hParentItem )
    {
        BackwardVirtualDir = GetTreeCtrl().GetItemText( hParentItem );
        VirDir.Insert( 0, '\\' );
        VirDir.Insert( 0, (LPCTSTR)BackwardVirtualDir );
        hParentItem = GetTreeCtrl().GetNextItem( hParentItem, TVGN_PARENT );
    }

    for( s32 i =0; i < m_hItemMoveList.GetCount(); i++ )
    {        
        DWORD Data = GetTreeCtrl().GetItemData( m_hItemMoveList[i] );
        xhandle PackageHandle = ( Data & ~PACKAGE );
        
        // Insert the package in the new branch.
        OnLoadAudioPackage( PackageHandle, m_CurrentItemHit );
                            
        // Save the path of the virtual dir again.
        x_strncpy( pDoc->m_AudioEditor.m_pDesc( PackageHandle ).m_pVirtualDirectory, VirDir, 128);
        pDoc->m_AudioEditor.m_pDesc( PackageHandle ).SetChanged( TRUE );

        // Delete the package in the old directory.
        GetTreeCtrl().DeleteItem( m_hItemMoveList[i] );
    }

    pDoc->m_pPropEditor->Refresh();
    
    x_catch_display;
}

//=========================================================================

void CAudioEditorView::OnCopyEvent( void )
{
    OnCopy();
}

xbool CAudioEditorView::OnCopy ( void )
{
    x_try;

    m_hItemCopyList.Clear();

    s32 i = 0;
    for( i = 1; i < m_hItemSelList.GetCount(); i++ )
    {
        DWORD PrevData  = GetTreeCtrl().GetItemData( m_hItemSelList[i-1] );
        DWORD Data      = GetTreeCtrl().GetItemData( m_hItemSelList[i] );
                
        // Get rid of the index data.
        Data = Data>>8;
        PrevData = PrevData>>8;

        if( Data != PrevData )
            x_throw( "Please select similar types (packages, descriptor or element) for this operation." );

    }

    m_hItemCopyList = m_hItemSelList;
    
    return TRUE;

    x_catch_begin;
    
        m_hItemCopyList.Clear();
        return FALSE;

    x_catch_end;
}

//=========================================================================

void CAudioEditorView::OnPaste ( void )
{
    x_try;

    CSoundDoc* pDoc =  GetDocument();

    s32 i = 0;

    for( i = 0; i < m_hItemCopyList.GetCount(); i++ )
    {
        DWORD Data = GetTreeCtrl().GetItemData( m_hItemCopyList[i] );
                
        if( Data & PACKAGE )
        {
            xhandle NewPackageHandle = OnNewAudioPackage();
            
            if( NewPackageHandle == -1 )
                x_throw( "Please retry the paste operation" );

            s32 PackageHandle = ( Data & ~PACKAGE );

            pDoc->m_AudioEditor.m_pDesc( NewPackageHandle )= pDoc->m_AudioEditor.m_pDesc( PackageHandle );
            GetTreeCtrl().SetItemText( m_NewestAddedPackage, pDoc->m_AudioEditor.m_pDesc( NewPackageHandle ).GetName() );

            // Add the packages descriptor and elements in to the virtual directory.
            for( s32 j = 0; j < pDoc->m_AudioEditor.m_pDesc( NewPackageHandle ).m_pDescriptorList.GetCount(); j++ )
            {
                editor_descriptor& rDesc = pDoc->m_AudioEditor.m_pDesc( NewPackageHandle ).m_pDescriptorList[j];
				HTREEITEM AddedItem = GetTreeCtrl().InsertItem( rDesc.m_Label, 4, 5, m_NewestAddedPackage);
                
                DWORD DescData = j;
                DescData |= DESCRIPTOR;

                GetTreeCtrl().SetItemData( AddedItem, DescData );
				
				// Check if this descriptor was being refrenced.
				for( s32 k = 0; k < rDesc.m_pReferencingElement.GetCount(); k++ )
				{
					// Append the refrence count for the elements that are refrencing this element.
					rDesc.m_pReferencingElement[k]->m_ReferenceDescIndex++;
				}
                
                s32 ElementCount = rDesc.m_pElements.GetCount();
                for( s32 p = 0; p < ElementCount; p++ )
                {
                    HTREEITEM ElementItem = GetTreeCtrl().InsertItem( rDesc.m_pElements[p].m_pSampleName, 6, 7, AddedItem);
                    
                    DWORD ElementData = p;
                    ElementData |= ELEMENT;

                    GetTreeCtrl().SetItemData( ElementItem, ElementData );
                }
            }

        }
        else if( Data & DESCRIPTOR )
        {
            s32 NewDesc = OnNewAudioDescriptor();

            if( NewDesc == -1 )
                x_throw( "Please retry the paste operation" );

            s32 DescIndex = (Data & ~DESCRIPTOR);
           
            // Get the package index.
            HTREEITEM hParent       = m_CurrentItemHit;
            DWORD ParentData        = GetTreeCtrl().GetItemData( hParent );
            xhandle PackageSelected = (ParentData & ~PACKAGE);
            
            // Get the index of the package that we are coping from.
            HTREEITEM hCopyParent       = GetTreeCtrl().GetNextItem( m_hItemCopyList[i], TVGN_PARENT );
            DWORD CopyParentData        = GetTreeCtrl().GetItemData( hCopyParent );
            xhandle CopyPackageSelected = (CopyParentData & ~PACKAGE);
            
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ NewDesc ] = 
                pDoc->m_AudioEditor.m_pDesc( CopyPackageSelected ).m_pDescriptorList[ DescIndex ];

            editor_descriptor& rDesc = pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ NewDesc ];
			
			GetTreeCtrl().SetItemText( m_NewestAddedDesc, rDesc.m_Label );

            // Check if this descriptor was being refrenced.
            s32 j = 0;
            for( ; j < rDesc.m_pReferencingElement.GetCount(); j++ )
            {
                // Append the refrence count for the elements that are refrencing this element.
                rDesc.m_pReferencingElement[j]->m_ReferenceDescIndex++;
            }

            s32 ElementCount = rDesc.m_pElements.GetCount();
            for( j = 0; j < ElementCount; j++ )
            {
                HTREEITEM ElementItem = GetTreeCtrl().InsertItem( rDesc.m_pElements[j].m_pSampleName, 6, 7, m_NewestAddedDesc );

                DWORD ElementData = j;
                ElementData |= ELEMENT;

                GetTreeCtrl().SetItemData( ElementItem, ElementData );
            }

        }
        else if( Data & ELEMENT )
        {            
			// Get the parent descriptors index.
            HTREEITEM Parent = m_CurrentItemHit;//GetTreeCtrl().GetParentItem( hItem );
            DWORD ParentData = GetTreeCtrl().GetItemData( Parent );
            s32 DescIndex    = (ParentData & ~DESCRIPTOR);
        
            // Get the package index.
            HTREEITEM hDescParent   = GetTreeCtrl().GetNextItem( Parent, TVGN_PARENT );
            DWORD DescParentData    = GetTreeCtrl().GetItemData( hDescParent );
            xhandle PackageSelected = (DescParentData & ~PACKAGE);

            // Get the copy parent descriptors index.
            HTREEITEM CopyParent = GetTreeCtrl().GetParentItem( m_hItemCopyList[i] );
            DWORD CopyParentData = GetTreeCtrl().GetItemData( CopyParent );
            s32 CopyDescIndex    = (CopyParentData & ~DESCRIPTOR);
        
            // Get the copy package index.
            HTREEITEM hCopyDescParent   = GetTreeCtrl().GetNextItem( CopyParent, TVGN_PARENT );
            DWORD CopyDescParentData    = GetTreeCtrl().GetItemData( hCopyDescParent );
            xhandle CopyPackageSelected = (CopyDescParentData & ~PACKAGE);

            s32 ElementIndex = ( Data & ~ELEMENT );
            CString PathName = pDoc->m_AudioEditor.m_pDesc( CopyPackageSelected ).m_pDescriptorList[ CopyDescIndex ].m_pElements[ ElementIndex ].m_pSamplePathName;
            s32 NewElement   = OnNewAudioElement( PathName );

            if( NewElement == -1 )
                x_throw( "Please retry the paste operation" );

            // Copy the element and the parent descriptor over.
			pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ NewElement ] =
                pDoc->m_AudioEditor.m_pDesc( CopyPackageSelected ).m_pDescriptorList[ CopyDescIndex ].m_pElements[ ElementIndex ];

            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ NewElement ].m_ParentDesc = 
                pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ];
			
			GetTreeCtrl().SetItemText( m_NewestAddedElement, pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ NewElement ].m_pSampleName );
        }
        else
        {
            ASSERT( 0 );
        }
    }
    
    x_catch_display;
}

//=========================================================================

void CAudioEditorView::OnTabActivate( BOOL bActivate )
{
	if( bActivate )
    {
        CSoundDoc* pDoc = GetDocument();
        xhandle PackageSelected = pDoc->m_AudioEditor.m_PackageSelected;

        //if( PackageSelected != -1 )
        //    pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_Selected = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_PrevSelected;

        GetDocument()->m_AudioEditor.SetUpdateLabelFn( this, UpdateLabelCallBack );

        if( PackageSelected != -1 )
        {
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_PrevSelected = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_Selected;
            //pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected = 0;
            //pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected |=  PACKAGE_SELECTED;
            

            GetDocument()->m_pPropEditor->Refresh();
        }

        GetDocument()->m_AudioEditor.ClearParamsMode();
    }
}

//=========================================================================

void CAudioEditorView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
    if( bActivate )
    {
	    CSoundDoc* pDoc = GetDocument();
        xhandle PackageSelected = pDoc->m_AudioEditor.m_PackageSelected;

        //if( PackageSelected != -1 )
        //    pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_Selected = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_PrevSelected;

        GetDocument()->m_AudioEditor.SetUpdateLabelFn( this, UpdateLabelCallBack );

        if( PackageSelected != -1 )
        {
            pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_PrevSelected = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_Selected;
            //pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected = 0;
            //pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_Selected |=  PACKAGE_SELECTED;

            GetDocument()->m_pPropEditor->Refresh();
        }

        GetDocument()->m_AudioEditor.ClearParamsMode();
    }

	CXTTreeView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

//=========================================================================

void CAudioEditorView::OnKillFocus(CWnd* pNewWnd) 
{
    CSoundDoc* pDoc = GetDocument();
    s32 PackageSelected = pDoc->m_AudioEditor.m_PackageSelected;
    
    //if( PackageSelected != -1 )
    //    pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_PrevSelected = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_Selected;

	//GetDocument()->RefreshIntensityView();

    CView::OnKillFocus(pNewWnd);
}

//=========================================================================

void CAudioEditorView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	
}

//=========================================================================

void CAudioEditorView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	(void)pResult;

    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    CTreeCtrl& TreeCtrl = GetTreeCtrl();   
	
    TreeCtrl.GetItemData( pNMTreeView->itemNew.hItem );

    x_DebugMsg( "Entering Drag, Item Selected Count [%d]\n", m_hItemSelList.GetCount() );
    if( OnCopy() == FALSE )
        return;
    
    if( TreeCtrl.GetItemData(pNMTreeView->itemNew.hItem) & PACKAGE )
        OnMoveTo();

    // So user cant anything but descriptors.
	//if( !(TreeCtrl.GetItemData(pNMTreeView->itemNew.hItem) & DESCRIPTOR ) ) 
        //return; 

    //if( GetDocument()->m_AudioEditor.GetMultiSelect() )
        //return;

	// Item user started dragging ...
	m_hItemDrag = pNMTreeView->itemNew.hItem;
	m_hItemDrop = NULL;

	// Get the image list for dragging
    m_pDragImage = TreeCtrl.CreateDragImage( m_hItemDrag );

	// CreateDragImage() returns NULL if no image list
	// associated with the tree view control
	if( !m_pDragImage )
		return;

	m_bLDragging = TRUE;
	m_pDragImage->BeginDrag(0, CPoint(-15,-15));
	POINT pt = pNMTreeView->ptDrag;
	TreeCtrl.ClientToScreen( &pt );
	m_pDragImage->DragEnter(NULL, pt);
	TreeCtrl.SetCapture();

    EnableToolTips( FALSE );
}

//=========================================================================

void CAudioEditorView::OnMouseMove(UINT nFlags, CPoint point) 
{
	HTREEITEM	hitem;
	UINT		flags;
    CTreeCtrl& TreeCtrl = GetTreeCtrl();   
	
	if (m_bLDragging)
	{
		POINT pt = point;
		ClientToScreen( &pt );
		CImageList::DragMove(pt);
        hitem = TreeCtrl.HitTest(point, &flags);

        // Is it a valid item and was a good hit detection.
		if ((hitem != NULL) && ((flags & TVHT_ONITEMLABEL)||(flags&TVHT_ONITEMICON)) )
		{
            CImageList::DragShowNolock(FALSE);
				
			// Is the drag data valid with the current drop item we are on now?
            DWORD DragData = TreeCtrl.GetItemData( m_hItemDrag );
            xbool ValidDragItem = FALSE;
            if( DragData & PACKAGE )
            {
                if( (TreeCtrl.GetItemData( hitem ) & FOLDER) )
                {
			        if(m_CursorArr != ::GetCursor()) 
                        ::SetCursor(m_CursorArr); 
			        TreeCtrl.SelectDropTarget(hitem);
                    ValidDragItem = TRUE;
                }
            }
            else if( DragData & DESCRIPTOR )
            {

                if( (TreeCtrl.GetItemData( hitem ) & PACKAGE) )
                {
			        if(m_CursorArr != ::GetCursor()) 
                        ::SetCursor(m_CursorArr); 
			        TreeCtrl.SelectDropTarget(hitem);
                    ValidDragItem = TRUE;
                }
                else if((TreeCtrl.GetItemData( hitem ) & ELEMENT) &&
                        (TreeCtrl.GetParentItem( hitem ) != m_hItemDrag) &&
                        (m_hItemSelList.GetCount() == 1) ) 
                {
			        if(m_CursorArr != ::GetCursor()) 
                        ::SetCursor(m_CursorArr); 
			        TreeCtrl.SelectDropTarget(hitem);
                    ValidDragItem = TRUE;
                }
                else if((TreeCtrl.GetItemData( hitem ) & DESCRIPTOR) &&
                        (hitem != m_hItemDrag)  &&
                        (m_hItemSelList.GetCount() == 1) ) 
                {
			        if(m_CursorArr != ::GetCursor()) 
                        ::SetCursor(m_CursorArr); 
			        TreeCtrl.SelectDropTarget(hitem);
                    ValidDragItem = TRUE;
                }

            }
            else if( DragData & ELEMENT )
            {
                if( (TreeCtrl.GetItemData( hitem ) & DESCRIPTOR) )
                {
			        if(m_CursorArr != ::GetCursor()) 
                        ::SetCursor(m_CursorArr); 
			        TreeCtrl.SelectDropTarget(hitem);
                    ValidDragItem = TRUE;
                }
            }
            else
            {
                ASSERT( 0 );
            }

            if( ValidDragItem == FALSE )
            // Tests if dragged item is over another child !
            //if ( (GetParentItem(hitem) != NULL) && (cursor_no != ::GetCursor())) 
            {
              ::SetCursor(m_CursorNo);
               // Dont want last selected target highlighted after mouse
               // has moved off of it, do we now ?
               TreeCtrl.SelectDropTarget(NULL);
            }

			m_hItemDrop = hitem;
			CImageList::DragShowNolock(TRUE);

            RECT Rect;
            TreeCtrl.GetItemRect( hitem, &Rect, 0 );
            InvalidateRect( &Rect );
		}
	}
	else 
	{
		// Set cursor to arrow if not dragged
		// Otherwise, cursor will stay hand or arrow depen. on prev setting
		::SetCursor(m_CursorArr);
	}

	CXTTreeView::OnMouseMove(nFlags, point);
}

//=========================================================================

void CAudioEditorView::OnLButtonUp(UINT nFlags, CPoint point) 
{

	CXTTreeView::OnLButtonUp(nFlags, point);
    CTreeCtrl& TreeCtrl = GetTreeCtrl();   

	if (m_bLDragging)
	{
		EnableToolTips( TRUE );

        m_bLDragging = FALSE;
		CImageList::DragLeave(this);
		CImageList::EndDrag();
		ReleaseCapture();

		if(m_pDragImage != NULL) 
		{ 
		    delete m_pDragImage; 
		    m_pDragImage = NULL; 
		} 

		// Remove drop target highlighting
		TreeCtrl.SelectDropTarget(NULL);

		if( m_hItemDrag == m_hItemDrop )
			return;

		HTREEITEM	hitem = TreeCtrl.HitTest(point, &nFlags);


        // Make sure pt is over some item
		if( (hitem == NULL) || !((nFlags & TVHT_ONITEMLABEL)||(nFlags&TVHT_ONITEMICON)) ) 
            return;
        
        // Set the drag and droped item to be the prev and the curr items.
        m_PrevItemHit       = m_hItemDrag;
        m_CurrentItemHit    = hitem;

        SetSelectedItemData();
        
	    // What type of data are we drop.
        DWORD DragData = TreeCtrl.GetItemData( m_hItemDrag );
        if( DragData & PACKAGE )
        {
            if( TreeCtrl.GetItemData( hitem ) & FOLDER )
            {
                if( TreeCtrl.GetParentItem( m_hItemDrag ) == hitem )
                    OnPaste();
                else
                    OnMovePaste();
            }
        }
        else if( DragData & DESCRIPTOR )
        {

            if( (TreeCtrl.GetItemData( hitem ) & ELEMENT) &&
                (TreeCtrl.GetParentItem( hitem ) != m_hItemDrag) &&
                (m_hItemSelList.GetCount() == 1) )
            {
                RefrenceDescriptor();
            }
            else if( TreeCtrl.GetItemData( hitem ) & PACKAGE )
            {
                OnPaste();
            }
            else if((TreeCtrl.GetItemData( hitem ) & DESCRIPTOR) &&
                    (hitem != m_hItemDrag) &&
                    (m_hItemSelList.GetCount() == 1) )
            {
                
                // Create a new element.
                HTREEITEM NewElement;
                if( CreateElement( m_CurrentItemHit, NewElement ) != -1 )
                {
                    // Reset the selection data.
                    //m_PrevItemHit       = m_CurrentItemHit;
                    m_CurrentItemHit    = NewElement;
                    
                    // Reference the new element that was created.
                    RefrenceDescriptor();

                    SetSelectedItemData();
                }
            }
        }
        else if( DragData & ELEMENT )
        {
            if( (TreeCtrl.GetItemData( hitem ) & DESCRIPTOR) )
            {
                OnPaste();
            }
        }
        else
        {
            ASSERT( 0 );
        }

        // Select all drop item.
        OnLButtonDown( 0, point );
	}
    else
    {
        if( m_CheckMultiSelectRelease )
        {
            GetDocument()->m_AudioEditor.SetMultiSelect( FALSE );
            m_CheckMultiSelectRelease = FALSE;
            OnLButtonDown( 0, point );
        }
    }
}

//=========================================================================

void CAudioEditorView::SetDefaultCursor( void )
{
    // Get the windows directory
    CString strWndDir;
    GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
    strWndDir.ReleaseBuffer();

    strWndDir += _T("\\winhlp32.exe");
    // This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
    HMODULE hModule = LoadLibrary(strWndDir);
    if (hModule) {
        HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
        if (hHandCursor)
	    {
            m_CursorHand = CopyCursor(hHandCursor);
	    }
		      
    }
    FreeLibrary(hModule);

    m_CursorArr	= ::LoadCursor(NULL, IDC_ARROW);
    m_CursorNo	= ::LoadCursor(NULL, IDC_NO) ;
}

//=========================================================================
