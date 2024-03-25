//=============================================================================
//
//  
//
//=============================================================================
#include "nav_map.hpp"
#include "ng_node2.hpp"
#include "ng_connection2.hpp"
#include "Entropy.hpp"
#include "MiscUtils/SimpleUtils.hpp"
#include "MiscUtils\PriorityQueue.hpp"
#include "AI\AIMgr.hpp"
#include "Objects/Actor/Actor.hpp"
#include "Characters/Character.hpp"

#ifdef X_EDITOR
#include "..\..\Apps\WorldEditor\nav_connection2_editor.hpp"
#endif // X_EDITOR
nav_map g_NavMap;

const f32 k_NavYIncrease = 225.0f;
const f32 k_NavYDecrease = 325.0f;
xbool nav_map::s_bDebugNavigation = FALSE;

//===========================================================================

step_data::step_data()
{
    Clear();
}

//===========================================================================

void step_data::Clear( void )
{
    m_CurrentConnection = NULL_NAV_SLOT;
    m_DestConnection    = NULL_NAV_SLOT;
    m_NodeToPassThrough = NULL_NAV_SLOT;
}

//===========================================================================

path_find_struct::path_find_struct()
{
    m_nSteps = 0;
    m_vStartPoint.Set( 0.f, 0.f, 0.f );
    m_vEndPoint = m_vStartPoint;
    m_bStartPointOnConnection = FALSE;
    m_bEndPointOnConnection = FALSE;
    m_bStraightPath = FALSE;
    m_StartConnectionSlotID = NULL_NAV_SLOT;    
    m_EndConnectionSlotID = NULL_NAV_SLOT;    
}

//===========================================================================

void path_find_struct::Clear( void )
{
    m_nSteps = 0;
    m_vStartPoint.Set( 0.f, 0.f, 0.f );
    m_vEndPoint = m_vStartPoint;
    m_bStartPointOnConnection = FALSE;
    m_bEndPointOnConnection = FALSE;
    m_bStraightPath = FALSE;
    m_StartConnectionSlotID = NULL_NAV_SLOT;    
    m_EndConnectionSlotID = NULL_NAV_SLOT;    
    for ( s32 i = 0; i < MAX_EDGES_IN_LIST; i++ )
    {
        m_StepData[i].Clear();
    }
    
#ifndef X_RETAIL    
    //  Debug data
    m_nClipLineConnections = 0;
    m_ClipLineStart.Zero();
    m_ClipLineEnd.Zero();
#endif    
}

//===========================================================================

#ifndef X_RETAIL

void draw_Connection( ng_connection2& Con, s32 i, const vector3& Offset, xcolor Color )
{
    // Get corner pts
    vector3 P[4];
    Con.GetCorners( P );

    // Draw interior
    draw_Begin( DRAW_QUADS, DRAW_USE_ALPHA | DRAW_CULL_NONE | DRAW_NO_ZWRITE );
    draw_Color( Color );
    draw_Vertex( P[0] + Offset );
    draw_Vertex( P[1] + Offset );
    draw_Vertex( P[2] + Offset );
    draw_Vertex( P[3] + Offset );
    draw_End();

    // Draw edges
    draw_Begin( DRAW_LINE_STRIPS, DRAW_CULL_NONE | DRAW_NO_ZWRITE );
    draw_Color( XCOLOR_WHITE );
    draw_Vertex( P[0] + Offset );
    draw_Vertex( P[1] + Offset );
    draw_Vertex( P[2] + Offset );
    draw_Vertex( P[3] + Offset );
    draw_Vertex( P[0] + Offset );
    draw_End();

    // Draw #
    draw_Label( ( ( P[0] + P[1] + P[2] + P[3] ) * 0.25f ) + Offset, XCOLOR_WHITE, "%d", i );
}

//=========================================================================
// DEBUG RENDER ANIMATION FUNCTIONS
//=========================================================================

#ifdef X_EDITOR

void navRenderFullScreenQuad( void )
{
    const view* pView = eng_GetView();
    matrix4 L2W;
    L2W.Identity();
    L2W.Scale( vector3(1000,1000,50) );
    L2W.RotateX( pView->GetViewZ().GetPitch() );
    L2W.RotateY( pView->GetViewZ().GetYaw() );
    L2W.Translate( pView->GetPosition() );
    draw_SetL2W(L2W);
    draw_Vertex(+1,+1,1);
    draw_Vertex(+1,-1,1);
    draw_Vertex(-1,+1,1);
    draw_Vertex(-1,+1,1);
    draw_Vertex(+1,-1,1);
    draw_Vertex(-1,-1,1);
    draw_End();
    draw_ClearL2W();
}

void navPrepD3DForStencil( void )
{
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE      );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE     );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE      );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE     );

    g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,  TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,    D3DCMP_ALWAYS );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,    0xFFFFFFFF );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,    D3DSTENCILOP_KEEP );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL,   D3DSTENCILOP_KEEP );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,    D3DSTENCILOP_REPLACE  );

    g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x0 );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );
}

void navPrepD3DForIncreStencil( void )
{
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE      );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE     );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE      );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE     );

    g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,  TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,    D3DCMP_ALWAYS );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,    0xFFFFFFFF );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,    D3DSTENCILOP_KEEP );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL,   D3DSTENCILOP_KEEP );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,    D3DSTENCILOP_INCR );

    g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x0 );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );
}

void navPrepD3DForDecreStencil( void )
{
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE      );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE     );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE      );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE     );

    g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,  TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,    D3DCMP_ALWAYS );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,    0xFFFFFFFF );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,    D3DSTENCILOP_KEEP );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL,   D3DSTENCILOP_KEEP );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,    D3DSTENCILOP_DECR );

    g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x0 );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );
}

void navDoFinalStencil()
{
    // Darken outside of frustum
    {
        draw_Begin( DRAW_TRIANGLES );
        draw_Color(xcolor(64,64,64,255));

        navPrepD3DForStencil();
        g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,       FALSE );
        g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,   0x0F );
        g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,        D3DCMP_EQUAL );
        g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,         0 );
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,           D3DBLEND_ZERO );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,          D3DBLEND_SRCCOLOR );
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,           D3DCULL_CW );

        navRenderFullScreenQuad();
    }

    // Illuminate inside of frustum
    {
        draw_Begin( DRAW_TRIANGLES );
        draw_Color(xcolor(45,45,45,255));

        navPrepD3DForStencil();
        g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,       FALSE );
        g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,   0x0F );
        g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,        D3DCMP_EQUAL );
        g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,         0xFFFFFFFF );
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,           D3DBLEND_ONE );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,          D3DBLEND_ONE );
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,           D3DCULL_CW );

        navRenderFullScreenQuad();
    }

    g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,  FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x0F );
}


void navRenderFrustum( const vector3* VertexList, xbool bInside )
{
    s16     IndexList[] = {0,4,1, 1,4,5, 
                           1,5,3, 3,5,7, 
                           3,7,2, 2,7,6, 
                           2,6,0, 0,6,4,
                           0,1,3, 0,3,2, 
                           4,6,7, 4,7,5 };

    // Render Front
    {
        draw_Begin( DRAW_TRIANGLES );
        draw_Color(xcolor(64,64,64,0));
        navPrepD3DForIncreStencil();
//        g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,     0x00000001 );
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
        draw_Verts( VertexList, 8 );
        draw_Execute( IndexList, sizeof(IndexList)/sizeof(s16) );
        draw_End();
    }

    // Clear Back
    {
        draw_Begin( DRAW_TRIANGLES );
        draw_Color(xcolor(0,0,0,0));
        navPrepD3DForDecreStencil();
//        g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,     0xFFFFFFFF );
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
        draw_Verts( VertexList, 8 );
        draw_Execute( IndexList, sizeof(IndexList)/sizeof(s16) );
        draw_End();
    }
}

// Draws a lit quad
void navdraw_LitQuad( const vector3& A, 
                  const vector3& B, 
                  const vector3& C, 
                  const vector3& D,
                  const vector3& LightDir,
                  f32      LightDirI,
                  f32      LightAmbI,
                  xcolor   LightColor ) 
{
    // Compute lighting
    vector3 N  = (A - C).Cross(B - A) ;
    N.Normalize() ;
    f32 Dot = N.Dot(LightDir) * LightDirI ;
    if (Dot < 0)
        Dot = 0 ;
    Dot += LightAmbI ;
    if (Dot > 1)
        Dot = 1 ;

    // Compute color
    LightColor.R = (u8)((f32)LightColor.R * Dot) ;
    LightColor.G = (u8)((f32)LightColor.G * Dot) ;
    LightColor.B = (u8)((f32)LightColor.B * Dot) ;

    // Draw quad
    draw_Color( LightColor );
    draw_Vertex(D) ;
    draw_Vertex(C) ;
    draw_Vertex(B) ;
    draw_Vertex(A) ;
}

//===========================================================================

void nav_map::RenderConnectionsBright()
{
#ifdef X_EDITOR
    if( g_AIMgr.GetRenderConnectionsBright() )
    {
        // Clear stencil buffer
        {
            draw_Begin( DRAW_TRIANGLES );
            draw_Color(xcolor(64,64,64,0));
            navPrepD3DForStencil();
            g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,     0x0 );
            g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
            navRenderFullScreenQuad();
        }

        // loop through each connection.
        slot_id iSlot = g_ObjMgr.GetFirst( object::TYPE_NAV_CONNECTION2_EDITOR );
        while( SLOT_NULL != iSlot )
        {
            object* pObj = g_ObjMgr.GetObjectBySlot( iSlot );
            if( pObj &&
                pObj->IsKindOf(nav_connection2_editor::GetRTTI()) )
            {
                nav_connection2_editor& navConnection = nav_connection2_editor::GetSafeType( *pObj );
                s32 InView = g_ObjMgr.IsBoxInView( navConnection.GetBBox(), XBIN(111111) );
                if( InView != -1 )
                {
                    // render each visible connection.
                    vector3 *renderCorners = navConnection.GetRenderCorners(); 
                    vector3 navCorners[8];
                    // top up
                    navCorners[2].Set(renderCorners[2].GetX(),renderCorners[2].GetY()+k_NavYIncrease,renderCorners[2].GetZ());
                    navCorners[3].Set(renderCorners[3].GetX(),renderCorners[3].GetY()+k_NavYIncrease,renderCorners[3].GetZ());
                    navCorners[6].Set(renderCorners[6].GetX(),renderCorners[6].GetY()+k_NavYIncrease,renderCorners[6].GetZ());
                    navCorners[7].Set(renderCorners[7].GetX(),renderCorners[7].GetY()+k_NavYIncrease,renderCorners[7].GetZ());
                    // bottom down
                    navCorners[0].Set(renderCorners[0].GetX(),renderCorners[0].GetY()-k_NavYDecrease,renderCorners[0].GetZ());
                    navCorners[1].Set(renderCorners[1].GetX(),renderCorners[1].GetY()-k_NavYDecrease,renderCorners[1].GetZ());
                    navCorners[4].Set(renderCorners[4].GetX(),renderCorners[4].GetY()-k_NavYDecrease,renderCorners[4].GetZ());
                    navCorners[5].Set(renderCorners[5].GetX(),renderCorners[5].GetY()-k_NavYDecrease,renderCorners[5].GetZ());
                    navRenderFrustum(navCorners,FALSE);
                }
            }
            iSlot = g_ObjMgr.GetNext( iSlot );
        }
        // do the final stuff.
        navDoFinalStencil();
    }
#endif
}

//=========================================================================

// Draws a solid, lit, bbox
void navdraw_LitSolidBBox( const bbox&    BBox,
                       const matrix4& L2W,
                       const vector3& WorldLightDir,
                       f32      LightDirI,
                       f32      LightAmbI,
                       xcolor   LightColor )
{
    // Compute corner points in local space
    vector3 P[8];
    P[0].GetX() = BBox.Min.GetX();    P[0].GetY() = BBox.Min.GetY();    P[0].GetZ() = BBox.Min.GetZ(); 
    P[1].GetX() = BBox.Min.GetX();    P[1].GetY() = BBox.Min.GetY();    P[1].GetZ() = BBox.Max.GetZ(); 
    P[2].GetX() = BBox.Min.GetX();    P[2].GetY() = BBox.Max.GetY();    P[2].GetZ() = BBox.Min.GetZ(); 
    P[3].GetX() = BBox.Min.GetX();    P[3].GetY() = BBox.Max.GetY();    P[3].GetZ() = BBox.Max.GetZ(); 
    P[4].GetX() = BBox.Max.GetX();    P[4].GetY() = BBox.Min.GetY();    P[4].GetZ() = BBox.Min.GetZ(); 
    P[5].GetX() = BBox.Max.GetX();    P[5].GetY() = BBox.Min.GetY();    P[5].GetZ() = BBox.Max.GetZ(); 
    P[6].GetX() = BBox.Max.GetX();    P[6].GetY() = BBox.Max.GetY();    P[6].GetZ() = BBox.Min.GetZ(); 
    P[7].GetX() = BBox.Max.GetX();    P[7].GetY() = BBox.Max.GetY();    P[7].GetZ() = BBox.Max.GetZ(); 

    // Setup light direction in local space
    matrix4 W2L ;
    W2L = L2W ;
    W2L.InvertSRT() ;
    vector3 LightDir = W2L.RotateVector(WorldLightDir) ;
    LightDir.Normalize() ;

    // Draw bbox
    draw_SetL2W(L2W) ;
    draw_Begin( DRAW_QUADS, (LightColor.A == 255) ? 0 : DRAW_USE_ALPHA );
    navdraw_LitQuad(P[0], P[2], P[6], P[4], LightDir, LightDirI, LightAmbI, LightColor) ; // F
    navdraw_LitQuad(P[1], P[5], P[7], P[3], LightDir, LightDirI, LightAmbI, LightColor) ; // B
    navdraw_LitQuad(P[4], P[6], P[7], P[5], LightDir, LightDirI, LightAmbI, LightColor) ; // R
    navdraw_LitQuad(P[0], P[1], P[3], P[2], LightDir, LightDirI, LightAmbI, LightColor) ; // L
    navdraw_LitQuad(P[2], P[3], P[7], P[6], LightDir, LightDirI, LightAmbI, LightColor) ; // T
    navdraw_LitQuad(P[0], P[4], P[5], P[1], LightDir, LightDirI, LightAmbI, LightColor) ; // B
    draw_End();
    draw_ClearL2W() ;
}
#endif
//===========================================================================

