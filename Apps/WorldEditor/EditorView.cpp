// EditorView.cpp : implementation of the CEditorView class
//

#include "StdAfx.h"

#include "..\Editor\MainFrm.h"
#include "EditorDoc.h"
#include "EditorView.h"
#include "EditorFrame.h"
#include "EditorLayerView.h"
#include "ai_editor.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Objects\Player.hpp"
#include "Objects\LevelSettings.hpp"
#include "WorldEditor.hpp"
#include "transaction_mgr.hpp"
#include "EditorPaletteDoc.h"
#include "nav_connection2_editor.hpp"
#include "Parsing\TextOut.hpp"
#include "..\EDRscDesc\RscDesc.hpp"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\editor\resource.h"
#include "..\Editor\Project.hpp"
#include "..\Support\GameTextMgr\GameTextMgr.hpp"
#include "..\WinControls\StringEntryDlg.h"
#include "..\Support\Render\LightMgr.hpp"
#include "Auxiliary/PCRE/regex.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_RAY_MULT     10000.0f


#define TIMER_FOR_WATCH_UPDATES 0.5


BOOL g_EditorShowNameFlag = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CEditorView

IMPLEMENT_DYNCREATE(CEditorView, CView3D)

BEGIN_MESSAGE_MAP(CEditorView, CView3D)
	//{{AFX_MSG_MAP(CEditorView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
    ON_COMMAND(ID_EDIT_UNDO, OnUndo)
    ON_COMMAND(ID_EDIT_REDO, OnRedo)
	ON_COMMAND(ID_EDIT_CUT,  OnCutObjects)
    ON_COMMAND(ID_EDIT_COPY, OnCopyObjects)  
    ON_COMMAND(ID_EDIT_PASTE,OnPasteObjects)       
    ON_COMMAND(ID_SHOW_REPORT, OnShowReport)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateRedo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateCopyObjects)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT,  OnUpdateCutObjects)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE,OnUpdatePasteObjects)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView3D::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView3D::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView3D::OnFilePrintPreview)
END_MESSAGE_MAP()
 
//=========================================================================
// CEditorView construction/destruction
//=========================================================================

CEditorView::CEditorView() :
m_bShowGrid(TRUE),
m_bShowSpacD(FALSE),
m_bShowAxis(TRUE),
m_pFrameEdit(NULL),
m_enMode(WEV_STANDARD_MODE),
m_bGridSnap(TRUE),
m_bAddWithRaySnap(FALSE),
m_bDraggingObject(FALSE),
m_bDragSelect(FALSE),
m_bDragUnSelect(FALSE),
m_bAllLayers(FALSE),
m_bShowSelectionBounds(FALSE),
m_bObjectMoved(FALSE),
m_bRenderPreview(FALSE),
m_bDoPortalWalk(FALSE),
m_FocusPos(0,0,0),
m_GuidToHighLight(0),
m_bRenderIcons(TRUE)
{
    m_ViewHistory.SetSize( 100 );
    m_Grid.SetTranslations(vector3(0,0,0));
    m_GridBaseline.SetTranslations(vector3(0,0,0));
    m_GridBaseline.SetColor(xcolor(128,128,128,128));
    m_Axis.SetScale( 3 );
    m_MsgTimer.Reset( );
    m_MsgTimer.Stop( );
    m_WatchUpdateTimer.Reset();
    m_WatchUpdateTimer.Stop();
}

//=========================================================================

CEditorView::~CEditorView()
{
    g_AudioMgr.UnloadAllPackages();
    g_AudioMgr.Kill();

    g_GameTextMgr.Kill();
}

//=========================================================================

BOOL CEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CView3D::PreCreateWindow(cs))
		return FALSE;

	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

//=========================================================================
// CEditorView drawing
//=========================================================================

void CEditorView::OnDraw(CDC* pDC)
{
	CEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

//=========================================================================
// CEditorView printing
//=========================================================================

BOOL CEditorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

//=========================================================================

void CEditorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

//=========================================================================

void CEditorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

//=========================================================================
// CEditorView diagnostics
//=========================================================================

#ifdef _DEBUG
void CEditorView::AssertValid() const
{
	CView3D::AssertValid();
}

//=========================================================================

void CEditorView::Dump(CDumpContext& dc) const
{
	CView3D::Dump(dc);
}

//=========================================================================

CEditorDoc* CEditorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEditorDoc)));
	return (CEditorDoc*)m_pDocument;
}
#endif //_DEBUG

//=========================================================================
// CEditorView message handlers
//=========================================================================

void CEditorView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// TODO: Add your message handler code here

	// Do not call CView3D::OnPaint() for painting messages

	d3deng_UpdateDisplayWindow( GetSafeHwnd() );

    if (m_MsgTimer.ReadSec() > 5.0f) //reset after 5 seconds
    {
        m_MsgTimer.Reset();
        m_MsgTimer.Stop();
    }

    CRect rc;
    GetClientRect(&rc);

    if (g_Project.IsProjectOpen())
    {
        Render( );

        if (m_bDragSelect || m_bDragUnSelect)
        {
            //show selection band
            if (m_bAllLayers)
            {
                dc.Draw3dRect(CRect(m_ptDragSelectBegin.x, m_ptDragSelectBegin.y, m_ptDragSelectEnd.x, m_ptDragSelectEnd.y),
                    RGB(255,255,0), RGB(128,128,0));
            }
            else
            {
                dc.Draw3dRect(CRect(m_ptDragSelectBegin.x, m_ptDragSelectBegin.y, m_ptDragSelectEnd.x, m_ptDragSelectEnd.y),
                    RGB(255,255,255), RGB(128,128,128));
            }
        }

        if (m_MsgTimer.IsRunning())
        {
            dc.DrawText(m_strLastMsg, rc, DT_WORDBREAK);
        }
    }
    else
    {
        dc.FillSolidRect(rc,RGB(255,255,255));
    }

//    f32 frames = eng_GetFPS();
//    dc.TextOut(10,10,CString(xfs("fps(%d)",frames)));
}

//=========================================================================

BOOL CEditorView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return TRUE;//CView3D::OnEraseBkgnd(pDC);
}

//=========================================================================

int CEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView3D::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	Initialize();

	return 0;
}

//=========================================================================

void CEditorView::Initialize()
{
    m_View.SetXFOV( R_60 );
    m_View.SetPosition( vector3(2000,1000,2000) );
    m_View.LookAtPoint( vector3(  0,  0,  0) );
	m_View.SetZLimits ( 10, GetDocument()->GetFarZLimit() );

    m_Grid.SetSeparation(float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()));

	CameraFreeFlyMode();
    d3deng_SetAmbientLight(xcolor(255,255,255,255));
}

//=========================================================================

void CEditorView::DoPortalWalk(BOOL bWalkPortals)
{
    m_bDoPortalWalk = bWalkPortals;
    if ( !m_bDoPortalWalk )
        g_ZoneMgr.TurnOff();
}

//=========================================================================

xbool CEditorView::SetupView( const view* pPlayerView, view& PortalView, xbool FPV )
{
    // grab some default z limits
    f32 ZNear, ZFar;
    ZNear = 10.0f;
    ZFar  = GetDocument()->GetFarZLimit();

    // figure out the engine view
    view EngView;
    if ( FPV && pPlayerView )
        EngView = *pPlayerView;
    else
        EngView = m_View;
    EngView.SetZLimits( ZNear, ZFar );

    // are we doing portal culling?
    xbool   bDoPortalWalk = pPlayerView && GetDocument()->IsGameRunning() && m_bDoPortalWalk;
    if ( bDoPortalWalk )
    {
        // get the z-limits based on the level settings
        slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_LEVEL_SETTINGS );
        if ( SlotID != SLOT_NULL )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            ASSERT( pObject );
            level_settings& Settings = level_settings::GetSafeType( *pObject );
            ZNear = 10.0f;
            ZFar  = Settings.GetFarPlane();
        }
        else
        {
            ZNear = 10.0f;
            ZFar  = 8000.0f;
        }
    }

    // the portal view should always be set up to the player's view
    if ( bDoPortalWalk )
        PortalView = *pPlayerView;
    else
        PortalView = EngView;

    // set in the near and far planes
    PortalView.SetZLimits( ZNear, ZFar );
    if ( FPV && bDoPortalWalk )
        EngView.SetZLimits( ZNear, ZFar );

    // activate the view
    eng_SetView( EngView );
    eng_SetViewport( EngView );

    if( g_pd3dDevice )
    {
        g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&(EngView.GetW2V() ));
        g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&(EngView.GetV2C() ));
    }

    return bDoPortalWalk;
}

//=========================================================================

void CEditorView::GetViewRatio( xbool bFirstPerson, s32& XRes, s32& YRes, f32& PixelScale )
{
    // Use exact PS2 screen?
    if ( m_bRenderPreview )
    {
        // PS2 screen
        XRes       = PS2_VIEWPORT_WIDTH;
        YRes       = PS2_VIEWPORT_HEIGHT;
        PixelScale = PS2_PIXEL_SCALE;
    }
    // Always use PS2 ratio in first person so cinema view is the same
    else if( bFirstPerson )
    {
        // Start with full window size
        eng_GetRes( XRes, YRes );
        f32 W     = (f32)XRes;
        f32 H     = (f32)YRes;
        f32 Ratio = (f32)PS2_VIEWPORT_HEIGHT / (f32)PS2_VIEWPORT_WIDTH;
        
        // Scale to match PS2 ratio        
        H = W * Ratio;

        // Keep the view on the screen vertically
        if( H > YRes )
        {
            W *= YRes / H;
            H = (f32)YRes;
        }            
        
        // Setup outputs
        XRes       = (s32)W;
        YRes       = (s32)H;
        PixelScale = PS2_PIXEL_SCALE;
    }
    else
    {
        // Use full screen
        eng_GetRes( XRes, YRes );
        PixelScale = 1.0f;
    }
}

//=========================================================================

