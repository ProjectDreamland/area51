// CPushButtonBmp.cpp : implementation file
//

#include "stdafx.h"
#include "PartEd.h"
#include "PushButtonBmp.h"
#include "DoubleBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPushButtonBmp

CPushButtonBmp::CPushButtonBmp()
{
    m_ButtonType    = BUTTON_TYPE_PUSHBUTTON;
    m_IsChecked     = false;
    m_IsHot         = false;
    m_IsPressed     = false;

    m_Width         = 0;
    m_Height        = 0;

    m_Label         = "";

    m_ColorChecked  = RGB( 255, 255, 255 );

    m_Font.CreateFont( 16,                          // Height
                       0,                           // Width (0 = AutoWidth)
                       0,                           // Escapement
                       0,                           // Orientation
                       FW_NORMAL,                   // Weight
                       FALSE,                       // Italic
                       FALSE,                       // Underline
                       FALSE,                       // Strike-Out
                       ANSI_CHARSET,                // Character Set
                       OUT_DEFAULT_PRECIS,          // Output precision
                       CLIP_DEFAULT_PRECIS,         // Clip precision
                       DEFAULT_QUALITY,             // Quality
                       DEFAULT_PITCH | FF_DONTCARE, // Pitch and Family
                       "Arial" );                   // Font
}

CPushButtonBmp::~CPushButtonBmp()
{

}


BEGIN_MESSAGE_MAP(CPushButtonBmp, CWnd)
	//{{AFX_MSG_MAP(CPushButtonBmp)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()

    ON_MESSAGE( WM_MOUSELEAVE, OnMouseLeave )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPushButtonBmp Public Functions

void CPushButtonBmp::SetButtonType( ButtonType BtnType )
{
    m_ButtonType    = BtnType;
}

void CPushButtonBmp::SetIsChecked( bool IsChecked )
{
    m_IsChecked     = IsChecked;
    RedrawWindow();
}

void CPushButtonBmp::SetCheckColor( COLORREF CheckColor )
{
    m_ColorChecked  = CheckColor;
}

void CPushButtonBmp::SetPushBitmapUp( UINT nIDResourceColor, UINT nIDResourceAlpha )
{
    m_BmpPushColor_Up.LoadBitmap( nIDResourceColor );

    if( nIDResourceAlpha != NULL )
    {
        m_BmpPushAlpha_Up.LoadBitmap( nIDResourceAlpha );
    }
}

void CPushButtonBmp::SetToggleBitmapUp( UINT nIDResourceColor, UINT nIDResourceAlpha )
{
    m_BmpToggleColor_Up.LoadBitmap( nIDResourceColor );

    if( nIDResourceAlpha != NULL )
    {
        m_BmpToggleAlpha_Up.LoadBitmap( nIDResourceAlpha );
    }
}


/////////////////////////////////////////////////////////////////////////////
// CPushButtonBmp message handlers

