//=========================================================================
//
//  OCCLUDERMGR.CPP
// 
//=========================================================================

#include "entropy.hpp"
#include "occludermgr.hpp"
#include "Obj_Mgr\Obj_mgr.hpp"
#include "Objects\InvisWall.hpp"

//=========================================================================

#define MAX_NGON_POINTS (6+MAX_OCCLUDERS)

//=========================================================================

occluder_mgr g_OccluderMgr;

//=========================================================================

occluder_mgr::occluder_mgr( void )
{
    Clear();
}

//=========================================================================

occluder_mgr::~occluder_mgr( void )
{
    Clear();
}

//=========================================================================

void occluder_mgr::Init( void )
{
    Clear();
}

//=========================================================================

void occluder_mgr::Kill( void )
{
    Clear();
}

//==============================================================================

void occluder_mgr::GatherOccluders( void )
{
    Clear();

    //
    //  Iterate through all invisible wall objects, and look for any that are flagged
    //  to be occluders
    //
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_INVISIBLE_WALL_OBJ );
    while( SlotID != SLOT_NULL )
    {
		invisible_wall_obj* pWall = (invisible_wall_obj*)g_ObjMgr.GetObjectBySlot(SlotID);

        vector3 Pt[4];
        if( pWall->GetOccluderPoints( Pt ) )
        {
            AddOccluder( Pt, 4 );
        }
        SlotID = g_ObjMgr.GetNext( SlotID );
    }
}

//=========================================================================

void occluder_mgr::Clear( void )
{
    m_bDirtyOccluders = FALSE;
    m_nOccluders = 0;
    m_UseOccluders = TRUE;
    m_Stats.nBBoxOccludedFalse = 0;
    m_Stats.nBBoxOccludedTrue = 0;
    x_memset( m_Occluder, 0, sizeof(m_Occluder) );
}

//=========================================================================

void occluder_mgr::UseOccluders( xbool OnOff )
{
    m_UseOccluders = OnOff;
}

//=========================================================================

void occluder_mgr::AddOccluder( const vector3* pPoint, s32 nPoints )
{
    s32 i,j;

    // Topping out on occluders shouldn't be fatal
    if (m_nOccluders >= MAX_OCCLUDERS)
        return;

    ASSERT( m_nOccluders < MAX_OCCLUDERS );
    ASSERT( nPoints <= MAX_OCCLUDER_POINTS );

    occluder& O = m_Occluder[m_nOccluders];
    m_nOccluders++;
    O.bActive = TRUE;

    // Copy over points
    for( i=0; i<nPoints; i++ )
        O.Point[i] = pPoint[i];
    O.nPoints = nPoints;

    // Compute bbox
    O.BBox.Clear();
    O.BBox.AddVerts( O.Point, O.nPoints );
    O.BBox.Inflate(1,1,1);

    // Compute center of ngon
    vector3 Center(0,0,0);
    for( j=0; j<O.nPoints; j++ )
        Center += O.Point[j];
    Center /= (f32)O.nPoints;
    O.Center = Center;

    // Compute plane of ngon
    vector3 Normal(0,0,0);
    for( j=0; j<O.nPoints-1; j++ )
    {
        vector3 N = v3_Cross( O.Point[j]-Center, O.Point[j+1]-Center );
        Normal += N;
    }
    O.Area = Normal.Length() * 0.5f;
    Normal.Normalize();

    O.Plane.Setup( Center, Normal );
}

//=========================================================================