void CEditorView::RenderFPV()
{
    // get pointers to each of the players, we'll need them for setting up the
    // viewports
    s32     nPlayers = 0;
    player* pPlayers[MAX_LOCAL_PLAYERS];
    slot_id ID = g_ObjMgr.GetFirst(object::TYPE_PLAYER);
    while ( (ID != SLOT_NULL) && (nPlayers < MAX_LOCAL_PLAYERS) )
    {
        object* pObj         = g_ObjMgr.GetObjectBySlot(ID);
        pPlayers[nPlayers++] = &player::GetSafeType( *pObj );
        ID                   = g_ObjMgr.GetNext(ID);
    }

    // Setup view 
    s32 XRes, YRes;   
    f32 PixelScale;
    GetViewRatio( TRUE, XRes, YRes, PixelScale );
            
    // SB: Not sure what this is for - you can't have more than 1 player in the editor?!
    switch ( nPlayers )
    {
    default:
    case 0:
        break;
    
    case 1:
        {
            // one view, set it to the entire screen
            view& rView0 = player::GetView( pPlayers[0]->GetLocalSlot() );
            rView0.SetViewport( 0, 0, XRes, YRes );
            rView0.SetPixelScale( PixelScale );
            pPlayers[0]->ComputeView( rView0 );
        }
        break;

    case 2:
    case 3:
        {
            // two views, set them to a horizontal split
            view& rView0 = player::GetView( pPlayers[0]->GetLocalSlot() );
            view& rView1 = player::GetView( pPlayers[1]->GetLocalSlot() );
            rView0.SetViewport( 0, 0,      XRes, YRes/2 );      // top
            rView1.SetViewport( 0, YRes/2, XRes, YRes   );      // bottom
            rView0.SetPixelScale( PixelScale );
            rView1.SetPixelScale( PixelScale );
            pPlayers[0]->ComputeView( rView0 );
            pPlayers[1]->ComputeView( rView1 );
        }
        break;

    case 4:
        {
                // four views, set them to a 4-way split
                view& rView0 = player::GetView( pPlayers[0]->GetLocalSlot() );
                view& rView1 = player::GetView( pPlayers[1]->GetLocalSlot() );
                view& rView2 = player::GetView( pPlayers[2]->GetLocalSlot() );
                view& rView3 = player::GetView( pPlayers[3]->GetLocalSlot() ); 
                rView0.SetViewport( 0,      0,      XRes/2, YRes/2 );   // upper-left
                rView1.SetViewport( XRes/2, 0,      XRes,   YRes/2 );   // upper-right
                rView2.SetViewport( 0,      YRes/2, XRes/2, YRes   );   // lower-left
                rView3.SetViewport( XRes/2, YRes/2, XRes,   YRes   );   // lower-right
                rView0.SetPixelScale( PixelScale );
                rView1.SetPixelScale( PixelScale );
                rView2.SetPixelScale( PixelScale );
                rView3.SetPixelScale( PixelScale );
                pPlayers[0]->ComputeView( rView0 );
                pPlayers[1]->ComputeView( rView1 );
                pPlayers[2]->ComputeView( rView2 );
                pPlayers[3]->ComputeView( rView3 );
        }
        break;
    }

    for ( s32 iPlayer = 0; iPlayer < nPlayers; iPlayer++ )
    {
        // set this player as the active one
        for ( s32 iActive = 0; iActive < nPlayers; iActive++ )
            pPlayers[iActive]->SetAsActivePlayer((iPlayer==iActive) ? TRUE : FALSE);

        // set up the view
        view PortalView;
        xbool bDoPortalWalk = SetupView(&pPlayers[iPlayer]->GetView(), PortalView, TRUE);

        // render all objects
        g_WorldEditor.RenderObjects( bDoPortalWalk,
                                     PortalView,
                                     bDoPortalWalk ? pPlayers[iPlayer]->GetPlayerViewZone() : 0 );
    }
}

//=========================================================================

void CEditorView::RenderNormal()
{
    if (!GetDocument())
        return;

    // Setup view 
    s32 XRes, YRes;   
    f32 PixelScale;
    GetViewRatio( FALSE, XRes, YRes, PixelScale );
    m_View.SetViewport( 0, 0, XRes, YRes );
    m_View.SetPixelScale( PixelScale );

    //
    // Render the world 
    //
    view    PortalView;
    player* pPlayer       = SMP_UTIL_GetActivePlayer();
    xbool   bDoPortalWalk = SetupView(pPlayer?&pPlayer->GetView() : NULL, PortalView, FALSE);
    if (m_bShowSpacD) g_WorldEditor.RenderSpacialDBase();
    g_WorldEditor.RenderObjects( bDoPortalWalk,
                                 PortalView,
                                 bDoPortalWalk ? pPlayer->GetPlayerViewZone() : 0 );

    //
    // Render the icons world 
    //
    if( m_bRenderIcons )
    {
        g_ObjMgr.RenderIcons();
    }

    if (m_bShowSelectionBounds) g_WorldEditor.RenderSelectedObjectCollision();

    ai_editor::GetAIEditor()->Render();

    m_Grid.SetTranslations(GetDocument()->GetGridPos());
    m_GridBaseline.SetTranslations(GetDocument()->GetGridPos());

    if( eng_Begin("WorldEditor") )
    {
        //
        // Show Focus
        //
        if (m_bShowAxis) 
        {
            m_Axis.SetPosition( m_FocusPos );
            m_Axis.Render();

            //
            // Set the marker to indicate the center of the world
            //
            draw_Marker(vector3(0,0,0), xcolor(255,0,0,255) );
        }

        if (m_bShowGrid) 
        {
            //render secondary grid
            m_Grid.SetSeparation(float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()));
            m_Grid.SetSize(float(GetDocument()->GetGridSnap()*100),float(GetDocument()->GetGridSnap()*100));
            m_Grid.Render();            
            
            if (GetDocument()->IsBaselineGridVisible() && GetDocument()->GetGridSnap() != 100)
            {
                m_GridBaseline.Render();
            }
        }

        if (m_HighlightVolume.GetRadius() > 1)
        {
            //volume exists, so render
            draw_Volume(m_HighlightVolume, m_HighlightColor);
            xcolor FrameColor = m_HighlightColor;
            FrameColor.A = 255;
            draw_BBox(m_HighlightVolume,FrameColor);
        }

        guid GuidToSelect = 0;
        if (g_WorldEditor.IsInHoverSelectMode())
        {
            //must draw the selection if there is one
            GuidToSelect = g_WorldEditor.GetHoverSelectionGuid();
            if (GuidToSelect.Guid != 0)
            {
                if (g_WorldEditor.IsHoverSelectionABlueprint())
                {
                    bbox BBoxGuid = g_WorldEditor.GetObjectsBoundingBox(GuidToSelect);
                    if( BBoxGuid.Min != BBoxGuid.Max )
                    {
                        xcolor SelectColor = xcolor(70,218,225,100);
                        draw_Volume(BBoxGuid, SelectColor);
                        SelectColor.A = 255;
                        draw_BBox(BBoxGuid,SelectColor);
                    }
                }
                else
                {
                    bbox BBoxGuid = g_WorldEditor.GetObjectsBoundingBox(GuidToSelect);
                    if( BBoxGuid.Min != BBoxGuid.Max )
                    {
                        xcolor SelectColor = xcolor(189,230,66,100);
                        draw_Volume(BBoxGuid, SelectColor);
                        SelectColor.A = 255;
                        draw_BBox(BBoxGuid,SelectColor);
                    }
                }
            }
        }

        //highlight selected guid
        if (m_GuidToHighLight && (GuidToSelect != m_GuidToHighLight))
        {
            bbox BBoxGuid = g_WorldEditor.GetObjectsBoundingBox(m_GuidToHighLight);
            if( BBoxGuid.Min != BBoxGuid.Max )
            {
                xcolor SelectColor = xcolor(128,0,128,100);
                draw_Volume(BBoxGuid, SelectColor);
                SelectColor.A = 255;
                draw_BBox(BBoxGuid,SelectColor);
            }
        }

        //render lookup info
        xarray<guid>& GuidList = g_WorldEditor.GetGuidLookupList();
        if (GuidList.GetCount() > 0)
        {
            guid LookupGuid = g_WorldEditor.GetLookupGuid();
            if (LookupGuid != 0)
            {
                //this is a guid lookup
                bbox BBoxGuid = g_WorldEditor.GetObjectsBoundingBox(LookupGuid);
                if( BBoxGuid.Min != BBoxGuid.Max )
                {
                    BBoxGuid.Inflate(20,20,20);
                    xcolor SelectColor(230,255,255,100);
                    draw_Volume(BBoxGuid, SelectColor);
                    SelectColor.A = 255;
                    draw_BBox(BBoxGuid,SelectColor);
                }

                for (s32 i=0; i < GuidList.GetCount(); i++)
                {
                    guid AttachGuid = GuidList.GetAt(i);
                
                    bbox BBoxAttach = g_WorldEditor.GetObjectsBoundingBox(AttachGuid);
                    if( (BBoxGuid.Min != BBoxGuid.Max) && (BBoxAttach.Min != BBoxAttach.Max) )
                    {
                        BBoxAttach.Inflate(20,20,20);

                        xcolor SelectColor(69,231,231,100);
                        draw_Volume(BBoxAttach, SelectColor);
                        SelectColor.A = 255;
                        draw_BBox(BBoxAttach,SelectColor);

                        draw_Line( BBoxGuid.GetCenter(), BBoxAttach.GetCenter(), SelectColor );
                    }
                }
            }
            else
            {
                //this is a global guid lookup
                for (s32 i=0; i < GuidList.GetCount(); i++)
                {
                    guid AttachGuid = GuidList.GetAt(i);
                
                    bbox BBoxAttach = g_WorldEditor.GetObjectsBoundingBox(AttachGuid);
                    if( BBoxAttach.Min != BBoxAttach.Max )
                    {
                        BBoxAttach.Inflate(20,20,20);

                        xcolor SelectColor = xcolor(166,255,210,100);
                        draw_Volume(BBoxAttach, SelectColor);
                        SelectColor.A = 255;
                        draw_BBox(BBoxAttach,SelectColor);
                    }
                }
            }
        }

        if (GetDocument()->IsSchematicLoaded() && GetDocument()->DrawSchematic())
        {
            draw_Begin( DRAW_QUADS, DRAW_TEXTURED | DRAW_USE_ALPHA );
            draw_SetTexture( GetDocument()->GetSchematic() );
        
            draw_Color( xcolor( 255,255,255,GetDocument()->GetSchematicAlpha() ) );

            s32 nScale = GetDocument()->GetSchematicScale();

            draw_UV    ( 0,1 );     draw_Vertex( vector3( -float(nScale*100), GetDocument()->GetGridPos().GetY()-1, float(nScale*100) ) );
            draw_UV    ( 1,1 );     draw_Vertex( vector3( float(nScale*100), GetDocument()->GetGridPos().GetY()-1, float(nScale*100)  ) );
            draw_UV    ( 1,0 );     draw_Vertex( vector3( float(nScale*100), GetDocument()->GetGridPos().GetY()-1, -float(nScale*100) ) );
            draw_UV    ( 0,0 );     draw_Vertex( vector3( -float(nScale*100), GetDocument()->GetGridPos().GetY()-1, -float(nScale*100) ) );

            draw_End();
        }

        eng_End();
    }
}

//=========================================================================

