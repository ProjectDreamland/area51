// ParticleView3D.cpp : implementation file
//

#include "stdafx.h"
#include "PartEd.h"
#include "MainFrm.h"
#include "PartEdDoc.h"
#include "ParticleView3D.h"
#include "e_Draw.hpp"
#include "ManipulatorMgr.h"
#include "Manipulator.h"
#include "ManipTranslate.h"
#include "SelectGizmo.hpp"
#include "Auxiliary/fx_core/element_sprite.hpp"
#include "Auxiliary/fx_core/element_spemitter.hpp"
#include "Auxiliary/fx_core/element_mesh.hpp"
#include "Auxiliary/fx_core/element_plane.hpp"
#include "Auxiliary/fx_core/element_ribbon.hpp"
#include "Auxiliary/fx_core/element_cylinder.hpp"
#include "Auxiliary/fx_core/element_sphere.hpp"
#include "Auxiliary/fx_core/element_shockwave.hpp"
#include "MouseMode.hpp"
#include "ElemBrowser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

enum create_type
{
    CREATE_SPRITE,
    CREATE_SPEMITTER,
    CREATE_MESH,
    CREATE_PLANE,
    CREATE_RIBBON,
    CREATE_SPHERE,
    CREATE_CYLINDER,
    CREATE_SHOCKWAVE
};

create_type     s_CreateType;
mousestate      g_MouseState;
gizmo_xlate*    g_pTranslate;
static s32      s_ElemCount = 0;
static xbool    g_GridEnabled = TRUE;

/////////////////////////////////////////////////////////////////////////////
// CParticleView3D

IMPLEMENT_DYNCREATE(CParticleView3D, CView3D)

CParticleView3D::CParticleView3D()
{
    g_MouseState.SetupPlane ( m_Grid.m_Plane );
    g_MouseState.SetView    ( &m_View );

    g_pTranslate            = NULL;

    m_IsPopupMenuActive     = false;

    // Initialize View
    m_View.SetPosition      ( vector3(300,100, 500) );
    m_View.LookAtPoint      ( vector3(0,  0,   0) );
}

CParticleView3D::~CParticleView3D()
{
    if ( g_pTranslate )
    {
        delete g_pTranslate;
        g_pTranslate = NULL;
    }
}


BEGIN_MESSAGE_MAP(CParticleView3D, CView3D)
	//{{AFX_MSG_MAP(CParticleView3D)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_KEYUP()
	ON_COMMAND(ID_VIEW_VIEWPORT_GRIDS, OnViewViewportGrids)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIEWPORT_GRIDS, OnUpdateViewViewportGrids)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BBOX, OnUpdateViewBbox)
	ON_COMMAND(ID_EDIT_SELECT_BYNAME, OnEditSelectByname)
	ON_COMMAND(ID_EDIT_CLONE, OnEditClone)
    ON_COMMAND(ID_EDIT_MOVETO_000, OnEditMoveToZero)
	ON_COMMAND(ID_VIEW_REDRAW_ALL, OnViewRedrawAll)
	ON_COMMAND(ID_VIEW_BBOX, OnViewBbox)
    ON_COMMAND(ID_VIEW_TRAJECTORY, OnViewTrajectory)
    ON_COMMAND(ID_VIEW_VELOCITY, OnViewVelocity)
    ON_COMMAND(ID_VIEW_HIDE_SELECTED, OnViewHideSelected)
    ON_COMMAND(ID_VIEW_UNHIDE_ALL, OnViewUnhideAll)
	ON_COMMAND(ID_CREATE_SPRITE, OnCreateSprite)
	ON_COMMAND(ID_CREATE_SPEMITTER, OnCreateSpemitter)
	ON_COMMAND(ID_CREATE_MESH, OnCreateMesh)
	ON_COMMAND(ID_CREATE_PLANE, OnCreatePlane)
	ON_COMMAND(ID_CREATE_RIBBON, OnCreateRibbon)
	ON_COMMAND(ID_CREATE_CYLINDER, OnCreateCylinder)
	ON_COMMAND(ID_CREATE_SPHERE, OnCreateSphere)
	ON_COMMAND(ID_CREATE_SHOCKWAVE, OnCreateShockWave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParticleView3D - Public Functions
/////////////////////////////////////////////////////////////////////////////

//--------------------------+
//  SetCameraPos            |
//--------------------------+

void    CParticleView3D::SetCameraPos( float PosX, float PosY, float PosZ )
{
    m_CameraPos         = vector3( PosX, PosY, PosZ );
    m_View.SetPosition  ( m_CameraPos );
    m_View.LookAtPoint  ( vector3(0, 0, 0) );
}

void    CParticleView3D::SetCameraPos( const vector3& Pos )
{
    m_CameraPos         = Pos;
    m_View.SetPosition  ( m_CameraPos );
    m_View.LookAtPoint  ( vector3(0, 0, 0) );
}

//--------------------------+
//  SetCamera_Top           |
//--------------------------+

void    CParticleView3D::SetCamera_Top( void )
{
//    m_View.SetOrtho( true );
}

//--------------------------+
//  SetCamera_Bottom        |
//--------------------------+

void    CParticleView3D::SetCamera_Bottom( void )
{
//    m_View.SetOrtho( true );
}

//--------------------------+
//  SetCamera_Left          |
//--------------------------+

void    CParticleView3D::SetCamera_Left( void )
{
//    m_View.SetOrtho( true );
}

//--------------------------+
//  SetCamera_Right         |
//--------------------------+

void    CParticleView3D::SetCamera_Right( void )
{
//    m_View.SetOrtho( true );
}

//--------------------------+
//  SetCamera_Front         |
//--------------------------+

void    CParticleView3D::SetCamera_Front( void )
{
//    m_View.SetOrtho( true );
}

//--------------------------+
//  SetCamera_Back          |
//--------------------------+

void    CParticleView3D::SetCamera_Back( void )
{
//    m_View.SetOrtho( true );
}

//--------------------------+
//  SetCamera_User          |
//--------------------------+

void    CParticleView3D::SetCamera_User( void )
{
//    m_View.SetOrtho( true );
}

//--------------------------+
//  SetCamera_Perspective   |
//--------------------------+

void    CParticleView3D::SetCamera_Perspective( void )
{
//    m_View.SetOrtho( false );
}


/////////////////////////////////////////////////////////////////////////////
// CParticleView3D drawing

void CParticleView3D::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
}

