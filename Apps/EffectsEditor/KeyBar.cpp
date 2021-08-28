// KeyBar.cpp : implementation file
//

#include "stdafx.h"
#include "PartEd.h"
#include "KeyBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyBar

CKeyBar::CKeyBar()
{
    m_FrameStart        = 0;
    m_FrameEnd          = 0;
    m_FrameCurrent      = 0.0f;

    m_nKeyFilters       = 0;
    m_pKeyFilters       = NULL;

    m_PlayTimer         = 0;

    m_IsAnimateOn       = false;
    m_IsPlayOn          = false;

    m_DoLoopPlayback    = true;
    m_DoKeyStep         = false;
}

CKeyBar::~CKeyBar()
{
    if( m_pKeyFilters )
    {
        delete[] m_pKeyFilters;
        m_pKeyFilters = NULL;
    }
}


BEGIN_MESSAGE_MAP(CKeyBar, CControlBar)
	//{{AFX_MSG_MAP(CKeyBar)
	ON_WM_PAINT()
    ON_MESSAGE( WM_USER_MSG_EDIT_ENTERED,                       OnEdit_Entered                      )

    ON_MESSAGE( WM_USER_MSG_TIMESLIDER_EDIT_BEGIN,              OnTimeSlider_Edit_Begin             )

    ON_MESSAGE( WM_USER_MSG_TIMESLIDER_FRAMESTART_CHANGED,      OnTimeSlider_FrameStart_Changed     )
    ON_MESSAGE( WM_USER_MSG_TIMESLIDER_FRAMEEND_CHANGED,        OnTimeSlider_FrameEnd_Changed       )
    ON_MESSAGE( WM_USER_MSG_TIMESLIDER_FRAMECURRENT_CHANGED,    OnTimeSlider_FrameCurrent_Changed   )

    ON_MESSAGE( WM_USER_MSG_TIMESLIDER_KEYS_SELECTION_CHANGED,  OnTimeSlider_Keys_Selection_Changed )
    ON_MESSAGE( WM_USER_MSG_TIMESLIDER_KEYS_MOVED,              OnTimeSlider_Keys_Moved             )
    ON_MESSAGE( WM_USER_MSG_TIMESLIDER_KEYS_DELETED,            OnTimeSlider_Keys_Deleted           )
    ON_MESSAGE( WM_USER_MSG_TIMESLIDER_KEYS_CHANGED,            OnTimeSlider_Keys_Changed           )

    ON_MESSAGE( WM_USER_MSG_PUSHBTN_CLICKED,                    OnPushButton_Clicked                )
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CKeyBar Public Functions

f32 CKeyBar::GetTime( void ) const
{
    return m_FrameCurrent;
}

int CKeyBar::GetTimeRangeStart( void )  const
{
    return (int)m_FrameStart;
}

int CKeyBar::GetTimeRangeEnd( void ) const
{
    return (int)m_FrameEnd;
}

int CKeyBar::SetTime( int Frame )
{
    if( m_FrameCurrent == (f32)Frame )
        return Frame;

    m_FrameCurrent  = (f32)Frame;

    // Validate current frame
    if( m_FrameCurrent < m_FrameStart )
    {
        m_FrameCurrent  = (f32)m_FrameStart;
    }
    else if( m_FrameCurrent > m_FrameEnd )
    {
        m_FrameCurrent  = (f32)m_FrameEnd;
    }

    // Update Time Slider & Edit Box
    m_TimeSlider.SetTime            ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );
    m_Edit_FrameCurrent.SetValue    ( (int)m_FrameCurrent );

    //RedrawWindow();

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

    // Return the current frame...so, if we changed the current time can know
    return (int)m_FrameCurrent;
}

