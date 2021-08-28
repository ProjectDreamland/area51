// EditInt.cpp : implementation file
//

#include "stdafx.h"
#include "PartEd.h"
#include "EditInt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditInt

CEditInt::CEditInt()
{
    m_Value             = 0;

    m_clrText           = RGB( 255, 255, 255 );
    m_clrBackground     = RGB(  32,  32,  32 );

    m_brBackground.CreateSolidBrush     ( m_clrBackground );
}

CEditInt::~CEditInt()
{
}


BEGIN_MESSAGE_MAP(CEditInt, CEdit)
	//{{AFX_MSG_MAP(CEditInt)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CKeyBar Public Functions

int CEditInt::GetValue( void ) const
{
    return m_Value;
}

int CEditInt::SetValue( int Value )
{
    // Store the new value
    m_Value = Value;

    // Update the window text to reflect new value
    char            buffer[256];
    itoa            ( Value, buffer, 10 );
    SetWindowText   ( buffer );

    // Return final value...which is the same for now
    // But in the future we may need validation in this function
    return m_Value;
}

/////////////////////////////////////////////////////////////////////////////
// CEditInt message handlers

HBRUSH CEditInt::CtlColor(CDC* pDC, UINT nCtlColor) 
{
    // Alter the colors during an edit
    pDC->SetTextColor   ( m_clrText );
    pDC->SetBkColor     ( m_clrBackground );

    return m_brBackground;
}

void CEditInt::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// Get the Client Rectangle
    CRect             rcClient;
    GetClientRect     ( rcClient );

	// Get the Text
    CString         editString;
    GetWindowText   ( editString );

	// Draw the background
    CRgn        rgn;
    rgn.CreateRoundRectRgn  ( 0, 0, rcClient.right, rcClient.bottom, 4, 4 );
    dc.FillRgn              ( &rgn, &m_brBackground );

	// Draw the text
    dc.SetBkColor   ( m_clrBackground );
    dc.SetBkMode    ( TRANSPARENT );
    dc.SetTextColor ( m_clrText );
    dc.ExtTextOut   ( ((rcClient.right - 3) - (editString.GetLength() * 8)), 2, ETO_CLIPPED, rcClient, editString, NULL );

    // Release the text buffer
    editString.ReleaseBuffer();

	// Do not call CEdit::OnPaint() for painting messages
}

void CEditInt::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CEdit::OnLButtonDown(nFlags, point);

    SetSel( 0, -1, FALSE ); // Select All

    Invalidate(FALSE);
}

void CEditInt::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);

    SetSel( 0, -1, FALSE ); // Select All
	
    Invalidate(FALSE);
}

void CEditInt::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);

    Invalidate(FALSE);
}

BOOL CEditInt::PreTranslateMessage(MSG* pMsg) 
{
    // If the message is not intended for this window pass it on
    if( pMsg->hwnd != m_hWnd )
    {
    	return CEdit::PreTranslateMessage(pMsg);
    }

    // Do our own thing with any input
    if( pMsg->message == WM_KEYDOWN )
    {
        if( pMsg->wParam == VK_RETURN )
        {
            // Select everything
            SetSel( 0, -1, FALSE );

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_EDIT_ENTERED, controlID, (LPARAM)0 );
        }

        if( pMsg->wParam == VK_TAB )
        {
            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_EDIT_ENTERED, controlID, (LPARAM)0 );

            // Now do the default thing
        	return CEdit::PreTranslateMessage(pMsg);
        }

        if( pMsg->wParam == VK_ESCAPE )
        {
            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_EDIT_ENTERED, controlID, (LPARAM)0 );

            // Now kill focus of Edit Control
            GetParent()->SetFocus();
        }

        // Pass allowed keys on for generic processing

        if( ((pMsg->wParam >= 48) && (pMsg->wParam <=  57))     ||  // Reguler Number Keys
            ((pMsg->wParam >= 96) && (pMsg->wParam <= 105))     ||  // Numeric Keypad Number Keys
            pMsg->wParam == 189                                 ||  // Regular Minus
            pMsg->wParam == VK_SUBTRACT                         ||  // Numeric Keypad Minus
            pMsg->wParam == VK_LSHIFT                           ||  // Left Shift
            pMsg->wParam == VK_RSHIFT                           ||  // Right Shift
            pMsg->wParam == VK_LCONTROL                         ||  // Left Control
            pMsg->wParam == VK_RCONTROL                         ||  // Right Control
            pMsg->wParam == VK_HOME                             ||  // Home
            pMsg->wParam == VK_END                              ||  // End
            pMsg->wParam == VK_LEFT                             ||  // Left Arrow
            pMsg->wParam == VK_RIGHT                            ||  // Right Arrow
            pMsg->wParam == VK_DELETE                           ||  // Delete
            pMsg->wParam == VK_BACK                             )   // Backspace
        {
	        return CEdit::PreTranslateMessage(pMsg);
        }

        return TRUE;
    }

    // Let the default function handle any other messages
	return CEdit::PreTranslateMessage(pMsg);
}

void CEditInt::OnUpdate() 
{
    // Store the new value
    char            buffer[256];
    GetWindowText   ( buffer, 256 );
    m_Value         = atoi( buffer );
}