/////////////////////////////////////////////////////////////////////////////
// CParticleView3D diagnostics

#ifdef _DEBUG
void CParticleView3D::AssertValid() const
{
	CView3D::AssertValid();
}

void CParticleView3D::Dump(CDumpContext& dc) const
{
	CView3D::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CParticleView3D message handlers

void CParticleView3D::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CView3D::OnPaint();

    // Get document
    CPartEdDoc* pDoc = (CPartEdDoc*)GetDocument();
    ASSERT( pDoc );

    if( eng_InBeginEnd() )
        return;

    eng_SetBackColor( xcolor(0x0f, 0x0f, 0x0f, 0xff) );

    //-------------------------------------------------------------------------
    //  Read the frame we are rendering, with sub frame accuracy
    //-------------------------------------------------------------------------

    f32 RenderFrame = g_pMainFrame->GetGlobalTime();
    //x_printfxy( 1, 1, "RenderFrame = %f", RenderFrame );

    //-------------------------------------------------------------------------

    eng_Begin( "ParticleView3D" );

        // Render effect background
        pDoc->m_Effect.RenderBackground( RenderFrame );

        // Render Grid
        if( g_GridEnabled )
        {
            m_Grid.Render();
        }

        // Render the effect
        pDoc->m_Effect.Render( RenderFrame );
    
    eng_End();

    //-------------------------------------------------------------------------

    // Update manipulator
    if ( pDoc->m_SelSet.GetCount() )
    {        
        if ( !g_pTranslate )
        {
            g_pTranslate = new gizmo_xlate();
            ASSERT( g_pTranslate );
        }

        if ( g_pTranslate && (g_MouseState.GetState() != mousestate::BEING_DRAGGED) )
        {
            vector3     Point;
            POSITION    Pos     =   pDoc->m_SelSet.GetHeadPosition();
            g_pTranslate->m_StartPos.Set(0,0,0);
            while ( Pos )
            {
                fx_core::element* pElem = pDoc->m_SelSet.GetNext( Pos );
                pElem->GetPositionAtTime( RenderFrame, Point );
                g_pTranslate->AddPoint( Point );
            }
            g_pTranslate->CalcAvg();
            g_pTranslate->m_pTrans->SetPosition( g_pTranslate->m_StartPos );
        }
    }
    else
    {
        // no items, make sure gizmo is gone
        if ( g_pTranslate )
        {
            delete g_pTranslate;
            g_pTranslate = NULL;
        }
    }

    //-------------------------------------------------------------------------

    // Render the manipulators
    g_ManipulatorMgr.Render( m_View );

    // Done
    eng_PageFlip();

    // Now draw the rubber-banding box if req'd
    if ( g_MouseState.GetContext() == this )
    {
        if ( g_MouseState.GetState() == mousestate::SELECT2 )
        {
            POINT p1, p2;
            g_MouseState.GetPickPoint( 0, p1 );
            g_MouseState.GetPickPoint( 1, p2 );
            dc.SelectStockObject( WHITE_PEN );
            dc.MoveTo( p1.x, p1.y );
            dc.LineTo( p2.x, p1.y );
            dc.LineTo( p2.x, p2.y );
            dc.LineTo( p1.x, p2.y );
            dc.LineTo( p1.x, p1.y );
        }
    }

    // UpdateKeyBar();
	// Do not call CView3D::OnPaint() for painting messages
}