int CKeyBar::SetTimeRange( int FrameStart, int FrameEnd )
{
    ASSERT( FrameStart <= FrameEnd );

    m_FrameStart    = (f32)FrameStart;
    m_FrameEnd      = (f32)FrameEnd;
    
    // Validate current frame
    if( m_FrameCurrent < m_FrameStart )
    {
        m_FrameCurrent  = m_FrameStart;
    }
    else if( m_FrameCurrent > m_FrameEnd )
    {
        m_FrameCurrent  = m_FrameEnd;
    }

    // Update Time Slider & Edit Box
    m_TimeSlider.SetTime            ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );

    m_Edit_FrameStart.SetValue      ( (int)m_FrameStart   );
    m_Edit_FrameEnd.SetValue        ( (int)m_FrameEnd     );
    m_Edit_FrameCurrent.SetValue    ( (int)m_FrameCurrent );

    RedrawWindow();

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

    // Return the current frame...so, if we changed the current time can know
    return (int)m_FrameCurrent;
}

int CKeyBar::SetKeySets( int nKeySets, KeySet* pKeySets )
{
    // Just pass the command onto the TimeSlider
    return m_TimeSlider.SetKeySets( nKeySets, pKeySets );
}

void CKeyBar::SetKeySet( int KeySetIdx, KeySet* pKeySet )
{
    m_TimeSlider.SetKeySet( KeySetIdx, pKeySet );
}

void CKeyBar::SetKeySetCount( int nKeySets )
{
    m_TimeSlider.SetKeySetCount( nKeySets );
}

int CKeyBar::SetKeyFilters( int nKeyFilters, KeyFilter* pKeyFilters )
{
    // Store the number of filters
    m_nKeyFilters = nKeyFilters;

    // Initialize the Filter Menu
    m_FilterMenu.CreatePopupMenu();

    for( int i = 0; i < nKeyFilters; i++ )
    {
        m_FilterMenu.AppendMenu( MF_STRING | MF_ENABLED | MF_CHECKED, i, pKeyFilters[i].m_Name );
    }

    m_FilterMenu.AppendMenu( MF_SEPARATOR,           nKeyFilters             );
    m_FilterMenu.AppendMenu( MF_STRING | MF_ENABLED, nKeyFilters + 1, "All"  );
    m_FilterMenu.AppendMenu( MF_STRING | MF_ENABLED, nKeyFilters + 2, "None" );

    // Now pass the command onto the TimeSlider
    return m_TimeSlider.SetKeyFilters( nKeyFilters, pKeyFilters );
}


/////////////////////////////////////////////////////////////////////////////
// CKeyBar message handlers

void CKeyBar::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler )
{

}

CSize CKeyBar::CalcFixedLayout( BOOL bStretch, BOOL bHorz )
{
    CRect   rc;
    GetParent()->GetClientRect( &rc );

    return CSize( rc.right, 64 );
}

void CKeyBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Draw the background
    CRect               rcClient;
    GetClientRect       ( rcClient );
    dc.FillSolidRect    ( rcClient, RGB(160,160,160) );

	// Compute Control placement values
    int VCRWidth        = 416;
    int VCRStart        = ((rcClient.right - VCRWidth) / 2) - 2;
    if( VCRStart < 0 )
    {
        VCRStart = 0;
    }

    int posEditStart        = 6;
    int posEditEnd          = rcClient.right - 70;

    int posTimeSlider       = 76;
    int widthTimeSlider     = posEditEnd - posTimeSlider - 7;

    int posVCR_Filter       = VCRStart;
    int posVCR_Animate      = posVCR_Filter      + 64;
    int posVCR_Home         = posVCR_Animate     + 64;
    int posVCR_StepBack     = posVCR_Home        + 32;
    int posVCR_Play         = posVCR_StepBack    + 32;
    int posVCR_StepForward  = posVCR_Play        + 32;
    int posVCR_End          = posVCR_StepForward + 32;
    int posVCR_Repeat       = posVCR_End         + 32;
    int posVCR_KeyStep      = posVCR_Repeat      + 32;
    int posEditCurrent      = posVCR_KeyStep     + 32;

    int posStatusLeft       = 6;
    int posStatusRight      = posEditCurrent     + 67;
    int widthStatusBox      = VCRStart - 5;

	// Draw the Edit Controls
	m_Edit_FrameStart.MoveWindow    ( posEditStart,    4, 64, 24, TRUE );
	m_Edit_FrameEnd.MoveWindow      ( posEditEnd,      4, 64, 24, TRUE );
	m_Edit_FrameCurrent.MoveWindow  ( posEditCurrent, 36, 64, 24, TRUE );

	// Draw the Time Slider
    m_TimeSlider.MoveWindow         ( posTimeSlider,   2, widthTimeSlider, 28, TRUE );

	// Draw the VCR Buttons
    m_VCR_Filter.MoveWindow         ( posVCR_Filter,      32, 64, 32, TRUE );
    m_VCR_Animate.MoveWindow        ( posVCR_Animate,     32, 64, 32, TRUE );

    m_VCR_Home.MoveWindow           ( posVCR_Home,        32, 32, 32, TRUE );
    m_VCR_StepBack.MoveWindow       ( posVCR_StepBack,    32, 32, 32, TRUE );
    m_VCR_Play.MoveWindow           ( posVCR_Play,        32, 32, 32, TRUE );
    m_VCR_StepForward.MoveWindow    ( posVCR_StepForward, 32, 32, 32, TRUE );
    m_VCR_End.MoveWindow            ( posVCR_End,         32, 32, 32, TRUE );
    m_VCR_Repeat.MoveWindow         ( posVCR_Repeat,      32, 32, 32, TRUE );
    m_VCR_KeyStep.MoveWindow        ( posVCR_KeyStep,     32, 32, 32, TRUE );

	// Draw the Status Boxes
	m_StatusLeft.MoveWindow         ( posStatusLeft,      36, widthStatusBox, 24, TRUE );
	m_StatusRight.MoveWindow        ( posStatusRight,     36, widthStatusBox, 24, TRUE );
}