void occluder_mgr::RenderAllOccluders( void )
{
#if !defined(X_RETAIL)
    s32 i;

    // Render Alpha filler
    draw_Begin(DRAW_TRIANGLES,DRAW_USE_ALPHA);
    draw_Color(xcolor(255,0,0,32));
    for( i=0; i<m_nOccluders; i++ )
    {
        occluder& O = m_Occluder[ i ];

        for( s32 j=1; j<O.nPoints-1; j++ )
        {
            draw_Vertex(O.Point[0]);
            draw_Vertex(O.Point[j]);
            draw_Vertex(O.Point[j+1]);
        }
    }
    draw_End();

    // Render Wire outline
    draw_Begin(DRAW_LINES,DRAW_NO_ZBUFFER);
    draw_Color(xcolor(255,0,0,255));
    for( i=0; i<m_nOccluders; i++ )
    {
        s32 P = m_Occluder[i].nPoints-1;
        for( s32 j=0; j<m_Occluder[i].nPoints; j++ )
        {
            draw_Vertex(m_Occluder[i].Point[P]);
            draw_Vertex(m_Occluder[i].Point[j]);
            P = j;
        }
    }
    draw_End();
/*
    // Render markers
    for( i=0; i<m_nOccluders; i++ )
    {
        draw_Marker( m_Occluder[i].Center, XCOLOR_YELLOW );
    }
*/
#endif
}

//=========================================================================

void occluder_mgr::RenderUsableOccluders( void )
{
#if !defined(X_RETAIL)
    s32 i;

    // Render Alpha filler
    draw_Begin(DRAW_TRIANGLES,DRAW_USE_ALPHA);
    draw_Color(xcolor(0,255,0,32));
    for( i=0; i<m_nUsableOccluders; i++ )
    {
        occluder& O = m_Occluder[ m_UsableOccluderIndex[i] ];

        for( s32 j=1; j<O.nPoints-1; j++ )
        {
            draw_Vertex(O.Point[0]);
            draw_Vertex(O.Point[j]);
            draw_Vertex(O.Point[j+1]);
        }
    }
    draw_End();

    // Render Wire outline
    draw_Begin(DRAW_LINES,DRAW_NO_ZBUFFER);
    draw_Color(xcolor(0,255,0,255));
    for( i=0; i<m_nUsableOccluders; i++ )
    {
        occluder& O = m_Occluder[ m_UsableOccluderIndex[i] ];

        s32 P = O.nPoints-1;
        for( s32 j=0; j<O.nPoints; j++ )
        {
            draw_Vertex(O.Point[P]);
            draw_Vertex(O.Point[j]);
            P = j;
        }
    }
    draw_End();
/*
    // Render markers
    for( i=0; i<m_nUsableOccluders; i++ )
    {
        occluder& O = m_Occluder[ m_UsableOccluderIndex[i] ];
        draw_Label( O.Center, XCOLOR_YELLOW, xfs("(%1d)",i) );
        //draw_Marker( O.Center, XCOLOR_YELLOW );
    }
*/
#endif
}

//=========================================================================

void occluder_mgr::RenderFrustums( void )
{
#if !defined(X_RETAIL)
    s32 i,j;

    for( i=0; i<m_nOccluders; i++ )
    {
        // Loop through points
        for( j=0; j<m_Occluder[i].nPoints; j++ )
        {
            vector3 Delta = m_Occluder[i].Point[j] - m_EyePos;
            Delta.Normalize();
            draw_Line( m_EyePos, m_Occluder[i].Point[j]+(Delta*5000), XCOLOR_WHITE );
        }
    }
#endif
}

//=========================================================================

