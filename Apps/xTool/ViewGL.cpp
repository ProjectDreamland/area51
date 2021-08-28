// ViewGL.cpp : implementation file
//

#include "stdafx.h"
#include "ViewGL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

#pragma comment( lib, "opengl32.lib" )
#pragma comment( lib, "glu32.lib" )


#define MAX_POINTS 1000000

struct color
{
    GLbyte  r,g,b,a;
};

vector3     s_Points[MAX_POINTS];
color       s_Colors[MAX_POINTS];

/////////////////////////////////////////////////////////////////////////////
// CViewGL

IMPLEMENT_DYNCREATE(CViewGL, CViewBase)

CViewGL::CViewGL()
{
    m_MouseCaptured = FALSE;
    m_TrackingLeave = FALSE;
    m_UpdateTimerID = 0;
    m_NewDataFlag   = FALSE;
}

CViewGL::~CViewGL()
{
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CViewGL, CViewBase)
	//{{AFX_MSG_MAP(CViewGL)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_TIMER()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
//    ON_WM_RBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE( WM_MOUSELEAVE, OnMouseLeave )
//    ON_WM_MOUSEWHEEL()
//    ON_WM_CONTEXTMENU()
//    ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewGL precreate

BOOL CViewGL::PreCreateWindow( CREATESTRUCT& cs )
{
    cs.style |= CS_OWNDC;

    return CViewBase::PreCreateWindow( cs );
}

/////////////////////////////////////////////////////////////////////////////
// CViewGL drawing

void CViewGL::OnDraw(CDC* pDC)
{
    static f32 rAngle = 0;
    rAngle += 2;

    wglMakeCurrent  (m_hDC, m_hRC); 
    glClear         ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
    glMatrixMode    ( GL_MODELVIEW ); 
    glLoadIdentity  ( );
    glTranslatef    ( 0.0, 0.0, -500.0 ); 
    glRotatef       ( rAngle, 0.0f, 1.0f, 0.0f );
    glRotatef       ( rAngle/3.134323245f, 1.0f, 0.0f, 0.0f );

    glColor4f   ( 1.0, 0.0, 0.0, 1.0 ); 
/*
    glBegin     ( GL_QUADS       ); 
    glVertex2f  ( -20.0f, -20.0f ); 
    glVertex2f  ( -20.0f,  20.0f ); 
    glVertex2f  (  20.0f,  20.0f ); 
    glVertex2f  (  20.0f, -20.0f ); 
    glEnd       ( ); 
*/

    glEnable( GL_BLEND );

#if 1
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    glVertexPointer ( 3, GL_FLOAT, sizeof(vector3), s_Points );
    glColorPointer  ( 4,  GL_BYTE, 4, s_Colors );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY );

//    glPointSize     ( 10.0f );
    glDepthMask     ( 0 );
    glDrawArrays    ( GL_POINTS, 0, MAX_POINTS );
    glDepthMask     ( 1 );

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
#endif

#if 0
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glEnable( GL_LIGHT1 );

    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    GLUquadric* pObj = gluNewQuadric();

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glColor4f( 0.5f, 1.0f, 0.5f, 0.5f );
    gluSphere( pObj, 100.0f, 20, 20 );

    glTranslatef( 100.0f, 0.0f, 0.0f );
    glColor4f( 1.0f, 0.5f, 0.5f, 0.5f );
    gluSphere( pObj, 100.0f, 20, 20 );

    gluDeleteQuadric( pObj );