void path_find_struct::RenderPath( f32 Radius )
{    
    s32 i;
    
    // Draw start and end labels
    draw_ClearL2W();
    draw_Label( m_vStartPoint, XCOLOR_WHITE, "Start" );
    draw_Label( m_vEndPoint,   XCOLOR_WHITE, "End" );

    // Draw straight line info
    vector3 Offset;
    Offset.Set( 0.0f, 30.0f, 0.0f );
    draw_Sphere( m_ClipLineStart + Offset, 10.0f, XCOLOR_GREEN );
    draw_Sphere( m_ClipLineEnd   + Offset, 10.0f, XCOLOR_BLUE );
    draw_Line  ( m_ClipLineStart + Offset, m_ClipLineEnd + Offset, XCOLOR_RED );
    draw_Label ( m_ClipLineStart + Offset, XCOLOR_WHITE, "ClipLineCalls:%d", m_nClipLineConnections );

    // Straight line?
    Offset.Set( 0.0f, 25.0f, 0.0f );
    if( m_nSteps == 0 )        
    {
        // Draw connections
        if( m_StepData[0].m_CurrentConnection != NULL_NAV_SLOT )
            draw_Connection( g_NavMap.GetConnectionByID( m_StepData[0].m_CurrentConnection ), 0, Offset, xcolor( 255, 0, 255, 128) );
        
        if( m_StepData[0].m_DestConnection != NULL_NAV_SLOT )
            draw_Connection( g_NavMap.GetConnectionByID( m_StepData[0].m_DestConnection ), 0, Offset, xcolor( 255, 0, 255, 128) );

        // Draw path
        draw_Line( m_vStartPoint, m_vEndPoint, XCOLOR_GREEN );
        draw_Sphere( m_vStartPoint, Radius, XCOLOR_YELLOW );
        draw_Sphere( m_vEndPoint,   Radius, XCOLOR_YELLOW );
        return;
    }

    // Render connections
    for( i = 0; i < m_nSteps; i++)
    {
        // Get connection
        ng_connection2& Con = g_NavMap.GetConnectionByID( m_StepData[i].m_CurrentConnection );

        // Draw it        
        if( i == 0 )
            draw_Connection( Con, i, Offset, xcolor( 255, 250, 0, 128) );
        else if( i == ( m_nSteps - 1 ) )            
            draw_Connection( Con, i, Offset, xcolor( 255, 250, 0, 128 ) );
        else            
            draw_Connection( Con, i, Offset, xcolor( 0, 255, 0, 128 ) );
    }

    // Render path pts
    vector3 MyPos = m_vStartPoint;
    draw_Sphere( MyPos, Radius, XCOLOR_GREEN );
    for( i = 0; i < m_nSteps; i++ )
    {
        vector3 vMoveTo;
        vector3 RemoteEnd;
        if (i < (m_nSteps))
        {
            RemoteEnd = g_NavMap.GetBestGuessDestination( m_StepData[i].m_CurrentConnection,
                m_StepData[i].m_DestConnection,
                MyPos,
                m_vEndPoint );
        }
        else
            RemoteEnd = m_vEndPoint;

        g_NavMap.GetClosestPointInOverlap( m_StepData[i].m_CurrentConnection, 
            m_StepData[i].m_DestConnection, 
            MyPos,
            RemoteEnd,
            Radius,
            vMoveTo );

        draw_Line  (MyPos,vMoveTo,XCOLOR_GREEN);
        draw_Marker( vMoveTo, XCOLOR_GREEN );
        draw_Label ( vMoveTo, XCOLOR_BLUE, "%d", i );

        MyPos = vMoveTo;
    }

    // Draw start and end pts
    draw_Sphere( m_vStartPoint, Radius, XCOLOR_YELLOW );
    draw_Sphere( m_vEndPoint,   Radius, XCOLOR_YELLOW );
}

#endif

//=============================================================================

nav_map::nav_map(void)  
{
    m_pOverlapData      = NULL;
    m_nOverlapData      = 0;

    m_pConnectivityData = NULL;
    m_nConnectivityData = 0;

    m_pConnectionData   = NULL;  
    m_nConnectionData   = 0;

    m_pLookupBox        = NULL;  
    m_nLookupBoxes      = 0;
    m_pLookupIndex      = NULL;  
    m_nLookupIndices    = 0;

    m_pOverlapVerts     = NULL;    
    m_nOverlapVerts     = 0;    

    m_pNode             = NULL;
    m_pConnection       = NULL;

#ifdef X_EDITOR
    m_pConnectionGuids  = NULL;
#endif // X_EDITOR

    m_nClipLineConnections = 0;

    Reset() ;
}

//=============================================================================
nav_map::~nav_map()
{
}


//=============================================================================
//
//  Reset
//
//      Resets the nav_map to a base state.  It also frees up all the memory
//      from the map.  
//
//=============================================================================
void nav_map::Reset(void)
{
    m_nOverlapData      = 0;
    m_nConnectivityData = 0 ;
    m_nConnectionData   = 0;
    m_nOverlapVerts     = 0;

    if(m_pOverlapData != NULL )
    {
        delete [] m_pOverlapData;
        m_pOverlapData              = NULL;
    }
    if(m_pConnectivityData != NULL )
    {
        delete [] m_pConnectivityData;
        m_pConnectivityData              = NULL;
    }
    if(m_pConnectionData != NULL )
    {
        delete [] m_pConnectionData;
        m_pConnectionData              = NULL;
    }
    if(m_pLookupBox != NULL )
    {
        delete [] m_pLookupBox;
        m_pLookupBox              = NULL;
    }
    if(m_pLookupIndex != NULL )
    {
        delete [] m_pLookupIndex;
        m_pLookupIndex              = NULL;
    }
    if(m_pOverlapVerts != NULL )
    {
        delete [] m_pOverlapVerts;
        m_pOverlapVerts              = NULL;
    }
    if(m_pConnection != NULL )
    {
        delete [] m_pConnection;
        m_pConnection              = NULL;
    }
    if(m_pNode != NULL )
    {
        delete [] m_pNode;
        m_pNode              = NULL;
    }

    m_GuidLookup.Clear();
    m_GuidLookup.SetCapacity( 32, TRUE );

#ifdef X_EDITOR
    m_bIsLoaded = FALSE ;
    if ( m_pConnectionGuids != NULL )
    {
        delete[] m_pConnectionGuids;
        m_pConnectionGuids = NULL;
    }
#endif // X_EDITOR

    s32 i;
    for (i=0;i<CONNECTION_QUERY_CACHE_SIZE;i++)
    {
        m_RecentConnections[ i ] = NULL_NAV_SLOT;
    }
    m_iNextRecentConnection = 0;
    m_RecentCacheHits       = 0;
    m_RecentCacheMisses     = 0;
    m_nClipLineConnections  = 0;
}

//=============================================================================
//
//  Init
//
//      Prepares the map to have data loaded into it
//
//
//=============================================================================
void nav_map::Init ( s32 nConnections, 
                     s32 nRequiredOverlaps, 
                     s32 nRequiredOverlapInfos, 
                     s32 nRequiredOverlapVerts )
{
    s32 i;

    (void)nRequiredOverlapInfos;
    m_pOverlapData      = new overlap_data[ nRequiredOverlaps ];
    m_nOverlapData      = nRequiredOverlaps;

    m_pConnectivityData = new connection2_connectivity_data[ nRequiredOverlaps*2 ];
    m_nConnectivityData = nRequiredOverlaps*2;

    m_pConnectionData   = new connection2_data[ nConnections ];
    m_nConnectionData   = nConnections;

    m_pOverlapVerts     = new overlap_vert[ nRequiredOverlapVerts ];
    m_nOverlapVerts     = nRequiredOverlapVerts;

    ASSERT( m_pOverlapData );
    ASSERT( m_pConnectivityData );
    ASSERT( m_pConnectionData );
    ASSERT( m_pOverlapVerts );

    //
    // Clear current lookup boxes
    //
    {
        if( m_pLookupBox )
        {
            delete[] m_pLookupBox;
            m_pLookupBox = NULL;
        }
        m_nLookupBoxes = 0;

        if( m_pLookupIndex )
        {
            delete[] m_pLookupIndex;
            m_pLookupIndex = NULL;
        }
        m_nLookupIndices = 0;
    }

#ifdef X_EDITOR
    m_pConnectionGuids  = new guid[ nConnections ];
    ASSERT( m_pConnectionGuids );
    for (i=0;i<nConnections;i++)
    {
        m_pConnectionGuids[i] = 0;
    }
#endif // X_EDITOR

    m_pConnection = new ng_connection2[ nConnections ];
    ASSERT( m_pConnection );

    if (nRequiredOverlaps > 0)
    {
        m_pNode       = new ng_node2[ nRequiredOverlaps ];
        ASSERT( m_pNode );
    }

          
    for (i=0;i<nConnections;i++)
    {
        m_pConnection[i].SetSlotID(i);
    }
    for (i=0;i<nRequiredOverlaps;i++)
    {
        m_pNode[i].SetSlotID(i);
    }
}

//=============================================================================

nav_map::connection2_data& nav_map::GetConnectionData( s32 iConnection )
{
    ASSERT(( iConnection >= 0) && ( iConnection < m_nConnectionData ));
    return m_pConnectionData[ iConnection ];
}

//=============================================================================

nav_map::overlap_data& nav_map::GetOverlapData( s32 iConnection )
{
    ASSERT(( iConnection >= 0) && ( iConnection < m_nOverlapData ));
    return m_pOverlapData[ iConnection ];
}

//=============================================================================

nav_map::connection2_connectivity_data&  nav_map::GetConnectivityData( s32 iConnection )
{   
    ASSERT(( iConnection >= 0) && ( iConnection < m_nConnectivityData ));
    return m_pConnectivityData[ iConnection ];
}

//=============================================================================
//
//  GetNeaestNode
//
//      Returns a pointer to the node nearest a point.
//
//      Temp version in place that checks against all nodes
//
//=============================================================================
nav_node_slot_id nav_map::GetNearestNode( const vector3 &ThisPoint )
{
    CONTEXT("nav_map::GetNearestNode") ;

    // until we have zone info, just using the slowest version possible.
    // just walk through the nodes and check lengths
    

    f32                 fBestDistSquared = F32_MAX ;
    nav_node_slot_id    iNearest         = NULL_NAV_SLOT ;
    s32                 i;

    for (i=0;i<m_nOverlapData;i++)
    {
        f32 thisDistance;

        thisDistance = (m_pOverlapData[ i ].m_Center - ThisPoint).LengthSquared();
        if(thisDistance < fBestDistSquared )
        {
            fBestDistSquared = thisDistance;
            iNearest = i;
        }
    }

    return iNearest;    
}

//=============================================================================
//
//  GetNeaestNode
//
//      Returns a pointer to the node nearest a point.
//
//      Temp version in place that checks against all nodes
//
//=============================================================================

//=============================================================================
//
//  GetNeaestNode
//
//      Returns a pointer to the node nearest a point.
//
//      Temp version in place that checks against all nodes
//
//=============================================================================
nav_node_slot_id nav_map::GetNearestWithIgnore(const vector3& Position, nav_node_slot_id* pNavNodes, s32 nNodes )
{
    CONTEXT("nav_map::GetNearestWithIgnore");

    f32                 fBestDistSquared = F32_MAX;
    nav_node_slot_id    iNearest         = NULL_NAV_SLOT;
    s32                 i;

    for ( i=0; i<m_nOverlapData; i++ )
    {
        f32 thisDistance;

        thisDistance = (m_pOverlapData[ i ].m_Center - Position).LengthSquared();

        xbool bInIgnore = FALSE ;
        if(thisDistance < fBestDistSquared )
        {
            for ( s32 j = 0; j < nNodes; j++ )
            {
                if ( i == pNavNodes[j] )
                {
                    bInIgnore = TRUE;
                    break;
                }
            }

            if ( !bInIgnore )
            {
                fBestDistSquared = thisDistance;
                iNearest = i;
            }
        }
    }

    return iNearest;    
}

