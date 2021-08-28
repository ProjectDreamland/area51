//=========================================================================
// SOUNDVIEW.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdafx.h"
#include <math.h>
#include "Resource.h"
#include "SoundDoc.h"
#include "SoundView.h"
#include "AudioEditorFrame.h"
#include "DoubleBuffer.h"
#include "AudioEditorView.h"
#include "..\Editor\UserMessage.h"
#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\Editor\Project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//=========================================================================

#define WAVE_FORM_HEIGHT        100
#define MIN_PIXELS_PER_TICK     100
#define SAMPLES_PER_5_MS        110.25f
#define TIME_BAR_HEIGHT         16
#define MARKER_BAR_HEIGHT       16
#define TIME_PER_PIXEL          (1.0f/100000.0f)

IMPLEMENT_DYNCREATE(CSoundView, CView)

//=========================================================================
// MESSAGE PUMP
//=========================================================================
BEGIN_MESSAGE_MAP(CSoundView, CView)
	//{{AFX_MSG_MAP(CSoundView)
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
    ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_COMMAND( ID_SOUND_ZOOM_IN,               OnSoundZoomIn           )
	ON_COMMAND( ID_SOUND_ZOOM_IN_FULL,          OnSoundZoomInFull       )
	ON_COMMAND( ID_SOUND_ZOOM_OUT,              OnSoundZoomOut          )
	ON_COMMAND( ID_SOUND_ZOOM_OUT_FULL,         OnSoundZoomOutFull      )
	ON_COMMAND( ID_SOUND_ZOOM_SELECTION,        OnSoundZoomSelection    )
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
    ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

//=========================================================================

CSoundView::CSoundView()
{
    m_ZoomFactor            = 1;
    m_MoveSample            = FALSE;
    m_SampleOffsetMouseUp   = 0;
    m_PreferredZoom         = 1;
}

//=========================================================================

CSoundView::~CSoundView()
{
}

//=========================================================================

int CSoundView::GetClientWidth( void )
{
	CRect r;
    GetClientRect( &r );
    return r.Width();
}

//=========================================================================

int CSoundView::GetClientHeight( void )
{
	CRect r;
    GetClientRect( &r );
    return r.Height();
}

//=========================================================================

BOOL CSoundView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CView::PreCreateWindow(cs))
		return FALSE;

    cs.style |= WS_HSCROLL;
    cs.style |= WS_VSCROLL;

	return TRUE;
}

//=========================================================================

int CSoundView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

//=========================================================================

void CSoundView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	// Set the font.
    m_Font.CreatePointFont( 80, "MS Serif" );
	SetFont( &m_Font );

}

//=========================================================================

static CString PrettyNumber( int Number )
{
    CString String;

    ASSERT( Number < (INT_MAX/10) );

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
        String = "0";

    return String;
}

//=========================================================================

static CString PrettyFloat( float Number )
{
    xstring String = (const char*)xfs( "%f", Number );
    
    s32 Index           = 0;
    s32 LastZeroIndex   = -1;
    s32 DecimalIndex    = String.Find( '.' );
    
    while( Index < String.GetLength() )
    {
        if( (String[Index] != '0') && (String[Index] != '.') )
            LastZeroIndex = -1;

        if( ((String[Index] == '0') || (String[Index] == '.')) && (LastZeroIndex == -1) )
            LastZeroIndex = Index;

        Index++;
    }
    
    if( (LastZeroIndex != -1) && (LastZeroIndex <= DecimalIndex) )
        LastZeroIndex = DecimalIndex;

    if( LastZeroIndex != -1 )
        return (const char*)String.Left( LastZeroIndex );
    else
        return (const char*)String;
}

//=========================================================================