#endif

    SwapBuffers     ( m_hDC ); 
    wglMakeCurrent  ( NULL, NULL ); 
    ValidateRect    ( NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CViewGL diagnostics

#ifdef _DEBUG
void CViewGL::AssertValid() const
{
	CViewBase::AssertValid();
}

void CViewGL::Dump(CDumpContext& dc) const
{
	CViewBase::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewGL message handlers

float g_Tweak = 0.1f;

int CViewGL::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

    ASSERT( m_pDocument );

    PIXELFORMATDESCRIPTOR   pfd; 
    int     pixelFormat; 

    m_hDC = ::GetDC(m_hWnd); 

    pfd.nSize =             sizeof(PIXELFORMATDESCRIPTOR); 
    pfd.nVersion =          1; 
    pfd.dwFlags =           PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; 
    pfd.iPixelType =        PFD_TYPE_RGBA; 
    pfd.cColorBits =        32; 
    pfd.cRedBits =          0; 
    pfd.cRedShift =         0; 
    pfd.cGreenBits =        0; 
    pfd.cGreenShift =       0; 
    pfd.cBlueBits =         0; 
    pfd.cBlueShift =        0; 
    pfd.cAlphaBits =        0; 
    pfd.cAlphaShift =       0; 
    pfd.cAccumBits =        0; 
    pfd.cAccumRedBits =     0; 
    pfd.cAccumGreenBits =   0; 
    pfd.cAccumBlueBits =    0; 
    pfd.cAccumAlphaBits =   0; 
    pfd.cDepthBits =        24; 
    pfd.cStencilBits =      0; 
    pfd.cAuxBuffers =       0; 
    pfd.iLayerType =        PFD_MAIN_PLANE; 
    pfd.bReserved =         0; 
    pfd.dwLayerMask =       0; 
    pfd.dwVisibleMask =     0; 
    pfd.dwDamageMask =      0; 

    pixelFormat = ChoosePixelFormat(m_hDC, &pfd); 

    DescribePixelFormat(m_hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 

    if( SetPixelFormat(m_hDC, pixelFormat, &pfd) != FALSE )
    {
        m_hRC = wglCreateContext( m_hDC ); 

        wglMakeCurrent( m_hDC, m_hRC ); 

        glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );     // This Will Clear The Background Color To Black
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );     // This Will Clear The Background Color To Black
        glClearDepth( 1.0f );                       // Enables Clearing Of The Depth Buffer
        glDepthFunc ( GL_LEQUAL );                  // The Type Of Depth Test To Do
        glEnable    ( GL_DEPTH_TEST );              // Enables Depth Testing
        glShadeModel( GL_SMOOTH );                  // Enables Smooth Color Shading

        glAlphaFunc( GL_ALWAYS, 1.0f );

        wglMakeCurrent( NULL, NULL ); 
    }

    // Make some random points
    random r;
    for( s32 i=0 ; i<MAX_POINTS ; i++ )
    {
        vector3 v( r.frand(-100.0f, 100.0f), r.frand(-100.0f, 100.0f), r.frand(-100.0f, 100.0f) );
//        vector3 v = r.v3( -100.0f, 100.0f, -100.0f, 100.0f, -100.0f, 100.0f );
        s_Points[i] = v;
        s_Colors[i].r = r.irand( 0, 127 );
        s_Colors[i].g = r.irand( 0, 127 );
        s_Colors[i].b = r.irand( 0, 127 );

        vector3 t = v;
        t.Normalize();
        
        f32 Dot1 = t.Dot( vector3(0,1,0) );
        Dot1 = x_sin( Dot1 * 10.0f );

        f32 Dot2 = t.Dot( vector3(1,0,0) );
        Dot2 = x_sin( Dot2 * 9.0f );

        f32 Dot3 = t.Dot( vector3(0,0,1) );
        Dot3 = x_sin( Dot3 * 7.0f );

        s_Colors[i].a = (GLbyte)(( x_sin(v.Length() * g_Tweak) * 63 + 63 ) * Dot1 * Dot2 * Dot3);
    }

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CViewGL::OnDestroy()
{
    wglMakeCurrent  ( NULL, NULL ); 
    wglDeleteContext( m_hRC ); 
    ::ReleaseDC     ( m_hWnd, m_hDC );
}

/////////////////////////////////////////////////////////////////////////////

BOOL CViewGL::OnEraseBkgnd(CDC* pDC) 
{
    return 1;
}

/////////////////////////////////////////////////////////////////////////////

void CViewGL::OnSize(UINT nType, int cx, int cy) 
{
	CViewBase::OnSize(nType, cx, cy);

    // Prevent A Divide By Zero If The Window Is Too Small
    if( cy == 0 )
        cy = 1 ;

    // Reset The Current Viewport And Perspective Transformation
    wglMakeCurrent  ( m_hDC, m_hRC ); 
    glViewport      ( 0, 0, cx, cy );
    glMatrixMode    ( GL_PROJECTION );
    glLoadIdentity  ( );
    gluPerspective  ( 45.0f, (GLfloat)cx/(GLfloat)cy, 0.1f, 1000.0f );
    glMatrixMode    ( GL_MODELVIEW );
    glLoadIdentity  ( );
    InvalidateRect  ( NULL );
    wglMakeCurrent  ( NULL, NULL ); 
}

/////////////////////////////////////////////////////////////////////////////

void CViewGL::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
/*
    CxToolDoc* pDoc = GetDocument();
    ASSERT( pDoc );

    if( (lHint == 0) ||
        (lHint == CxToolDoc::HINT_NEW_LOG_DATA) ||
        (lHint == CxToolDoc::HINT_NEW_MEMORY_DATA ) || // TODO: Check for filtered memory data here
        (lHint == CxToolDoc::HINT_LOG_FILTER) ||
        (lHint == CxToolDoc::HINT_LOG_REDRAW) )
    {
        // If we don't have a timer then start one, this mechanism is used to limit
        // the number of updates reflected visually to 1 every 50ms
        if( m_UpdateTimerID == 0 )
        {
            m_UpdateTimerID = SetTimer( 1, 50, NULL );
            ASSERT( m_UpdateTimerID );
        }

        // If this is not a filter channels change then set the new data flag
        if( (lHint == CxToolDoc::HINT_NEW_LOG_DATA) || (lHint == 0) )
        {
            m_NewDataFlag = TRUE;
        }
    }

    // If the filter was changed then set the number of items and rebuild the selection set
    if( lHint == CxToolDoc::HINT_LOG_FILTER )
    {
        // Get document
        CxToolDoc*  pDoc = GetDocument();
        log_array&  Log  = pDoc->GetFilteredLog();

        // Resize the list with no scrolling or unnecessary redraw
        m_wndList.SetItemCountEx( Log.GetSize(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL );

        // Rebuild the selection set
        m_wndList.BuildSelectionSet();
    }

    if( lHint == CxToolDoc::HINT_LOG_FIXED_FONT_CHANGED )
    {
        if( pDoc->GetLogViewFixedFont() )
        {
            m_wndList.SetFont( &m_Font );
            CHeaderCtrl* pHeader = m_wndList.GetHeaderCtrl();
            pHeader->SetFont( &m_DefaultFont );
        }
        else
        {
            m_wndList.SetFont( &m_DefaultFont );
        }
    }
*/
}

