//==============================================================================
//
//  VerletCollision.hpp
//
//  A collection of fast world collision functions for vertlet sphere based physics 
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "VerletCollision.hpp"
#include "Render\Geom.hpp"
#include "Entropy.hpp"
#include "Objects\PlaySurface.hpp"
#include "PlaySurfaceMgr\PlaySurfaceMgr.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "CollisionMgr\PolyCache.hpp"

#define VERLET_MAX_COLL_CLUSTERS    128
poly_cache::cluster* s_ClusterPtr[VERLET_MAX_COLL_CLUSTERS];
s32                  s_nClusters;
bbox                 s_ClusterListBBox;

//==============================================================================
// DEBUG FUNCTIONS
//==============================================================================

//#define VERLET_DEBUG_COLLISION

//==============================================================================

#if !defined( CONFIG_RETAIL )

void VerletCollision_Render( void )
{
    draw_ClearL2W();
    draw_BBox( s_ClusterListBBox, XCOLOR_RED );

    for( s32 i=0; i<s_nClusters; i++ )
    {
        poly_cache::cluster* pCL = s_ClusterPtr[i];
        draw_BBox(pCL->BBox,XCOLOR_WHITE);
    }
}

#endif // !defined( CONFIG_RETAIL )

//==============================================================================

extern
xbool ComputeSphereTriCollision( const vector3* Tri, 
                                 const vector3& Start, 
                                 const vector3& End, 
                                       f32      Radius,
                                       f32&     FinalT,
                                       vector3& FinalHitPoint );

//==============================================================================
// PUBLIC FUNCTIONS
//==============================================================================

// Fast sphere cast.
xbool VerletCollision_SphereCast ( const vector3&       aStart,
                                   const vector3&       aEnd,
                                   const f32            aRadius,
                                         sphere_cast&   Cast )
{
#ifdef X_DEBUG
    // Check to make sure start and end spheres are inside bbox that was setup earlier!
    // (same as shrinking bbox by radius and checking points)
    bbox CheckBBox = s_ClusterListBBox ;
    CheckBBox.Deflate( aRadius, aRadius, aRadius ) ;
    ASSERT( CheckBBox.Intersect( aStart ) == TRUE ) ;
    ASSERT( CheckBBox.Intersect( aEnd   ) == TRUE ) ;
#endif

    // Compute dynamic bbox
    bbox DynamicBBox(aStart,aEnd);
    DynamicBBox.Inflate(aRadius+1,aRadius+1,aRadius+1);

    //
    // Were there no clusters?
    //
    if( s_nClusters==0 )
        return FALSE;

    vector3 SphereStart = aStart;
    vector3 SphereEnd   = aEnd;
    f32     Radius      = aRadius;

    //
    // Build culling flags
    //
    u32 CullFlags=0;
    vector3 Dir = SphereEnd - SphereStart;
    Dir.Normalize();
    if( Dir.GetX() > +0.001f ) CullFlags |= BOUNDS_X_POS;
    if( Dir.GetX() < -0.001f ) CullFlags |= BOUNDS_X_NEG;
    if( Dir.GetY() > +0.001f ) CullFlags |= BOUNDS_Y_POS;
    if( Dir.GetY() < -0.001f ) CullFlags |= BOUNDS_Y_NEG;
    if( Dir.GetZ() > +0.001f ) CullFlags |= BOUNDS_Z_POS;
    if( Dir.GetZ() < -0.001f ) CullFlags |= BOUNDS_Z_NEG;

    Cast.m_CollT = F32_MAX;

    //
    // Loop through the clusters and process the triangles
    //
    for( s32 iCL=0; iCL<s_nClusters; iCL++ )
    {
        poly_cache::cluster& CL = *s_ClusterPtr[iCL];

        // Skip over this cluster if dynamic bbox doesn't intersect
        if ( !DynamicBBox.Intersect(CL.BBox) )
            continue;

        s32 iQ = -1;
        while( 1 )
        {
            // Do tight loop on bbox checks and cull flags
            {
                iQ++;
                while( iQ < (s32)CL.nQuads )
                {
                    // Do flag culling
                    if( (CL.pBounds[iQ].Flags & CullFlags) == 0 )
                    {
                        // Do bbox culling
                        bbox* pBBox = (bbox*)(&CL.pBounds[iQ]);
                        if( DynamicBBox.Intersect( *pBBox ) ) 
                        {
                            break;
                        }
                    }
                    iQ++;
                }
                if( iQ==(s32)CL.nQuads )
                    break;
            }

            // Get access to this quad
            poly_cache::cluster::quad& QD = CL.pQuad[iQ];

            // Skip if moving away from quad
            vector3& N = CL.pNormal[ QD.iN ];
            if( N.Dot(Dir) > 0 )
                continue;

            // Check if starting sphere is behind plane
            if( (SphereStart.Dot(N) + CL.pBounds[iQ].PlaneD) < -Radius )
                continue;

            // Check if ending sphere is in front of plane
            if( (SphereEnd.Dot(N) + CL.pBounds[iQ].PlaneD) > Radius )
                continue;

            // Get the verts and call the actual collision routines
            {
                vector3 P[4];
                P[0] = CL.pPoint[ QD.iP[0] ];
                P[1] = CL.pPoint[ QD.iP[1] ];
                P[2] = CL.pPoint[ QD.iP[2] ];
                P[3] = CL.pPoint[ QD.iP[3] ];

                f32 FinalT;
                vector3 FinalHitPoint;

                if( ComputeSphereTriCollision( P, SphereStart, SphereEnd, Radius, FinalT, FinalHitPoint ) )
                {
                    if( FinalT < Cast.m_CollT )
                    {
                        Cast.m_CollT = FinalT;
                        vector3 CP = SphereStart + FinalT*(SphereEnd-SphereStart);
                        vector3 NM = CP - FinalHitPoint;
                        NM.Normalize();
                        Cast.m_CollPlane.Setup(FinalHitPoint,NM);
                    }
                }

                if( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD )
                {
                    P[1] = P[0];
                    if( ComputeSphereTriCollision( &P[1], SphereStart, SphereEnd, Radius, FinalT, FinalHitPoint ) )
                    {
                        if( FinalT < Cast.m_CollT )
                        {
                            Cast.m_CollT = FinalT;
                            vector3 CP = SphereStart + FinalT*(SphereEnd-SphereStart);
                            vector3 NM = CP - FinalHitPoint;
                            NM.Normalize();
                            Cast.m_CollPlane.Setup(FinalHitPoint,NM);
                        }
                    }
                }
            }
        }

    }

    return ( Cast.m_CollT != F32_MAX );
}

//==============================================================================

// Collects all the potential collidable objects within the world bbox
void VerletCollision_CollectObjects( const bbox& WorldBBox )
{
    (void)WorldBBox;

    //
    // Gather factored out list of clusters in dynamic area
    //
    s_ClusterListBBox = WorldBBox;
    g_PolyCache.BuildClusterList( s_ClusterListBBox, object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );
    
    //
    // Copy clusters into local array
    //
    s_nClusters = g_PolyCache.m_nClusters;
    ASSERT( s_nClusters < VERLET_MAX_COLL_CLUSTERS );
    if( s_nClusters > VERLET_MAX_COLL_CLUSTERS )
        s_nClusters = VERLET_MAX_COLL_CLUSTERS;
    x_memcpy( s_ClusterPtr, g_PolyCache.m_ClusterList, sizeof(poly_cache::cluster*)*s_nClusters);
}

//==============================================================================
