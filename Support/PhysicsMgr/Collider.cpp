//==============================================================================
//
//  Collider.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "Collider.hpp"
#include "RigidBody.hpp"
#include "PhysicsMgr.hpp"
#include "CollisionShape.hpp"


//==============================================================================
// TYPES
//==============================================================================

// Pointer to function for collider class collision function
typedef void ( collider::* collider_coll_func )( collision_shape* pColl0, collision_shape* pColl1 );

// Collider collision function info (used to define collision function lookup table)
struct coll_func_info
{
    collision_shape::type   m_Type0;        // Type of collision shape0
    collision_shape::type   m_Type1;        // Type of collision shape1
    collider_coll_func      m_Function;     // Collider collision function to call
};

//==============================================================================
// DATA
//==============================================================================
collider g_Collider;


// Collision lookup table for "( collision_shape::type, collision_shape::type )"
//      0: TYPE_SPHERE
//      1: TYPE_BOX
//      2: TYPE_CAPSULE
//      3: TYPE_WORLD
static coll_func_info s_CollFuncInfo[collision_shape::TYPE_COUNT][collision_shape::TYPE_COUNT] =
{
    // 0: TYPE_SPHERE versus ???
    {    
        {   collision_shape::TYPE_SPHERE,   collision_shape::TYPE_SPHERE,   &collider::CheckSpheresSpheres },   // TYPE_SPHERE v TYPE_SHERE
        {   collision_shape::TYPE_SPHERE,   collision_shape::TYPE_BOX,      &collider::CheckSpheresSpheres },   // TYPE_SPHERE v TYPE_BOX
        {   collision_shape::TYPE_SPHERE,   collision_shape::TYPE_CAPSULE,  &collider::CheckSpheresCapsule },   // TYPE_SPHERE v TYPE_CAPSULE
        {   collision_shape::TYPE_SPHERE,   collision_shape::TYPE_WORLD,    &collider::CheckSpheresWorld   },   // TYPE_SPHERE v TYPE_WORLD
    },

    // 1: TYPE_BOX versus ???
    {    
        {   collision_shape::TYPE_BOX,      collision_shape::TYPE_SPHERE,   NULL                            },  // TYPE_BOX v TYPE_SPHERE
        {   collision_shape::TYPE_BOX,      collision_shape::TYPE_BOX,      &collider::CheckSpheresSpheres  },  // TYPE_BOX v TYPE_BOX
        {   collision_shape::TYPE_BOX,      collision_shape::TYPE_CAPSULE,  &collider::CheckSpheresCapsule  },  // TYPE_BOX v TYPE_CAPSULE
        {   collision_shape::TYPE_BOX,      collision_shape::TYPE_WORLD,    &collider::CheckSpheresWorld    },  // TYPE_BOX v TYPE_WORLD
    },

    // 2: TYPE_CAPSULE versus ???
    {    
        {   collision_shape::TYPE_CAPSULE,  collision_shape::TYPE_SPHERE,   NULL                            },  // TYPE_CAPSULE v TYPE_SPHERE
        {   collision_shape::TYPE_CAPSULE,  collision_shape::TYPE_BOX,      NULL                            },  // TYPE_CAPSULE v TYPE_BOX
        {   collision_shape::TYPE_CAPSULE,  collision_shape::TYPE_CAPSULE,  &collider::CheckCapsuleCapsule  },  // TYPE_CAPSULE v TYPE_CAPSULE
        {   collision_shape::TYPE_CAPSULE,  collision_shape::TYPE_WORLD,    &collider::CheckSpheresWorld    },  // TYPE_CAPSULE v TYPE_WORLD
    },

        // 3: TYPE_WORLD versus ???
    {    
        {   collision_shape::TYPE_WORLD,    collision_shape::TYPE_SPHERE,   NULL                            },  // TYPE_WORLD v TYPE_SPHERE
        {   collision_shape::TYPE_WORLD,    collision_shape::TYPE_BOX,      NULL                            },  // TYPE_WORLD v TYPE_BOX
        {   collision_shape::TYPE_WORLD,    collision_shape::TYPE_CAPSULE,  NULL                            },  // TYPE_WORLD v TYPE_CAPSULE
        {   collision_shape::TYPE_WORLD,    collision_shape::TYPE_WORLD,    NULL                            },  // TYPE_WORLD v TYPE_WORLD
    },
};


//==============================================================================
// CONSTRUCTOR/DESTRUCTOR
//==============================================================================

collider::collider()
{
    PHYSICS_DEBUG_STATIC_MEM_ALLOC( sizeof( collider ) );

    m_pMovingColl    = NULL;
    m_nMaxCollisions = 4;
    m_nClusters      = 0;
    m_ClusterListBBox.Clear();
}

//==============================================================================

collider::~collider()
{
    PHYSICS_DEBUG_STATIC_MEM_FREE( sizeof( collider ) );
}

//==============================================================================
// FUNCTIONS
//==============================================================================

// Clears internal collisions
void collider::ClearCollisions( void )
{
    // Clear collisions
    for( s32 i = 0; i < m_nMaxCollisions; i++ )
        m_Collisions[i].m_Depth = -F32_MAX;
}

//==============================================================================

