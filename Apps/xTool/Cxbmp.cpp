// Cxbmp.cpp : implementation file
//

#include "stdafx.h"
#include "Cxbmp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

Cxbmp::Cxbmp()
{
    m_Width     = 0;
    m_Height    = 0;
    m_pBitmap   = NULL;
}

/////////////////////////////////////////////////////////////////////////////

Cxbmp::~Cxbmp()
{
    FreeBitmap();
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::FreeBitmap( void )
{
    if( m_pBitmap )
    {
        delete m_pBitmap;
        m_pBitmap = NULL;
        m_pData = NULL;
        m_Width = 0;
        m_Height = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::AllocateBitmap( void )
{
    m_pData = (u32*)x_malloc( m_Width * m_Height * sizeof(xcolor) );
    m_pBitmap = new xbitmap;
    ASSERT( m_pBitmap );
    m_pBitmap->Setup( xbitmap::FMT_32_ARGB_8888, m_Width, m_Height, TRUE, (byte*)m_pData );
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::SetSize( s32 Width, s32 Height )
{
    // Check if it's already that size
    if( (Width != m_Width) || (Height != m_Height) )
    {
        FreeBitmap();

        m_Width  = Width;
        m_Height = Height;

        AllocateBitmap();
    }
}

/////////////////////////////////////////////////////////////////////////////

xbitmap* Cxbmp::GetBitmap( void )
{
    return m_pBitmap;
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::Clear( xcolor Color )
{
    u32* pData = m_pData;
    u32  c     = (u32)Color;
    s32  Count = m_Width * m_Height;
    while( Count-- > 0 )
        *pData++ = c;
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::HorizontalLine( s32 y, s32 x1, s32 x2, xcolor Color )
{
    // Trivial reject
    if( (y >= 0) && (y < m_Height) )
    {
        // Order x's
        if( x1 > x2 )
        {
            s32 t = x1;
            x1 = x2;
            x2 = t;
        }

        // Clip
        if( x1 < 0         ) x1 = 0;
        if( x2 > m_Width-1 ) x2 = m_Width-1;

        // Setup
        u32  c     = (u32)Color;
        u32* pData = &m_pData[x1 + y*m_Width];
        s32  Count = x2-x1+1;

        // Fill
        while( Count-- > 0 )
        {
            *pData++ = c;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::VerticalLine( s32 x, s32 y1, s32 y2, xcolor Color )
{
    u32 c = (u32)Color;

    // Trivial reject
    if( (x >= 0) && (x<m_Width) )
    {
        // Order y's
        if( y1 > y2 )
        {
            s32 t = y1;
            y1 = y2;
            y2 = t;
        }

        // Clip
        if( y1 < 0          ) y1 = 0;
        if( y2 > m_Height-1 ) y2 = m_Height-1;

        // Setup
        u32* pData = &m_pData[x + y1*m_Width];
        s32  Count = y2-y1+1;

        // Fill
        while( Count-- > 0 )
        {
            *pData = c;
            pData += m_Width;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::SolidRect( const CRect& r, xcolor Color )
{
    // Trivial reject
    if( (r.left   <  m_Width ) &&
        (r.right  >= 0       ) &&
        (r.top    <  m_Height) &&
        (r.bottom >= 0       ) )
    {
        // Clip rect
        CRect cr = r;
        if( cr.left   < 0          ) cr.left   = 0;
        if( cr.right  > m_Width-1  ) cr.right  = m_Width-1;
        if( cr.top    < 0          ) cr.top    = 0;
        if( cr.bottom > m_Height-1 ) cr.bottom = m_Height-1;

        // Setup for fill
        u32  cFill   = (u32)Color;
        u32* pLine   = &m_pData[cr.left + cr.top*m_Width];
        s32  rWidth  = cr.Width();
        s32  rHeight = cr.Height();

        // Fill Loop
        while( rHeight-- > 0 )
        {
            // Setup for span
            u32* pData  = pLine;
            pLine      += m_Width;
            s32  Count  = rWidth;

            // Fill span
            while( Count-- > 0 )
                *pData++ = cFill;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::FramedRect( const CRect& r, xcolor ColorFill, xcolor ColorOutline )
{
    // Draw solid part
    CRect cr = r;
    cr.DeflateRect( 1, 1, 1, 1 );
    SolidRect( cr, ColorFill );

    // Draw outline lines
    VerticalLine  ( r.left    , r.top , r.bottom-1, ColorOutline );
    VerticalLine  ( r.right-1 , r.top , r.bottom-1, ColorOutline );
    HorizontalLine( r.top     , r.left, r.right-1 , ColorOutline );
    HorizontalLine( r.bottom-1, r.left, r.right-1 , ColorOutline );
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::CopyLine( s32 ySrc, s32 yDst )
{
    ASSERT( (ySrc >= 0) && (ySrc < m_Height) );
    ASSERT( (yDst >= 0) && (yDst < m_Height) );
    ASSERT( ySrc != yDst );

    u32* pSrc = &m_pData[ySrc*m_Width];
    u32* pDst = &m_pData[yDst*m_Width];

    s32 Count = m_Width;

    while( Count-- > 0 )
        *pDst++ = *pSrc++;
}

/////////////////////////////////////////////////////////////////////////////

void Cxbmp::Draw( CDC* pDC, CRect& r )
{
    if( m_pBitmap )
    {
        // Set stretch mode
        pDC->SetStretchBltMode( COLORONCOLOR );

        // Get data from xbitmap
        xbitmap& Bitmap = *m_pBitmap;
        void* pImage = (void*)Bitmap.GetPixelData();

        // Create a windows bitmap with it
        CBitmap WinBitmap;
        WinBitmap.CreateBitmap( Bitmap.GetWidth(), Bitmap.GetHeight(), 1, 32, pImage );

        // Create back DC
        CDC BackDC;
        BackDC.CreateCompatibleDC( pDC );

        // Select Bitmap into back DC & Blit
        CBitmap* pOldBitmap = BackDC.SelectObject( &WinBitmap );

//        if( (Bitmap.GetWidth() != r.Width()) || (Bitmap.GetHeight() != r.Height()) )
        {
            pDC->StretchBlt( r.left, r.top             , r.Width(), 1           , &BackDC, 0, 0, Bitmap.GetWidth(), 1, SRCCOPY );
            pDC->StretchBlt( r.left, r.top+1           , r.Width(), r.Height()-2, &BackDC, 0, 1, Bitmap.GetWidth(), 1, SRCCOPY );
            pDC->StretchBlt( r.left, r.top+r.Height()-1, r.Width(), 1           , &BackDC, 0, 0, Bitmap.GetWidth(), 1, SRCCOPY );
        }
//        else
//            pDC->BitBlt( r.left, r.top, Bitmap.GetWidth(), Bitmap.GetHeight(), &BackDC, 0, 0, SRCCOPY );

        // Restore bitmap to back dc
        BackDC.SelectObject( pOldBitmap );
    }
}

/////////////////////////////////////////////////////////////////////////////