void CParticleView3D::OnMouseMove(UINT nFlags, CPoint point) 
{
    CPartEdDoc* pDoc = (CPartEdDoc*)GetDocument();

    // Update mouse tracking in base view
    CBaseView::OnMouseMove  ( nFlags, point );

    // set the context and the current view
    g_MouseState.SetContext ( (CView&)*this );
    g_MouseState.SetView    ( &m_View );

    // Grab the mouse movement
    s32     mouseDX         = GetMouseDeltaX();
    s32     mouseDY         = GetMouseDeltaY();

    // Left Button - Object Editing( Select, Create, etc )
    if( nFlags & MK_LBUTTON )
    {
        mousestate::state State = g_MouseState.GetState();

        switch( State )
        {
            case mousestate::BEING_DRAGGED:
            {
                // translate items?
                if ( g_pTranslate )
                {
                    switch( g_pMainFrame->GetManipulateMode() )
                    {
                        case CMainFrame::MANIPULATE_MOVE:
                        {
                            // Update any associated items in the selection set
                            vector3 NewPos              = g_pTranslate->m_pTrans->GetPosition();
                            vector3 Delta               = NewPos - g_pTranslate->m_StartPos;
                            g_pTranslate->m_StartPos    = NewPos;

                            POSITION ElemPos = pDoc->m_SelSet.GetHeadPosition();

                            // if keyframing, create the key(s)
                            if( pDoc->GetAnimMode() )
                            { 
                                g_pTranslate->m_PointCount = 0;

                                while( ElemPos )
                                {
                                    fx_core::element* pElem = pDoc->m_SelSet.GetNext( ElemPos );
                                    vector3 Pos;
                                    pElem->GetPositionAtTime( g_pMainFrame->GetGlobalTime(), Pos );
                                    pElem->AddKey_Pos( (s32)g_pMainFrame->GetGlobalTime(), Pos + Delta );
                                }

                                UpdateKeyBar();
                            }
                            else // otherwise, shift all keys by delta
                            {
                                while( ElemPos )
                                {
                                    fx_core::element* pElem = pDoc->m_SelSet.GetNext( ElemPos );
                                    pElem->Translate( Delta );
                                }
                            }

                            break;
                        }

                        case CMainFrame::MANIPULATE_ROTATE:
                        {
                            // Update any associated items in the selection set
                            vector3 NewPos              = g_pTranslate->m_pTrans->GetPosition();
                            vector3 Delta               = NewPos - g_pTranslate->m_StartPos;
                            g_pTranslate->m_StartPos    = NewPos;

                            POSITION ElemPos = pDoc->m_SelSet.GetHeadPosition();

                            // if keyframing, create the key(s)
                            if( pDoc->GetAnimMode() )
                            { 
                                g_pTranslate->m_PointCount = 0;

                                while( ElemPos )
                                {
                                    fx_core::element* pElem = pDoc->m_SelSet.GetNext( ElemPos );

                                    vector3 Rot;
                                    pElem->GetRotationAtTime( g_pMainFrame->GetGlobalTime(), Rot );
                                    Rot += ( 0.02f * Delta );

                                    radian3 NewRot( Rot.GetX(), Rot.GetY(), Rot.GetZ() );
                                    pElem->AddKey_Rotation( (s32)g_pMainFrame->GetGlobalTime(), NewRot );
                                }

                                UpdateKeyBar();
                            }
                            else // otherwise, shift all keys by delta
                            {
                                while( ElemPos )
                                {
                                    fx_core::element* pElem = pDoc->m_SelSet.GetNext( ElemPos );
                                    pElem->Rotate( 0.02f * radian3(Delta.GetX(), Delta.GetY(), Delta.GetZ()) );
                                }
                            }

                            break;
                        }

                        case CMainFrame::MANIPULATE_SCALE:
                        {

                            break;
                        }
                    }
                }
                pDoc->UpdateAllViews(NULL);

                break;
            }
            
            case mousestate::SELECT2:
            {
                g_MouseState.SetPickPoint( 1, point.x, point.y );
                // make sure we're in the same window we started in
                if ( this != g_MouseState.GetContext() )
                {
                    g_MouseState.SetState( mousestate::SELECT );
                }
                else
                    Invalidate(false);

                break;
            }

            default:
                break;
        }
    }

    // Middle Butotn - Viewport Navigation( Pan, Orbit, Look, Zoom )
    if( nFlags & MK_MBUTTON )
    {
        xbool   Control_Pushed  = ( nFlags & MK_CONTROL );
        xbool   Shift_Pushed    = ( nFlags & MK_SHIFT );
        xbool   Alt_Pushed      = ( GetAsyncKeyState(VK_MENU) & 0x8000 );

        if( !Control_Pushed && !Alt_Pushed && !Shift_Pushed )       // MMB (No modifier keys)
        {
            // Pan
            m_View.Translate( vector3( (f32)GetMouseDeltaX(), (f32)GetMouseDeltaY(), 0), view::VIEW );
        }
        else if( !Control_Pushed && !Alt_Pushed && Shift_Pushed )   // Shift + MMB
        {
            // Pan - ACCELERATED
            m_View.Translate( vector3( (f32)GetMouseDeltaX() * 5.0f, (f32)GetMouseDeltaY() * 5.0f, 0), view::VIEW );
        }
        else if( !Control_Pushed && Alt_Pushed && !Shift_Pushed )   // Alt + MMB
        {
            // Orbit
            CPartEdDoc*     pDoc            = (CPartEdDoc*)GetDocument();
            int             numSelElements  = pDoc->m_SelSet.GetCount();

            // Orbit around center of selection...if there is one.  Otherwise use existing view center
            if( numSelElements > 0 )
            {
                fx_core::element*   pElem;
                vector3             selElementPos   (0,0,0);
                vector3             orbitPivot      (0,0,0);
                POSITION            Pos             = pDoc->m_SelSet.GetHeadPosition();

                // Get orbit pivot point(Center of selection)
                while(Pos)
                {
                    pElem = pDoc->m_SelSet.GetNext(Pos);
                    pElem->GetPositionAtTime( g_pMainFrame->GetGlobalTime(), selElementPos );
                    orbitPivot += selElementPos;
                }

                orbitPivot  /= (float)numSelElements;

                // Set to orbit around the center of the selection
                CameraOrbitMode( orbitPivot );
            }
            else
            {
                // Set to orbit around existing view camera target
                CameraOrbitMode();
            }

             CameraRotate( GetMouseDeltaX(), GetMouseDeltaY() );
        }
        else if( Control_Pushed && !Alt_Pushed && !Shift_Pushed )    // Ctrl + MMB
        {
            // Look
            CameraFreeFlyMode();
            CameraRotate( GetMouseDeltaX(), GetMouseDeltaY() );
        }
        else if( Control_Pushed && Alt_Pushed && !Shift_Pushed )    // Ctrl + Alt + MMB
        {
            // Zoom
            m_View.Translate( vector3( 0, 0, (f32)GetMouseDeltaY() * -2.5f ), view::VIEW );
        }

        Invalidate();
    }

    // Update manipulator manager using the ray
    xbool   Dirty   = g_ManipulatorMgr.Update( m_View, (f32)GetMouseX(), (f32)GetMouseY() );

    // If manipulators are dirty then redraw the views
    if( Dirty )
    {
        RedrawWindow();
//        GetDocument()->UpdateAllViews( NULL );
    }
}

int CParticleView3D::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView3D::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    ModifyStyleEx( WS_EX_CLIENTEDGE, 0 );

    // Initialize the Popup Menu

    m_PopupMenu.LoadMenu( IDR_MENU_VIEW3D );

	return 0;
}

void CParticleView3D::OnLButtonDown(UINT nFlags, CPoint point) 
{
    g_MouseState.SetContext( (CView&)*this );
    g_MouseState.SetView( &m_View );
    SetCapture();

    if ( g_MouseState.GetState() == mousestate::SELECT )
    {
        if ( !g_ManipulatorMgr.BeginDrag( m_View, (f32)GetMouseX(), (f32)GetMouseY()) )
        {
            // also drag a rectangle
            g_MouseState.SetPickPoint( 0, point.x, point.y );
            g_MouseState.SetState( mousestate::SELECT2 );
        }
        else
            g_MouseState.SetState( mousestate::BEING_DRAGGED );
    }	

    // Reset the popup menu status...since a left click should always close it down
    m_IsPopupMenuActive = false;

    CView3D::OnLButtonDown(nFlags, point);
}