BOOL CKeyBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
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

    // Create Buttons
    m_VCR_Filter.Create         ( this, "Filter",   32,  36, 32, 32, IDC_VCR_FILTER      );
    m_VCR_Animate.Create        ( this, "Animate",  32,  36, 32, 32, IDC_VCR_ANIMATE     );

    m_VCR_Home.Create           ( this, "A",        32,  36, 32, 32, IDC_VCR_HOME        );
    m_VCR_StepBack.Create       ( this, "B",        64,  36, 32, 32, IDC_VCR_STEPBACK    );
    m_VCR_Play.Create           ( this, "C",        96,  36, 32, 32, IDC_VCR_PLAY        );
    m_VCR_StepForward.Create    ( this, "D",        128, 36, 32, 32, IDC_VCR_STEPFORWARD );
    m_VCR_End.Create            ( this, "E",        160, 36, 32, 32, IDC_VCR_END         );
    m_VCR_Repeat.Create         ( this, "F",        192, 36, 32, 32, IDC_VCR_REPEAT      );
    m_VCR_KeyStep.Create        ( this, "G",        224, 36, 32, 32, IDC_VCR_KEYSTEP     );

    // Set Button Types
    m_VCR_Animate.SetButtonType     ( CPushButtonBmp::BUTTON_TYPE_CHECKBUTTON );
    m_VCR_Play.SetButtonType        ( CPushButtonBmp::BUTTON_TYPE_TOGGLEBUTTON );
    m_VCR_Repeat.SetButtonType      ( CPushButtonBmp::BUTTON_TYPE_TOGGLEBUTTON );
    m_VCR_KeyStep.SetButtonType     ( CPushButtonBmp::BUTTON_TYPE_CHECKBUTTON );

    m_VCR_Animate.SetCheckColor     ( RGB(192,  0,  0) );
    m_VCR_KeyStep.SetCheckColor     ( RGB(153,172,185) );

    // Set Button Bitmaps
    m_VCR_Home.SetPushBitmapUp          ( IDB_VCR_HOME_COLOR,        IDB_VCR_HOME_ALPHA        );
    m_VCR_StepBack.SetPushBitmapUp      ( IDB_VCR_STEPBACK_COLOR,    IDB_VCR_STEPBACK_ALPHA    );
    m_VCR_Play.SetPushBitmapUp          ( IDB_VCR_PLAY_COLOR,        IDB_VCR_PLAY_ALPHA        );
    m_VCR_StepForward.SetPushBitmapUp   ( IDB_VCR_STEPFORWARD_COLOR, IDB_VCR_STEPFORWARD_ALPHA );
    m_VCR_End.SetPushBitmapUp           ( IDB_VCR_END_COLOR,         IDB_VCR_END_ALPHA         );
    m_VCR_Repeat.SetPushBitmapUp        ( IDB_VCR_LOOPON_COLOR,      IDB_VCR_LOOPON_ALPHA      );
    m_VCR_KeyStep.SetPushBitmapUp       ( IDB_VCR_KEYSTEP_COLOR,     IDB_VCR_KEYSTEP_ALPHA     );

    m_VCR_Play.SetToggleBitmapUp        ( IDB_VCR_PAUSE_COLOR,       IDB_VCR_PAUSE_ALPHA       );
    m_VCR_Repeat.SetToggleBitmapUp      ( IDB_VCR_LOOPOFF_COLOR,     IDB_VCR_LOOPOFF_ALPHA     );

    // Create EditInt Boxes
    m_Edit_FrameStart.Create    ( WS_TABSTOP | WS_BORDER | WS_VISIBLE | ES_RIGHT, CRect(0,0,0,0), this, IDC_KEYBAR_EDIT_FRAMESTART );
    m_Edit_FrameEnd.Create      ( WS_TABSTOP | WS_BORDER | WS_VISIBLE | ES_RIGHT, CRect(0,0,0,0), this, IDC_KEYBAR_EDIT_FRAMEEND );
    m_Edit_FrameCurrent.Create  ( WS_TABSTOP | WS_BORDER | WS_VISIBLE | ES_RIGHT, CRect(0,0,0,0), this, IDC_KEYBAR_EDIT_FRAMECURRENT );

    // Set Status Boxes
    m_StatusLeft.Create         ( this, "Create a bad-ass effect",   32, 36, 128, 24, IDC_KEYBAR_STATUSLEFT  );
    m_StatusRight.Create        ( this, "Then put it in the game!", 800, 36, 128, 24, IDC_KEYBAR_STATUSRIGHT );

    // Create Time Slider
    m_TimeSlider.Create         ( this, 76, 2, 1127, 28, IDC_KEYBAR_TIMESLIDER );

    // Set Time Values
    m_FrameStart                = 0;
    m_FrameEnd                  = 90;
    m_FrameCurrent              = 0;

    m_TimeSlider.SetTime                ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );

    m_Edit_FrameStart.SetValue          ( (int)m_FrameStart   );
    m_Edit_FrameEnd.SetValue            ( (int)m_FrameEnd     );
    m_Edit_FrameCurrent.SetValue        ( (int)m_FrameCurrent );

    return TRUE;
}

LRESULT CKeyBar::OnEdit_Entered( WPARAM wParam, LPARAM lParam )
{
    switch( wParam )
    {
        case IDC_KEYBAR_EDIT_FRAMESTART:
        {
            m_FrameStart = (f32)m_Edit_FrameStart.GetValue();

            // Validate range
            if( m_FrameStart > m_FrameEnd )
            {
                m_FrameEnd              = m_FrameStart + 1;
                m_Edit_FrameEnd.SetValue( (int)m_FrameEnd );
            }

            // Validate current frame
            if( m_FrameCurrent < m_FrameStart )
            {
                m_FrameCurrent  = m_FrameStart;
                m_Edit_FrameCurrent.SetValue( (int)m_FrameCurrent );
            }

            m_TimeSlider.SetTime        ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

            break;
        }

        case IDC_KEYBAR_EDIT_FRAMEEND:
        {
            m_FrameEnd = (f32)m_Edit_FrameEnd.GetValue();

            // Validate range
            if( m_FrameEnd < m_FrameStart )
            {
                m_FrameStart    = m_FrameEnd - 1;
                m_Edit_FrameEnd.SetValue( (int)m_FrameEnd );
            }

            // Validate current frame
            if( m_FrameCurrent > m_FrameEnd )
            {
                m_FrameCurrent  = m_FrameEnd;
                m_Edit_FrameCurrent.SetValue( (int)m_FrameCurrent );
            }

            m_TimeSlider.SetTime        ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

            break;
        }

        case IDC_KEYBAR_EDIT_FRAMECURRENT:
        {
            m_FrameCurrent = (f32)m_Edit_FrameCurrent.GetValue();

            // Validate current frame
            if( m_FrameCurrent < m_FrameStart )
            {
                m_FrameCurrent  = m_FrameStart;
                m_Edit_FrameCurrent.SetValue( (int)m_FrameCurrent );
            }
            else if( m_FrameCurrent > m_FrameEnd )
            {
                m_FrameCurrent  = m_FrameEnd;
                m_Edit_FrameCurrent.SetValue( (int)m_FrameCurrent );
            }

            m_TimeSlider.SetTime        ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

            break;
        }
    }

    RedrawWindow();

    return TRUE;
}

