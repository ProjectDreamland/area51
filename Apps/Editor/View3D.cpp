// View3D1.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "editor.h"
#include "View3D.h"
#include "RealTimeMessage.h"
#include "..\Apps\WorldEditor\EditorDoc.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MFC STUFF
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CView3D, CBaseView)

BEGIN_MESSAGE_MAP(CView3D, CBaseView)
	//{{AFX_MSG_MAP(CView3D)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CView3D diagnostics

#ifdef _DEBUG
void CView3D::AssertValid() const
{
	CBaseView::AssertValid();
}

void CView3D::Dump(CDumpContext& dc) const
{
	CBaseView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//===========================================================================

CView3D::CView3D() 
{
    // Initialize the view to something resanable
    m_View.SetXFOV( R_60 );
    m_View.SetPosition( vector3(100,100,200) );
    m_View.LookAtPoint( vector3(  0,  0,  0) );
    m_View.SetZLimits ( 1, 50000 );

    // Initialize the camera type
    m_CamType           = CAMERA_FREEFLY;
    m_CamRotateSpeed    = 1;
    m_CamTranslateSpeed = 1;

    m_bDirtyView        = FALSE;
}
//===========================================================================

CView3D::~CView3D()
{
}

//===========================================================================

void CView3D::SetViewDirty( void )
{
    //Update Camera view for user-settings
    g_SaveTrackUserSettings.CameraPos = m_View.GetPosition();
    m_View.GetPitchYaw(g_SaveTrackUserSettings.Pitch, g_SaveTrackUserSettings.Yaw); 

    //Check to see if Camera should be updated on load user settings
    if(g_LoadUpdateUserSettings.UpdateCameraFlag)
    {
        //Update View
        m_View.SetPosition(g_LoadUpdateUserSettings.CameraPos);
        m_View.SetRotation( radian3( g_LoadUpdateUserSettings.Pitch, g_LoadUpdateUserSettings.Yaw,0 ) );
        g_LoadUpdateUserSettings.UpdateCameraFlag = FALSE;
    }

    m_bDirtyView = TRUE;
}

//===========================================================================

void CView3D::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//===========================================================================

BOOL CView3D::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default	
	return TRUE; //CBaseView::OnEraseBkgnd(pDC);
}

//===========================================================================
void CView3D::OnPaint() 
{
    CPaintDC dc(this); // device context for painting

    // Clear the dirty window flag
    m_bDirtyView = FALSE;

    // No need to call the parent this is the top most
    d3deng_UpdateDisplayWindow( GetSafeHwnd() );
    eng_MaximizeViewport( m_View );
    eng_SetView         ( m_View );
}

//===========================================================================

void CView3D::CameraRotate( s32 MouseDeltaX, s32 MouseDeltaY )
{
    if( m_CamType == CAMERA_FREEFLY )
    {
        const f32   R = 0.005f;
        radian      Pitch;
        radian      Yaw;

        m_View.GetPitchYaw( Pitch, Yaw );       
        Pitch += MouseDeltaY * R * m_CamRotateSpeed;
        Yaw   -= MouseDeltaX * R * m_CamRotateSpeed;

        Pitch = fMax( Pitch, -(R_90-R_1) );
        Pitch = fMin( Pitch, R_90-R_1 );
        m_View.SetRotation( radian3(Pitch,Yaw,0) );
    }
    else if ( m_CamType == CAMERA_ORBIT )
    {
        radian      Pitch;
        radian      Yaw;
        vector3     Dir      = m_View.GetPosition() - m_CamFocus;
        f32         Distance = Dir.Length();

        Dir.GetPitchYaw( Pitch, Yaw );

        Pitch -= MouseDeltaY * 0.005f * m_CamRotateSpeed;
        Yaw   -= MouseDeltaX * 0.005f * m_CamRotateSpeed;

        if( Pitch >  R_89 ) Pitch =  R_89;
        if( Pitch < -R_89 ) Pitch = -R_89;

        m_View.OrbitPoint( m_CamFocus, Distance, Pitch, Yaw );
    }
}

//===========================================================================

void CView3D::CameraTranslate( s32 MouseDeltaX, s32 MouseDeltaY, xbool bVertical )
{
    if( m_CamType == CAMERA_FREEFLY )
    {
        f32 Foward = 5 * MouseDeltaY * m_CamTranslateSpeed;
        f32 Side   = 5 * MouseDeltaX * m_CamTranslateSpeed;

        if( bVertical ) 
            m_View.Translate( vector3( 0, -Foward, 0 ), view::WORLD );
        else 
            m_View.Translate( vector3( 0, 0, -Foward ), view::VIEW );
        

        m_View.Translate( vector3( -Side, 0, 0), view::VIEW );
    }
    else if ( m_CamType == CAMERA_ORBIT )
    {
        radian      Pitch    = 0;
        radian      Yaw      = 0;
        vector3     Dir      = m_View.GetPosition() - m_CamFocus;
        f32         Distance = Dir.Length();
        f32         Foward   = Distance + Distance * MouseDeltaY * 0.005f * m_CamTranslateSpeed;

        if( Foward < 1 ) Foward = 1;
        Dir.GetPitchYaw( Pitch, Yaw );
        if (bVertical)
        {
            Pitch -= MouseDeltaY * 0.005f * m_CamRotateSpeed;
            Yaw   -= MouseDeltaX * 0.005f * m_CamRotateSpeed;
            m_View.OrbitPoint( m_CamFocus, Distance, Pitch, Yaw );
        }
        else
        {
            Yaw   -= MouseDeltaX * 0.005f * m_CamRotateSpeed;
            m_View.OrbitPoint( m_CamFocus, Foward, Pitch, Yaw );
        }
    }
}