void CParticleView3D::OnLButtonUp(UINT nFlags, CPoint point) 
{
    g_MouseState.SetContext( (CView&)*this );
    g_MouseState.SetView( &m_View );
    ReleaseCapture();

    vector3 Pos = g_MouseState.GetPoint( point.x, point.y );
    CPartEdDoc* pDoc = (CPartEdDoc*)GetDocument();
    bbox BBox;

    pDoc->SetModifiedFlag(TRUE);

    switch( g_MouseState.GetState() )
    {
        case mousestate::BEING_DRAGGED:
            g_ManipulatorMgr.EndDrag( m_View );
            g_MouseState.SetState( mousestate::SELECT );
            pDoc->PopulatePropertyControl();
            break;

        case mousestate::SELECT:
            break;
        case mousestate::SELECT2:
            {
                BOOL WasDragging = g_ManipulatorMgr.EndDrag( m_View );

                g_MouseState.SetState( mousestate::SELECT );

                // create a selection set
                // Should we try to select different items?
                if ( WasDragging == FALSE )
                {
                    delete g_pTranslate;

                    // alloc gizmo
                    g_pTranslate = new gizmo_xlate();
                    ASSERT( g_pTranslate );

                    // Set final pick point
                    g_MouseState.SetPickPoint( 1, point.x, point.y );

                    // select me some objects
                    POSITION DataPos = pDoc->m_SelSet.GetHeadPosition();

                    pDoc->m_SelSet.RemoveAll();
                    g_pMainFrame->m_KeyBar.SetKeySets( 0, NULL );
                    
                    g_MouseState.SetupSelVol();

                    // search the elements
                    fx_core::effect& Effect = pDoc->m_Effect;
                    for( s32 i=0 ; i<Effect.GetNumElements() ; i++ )
                    {
                        fx_core::element* pElement = Effect.GetElement( i );

                        // Clear the selected flag
                        pElement->SetSelected( FALSE );

                        // Get World BBox & test if object exists at T
                        if( pElement->GetWorldBBoxAtTime( g_pMainFrame->GetGlobalTime(), BBox ) == TRUE )
                        {
                            if ( pElement->IsHidden() == FALSE )
                            {
                                if( g_MouseState.BBoxInSelVol( BBox ) )
                                {
                                    // add to the selection set
                                    pDoc->m_SelSet.AddTail( pElement );

                                    // Tell the element it is selected
                                    pElement->SetSelected( TRUE );

                                    // gather position data for positioning the manipulator
                                    vector3 Point;
                                    pElement->GetPositionAtTime( g_pMainFrame->GetGlobalTime(), Point );
                                }
                            }
                        }
                    }
                }

                // Update controls
                UpdateKeyBar();
                pDoc->PopulatePropertyControl();
            }

            // Force redraw of all views
            GetDocument()->UpdateAllViews( NULL );
            break;
        
        case mousestate::CREATE:
            {
                s32 CreateTime = pDoc->GetAnimMode()?(s32)g_pMainFrame->GetGlobalTime():0;

                // create the new element
                switch( s_CreateType )
                {
                case CREATE_SPRITE:
                    {
                        fx_core::element_sprite* pSprite = new fx_core::element_sprite;
                        s32 index = 1;
                        char ID[64];
                        do 
                        {
                            x_sprintf( ID, "SPRITE %-3.3d", index++ );

                        } while( pDoc->m_Effect.FindElementByName(ID) != NULL );
                        
                        pSprite->Create( ID, pDoc->m_Effect );

                        pSprite->AddKey_SRT( CreateTime, 
                                                vector3(100,100,100),
                                                radian3(0,0,0),
                                                g_MouseState.GetPoint( point.x, point.y ) );
                        pSprite->AddKey_Color( CreateTime, xcolor(255,255,255,255) );

                        pDoc->m_SelSet.RemoveAll();
                        pDoc->m_Effect.SetAllElementsSelected( FALSE );
                        pDoc->m_SelSet.AddHead( pSprite );
                        pSprite->SetSelected( TRUE );

                        pDoc->UpdateAllViews( NULL );
                        pDoc->PopulatePropertyControl();
                        UpdateKeyBar();
                    }
                    break;
                case CREATE_SPEMITTER:
                    {
                        fx_core::element_spemitter* pEmitter = new fx_core::element_spemitter;
                        
                        s32 index = 1;
                        char ID[64];
                        do 
                        {
                            x_sprintf( ID, "SPEMITTER %-3.3d", index++ );

                        } while( pDoc->m_Effect.FindElementByName(ID) != NULL );

                        pEmitter->Create( ID, pDoc->m_Effect );
                        //pEmitter->AddKey_Pos( g_pMainFrame->GetGlobalTime(), g_MouseState.GetPoint( point.x, point.y ) );
                        pEmitter->AddKey_SRT( CreateTime, 
                                                vector3(1,1,1),
                                                radian3(0,0,0),
                                                g_MouseState.GetPoint( point.x, point.y ) );
                        pEmitter->AddKey_Color( CreateTime, xcolor(255,255,255,255) );

                        pDoc->m_SelSet.RemoveAll();
                        pDoc->m_Effect.SetAllElementsSelected( FALSE );
                        pDoc->m_SelSet.AddHead( pEmitter );
                        pEmitter->SetSelected( TRUE );

                        pDoc->UpdateAllViews( NULL );
                        pDoc->PopulatePropertyControl();
                        UpdateKeyBar();
                    }
                    break;
                case CREATE_MESH:
                    {
                        fx_core::element_mesh* pMesh = new fx_core::element_mesh;
                        s32 index = 1;
                        char ID[64];
                        do 
                        {
                            x_sprintf( ID, "MESH %-3.3d", index++ );

                        } while( pDoc->m_Effect.FindElementByName(ID) != NULL );
                        pMesh->Create( ID, pDoc->m_Effect );
                        //pMesh->AddKey_Pos( g_pMainFrame->GetGlobalTime(), g_MouseState.GetPoint( point.x, point.y ) );
                        pMesh->AddKey_SRT( CreateTime, 
                                           vector3(1,1,1),
                                           radian3(0,0,0),
                                           g_MouseState.GetPoint( point.x, point.y ) );
                        pMesh->AddKey_Color( CreateTime, xcolor(255,255,255,255) );

                        pDoc->m_SelSet.RemoveAll();
                        pDoc->m_Effect.SetAllElementsSelected( FALSE );
                        pDoc->m_SelSet.AddHead( pMesh );
                        pMesh->SetSelected( TRUE );

                        pDoc->UpdateAllViews( NULL );
                        pDoc->PopulatePropertyControl();
                        UpdateKeyBar();
                    }
                    break;
                case CREATE_PLANE:
                    {
                        fx_core::element_plane* pPlane = new fx_core::element_plane;
                        s32 index = 1;
                        char ID[64];
                        do 
                        {
                            x_sprintf( ID, "PLANE %-3.3d", index++ );

                        } while( pDoc->m_Effect.FindElementByName(ID) != NULL );
                        
                        pPlane->Create( ID, pDoc->m_Effect );

                        pPlane->AddKey_SRT( CreateTime, 
                                            vector3(100,100,1),
                                            radian3(0,0,0),
                                            g_MouseState.GetPoint( point.x, point.y ) );
                        pPlane->AddKey_Color( CreateTime, xcolor(255,255,255,255) );

                        pDoc->m_SelSet.RemoveAll();
                        pDoc->m_Effect.SetAllElementsSelected( FALSE );
                        pDoc->m_SelSet.AddHead( pPlane );
                        pPlane->SetSelected( TRUE );

                        pDoc->UpdateAllViews( NULL );
                        pDoc->PopulatePropertyControl();
                        UpdateKeyBar();
                    }
                    break;
                case CREATE_RIBBON:
                    {
                        fx_core::element_ribbon* pRibbon = new fx_core::element_ribbon;
                        s32 index = 1;
                        char ID[64];
                        do 
                        {
                            x_sprintf( ID, "RIBBON %-3.3d", index++ );

                        } while( pDoc->m_Effect.FindElementByName(ID) != NULL );
                        
                        pRibbon->Create( ID, pDoc->m_Effect );

                        pRibbon->AddKey_SRT( CreateTime, 
                                              vector3(100,100,100),
                                              radian3(0,0,0),
                                              g_MouseState.GetPoint( point.x, point.y ) );
                        pRibbon->AddKey_Color( CreateTime, xcolor(255,255,255,255) );

                        pDoc->m_SelSet.RemoveAll();
                        pDoc->m_Effect.SetAllElementsSelected( FALSE );
                        pDoc->m_SelSet.AddHead( pRibbon );
                        pRibbon->SetSelected( TRUE );

                        pDoc->UpdateAllViews( NULL );
                        pDoc->PopulatePropertyControl();
                        UpdateKeyBar();
                    }
                    break;
                case CREATE_SPHERE:
                    {
                        fx_core::element_sphere* pSphere = new fx_core::element_sphere;
                        s32 index = 1;
                        char ID[64];
                        do 
                        {
                            x_sprintf( ID, "SPHERE %-3.3d", index++ );

                        } while( pDoc->m_Effect.FindElementByName(ID) != NULL );
                        
                        pSphere->Create( ID, pDoc->m_Effect );

                        pSphere->AddKey_SRT( CreateTime, 
                                             vector3(100,100,100),
                                             radian3(0,0,0),
                                             g_MouseState.GetPoint( point.x, point.y ) );
                        pSphere->AddKey_Color( CreateTime, xcolor(255,255,255,255) );

                        pDoc->m_SelSet.RemoveAll();
                        pDoc->m_Effect.SetAllElementsSelected( FALSE );
                        pDoc->m_SelSet.AddHead( pSphere );
                        pSphere->SetSelected( TRUE );

                        pDoc->UpdateAllViews( NULL );
                        pDoc->PopulatePropertyControl();
                        UpdateKeyBar();
                    }
                    break;
                case CREATE_CYLINDER:
                    {
                        fx_core::element_cylinder* pCylinder = new fx_core::element_cylinder;
                        s32 index = 1;
                        char ID[64];
                        do 
                        {
                            x_sprintf( ID, "CYLINDER %-3.3d", index++ );

                        } while( pDoc->m_Effect.FindElementByName(ID) != NULL );
                        
                        pCylinder->Create( ID, pDoc->m_Effect );

                        pCylinder->AddKey_SRT( CreateTime, 
                                               vector3(100,100,100),
                                               radian3(0,0,0),
                                               g_MouseState.GetPoint( point.x, point.y ) );
                        pCylinder->AddKey_Color( CreateTime, xcolor(255,255,255,255) );

                        pDoc->m_SelSet.RemoveAll();
                        pDoc->m_Effect.SetAllElementsSelected( FALSE );
                        pDoc->m_SelSet.AddHead( pCylinder );
                        pCylinder->SetSelected( TRUE );

                        pDoc->UpdateAllViews( NULL );
                        pDoc->PopulatePropertyControl();
                        UpdateKeyBar();
                    }
                    break;
                case CREATE_SHOCKWAVE:
                    {
                        fx_core::element_shockwave* pShockWave = new fx_core::element_shockwave;
                        s32 index = 1;
                        char ID[64];
                        do 
                        {
                            x_sprintf( ID, "SHOCKWAVE %-3.3d", index++ );

                        } while( pDoc->m_Effect.FindElementByName(ID) != NULL );
                        
                        pShockWave->Create( ID, pDoc->m_Effect );

                        pShockWave->AddKey_SRT( CreateTime, 
                                                vector3(100,10,100),
                                                radian3(0,0,0),
                                                g_MouseState.GetPoint( point.x, point.y ) );
                        pShockWave->AddKey_Color( CreateTime, xcolor(255,255,255,255) );

                        pDoc->m_SelSet.RemoveAll();
                        pDoc->m_Effect.SetAllElementsSelected( FALSE );
                        pDoc->m_SelSet.AddHead( pShockWave );
                        pShockWave->SetSelected( TRUE );

                        pDoc->UpdateAllViews( NULL );
                        pDoc->PopulatePropertyControl();
                        UpdateKeyBar();
                    }
                    break;
                }
                g_MouseState.SetState( mousestate::SELECT );
                break;
            }

        default:
            break;
    }
	
	CView3D::OnLButtonUp(nFlags, point);
}