void CEditorView::Render( )
{
    x_try;

    eng_SetBackColor( GetDocument()->GetBackgroundColor());

    // force the player's view to update
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if( pPlayer != NULL )
    {
        pPlayer->ComputeView( pPlayer->GetView() );
    }   

    // if we're in first-person mode, we handle the view rendering differently
    if ( GetDocument()->IsFPV() )
    {
        RenderFPV();
    }
    else
    {
        RenderNormal();
    }

    //show stats
    if( eng_Begin("Stats") )
    {
        if (GetDocument()->IsGameRunning() && GetDocument()->ShowEngineStats())
        {
            eng_PrintStats();
        }
        eng_End();
    }

    #if ENABLE_RENDER_STATS
    render::GetStats().Print( render::stats::OUTPUT_TO_SCREEN );
    #endif

    eng_PageFlip();

    x_catch_begin;

    // Reset render operations
    draw_ResetAfterException();
    eng_ResetAfterException();
    render::ResetAfterException();
    g_LightMgr.ResetAfterException();

    x_display_exception_msg( "CEditorView::Render exception caught, attempting to cleanup!" );

    x_catch_end;
}

//=========================================================================

void CEditorView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CView3D::OnLButtonDown(nFlags, point);

    m_HighlightVolume.Clear();

    if (g_WorldEditor.IsInHoverSelectMode())
    {
        //update the hover select property
        g_WorldEditor.DoGuidSelect();

        //now clear the hover select stuff
        g_WorldEditor.ClearHoverSelection();
        g_WorldEditor.SetGuidSelectMode("",FALSE);
        return;
    }

    if( IsActionMode() )
        return;

    if (IsStandardMode())
    {
        if (( GetKeyState( VK_CONTROL ) & ~1 ) == 0)
        {
            //control not pressed, so empty list
            g_WorldEditor.ClearSelectedObjectList();
        }

        if (( GetKeyState( 'B' ) & ~1 ) != 0) //B selected
        {
            m_bDragSelect   = TRUE;
            m_bAllLayers    = (( GetKeyState( VK_LSHIFT ) & ~1 ) != 0);
            m_ptDragSelectBegin = point;
            m_ptDragSelectEnd   = point;
        }
        else
        {
            vector3 P0;
            vector3 P1;
            CastRayFrom2D( point.x, point.y, P0, P1 );

            xtimer LoadTimer;
            LoadTimer.Start();

            guid ObjGuid = g_WorldEditor.SelectObjectWithRay(P0, P1, m_bRenderIcons);

            f32 TimeSelect = LoadTimer.TripSec();
            //CString strItemSearch;

            CEditorLayerView* pView = (CEditorLayerView*)GetFrame()->FindViewFromTab( GetFrame()->m_wndWrkspBar,RUNTIME_CLASS(CEditorLayerView));
            if (pView)
            {
                //select object in layer view
                guid BPGuid = g_WorldEditor.GetBlueprintGuidContainingObject(ObjGuid);
                if (BPGuid.Guid == 0)
                {
                    //not in a blueprint
                    pView->SelectObject(ObjGuid);
                    //strItemSearch.Format("SelectObjectInLayerTree(%s)[%g sec]",guid_ToString(ObjGuid),LoadTimer.TripSec());
                }
                else
                {
                    //in a blueprint
                    pView->SelectBlueprint(BPGuid);
                    //strItemSearch.Format("SelectBlueprintInLayerTree(%s)[%g sec]",guid_ToString(BPGuid),LoadTimer.TripSec());
                }
            }
/*
            if(ai_editor::GetAIEditor()->IsInConnectionMode() )
            {
                ai_editor::GetAIEditor()->ObjectSelected( ObjGuid );
            }
            else                
            {
                ai_editor::GetAIEditor()->ObjectSelected( 0 );
            }
*/
            //
            // TODO: We must formalize the focus pos.
            //
            if( g_WorldEditor.GetSelectedCount() )
            {
                m_FocusPos = g_WorldEditor.GetMinPositionForSelected();
                m_FocusPos.GetY() = GetDocument()->GetGridPos().GetY();
                //don't want snap since we want to lock to item pos
                //m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );
            }
        }
    }
	else if (IsMovementMode() || IsPlacementMode() || IsBlueprintMode())
    {
        //go into drag mode
        m_bDraggingObject = TRUE;
    }
    else if (IsDecalPlacementMode())
    {
        // plop down a decal at the appropriate point
        g_WorldEditor.StartNewDecal();
        OnMoveObjects( point );

        //go into drag mode
        m_bDraggingObject = TRUE;
    }

//    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
	SetViewDirty();
}

//=========================================================================

void CEditorView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if (m_bDragSelect || m_bDragUnSelect)
    {
        m_ptDragSelectEnd   = point;

        //do band selection
        vector3 Endpoint[4];

        // get the 4 ends
        Endpoint[0] = Get3DPointFrom2D( MIN(m_ptDragSelectBegin.x,m_ptDragSelectEnd.x), 
                                        MIN(m_ptDragSelectBegin.y,m_ptDragSelectEnd.y) );
        Endpoint[1] = Get3DPointFrom2D( MAX(m_ptDragSelectBegin.x,m_ptDragSelectEnd.x), 
                                        MIN(m_ptDragSelectBegin.y,m_ptDragSelectEnd.y) );
        Endpoint[2] = Get3DPointFrom2D( MAX(m_ptDragSelectBegin.x,m_ptDragSelectEnd.x), 
                                        MAX(m_ptDragSelectBegin.y,m_ptDragSelectEnd.y) );
        Endpoint[3] = Get3DPointFrom2D( MIN(m_ptDragSelectBegin.x,m_ptDragSelectEnd.x), 
                                        MAX(m_ptDragSelectBegin.y,m_ptDragSelectEnd.y) );
        // get the camera position
        vector3 Viewpoint = m_View.GetPosition();

        // now setup the frustum
        m_plDragSelect[0].Setup( Endpoint[3], Endpoint[0], Viewpoint   );       // Left
        m_plDragSelect[1].Setup( Endpoint[0], Endpoint[1], Viewpoint   );       // Top   
        m_plDragSelect[2].Setup( Endpoint[2], Viewpoint,   Endpoint[1] );       // Right
        m_plDragSelect[3].Setup( Endpoint[3], Viewpoint,   Endpoint[2] );       // Bottom
    
        m_plDragSelect[4].Setup( Viewpoint, m_View.GetViewZ() ) ;               // Near

        if (m_bAllLayers)
        {
            xarray<xstring> ListLayers;
            xarray<guid> ObjList;
            xarray<editor_blueprint_ref> BPList;
            g_WorldEditor.GetLayerNames ( ListLayers );
            for (int j=0; j < ListLayers.GetCount(); j++)
            {
                if (g_WorldEditor.GetObjectsInLayer( ListLayers.GetAt(j), ObjList ))
                {
                    for (int i=0; i<ObjList.GetCount(); i++)
                    {
                        guid& ObjGuid = ObjList.GetAt(i);
                        object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid) ;
                        if( pObject && (pObject->IsSelectable() == TRUE) && (pObject->IsHidden()==FALSE))
                        {
                            bbox BBox = g_WorldEditor.GetObjectsBoundingBox(ObjGuid);
                            if( BBox.Min != BBox.Max )
                            {
                                if( BBoxInDragBand( BBox ) )
                                {
                                    g_WorldEditor.SelectObject(ObjGuid, FALSE);
                                    // Tell object it got selected from a "band select" command.
                                    // eg. the path object selects individual keys
                                    pObject->GetTypeDesc().OnEditorBandSelected(*pObject, m_plDragSelect) ;
                                }
                            }
                        }
                    }
                }

                if (g_WorldEditor.GetBlueprintsInLayer( ListLayers.GetAt(j), BPList ))
                {
                    for (int i=0; i<BPList.GetCount(); i++)
                    {
                        editor_blueprint_ref& BPRef = BPList.GetAt(i);
                        if (BBoxInDragBand(g_WorldEditor.GetBlueprintsBoundingBox(BPRef)))
                        {
                            g_WorldEditor.SelectBlueprintObjects(BPRef, TRUE);
                        }
                    }
                }
            }
        }
        else
        {
            xarray<guid> ObjList;
            xarray<editor_blueprint_ref> BPList;

            //now check all objects in the active layer
            if (g_WorldEditor.GetObjectsInLayer( g_WorldEditor.GetActiveLayer(), ObjList ))
            {
                for (int i=0; i<ObjList.GetCount(); i++)
                {
                    guid& ObjGuid = ObjList.GetAt(i);
                    object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid) ;
                    if( pObject && (pObject->IsSelectable() == TRUE) && (pObject->IsHidden()==FALSE))
                    {
                        bbox BBox = g_WorldEditor.GetObjectsBoundingBox(ObjGuid);
                        if( BBox.Min != BBox.Max )
                        {
                            if( BBoxInDragBand( BBox ) )
                            {
                                g_WorldEditor.SelectObject(ObjGuid, FALSE);

                                // Tell object it got selected from a "band select" command.
                                // eg. the path object selects individual keys
                                pObject->GetTypeDesc().OnEditorBandSelected(*pObject, m_plDragSelect) ;
                            }
                        }
                    }
                }
            }

            if (g_WorldEditor.GetBlueprintsInLayer( g_WorldEditor.GetActiveLayer(), BPList ))
            {
                for (int i=0; i<BPList.GetCount(); i++)
                {
                    editor_blueprint_ref& BPRef = BPList.GetAt(i);
                    if (BBoxInDragBand(g_WorldEditor.GetBlueprintsBoundingBox(BPRef)))
                    {
                        g_WorldEditor.SelectBlueprintObjects(BPRef, TRUE);
                    }
                }
            }
        }

        m_bDragUnSelect     = FALSE;
        m_bDragSelect       = FALSE;
    }

    SetViewDirty();
	m_bDraggingObject = FALSE;
    m_pFrameEdit->GetPropertyEditorDoc()->Refresh(); 

	CView3D::OnLButtonUp(nFlags, point);
}

//=========================================================================

void CEditorView::CastRayFrom2D( s32 X, s32 Y, vector3& Start, vector3& End )
{
    vector3 Ray = m_View.RayFromScreen( (f32)X, (f32)Y, view::VIEW );
    
    Ray.Normalize();
    Start.Set(0.0f,0.0f,0.0f);
    End = Ray * MAX_RAY_MULT;

    // move them into world space
    Start = m_View.ConvertV2W( Start );
    End   = m_View.ConvertV2W( End );
}

//==============================================================================
// Get a 3D point given a 2D screen position
vector3 CEditorView::Get3DPointFrom2D( s32 X, s32 Y )
{   
    plane   Plane( vector3( 0,1,0 ), -(GetDocument()->GetGridPos().GetY()) );
    vector3 P0;
    vector3 P1;
    f32     t;

    CastRayFrom2D( X, Y, P0, P1 );
    Plane.Intersect( t, P0, P1 );
    return (P0 + t*( P1 - P0 ));
}

