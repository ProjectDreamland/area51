// AudioFileTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "AudioFileTree.h"
#include "..\WinControls\FileSearch.h"
#include "resource.h"
#include "SoundDoc.h"
#include "AudioEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioFileTreeCtrl

BEGIN_MESSAGE_MAP(CAudioFileTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CAudioFileTreeCtrl)
    ON_NOTIFY_REFLECT(NM_CLICK, OnLclick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================

CAudioFileTreeCtrl::CAudioFileTreeCtrl() :
{
    m_bUsePrev = FALSE;
    m_bInit = FALSE;
    m_hItemFirstSel = NULL;
    m_hItemSelList.SetGrowAmount( 10 );
}

//=========================================================================

CAudioFileTreeCtrl::~CAudioFileTreeCtrl()
{
    if (m_bInit)
	    m_imageList.DeleteImageList();
}


//=========================================================================

void CAudioFileTreeCtrl::OnLclick(NMHDR* pNMHDR, LRESULT* pResult) 
{

    //Work out the position of where the context menu should be
    CPoint p(GetCurrentMessage()->pt);
    CPoint point(p);
    ScreenToClient(&point);

	if (( GetKeyState( VK_SHIFT ) & ~1 ) != 0)
	{
        // Are there any items selected by control.
        if( m_hItemSelList.GetCount() > 0 )
        {
            // Use the head of the list as the first item selected.
            m_hItemFirstSel = m_hItemSelList[ 0 ];
        }

		// Shift key is down
		UINT flag;
		HTREEITEM hItem = HitTest( point, &flag );

		// Initialize the reference item if this is the first shift selection
		if( !m_hItemFirstSel )
			m_hItemFirstSel = GetSelectedItem();

		// Select new item
		if( GetSelectedItem() == hItem )
			SelectItem( NULL );			// to prevent edit
		
        //CTreeCtrl::OnLButtonDown(nFlags, point);

		if( m_hItemFirstSel )
		{

            ClearSelection();
            m_hItemSelList.Clear();
            
            AddItemsToList( m_hItemFirstSel, hItem, m_hItemSelList );

            SelectControlItem( m_hItemSelList );

			return;
		}
	}
	else if(( GetKeyState( VK_CONTROL ) & ~1 ) != 0)    
	{

        UINT flag;
        HTREEITEM hItem = HitTest( point, &flag );
        HTREEITEM hSelItem = GetSelectedItem();

		// Select new item
//		if( hSelItem == hItem )
//			SelectItem( NULL );			// to prevent edit


        for( s32 i = 0; i < m_hItemSelList.GetCount(); i++ )
        {
            if( m_hItemSelList[i] == hItem )
            {
                SelectItem( NULL );			// to prevent edit
                
                ClearSelection();
                m_hItemSelList.Delete( i );
               
                SelectControlItem( m_hItemSelList );
                return;

            }
        }

        m_hItemSelList.Append( hItem );

        SelectControlItem( m_hItemSelList );

        return;
	}
	else
	{
		// Normal - remove all selection and let default 
		// handler do the rest
		ClearSelection();
        m_hItemSelList.Clear();
        m_hItemFirstSel = HitTest(point);
        m_hItemSelList.Append( m_hItemFirstSel );

        SelectControlItem( m_hItemSelList );
		
	}

//    Select(HitTest(pt), TVGN_CARET);

    *pResult = 0;

}

//=========================================================================

void CAudioFileTreeCtrl::ClearSelection( void )
{
    for( s32 i = 0; i < m_hItemSelList.GetCount(); i++ )
        SetItemState( m_hItemSelList[i], 0, TVIS_SELECTED );
}

//=========================================================================

xbool CAudioFileTreeCtrl::AddItemsToList(HTREEITEM hItemFrom, HTREEITEM hItemTo, xarray<HTREEITEM>& ItemList)
{
	HTREEITEM hItem = GetRootItem();

	// Clear selection upto the first item
	while ( (hItem) && (hItem != hItemFrom) && (hItem != hItemTo) )
	{
		hItem = GetNextVisibleItem( hItem );
		SetItemState( hItem, 0, TVIS_SELECTED );
	}

	if ( !hItem )
		return FALSE;	// Item is not visible

	SelectItem( hItemTo );

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
		//SetItemState( hItem, bSelect ? TVIS_SELECTED : 0, TVIS_SELECTED );
        if( bSelect )
            ItemList.Append( hItem );

		// Do we need to start removing items from selection
		if( hItem == hItemTo ) 
			bSelect = FALSE;

		hItem = GetNextVisibleItem( hItem );
	}

	return TRUE;
}

//=========================================================================

HTREEITEM CAudioFileTreeCtrl::GetFirstSelectedItem()
{
	for ( HTREEITEM hItem = GetRootItem(); hItem != NULL; hItem = GetNextItem( hItem ) )
		if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CAudioFileTreeCtrl::GetNextSelectedItem( HTREEITEM hItem )
{
	for ( hItem = GetNextItem( hItem ); hItem != NULL; hItem = GetNextItem( hItem ) )
		if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
			return hItem;

	return NULL;
}

//=========================================================================

HTREEITEM CAudioFileTreeCtrl::GetNextItem( HTREEITEM hItem )
{
        HTREEITEM       hti;

        if( ItemHasChildren( hItem ) )
                return GetChildItem( hItem );           // return first child
        else{
                // return next sibling item
                // Go up the tree to find a parent's sibling if needed.
                while( (hti = GetNextSiblingItem( hItem )) == NULL ){
                        if( (hItem = GetParentItem( hItem ) ) == NULL )
                                return NULL;
                }
        }
        return hti;
}

//=========================================================================

void CAudioFileTreeCtrl::SelectControlItem ( xarray<HTREEITEM>& ItemList )
{
    x_DebugMsg( "%d Items in the list\n", ItemList.GetCount() );

	HTREEITEM hItem = GetRootItem();
    s32 i = 0;

//    for( s32 i = 0; i < ItemList.GetCount(); i++ )
//        SetItemState( ItemList[i] ,  0 , TVIS_SELECTED );

    ClearSelection();

    for( i = 0; i < ItemList.GetCount(); i++ )
        SetItemState( ItemList[i] ,  TVIS_SELECTED , TVIS_SELECTED );
}