void occluder_mgr::SetView( const view& View )
{
    s32 i,j;

    // Clear Stats
    m_Stats.nBBoxOccludedFalse = 0;
    m_Stats.nBBoxOccludedTrue = 0;
    m_Stats.BBoxOccludeTime = 0;
    m_Stats.SetViewTime = 0;

    if( !m_UseOccluders )
        return;

    if( m_bDirtyOccluders )
        GatherOccluders();

    xtimer Timer;
    Timer.Start();

    //
    // Gather info from view
    //
    m_EyePos = View.GetPosition();
    m_EyePlane.Normal = View.GetViewZ();
    m_EyePlane.ComputeD( m_EyePos );
    m_EyeX = View.GetViewX();
    m_EyeY = View.GetViewY();
    m_EyeZ = View.GetViewZ();

    //x_printfxy(30,30,"%f %f %f",m_EyePos.X,m_EyePos.Y, m_EyePos.Z);

    s32 nTested=0;
    s32 nBBoxCulled=0;
    s32 nBBoxAccept=0;
    //s32 nClipCulled=0;
    //s32 nClipAccept=0;

    //
    // Determine usable occluders and compute planes
    //
    m_nUsableOccluders = 0;
    for( i=0; i<m_nOccluders; i++ )
    if( m_Occluder[i].bActive )
    {
        nTested++;

        // Get reference to occluder
        occluder& O = m_Occluder[i];
        O.Score = 0;
        O.bPrepared = FALSE;
        
        // Check if bbox of ngon is in view
        s32 Vis = View.BBoxInView( O.BBox );
        if( !Vis )
        {
            nBBoxCulled++;
            continue;
        }

        // If ngon bbox is partially in view clip to check ngon
        xbool    IsUsable = TRUE;
/*
        if( Vis==2 )
        {
            vector3  PB[(MAX_OCCLUDER_POINTS+6)*2];
            vector3* pSrc = O.Point;
            vector3* pDst = PB;
            s32      nPoints = O.nPoints;
            const plane* pPlane = View.GetViewPlanes();

            for( j=0; j<6; j++ )
            if( nPoints )
            {
                pPlane[j].ClipNGon( pDst, nPoints, pSrc, nPoints );
                pSrc = pDst;
                pDst = (pDst==PB) ? (PB+(MAX_OCCLUDER_POINTS+6)) : PB;
            }

            if( nPoints == 0 )
            {
                IsUsable = FALSE;
                nClipCulled++;
            }
            else
            {
                nClipAccept++;
            }
        }
        else
*/
        {
            // BBox was completely in view
            nBBoxAccept++;
        }

        if( !IsUsable )
            continue;
/*
        // Compute screen points and score of occluder
        if( 1 )
        {
            s32 i;
            ASSERT( nPoints < MAX_OCCLUDER_POINTS+6 );
            vector3 ScreenP[MAX_OCCLUDER_POINTS+6];
            for( i=0; i<nPoints; i++ )
            {
                ScreenP[i] = View.PointToScreen(pSrc[i]);
                ScreenP[i].GetZ() = 0;
            }

            vector3 TotalCross;
            TotalCross.Zero();
            for( i=1; i<nPoints-1; i++ )
            {
                vector3 Cross = v3_Cross((ScreenP[i]-ScreenP[0]),(ScreenP[i+1]-ScreenP[0]));
                TotalCross += Cross;
            }
            O.Score = TotalCross.Length()*0.5f;
        }
        else
*/
        {
            // Compute score for occluder
            f32 DistToCenter2 = (O.Center - m_EyePos).LengthSquared();
            f32 Dot = x_abs(m_EyePlane.Normal.Dot( O.Plane.Normal ));
            O.Score = (O.Area*Dot) / DistToCenter2;
        }

        // Build list of usable occluders
        m_UsableOccluderIndex[m_nUsableOccluders] = i;
        m_nUsableOccluders++;
    }

    //
    // Sort usable occluders based on score
    //
    for( i=0; i<m_nUsableOccluders; i++ )
    {
        s32 BestI = i;
        f32 BestS = m_Occluder[m_UsableOccluderIndex[BestI]].Score;
        for( j=i+1; j<m_nUsableOccluders; j++ )
        {
            if( m_Occluder[m_UsableOccluderIndex[j]].Score > BestS )
            {
                BestI = j;
                BestS = m_Occluder[m_UsableOccluderIndex[j]].Score;
            }
        }

        s32 T = m_UsableOccluderIndex[i];
        m_UsableOccluderIndex[i] = m_UsableOccluderIndex[BestI];
        m_UsableOccluderIndex[BestI] = T;
    }

    //
    // Truncate number of usable occluders to a certain value
    //
    m_nUsableOccluders = MIN(MAX_USABLE_OCCLUDERS,m_nUsableOccluders);

    //
    // Build extra info for only usable occluders
    //
    for( i=0; i<m_nUsableOccluders; i++ )
    {
        occluder& O = m_Occluder[m_UsableOccluderIndex[i]];
        O.bPrepared = TRUE;

        // Copy over plane containing ngon
        O.ViewPlane[ O.nPoints ] = O.Plane;

        // Build side planes
        for( j=0; j<O.nPoints; j++ )
        {
            // Compute normal of plane
            s32 j2 = j+1;
            if( j2 == O.nPoints ) j2=0;

            vector3 N = v3_Cross( O.Point[j]-m_EyePos,O.Point[j2]-m_EyePos );
            N.Normalize();
            O.ViewPlane[j].Setup( m_EyePos, N );
        }

        // Flip planes so that eye is behind ngon
        if( O.Plane.Distance( m_EyePos ) > 0 )
        {
            for( j=0; j<=O.nPoints; j++ )
                O.ViewPlane[j].Negate();
        }

        // Gather bbox indices for planes
        for( j=0; j<=O.nPoints; j++ )
        {
            O.ViewPlane[j].GetBBoxIndices( &O.BBoxMinIndex[j*3], 
                                           &O.BBoxMaxIndex[j*3] );
        }
    }

    m_Stats.SetViewTime = Timer.ReadSec();
/*
#if !defined(X_RETAIL)
    if( 0 )
    {
        s32 L=12;
        x_printfxy(0,L++,"nTested       %d", nTested  );
        x_printfxy(0,L++,"nBBoxCulled   %d", nBBoxCulled );
        x_printfxy(0,L++,"nBBoxAccept   %d", nBBoxAccept );
        x_printfxy(0,L++,"nClipCulled   %d", nClipCulled );
        x_printfxy(0,L++,"nClipAccept   %d", nClipAccept );
    }
#endif
*/
}

