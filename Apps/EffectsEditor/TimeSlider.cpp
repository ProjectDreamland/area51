// TimeSlider.cpp : implementation file
//

#include "stdafx.h"
#include "PartEd.h"
#include "TimeSlider.h"
#include "DoubleBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTimeSlider

CTimeSlider::CTimeSlider()
{
    // Draw Properties
    m_ColorBorder               = RGB( 128, 128, 128 );
    m_ColorBackground           = RGB( 160, 160, 160 );
                                
    m_ColorFrameLines           = RGB( 121, 126, 129 );
    m_ColorFrameText            = RGB(   0,   0,   0 );
                                
    m_ColorTimeFill             = RGB( 153, 172, 185 );
    m_ColorTimeBorder           = RGB(  96,  96,  96 );
                                
    m_ColorKeyBorder            = RGB(  48,  48,  48 );
                                
    m_Font.CreateFont           ( 15,                          // Height
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

    m_Width                     = 0;
    m_WidthFrames               = 0;
    m_Height                    = 0;
    m_HeightFrames              = 0;

    m_DrawSelectMarquee         = false;
    m_AnimateModeOn             = false;

    // Time Properties
    m_FrameStart                = 0;
    m_FrameEnd                  = 85;
    m_FrameCurrent              = 0;

    // Key Properties
    m_nKeyFilters               = 0;
    m_pKeyFilters               = NULL;

    m_nKeySets                  = 0;
    m_pKeySets                  = NULL;

    m_nEditKeySets              = 0;
    m_piEditKeySets             = NULL;

    m_pKeySets_Edit_InActive    = NULL;
    m_pKeySets_Edit_Active      = NULL;
    m_pKeySets_Edit_ActiveRef   = NULL;

    // Navigation Properties
    m_NavigateType              = NAVIGATE_NULL;

    m_NavigateTimeStart         = 0;
    m_NavigateTimeEnd           = 0;
    m_NavigateTimeCurrent       = 0;
}

CTimeSlider::~CTimeSlider()
{
    if( m_pKeyFilters )
    {
        delete[] m_pKeyFilters;
        m_pKeyFilters = NULL;
    }

    if( m_pKeySets )
    {
        delete[] m_pKeySets;
        m_pKeySets = NULL;
    }

    if( m_piEditKeySets )
    {
        delete[] m_piEditKeySets;
        m_piEditKeySets = NULL;
    }

    if( m_pKeySets_Edit_InActive )
    {
        delete[] m_pKeySets_Edit_InActive;
        m_pKeySets_Edit_InActive = NULL;
    }

    if( m_pKeySets_Edit_Active )
    {
        delete[] m_pKeySets_Edit_Active;
        m_pKeySets_Edit_Active = NULL;
    }

    if( m_pKeySets_Edit_ActiveRef )
    {
        delete[] m_pKeySets_Edit_ActiveRef;
        m_pKeySets_Edit_ActiveRef = NULL;
    }
}


BEGIN_MESSAGE_MAP(CTimeSlider, CWnd)
	//{{AFX_MSG_MAP(CTimeSlider)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeSlider Public Functions
void CTimeSlider::SetKeySetCount( int nKeySets )
{
    // Clear out the old KeySets
    if( m_pKeySets )
    {
        delete[]    m_pKeySets;
        m_pKeySets  = NULL;
    }

    if ( nKeySets == 0 )
        return;

    m_nKeySets  = nKeySets;
    m_pKeySets  = new KeySet[nKeySets];
}

void CTimeSlider::SetKeySet( int KeySetIdx, KeySet* pKeySet )
{
    ASSERT( KeySetIdx < m_nKeySets );
    // ASSERT( m_pKeySets );

    m_pKeySets[KeySetIdx].m_Category    = pKeySet->m_Category;
    m_pKeySets[KeySetIdx].m_nKeys       = pKeySet->m_nKeys;
    m_pKeySets[KeySetIdx].m_KeyDataSize = pKeySet->m_KeyDataSize;
    m_pKeySets[KeySetIdx].m_UserData    = pKeySet->m_UserData;

    int     KeyDataSize                 = pKeySet->m_KeyDataSize;

    if( m_pKeySets[KeySetIdx].m_nKeys > 0 )
    {
        if ( m_pKeySets[KeySetIdx].m_pKeys )
        {
            delete[] m_pKeySets[KeySetIdx].m_pKeys;
            m_pKeySets[KeySetIdx].m_pKeys = NULL;
        }

        m_pKeySets[KeySetIdx].m_pKeys   = new KeyBarKey[ m_pKeySets[KeySetIdx].m_nKeys ];

        for( int j = 0; j < m_pKeySets[KeySetIdx].m_nKeys; j++ )
        {
            m_pKeySets[KeySetIdx].m_pKeys[j].m_Time         = pKeySet->m_pKeys[j].m_Time;
            m_pKeySets[KeySetIdx].m_pKeys[j].m_IsSelected   = pKeySet->m_pKeys[j].m_IsSelected;

            // Allocate and copy in key data
            m_pKeySets[KeySetIdx].m_pKeys[j].m_pData        = malloc( KeyDataSize );

            memcpy( m_pKeySets[KeySetIdx].m_pKeys[j].m_pData, pKeySet->m_pKeys[j].m_pData, KeyDataSize );
        }
    }

    RedrawWindow();
}


int CTimeSlider::SetKeySets( int nKeySets, KeySet* pKeySets )
{
    // Clear out the old KeySets
    if( m_pKeySets )
    {
        delete[]    m_pKeySets;
        m_pKeySets  = NULL;

        if ( !nKeySets )
        {
            m_nKeySets = 0;
            RedrawWindow();
            return 0;
        }
    }

    // If there are no KeySets to use then drop out
    if( (nKeySets <= 0) || !pKeySets )
    {
        m_nKeySets = 0;
        return m_nKeySets;
    }

    // We have new KeySets...so let's make our own copy of them!
    m_nKeySets  = nKeySets;
    m_pKeySets  = new KeySet[nKeySets];

    int     KeyDataSize;

    for( int i = 0; i < nKeySets; i++ )
    {
        m_pKeySets[i].m_Category    = pKeySets[i].m_Category;
        m_pKeySets[i].m_nKeys       = pKeySets[i].m_nKeys;
        m_pKeySets[i].m_KeyDataSize = pKeySets[i].m_KeyDataSize;
        m_pKeySets[i].m_UserData    = pKeySets[i].m_UserData;

        KeyDataSize                 = pKeySets[i].m_KeyDataSize;

        if( m_pKeySets[i].m_nKeys > 0 )
        {
            m_pKeySets[i].m_pKeys   = new KeyBarKey[ m_pKeySets[i].m_nKeys ];

            for( int j = 0; j < m_pKeySets[i].m_nKeys; j++ )
            {
                m_pKeySets[i].m_pKeys[j].m_Time         = pKeySets[i].m_pKeys[j].m_Time;
                m_pKeySets[i].m_pKeys[j].m_IsSelected   = pKeySets[i].m_pKeys[j].m_IsSelected;

                // Allocate and copy in key data
                m_pKeySets[i].m_pKeys[j].m_pData        = malloc( KeyDataSize );

                memcpy( m_pKeySets[i].m_pKeys[j].m_pData, pKeySets[i].m_pKeys[j].m_pData, KeyDataSize);
            }
        }
    }

    RedrawWindow();

    // Return the final number of KeySets we're using
    return m_nKeySets;
}

int CTimeSlider::SetKeyFilters( int nKeyFilters, KeyFilter* pKeyFilters )
{
    ASSERT( pKeyFilters );  // Don't accept a NULL set of filters

    // Copy the filters into local storage
    m_nKeyFilters   = nKeyFilters;

    if ( m_pKeyFilters )
    {
        delete[] m_pKeyFilters;
        m_pKeyFilters = NULL;
    }

    m_pKeyFilters   = new KeyFilter[ m_nKeyFilters ];

    for( int i = 0; i < nKeyFilters; i++ )
    {
        m_pKeyFilters[i].m_Name         = pKeyFilters[i].m_Name;
        m_pKeyFilters[i].m_Color        = pKeyFilters[i].m_Color;
        m_pKeyFilters[i].m_IsVisible    = pKeyFilters[i].m_IsVisible;
    }

    return m_nKeyFilters;
}

bool    CTimeSlider::SetKeyFilterState( int iKeyFilter, bool FilterState )
{
    ASSERT( iKeyFilter < m_nKeyFilters );  // If the filter index is out of range...something's wrong

    // Update the Filter State
    m_pKeyFilters[iKeyFilter].m_IsVisible   = FilterState;

    // Update the keyframe display
   RedrawWindow();

    return FilterState;
}

bool    CTimeSlider::CheckFilter( KeySet& CheckKeySet ) const
{
    // If the app has no filters to check, then all KeySets should be shown
    if( m_nKeyFilters <= 0 )
    {
        return true;
    }

    // Check the filters
    for( int i = 0; i < m_nKeyFilters; i++)
    {
        if( m_pKeyFilters[i].m_IsVisible )
        {
            if( m_pKeyFilters[i].m_Name == CheckKeySet.m_Category )
            {
                return true; // We found a name match that is turned on...success!
            }
        }
    }

    // No valid filters were found
    return false;
}

COLORREF    CTimeSlider::GetFilterColor( KeySet& CheckKeySet ) const
{
    for( int i = 0; i < m_nKeyFilters; i++)
    {
        if( m_pKeyFilters[i].m_Name == CheckKeySet.m_Category )
        {
            return m_pKeyFilters[i].m_Color; // We found a name match...success!
        }
    }

    // No match was found or the app has no filters
    // So...return a default key color
    return RGB( 86, 128, 162 );
}

int     CTimeSlider::GetNumSelectedKeys( void ) const
{
    // ASSERT( m_pKeySets );    // We should never have a NULL KeySet...a KeySet with zero keys is fine though

    int numSelKeys  = 0;

    // Count up all the selected keys in our KeySets that are not hidden by a filter
    for( int i = 0; i < m_nKeySets; i++ )
    {
        if( CheckFilter( m_pKeySets[i] ) )
        {
            for( int j = 0; j < m_pKeySets[i].m_nKeys; j++ )
            {
                if( m_pKeySets[i].m_pKeys[j].m_IsSelected )
                {
                    numSelKeys++;
                }
            }
        }
    }

    return numSelKeys;
}

void    CTimeSlider::DeselectVisibleKeys( void )
{
    // ASSERT( m_pKeySets );    // We should never have a NULL KeySet...a KeySet with zero keys is fine though

    for( int i = 0; i < m_nKeySets; i++ )
    {
        // Check filters
        if( CheckFilter( m_pKeySets[i] ) )
        {
            for( int j = 0; j < m_pKeySets[i].m_nKeys; j++ )
            {
                m_pKeySets[i].m_pKeys[j].m_IsSelected = false;
            }
        }
    }

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_KEYS_SELECTION_CHANGED, controlID, 0 );
}