LRESULT CKeyBar::OnTimeSlider_Edit_Begin( WPARAM wParam, LPARAM lParam )
{
    StopPlayback();

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

    return TRUE;
}

LRESULT CKeyBar::OnTimeSlider_FrameStart_Changed( WPARAM wParam, LPARAM lParam )
{
    m_FrameStart = (f32)lParam;
    m_Edit_FrameStart.SetValue( (int)m_FrameStart );

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

    return TRUE;
}

LRESULT CKeyBar::OnTimeSlider_FrameEnd_Changed( WPARAM wParam, LPARAM lParam )
{
    m_FrameEnd = (f32)lParam;
    m_Edit_FrameEnd.SetValue( (int)m_FrameEnd );

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

    return TRUE;
}

LRESULT CKeyBar::OnTimeSlider_FrameCurrent_Changed( WPARAM wParam, LPARAM lParam )
{
    m_FrameCurrent = (f32)lParam;
    m_Edit_FrameCurrent.SetValue( (int)m_FrameCurrent );

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

    return TRUE;
}

LRESULT CKeyBar::OnTimeSlider_Keys_Selection_Changed( WPARAM wParam, LPARAM lParam )
{
    char        buffer[256];
    CString     StatusText;

    itoa        ( lParam, buffer, 10 );

    StatusText  = "Keys Selected: ";
    StatusText  += buffer;

    // Update the Time Slider Status Box
    m_StatusRight.SetStatusText( StatusText );

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_KEYS_SELECTION_CHANGED, controlID, 123456789 );

    return TRUE;
}

LRESULT CKeyBar::OnTimeSlider_Keys_Moved( WPARAM wParam, LPARAM lParam )
{
    char        buffer[256];
    CString     StatusText;

    itoa        ( lParam, buffer, 10 );

    StatusText  = "Keys Moved: ";
    if( lParam > 0 )
    {
        StatusText  += "+";
    }
    StatusText  += buffer;

    // Update the Time Slider Status Box
    m_StatusRight.SetStatusText( StatusText );

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_KEYS_MOVED, controlID, 123456789 );

    return TRUE;
}

LRESULT CKeyBar::OnTimeSlider_Keys_Deleted( WPARAM wParam, LPARAM lParam )
{
    char        buffer[256];
    CString     StatusText;

    itoa        ( lParam, buffer, 10 );

    StatusText  = "Keys Deleted: ";
    StatusText  += buffer;

    // Update the Time Slider Status Box
    m_StatusRight.SetStatusText( StatusText );

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_KEYS_DELETED, controlID, 123456789 );

    return TRUE;
}

LRESULT CKeyBar::OnTimeSlider_Keys_Changed( WPARAM wParam, LPARAM lParam )
{
    // Just pass this up to the parent window
    GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_KEYS_CHANGED, wParam, lParam );

    return TRUE;
}