//=========================================================================

bool CEditorView::BBoxInDragBand( const bbox& BBox )
{
    // Loop through planes looking for a trivial reject.
    s32 InFrontOf = 0;

    for( s32 i=0; i<5; i++ )
    {
        // Compute max dist along normal
        if ( m_plDragSelect[i].InFront( vector3(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ()) ) ||
             m_plDragSelect[i].InFront( vector3(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Min.GetZ()) ) ||
             m_plDragSelect[i].InFront( vector3(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Min.GetZ()) ) ||
             m_plDragSelect[i].InFront( vector3(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Min.GetZ()) ) ||
             m_plDragSelect[i].InFront( vector3(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Max.GetZ()) ) ||
             m_plDragSelect[i].InFront( vector3(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Max.GetZ()) ) ||
             m_plDragSelect[i].InFront( vector3(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ()) ) ||
             m_plDragSelect[i].InFront( vector3(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Max.GetZ()) ) )
        {
            InFrontOf++;
        }
    }

    if ( InFrontOf == 5 )
        return true;
    else
        return false;
} 

//=========================================================================

void CEditorView::OnActivateFrame( UINT nState, CFrameWnd* pFrameWnd )
{
	if (!m_pFrameEdit)
    {
	    m_pFrameEdit = (CEditorFrame*) pFrameWnd;

        if (m_pFrameEdit)
        {
            m_pFrameEdit->m_pWorldEditView = this;
            CPropertyEditorDoc* pDoc = m_pFrameEdit->GetPropertyEditorDoc();
            if (pDoc)
            {
                pDoc->SetInterface(g_WorldEditor);
            }
        }
    }
	CView3D::OnActivateFrame( nState, pFrameWnd );
}

//=========================================================================
#define VK_BRL        0xDB
#define VK_BRR        0xDD

