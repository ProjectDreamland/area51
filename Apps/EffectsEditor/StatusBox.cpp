// StatusBox.cpp : implementation file
//

#include "stdafx.h"
#include "PartEd.h"
#include "StatusBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStatusBox

CStatusBox::CStatusBox()
{
    m_Width         = 0;
    m_Height        = 0;
    m_StatusText    = "";

    m_Font.CreateFont( 17,                          // Height
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

CStatusBox::~CStatusBox()
{
}


BEGIN_MESSAGE_MAP(CStatusBox, CWnd)
	//{{AFX_MSG_MAP(CStatusBox)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CStatusBox - Public Functions

void CStatusBox::SetStatusText( const char* Text )
{
    m_StatusText    = Text;
    RedrawWindow();
}

void CStatusBox::SetBackgroundColor( COLORREF bkColor )
{
    m_clrBackground = bkColor;
    RedrawWindow();
}

void CStatusBox::SetTextColor( COLORREF txtColor )
{
    m_clrText       = txtColor;
    RedrawWindow();
}


/////////////////////////////////////////////////////////////////////////////
// CStatusBox message handlers

BOOL CStatusBox::Create(CWnd* pParentWnd, const char* Text, int posX, int posY, int nWidth, int nHeight, UINT nID, COLORREF bkColor, COLORREF txtColor)
{
    // Set preferences
    m_StatusText    = Text;
    m_Width         = nWidth;
    m_Height        = nHeight;
    m_clrText       = txtColor;
    m_clrBackground = bkColor;

    // Make the window
    CString         winClassName;
    RECT            winRect;

	winClassName    = AfxRegisterWndClass   ( CS_DBLCLKS,                       // Class Style
                                              ::LoadCursor(NULL, IDC_ARROW),    // Cursor
                                              NULL,                             // Background
                                              0 );                              // Icon

    winRect.left    = posX;
    winRect.right   = posX + m_Width;
    winRect.top     = posY;
    winRect.bottom  = posY + m_Height;

	return CWnd::Create(winClassName, "", WS_VISIBLE, winRect, pParentWnd, nID, NULL);
}

void CStatusBox::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint )
{
    // Need to reset internal values
    m_Width         = nWidth;
    m_Height        = nHeight;

    // Now we can go ahead and move/resize
    CWnd::MoveWindow( x, y, m_Width, m_Height, TRUE );
}

void CStatusBox::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Get the Client Rectangle
    CRect             rcClient;
    GetClientRect     ( rcClient );

	// Fill the background
    CRgn        rgn;
    CBrush      brFill;
    CBrush      brBorder;

    rgn.CreateRoundRectRgn      ( 0, 0, m_Width - 1, m_Height - 1, 4, 4 );
    brFill.CreateSolidBrush     ( m_clrBackground );
    brBorder.CreateSolidBrush   ( RGB(128,128,128) );
    dc.SetTextColor             ( m_clrText );

    dc.FillRgn                  ( &rgn, &brFill );
    dc.FrameRgn                 ( &rgn, &brBorder, 1, 1 );

    dc.SelectObject             ( m_Font );
    dc.SetBkMode                ( TRANSPARENT );
    dc.ExtTextOut               ( 6, 2, ETO_CLIPPED, rcClient, m_StatusText, NULL );
	
	// Do not call CWnd::OnPaint() for painting messages
}