/*
//=============================================================================
//
//  IsPatrolNode
//
//      Returns TRUE if node is a patrol node
//
//=============================================================================
xbool nav_map::IsPatrolNode( nav_node_slot_id NodeId )
{
    ng_node& Node = GetNodeByID(NodeId) ;

    // If connections are flagged as patrol routes, then this is a patrol node
    for (s32 i = 0 ; i < Node.GetConnectionCount() ; i++)
    {
        // Get connection
        ng_connection2& Connection = Node.GetConnectionByID(i) ;

        // Patrol?
        if (Connection.GetAIHints() & ng_connection2::HINT_PATROL_ROUTE)
            return TRUE ;
    }
    
    // No patrol connection found
    return FALSE ;
}

//=============================================================================
//
//  GetNeaestPatrolNode
//
//      Returns a pointer to the node nearest a point.
//
//      Temp version in place that checks against all nodes
//
//=============================================================================
nav_node_slot_id nav_map::GetNearestPatrolNode( const vector3 &thisPoint )
{
    CONTEXT("nav_map::GetNearestPatrolNode") ;

    // until we have zone info, just using the slowest version possible.
    // just walk through the nodes and check lengths


    f32 nearestDistanceSquared = F32_MAX ;
    nav_node_slot_id nearestNodeSlot = NULL_NAV_SLOT;
    s32 count;

    for (count =0; count < m_NodeCount; count++)
    {
        if (IsPatrolNode(count))
        {
            f32 thisDistance;

            thisDistance = (m_Node[count].GetPosition() - thisPoint).LengthSquared();
            if(thisDistance < nearestDistanceSquared )
            {
                nearestDistanceSquared = thisDistance;
                nearestNodeSlot = count;

            }
        }
    }

    return nearestNodeSlot;    
}
*/
//===========================================================================
//
//  GetNearestConnection
//
//      Returns the nearest connection to a point.  It looks up the zone that
//      the point we want to go from is in, and then checks the connections in
//      that zone.
//
//===========================================================================
nav_connection_slot_id  nav_map::GetNearestConnection(const vector3 &thisPoint )
{
    return GetNearestConnectionNoZoneCheck( thisPoint );
/*
    // First, find out what zone 'thisPoint' is in.
    u16 ZoneForPos = SMP_UTIL_GetZoneForPosition( thisPoint );
    (void)ZoneForPos;
    u16 Count = 0;
    u16* pConnectionList = NULL;

    // Now get a reference to the list of connections in the same zone as this one from connection_zone_mgr
    //pConnectionList = m_ConnectionZoneMgr.GetConnectionListForZone( (u8) ZoneForPos, Count );
    

    // if none found, brute force it.
    if( Count == 0 )
    {
        return GetNearestConnectionNoZoneCheck( thisPoint );        
    }

    f32 nearestDistance = F32_MAX ;
    nav_connection_slot_id nearestConnectionID = NULL_NAV_SLOT;

    for ( u16 i = 0; i < Count; i++ )
    {
        vector3 vPointOnConnection = GetNearestPointOnConnection( pConnectionList[i], thisPoint );
        f32 TestDistanceSqr = ( thisPoint - vPointOnConnection ).LengthSquared();
        if (  TestDistanceSqr < nearestDistance )
        {
            nearestDistance = TestDistanceSqr;
            nearestConnectionID = pConnectionList[i];
        }
    }
    
    return nearestConnectionID;
*/
}

//===========================================================================

nav_connection_slot_id  nav_map::GetNearestConnectionNoZoneCheck( const vector3& Point )
{
    return GetNearestConnectionInGrid( Point, 0xFF );
}

//===========================================================================

nav_connection_slot_id  nav_map::GetNearestConnectionInGrid( const vector3& Point, u8 GridID )
{       
    /* trap for null vector input ...
    if ( Point.LengthSquared() < 0.000001f )
    {
        x_DebugMsg("GetNearestConnectionInGrid being called with null vector\n");
    }
    */

    nav_connection_slot_id Result;
    xbool bInside;
    f32 Dist;
    
    s32                     i;
    f32                     BestInsideDist      = F32_MAX;
    nav_connection_slot_id  BestInsideSlotID    = NULL_NAV_SLOT;
    f32                     BestOutsideDist     = F32_MAX;
    nav_connection_slot_id  BestOutsideSlotID   = NULL_NAV_SLOT;
    f32                     BestDist            = F32_MAX;
    s32                     BestCacheI          = 0;

    // Check the cache to see if the point is inside one of those connections
    for (i=0;i<CONNECTION_QUERY_CACHE_SIZE;i++)
    {
        if (m_RecentConnections[i] != NULL_NAV_SLOT)
        {
            ng_connection2& Conn = GetConnectionByID( m_RecentConnections[i] );
            if( ((GridID==0xFF)||(Conn.GetGridID()==GridID))&&
                Conn.GetEnabled() && 
                Conn.IsPointInConnection(Point,0) )
            {
                m_RecentCacheHits++;

                // Compute dist from center of connection
                vector3 Center  = 0.5f * ( Conn.GetStartPosition() + Conn.GetEndPosition() );
                f32     CurDist = ( Point - Center ).LengthSquared();

                // Best so far?
                if( CurDist < BestInsideDist )
                {
                    // Keep
                    BestCacheI = i;
                    BestInsideDist   = CurDist;
                    BestInsideSlotID = m_RecentConnections[i];
                    BestDist = BestInsideDist;
                    bInside = TRUE;
                    Dist = CurDist;
                }
            }
        }
    }

    // Loop through connections and find anything closer
    if( BestInsideSlotID!=NULL_NAV_SLOT )
    {
        OptimizeConnectionCache(BestCacheI);
    }
    else
    for( i=0; i<m_nLookupBoxes; i++ )
    {
        connection_lookup_box& LB = m_pLookupBox[i];
        if( (GridID==0xFF) || (LB.m_iGrid == GridID) )
        {
            f32 ExternalSquaredDist = 0;
            xbool bPointInBBox = LB.m_BBox.Intersect(Point);
            if( bPointInBBox==FALSE )
            {
                // Check if external distance to bbox makes closer point possible.
                vector3 ClosestPtOnBBox = Point;
                ClosestPtOnBBox.GetX() = x_max( ClosestPtOnBBox.GetX(), LB.m_BBox.Min.GetX() );
                ClosestPtOnBBox.GetY() = x_max( ClosestPtOnBBox.GetY(), LB.m_BBox.Min.GetY() );
                ClosestPtOnBBox.GetZ() = x_max( ClosestPtOnBBox.GetZ(), LB.m_BBox.Min.GetZ() );
                ClosestPtOnBBox.GetX() = x_min( ClosestPtOnBBox.GetX(), LB.m_BBox.Max.GetX() );
                ClosestPtOnBBox.GetY() = x_min( ClosestPtOnBBox.GetY(), LB.m_BBox.Max.GetY() );
                ClosestPtOnBBox.GetZ() = x_min( ClosestPtOnBBox.GetZ(), LB.m_BBox.Max.GetZ() );
                ExternalSquaredDist = (ClosestPtOnBBox - Point).LengthSquared();
            }

            // If we are too far from the bbox then skip everything in the bbox
            if( (ExternalSquaredDist <= BestDist) && (bPointInBBox || (BestInsideSlotID == NULL_NAV_SLOT)) )
            {

                // Loop through connections in box
                s32* pConnI = &m_pLookupIndex[LB.m_iConnection];
                s32 j;
                for( j=0; j<LB.m_nConnections; j++ )
                {
                    // Get the connection.
                    s32 ConnI = pConnI[j];
                    ng_connection2& Conn = GetConnectionByID(ConnI);
    
                    // Inside connection?
                    if( bPointInBBox && 
                        Conn.GetEnabled() && 
                        Conn.IsPointInConnection( Point, 0 ) )
                    {
                        // Compute dist from center of connection
                        vector3 Center = 0.5f * ( Conn.GetStartPosition() + Conn.GetEndPosition() );
                        f32     CurDist   = ( Point - Center ).LengthSquared();

                        // Closest inside so far?
                        if( CurDist < BestInsideDist )
                        {
                            // Record
                            BestInsideDist   = CurDist;
                            BestInsideSlotID = ConnI;
                            BestDist         = BestInsideDist;
                        }
                    }
                    else if( BestInsideSlotID == NULL_NAV_SLOT &&
                             Conn.GetEnabled() )
                    {
                        // Get distance from connection
                        vector3 PointOnConnection = GetNearestPointOnConnection( ConnI, Point );
                        f32     CurDist           = ( Point - PointOnConnection ).LengthSquared();
                    
                        // Closest outside so far?
                        if( CurDist < BestOutsideDist )
                        {
                            // Record
                            BestOutsideDist   = CurDist;
                            BestOutsideSlotID = ConnI;
                            BestDist = BestOutsideDist;
                        }
                    }
                }
            }
        }
    }

    if( BestInsideSlotID != NULL_NAV_SLOT )
    {
        Result  = BestInsideSlotID;
        AddToConnectionCache( Result );
        bInside = TRUE;
        Dist    = BestInsideDist;
    }
    else
    {
        Result  = BestOutsideSlotID;
        bInside = FALSE;
        Dist    = BestOutsideDist;
    }
 
    return Result;
}

//===========================================================================

xbool nav_map::GetConnectionContainingPoint( nav_connection_slot_id& connectionList, const vector3& testPoint )
{
    //
    // Faster version using lookup boxes
    //
    xbool Result=FALSE;
    nav_connection_slot_id ConnectionSlot=0xFFFF;
    s32  i;

    // Start running through the bboxes looking for the first one that contains
    // the point
    s32 iFirstBBox;
    for( iFirstBBox=0; iFirstBBox<m_nLookupBoxes; iFirstBBox++ )
    if( m_pLookupBox[iFirstBBox ].m_BBox.Intersect(testPoint) )
    {
        break;
    }

    // If no bbox was found then we know it can't be found.
    if( iFirstBBox!=m_nLookupBoxes )
    {    
        // Check the cache to see if the point is inside one of those connections
        for (i=0;i<CONNECTION_QUERY_CACHE_SIZE;i++)
        {
            if (m_RecentConnections[i] != NULL_NAV_SLOT)
            {
                ng_connection2& Conn = GetConnectionByID( m_RecentConnections[i] );
                if( Conn.GetEnabled() && 
                    Conn.m_ConnectionData->m_BBox.Intersect(testPoint) &&
                    Conn.IsPointInConnection(testPoint,0) )
                {
                    m_RecentCacheHits++;                    
                    connectionList = m_RecentConnections[i];
                    ConnectionSlot = connectionList;

                    // optimize cache only after we grab the connection ID from the cache at i
                    OptimizeConnectionCache(i);
                    Result = TRUE;
                    break;
                }
            }
        }
        
    
        if( !Result )
        {
            for( i=iFirstBBox; i<m_nLookupBoxes; i++ )
            if( m_pLookupBox[i].m_BBox.Intersect(testPoint) )
            {
                connection_lookup_box& LB = m_pLookupBox[i];
                s32* pConnI = &m_pLookupIndex[LB.m_iConnection];
                s32 j;
                for( j=0; j<LB.m_nConnections; j++ )
                {
                    // Get the connection.
                    s32 ConnI = pConnI[j];
                    ng_connection2& Conn = GetConnectionByID(ConnI);
                    if( Conn.GetEnabled() && 
                        Conn.m_ConnectionData->m_BBox.Intersect(testPoint) &&
                        Conn.IsPointInConnection(testPoint) )
                    {
                        AddToConnectionCache( ConnI );
                        connectionList = ConnI;
                        ConnectionSlot = connectionList;
                        Result = TRUE;
                        break;
                    }
                }
                if( Result )
                    break;
            }
        }
    }

    connectionList = ConnectionSlot;
    return Result;
}


//=============================================================================
//  FILE VERSION!
//
//      Increase this number when the binary file format changes.
//      We will keep all the old versions of the loading code in place but 
//      only the newest version of the writing code.
//  
//=============================================================================

const s32 k_NAV_FILE_VERSION = 107;
// Version 100 was first version
// Version 101 added GridID to nodes
// Version 102 sorting connections by zone for faster searching.
// Version 103 New nav version2 system - mostly a rewrite
// Version 104 Added grid linking info
// Version 105 Added guids to support (dis/en)abling nav connections
// Version 106 is with the new vector3 so data layout changed
// Version 107 added new lookup_boxes for faster queries
//=============================================================================
//
//  Load
//
//      This version loads with file pointer.  Logic is pretty simple, just 
//      load the chunks then call the inits for the objects
//
//=============================================================================
void nav_map::Load ( X_FILE *filePointer )
{

    MEMORY_OWNER( "NAVMAP DATA" );

    ASSERT(filePointer);
    s32 nOverlapData, nConnectivityData, nConnectionData, nOverlapVerts;
    s32 fileVersion;

    //  Check fithe file version.  As we get new file versions we'll call the load
    //  based on the version.  The save always saves as newest format though
    x_fread( &fileVersion,      sizeof(fileVersion),        1,  filePointer );
    if(fileVersion == k_NAV_FILE_VERSION )
    {
        s32 readSize;
        readSize = x_fread( &nOverlapData,        sizeof(nOverlapData),     1,  filePointer );
        ASSERT(readSize == 1);

        readSize = x_fread( &nConnectivityData,   sizeof(nConnectivityData),1,  filePointer );
        ASSERT( readSize == 1 );

        readSize = x_fread( &nConnectionData,     sizeof(nConnectionData),  1,  filePointer );
        ASSERT( readSize == 1 );

        s32 nLookupBoxes;
        s32 nLookupIndices;

        readSize = x_fread( &nLookupBoxes,      sizeof(m_nLookupBoxes),  1,  filePointer );
        ASSERT( readSize == 1 );

        readSize = x_fread( &nLookupIndices,    sizeof(m_nLookupIndices),  1,  filePointer );
        ASSERT( readSize == 1 );

        readSize = x_fread( &nOverlapVerts,       sizeof(nOverlapVerts),    1,  filePointer );
        ASSERT( readSize == 1 );

        //m_ConnectionZoneMgr.LoadCountData( filePointer );


        //  Got the number of each major data element, time to Init our data structures
        Init ( nConnectionData, nOverlapData, nConnectivityData, nOverlapVerts );
        m_nLookupBoxes = nLookupBoxes;
        m_nLookupIndices = nLookupIndices;

        //  Allocate lookupboxes and indices
        m_pLookupBox = new connection_lookup_box[ m_nLookupBoxes ];
        m_pLookupIndex = new s32[ m_nLookupIndices ];
        
        //  Time to read in the actual data in 3 big chunks
        readSize = x_fread( m_pOverlapData,         sizeof(overlap_data),                   m_nOverlapData,         filePointer );
        ASSERT( readSize == m_nOverlapData );

        readSize = x_fread( m_pConnectivityData,    sizeof(connection2_connectivity_data),  m_nConnectivityData,    filePointer );
        ASSERT( readSize == m_nConnectivityData  );

        readSize = x_fread( m_pConnectionData,      sizeof(connection2_data),               m_nConnectionData,      filePointer );
        ASSERT( readSize ==  m_nConnectionData  );

        readSize = x_fread( m_pLookupBox,      sizeof(connection_lookup_box),               m_nLookupBoxes,      filePointer );
        ASSERT( readSize ==  m_nLookupBoxes  );

        readSize = x_fread( m_pLookupIndex,      sizeof(s32),                               m_nLookupIndices,      filePointer );
        ASSERT( readSize ==  m_nLookupIndices  );

        readSize = x_fread( m_pOverlapVerts,        sizeof(overlap_vert),                   m_nOverlapVerts,        filePointer );
        ASSERT( readSize ==  m_nOverlapVerts  );

        if (m_nConnectionData > 0)
        {
            guid*   pTempGuids = new guid[ m_nConnectionData ];
            readSize = x_fread( pTempGuids,     sizeof(guid),                           m_nConnectionData,      filePointer );
            ASSERT( readSize ==  m_nConnectionData  );

            s32 i;
            for (i=0;i<m_nConnectionData;i++)
            {
                m_GuidLookup.Add( pTempGuids[i], i );
            }
            delete[] pTempGuids;
        }

        //m_ConnectionZoneMgr.LoadListData( filePointer );

        // As this point we should have valid data and we need to set the data
        // for the node and connection classes

        SetData();
    }
    else
    {
        x_DebugMsg("Wrong map version.  Please export data with latest editor\n");
    }

#ifdef X_EDITOR
    m_bIsLoaded = TRUE ;
#endif // X_EDITOR

}