void CEditorView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    CRect rc;
    GetClientRect(&rc);
    CPoint pt;
    GetCursorPos(&pt);
    ScreenToClient(&pt);

	switch(nChar)
	{
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            {
                s32 iVal = nChar - 49; //49 is ascii val for 1
                GetDocument()->SetGridPreset(iVal);
                m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
                SetViewDirty();            
            }
            break;
        case '6':
        case '7':
        case '8':
        case '9':
            {
                s32 iVal = nChar - 54; //ascii val for 6
                GetDocument()->SetRotatePreset(iVal);
                m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
                SetViewDirty();            
            }
            break;
        case '0':
            GetDocument()->SetRotatePreset(4);
            m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
            SetViewDirty();
            break;
        case VK_DIVIDE:
            GetDocument()->DecrementGridSnap();
            m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
            SetViewDirty();
            break;
        case VK_MULTIPLY:
            GetDocument()->IncrementGridSnap();
            m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
            SetViewDirty();
            break;
        case VK_SUBTRACT:
            GetDocument()->DecrementRotateSnap();
            m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
            SetViewDirty();
            break;
        case VK_ADD:
            GetDocument()->IncrementRotateSnap();
            m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
            SetViewDirty();
            break;
        case 'M':
            MoveGridToSelectedObjects();
            break;        
        case 'Q':
            if (( GetKeyState( VK_CONTROL ) & ~1 ) != 0)
            {
                g_EditorShowNameFlag ^= 1;
                SetViewDirty();
            }
        break;
      
        case 'I':
            if (( GetKeyState( VK_CONTROL ) & ~1 ) != 0)
            {
                g_WorldEditor.ZoneSanityCheck();
                break;
            }
        case 'V':
            //this is for setting the cursor position
            if (rc.PtInRect(pt))
            {
                plane   Plane( vector3( 0,1,0 ), -(GetDocument()->GetGridPos().GetY()) );
                vector3 P0, P1;
                f32     t;

                CastRayFrom2D( pt.x, pt.y, P0, P1 );

                if( Plane.Intersect( t, P0, P1 ) == TRUE )
                {
                    m_FocusPos = P0 + t*( P1 - P0 );

                    if (m_bGridSnap)
                        m_FocusPos.Snap( float(GetDocument()->GetGridSnap()), float(GetDocument()->GetGridSnap()), float(GetDocument()->GetGridSnap()) );
                }

                SetViewDirty();
            }
            break;
        case 'T':
            if (( GetKeyState( VK_CONTROL ) & ~1 ) != 0)
            {
                //ctrl-T is pressed
                if (IsStandardMode())
                {
                    if (g_WorldEditor.CaptureGuid())
                    {
                        CString strMsg;
                        guid CapGuid = g_WorldEditor.GetCapturedGuid();
                        strMsg.Format("Captured Guid %s",(const char*)guid_ToString(CapGuid));
                        SetMessage(strMsg);

                        //copy guid to clipboard
                        if (::OpenClipboard(m_hWnd))
                        {
                            ::EmptyClipboard();
                            CString strText(guid_ToString(CapGuid));
 		                    HGLOBAL hGlobalBuff = ::GlobalAlloc(GMEM_MOVEABLE, strText.GetLength()+1);

		                    TCHAR* szBuffer = (TCHAR*)::GlobalLock(hGlobalBuff);

		                    _tcscpy(szBuffer, strText);
		                    ::GlobalUnlock(hGlobalBuff);

                            if (::SetClipboardData(CF_TEXT, hGlobalBuff) == NULL)
                            {
                                ASSERT(FALSE);
                            }

                            ::CloseClipboard();                           
                        }
                        SetViewDirty();
                    }
                }
            }
            break;
        case 'G':
            if (( GetKeyState( VK_CONTROL ) & ~1 ) != 0)
            {
                //ctrl-G
                g_WorldEditor.PrintSelectedRigidInstanceInfo();
            }
            else
            {
                FocusCamera();
            }
            break;
        case 'O':
            {
                if (g_WorldEditor.SetCurrentObjectsLayerAsActive())
                {
                    CString strMsg;
                    strMsg.Format("New Active Layer \"%s%s\"",
                        g_WorldEditor.GetActiveLayer(),
                        g_WorldEditor.GetActiveLayerPath());
                    SetMessage(strMsg);
                    m_pFrameEdit->OnLoadLayers();
                    SetViewDirty();
                }
            }
            break;
        case 'L':
            {
                GetFrame()->OnWetbMoveObjectsToActiveLayer();
            }
            break;            
        
        case 'J':
        
            // Only do if game is not running
            if (!GetDocument()->IsGameRunning())
            {
                // Clear current test path?
                if (( GetKeyState( VK_CONTROL ) & ~1 ) != 0)
                {
                    ai_editor::GetAIEditor()->ClearTestPath();
                }
                else
                {        
                    // Setup mouse ray cast
                    vector3 Ray = m_View.RayFromScreen( (f32)pt.x, (f32)pt.y, view::VIEW );
                    Ray.Normalize();

                    // Compute local space ray
                    vector3 Start( 0.0f, 0.0f, 0.0f );
                    vector3 End = Ray * MAX_RAY_MULT;

                    // Convert to world space
                    Start = m_View.ConvertV2W( Start );
                    End   = m_View.ConvertV2W( End );

                    // Setup path and refresh
                    ai_editor::GetAIEditor()->ComputeTestPath( Start, End );
                }
                
                // Update view                    
                SetViewDirty();
            }            
            break;

        case 'U':
            {
                g_WorldEditor.UnHideObjects();
            }
            break;
        case 'H':
            {
                g_WorldEditor.HideUnselectedObjects();
            }
            break;
        case VK_BRL:
            {
                //handle grid snap
                s32 Snap = GetDocument()->GetGridSnap();
                if (Snap <= 10)
                    Snap = 1;
                else if (Snap <= 50)
                    Snap = 10;
                else if (Snap <= 100)
                    Snap = 50;
                else
                    Snap = 100;

                GetDocument()->SetGridSnap(Snap);
                m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
                SetViewDirty();
            }
            break;  
        case VK_BRR:
            {
                //handle grid snap
                s32 Snap = GetDocument()->GetGridSnap();
                if (Snap >= 50)
                    Snap = 100;
                else if (Snap >= 10)
                    Snap = 50;
                else if (Snap >= 1)
                    Snap = 10;
                else
                    Snap = 1;

                GetDocument()->SetGridSnap(Snap);
                m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
                SetViewDirty();
            }
            break;             
        case VK_F2:
            {
                //Guid Lookup
                CStringEntryDlg dlg;
                dlg.SetDisplayText( "Enter: GUID (00000000:00000000) | location ( xxx, yyy, zzz ) | name \"name\"" );
                dlg.SetEntryText( "" );
                if (dlg.DoModal() == IDOK)
                {
                    CString Str = dlg.GetEntryText();

                    // Is this a guid?
                    regex r1( "[0-9a-fA-F]{8}:?[0-9a-fA-F]{8}" );
                    if( r1.Match( Str ) )
                    {
                        xstring GuidString = r1.GetSubstring(0);
                        guid GuidToFind = guid_FromString( GuidString );

                        if (g_WorldEditor.SelectObject(GuidToFind))
                        {
                            // Move the camera focus
                            vector3 Pos = g_WorldEditor.GetMinPositionForSelected();
                            SetFocusPos( Pos );
                            FocusCameraWithUndo( Pos );
                            m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                            SetViewDirty();
                        }
                        else
                        {
                            ::AfxMessageBox("Guid Not Found.");
                        }

                        break;
                    }

                    // Is this a location?
                    regex r2( "(-?\\d*\\.?\\d+)\\s*,\\s*(-?\\d*\\.?\\d+)\\s*,\\s*(-?\\d*\\.?\\d+)" );
                    if( r2.Match( Str ) )
                    {
                        xstring x = r2.GetSubstring( 1 );
                        xstring y = r2.GetSubstring( 2 );
                        xstring z = r2.GetSubstring( 3 );

                        // Move the camera focus
                        vector3 Pos = vector3( x_atof(x), x_atof(y), x_atof(z) );
                        SetFocusPos( Pos );
                        FocusCameraWithUndo( Pos );
                        m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                        SetViewDirty();
                        break;
                    }

                    // Find it by name
                    object* pObject = g_ObjMgr.GetObjectByName( Str );
                    if( pObject )
                    {
                        if( g_WorldEditor.SelectObject( pObject->GetGuid() ) )
                        {
                            // Move the camera focus
                            vector3 Pos = g_WorldEditor.GetMinPositionForSelected();
                            SetFocusPos( Pos );
                            FocusCameraWithUndo( Pos );
                            m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                            SetViewDirty();
                        }
                    }
                }
            }
            break;
        case VK_F3:
            {
                //last selected
                g_WorldEditor.SelectLastSelected();
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                SetViewDirty();
            }            
        case VK_F12:
            if ((( GetKeyState( VK_CONTROL ) & ~1 ) != 0) &&
                (( GetKeyState( VK_SHIFT   ) & ~1 ) != 0))
            {
                //ctrl-shift-f12 allows for selecting of objects even in read only mode
                g_WorldEditor.m_bIgnoreReadOnlyChecksForDebug = !g_WorldEditor.m_bIgnoreReadOnlyChecksForDebug;
            }
            break;
        case VK_BACK:
            {
                if (!GetDocument()->IsGameRunning())
                {
                    if (IsPlacementMode())
                    {   
                        CancelPlacementMode();
                    }
                    else if (IsBlueprintMode())
                    {   
                        CancelBlueprintMode();
                    }
                    else if (IsDecalPlacementMode())
                    {
                        CancelDecalPlacementMode();
                    }
                    else if (g_WorldEditor.GetSelectedCount()>0)
                    {
                        if (IsMovementMode())
                        {
                            //for UNDO purposes, must cancel movement mode before deleting objects
                            CancelMovementMode();
                        }

                        if (m_pFrameEdit) m_pFrameEdit->OnWetbDeleteSelected();

                        if (m_pFrameEdit && m_pFrameEdit->GetPropertyEditorDoc()) 
                            m_pFrameEdit->GetPropertyEditorDoc()->ClearGrid();
                        m_enMode = WEV_STANDARD_MODE;
                        SetViewDirty();
                    }
                }
			}	
			break;

        case VK_PRIOR:
            if (m_bGridSnap)
            {
                s32 iNewLocation = ((s32)(GetDocument()->GetGridPos().GetY()) % GetDocument()->GetGridSnap()) + GetDocument()->GetGridSnap();
                
                vector3 GridPos = GetDocument()->GetGridPos();
                GridPos.GetY() += iNewLocation;
                GetDocument()->SetGridPos(GridPos);
                
                m_FocusPos.GetY() = GridPos.GetY();
                m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );               
            }
            else
            {
                vector3 GridPos = GetDocument()->GetGridPos();
                GridPos.GetY() += 1.0f;
                GetDocument()->SetGridPos(GridPos);
            }
            m_Grid.SetTranslations(GetDocument()->GetGridPos());
            m_GridBaseline.SetTranslations(GetDocument()->GetGridPos());
            if (IsMovementMode()) 
            {
                g_WorldEditor.MoveSelectedObjects(m_FocusPos);
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
            }
            else if (IsPlacementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
            {
                m_bObjectMoved = TRUE;
                g_WorldEditor.MoveTemporaryObjects(m_FocusPos);
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
            }
            SetViewDirty();
            break;

        case VK_NEXT :
            if (m_bGridSnap)
            {
                s32 iNewLocation = ((s32)(GetDocument()->GetGridPos().GetY()) % GetDocument()->GetGridSnap()) + GetDocument()->GetGridSnap();

                vector3 GridPos = GetDocument()->GetGridPos();
                GridPos.GetY() -= iNewLocation;
                GetDocument()->SetGridPos(GridPos);

                m_FocusPos.GetY() = GridPos.GetY();
                m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );
            }
            else
            {
                vector3 GridPos = GetDocument()->GetGridPos();
                GridPos.GetY() -= 1.0f;
                GetDocument()->SetGridPos(GridPos);
            }
            m_Grid.SetTranslations(GetDocument()->GetGridPos());
            m_GridBaseline.SetTranslations(GetDocument()->GetGridPos());
            if (IsMovementMode()) 
            {
                g_WorldEditor.MoveSelectedObjects(m_FocusPos);
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
            }
            else if (IsPlacementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
            {
                m_bObjectMoved = TRUE;
                g_WorldEditor.MoveTemporaryObjects(m_FocusPos);
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
            }
            SetViewDirty();
            break;

            //
            // TODO: The concept of focus position needs to be formalize in the editor
            //
        case VK_UP:
            {
               vector3 V = m_View.GetViewZ();
               V.GetY() = 0;
               V.Normalize();
               m_FocusPos += V * float(GetDocument()->GetGridSnap());         
               m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );
               if (IsMovementMode()) 
               {
                    g_WorldEditor.MoveSelectedObjects(m_FocusPos);
                    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
               }
               else if (IsPlacementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
               {
                    m_bObjectMoved = TRUE;
                    g_WorldEditor.MoveTemporaryObjects(m_FocusPos);
                    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
               }
               SetViewDirty();
               break;
            }

        case VK_DOWN:
            {
               vector3 V = m_View.GetViewZ();
               V.GetY() = 0;
               V.Normalize();
               m_FocusPos -= V * float(GetDocument()->GetGridSnap());
               m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );
               if (IsMovementMode()) 
               {
                    g_WorldEditor.MoveSelectedObjects(m_FocusPos);
                    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
               }
               else if (IsPlacementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
               {
                    m_bObjectMoved = TRUE;
                    g_WorldEditor.MoveTemporaryObjects(m_FocusPos);
                    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
               }
               SetViewDirty();
               break;
            }

        case VK_LEFT:
            {
               vector3 V = m_View.GetViewX();
               V.GetY() = 0;
               V.Normalize();
               m_FocusPos += V * float(GetDocument()->GetGridSnap());
               m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );
               if (IsMovementMode()) 
               {
                    g_WorldEditor.MoveSelectedObjects(m_FocusPos);
                    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
               }
               else if (IsPlacementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
               {
                    m_bObjectMoved = TRUE;
                    g_WorldEditor.MoveTemporaryObjects(m_FocusPos);
                    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
               }
               SetViewDirty();
               break;
            }

        case VK_RIGHT:
            {
               vector3 V = m_View.GetViewX();
               V.GetY() = 0;
               V.Normalize();
               m_FocusPos -= V * float(GetDocument()->GetGridSnap());
               m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );
               if (IsMovementMode()) 
               {
                    g_WorldEditor.MoveSelectedObjects(m_FocusPos);
                    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
               }
               else if (IsPlacementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
               {
                    m_bObjectMoved = TRUE;
                    g_WorldEditor.MoveTemporaryObjects(m_FocusPos);
                    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
               }
               SetViewDirty();
               break;
            }
        case VK_NUMPAD4:
        case VK_DELETE: 
            if (IsPlacementMode() || IsMovementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
            {
                OnRotateObjects(0,(GetDocument()->GetRotateSnap()),0);
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                SetViewDirty();
                m_bObjectMoved = TRUE;
            }
            break;
        case VK_NUMPAD6:
        case VK_END:
            if (IsPlacementMode() || IsMovementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
            {
                OnRotateObjects(0,-(GetDocument()->GetRotateSnap()),0);
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                SetViewDirty();
                m_bObjectMoved = TRUE;
            }
            break;        
        case VK_NUMPAD8:
            if (IsPlacementMode() || IsMovementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
            {
                OnRotateObjects((GetDocument()->GetRotateSnap()),0,0);
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                SetViewDirty();
                m_bObjectMoved = TRUE;
            }
            break;
        case VK_NUMPAD2:
            if (IsPlacementMode() || IsMovementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
            {
                OnRotateObjects(-(GetDocument()->GetRotateSnap()),0,0);
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                SetViewDirty();
                m_bObjectMoved = TRUE;
            }
            break;
        case VK_NUMPAD7:
            if (IsPlacementMode() || IsMovementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
            {
                OnRotateObjects(0,0,(GetDocument()->GetRotateSnap()));
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                SetViewDirty();
                m_bObjectMoved = TRUE;
            }
            break;
        case VK_NUMPAD9:
            if (IsPlacementMode() || IsMovementMode() || IsBlueprintMode() || IsDecalPlacementMode()) 
            {
                OnRotateObjects(0,0,-(GetDocument()->GetRotateSnap()));
                m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
                SetViewDirty();
                m_bObjectMoved = TRUE;
            }
            break;
        case VK_ESCAPE:
            //always clear guidLookupDisplay and guid highlight
//            g_WorldEditor.ClearGuidLookupList();
            m_GuidToHighLight = 0;

            if (g_WorldEditor.IsInHoverSelectMode())
            {
                //clear the hover select stuff
                g_WorldEditor.ClearHoverSelection();
                g_WorldEditor.SetGuidSelectMode("",FALSE);
            }
            else if (IsPlacementMode())
            {
                CancelPlacementMode();
            }
            else if (IsBlueprintMode())
            {
                CancelBlueprintMode();
            }
            else if (IsDecalPlacementMode())
            {
                CancelDecalPlacementMode();
            }
            else if (IsMovementMode())
            {
                g_WorldEditor.CheckSelectedForDuplicates();
                CancelMovementMode();
            }
            else if (GetDocument()->IsGameRunning())
            {
                GetDocument()->StopGame();
                SetViewDirty();    
            }
            SetViewDirty();
            break;
        case VK_SPACE:
            if (IsPlacementMode())
            {
                //place object
                guid Guid = g_WorldEditor.PlaceObjectsFromTemporary();
                ai_editor::GetAIEditor()->UpdateAI();
                if (Guid.Guid!=0)
                {
                    m_pFrameEdit->AddObjectToActiveLayerView(Guid);
                }
            }
            else if (IsBlueprintMode())
            {
                //now do the copy
                guid BPGuid = g_WorldEditor.PlaceObjectsFromTemporary();
                if (BPGuid.Guid!=0)
                {
                    //Add to layers
                    m_pFrameEdit->AddBlueprintToActiveLayerView(BPGuid);
                }
            }
            else if (IsDecalPlacementMode())
            {
                guid Guid = g_WorldEditor.PlaceDecalFromTemporary();
                if ( Guid.Guid!=0 )
                {
                    m_pFrameEdit->AddObjectToActiveLayerView(Guid);
                }
            }
            else if (IsMovementMode())
            {
                g_WorldEditor.CheckSelectedForDuplicates();
                CancelMovementMode();
            }
            else if (GetDocument()->IsGameRunning())
            {
                //toggle pause
                GetDocument()->PauseGame(!GetDocument()->IsGamePaused());
            }
            m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
            SetViewDirty();
            break;
        case VK_F7:
            {           
                if ( (( GetKeyState( VK_CONTROL ) & ~1 ) != 0) &&
                     (( GetKeyState( VK_LSHIFT ) & ~1 ) != 0) )
                {
                    s32 nObjects = g_WorldEditor.GetSelectedCount();
                    s32 i;
                    xarray<u16> Zones;

                    // Build a list of zones that we are going to select from

                    for (i=0;i<nObjects;i++)
                    {
                        guid SelObjGuid = g_WorldEditor.GetSelectedObjectsByIndex( i );
                        object* pSelObj = g_ObjMgr.GetObjectByGuid( SelObjGuid );
                        if (NULL == pSelObj)
                            continue;

                        u16 iZone1 = pSelObj->GetZone1();
                        u16 iZone2 = pSelObj->GetZone2();
                        xbool bAdd1,bAdd2;

                        bAdd1 = bAdd2 = TRUE;

                        if (iZone1 == 0)
                            bAdd1 = FALSE;
                        if (iZone2 == 0)
                            bAdd2 = FALSE;

                        s32 j;
                        for (j=0;j<Zones.GetCount();j++)
                        {
                            if (Zones[j] == iZone1)
                                bAdd1 = FALSE;
                            if (Zones[j] == iZone2)
                                bAdd2 = FALSE;
                        }
                        if (bAdd1)
                            Zones.Append( iZone1 );
                        if (bAdd2)
                            Zones.Append( iZone2 );
                    }

                    // Look through all objects and add it to the selection set
                    // if it belongs to one of the targeted zones
                    if (Zones.GetCount() > 0)
                    {
                        s32 iType = object::TYPE_NULL+1;
                        while (iType != object::TYPE_END_OF_LIST)
                        {
                            slot_id iSlot = g_ObjMgr.GetFirst( (object::type) iType );
                            while (iSlot != SLOT_NULL)
                            {
                                object* pObj = g_ObjMgr.GetObjectBySlot( iSlot );
                                if (pObj)
                                {
                                    u16 iZone1,iZone2;
                                    iZone1 = pObj->GetZone1();
                                    iZone2 = pObj->GetZone2();

                                    s32 j;
                                    for (j=0;j<Zones.GetCount();j++)
                                    {
                                        if ((Zones[j] == iZone1) || (Zones[j] == iZone2))
                                        {
                                            g_WorldEditor.SelectObject( pObj->GetGuid(), FALSE );
                                            break;
                                        }
                                    }                                    
                                }
                                iSlot = g_ObjMgr.GetNext( iSlot );
                            }

                            iType++;
                        }
                    }

                    m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
                    SetViewDirty();
                }
            }
            break;
        case VK_F8:	
			{
				//
				//	Select all objects matching the type of the currently selected one.
				//	If the CTRL modifier is used, only select matching objects in the 
				//  same zone as the current one.
				//
				editor_blueprint_ref	BPRef;
				if (g_WorldEditor.IsOneBlueprintSelected(BPRef))
				{
					// Handle blueprints instead of single objects
					g_WorldEditor.SelectAllMatchingBlueprints( BPRef );
				}
				else
				{
					if( g_WorldEditor.GetSelectedCount() != 1)
						break;

					xbool bSelectedGridOnly = FALSE;

					if (( GetKeyState( VK_CONTROL ) & ~1 ) != 0)
						bSelectedGridOnly = TRUE;

					guid SelObjGuid = g_WorldEditor.GetSelectedObjectsByIndex( 0 );
					object* pSelObj = g_ObjMgr.GetObjectByGuid( SelObjGuid );
					if (NULL == pSelObj)
						break;

					slot_id SlotID = g_ObjMgr.GetFirst( pSelObj->GetType() );

					u16 Zone1, Zone2;

					Zone1 = pSelObj->GetZone1();
					Zone2 = pSelObj->GetZone2();

					while ( SLOT_NULL != SlotID )
					{
						object* pCurObj = g_ObjMgr.GetObjectBySlot( SlotID );
						if (NULL == pCurObj)
							break;

						xbool bAddThis = TRUE;

						if (bSelectedGridOnly)
						{
							// Check for exclusion
							if ((Zone1 != 0) && (pCurObj->GetZone1() != Zone1) && (pCurObj->GetZone2() != Zone1))
								bAddThis = FALSE;
							else if ((Zone2 != 0) && (pCurObj->GetZone1() != Zone2) && (pCurObj->GetZone2() != Zone2))
								bAddThis = FALSE;
						}

						if (bAddThis)
						{
							g_WorldEditor.SelectObject( pCurObj->GetGuid(), FALSE );
						}

						SlotID = g_ObjMgr.GetNext( SlotID );
					}
				}

				m_pFrameEdit->GetSettingsEditorDoc()->Refresh();
                SetViewDirty();
			}
            break;
        default:
	        CView3D::OnKeyDown(nChar, nRepCnt, nFlags);
            break;
    }
}

//=========================================================================

void CEditorView::MoveGridToSelectedObjects()
{
    vector3 minPos = g_WorldEditor.GetMinPositionForSelected( );
    GetDocument()->SetGridPos(minPos);
    m_Grid.SetTranslations(minPos);
    m_GridBaseline.SetTranslations(minPos);
    SetFocusPos(minPos);   
    SetViewDirty();
}

//=========================================================================

void CEditorView::OnRotateObjects(s32 rX, s32 rY, s32 rZ)
{   
    radian3 r(DEG_TO_RAD(rX),DEG_TO_RAD(rY),DEG_TO_RAD(rZ));
    if (IsPlacementMode() || IsBlueprintMode() || IsDecalPlacementMode())
    {
        g_WorldEditor.RotateTemporaryObjects( r );
        m_FocusPos = g_WorldEditor.GetMinPositionForTempObjects( );
        if (m_bGridSnap)
        {
            m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );
            g_WorldEditor.MoveTemporaryObjects(m_FocusPos);
        }
    }
    else if (IsMovementMode())
    {
        //undo entry already started
        g_WorldEditor.RotateSelectedObjects( r );
        m_FocusPos = g_WorldEditor.GetMinPositionForSelected();
        if (m_bGridSnap)
        {
            m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );
            g_WorldEditor.MoveSelectedObjects(m_FocusPos);
        }
    }
    else
    {
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(xfs("Keyboard Rotate %d Objects",g_WorldEditor.GetSelectedCount())));
        g_WorldEditor.RotateSelectedObjects( r );
        m_FocusPos = g_WorldEditor.GetMinPositionForSelected();
        if (m_bGridSnap)
        {
            m_FocusPos.Snap( float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()),float(GetDocument()->GetGridSnap()) );
            g_WorldEditor.MoveSelectedObjects(m_FocusPos);
        }
        g_WorldEditor.CommitCurrentUndoEntry();
    }

    m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
    SetViewDirty();
}