BOOL CParticleView3D::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    // set the context and the current view
    g_MouseState.SetContext     ( (CView&)*this );
    g_MouseState.SetView        ( &m_View );

    if( nFlags & MK_CONTROL )
    {
        // Zoom - ACCLERATED
        m_View.Translate( vector3( 0, 0, (float)zDelta * 10.0f ), view::VIEW );
    }
    else
    {
        // Zoom
        m_View.Translate( vector3( 0, 0, (float)zDelta * 2.5f ), view::VIEW );
    }


	SetViewDirty();

    // TODO: Fix window to get timer update messages and remove this Redraw call
    RedrawWindow();

    return CBaseView::OnMouseWheel(nFlags, zDelta, pt);
}

void CParticleView3D::OnRButtonDown(UINT nFlags, CPoint point) 
{
    // If popup menu is already there, kill it....otherwise show it
    if( m_IsPopupMenuActive )
    {
        m_IsPopupMenuActive = false;
    }
    else
    {
        m_IsPopupMenuActive = true;

        ClientToScreen( &point );

//#ifdef jversluis
//        CMenu*  pPopup      = m_PopupMenu.GetSubMenu(1); // Experimental stuff
//#else
      CMenu*  pPopup      = m_PopupMenu.GetSubMenu(0);
//#endif
        int     popupCmd    = pPopup->TrackPopupMenu( TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, this );

        // Check to see if the user clicked on an actual command
        if( popupCmd != 0 )
        {
            // They did!  So, reset the popmenu state and send the command ID on
            m_IsPopupMenuActive = false;
            SendMessage( WM_COMMAND, popupCmd, 0 );
        }
    }
}