void CSoundView::DrawRuler( CDC* pDC, CRect& r )
{
    f32 TickOrder   = 1;
    f32 TickStep    = 1;
    CSoundDoc* pDoc = GetDocument();

    f32 UnitType    = MIN_PIXELS_PER_TICK;

    if( pDoc->m_SoundUnits == CSoundDoc::SECONDS )
    {
        TickOrder   = ((f32)m_ZoomFactor*TIME_PER_PIXEL);
    }

    // Determine label scale
    while( TickOrder < 1000000 )
    {
        f32 PixelsPerTick = TickOrder / m_ZoomFactor;

        if( (PixelsPerTick * 1.0f) > UnitType )
        {
            TickStep = (TickOrder * 1.0f);
            break;
        }
        else if( (PixelsPerTick * 2.0f) > UnitType )
        {
            TickStep = (TickOrder * 1.0f);
            break;
        }
        else if( (PixelsPerTick * 5.0f) > UnitType )
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
    pDC->SelectObject( &m_Font );
    pDC->SetTextColor( RGB(0,0,0) );
    pDC->SetBkMode   ( TRANSPARENT );

    // Clear background
    pDC->FillSolidRect( r.left, r.top, r.Width(), r.Height(), xtAfxData.clr3DFace );
    pDC->MoveTo( r.left , r.bottom-1 );
    pDC->LineTo( r.right, r.bottom-1 );

    // Determine first tick to render
    f32 FirstVisibleUnit = (f32)(GetScrollPos( SB_HORZ ));
    f32 FirstTick        = (f32)(FirstVisibleUnit - fmod( FirstVisibleUnit, TickStep ));

    // Setup for drawing ticks
    f32 u  = FirstTick*TIME_PER_PIXEL;
    f32 du = TickStep*TIME_PER_PIXEL;

    s32 y1 = r.bottom -  1;
    s32 y2 = r.bottom -  5;
    s32 y3 = r.bottom -  7;
    s32 y4 = r.bottom - 11;
    f32 dx = (TickStep / 10.0f) / m_ZoomFactor;
    f32 x  = (f32)r.left - GetScrollPos( SB_HORZ ) / m_ZoomFactor + FirstTick / m_ZoomFactor;

    // Loop until past end of window
    while( x < (f32)r.right )
    {
        // Draw large tick
        pDC->MoveTo( (s32)x, y1 );
        pDC->LineTo( (s32)x, y4 );

        // Draw text
        CString String;
        String = PrettyFloat( u );

        pDC->TextOut( (int)x+1, 1, (const char*)String, String.GetLength() );

        // Loop through small ticks
        for( s32 i=0 ; i<10 ; i++ )
        {
            // Draw small tick
            pDC->MoveTo( (s32)x, y1 );

            if( i != 5 )
                pDC->LineTo( (s32)x, y2 );
            else
                pDC->LineTo( (s32)x, y3 );

            x += dx;
        }

        u += du;
    }
    
    // Select old pen back into DC
    pDC->SelectObject( pOldPen );
}


//=========================================================================

void CSoundView::DrawMarker( CDC* pDC, CRect& r )
{
    // Create pen to render the ruler
    CPen Pen( PS_SOLID, 1, RGB(1,0,0) );
    CPen* pOldPen = pDC->SelectObject( &Pen );

    // Clear background
    pDC->FillSolidRect( r.left, r.top, r.Width(), r.Height(), xtAfxData.clr3DFace );
    pDC->MoveTo( r.left , r.bottom-1 );
    pDC->LineTo( r.right, r.bottom-1 );
    
    // Draw the markers

    // Select old pen back into DC
    pDC->SelectObject( pOldPen );
}

//=========================================================================

void CSoundView::OnDraw(CDC* pDC)
{
	CSoundDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    
    s32 Offset;

    // Does the sample even exist.
    if( GetDocument()->m_SoundFileCount > 0 )
        Offset = (s32)((pDoc->m_pOffset[GetDocument()->m_MaxTimeIndex]/m_ZoomFactor)/TIME_PER_PIXEL);
    else
        Offset = 0;

    // Double buffer the DC
    CDoubleBuffer db( pDC, Offset );

    // Create pen to render the waveform
    CPen Pen    ( PS_SOLID, 1, RGB(0,0,0) );    
    CPen Red    ( PS_SOLID, 1, RGB(255,0,0) );
    CPen Yellow ( PS_SOLID, 1, RGB(0,255,255) );

    CPen* pOldPen = pDC->SelectObject( &Pen );
    
    m_ClientRect = db.GetRect();

    SCROLLINFO Info;
    GetScrollInfo( SB_VERT, &Info );

    s32 RectTop = 0;
    
    if( Info.nPos > (MARKER_BAR_HEIGHT) )
        RectTop -= Info.nPos;
    else
        RectTop = (MARKER_BAR_HEIGHT) - Info.nPos;
    
    s32 i = 0;
    for( ; i < pDoc->m_SoundFileCount; i++ )
    {
        // Go to the next sample
        if( !pDoc->GetIsLoaded( i ) )
            continue;

        if( pDoc->GetNumChannels(i) == 1 )
        {
            // Get rectangle for ruler & left & right views
            CRect r         = db.GetRect();
            r     .top      = RectTop;
            r     .bottom   = r.top + WAVE_FORM_HEIGHT;
            
            s32 Offset = (s32)((pDoc->m_pOffset[i]/m_ZoomFactor)/TIME_PER_PIXEL);            
            if( !CheckElementOffset( Offset, i ) )
                Offset = 0;

            // Setup
            s32     NumSamples  = pDoc->GetNumSamples(i);
            s16*    pSamples    = pDoc->GetChannelData( 0, i );
            s32     iFirst      = GetScrollPos( SB_HORZ );
            s32     Start       = max( Offset-(iFirst/m_ZoomFactor), 0 );
            
            xarray<aiff_file::breakpoint> SampleBreakPoints;
            pDoc->GetBreakPoints( SampleBreakPoints, i );

            f32     BeginTime   = max( (f32)(iFirst*TIME_PER_PIXEL)-pDoc->m_pOffset[i], 0.0f );
            f32     DeltaTime   = (f32)m_ZoomFactor*TIME_PER_PIXEL;
            f32     CurrentTime = BeginTime;

            // Is the wav selected.
            if( pDoc->m_SampleSelected == i )
                pDC->FillSolidRect( r.left+Start, r.top, r.Width(), r.Height(), RGB(150,150,255) );
            else
                pDC->FillSolidRect( r.left+Start, r.top, r.Width(), r.Height(), RGB(255,255,255) );

            pDC->FillSolidRect( 0, r.top, r.left + Start, r.Height(), RGB(192,192,192) );

            // Draw seperator
            pDC->MoveTo( r.right, r.bottom-1 );
            pDC->LineTo( r.left , r.bottom-1 );
            
            // Draw the waveform
            for( int x=Start ; x<db.w+Start ; x++ )
            {
                iFirst = (s32)(CurrentTime*(f32)pDoc->GetSampleRate( i ));
               
                CurrentTime += DeltaTime;    
                // Get index of last sample for x position
                s32 iLast  = min( (s32)(CurrentTime*(f32)pDoc->GetSampleRate( i )), NumSamples );

                // Exit if past data
                if( iFirst > NumSamples )
                {
                    // Fill remainder of window
                    pDC->FillSolidRect( x, r.top, r.right - x, r.Height(), RGB(192,192,192) );
                    break;
                }

                // Find min and max of samples for current x position
                s16 s   = pSamples[iFirst];
                s32 Min = s;
                s32 Max = s;
                while( ++iFirst < iLast )
                {
                    s = pSamples[iFirst];
                    if( s < Min ) Min = s;
                    if( s > Max ) Max = s;
                }

                // Draw lines to fill in the data
                s32 yMin = r.bottom - r.Height()/2 + Min * r.Height() / 65536;
                s32 yMax = r.bottom - r.Height()/2 + Max * r.Height() / 65536;
                s32 yAvg = (yMin + yMax) / 2;

                if( x == Start )
                    pDC->MoveTo( x, yAvg );
                else
                    pDC->LineTo( x, yAvg );

                if( m_ZoomFactor > 1 )
                {
                    pDC->LineTo( x, yMin );
                    pDC->LineTo( x, yMax );
                    pDC->MoveTo( x, yAvg );
                }
            }
            
            // The the max time availabel in the view.
            f32 EndTime    = BeginTime + DeltaTime*(x-Offset);
            iFirst         = GetScrollPos( SB_HORZ );
            CPen* pPrevPen;
            
            // Change the pen if the sample is selected.
            if( pDoc->m_SampleSelected == i )
                pPrevPen = pDC->SelectObject( &Yellow );
            else
                pPrevPen = pDC->SelectObject( &Red );

            for( s32 j = 0; j < SampleBreakPoints.GetCount(); j++ )
            {
                // Is this maker in view.
                if( (SampleBreakPoints[j].Position >= BeginTime) && (SampleBreakPoints[j].Position <= EndTime) )
                {
                    // Get the Draw Pixel, take the scroll and the complex effect offset in to consideration.
                    s32 MarkerPixel = ((s32)(SampleBreakPoints[j].Position/DeltaTime)+Offset)-(iFirst/m_ZoomFactor);
                    pDC->MoveTo( MarkerPixel, r.top );
                    pDC->LineTo( MarkerPixel, r.bottom );
                }
            }
            
            pDC->SelectObject( pPrevPen );
            RectTop += WAVE_FORM_HEIGHT;
        }
        else if( pDoc->GetNumChannels(i) == 2 )
        {
            // Get rectangle for ruler & left & right views
            //CRect rRuler = db.GetRect();
            CRect rLeft  = db.GetRect();
            CRect rRight = db.GetRect();

            rLeft .top      = RectTop;
            rLeft .bottom   = rLeft.top + WAVE_FORM_HEIGHT;
            rRight.top      = rLeft.bottom;
            rRight.bottom   = rRight.top + WAVE_FORM_HEIGHT;

            s32 Offset = (s32)((GetDocument()->m_pOffset[i]/m_ZoomFactor)/TIME_PER_PIXEL);            
            if( CheckElementOffset( Offset, i ) )
                Offset = 0;

            // Clear waveform background
/*            if( pDoc->m_SampleSelected == i )
            {
                pDC->FillSolidRect( rLeft .left + Start, rLeft .top, rLeft .Width(), rLeft .Height(), RGB(150,150,255) );
                pDC->FillSolidRect( rRight.left + Start, rRight.top, rRight.Width(), rRight.Height(), RGB(150,150,255) );
            }
            else
            {
                pDC->FillSolidRect( rLeft .left + Start, rLeft .top, rLeft .Width(), rLeft .Height(), RGB(255,255,255) );
                pDC->FillSolidRect( rRight.left + Start, rRight.top, rRight.Width(), rRight.Height(), RGB(255,255,255) );
            }

            // Draw seperator
            pDC->MoveTo( rLeft.right, rLeft.bottom );
            pDC->LineTo( rLeft.left , rLeft.bottom );

            // Draw seperator
            pDC->MoveTo( rRight.right, rRight.bottom-1 );
            pDC->LineTo( rRight.left , rRight.bottom-1 );
*/
            CRect r;

            for( int Channel=0 ; Channel<2 ; Channel++ )
            {
                // Setup rect for channel
                CRect r = (Channel == 0) ? rLeft : rRight;

                // Setup
                s32     NumSamples  = pDoc->GetNumSamples(i);
                s16*    pSamples    = pDoc->GetChannelData( 0, i );
                s32     iFirst      = GetScrollPos( SB_HORZ );
                s32     Start       = max( Offset-(iFirst/m_ZoomFactor), 0 );

                xarray<aiff_file::breakpoint> SampleBreakPoints;
                pDoc->GetBreakPoints( SampleBreakPoints, i );

                f32     BeginTime   = max( (f32)(iFirst*TIME_PER_PIXEL)-pDoc->m_pOffset[i], 0.0f );
                f32     DeltaTime   = (f32)m_ZoomFactor*TIME_PER_PIXEL;
                f32     CurrentTime = BeginTime;

                // Is the wav selected.
                if( pDoc->m_SampleSelected == i )
                    pDC->FillSolidRect( r.left+Start, r.top, r.Width(), r.Height(), RGB(150,150,255) );
                else
                    pDC->FillSolidRect( r.left+Start, r.top, r.Width(), r.Height(), RGB(255,255,255) );

                pDC->FillSolidRect( 0, r.top, r.left + Start, r.Height(), RGB(192,192,192) );

                // Draw seperator
                pDC->MoveTo( r.right, r.bottom-1 );
                pDC->LineTo( r.left , r.bottom-1 );
            
                // Draw the waveform
                for( int x=Start ; x<db.w+Start ; x++ )
                {
                    iFirst = (s32)(CurrentTime*(f32)pDoc->GetSampleRate( i ));
               
                    CurrentTime += DeltaTime;    
                    // Get index of last sample for x position
                    s32 iLast  = min( (s32)(CurrentTime*(f32)pDoc->GetSampleRate( i )), NumSamples );

                    // Exit if past data
                    if( iFirst > NumSamples )
                    {
                        // Fill remainder of window
                        pDC->FillSolidRect( x, r.top, r.right - x, r.Height(), RGB(192,192,192) );
                        break;
                    }

                    // Find min and max of samples for current x position
                    s16 s   = pSamples[iFirst];
                    s32 Min = s;
                    s32 Max = s;
                    while( ++iFirst < iLast )
                    {
                        s = pSamples[iFirst];
                        if( s < Min ) Min = s;
                        if( s > Max ) Max = s;
                    }

                    // Draw lines to fill in the data
                    s32 yMin = r.bottom - r.Height()/2 + Min * r.Height() / 65536;
                    s32 yMax = r.bottom - r.Height()/2 + Max * r.Height() / 65536;
                    s32 yAvg = (yMin + yMax) / 2;

                    if( x == Start )
                        pDC->MoveTo( x, yAvg );
                    else
                        pDC->LineTo( x, yAvg );

                    if( m_ZoomFactor > 1 )
                    {
                        pDC->LineTo( x, yMin );
                        pDC->LineTo( x, yMax );
                        pDC->MoveTo( x, yAvg );
                    }
                }
            
                // The the max time availabel in the view.
                f32 EndTime    = BeginTime + DeltaTime*(x-Offset);
                iFirst         = GetScrollPos( SB_HORZ );
                CPen* pPrevPen;
            
                // Change the pen if the sample is selected.
                if( pDoc->m_SampleSelected == i )
                    pPrevPen = pDC->SelectObject( &Yellow );
                else
                    pPrevPen = pDC->SelectObject( &Red );

                for( s32 j = 0; j < SampleBreakPoints.GetCount(); j++ )
                {
                    // Is this maker in view.
                    if( (SampleBreakPoints[j].Position >= BeginTime) && (SampleBreakPoints[j].Position <= EndTime) )
                    {
                        // Get the Draw Pixel, take the scroll and the complex effect offset in to consideration.
                        s32 MarkerPixel = ((s32)(SampleBreakPoints[j].Position/DeltaTime)+Offset)-(iFirst/m_ZoomFactor);
                        pDC->MoveTo( MarkerPixel, r.top );
                        pDC->LineTo( MarkerPixel, r.bottom );
                    }
                }
            
                pDC->SelectObject( pPrevPen );
                RectTop += WAVE_FORM_HEIGHT;
            }
            
            // Double the width to added since this was a stero sample.
            //RectTop += WAVE_FORM_HEIGHT * 2;

            // Draw the ruler
            //DrawRuler( pDC, rRuler );
        }
        else
        {
            // Unsupported number of channels
            ASSERTS( 0, xfs("Unsupported number of channels - %d '%s' sample %d", pDoc->GetNumChannels(i), (const char*)pDoc->GetAudioSourcePath(), i) );
        }
    }
    CRect rRuler  = db.GetRect();
    rRuler.bottom = TIME_BAR_HEIGHT;

    // Draw the ruler
    DrawRuler( pDC, rRuler );
    
    //rRuler.top    = rRuler.bottom;
    //rRuler.bottom = rRuler.top + MARKER_BAR_HEIGHT;
    
    // Draw the marker.
    //DrawMarker( pDC, rRuler );

    // Select old pen back into DC
    pDC->SelectObject( pOldPen );
}

//=========================================================================

BOOL CSoundView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

//=========================================================================

void CSoundView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

//=========================================================================

void CSoundView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

//=========================================================================

#ifdef _DEBUG
void CSoundView::AssertValid() const
{
	CView::AssertValid();
}

//=========================================================================

void CSoundView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

//=========================================================================

CSoundDoc* CSoundView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSoundDoc)));
	return (CSoundDoc*)m_pDocument;
}
#endif //_DEBUG

