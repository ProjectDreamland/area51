// ViewportToolbar.cpp : implementation file
//

#include "stdafx.h"
#include "parted.h"
#include "ViewportToolbar.h"
#include "SplitFrame.h"
#include "ParametricSplitter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewportToolbar

CViewportToolbar::CViewportToolbar()
{
    m_IsMaximized   = false;
}

CViewportToolbar::~CViewportToolbar()
{

}


BEGIN_MESSAGE_MAP(CViewportToolbar, CControlBar)
	//{{AFX_MSG_MAP(CViewportToolbar)
	ON_WM_PAINT()
    ON_MESSAGE( WM_USER_MSG_PUSHBTN_CLICKED, OnPushButton_Clicked )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CViewportToolbar message handlers

void CViewportToolbar::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler )
{

}

CSize CViewportToolbar::CalcFixedLayout( BOOL bStretch, BOOL bHorz )
{
    // Get Window Dimensions
    CRect   rc;
    GetParent()->GetClientRect( &rc );

    // Reposition Toolbar Controls
    m_Btn_Maximize.MoveWindow( (rc.right - 24), 0, 24, 24 );

    return CSize( rc.right, 24 );
}

BOOL CViewportToolbar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
    // Make the window
    m_dwStyle       = dwStyle;

    // Make the window
    CString         winClassName;
	winClassName    = AfxRegisterWndClass   ( CS_DBLCLKS,                       // Class Style
                                              ::LoadCursor(NULL, IDC_ARROW),    // Cursor
                                              NULL,                             // Background
                                              0 );                              // Icon



	if( !CControlBar::Create(winClassName, "", m_dwStyle | WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), pParentWnd, nID, NULL) )
    {
        return FALSE; // Creation of KeyBar failed!
    }

    // Create the Camera Flyout List
    CString        pCameraNames[8];

    pCameraNames[0] = "Top";
    pCameraNames[1] = "Bottom";
    pCameraNames[2] = "Left";
    pCameraNames[3] = "Right";
    pCameraNames[4] = "Front";
    pCameraNames[5] = "Back";
    pCameraNames[6] = "User";
    pCameraNames[7] = "Perspective";

    m_CameraList.Create             ( this, 8, pCameraNames, 5, 3, 77, 18, IDC_VIEWPORT_CAMERA_LIST );

    // Create Buttons
    m_Btn_Maximize.Create           ( this, "Maximize", 0, 0, 24, 24, IDC_VIEWPORT_MAXIMIZE    );
    m_Btn_Maximize.SetPushBitmapUp  ( IDB_VIEWPORT_MAXIMIZE_COLOR, IDB_VIEWPORT_MAXIMIZE_ALPHA );

    return TRUE;
}

void CViewportToolbar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Draw the background
    CRect               rcClient;
    GetClientRect       ( rcClient );
    dc.FillSolidRect    ( rcClient, RGB(160,160,160) );

}

LRESULT CViewportToolbar::OnPushButton_Clicked( WPARAM wParam, LPARAM lParam )
{
    switch( wParam )
    {
        case IDC_VIEWPORT_MAXIMIZE:
        {
            m_IsMaximized   = !m_IsMaximized;

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
//            GetParent()->SendMessage    ( WM_USER_MSG_VIEWPORT_TOOLBAR_MINMAX_CHANGED, controlID, m_IsMaximized );

            CSplitFrame* pFrame = (CSplitFrame*)GetParent();
            ASSERT( pFrame );
            CParametricSplitter* pSplitter = (CParametricSplitter*)pFrame->GetParent();
            ASSERT( pSplitter );
            pSplitter->SetActiveViewport( pFrame->GetDlgCtrlID() );

            AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_VIEW_VIEWPORT_MAXIMIZE, 0 );
            break;
        }
    }
    return TRUE;
}