//=========================================================================

xbool occluder_mgr::IsBBoxCompletelyInsidePlanes( const occluder& O, const bbox& BBox )
{
    s32 j;
    
    ASSERT( O.bPrepared );

    const f32* pF = (const f32*)&BBox;
    const s32* pMinI = O.BBoxMinIndex;

    // Loop through each plane of this occluder
    for( j=0; j<=O.nPoints; j++ )
    {
        const plane& P = O.ViewPlane[j];

        // Compute distance of least likely point to plane
        f32 MinDist = P.Normal.GetX() * pF[pMinI[0]] +
                      P.Normal.GetY() * pF[pMinI[1]] +
                      P.Normal.GetZ() * pF[pMinI[2]] +
                      P.D;
        pMinI += 3;

        // If the min point is in front of the plane then the entire
        // bbox is in front of the plane.  Otherwise at least some
        // part of the bbox is behind the plane and occlusion is
        // impossible
        if( MinDist < 0 )
            break;
    }

    // Did we survive all the planes?
    if( j>O.nPoints )
        return TRUE;

    return FALSE;
}

//=========================================================================

xbool occluder_mgr::IsBBoxOccluded( const bbox& BBox )
{
    // Bail if no occluder info
    if( (m_nUsableOccluders==0) || (!m_UseOccluders) )
        return FALSE;

    s32 i;

    //m_Stats.BBoxOccludedTimer.Start();

    xbool Result = FALSE;
    {
        // The bbox must be entirely in front of the planes of one of
        // the occluders to be culled
        for( i=0; i<m_nUsableOccluders; i++ )
        {
            occluder& O = m_Occluder[m_UsableOccluderIndex[i]];

            if( IsBBoxCompletelyInsidePlanes( O, BBox ) )
            {
                Result = TRUE;
                break;
            }
        }
    }

    if( Result ) m_Stats.nBBoxOccludedTrue++;
    else         m_Stats.nBBoxOccludedFalse++;

    //m_Stats.BBoxOccludedTimer.Stop();

    return Result;
}

//=========================================================================

xbool occluder_mgr::IsPointOccluded( const vector3& Point )
{
    s32 i,j;

    if( !m_UseOccluders )
        return FALSE;

    for( i=0; i<m_nUsableOccluders; i++ )
    {
        occluder& O = m_Occluder[m_UsableOccluderIndex[i]];

        for( j=0; j<=O.nPoints; j++ )
        {
            if( O.ViewPlane[j].Distance( Point ) < 0 )
                break;
        }

        // Did we survive all the planes?
        if( j>O.nPoints )
        {
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================
void occluder_mgr::DirtyOccluders( void )
{
    m_bDirtyOccluders = TRUE;
}
