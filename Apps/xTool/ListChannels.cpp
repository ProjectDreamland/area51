// ListChannels.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "ListChannels.h"
#include "DlgFindLog.h"
#include "xToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListChannels

CListChannels::CListChannels()
{
    m_pDocument = NULL;
    m_ExtendedStyle = LVS_EX_CHECKBOXES;
}

CListChannels::~CListChannels()
{
}

/////////////////////////////////////////////////////////////////////////////
// SetDocument
void CListChannels::SetDocument( CxToolDoc* pDoc )
{
    m_pDocument = pDoc;
}

/////////////////////////////////////////////////////////////////////////////
// GetColumnText

const CString& CListChannels::GetCellText( log_channel* pEntry, int iCol )
{
    static CString s;

    switch( iCol )
    {
    case log_channel::field_Name:
        s = pEntry->GetName();
        break;
    case log_channel::field_Thread:
        s.Format( _T("%d"), pEntry->GetThreadID() );
        break;
    }

    return s;
}

/////////////////////////////////////////////////////////////////////////////

int CListChannels::FindNextItem( int nItem, const CString& SearchText, int Delta )
{
    ASSERT( m_pDocument );

    CString SearchTextUpper = SearchText;
    SearchTextUpper.MakeUpper();

    // Get the channels
    channel_array& Channels = m_pDocument->GetFilteredChannels();

    // Are there any channel entries?
    if( GetItemCount() > 0 )
    {
        // Iterate over items
        int StartItem = nItem;
        nItem = (nItem + Delta) % GetItemCount();
        if( nItem < 0 )
            nItem += GetItemCount();
        while( nItem != StartItem )
        {
            ASSERT( nItem < Channels.GetSize() );

            // Get the channel entry text
            log_channel* pEntry = Channels[nItem];
            CString ItemText = GetCellText( pEntry, log_channel::field_Name );
            ItemText.MakeUpper();

            // Compare
            if( ItemText.Find( SearchTextUpper ) != -1 )
            {
                return nItem;
            }

            // Next item
            nItem = (nItem + Delta) % GetItemCount();
            if( nItem < 0 )
                nItem += GetItemCount();
        }
    }

    return -1;
}

/////////////////////////////////////////////////////////////////////////////

void CListChannels::Sort( void )
{
    OnSort();
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CListChannels, CVirtualListCtrl)
	//{{AFX_MSG_MAP(CListChannels)
        ON_MESSAGE( AM_FIND    , OnMessageFind    )
        ON_MESSAGE( AM_MARK_ALL, OnMessageMarkAll )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CListChannels message handlers

/////////////////////////////////////////////////////////////////////////////
// OnGetSelected

bool CListChannels::OnGetSelected( int iRow )
{
    if( m_pDocument == NULL )
        return FALSE;

    // Get the log entry
    channel_array&  Channels = m_pDocument->GetFilteredChannels();
    log_channel*    pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Channels.GetSize()) )
    {
        return FALSE;
    }

    // Get the channel entry
    pEntry = Channels[iRow];

    // Read the flag
    return( pEntry->GetFlags( log_channel::flag_selected ) != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// OnSetSelected

void CListChannels::OnSetSelected( int iRow, bool State )
{
    if( m_pDocument == NULL )
        return;

    // Get the log entry
    channel_array&  Channels = m_pDocument->GetFilteredChannels();
    log_channel*    pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Channels.GetSize()) )
    {
        return;
    }

    // Get the channel entry
    pEntry = Channels[iRow];

    // Set the flag
    pEntry->SetFlags( State ? log_channel::flag_selected : 0, log_channel::flag_selected );

    // Cause the log view to update
    m_pDocument->UpdateAllViews( NULL, CxToolDoc::HINT_LOG_REDRAW );
}

/////////////////////////////////////////////////////////////////////////////
// OnGetMarked