// Reports collisions to physics manager
s32 collider::ReportCollisions( collision_shape* pColl0, collision_shape* pColl1 )
{
/*
    // Lookup rigid bodies
    rigid_body* pBody0 = pColl0->GetOwner();
    rigid_body* pBody1 = pColl1->GetOwner();
    
    // Report any collisions?
    s32 nCollisions = 0;
    for( s32 i = 0; i < m_nMaxCollisions; i++)
    {
        // Collision setup
        collision& Collision = m_Collisions[i];
        if( Collision.m_Depth > -F32_MAX )
        {
            // Update stats
            nCollisions++;

            // Add world space collision
            g_PhysicsMgr.AddCollision( pBody0, 
                                       pBody1, 
                                       Collision.m_Point,
                                       Collision.m_Normal,
                                       Collision.m_Depth );
        }
    }
*/

    // Count the # of collisions to report and compute average collision
    s32 nCollisions = 0;
    s32 i;
    vector3 Position( 0.0f, 0.0f, 0.0f );
    vector3 Normal  ( 0.0f, 0.0f, 0.0f );
    f32     Depth = 0.0f;
    for( i = 0; i < m_nMaxCollisions; i++)
    {
        // Collision setup
        collision& Collision = m_Collisions[i];
        if( Collision.m_Depth > -F32_MAX )
        {
            // Update stats
            nCollisions++;

            // Accumulate
            Position += Collision.m_Point;
            Normal   += Collision.m_Normal;
            Depth    += Collision.m_Depth;
        }
    }
    // Nothing to do?
    if( !nCollisions )
        return 0;

    // Lookup rigid bodies
    rigid_body* pBody0 = pColl0->GetOwner();
    rigid_body* pBody1 = pColl1->GetOwner();

    // Report average?
    if( nCollisions > 1 )
    {
        // Compute average
        f32 Scale = 1.0f / (f32)nCollisions;
        Position *= Scale;
        Normal   *= Scale;
        Depth    *= Scale;
    
        // Report the average collision
        g_PhysicsMgr.AddCollision( pBody0, 
                                   pBody1, 
                                   Position,
                                   Normal,
                                   Depth );
    }
    
    // Report other collisions
    for( i = 0; i < m_nMaxCollisions; i++)
    {
        // Collision setup
        collision& Collision = m_Collisions[i];
        if( Collision.m_Depth > -F32_MAX )
        {
            // Add world space collision
            g_PhysicsMgr.AddCollision( pBody0, 
                                       pBody1, 
                                       Collision.m_Point,
                                       Collision.m_Normal,
                                       Collision.m_Depth );
        }
    }
    
    return nCollisions;
}

//==============================================================================

// Adds to collision list if it's one of the top deepest so far
void collider::RecordCollision( const collision& Collision )
{
    ASSERT( Collision.m_Depth != F32_MAX );
    ASSERT( Collision.m_Depth != -F32_MAX );

    // Find the least deepest current collision
    s32 I = 0;
    f32 D = m_Collisions[0].m_Depth; // Counter act expand!
    for( s32 i = 1; i < m_nMaxCollisions; i++ )
    {
        if( m_Collisions[i].m_Depth < D )
        {
            I = i;
            D = m_Collisions[i].m_Depth;
        }
    }

    // Record?
    if( Collision.m_Depth > D )
        m_Collisions[I] = Collision;
}

//==============================================================================

template< class T > inline void x_swap( T& a, T& b )  { T c = a; a = b; b = c; }
 
//==============================================================================

// Checks collision between 2 moving shapes
s32 collider::Check( collision_shape* pColl0, collision_shape* pColl1 )
{
    // Make sure they are valid
    ASSERT( pColl0 );
    ASSERT( pColl1 );
    ASSERT( pColl0 != pColl1 );
    
    // Lookup owner rigid bodies
    rigid_body* pBody0 = pColl0->GetOwner();
    rigid_body* pBody1 = pColl1->GetOwner();
    ASSERT( pBody0 );
    ASSERT( pBody1 );
    ASSERT( pBody0 != pBody1 );
    
    // Clear collision list
    ClearCollisions();

    // Make sure types are setup correctly
    ASSERT( pColl0->GetType() >= 0 );
    ASSERT( pColl1->GetType() >= 0 );
    ASSERT( pColl0->GetType() < collision_shape::TYPE_COUNT );
    ASSERT( pColl1->GetType() < collision_shape::TYPE_COUNT );

    // Make the higher shape be the moving one for better stacking
    if(     ( pBody1 == &g_ActorBody ) 
        ||  ( pBody1 == &g_WorldBody ) 
        ||  ( pBody0->GetPosition().GetY() > pBody1->GetPosition().GetY() ) )
    {            
        m_pMovingColl = pColl0;
    }
    else
    {            
        m_pMovingColl = pColl1;
    }
    
    // Fixup order ready for calling collision functions
    if( pColl1->GetType() < pColl0->GetType() )
        x_swap( pColl0, pColl1 );
    ASSERT( pColl0->GetType() <= pColl1->GetType() );
        
    // Lookup collision function
    coll_func_info& CollFuncInfo = s_CollFuncInfo[ pColl0->GetType() ][ pColl1->GetType() ];
    
    // Validate table and collision shape types are setup correctly
    ASSERT( CollFuncInfo.m_Type0 == pColl0->GetType() );
    ASSERT( CollFuncInfo.m_Type1 == pColl1->GetType() );
    ASSERT( CollFuncInfo.m_Function );
    
    // Check collision
    (this->*CollFuncInfo.m_Function)( pColl0, pColl1 );
        
    // Clear moving shape
    m_pMovingColl = NULL;
        
    // Report any collisions to physics manager
    return ReportCollisions( pColl0, pColl1 );
}