void CParticleView3D::OnMButtonDown(UINT nFlags, CPoint point) 
{
    SetCapture();
}

void CParticleView3D::OnMButtonUp(UINT nFlags, CPoint point) 
{
    ReleaseCapture();
}

void CParticleView3D::OnCreateSprite() 
{
    s_CreateType = CREATE_SPRITE;
    g_MouseState.SetState( mousestate::CREATE );
}

void CParticleView3D::OnCreateSpemitter() 
{
    s_CreateType = CREATE_SPEMITTER;
    g_MouseState.SetState( mousestate::CREATE );
}

void CParticleView3D::OnCreateMesh() 
{
	s_CreateType = CREATE_MESH;
    g_MouseState.SetState( mousestate::CREATE );
}

void CParticleView3D::OnCreatePlane() 
{
    s_CreateType = CREATE_PLANE;
    g_MouseState.SetState( mousestate::CREATE );
}

void CParticleView3D::OnCreateRibbon() 
{
    s_CreateType = CREATE_RIBBON;
    g_MouseState.SetState( mousestate::CREATE );
}

void CParticleView3D::OnCreateSphere() 
{
    s_CreateType = CREATE_SPHERE;
    g_MouseState.SetState( mousestate::CREATE );
}

void CParticleView3D::OnCreateCylinder() 
{
    s_CreateType = CREATE_CYLINDER;
    g_MouseState.SetState( mousestate::CREATE );
}

void CParticleView3D::OnCreateShockWave() 
{
    s_CreateType = CREATE_SHOCKWAVE;
    g_MouseState.SetState( mousestate::CREATE );
}

BOOL CParticleView3D::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    // Set appropriate cursor image
    switch( g_pMainFrame->GetManipulateMode() )
    {
        case CMainFrame::MANIPULATE_SELECT:
            SetCursor( ::LoadCursor(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_SELECT) ) );
            break;
        case CMainFrame::MANIPULATE_MOVE:
            SetCursor( ::LoadCursor(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_MOVE) ) );
            break;
        case CMainFrame::MANIPULATE_ROTATE:
            SetCursor( ::LoadCursor(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_ROTATE) ) );
            break;
        case CMainFrame::MANIPULATE_SCALE:
            SetCursor( ::LoadCursor(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_SCALE) ) );
            break;
    }
	
    return TRUE; // return CView3D::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CParticleView3D::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= WS_CLIPSIBLINGS|WS_CLIPCHILDREN;

	return CView3D::PreCreateWindow(cs);
}

void CParticleView3D::UpdateKeyBar( void )
{
    // Put the appropriate keys in the KeyBar
    keylist         KeySets;
    POSITION        DataPos;
    CPartEdDoc*     pDoc    = (CPartEdDoc*)GetDocument();

    if ( pDoc->m_SelSet.GetCount() )
    {
        // add each one
        DataPos = pDoc->m_SelSet.GetHeadPosition();
        while(DataPos)
        {
            fx_core::element*   pElem = pDoc->m_SelSet.GetNext(DataPos);
            KeySet*             pKey;

            pKey = new KeySet;
            pKey->m_Category = "Scale";
            CollectKeys( pElem->m_pScale, pKey );
            KeySets.AddTail( pKey );

            pKey = new KeySet;
            pKey->m_Category = "Rotation";
            CollectKeys( pElem->m_pRotation, pKey );
            KeySets.AddTail( pKey );
            
            pKey = new KeySet;
            pKey->m_Category = "Position";
            CollectKeys( pElem->m_pTranslation, pKey );
            KeySets.AddTail( pKey );
            
            pKey = new KeySet;
            pKey->m_Category = "Color";
            CollectKeys( pElem->m_pColor, pKey );
            KeySets.AddTail( pKey );
            
            pKey = new KeySet;
            pKey->m_Category = "Alpha";
            CollectKeys( pElem->m_pAlpha, pKey );
            KeySets.AddTail( pKey );        
        }

        // Key sets retrieved, give them to the time slider
        g_pMainFrame->m_KeyBar.SetKeySetCount( KeySets.GetCount() );
    }
    else
    {
        // Clear the KeyBar's key sets
        g_pMainFrame->m_KeyBar.SetKeySets( 0, NULL );
    }
    
    DataPos = KeySets.GetHeadPosition();
    s32 Count = 0;
    while (DataPos)
    {
        KeySet* pKeySet = KeySets.GetNext(DataPos);
        g_pMainFrame->m_KeyBar.SetKeySet( Count++, pKeySet );

        // uncollect the keys, no longer needed
        // controller* pCont = (controller*)pKeySet->m_UserData;
        // UncollectKeys( pKeySet );
        delete pKeySet;
    }

    if ( !pDoc->m_SelSet.GetCount() )
    {
        delete g_pTranslate;
        g_pTranslate = NULL;
//        g_pProperties->EraseAll();
    }

}