//=========================================================================

void CSoundView::CalibrateScrollbar( void )
{
	// Set the page value in the hscroll bar to reflect the width of the window
    SCROLLINFO Info;
    CSoundDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    Info.cbSize     = sizeof(Info);
    Info.fMask      = SIF_PAGE | SIF_RANGE;
    Info.nPage      = GetClientWidth() * m_ZoomFactor;
    Info.nMin       = 0;

    if( pDoc->m_SoundFileCount > 0 )
        Info.nMax  = (s32)(pDoc->m_MaxTime/TIME_PER_PIXEL);
    else
        Info.nMax  = (s32)(1.0f/TIME_PER_PIXEL);

    SetScrollInfo( SB_HORZ, &Info );
}

//=========================================================================

BOOL CSoundView::OnEraseBkgnd(CDC* pDC) 
{
    return 1;
	//return CView::OnEraseBkgnd(pDC);
}

//=========================================================================

void CSoundView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    // Get scrollbar state
    SCROLLINFO Info;
    GetScrollInfo( SB_HORZ, &Info );
    int Old;
    int Cur;
    Cur = GetScrollPos( SB_HORZ );
    Old = Cur;
    s32 ZoomFactor = m_ZoomFactor;//(s32)((f32)m_ZoomFactor*TIME_PER_PIXEL);
    if( ZoomFactor < 1 )
        ZoomFactor = 1;
    if( Cur != Old)
        Cur = Cur;
    
    // Modify position based on nSBCode
    switch( nSBCode )
    {
    case SB_LEFT:
        Cur = Info.nMin;
        break;
    case SB_LINELEFT:
        Cur = max( Cur-ZoomFactor, Info.nMin );
        break;
    case SB_PAGELEFT:
        Cur = max( Cur-Info.nPage, (unsigned int)Info.nMin );
        break;
    case SB_RIGHT:
        Cur = Info.nMax;
        break;
    case SB_LINERIGHT:
        Cur = min( Cur+ZoomFactor, Info.nMax );
        break;
    case SB_PAGERIGHT:
        Cur = min( Cur+Info.nPage, (unsigned int)Info.nMax );
        break;

    case SB_THUMBTRACK:
        {
            int Fraction = Cur % ZoomFactor;
            Cur = Info.nTrackPos - (Info.nTrackPos % ZoomFactor) + Fraction;
        }
        break;
    case SB_THUMBPOSITION:
        {
            int Fraction = Cur % ZoomFactor;
            Cur = Info.nTrackPos - (Info.nTrackPos % ZoomFactor) + Fraction;
        }
        break;

    case SB_ENDSCROLL:
        break;
    }

    // Update the scrollbar and redraw the view
    if( Old != Cur )
    {
        SetScrollPos( SB_HORZ, Cur );
        Invalidate();
    }

	CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