bool    CTimeSlider::IsMouseOverValidKey( CPoint point, bool CheckSelection ) const
{
    // ASSERT( m_pKeySets );    // We should never have a NULL KeySet...a KeySet with zero keys is fine though

    float       frameRes;
    int         keyTime;
    int         keyTimeOffset;
    CRect       rcKey;

    frameRes    = float(m_WidthFrames) / float(m_FrameEnd - m_FrameStart);

    for( int i = 0; i < m_nKeySets; i++ )
    {
        // Check filters
        if( CheckFilter( m_pKeySets[i] ) )
        {
            for( int j = 0; j < m_pKeySets[i].m_nKeys; j++ )
            {
                keyTime     = m_pKeySets[i].m_pKeys[j].m_Time;

                // Only check keys in the visible time range
                if( (keyTime >= m_FrameStart) && (keyTime <= m_FrameEnd) )
                {
                    keyTimeOffset       = keyTime - m_FrameStart;
                    rcKey.left          = 10 + ( int(keyTimeOffset * frameRes) ) - 5;
                    rcKey.right         = 10 + ( int(keyTimeOffset * frameRes) ) + 6;
                    rcKey.top           = 1;
                    rcKey.bottom        = m_Height - 16;

                    if( (point.x >= rcKey.left) && (point.x <= rcKey.right) )
                    {
                        if( (point.y >= rcKey.top) && (point.y <= rcKey.bottom) )
                        {
                            if( CheckSelection )
                            {
                                if( m_pKeySets[i].m_pKeys[j].m_IsSelected )
                                {
                                    return true;    // SUCCESS!!
                                }
                            }
                            else
                            {
                                return true;        // SUCCESS!!
                            }
                        }
                    }
                }
            }
        }
    }

    // No visible key was found 
    return false;
}

void    CTimeSlider::SetKeyIsSelectedByPoint( CPoint point, bool IsSelected )
{
    // ASSERT( m_pKeySets );    // We should never have a NULL KeySet...a KeySet with zero keys is fine though

    float       frameRes;
    int         keyTime;
    int         keyTimeOffset;
    CRect       rcKey;

    frameRes    = float(m_WidthFrames) / float(m_FrameEnd - m_FrameStart);

    for( int i = 0; i < m_nKeySets; i++ )
    {
        // Check filters
        if( CheckFilter( m_pKeySets[i] ) )
        {
            for( int j = 0; j < m_pKeySets[i].m_nKeys; j++ )
            {
                keyTime     = m_pKeySets[i].m_pKeys[j].m_Time;

                // Only check keys in the visible time range
                if( (keyTime >= m_FrameStart) && (keyTime <= m_FrameEnd) )
                {
                    keyTimeOffset       = keyTime - m_FrameStart;
                    rcKey.left          = 10 + ( int(keyTimeOffset * frameRes) ) - 5;
                    rcKey.right         = 10 + ( int(keyTimeOffset * frameRes) ) + 6;
                    rcKey.top           = 1;
                    rcKey.bottom        = m_Height - 16;

                    if( (point.x >= rcKey.left) && (point.x <= rcKey.right) )
                    {
                        if( (point.y >= rcKey.top) && (point.y <= rcKey.bottom) )
                        {
                            m_pKeySets[i].m_pKeys[j].m_IsSelected = IsSelected;   // SUCCESS!!
                        }
                    }
                }
            }
        }
    }

    int         totalKeysSelected   = GetNumSelectedKeys();
    
    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_KEYS_SELECTION_CHANGED, controlID, totalKeysSelected );
}