void CPushButtonBmp::OnPaint() 
{
	CPaintDC*       pDC = new CPaintDC(this);
    CDoubleBuffer   db( pDC );

	// Get the Client Rectangle
    CRect             rcClient;
    GetClientRect     ( rcClient );

	// Fill the background
    CRgn        rgn;
    CBrush      brFill;
    CBrush      brBorder;
    CPen        penBorder;

    if( m_IsHot )
    {
        rgn.CreateRoundRectRgn      ( 0, 0, m_Width, m_Height, 4, 4 );
        brBorder.CreateSolidBrush   ( RGB(0,0,0) );
        penBorder.CreatePen         ( PS_SOLID, 1, RGB( 0, 0, 0 ) );

        if( m_IsChecked && (m_ButtonType == BUTTON_TYPE_CHECKBUTTON) )
        {
            pDC->SetTextColor           ( RGB(255,255,255) );
            brFill.CreateSolidBrush     ( m_ColorChecked );
        }
        else
        {
            pDC->SetTextColor           ( RGB(0,0,0) );
            brFill.CreateSolidBrush     ( RGB(196,196,196) );
        }
    }
    else if( m_IsChecked && (m_ButtonType == BUTTON_TYPE_CHECKBUTTON) )
    {
        rgn.CreateRoundRectRgn      ( 3, 3, m_Width - 3, m_Height - 3, 4, 4 );
        brBorder.CreateSolidBrush   ( RGB(0,0,0) );
        penBorder.CreatePen         ( PS_SOLID, 1, RGB( 0, 0, 0 ) );

        pDC->SetTextColor           ( RGB(255,255,255) );
        brFill.CreateSolidBrush     ( m_ColorChecked );
    }
    else
    {
        rgn.CreateRoundRectRgn      ( 3, 3, m_Width - 3, m_Height - 3, 4, 4 );
        brBorder.CreateSolidBrush   ( RGB(128,128,128) );
        penBorder.CreatePen         ( PS_SOLID, 1, RGB( 128, 128, 128 ) );

        pDC->SetTextColor           ( RGB(16,16,16) );
        brFill.CreateSolidBrush     ( RGB(160, 160, 160) );
    }

    // Clear & draw background
    pDC->FillSolidRect              ( &rcClient, RGB(160,160,160) );
    pDC->FillRgn                    ( &rgn, &brFill );

    if( m_IsPressed )
    {
        CBrush      brPressFill;
        CPen        penTop1;        // Outside line
        CPen        penTop2;        // Inside line
        CPen        penBottom1;     // Inside line
        CPen        penBottom2;     // Outside line

        brPressFill.CreateSolidBrush    ( RGB(160, 160, 160) );
        penTop1.CreatePen               ( PS_SOLID, 1, RGB(  96,  96,  96) );
        penTop2.CreatePen               ( PS_SOLID, 1, RGB( 128, 128, 128) );
        penBottom1.CreatePen            ( PS_SOLID, 1, RGB( 176, 176, 176) );
        penBottom2.CreatePen            ( PS_SOLID, 1, RGB( 196, 196, 196) );

        pDC->FillRgn      ( &rgn, &brPressFill );

        pDC->SelectObject ( &penTop1 );
        pDC->MoveTo       ( m_Width - 2, 1 );
        pDC->LineTo       ( m_Width - 3, 0 );
        pDC->LineTo       ( 2, 0 );
        pDC->LineTo       ( 0, 2 );
        pDC->LineTo       ( 0, m_Height - 3 );

        pDC->SelectObject ( &penTop2 );
        pDC->MoveTo       ( m_Width - 3, 1 );
        pDC->LineTo       ( 2, 1 );
        pDC->LineTo       ( 2, 2 );
        pDC->LineTo       ( 1, 2 );
        pDC->LineTo       ( 1, m_Height - 3 );

        pDC->SelectObject ( &penBottom1 );
        pDC->MoveTo       ( m_Width - 2, 2 );
        pDC->LineTo       ( m_Width - 2, m_Height - 3 );
        pDC->LineTo       ( m_Width - 3, m_Height - 2 );
        pDC->LineTo       ( 2, m_Height - 2 );

        pDC->SelectObject ( &penBottom2 );
        pDC->MoveTo       ( m_Width - 1, 2 );
        pDC->LineTo       ( m_Width - 1, m_Height - 3 );
        pDC->LineTo       ( m_Width - 3, m_Height - 1 );
        pDC->LineTo       ( 2, m_Height - 1 );
        pDC->LineTo       ( 1, m_Height - 2 );
    }

    // Get ready to draw bitmaps
    CDC         dcColor;
    CBitmap*    pOldBmp         = NULL;
    dcColor.CreateCompatibleDC  ( pDC );

    if( m_IsChecked && (m_ButtonType == BUTTON_TYPE_TOGGLEBUTTON) )
    {
        // Draw the color bitmap
        pOldBmp     = dcColor.SelectObject( &m_BmpToggleColor_Up );
        pDC->BitBlt   ( 0, 0, m_Width, m_Height, &dcColor, 0, 0, SRCINVERT);

        // Draw the alpha bitmap
        pOldBmp     = dcColor.SelectObject( &m_BmpToggleAlpha_Up );
        pDC->BitBlt   ( 0, 0, m_Width, m_Height, &dcColor, 0, 0, SRCAND );

        // Draw the color bitmap
        pOldBmp     = dcColor.SelectObject( &m_BmpToggleColor_Up );
        pDC->BitBlt   ( 0, 0, m_Width, m_Height, &dcColor, 0, 0, SRCINVERT);
    }
    else
    {
        // Draw the color bitmap
        pOldBmp     = dcColor.SelectObject( &m_BmpPushColor_Up );
        pDC->BitBlt   ( 0, 0, m_Width, m_Height, &dcColor, 0, 0, SRCINVERT);

        // Draw the alpha bitmap
        pOldBmp     = dcColor.SelectObject( &m_BmpPushAlpha_Up );
        pDC->BitBlt   ( 0, 0, m_Width, m_Height, &dcColor, 0, 0, SRCAND );

        // Draw the color bitmap
        pOldBmp     = dcColor.SelectObject( &m_BmpPushColor_Up );
        pDC->BitBlt   ( 0, 0, m_Width, m_Height, &dcColor, 0, 0, SRCINVERT);
    }


    // Draw button border...if it's not being pressed currently
    if( !m_IsPressed )
    {
        // Filling a region is fine...but we don't want to use FrameRgn...it gets thick around the corners
        //pDC->FrameRgn                 ( &rgn, &brBorder, 1, 1 );

        pDC->SelectObject   ( &penBorder );

        if( m_IsHot )
        {
            pDC->MoveTo         ( 2, 0 );
            pDC->LineTo         ( m_Width - 4, 0 );
            pDC->LineTo         ( m_Width - 2, 2 );
            pDC->LineTo         ( m_Width - 2, m_Height - 4 );
            pDC->LineTo         ( m_Width - 4, m_Height - 2 );
            pDC->LineTo         ( 2, m_Height - 2 );
            pDC->LineTo         ( 0, m_Height - 4 );
            pDC->LineTo         ( 0, 2 );
            pDC->LineTo         ( 2, 0 );
        }
        else
        {
            pDC->MoveTo         ( 5, 3 );
            pDC->LineTo         ( m_Width - 7, 3 );
            pDC->LineTo         ( m_Width - 5, 5 );
            pDC->LineTo         ( m_Width - 5, m_Height - 7 );
            pDC->LineTo         ( m_Width - 7, m_Height - 5 );
            pDC->LineTo         ( 5, m_Height - 5 );
            pDC->LineTo         ( 3, m_Height - 7 );
            pDC->LineTo         ( 3, 5 );
            pDC->LineTo         ( 5, 3 );
        }
    }

    // Draw text label...if no bitmap has been set
    if( HBITMAP(m_BmpPushColor_Up) == NULL )
    {
        int     textPosX            = ( m_Width  / 2 ) - ( pDC->GetTextExtent(m_Label).cx / 2 ) + 2;
        int     textPosY            = ( m_Height / 2 ) - ( pDC->GetTextExtent(m_Label).cy / 2 );

        pDC->SelectObject             ( m_Font );
        pDC->SetBkMode                ( TRANSPARENT );
        pDC->TextOut                  ( textPosX, textPosY, m_Label );
    }

    // Clean up
    dcColor.SelectObject( pOldBmp );
    dcColor.DeleteDC();

	// Do not call CWnd::OnPaint() for painting messages
}