//==============================================================================
// Private functions
//==============================================================================

// Checks static sphere V sphere collision
void collider::SphereSphereCollision( const vector3&     SpherePos0,
                                      const f32          SphereRadius0,
                                      const vector3&     SpherePos1,
                                      const f32          SphereRadius1 )
{
    // Update stats
    PHYSICS_DEBUG_INC_COUNT( g_PhysicsMgr.m_Profile.m_nSphereSphereCollTests );
   
    // Compute distance to be apart
    f32 CheckDistSqr = x_sqr( SphereRadius0 + SphereRadius1 );
    
    // Intersecting?
    vector3 Dir     = SpherePos0 - SpherePos1;
    f32     DistSqr = Dir.LengthSquared();
    if( DistSqr < CheckDistSqr )
    {
        // Compute intersection depth
        f32 Depth = CheckDistSqr - DistSqr;
        if( Depth > 0.001f )
        {
            // Build collision
            collision Collision;
            Collision.m_Depth  = x_sqrt( Depth );
            Collision.m_Normal = Dir;
            Collision.m_Normal.Normalize();
            
            // Use average of collision pt on both spheres for final collision pt
            Collision.m_Point =  0.5f * (   ( SpherePos0 - ( Collision.m_Normal * SphereRadius0 ) )
                                          + ( SpherePos1 + ( Collision.m_Normal * SphereRadius1 ) ) );

            // Record
            RecordCollision( Collision );
        }
    }
}

//==============================================================================

// Checks sphere V sphere collision
void collider::SphereCapsuleCollision( const vector3&    SpherePos,
                                       const f32         SphereRadius,
                                       const vector3&    CapsuleStartPos,
                                       const vector3&    CapsuleEndPos,
                                       const f32         CapsuleRadius )
{
    // Update stats
    PHYSICS_DEBUG_INC_COUNT( g_PhysicsMgr.m_Profile.m_nSphereCapsuleCollTests );

    // Get delta and distance from sphere to closest pt on capsule
    vector3 Delta   = SpherePos.GetClosestVToLSeg( CapsuleStartPos, CapsuleEndPos  );
    f32     DistSqr = Delta.LengthSquared();
    
    // Overlapping?
    f32 CheckDistSqr = x_sqr( SphereRadius + CapsuleRadius );
    if( DistSqr < CheckDistSqr )
    {
        // Compute intersection depth
        f32 Depth = CheckDistSqr - DistSqr;
        if( Depth > 0.001f )
        {
            // Build collision
            collision Collision;
            Collision.m_Depth  = x_sqrt( Depth );
            Collision.m_Normal = -Delta;
            Collision.m_Normal.Normalize();
            Collision.m_Point = SpherePos - ( Collision.m_Normal * SphereRadius );
            
            // Record
            RecordCollision( Collision );
        }
    }
}

//==============================================================================
 
// Checks static capsule V capsule collision
void collider::CapsuleCapsuleCollision( const vector3&    CapsuleStartPos0,
                                        const vector3&    CapsuleEndPos0,
                                        const f32         CapsuleRadius0,
                                        const vector3&    CapsuleStartPos1,
                                        const vector3&    CapsuleEndPos1,
                                        const f32         CapsuleRadius1 )
{
    // Update stats
    PHYSICS_DEBUG_INC_COUNT( g_PhysicsMgr.m_Profile.m_nCapsuleCapsuleCollTests );

    // Compute radius checking distance
    f32 CheckDistSqr = x_sqr( CapsuleRadius0 + CapsuleRadius1 );
    
    // Compute closest points on lines
    vector3 P0,P1;    
    x_ClosestPtsOnLineSegs( CapsuleStartPos0, CapsuleEndPos0,
                            CapsuleStartPos1, CapsuleEndPos1,
                            P0, P1 );
                            
    // Get distance between them
    vector3 Delta   = P1 - P0;
    f32     DistSqr = Delta.LengthSquared();

    // Overlapping?    
    if( DistSqr < CheckDistSqr )
    {
        // Compute intersection depth
        f32 Depth = CheckDistSqr - DistSqr;
        if( Depth > 0.001f )
        {
            // Build collision
            collision Collision;
            Collision.m_Depth  = x_sqrt( Depth );
            Collision.m_Normal = -Delta;
            Collision.m_Normal.Normalize();
            
            // Use average of collision pt on both spheres for final collision pt
            Collision.m_Point =  0.5f * (     ( P0 - ( Collision.m_Normal * CapsuleRadius0 ) )
                                            + ( P1 + ( Collision.m_Normal * CapsuleRadius1 ) ) );

            // Record
            RecordCollision( Collision );
        }
    }
}

//==============================================================================

