// PreviewBar.cpp : implementation file
//

#include "stdafx.h"
#include "xbmpViewer.h"
#include "PreviewBar.h"
#include "aux_bitmap.hpp"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewBar

CPreviewBar::CPreviewBar()
{
    m_pBitmap = NULL;
}

CPreviewBar::~CPreviewBar()
{
    if( m_pBitmap )
        delete m_pBitmap;
}

BEGIN_MESSAGE_MAP(CPreviewBar, CXTDockWindow)
    //{{AFX_MSG_MAP(CPreviewBar)
    ON_WM_CREATE()
    ON_WM_WINDOWPOSCHANGED()
    ON_WM_SIZE()
    ON_WM_HSCROLL()
    ON_MESSAGE( NM_NEWBITMAP, OnNewBitmap )
    ON_MESSAGE( NM_NEWMIPLEVEL, OnNewMipLevel )
    ON_WM_VSCROLL()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CExplorerBar message handlers

int CPreviewBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CXTDockWindow::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    // TODO: Add your specialized creation code here
    m_wndBitmap1.CreateEx( WS_EX_CLIENTEDGE, NULL, NULL, WS_VISIBLE|WS_CHILD, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST );
    m_wndBitmap2.CreateEx( WS_EX_CLIENTEDGE, NULL, NULL, WS_VISIBLE|WS_CHILD, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST+1 );
    m_wndBitmap2.SetAlpha();

    m_wndMip.Create( WS_VISIBLE|WS_CHILD|TBS_AUTOTICKS|TBS_VERT|TBS_BOTH|TBS_TOOLTIPS, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST+2 );
    m_wndMip.SetRange( 0, 4 );
//    m_wndMip.

    return 0;
}

void CPreviewBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
    CXTDockWindow::OnWindowPosChanged(lpwndpos);
    
    CRect rc;
    GetInsideRect( rc );
    rc.DeflateRect( 1, 1 );
}

LRESULT CPreviewBar::OnNewBitmap( WPARAM wParam, LPARAM lParam )
{
    const char* pPath = (const char*)wParam;
    file_rec* pFile = (file_rec*)lParam;

    // Early out
    if( m_Path == pPath )
        return 0;

    // Get Size String
    xstring SizeString = xfs("%d", pFile->Size/1024);
    xstring SizeString2;
    s32 c = SizeString.GetLength();
    for( s32 j=0 ; j<SizeString.GetLength() ; j++ )
    {
        SizeString2 += SizeString[j];
        c--;
        if( ((c % 3) == 0) && (c>0) )
            SizeString2 += ',';
    }
    SizeString2 += " KB";

    // Set status string
    CString s, s1, s2;
    s1.Format( _T("%s (%s)"), (const char*)pFile->Name, (const char*)SizeString2 );
    if( pFile->GotInfo )
        s2.Format( _T(" (%dx%dx%db) (%d %s) (%s)"), pFile->Width, pFile->Height, pFile->BitDepth,
                                                    pFile->nMips, (pFile->nMips==1)?_T("mip"):_T("mips"),
                                                    (const char*)pFile->Format );
    s = "   " + s1 + s2 + "   ";
    g_pMainFrame->SetStatusPane( INDICATOR_FOCUS, s );

    // Load if different bitmap
    if( x_strcmp( pPath, m_Path ) != 0 )
    {
        // Set path
        m_Path = pPath;

        if( m_pBitmap )
            delete m_pBitmap;

        m_pBitmap = new xbitmap;
        ASSERT( m_pBitmap );
        auxbmp_Load( *m_pBitmap, m_Path );

        if( m_pBitmap->GetFlags() & xbitmap::FLAG_GCN_DATA_SWIZZLED )
        {
            m_pBitmap->GCNUnswizzleData();
        }

        switch( m_pBitmap->GetFormat() )
        {
            case xbitmap::FMT_DXT1:
            case xbitmap::FMT_DXT3:
            case xbitmap::FMT_DXT5:
                auxbmp_Decompress( *m_pBitmap );
                break;
        }
    }

//    m_pBitmap->ConvertFormat( xbitmap::FMT_32_ARGB_8888 );

    // TODO: save off if the bitmap is palettized or has an alpha channel
//    s32 nMips = m_pBitmap->GetNMips();
//    m_pBitmap->ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
//    m_pBitmap->BuildMips( nMips );

    m_wndMip.SetRangeMax( m_pBitmap->GetNMips(), TRUE );
    m_wndBitmap1.SetBitmap( m_pBitmap, m_wndMip.GetPos() );
    m_wndBitmap2.SetBitmap( m_pBitmap, m_wndMip.GetPos() );
    RepositionWindows();
    m_wndBitmap1.RedrawWindow();
    m_wndBitmap2.RedrawWindow();

    // Wake the thread that reads the bitmap
//    s_MessageQueue.Send( (void*)1, MQ_BLOCK );

    return 0;
}