//=========================================================================

void CEditorView::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (g_WorldEditor.IsInHoverSelectMode())
    {
        //must keep focus in this window
        SetFocus();

        //hey we are in hover mode so we need to get ray cast and highlight guid
        vector3 P0;
        vector3 P1;
        CastRayFrom2D( point.x, point.y, P0, P1 );
        g_WorldEditor.DoHoverSelect(P0, P1, m_bRenderIcons);
        SetViewDirty();

        if  (IsActionMode())
        {
            if (( GetKeyState( 'C' ) & ~1 ) != 0)
            {
                // orbit cam
                CameraOrbitMode(m_FocusPos);
            }
            else
            {
                CameraFreeFlyMode();
            }
        }

	    CView3D::OnMouseMove(nFlags, point);
    }
    else if (m_bDragSelect||m_bDragUnSelect)
    {
        m_ptDragSelectEnd = point;
  	    SetViewDirty();
    }
    else
    {
	    if (m_bDraggingObject)
        {
    	    SetViewDirty();
            OnMoveObjects(point);
        }
	    
        if  (IsActionMode())
        {
            if (( GetKeyState( 'C' ) & ~1 ) != 0)
            {
                // orbit cam
                CameraOrbitMode(m_FocusPos);
            }
            else
            {
                CameraFreeFlyMode();
            }
        }

	    CView3D::OnMouseMove(nFlags, point);
    }
}

//=========================================================================

void CEditorView::OnMoveObjects(CPoint point)
{
    plane   Plane( vector3( 0,1,0 ), -(GetDocument()->GetGridPos().GetY()) );

    vector3 P0;
    vector3 P1;
    f32     t;
    CastRayFrom2D( point.x, point.y, P0, P1 );

    if( Plane.Intersect( t, P0, P1 ) == TRUE )
    {
        m_FocusPos = P0 + t*( P1 - P0 );

        if (m_bGridSnap)
            m_FocusPos.Snap( float(GetDocument()->GetGridSnap()), float(GetDocument()->GetGridSnap()), float(GetDocument()->GetGridSnap()) );

        if (IsPlacementMode() || IsBlueprintMode())
        {
            if (m_bAddWithRaySnap)
            {
                vector3 CollisionPt;
                if (g_WorldEditor.GetCollisionPointIgnoreTemp(P0,P1,CollisionPt))
                {
                    m_FocusPos = CollisionPt;
                }
            }

            g_WorldEditor.MoveTemporaryObjects(m_FocusPos);
        }
        else if (IsDecalPlacementMode())
        {
            if ( g_WorldEditor.CalcDecalPlacement( P0, P1 ) )
            {
                SetFocusPos(g_WorldEditor.GetMinPositionForSelected());
            }
        }
        else if (IsMovementMode())
        {
            if (m_bAddWithRaySnap)
            {
                vector3 CollisionPt;
                if (g_WorldEditor.GetCollisionPointIgnoreSel(P0,P1,CollisionPt))
                {
                    m_FocusPos = CollisionPt;
                }
            }

            //for undo, dirty flag
            m_bObjectMoved = TRUE;

            g_WorldEditor.MoveSelectedObjects(m_FocusPos);
        }
    }
    SetViewDirty();
}

//=========================================================================

void CEditorView::OnKillFocus(CWnd* pNewWnd) 
{
	CView3D::OnKillFocus(pNewWnd);
	
	SetViewDirty();
}

//=========================================================================

BOOL CEditorView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    LPARAM lParam = MAKELPARAM(pt.x,pt.y);
    WPARAM wParam = MAKEWPARAM(0, zDelta);

    x_try;
    eng_D3DWndProc( m_hWnd, WM_MOUSEWHEEL, wParam, lParam );
	x_catch_display;

    xarray<guid> selectedObjects;
    xbool WheelForNavConns = true;
    if (( GetKeyState( VK_CONTROL ) & ~1 ) == 0)
    {
        //CONTROL is not held down, therefore scroll normally
        WheelForNavConns = false;
    }
    else if( g_WorldEditor.GetSelectedCount() )
    {
        g_WorldEditor.GetSelectedList(selectedObjects);
        s32 count;
        for(count =0; (u32)count < g_WorldEditor.GetSelectedCount() ; count++ )
        {
            guid tempGuid = selectedObjects[count];
            object* tempObject = g_ObjMgr.GetObjectByGuid(tempGuid);
            if(tempObject)
            {
                object::type objType = tempObject->GetType();
                if (objType == object::TYPE_NAV_CONNECTION2_EDITOR) 
                {
                    //change nav conn width
                    nav_connection2_editor& tempNavConnection = nav_connection2_editor::GetSafeType(*tempObject);
                    tempNavConnection.ScaleWidth( ((f32)zDelta)/50.0f);
                    g_WorldEditor.SetObjectsLayerAsDirty( tempGuid );
                    SetViewDirty();
                }
            }
        }
    }

    if(!WheelForNavConns)
    {
        SetViewDirty();
	    return CView3D::OnMouseWheel(nFlags, zDelta, pt);
    }
    else
    {
        return true;
    }
}