//=========================================================================

void CSoundView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    // Get scrollbar state
    SCROLLINFO Info;
    GetScrollInfo( SB_VERT, &Info );
    int Old;
    int Cur;
    Cur = GetScrollPos( SB_VERT );
    Old = Cur;

    // Modify position based on nSBCode
    switch( nSBCode )
    {
    case SB_BOTTOM:
        Cur = Info.nMax;
        break;
    case SB_LINEUP:
        Cur = Info.nMin;
        break;
    case SB_PAGEUP:
        Cur = Info.nMin;
        break;
    case SB_TOP:
        Cur = Info.nMin;
        break;
    case SB_LINEDOWN:
        Cur = min( (Info.nPage + 5), (u32)Info.nMax );
        break;
    case SB_PAGEDOWN:
        Cur = min( Info.nPage+Cur, (u32)Info.nMax );
        break;

    case SB_THUMBTRACK:
        {
            //int Fraction = Cur % m_ZoomFactor;
            Cur = Info.nTrackPos + 5;// - (Info.nTrackPos % m_ZoomFactor) + Fraction;
        }
        break;
    case SB_THUMBPOSITION:
        {
            //int Fraction = Cur % m_ZoomFactor;
            Cur = Info.nTrackPos + 5;// - (Info.nTrackPos % m_ZoomFactor) + Fraction;
        }
        break;

    case SB_ENDSCROLL:
        break;
    }

    // Update the scrollbar and redraw the view
    if( Old != Cur )
    {
        SetScrollPos( SB_VERT, Cur );
        Invalidate();
    }

	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