//=============================================================================
//
//  Load
//
//      Loads using a file name and x_fread.  Soon to be replaced by fileio
//      version.  Use for debugging only.  This version just opents the file
//      and calls the Load with file pointer version
//
//=============================================================================
void nav_map::Load ( const char* fileName )
{
    X_FILE* tempFile;
    tempFile = x_fopen( xfs("%s.new",fileName ), "rb" );
    if(tempFile)
    {
    
        Load(tempFile);
        x_fclose(tempFile);
    }

    
}

//=============================================================================
//
//  Save
//
//      Saves the file data.   Just as simple as it looks.  Just writes out
//      the data chunks that aren't calculated on the fly.
//
//=============================================================================
#ifdef X_EDITOR

void nav_map::Save ( X_FILE *filePointer )
{
    s32 writeSize;
    ASSERT(filePointer);
    writeSize = x_fwrite( &k_NAV_FILE_VERSION,  sizeof(k_NAV_FILE_VERSION),     1,  filePointer );
    ASSERT(writeSize == 1 );
    
    writeSize = x_fwrite( &m_nOverlapData,      sizeof(m_nOverlapData),         1,  filePointer );
    ASSERT(writeSize == 1 );

    writeSize = x_fwrite( &m_nConnectivityData, sizeof(m_nConnectivityData),    1,  filePointer );
    ASSERT( writeSize == 1 );

    writeSize = x_fwrite( &m_nConnectionData,   sizeof(m_nConnectionData),      1,  filePointer );
    ASSERT( writeSize == 1 );

    writeSize = x_fwrite( &m_nLookupBoxes,   sizeof(m_nLookupBoxes),      1,  filePointer );
    ASSERT( writeSize == 1 );

    writeSize = x_fwrite( &m_nLookupIndices,   sizeof(m_nLookupIndices),      1,  filePointer );
    ASSERT( writeSize == 1 );

    writeSize = x_fwrite( &m_nOverlapVerts,     sizeof(m_nOverlapVerts),        1,  filePointer );
    ASSERT( writeSize == 1 );

    //m_ConnectionZoneMgr.WriteCountData( filePointer );

    writeSize = x_fwrite( m_pOverlapData,       sizeof(overlap_data),           m_nOverlapData,     filePointer );
    ASSERT( writeSize == m_nOverlapData );

    writeSize = x_fwrite( m_pConnectivityData,  sizeof(connection2_connectivity_data),    m_nConnectivityData,      filePointer );
    ASSERT( writeSize == m_nConnectivityData );

    writeSize = x_fwrite( m_pConnectionData,    sizeof(connection2_data),       m_nConnectionData,  filePointer );
    ASSERT( writeSize == m_nConnectionData );

    writeSize = x_fwrite( m_pLookupBox,    sizeof(connection_lookup_box),       m_nLookupBoxes,  filePointer );
    ASSERT( writeSize == m_nLookupBoxes );

    writeSize = x_fwrite( m_pLookupIndex,    sizeof(s32),       m_nLookupIndices,  filePointer );
    ASSERT( writeSize == m_nLookupIndices );

    writeSize = x_fwrite( m_pOverlapVerts,      sizeof(overlap_vert),           m_nOverlapVerts,    filePointer );
    ASSERT( writeSize == m_nOverlapVerts );

    writeSize = x_fwrite( m_pConnectionGuids,   sizeof(guid),                   m_nConnectionData,  filePointer );
    ASSERT( writeSize == m_nConnectionData );
    
    //m_ConnectionZoneMgr.WriteListData( filePointer );
   

}

#endif // X_EDITOR

//=============================================================================

void nav_map::StoreDynamicData( bitstream& BitStream )
{
    // Write the data version number
    BitStream.WriteS32( k_NAV_FILE_VERSION );

    // Write count
    BitStream.WriteS32( m_nConnectivityData );

    // Write data
    s32 i;
    for( i=0; i<m_nConnectivityData; i++ )
    {
        BitStream.WriteU16( m_pConnectivityData[i].m_iRemoteConnection );
        BitStream.WriteU16( m_pConnectivityData[i].m_iOverlapData );
    }

    BitStream.WriteVariableLenS32( m_nConnectionData );
    for ( i=0;i<m_nConnectionData;i++)
    {
        BitStream.WriteVariableLenU32( m_pConnectionData[i].m_Flags );
    }
}

//=============================================================================

void nav_map::RestoreDynamicData( bitstream& BitStream )
{
    // Read the data version number
    s32 Version;
    BitStream.ReadS32( Version );

    // Read count
    s32 Count;
    BitStream.ReadS32( Count );

    // check version
    if ( Version != k_NAV_FILE_VERSION )
    {
        // Oops!
        x_DebugMsg("Save data has wrong map version.\n");
        
        // Skip data
        BitStream.SetCursor( BitStream.GetCursor() + (Count*4*8) );
    }

    if( Count != m_nConnectivityData )
    {
        // different number of nav maps - skip data
        x_DebugMsg("Save data has wrong number of nav maps.\n");

        // Skip data
        BitStream.SetCursor( BitStream.GetCursor() + (Count*4*8) );
    }

    // read in the data!
    s32 i;
    for(  i=0; i<m_nConnectivityData; i++ )
    {
        BitStream.ReadU16( m_pConnectivityData[i].m_iRemoteConnection );
        BitStream.ReadU16( m_pConnectivityData[i].m_iOverlapData );
    }


    // Read in the flags
    BitStream.ReadVariableLenS32( Count );
    if (Count != m_nConnectionData)
    {
        // different number of nav maps - skip data
        ASSERT("Save data has wrong number of nav connections.");
        BREAK;
    }

    for ( i=0;i<m_nConnectionData;i++)
    {
        BitStream.ReadVariableLenU32( m_pConnectionData[i].m_Flags );
    }
}

//=============================================================================

void nav_map::DumpDynamicData( bitstream& BitStream )
{
    x_DebugMsg( 0, "------------------------\n" );
    x_DebugMsg( 0, "       NAV DATA         \n" );
    x_DebugMsg( 0, "------------------------\n" );

    // Read the data version number
    s32 Version;
    BitStream.ReadS32( Version );
    x_DebugMsg( 0, "Version = %d\n", Version );

    // Read count
    s32 Count;
    BitStream.ReadS32( Count );
    x_DebugMsg( 0, "Count = %d\n", Count );

    // check version
    if ( Version != k_NAV_FILE_VERSION )
    {
        // Oops!
        x_DebugMsg("Save data has wrong map version.\n");
        
        // Skip data
        BitStream.SetCursor( BitStream.GetCursor() + (Count*4*8) );
    }

    if( Count != m_nConnectivityData )
    {
        // different number of nav maps - skip data
        x_DebugMsg("Save data has wrong number of nav maps.\n");

        // Skip data
        BitStream.SetCursor( BitStream.GetCursor() + (Count*4*8) );
    }

    // read in the data!
    for( s32 i=0; i<m_nConnectivityData; i++ )
    {
        x_DebugMsg( 0, "------------------------\n" );

        u16 RemoteConnection;
        BitStream.ReadU16( RemoteConnection );
        x_DebugMsg( 0, "Remote Connection = %d\n", RemoteConnection );
        
        u16 OverlapData;
        BitStream.ReadU16( OverlapData );
        x_DebugMsg( 0, "Overlap Data = %d\n", OverlapData );
    }
}

//=============================================================================

void nav_map::SetData( void )
{
    s32 count;
    for(count = 0; count < m_nOverlapData; count++ )
    {
        m_pNode[ count ].Init(&(m_pOverlapData[count]), this, count );
    }

    for(count = 0; count < m_nConnectionData; count++ )
    {
        m_pConnection[ count ].Init(&(m_pConnectionData[count]), this, count);
        m_pConnection[ count ].CalculateLength();
    }
}

//=============================================================================
/*
void nav_map::RenderExportedMap( void )
{
#ifdef TARGET_PC
    ASSERT( m_bIsLoaded ) ;
//	RenderAllNodes() ;
	RenderAllConnections() ;
#endif
}
*/
//=============================================================================
/*
void nav_map::RenderAllNodes( void )
{
    for ( s32 i = 0; i < m_NodeCount; i++ )
    {
        vector3 vPos = m_NodeData[i].m_Position ;
        draw_Sphere( vPos, 10.f, XCOLOR_AQUA ) ;
    }
}
*/
//=============================================================================
/*
void nav_map::RenderAllConnections( void )
{
    
    xcolor  CurrentColor( 0, 0, 0, 150 ) ;

    for( s32 i = 0; i < m_ConnectionCount; i++ )
    {
        ng_node& N0 = GetNodeByID( m_ConnectionData[i].m_Node[0] ) ;
        ng_node& N1 = GetNodeByID( m_ConnectionData[i].m_Node[1] ) ;

        u8 GridID = N1.GetGridID() ;

        s32 ColorDeterminer = GridID % 3 ;
        s32 IntensityDeterminer = 255;

        switch( ColorDeterminer )
        {
            case 0:
                CurrentColor.R += IntensityDeterminer ;
                break ;

            case 1:
                CurrentColor.G += IntensityDeterminer ;
                break ;
        
            case 2:
                CurrentColor.B += IntensityDeterminer ;
                break ;
        
            default:
                break ;
        }



        vector3 P1 = N0.GetPosition() ;
        vector3 P2 = N1.GetPosition() ;

        draw_Volume( P1, P2, m_ConnectionData[i].m_Width, 1.f, CurrentColor ) ;
    }
}
*/

//===========================================================================

xbool nav_map::IsPointInGrid( const vector3& testPoint, u8 testGrid )
{
    // Check the cache to see if the point is inside one of those connections
    s32 i;
    for (i=0;i<CONNECTION_QUERY_CACHE_SIZE;i++)
    {
        if (m_RecentConnections[i] != NULL_NAV_SLOT)
        {
            ng_connection2& Conn = GetConnectionByID( m_RecentConnections[i] );
            if  (    (Conn.GetGridID() == testGrid)
                  && (Conn.GetEnabled()) )
            {
                if (Conn.IsPointInConnection( testPoint, 0 ))
                {
                    m_RecentCacheHits++;
                    return TRUE;
                }
            }
        }
    }


    for ( i = 0; i < GetConnectionCount(); i++ )
    {
        ng_connection2& Conn = GetConnectionByID( i );

        if (!Conn.GetEnabled() || Conn.GetGridID() != testGrid )
            continue;

        if( Conn.IsPointInConnection(testPoint), 0.0f )
        {
            AddToConnectionCache( i );
            return TRUE;
        }
    }
    return FALSE;
}

//===========================================================================

xbool nav_map::IsPointInMap( const vector3& testPoint )
{	
    nav_connection_slot_id ConnID;
    return GetConnectionContainingPoint( ConnID, testPoint );
}

//===========================================================================

vector3 nav_map::GetNearestPointInNavMap( const vector3& thisPoint )
{
    nav_connection_slot_id C = GetNearestConnection( thisPoint );

    if (NULL_NAV_SLOT == C)
        return thisPoint;

    return GetNearestPointOnConnection( C, thisPoint );
}

//===========================================================================

vector3 nav_map::GetInfoForNearestPointInNavmap  ( const vector3&          thisPoint,
                                                nav_connection_slot_id& ClosestConnection,
                                                nav_node_slot_id&       ClosestNode )
{
    ClosestConnection = GetNearestConnection( thisPoint );
    ClosestNode = GetNearestNode( thisPoint );

    vector3 Nearest = GetNearestPointOnConnection( ClosestConnection, thisPoint );
   
    return Nearest;
}

//===========================================================================
#if !defined( CONFIG_RETAIL )

void nav_map::RenderNavigationSpine( void )
{
    s32 i,j;

    for (i=0;i<m_nConnectionData;i++)
    {
        ng_connection2& Conn = g_NavMap.GetConnectionByID( i );
        
        draw_Line( m_pConnectionData[i].m_StartPt + vector3(0,50,0), 
                   m_pConnectionData[i].m_EndPt   + vector3(0,50,0), 
                   XCOLOR_PURPLE );

        vector3 Center = m_pConnectionData[i].m_StartPt +
                         m_pConnectionData[i].m_EndPt;
        Center.Scale(0.5f);
        
        if (g_AIMgr.GetShowConnectionIDs())
            draw_Label( Center, XCOLOR_YELLOW, "ID:%d",i);

        for (j=0;j<Conn.GetOverlapCount();j++)
        {
            nav_node_slot_id OID = Conn.GetOverlapNodeID(j);
            overlap_data& OD = g_NavMap.GetOverlapData( OID );
                       
            vector3 Pt = OD.m_Center;
            
            vector3 A,B;
            
            A = Conn.GetStartPosition();
            B = Conn.GetEndPosition();

            A += vector3(0,50,0);
            B += vector3(0,50,0);
            
            vector3 Dir = B-A;
            vector3 P = Pt-A;
            vector3 N;

            f32 T = P.Dot(Dir) / x_sqr(Dir.Length());

            if (T > 1)
                N = B;
            else if (T < 0)
                N = A;
            else
            {
                Dir.Scale(T);
                N = A + Dir;
            }

            draw_Line( Pt, N, XCOLOR_RED );
        }
    }

    for (i=0;i<m_nOverlapData;i++)
    {
        overlap_data&   OD = m_pOverlapData[ i ];
        xcolor Clr;

        for (j=0;j<OD.m_nOverlapPts;j++)
        {
            u16 Flags = m_pOverlapVerts[ OD.m_iFirstOverlapPt + j ].m_Flags;
            
            Clr = XCOLOR_PURPLE;

            if (Flags & overlap_vert::FLAG_OUTSIDE)
                Clr = XCOLOR_RED;

            draw_Sphere( m_pOverlapVerts[ OD.m_iFirstOverlapPt + j ].m_Pos, 10, Clr );
        }

        draw_Sphere( OD.m_Center, 10, XCOLOR_GREEN);

        draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA | DRAW_CULL_NONE);
        draw_Color(255,0,40,128);
        for (j=0;j<OD.m_nOverlapPts-1;j++)
        {
            draw_Vertex( m_pOverlapVerts[ OD.m_iFirstOverlapPt + 0 ].m_Pos );
            draw_Vertex( m_pOverlapVerts[ OD.m_iFirstOverlapPt + j ].m_Pos );
            draw_Vertex( m_pOverlapVerts[ OD.m_iFirstOverlapPt + j+1 ].m_Pos );
        }
        draw_End();
    }
}

