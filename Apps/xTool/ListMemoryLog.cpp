// ListMemoryLog.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "ListMemoryLog.h"
#include "DlgFindLog.h"
#include "xToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListMemoryLog

CListMemoryLog::CListMemoryLog()
{
    m_pDocument = NULL;
    m_ExtendedStyle = LVS_EX_GRIDLINES;
}

CListMemoryLog::~CListMemoryLog()
{
}

/////////////////////////////////////////////////////////////////////////////
// SetDocument
void CListMemoryLog::SetDocument( CxToolDoc* pDoc )
{
    m_pDocument = pDoc;
}

/////////////////////////////////////////////////////////////////////////////
// GetColumnText

const CString& CListMemoryLog::GetCellText( log_memory* pEntry, int iCol )
{
    static CString s;

    s32 Operation = pEntry->GetOperation();

    if( Operation == xtool::LOG_MEMORY_MARK )
    {
        if( iCol > 3 )
        {
            s.Empty();
            return s;
        }
    }

    switch( iCol )
    {
    case 0:
        s.Format( _T("%d"), pEntry->GetSequence() );
        break;
    case 1:
        s.Format( _T("%.5f"), (pEntry->GetTicks() - m_pDocument->GetBaselineTicks()) / m_pDocument->GetTicksPerSecond() );
        break;
    case 2:
        s.Format( _T("%d"), pEntry->GetThreadID() );
        break;
    case 3:
        switch( Operation )
        {
        case xtool::LOG_MEMORY_MALLOC:
            s = _T("malloc");
            break;
        case xtool::LOG_MEMORY_REALLOC:
            s.Format( _T("realloc 0x%08X"), pEntry->GetOldAddress() );
            break;
        case xtool::LOG_MEMORY_FREE:
            s = _T("free");
            break;
        case xtool::LOG_MEMORY_MARK:
            s = pEntry->GetFile();
            break;
        default:
            ASSERT( 0 );
            break;
        }
        break;
    case 4:
        s.Format( _T("0x%08X"), pEntry->GetAddress() ); //ThreadID() );
        break;
    case 5:
        PrettyInt( s, pEntry->GetSize() );
        //s.Format( _T("0x%08X"), pEntry->GetSize() );
        break;
    case 6:
        PrettyInt( s, pEntry->GetCurrentBytes() );
        //s.Format( _T("0x%08X"), pEntry->GetCurrentBytes() );
        break;
    case 7:
        s.Format( _T("%d"), pEntry->GetLine() );
        break;
    case 8:
        s = pEntry->GetFile();
        break;
    case 9:
        {
            s32 Index = pEntry->GetCallStackIndex();
            u32* pCallStack = m_pDocument->GetCallStackEntry( Index );
            s.Empty();
            while( *pCallStack != 0 )
            {
                CString t;
                const char* pSymbol = m_pDocument->AddressToSymbol( *pCallStack );

                if( pSymbol )
                {
                    t = pSymbol;
                    int Index = t.Find( "(" );
                    if( Index != -1 )
                        t = t.Left( Index );
                }
                else
                {
                    t.Format( "%08x", *pCallStack );
                }

                if( !s.IsEmpty() )
                    s += " - ";

                s += t;

                pCallStack++;
            }
        }
        break;
    }

    return s;
}

/////////////////////////////////////////////////////////////////////////////

