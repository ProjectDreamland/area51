// ListLog.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "ListLog.h"
#include "DlgFindLog.h"
#include "xToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListLog

CListLog::CListLog()
{
    m_pDocument = NULL;
    m_ExtendedStyle = LVS_EX_GRIDLINES;
}

CListLog::~CListLog()
{
}

/////////////////////////////////////////////////////////////////////////////
// SetDocument
void CListLog::SetDocument( CxToolDoc* pDoc )
{
    m_pDocument = pDoc;
}

/////////////////////////////////////////////////////////////////////////////
// GetColumnText

const CString& CListLog::GetCellText( log_entry* pEntry, int iCol )
{
    static CString s;

    switch( iCol )
    {
    case 0:
        s.Format( _T("%d"), pEntry->GetSequence() );
        break;
    case 1:
        // Time in seconds
        s.Format( _T("%.5f"), (pEntry->GetTicks() - m_pDocument->GetBaselineTicks()) / m_pDocument->GetTicksPerSecond() );
        break;
    case 2:
        s = pEntry->GetChannel();
        break;
    case 3:
        s.Format( _T("%d"), pEntry->GetThreadID() );
        break;
    case 4:
        s = pEntry->GetMessage();
        break;
    case 5:
        s.Format( _T("%d"), pEntry->GetLine() );
        break;
    case 6:
        s = pEntry->GetFile();
        break;
    default:
        s.Empty();
        break;
    }

    return s;
}

/////////////////////////////////////////////////////////////////////////////