s32 collider::CollectWorldPolys( const bbox& WorldBBox )
{
#ifndef X_EDITOR
#ifdef sbroumley
    
    ASSERT( WorldBBox.IsValid() );
    
    ASSERT( WorldBBox.Min.GetX() != F32_MAX );
    ASSERT( WorldBBox.Min.GetY() != F32_MAX );
    ASSERT( WorldBBox.Min.GetZ() != F32_MAX );

    ASSERT( WorldBBox.Max.GetX() != -F32_MAX );
    ASSERT( WorldBBox.Max.GetY() != -F32_MAX );
    ASSERT( WorldBBox.Max.GetZ() != -F32_MAX );

    // TO DO - Draw a red bbox around the ragdoll that is causing this!
    ASSERT( WorldBBox.GetSize().GetX() < (100.0f * 50.0f ) );    
    ASSERT( WorldBBox.GetSize().GetY() < (100.0f * 50.0f ) );    
    ASSERT( WorldBBox.GetSize().GetZ() < (100.0f * 50.0f ) );    

#endif
#endif

    // Skip if not using polycache, but return 1 so collision is called
    if( !g_PhysicsMgr.m_Settings.m_bUsePolycache )
        return 1;

    // Gather factored out list of clusters in dynamic area
    m_ClusterListBBox = WorldBBox;
    g_PolyCache.BuildClusterList( m_ClusterListBBox, 
                                  object::TYPE_ALL_TYPES, 
                                  object::ATTR_BLOCKS_RAGDOLL, 
                                  object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );

    // Copy clusters into local array
    m_nClusters = g_PolyCache.m_nClusters;
    ASSERT( m_nClusters < MAX_CLUSTERS );
    if( m_nClusters > MAX_CLUSTERS )
        m_nClusters = MAX_CLUSTERS;
    x_memcpy( m_pClusters, g_PolyCache.m_ClusterList, sizeof(poly_cache::cluster*) * m_nClusters );
    
    // Update stats
    PHYSICS_DEBUG_ADD_COUNT( g_PhysicsMgr.m_Profile.m_nClusters, m_nClusters );
    
#ifdef ENABLE_PHYSICS_DEBUG    
    // Loop through the clusters and process the triangles
    for( s32 iCL=0; iCL < m_nClusters; iCL++ )
    {
        // Lookup cluster
        poly_cache::cluster& CL = *m_pClusters[iCL];
        
        // Update count
        g_PhysicsMgr.m_Profile.m_nNGons += CL.nQuads;        
    }
#endif        
    
    // Return cluster count
    return m_nClusters;
}

//==============================================================================

// Returns TRUE if sphere intersects NGon
xbool collider::SphereNGonIntersection( const plane&   Plane,
                                        const vector3* Verts, 
                                        const s32      nVerts, 
                                        const vector3& SpherePos, 
                                        const f32      SphereRadius )
{
    s32 i;

    // Update stats
    PHYSICS_DEBUG_INC_COUNT( g_PhysicsMgr.m_Profile.m_nSphereNGonIntersecTests );

    // Does sphere intersect the infinite plane?
    f32 Dist = Plane.Distance( SpherePos );
    if( x_abs( Dist ) > SphereRadius )
        return FALSE;

    // Check to see if sphere is within all edges
    for( i = 0; i < nVerts; i++ )
    {
        // Lookup edge end pts
        const vector3& EdgeStart = Verts[( i == 0 ) ? nVerts-1 : i-1 ];
        const vector3& EdgeEnd   = Verts[ i ];

        // Exit loop if point is outside of edge
        vector3 EdgeDir        = EdgeEnd - EdgeStart;
        vector3 EdgeNormal     = EdgeDir.Cross( Plane.Normal );
        vector3 EdgePointDelta = EdgeStart - SpherePos;
        if( EdgeNormal.Dot( EdgePointDelta ) < 0 )
            break;
    }

    // If point is in NGon then sphere intersects
    if( i == nVerts )
    {
        return TRUE;
    }

    // Intersection pt is not in NGon, so check for sphere v edge intersections
    f32 SphereRadiusSqr = SphereRadius * SphereRadius;
    for( i = 0; i < nVerts; i++ )
    {
        // Lookup edge end pts
        const vector3& EdgeStart = Verts[( i == 0 ) ? nVerts-1 : i-1 ];
        const vector3& EdgeEnd   = Verts[ i ];

        // Does edge intersection sphere?
        f32 DistSqr = SpherePos.GetSqrtDistToLineSeg( EdgeStart, EdgeEnd );
        if( DistSqr < SphereRadiusSqr )
            return TRUE;
    }

    // No intersection
    return FALSE;
}

//==============================================================================

