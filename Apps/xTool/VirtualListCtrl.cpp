// VirtualListCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "Header.h"
#include "VirtualListCtrl.h"

#if (_WIN32_WINNT < 0x501)
#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMemDC

class CMemDC : public CDC
{
private:       
    CBitmap    m_bitmap;        // Offscreen bitmap
    CBitmap*   m_oldBitmap;     // bitmap originally found in CMemDC
    CDC*       m_pDC;           // Saves CDC passed in constructor
    CRect      m_rect;          // Rectangle of drawing area.
    BOOL       m_bMemDC;        // TRUE if CDC really is a Memory DC.
public:
    
    CMemDC(CDC* pDC, const CRect* pRect = NULL) : CDC()
    {
        ASSERT(pDC != NULL); 
 
        // Some initialization
        m_pDC = pDC;
        m_oldBitmap = NULL;
        m_bMemDC = !pDC->IsPrinting();
 
        // Get the rectangle to draw
        if( pRect == NULL )
        {
             pDC->GetClipBox(&m_rect);
        }
        else
        {
             m_rect = *pRect;
        }
 
        if( m_bMemDC )
        {
             // Create a Memory DC
             CreateCompatibleDC(pDC);
             pDC->LPtoDP(&m_rect);
 
             m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), 
                                                  m_rect.Height());
             m_oldBitmap = SelectObject(&m_bitmap);
 
             SetMapMode(pDC->GetMapMode());
 
             SetWindowExt(pDC->GetWindowExt());
             SetViewportExt(pDC->GetViewportExt());
 
             pDC->DPtoLP(&m_rect);
             SetWindowOrg(m_rect.left, m_rect.top);
        }
        else
        {
             // Make a copy of the relevent parts of the current 
             // DC for printing
             m_bPrinting = pDC->m_bPrinting;
             m_hDC       = pDC->m_hDC;
             m_hAttribDC = pDC->m_hAttribDC;
        }
 
        // Fill background 
        FillSolidRect(m_rect, pDC->GetBkColor());
    }
    
    ~CMemDC()      
    {          
        if (m_bMemDC)
        {
             // Copy the offscreen bitmap onto the screen.
             m_pDC->BitBlt(m_rect.left, m_rect.top, 
                           m_rect.Width(),  m_rect.Height(),
                  this, m_rect.left, m_rect.top, SRCCOPY);            
             
             //Swap back the original bitmap.
             SelectObject(m_oldBitmap);        
        }
        else
        {
             // All we need to do is replace the DC with an illegal
             // value, this keeps us from accidentally deleting the 
             // handles associated with the CDC that was passed to 
             // the constructor.              
             m_hDC = m_hAttribDC = NULL;
        }       
    }
    
    // Allow usage as a pointer    
    CMemDC* operator->() 
    {
        return this;
    }       
 
    // Allow usage as a pointer    
    operator CMemDC*() 
    {
        return this;
    }
};

/////////////////////////////////////////////////////////////////////////////
// Extended style access macros

#define HAS_GRIDLINES()     (m_ExtendedStyle & LVS_EX_GRIDLINES )
#define HAS_CHECKBOXES()    (m_ExtendedStyle & LVS_EX_CHECKBOXES)

#define CHECKBOX_SPACE      2           // Visual pading for checkboxes

/////////////////////////////////////////////////////////////////////////////
// CVirtualListCtrl

CVirtualListCtrl::CVirtualListCtrl()
{
    m_LineMode          = TRUE; //FALSE;
    m_ExtendedStyle     = 0;

    m_IsFocusWnd        = FALSE;
    m_TrackingLeave     = FALSE;
    m_HotCheck          = -1;

    m_MouseCaptured     = FALSE;

    m_HorzSBVisible     = FALSE;
    m_VertSBVisible     = FALSE;
    m_ScrollX           = 0;
    m_ScrollY           = 0;

    m_SortColumn        = -1;
    m_SortAscending     = FALSE;

    m_CellHeight        = 1;
    m_RowHeight         = 1;
    m_nItems            = 0;
    m_FocusItem         = 0;
    m_SelectionRoot     = 0;

    m_hTheme            = NULL;

    m_ColorSeperator    = ::GetSysColor( COLOR_3DFACE );
}

/////////////////////////////////////////////////////////////////////////////

CVirtualListCtrl::~CVirtualListCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// Create

BOOL CVirtualListCtrl::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
    dwStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    // Call base class CWnd create
    BOOL Success = CWnd::Create( NULL, NULL, dwStyle, rect, pParentWnd, nID, NULL );
    ASSERT( Success );

    return Success;
}

/////////////////////////////////////////////////////////////////////////////
// SetExtendedStyle

DWORD CVirtualListCtrl::SetExtendedStyle( DWORD dwNewStyle )
{
    DWORD OldStyle = m_ExtendedStyle;

    // Set new style and force updates
    m_ExtendedStyle = dwNewStyle;
    UpdateScrollbars();
    RedrawWindow();

    return OldStyle;
}

/////////////////////////////////////////////////////////////////////////////
// GetFont

CFont* CVirtualListCtrl::GetFont( void )
{
    return &m_Font;
}

/////////////////////////////////////////////////////////////////////////////
// SetFont