int CListMemoryLog::FindNextItem( int nItem, const CString& SearchText, int Delta )
{
    ASSERT( m_pDocument );

    CString SearchTextUpper = SearchText;
    SearchTextUpper.MakeUpper();

    // Get the log
    log_array&  Log = m_pDocument->GetFilteredMemLog();

    xstring ItemText;

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
            log_memory* pEntry = (log_memory*)Log[nItem];

            ItemText.Clear();
            for( s32 iCol=0; iCol< m_Columns.GetCount(); iCol++ )
            {
                ItemText += GetCellText( pEntry, iCol );
                ItemText += " ";
            }
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

int CListMemoryLog::FindNextMemMark( int nItem, int Delta )
{
    ASSERT( m_pDocument );

    // Get the log
    log_array&  Log = m_pDocument->GetFilteredMemLog();

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
            log_memory* pEntry = (log_memory*)Log[nItem];

            // Active?
            if( pEntry->GetOperation() == xtool::LOG_MEMORY_MARK )
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

int CListMemoryLog::FindNextActiveAllocation( int nItem, int Delta )
{
    ASSERT( m_pDocument );

    // Get the log
    log_array&  Log = m_pDocument->GetFilteredMemLog();

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
            log_memory* pEntry = (log_memory*)Log[nItem];

            // Active?
            if( pEntry->GetFlags() & log_entry::flag_memory_active )
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

BEGIN_MESSAGE_MAP(CListMemoryLog, CVirtualListCtrl)
	//{{AFX_MSG_MAP(CListMemoryLog)
	ON_WM_KEYDOWN()
    ON_MESSAGE( AM_FIND    , OnMessageFind    )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CListMemoryLog message handlers

/////////////////////////////////////////////////////////////////////////////
// OnGetSelected

bool CListMemoryLog::OnGetSelected( int iRow )
{
    if( m_pDocument == NULL )
        return FALSE;

    // Get the log entry
    log_array&  Log = m_pDocument->GetFilteredMemLog();
    log_entry*  pEntry;

    // Exit if out of range
    if( iRow >= Log.GetSize() )
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

void CListMemoryLog::OnSetSelected( int iRow, bool State )
{
    if( m_pDocument == NULL )
        return;

    // Get the log entry
    log_array&  Log = m_pDocument->GetFilteredMemLog();
    log_entry*  pEntry;

    // Exit if out of range
    if( iRow >= Log.GetSize() )
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

bool CListMemoryLog::OnGetMarked( int iRow )
{
    if( m_pDocument == NULL )
        return FALSE;

    // Get the log entry
    log_array&  Log = m_pDocument->GetFilteredMemLog();
    log_entry*  pEntry;

    // Exit if out of range
    if( iRow >= Log.GetSize() )
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

void CListMemoryLog::OnSetMarked( int iRow, bool State )
{
    if( m_pDocument == NULL )
        return;

    // Get the log entry
    log_array&  Log = m_pDocument->GetFilteredMemLog();
    log_entry*  pEntry;

    // Exit if out of range
    if( iRow >= Log.GetSize() )
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

BOOL CListMemoryLog::OnDrawCell( CDC* pDC, CRect& rCell, int iRow, int iCol )
{
    ASSERT( m_pDocument );

    // Get the log
    log_array&  Log = m_pDocument->GetFilteredMemLog();

    // Exit if out of range
    if( iRow >= Log.GetSize() )
    {
        return FALSE;
    }

    // Get the log entry
    log_memory* pEntry = (log_memory*)Log[iRow];

    // Make the text rectangle
    CRect rCellText = rCell;
    rCellText.DeflateRect( 2, 0, 2, 0 );

    // Get the colors
    COLORREF ct;
    COLORREF cb;
    switch( pEntry->GetOperation() )
    {
    case xtool::LOG_MEMORY_MALLOC:
        ct = RGB( 192,   0,   0 );
        cb = RGB( 255, 255, 255 );
        break;
    case xtool::LOG_MEMORY_FREE:
        ct = RGB(   0, 192,   0 );
        cb = RGB( 255, 255, 255 );
        break;
    case xtool::LOG_MEMORY_REALLOC:
        ct = RGB( 192, 192,   0 );
        cb = RGB( 255, 164, 164 );
        break;
    case xtool::LOG_MEMORY_MARK:
        ct = RGB(   0, 192,   0 );
        cb = RGB( 255, 192, 255 );
        break;
    }

    // Color entries that are active
    if( pEntry->GetFlags( log_entry::flag_memory_active ) )
    {
        cb = RGB( 224, 255, 224 );
    }

    // Color entries that have an error
    if( pEntry->GetFlags( log_entry::flag_system_error ) )
    {
        cb = RGB( 255, 192, 192 );
    }

    // The first column always reflects the base color
    if( iCol != 0 )
    {
        // Change color for marked items
        if( OnGetMarked( iRow ) )
        {
            cb = RGB( 192,192,255 );
        }
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

void CListMemoryLog::OnSort( void )
{
    if( m_SortColumn != -1 )
    {
        CxToolDoc* pDoc = m_pDocument;
        if( pDoc )
        {
            // Get the current focus item
            int iItemFocus = m_FocusItem;
            log_entry* pEntry = (log_entry*)pDoc->GetFilteredMemLog()[iItemFocus];

            // Sort the memory log
            pDoc->SortMemLog( m_SortColumn, m_SortAscending );

            // Rebuild the selection set from the newly sorted log
            BuildSelectionSet();

            // Find the item in the newly sorted list
            iItemFocus = pDoc->GetFilteredMemLog().Find( pEntry );
            if( iItemFocus != -1 )
            {
                if( iItemFocus > GetItemCount() )
                    iItemFocus = GetItemCount();
                if( iItemFocus < 0 )
                    iItemFocus = 0;

                // Give it focus
                m_FocusItem = iItemFocus;
                SetSelected( iItemFocus, true );

                // Ensure the new item is visible
                EnsureVisible( iItemFocus );
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnGetColumnFitWidth

int CListMemoryLog::OnGetColumnFitWidth( int iCol )
{
    // Bail if no document
    if( m_pDocument == NULL )
        return -1;

    int Width = -1;

    // Get DC and set font into it
    CDC* pDC = GetDC();
    CFont* pOriginalFont = pDC->SelectObject( &m_Font );

    // Iterate over log entries finding the widest
    log_array&  Log = m_pDocument->GetFilteredMemLog();
    for( int i=0 ; i<Log.GetSize() ; i++ )
    {
        log_memory* pEntry = (log_memory*)Log[i];
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

void CListMemoryLog::OnFind( void )
{
    CRecentList RecentList;

    RecentList.LoadRegistry( _T("Recent Find List") );

    // Create dialog
    CDlgFindLog Dialog( _T("Find in memory log"), this );
    Dialog.SetRecentList( RecentList );

    // Execute dialog
    Dialog.DoModal();

    RecentList.SaveRegistry( _T("Recent Find List") );

    // Restore focus to the list
    SetFocus();
}

/////////////////////////////////////////////////////////////////////////////

void CListMemoryLog::OnFindNext( void )
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

void CListMemoryLog::OnFindPrevious( void )
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
// OnFocusChanged

void CListMemoryLog::OnFocusChanged( void )
{
    // Bail if no document
    if( m_pDocument == NULL )
        return;

    // Get the log and entry
    log_array&  Log     = m_pDocument->GetFilteredMemLog();
    log_memory* pEntry  = NULL;

    // Read log entry
    if( m_FocusItem != -1 )
        pEntry = (log_memory*)Log[m_FocusItem];

    // Update all views to reflect this new memory block selection
    m_pDocument->UpdateAllViews( NULL, CxToolDoc::HINT_SELECT_MEMBLOCK, (CObject*)pEntry );
}

/////////////////////////////////////////////////////////////////////////////

void CListMemoryLog::BuildSelectionSet( void )
{
    CxToolDoc* pDoc = m_pDocument;
    if( pDoc )
    {
        // Rebuild the selection set
        pDoc->GetFilteredMemLog().GetSelectionSet( m_Selection );
    }
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CListMemoryLog::OnMessageFind( WPARAM wParam, LPARAM lParam )
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

void CListMemoryLog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    BOOL Control = ::GetKeyState( VK_CONTROL ) & 0x8000;
    BOOL Shift   = ::GetKeyState( VK_SHIFT   ) & 0x8000;

    // Bail if no document
    if( m_pDocument == NULL )
        return;

    // Get the log and entry
    log_array&  Log     = m_pDocument->GetFilteredMemLog();
    log_memory* pEntry  = NULL;

    // Read current focus item
    int nRow = m_FocusItem;

    // Check key pressed
    switch( nChar )
    {
    case 'T':
        if( m_FocusItem != -1 )
            pEntry = (log_memory*)Log[m_FocusItem];
        break;
    case 0xbc: // '<'
        if( m_FocusItem > 0 )
        {
            SetFocusItem( m_FocusItem-1 );
            if( IsVisible( m_FocusItem ) )
                RedrawWindow();
            else
                EnsureVisible( m_FocusItem );
            pEntry = (log_memory*)Log[m_FocusItem];
        }
        break;
    case 0xbe: // '>'
        if( m_FocusItem < Log.GetSize() )
        {
            SetFocusItem( m_FocusItem+1 );
            if( IsVisible( m_FocusItem ) )
                RedrawWindow();
            else
                EnsureVisible( m_FocusItem );
            pEntry = (log_memory*)Log[m_FocusItem];
        }
        break;
    case VK_F4:
        nRow = FindNextMemMark( nRow, Shift ? -1 : 1 );
        if( nRow != -1 )
        {
            SetFocusItem( nRow );
            if( !IsVisible( nRow ) )
                EnsureCentered( nRow );
        }
        break;
    case VK_F5:
        nRow = FindNextActiveAllocation( nRow, Shift ? -1 : 1 );
        if( nRow != -1 )
        {
            SetFocusItem( nRow );
            if( !IsVisible( nRow ) )
                EnsureCentered( nRow );
        }
    }

    // Generate the memory state at the time of the selected log entry
    if( pEntry )
    {
        m_pDocument->CreateHeapState( pEntry->GetTicks() );
        m_pDocument->UpdateAllViews( NULL, CxToolDoc::HINT_REDRAW_MEMORY_GRAPH );
    }

	CVirtualListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////

void CListMemoryLog::OnCopy( void )
{
    if( OpenClipboard() )
    {
        if( EmptyClipboard() )
        {
            CSharedFile sf( GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, 1024*1024 );


            // Get text to put on clipboard
            CString		    text;

            // Get the log
            log_array&  Log = m_pDocument->GetFilteredMemLog();

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