void CParticleView3D::CollectKeys( fx_core::ctrl_key* pCont, KeySet* pKeySet )
{
    pKeySet->m_KeyDataSize = pCont->GetNumFloats() * sizeof(f32);
    pKeySet->m_nKeys = pCont->GetKeyCount();
    pKeySet->m_pKeys = new KeyBarKey[pKeySet->m_nKeys];

    s32 i;

    pKeySet->m_UserData = (DWORD)pCont;

    for ( i = 0; i < pKeySet->m_nKeys; i++ )
    {
        fx_core::key* pKey = pCont->GetKeyByIndex(i);
        pKeySet->m_pKeys[i].m_pData = new f32[ pCont->GetNumFloats() ];
        pKeySet->m_pKeys[i].m_Time = pKey->GetKeyTime();
        x_memcpy( pKeySet->m_pKeys[i].m_pData, pKey->GetKeyValue(), pCont->GetNumFloats() * sizeof(f32) );        
    }
}


void CParticleView3D::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    CPartEdDoc*     pDoc    = (CPartEdDoc*)GetDocument();
    ASSERT(pDoc);

	if( nChar == VK_DELETE )
    {
        // delete the selected elements
        POSITION Pos = pDoc->m_SelSet.GetHeadPosition();

        while( Pos )
        {
            fx_core::element* pElem = pDoc->m_SelSet.GetNext( Pos );
            pDoc->m_Effect.RemoveElement( pElem );
        }

        pDoc->m_SelSet.RemoveAll();

        // remove translation gizmo
        if ( g_pTranslate )
        {
            delete g_pTranslate;
            g_pTranslate = NULL;
        }

        // clear selected keys
        g_pMainFrame->m_KeyBar.SetKeySets( 0, NULL );

        // update property view
        pDoc->PopulatePropertyControl();

        pDoc->UpdateAllViews(NULL);
    }

    else if( nChar == 36 ) // Hokey hard-coded "Frame Home" hotkey = "HOME"....until we get a real hotkey system
    {
        if( g_pMainFrame->m_KeyBar.IsPlayOn() )
        {
            g_pMainFrame->m_KeyBar.StopPlayback();
        }

        g_pMainFrame->m_KeyBar.SetTime( g_pMainFrame->m_KeyBar.GetTimeRangeStart() );
    }

    else if( nChar == 35 ) // Hokey hard-coded "Frame End" hotkey = "END"....until we get a real hotkey system
    {
        if( g_pMainFrame->m_KeyBar.IsPlayOn() )
        {
            g_pMainFrame->m_KeyBar.StopPlayback();
        }

        g_pMainFrame->m_KeyBar.SetTime( g_pMainFrame->m_KeyBar.GetTimeRangeEnd() );
    }

    else if( nChar == 188 ) // Hokey hard-coded "Frame Back" hotkey = ","....until we get a real hotkey system
    {
        if( g_pMainFrame->m_KeyBar.IsPlayOn() )
        {
            g_pMainFrame->m_KeyBar.StopPlayback();
        }

        g_pMainFrame->m_KeyBar.SetTime( (s32)g_pMainFrame->m_KeyBar.GetTime() - 1 );
    }

    else if( nChar == 190 ) // Hokey hard-coded "Frame Forward" hotkey = "."....until we get a real hotkey system
    {
        if( g_pMainFrame->m_KeyBar.IsPlayOn() )
        {
            g_pMainFrame->m_KeyBar.StopPlayback();
        }

        g_pMainFrame->m_KeyBar.SetTime( (s32)g_pMainFrame->m_KeyBar.GetTime() + 1 );
    }

    else if( nChar == 191 ) // Hokey hard-coded play hotkey = "/"....until we get a real hotkey system
    {
        if( g_pMainFrame->m_KeyBar.IsPlayOn() )     { g_pMainFrame->m_KeyBar.StopPlayback();  }
        else                                        { g_pMainFrame->m_KeyBar.StartPlayback(); }
    }


    else if( nChar == 72 ) // Hokey hard-coded "Select by Name" hotkey = "h"....until we get a real hotkey system
    {
        OnEditSelectByname();
    }
    else if( nChar == 192 ) // Hokey hard-coded "Redraw All" hotkey = "`"....until we get a real hotkey system
    {
        OnViewRedrawAll();
    }
    else if( nChar == 71 ) // Hokey hard-coded "Show Viewport Grid" hotkey = "g"....until we get a real hotkey system
    {
        OnViewViewportGrids();
    }
    else if( nChar == 16 ) // Hokey hard-coded move selection to [0,0,0] = "SHIFT + NUMPAD_ZERO"....until we get a real hotkeys
    {
        OnEditMoveToZero();
    }
    else if( nChar == 74 ) // Hokey hard-coded to toggle "Show Trajectory" hotkey = "j"....until we get a real hotkeys
    {
        OnViewTrajectory();
    }
    else if( nChar == 86 ) // Hokey hard-coded to toggle "Show Velocity" hotkey = "v"....until we get a real hotkeys
    {
        OnViewVelocity();
    }
    else if( nChar == 66 ) // Hokey hard-coded to toggle "Show BBoxes" hotkey = "b"....until we get a real hotkeys
    {
        OnViewBbox();
    }
    else if( nChar == 107 ) // Hokey hard-coded "Unhide All" hotkey = "Numpad +"....until we get a real hotkeys
    {
        OnViewUnhideAll();
    }
    else if( nChar == 109 ) // Hokey hard-coded "Hide Selected" hotkey = "Numpad -"....until we get a real hotkeys
    {
        OnViewHideSelected();
    }

	CView3D::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CParticleView3D::OnViewViewportGrids() 
{
	g_GridEnabled = !g_GridEnabled;
    GetDocument()->UpdateAllViews( NULL );
}

void CParticleView3D::OnUpdateViewViewportGrids(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( g_GridEnabled );
}

void CParticleView3D::GetCameraInfo( vector3& Pos, quaternion& Quat )
{
    Pos = m_View.GetPosition();
    Quat = m_View.GetV2W().GetQuaternion();
}

void CParticleView3D::SetCameraInfo( const vector3& Pos, const quaternion& Quat )
{
    matrix4 M;

    M.Setup( Quat );

    m_View.SetV2W( M );
    m_View.SetPosition( Pos );
}