// Returns TRUE if sphere intersects any of world
xbool collider::SphereWorldIntersection( const vector3& SpherePos, const f32 SphereRadius )
{
    // Use ground plane?
    if( !g_PhysicsMgr.m_Settings.m_bUsePolycache )
    {
        // Setup ground plane
        plane Plane;
        Plane.Setup( vector3( 0.0f, 1.0f, 0.0f ), 0.0f );

        // Call NGon function
        return SphereNGonIntersection( Plane, NULL, 0, SpherePos, SphereRadius );
    }
    
    // Should only call if clusters are present
    ASSERT( m_nClusters );

    // Create bounding box for sphere
    bbox SphereBBox( SpherePos, SphereRadius );

#ifdef X_DEBUG
    // Check to make sure sphere is inside bbox that was setup earlier!
    ASSERT( SphereBBox.Min.GetX() >= m_ClusterListBBox.Min.GetX() );
    ASSERT( SphereBBox.Min.GetY() >= m_ClusterListBBox.Min.GetY() );
    ASSERT( SphereBBox.Min.GetY() >= m_ClusterListBBox.Min.GetY() );
    ASSERT( SphereBBox.Max.GetX() <= m_ClusterListBBox.Max.GetX() );
    ASSERT( SphereBBox.Max.GetY() <= m_ClusterListBBox.Max.GetY() );
    ASSERT( SphereBBox.Max.GetY() <= m_ClusterListBBox.Max.GetY() );
#endif

    // Keeping these out the loop stops debug crashes?!
    plane   Plane;
    vector3 Verts[4];

    // Loop through the clusters and process the triangles
    for( s32 iCL=0; iCL < m_nClusters; iCL++ )
    {
        poly_cache::cluster& CL = *m_pClusters[iCL];

        // Skip over this cluster if sphere bbox doesn't intersect cluster bbox
        if ( !SphereBBox.Intersect(CL.BBox) )
            continue;

        s32 iQ = -1;
        while( 1 )
        {
            // Do tight loop on bbox checks and cull flags
            {
                iQ++;
                while( iQ < (s32)CL.nQuads )
                {
                    // Do bbox culling
                    bbox* pBBox = (bbox*)(&CL.pBounds[iQ]);
                    if( SphereBBox.Intersect( *pBBox ) ) 
                    {
                        break;
                    }
                    iQ++;
                }
                if( iQ==(s32)CL.nQuads )
                    break;
            }

            // Get access to this quad
            poly_cache::cluster::quad& QD = CL.pQuad[iQ];

            // Setup plane
            Plane.Normal = CL.pNormal[ QD.iN ];
            Plane.D      = CL.pBounds[ iQ ].PlaneD;

            // Check intersection
            if( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD )
            {
                // Get verts
                Verts[0] = CL.pPoint[ QD.iP[0] ];
                Verts[1] = CL.pPoint[ QD.iP[1] ];
                Verts[2] = CL.pPoint[ QD.iP[2] ];
                Verts[3] = CL.pPoint[ QD.iP[3] ];
            
                // Check for quad intersection
                if( SphereNGonIntersection( Plane, Verts, 4, SpherePos, SphereRadius ) )
                    return TRUE;
            }
            else
            {
                // Get verts
                Verts[0] = CL.pPoint[ QD.iP[0] ];
                Verts[1] = CL.pPoint[ QD.iP[1] ];
                Verts[2] = CL.pPoint[ QD.iP[2] ];
            
                // Check for tri intersection
                if( SphereNGonIntersection( Plane, Verts, 3, SpherePos, SphereRadius ) )
                    return TRUE;
            }                        
        }            
    }
    
    // No intersection
    return FALSE;
}

//==============================================================================

// Check sweeping sphere V NGon collision
xbool collider::SphereNGonCollision( const plane&   Plane,
                                     const vector3* Verts,
                                     const s32      nVerts,
                                     const vector3& SphereStartPos,
                                     const vector3& SphereEndPos,
                                     const f32      SphereRadius,
                                           f32&     CollT,
                                           plane&   CollPlane )
{
    s32 i;

    // Update stats    
    PHYSICS_DEBUG_INC_COUNT( g_PhysicsMgr.m_Profile.m_nSphereNGonCollTests );
    
    // Moving away from plane?
    vector3 Dir = SphereEndPos - SphereStartPos;
    f32 DirDotNormal = Plane.Normal.Dot( Dir );
    if( DirDotNormal > 0.0f )
        return FALSE;
     
    // Check if starting sphere is behind plane
    f32 StartDist = Plane.Distance( SphereStartPos );
    if( StartDist < -SphereRadius )
        return FALSE;

    // Check if ending sphere is in front of plane
    f32 EndDist = Plane.Distance( SphereEndPos );
    if( EndDist > SphereRadius )
        return FALSE;

    // Compute intersection ratio
    vector3 SphereBot = SphereStartPos - ( Plane.Normal * SphereRadius );
    f32     T         = -Plane.Distance( SphereBot ) / DirDotNormal;

    // No collision?
    if( ( T < 0.0f ) || ( T >= CollT ) )
        return FALSE;

    // Compute point on plane at intersection
    vector3 Point = SphereBot + ( T * Dir );

    // Check to see if point is within all edges
    for( i = 0; i < nVerts; i++ )
    {
        // Lookup edge end pts
        const vector3& EdgeStart = Verts[( i == 0 ) ? nVerts-1 : i-1 ];
        const vector3& EdgeEnd   = Verts[ i ];
    
        // Exit loop if point is outside of edge
        vector3 EdgeDir        = EdgeEnd - EdgeStart;
        vector3 EdgeNormal     = EdgeDir.Cross( Plane.Normal );
        vector3 EdgePointDelta = EdgeStart - Point;
        if( EdgeNormal.Dot( EdgePointDelta ) < 0 )
            break;
    }

    // If point is in NGon then collision is valid
    if( i == nVerts )
    {
        CollT     = T;
        CollPlane = Plane;
        return TRUE;
    }
            
    // Intersection pt is not in NGon, so check for sphere v edge collisions
    f32   SphereRadiusSqr = SphereRadius * SphereRadius;
    xbool bCollision = FALSE;
    for( i = 0; i < nVerts; i++ )
    {
        // Lookup edge end pts
        const vector3& EdgeStart = Verts[( i == 0 ) ? nVerts-1 : i-1 ];
        const vector3& EdgeEnd   = Verts[ i ];
    
        // Get closest pts between sphere movement and edge
        vector3 SpherePoint, EdgePoint;
        f32     SphereT, EdgeT;
        x_ClosestPtsOnLineSegs( SphereStartPos, SphereEndPos, 
                                EdgeStart,   EdgeEnd, 
                                SpherePoint, EdgePoint,
                                SphereT,     EdgeT );
    
        // Skip if not closest edge so far
        if( SphereT > CollT ) 
            continue;
            
        // Intersection - is distance between closest pts on line less than sphere radius?
        vector3 Delta   = SpherePoint - EdgePoint;
        f32     DistSqr = Delta.LengthSquared();
        if( DistSqr < SphereRadiusSqr )
        {
            // Really the edge -> sphere collision normal should be computed by casting
            // a ray in the opposite direction of the sphere movement towards the sphere
            // start pos, but we don't need that much accuracy.
        
            // Almost on edge?
            if( DistSqr < 0.00001f )
            {
                // Collide with plane
                bCollision = TRUE;
                CollT      = SphereT;
                CollPlane  = Plane;
            }
            else
            {
                // Setup plane from edge point and direction to sphere
                bCollision = TRUE;
                CollT      = SphereT;
                CollPlane.Setup( EdgePoint, Delta );
            }
        }
    }
    
    return bCollision;
}