void    CTimeSlider::SetKeyIsSelectedByRange( int posX1, int posX2, bool IsSelected )
{
    // ASSERT( m_pKeySets );    // We should never have a NULL KeySet...a KeySet with zero keys is fine though

    float       frameRes;
    int         keyTime;
    int         keyTimeOffset;
    CRect       rcKey;

    frameRes    = float(m_WidthFrames) / float(m_FrameEnd - m_FrameStart);

    for( int i = 0; i < m_nKeySets; i++ )
    {
        // Check filters
        if( CheckFilter( m_pKeySets[i] ) )
        {
            for( int j = 0; j < m_pKeySets[i].m_nKeys; j++ )
            {
                keyTime     = m_pKeySets[i].m_pKeys[j].m_Time;

                // Only check keys in the visible time range
                if( (keyTime >= m_FrameStart) && (keyTime <= m_FrameEnd) )
                {
                    keyTimeOffset       = keyTime - m_FrameStart;
                    rcKey.left          = 10 + ( int(keyTimeOffset * frameRes) ) - 5;
                    rcKey.right         = 10 + ( int(keyTimeOffset * frameRes) ) + 6;

                    // Check range using crossing(ie If the marquee slightly touches it, it will select/de-select)
                    if(  ((posX1 <= rcKey.right) && (posX2 >= rcKey.left))
                      || ((posX2 <= rcKey.right) && (posX1 >= rcKey.left)) )
                    {
                        m_pKeySets[i].m_pKeys[j].m_IsSelected = IsSelected;   // SUCCESS!!
                    }
                }
            }
        }
    }

    int         totalKeysSelected   = GetNumSelectedKeys();

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_KEYS_SELECTION_CHANGED, controlID, totalKeysSelected );
}

/////////////////////////////////////////////////////////////////////////////
// CTimeSlider message handlers

BOOL CTimeSlider::Create(CWnd* pParentWnd, int posX, int posY, int nWidth, int nHeight, UINT nID)
{
    m_Width         = nWidth;
    m_WidthFrames   = nWidth - 20;
    m_Height        = nHeight;
    m_HeightFrames  = m_Height - 2;

    CString         winClassName;
    CRect           winRect;

	winClassName    = AfxRegisterWndClass   ( CS_DBLCLKS,                       // Class Style
                                              ::LoadCursor(NULL, IDC_ARROW),    // Cursor
                                              NULL,                             // Background
                                              0 );                              // Icon
    winRect.left    = posX;
    winRect.right   = posX + m_Width;
    winRect.top     = posY;
    winRect.bottom  = posY + m_Height;

	return CWnd::Create(winClassName, "", WS_CHILD | WS_VISIBLE, winRect, pParentWnd, nID, NULL);
}

void CTimeSlider::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint )
{
    // Need to reset internal values
    m_Width         = nWidth;
    m_WidthFrames   = nWidth - 20;
    m_Height        = nHeight;
    m_HeightFrames  = nHeight - 2;

    // Now we can go ahead and move/resize
    CWnd::MoveWindow( x, y, m_Width, m_Height, TRUE );
}