//=========================================================================

void CSoundView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

    CalibrateScrollbar();
}

//=========================================================================

void CSoundView::OnSoundZoomIn() 
{
	m_ZoomFactor /= 2;
    if( m_ZoomFactor < 1 )
        m_ZoomFactor = 1;
    CalibrateScrollbar();
    Invalidate();

    CSoundDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    if( pDoc->m_pCommandHandler )
    {
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_ZOOM, m_ZoomFactor );
    }
}

//=========================================================================

void CSoundView::OnSoundZoomInFull() 
{
	m_ZoomFactor = 1;
    CalibrateScrollbar();
    Invalidate();

    CSoundDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    if( pDoc->m_pCommandHandler )
    {
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_ZOOM, m_ZoomFactor );
    }
}

//=========================================================================

void CSoundView::OnSoundZoomOut() 
{
    CSoundDoc* pDoc = GetDocument();
    ASSERT( pDoc );
    
    SCROLLINFO Info;
    GetScrollInfo( SB_HORZ, &Info );

    f32 MaxZoomNeeded = pDoc->m_MaxTime/TIME_PER_PIXEL;
    if( Info.nPage <= MaxZoomNeeded )
	    m_ZoomFactor *= 2;

    CalibrateScrollbar();
    Invalidate();

    if( pDoc->m_pCommandHandler )
    {
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_ZOOM, m_ZoomFactor );
    }
}