//=========================================================================

void CEditorView::CancelPlacementMode()
{
    //cancel placement mode
    g_WorldEditor.ClearTemporaryObjects();
    m_enMode = WEV_STANDARD_MODE;
    if (m_pFrameEdit && m_pFrameEdit->GetPropertyEditorDoc())
    {
        m_pFrameEdit->GetPropertyEditorDoc()->ClearGrid();
    }
    SetViewDirty();
}

//=========================================================================

void CEditorView::EnterPlacementMode()
{
    m_enMode = WEV_PLACEMENT_MODE;
    SetViewDirty();
}

//=========================================================================

void CEditorView::CancelBlueprintMode()
{
    //cancel placement mode
    g_WorldEditor.ClearTemporaryObjects();
    m_enMode = WEV_STANDARD_MODE;
    if (m_pFrameEdit && m_pFrameEdit->GetPropertyEditorDoc())
    {
        m_pFrameEdit->GetPropertyEditorDoc()->ClearGrid();
    }
    SetViewDirty();
}

//=========================================================================

void CEditorView::EnterBlueprintMode()
{
    m_enMode = WEV_BLUEPRINT_MODE;
    SetViewDirty();
}

//=========================================================================

void CEditorView::CancelMovementMode()
{
    //handle undo
    if (m_bObjectMoved)
    {
        g_WorldEditor.UndoCommitSelObjectsProps();
    }
    else
    {
        g_WorldEditor.ClearUndoEntry();
        g_WorldEditor.ClearUndoList();
    }

    m_enMode = WEV_STANDARD_MODE;
    if (m_pFrameEdit && m_pFrameEdit->GetPropertyEditorDoc())
    {
        m_pFrameEdit->GetPropertyEditorDoc()->Refresh();
    }
    SetViewDirty();
}

//=========================================================================

void CEditorView::EnterMovementMode()
{
    //handle undo
    g_WorldEditor.UndoBeginSelObjectsPropsChange();
    m_bObjectMoved = FALSE;

    m_enMode = WEV_MOVEMENT_MODE;
    SetViewDirty();
}

//=========================================================================

void CEditorView::CancelDecalPlacementMode()
{
    //handle undo
    // TODO:

    g_WorldEditor.ClearTemporaryObjects();
    m_enMode = WEV_STANDARD_MODE;
    if (m_pFrameEdit && m_pFrameEdit->GetPropertyEditorDoc())
    {
        m_pFrameEdit->GetPropertyEditorDoc()->ClearGrid();
    }
    SetViewDirty();
}

//=========================================================================

void CEditorView::EnterDecalPlacementMode()
{
    m_enMode = WEV_DECAL_PLACEMENT_MODE;
    SetViewDirty();
}

//=========================================================================

void CEditorView::OnTimer(UINT nIDEvent) 
{
    //run the game
    CEditorDoc* pDoc = (CEditorDoc*)GetDocument();
    ASSERT( pDoc );

    if (g_WorldEditor.ShouldRscRefresh())
    {
        g_WorldEditor.RefreshResources();
        SetViewDirty();
    }
    
    if (g_WorldEditor.IsPerformingAutomatedBuild())
    {
        if (g_WorldEditor.UpdateAutomatedBuild())
        {
            // Close any open project
            CMainFrame::s_pMainFrame->PostMessage( WM_CLOSE, 0, 0 );

            // Force the editor to shutdown
//            AfxGetMainWnd()->PostMessage(WM_CLOSE, 0, 0);
            return;
        }
    }
    
    pDoc->AdvanceLogic();

    if( pDoc->IsDocumentActive() == FALSE )
        return;

    CWnd*       pWnd = ::AfxGetMainWnd();

    if (pWnd)
    {
        CString strModeOut = "";
        CString strInfo;
        strInfo.Format("{GS:%d, RS:%d} {SEL:%d, TOT:%d}",GetDocument()->GetGridSnap(),GetDocument()->GetRotateSnap(),
                       g_WorldEditor.GetSelectedCount(), g_WorldEditor.GetObjectCount());

        switch(m_enMode)
        {
        case WEV_PLACEMENT_MODE:
            strModeOut = "Mode: Placement";
            break;
        case WEV_MOVEMENT_MODE:
            strModeOut = "Mode: MoveSelection";
            break;
        case WEV_BLUEPRINT_MODE:
            strModeOut = "Mode: AddBlueprints";
            break;
        case WEV_STANDARD_MODE:
            strModeOut = "Mode: Select";
            break;
        case WEV_DECAL_PLACEMENT_MODE:
            strModeOut = "Mode: DecalPlacement";
            break;
        default:
            strModeOut = "Mode: Unknown";
            break;
        }

        if (GetFocus() != this)
        {
            strModeOut += "(PAUSE)";
        }

        char* pszMode = strModeOut.GetBuffer(strModeOut.GetLength());
        pWnd->SendMessage(WM_USER_MSG_UPDATE_STATUS_BAR,ID_INDICATOR_MODE,(long)pszMode);
        strModeOut.ReleaseBuffer();

        char* pszData = strInfo.GetBuffer(strInfo.GetLength());
        pWnd->SendMessage(WM_USER_MSG_UPDATE_STATUS_BAR,ID_INDICATOR_DATA,(long)pszData);
        strInfo.ReleaseBuffer();
    }
    
    // !!!! This sould really be.... Is camera active. As in the editor camera. 
    // If not then this should do nothing. 
    if( (!GetDocument()->IsFPV()) && IsViewActive() && (GetFocus() == this))
    {
	    //ok do keyboard movement here
        float fMove;
        if (( GetKeyState( VK_LSHIFT ) & ~1 ) != 0)
            fMove = float(GetDocument()->GetMovementSpeed()*5);
        else
            fMove = float(GetDocument()->GetMovementSpeed());

        //editor mode
        if (( GetKeyState( 'W' ) & ~1 ) != 0)
        {
            m_View.Translate( vector3( 0, 0, fMove), view::VIEW );
            SetViewDirty();
        }
        if (( GetKeyState( 'S' ) & ~1 ) != 0)
        {
            m_View.Translate( vector3( 0, 0, -fMove), view::VIEW );
            SetViewDirty();
        }
        if (( GetKeyState( 'A' ) & ~1 ) != 0)
        {
            m_View.Translate( vector3( fMove, 0, 0), view::VIEW );
            SetViewDirty();
        }
        if (( GetKeyState( 'D' ) & ~1 ) != 0)
        {
            m_View.Translate( vector3( -fMove, 0, 0), view::VIEW );
            SetViewDirty();
        }
        if (( GetKeyState( 'R' ) & ~1 ) != 0)
        {
            m_View.Translate( vector3( 0, fMove, 0), view::VIEW );
            SetViewDirty();
        }
        if (( GetKeyState( 'F' ) & ~1 ) != 0)
        {
            m_View.Translate( vector3( 0, -fMove, 0), view::VIEW );
            SetViewDirty();
        }
    }

    if(!GetDocument()->IsFPV())
        CheckForCameraDrop();

    if( pDoc->IsGameRunning() )
    {
        SetViewDirty();
    }

    //hack fix for periodic pulse of temp objects
    if (IsPlacementMode() || IsBlueprintMode() || IsDecalPlacementMode())
    {
        SetViewDirty();
    }

    if (m_WatchUpdateTimer.ReadSec() >= TIMER_FOR_WATCH_UPDATES )
    {
        //update every second
        UpdateWatchTimer(TRUE);
        GetFrame()->RefreshWatchWindowIfActive();
    }
    
    //this will do the rendering
    CView3D::OnTimer(nIDEvent);
}

//=========================================================================

void CEditorView::UpdateWatchTimer(BOOL bActive)
{
    if (bActive)
    {
        m_WatchUpdateTimer.Reset(); 
        m_WatchUpdateTimer.Start(); 
    }
    else
    {
        m_WatchUpdateTimer.Reset(); 
        m_WatchUpdateTimer.Stop(); 
    }
}

//=========================================================================

void CEditorView::CleanView()
{
    switch(m_enMode)
    {
    case WEV_PLACEMENT_MODE:
        CancelPlacementMode();
        break;
    case WEV_MOVEMENT_MODE:
        CancelMovementMode();
        break;
    case WEV_BLUEPRINT_MODE:
        CancelBlueprintMode();
        break;
    case WEV_DECAL_PLACEMENT_MODE:
        CancelDecalPlacementMode();
        break;
    }
}

//=========================================================================

void CEditorView::FocusCamera()
{
    m_View.LookAtPoint( m_FocusPos );
    vector3 vPos = m_View.GetPosition() - m_FocusPos;
    f32 fDist    = vPos.Length();

    fDist -= 2000.0f;
    m_View.Translate( vector3( 0, 0, fDist), view::VIEW );

    SetViewDirty();
}

//=========================================================================

void CEditorView::FocusCameraWithUndo( const vector3& FocusPoint, f32 Distance )
{
    // Save current view to history buffer
    m_ViewHistory.AppendView( m_View, m_PrevFocusPos );

    m_View.LookAtPoint( FocusPoint );
    vector3 vPos = m_View.GetPosition() - FocusPoint;
    f32 fDist    = vPos.Length();

    fDist -= Distance;
    m_View.Translate( vector3( 0, 0, fDist), view::VIEW );

    SetViewDirty();
}

//=========================================================================

void CEditorView::FocusCameraWithUndoAndNoRotation( const vector3& FocusPoint )
{
    f32 d = 2000.0f;

    // Save current view to history buffer
    m_ViewHistory.AppendView( m_View, m_PrevFocusPos );

    if( (FocusPoint - m_View.GetPosition()).Length() > (d * 1.1f) )
    {
        // Get delta to move view and translate it
        m_FocusPos = FocusPoint;
        vector3 Delta = m_FocusPos - m_View.GetPosition() - m_View.GetViewZ() * d;
        m_View.Translate( Delta );
    }
    else
    {
        m_FocusPos = FocusPoint;
        m_View.LookAtPoint( m_FocusPos );
    }
    SetViewDirty();
}

//=========================================================================

void CEditorView::SetCameraWithUndo( const view& View, const vector3& FocusPoint )
{
    // Save current view to history buffer
    m_ViewHistory.AppendView( m_View, m_FocusPos );

    // Get delta to move view and translate it
    m_View      = View;
    m_FocusPos  = FocusPoint;
    SetViewDirty();
}

//=========================================================================

void CEditorView::OnUndo() 
{
    if (IsMovementMode())
    {
        CancelMovementMode();
    }
    
    ASSERT(!g_WorldEditor.InTransaction());
    int nUndone = transaction_mgr::Transaction()->Undo(1);
    ASSERT(nUndone == 1);

    GetFrame()->RefreshAll();
}