void CTimeSlider::OnPaint() 
{
	CPaintDC* pDC = new CPaintDC(this);     // device context for painting

    // Set clipping region so the double buffered blit will not blit unrendered areas
    CRgn r;
    r.CreateRoundRectRgn          ( 0, 0, m_Width, m_Height, 4, 4 );
    pDC->SelectClipRgn( &r, RGN_AND );
    
    // Create double buffer
    CDoubleBuffer db( pDC );

    int i;

    CRect           rcClient;
    CRect           rcTime;
    CRect           rcMarquee;

    CRgn            rgn;

    CBrush          brBorder;
    CBrush          brBackground;
    CBrush          brTimeFill;
    CBrush          brTimeBorder;
    CBrush          brKeyBorder;

    GetClientRect   ( &rcClient );

    if( m_AnimateModeOn )
    {
        brBorder.CreateSolidBrush       ( RGB(192,0,0) );
    }
    else
    {
        brBorder.CreateSolidBrush       ( m_ColorBorder );
    }

    brBackground.CreateSolidBrush   ( m_ColorBackground );
    brTimeFill.CreateSolidBrush     ( m_ColorTimeFill );
    brTimeBorder.CreateSolidBrush   ( m_ColorTimeBorder );
    brKeyBorder.CreateSolidBrush    ( m_ColorKeyBorder );

    pDC->SetBkMode                  ( TRANSPARENT );
    pDC->SetTextColor               ( m_ColorFrameText );

    int             framePos;
    int             iteration;
    float           frameRes        = float(m_WidthFrames) / float(m_FrameEnd - m_FrameStart);
    int             frameIndicator  = m_FrameCurrent - m_FrameStart;

    rcTime.left     = 10 + ( int(frameIndicator * frameRes) ) - 4;
    rcTime.right    = 10 + ( int(frameIndicator * frameRes) ) + 5;
    rcTime.top      = 1;
    rcTime.bottom   = m_Height - 1;

    // Set the selection marquee rect based on the navigation points that are set during mouse moves/clicks
    if( m_NavigatePoint.x < m_NavigateDragPoint.x )
    {
        rcMarquee.left      = m_NavigatePoint.x;
        rcMarquee.right     = m_NavigateDragPoint.x;
    }
    else
    {
        rcMarquee.left      = m_NavigateDragPoint.x;
        rcMarquee.right     = m_NavigatePoint.x;
    }

    rcMarquee.top           = 0;
    rcMarquee.bottom        = m_Height - 2;

    // Draw the Time Slider border outline
    rgn.CreateRoundRectRgn          ( 0, 0, m_Width, m_Height, 4, 4 );
    pDC->FillRgn                    ( &rgn, &brBackground );
    pDC->FrameRgn                   ( &rgn, &brBorder, 1, 1 );

    // Draw Selection Marquee background...if we're in selection mode
    if( m_DrawSelectMarquee )
    {
        pDC->FillSolidRect      ( rcMarquee, RGB(196,196,196) );
    }

    // Draw Current Time Indicator
    pDC->FillSolidRect              ( rcTime, m_ColorTimeFill );
    pDC->FrameRect                  ( rcTime, &brTimeBorder );

    // Draw Keyframes
    int         keyTime;
    int         keyTimeOffset;
    CRect       rcKey;
    COLORREF    keyColor;

    // // ASSERT( m_pKeySets );    // We should never have a NULL KeySet...a KeySet with zero keys is fine though

    for( i = 0; i < m_nKeySets; i++ )
    {
        // Check filters
        if( CheckFilter( m_pKeySets[i] ) )
        {
            keyColor    = GetFilterColor( m_pKeySets[i] );

            for( int j = 0; j < m_pKeySets[i].m_nKeys; j++ )
            {
                keyTime     = m_pKeySets[i].m_pKeys[j].m_Time;

                // Only draw keys that are in the visible time range
                if( (keyTime >= m_FrameStart) && (keyTime <= m_FrameEnd) )
                {
                    keyTimeOffset       = keyTime - m_FrameStart;
                    rcKey.left          = 10 + ( int(keyTimeOffset * frameRes) ) - 5;
                    rcKey.right         = 10 + ( int(keyTimeOffset * frameRes) ) + 6;
                    rcKey.top           = 1;
                    rcKey.bottom        = m_Height - 16;

                    // Draw selected keys white...draw others with filter color
                    if( m_pKeySets[i].m_pKeys[j].m_IsSelected )
                    {
                        pDC->FillSolidRect    ( rcKey, RGB(255,255,255) );
                        pDC->FrameRect        ( rcKey, &brKeyBorder );
                    }
                    else
                    {
                        pDC->FillSolidRect    ( rcKey, keyColor );
                        pDC->FrameRect        ( rcKey, &brKeyBorder );
                    }
                }
            }
        }
    }

    // Draw Frame Lines
    int             majorFrame;
    int             minorFrame;
    int             lineStart;
    char            buffer[256];
    pDC->SelectObject ( &m_Font );

    if( frameRes > 10.0 )           { majorFrame  =    5; }
    else if( frameRes > 5.00 )      { majorFrame  =   10; }
    else if( frameRes > 2.50 )      { majorFrame  =   25; }
    else if( frameRes > 1.00 )      { majorFrame  =   50; }
    else if( frameRes > 0.50 )      { majorFrame  =  100; }
    else if( frameRes > 0.25 )      { majorFrame  =  250; }
    else if( frameRes > 0.10 )      { majorFrame  =  500; }
    else                            { majorFrame  = 1000; }

    minorFrame      = majorFrame / 5;
    lineStart       = m_FrameStart + ( minorFrame - (m_FrameStart % minorFrame) );

    for( i = lineStart; i < m_FrameEnd + 1; i += minorFrame )
    {
        iteration           = i - m_FrameStart;
        framePos            = 10 + ( int(iteration * frameRes) );

        pDC->FillSolidRect    ( framePos, 1, 1, (m_Height - 16), m_ColorFrameLines );

        // Draw Major Frame Lines & Numbers on Major Frame numbers
        if( (i % majorFrame) == 0 )
        {
            // Draw Frame Lines
            iteration           = i - m_FrameStart;
            framePos            = 10 + ( int(iteration * frameRes) );

            pDC->FillSolidRect    ( framePos,  1, 1, (m_Height - 14), m_ColorFrameLines );
            pDC->FillSolidRect    ( framePos, (m_Height - 6), 1,  4, m_ColorFrameLines );

            // Draw Frame Numbers
            itoa        ( i, buffer, 10 );
            CString     frameString = buffer;

            framePos     = 10 + ( int(iteration * frameRes) );
            framePos     -= pDC->GetTextExtent(frameString).cx / 2;

            pDC->TextOut  ( framePos, (m_Height - 17), frameString );
        }
    }

    // Draw Selection Marquee Outline...if we're in selection mode
    if( m_DrawSelectMarquee )
    {
        // Now draw it!
        CPen                    penMarquee;
        penMarquee.CreatePen    ( PS_SOLID, 1, RGB(255,255,255) );  // Use PS_DOT for normal marquee style border

        pDC->SelectObject       ( penMarquee );
        pDC->MoveTo             ( rcMarquee.left,  rcMarquee.top    );
        pDC->LineTo             ( rcMarquee.right, rcMarquee.top    );
        pDC->LineTo             ( rcMarquee.right, rcMarquee.bottom );
        pDC->LineTo             ( rcMarquee.left,  rcMarquee.bottom );
        pDC->LineTo             ( rcMarquee.left,  rcMarquee.top    );
    }
}

void CTimeSlider::OnLButtonDown(UINT nFlags, CPoint point) 
{
    // Let the parent window know that we're about to start an edit....so they can stop playback, etc
    BeginEdit();

    // Make sure the Time Slider gets all further input
    SetFocus();

    // Capture the mouse
    SetCapture();

    // Store navigation info
    m_NavigatePoint             = point;                            // Save initial click point
    m_NavigateTimeStart         = m_FrameStart;                     // Save initial start frame
    m_NavigateTimeEnd           = m_FrameEnd;                       // Save initial end frame
    m_NavigateTimeCurrent       = m_FrameCurrent;                   // Save initial current frame

    if( IsMouseOverValidKey( point, false ) )
    {
        // Check to see if we need to change key selection
        if( nFlags & MK_CONTROL )
        {
            // Add to selection
            SetKeyIsSelectedByPoint( point, true );
        }
        else if( GetKeyState(VK_MENU) & 0x8000 ) // (VK_MENU == Alt Key) (0x8000 == Check high bit)
        {
            // Remove from selection
            SetKeyIsSelectedByPoint( point, false );
        }
        else
        {
            if( IsMouseOverValidKey( point, true ) )
            {
                // Mouse is over selected key....leave selection as-is for key move mode
            }
            else
            {
                // Replace selection
                DeselectVisibleKeys();
                SetKeyIsSelectedByPoint( point, true );
            }
        }

        // Set Cursor/NavMode depending on the new selection
        if( IsMouseOverValidKey( point, true ) )
        {
            if( nFlags & MK_SHIFT )
            {
                // Key Copy-and-Move mode
                m_NavigateType              = NAVIGATE_KEY_MOVE;                // Set navigation type
                SetCursor                   ( ::LoadCursor(NULL, IDC_SIZEWE) );

                // Set up KeySets for Editing
                KeyEditSetup_CopyMove();
            }
            else
            {
                // Key Move mode
                m_NavigateType              = NAVIGATE_KEY_MOVE;                // Set navigation type
                SetCursor                   ( ::LoadCursor(NULL, IDC_SIZEWE) );

                // Set up KeySets for Editing
                KeyEditSetup_Move();
            }
        }
        else
        {
            // Key Selection mode
            m_NavigateType              = NAVIGATE_KEY_SELECT;                  // Set navigation type
            SetCursor                   ( ::LoadCursor(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_SELECT)) );
        }
    }
    else
    {
        // Key Selection mode
        m_NavigateType          = NAVIGATE_KEY_SELECT;      // Set navigation type
    }

    RedrawWindow();
}

