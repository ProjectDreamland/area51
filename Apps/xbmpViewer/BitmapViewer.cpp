// BitmapViewer.cpp : implementation file
//

#include "stdafx.h"
#include "xbmpviewer.h"
#include "BitmapViewer.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitmapViewer

CBitmapViewer::CBitmapViewer()
{
    m_pBitmap		= NULL;
    m_Mip			= 0;
    m_hBitmap		= NULL;
    m_Alpha			= FALSE;
	m_TrackingMouse = FALSE;
}

CBitmapViewer::~CBitmapViewer()
{
    if( m_hBitmap )
    {
        ::DeleteObject( m_hBitmap );
        m_hBitmap = NULL;
    }
}

void CBitmapViewer::SetBitmap( xbitmap* pBitmap, s32 Mip )
{
    if( Mip > pBitmap->GetNMips() )
        Mip = pBitmap->GetNMips();

    // Set new bitmap pointer
    m_pBitmap = pBitmap;
    m_Mip     = Mip;

    // Delete the cached windows bitmap
    if( m_hBitmap )
    {
        ::DeleteObject( m_hBitmap );
        m_hBitmap = NULL;
    }
}

void CBitmapViewer::SetAlpha( void )
{
    m_Alpha = TRUE;
}

s32 CBitmapViewer::GetBitmapWidth( void )
{
    if( m_pBitmap )
        return m_pBitmap->GetWidth( m_Mip );
    else
        return 1;
}

s32 CBitmapViewer::GetBitmapHeight( void )
{
    if( m_pBitmap )
        return m_pBitmap->GetHeight( m_Mip );
    else
        return 1;
}

BEGIN_MESSAGE_MAP(CBitmapViewer, CWnd)
	//{{AFX_MSG_MAP(CBitmapViewer)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBitmapViewer message handlers

void CBitmapViewer::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
    CPaintDC* pDC = &dc;
    
    // Do we have a bitmap?
    if( m_pBitmap )
    {
        // Get reference to the xbitmap
        xbitmap& Bitmap = *m_pBitmap;

        // Get dimensions of mip
        s32 w  = Bitmap.GetWidth ( m_Mip );
        s32 pw = Bitmap.GetPWidth( m_Mip );
        s32 h  = Bitmap.GetHeight( m_Mip );

        // Do we have a windows bitmap cached?
        if( m_hBitmap == NULL )
        {
            // Get data from xbitmap
            void*   pImage      = (void*)Bitmap.GetPixelData( m_Mip );
            u8*     pNewImage   = NULL;

            // Display the alpha channel?
            if( m_Alpha )
            {
                // Create a 32bit RGB buffer with just alpha duplicated through RGBA
                pNewImage = (u8*)malloc( w*h*4 );
                ASSERT( pNewImage );

                for( s32 y=0 ; y<h ; y++ )
                {
                    for( s32 x=0 ; x<w ; x++ )
                    {
                        s32 iSrc = (x+y*pw)*4;
                        s32 iDst = (x+y* w)*4;
                        xcolor c = m_pBitmap->GetPixelColor( x, y, m_Mip );
                        pNewImage[iDst  ] = c.A;
                        pNewImage[iDst+1] = c.A;
                        pNewImage[iDst+2] = c.A;
                        pNewImage[iDst+3] = c.A;
                    }
                }

                pImage = (void*)pNewImage;
            }
            else
            {
                // Create a 32bit RGB buffer with no padding on rows
                pNewImage = (u8*)malloc( w*h*4 );
                ASSERT( pNewImage );

                for( s32 y=0 ; y<h ; y++ )
                {
                    for( s32 x=0 ; x<w ; x++ )
                    {
                        s32 iSrc = (x+y*pw)*4;
                        s32 iDst = (x+y* w)*4;
                        xcolor c = m_pBitmap->GetPixelColor( x, y, m_Mip );
                        pNewImage[iDst  ] = c.B;
                        pNewImage[iDst+1] = c.G;
                        pNewImage[iDst+2] = c.R;
                        pNewImage[iDst+3] = c.A;
                    }
                }

                pImage = (void*)pNewImage;
            }

            // Setup bitmap header
            BITMAPINFO bmi;
            bmi.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth           = w;
            bmi.bmiHeader.biHeight          = -h;
            bmi.bmiHeader.biPlanes          = 1;
            bmi.bmiHeader.biBitCount        = 32;
            bmi.bmiHeader.biCompression     = BI_RGB;
            bmi.bmiHeader.biSizeImage       = 0;
            bmi.bmiHeader.biXPelsPerMeter   = 1;
            bmi.bmiHeader.biYPelsPerMeter   = 1;
            bmi.bmiHeader.biClrUsed         = 0;
            bmi.bmiHeader.biClrImportant    = 0;

            // Create the bitmap
            m_hBitmap = ::CreateDIBitmap( pDC->GetSafeHdc(), &bmi.bmiHeader, CBM_INIT, pImage, &bmi, DIB_RGB_COLORS );
            ASSERT( m_hBitmap );

            // Free any memory
            if( pNewImage )
                free( pNewImage );
        }

        // Get client rect
        CRect r;
        GetClientRect( r );

        // Create back DC
        CDC BackDC;
        BackDC.CreateCompatibleDC( pDC );

        // Get a CBitmap to render with
        CBitmap* pWinBitmap = CBitmap::FromHandle( m_hBitmap );

        // Select Bitmap into back DC & Blit
        CBitmap* pOldBitmap = BackDC.SelectObject( pWinBitmap );

        // Set stretch mode and blit
        pDC->SetStretchBltMode( COLORONCOLOR );
        pDC->StretchBlt( r.left, r.top, r.Width(), r.Height(), &BackDC, 0, 0, w, h, SRCCOPY );

        // Restore bitmap to back dc
        BackDC.SelectObject( pOldBitmap );
    }
}

BOOL CBitmapViewer::OnEraseBkgnd(CDC* pDC) 
{
    // Don't erase the background
    return 1;
//	return CWnd::OnEraseBkgnd(pDC);
}

void CBitmapViewer::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Track mouse to get the leave event
	if( !m_TrackingMouse )
	{
		TRACKMOUSEEVENT ev;
		ev.cbSize = sizeof(TRACKMOUSEEVENT);
		ev.dwFlags = TME_LEAVE;
		ev.hwndTrack = GetSafeHwnd();
		ev.dwHoverTime = HOVER_DEFAULT;
		m_TrackingMouse = _TrackMouseEvent( &ev );
	}

	// Generate string and set into status bar
	if( m_pBitmap )
	{
		CRect cr;
		GetClientRect( &cr );

		CString s;
		int x = point.x;
		int y = point.y;

		x = x * m_pBitmap->GetWidth( m_Mip )  / cr.Width();
		y = y * m_pBitmap->GetHeight( m_Mip ) / cr.Height();
		xcolor Color = m_pBitmap->GetPixelColor( x, y, m_Mip );

		s.Format( "XY (%d,%d) RGBA (%d,%d,%d,%d)", x, y, Color.R, Color.G, Color.B, Color.A );
		g_pMainFrame->SetStatusPane( INDICATOR_COLOR, s );
	}

	CWnd::OnMouseMove(nFlags, point);
}

LRESULT CBitmapViewer::OnMouseLeave(WPARAM w, LPARAM l)
{
	m_TrackingMouse = FALSE;
	g_pMainFrame->SetStatusPane( INDICATOR_COLOR, "" );

    return 0;
}