void CPushButtonBmp::OnLButtonUp(UINT nFlags, CPoint point) 
{
    m_IsPressed = false;

    // Toggle the checked state for if we're a CheckButton or ToggleButton
    if( (m_ButtonType == BUTTON_TYPE_CHECKBUTTON) || (m_ButtonType == BUTTON_TYPE_TOGGLEBUTTON) )
    {
        m_IsChecked = !m_IsChecked;
    }

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_PUSHBTN_CLICKED, controlID, m_IsChecked );

    RedrawWindow();
}

void CPushButtonBmp::OnLButtonDown(UINT nFlags, CPoint point) 
{
    m_IsPressed = true;

    CWnd::RedrawWindow();
	CWnd::OnLButtonDown(nFlags, point);
}

BOOL CPushButtonBmp::Create(CWnd* pParentWnd, const char* Label, int posX, int posY, int nWidth, int nHeight, UINT nID)
{
    // Set preferences
    m_Width         = nWidth;
    m_Height        = nHeight;
    m_Label         = Label;

    // Make the window
    CString         winClassName;
    RECT            winRect;

	winClassName    = AfxRegisterWndClass   ( NULL, //CS_DBLCLKS,                       // Class Style
                                              ::LoadCursor(NULL, IDC_ARROW),    // Cursor
                                              NULL,                             // Background
                                              0 );                              // Icon

    winRect.left    = posX;
    winRect.right   = posX + m_Width;
    winRect.top     = posY;
    winRect.bottom  = posY + m_Height;

	return CWnd::Create(winClassName, "", WS_VISIBLE, winRect, pParentWnd, nID, NULL);
}

void CPushButtonBmp::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint )
{
    // Need to reset internal values
    m_Width         = nWidth;
    m_Height        = nHeight;

    // Now we can go ahead and move/resize
    CWnd::MoveWindow( x, y, m_Width, m_Height, TRUE );
}

void CPushButtonBmp::OnMouseMove(UINT nFlags, CPoint point) 
{
    m_IsHot = true;

    // Track the mouse...this causes OnMouseLeave() to get called when we leave this window
    ::TRACKMOUSEEVENT   tme;
    tme.cbSize          = sizeof(::TRACKMOUSEEVENT);
    tme.dwFlags         = TME_LEAVE;
    tme.hwndTrack       = m_hWnd;
    tme.dwHoverTime     = HOVER_DEFAULT;
    ::_TrackMouseEvent  ( &tme );

    CWnd::RedrawWindow();
}

LRESULT CPushButtonBmp::OnMouseLeave(WPARAM wParam, LPARAM lParam) 
{
    m_IsHot     = false;
    m_IsPressed = false;

    CWnd::RedrawWindow();
	
    return 0;
}