LRESULT CPreviewBar::OnNewMipLevel( WPARAM wParam, LPARAM lParam )
{
    m_wndBitmap1.SetBitmap( m_pBitmap, wParam );
    m_wndBitmap2.SetBitmap( m_pBitmap, wParam );
    RepositionWindows();
    m_wndBitmap1.RedrawWindow();
    m_wndBitmap2.RedrawWindow();

    return 0;
}

void CPreviewBar::RepositionWindows( void )
{
    if( m_pBitmap == NULL )
    {
        return;
    }

    CRect r;
    GetInsideRect( r );

    // Get rectangles
    CRect r1 = r;
    CRect r2 = r;
    CRect r3 = r;
    bool Vertical = false;
    if( r.Width() >= r.Height() )
    {
        r1.right = r1.left  + (r1.Width()-50)/2;
        r2.left  = r2.right - (r2.Width()-50)/2;
        r3.left  = r1.right;
        r3.right = r2.left;
        r3.DeflateRect( 4, 4, 4, 4 );
        Vertical = true;
    }
    else
    {
        r1.bottom = r1.top    + (r1.Height()-50)/2;
        r2.top    = r2.bottom - (r2.Height()-50)/2;
        r3.top    = r1.bottom;
        r3.bottom = r2.top;
        r3.DeflateRect( 4, 4, 4, 4 );
        Vertical  = false;
    }

    // Get bitmap rectangle (constrained to r1)
    s32 w = r1.Width();
    s32 h = r1.Height();
    f32 Aspect = (f32)m_wndBitmap1.GetBitmapWidth() / (f32)m_wndBitmap1.GetBitmapHeight();
    s32 w1 = (s32)(h * Aspect);
    s32 h1 = h;
    if( w1 > w )
    {
        w1 = w;
        h1 = (s32)(w / Aspect);
    }
    
    r1.SetRect( r1.left+(r1.Width()-w1)/2, r1.top+(r1.Height()-h1)/2, r1.left+(r1.Width()-w1)/2+w1, r1.top+(r1.Height()-h1)/2+h1 );
    r2.SetRect( r2.left+(r2.Width()-w1)/2, r2.top+(r2.Height()-h1)/2, r2.left+(r2.Width()-w1)/2+w1, r2.top+(r2.Height()-h1)/2+h1 );

    m_wndBitmap1.MoveWindow( &r1 );
    m_wndBitmap2.MoveWindow( &r2 );

    if( m_wndMip.GetSafeHwnd() )
    {
        m_wndMip.ModifyStyle( TBS_VERT|TBS_HORZ, Vertical ? TBS_VERT : TBS_HORZ );
        m_wndMip.MoveWindow( &r3 );
    }

    RedrawWindow();
}

void CPreviewBar::OnSize(UINT nType, int cx, int cy) 
{
    CXTDockWindow::OnSize(nType, cx, cy);
    
    // TODO: Add your message handler code here
    RepositionWindows( );
}

void CPreviewBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
    if( pSlider == &m_wndMip )
    {
        OnNewMipLevel( pSlider->GetPos(), 0 );
    }

    CXTDockWindow::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPreviewBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
    if( pSlider == &m_wndMip )
    {
        OnNewMipLevel( pSlider->GetPos(), 0 );
    }
    
    CXTDockWindow::OnVScroll(nSBCode, nPos, pScrollBar);
}