/////////////////////////////////////////////////////////////////////////////

void CViewGL::OnTimer(UINT nIDEvent)
{
    if( nIDEvent == m_UpdateTimerID )
    {
        // Kill the timer
        VERIFY( KillTimer( m_UpdateTimerID ) );
        m_UpdateTimerID = 0;

        BOOL    Shift           = ::GetKeyState( VK_SHIFT   ) & 0x8000;
        BOOL    Control         = ::GetKeyState( VK_CONTROL ) & 0x8000;
        if( !Shift )
        {
            m_UpdateTimerID = SetTimer( 1, 33, NULL );
            ASSERT( m_UpdateTimerID );
        }

        RedrawWindow();
    }

/*
    // Update timer message?
    if( nIDEvent == m_UpdateTimerID )
    {
        // Kill the timer
        VERIFY( KillTimer( m_UpdateTimerID ) );
        m_UpdateTimerID = 0;

        // Get document
        CxToolDoc*  pDoc = GetDocument();
        log_array&  Log  = pDoc->GetFilteredLog();

        // Check if the log needs updating
        if( m_wndList.GetItemCount() != Log.GetSize() )
        {
            // Determine if the last item has focus
            BOOL LastHasFocus = m_wndList.IsLastItemInFocus();

            // Disable redraw
            m_wndList.SetRedraw( FALSE );

            // Resize the list with no scrolling or unnecessary redraw
            m_wndList.SetItemCountEx( Log.GetSize(), LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL );

            // Keep the last item visible if it was previously visible
            if( LastHasFocus && (m_wndList.GetItemCount() > 0) )
            {
                // Move focus to the last item
                int iItemFocus = m_wndList.GetItemCount()-1;
                m_wndList.SetFocusItem( iItemFocus );

                // Ensure the last item is visible
                m_wndList.EnsureVisible( iItemFocus, FALSE );
            }

            // Enable & force redraw
            m_wndList.SetRedraw( TRUE );
            m_wndList.Invalidate();

            m_NewDataFlag = FALSE;
        }
        else
        {
            // No new data, just force a redraw
            m_wndList.RedrawWindow();

            // TODO: Clear m_NewData here?
        }
    }
*/
//	CViewBase::OnTimer(nIDEvent);
}