#endif // !defined( CONFIG_RETAIL )

//===========================================================================

vector3 nav_map::GetNearestPointOnConnection( nav_connection_slot_id iConnection, const vector3& vPointToCheck )
{
    if (NULL_NAV_SLOT == iConnection)
        return vPointToCheck;

    // Get the connection.
    ASSERT(iConnection<m_nConnectionData);

    connection2_data* pConnectionData = &m_pConnectionData[iConnection];

    // Here is the method used already.  Just giving back a point.
    vector3 startPoint, endPoint;
    startPoint = pConnectionData->m_StartPt;
    endPoint   = pConnectionData->m_EndPt;

    // This will get the point of intersetion to the line between the nav nodes.
    vector3 vIntersectionPoint = vPointToCheck.GetClosestPToLSeg( startPoint, endPoint );

    // Now we have the edge, we need to find the exact point on the edge.
    vector3 vY(0.f, 1.f, 0.f );
    vector3 vStartToEndDir = endPoint - startPoint;
    vStartToEndDir.Normalize();

    vector3 vCross = vY.Cross( vStartToEndDir );
    vCross.Scale( pConnectionData->m_Width );

    // New start and end for next calulation.
    startPoint = vIntersectionPoint + vCross;
    endPoint = vIntersectionPoint - vCross;

    vIntersectionPoint = vPointToCheck.GetClosestPToLSeg( startPoint, endPoint );

    return vIntersectionPoint;
}

//===========================================================================

xbool nav_map::DoOverlap( nav_connection_slot_id A, nav_connection_slot_id B, nav_node_slot_id* pNodeID )
{
    if (A == NULL_NAV_SLOT)
        return FALSE;
    if (B == NULL_NAV_SLOT)
        return FALSE;
    ASSERT(A<m_nConnectionData);
    ASSERT(B<m_nConnectionData);

    connection2_data& Data = GetConnectionData( A );

    s32 i;
    for (i=0;i<Data.m_nOverlaps;i++)
    {
        if (GetConnectivityData( Data.m_iFirstConnectivity + i).m_iRemoteConnection == B)
        {
            if (pNodeID)
                *pNodeID = GetConnectivityData( Data.m_iFirstConnectivity + i).m_iOverlapData;
            return TRUE;
        }
    }
    return FALSE;
}

//===========================================================================

xbool nav_map::GetClosestPointInOverlap(   nav_connection_slot_id SrcSlot, 
                                           nav_connection_slot_id DestSlot, 
                                           const vector3&         thisPoint, 
                                           const vector3&         destPoint,
                                           f32                    characterWidth,
                                           vector3&               ClosestPoint )
{
    ClosestPoint = destPoint;

    nav_node_slot_id    iOverlap = NULL_NAV_SLOT;

    if (!g_NavMap.DoOverlap( SrcSlot, DestSlot, &iOverlap ))
        return FALSE;

    if( SrcSlot == DestSlot )
    {
        return TRUE;
    }

    if( SrcSlot == NULL_NAV_SLOT )
    {
        ClosestPoint = GetNearestPointOnConnection( DestSlot,thisPoint);
        return TRUE;
    }  
    if( DestSlot == NULL_NAV_SLOT )
    {
        ClosestPoint = GetNearestPointOnConnection( SrcSlot,thisPoint);
        return TRUE;
    }  
    
    // Ok, so 1) they do overlap
    //        2) neither are NULL_NAV_SLOT
    //
    
    nav_map::overlap_vert*  pOverlapVerts = &(m_pOverlapVerts[ m_pOverlapData[ iOverlap ].m_iFirstOverlapPt ]);
    s32                     nOverlapVerts = m_pOverlapData[ iOverlap ].m_nOverlapPts;

    // We have the clipped points in world space.  Now just run some point to line tests to find the closest one.
    ClosestPoint = GetClosestPointToLine( pOverlapVerts, nOverlapVerts, thisPoint, destPoint );

    // ok, we now need to check the point's distance to each overlap pt that is an outside point. 
    // If too close, move us towards the center.
    s32 c;
    for(c=0;c<nOverlapVerts;c++)
    {
        vector3 vertToPoint = pOverlapVerts[c].m_Pos - ClosestPoint;
        if( vertToPoint.LengthSquared() <= characterWidth * characterWidth )
        {
            // we are too close to this vert, we must move away. 
            vector3 centerToPoint = ClosestPoint - m_pOverlapData[ iOverlap ].m_Center;
            vector3 centerToVert = pOverlapVerts[c].m_Pos - m_pOverlapData[ iOverlap ].m_Center;
            if( centerToVert.LengthSquared() <= characterWidth * characterWidth )
            {
                ClosestPoint =  m_pOverlapData[ iOverlap ].m_Center;
                break;
            }
            else
            {
                f32 movePointAmount = characterWidth - vertToPoint.Length();
                f32 lengthToCenter = centerToPoint.Length();
                lengthToCenter -= movePointAmount;
                ASSERT( lengthToCenter > 0.0f );                    
                centerToPoint.NormalizeAndScale(lengthToCenter);
                ClosestPoint = m_pOverlapData[ iOverlap ].m_Center + centerToPoint;
            }
        }
    }
    return TRUE;
}

//===========================================================================

vector3 nav_map::GetClosestPointToLine( nav_map::overlap_vert* pVerts, s32 nVerts, const vector3& vStart, const vector3& vEnd )
{
    if ( nVerts == 0 )
    {
        // This means that there is no intersection between the connections.  Bad news.
        ASSERT( FALSE );
        return vStart;
    }

    if ( nVerts == 1 )
    {
        // Only one intersection, return this point.  I don't think that this is possible.
        return pVerts[0].m_Pos;
    }

    // Now for some point to line checks.
    f32     fClosestDist = F32_MAX;
    vector3 vClosestPoint = pVerts[0].m_Pos;

    for ( s32 i = 0; i < nVerts; i++ )
    {
        //f32 fTestDist = vPos.GetSqrtDistToLineSeg( pVerts[i], pVerts[(i+1)%nVerts] );
        vector3 Pos = pVerts[i].m_Pos;
        f32 fTestDist = Pos.GetSqrtDistToLineSeg( vStart, vEnd );
        if ( fTestDist < fClosestDist )
        {
            fClosestDist = fTestDist;
            vClosestPoint = pVerts[i].m_Pos;
        }
        
        // Test for intersection
        //
        //  Ref: http://www.exaflop.org/docs/cgafaq/cga1.html 
        //         Subject 1.03
        //
        //  A     D
        //    \ /
        //     X
        //   /   \
        // C      B
        vector3 A,B,C,D;
        A = pVerts[i].m_Pos;
        B = pVerts[(i+1)%nVerts].m_Pos;
        C = vStart;
        D = vEnd;

        f32 denom = (B.GetX()-A.GetX())*(D.GetZ()-C.GetZ())-(B.GetZ()-A.GetZ())*(D.GetX()-C.GetX());
            
        // if denom == 0, lines are parallel
        if (denom != 0)
        {
            // Not parallel
            f32 numA = (A.GetZ()-C.GetZ())*(D.GetX()-C.GetX())-(A.GetX()-C.GetX())*(D.GetZ()-C.GetZ());
            f32 numB = (A.GetZ()-C.GetZ())*(B.GetX()-A.GetX())-(A.GetX()-C.GetX())*(B.GetZ()-A.GetZ());

            if (x_abs(numA) < 0.0001f )
            {
                //lines are coincident, so let's just return a point on the line
                vClosestPoint = pVerts[i].m_Pos;
                break;
            }

            f32 rA = numA / denom;
            f32 rB = numB / denom;

            if ((rA>=0) && (rA<=1) && (rB>=0) && (rB<=1))
            {
                // We have an intersection
                vector3 Pt = B-A;
                Pt.Scale( rA );
                Pt += A;

                vClosestPoint = Pt;
                break;
            }
        }
    }
/*
#if (defined TARGET_PC) && (defined shird)
    if( eng_Begin() )
    {
        draw_Line( vStart, vClosestPoint );
        eng_End();
    }
#endif
    */
    return vClosestPoint;
}

//===========================================================================

struct FindConnStruct
{
    nav_connection_slot_id  connectionID;
    nav_node_slot_id        nodeID;
};

/*
nav_node_slot_id nav_map::FindOverlapAtDistanceFromPoint(   f32 desiredDistance, 
                                                            vector3 fleeFromLocation, 
                                                            vector3 startLocation,
                                                            nav_connection_slot_id startConnection )
{    
    priority_queue<FindConnStruct,f32,64> connectionQueue;

    FindConnStruct firstStruct;
    firstStruct.connectionID    = startConnection;
    firstStruct.nodeID          = NULL_NAV_SLOT;

    connectionQueue.Push( firstStruct, -1.0f );

    nav_connection_slot_id  currentConnID   = NULL_NAV_SLOT;
    nav_node_slot_id        currentNodeID   = NULL_NAV_SLOT;
    nav_node_slot_id        finalNodeID     = NULL_NAV_SLOT;   

    while(finalNodeID == NULL_NAV_SLOT && !connectionQueue.IsEmpty() )
    {
        // get the best connection and pop it off.
        FindConnStruct currentFindConnStruct = connectionQueue.Pop();
        currentConnID = currentFindConnStruct.connectionID;
        currentNodeID = currentFindConnStruct.nodeID;
        
        ng_connection2 currentConnection = g_NavMap.GetConnectionByID( currentConnID );

        f32 currentNodeToTargetDist = 0.0f;
        if( currentNodeID != NULL_NAV_SLOT )
        {
            ng_node2 currentNode = g_NavMap.GetNodeByID( currentNodeID );
            currentNodeToTargetDist = ( fleeFromLocation - currentNode.GetPosition() ).LengthSquared();
        }

        s32 c;
        for(c=0;c<currentConnection.GetOverlapCount();c++)
        {
            nav_node_slot_id nextNodeID = currentConnection.GetOverlapNodeID( c );
            if( nextNodeID == NULL_NAV_SLOT || nextNodeID == currentNodeID )
            {
                continue;
            }
            ng_node2 nextNode = g_NavMap.GetNodeByID( nextNodeID );
            f32 nextNodeToTargetDistance = ( fleeFromLocation - nextNode.GetPosition()).LengthSquared();
            if( nextNodeToTargetDistance <= currentNodeToTargetDist )
            {
                continue;
            }
            // first check to see if we have achieved our goal.
            if( nextNodeToTargetDistance >= desiredDistance * desiredDistance )
            {
                finalNodeID = nextNodeID;
            }
            else
            {            
                vector3 toTarget    = fleeFromLocation - startLocation;
                vector3 toNextNode  = nextNode.GetPosition() - startLocation;
                toTarget.Normalize();
                toNextNode.Normalize();

                f32 nextNodeDot = toTarget.Dot(toNextNode);
                FindConnStruct nextConnStruct;
                nextConnStruct.connectionID = nextNode.GetOtherConnectionID( currentConnID );
                nextConnStruct.nodeID = nextNodeID;
                connectionQueue.Push(nextConnStruct,-nextNodeDot);
            }
        }
    }
    return finalNodeID;
}
*/