void CTimeSlider::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if( GetCapture() == this )
    {
        ::ReleaseCapture();

        if( m_NavigateType == NAVIGATE_KEY_SELECT )
        {
            m_DrawSelectMarquee = false;
            m_NavigateDragPoint = point;

            // Perform click-drag selection methods if we've dragged more than one pixel
            if( abs(m_NavigateDragPoint.x - m_NavigatePoint.x) > 1 )
            {
                // Check to see how we're selecting/de-selecting
                if( nFlags & MK_CONTROL )
                {
                    // Add to selection
                    SetKeyIsSelectedByRange( m_NavigatePoint.x, m_NavigateDragPoint.x, true );
                }
                else if( GetKeyState(VK_MENU) & 0x8000 ) // (VK_MENU == Alt Key) (0x8000 == Check high bit)
                {
                    // Remove from selection
                    SetKeyIsSelectedByRange( m_NavigatePoint.x, m_NavigateDragPoint.x, false );
                }
                else
                {
                    // Replace selection
                    DeselectVisibleKeys();
                    SetKeyIsSelectedByRange( m_NavigatePoint.x, m_NavigateDragPoint.x, true );
                }
            }
            else
            {
                if( !(IsMouseOverValidKey( point, false )) )
                {
                    if( ( nFlags & MK_CONTROL ) || ( GetKeyState(VK_MENU) & 0x8000 ) )
                    {
                        // If Ctrl & or Alt are pressed we just leave the selection alone
                    }
                    else
                    {
                        // Blank area was clicked...remove the selection
                        DeselectVisibleKeys();
                    }
                }
            }

            RedrawWindow();
        }
    }

    m_NavigateType  = NAVIGATE_NULL;            // Reset navigation type
}

void CTimeSlider::OnMButtonDown(UINT nFlags, CPoint point) 
{
    // Let the parent window know that we're about to start an edit....so they can stop playback, etc
    BeginEdit();

    // Make sure the Time Slider gets all further input
    SetFocus();

    // Capture the mouse
    SetCapture();

    // Store navigation info
    m_NavigatePoint             = point;                            // Save initial click point
    m_NavigateTimeStart         = m_FrameStart;                     // Save initial start frame
    m_NavigateTimeEnd           = m_FrameEnd;                       // Save initial end frame
    m_NavigateTimeCurrent       = m_FrameCurrent;                   // Save initial current frame

    // Set navigation type
    m_NavigateType              = NAVIGATE_TIME_PAN;
    SetCursor                   ( ::LoadCursor(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_PAN)) );
}

void CTimeSlider::OnMButtonUp(UINT nFlags, CPoint point) 
{
    ::ReleaseCapture();
    m_NavigateType  = NAVIGATE_NULL;            // Reset navigation type
}

void CTimeSlider::OnRButtonDown(UINT nFlags, CPoint point) 
{
    // Let the parent window know that we're about to start an edit....so they can stop playback, etc
    BeginEdit();

    // Make sure the Time Slider gets all further input
    SetFocus();

    if( GetCapture() == this )
    {
        // Do nothing...wait for RButtonUp to cancel other operations
    }
    else
    {
        SetCapture();                               // Capture the mouse
        SetTimeChange   ( point );                  // Re-calculate and draw frame indicator
        m_NavigateType  = NAVIGATE_TIME_CHANGE;     // Reset navigation type
    }
}

void CTimeSlider::OnRButtonUp(UINT nFlags, CPoint point) 
{
    if( GetCapture() == this )
    {
        if( m_NavigateType == NAVIGATE_KEY_SELECT )
        {
            m_DrawSelectMarquee = false;
            RedrawWindow();
        }
        else if( m_NavigateType == NAVIGATE_TIME_CHANGE )
        {
            SetTimeChange   ( point );                  // Re-calculate and draw frame indicator
        }
        else if( m_NavigateType == NAVIGATE_KEY_MOVE )
        {
            MoveSelectedKeys( m_NavigatePoint );        // Perform a move edit with zero movement to reset
        }
        else if( m_NavigateType == NAVIGATE_TIME_PAN )
        {
            // CANCEL:  Reset time back to how it was before panning began
            m_FrameCurrent  = m_NavigateTimeCurrent;
            SetTimePan      ( m_NavigatePoint );
        }

        ::ReleaseCapture();
        m_NavigateType  = NAVIGATE_NULL;            // Reset navigation type
    }
    else
    {

    }
}

void CTimeSlider::OnMouseMove(UINT nFlags, CPoint point) 
{
    if( GetCapture() == this )
    {
        if( m_NavigateType == NAVIGATE_KEY_SELECT )
        {
            m_DrawSelectMarquee = true;
            m_NavigateDragPoint = point;

            RedrawWindow();
        }
        else if( m_NavigateType == NAVIGATE_KEY_MOVE )
        {
            MoveSelectedKeys( point );
        }
        else if( m_NavigateType == NAVIGATE_TIME_CHANGE)
        {
            SetTimeChange( point );
        }
        else if( m_NavigateType == NAVIGATE_TIME_PAN)
        {
            SetTimePan( point );
        }
    }
    else
    {
        if( IsMouseOverValidKey( point, true) )
        {
            // Move Cursor
            SetCursor( ::LoadCursor(NULL, IDC_SIZEWE) );
        }
        else if( IsMouseOverValidKey( point, false) )
        {
            // Select Cursor
            SetCursor( ::LoadCursor(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_SELECT)) );
        }
    }
}

void CTimeSlider::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if( nChar == VK_DELETE )
    {
        // Don't try to delete if the user is in the middle of doing something with the mouse
        if( GetCapture() != this )
        {
            // Prepare the keysets for editing
            int     numEditKeySets      = KeyEditSetup_Delete();

            // If we have KeySets to edit then delete their selected keys
            if( numEditKeySets > 0 )
            {
                DeleteSelectedKeys();
                RedrawWindow();
            }
        }
    }
    
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// CTimeSlider custom functions
/////////////////////////////////////////////////////////////////////////////

void    CTimeSlider::BeginEdit( void )
{
    // Send an update message to parent window...so they can respond by stopping playback, etc
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_EDIT_BEGIN, controlID, m_FrameCurrent );
}

void    CTimeSlider::SetFrameStart( int frameStart )
{
    ASSERT( frameStart <= m_FrameEnd );

    m_FrameStart    = frameStart;
    RedrawWindow();
}

void    CTimeSlider::SetFrameEnd( int frameEnd )
{
    ASSERT( m_FrameStart <= frameEnd );

    m_FrameEnd      = frameEnd;
    RedrawWindow();
}

void    CTimeSlider::SetFrameCurrent( int frameCurrent )
{
    ASSERT( (frameCurrent >= m_FrameStart) && (frameCurrent <= m_FrameEnd) );

    m_FrameCurrent  = frameCurrent;
    RedrawWindow();
}


void    CTimeSlider::SetTime( int frameStart, int frameEnd, int frameCurrent )
{
    ASSERT( frameStart <= frameEnd );
    ASSERT( (frameCurrent >= frameStart) && (frameCurrent <= frameEnd) );

    m_FrameStart    = frameStart;
    m_FrameEnd      = frameEnd;
    m_FrameCurrent  = frameCurrent;

    RedrawWindow();
}

void    CTimeSlider::SetAnimateMode( bool AnimateModeOn )
{
    m_AnimateModeOn = AnimateModeOn;
        
    RedrawWindow();
}