/////////////////////////////////////////////////////////////////////////////
// OnLButtonDown

void CViewGL::OnLButtonDown(UINT nFlags, CPoint point) 
{
    // MK_CONTROL   Set if the CTRL key is down. 
    // MK_LBUTTON   Set if the left mouse button is down. 
    // MK_MBUTTON   Set if the middle mouse button is down. 
    // MK_RBUTTON   Set if the right mouse button is down. 
    // MK_SHIFT
    BOOL    Control         = ::GetKeyState( VK_CONTROL ) & 0x8000;

    if( Control )
    {
        m_UpdateTimerID = SetTimer( 1, 20, NULL );
        ASSERT( m_UpdateTimerID );
    }

    // Claim Keyboard input
    SetFocus();

    // Capture the mouse
    SetCapture();
    m_MouseCaptured = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// OnLButtonUp

void CViewGL::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // MK_CONTROL   Set if the CTRL key is down. 
    // MK_LBUTTON   Set if the left mouse button is down. 
    // MK_MBUTTON   Set if the middle mouse button is down. 
    // MK_RBUTTON   Set if the right mouse button is down. 
    // MK_SHIFT

    if( m_MouseCaptured )
    {
        ReleaseCapture();
        m_MouseCaptured = FALSE;
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnLButtonDblClk

void CViewGL::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
    // MK_CONTROL   Set if the CTRL key is down. 
    // MK_LBUTTON   Set if the left mouse button is down. 
    // MK_MBUTTON   Set if the middle mouse button is down. 
    // MK_RBUTTON   Set if the right mouse button is down. 
    // MK_SHIFT

    CViewBase::OnLButtonDblClk(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////
// OnMouseMove

void CViewGL::OnMouseMove(UINT nFlags, CPoint point) 
{
    // MK_CONTROL   Set if the CTRL key is down. 
    // MK_LBUTTON   Set if the left mouse button is down. 
    // MK_MBUTTON   Set if the middle mouse button is down. 
    // MK_RBUTTON   Set if the right mouse button is down. 
    // MK_SHIFT

    // Register for tracking mouse leave
    if( !m_TrackingLeave )
    {
        TRACKMOUSEEVENT EventTrack;
        EventTrack.cbSize       = sizeof(EventTrack);
        EventTrack.dwFlags      = TME_LEAVE;
        EventTrack.dwHoverTime  = HOVER_DEFAULT;
        EventTrack.hwndTrack    = GetSafeHwnd();
        _TrackMouseEvent( &EventTrack );
        m_TrackingLeave = TRUE;
    }
    if( nFlags & MK_LBUTTON )
    {
        Invalidate( TRUE );
    }
}

/////////////////////////////////////////////////////////////////////////////
// OnMouseLeave

LRESULT CViewGL::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    // Call the mouse move to do final processing
    CPoint pt;
    GetCursorPos( &pt );
    ScreenToClient( &pt );
    OnMouseMove( 0, pt );

    // No longer tracking
    m_TrackingLeave = FALSE;

    // Done
    return 0;
}

