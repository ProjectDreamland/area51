// Ruler.cpp : implementation file
//

#include "stdafx.h"
#include "Ruler.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

#define MIN_PIXELS_PER_TICK 100

/////////////////////////////////////////////////////////////////////////////

CRuler::CRuler()
{
    // Defaults
    m_Units = UNITS_INTEGER;

	// Create font.
    m_Font.CreatePointFont( 80, _T("MS Serif") );
}

/////////////////////////////////////////////////////////////////////////////

CRuler::~CRuler()
{
}

/////////////////////////////////////////////////////////////////////////////

void CRuler::PrettyInt( CString& String, int Number )
{
    ASSERT( Number < (INT_MAX/10) );

    String.Empty();

    // Determine Order of number
    int Order    = 0;
    int Divisor  = 1;
    while( Divisor <= Number )
    {
        Order++;
        Divisor *= 10;
    }
    Divisor /= 10;
    Order--;

    // Generate digits
    while( Order >= 0 )
    {
        int Value = Number / Divisor;
        String += (char)('0'+Value);
        if( Order && ((Order % 3) == 0) )
            String += ',';
        Number -= Value * Divisor;
        Divisor /= 10;
        Order--;
    }

    // Zero if nothing in string
    if( String.GetLength() == 0 )
        String = _T("0");
}

/////////////////////////////////////////////////////////////////////////////

void CRuler::PrettyFloat( CString& String, double Number, int nFractional )
{
    // Format number into string
    String.Format( _T("%.*f"), nFractional, Number );

    // Scan string and insert thousand seperators
    int i = String.Find( '.' );
    int nDigits = 1;
    while( --i > 0 )
    {
        if( (nDigits++ % 3) == 0)
        {
            String.Insert( i, ',' );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CRuler::PrettySeconds( CString& String, double Seconds, int nFractional )
{
    // Format number into string
    String.Format( _T("%.*f"), nFractional, Seconds );

    // Scan string and insert thousand seperators
    int i = String.Find( '.' );
    int nDigits = 1;
    while( --i > 0 )
    {
        if( (nDigits++ % 3) == 0)
        {
            String.Insert( i, ',' );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CRuler::DrawRuler( CDC* pDC, CRect& r, double Start, double Scale )
{
    double TickOrder = .001;
    double TickStep  = 1;

    ASSERT( Scale > 0.0f );

    // Determine label scale
    while( TickOrder < 1000000 )
    {
        double PixelsPerTick = TickOrder / Scale;

        if( (PixelsPerTick * 1.0f) > MIN_PIXELS_PER_TICK )
        {
            TickStep = (TickOrder * 1.0f);
            break;
        }
        else if( (PixelsPerTick * 2.0f) > MIN_PIXELS_PER_TICK )
        {
            TickStep = (TickOrder * 1.0f);
            break;
        }
        else if( (PixelsPerTick * 5.0f) > MIN_PIXELS_PER_TICK )
        {
            TickStep = (TickOrder * 5.0f);
            break;
        }
        else
        {
            TickOrder *= 10.0f;
            TickStep = TickOrder;
        }
    }

    // Create pen to render the ruler
    CPen Pen( PS_SOLID, 1, RGB(0,0,0) );
    CPen* pOldPen = pDC->SelectObject( &Pen );

    // Setup for text rendering
    CFont* pOldFont = pDC->SelectObject( &m_Font );
    pDC->SetTextColor( RGB(0,0,0) );
    pDC->SetBkMode   ( TRANSPARENT );

    // Clear background
    pDC->FillSolidRect( r.left, r.top, r.Width(), r.Height(), xtAfxData.clr3DFace );
    pDC->MoveTo( r.left , r.bottom-1 );
    pDC->LineTo( r.right, r.bottom-1 );

    // Determine first tick to render
    double FirstTick        = Start - fmod( Start, TickStep );

    // Setup for drawing ticks
    double  u  = FirstTick;
    int     y1 = r.bottom -  1;
    int     y2 = r.bottom -  5;
    int     y3 = r.bottom -  7;
    int     y4 = r.bottom - 11;
    double  dx = (TickStep / 10.0f) / Scale;
    double  du = TickStep;
    double  x  = (double)r.left - (Start / Scale) + (FirstTick / Scale);

    // Loop until past end of window
    CString String;
    while( x < (double)r.right )
    {
        // Draw large tick
        pDC->MoveTo( (int)x, y1 );
        pDC->LineTo( (int)x, y4 );

        // Draw text
        switch( m_Units )
        {
        case UNITS_INTEGER:
            PrettyInt( String, (int)u );
            break;
        case UNITS_FLOAT:
            PrettyFloat( String, u, 3 );
            break;
        case UNITS_SECONDS:
            PrettySeconds( String, u, 3 );
            break;
        default:
            ASSERT( 0 );
        }
            
        pDC->TextOut( (int)x+1, r.top + 1, (const char*)String, String.GetLength() );

        // Loop through small ticks
        for( int i=0 ; i<10 ; i++ )
        {
            // Draw small tick
            pDC->MoveTo( (int)x, y1 );

            if( i != 5 )
                pDC->LineTo( (int)x, y2 );
            else
                pDC->LineTo( (int)x, y3 );

            x += dx;
        }

        u += du;
    }

    // Select old objects back into DC
    pDC->SelectObject( pOldFont );
    pDC->SelectObject( pOldPen  );
}

/////////////////////////////////////////////////////////////////////////////
