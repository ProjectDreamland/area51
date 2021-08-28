// ViewMemoryGraph.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "ViewMemoryGraph.h"
#include "MemDC.h"
#include "HeapState.h"
#include "xToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryGraph

IMPLEMENT_DYNCREATE(CViewMemoryGraph, CViewBase)

CViewMemoryGraph::CViewMemoryGraph()
{
    m_UpdateTimerID     = 0;

    m_State             = STATE_NULL;
    m_FullMinAddress    = 0x0;
    m_FullMaxAddress    = 0x1;
    m_ZoomMinAddress    = 0x0;
    m_ZoomMaxAddress    = 0x1;
    m_ZoomCenterAddress = (m_ZoomMinAddress + m_ZoomMaxAddress)/2;
    m_ZoomAddressSet    = false;

    m_pSelectedEntry    = NULL;

//	m_TipWindow.SetMargins(CSize(20,5));
//	m_TipWindow.SetLineSpace(15);

}

CViewMemoryGraph::~CViewMemoryGraph()
{
}

BEGIN_MESSAGE_MAP(CViewMemoryGraph, CViewBase)
	//{{AFX_MSG_MAP(CViewMemoryGraph)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
    ON_MESSAGE( WM_MOUSEHOVER, OnMouseHover )
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

double CViewMemoryGraph::PixelToAddress( CRect& r, double Min, double Max, s32 x )
{
    ASSERT( r.Width() > 0 );

    double t = (double)(x-r.left)/(double)(r.Width()-1);
    u32 Address = (u32)((Max-Min)*t + 0.5 + Min);
    if( Address < (u32)Min ) Address = (u32)Min;
    if( Address > (u32)Max ) Address = (u32)Max;
    return Address;
}

/////////////////////////////////////////////////////////////////////////////

double CViewMemoryGraph::AddressToPixel( CRect& r, double Min, double Max, u32 Address, bool Clip )
{
    if( Max <= Min )
        Max = Min + 1;

    double t = ((double)Address-Min)/(Max-Min);
    s32 x = (s32)((r.Width())*t + 0.5f + r.left);
    if( Clip )
    {
        if( x < r.left  ) x = r.left;
        if( x > r.right ) x = r.right;
    }
    return x;
}

/////////////////////////////////////////////////////////////////////////////

log_memory* CViewMemoryGraph::AddressToLogEntry( u32 Address )
{
    // Scan existing list of blocks
    for( s32 i=0 ; i<m_MemBlocks.GetCount() ; i++ )
    {
        u32 BlockMinAddress = m_MemBlocks[i]->GetAddress() - MEMORY_HEADER_SIZE;
        u32 BlockMaxAddress = (m_MemBlocks[i]->GetAddress() + (m_MemBlocks[i]->GetSize() + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1));
        if( (BlockMinAddress <= Address) && (BlockMaxAddress >= Address) )
            return m_MemBlocks[i];
    }

    // Check with the document for the last thing occupying that space
    CxToolDoc* pDoc = GetDocument();
    ASSERT( pDoc );
    return pDoc->GetLastEntryAtAddress( Address );
}

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryGraph drawing

#define COLOR_BLACK             RGB(  0,  0,  0)
#define COLOR_FREE              RGB(255,255,255)
#define COLOR_MAP_OUTLINE       RGB(  0,  0,  0)
#define COLOR_BLOCK             RGB(128,128,255)
#define COLOR_BLOCK_OUTLINE     RGB(  0,  0,128)
#define COLOR_BLOCK_HI          RGB(255,255,128)
#define COLOR_BLOCK_HI_OUTLINE  RGB(128,128,  0)
#define COLOR_CURVE_FILL        RGB(192,192,192)
#define COLOR_FRAME_TEXT        RGB(  0, 70,213)

#define XCOLOR_FREE             ARGB(255,255,255,255)
#define XCOLOR_BLOCK            ARGB(255,128,128,255)
#define XCOLOR_BLOCK_OUTLINE    ARGB(255,  0,  0,128)
#define XCOLOR_BLOCK_HI         ARGB(255,255,255,128)
#define XCOLOR_BLOCK_HI_OUTLINE ARGB(255,128,128,  0)