void CParticleView3D::OnEditSelectByname() 
{
    CElemBrowser    Browser;
    CPartEdDoc*     pDoc        =   (CPartEdDoc*)GetDocument();    

    g_pMainFrame->m_KeyBar.StopPlayback();

    Browser.SetEffectDocument( pDoc );

    if ( Browser.DoModal() == IDOK )
    {
        pDoc->m_SelSet.RemoveAll();                         // Clear the current selection set

        pDoc->m_Effect.SetAllElementsSelected( FALSE );

        s32 SelCount = Browser.GetSelCount();

        for ( s32 i = 0; i < SelCount; i++ )                // select each element in turn
        {
            fx_core::element* pElem = (fx_core::element*)Browser.GetSelItem(i);
            
            if ( Browser.m_ShouldHide == FALSE )
            {
                pDoc->m_SelSet.AddTail( pElem );
                pElem->SetSelected( TRUE );
                pElem->Show();
            }
            else
            {
                pElem->Hide();
            }
        }

        UpdateKeyBar();
        pDoc->PopulatePropertyControl();
        pDoc->UpdateAllViews(NULL);
    }
}

void CParticleView3D::OnEditClone()
{
    CPartEdDoc*     pDoc        =   (CPartEdDoc*)GetDocument();

    g_pMainFrame->m_KeyBar.StopPlayback();

    // Duplicate the selected elements
    CList<fx_core::element*, fx_core::element*>   NewElements;
    POSITION Pos = pDoc->m_SelSet.GetHeadPosition();

    while( Pos )
    {
        fx_core::element* pElem = pDoc->m_SelSet.GetNext( Pos );

        fx_core::element* pNewElem = pElem->Duplicate();
        pDoc->m_Effect.AddElement( pNewElem );
        pElem->SetSelected( FALSE );

        NewElements.AddTail( pNewElem );
    }

    // Set the selection to the new duplicates, instead of the original elements
    pDoc->m_SelSet.RemoveAll();
    g_pMainFrame->m_KeyBar.SetKeySets( 0, NULL );
    Pos = NewElements.GetHeadPosition();

    while( Pos )
    {
        fx_core::element* pElem = NewElements.GetNext( Pos );
        pDoc->m_SelSet.AddTail( pElem );
        pElem->SetSelected( TRUE );
    }

    // Update controls
    UpdateKeyBar();
    pDoc->PopulatePropertyControl();
    GetDocument()->UpdateAllViews( NULL );
}

void CParticleView3D::OnEditMoveToZero()
{
    CPartEdDoc*     pDoc    = (CPartEdDoc*)GetDocument();
    POSITION        Pos     = pDoc->m_SelSet.GetHeadPosition();

    while( Pos )
    {
        fx_core::element*   pElem   = pDoc->m_SelSet.GetNext( Pos );
        vector3             Offset;
        pElem->GetPositionAtTime( g_pMainFrame->GetGlobalTime(), Offset );
        pElem->Translate( -Offset );
    }

    // update Property View & Viewports
    pDoc->UpdateAllViews(NULL);
    pDoc->PopulatePropertyControl();
}

void CParticleView3D::OnViewRedrawAll()
{
    CPartEdDoc*     pDoc        =   (CPartEdDoc*)GetDocument();    

    // update Property View & Viewports
    pDoc->UpdateAllViews(NULL);
    pDoc->PopulatePropertyControl();
}

void CParticleView3D::OnViewBbox()
{
    CPartEdDoc*     pDoc        =   (CPartEdDoc*)GetDocument();

    // Toggle state of the RenderBBoxesEnabled flag in the effect
    pDoc->m_Effect.EnableRenderBBoxes( !pDoc->m_Effect.RenderBBoxesEnabled() );

    // Update Viewports
    pDoc->UpdateAllViews(NULL);
}

void CParticleView3D::OnUpdateViewBbox(CCmdUI* pCmdUI) 
{
    CPartEdDoc*     pDoc        =   (CPartEdDoc*)GetDocument();

	pCmdUI->SetCheck( pDoc->m_Effect.RenderBBoxesEnabled() );
}

void CParticleView3D::OnViewTrajectory()
{
    CPartEdDoc*     pDoc    = (CPartEdDoc*)GetDocument();
    POSITION        Pos     = pDoc->m_SelSet.GetHeadPosition();

    while( Pos )
    {
        fx_core::element* pElem  = pDoc->m_SelSet.GetNext( Pos );
        pElem->SetShowTrajectory( !(pElem->GetShowTrajectory()) );
    }

    // update Property View & Viewports
    pDoc->UpdateAllViews(NULL);
    pDoc->PopulatePropertyControl();
}

void CParticleView3D::OnViewVelocity()
{
    CPartEdDoc*     pDoc    = (CPartEdDoc*)GetDocument();
    POSITION        Pos     = pDoc->m_SelSet.GetHeadPosition();

    while( Pos )
    {
        fx_core::element* pElem  = pDoc->m_SelSet.GetNext( Pos );

        if( x_strcmp( pElem->GetType(), "Spemitter" ) == 0 )
        {
            ((fx_core::element_spemitter*)pElem)->SetShowVelocity( !((fx_core::element_spemitter*)pElem)->GetShowVelocity() );
        }
    }

    // update Property View & Viewports
    pDoc->UpdateAllViews(NULL);
    pDoc->PopulatePropertyControl();
}

void CParticleView3D::OnViewHideSelected()
{
    CPartEdDoc*     pDoc    = (CPartEdDoc*)GetDocument();
    POSITION        Pos     = pDoc->m_SelSet.GetHeadPosition();

    while( Pos )
    {
        fx_core::element* pElem  = pDoc->m_SelSet.GetNext( Pos );
        pElem->Hide();
    }

    pDoc->m_SelSet.RemoveAll();
    pDoc->m_Effect.SetAllElementsSelected( FALSE );
    g_pMainFrame->m_KeyBar.SetKeySets( 0, NULL );

    // update Property View & Viewports
    pDoc->PopulatePropertyControl();
    pDoc->UpdateAllViews(NULL);
    UpdateKeyBar();
}

void CParticleView3D::OnViewUnhideAll()
{
    CPartEdDoc*     pDoc            = (CPartEdDoc*)GetDocument();
    s32             NumElements     = pDoc->m_Effect.GetNumElements();

    for( s32 i = 0; i < NumElements; i++ )
    {
        fx_core::element*    pElement    = pDoc->m_Effect.GetElement( i );
        pElement->Show();
    }

    // update Property View & Viewports
    pDoc->UpdateAllViews(NULL);
    pDoc->PopulatePropertyControl();
}