//=========================================================================

void CEditorView::OnUpdateUndo(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(transaction_mgr::Transaction()->CanUndo());
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorView::OnRedo() 
{
    if (IsMovementMode())
    {
        CancelMovementMode();
    }

    ASSERT(!g_WorldEditor.InTransaction());
    int nRedone = transaction_mgr::Transaction()->Redo(1);
    ASSERT(nRedone == 1);

    GetFrame()->RefreshAll();
}

//=========================================================================

void CEditorView::OnUpdateRedo(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(transaction_mgr::Transaction()->CanRedo() && 
        (!IsMovementMode())); //not allowed while moving
    pCmdUI->SetCheck(FALSE);	
}

//=========================================================================

void CEditorView::OnCutObjects() 
{
    if (IsMovementMode())
    {
        g_WorldEditor.CheckSelectedForDuplicates();
	    CancelMovementMode();
    }
    else
    {
        //setup generic undo entry for moving of objects
        g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(
            xfs("Moving %d Object(s)",g_WorldEditor.GetSelectedCount())));

	    EnterMovementMode();
    }
    SetFocus(); //set focus to view
}

//=========================================================================

void CEditorView::OnUpdateCutObjects(CCmdUI* pCmdUI) 
{
    if ((g_WorldEditor.GetSelectedCount() > 0) &&
        g_Project.IsProjectOpen() &&
        (!GetDocument()->IsGameRunning()))
    {
        pCmdUI->Enable(IsStandardMode() || IsMovementMode());	
        pCmdUI->SetCheck(IsMovementMode());	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(FALSE);
    }
}

//=========================================================================

void CEditorView::OnCopyObjects() 
{
    xarray<guid> lstItems;
    xarray<guid> lstBPRefs;

    if (g_WorldEditor.IsLayerReadonly(g_WorldEditor.GetActiveLayer()))
    {
        //can't copy
        MessageBeep(MB_ICONASTERISK);
        return;
    }

    //setup generic undo entry for copying of objects
    g_WorldEditor.SetCurrentUndoEntry(new transaction_entry(
        xfs("Copying %d Object(s)",g_WorldEditor.GetSelectedCount())));

    //now do the copy
    g_WorldEditor.CopySelectedObjects(lstItems, lstBPRefs);
    
    //Add to layers
    for (int i=0; i<lstItems.GetCount(); i++)
    {
        guid& ObjGuid = lstItems.GetAt(i);
        GetFrame()->AddObjectToActiveLayerView(ObjGuid);
    }
    for (i=0; i<lstBPRefs.GetCount(); i++)
    {
        guid& BPGuid = lstBPRefs.GetAt(i);
        GetFrame()->AddBlueprintToActiveLayerView(BPGuid);
    }

    if ((lstItems.GetCount() + lstBPRefs.GetCount()) > 0)
    {
	    EnterMovementMode();
        //for UNDO, must be done after entering movement mode, force UNDO commit after moving
        ForceMovementUndoRecording(); 
    }
    else
    {
        g_WorldEditor.ClearUndoEntry();
    }   
    SetViewDirty();
    SetFocus(); //set focus to view
}

//=========================================================================

void CEditorView::OnUpdateCopyObjects(CCmdUI* pCmdUI) 
{
    if ((g_WorldEditor.GetSelectedCount() > 0) &&
        g_Project.IsProjectOpen() &&
        (!GetDocument()->IsGameRunning()))
    {
        pCmdUI->Enable(IsStandardMode());	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorView::OnPasteObjects() 
{
    g_WorldEditor.CheckSelectedForDuplicates();       
	CancelMovementMode();
    SetViewDirty();
    SetFocus();
}

//=========================================================================

void CEditorView::OnUpdatePasteObjects(CCmdUI* pCmdUI) 
{
    if (g_Project.IsProjectOpen() && IsMovementMode())
    {
        pCmdUI->Enable(TRUE);	
    }
    else 
    {
        pCmdUI->Enable(FALSE);
    }
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorView::OnShowReport()
{
    CWaitCursor wc;

    CString strReport;
    strReport.Format("%s\\ResourceReport.txt", g_Project.GetWorkingPath());

    //iterate through the resources
    xarray<xstring> lstRigidGeoms;
	for (int i =0; i < g_RescDescMGR.GetRscDescCount(); i++)
    {
        rsc_desc_mgr::node &Node = g_RescDescMGR.GetRscDescIndex(i);
        if (x_stricmp(Node.pDesc->GetType(),"RigidGeom") == 0 )
        {
            lstRigidGeoms.Append(Node.pDesc->GetName());
        }
    }

    g_WorldEditor.WriteRigidGeomReport(strReport, lstRigidGeoms);
    
    ::ShellExecute(m_hWnd, "open", "notepad.exe", strReport, NULL, SW_SHOWNORMAL);
}

//=========================================================================

void CEditorView::SetViewDirty( void )
{
    CView3D::SetViewDirty();
}

//=========================================================================

xbool CEditorView::CheckForCameraDrop( void )
{
    //Do Drop Camera
    if(     ( !GetDocument()->IsFPV() )
         && ( IsViewActive() )  
         && ( GetFocus() == this)
         && ( GetKeyState( VK_F9 ) & ~1 ) )
    {
        //Drop Player Two meters below camera

        player* pPlayer = SMP_UTIL_GetActivePlayer();
        if(pPlayer)
        {
            vector3 Start = m_View.GetPosition();
            vector3 End = Start + vector3(0,-200,0);

            g_CollisionMgr.CylinderSetup( pPlayer->GetGuid(), 
                                            Start, 
                                            End,
                                            40.0f,
                                            200.0f);
            g_CollisionMgr.CheckCollisions();
            if(g_CollisionMgr.m_nCollisions)
            {
                End = Start + g_CollisionMgr.m_Collisions[0].T*(End-Start);
            }

            matrix4 L2W = pPlayer->GetL2W();
            L2W.SetTranslation( End + vector3(0,2,0) );
            //pPlayer->OnMove( End + vector3(0,2,0) );
            ((object*)pPlayer)->OnTransform( L2W );
            return TRUE;
        }
    }
    return FALSE;
}   

//=========================================================================

void CEditorView::CameraUndo( void )
{
    if( m_ViewHistory.CanUndo() )
    {
        view_history::entry e = m_ViewHistory.Undo( m_View, m_FocusPos );
        m_View     = e.m_View;
        m_FocusPos = e.m_FocusPos;
        SetViewDirty();
    }
}

void CEditorView::CameraRedo( void )
{
    if( m_ViewHistory.CanRedo() )
    {
        view_history::entry e = m_ViewHistory.Redo( m_View, m_FocusPos );
        m_View     = e.m_View;
        m_FocusPos = e.m_FocusPos;
        SetViewDirty();
    }
}

xbool CEditorView::CameraCanUndo( void )
{
    return m_ViewHistory.CanUndo();
}

xbool CEditorView::CameraCanRedo( void )
{
    return m_ViewHistory.CanRedo();
}

void CEditorView::CameraGotoPlayer( void )
{
    g_ObjMgr.SelectByAttribute( object::ATTR_ALL, object::TYPE_PLAYER );
    slot_id Slot = g_ObjMgr.StartLoop();
    while( Slot != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( Slot );
        if( pObject && pObject->IsKindOf( player::GetRTTI() ) )
        {
            if( g_WorldEditor.SelectObject( pObject->GetGuid() ) )
            {
                // Move the camera focus
                vector3 Pos = pObject->GetPosition(); // g_WorldEditor.GetMinPositionForSelected();
                SetFocusPos( Pos );
                FocusCameraWithUndo( Pos, 1000.0f );
                break;
            }
        }

        Slot = g_ObjMgr.GetNextResult( Slot );
    }
    g_ObjMgr.EndLoop();
}

//=========================================================================

view_history::view_history( void )
{
    // Give it a default of 100 place history
    s32 Size = 100;
    m_Buffer.SetCapacity( Size );
    m_Buffer.SetCount( Size );
    m_BufferSize   = Size;
    m_BufferBase   = 0;
    m_BufferLast   = 0;
    m_BufferCursor = 0;
    m_Empty        = TRUE;
}

view_history::~view_history( void )
{
}

s32 view_history::BasedIndex( s32 Index )
{
    return( (m_BufferBase + Index) % m_BufferSize );
}

void view_history::SetSize( s32 Size )
{
    // Increase size to deal with a hidden entry in the buffer
    Size++;

    // Must be at least 3 elements to allow undo / redo to work
    Size = MAX( Size, 3 );

    m_Buffer.Clear();
    m_Buffer.SetCapacity( Size );
    m_Buffer.SetCount( Size );
    m_BufferSize   = Size;
    m_BufferBase   = 0;
    m_BufferLast   = 0;
    m_BufferCursor = 0;
    m_Empty        = TRUE;
}

void view_history::AppendView( const view& View, const vector3& FocusPos )
{
    m_Empty = FALSE;

    entry e( View, FocusPos );

    m_Buffer[BasedIndex(m_BufferCursor)] = e;
    m_BufferLast = m_BufferCursor++;

    if( (m_BufferCursor+1) >= m_BufferSize )
    {
        m_BufferBase = BasedIndex( 1 );
        m_BufferLast--;
        m_BufferCursor--;
    }
}

xbool view_history::CanUndo( void )
{
    return( !m_Empty && (m_BufferCursor > 0) );
}

xbool view_history::CanRedo( void )
{
    return( !m_Empty && (m_BufferCursor <= m_BufferLast) );
}

view_history::entry view_history::Undo( const view& View, const vector3& FocusPos )
{
    ASSERT( CanUndo() );

    entry e( View, FocusPos );

    if( CanUndo() )
    {
        // Store current view if it has changed
        if( x_memcmp( &m_Buffer[BasedIndex(m_BufferCursor)], &e, sizeof(entry) ) != 0 )
        {
            m_Buffer[BasedIndex(m_BufferCursor)] = e;
        }

        // Undo to previous view
        m_BufferCursor--;

        return m_Buffer[BasedIndex(m_BufferCursor)];
    }
    else
    {
        return e;
    }
}

view_history::entry view_history::Redo( const view& View, const vector3& FocusPos )
{
    ASSERT( CanRedo() );

    entry e( View, FocusPos );

    if( CanRedo() )
    {
        // Store current view if it has changed
        if( x_memcmp( &m_Buffer[BasedIndex(m_BufferCursor)], &e, sizeof(entry) ) != 0 )
        {
            m_Buffer[BasedIndex(m_BufferCursor)] = e;
        }

        // Redo to next
        m_BufferCursor++;

        return m_Buffer[BasedIndex(m_BufferCursor)];
    }
    else
    {
        return e;
    }
}

//=========================================================================