void    CTimeSlider::SetTimeChange( CPoint point )
{
    CRect           rcClient;
    float           frameRes;
    float           normalizedClick;

    GetClientRect   ( &rcClient );
    frameRes        = float(m_WidthFrames) / float(m_FrameEnd - m_FrameStart);
    normalizedClick = float(point.x) / float(rcClient.right);
    int NewFrame    = m_FrameStart + int( (m_FrameEnd - m_FrameStart + 1) * normalizedClick );

    if( NewFrame == m_FrameCurrent )
        return;
    else
        m_FrameCurrent = NewFrame;

    // Clamp to visible range
    if( m_FrameCurrent < m_FrameStart )     { m_FrameCurrent = m_FrameStart; }
    if( m_FrameCurrent > m_FrameEnd   )     { m_FrameCurrent = m_FrameEnd;   }

    // Draw the new frame indicator
    Invalidate();
    UpdateWindow();

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_FRAMECURRENT_CHANGED, controlID, m_FrameCurrent );
}

void    CTimeSlider::SetTimePan( CPoint point )
{
    CRect           rcClient;
    int             deltaPoint;
    int             deltaFrame;
    float           frameRes;

    frameRes        = float(m_WidthFrames) / float(m_FrameEnd - m_FrameStart);

    deltaPoint      = point.x - m_NavigatePoint.x;
    deltaFrame      = int( float(deltaPoint) / frameRes );

    // Pan the time
    m_FrameStart    = m_NavigateTimeStart - deltaFrame;
    m_FrameEnd      = m_NavigateTimeEnd   - deltaFrame;

    // Clamp current frame to visible range
    if( m_FrameCurrent < m_FrameStart )     { m_FrameCurrent = m_FrameStart; }
    if( m_FrameCurrent > m_FrameEnd   )     { m_FrameCurrent = m_FrameEnd;   }

    // Draw the new frame indicator
    Invalidate();
    UpdateWindow();

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_FRAMESTART_CHANGED,    controlID, m_FrameStart   );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_FRAMEEND_CHANGED,      controlID, m_FrameEnd     );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_FRAMECURRENT_CHANGED,  controlID, m_FrameCurrent );
}

void    CTimeSlider::DeleteSelectedKeys( void )
{
    // Make sure we have valid keysets for editing
    ASSERT( m_pKeySets                );
    ASSERT( m_pKeySets_Edit_InActive  );
    ASSERT( m_piEditKeySets           );
    ASSERT( m_nEditKeySets > 0        );

    int     numFinalKeys;
    int     iFinalKeySet;

    int     timeInActive;
    bool    isSelectedInActive;

    int     totalKeysDeleted    = 0;

    int     KeyDataSize;
    void*   pData;

    // Compile the InActive Edit KeySets' keys back into the real KeySets
    for( int i = 0; i < m_nEditKeySets; i++ )
    {
        iFinalKeySet        = m_piEditKeySets[i];
        numFinalKeys        = m_pKeySets_Edit_InActive[i].m_nKeys;

        totalKeysDeleted    += m_pKeySets[iFinalKeySet].m_nKeys - numFinalKeys;

        //----------------------------------------------------------------------+
        // Re-allocate memory for the real KeySets...if needed                  |
        //----------------------------------------------------------------------+
        KeyDataSize         = m_pKeySets[iFinalKeySet].m_KeyDataSize;

        if( numFinalKeys != m_pKeySets[iFinalKeySet].m_nKeys )
        {
            if( m_pKeySets[iFinalKeySet].m_pKeys )
            {
               delete[] m_pKeySets[iFinalKeySet].m_pKeys;
               m_pKeySets[iFinalKeySet].m_pKeys = NULL;
            }

            // Allocate memory for keys
            m_pKeySets[iFinalKeySet].m_pKeys    = new KeyBarKey[ numFinalKeys ];
            m_pKeySets[iFinalKeySet].m_nKeys    = numFinalKeys;

            // Allocate memory for key data
            for( int j = 0; j < numFinalKeys; j++ )
            {
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_pData     = malloc( KeyDataSize );
            }
        }

        //----------------------------------------------------------------------+
        // Assign the result of the edit to the real KeySets                    |
        //----------------------------------------------------------------------+
        for( int j = 0; j < numFinalKeys; j++ )
        {
            timeInActive        = m_pKeySets_Edit_InActive[i].m_pKeys[j].m_Time;
            isSelectedInActive  = m_pKeySets_Edit_InActive[i].m_pKeys[j].m_IsSelected;
            pData               = m_pKeySets_Edit_InActive[i].m_pKeys[j].m_pData;

            // Copy key info
            m_pKeySets[iFinalKeySet].m_pKeys[j].m_Time          = timeInActive;
            m_pKeySets[iFinalKeySet].m_pKeys[j].m_IsSelected    = isSelectedInActive;

            memcpy( m_pKeySets[iFinalKeySet].m_pKeys[j].m_pData, pData, KeyDataSize );
        }

        // Send a message to let the app know this KeySet has been changed
        DWORD       UserData            = m_pKeySets[iFinalKeySet].m_UserData;  // So app can link to it's own stuff
        KeySet*     pKeySetChanged      = &m_pKeySets[iFinalKeySet];            // So app can get at our changes

        GetParent()->SendMessage( WM_USER_MSG_TIMESLIDER_KEYS_CHANGED, (WPARAM)UserData, (LPARAM)pKeySetChanged );
    }

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_KEYS_DELETED, controlID, totalKeysDeleted );
}