//=========================================================================

void CSoundView::OnSoundZoomOutFull() 
{
    CSoundDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    s32 ClientWidth = GetClientWidth();

    f32 ZoomNeeded = pDoc->m_MaxTime/TIME_PER_PIXEL;
    s32 Zoom = 1;
    while( (ClientWidth*Zoom) <= ZoomNeeded )
    {
        Zoom *= 2;
    }

    m_ZoomFactor = Zoom;
    CalibrateScrollbar();
    Invalidate();

    if( pDoc->m_pCommandHandler )
    {
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_ZOOM, m_ZoomFactor );
    }
}

//=========================================================================

void CSoundView::OnSoundZoomSelection() 
{
}

//=========================================================================

BOOL CSoundView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    static int WheelPos = 0;

    WheelPos += zDelta;

    if( WheelPos <= -WHEEL_DELTA )
    {
        WheelPos += WHEEL_DELTA;
        OnSoundZoomOut();

    }
    else if( WheelPos >= WHEEL_DELTA )
    {
        WheelPos -= WHEEL_DELTA;
        OnSoundZoomIn();
    }

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

//=========================================================================

void CSoundView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    // Get scrollbar state
    SCROLLINFO Info;
    GetScrollInfo( SB_HORZ, &Info );
    int Old;
    int Cur;
    Cur = GetScrollPos( SB_HORZ );
    Old = Cur;

    switch( nChar )
    {
    case 37:
        // Left
        Cur = max( Cur-m_ZoomFactor, Info.nMin );
        break;
    case 39:
        Cur = min( Cur+m_ZoomFactor, Info.nMax );
        // Right
        break;
    case 38:
        // Up
        OnSoundZoomIn();
        break;
    case 40:
        // Down
        OnSoundZoomOut();
        break;
    }

    // Update the scrollbar and redraw the view
    if( Old != Cur )
    {
        SetScrollPos( SB_HORZ, Cur );
        Invalidate();
    }

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

//=========================================================================