LRESULT CKeyBar::OnPushButton_Clicked( WPARAM wParam, LPARAM lParam )
{
    switch( wParam )
    {
        case IDC_VCR_FILTER:
        {
            CRect   rcFilter;

            m_VCR_Filter.GetWindowRect( &rcFilter );
            m_FilterMenu.TrackPopupMenu( TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON, rcFilter.left, rcFilter.top, this );

            break;
        }

        case IDC_VCR_ANIMATE:
        {
            // Get anim state from lParam
            m_IsAnimateOn = (lParam != 0);

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_ANIMMODECHANGE, controlID, m_IsAnimateOn );

            // Change the keybar components to animate mode colors
            m_TimeSlider.SetAnimateMode         ( m_IsAnimateOn );

            if( m_IsAnimateOn )
            {
                m_StatusLeft.SetBackgroundColor     ( RGB(192,0,0) );
                m_StatusRight.SetBackgroundColor    ( RGB(192,0,0) );
            }
            else
            {
                m_StatusLeft.SetBackgroundColor     (); // Go back to default color
                m_StatusRight.SetBackgroundColor    (); // Go back to default color
            }

            break;
        }

        case IDC_VCR_HOME:
        {
            m_FrameCurrent                  = m_FrameStart;
            m_TimeSlider.SetTime            ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );
            m_Edit_FrameCurrent.SetValue    ( (int)m_FrameCurrent );

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

            break;
        }

        case IDC_VCR_STEPBACK:
        {
            m_FrameCurrent -= 1;

            // Don't allow us to back up before the start time
            if( m_FrameCurrent < m_FrameStart )     { m_FrameCurrent = m_FrameStart; }

            // Update the Time Slider...but only if we're still inside it's time rage
            if( m_FrameCurrent <= m_FrameEnd   )
            {
                m_TimeSlider.SetTime            ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );
            }

            m_Edit_FrameCurrent.SetValue    ( (int)m_FrameCurrent );

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

            break;
        }

        case IDC_VCR_PLAY:
        {
            if( m_PlayTimer != 0 )
            {
                KillTimer   ( m_PlayTimer );
                m_PlayTimer = 0;
                m_IsPlayOn  = false;
                m_Timer.Stop();

                // JV 3-17-2003: Not sure why this was here...taking it out so that it
                // stops on the frame you were at, instead of clamping to the end frame if you were out of bounds
                //SetTime( (int)GetTime() );
            }
            else
            {
                // If Looping is off and we're on the last frame then reset time before playing
                if( !m_DoLoopPlayback && (m_FrameCurrent == m_FrameEnd) )
                {
                    m_FrameCurrent = m_FrameStart;
                }

                m_PlayTimer = SetTimer( 1, 33, NULL );
                m_Timer.Reset();
                m_Timer.Start();
                m_IsPlayOn  = true;
            }

            break;
        }

        case IDC_VCR_STEPFORWARD:
        {
            m_FrameCurrent += 1;

            if( m_FrameCurrent > m_FrameEnd )
            {
                m_FrameCurrent = m_FrameEnd;
            }

            m_TimeSlider.SetTime            ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );
            m_Edit_FrameCurrent.SetValue    ( (int)m_FrameCurrent );

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

            break;
        }

        case IDC_VCR_END:
        {
            m_FrameCurrent                  = m_FrameEnd;
            m_TimeSlider.SetTime            ( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );
            m_Edit_FrameCurrent.SetValue    ( (int)m_FrameCurrent );

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

            break;
        }

        case IDC_VCR_REPEAT:
        {
            // Get the inverted toggle button state, since our unchecked(false) state really means do(true) loop
            m_DoLoopPlayback = (lParam != 1);

            // Send an update message to parent window
            LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
            GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_PLAYBACKMODECHANGE, controlID, m_DoLoopPlayback );

            break;
        }

        case IDC_VCR_KEYSTEP:
        {
            m_DoKeyStep = !m_DoKeyStep;
            break;
        }
    }

    return TRUE;
}