//===========================================================================

void CView3D::OnMouseMove(UINT nFlags, CPoint point) 
{
    BOOL bBaseViewCalled = FALSE;
    if( IsActionMode() )
    {
        //lets force the mouse pointer to the center of the screen
        //if the mouse point has moved close to the window edge
        //so the users never move the mouse out of bounds
        CRect rcWindow;
        GetWindowRect(&rcWindow);
        CPoint ptMouse(rcWindow.left + (rcWindow.Width()/2), rcWindow.top + (rcWindow.Height()/2));
        rcWindow.DeflateRect(10,10);

        CPoint ptLoc = point;
        ::ClientToScreen(m_hWnd, &ptLoc);
        if (!rcWindow.PtInRect(ptLoc))
        {
            CRect rcClient;
            GetClientRect(&rcClient);
            m_MouseOldPos = CPoint(rcClient.Width()/2,rcClient.Height()/2);
        	CBaseView::OnMouseMove(nFlags, m_MouseOldPos);
            bBaseViewCalled = TRUE;
            ::SetCursorPos(ptMouse.x,ptMouse.y);
        }
    }

    if (!bBaseViewCalled)
    {
    	CBaseView::OnMouseMove(nFlags, point);
    }
    
    //
    // Handle Camera
    //
    if( IsActionMode() )
    {
        s32 WhatToDo;

        WhatToDo = (s32)GetMouseLeftButton() | (((s32)GetMouseRightButton())<<1);

        if( m_CamType == CAMERA_FREEFLY )
        {
            if( nFlags & MK_MBUTTON )
            {
                CameraTranslate( GetMouseDeltaX(), GetMouseDeltaY(), TRUE );
                SetViewDirty();
            }
            else
            {
                switch( WhatToDo )
                {
                case 0:     // Nothing to do ( No buttons down )
                            break;      

                case 1:     // Left Mouse so then rotate
                            CameraRotate( GetMouseDeltaX(), GetMouseDeltaY() );
                            SetViewDirty();
                            break;

                case 2:     // Right Mouse then translate
                            CameraRotate( GetMouseDeltaX(), GetMouseDeltaY() );
                            SetViewDirty();
                            break;

                case 3:     // Both buttons then translate vertically
                            CameraTranslate( GetMouseDeltaX(), GetMouseDeltaY(), FALSE );
                            SetViewDirty();
                            break;
                }
            }

            /*
            RECT Rect;
            GetWindowRect( &Rect );

            s32 CenterX = (Rect.right + Rect.left) >> 1;
            s32 CenterY = (Rect.bottom + Rect.top) >> 1;

            SetCursorPos( CenterX, CenterY );

            GetClientRect( &Rect );

            CenterX = (Rect.right + Rect.left) >> 1;
            CenterY = (Rect.bottom + Rect.top) >> 1;

            m_MouseOldPos.x = CenterX;
            m_MouseOldPos.y = CenterY;
            */
        }
        else if( m_CamType == CAMERA_ORBIT )
        {
            switch( WhatToDo )
            {
            case 0:     // Nothing to do ( No buttons down )
                        break;      

            case 2:     
                        CameraRotate( GetMouseDeltaX(), GetMouseDeltaY() );
                        SetViewDirty();
                        break;

            case 3:     
                        CameraTranslate( GetMouseDeltaX(), GetMouseDeltaY(), FALSE );
                        SetViewDirty();
                        break;

            case 1:     
                        CameraTranslate( GetMouseDeltaX(), GetMouseDeltaY(), TRUE );
                        SetViewDirty();
                        break;
            }
        }
    }
}

//===========================================================================

void CView3D::CameraOrbitMode( const vector3& Focus )
{
    m_CamFocus    = Focus;
    m_View.LookAtPoint( m_CamFocus );
    m_CamType = CAMERA_ORBIT;
    SetViewDirty();
}

//===========================================================================

void CView3D::CameraOrbitMode( void )
{
    f32 CamDistance = 300;
    
    m_CamFocus    = m_View.ConvertV2W( vector3( 0, 0, CamDistance ) );

    m_View.LookAtPoint( m_CamFocus );
    m_CamType = CAMERA_ORBIT;
    SetViewDirty();
}

//===========================================================================

void CView3D::CameraFreeFlyMode( void )
{
    m_CamType = CAMERA_FREEFLY;
    SetViewDirty();
}

//===========================================================================

void CView3D::CameraSetFocus( const vector3& Focus )
{
    m_CamFocus = Focus;
    SetViewDirty();
}

//===========================================================================

BOOL CView3D::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    m_View.Translate( vector3( 0, 0, (float)zDelta), view::VIEW );
	SetViewDirty();
	return CBaseView::OnMouseWheel(nFlags, zDelta, pt);
}

//===========================================================================

void CView3D::OnInitialUpdate() 
{
	CBaseView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	CRealTimeMessage* pTimer = new CRealTimeMessage( GetSafeHwnd(), 16, WM_TIMER );
    if( pTimer )
        pTimer->CreateThread(0, 0, NULL);
}

//===========================================================================

void CView3D::OnTimer(UINT nIDEvent) 
{
    if( m_bDirtyView )
    {
        m_bDirtyView = FALSE;
        RedrawWindow();
    }
}

//===========================================================================

xbool CView3D::IsActionMode( void )
{
    return GetMouseRightButton();
}