void CViewMemoryGraph::OnDraw(CDC* pInDC)
{
    CMemDC pDC( pInDC );

    // Create background color brush
    CBrush BackgroundBrush;
    BackgroundBrush.CreateSysColorBrush( COLOR_BTNFACE );

    // Create Pens
    CPen BlackPen;
    CPen FramePen;
    CPen CurvePen;
    BlackPen.CreatePen( PS_SOLID, 1, COLOR_BLACK );
    FramePen.CreatePen( PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW) );
    CurvePen.CreatePen( PS_SOLID, 1, COLOR_CURVE_FILL );

    // Select a pen
    CPen* pOldPen = pDC->SelectObject( &BlackPen );

    // Clear background
    CBrush* pOldBrush = pDC->SelectObject( &BackgroundBrush );
    pDC->FillSolidRect( &m_rClient, GetSysColor( COLOR_BTNFACE ) );

    // Setup for text rendering
    CFont* pOldFont = pDC->SelectObject( &m_Font );
    pDC->SetTextColor( RGB(0,0,0) );
    pDC->SetBkMode   ( TRANSPARENT );

    // Get the heap details needed
	CxToolDoc* pDoc = GetDocument();
    CHeapState& HeapState = pDoc->GetHeapState();
    HeapState.GetActiveAllocations( m_MemBlocks );

    // Get address range for full and zoom
    m_FullMinAddress = HeapState.GetMinAddress();
    m_FullMaxAddress = HeapState.GetMaxAddress();
    if( m_FullMinAddress == U32_MAX )
        m_FullMinAddress = 0;
    if( m_FullMaxAddress <= m_FullMinAddress )
        m_FullMaxAddress = m_FullMinAddress + 1;
    if( !m_ZoomAddressSet )
    {
        m_ZoomMinAddress = m_FullMinAddress;
        m_ZoomMaxAddress = m_FullMaxAddress;
    }

    // Collect some stats from the allocations
    u32     MinSequence = 0;
    u32     MaxSequence = 0;
    u32     MinSize     = 0;
    u32     MaxSize     = 0;
    double  MinTicks    = 0;
    double  MaxTicks    = 0;
    if( m_MemBlocks.GetCount() > 0 )
    {
        log_memory* pEntry = m_MemBlocks[0];
        MinSequence = MaxSequence = pEntry->GetSequence();
        MinTicks    = MaxTicks    = pEntry->GetTicks();

        for( s32 i=1 ; i<m_MemBlocks.GetCount() ; i++ )
        {
            log_memory* pEntry = m_MemBlocks[i];
            u32     Sequence = pEntry->GetSequence();
            u32     Size     = pEntry->GetSize();
            double  Ticks    = pEntry->GetTicks();

            MinSequence = MIN( MinSequence, Sequence );
            MaxSequence = MAX( MaxSequence, Sequence );
            MinSize     = MIN( MinSize,     Size     );
            MaxSize     = MAX( MaxSize,     Size     );
            MinTicks    = MIN( MinTicks,    Ticks    );
            MaxTicks    = MAX( MaxTicks,    Ticks    );
        }
    }

    // Prevent divide by 0
    if( MaxSequence == MinSequence ) MaxSequence = MinSequence + 1;
    if( MaxSize     == MinSize     ) MaxSize     = MinSize     + 1;
    if( MaxTicks    == MinTicks    ) MaxTicks    = MinTicks    + 1;

    // Draw full allocation map
    if( (m_BitmapFull. GetWidth() != m_rFull. Width()) ||
        (m_BitmapFull.GetHeight() != m_rFull.Height()) ||
        1 )
    {
        // Setup
        m_BitmapFull.SetSize( m_rFull.Width(), 3 ); //m_rFull.Height() );
        CRect rFull = m_rFull;
        rFull.OffsetRect( -rFull.TopLeft() );
        CRect bFull = rFull;
        bFull.bottom = bFull.top + 3;
        m_BitmapFull.Clear( XCOLOR_FREE );

        // Loop over memory blocks
        for( s32 i=0 ; i<m_MemBlocks.GetCount() ; i++ )
        {
            // Get block addresses accounting for header and granularity
            u32 BlockMinAddress = m_MemBlocks[i]->GetAddress() - MEMORY_HEADER_SIZE;
            u32 BlockMaxAddress = (m_MemBlocks[i]->GetAddress() + (m_MemBlocks[i]->GetSize() + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1));

            // Get rect
            bFull.left  = (s32)(AddressToPixel( rFull, m_FullMinAddress, m_FullMaxAddress, BlockMinAddress ) );
            bFull.right = (s32)(AddressToPixel( rFull, m_FullMinAddress, m_FullMaxAddress, BlockMaxAddress ) );

            // Is this block selected?
            bool Selected = ( m_MemBlocks[i] == m_pSelectedEntry );

            // Draw block
            m_BitmapFull.FramedRect( bFull, Selected ? XCOLOR_BLOCK_HI : XCOLOR_BLOCK, Selected ? XCOLOR_BLOCK_HI_OUTLINE : XCOLOR_BLOCK_OUTLINE );
//            m_BitmapFull.HorizontalLine( 0, bFull.left, bFull.right-1, Selected ? XCOLOR_BLOCK_HI : XCOLOR_BLOCK );
        }
    }

    // Draw zoomed allocation map
    {
        m_BitmapZoom.SetSize( m_rZoom.Width(), 3 ); //m_rZoom.Height() );
        CRect rZoom = m_rZoom;
        rZoom.OffsetRect( -rZoom.TopLeft() );
        CRect bZoom = rZoom;
        bZoom.bottom = bZoom.top + 3;
        m_BitmapZoom.Clear( XCOLOR_FREE );

        for( s32 i=0 ; i<m_MemBlocks.GetCount() ; i++ )
        {
            // Get block addresses accounting for header and granularity
            log_memory* pEntry = m_MemBlocks[i];
            u32 BlockMinAddress = pEntry->GetAddress() - MEMORY_HEADER_SIZE;
            u32 BlockMaxAddress = (pEntry->GetAddress() + (m_MemBlocks[i]->GetSize() + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1));

            // Draw Zoom Block
            double l = AddressToPixel( rZoom, m_ZoomMinAddress, m_ZoomMaxAddress, BlockMinAddress, false );
            double r = AddressToPixel( rZoom, m_ZoomMinAddress, m_ZoomMaxAddress, BlockMaxAddress, false );
            if( (r >= rZoom.left) && (l < rZoom.right) )
            {
                // Clip double precision zoomed values before conversion to s32
                if( l < (rZoom.left-1) ) l = rZoom.left-1;
                if( r > (rZoom.right ) ) r = rZoom.right;
                bZoom.left  = (s32)l;
                bZoom.right = (s32)r;

                // Is this block selected?
                bool Selected = ( m_MemBlocks[i] == m_pSelectedEntry );

                // Draw block
                m_BitmapZoom.FramedRect( bZoom, Selected ? XCOLOR_BLOCK_HI : XCOLOR_BLOCK, Selected ? XCOLOR_BLOCK_HI_OUTLINE : XCOLOR_BLOCK_OUTLINE );
//                m_BitmapZoom.SolidRect( bZoom, Selected ? XCOLOR_BLOCK_HI : XCOLOR_BLOCK );
            }
        }
    }

    s32 y;
    for( y=2 ; y<m_BitmapFull.GetHeight()-1 ; y++ )
    {
        m_BitmapFull.CopyLine( 1, y );
    }
    m_BitmapFull.CopyLine( 0, m_BitmapFull.GetHeight()-1 );

    for( y=2 ; y<m_BitmapZoom.GetHeight()-1 ; y++ )
    {
        m_BitmapZoom.CopyLine( 1, y );
    }
    m_BitmapZoom.CopyLine( 0, m_BitmapFull.GetHeight()-1 );

    m_BitmapFull.Draw( pDC, m_rFull );
    m_BitmapZoom.Draw( pDC, m_rZoom );

    // Draw highlight
    switch( m_State )
    {
    case STATE_SELECT_FULL:
        {
            CRect rFull = m_rFull;
            rFull.left  = m_PointButtonDown.x;
            rFull.right = m_PointButtonUp.x;
            rFull.NormalizeRect();
            pDC->PatBlt( rFull.left, rFull.top, rFull.Width(), rFull.Height(), DSTINVERT );
        }
        break;
    case STATE_SELECT_ZOOM:
        {
            CRect rZoom = m_rZoom;
            rZoom.left  = m_PointButtonDown.x;
            rZoom.right = m_PointButtonUp.x;
            rZoom.NormalizeRect();
            pDC->PatBlt( rZoom.left, rZoom.top, rZoom.Width(), rZoom.Height(), DSTINVERT );
        }
        break;
    default:
        break;
    }

    // Draw zoom curve
    s32 CURVE_H = m_rZoom.top - m_rFull.bottom - 1;
    for( s32 Pass = 0 ; Pass < 3 ; Pass++ )
    {
        if( Pass == 0 )
            pDC->SelectObject( &CurvePen );
        else
            pDC->SelectObject( &BlackPen );

        s32 fx1 = (s32)AddressToPixel( m_rFull, m_FullMinAddress, m_FullMaxAddress, (u32)m_ZoomMinAddress );
        s32 fx2 = (s32)AddressToPixel( m_rFull, m_FullMinAddress, m_FullMaxAddress, (u32)m_ZoomMaxAddress );

        s32 zx1 = m_rZoom.left;
        s32 zx2 = m_rZoom.right-1;
        s32 x1;
        s32 x2;
        s32 y;
        for(  y=0 ; y <= CURVE_H ; y++ )
        {
            x1 = (s32)(fx1 - (fx1-zx1) * (1 - cos( (PI*.02)+((PI*.96)*y/CURVE_H)))/2);
            x2 = (s32)(fx2 + (zx2-fx2) * (1 - cos( (PI*.02)+((PI*.96)*y/CURVE_H)))/2);

            switch( Pass )
            {
            case 0:
                pDC->MoveTo( x1, m_rFull.bottom + y );
                pDC->LineTo( x2, m_rFull.bottom + y );
                break;
            case 1:
                if( y == 0 )
                    pDC->MoveTo( x1, m_rFull.bottom + y );
                else
                {
                    pDC->LineTo( x1, m_rFull.bottom + y );
                    if( y == CURVE_H )
                        pDC->LineTo( x1, m_rFull.bottom + y + 1 );
                }
                break;
            case 2:
                if( y == 0 )
                    pDC->MoveTo( x2, m_rFull.bottom + y );
                else
                {
                    pDC->LineTo( x2, m_rFull.bottom + y );
                    if( y == CURVE_H )
                        pDC->LineTo( x2, m_rFull.bottom + y + 1 );
                }
                break;
            }
        }
    }

    // Display addresses
    {
        CString String;
        CSize   Size;
        String.Format( "0x%08X", m_FullMinAddress );
        pDC->TextOut( m_rFull.left, m_rFull.top - 14, String, String.GetLength() );

        String.Format( "0x%08X", m_FullMaxAddress );
        Size = pDC->GetTextExtent( String );
        pDC->TextOut( m_rFull.right-Size.cx, m_rFull.top - 14, String, String.GetLength() );

        String.Format( "0x%08X", (u32)m_ZoomMinAddress );
        pDC->TextOut( m_rZoom.left, m_rZoom.bottom, String, String.GetLength() );

        String.Format( "0x%08X", (u32)m_ZoomMaxAddress );
        Size = pDC->GetTextExtent( String );
        pDC->TextOut( m_rZoom.right-Size.cx, m_rZoom.bottom, String, String.GetLength() );
    }

    // Draw frames for Summary and Details
    {
        pDC->SelectObject( &FramePen );
        pDC->SetTextColor( COLOR_FRAME_TEXT );
        pDC->SetBkColor( GetSysColor(COLOR_BTNFACE) );
        pDC->SetBkMode( OPAQUE );
        m_rSummary.DeflateRect( 0, 6, 0, 0 );
        m_rDetails.DeflateRect( 0, 6, 0, 0 );
        pDC->RoundRect( &m_rSummary, CPoint(6,8) );
        pDC->RoundRect( &m_rDetails, CPoint(6,8) );
        m_rSummary.InflateRect( 0, 6, 0, 0 );
        m_rDetails.InflateRect( 0, 6, 0, 0 );
        CString s = _T("Summary");
        pDC->TextOut( m_rSummary.left + 8, m_rSummary.top, s, s.GetLength() );
        s = _T("Details");
        pDC->TextOut( m_rDetails.left + 10, m_rDetails.top, s, s.GetLength() );
    }

    // Draw Summary
    {
        CRect   r1, r2, rt;
        CString s;

        r1 = m_rSummary;
        r1.DeflateRect( 2, 16, 2, 2 );
        r1.bottom = r1.top + 14;
        r2 = r1;
        r1.right = min( r1.right, r1.left + 112 );
        r2.left  = r1.right + 16;

        pDC->SetTextColor( COLOR_BLACK );

        CHeapState& HeapState = pDoc->GetHeapState();
        u32 MinAddress              = HeapState.GetMinAddress();
        u32 MaxAddress              = HeapState.GetMaxAddress();
        u32 CurrentlyAllocated      = HeapState.GetCurrentBytes();
        u32 MaximumAllocated        = HeapState.GetMaximumBytes();
        s32 NumActiveAllocations    = HeapState.GetNumActiveAllocations();

        // Print allocated
        s = _T("Currently allocated:");
        pDC->DrawText( s, &r1, DT_RIGHT );
        PrettyInt( s, CurrentlyAllocated );
        pDC->DrawText( s, &r2, DT_LEFT );
        rt = r2;
        rt.left += 80;
        s.Format( "0x%08X", CurrentlyAllocated );
        pDC->DrawText( s, &rt, DT_LEFT );

        // Print max allocated
        r1.OffsetRect( 0, r1.Height() );
        r2.OffsetRect( 0, r2.Height() );
        s = _T("Maximum allocated:");
        pDC->DrawText( s, &r1, DT_RIGHT );
        PrettyInt( s, MaximumAllocated );
        pDC->DrawText( s, &r2, DT_LEFT );
        rt = r2;
        rt.left += 80;
        s.Format( "0x%08X", MaximumAllocated );
        pDC->DrawText( s, &rt, DT_LEFT );

        // Print num allocations
        r1.OffsetRect( 0, r1.Height() );
        r2.OffsetRect( 0, r2.Height() );
        s = _T("Active allocations:");
        pDC->DrawText( s, &r1, DT_RIGHT );
        PrettyInt( s, NumActiveAllocations );
        pDC->DrawText( s, &r2, DT_LEFT );
        rt = r2;
        rt.left += 80;
        s.Format( "0x%08X", NumActiveAllocations );
        pDC->DrawText( s, &rt, DT_LEFT );

        // Print free
        r1.OffsetRect( 0, r1.Height() );
        r2.OffsetRect( 0, r2.Height() );
        s = _T("Currently free:");
        pDC->DrawText( s, &r1, DT_RIGHT );
        PrettyInt( s, (MaxAddress-MinAddress)-CurrentlyAllocated );
        pDC->DrawText( s, &r2, DT_LEFT );
        rt = r2;
        rt.left += 80;
        s.Format( "0x%08X", (MaxAddress-MinAddress)-CurrentlyAllocated );
        pDC->DrawText( s, &rt, DT_LEFT );

        // Print min free
        r1.OffsetRect( 0, r1.Height() );
        r2.OffsetRect( 0, r2.Height() );
        s = _T("Minimum free:");
        pDC->DrawText( s, &r1, DT_RIGHT );
        PrettyInt( s, (MaxAddress-MinAddress)-MaximumAllocated );
        pDC->DrawText( s, &r2, DT_LEFT );
        rt = r2;
        rt.left += 80;
        s.Format( "0x%08X", (MaxAddress-MinAddress)-MaximumAllocated );
        pDC->DrawText( s, &rt, DT_LEFT );

        // Print fragments
        r1.OffsetRect( 0, r1.Height() );
        r2.OffsetRect( 0, r2.Height() );
        s = _T("Fragment count:");
        pDC->DrawText( s, &r1, DT_RIGHT );
        s = _T("???"); //PrettyInt( s, 0 );
        pDC->DrawText( s, &r2, DT_LEFT );
        rt = r2;
        rt.left += 80;
        //s.Format( "0x%08X", 0 );
        pDC->DrawText( s, &rt, DT_LEFT );
    }

    // Select old objects back into DC
    pDC->SelectObject( pOldPen );
    pDC->SelectObject( pOldFont );
    pDC->SelectObject( pOldBrush );
}

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryGraph diagnostics