void CSoundView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    // Skip the first 16 pixels for the bar.
    SCROLLINFO Info;
    GetScrollInfo( SB_VERT, &Info );

    s32 RectTop = 16 - Info.nPos;
    s32 i = 0;

    // Get the document.
    CSoundDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    // Go through all the samples to see which one was clicked on.
    for( ; i < pDoc->m_SoundFileCount; i++ )
    {
        if( pDoc->GetNumChannels(i) == 1 )
        {
            CRect r = m_ClientRect;
            r.top      = RectTop;
            r.bottom   = r.top + WAVE_FORM_HEIGHT;	
            
            // Did we click on the this one, if we did then set the focus to the selected sample.
            if( r.PtInRect( point ) )
            {
                break;
            }


            RectTop += WAVE_FORM_HEIGHT;
        }
        else if( pDoc->GetNumChannels(i) == 2 )
        {

            CRect r = m_ClientRect;
            r.top      = RectTop;
            r.bottom   = r.top + (WAVE_FORM_HEIGHT * 2);

            // Did we click on the this one, if we did then set the focus to the selected sample.
            if( r.PtInRect( point ) )
            {
                break;
            }

            RectTop += WAVE_FORM_HEIGHT * 2;
        }
    }
    
    if( pDoc->m_SoundFileCount == i )
    {
	    CView::OnLButtonDown(nFlags, point);
        return;
    }

    // Set the selected and set the focus of the status bar to the selected sample.
    pDoc->ElementSelectionChange( i );
    CalibrateScrollbar();

    m_MoveSample = TRUE;
    if( pDoc->m_pCommandHandler )
    {
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_NUM_SAMPLES, pDoc->GetNumSamples( i ) );
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_SAMPLE_RATE, pDoc->GetSampleRate( i ) );
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_SELECTION_START, pDoc->GetSelectionStart() );
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_SELECTION_END, pDoc->GetSelectionEnd() );
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_ZOOM, m_ZoomFactor );
    }
    Invalidate();

    m_StartOffset = point;
    m_SampleOffsetMouseUp = (s32)((pDoc->m_pOffset[ pDoc->m_SampleSelected ]/m_ZoomFactor)/TIME_PER_PIXEL);
            
	CView::OnLButtonDown(nFlags, point);
}

//=========================================================================

void CSoundView::OnLButtonUp(UINT nFlags, CPoint point) 
{	
    CSoundDoc* pDoc = GetDocument();
    if( m_MoveSample )
    {
        s32 Offset = 0;
    
        // Does the sample even exist.
        if( pDoc->m_SoundFileCount > 0 )
        {
             Offset = (s32)((pDoc->m_pOffset[ pDoc->m_SampleSelected ]/m_ZoomFactor)/TIME_PER_PIXEL);
        }
    
        //m_SampleOffsetMouseUp = Offset;

        if( (pDoc->m_pCommandHandler) && (pDoc->m_SoundFileCount > 0) )
        {   
            UpdateElementOffset( Offset,pDoc->m_SampleSelected );   
        }
    }

	m_MoveSample = FALSE;

	CView::OnLButtonUp(nFlags, point);
}

//=========================================================================

void CSoundView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect Rect;
    GetClientRect(&Rect );
    CSoundDoc* pDoc = GetDocument();

    if( !Rect.PtInRect( point ) )
        m_MoveSample = FALSE;

    if( m_MoveSample )
    {
        
        s32 Offset = (point.x - m_StartOffset.x) + m_SampleOffsetMouseUp;

        if( Offset < 0 )
        {
            Offset = 0;
        }

        // Does the sample even exist.	
        if( pDoc->m_SoundFileCount > 0 )	
            pDoc->m_pOffset[pDoc->m_SampleSelected] = ((f32)Offset*(f32)m_ZoomFactor)*TIME_PER_PIXEL;	
        
        // Only offset if its a complex effect.
        if( (pDoc->m_pCommandHandler) && (pDoc->m_SoundFileCount > 0) )
        {   
            CheckElementOffset( Offset, pDoc->m_SampleSelected );   
        }
        
        Invalidate();
    }

	CView::OnMouseMove(nFlags, point);
}

//=========================================================================

