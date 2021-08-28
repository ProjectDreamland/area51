// FlyoutList.cpp : implementation file
//

#include "stdafx.h"
#include "parted.h"
#include "DoubleBuffer.h"
#include "FlyoutList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFlyoutList

CFlyoutList::CFlyoutList()
{
    // FlyoutList Properties
    m_Selection     = 0;
    m_nItems        = 0;

    m_pItemLabels   = NULL;

    // Drawing Properties
    m_Width             = 0;
    m_Height            = 0;

    m_ColorBackground   = RGB( 188, 188, 192 );

    m_FontSelected.CreateFont   ( 14,                           // Height
                                  0,                            // Width (0 = AutoWidth)
                                  0,                            // Escapement
                                  0,                            // Orientation
                                  FW_BOLD,                      // Weight
                                  FALSE,                        // Italic
                                  FALSE,                        // Underline
                                  FALSE,                        // Strike-Out
                                  ANSI_CHARSET,                 // Character Set
                                  OUT_DEFAULT_PRECIS,           // Output precision
                                  CLIP_DEFAULT_PRECIS,          // Clip precision
                                  DEFAULT_QUALITY,              // Quality
                                  DEFAULT_PITCH | FF_DONTCARE,  // Pitch and Family
                                  "Tahoma" );                   // Font

    m_FontList.CreateFont       ( 14,                           // Height
                                  0,                            // Width (0 = AutoWidth)
                                  0,                            // Escapement
                                  0,                            // Orientation
                                  FW_NORMAL,                    // Weight
                                  FALSE,                        // Italic
                                  FALSE,                        // Underline
                                  FALSE,                        // Strike-Out
                                  ANSI_CHARSET,                 // Character Set
                                  OUT_DEFAULT_PRECIS,           // Output precision
                                  CLIP_DEFAULT_PRECIS,          // Clip precision
                                  DEFAULT_QUALITY,              // Quality
                                  DEFAULT_PITCH | FF_DONTCARE,  // Pitch and Family
                                  "Tahoma" );                   // Font
                                  //"Arial" );                  // Font
}

CFlyoutList::~CFlyoutList()
{
    if( m_pItemLabels )
    {
        delete[] m_pItemLabels;
        m_pItemLabels = NULL;
    }
}


BEGIN_MESSAGE_MAP(CFlyoutList, CWnd)
	//{{AFX_MSG_MAP(CFlyoutList)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFlyoutList message handlers

BOOL CFlyoutList::Create(CWnd* pParentWnd, int NumItems, CString* pItemLabels, int posX, int posY, int nWidth, int nHeight, UINT nID)
{
    // Set preferences
    m_Width         = nWidth;
    m_Height        = nHeight;
    m_nItems        = NumItems;

    // Initialize Item Labels
    if( m_nItems > 0 )
    {
        ASSERT( pItemLabels );
        m_pItemLabels   = new CString[NumItems];

        for( int i = 0; i < NumItems; i++ )
        {
            m_pItemLabels[i]    = pItemLabels[i];
        }
    }

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

	return CWnd::Create(winClassName, "", WS_VISIBLE, winRect, pParentWnd->GetParent(), nID, NULL);
}

void CFlyoutList::OnPaint() 
{
	// Do not call CWnd::OnPaint() for painting messages
	CPaintDC*       pDC = new CPaintDC(this);
    CDoubleBuffer   db( pDC );

	// Get the Client Rectangle
    CRect             rcClient;
    GetClientRect     ( rcClient );

    // Fill the background
    //pDC->FillSolidRect( &rcClient, m_ColorBackground );
    pDC->FillSolidRect( &rcClient, RGB(160,160,160) );

    // Draw the selected item underline
    CPen        penUnderline;
    penUnderline.CreatePen( PS_SOLID, 1, RGB( 48, 48, 48 ) );

    pDC->SelectObject   ( &penUnderline );
    pDC->MoveTo         ( 4, m_Height - 2 );
    pDC->LineTo         ( m_Width - 4, m_Height - 2 );

    // Draw the selected item label
    pDC->SelectObject             ( m_FontSelected );
    pDC->SetBkMode                ( TRANSPARENT );
    pDC->TextOut                  ( 4, 0, m_pItemLabels[7] );

    // Draw the listed items' labels
    pDC->SelectObject             ( m_FontList );
    pDC->SetBkMode                ( TRANSPARENT );

    for( int i = 0; i < m_nItems; i++ )
    {
        //pDC->TextOut( 4, (i * m_Height), m_pItemLabels[i] );
    }
}
