// View3D1.cpp : implementation file
//

//#include "BaseStdAfx.h"
//#include "editor.h"
#include "stdafx.h"
#include "View3D.h"
//#include "RealTimeMessage.h"


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
    m_bDirtyView = TRUE;
}

//===========================================================================

void CView3D::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
}

//===========================================================================

BOOL CView3D::OnEraseBkgnd(CDC* pDC) 
{
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
        f32 Foward = MouseDeltaY * m_CamTranslateSpeed;
        f32 Side   = MouseDeltaX * m_CamTranslateSpeed;

        if( bVertical ) 
            m_View.Translate( vector3( 0, Foward, 0 ), view::WORLD );
        else 
            m_View.Translate( vector3( 0, 0, Foward ), view::VIEW );
        

        m_View.Translate( vector3( Side, 0, 0), view::VIEW );
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
        Yaw   -= MouseDeltaX * 0.005f * m_CamRotateSpeed;
        m_View.OrbitPoint( m_CamFocus, Foward, Pitch, Yaw );
    }
}

//===========================================================================

void CView3D::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBaseView::OnMouseMove(nFlags, point);
    
    //
    // Handle Camera
    //
    if( IsActionMode() )
    {
        s32 WhatToDo;

        WhatToDo = (s32)GetMouseLeftButton() | (((s32)GetMouseRightButton())<<1);

        if( m_CamType == CAMERA_FREEFLY )
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
                        CameraTranslate( GetMouseDeltaX(), GetMouseDeltaY(), FALSE );
                        SetViewDirty();
                        break;

            case 3:     // Both buttons then translate vertically
                        CameraTranslate( GetMouseDeltaX(), GetMouseDeltaY(), TRUE );
                        SetViewDirty();
                        break;
            }
        }
        else if( m_CamType == CAMERA_ORBIT )
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
                        CameraTranslate( GetMouseDeltaX(), GetMouseDeltaY(), FALSE );
                        SetViewDirty();
                        break;

            case 3:     // Both buttons then translate vertically
                        CameraTranslate( GetMouseDeltaX(), GetMouseDeltaY(), TRUE );
                        SetViewDirty();
                        break;
            }
        }
    }
    else if (nFlags & MK_MBUTTON)
    {
        //rotate camera
        CameraRotate( GetMouseDeltaX(), GetMouseDeltaY() );
        SetViewDirty();
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
    // TODO: Fix window to get timer update messages and remove this Redraw call
    RedrawWindow();
	return CBaseView::OnMouseWheel(nFlags, zDelta, pt);
}

//===========================================================================

void CView3D::OnInitialUpdate() 
{
	CBaseView::OnInitialUpdate();
	
//	CRealTimeMessage* pTimer = new CRealTimeMessage( GetSafeHwnd(), 16, WM_TIMER );
//    if( pTimer )
//        pTimer->CreateThread(0, 0, NULL);
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