nav_node_slot_id nav_map::FindOverlapAtDistanceFromPoint(   f32 desiredDistance, 
                                                            const vector3& fleeFromLocation, 
                                                            const vector3& startLocation,
                                                            nav_connection_slot_id startConnectionID )
{    
    priority_queue<FindConnStruct,f32,64> connectionQueue;

    ng_connection2 startConnection = g_NavMap.GetConnectionByID( startConnectionID );

    FindConnStruct firstStruct;
    firstStruct.connectionID    = startConnectionID;
    firstStruct.nodeID          = NULL_NAV_SLOT;

    connectionQueue.Push( firstStruct, -1.0f );

    nav_connection_slot_id  currentConnID   = NULL_NAV_SLOT;
    nav_node_slot_id        currentNodeID   = NULL_NAV_SLOT;
    nav_node_slot_id        finalNodeID     = NULL_NAV_SLOT;   

    f32                 BestScore   = 5.0f;
    nav_node_slot_id    BestNode    = NULL_NAV_SLOT;

    desiredDistance *= desiredDistance;


    while(finalNodeID == NULL_NAV_SLOT && !connectionQueue.IsEmpty() )
    {
        // get the best connection and pop it off.
        FindConnStruct currentFindConnStruct = connectionQueue.Pop();
        currentConnID = currentFindConnStruct.connectionID;
        currentNodeID = currentFindConnStruct.nodeID;
        
        ng_connection2 currentConnection = g_NavMap.GetConnectionByID( currentConnID );

        if( currentConnection.GetGridID() != startConnection.GetGridID() )
        {
            continue;
        }

        f32 currentNodeToTargetDist = 0.0f;
        vector3 currentNodePos = startLocation;
        if( currentNodeID != NULL_NAV_SLOT )
        {
            ng_node2 currentNode = g_NavMap.GetNodeByID( currentNodeID );
            currentNodeToTargetDist = ( fleeFromLocation - currentNode.GetPosition() ).LengthSquared();
            currentNodePos = currentNode.GetPosition();
        }

        if (currentNodeToTargetDist >= desiredDistance)
        {
            finalNodeID = currentConnID;
            break;
        }

        s32 c;
        for(c=0;c<currentConnection.GetOverlapCount();c++)
        {
            nav_node_slot_id nextNodeID = currentConnection.GetOverlapNodeID( c );
            if( nextNodeID == NULL_NAV_SLOT || nextNodeID == currentNodeID )
            {
                continue;
            }
            ng_node2 nextNode = g_NavMap.GetNodeByID( nextNodeID );
            f32 nextNodeToTargetDistance = ( fleeFromLocation - nextNode.GetPosition()).LengthSquared();
            if( nextNodeToTargetDistance <= currentNodeToTargetDist )
            {
                continue;
            }

            f32 Dir,Dist;

            vector3 ToScaryThing = fleeFromLocation - currentNodePos;
            vector3 ToOverlap    = nextNode.GetPosition() - currentNodePos;

            ToScaryThing.Normalize();
            ToOverlap.Normalize();


            Dist = x_abs( desiredDistance - nextNodeToTargetDistance ) / desiredDistance;
            Dir  = ToScaryThing.Dot(ToOverlap) / 2.0f + 0.5f;
            
            // first check to see if we have achieved our goal.
            /*
            if( nextNodeToTargetDistance >= desiredDistance * desiredDistance )
            {
                finalNodeID = nextNodeID;
            }
            else*/

            f32 Score = Dir + Dist;

            if (Score < BestScore)
            {
                BestScore = Score;
                BestNode  = nextNodeID;
            }


            {            
                /*
                vector3 toTarget    = fleeFromLocation - currentNodePos;
                vector3 toNextNode  = nextNode.GetPosition() - currentNodePos;
                toTarget.Normalize();
                toNextNode.Normalize();                
                f32 nextNodeDot = toTarget.Dot(toNextNode);
                */

                FindConnStruct nextConnStruct;
                nextConnStruct.connectionID = nextNode.GetOtherConnectionID( currentConnID );
                nextConnStruct.nodeID = nextNodeID;
                connectionQueue.Push(nextConnStruct, Dir );
            }
        }
    }
    
    if( finalNodeID == NULL_NAV_SLOT )
    {
        finalNodeID = BestNode;
    }

    return finalNodeID;
}


//===========================================================================
// Cribbing and modifying some of Darrin's code here.
void nav_map::ClipConnections( vector3* pDstPos, const vector3* pSrcPos, s32& NDstVerts, s32 NSrcVerts, f32* pClipEdgeValues )
{
    vector3 TempPos0[4+4+1];
    vector3 TempPos1[4+4+1];

    // copy the src verts into our temp buffer and duplicate the last one
    s32 iVert;
    for ( iVert = 0; iVert < NSrcVerts; iVert++ )
    {
        TempPos0[iVert] = pSrcPos[iVert];
    }
    TempPos0[NSrcVerts] = pSrcPos[0];

    vector3* pSP = TempPos0;
    vector3* pDP = TempPos1;

    // clip the 4 edges
    for ( s32 Edge = 0; Edge < CLIP_MAX; Edge++ )
    {
        NDstVerts = 0;
        for ( iVert = 0; iVert < NSrcVerts; iVert++ )
        {
            xbool AddVert         = FALSE;
            xbool AddIntersection = FALSE;
            f32   T = 0.0f;
            switch ( Edge )
            {
            case CLIP_LEFT:
                if ( pSP[iVert].GetX() >= pClipEdgeValues[CLIP_LEFT] ) AddVert = TRUE;
                if ( (( pSP[iVert].GetX() < pClipEdgeValues[CLIP_LEFT] ) && (pSP[iVert+1].GetX()>pClipEdgeValues[CLIP_LEFT] )) ||
                     (( pSP[iVert].GetX() > pClipEdgeValues[CLIP_LEFT] ) && (pSP[iVert+1].GetX()<pClipEdgeValues[CLIP_LEFT] )) )
                {
                    AddIntersection = TRUE;
                    T = ( pClipEdgeValues[CLIP_LEFT] - pSP[iVert].GetX()) / (pSP[iVert+1].GetX() - pSP[iVert].GetX());
                }
                break;

            case CLIP_RIGHT:
                if ( pSP[iVert].GetX() <= pClipEdgeValues[CLIP_RIGHT] ) AddVert = TRUE;
                if (( (pSP[iVert].GetX()>pClipEdgeValues[CLIP_RIGHT] ) && (pSP[iVert+1].GetX()<pClipEdgeValues[CLIP_RIGHT])) ||
                   (( pSP[iVert].GetX()<pClipEdgeValues[CLIP_RIGHT]) && (pSP[iVert+1].GetX()>pClipEdgeValues[CLIP_RIGHT])) )
                {
                    AddIntersection = TRUE;
                    T = (pClipEdgeValues[CLIP_RIGHT] - pSP[iVert].GetX()) / (pSP[iVert+1].GetX() - pSP[iVert].GetX());
                }
                break;

            case CLIP_TOP:
                if ( pSP[iVert].GetZ() <= pClipEdgeValues[CLIP_TOP] ) AddVert = TRUE;
                if ( ((pSP[iVert].GetZ()>pClipEdgeValues[CLIP_TOP]) && (pSP[iVert+1].GetZ()<pClipEdgeValues[CLIP_TOP])) ||
                     ((pSP[iVert].GetZ()<pClipEdgeValues[CLIP_TOP]) && (pSP[iVert+1].GetZ()>pClipEdgeValues[CLIP_TOP])) )
                {
                    AddIntersection = TRUE;
                    T = (pClipEdgeValues[CLIP_TOP] - pSP[iVert].GetZ()) / (pSP[iVert+1].GetZ() - pSP[iVert].GetZ());
                }
                break;

            case CLIP_BOTTOM:
              if ( pSP[iVert].GetZ() >= pClipEdgeValues[CLIP_BOTTOM]) AddVert = TRUE;
              if ( ((pSP[iVert].GetZ()<pClipEdgeValues[CLIP_BOTTOM]) && (pSP[iVert+1].GetZ()>pClipEdgeValues[CLIP_BOTTOM])) ||
                   ((pSP[iVert].GetZ()>pClipEdgeValues[CLIP_BOTTOM]) && (pSP[iVert+1].GetZ()<pClipEdgeValues[CLIP_BOTTOM])) )
              {
                  AddIntersection = TRUE;
                  T = (pClipEdgeValues[CLIP_BOTTOM] - pSP[iVert].GetZ()) / (pSP[iVert+1].GetZ() - pSP[iVert].GetZ());
              }
                break;
            }

            if ( AddVert )
            {
                pDP[NDstVerts] = pSP[iVert];
                NDstVerts++;
            }

            if ( AddIntersection )
            {
                ASSERT( (T>=0.0f) && (T<1.0f) );
                vector3 NewPos = pSP[iVert] + T * (pSP[iVert+1]-pSP[iVert]);
                pDP[NDstVerts] = NewPos;
                NDstVerts++;
            }
        }

        // duplicate the last vert
        pDP[NDstVerts] = pDP[0];
        
        // swap dst and src
        vector3* Temp3 = pDP;
        pDP = pSP;
        pSP = Temp3;
        NSrcVerts = NDstVerts;

        // bail if we're completely culled
        if ( NSrcVerts == 0 )
            break;
    }

    // copy into the new array    
    for ( iVert = 0; iVert < NSrcVerts; iVert++ )
    {
        pDstPos[iVert] = pSP[iVert];
    }
}

//===========================================================================
/*
radian nav_map::GetAngleBetweenConnections( nav_connection_slot_id SrcSlot, nav_connection_slot_id DestSlot )
{
    ng_connection2& SrcConnection = GetConnectionByID( SrcSlot );
    ng_connection2& DstConnection = GetConnectionByID( DestSlot );

    // Get the position of the node that is on SrcConnection that is not shared with DestConnection
    nav_node_slot_id StartNodeID = SrcConnection.GetStartNodeID();
    nav_node_slot_id MiddleNodeID;
    nav_node_slot_id EndNodeID;

    if ( StartNodeID == DstConnection.GetStartNodeID() || StartNodeID == DstConnection.GetEndNodeID() )
    {
        StartNodeID = SrcConnection.GetEndNodeID();
    }

    MiddleNodeID = SrcConnection.GetOtherNodeID( StartNodeID );
    EndNodeID = DstConnection.GetOtherNodeID( MiddleNodeID );

    vector3 vStartMiddle = GetNodeByID(MiddleNodeID).GetPosition() - GetNodeByID( StartNodeID ).GetPosition();
    vector3 vMiddleEnd = GetNodeByID( EndNodeID ).GetPosition() - GetNodeByID( MiddleNodeID ).GetPosition();

    return x_MinAngleDiff( vStartMiddle.GetYaw(), vMiddleEnd.GetYaw() );
}
*/
//===========================================================================

void nav_map::GetPointOfIntersection( const vector3& vLineStartPoint, const vector3& vLineEndPoint, const vector3& vPointToCheck, vector3& vIntersectionPoint )
{
    vector3 v = vLineEndPoint - vLineStartPoint;
    vector3 w = vPointToCheck - vLineStartPoint;

    f32 c1 = w.Dot(v);
    if ( c1 <= 0 )
    {
        vIntersectionPoint = vLineStartPoint;
        return ;
    }

    f32 c2 = v.Dot(v);
    if ( c2 <= c1 )
    {
        vIntersectionPoint = vLineEndPoint;
        return;
    }

    f32 b = c1 / c2;
    vIntersectionPoint = vLineStartPoint + b * v;
    return;
}


//===========================================================================
#ifdef X_EDITOR

/*
void nav_map::InitConnectionZoneMgr( void )
{
    // Let's clear everything out in the ConnectionZoneManager
    m_ConnectionZoneMgr.Reset();

    // First, get the sizes of the arrays that we're going to need.
    m_ConnectionZoneMgr.EditorPreComputeCounts( m_Connection, m_ConnectionCount );
    m_ConnectionZoneMgr.SetZoneMgrData( m_Connection, m_ConnectionCount );
}
*/


#endif // X_EDITOR


//===========================================================================

void nav_map::ClipLine( ng_connection2& Con, vector3& LineStart, vector3& LineEnd )
{
    // Reached max allowed connections?
    if( m_nClipLineConnections == MAX_CLIP_LINE_CONNECTIONS )
        return;
        
    // This connection should not have been considered before!
    ASSERT( Con.IsClosed() == FALSE );

    // Flag that this connection has now been checked against
    Con.SetClosed();

    // Add to list
    ASSERT( m_nClipLineConnections < MAX_CLIP_LINE_CONNECTIONS );
    m_ClipLineConnections[ m_nClipLineConnections++ ] = Con.GetSlotID();
    
    // Clipped start of line?
    if( Con.ClipLine( LineStart, LineEnd ) )
    {
        // Check all connected connections
        for( s32 i = 0; i < Con.GetOverlapCount(); i++ )
        {
            // Get connected connection
            ng_connection2& NextCon = GetConnectionByID( Con.GetOverlapRemoteConnectionID( i ) );
            
            // Recurse on connection if it hasn't been considered yet
            if( NextCon.IsClosed() == FALSE )        
                ClipLine( NextCon, LineStart, LineEnd );
        }            
    }
}

//===========================================================================

xbool nav_map::DoesStraightPathExist( path_find_struct& PathFindStruct )
{
#ifndef X_RETAIL
    // Init debug info
    PathFindStruct.m_nClipLineConnections = 0;
    PathFindStruct.m_ClipLineStart = PathFindStruct.m_vStartPoint;
    PathFindStruct.m_ClipLineEnd   = PathFindStruct.m_vEndPoint;
#endif

    // Clear flag
    PathFindStruct.m_bStraightPath = FALSE;

    // Must be in connections
    if(     ( PathFindStruct.m_StartConnectionSlotID == NULL_NAV_SLOT )
        || ( PathFindStruct.m_EndConnectionSlotID   == NULL_NAV_SLOT ) )
    {         
        return FALSE;
    }

    // Lookup connections
    ng_connection2& StartCon = GetConnectionByID( PathFindStruct.m_StartConnectionSlotID );
    ng_connection2& EndCon   = GetConnectionByID( PathFindStruct.m_EndConnectionSlotID );

    // Must be inside connections
    if(     ( StartCon.IsPointInConnection( PathFindStruct.m_vStartPoint ) == FALSE )
        ||  ( EndCon.IsPointInConnection( PathFindStruct.m_vEndPoint ) == FALSE ) )
    {        
        return FALSE;
    }

    // In same connection?
    if( PathFindStruct.m_StartConnectionSlotID == PathFindStruct.m_EndConnectionSlotID )
    {
        PathFindStruct.m_bStraightPath = TRUE;
        return TRUE;
    }

    // Is start point in end connection?  (Connections overlap, so this is entirely possible).
    if ( EndCon.IsPointInConnection( PathFindStruct.m_vStartPoint ) == TRUE )
    {
        PathFindStruct.m_bStraightPath = TRUE;
        return TRUE;
    }

    // In different grids?
    if( StartCon.GetGridID() != EndCon.GetGridID() )
    {    
        return FALSE;
    }

    // Init line
    vector3 LineStart( PathFindStruct.m_vStartPoint );
    vector3 LineEnd( PathFindStruct.m_vEndPoint );

    // Clear clip line connection count
    m_nClipLineConnections = 0;

    // Clip line start towards the line end
    ClipLine( StartCon, LineStart, LineEnd );

    // Clear connection closed flags that were set during recursive line clip
    xbool bEndConnectionReached = FALSE;
    for( s32 i = 0; i < m_nClipLineConnections; i++ )        
    {
        // Did we reach the end connection?
        if( PathFindStruct.m_EndConnectionSlotID == m_ClipLineConnections[i] )
            bEndConnectionReached = TRUE;

        ng_connection2& Con = GetConnectionByID( m_ClipLineConnections[i] );
        Con.ResetPathingFlags();
    }

#ifndef X_RETAIL
    // Setup debug info
    PathFindStruct.m_ClipLineStart        = LineStart;
    PathFindStruct.m_ClipLineEnd          = LineEnd;
    PathFindStruct.m_nClipLineConnections = m_nClipLineConnections;
#endif

    // If end connection was reached and the line was clipped to where start and end are in 
    // the same connection then a straight path was found!
    if(     ( bEndConnectionReached )
        &&  ( EndCon.IsPointInConnection( LineStart ) ) )
    {
        PathFindStruct.m_bStraightPath = TRUE;
        return TRUE;
    }        
    else
    {
        return FALSE;
    }        
}