void    CTimeSlider::MoveSelectedKeys( CPoint point )
{
    // Make sure we have valid keysets for editing
    ASSERT( m_pKeySets                );
    ASSERT( m_pKeySets_Edit_InActive  );
    ASSERT( m_pKeySets_Edit_Active    );
    ASSERT( m_pKeySets_Edit_ActiveRef );
    ASSERT( m_piEditKeySets           );
    ASSERT( m_nEditKeySets > 0        );

    int             deltaPoint;
    int             deltaFrame;
    float           frameRes;

    int             timeActive;
    int             timeInActive;

    int             numActiveKeys;
    int             numInActiveKeys;
    int             numFinalKeys;
    int             iFinalKeySet;

    frameRes        = float(m_WidthFrames) / float(m_FrameEnd - m_FrameStart);

    deltaPoint      = point.x - m_NavigatePoint.x;
    deltaFrame      = int( float(deltaPoint) / frameRes );

    // Move the keys in the Active Edit KeySet
    for( int i = 0; i < m_nEditKeySets; i++ )
    {
        for( int j = 0; j < m_pKeySets_Edit_Active[i].m_nKeys; j++ )
        {
            m_pKeySets_Edit_Active[i].m_pKeys[j].m_Time = m_pKeySets_Edit_ActiveRef[i].m_pKeys[j].m_Time + deltaFrame;
        }
    }

    // Compile the Edit KeySets into the real KeySets
    for( i = 0; i < m_nEditKeySets; i++ )
    {
        iFinalKeySet    = m_piEditKeySets[i];

        //----------------------------------------------------------------------+
        // Figure out how many keys this set will end up with after the edit    |
        //----------------------------------------------------------------------+

        // Start out by adding up the total keys in each KeySet
        numActiveKeys       = m_pKeySets_Edit_Active[i].m_nKeys;
        numInActiveKeys     = m_pKeySets_Edit_InActive[i].m_nKeys;

        numFinalKeys        = numActiveKeys + numInActiveKeys;

        // Now take into account time conflicts....Where an Active key over-writes an Inactive key
        for( int j = 0; j < numActiveKeys; j++ )
        {
            timeActive      = m_pKeySets_Edit_Active[i].m_pKeys[j].m_Time;

            for( int k = 0; k < numInActiveKeys; k++ )
            {
                timeInActive    = m_pKeySets_Edit_InActive[i].m_pKeys[k].m_Time;

                if( timeActive == timeInActive )
                {
                    numFinalKeys    -= 1; // An over-write will occur...that means one less key in total
                }
            }
        }

        //----------------------------------------------------------------------+
        // Re-allocate memory for the real KeySets...if needed                  |
        //----------------------------------------------------------------------+

        // Get Data Size...should be same for all edit keysets since they're editing the same data
        int     KeyDataSize     = m_pKeySets[iFinalKeySet].m_KeyDataSize;

        if( numFinalKeys != m_pKeySets[iFinalKeySet].m_nKeys )
        {
            if( m_pKeySets[iFinalKeySet].m_pKeys )
            {
               delete[] m_pKeySets[iFinalKeySet].m_pKeys;
               m_pKeySets[iFinalKeySet].m_pKeys = NULL;
            }

            m_pKeySets[iFinalKeySet].m_pKeys    = new KeyBarKey[ numFinalKeys ];
            m_pKeySets[iFinalKeySet].m_nKeys    = numFinalKeys;

            // Allocate memory for key data
            for( int j = 0; j < numFinalKeys; j++ )
            {
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_pData     = malloc( KeyDataSize );
            }
        }

        //----------------------------------------------------------------------+
        // Assign the result of the edit to the real KeySets                    |
        //----------------------------------------------------------------------+
        int     iActive     = 0;
        int     iInActive   = 0;

        bool    isSelectedActive;
        bool    isSelectedInActive;

        void*   pDataActive;
        void*   pDataInActive;

        for( j = 0; j < numFinalKeys; j++ )
        {
            // Get the info for the next Active and InActive key
            timeActive          = m_pKeySets_Edit_Active[i].m_pKeys[ iActive ].m_Time;
            isSelectedActive    = m_pKeySets_Edit_Active[i].m_pKeys[ iActive ].m_IsSelected;
            pDataActive         = m_pKeySets_Edit_Active[i].m_pKeys[ iActive ].m_pData;

            timeInActive        = m_pKeySets_Edit_InActive[i].m_pKeys[ iInActive ].m_Time;
            isSelectedInActive  = m_pKeySets_Edit_InActive[i].m_pKeys[ iInActive ].m_IsSelected;
            pDataInActive       = m_pKeySets_Edit_InActive[i].m_pKeys[ iInActive ].m_pData;
            
            if( iInActive == numInActiveKeys )
            {
                // Add the Active key..we're out of InActive keys
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_Time          = timeActive;
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_IsSelected    = isSelectedActive;

                // Copy key data
                memcpy( m_pKeySets[iFinalKeySet].m_pKeys[j].m_pData, pDataActive, KeyDataSize );

                iActive++;
            }
            else if( iActive == numActiveKeys )
            {
                // Add the InActive key..we're out of Active keys
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_Time          = timeInActive;
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_IsSelected    = isSelectedInActive;

                // Copy key data
                memcpy( m_pKeySets[iFinalKeySet].m_pKeys[j].m_pData, pDataInActive, KeyDataSize );

                iInActive++;
            }
            else if( timeInActive < timeActive )
            {
                // Add the InActive key
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_Time          = timeInActive;
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_IsSelected    = isSelectedInActive;

                // Copy key data
                memcpy( m_pKeySets[iFinalKeySet].m_pKeys[j].m_pData, pDataInActive, KeyDataSize );

                iInActive++;
            }
            else if( timeActive < timeInActive )
            {
                // Add the Active key
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_Time          = timeActive;
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_IsSelected    = isSelectedActive;

                // Copy key data
                memcpy( m_pKeySets[iFinalKeySet].m_pKeys[j].m_pData, pDataActive, KeyDataSize );

                iActive++;
            }
            else    // timeActive == timeInActive
            {
                // Add the Active key...it takes precedence over the InActive one
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_Time          = timeActive;
                m_pKeySets[iFinalKeySet].m_pKeys[j].m_IsSelected    = isSelectedActive;

                // Copy key data
                memcpy( m_pKeySets[iFinalKeySet].m_pKeys[j].m_pData, pDataActive, KeyDataSize );

                // Update both counters since they shared the same time
                iActive++;
                iInActive++;
            }
        }

        // Send a message to let the app know this KeySet has been changed
        DWORD       UserData            =  m_pKeySets[iFinalKeySet].m_UserData; // So app can link to it's own stuff
        KeySet*     pKeySetChanged      = &m_pKeySets[iFinalKeySet];            // So app can get at our changes

        GetParent()->SendMessage( WM_USER_MSG_TIMESLIDER_KEYS_CHANGED, (WPARAM)UserData, (LPARAM)pKeySetChanged );
    }

    // Send an update message to parent window
    LONG    controlID           = ::GetWindowLong( m_hWnd, GWL_ID );
    GetParent()->SendMessage    ( WM_USER_MSG_TIMESLIDER_KEYS_MOVED, controlID, deltaFrame );

    // Draw the new frame indicator
    Invalidate();
    UpdateWindow();
}

int     CTimeSlider::GetEditableKeySets( int*& pResult_KeySetIndices )
{
    // We should never have a NULL KeySet...a KeySet with zero keys is fine though
    // // ASSERT( m_pKeySets );

    // Get the number of Editable KeySets...which are VISIBLE KEYSETS that HAVE KEYS that are SELECTED
    int     nEditKeySets    = 0;

    for( int i = 0; i < m_nKeySets; i++ )
    {
        // Check for visible keysets
        if( CheckFilter( m_pKeySets[i] ) )
        {
            for( int j = 0; j < m_pKeySets[i].m_nKeys; j++ )
            {
                if( m_pKeySets[i].m_pKeys[j].m_IsSelected )
                {
                    nEditKeySets += 1;  // SUCCESS!!  Tally the counter up
                    break;              // Get out...we don't want to count this KeySet more than once!
                }
            }
        }
    }

    // First, clean up the previous index array
    if( pResult_KeySetIndices )
    {
        delete[] pResult_KeySetIndices;
        pResult_KeySetIndices = NULL;
    }

    // Now gather an array of indices for the Editable KeySets...if we found any
    if( nEditKeySets > 0 )
    {
        pResult_KeySetIndices   = new int[nEditKeySets];
        
        int     nGathered       = 0;

        for( int i = 0; i < m_nKeySets; i++ )
        {
            // Check for visible keysets
            if( CheckFilter( m_pKeySets[i] ) )
            {
                for( int j = 0; j < m_pKeySets[i].m_nKeys; j++ )
                {
                    if( m_pKeySets[i].m_pKeys[j].m_IsSelected )
                    {
                        // SUCCESS!!  Assign the index
                        pResult_KeySetIndices[nGathered] = i;

                        // Tally up the counter and check how many we've gathered
                        nGathered += 1;

                        if( nGathered < nEditKeySets )
                        {
                            break;                  // There's more left to gather...move onto the next one
                        }
                        else
                        {
                            return nEditKeySets;    // We got 'em all...bail out!
                        }
                    }
                }
            }
        }
    }

    return nEditKeySets;
}