bool CListChannels::OnGetMarked( int iRow )
{
    if( m_pDocument == NULL )
        return FALSE;

    // Get the channel entry
    channel_array&  Channels = m_pDocument->GetFilteredChannels();
    log_channel*    pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Channels.GetSize()) )
    {
        return FALSE;
    }

    // Get the channel entry
    pEntry = Channels[iRow];

    // Read the flag
    return( pEntry->GetFlags( log_channel::flag_marked ) != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// OnSetMarked

void CListChannels::OnSetMarked( int iRow, bool State )
{
    if( m_pDocument == NULL )
        return;

    // Get the log entry
    channel_array&  Channels = m_pDocument->GetFilteredChannels();
    log_channel*    pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Channels.GetSize()) )
    {
        return;
    }

    // Get the channel entry
    pEntry = Channels[iRow];

    // Set the flag
    pEntry->SetFlags( State ? log_channel::flag_marked : 0, log_channel::flag_marked );
}

/////////////////////////////////////////////////////////////////////////////
// OnGetChecked

bool CListChannels::OnGetChecked( int iRow )
{
    if( m_pDocument == NULL )
        return FALSE;

    // Get the channel entry
    channel_array&  Channels = m_pDocument->GetFilteredChannels();
    log_channel*    pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Channels.GetSize()) )
    {
        return FALSE;
    }

    // Get the channels entry
    pEntry = Channels[iRow];

    // Read the flag
    return( pEntry->GetFlags( log_channel::flag_checked ) != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// OnSetChecked

void CListChannels::OnSetChecked( int iRow, bool State )
{
    if( m_pDocument == NULL )
        return;

    // Get the channel entry
    channel_array&  Channels = m_pDocument->GetFilteredChannels();
    log_channel*    pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Channels.GetSize()) )
    {
        return;
    }

    // Get the channels entry
    pEntry = Channels[iRow];

    // Set the flag
    pEntry->SetFlags( State ? log_channel::flag_checked : 0, log_channel::flag_checked );
}

/////////////////////////////////////////////////////////////////////////////
// OnDrawCell