//===========================================================================

//  With NAVMAP_ALLOW_GUESS_ADJUSTMENT_TO_ENDPOINT defined, the guess
//  will be tweaked to select the FinalDestination if the delta yaw
//  between the final dest and the remote endpoint of the end connection
//  are within a certain tolerance
//
//
//#define NAVMAP_ALLOW_GUESS_ADJUSTMENT_TO_ENDPOINT   

vector3 nav_map::GetBestGuessDestination( nav_connection_slot_id StartConnectionSlotID,
                                          nav_connection_slot_id EndConnectionSlotID,
                                          const vector3&         StartPosition,
                                          const vector3&         FinalDestination
                                        )
{
    (void)StartConnectionSlotID;
    (void)EndConnectionSlotID;
    (void)StartPosition;

    return FinalDestination;
    /*
#ifndef NAVMAP_ALLOW_GUESS_ADJUSTMENT_TO_ENDPOINT
    (void)StartPosition;
    (void)FinalDestination;
#endif
    vector3 RemoteEnd;
    
    if( StartConnectionSlotID == NULL_NAV_SLOT )
    {
        return ( GetNearestPointOnConnection(EndConnectionSlotID,StartPosition) );
    }

    ng_connection2& SrcConnection = g_NavMap.GetConnectionByID( StartConnectionSlotID );
    ng_connection2& DstConnection = g_NavMap.GetConnectionByID( EndConnectionSlotID );

    // Get the position of the node on the dest connection that is not shared between the connections
    nav_node_slot_id vEndNodeID = DstConnection.GetStartNodeID();
    if ( vEndNodeID == SrcConnection.GetStartNodeID() || vEndNodeID == SrcConnection.GetEndNodeID() )
    {
        vEndNodeID = DstConnection.GetEndNodeID();
    }

    RemoteEnd = g_NavMap.GetNodeByID( vEndNodeID ).GetPosition();    

#ifdef NAVMAP_ALLOW_GUESS_ADJUSTMENT_TO_ENDPOINT
    f32 Yaw            = (RemoteEnd-StartPosition).GetYaw();
    f32 YawToFinalDest = (FinalDestination - StartPosition).GetYaw();

    if (x_abs( Yaw-YawToFinalDest ) < R_25)
    {
        RemoteEnd = FinalDestination;
    }
#endif

    return RemoteEnd;
    */
}

//===========================================================================

ng_connection2& nav_map::GetConnectionByID( nav_connection_slot_id iConnection )
{
    ASSERT( iConnection<m_nConnectionData );
    iConnection = MINMAX(0,iConnection,m_nConnectionData-1);
    return m_pConnection[ iConnection ];
}

//===========================================================================

ng_node2& nav_map::GetNodeByID( nav_node_slot_id iNode )
{
    ASSERT( iNode<m_nOverlapData );
    iNode = MINMAX(0,iNode,m_nOverlapData-1);
    return m_pNode[ iNode ];
}

//===========================================================================

u8 nav_map::GetConnectionGridID( nav_connection_slot_id iConnection )
{
    ASSERT( iConnection<m_nConnectionData );
    iConnection = MINMAX(0,iConnection,m_nConnectionData-1);

    return m_pConnectionData[ iConnection ].m_iGrid;
}

//===========================================================================

nav_connection_slot_id nav_map::GetNearestConnectionInGrid( nav_connection_slot_id  StartConnection, 
                                                            const vector3&          Point )
{    
    /*  trap for null vector input
    if ( Point.LengthSquared() < 0.000001f )
    {
        x_DebugMsg("GetNearestConnectionInGrid being called with null vector\n");
    }
    */

    nav_connection_slot_id Result=NULL_NAV_SLOT;
    f32 Dist=0;

    nav_connection_slot_id iBestConn        = NULL_NAV_SLOT;
    f32                    fBestSquaredDist = 1e30f;

    if( StartConnection != NULL_NAV_SLOT )
    {
        s32 i;
        ng_connection2& StartConn = GetConnectionByID( StartConnection );
        s32 iGrid = StartConn.m_ConnectionData->m_iGrid;

        // Prime best with starting connection
        if( StartConn.GetEnabled() )
        {            
            vector3 Pos = GetNearestPointOnConnection( StartConnection, Point );
            fBestSquaredDist = (Point-Pos).LengthSquared();
            iBestConn = StartConnection;
        }

        // Loop through remaining connections in the same grid and find anything closer
        for( i=0; i<m_nLookupBoxes; i++ )
        {
            connection_lookup_box& LB = m_pLookupBox[i];
            if( LB.m_iGrid == iGrid )
            {
                f32 ExternalSquaredDist = 0;
                
                // if Point does not intersect this bbox, is it even possible that a closer Connection can be found?
                // if Point DOES intersect, we will check all of this box's Connections below.
                if( LB.m_BBox.Intersect(Point)==FALSE )
                {
                    // Check if external distance to bbox makes closer point possible.
                    vector3 ClosestPtOnBBox = Point;
                    ClosestPtOnBBox.GetX() = x_max( ClosestPtOnBBox.GetX(), LB.m_BBox.Min.GetX() );
                    ClosestPtOnBBox.GetY() = x_max( ClosestPtOnBBox.GetY(), LB.m_BBox.Min.GetY() );
                    ClosestPtOnBBox.GetZ() = x_max( ClosestPtOnBBox.GetZ(), LB.m_BBox.Min.GetZ() );
                    ClosestPtOnBBox.GetX() = x_min( ClosestPtOnBBox.GetX(), LB.m_BBox.Max.GetX() );
                    ClosestPtOnBBox.GetY() = x_min( ClosestPtOnBBox.GetY(), LB.m_BBox.Max.GetY() );
                    ClosestPtOnBBox.GetZ() = x_min( ClosestPtOnBBox.GetZ(), LB.m_BBox.Max.GetZ() );
                    ExternalSquaredDist = (ClosestPtOnBBox - Point).LengthSquared();
                }

                // Loop through connections in box
                if( ExternalSquaredDist <= fBestSquaredDist )
                {
                    s32* pConnI = &m_pLookupIndex[LB.m_iConnection];
                    s32 j;
                    for( j=0; j<LB.m_nConnections; j++ )
                    {
                        // Get the connection.
                        s32 ConnI = pConnI[j];

                        // no need to check StartConnection again..
                        if ( ConnI == StartConnection )
                            continue;

                        ng_connection2& Conn = GetConnectionByID(ConnI);
                        
                        if( Conn.GetEnabled() )
                        {
                            vector3 Pos = GetNearestPointOnConnection( ConnI, Point );
                            f32 fDist = (Point-Pos).LengthSquared();
                            if (fDist < fBestSquaredDist)
                            {
                                fBestSquaredDist = fDist;
                                iBestConn        = ConnI;
                            }
                        }
                    }
                }
            }
        }
    }

    Result = iBestConn;
    Dist   = fBestSquaredDist;

    return Result;
}

//===========================================================================

const nav_map::overlap_vert& nav_map::GetOverlapVert( s32 iVert ) const
{
    ASSERT( iVert >= 0);
    ASSERT( iVert < m_nOverlapVerts );

    return m_pOverlapVerts[ iVert ];
}

//===========================================================================

void nav_map::SetConnectionStatus( guid ConnectionGuid, xbool bOnOff )
{
    s32 iConn = -1;

    if (!m_GuidLookup.Find( ConnectionGuid, iConn ))
        return;

    ASSERT( iConn >= 0);
    ASSERT( iConn <m_nConnectionData );

    if ((iConn < 0) || (iConn >= m_nConnectionData))
        return;

    ng_connection2& Conn = GetConnectionByID( iConn );

    Conn.SetEnabled( bOnOff );
    
#ifdef X_EDITOR
    object* pObj = g_ObjMgr.GetObjectByGuid( ConnectionGuid );
    if (pObj)
    {
        if (pObj->IsKindOf(nav_connection2_editor::GetRTTI() ) )
        {
            nav_connection2_editor& EdConn = nav_connection2_editor::GetSafeType( *pObj );

            EdConn.SetEnabled( bOnOff );
        }
    }
#endif // X_EDITOR
   
    //
    //  Notify all active characters that a connection has changed status
    //
    update_info UpdateInfo;
    UpdateInfo.iUpdatedConnection = iConn;
    actor* pActor = actor::m_pFirstActive;
    while( pActor )
    {
        if (pActor->IsKindOf(character::GetRTTI() ) )
        {
            character& Char = character::GetSafeType( *pActor );       
            Char.HandleNavMapUpdate( UpdateInfo );
        }
        // move to next actor
        pActor = pActor->m_pNextActive;
    }  
}

//===========================================================================

xbool nav_map::GetConnectionStatus( guid ConnectionGuid )
{
    s32 iConn = -1;

    if (!m_GuidLookup.Find( ConnectionGuid, iConn ))
        return FALSE;

    ASSERT( iConn >= 0);
    ASSERT( iConn <m_nConnectionData );

    if ((iConn < 0) || (iConn >= m_nConnectionData))
        return FALSE;

    ng_connection2& Conn = GetConnectionByID( iConn );

    return Conn.GetEnabled();
}

//===========================================================================

void nav_map::AddToConnectionCache( nav_connection_slot_id NewID )
{
    // Find location of connection in list
    s32 i;
    for( i=0; i<CONNECTION_QUERY_CACHE_SIZE; i++ )
    {
        if( m_RecentConnections[ i ] == NewID )
            break;
    }

    // If new add to end of list
    if( i==CONNECTION_QUERY_CACHE_SIZE )
    {
        i = CONNECTION_QUERY_CACHE_SIZE-1;
        m_RecentConnections[i] = NewID;
    }

    OptimizeConnectionCache( i );

    m_RecentCacheMisses++;
}

//===========================================================================

void nav_map::OptimizeConnectionCache( s32 iEntry )
{
    s32 ID = m_RecentConnections[iEntry];

    // Adjust list order
    for( s32 j=iEntry; j>=1; j-- )
        m_RecentConnections[j] = m_RecentConnections[j-1];

    // Put new value at top
    m_RecentConnections[0] = ID;
}

//===========================================================================
#if !defined(X_RETAIL) || defined(X_QA) 

static f32 NAV_RENDER_CULL_DIST                 = 800;
static f32 NAV_RENDER_CULL_DIST_SQUARED         = (NAV_RENDER_CULL_DIST*NAV_RENDER_CULL_DIST);

void nav_map::RenderNavNearView( void )
{
    if (!s_bDebugNavigation)
        return;

    s32 i,j;

    const view* pView = eng_GetView();
    if (NULL == pView)
        return;

    vector3 ViewPos = pView->GetPosition();

    for (i=0;i<m_nConnectionData;i++)
    {
        ng_connection2& Conn = g_NavMap.GetConnectionByID( i );

        f32 Dist1 = (m_pConnectionData[i].m_StartPt - ViewPos).LengthSquared();
        f32 Dist2 = (m_pConnectionData[i].m_EndPt - ViewPos).LengthSquared();
        f32 MinDist = ViewPos.GetSqrtDistToLineSeg( m_pConnectionData[i].m_StartPt, m_pConnectionData[i].m_EndPt );

        if ((Dist1 > NAV_RENDER_CULL_DIST_SQUARED) &&
            (Dist2 > NAV_RENDER_CULL_DIST_SQUARED) &&
            (MinDist > NAV_RENDER_CULL_DIST_SQUARED))
        {
            continue;
        }

        draw_Line( m_pConnectionData[i].m_StartPt + vector3(0,50,0), 
            m_pConnectionData[i].m_EndPt   + vector3(0,50,0), 
            XCOLOR_PURPLE );

        // Need to draw body of connection
        vector3 LengthDelta = m_pConnectionData[i].m_EndPt - m_pConnectionData[i].m_StartPt;
        vector3 Side = LengthDelta.Cross(vector3(0,1,0));
        Side.NormalizeAndScale( m_pConnectionData[i].m_Width );

        vector3 Corner[4];
        Corner[0] = m_pConnectionData[i].m_StartPt + Side + vector3(0,25,0);
        Corner[1] = m_pConnectionData[i].m_EndPt   + Side + vector3(0,25,0);
        Corner[2] = m_pConnectionData[i].m_EndPt   - Side + vector3(0,25,0);
        Corner[3] = m_pConnectionData[i].m_StartPt - Side + vector3(0,25,0);

        draw_Begin( DRAW_TRIANGLES, DRAW_CULL_NONE | DRAW_USE_ALPHA );
        draw_ClearL2W();
        draw_Color(0,0,0.5,0.5);
        draw_Vertex( Corner[0] );
        draw_Vertex( Corner[1] );
        draw_Vertex( Corner[2] );

        draw_Vertex( Corner[0] );
        draw_Vertex( Corner[2] );
        draw_Vertex( Corner[3] );
        draw_End();

        vector3 Center = m_pConnectionData[i].m_StartPt + m_pConnectionData[i].m_EndPt;
        Center.Scale(0.5f);

        if (g_AIMgr.GetShowConnectionIDs())
            draw_Label( Center, XCOLOR_YELLOW, "ID:%d",i);

        for (j=0;j<Conn.GetOverlapCount();j++)
        {
            nav_node_slot_id OID = Conn.GetOverlapNodeID(j);
            overlap_data& OD = g_NavMap.GetOverlapData( OID );

            vector3 Pt = OD.m_Center;

            vector3 A,B;

            A = Conn.GetStartPosition();
            B = Conn.GetEndPosition();

            A += vector3(0,50,0);
            B += vector3(0,50,0);

            vector3 Dir = B-A;
            vector3 P = Pt-A;
            vector3 N;

            f32 T = P.Dot(Dir) / x_sqr(Dir.Length());

            if (T > 1)
                N = B;
            else if (T < 0)
                N = A;
            else
            {
                Dir.Scale(T);
                N = A + Dir;
            }

            draw_Line( Pt, N, XCOLOR_RED );
        }
    }

    for (i=0;i<m_nOverlapData;i++)
    {
        overlap_data&   OD = m_pOverlapData[ i ];

        f32 Dist1 = (OD.m_Center - ViewPos).LengthSquared();
        
        if (Dist1 > NAV_RENDER_CULL_DIST_SQUARED)
        {
            continue;
        }

/*
        xcolor Clr;

        for (j=0;j<OD.m_nOverlapPts;j++)
        {
            u16 Flags = m_pOverlapVerts[ OD.m_iFirstOverlapPt + j ].m_Flags;

            Clr = XCOLOR_PURPLE;

            if (Flags & overlap_vert::FLAG_OUTSIDE)
                Clr = XCOLOR_RED;

            draw_Sphere( m_pOverlapVerts[ OD.m_iFirstOverlapPt + j ].m_Pos, 10, Clr );
        }

        draw_Sphere( OD.m_Center, 10, XCOLOR_GREEN);
*/
        draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA | DRAW_CULL_NONE);
        draw_Color(255,0,40,128);
        for (j=0;j<OD.m_nOverlapPts-1;j++)
        {
            draw_Vertex( m_pOverlapVerts[ OD.m_iFirstOverlapPt + 0 ].m_Pos );
            draw_Vertex( m_pOverlapVerts[ OD.m_iFirstOverlapPt + j ].m_Pos );
            draw_Vertex( m_pOverlapVerts[ OD.m_iFirstOverlapPt + j+1 ].m_Pos );
        }
        draw_End();
    }
}