void CSoundView::RefreshSampleView ( void )
{
    CSoundDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    CRect r;
    GetClientRect( &r );

    SCROLLINFO Info;
    Info.cbSize     = sizeof(Info);
    Info.fMask      = SIF_ALL;
    Info.nMin       = 0;
    Info.nPage      = (r.right - r.left) * m_ZoomFactor;
    Info.nPos       = 0;
    Info.nTrackPos  = 0;
   
    if( pDoc->m_SoundFileCount > 0 )
        Info.nMax  = (s32)(pDoc->m_MaxTime/TIME_PER_PIXEL);
    else
        Info.nMax  = (s32)(1.0f/TIME_PER_PIXEL);

    SetScrollInfo( SB_HORZ, &Info );
    
    // Get the vertical max.
    Info.nMax = 0;
    for( s32 i = 0; i < pDoc->m_SoundFileCount; i++ )
        Info.nMax += (pDoc->GetNumChannels(i) == 1) ? WAVE_FORM_HEIGHT : WAVE_FORM_HEIGHT*2;

    Info.nPage     = r.bottom - r.top;
    SetScrollInfo( SB_VERT, &Info );
    
    // Set the focus on the newly added sample.
    m_MoveSample        = FALSE;

    if( pDoc->m_pCommandHandler )
    {
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_NUM_SAMPLES, pDoc->GetNumSamples( pDoc->m_SampleSelected ) );
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_SAMPLE_RATE, pDoc->GetSampleRate( pDoc->m_SampleSelected ) );
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_SELECTION_START, pDoc->GetSelectionStart() );
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_SELECTION_END, pDoc->GetSelectionEnd() );
        pDoc->m_pCommandHandler->SendMessage( WM_USER_MSG_SET_STATUS_ZOOM, m_ZoomFactor );
    }
    
    Invalidate();
}
//=========================================================================

void CSoundView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
    if( lHint == REFRESH_SAMPLE_VIEW )
    {
        RefreshSampleView();
    }
}

//=========================================================================

xbool CSoundView::UpdateElementOffset(s32 SampleOffset, s32 SampleIndex)
{
    CSoundDoc*  pDoc = GetDocument();
    DWORD Data = pDoc->GetSelectedItemData( );

    // If the descriptor is selected then we have to use SampleIndex for the element index.
    if( Data & DESCRIPTOR )
    {
        xhandle PackageHandle   = pDoc->m_AudioEditor.m_PackageSelected;
        s32 DescIndex           = pDoc->m_AudioEditor.m_pDesc( PackageHandle ).m_DescriptorSelected;
        
        // Check if its a complex effect.
        if( pDoc->m_AudioEditor.m_pDesc( PackageHandle ).m_pDescriptorList[ DescIndex ].m_Type == EDITOR_COMPLEX )
        {
            editor_descriptor& rDesc = pDoc->m_AudioEditor.m_pDesc( PackageHandle ).m_pDescriptorList[ DescIndex ];
            rDesc.m_pElements[ SampleIndex ].m_Delta = ((f32)SampleOffset*(f32)m_ZoomFactor)*TIME_PER_PIXEL;
            pDoc->m_pPropEditor->Refresh();
        }
        else
        {
            pDoc->m_pOffset[ SampleIndex ] = 0.0f;
        }
    }
    else if( Data & ELEMENT )
    {
        // Get the index for the descriptor and the element.
        xhandle PackageSelected = pDoc->m_AudioEditor.m_PackageSelected;
        s32 DescIndex           = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_DescriptorSelected;
        s32 ElementIndex        = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_ElementSelected;
        
        // Check if its a complex effect.
        if( pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_Type == EDITOR_COMPLEX )
        {
            editor_descriptor& rDesc = pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ];
            rDesc.m_pElements[ ElementIndex ].m_Delta = ((f32)SampleOffset*(f32)m_ZoomFactor)*TIME_PER_PIXEL;
            pDoc->m_pPropEditor->Refresh();
        }
        else
        {
            pDoc->m_pOffset[ 0 ] = 0.0f;
        }
    }
    return 1;
}

//================================================================

xbool CSoundView::CheckElementOffset(s32 SampleOffset, s32 SampleIndex)
{
    CSoundDoc*  pDoc        = GetDocument();
    DWORD Data              = pDoc->GetSelectedItemData( );
    xhandle PackageSelected = pDoc->m_AudioEditor.m_PackageSelected;

    // If the descriptor is selected then we have to use SampleIndex for the element index.
    if( Data & DESCRIPTOR )
    {
        s32 DescIndex = pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_DescriptorSelected;
        
        // Check if its a complex effect.
        if( pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_Type != EDITOR_COMPLEX )
        {
            pDoc->m_pOffset[ SampleIndex ] = 0.0f;
            return 0;
        }
    }
    else if( Data & ELEMENT )
    {
        // Get the index for the descriptor and the element.
        s32 DescIndex = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_DescriptorSelected;
        s32 ElementIndex = pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_ElementSelected;
        
        // Check if its a complex effect.
        if( pDoc->m_AudioEditor.m_pDesc( PackageSelected ).m_pDescriptorList[ DescIndex ].m_Type != EDITOR_COMPLEX )
        {
            pDoc->m_pOffset[ SampleIndex ] = 0.0f;
            return 0;
        }
    }

    return 1;
}

//================================================================