//==============================================================================

// Checks sweeping sphere V world collision        
xbool collider::SphereWorldCollision( const vector3& SphereStartPos,
                                      const vector3& SphereEndPos,
                                      const f32      SphereRadius,
                                            f32&     CollT,
                                            plane&   CollPlane )
{
    // Use ground plane?
    if( !g_PhysicsMgr.m_Settings.m_bUsePolycache )
    {
        // Setup ground plane
        plane Plane;
        Plane.Setup( vector3( 0.0f, 1.0f, 0.0f ), 0.0f );

        // Collide with ground plane
        return SphereNGonCollision( Plane, NULL, 0, SphereStartPos, SphereEndPos, SphereRadius, CollT, CollPlane );
    }

    // Should only call if clusters are present
    ASSERT( m_nClusters );

    // Compute dynamic bbox
    bbox DynamicBBox( SphereStartPos, SphereEndPos );
    DynamicBBox.Inflate( SphereRadius + 1.0f, SphereRadius + 1.0f, SphereRadius + 1.0f );
    
#ifdef X_DEBUG
    // Check to make sure start and end spheres are inside bbox that was setup earlier!
    bbox SphereBBox( SphereStartPos, SphereEndPos );
    SphereBBox.Inflate( SphereRadius, SphereRadius, SphereRadius );
    ASSERT( SphereBBox.Min.GetX() >= m_ClusterListBBox.Min.GetX() );
    ASSERT( SphereBBox.Min.GetY() >= m_ClusterListBBox.Min.GetY() );
    ASSERT( SphereBBox.Min.GetY() >= m_ClusterListBBox.Min.GetY() );
    ASSERT( SphereBBox.Max.GetX() <= m_ClusterListBBox.Max.GetX() );
    ASSERT( SphereBBox.Max.GetY() <= m_ClusterListBBox.Max.GetY() );
    ASSERT( SphereBBox.Max.GetY() <= m_ClusterListBBox.Max.GetY() );
#endif

    // Keeping these out the loop stops debug crashes?!
    plane   Plane;
    vector3 Verts[4];

    // Build culling flags
    u32     CullFlags = 0;
    vector3 Dir       = SphereEndPos - SphereStartPos;
    Dir.Normalize();
    if( Dir.GetX() > +0.001f ) CullFlags |= BOUNDS_X_POS;
    if( Dir.GetX() < -0.001f ) CullFlags |= BOUNDS_X_NEG;
    if( Dir.GetY() > +0.001f ) CullFlags |= BOUNDS_Y_POS;
    if( Dir.GetY() < -0.001f ) CullFlags |= BOUNDS_Y_NEG;
    if( Dir.GetZ() > +0.001f ) CullFlags |= BOUNDS_Z_POS;
    if( Dir.GetZ() < -0.001f ) CullFlags |= BOUNDS_Z_NEG;
    
    // Loop through the clusters and process the triangles
    xbool bCollision = FALSE;
    for( s32 iCL=0; iCL < m_nClusters; iCL++ )
    {
        poly_cache::cluster& CL = *m_pClusters[iCL];

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

            // Setup plane
            Plane.Normal = CL.pNormal[QD.iN];
            Plane.D      = CL.pBounds[iQ].PlaneD;

            // Check collision
            if( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD )
            {
                // Get verts
                Verts[0] = CL.pPoint[ QD.iP[0] ];
                Verts[1] = CL.pPoint[ QD.iP[1] ];
                Verts[2] = CL.pPoint[ QD.iP[2] ];
                Verts[3] = CL.pPoint[ QD.iP[3] ];
            
                // Check for quad intersection
                bCollision |= SphereNGonCollision( Plane, Verts, 4, 
                                                   SphereStartPos, SphereEndPos, SphereRadius, 
                                                   CollT, CollPlane );
            }
            else
            {
                // Get verts
                Verts[0] = CL.pPoint[ QD.iP[0] ];
                Verts[1] = CL.pPoint[ QD.iP[1] ];
                Verts[2] = CL.pPoint[ QD.iP[2] ];
            
                // Check for tri intersection
                bCollision |= SphereNGonCollision( Plane, Verts, 3, 
                                                    SphereStartPos, SphereEndPos, SphereRadius, 
                                                    CollT, CollPlane );
            }                        
        }
    }
    return bCollision;
}