int     CTimeSlider::KeyEditSetup_Delete( void )
{
    m_nEditKeySets              = GetEditableKeySets( m_piEditKeySets );

    // Make sure we have keys to delete before we do anything
    if( (m_nEditKeySets > 0) && (m_piEditKeySets != NULL) )
    {
        // Get Editing KeySet info
        CopyKeySets( m_pKeySets, m_pKeySets_Edit_InActive,  m_nEditKeySets, m_piEditKeySets, false, true,  false, true  );
        CopyKeySets( m_pKeySets, m_pKeySets_Edit_Active,    m_nEditKeySets, m_piEditKeySets, false, false, false, false );
        CopyKeySets( m_pKeySets, m_pKeySets_Edit_ActiveRef, m_nEditKeySets, m_piEditKeySets, false, false, false, false );
    }

    return m_nEditKeySets;
}

int     CTimeSlider::KeyEditSetup_Move( void )
{
    m_nEditKeySets              = GetEditableKeySets( m_piEditKeySets );

    ASSERT( m_nEditKeySets  >  0    );  // We shouldn't be here if we have nothing to edit
    ASSERT( m_piEditKeySets != NULL );  // Make sure GetEditableKeySets() gave us something to work with

    // Get Editing KeySet info
    CopyKeySets( m_pKeySets, m_pKeySets_Edit_InActive,  m_nEditKeySets, m_piEditKeySets, false, true,  false, true  );
    CopyKeySets( m_pKeySets, m_pKeySets_Edit_Active,    m_nEditKeySets, m_piEditKeySets, true,  false, true,  false );
    CopyKeySets( m_pKeySets, m_pKeySets_Edit_ActiveRef, m_nEditKeySets, m_piEditKeySets, true,  false, true,  false );

    return m_nEditKeySets;
}

int     CTimeSlider::KeyEditSetup_CopyMove( void )
{
    m_nEditKeySets              = GetEditableKeySets( m_piEditKeySets );

    ASSERT( m_nEditKeySets  >  0    );  // We shouldn't be here if we have nothing to edit
    ASSERT( m_piEditKeySets != NULL );  // Make sure GetEditableKeySets() gave us something to work with

    // Get Editing KeySet info
    CopyKeySets( m_pKeySets, m_pKeySets_Edit_InActive,  m_nEditKeySets, m_piEditKeySets, true,  true,  true,  true  );
    CopyKeySets( m_pKeySets, m_pKeySets_Edit_Active,    m_nEditKeySets, m_piEditKeySets, true,  false, true,  false );
    CopyKeySets( m_pKeySets, m_pKeySets_Edit_ActiveRef, m_nEditKeySets, m_piEditKeySets, true,  false, true,  false );

    return m_nEditKeySets;
}

bool    CTimeSlider::CopyKeySets( KeySet*   pSource,
                                  KeySet*&  pTarget,
                                  int       nKeySets,
                                  int*      pSourceIndices,
                                  bool      CopyVisible_Selected,
                                  bool      CopyVisible_UnSelected,
                                  bool      CopyInvisible_Selected,
                                  bool      CopyInvisible_UnSelected )
{
    ASSERT( pSource );  // We shouldn't be here unless we have something to copy

    int j;

    int         copyIndex;
    int         nKeys;
    int         nCopyKeys;
    int         nKeysCopied;
    int         keyTime;
    bool        keyIsVisible;
    bool        keyIsSelected;

    int         KeyDataSize;
    void*       pKeyData;

    // Cleanup target
    if( pTarget )
    {
        delete[] pTarget;
        pTarget = NULL;
    }

    // Copy the KeySets while taking into account the flags for Keys
    pTarget     = new KeySet[ nKeySets ];

    for( int i = 0; i < nKeySets; i++ )
    {
        copyIndex   = pSourceIndices[i];
        nKeys       = pSource[copyIndex].m_nKeys;

        ASSERT( nKeys > 0 );  // We shouldn't be editing a keyset that has no keys!

        // Get the number of keys to copy
        nCopyKeys   = 0;

        for( j = 0; j < nKeys; j++ )
        {
            // Get Key Info
            keyTime             = pSource[copyIndex].m_pKeys[j].m_Time;
            keyIsSelected       = pSource[copyIndex].m_pKeys[j].m_IsSelected;
            keyIsVisible        = (keyTime >= m_FrameStart) && (keyTime <= m_FrameEnd);

            if(     ( CopyVisible_Selected      &&  keyIsVisible    &&  keyIsSelected  )
                ||  ( CopyVisible_UnSelected    &&  keyIsVisible    &&  !keyIsSelected )
                ||  ( CopyInvisible_Selected    &&  !keyIsVisible   &&  keyIsSelected  )
                ||  ( CopyInvisible_UnSelected  &&  !keyIsVisible   &&  !keyIsSelected )    )
            {
                nCopyKeys += 1;
            }
        }

        // Copy the KeySet info
        pTarget[i].m_Category       = pSource[copyIndex].m_Category;
        pTarget[i].m_KeyDataSize    = pSource[copyIndex].m_KeyDataSize;
        pTarget[i].m_UserData       = pSource[copyIndex].m_UserData;
        pTarget[i].m_nKeys          = nCopyKeys;

        // Allocate memory for key data
        pTarget[i].m_pKeys      = new KeyBarKey[ nCopyKeys ];
        KeyDataSize             = pSource[copyIndex].m_KeyDataSize;

        for( j = 0; j < nCopyKeys; j++ )
        {
            pTarget[i].m_pKeys[j].m_pData     = malloc( KeyDataSize );
        }

        // Copy the appropriate keys
        nKeysCopied             = 0;

        for( j = 0; j < nKeys; j++ )
        {
            // Get Key Info
            keyTime             = pSource[copyIndex].m_pKeys[j].m_Time;
            keyIsSelected       = pSource[copyIndex].m_pKeys[j].m_IsSelected;
            pKeyData            = pSource[copyIndex].m_pKeys[j].m_pData;

            keyIsVisible        = (keyTime >= m_FrameStart) && (keyTime <= m_FrameEnd);

            if(     ( CopyVisible_Selected      &&  keyIsVisible    &&  keyIsSelected  )
                ||  ( CopyVisible_UnSelected    &&  keyIsVisible    &&  !keyIsSelected )
                ||  ( CopyInvisible_Selected    &&  !keyIsVisible   &&  keyIsSelected  )
                ||  ( CopyInvisible_UnSelected  &&  !keyIsVisible   &&  !keyIsSelected )    )
            {
                pTarget[i].m_pKeys[nKeysCopied].m_Time          = keyTime;
                pTarget[i].m_pKeys[nKeysCopied].m_IsSelected    = keyIsSelected;

                // Copy key data
                memcpy( pTarget[i].m_pKeys[nKeysCopied].m_pData, pKeyData, KeyDataSize );

                nKeysCopied += 1;
 
                if( nKeysCopied >= nCopyKeys )
                    break; // We've copied all the keys we need for this set
            }
        }
    }

    return true;
}