#ifdef _DEBUG
void CViewMemoryGraph::AssertValid() const
{
	CViewBase::AssertValid();
}

void CViewMemoryGraph::Dump(CDumpContext& dc) const
{
	CViewBase::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryGraph message handlers

int CViewMemoryGraph::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

    ASSERT( m_pDocument );

    // Create font - Tahoma 11 pixel just the same as the listctrl default
    m_Font.CreateFont( -11, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, "Tahoma" );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnSize(UINT nType, int cx, int cy) 
{
	CViewBase::OnSize(nType, cx, cy);

    // Save Client Rect
    m_rClient.SetRect( 0, 0, cx, cy );

    // Make rects for Full and Zoom areas
    CRect r( 0, 14, cx, cy-12-112 );
    if( r.Height() < 32 )
        r.top = r.bottom - 32;
    s32 BarHeight = (r.Height()-r.Height()/5)/2;
    r.DeflateRect( 2, 0, 1, 0 );
    m_rFull.SetRect( r.left, r.top, r.right, r.top + BarHeight );
    m_rZoom.SetRect( r.left, r.bottom - BarHeight, r.right, r.bottom );

    // Make rects for Summary and Details area
    r.SetRect( 0, cy-112, cx, cy );
    r.DeflateRect( 2, 2, 2, 2 );
    m_rSummary = r;
    m_rSummary.right = m_rSummary.left + 288;
    m_rDetails = r;
    m_rDetails.left = m_rSummary.right + 4;
    m_rDetails.right = max( m_rDetails.right, m_rDetails.left + 64 );
}

/////////////////////////////////////////////////////////////////////////////

BOOL CViewMemoryGraph::OnEraseBkgnd(CDC* pDC) 
{
//    CRect r;
//    GetClientRect( &r );
//    pDC->FillSolidRect( &r, GetSysColor(COLOR_BTNFACE) );

    return 0;
//	return CView::OnEraseBkgnd(pDC);
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::MousePointsToZoomAddresses( void )
{
    double a1 = PixelToAddress( m_rFull, m_FullMinAddress, m_FullMaxAddress, m_PointButtonDown.x );
    double a2 = PixelToAddress( m_rFull, m_FullMinAddress, m_FullMaxAddress, m_PointButtonUp.x );

    if( a2 < a1 )
    {
        double t = a1;
        a1 = a2;
        a2 = t;
    }

    m_ZoomMinAddress = a1;
    m_ZoomMaxAddress = a2;
    m_ZoomAddressSet = true;
    m_ZoomCenterAddress = (m_ZoomMinAddress + m_ZoomMaxAddress)/2;
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CxToolDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    if( m_State == STATE_NULL )
    {
        if( m_rFull.PtInRect( point ) )
        {
            // Save position
            m_PointButtonDown   = point;
            m_PointButtonUp     = point;

            // Convert location to address & select entry by address
            double Address = PixelToAddress( m_rFull, m_FullMinAddress, m_FullMaxAddress, point.x );
            log_memory* pEntry = AddressToLogEntry( (u32)Address );
            m_pSelectedEntry = pEntry;
            pDoc->UpdateAllViews( this, CxToolDoc::HINT_SELECT_MEMBLOCK, (CObject*)pEntry );

            // Force redraw
            Invalidate();
        }

        if( m_rZoom.PtInRect( point ) )
        {
            // Save position
            m_PointButtonDown   = point;
            m_PointButtonUp     = point;

            // Convert location to address & select entry by address
            double Address = PixelToAddress( m_rZoom, m_ZoomMinAddress, m_ZoomMaxAddress, point.x );
            log_memory* pEntry = AddressToLogEntry( (u32)Address );
            m_pSelectedEntry = pEntry;
            pDoc->UpdateAllViews( this, CxToolDoc::HINT_SELECT_MEMBLOCK, (CObject*)pEntry );

            // Force redraw
            Invalidate();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnMButtonDown(UINT nFlags, CPoint point) 
{
	if( m_State == STATE_NULL )
    {
        if( m_rFull.PtInRect( point ) )
        {
            // Capture the mouse
            SetCapture();

            // Set state
            m_State             = STATE_SELECT_FULL;
            m_PointButtonDown   = point;
            m_PointButtonUp     = point;
            m_ZoomMinAddress    = PixelToAddress( m_rFull, m_FullMinAddress, m_FullMaxAddress, point.x );
            m_ZoomMaxAddress    = PixelToAddress( m_rFull, m_FullMinAddress, m_FullMaxAddress, point.x );
            m_ZoomAddressSet    = true;
            m_ZoomCenterAddress = (m_ZoomMinAddress + m_ZoomMaxAddress)/2;

            // Force redraw
            Invalidate();
        }

        if( m_rZoom.PtInRect( point ) )
        {
            // Capture the mouse
            SetCapture();

            // Set state
            m_State             = STATE_DRAG_ZOOM;
            m_PointButtonDown   = point;
            m_PointButtonUp     = point;
            m_OldZoomMinAddress = m_ZoomMinAddress;
            m_OldZoomMaxAddress = m_ZoomMaxAddress;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if( m_State == STATE_NULL )
    {
        if( m_rFull.PtInRect( point ) )
        {
            // Capture the mouse
            SetCapture();

            // Set state
            m_State             = STATE_SELECT_FULL;
            m_PointButtonDown   = point;
            m_PointButtonUp     = point;
            m_ZoomMinAddress    = PixelToAddress( m_rFull, m_FullMinAddress, m_FullMaxAddress, point.x );
            m_ZoomMaxAddress    = PixelToAddress( m_rFull, m_FullMinAddress, m_FullMaxAddress, point.x );
            m_ZoomAddressSet    = true;
            m_ZoomCenterAddress = (m_ZoomMinAddress + m_ZoomMaxAddress)/2;

            // Force redraw
            Invalidate();
        }

        if( m_rZoom.PtInRect( point ) )
        {
            // Capture the mouse
            SetCapture();

            // Set state
            m_State             = STATE_DRAG_ZOOM;
            m_PointButtonDown   = point;
            m_PointButtonUp     = point;
            m_OldZoomMinAddress = m_ZoomMinAddress;
            m_OldZoomMaxAddress = m_ZoomMaxAddress;
        }
    }
	CViewBase::OnRButtonDown(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnLButtonUp(UINT nFlags, CPoint point) 
{
    switch( m_State )
    {
    case STATE_SELECT_ZOOM:
        {
            ReleaseCapture();
            m_PointButtonUp = point;
//            MousePointsToZoomAddresses();
            m_State = STATE_NULL;
            Invalidate();
        }
        break;
    default:
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnMButtonUp(UINT nFlags, CPoint point) 
{
    switch( m_State )
    {
    case STATE_SELECT_FULL:
        {
            ReleaseCapture();
            m_PointButtonUp = point;
            MousePointsToZoomAddresses();
            m_State = STATE_NULL;
            Invalidate();
        }
        break;
    case STATE_DRAG_ZOOM:
        {
            ReleaseCapture();
            m_PointButtonUp = point;
            m_State = STATE_NULL;
        }
        break;
    default:
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnRButtonUp(UINT nFlags, CPoint point) 
{
    switch( m_State )
    {
    case STATE_SELECT_FULL:
        {
            ReleaseCapture();
            m_PointButtonUp = point;
            MousePointsToZoomAddresses();
            m_State = STATE_NULL;
            Invalidate();
        }
        break;
    case STATE_DRAG_ZOOM:
        {
            ReleaseCapture();
            m_PointButtonUp = point;
            m_State = STATE_NULL;
        }
        break;
    default:
        break;
    }
	CViewBase::OnRButtonUp(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnMouseMove(UINT nFlags, CPoint point) 
{
    switch( m_State )
    {
    case STATE_NULL:
        {
            TRACKMOUSEEVENT Event;
            Event.cbSize      = sizeof(Event);
            Event.dwFlags     = TME_HOVER;
            Event.hwndTrack   = GetSafeHwnd();
            Event.dwHoverTime = HOVER_DEFAULT;
//            _TrackMouseEvent( &Event );
        }
        break;
    case STATE_SELECT_FULL:
        {
            m_PointButtonUp = point;
            MousePointsToZoomAddresses();
            Invalidate();
        }
        break;
    case STATE_SELECT_ZOOM:
        {
            m_PointButtonUp = point;
//            MousePointsToZoomAddresses();
            Invalidate();
        }
        break;
    case STATE_DRAG_ZOOM:
        {
            m_PointButtonUp = point;
            double BytesPerPixel = (m_OldZoomMaxAddress - m_OldZoomMinAddress) / (m_rZoom.Width()-1);
            double Delta = (m_PointButtonDown.x - m_PointButtonUp.x) * BytesPerPixel;
            m_ZoomMinAddress = m_OldZoomMinAddress + Delta;
            m_ZoomMaxAddress = m_OldZoomMaxAddress + Delta;
            if( m_ZoomMinAddress < (double)m_FullMinAddress ) m_ZoomMinAddress = (double)m_FullMinAddress;
            if( m_ZoomMaxAddress < (double)m_FullMinAddress ) m_ZoomMaxAddress = (double)m_FullMinAddress;
            if( m_ZoomMinAddress > (double)m_FullMaxAddress ) m_ZoomMinAddress = (double)m_FullMaxAddress;
            if( m_ZoomMaxAddress > (double)m_FullMaxAddress ) m_ZoomMaxAddress = (double)m_FullMaxAddress;
            if( (m_ZoomMinAddress != m_OldZoomMinAddress) || (m_ZoomMaxAddress != m_OldZoomMaxAddress) )
                m_ZoomAddressSet    = true;

            Invalidate();
        }
        break;
    default:
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CViewMemoryGraph::OnMouseHover( WPARAM wParam, LPARAM lParam )
{
    bool    Valid = false;
    double  Address = 0;

    POINTS  Point = MAKEPOINTS(lParam);
    CPoint  point(Point.x, Point.y);

    if( m_rFull.PtInRect( point ) )
    {
        Address = PixelToAddress( m_rFull, m_FullMinAddress, m_FullMaxAddress, point.x );
        Valid = true;
    }
    else if( m_rZoom.PtInRect( point ) )
    {
        Address = PixelToAddress( m_rZoom, m_ZoomMinAddress, m_ZoomMaxAddress, point.x );
        Valid = true;
    }

    if( Valid )
    {
        log_memory* pEntry = AddressToLogEntry( (u32)Address );
        if( pEntry )
        {
//            CXTString sTemp( _T("Testing\nLines") );
//         m_TipWindow.SetTipText( _T("Title"), sTemp );
//            ClientToScreen( &point );
//         m_TipWindow.ShowTipWindow( point, this, TWS_XT_ALPHASHADOW|TWS_XT_DROPSHADOW, 2000 );
        }
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::ZoomIn( void )
{
    double Address = ((double)m_ZoomMaxAddress + (double)m_ZoomMinAddress)/2;
    double Delta   = ((double)m_ZoomMaxAddress - (double)m_ZoomMinAddress)/2;

    if( Delta < 256 )
        Delta = 256;

    m_ZoomMinAddress = Address - Delta * 0.75;
    m_ZoomMaxAddress = Address + Delta * 0.75;
    m_ZoomAddressSet = true;

    if( m_ZoomMinAddress < (double)m_FullMinAddress ) m_ZoomMinAddress = (double)m_FullMinAddress;
    if( m_ZoomMaxAddress > (double)m_FullMaxAddress ) m_ZoomMaxAddress = (double)m_FullMaxAddress;

    Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::ZoomOut( void )
{
    double Address = ((double)m_ZoomMaxAddress + (double)m_ZoomMinAddress)/2;
    double Delta   = ((double)m_ZoomMaxAddress - (double)m_ZoomMinAddress)/2;

    m_ZoomMinAddress = Address - Delta * 1.25;
    m_ZoomMaxAddress = Address + Delta * 1.25;
    m_ZoomAddressSet = true;

    if( m_ZoomMinAddress < (double)m_FullMinAddress ) m_ZoomMinAddress = (double)m_FullMinAddress;
    if( m_ZoomMaxAddress > (double)m_FullMaxAddress ) m_ZoomMaxAddress = (double)m_FullMaxAddress;

    Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CViewMemoryGraph::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    static int WheelPos = 0;

    WheelPos += zDelta;

    while( WheelPos <= -WHEEL_DELTA )
    {
        WheelPos += WHEEL_DELTA;
        ZoomOut();

    }
    while( WheelPos >= WHEEL_DELTA )
    {
        WheelPos -= WHEEL_DELTA;
        ZoomIn();
    }

	return CViewBase::OnMouseWheel(nFlags, zDelta, pt);
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    CxToolDoc* pDoc = GetDocument();
    ASSERT( pDoc );
	
    switch( nChar )
    {
    case VK_LEFT:
        // Left
        if( m_pSelectedEntry )
        {
            m_pSelectedEntry = pDoc->GetHeapState().GetPrevEntry( m_pSelectedEntry );
            pDoc->UpdateAllViews( this, CxToolDoc::HINT_SELECT_MEMBLOCK, (CObject*)m_pSelectedEntry );
            RedrawWindow();
        }
        break;
    case VK_RIGHT:
        // Right
        if( m_pSelectedEntry )
        {
            m_pSelectedEntry = pDoc->GetHeapState().GetNextEntry( m_pSelectedEntry );
            pDoc->UpdateAllViews( this, CxToolDoc::HINT_SELECT_MEMBLOCK, (CObject*)m_pSelectedEntry );
            RedrawWindow();
        }
        break;
    case 38:
        // Up
        ZoomIn();
        break;
    case 40:
        // Down
        ZoomOut();
        break;
    }

	CViewBase::OnKeyDown(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
    if( lHint == CxToolDoc::HINT_SELECT_MEMBLOCK )
    {
        log_memory* pNewEntry = (log_memory*)pHint;
        if( pNewEntry != m_pSelectedEntry )
        {
            m_pSelectedEntry = pNewEntry;
            RedrawWindow();
        }
    }

    if( (lHint == 0 ) ||
        (lHint == CxToolDoc::HINT_NEW_MEMORY_DATA) )
    {
        // If we don't have a timer then start one, this mechanism is used to limit
        // the number of updates reflected visually to 1 every 50ms
        if( m_UpdateTimerID == 0 )
        {
            m_UpdateTimerID = SetTimer( 1, 200, NULL );
            ASSERT( m_UpdateTimerID );
        }
    }

    if( lHint == CxToolDoc::HINT_REDRAW_MEMORY_GRAPH )
    {
        RedrawWindow();
    }
}

/////////////////////////////////////////////////////////////////////////////

void CViewMemoryGraph::OnTimer(UINT nIDEvent) 
{
    // Update timer message?
    if( nIDEvent == m_UpdateTimerID )
    {
        // Kill the timer
        VERIFY( KillTimer( m_UpdateTimerID ) );
        m_UpdateTimerID = 0;

        // Force redraw
        Invalidate();
    }

	CViewBase::OnTimer(nIDEvent);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CViewMemoryGraph::PreCreateWindow(CREATESTRUCT& cs) 
{
    cs.style |= WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
    cs.style &= ~WS_BORDER;
	
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