BOOL CKeyBar::OnCommand(WPARAM wParam, LPARAM lParam) 
{
    // Check to see if a menu sent the command ( HIWORD(wParam) == 0 means it was a menu)
    if( HIWORD(wParam) == 0 )
    {
        int nID = LOWORD(wParam);

        if( nID == (m_nKeyFilters + 1) )
        {
            // Turn all filters ON
            for( int i = 0; i < m_nKeyFilters; i++ )
            {
                m_FilterMenu.CheckMenuItem      ( i, MF_CHECKED | MF_BYCOMMAND );
                m_TimeSlider.SetKeyFilterState  ( i, true );
            }

	        return TRUE;
        }
        else if( nID == (m_nKeyFilters + 2) )
        {
            // Turn all filters OFF
            for( int i = 0; i < m_nKeyFilters; i++ )
            {
                m_FilterMenu.CheckMenuItem      ( i, MF_UNCHECKED | MF_BYCOMMAND );
                m_TimeSlider.SetKeyFilterState  ( i, false );
            }

	        return TRUE;
        }
        else if( m_FilterMenu.GetMenuState(nID, MF_BYCOMMAND) & MF_CHECKED )
        {
            // It was checked...so now it's OFF
            m_FilterMenu.CheckMenuItem      ( nID, MF_UNCHECKED | MF_BYCOMMAND );
            m_TimeSlider.SetKeyFilterState  ( nID, false );

	        return TRUE;
        }
        else
        {
            // It was checked...so now it's ON
            m_FilterMenu.CheckMenuItem      ( nID, MF_CHECKED | MF_BYCOMMAND );
            m_TimeSlider.SetKeyFilterState  ( nID, true );

	        return TRUE;
        }
    }

    // Pass on any other messages to default handler	
	return CControlBar::OnCommand(wParam, lParam);
}

void CKeyBar::OnTimer(UINT nIDEvent) 
{
    if( nIDEvent == m_PlayTimer )
    {
        KillTimer( m_PlayTimer );
        m_PlayTimer = 0;

        m_FrameCurrent += m_Timer.ReadSec() * 30.0f;
        m_Timer.Reset();
        m_Timer.Start();

        if( m_FrameCurrent > m_FrameEnd )
        {
            if( m_DoLoopPlayback )
            {
                m_FrameCurrent = m_FrameStart;
                m_Timer.Reset();
                m_Timer.Start();
            }
            else
            {
                //**************************************************************************************
                // For now, play once will continue to play into infinity
                // This may change later if we get a better way to handle inifinite and non-infinite f/x
                // So, I'm just commenting this code out for now ( Nov. 11, 2002 John Versluis )
                //**************************************************************************************
                /***************************************************************************************
                m_FrameCurrent  = m_FrameEnd;
                KillTimer       ( m_PlayTimer );
                m_PlayTimer     = 0;
                m_IsPlayOn      = false;
                ****************************************************************************************/
            }
        }

        //**********************************************************************************************
        // Only update Time Slider if we're still in bounds...we may go out when playing an infinite effect
        // This behavior may also be temporary if we update how to handle infinite/non-infinite f/x
        if( m_FrameCurrent <= m_FrameEnd )
        {
            m_TimeSlider.SetTime( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameCurrent );
        }
        else
        {
            m_TimeSlider.SetTime( (int)m_FrameStart, (int)m_FrameEnd, (int)m_FrameEnd );
        }


        // Update the Edit Box either way to show an accurate view of what frame we're really playing
        m_Edit_FrameCurrent.SetValue    ( (int)m_FrameCurrent );
        //**********************************************************************************************

        // Send an update message to parent window
        LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
        GetParent()->SendMessage    ( WM_USER_MSG_KEYBAR_TIMECHANGE, controlID, (int)m_FrameCurrent );

        m_PlayTimer = SetTimer( 1, 33, NULL );
    }
    else
    {
	    CControlBar::OnTimer(nIDEvent);
    }
}

bool    CKeyBar::IsPlayOn( void ) 
{
    return m_IsPlayOn;
}

void    CKeyBar::StartPlayback( void ) 
{
    if ( m_IsPlayOn == FALSE )
    {
        OnPushButton_Clicked( IDC_VCR_PLAY, 0 );
        m_VCR_Play.SetIsChecked( true );
    }
}

void    CKeyBar::StopPlayback( void ) 
{
    if ( m_IsPlayOn == TRUE )
    {
        OnPushButton_Clicked( IDC_VCR_PLAY, 0 );
        m_VCR_Play.SetIsChecked( false );
    }
}