BOOL CListChannels::OnDrawCell( CDC* pDC, CRect& rCell, int iRow, int iCol )
{
    ASSERT( m_pDocument );

    // Get the channel entry
    channel_array& Channels = m_pDocument->GetFilteredChannels();

    // Exit if out of range
    if( iRow >= Channels.GetSize() )
    {
        return FALSE;
    }

    // Get the channel entry
    log_channel* pEntry = Channels[iRow];

    // Make the text rectangle
    CRect rCellText = rCell;
    rCellText.DeflateRect( 2, 0, 2, 0 );

    // Get the colors
    COLORREF ct;
    COLORREF cb;
    ct = RGB(   0,   0,   0 );
    cb = RGB( 255, 255, 255 );

    // Change color for marked items
    if( OnGetMarked( iRow ) )
    {
        cb = RGB( 192,192,255 );
    }

    // Change color every other row
    if( iRow & 1 )
    {
        cb = DarkenColor( cb, 8, 8, 0 );
    }

    // Modify for sorting and selection
    int d = 0;
    if( OnGetSelected( iRow ) )
        d = 40;
    else if( iCol == m_SortColumn )
        d = 10;
    if( !m_IsFocusWnd )
        d /= 2;
    cb = DarkenColor( cb, d, d, d );

    // Fill the cell
    pDC->FillSolidRect( &rCell, cb );

    // Get the string
    const CString& Text = GetCellText( pEntry, iCol );

    // Draw the text
    pDC->DrawText( Text, &rCellText, m_Columns[iCol].m_TextAlign | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// OnSort

void CListChannels::OnSort( void )
{
    if( m_SortColumn != -1 )
    {
        CxToolDoc* pDoc = m_pDocument;
        if( pDoc )
        {
            // Exit if there are no channels
            channel_array& Channels = pDoc->GetFilteredChannels();
            if( Channels.GetSize() < 1 )
                return;

            // Get the current focus item if we have one
            int iItemFocus = m_FocusItem;
            log_channel* pEntry = NULL;
            if( iItemFocus != -1 )
                pEntry = (log_channel*)Channels[iItemFocus];

            // Sort the channels
            pDoc->SortChannels( m_SortColumn, m_SortAscending );

            // Rebuild the selection set from the newly sorted log
            BuildSelectionSet();

            // Find the item in the newly sorted list
            iItemFocus = pDoc->GetFilteredChannels().Find( pEntry );
            if( iItemFocus != -1 )
            {
                if( iItemFocus > GetItemCount() )
                    iItemFocus = GetItemCount();
                if( iItemFocus < 0 )
                    iItemFocus = 0;

                // Give it focus
                SetFocusItem( iItemFocus );

                // Ensure the new item is visible
                EnsureVisible( iItemFocus );
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnGetColumnFitWidth

int CListChannels::OnGetColumnFitWidth( int iCol )
{
    // Bail if no document
    if( m_pDocument == NULL )
        return -1;

    int Width = -1;

    // Get DC and set font into it
    CDC* pDC = GetDC();
    CFont* pOriginalFont = pDC->SelectObject( &m_Font );

    // Iterate over channel entries finding the widest
    channel_array& Channels = m_pDocument->GetFilteredChannels();
    for( int i=0 ; i<Channels.GetSize() ; i++ )
    {
        log_channel* pEntry = Channels[i];
        ASSERT( pEntry );

        const CString& Text = GetCellText( pEntry, iCol );
        CSize Size = pDC->GetTextExtent( Text );
        Size.cx += 5;
        if( Size.cx > Width )
            Width = Size.cx;
    }

    // Release the DC
    pDC->SelectObject( pOriginalFont );
    ReleaseDC( pDC );

    // Return the width
    return Width;
}

/////////////////////////////////////////////////////////////////////////////
// OnFind

void CListChannels::OnFind( void )
{
    CRecentList RecentList;

    RecentList.LoadRegistry( _T("Recent Find List") );

    // Create dialog
    CDlgFindLog Dialog( _T("Find in channels"), this );
    Dialog.SetRecentList( RecentList );

    // Execute dialog
    Dialog.DoModal();

    RecentList.SaveRegistry( _T("Recent Find List") );

    // Restore focus to the list
    SetFocus();
}

/////////////////////////////////////////////////////////////////////////////

void CListChannels::OnFindNext( void )
{
    // Is search string defined?
    if( m_FindString.GetLength() > 0 )
    {
        // Do the search for the next occurence of string
        int NextItem = FindNextItem( m_FocusItem, m_FindString, 1 );
        if( (NextItem != m_FocusItem) && (NextItem != -1) )
        {
            // Select the found item
            SetFocusItem( NextItem );
            EnsureCentered( m_FocusItem );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CListChannels::OnFindPrevious( void )
{
    // Is search string defined?
    if( m_FindString.GetLength() > 0 )
    {
        // Do the search for the next occurence of string
        int NextItem = FindNextItem( m_FocusItem, m_FindString, -1 );
        if( (NextItem != m_FocusItem) && (NextItem != -1) )
        {
            // Select the found item
            SetFocusItem( NextItem );
            EnsureCentered( m_FocusItem );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CListChannels::BuildSelectionSet( void )
{
    CxToolDoc* pDoc = m_pDocument;
    if( pDoc )
    {
        // Rebuild the selection set
        pDoc->GetFilteredChannels().GetSelectionSet( m_Selection );
    }
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CListChannels::OnMessageFind( WPARAM wParam, LPARAM lParam )
{
    // Get the string
    CString* pString = (CString*)wParam;
    ASSERT( pString );
    m_FindString = *pString;

    // Do the find
    OnFindNext();

    // Done
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CListChannels::OnMessageMarkAll( WPARAM wParam, LPARAM lParam )
{
    // Get the string
    CString* pString = (CString*)wParam;
    ASSERT( pString );
    m_FindString = *pString;

    // Is search string defined?
    if( m_FindString.GetLength() > 0 )
    {
        // Find the first occurence of string
        int FirstItem = FindNextItem( m_FocusItem, m_FindString, 1 );
        int NextItem = FirstItem;

        // Iterate until we loop
        do
        {
            // Mark the item
            OnSetMarked( NextItem, true );

            // Find the next item
            NextItem  = FindNextItem( NextItem, m_FindString, 1 );
        } while( NextItem != FirstItem );

        // Force a redraw
        RedrawWindow();
    }

    // Done
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