int CListLog::FindNextItem( int nItem, const CString& SearchText, int Delta )
{
    ASSERT( m_pDocument );

    CString SearchTextUpper = SearchText;
    SearchTextUpper.MakeUpper();

    // Get the log
    log_array&  Log = m_pDocument->GetFilteredLog();

    // Are there any log entries?
    if( GetItemCount() > 0 )
    {
        // Iterate over items
        int StartItem = nItem;
        nItem = (nItem + Delta) % GetItemCount();
        if( nItem < 0 )
            nItem += GetItemCount();
        while( nItem != StartItem )
        {
            ASSERT( nItem < Log.GetSize() );

            // Get the log entry text
            log_entry* pEntry = Log[nItem];
            CString ItemText = GetCellText( pEntry, 4 );
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

int CListLog::FindNextError( int nItem, int Delta )
{
    ASSERT( m_pDocument );

    // Get the log
    log_array&  Log = m_pDocument->GetFilteredLog();

    // Are there any log entries?
    if( GetItemCount() > 0 )
    {
        // Iterate over items
        int StartItem = nItem;
        nItem = (nItem + Delta) % GetItemCount();
        if( nItem < 0 )
            nItem += GetItemCount();
        while( nItem != StartItem )
        {
            ASSERT( nItem < Log.GetSize() );

            // Get the log entry
            log_entry* pEntry = Log[nItem];
            xtool::log_severity Severity = pEntry->GetSeverity();

            // Error?
            if( Severity != xtool::LOG_SEVERITY_MESSAGE )
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

int CListLog::FindNextChannel( int nItem, int Delta )
{
    ASSERT( m_pDocument );

    // Get the log
    log_array&  Log = m_pDocument->GetFilteredLog();

    // Are there any log entries?
    if( GetItemCount() > 0 )
    {
        // Iterate over items
        int StartItem = nItem;
        nItem = (nItem + Delta) % GetItemCount();
        if( nItem < 0 )
            nItem += GetItemCount();
        while( nItem != StartItem )
        {
            ASSERT( nItem < Log.GetSize() );

            // Get the log entry
            log_entry* pEntry = Log[nItem];
            int ChannelID = pEntry->GetChannelID();
            bool ChannelSelected = (m_pDocument->GetChannels()[ChannelID]->GetFlags( log_channel::flag_selected ) != 0);

            // Selected?
            if( ChannelSelected )
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

BEGIN_MESSAGE_MAP(CListLog, CVirtualListCtrl)
	//{{AFX_MSG_MAP(CListLog)
    ON_MESSAGE( AM_FIND    , OnMessageFind    )
    ON_MESSAGE( AM_MARK_ALL, OnMessageMarkAll )
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CListLog message handlers

/////////////////////////////////////////////////////////////////////////////
// OnGetSelected

bool CListLog::OnGetSelected( int iRow )
{
    if( m_pDocument == NULL )
        return FALSE;

    // Get the log entry
    log_array&  Log = m_pDocument->GetFilteredLog();
    log_entry*  pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Log.GetSize()) )
    {
        return FALSE;
    }

    // Get the log entry
    pEntry = Log[iRow];

    // Read the flag
    return( pEntry->GetFlags( log_entry::flag_selected ) != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// OnSetSelected

void CListLog::OnSetSelected( int iRow, bool State )
{
    if( m_pDocument == NULL )
        return;

    // Get the log entry
    log_array&  Log = m_pDocument->GetFilteredLog();
    log_entry*  pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Log.GetSize()) )
    {
        return;
    }

    // Get the log entry
    pEntry = Log[iRow];

    // Read the flag
    pEntry->SetFlags( State ? log_entry::flag_selected : 0, log_entry::flag_selected );
}

/////////////////////////////////////////////////////////////////////////////
// OnGetMarked

bool CListLog::OnGetMarked( int iRow )
{
    if( m_pDocument == NULL )
        return FALSE;

    // Get the log entry
    log_array&  Log = m_pDocument->GetFilteredLog();
    log_entry*  pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Log.GetSize()) )
    {
        return FALSE;
    }

    // Get the log entry
    pEntry = Log[iRow];

    // Read the flag
    return( pEntry->GetFlags( log_entry::flag_marked ) != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// OnSetMarked

void CListLog::OnSetMarked( int iRow, bool State )
{
    if( m_pDocument == NULL )
        return;

    // Get the log entry
    log_array&  Log = m_pDocument->GetFilteredLog();
    log_entry*  pEntry;

    // Exit if out of range
    if( (iRow < 0) || (iRow >= Log.GetSize()) )
    {
        return;
    }

    // Get the log entry
    pEntry = Log[iRow];

    // Read the flag
    pEntry->SetFlags( State ? log_entry::flag_marked : 0, log_entry::flag_marked );
}

/////////////////////////////////////////////////////////////////////////////
// OnDrawCell

BOOL CListLog::OnDrawCell( CDC* pDC, CRect& rCell, int iRow, int iCol )
{
    ASSERT( m_pDocument );

    // Get the log entry
    log_array&  Log = m_pDocument->GetFilteredLog();

    // Exit if out of range
    ASSERT( iRow < Log.GetSize() );

    // Get the log entry
    log_entry* pEntry = Log[iRow];

    // Make the text rectangle
    CRect rCellText = rCell;
    rCellText.DeflateRect( 2, 0, 2, 0 );

    int ChannelID = pEntry->GetChannelID();
    bool ChannelSelected = (m_pDocument->GetChannels()[ChannelID]->GetFlags( log_channel::flag_selected ) != 0);

    // Get the colors
    COLORREF ct;
    COLORREF cb;
    switch( pEntry->GetSeverity() )
    {
    case xtool::LOG_SEVERITY_MESSAGE:
        ct = RGB(   0,   0,   0 );
        cb = RGB( 255, 255, 255 );
        break;
    case xtool::LOG_SEVERITY_WARNING:
        ct = RGB(   0,   0,   0 );
        cb = RGB( 255, 255, 164 );
        break;
    case xtool::LOG_SEVERITY_ERROR:
        ct = RGB(   0,   0,   0 );
        cb = RGB( 255, 192, 192 );
        break;
    case xtool::LOG_SEVERITY_ASSERT:
        ct = RGB(   0,   0,   0 );
        cb = RGB( 255,  64,  64 );
        break;
    }

    // The first column always reflects the base color
    if( iCol != 0 )
    {
        // Change color for channel selected items
        if( ChannelSelected )
        {
            cb = RGB( 208,255,208 );
        }

        // Change color for marked items
        if( OnGetMarked( iRow ) )
        {
            cb = RGB( 208,208,255 );
        }
    }

    // Change color every other row
    if( iRow & 1 )
    {
        cb = DarkenColorPercent( cb, 2, 2, 0 );
    }

    // Modify for sorting and selection
    int d = 0;
    if( OnGetSelected( iRow ) )
        d = 15;
    else if( iCol == m_SortColumn )
        d = 4;
    if( !m_IsFocusWnd )
        d /= 2;
    cb = DarkenColorPercent( cb, d, d, d );

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

void CListLog::OnSort( void )
{
    if( m_SortColumn != -1 )
    {
        CxToolDoc* pDoc = m_pDocument;
        if( pDoc )
        {
            // Get the current focus item
            int iItemFocus = m_FocusItem;
            log_entry* pEntry = NULL;
            if( iItemFocus != -1 )
                pEntry = (log_entry*)pDoc->GetFilteredLog()[iItemFocus];

            // Sort the log
            pDoc->SortLog( m_SortColumn, m_SortAscending );

            // Rebuild the selection set from the newly sorted log
            BuildSelectionSet();

            // Find the item in the newly sorted list
            iItemFocus = pDoc->GetFilteredLog().Find( pEntry );
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

int CListLog::OnGetColumnFitWidth( int iCol )
{
    // Bail if no document
    if( m_pDocument == NULL )
        return -1;

    int Width = -1;

    // Get DC and set font into it
    CDC* pDC = GetDC();
    CFont* pOriginalFont = pDC->SelectObject( &m_Font );

    // Iterate over log entries finding the widest
    log_array&  Log = m_pDocument->GetFilteredLog();
    for( int i=0 ; i<Log.GetSize() ; i++ )
    {
        log_entry* pEntry = Log[i];
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

void CListLog::OnFind( void )
{
    CRecentList RecentList;

    RecentList.LoadRegistry( _T("Recent Find List") );

    // Create dialog
    CDlgFindLog Dialog( _T("Find in log"), this );
    Dialog.SetRecentList( RecentList );

    // Execute dialog
    Dialog.DoModal();

    RecentList.SaveRegistry( _T("Recent Find List") );

    // Restore focus to the list
    SetFocus();
}

/////////////////////////////////////////////////////////////////////////////

void CListLog::OnFindNext( void )
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

void CListLog::OnFindPrevious( void )
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

void CListLog::BuildSelectionSet( void )
{
    CxToolDoc* pDoc = m_pDocument;
    if( pDoc )
    {
        // Rebuild the selection set
        pDoc->GetFilteredLog().GetSelectionSet( m_Selection );
    }
}

/////////////////////////////////////////////////////////////////////////////
        
void CListLog::GotoNextError( s32 Direction )
{
    s32 nRow = FindNextError( m_FocusItem, Direction );
    if( nRow != -1 )
    {
        SetFocusItem( nRow );
        if( !IsVisible( nRow ) )
            EnsureCentered( nRow );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CListLog::GotoNextChannel( s32 Direction )
{
    s32 nRow = FindNextChannel( m_FocusItem, Direction );
    if( nRow != -1 )
    {
        SetFocusItem( nRow );
        if( !IsVisible( nRow ) )
            EnsureCentered( nRow );
    }
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CListLog::OnMessageFind( WPARAM wParam, LPARAM lParam )
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

LRESULT CListLog::OnMessageMarkAll( WPARAM wParam, LPARAM lParam )
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

        // Fail if the first item not found
        if( FirstItem == -1 )
            return 0;

        // Set NextItem for looping
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

void CListLog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    BOOL Control = ::GetKeyState( VK_CONTROL ) & 0x8000;
    BOOL Shift   = ::GetKeyState( VK_SHIFT   ) & 0x8000;

    // Check key pressed
    switch( nChar )
    {
    case VK_F4:
        GotoNextError( Shift ? -1 : 1 );
        break;
    case VK_F5:
        GotoNextChannel( Shift ? -1 : 1 );
        break;
    }

	CVirtualListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////

void CListLog::OnCopy( void )
{
    if( OpenClipboard() )
    {
        if( EmptyClipboard() )
        {
            CSharedFile sf( GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, 1024*1024 );


            // Get text to put on clipboard
            CString		    text;

            // Get the log
            log_array&  Log = m_pDocument->GetFilteredLog();

            // Iterate over selection
            s32 iRow;
            s32 Count = 0;
            if( m_Selection.BeginIterate( iRow ) )
            {
                do{
                    // Get the log entry
                    log_memory* pEntry = (log_memory*)Log[iRow];

                    // Get the text for this row
                    for( s32 iCol=0 ; iCol<m_Columns.GetCount() ; iCol++ )
                    {
                        // Add column seperator
                        if( iCol != 0 )
                            text += ",";

                        text += GetCellText( pEntry, iCol );
                    }
                    text += "\r\n";

                    // Write the data to the clipboard 'file'
                    sf.Write( text, text.GetLength() );
                    text.Empty();
                }while( m_Selection.Iterate( iRow ) );
            }

            // Set the data to the clipboard
            SetClipboardData( CF_TEXT, sf.Detach() );

            // Close it
            CloseClipboard();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