//==============================================================================

// Checks collision between moving sphere list and world
void collider::CheckSpheresWorld( collision_shape* pColl0, collision_shape* pColl1 )
{
    // Collision shapes should be valid and contain data!
    ASSERT( pColl0 );
    ASSERT( pColl0->GetNSpheres() );
    ASSERT( pColl1 );
    ASSERT( pColl1 == &g_WorldColl );
    (void)pColl1;

    s32     i;
    vector3 Dir;
    f32     Dist, DistSqr;

#ifdef ENABLE_PHYSICS_DEBUG
    // Clear debug collision rendering
    pColl0->DebugClearCollision();
#endif            

    // If there are no cluster, then just update spheres fast and get out of here
    if( m_nClusters == 0 )
    {
        // Update all spheres
        for( i = 0; i < pColl0->GetNSpheres(); i++ )
        {
            // Lookup sphere
            collision_shape::sphere& Sphere = pColl0->GetSphere( i );

            // Update collision free pos
            Sphere.m_CollFreePos = Sphere.m_CurrPos;
        }
        
        // Nothing else to do...
        return;    
    }

/*
    // Compute bounding bbox around all start positions of collision shape
    bbox BodyBBox;
    BodyBBox.Clear();
    for( i = 0; i < pColl0->GetNSpheres(); i++ )
        BodyBBox += pColl0->GetSphere(i).m_PrevPos;
    f32 Inflate = pColl0->GetSphereRadius() + 1.0f;
    BodyBBox.Inflate( Inflate, Inflate, Inflate );

    // Compute bounding sphere    
    vector3 BodyCenter = BodyBBox.GetCenter();
    f32     BodyRadius = BodyBBox.GetRadius();
    
    // If shape start does not intersect the world, then update the collision free position
    if( SphereWorldIntersection( BodyCenter, BodyRadius ) == FALSE )
    {
        // Update all spheres
        for( i = 0; i < pColl0->GetNSpheres(); i++ )
        {
            // Lookup sphere
            collision_shape::sphere& Sphere = pColl0->GetSphere( i );
            
            // Update collision free pos
            Sphere.m_CollFreePos = Sphere.m_CurrPos;
        }
    }
*/

    // Lookup back off dist
    f32 HitBackoffDist = g_PhysicsMgr.m_Settings.m_CollisionHitBackoffDist;

    // Check all spheres against world for first collision
    const f32 SphereRadius = pColl0->m_Radius;
    for( i = 0; i < pColl0->GetNSpheres(); i++ )
    {
        // Get sphere info
        collision_shape::sphere& Sphere = pColl0->GetSphere( i );

        // If movement is collision free, then update collision 
        // free pos to stop ragdoll getting hung up on stuff
        bbox    MoveBBox( Sphere.m_PrevPos, Sphere.m_CurrPos );
        MoveBBox.Inflate( SphereRadius, SphereRadius, SphereRadius );
        if( SphereWorldIntersection( MoveBBox.GetCenter(), MoveBBox.GetRadius() ) == FALSE )
        {
            // No collision, so just update collision pos
            Sphere.m_CollFreePos = Sphere.m_CurrPos;
        }
        else
        {
            // Lookup movement
            const vector3& SphereStartPos = Sphere.m_CollFreePos;
            const vector3& SphereEndPos   = Sphere.m_CurrPos;

            // Skip if not moving very far
            Dir     = SphereEndPos - SphereStartPos;
            DistSqr = Dir.LengthSquared();
            if( DistSqr < x_sqr( 0.001f ) )
                continue;
            
            // Collision with world?
            f32   CollT = F32_MAX;
            plane CollPlane;
            if( SphereWorldCollision( SphereStartPos, SphereEndPos, SphereRadius, CollT, CollPlane ) )
            {
                // Record for debug render
                #ifdef ENABLE_PHYSICS_DEBUG
                Sphere.m_bCollision = TRUE;
                #endif

                // Record the collision (keep as if it happened at the end position)
                collision   Collision;
                Collision.m_Normal = CollPlane.Normal;
                Collision.m_Point  = SphereEndPos - ( Collision.m_Normal * SphereRadius );
                Collision.m_Depth  = -CollPlane.Distance( Collision.m_Point );
                RecordCollision( Collision );
                
                // Pull back from collision a bit
                Dist  = x_sqrt( DistSqr );
                CollT -= HitBackoffDist / Dist;
                if( CollT < 0 )
                    CollT = 0.0f;

                // Update collision free pos to be just before the collision
                Sphere.m_CollFreePos += CollT * ( SphereEndPos - SphereStartPos );
            }
            else
            {
                // No collision so update collision free pos to be end of movement
                Sphere.m_CollFreePos = Sphere.m_CurrPos;
            }
        }
    }
}