void CVirtualListCtrl::SetFont( CFont* pFont, BOOL bRedraw )
{
    if( pFont )
    {
        // Get LogFont
        LOGFONT LogFont;
        pFont->GetLogFont( &LogFont );

        // Delete current font
        m_Font.DeleteObject();

        // Create new font
        m_Font.CreateFontIndirect( &LogFont );

        // Measure the new font
        CDC* pDC = GetDC();
        CFont* pOriginalFont = pDC->SelectObject( & m_Font );
        TEXTMETRIC tm;
        pDC->GetTextMetrics( &tm );
        //pDC->SelectObject( pOriginalFont );
        ReleaseDC( pDC );
        m_CellHeight = tm.tmHeight;
        // TODO: Modify m_CellHeight to account for checkbox
        m_RowHeight  = m_CellHeight + 1;

        // Update the scrollbars
        UpdateScrollbars();

        // Redraw window
        if( bRedraw )
        {
            RedrawWindow();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// GetHeaderCtrl

CHeaderCtrl* CVirtualListCtrl::GetHeaderCtrl( void )
{
    return &m_Header;
}

/////////////////////////////////////////////////////////////////////////////
// InsertColumn

int CVirtualListCtrl::InsertColumn( int nCol, const LVCOLUMN* pColumn )
{
    // Clamp index
    int Index = nCol;
    if( Index > m_Columns.GetSize() )
        Index = m_Columns.GetSize();
    if( Index < 0 )
        Index = 0;

    // Get text alignment for column
    DWORD Align;
    switch( pColumn->fmt & HDF_JUSTIFYMASK )
    {
    case HDF_CENTER:
        Align = DT_CENTER;
        break;
    case HDF_RIGHT:
        Align = DT_RIGHT;
        break;
    default:
        Align = DT_LEFT;
        break;
    }

    // Create a column_info and add into the columns array
    column_info c;
    c.m_Visible     = TRUE;
    c.m_Name        = pColumn->pszText;
    c.m_Format      = pColumn->fmt;
    c.m_TextAlign   = Align;
    c.m_Width       = pColumn->cx;
    c.m_nSubItem    = pColumn->iSubItem;
    c.m_Index       = Index;
    m_Columns.InsertAt( Index, c );

    // Create the column in the header control
    HDITEM Item;
    Item.mask    = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
    Item.cxy     = c.m_Width;
    Item.fmt     = HDF_STRING | ((c.m_Format & LVCFMT_CENTER) ? HDF_CENTER : 0) | ((c.m_Format & LVCFMT_RIGHT) ? HDF_RIGHT : 0) ;
    Item.pszText = c.m_Name.GetBuffer(0);
    m_Header.InsertItem( Index, &Item );

    // Return the column index
    return Index;
}

/////////////////////////////////////////////////////////////////////////////
// InsertColumn

int CVirtualListCtrl::InsertColumn( int nCol, LPCTSTR lpszColumnHeading, int nFormat, int nWidth, int nSubItem )
{
    // Create a LVCOLUMN structure and call the other InsertColumn
    LVCOLUMN    c;
    c.mask      = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    c.fmt       = nFormat;
    c.cx        = nWidth;
    c.pszText   = (char*)lpszColumnHeading;
    c.iSubItem  = nSubItem;
    return InsertColumn( nCol, &c );
}

/////////////////////////////////////////////////////////////////////////////
// DeleteColumn

BOOL CVirtualListCtrl::DeleteColumn( int nCol )
{
    // TODO:
    ASSERT( 0 );

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// GetItemCount

int CVirtualListCtrl::GetItemCount( void )
{
    return m_nItems;
}

/////////////////////////////////////////////////////////////////////////////
// SetItemCount

void CVirtualListCtrl::SetItemCount( int nItems )
{
    int i;

    // Keep old focus item
    int OldFocusItem = m_FocusItem;

    // Update selected count
    if( nItems > m_nItems )
    {
        // TODO: Make this more intelligent, add groups not single items
        for( i=m_nItems ; i<nItems ; i++ )
        {
            if( OnGetSelected( i ) )
            {
                m_Selection.Select( i, i );
            }
        }
    }
    else
    {
        m_Selection.Deselect( nItems+1, m_nItems );
    }

    // Set the new number of items
    m_nItems = nItems;

    // Limit variables with this new range
    if( m_FocusItem >= nItems )
        m_FocusItem = nItems-1;
    if( m_SelectionRoot >= nItems )
        m_SelectionRoot = nItems-1;

    // Redraw
    UpdateScrollbars();
    Invalidate();

    // Broadcast any change in focus
    if( m_FocusItem != OldFocusItem )
        OnFocusChanged();
}

/////////////////////////////////////////////////////////////////////////////
// SetItemCountEx

void CVirtualListCtrl::SetItemCountEx( int nItems, int Flags )
{
    SetItemCount( nItems );
}

/////////////////////////////////////////////////////////////////////////////
// IsValidItem

bool CVirtualListCtrl::IsValidItem( int nItems )
{
    return( (nItems >= 0) && (nItems < GetItemCount()) );
}

/////////////////////////////////////////////////////////////////////////////
// IsVisible

bool CVirtualListCtrl::IsVisible( int nItem )
{
    if( IsValidItem( nItem ) )
    {
        CRect rCell;
        GetCellRect( nItem, 0, &rCell );

        return( (rCell.top >= 0) && (rCell.bottom <= m_UserRect.Height()) );
    }
    else
    {
        return FALSE;
    }
}

/////////////////////////////////////////////////////////////////////////////
// EnsureVisible

BOOL CVirtualListCtrl::EnsureVisible( int nItem, BOOL PartialOk )
{
    CRect rCell;
    GetCellRect( nItem, 0, &rCell );

    SCROLLINFO si;
    m_VertScrollbar.GetScrollInfo( &si );

    // If nItem is off the top
    if( rCell.top < 0 )
    {
        if( m_LineMode )
            SetVertScroll( nItem );
        else
            SetVertScroll( nItem * m_RowHeight );
    }
    // If nItem if off the bottom
    else if( rCell.bottom >= m_UserRect.Height() )
    {
        if( m_LineMode )
        {
            int nVisibleRows = m_LineMode ? si.nPage : (si.nPage / m_RowHeight);
            if( nVisibleRows < 1 )
                nVisibleRows = 1;

            nItem -= (nVisibleRows-1);
            if(nItem < 0 )
                nItem = 0;

            SetVertScroll( nItem );
        }
        else
        {
            SetVertScroll( nItem * m_RowHeight - (m_UserRect.Height() - m_RowHeight) );
        }
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// EnsureCentered

void CVirtualListCtrl::EnsureCentered( int nItem )
{
    // Where is the cell we want centered
    CRect rCell;
    GetCellRect( nItem, 0, &rCell );

    SCROLLINFO si;
    m_VertScrollbar.GetScrollInfo( &si );

    // Line mode or pixel mode?
    if( m_LineMode )
    {
        int nVisibleRows = m_LineMode ? si.nPage : (si.nPage / m_RowHeight);
        if( nVisibleRows < 1 )
            nVisibleRows = 1;

        nItem -= (nVisibleRows/2);
        if(nItem < 0 )
            nItem = 0;

        SetVertScroll( nItem );
    }
    else
    {
        SetVertScroll( nItem * m_RowHeight - (m_UserRect.Height() / 2) );
    }
}

/////////////////////////////////////////////////////////////////////////////
// IsLastItemInFocus

BOOL CVirtualListCtrl::IsLastItemInFocus( void )
{
    return( (GetItemCount() == 0) || (m_FocusItem == GetItemCount()-1) );
}

/////////////////////////////////////////////////////////////////////////////
// SetFocusItem

void CVirtualListCtrl::SetFocusItem( int nItem )
{
    // Exit if the same
    if( nItem == m_FocusItem )
        return;

    // Range validate
    if( nItem >= GetItemCount() )
        nItem = GetItemCount()-1;
    if( nItem < 0 )
        nItem = 0;

    // Clear old selection and set new selection
    SetSelected( m_FocusItem, false );
    m_FocusItem = nItem;
    SetSelected( m_FocusItem, true );

    // Set the selection root
    m_SelectionRoot = m_FocusItem;

    // Redraw
    Invalidate();

    // Broadcast any change in focus
    OnFocusChanged();
}

/////////////////////////////////////////////////////////////////////////////
// GetFocusItem

int CVirtualListCtrl::GetFocusItem( void )
{
    return m_FocusItem;
}

/////////////////////////////////////////////////////////////////////////////
// OnFocusChanged

void CVirtualListCtrl::OnFocusChanged( void )
{
    // Send a notification
    CWnd* pParent = GetParent();
    if( pParent )
    {
        NMHDR nm;
        nm.hwndFrom = GetSafeHwnd();
        nm.idFrom   = GetDlgCtrlID();
        nm.code     = VLC_FOCUS_CHANGED;
        pParent->SendNotifyMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nm );
    }
}

/////////////////////////////////////////////////////////////////////////////
// DarkenColor

COLORREF CVirtualListCtrl::DarkenColor( COLORREF Color, int r, int g, int b )
{
    int r1 = GetRValue(Color) - r;
    int g1 = GetGValue(Color) - g;
    int b1 = GetBValue(Color) - b;

    if( r1 < 0 ) r1 = 0;
    if( g1 < 0 ) g1 = 0;
    if( b1 < 0 ) b1 = 0;

    return RGB(r1,g1,b1);
}

/////////////////////////////////////////////////////////////////////////////
// DarkenColorPercent

COLORREF CVirtualListCtrl::DarkenColorPercent( COLORREF Color, int r, int g, int b )
{
    int r1 = GetRValue(Color);
    int g1 = GetGValue(Color);
    int b1 = GetBValue(Color);

    r1 -= (r1 * r) / 100;
    g1 -= (g1 * g) / 100;
    b1 -= (b1 * b) / 100;

    if( r1 < 0 ) r1 = 0;
    if( g1 < 0 ) g1 = 0;
    if( b1 < 0 ) b1 = 0;

    return RGB(r1,g1,b1);
}

/////////////////////////////////////////////////////////////////////////////
// GetCellRect

BOOL CVirtualListCtrl::GetCellRect( int iRow, int iCol, CRect* pRect )
{
    // Exit if nowhere to put the result
    if( pRect == NULL )
        return FALSE;

    // Exit if column or row invalid
    if( (iCol < 0) || (iCol > m_Columns.GetSize()) || !IsValidItem(iRow) )
    {
        pRect->SetRectEmpty();
        return FALSE;
    }

    // Exit if column hidden
    if( !m_Columns[iCol].m_Visible )
    {
        pRect->SetRectEmpty();
        return FALSE;
    }

    // Get rect for the column from the header
    CRect rColumn;
    m_Header.GetItemRect( iCol, &rColumn );

    // Setup the rect appropriately
    int x1 = rColumn.left - m_ScrollX;
    int y1 = iRow * m_RowHeight - m_ScrollY;
    int x2 = rColumn.right - 1 - m_ScrollX;
    int y2 = y1 + m_CellHeight;
    pRect->SetRect( x1, y1, x2, y2 );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// PointToRow

int CVirtualListCtrl::PointToRow( const CPoint& Point, BOOL Clip )
{
    int y = Point.y + m_ScrollY - m_HeaderRect.bottom;
    int Row = y / m_RowHeight;
    
    if( Row < 0 )
    {
        if( Clip )
            Row = 0;
        else
            Row = -1;
    }
    
    if( Row >= GetItemCount() )
    {
        if( Clip )
            Row = GetItemCount()-1;
        else
            Row = -1;
    }

    return Row;
}

/////////////////////////////////////////////////////////////////////////////
// PointToCol

int CVirtualListCtrl::PointToCol( const CPoint& Point )
{
    // TODO:
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// GetTotalColumnWidth

int CVirtualListCtrl::GetHeaderWidth( void )
{
    int     HeaderWidth = 0;
    int     nColumns = m_Header.GetItemCount();
    CRect   rColumn;
    for( int iCol=0 ; iCol<nColumns ; iCol++ )
    {
        m_Header.GetItemRect( iCol, &rColumn );
        HeaderWidth += rColumn.Width();
    }

    return HeaderWidth;
}

/////////////////////////////////////////////////////////////////////////////
// PositionHeader

void CVirtualListCtrl::PositionHeader( void )
{
    CRect HeaderPos = m_HeaderRect;
    HeaderPos.left -= m_ScrollX;
    m_Header.MoveWindow( &HeaderPos );
}

/////////////////////////////////////////////////////////////////////////////
// UpdateScrollbars

void CVirtualListCtrl::UpdateScrollbars( int cx, int cy )
{
    BOOL Iterate;

    // Get width of all columns
    int HeaderWidth = GetHeaderWidth();

    // Get width and height of scrollbars
    int SBh = ::GetSystemMetrics( SM_CYHSCROLL );
    int SBw = ::GetSystemMetrics( SM_CXVSCROLL );

    // Need to iterate because changing state of 1 scrollbar effects the need for the other
    CRect rClient;
    do
    {
        Iterate = FALSE;

        // Get client rect
        rClient.SetRect( 0, 0, cx, cy );
        rClient.top = m_HeaderRect.bottom;

        // Modify rect based on scrollbars
        if( m_VertSBVisible )
            rClient.right -= SBw;
        if( m_HorzSBVisible )
            rClient.bottom -= SBh;

        if( (GetItemCount() * m_RowHeight) > rClient.Height() )
        {
            if( !m_VertSBVisible )
            {
                Iterate = TRUE;
                m_VertSBVisible = TRUE;
            }
        }
        else
        {
            if( m_VertSBVisible )
            {
                Iterate = TRUE;
                m_VertSBVisible = FALSE;
            }
        }

        if( HeaderWidth > rClient.Width() )
        {
            if( !m_HorzSBVisible )
            {
                Iterate = TRUE;
                m_HorzSBVisible = TRUE;
            }
        }
        else
        {
            if( m_HorzSBVisible )
            {
                Iterate = TRUE;
                m_HorzSBVisible = FALSE;
            }
        }

    } while( Iterate );

    // Save the user rect
    m_UserRect = rClient;

    // Are both scrollbars visible?
    BOOL BothVisible = m_HorzSBVisible && m_VertSBVisible;

    // Calculate rectangles to position scrollbars
    CRect HorzSBRect ( 0, cy-SBh, BothVisible ? cx-SBw : cx, cy );
    CRect VertSBRect ( cx-SBw, 0, cx, BothVisible ? cy-SBh : cy );
    CRect SizeBoxRect( cx-SBw, cy-SBh, cx, cy );

    // Move scrollbars & set parameters if they're visible
    if( m_HorzSBVisible && m_HorzScrollbar.GetSafeHwnd() )
    {
        m_HorzScrollbar.MoveWindow( &HorzSBRect );
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask  = SIF_PAGE | SIF_RANGE;
        si.nMin   = 0;
        si.nMax   = HeaderWidth;
        si.nPage  = rClient.Width();
        m_HorzScrollbar.SetScrollInfo( &si );

        // Keep track of scroll position
        m_HorzScrollbar.GetScrollInfo( &si, SIF_POS );
        if( m_ScrollX != si.nPos )
        {
            m_ScrollX = si.nPos;
            Invalidate();

            // Move the header
            PositionHeader();
        }
    }
    if( m_VertSBVisible && m_VertScrollbar.GetSafeHwnd() )
    {
        m_VertScrollbar.MoveWindow( &VertSBRect );
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask  = SIF_PAGE | SIF_RANGE;
        si.nMin   = 0;
        si.nMax   = m_LineMode ? (GetItemCount() - 1) : (GetItemCount() * m_RowHeight - 1);
        si.nPage  = m_LineMode ? (rClient.Height() / m_RowHeight) : rClient.Height();
        m_VertScrollbar.SetScrollInfo( &si );

        // Keep track of scroll position
        m_VertScrollbar.GetScrollInfo( &si, SIF_POS );
        int Pos = m_LineMode ? (si.nPos*m_RowHeight) : si.nPos;
        if( m_ScrollY != Pos )
        {
            m_ScrollY = Pos;
            Invalidate();
        }
    }
    if( BothVisible && m_SizeBox.GetSafeHwnd() )
    {
        m_SizeBox.MoveWindow( &SizeBoxRect );
    }

    // Show or Hide scrollbars and sizebox
    if( m_HorzScrollbar.GetSafeHwnd() )
    {
        m_HorzScrollbar.ShowScrollBar( m_HorzSBVisible );
        m_HorzScrollbar.RedrawWindow();

        // Clear scroll position if bar has disappeared
        if( !m_HorzSBVisible )
        {
            m_ScrollX = 0;
            m_HorzScrollbar.SetScrollPos( 0 );
            PositionHeader();
        }
    }
    if( m_VertScrollbar.GetSafeHwnd() )
    {
        m_VertScrollbar.ShowScrollBar( m_VertSBVisible );
        m_VertScrollbar.RedrawWindow();

        // Clear scroll position if bar has disappeared
        if( !m_VertSBVisible )
        {
            m_ScrollY = 0;
            m_VertScrollbar.SetScrollPos( 0 );
        }
    }
    if( m_SizeBox.GetSafeHwnd() )
    {
        m_SizeBox.ShowScrollBar( BothVisible );
        m_SizeBox.RedrawWindow();
    }
}

/////////////////////////////////////////////////////////////////////////////
// UpdateScrollbars

void CVirtualListCtrl::UpdateScrollbars( void )
{
    CRect r;
    GetClientRect( &r );
    UpdateScrollbars( r.Width(), r.Height() );
}

/////////////////////////////////////////////////////////////////////////////
// SetVertScroll

void CVirtualListCtrl::SetVertScroll( int Value )
{
    SCROLLINFO si;
    m_VertScrollbar.GetScrollInfo( &si, SIF_POS );
    si.fMask = SIF_POS;
    si.nPos = Value;
    m_VertScrollbar.SetScrollInfo( &si );

    // Read the validated position
    m_VertScrollbar.GetScrollInfo( &si, SIF_POS );

    // Keep track of scroll position
    int Pos = m_LineMode ? (si.nPos*m_RowHeight) : si.nPos;
    if( m_ScrollY != Pos )
    {
        m_ScrollY = Pos;
        RedrawWindow();
    }

}

/////////////////////////////////////////////////////////////////////////////
// ToggleMarkSelected

void CVirtualListCtrl::ToggleMarkSelected( void )
{
    // Get current state from focus item
    bool CurrentState = GetMarked( m_FocusItem );

    // Set marks on all selected rows
    s32 Item;
    if( m_Selection.BeginIterate( Item ) )
    {
        do{
            OnSetMarked( Item, !CurrentState );
        }while( m_Selection.Iterate( Item ) );
    }
}

/////////////////////////////////////////////////////////////////////////////
// GotoNextMark

void CVirtualListCtrl::GotoNextMark( s32 Direction )
{
    // Get starting row
    int nRow = m_FocusItem;

    // Goto the prev or next mark, check if focus is valid
    if( IsValidItem( nRow ) )
    {
        bool Found = false;

        // Start at the next row
        int iRow = nRow + Direction;
        if( iRow >= GetItemCount() )
            iRow = 0;
        if( iRow < 0 )
            iRow = GetItemCount() - 1;

        // Loop until we are back where we started
        while( iRow != nRow )
        {
            // Is this row marked?
            if( OnGetMarked( iRow ) )
            {
                // Done
                nRow = iRow;
                Found = true;
                break;
            }

            // Advance to next row with looping
            iRow += Direction;
            if( iRow >= GetItemCount() )
                iRow = 0;
            if( iRow < 0 )
                iRow = GetItemCount() - 1;
        }

        // Set the focus
        SetFocusItem( nRow );

        // Move marked row to center of view if found and not currently in view
        if( Found && !IsVisible( nRow ) )
            EnsureCentered( nRow );
    }
}

/////////////////////////////////////////////////////////////////////////////
// GetSelectionSet

const CSelectionSet& CVirtualListCtrl::GetSelectionSet( void )
{
    return m_Selection;
}

/////////////////////////////////////////////////////////////////////////////
// BuildSelectionSet

void CVirtualListCtrl::BuildSelectionSet( void )
{
    m_Selection.Clear();

    int Count = GetItemCount();
    int Start = Count;
    for( int i=0 ; i<Count ; i++ )
    {
        if( OnGetSelected(i) )
        {
            if( Start == Count )
                Start = i;
        }
        else
        {
            if( Start != Count )
            {
                m_Selection.Select( Start, i-1 );
                Start = Count;
            }
        }
    }
    if( Start != Count )
    {
        m_Selection.Select( Start, Count-1 );
    }
}

/////////////////////////////////////////////////////////////////////////////
// ClearSelection

void CVirtualListCtrl::ClearSelection( void )
{
    s32 Item;
    if( m_Selection.BeginIterate( Item ) )
    {
        do {
            OnSetSelected( Item, false );
        } while( m_Selection.Iterate( Item ) );
    }

//    for( int iRow=GetItemCount()-1 ; iRow>=0 ; iRow-- )
//        OnSetSelected( iRow, false );

    m_Selection.Clear();
}

/////////////////////////////////////////////////////////////////////////////
// GetSelected

bool CVirtualListCtrl::GetSelected( int iRow )
{
    return OnGetSelected( iRow );
}

/////////////////////////////////////////////////////////////////////////////
// SetSelected

void CVirtualListCtrl::SetSelected( int iRow, bool State )
{
    if( (iRow >= 0) && (iRow < GetItemCount()) )
    {
        if( State )
        {
            if( !GetSelected( iRow ) )
            {
                m_Selection.Select( iRow, iRow );
            }
        }
        else
        {
            if( GetSelected( iRow ) )
            {
                m_Selection.Deselect( iRow, iRow );
            }
        }
        OnSetSelected( iRow, State );
    }
}

/////////////////////////////////////////////////////////////////////////////
// SetSelected

void CVirtualListCtrl::SetSelected( int iRowStart, int iRowEnd, bool State )
{
    ASSERT( iRowStart <= iRowEnd );
    ASSERT( iRowStart >= 0 );
    ASSERT( iRowEnd   <= GetItemCount() );

    m_Selection.Select( iRowStart, iRowEnd );
    for( int iRow=iRowStart ; iRow<=iRowEnd ; iRow++ )
        OnSetSelected( iRow, State );
}

/////////////////////////////////////////////////////////////////////////////
// ClearMarked

void CVirtualListCtrl::ClearMarked( void )
{
    for( int iRow=GetItemCount()-1 ; iRow>=0 ; iRow-- )
        OnSetMarked( iRow, false );
}

/////////////////////////////////////////////////////////////////////////////
// GetMarked

bool CVirtualListCtrl::GetMarked( int iRow )
{
    return OnGetMarked( iRow );
}

/////////////////////////////////////////////////////////////////////////////
// SetMarked

void CVirtualListCtrl::SetMarked( int iRow, bool State )
{
    if( (iRow >= 0) && (iRow < GetItemCount()) )
        OnSetMarked( iRow, State );
}

/////////////////////////////////////////////////////////////////////////////
// ClearChecked

void CVirtualListCtrl::ClearChecked( void )
{
    for( int iRow=GetItemCount()-1 ; iRow>=0 ; iRow-- )
        OnSetChecked( iRow, false );
}

/////////////////////////////////////////////////////////////////////////////
// GetChecked

bool CVirtualListCtrl::GetChecked( int iRow )
{
    return OnGetChecked( iRow );
}

/////////////////////////////////////////////////////////////////////////////
// SetChecked

void CVirtualListCtrl::SetChecked( int iRow, bool State )
{
    if( (iRow >= 0) && (iRow < GetItemCount()) )
        OnSetChecked( iRow, State );
}

/////////////////////////////////////////////////////////////////////////////
// OnGetSelected

bool CVirtualListCtrl::OnGetSelected( int iRow )
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////
// OnSetSelected

void CVirtualListCtrl::OnSetSelected( int iRow, bool State )
{
}

/////////////////////////////////////////////////////////////////////////////
// OnGetMarked

bool CVirtualListCtrl::OnGetMarked( int iRow )
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////
// OnSetMarked

void CVirtualListCtrl::OnSetMarked( int iRow, bool State )
{
}

/////////////////////////////////////////////////////////////////////////////
// OnGetChecked

bool CVirtualListCtrl::OnGetChecked( int iRow )
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////
// OnSetChecked

void CVirtualListCtrl::OnSetChecked( int iRow, bool State )
{
}

/////////////////////////////////////////////////////////////////////////////
// OnDrawCell

BOOL CVirtualListCtrl::OnDrawCell( CDC* pDC, CRect& rCell, int iRow, int iCol )
{
    CRect rCellText = rCell;
    rCellText.DeflateRect( 2, 0, 2, 0 );

    // Get the background color
    COLORREF Color;
    Color = RGB(255,255,255);
    if( iCol == m_SortColumn )
        Color = DarkenColor( Color, 16, 16, 16 );

    if( OnGetSelected( iRow ) )
        Color = DarkenColor( Color, 32, 32, 32 );

    // Fill the cell
    pDC->FillSolidRect( &rCell, Color );

    // Draw the text
    CString s;
    s.Format( "Cell %d %d", iCol, iRow );
    pDC->DrawText( s, &rCellText, m_Columns[iCol].m_TextAlign | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// OnSort

void CVirtualListCtrl::OnSort( void )
{
}

/////////////////////////////////////////////////////////////////////////////
// OnGetColumnFitWidth

int CVirtualListCtrl::OnGetColumnFitWidth( int iCol )
{
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// OnFind

void CVirtualListCtrl::OnFind( void )
{
}

/////////////////////////////////////////////////////////////////////////////
// OnFindNext

void CVirtualListCtrl::OnFindNext( void )
{
}

/////////////////////////////////////////////////////////////////////////////
// OnFindPrevious

void CVirtualListCtrl::OnFindPrevious( void )
{
}

/////////////////////////////////////////////////////////////////////////////
// CopySelectionToClipboard

void CVirtualListCtrl::OnCopy( void )
{
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Message Map

BEGIN_MESSAGE_MAP(CVirtualListCtrl, CWnd)
	//{{AFX_MSG_MAP(CVirtualListCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CONTEXTMENU()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_DESTROY()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
    ON_MESSAGE( WM_MOUSELEAVE, OnMouseLeave )
    ON_NOTIFY( HDN_BEGINTRACK     , 1, OnHeaderBeginTrack      )
    ON_NOTIFY( HDN_ITEMCHANGED    , 1, OnHeaderItemChanged     )
    ON_NOTIFY( HDN_ENDDRAG        , 1, OnHeaderEndDrag         )
    ON_NOTIFY( HDN_ITEMCLICK      , 1, OnHeaderItemClick       )
    ON_NOTIFY( HDN_DIVIDERDBLCLICK, 1, OnHeaderDividerDblClick )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OnCreate

int CVirtualListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    // Call base class
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    // Create the scrollbars and size box
    m_HorzScrollbar.Create( SBS_HORZ | SBS_BOTTOMALIGN | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, CRect(0,0,0,0), this, 100 );
    m_VertScrollbar.Create( SBS_VERT | SBS_RIGHTALIGN  | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, CRect(0,0,0,0), this, 101 );
    m_SizeBox.Create( SBS_SIZEBOX | SBS_SIZEBOXTOPLEFTALIGN | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, CRect(0,0,0,0), this, 102 );
    m_SizeBox.EnableScrollBar( ESB_DISABLE_BOTH );

    // Create the header control
	VERIFY( m_Header.Create( WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|HDS_BUTTONS|HDS_HOTTRACK|HDS_DRAGDROP|HDS_FULLDRAG|HDS_HORZ, CRect( 0,0,0,0 ), this, 1 ) );

    // Create font - Tahoma 11 pixel just the same as the CListCtrl default and set into window and header control
    m_Font.CreateFont( -11, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, "Tahoma" );
    m_Header.SetFont( &m_Font );

    // Initialize cell and row heights
    TEXTMETRIC tm;
    CDC* pDC = GetDC();
    CFont *pOriginalFont = pDC->SelectObject( &m_Font );
    pDC->GetTextMetrics( &tm );
    m_CellHeight = tm.tmHeight;
    // TODO: Modify m_CellHeight to account for checkbox
    m_RowHeight  = m_CellHeight + 1;
    //pDC->SelectObject( pOriginalFont );
    ReleaseDC( pDC );

    // Get theme data
    m_hTheme = OpenThemeData( GetSafeHwnd(), L"Button" );
    if( m_hTheme )
    {
        GetThemePartSize( m_hTheme, pDC->m_hDC, BP_CHECKBOX, CBS_NORMAL, NULL, TS_DRAW, &m_CheckSize );
    }
    else
    {
        m_CheckSize = CSize( ::GetSystemMetrics( SM_CXMENUCHECK ), ::GetSystemMetrics( SM_CYMENUCHECK ) );
    }

    if( HAS_CHECKBOXES() )
    {
        m_CellHeight = m_CheckSize.cy + CHECKBOX_SPACE + 1;
        m_RowHeight  = m_CellHeight + 1;
    }

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// OnSize

void CVirtualListCtrl::OnSize(UINT nType, int cx, int cy) 
{
    // Call base class OnSize
	CWnd::OnSize(nType, cx, cy);

    // Update scrollbars based on new window size
    UpdateScrollbars( cx, cy );

    // Position the header control at the top of the window
    if( m_Header.GetSafeHwnd() )
    {
        // Layout the header control in the client rect
        CRect       Rect( 0, 0, cx, cy );
        WINDOWPOS   WindowPos;
        HDLAYOUT    Layout;
        Layout.prc      = &Rect;
        Layout.pwpos    = &WindowPos;
        m_Header.Layout( &Layout );

        // Save the rect and move with window
        m_HeaderRect.SetRect( WindowPos.x, WindowPos.y, WindowPos.cx, WindowPos.cy );
        PositionHeader();
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnPaint

void CVirtualListCtrl::OnPaint() 
{
    // Create MemDC for flicker free rendering
	CPaintDC dc(this);
    CMemDC   MemDC( &dc );
    CDC*     pDC = &MemDC;

    // Select font
    CFont* pOriginalFont = pDC->SelectObject( &m_Font );
    pDC->SetBkMode( TRANSPARENT );

    // Get client rect
    CRect rClient;
    GetClientRect( &rClient );

    // Clear the background
    pDC->FillSolidRect( &rClient, RGB(255,255,255) );

    // Get number of columns
    int nColumns = m_Header.GetItemCount();

    // Only proceed if we have columns
    if( nColumns > 0 )
    {
        int iRow;
        int iCol;

        // Get rects & info & for all the columns
        CRect*  pRect = new CRect [nColumns];
        for( iCol=0 ; iCol<nColumns ; iCol++ )
        {
            // TODO: Use local copy of column structure, make sure it gets updated when columns are sized
            m_Header.GetItemRect( iCol, &pRect[iCol] );
            pRect[iCol].OffsetRect( -m_ScrollX, 0 );
        }

        // Get order array
        int* pOrderArray = new int[nColumns];
        m_Header.GetOrderArray( pOrderArray, nColumns );

        // Create pen
        CPen PenSeperator( PS_SOLID, 1, m_ColorSeperator );
        CPen* pOriginalPen = (CPen*)pDC->SelectObject( &PenSeperator );

        // Read total width of the header control
        int HeaderWidth = GetHeaderWidth();

        // Does the control have gridlines enabled?
        if( HAS_GRIDLINES() )
        {
            // Draw the column seperators
            int y1 = m_HeaderRect.bottom;
            int y2 = rClient.bottom;
            if( y2 > rClient.bottom )
                y2 = rClient.bottom;
            for( iCol=0 ; iCol<nColumns ; iCol++ )
            {
                pDC->MoveTo( pRect[iCol].right-1, y1 );
                pDC->LineTo( pRect[iCol].right-1, y2 );
            }

            // Draw the row seperators
            int x1 = rClient.left;
            int x2 = rClient.right;
            int y  = m_HeaderRect.bottom - m_ScrollY % m_RowHeight - 1;
            while( y < rClient.bottom )
            {
                pDC->MoveTo( x1, y );
                pDC->LineTo( x2, y );
                y += m_RowHeight;
            }
        }

        // Draw the cells
        CRect   rCell;
        int iRowStart = m_ScrollY / m_RowHeight;
        int iRowEnd   = iRowStart + (rClient.Height() - m_HeaderRect.bottom) / m_RowHeight + 2;
        iRowEnd = min( iRowEnd, GetItemCount() );
        for( iRow=iRowStart ; iRow<iRowEnd ; iRow++ )
        {
            for( iCol=0 ; iCol<nColumns ; iCol++ )
            {
                // Generate rect for cell
                int y = m_HeaderRect.bottom + iRow * m_RowHeight - m_ScrollY;
                if( HAS_GRIDLINES() )
                    rCell.SetRect( pRect[iCol].left, y, pRect[iCol].right-1, y + m_CellHeight );
                else
                    rCell.SetRect( pRect[iCol].left, y, pRect[iCol].right  , y + m_CellHeight + 1 );

                // Draw the cell if it is visible
                if( rCell.Width() > 0 )
                {
                    if( (iCol == 0) && (HAS_CHECKBOXES()) )
                    {
                        // Draw the check
                        if( m_hTheme )
                        {
                            CRect rCheck  = rCell;
                            rCheck.left  += CHECKBOX_SPACE;
                            rCheck.right  = rCheck.left + m_CheckSize.cx;
                            rCell.left    = rCheck.right + CHECKBOX_SPACE;

                            bool Hot     = (iRow == m_HotCheck);
                            bool Checked = OnGetChecked( iRow );
                            int  State   = 0;

                            if( Checked && !Hot )
                                State = CBS_CHECKEDNORMAL;
                            else if( Checked && Hot )
                                State = CBS_CHECKEDHOT;
                            else if( !Checked && !Hot )
                                State = CBS_UNCHECKEDNORMAL;
                            else if( !Checked && Hot )
                                State = CBS_UNCHECKEDHOT;

                            DrawThemeBackground( m_hTheme, pDC->m_hDC, BP_CHECKBOX, State, &rCheck, NULL );
                        }
                        else
                        {
                            CRect rCheck  = rCell;
                            rCheck.left  += CHECKBOX_SPACE;
                            rCheck.right  = rCheck.left + m_CheckSize.cx;
                            rCell.left    = rCheck.right + CHECKBOX_SPACE;

                            bool Hot     = (iRow == m_HotCheck);
                            bool Checked = OnGetChecked( iRow );
                            int  State   = 0;

                            if( Checked && !Hot )
                                State = DFCS_CHECKED;
                            else if( Checked && Hot )
                                State = DFCS_CHECKED|DFCS_PUSHED;
                            else if( !Checked && !Hot )
                                State = 0;
                            else if( !Checked && Hot )
                                State = DFCS_PUSHED;

                            pDC->DrawFrameControl( &rCheck, DFC_BUTTON, State|DFCS_FLAT );
                        }
                    }

                    // Invoke virtual function to draw the cell
                    if( !OnDrawCell( pDC, rCell, iRow, m_Columns[iCol].m_Index ) )
                    {
                        // Default fill of cell
                        COLORREF Color;
                        Color = RGB(255,255,255);
                        if( iCol == m_SortColumn )
                            Color = DarkenColor( Color, 16, 16, 16 );

                        if( OnGetSelected( iRow ) )
                            Color = DarkenColor( Color, 32, 32, 32 );

                        // Fill the cell
                        pDC->FillSolidRect( &rCell, Color );
                    }
                }
            }
        }

        // Draw the focus rect
        if( IsValidItem( m_FocusItem ) )
        {
            rCell.SetRect( -m_ScrollX, m_HeaderRect.bottom + m_FocusItem * m_RowHeight - m_ScrollY, HeaderWidth - m_ScrollX, m_HeaderRect.bottom + m_FocusItem * m_RowHeight + m_RowHeight - m_ScrollY );
            pDC->DrawFocusRect( &rCell );
        }

        // Select original objects back into dc
        pDC->SelectObject( pOriginalPen  );
        pDC->SelectObject( pOriginalFont );

        // Delete the order array & array of rects & items for the columns
        if( pOrderArray )
            delete[] pOrderArray;
        if( pRect )
            delete[] pRect;
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnEraseBkgnd

BOOL CVirtualListCtrl::OnEraseBkgnd(CDC* pDC) 
{
    // Do not erase the background OnPaint takes care of everything
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// OnHeaderBeginTrack

void CVirtualListCtrl::OnHeaderBeginTrack( NMHDR* pNotifyStruct, LRESULT* pResult )
{
    NMHEADER* pNMH = (NMHEADER*)pNotifyStruct;

    // Capture keyboard & redraw
    SetFocus();

    *pResult = 0;
}
    
/////////////////////////////////////////////////////////////////////////////
// OnHeaderItemChanged

void CVirtualListCtrl::OnHeaderItemChanged( NMHDR* pNotifyStruct, LRESULT* pResult )
{
    NMHEADER* pNMH = (NMHEADER*)pNotifyStruct;
    
    // Invalidate if the width of the item is changing (usually from dragging header splitter)
    if( pNMH->pitem )
    {
        if( pNMH->pitem->mask & (HDI_WIDTH|HDI_ORDER) )
        {
            // Force a redraw
            UpdateScrollbars();
            RedrawWindow();
        }
    }

    *pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// OnHeaderEndDrag

void CVirtualListCtrl::OnHeaderEndDrag( NMHDR* pNotifyStruct, LRESULT* pResult )
{
    // Force a redraw
    UpdateScrollbars();
    Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// OnHeaderItemClick

void CVirtualListCtrl::OnHeaderItemClick( NMHDR* pNotifyStruct, LRESULT* pResult )
{
    NMHEADER*   pNMH  = (NMHEADER*)pNotifyStruct;
    int         nCol  = pNMH->iItem;

    // Reset sort direction on change of sort column
    if( nCol != m_SortColumn )
        m_SortAscending = FALSE;

    SetSortColumn( nCol, !m_SortAscending );
    SetFocus();
}

/////////////////////////////////////////////////////////////////////////////
// SetSortColumn

void CVirtualListCtrl::SetSortColumn( int nCol, BOOL Ascending )
{

    HDITEM  Item;
    Item.mask = HDI_FORMAT;

    // Clear current sort icon if we have one & have changed column
    if( (m_SortColumn != -1) && (nCol != m_SortColumn) )
    {
        // Clear sort icon
        m_Header.GetItem( m_SortColumn, &Item );
        Item.fmt &= ~(HDF_SORTUP|HDF_SORTDOWN);
        m_Header.SetItem( m_SortColumn, &Item );
    }

    // Set new sort icons
    {
        // Save column & toggle direction
        m_SortColumn    = nCol;
        m_SortAscending = Ascending;

        // Set sort icon
        m_Header.GetItem( m_SortColumn, &Item );
        Item.fmt &= ~(HDF_SORTUP|HDF_SORTDOWN);
        Item.fmt |= m_SortAscending ? HDF_SORTUP : HDF_SORTDOWN;
        m_Header.SetItem( m_SortColumn, &Item );
    }

    // Call the sort handler
    OnSort();

    // Set focus to this window
    SetFocus();

    // Redraw
    Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// OnHeaderDividerDblClick

void CVirtualListCtrl::OnHeaderDividerDblClick( NMHDR* pNotifyStruct, LRESULT* pResult )
{
    NMHEADER*   pNMH    = (NMHEADER*)pNotifyStruct;
    int         Column  = pNMH->iItem;

    // Call virtual function to get the fit width
    int Width = OnGetColumnFitWidth( Column );

    if( Width != -1 )
    {
        // Modify width for checkboxes
        if( HAS_CHECKBOXES() && (Column == 0) )
        {
            Width += m_CheckSize.cx + CHECKBOX_SPACE * 2;
        }

        // Apply the fit width
        HDITEM  Item;
        Item.mask = HDI_WIDTH;
        Item.cxy  = Width;
        m_Header.SetItem( Column, &Item );

        // The redraw will be handled by OnItemChanged
    }

    // Set focus to this window
    SetFocus();
}

/////////////////////////////////////////////////////////////////////////////
// OnContextMenu

void CVirtualListCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Build context menu for header fields
    if( pWnd == &m_Header )
    {
        CXTCoolMenu	CoolMenu;
        CoolMenu.HookWindow( this );

        CXTMenu Menu;

        if( Menu.CreatePopupMenu() )
        {
            for( int i = m_Header.GetItemCount()-1 ; i >= 0 ; i-- )
            {
                // TODO: Complete this
/*
                Menu.InsertMenu( 0,
                                 MF_BYPOSITION |
                                 (m_aColumnData[i]->m_bVisible ? MF_CHECKED : MF_UNCHECKED) |
                                 (m_aColumnData[i]->m_bHidingAllowed ? 0 : MF_GRAYED)       |
                                 MF_STRING,
                                 i+1,
                                 m_aColumnData[i]->m_pLVColumn->pszText );
*/
                CString s;
                s.Format( "%s", m_Columns[i].m_Name );
                Menu.InsertMenu( 0,
                                 MF_BYPOSITION |
                                 MF_CHECKED |
                                 MF_GRAYED       |
                                 MF_STRING,
                                 i+1,
                                 s );
            }

            CPoint pt(0, 0);
            GetCursorPos( &pt );
            Menu.TrackPopupMenu( TPM_LEFTALIGN, pt.x, pt.y, this, 0 );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnHScroll

void CVirtualListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    m_HorzScrollbar.GetScrollInfo( &si );

    int Pos    = si.nPos;
    int OldPos = Pos;

    switch( nSBCode )
    {
    case SB_LEFT:
        Pos = 0;
        break;
    case SB_RIGHT:
        Pos = si.nMax;
        break;
    case SB_LINELEFT:
        Pos -= 1;
        break;
    case SB_LINERIGHT:
        Pos += 1;
        break;
    case SB_PAGELEFT:
        Pos -= si.nPage;
        break;
    case SB_PAGERIGHT:
        Pos += si.nPage;
        break;
    case SB_THUMBTRACK:
        Pos = si.nTrackPos;
        break;
    case SB_THUMBPOSITION:
        break;
    }

    // Has pos changed?
    if( OldPos != Pos )
    {
        // Set and read pos to validate it
        si.fMask = SIF_POS;
        si.nPos  = Pos;
        m_HorzScrollbar.SetScrollInfo( &si );
        m_HorzScrollbar.GetScrollInfo( &si, SIF_POS );

        // Keep track of scroll position
        if( m_ScrollX != si.nPos )
        {
            m_ScrollX = si.nPos;
            RedrawWindow();

            // Move the header
            PositionHeader();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnVScroll

void CVirtualListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    m_VertScrollbar.GetScrollInfo( &si );

    int Pos    = si.nPos;
    int OldPos = Pos;

    switch( nSBCode )
    {
    case SB_TOP:
        Pos = 0;
        break;
    case SB_BOTTOM:
        Pos = si.nMax;
        break;
    case SB_LINEUP:
        Pos -= m_LineMode ? 1 : m_RowHeight;
        break;
    case SB_LINEDOWN:
        Pos += m_LineMode ? 1 : m_RowHeight;
        break;
    case SB_PAGEUP:
        Pos -= si.nPage;
        break;
    case SB_PAGEDOWN:
        Pos += si.nPage;
        break;
    case SB_THUMBTRACK:
        Pos = si.nTrackPos;
        break;
    case SB_THUMBPOSITION:
        break;
    }

    // Has pos changed?
    if( OldPos != Pos )
    {
        // Set and read pos to validate it
        si.fMask = SIF_POS;
        si.nPos  = Pos;
        m_VertScrollbar.SetScrollInfo( &si );
        m_VertScrollbar.GetScrollInfo( &si, SIF_POS );

        // Keep track of scroll position
        int Pos = m_LineMode ? (si.nPos*m_RowHeight) : si.nPos;
        if( m_ScrollY != Pos )
        {
            m_ScrollY = Pos;
            Invalidate();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnMouseMove

void CVirtualListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
    // Register for tracking mouse leave
    if( !m_TrackingLeave )
    {
        TRACKMOUSEEVENT EventTrack;
        EventTrack.cbSize       = sizeof(EventTrack);
        EventTrack.dwFlags      = TME_LEAVE;
        EventTrack.dwHoverTime  = HOVER_DEFAULT;
        EventTrack.hwndTrack    = GetSafeHwnd();
        _TrackMouseEvent( &EventTrack );
        m_TrackingLeave = TRUE;
    }

    // Highlight checkbox
    if( HAS_CHECKBOXES() )
    {
	    int iRow = PointToRow( point, FALSE );
        if( iRow != -1 )
        {
            CRect rCheck;
            GetCellRect( iRow, 0, &rCheck );

            point.Offset( 0, -m_HeaderRect.bottom );

            rCheck.OffsetRect( CHECKBOX_SPACE, CHECKBOX_SPACE );
            rCheck.right = rCheck.left + m_CheckSize.cx;
            rCheck.bottom = rCheck.top + m_CheckSize.cy;

            // Verify cursor is over checkbox
            if( !rCheck.PtInRect( point ) )
                iRow = -1;
        }

        // Update hot tracked checkbox
        if( iRow != -1 )
        {
            if( m_HotCheck != iRow )
            {
                m_HotCheck = iRow;
                RedrawWindow();
            }
        }
        else
        {
            if( m_HotCheck != -1 )
            {
                m_HotCheck = -1;
                RedrawWindow();
            }
        }
    }

    if( m_MouseCaptured )
    {
/*
	    int iRow = PointToRow( point, TRUE );
        m_FocusRow = iRow;
        ClearSelection();
        SetSelected( min(m_SelectionRoot,m_FocusRow), max(m_SelectionRoot,m_FocusRow), true );
        Invalidate();
*/
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnMouseLeave

LRESULT CVirtualListCtrl::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    // Call the mouse move to do final processing
    CPoint pt;
    GetCursorPos( &pt );
    ScreenToClient( &pt );
    OnMouseMove( 0, pt );

    // No longer tracking
    m_TrackingLeave = FALSE;

    // Currently hot tracking a checkbox?
    if( m_HotCheck != -1 )
    {
        // No longer hot on checkbox
        m_HotCheck = -1;

        // Force redraw
        RedrawWindow();
    }

    // Done
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// OnLButtonDown

void CVirtualListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
    BOOL    Control         = ::GetKeyState( VK_CONTROL ) & 0x8000;
    BOOL    Shift           = ::GetKeyState( VK_SHIFT   ) & 0x8000;
    int     OldFocusItem    = m_FocusItem;

    // Claim Keyboard input
    SetFocus();

    // Capture the mouse
    SetCapture();
    m_MouseCaptured = TRUE;

    // Check for hot tracking checkbox
    if( HAS_CHECKBOXES() && (m_HotCheck != -1) )
    {
        // Get new state
        bool State = !OnGetChecked( m_HotCheck );

        // Run some selection logic
        if( !OnGetSelected( m_HotCheck ) )
        {
            if( Shift && Control )
            {
                m_FocusItem = m_HotCheck;
                SetSelected( min(m_FocusItem,m_SelectionRoot), max(m_FocusItem,m_SelectionRoot), true );
            }
            if( Control || Shift )
            {
                m_FocusItem = m_HotCheck;
                m_SelectionRoot = m_FocusItem;
                SetSelected( m_FocusItem, true );
            }
            else
            {
                m_FocusItem = m_HotCheck;
                m_SelectionRoot = m_FocusItem;
                ClearSelection();
                SetSelected( m_FocusItem, true );
            }
        }

        // Set Focus
        m_FocusItem = m_HotCheck;

        // Set state for all selected items
        s32 Item;
        if( m_Selection.BeginIterate( Item ) )
        {
            do{
                OnSetChecked( Item, State );
            }while( m_Selection.Iterate( Item ) );
        }

        // Send a notification
        CWnd* pParent = GetParent();
        if( pParent )
        {
            NMHDR nm;
            nm.hwndFrom = GetSafeHwnd();
            nm.idFrom   = GetDlgCtrlID();
            nm.code     = VLC_CHECK_CHANGED;
            pParent->SendNotifyMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nm );
        }

        // Force a redraw
        RedrawWindow();
    }
    else
    {
        int iRow = PointToRow( point );

        if( iRow != -1 )
        {
            // Save click position
    	    m_ClickedPoint = point;
            m_ClickedRow   = iRow;

            // Set new focus item
            m_FocusItem = iRow;

            if( Shift && Control )
            {
                SetSelected( min(m_FocusItem,m_SelectionRoot), max(m_FocusItem,m_SelectionRoot), true );
            }
            else if( Shift )
            {
                ClearSelection();
                SetSelected( min(m_FocusItem,m_SelectionRoot), max(m_FocusItem,m_SelectionRoot), true );
            }
            else if( Control )
            {
                m_SelectionRoot = m_FocusItem;
                SetSelected( m_FocusItem, !GetSelected( m_FocusItem ) );
            }
            else
            {
                m_SelectionRoot = m_FocusItem;
                ClearSelection();
                SetSelected( m_FocusItem, true );
            }

            // Force a redraw
            RedrawWindow();
        }
        else
        {
            m_FocusItem = -1;
            ClearSelection();
            RedrawWindow();
        }
    }

    // Broadcast any change in focus
    if( m_FocusItem != OldFocusItem )
        OnFocusChanged();
}

/////////////////////////////////////////////////////////////////////////////
// OnLButtonUp

void CVirtualListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if( m_MouseCaptured )
    {
    	ReleaseCapture();
        m_MouseCaptured = FALSE;
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnLButtonDblClk

void CVirtualListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
    // Toggle check
    if( HAS_CHECKBOXES() )
    {
        // TODO: Add double click handler
    }
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////
// OnKeyDown

void CVirtualListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    BOOL    Control         = ::GetKeyState( VK_CONTROL ) & 0x8000;
    BOOL    Shift           = ::GetKeyState( VK_SHIFT   ) & 0x8000;
    int     OldFocusItem    = m_FocusItem;

    SCROLLINFO si;
    m_VertScrollbar.GetScrollInfo( &si );
    int Page = m_LineMode ? si.nPage : (si.nPage / m_RowHeight);
    if( Page < 1 )
        Page = 1;

    // Read current focus item
    int nRow = m_FocusItem;

    switch( nChar )
    {
    case VK_HOME:
        nRow = 0;
        break;
    case VK_PRIOR:
        nRow -= Page-1;
        break;
    case VK_UP:
        nRow--;
        break;
    case VK_DOWN:
        nRow++;
        break;
    case VK_NEXT:
        nRow += Page-1;
        break;
    case VK_END:
        nRow = GetItemCount()-1;
        break;
    case VK_SPACE:
        if( HAS_CHECKBOXES() )
        {
            bool State = !OnGetChecked( m_FocusItem );

            // Set state for all selected items
            s32 Item;
            if( m_Selection.BeginIterate( Item ) )
            {
                do{
                    OnSetChecked( Item, State );
                }while( m_Selection.Iterate( Item ) );
            }

            // Send a notification
            CWnd* pParent = GetParent();
            if( pParent )
            {
                NMHDR nm;
                nm.hwndFrom = GetSafeHwnd();
                nm.idFrom   = GetDlgCtrlID();
                nm.code     = VLC_CHECK_CHANGED;
                pParent->SendNotifyMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nm );
            }

            // Force a redraw
            RedrawWindow();
        }
        else
        {
            SetSelected( m_FocusItem, !GetSelected( m_FocusItem ) );
            Invalidate();
        }
        break;
    case 'A':
        if( Control )
        {
            SetSelected( 0, GetItemCount()-1, true );
            RedrawWindow();
        }
        break;
    case 'C':
        if( Control )
        {
            OnCopy();
        }
        break;
    case 'F':
        if( Control )
        {
            OnFind();
            nRow = m_FocusItem;
        }
        break;
    case VK_F3:
        if( Shift )
            OnFindPrevious();
        else
            OnFindNext();
        nRow = m_FocusItem;
        break;
    case VK_F2:
        if( Control )
        {
            ToggleMarkSelected();
            RedrawWindow();
        }
        else
        {
            GotoNextMark( Shift ? -1 : 1 );
            nRow = m_FocusItem;
        }

        // Clear shift and control flags
        Shift   = FALSE;
        Control = FALSE;

        break;
    }

    // Check for change of focus item
    if( nRow != m_FocusItem )
    {
        // Limit row to range
        if( nRow >= GetItemCount() )
            nRow = GetItemCount() - 1;
        if( nRow < 0 )
            nRow = 0;

        // Save old focus item and set new focus item
        int OldFocusItem = m_FocusItem;
        m_FocusItem = nRow;

        // Set selection state of rows
        if( Shift && Control )
        {
            // Move Focus and select from root to focus
            SetSelected( min(m_FocusItem,m_SelectionRoot), max(m_FocusItem,m_SelectionRoot), true );
        }
        else if( Shift )
        {
            // Move Focus and select only from root to focus
            ClearSelection();
            SetSelected( min(m_FocusItem,m_SelectionRoot), max(m_FocusItem,m_SelectionRoot), true );
        }
        else if( Control )
        {
            // Move Focus but don't change selection
        }
        else
        {
            // Move Focus and select that single row
            ClearSelection();
            m_SelectionRoot = m_FocusItem;
            SetSelected( nRow, true );
        }

        // Bring the new focus into view
        EnsureVisible( m_FocusItem );

        // Froce redraw
        RedrawWindow();
    }

    // Broadcast any change in focus
    if( m_FocusItem != OldFocusItem )
        OnFocusChanged();

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// OnMouseWheel

BOOL CVirtualListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    SCROLLINFO si;
    m_VertScrollbar.GetScrollInfo( &si );
    si.fMask = SIF_POS;
    int Pos = si.nPos;
    int nMoves = (zDelta / WHEEL_DELTA);
    Pos -= (nMoves) * (m_LineMode ? 1 : m_RowHeight);
	SetVertScroll( Pos );

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

/////////////////////////////////////////////////////////////////////////////
// OnSetFocus

void CVirtualListCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
    // Set state
	m_IsFocusWnd = TRUE;

    // Redraw
    RedrawWindow();
}

/////////////////////////////////////////////////////////////////////////////
// OnKillFocus

void CVirtualListCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	
    // Set State
    m_IsFocusWnd = FALSE;

    // Redraw
    RedrawWindow();
}

/////////////////////////////////////////////////////////////////////////////

void CVirtualListCtrl::OnDestroy() 
{
	CWnd::OnDestroy();

    // Close the theme
    if( m_hTheme )
    {
        CloseThemeData( m_hTheme );
        m_hTheme = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CVirtualListCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
    BOOL    Control         = ::GetKeyState( VK_CONTROL ) & 0x8000;
    BOOL    Shift           = ::GetKeyState( VK_SHIFT   ) & 0x8000;
    int     OldFocusItem    = m_FocusItem;
    int     iRow            = PointToRow( point );

    if( !Shift && !Control )
    {
        // Set new focus item
        m_FocusItem = iRow;

        if( iRow != -1 )
        {
            if( !GetSelected( m_FocusItem ) )
            {
                ClearSelection();
                SetSelected( m_FocusItem, true );
                m_SelectionRoot = m_FocusItem;
            }
        }
        else
        {
            ClearSelection();
        }

        // Force a redraw
        RedrawWindow();
    }

    // Send a notification
    CWnd* pParent = GetParent();
    if( pParent )
    {
        NMHDR nm;
        nm.hwndFrom = GetSafeHwnd();
        nm.idFrom   = GetDlgCtrlID();
        nm.code     = VLC_CONTEXT_MENU;
        pParent->SendNotifyMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nm );
    }
}

/////////////////////////////////////////////////////////////////////////////