#endif // !defined(X_RETAIL) || defined(X_QA) 

//===========================================================================
//===========================================================================
// ROUTINES TO COMPILE THE LOOKUP BOXES 
//===========================================================================
//===========================================================================
#ifdef X_EDITOR
//===========================================================================

//===========================================================================

lbox::lbox()
{
    BBox.Clear();
    Index.Clear();
    iGrid = -1;
    bFinished = FALSE;
    Score = 0;
};

lbox::~lbox()
{
}

//===========================================================================

void nav_map::ScoreLBox( lbox& LBox )
{
    LBox.Score = (f32)LBox.Index.GetCount();
}

xbool nav_map::SplitLBox( lbox& LBox, lbox& NewLBox )
{
    s32 MinConnsPerBox = 8;

    s32 DimSlices[3] = {8,8,8};
    xarray<s32> BestSplitLeft;
    xarray<s32> BestSplitRight;
    bbox        BestShrunkLeftBBox;
    bbox        BestShrunkRightBBox;
    f32         BestLeftScore=0;
    f32         BestRightScore=0;
    f32         BestScore=F32_MAX;

    for( s32 Dim=0; Dim<3; Dim++ )
    {
        s32 nSlices = DimSlices[Dim];
        f32 dT = 1.0f / (f32)(nSlices+1);
        for( s32 Slice=0; Slice<nSlices; Slice++ )
        {
            // Compute the slice boundary for this slice and this dimension
            f32 T = (Slice+1)*dT;
            f32 SliceValue = LBox.BBox.Min[Dim] + T*(LBox.BBox.Max[Dim]-LBox.BBox.Min[Dim]);

            // Build the two child bboxes
            bbox LeftBBox   = LBox.BBox;
            bbox RightBBox  = LBox.BBox;
            LeftBBox.Max[Dim] = SliceValue;
            RightBBox.Min[Dim] = SliceValue;

            // Split connections based on bboxes
            s32         i;
            xarray<s32> SplitLeft;
            xarray<s32> SplitRight;
            bbox        ShrunkLeftBBox;
            bbox        ShrunkRightBBox;
            ShrunkLeftBBox.Clear();
            ShrunkRightBBox.Clear();

            s32 nCuts=0;
            for( i=0; i<LBox.Index.GetCount(); i++ )
            {
                s32 iConnection = LBox.Index[i];
                ASSERT( (iConnection>=0) && (iConnection<m_nConnectionData) );
                connection2_data& C = m_pConnectionData[iConnection];
                
                bbox ConnBBox = C.m_BBox;
                vector3 ConnCenter = ConnBBox.GetCenter();

                if( LeftBBox.Intersect(ConnBBox) && RightBBox.Intersect(ConnBBox) )
                    nCuts++;

                if( LeftBBox.Intersect(ConnCenter) )
                {
                    SplitLeft.Append( iConnection );
                    ShrunkLeftBBox += ConnBBox;
                }
                else
                {
                    ASSERT( RightBBox.Intersect(ConnCenter) );
                    SplitRight.Append( iConnection );
                    ShrunkRightBBox += ConnBBox;
                }
            }

            ShrunkLeftBBox.Inflate(1,1,1);
            ShrunkRightBBox.Inflate(1,1,1);

            // Is this a possible keeper?
            if( (SplitLeft.GetCount()  >= MinConnsPerBox) && 
                (SplitRight.GetCount() >= MinConnsPerBox) )
            {
                // Compute Score
                s32 Min = MIN(SplitLeft.GetCount(),SplitRight.GetCount());
                s32 Max = MAX(SplitLeft.GetCount(),SplitRight.GetCount());
                f32 Ratio = (f32)(Max+nCuts) / (f32)Min;
                f32 Score = Ratio;

                if( Score < BestScore )
                {
                    BestScore = Score;
                    BestSplitLeft = SplitLeft;
                    BestSplitRight = SplitRight;
                    BestShrunkLeftBBox = ShrunkLeftBBox;
                    BestShrunkRightBBox = ShrunkRightBBox;
                    BestLeftScore = (f32)SplitLeft.GetCount();
                    BestRightScore = (f32)SplitRight.GetCount();
                }
            }
        }
    }

    // Did we find a good split?
    if( BestScore == F32_MAX )
    {
        // Nope.
        LBox.bFinished = TRUE;
        return FALSE;
    }

    // Copy split into box
    lbox& LB = LBox;
    lbox& RB = NewLBox;
    
    LB.BBox     = BestShrunkLeftBBox;
    LB.Index    = BestSplitLeft;
    LB.Score    = BestLeftScore;
    LB.bFinished= FALSE;
    LB.iGrid    = LBox.iGrid;

    RB.BBox     = BestShrunkRightBBox;
    RB.Index    = BestSplitRight;
    RB.Score    = BestRightScore;
    RB.bFinished= FALSE;
    RB.iGrid    = LBox.iGrid;

    return TRUE;
}

//===========================================================================

void nav_map::CompileLookupBoxes( void )
{
    #define MAX_LBOXES  64
    lbox    LBox[MAX_LBOXES];
    s32     nLBoxes = 0;

    //
    // Compute bboxes for connections
    //
    {
        s32 i;
        for( i=0; i<m_nConnectionData; i++ )
        {
            connection2_data& C = m_pConnectionData[i];

            // Build side vector
            vector3 LengthDelta = C.m_EndPt - C.m_StartPt;
            vector3 Side = LengthDelta.Cross(vector3(0,1,0));
            Side.NormalizeAndScale( C.m_Width );

            // Build corners
            vector3 Corner[4];
            Corner[0] = C.m_StartPt + Side;
            Corner[1] = C.m_EndPt   + Side;
            Corner[2] = C.m_EndPt   - Side;
            Corner[3] = C.m_StartPt - Side;

            C.m_BBox.Clear();
            C.m_BBox.AddVerts( Corner, 4 );
            C.m_BBox.Inflate(10,200,10);        // y-axis size is intentionally larger so that point-checks have more room for error along that axis
        }
    }

    //
    // Seed the LBox list by grid index
    // i.e., partition existing Connections and their corresponding bBox's by grid.  
    // after this loop, there will be n LBox entries where n is the number of Grids in the nav_map
    // TODO:  Better handling if (nGrids > MAX_LBOXES)
    //
    {
        s32 i,j;
        for( i=0; i<m_nConnectionData; i++ )
        {
            connection2_data& C = m_pConnectionData[i];

            // find the current connection's Grid index in the local array of LBoxes
            for( j=0; j<nLBoxes; j++ )
            {
                if( LBox[j].iGrid == C.m_iGrid )
                    break;
            }

            // create new entry in LBox array if we couldn't find a match
            if( j==nLBoxes )
            {
                ASSERT( nLBoxes < MAX_LBOXES );
                LBox[nLBoxes].iGrid = C.m_iGrid;
                LBox[nLBoxes].BBox.Clear();
                nLBoxes++;
            }

            LBox[j].Index.Append( i );
            LBox[j].BBox += C.m_BBox;

            x_DebugMsg("BBox [%03d] (%8.1f %8.1f %8.1f) (%8.1f %8.1f %8.1f) %2d (%8.1f %8.1f %8.1f)\n",i,
                C.m_BBox.GetCenter().GetX(),
                C.m_BBox.GetCenter().GetY(),
                C.m_BBox.GetCenter().GetZ(),
                C.m_BBox.GetSize().GetX(),
                C.m_BBox.GetSize().GetY(),
                C.m_BBox.GetSize().GetZ(),
                LBox[j].iGrid,
                LBox[j].BBox.GetSize().GetX(),
                LBox[j].BBox.GetSize().GetY(),
                LBox[j].BBox.GetSize().GetZ()
                );
        }

        for( i=0; i<nLBoxes; i++ )
            ScoreLBox( LBox[i] );
    }

    //
    // Loop until we are happy with LBoxes
    // i.e., Split up the LBox with the "best score" (the LBox with the most connections) that hasn't been marked as bFinished
    //
    while( 1 )
    {
        // Terminate if we have enough boxes
        if( nLBoxes==MAX_LBOXES )
            break;

        // Find the best score for splitting
        s32 i;
        f32 BestScore=0;
        s32 iBest=-1;
        for( i=0; i<nLBoxes; i++ )
        {
            if( (LBox[i].bFinished==FALSE) && (LBox[i].Score > BestScore) )
            {
                BestScore = LBox[i].Score;
                iBest = i;
            }
        }

        // If no best score then terminate
        if( iBest==-1 )
            break;

        // Split the best if we can.
        if( SplitLBox( LBox[iBest], LBox[nLBoxes] ) )
            nLBoxes++;

    }

    //
    // Do sanity check on LBoxes
    //
    {
        s32 i,j;
        for( i=0; i<nLBoxes; i++ )
        {
            lbox& LB = LBox[i];
            for( j=0; j<LB.Index.GetCount(); j++ )
            {
                s32 I = LB.Index[j];
                ASSERT( (I>=0) && (I<m_nConnectionData) );
                bbox CB = m_pConnectionData[I].m_BBox;
                ASSERT( CB.Min.GetX() >= LB.BBox.Min.GetX() );
                ASSERT( CB.Min.GetY() >= LB.BBox.Min.GetY() );
                ASSERT( CB.Min.GetZ() >= LB.BBox.Min.GetZ() );
                ASSERT( CB.Max.GetX() <= LB.BBox.Max.GetX() );
                ASSERT( CB.Max.GetY() <= LB.BBox.Max.GetY() );
                ASSERT( CB.Max.GetZ() <= LB.BBox.Max.GetZ() );

                ASSERT( m_pConnectionData[I].m_iGrid == LB.iGrid );
            }
        }

        //xbool* pUsed = (xbool*)x_malloc(sizeof(m_nConnectionData)*sizeof(xbool));
    }

    //
    // Dump out stats on final boxes
    //
    {
        s32 i;
        x_DebugMsg("-------------------- LBOXES -------------------------\n");
        for( i=0; i<nLBoxes; i++ )
        {
            x_DebugMsg("%2d] %4d %2d (%8.1f,%8.1f,%8.1f)\n",
                i, LBox[i].Index.GetCount(), LBox[i].iGrid,
                LBox[i].BBox.GetSize().GetX(),
                LBox[i].BBox.GetSize().GetY(),
                LBox[i].BBox.GetSize().GetZ() );
        }
    }



    //
    // Build final structures
    //
    {
        s32 i;

        // Clear current structures if allocated
        if( m_pLookupBox )
        {
            delete[] m_pLookupBox;
            m_pLookupBox = NULL;
        }
        m_nLookupBoxes = 0;

        if( m_pLookupIndex )
        {
            delete[] m_pLookupIndex;
            m_pLookupIndex = NULL;
        }
        m_nLookupIndices = 0;

        
        m_nLookupBoxes = nLBoxes;
        m_pLookupBox   = new connection_lookup_box[m_nLookupBoxes];

        m_nLookupIndices = 0;
        for( i=0; i<m_nLookupBoxes; i++ )
        {
            m_pLookupBox[i].m_BBox          = LBox[i].BBox;
            m_pLookupBox[i].m_iGrid         = LBox[i].iGrid;
            m_pLookupBox[i].m_iConnection   = m_nLookupIndices;
            m_pLookupBox[i].m_nConnections  = LBox[i].Index.GetCount();
            m_nLookupIndices += LBox[i].Index.GetCount();
        }

        
        m_pLookupIndex = new s32[m_nLookupIndices];

        s32 j,k=0;
        for( i=0; i<m_nLookupBoxes; i++ )
        {
            for( j=0; j<LBox[i].Index.GetCount(); j++ )
                m_pLookupIndex[k++] = LBox[i].Index[j];
        }
    }
}

//===========================================================================

//===========================================================================
#endif // EDITOR
//===========================================================================