//==============================================================================

// Checks collision between 2 moving sphere lists
void collider::CheckSpheresSpheres( collision_shape* pColl0, collision_shape* pColl1 )
{
    // Validate
    ASSERT( pColl0 );
    ASSERT( pColl1 );
    ASSERT( pColl0->GetNSpheres() );
    ASSERT( pColl1->GetNSpheres() );
    ASSERT(     ( pColl0->GetType() == collision_shape::TYPE_SPHERE  )
            ||  ( pColl0->GetType() == collision_shape::TYPE_BOX     )
            ||  ( pColl0->GetType() == collision_shape::TYPE_CAPSULE ) );
    ASSERT(     ( pColl1->GetType() == collision_shape::TYPE_SPHERE  )
            ||  ( pColl1->GetType() == collision_shape::TYPE_BOX     )
            ||  ( pColl1->GetType() == collision_shape::TYPE_CAPSULE ) );

    // Lookup radii
    const f32 Radius0 = pColl0->GetRadius();
    const f32 Radius1 = pColl1->GetRadius();

    // Check all spheres against all other spheres    
    for( s32 i = 0; i < pColl0->GetNSpheres(); i++ )
    {
        // Get sphere0
        collision_shape::sphere& Sphere0 = pColl0->GetSphere( i );

        // Check against all other spheres    
        for( s32 j = 0; j < pColl1->GetNSpheres(); j++ )
        {
            // Get sphere1
            collision_shape::sphere& Sphere1 = pColl1->GetSphere( j );

            // Check for collision      
            if( m_pMovingColl == pColl0 )
            {              
                SphereSphereCollision( Sphere0.m_CurrPos, Radius0,
                                       Sphere1.m_PrevPos, Radius1 );
            }                                    
            else
            {              
                SphereSphereCollision( Sphere0.m_PrevPos, Radius0,
                                       Sphere1.m_CurrPos, Radius1 );
            }                                    
        }                                   
    }
}

//==============================================================================

// Checks collision between moving sphere list and a capsule
void collider::CheckSpheresCapsule( collision_shape* pColl0, collision_shape* pColl1 )
{
    // Validate
    ASSERT( pColl0 );
    ASSERT( pColl1 );
    ASSERT( pColl0->GetNSpheres() );
    ASSERT( pColl1->GetNSpheres() );
    ASSERT(     ( pColl0->GetType() == collision_shape::TYPE_SPHERE  )
            ||  ( pColl0->GetType() == collision_shape::TYPE_BOX     )
            ||  ( pColl0->GetType() == collision_shape::TYPE_CAPSULE ) );
    ASSERT( pColl1->GetType() == collision_shape::TYPE_CAPSULE );

    // Loop through all spheres and collide with capsule
    for( s32 i = 0; i < pColl0->GetNSpheres(); i++ )
    {
        // Lookup sphere
        collision_shape::sphere& Sphere0 = pColl0->GetSphere( i );
        
        // Check for collision
        if( m_pMovingColl == pColl0 )
        {        
            SphereCapsuleCollision( Sphere0.m_CurrPos,
                                    pColl0->GetRadius(),
                                    pColl1->GetCapsulePrevStartPos(),
                                    pColl1->GetCapsulePrevEndPos(),
                                    pColl1->GetRadius() );
        }
        else
        {
            SphereCapsuleCollision( Sphere0.m_PrevPos,
                                    pColl0->GetRadius(),
                                    pColl1->GetCapsuleCurrStartPos(),
                                    pColl1->GetCapsuleCurrEndPos(),
                                    pColl1->GetRadius() );
        }
    }
}

//==============================================================================

// Checks collision between 2 capsules
void collider::CheckCapsuleCapsule( collision_shape* pColl0, collision_shape* pColl1 )
{
    // Validate
    ASSERT( pColl0 );
    ASSERT( pColl1 );
    ASSERT( pColl0->GetNSpheres() );
    ASSERT( pColl1->GetNSpheres() );
    ASSERT( pColl0->GetType() == collision_shape::TYPE_CAPSULE );
    ASSERT( pColl1->GetType() == collision_shape::TYPE_CAPSULE );
    
    // Check for collision
    if( m_pMovingColl == pColl0 )
    {
        CapsuleCapsuleCollision( pColl0->GetCapsuleCurrStartPos(),
                                 pColl0->GetCapsuleCurrEndPos(),
                                 pColl0->GetRadius(),
                                 pColl1->GetCapsulePrevStartPos(),
                                 pColl1->GetCapsulePrevEndPos(),
                                 pColl1->GetRadius() );
    }
    else
    {
        CapsuleCapsuleCollision( pColl0->GetCapsulePrevStartPos(),
                                 pColl0->GetCapsulePrevEndPos(),
                                 pColl0->GetRadius(),
                                 pColl1->GetCapsuleCurrStartPos(),
                                 pColl1->GetCapsuleCurrEndPos(),
                                 pColl1->GetRadius() );
    }
}

//==============================================================================

