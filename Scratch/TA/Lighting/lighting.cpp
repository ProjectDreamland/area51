
#include "lighting.hpp"
#include "Entropy.hpp"
#include <crtdbg.h>
//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

lighting::~lighting( void )
{
    s32 i;

    //
    // Clear all the plane nodes
    //
    for( i=0; i<m_nNodes; i++ )
    {
        if( m_pNode[i].pFacet   ) x_free( m_pNode[i].pFacet );

        m_pNode[i].pFacet   = NULL;
    }

    //
    // Clear all the facets/vert/lights
    //
    if( m_pVertex  )  x_free( m_pVertex  );
    if( m_pFacet   )  x_free( m_pFacet   );
    if( m_pLight   )  x_free( m_pLight   );
    if( m_pNode    )  x_free( m_pNode    );

    m_pVertex   = NULL;
    m_pFacet    = NULL;
    m_pLight    = NULL;
    m_pNode     = NULL;
}

//=========================================================================

lighting::lighting( void )
{
    m_nNodes = 0;

    //
    // Buffer and lights
    //
    m_BuffNodeSize = 1000;
    m_nNodes       = 0;
    m_pNode        = (node*)x_malloc( sizeof(node) * m_BuffNodeSize );

    m_VBuffSize    = 1000;
    m_nVertices    = 0;
    m_pVertex      = (vertex*)x_malloc( sizeof(vertex) * m_VBuffSize );

    m_TBuffSize    = 1000;
    m_nFacets      = 0;
    m_pFacet       = (facet*)x_malloc( sizeof(facet) * m_TBuffSize );

    m_LBuffSize    = 100;
    m_nLights      = 0;
    m_pLight       = (light*)x_malloc( sizeof(light) * m_LBuffSize );

    if( m_pVertex == NULL || m_pFacet == NULL || m_pLight == NULL || m_pNode == NULL )
    {
        x_throw( "Out fo memory" );
    }

    //
    // Stats
    //
    m_TotalVerticesRemoved          = 0;
    m_BSPNumberOfFacetsResolved     = 0;
    m_BSPDuplicatedFacets           = 0;

    m_BBox.Clear();

    //
    // Collision info
    //
    m_bFindClosestIntersection      = FALSE;
    m_CollisionIndexFacet           = -1;
    m_CollisionT                    = -1;
    m_NumberOfTriRayIntersections   =  0;
    m_NumberOfRaysCheck             =  0;
    m_NumberOfNodeTravel            =  0;
}

//=========================================================================

void lighting::AddLight( vector3& Pos, f32 AttenR, xcolor Color, xcolor AmbColor )
{
    //
    // Make sure that we have space for the light
    //
    if( m_nLights >= (m_LBuffSize - 1) )
    {
        m_LBuffSize += m_LBuffSize/2;
        m_pLight      = (light*)x_realloc( m_pLight, sizeof(light) * m_LBuffSize );
        if( m_pLight == NULL ) 
            x_throw( "Out of memory" );
    }

    light& Light = m_pLight[ m_nLights++ ];
    Light.Position  = Pos;
    Light.AttenR    = AttenR;
    Light.Color     = Color;
    Light.AmbLight  = AmbColor;

    Light.BBox.Set( Light.Position, Light.AttenR );
}

//=========================================================================

void lighting::AddFacet( 
    vector3& P0, 
    vector3& N0, 
    vector3& P1, 
    vector3& N1,
    vector3& P2, 
    vector3& N2, 
    guid     Object, 
    u32      UserData )
{
    //
    // Make sure that we have all the memory that we need
    //
    if( m_nFacets >= (m_TBuffSize - 1) )
    {
        m_TBuffSize += m_TBuffSize/2;
        m_pFacet      = (facet*)x_realloc( m_pFacet, sizeof(facet) * m_TBuffSize );
        if( m_pFacet == NULL ) 
            x_throw( "Out of memory" );
    }

    if( m_nVertices >= (m_VBuffSize-3) )
    {
        m_VBuffSize += m_VBuffSize/2;
        m_pVertex      = (vertex*)x_realloc( m_pVertex, sizeof(vertex) * m_VBuffSize );
        if( m_pVertex == NULL ) 
            x_throw( "Out of memory" );
    }

    //
    // Add the triangle in the list
    //
    m_pFacet[ m_nFacets ].gObject    = Object;
    m_pFacet[ m_nFacets ].UserData   = UserData;

    m_pFacet[ m_nFacets ].BBox.Clear();
    m_pFacet[ m_nFacets ].BBox.AddVerts( &P0, 1 );
    m_pFacet[ m_nFacets ].BBox.AddVerts( &P1, 1 );
    m_pFacet[ m_nFacets ].BBox.AddVerts( &P2, 1 );

    // Include this facet in the main bbox
    m_BBox += m_pFacet[ m_nFacets ].BBox;

    m_pFacet[ m_nFacets ].iVertex[0] = m_nVertices+0;
    m_pFacet[ m_nFacets ].iVertex[1] = m_nVertices+1;
    m_pFacet[ m_nFacets ].iVertex[2] = m_nVertices+2;

    // Build the plane for the triangle
    vector3 Normal  = v3_Cross( P1-P0, P2-P0 );
    f32     Area    = Normal.Dot( Normal );

    // The area of this triangle is very close to zero or zero
    if( Area < 0.001f )
        return;

    // Set the main triangle normal
    Normal.Scale( (f32)sqrt(Area) );
    m_pFacet[ m_nFacets ].Plane.Normal = Normal;
    m_pFacet[ m_nFacets ].Plane.D      = -Normal.Dot( P0 );

    // now build the planes for the edges. (This could be factor out for all facets)
    m_pFacet[ m_nFacets ].EdgePlane[0].Setup( P0, P1, P0 + m_pFacet[ m_nFacets ].Plane.Normal );
    m_pFacet[ m_nFacets ].EdgePlane[1].Setup( P1, P2, P1 + m_pFacet[ m_nFacets ].Plane.Normal );
    m_pFacet[ m_nFacets ].EdgePlane[2].Setup( P2, P0, P2 + m_pFacet[ m_nFacets ].Plane.Normal );

    ASSERT( m_pFacet[ m_nFacets ].EdgePlane[0].InFront( P2 ) == FALSE );
    ASSERT( m_pFacet[ m_nFacets ].EdgePlane[1].InFront( P0 ) == FALSE );
    ASSERT( m_pFacet[ m_nFacets ].EdgePlane[2].InFront( P1 ) == FALSE );

    // Done building the facet
    m_nFacets++;
    
    //
    // Add the vertices
    //
    x_memset( &m_pVertex[m_nVertices], 0, sizeof(vertex) );
    m_pVertex[m_nVertices].Position = P0;
    m_pVertex[m_nVertices].Normal   = N0;
    m_pVertex[m_nVertices].Normal.Normalize();
    m_nVertices++;

    x_memset( &m_pVertex[m_nVertices], 0, sizeof(vertex) );
    m_pVertex[m_nVertices].Position = P1;
    m_pVertex[m_nVertices].Normal   = N1;
    m_pVertex[m_nVertices].Normal.Normalize();
    m_nVertices++;

    x_memset( &m_pVertex[m_nVertices], 0, sizeof(vertex) );
    m_pVertex[m_nVertices].Position = P2;
    m_pVertex[m_nVertices].Normal   = N2;
    m_pVertex[m_nVertices].Normal.Normalize();
    m_nVertices++;
}

//=========================================================================

xbool lighting::TempVCompare( const vertex& A, const vertex& B )
{
    // Check position
    {
        static const f32 PEpsilon = 0.001f; 
        vector3 T;
        T = A.Position - B.Position;
        f32 d = T.Dot( T );
        if( d > PEpsilon ) return FALSE;
    }
    
    // Check normal
    {
        static const f32 NEpsilon = 0.001f;
        vector3 T;
        T = B.Normal - A.Normal;
        f32 d = T.Dot( T );
        if( d > NEpsilon ) return FALSE;
    }

    return TRUE;
}

//=========================================================================
// Collapse vertices that are too close from each other and have 
// the same properties.
//=========================================================================
void lighting::CollapseVertexPositions( void )
{
    struct hash
    {
        s32 iVRemap;
        s32 iNext;
    };

    struct tempv
    {
        s32     RemapIndex;     // Which vertex Index it shold now use. 
        s32     Index;          // Inde to the original vertex
        s32     iNext;          // next node in the has 
    };

    if( m_nVertices <= 0 )
        x_throw( "RawMesh has not vertices" );

    s32         i;
    hash*       pHash     = NULL;
    tempv*      pTempV    = NULL;
    vertex*     pVertex   = NULL;
    s32         HashSize  = MAX( 20, m_nVertices*10 );
    f32         MaxX, MinX, Shift;

    // begin the error block
    x_try;

    // Allocate memory
    pHash   = new hash  [ HashSize ];
    pTempV  = new tempv [ m_nVertices ];

    if( pTempV == NULL || pHash == NULL )
        x_throw( "Out of memory" );

    // Initialize the hash with terminators
    for( i=0; i<HashSize; i++) 
    {
        pHash[i].iNext = -1;
    }

    // Fill the nodes for each of the dimensions
    MaxX = m_pVertex[0].Position.X;
    MinX = MaxX;
    for( i=0; i<m_nVertices; i++) 
    {
        pTempV[i].Index         =  i;
        pTempV[i].iNext         = -1;
        pTempV[i].RemapIndex    =  i;
       
        MaxX = MAX( MaxX, m_pVertex[i].Position.X );
        MinX = MIN( MinX, m_pVertex[i].Position.X );
    }

    // Hash all the vertices into the hash table
    Shift = HashSize/(MaxX-MinX+1);
    for( i=0; i<m_nVertices; i++) 
    {
        s32 OffSet = (s32)(( m_pVertex[i].Position.X - MinX ) * Shift);

        ASSERT(OffSet >= 0 );
        ASSERT(OffSet < HashSize );

        pTempV[i].iNext  = pHash[ OffSet ].iNext;
        pHash[ OffSet ].iNext = i;
    }

    // Now do a seach for each vertex
    for( i=0; i<HashSize; i++ )
    {
        for( s32 k = pHash[i].iNext;  k != -1; k = pTempV[k].iNext )
        {
            s32 j;

            // This vertex has been remap
            if( pTempV[k].RemapIndex != pTempV[k].Index )
                continue;

            // Seach in the current hash 
            for( j = pTempV[k].iNext; j != -1; j = pTempV[j].iNext )
            {                
                // This vertex has been remap
                if( pTempV[j].RemapIndex != pTempV[j].Index )
                    continue;

                // If both vertices are close then remap vertex
                if( TempVCompare( m_pVertex[ k ], m_pVertex[ j ] ))
                    pTempV[j].RemapIndex    = k;
            }

            // Searchin the hash on the left
            if( (i+1)< HashSize )
            {
                for( j = pHash[i+1].iNext; j != -1; j = pTempV[j].iNext )
                {                
                    // This vertex has been remap
                    if( pTempV[j].RemapIndex != pTempV[j].Index )
                        continue;

                    // If both vertices are close then remap vertex
                    if( TempVCompare( m_pVertex[ k ], m_pVertex[ j ] ))
                        pTempV[j].RemapIndex    = k;
                }
            }
        }
    }

    // Okay now we must collapse all the unuse vertices
    s32 nVerts=0;
    for( i=0; i<m_nVertices; i++ )
    {
        if( pTempV[i].RemapIndex == pTempV[i].Index )
        {
            pTempV[i].RemapIndex = nVerts;
            pTempV[i].Index      = -1;      // Mark as we have cranch it
            nVerts++;
        }
    }

    // OKay now get all the facets and remap their indices
    for( i=0; i<m_nFacets; i++ )
    for( s32 j=0; j<3; j++ )
    {
        s32 iRemap = pTempV[ m_pFacet[i].iVertex[j] ].RemapIndex;

        if( pTempV[ m_pFacet[i].iVertex[j] ].Index == -1 )
        {
            m_pFacet[i].iVertex[j] = iRemap;
        }
        else
        {
            m_pFacet[i].iVertex[j] = pTempV[ iRemap ].RemapIndex;
        }
    }

    // Now copy the vertices to their final location
    pVertex = (vertex*)x_malloc( sizeof(vertex) * nVerts );
    if( pVertex == NULL ) 
        x_throw( "Out of memory" );

    for( i=0; i<m_nVertices; i++ )
    {
        s32 iRemap = pTempV[ i ].RemapIndex;

        if( pTempV[ i ].Index == -1 )
        {
            pVertex[ iRemap ] = m_pVertex[i];
        }
        else
        {
            pVertex[ pTempV[ iRemap ].RemapIndex ] = m_pVertex[i];
        }
    }

    // Finally set the new count and 
    x_free( m_pVertex );
    m_TotalVerticesRemoved += m_nVertices - nVerts;
    m_pVertex   = pVertex;
    m_nVertices = nVerts;

    // clean up
    delete[]pHash;
    delete[]pTempV;

    // Handle the errors
    x_catch_begin;

    if( pHash   ) delete[]pHash;
    if( pTempV  ) delete[]pTempV;
    if( pVertex ) x_free( pVertex );

    x_catch_end_ret;
}

//=========================================================================

s32 lighting::BuildBranch( s32* piFacet, s32 Size, s32 Axis, const vector3& Min, const vector3& Max, s32 Depth )
{
    s32 i;
//ASSERT( _CrtCheckMemory() );
    //
    // Do we have free entries?
    //
    if( m_nNodes >= (m_BuffNodeSize-1) )
    {
        m_BuffNodeSize += m_BuffNodeSize/2;

        m_pNode = (node*)x_realloc( m_pNode, sizeof(node)*m_BuffNodeSize );
        if( m_pNode == NULL )
            x_throw( "Out of memory" );
    }

    //
    // determine if this should be a leaf by termination criteria
    // This is a termination conditions.
    //
    if( Size <= LEAF_SIZE || Depth >= MAX_DEPTH )
    {
        m_BSPNumberOfFacetsResolved += Size;

        //
        // Add the leaf node
        //

        // Allocate this node for us
        s32 iNode = m_nNodes;
        m_nNodes++;

        // Create a reference to our new node
        node& Node = m_pNode[ iNode ];

        // Clear it
        x_memset( &Node, 0, sizeof(node) );

        Node.BBox.Min   = Min;
        Node.BBox.Max   = Max;
        Node.AxisOffset = -1;
        Node.Axis       = -1;
        Node.iLeft      = -1;
        Node.iRight     = -1;
        Node.nFacets    = Size;
        Node.pFacet     = piFacet;

        return iNode;
    }

    //
    // find a good split plane   
    //
    f32 BestPlane, SplitPlane;
    s32 BestBoth,  Both;

    // check plane at middle
    Both       = 0;
    SplitPlane = (Min[Axis] + Max[Axis] + .00000000001f) / 2.0f;

    for( i = 0; i < Size; i++ )
    {
        facet& Facet = m_pFacet[ piFacet[i] ];

        if( (Facet.BBox.Min[Axis] <= SplitPlane) && (Facet.BBox.Max[Axis] > SplitPlane) )
        {
            Both++;
        }
    }

    BestBoth  = Both;
    BestPlane = SplitPlane;

    // now check at regular intervals from .4 to .6
    f32  PlaneIncrement = (Max[Axis] - Min[Axis]) * 0.01f;
    f32  Stop           = SplitPlane + 0.10f * (Max[Axis] - Min[Axis]);
    f32  Start          = SplitPlane - 0.10f * (Max[Axis] - Min[Axis]);

    for( SplitPlane = Start; SplitPlane <= Stop; SplitPlane += PlaneIncrement )
    {
        Both = 0;

        for( i = 0; i < Size; i++)
        {
            facet& Facet = m_pFacet[ piFacet[i] ];
            if( (Facet.BBox.Min[Axis] <= SplitPlane) && (Facet.BBox.Max[Axis] > SplitPlane) )
            {
                Both++;
            }
        }

        if( Both < BestBoth)
        {
            BestBoth  = Both;
            BestPlane = SplitPlane;
        }
    }
    SplitPlane = BestPlane;

    //
    // now find the size of left and right child lists
    //
    s32     nLeft   = 0;
    s32     nRight  = 0;
    s32*    pLeft   = NULL;
    s32*    pRight  = NULL;

    for( i = 0; i < Size; i++ )
    {
        facet& Facet = m_pFacet[ piFacet[i] ];
        if( Facet.BBox.Min[Axis] <= SplitPlane ) nLeft++;
        if( Facet.BBox.Max[Axis] >  SplitPlane ) nRight++;
    }

    // tried adding if left or right == 0, didnt work
    if( BestBoth == Size )
    {       
        return BuildBranch( piFacet, Size, (Axis+1) % 3, Min, Max, Depth+1 );
    }

    // Lets make sure that our progress-bar does get affected because we are duplicating facets
    m_BSPDuplicatedFacets       += BestBoth;
    m_BSPNumberOfFacetsResolved -= BestBoth;

    //
    // okay we are ready to allocate our node
    //

    // Allocate this node for us
    s32 iNode = m_nNodes;
    m_nNodes++;

    // Create a reference to our new node
    // node& Node = m_pNode[ iNode ]; ( I had this before but I had a bug because it was reallocting! (Don't use it)

    // Clear it
    x_memset( &m_pNode[ iNode ], 0, sizeof(node) );

    m_pNode[ iNode ].AxisOffset = SplitPlane;
    m_pNode[ iNode ].Axis       = Axis;
    m_pNode[ iNode ].BBox.Min   = Min;
    m_pNode[ iNode ].BBox.Max   = Max;
 
    //
    // Now populate lists
    //
    s32 iLeft  = 0;
    s32 iRight = 0;

    // Allocate both lists
    pLeft      = new s32[ nLeft  ];
    pRight     = new s32[ nRight ];

    if( pRight == NULL || pLeft == NULL )
        x_throw( "Out of memory" );

    for( i = 0; i < Size; i++)
    {
        facet& Facet = m_pFacet[ piFacet[i] ];

        if( Facet.BBox.Min[Axis] <= SplitPlane )
        {
            pLeft[ iLeft ] = piFacet[i];
            iLeft++;
        }

        if( Facet.BBox.Max[Axis] > SplitPlane)
        {
            pRight[ iRight ] = piFacet[i];
            iRight++;
        }
    }
    ASSERT( iLeft  == nLeft );
    ASSERT( iRight == nRight );

    //
    // Okay now lets recurse
    //
    vector3 LeftMax, RightMin;

    LeftMax  = Max;
    RightMin = Min;

    LeftMax [ Axis ] = SplitPlane;
    RightMin[ Axis ] = SplitPlane;

    m_pNode[ iNode ].iLeft  = BuildBranch( pLeft,  nLeft,  (Axis+1) % 3, Min,      LeftMax, Depth+1 );
    m_pNode[ iNode ].iRight = BuildBranch( pRight, nRight, (Axis+1) % 3, RightMin, Max,     Depth+1 );

    // If the node is not a leaf then it means that we don't need the facets list.
    if( m_pNode[ m_pNode[ iNode ].iLeft ].Axis != -1 )
    {
        delete []pLeft;
        pLeft = NULL;
    }

    if( m_pNode[ m_pNode[ iNode ].iRight ].Axis != -1 )
    {
        delete []pRight;
        pRight = NULL;
    }

    m_BSPBuildNodeIterator++;
    if( (m_BSPBuildNodeIterator&0xff)==0 )
    {
        x_DebugMsg( "%f%%\n",                           (m_BSPNumberOfFacetsResolved*100.0f)/(f32)m_nFacets );
        x_DebugMsg( "Number of facets resolved %d\n",   m_BSPNumberOfFacetsResolved );
        x_DebugMsg( "Number of duplicated facets %d\n", m_BSPDuplicatedFacets );
        x_DebugMsg( "Iterator %d\n",                    m_BSPBuildNodeIterator );
    }
    // Okay lets return our node since we are done
    return iNode;
}

//=========================================================================

void lighting::BuildNodes( void )
{
    s32 i;

    //
    // Create the initial index list for the facets
    //
    s32* pInitialIndexList = new s32[m_nFacets];
    if( pInitialIndexList == NULL )
        x_throw( "Ouf of memory" );

    for( i=0; i<m_nFacets; i++ )
    {
        pInitialIndexList[i] = i;
    }

    //
    // Okay start building the tree
    //
    m_BSPBuildNodeIterator = 0;
    BuildBranch( pInitialIndexList, m_nFacets, 2, m_BBox.Min, m_BBox.Max, 0 );

    //
    // Clean up the initial index list
    // just make sure that the top triangle list is not already a leaf
    //
    if( m_pNode[0].Axis != -1 )
    {
        delete []pInitialIndexList;
    }
}

//=========================================================================

xbool lighting::ComputeRayTriCollision( 
    const facet&    Facet,
    const vector3&  Start, 
    const vector3&  Dir, 
    f32&            T,
    vector3&        HitPoint )
{
    m_NumberOfTriRayIntersections++;

    // Are we completely in front or starting from behind?
    if( !( (Facet.Plane.Distance( Start )>=0) && (Facet.Plane.Distance( Start+Dir )<0) ) )
    {
        return FALSE;
    }
        
    T = Facet.Plane.Normal.Dot( Dir );

    // check wether we have a parallel line (also makes our division zero)
    if( x_abs(T) <= 0.00001f )
        return FALSE;

    T = -Facet.Plane.Distance( Start ) / T;
    if( T>1 || T<0 )
        return FALSE;

    // Compute the actual contact point
    HitPoint = Start + ( Dir * T);

    // See if hit point is inside tri
    if( Facet.EdgePlane[0].InFront( HitPoint ) )
        return FALSE;

    if( Facet.EdgePlane[1].InFront( HitPoint ) )
        return FALSE;

    if( Facet.EdgePlane[2].InFront( HitPoint ) )
        return FALSE;

    // Collision
    return TRUE;
}


//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
/*
#define D3_EPSILON 0.00001f
#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2)\
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 

// Cull backfacing polygons?
#define OPT_CULL_RAY_TRIANGLE


xbool RayTriangleIntersect(const f32 orig[3],const f32 dir[3],  const f32 vert0[3],const f32 vert1[3],const f32 vert2[3],
  f32 *t,f32 *u,f32 *v)
// From Real-Time Rendering, page 305
// Returns 0 if not hit is found
// If a hit is found, t contains the distance along the ray (dir)
// and u/v contain u/v coordinates into the triangle (like texture
// coordinates).
{
  f32 edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
  f32 det,inv_det;

   // find vectors for two edges sharing vert0 
   SUB(edge1, vert1, vert0);
   SUB(edge2, vert2, vert0);

   // begin calculating determinant - also used to calculate U parameter 
   CROSS(pvec, dir, edge2);

   // if determinant is near zero, ray lies in plane of triangle 
   det = DOT(edge1, pvec);

#ifdef OPT_CULL_RAY_TRIANGLE
  // Culling section; triangle will be culled if not facing the right
  // way.
  if(det<D3_EPSILON)
    return 0;

   // calculate distance from vert0 to ray origin 
   SUB(tvec, orig, vert0);

   // calculate U parameter and test bounds 
   *u = DOT(tvec, pvec);
   if (*u < 0.0 || *u > det)
      return 0;

   // prepare to test V parameter 
   CROSS(qvec, tvec, edge1);

    // calculate V parameter and test bounds 
   *v = DOT(dir, qvec);
   if (*v < 0.0 || *u + *v > det)
      return 0;

   // calculate t, scale parameters, ray intersects triangle 
   *t = DOT(edge2, qvec);
   inv_det = 1.0f / det;
   *t *= inv_det;
   *u *= inv_det;
   *v *= inv_det;
#else
  // The non-culling branch
  if(det>-D3_EPSILON&&det<D3_EPSILON)
    return 0;
  inv_det = 1.0 / det;

  // calculate distance from vert0 to ray origin 
  SUB(tvec, orig, vert0);

  // calculate U parameter and test bounds 
  *u = DOT(tvec, pvec) * inv_det;
  if (*u < 0.0 || *u > 1.0)
    return 0;

  // prepare to test V parameter 
  CROSS(qvec, tvec, edge1);

  // calculate V parameter and test bounds 
  *v = DOT(dir, qvec) * inv_det;
  if (*v < 0.0 || *u + *v > 1.0)
    return 0;

  // calculate t, ray intersects triangle 
  *t = DOT(edge2, qvec) * inv_det;
#endif
  // We've got an intersection!
  return 1;
}

//=========================================================================

xbool lighting::ComputeRayTriCollision( 
    const facet&    Facet,
    const vector3&  Start, 
    const vector3&  Dir, 
    f32&            T,
    vector3&        HitPoint )
{
    vector3 Tri[3];
    xbool bHit;
    f32     t,u,v;

    Tri[0] = m_pVertex[ Facet.iVertex[0] ].Position;
    Tri[1] = m_pVertex[ Facet.iVertex[1] ].Position;
    Tri[2] = m_pVertex[ Facet.iVertex[2] ].Position;

    bHit = RayTriangleIntersect( Start, Dir, Tri[0], Tri[1], Tri[2], &t, &u, &v );

    if( bHit )
    {
        T = t;
        HitPoint = Start + Dir * T;
    }

    return bHit;
}
*/
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================


//=========================================================================

xbool lighting::NodeRayIntersection( const node& Node, const vector3& Origin, const vector3& Dir, f32 tMin, f32 tMax )
{
    vector3 Start;
    vector3 NewDir;
    vector3 HitPoint;
    vector3 BestHitPoint;
    xbool   bCol    = FALSE;
    s32     cFacet  = -1;
    f32     BestT   = 9999999999999.0f;
    f32     T;

    Start  = Origin + Dir * tMin;
    NewDir = Dir * tMax;

    for( s32 i=0; i<Node.nFacets; i++ )
    {
        s32    Index = Node.pFacet[i];
        facet& Facet = m_pFacet[ Index ];

        if( ComputeRayTriCollision( Facet, Start, NewDir, T, HitPoint ) )
        {
            bCol = TRUE;

            // We don't need to do anything else
            if( m_bFindClosestIntersection == FALSE )
            {
                BestT        = T;
                cFacet       = Index;
                BestHitPoint = HitPoint;
                break;
            }
            else
            {
                if( T < BestT )
                {
                    BestT  = T;
                    cFacet = Index;
                    BestHitPoint = HitPoint;
                }
            }
        }
    }

    //
    // We use this to render which zones we have visit
    //
    if( 0 )
    {
        eng_Begin( "DebugStuff" );

        draw_Begin( DRAW_TRIANGLES, DRAW_WIRE_FRAME  );
        draw_ClearL2W();

        for( i=0; i<Node.nFacets; i++ )
        {
            s32    Index = Node.pFacet[i];
            facet& Facet = m_pFacet[ Index ];

            for( s32 j=0; j<3; j++ )
            {
                vertex& V = m_pVertex[ Facet.iVertex[j] ];
                draw_Color ( xcolor( 0, 0,255,255) );
                draw_Vertex( V.Position );
            }
        }
        draw_End();

        draw_BBox( Node.BBox );
        eng_End();
    }

    //
    // Copy the final collision data
    //
    if( bCol )
    {
        m_CollisionIndexFacet = cFacet;
        m_CollisionT          = T;
        m_CollisionPoint      = BestHitPoint;
        m_iLastFacetCollision = m_CollisionIndexFacet;
    }

    return bCol;
}

//=========================================================================
// Few optimizations could be done such having a 1/Direction.
// Don't throwing the rays from the top node if we know which node we are.
//=========================================================================
xbool lighting::RayHit( s32 iBaseNode, const vector3& Origin, const vector3& Direction, f32 TMin, f32 TMax )
{  
    f32 tNodeEntry;
    f32 tNodeExit;

    //
    // first intersect the ray with the root bounding box
    //
    {
        f32  tx_min, tx_max;
        f32  ty_min, ty_max;
        f32  tz_min, tz_max;
        f32  t_min,  t_max;
   
        if( Direction.X >= 0 )
        {  
            t_min = tx_min = (m_BBox.Min.X - Origin.X) / Direction.X;
            t_max = tx_max = (m_BBox.Max.X - Origin.X) / Direction.X;
        }
        else
        {  
            t_min = tx_min = (m_BBox.Max.X - Origin.X) / Direction.X;
            t_max = tx_max = (m_BBox.Min.X - Origin.X) / Direction.X;
        }
   
        if( Direction.Y >= 0)
        {  
            ty_min = (m_BBox.Min.Y - Origin.Y) / Direction.Y;
            ty_max = (m_BBox.Max.Y - Origin.Y) / Direction.Y;
        }
        else
        {  
            ty_min = (m_BBox.Max.Y - Origin.Y) / Direction.Y;
            ty_max = (m_BBox.Min.Y - Origin.Y) / Direction.Y;
        }

        if( t_min > ty_max || ty_min > t_max )
            return FALSE;
   
        if( t_min < ty_min )
            t_min = ty_min;

        if( t_max > ty_max )
            t_max = ty_max;
   
        if( Direction.Z >= 0 )
        {  
            tz_min = (m_BBox.Min.Z - Origin.Z) / Direction.Z;
            tz_max = (m_BBox.Max.Z - Origin.Z) / Direction.Z;
        }
        else
        {  
            tz_min = (m_BBox.Max.Z - Origin.Z) / Direction.Z;
            tz_max = (m_BBox.Min.Z - Origin.Z) / Direction.Z;
        }
   
        if( t_min > tz_max || tz_min > t_max )
            return FALSE;
   
        if( t_min < tz_min )
            t_min = tz_min;

        if( t_max > tz_max )
            t_max = tz_max;
   
       if( !(t_min <= TMax && TMin <= t_max) ) 
           return FALSE;

       // Set the initial entry and exit points of the ray
        tNodeEntry    = t_min;
        tNodeExit     = t_max;
    }

    //
    // if it makes it past the bounding box intersection
    //
    struct stack
    {
        s32     iNode;
        f32     tEntry;
        f32     tExit;
    };

    f32         SplitDist;
    s32         iCurrNode;
    s32         iNearChild;
    s32         iFarChild;
    const s32   MaxStack = MAX_DEPTH * 10;
    stack       Stack[ MaxStack ];
    s32         StackPointer;
   
    // set up first stack element 
    StackPointer = 0;
    Stack[ StackPointer ].iNode  = iBaseNode;
    Stack[ StackPointer ].tEntry = tNodeEntry;
    Stack[ StackPointer ].tExit  = tNodeExit;
    StackPointer++;
   
    while( StackPointer > 0 )
    {  
        StackPointer--;
        tNodeEntry = Stack[ StackPointer ].tEntry;
        tNodeExit  = Stack[ StackPointer ].tExit;
        iCurrNode  = Stack[ StackPointer ].iNode;

        // while current node is an interior node
        while( m_pNode[ iCurrNode ].Axis != -1 )  
        {  
            const node& Node = m_pNode[ iCurrNode ];
            s32         Axis = Node.Axis;
            f32         Diff = m_pNode[ Node.iRight].BBox.Min[ Axis ] - Origin[ Axis ];

            // Collect some stats
            m_NumberOfNodeTravel++;

            // signed distance to splitting plane
            SplitDist = Diff / Direction[Axis];          // ******* We need to have 1/dir
            //if (Diff == 0.0f) cerr << " diff is " << diff << endl; 

            // find which side of the splitting plane the ray origin is on
            if( Diff > 0.0f )
            { 
                iNearChild = Node.iLeft;
                iFarChild  = Node.iRight;
            }
            else // if (diff <= 0.0f)
            {  
                iNearChild = Node.iRight;
                iFarChild  = Node.iLeft;
            }
         
            // find which of four traversal cases we have
            if( SplitDist > tNodeExit || SplitDist < 0.0f )
            {  
                // we only need to test near child
                iCurrNode = iNearChild;             
            }
            else
            {  
                if( SplitDist < tNodeEntry )
                {  
                    // we only need to test far child
                    iCurrNode = iFarChild;           
                }
                else
                {  
                    // we need to test both children so push far child onto stack
                    Stack[ StackPointer ].iNode  = iFarChild;
                    Stack[ StackPointer ].tEntry = SplitDist;
                    Stack[ StackPointer ].tExit  = tNodeExit;
                    StackPointer++;
                    ASSERT( StackPointer < MaxStack );

                    iCurrNode = iNearChild;
                    tNodeExit = SplitDist;
                }
            }             
        } 

        // make sure that ie a leaf node
        ASSERT( m_pNode[ iCurrNode ].iLeft       == -1 );
        ASSERT( m_pNode[ iCurrNode ].iRight      == -1 );
        ASSERT( m_pNode[ iCurrNode ].AxisOffset  == -1 );
        ASSERT( m_pNode[ iCurrNode ].Axis        == -1 );

        // now we are in a leaf test all the node triangles with the ray
        if( NodeRayIntersection( m_pNode[ iCurrNode ], Origin, Direction, TMin, TMax ) )
        {
            m_iLastNodeCollision = iCurrNode;
            return TRUE;
        }
    } 

   return FALSE;
}

//=========================================================================

xbool lighting::CanSeeLight( light& Light, vector3& Point )
{
    m_NumberOfRaysCheck++;

    vector3 Dir = Point - Light.Position;

    //
    // First lets try the mini chaches
    // 
    if( m_iLastFacetCollision != -1 )
    {
        if( ComputeRayTriCollision( m_pFacet[ m_iLastFacetCollision ], Light.Position, Dir, m_CollisionT, m_CollisionPoint ) )
        {
            m_NumberFacetCacheSuccess++;
            return TRUE;
        }

        m_iLastFacetCollision = -1;
    }

    if( m_iLastNodeCollision != -1 )
    {
        if( lighting::NodeRayIntersection( m_pNode[m_iLastNodeCollision], Light.Position, Dir, 0, 1 ) )
        {
            m_NumberNodeChaceSuccess++;
            return TRUE;
        }

        m_iLastNodeCollision = -1;
    }

    //
    // Okay we are raedy to go throw the hold process
    //
    return RayHit( 0, Light.Position, Dir, 0, 1 );
}

//=========================================================================

void lighting::LightingTriangle( s32 i )
{
    f32     Step = 1.0f/((f32)LIGHTING_STEPS);
    f32     u,v,w;
    facet&  Tri = m_pFacet[i];

    for( u=0; u<=1.0f; u+=Step )
    for( v=0; v<=(1-u); v+=Step )
    {
        w = 1 - u - v;

        vector3 P = u*m_pVertex[ Tri.iVertex[0] ].Position + 
                    v*m_pVertex[ Tri.iVertex[1] ].Position + 
                    w*m_pVertex[ Tri.iVertex[2] ].Position;

        vector3 N = u*m_pVertex[ Tri.iVertex[0] ].Normal + 
                    v*m_pVertex[ Tri.iVertex[1] ].Normal + 
                    w*m_pVertex[ Tri.iVertex[2] ].Normal;

        N.Normalize();

        vector3 L1(0,0,0);
        vector3 L2(0,0,0);

        // Accumulate lighting
        for( s32 j=0; j<m_nLights; j++ )
        {
            light&  Light    = m_pLight[j];
            vector3 LightPos = Light.Position;

            if( Light.BBox.Intersect( P ) == FALSE )
                continue;

            vector3 Diff = LightPos - P;
            f32     D    = Diff.Length();

            // Compute lighting
            xbool InShadow = CanSeeLight( Light, (P+(N*0.5f)) );

            f32 NI = N.Dot( Diff ) / D;
            if( NI < 0 ) NI = 0;

            f32 IAtten = (Light.AttenR - D) / (Light.AttenR);
            if( IAtten < 0 ) IAtten = 0;
            if( IAtten > 1 ) IAtten = 1;

            f32 I = IAtten * NI;
            if( I < 0 ) I = 0;
            if( I > 2 ) I = 2;

            if( !InShadow )
            {
                L2.X += I * Light.Color.R;
                L2.Y += I * Light.Color.G;
                L2.Z += I * Light.Color.B;
            }

            L2.X += I * Light.AmbLight.R;
            L2.Y += I * Light.AmbLight.G;
            L2.Z += I * Light.AmbLight.B;
            
            L1.X += I * Light.Color.R;
            L1.Y += I * Light.Color.G;
            L1.Z += I * Light.Color.B;
        }

        if( L1.X <   0 ) L1.X = 0;
        if( L1.X > 255 ) L1.X = 255;
        if( L1.Y <   0 ) L1.Y = 0;
        if( L1.Y > 255 ) L1.Y = 255;
        if( L1.Z <   0 ) L1.Z = 0;
        if( L1.Z > 255 ) L1.Z = 255;

        if( L2.X <   0 ) L2.X = 0;
        if( L2.X > 255 ) L2.X = 255;
        if( L2.Y <   0 ) L2.Y = 0;
        if( L2.Y > 255 ) L2.Y = 255;
        if( L2.Z <   0 ) L2.Z = 0;
        if( L2.Z > 255 ) L2.Z = 255;

//**** Add critical section here if we do multiprocessing
        m_pVertex[ Tri.iVertex[0] ].C1 += L1 * u;
        m_pVertex[ Tri.iVertex[1] ].C1 += L1 * v;
        m_pVertex[ Tri.iVertex[2] ].C1 += L1 * w;
        m_pVertex[ Tri.iVertex[0] ].C2 += L2 * u;
        m_pVertex[ Tri.iVertex[1] ].C2 += L2 * v;
        m_pVertex[ Tri.iVertex[2] ].C2 += L2 * w;
        m_pVertex[ Tri.iVertex[0] ].W += u;
        m_pVertex[ Tri.iVertex[1] ].W += v;
        m_pVertex[ Tri.iVertex[2] ].W += w;
//**** End here
    }
}


//
// This version should be better than the other one since first 
// checks to see if the light can see the facet and then for each
// light that can checks all the facet points againts that light.
// much more cache happy. Must talk to Andy about fixing it tought.
//
/* 
void lighting::LightingTriangle( s32 i )
{
    f32     Step = 1.0f/((f32)LIGHTING_STEPS);
    f32     u,v,w;
    facet&  Tri = m_pFacet[i];

    for( s32 j=0; j<m_nLights; j++ )
    {
        light&  Light    = m_pLight[j];
        vector3 LightPos = Light.Position;

        if( Light.BBox.Intersect( Tri.BBox ) == FALSE )
            continue;

        for( u=0; u<=1.0f; u+=Step )
        for( v=0; v<=(1-u); v+=Step )
        {
            w = 1 - u - v;

            vector3 P = u*m_pVertex[ Tri.iVertex[0] ].Position + 
                        v*m_pVertex[ Tri.iVertex[1] ].Position + 
                        w*m_pVertex[ Tri.iVertex[2] ].Position;

            vector3 N = u*m_pVertex[ Tri.iVertex[0] ].Normal + 
                        v*m_pVertex[ Tri.iVertex[1] ].Normal + 
                        w*m_pVertex[ Tri.iVertex[2] ].Normal;

            N.Normalize();

            vector3 L1(0,0,0);
            vector3 L2(0,0,0);

            // Accumulate lighting
            {
                vector3 Diff = LightPos - P;
                f32     D    = Diff.Length();

                // Compute lighting
                xbool InShadow = CanSeeLight( Light, (P+(N*0.5f)) );

                f32 NI = N.Dot( Diff ) / D;
                if( NI < 0 ) NI = 0;

                f32 IAtten = (Light.AttenR - D) / (Light.AttenR);
                if( IAtten < 0 ) IAtten = 0;
                if( IAtten > 1 ) IAtten = 1;

                f32 I = IAtten * NI;
                if( I < 0 ) I = 0;
                if( I > 2 ) I = 2;

                if( !InShadow )
                {
                    L2.X += I * Light.Color.R;
                    L2.Y += I * Light.Color.G;
                    L2.Z += I * Light.Color.B;
                }

                L2.X += I * Light.AmbLight.R;
                L2.Y += I * Light.AmbLight.G;
                L2.Z += I * Light.AmbLight.B;
            
                L1.X += I * Light.Color.R;
                L1.Y += I * Light.Color.G;
                L1.Z += I * Light.Color.B;
            }

            if( L1.X <   0 ) L1.X = 0;
            if( L1.X > 255 ) L1.X = 255;
            if( L1.Y <   0 ) L1.Y = 0;
            if( L1.Y > 255 ) L1.Y = 255;
            if( L1.Z <   0 ) L1.Z = 0;
            if( L1.Z > 255 ) L1.Z = 255;

            if( L2.X <   0 ) L2.X = 0;
            if( L2.X > 255 ) L2.X = 255;
            if( L2.Y <   0 ) L2.Y = 0;
            if( L2.Y > 255 ) L2.Y = 255;
            if( L2.Z <   0 ) L2.Z = 0;
            if( L2.Z > 255 ) L2.Z = 255;

    //**** Add critical section here if we do multiprocessing
            m_pVertex[ Tri.iVertex[0] ].C1 += L1 * u;
            m_pVertex[ Tri.iVertex[1] ].C1 += L1 * v;
            m_pVertex[ Tri.iVertex[2] ].C1 += L1 * w;
            m_pVertex[ Tri.iVertex[0] ].C2 += L2 * u;
            m_pVertex[ Tri.iVertex[1] ].C2 += L2 * v;
            m_pVertex[ Tri.iVertex[2] ].C2 += L2 * w;
            m_pVertex[ Tri.iVertex[0] ].W += u;
            m_pVertex[ Tri.iVertex[1] ].W += v;
            m_pVertex[ Tri.iVertex[2] ].W += w;
    //**** End here
        }
    }
}
*/

//=========================================================================

void lighting::LightingEnd( void )
{
    for( s32 i=0; i<m_nVertices; i++ )
    {
        vector3 L1 = m_pVertex[i].C1;

        if( m_pVertex[i].W == 0 )
            L1.Set( 0,0,0 );
        else
            L1 /= m_pVertex[i].W;

//        L1.X += m_AmbientLight.R;
//        L1.Y += m_AmbientLight.G;
//        L1.Z += m_AmbientLight.B;
        if( L1.X <   0 ) L1.X = 0;
        if( L1.X > 255 ) L1.X = 255;
        if( L1.Y <   0 ) L1.Y = 0;
        if( L1.Y > 255 ) L1.Y = 255;
        if( L1.Z <   0 ) L1.Z = 0;
        if( L1.Z > 255 ) L1.Z = 255;


        vector3 L2 = m_pVertex[i].C2;
        if( m_pVertex[i].W == 0 )
            L2.Set( 0,0,0 );
        else
            L2 /= m_pVertex[i].W;

//        L2.X += m_AmbientLight.R;
//        L2.Y += m_AmbientLight.G;
//        L2.Z += m_AmbientLight.B;
        if( L2.X <   0 ) L2.X = 0;
        if( L2.X > 255 ) L2.X = 255;
        if( L2.Y <   0 ) L2.Y = 0;
        if( L2.Y > 255 ) L2.Y = 255;
        if( L2.Z <   0 ) L2.Z = 0;
        if( L2.Z > 255 ) L2.Z = 255;

        f32 t = 0;
        vector3 Final = L2;//L1 + t*( L2 - L1 );

        m_pVertex[i].FinalColor = xcolor( (byte)Final.X, (byte)Final.Y, (byte)Final.Z );
    }
}

//=========================================================================

void lighting::CompileData( void )
{
    xtimer Timer;

    x_DebugMsg( "Collapsing all the positions...\n");
    Timer.Start();
    CollapseVertexPositions();
    Timer.Stop();
    x_DebugMsg( "Done in %f Minutes. %d vertices removed.\n", Timer.ReadSec()/60.0f, m_TotalVerticesRemoved );

    
    x_DebugMsg( "Start Building BSP tree...\n" );
    Timer.Start();
    BuildNodes();
    Timer.Stop();
    x_DebugMsg( "Done in %f Minutes.\n", Timer.ReadSec()/60.0f );
}

//=========================================================================

void lighting::ComputeLighting( void )
{
    ComputePartialLighting( 0, m_nFacets );
    LightingEnd();
}

//=========================================================================

void lighting::ComputePartialLighting( s32 iStart, s32 iFinish )
{
    xtimer Timer;
    f32 BigTotal = 100.0f/(f32)(iFinish - iStart);
    f32 Progress = 0;

    // Clear all the mini chaches
    m_iLastNodeCollision        = -1;
    m_iLastFacetCollision       = -1;
    m_NumberNodeChaceSuccess    = 0;
    m_NumberFacetCacheSuccess   = 0;

    Timer.Start();
    for( s32 i = iStart; i<iFinish; i++, Progress++ )
    {
        f32 Percentage = Progress*BigTotal;

        LightingTriangle( i );

        if( (i&0xff) == 0 ) 
        {
            x_DebugMsg( "%f%%\n", Percentage );
            x_DebugMsg( "# Tri//Ray Intersections  : %f\n", (f64)m_NumberOfTriRayIntersections);
            x_DebugMsg( "# Ray checks Intersections: %f\n", (f64)m_NumberOfRaysCheck);
            x_DebugMsg( "# BSP Nodes Travel        : %f\n", (f64)m_NumberOfNodeTravel);            
            x_DebugMsg( "# Node  Cache Success     : %d\n", m_NumberNodeChaceSuccess );            
            x_DebugMsg( "# Facet Cache Success     : %d\n", m_NumberFacetCacheSuccess );            
            
        }
    }
    Timer.Stop();
    x_DebugMsg( "Done In %f \n", Timer.ReadSec()/60.0f );            
}

//=========================================================================

xbool lighting::CheckCollision( vector3& Start, vector3& Dir )
{
    xbool   bHit;
    xbool   bDebug = FALSE;

    if( bDebug == FALSE )
    {

        m_bFindClosestIntersection = TRUE;
        bHit = RayHit( 0, Start, Dir, 0, 1 );
        m_bFindClosestIntersection = FALSE;
    }
    else
    {
        vector3 HitPoint;
        vector3 BestHitPoint;
        s32     cFacet  = -1;
        f32     BestT   = 9999999999999.0f;
        f32     T;

        bHit = FALSE;
        for( s32 i=0; i<m_nFacets; i++) 
        {
            facet& F = m_pFacet[i];

            if( ComputeRayTriCollision( F, Start, Dir, T, HitPoint ) )
            {
                bHit = TRUE;
                if( T < BestT )
                {
                    BestT  = T;
                    cFacet = i;
                    BestHitPoint = HitPoint;
                }
            }
        }
 
        if( bHit )
        {
            m_CollisionIndexFacet = cFacet;
            m_CollisionT          = BestT;
            m_CollisionPoint      = BestHitPoint;
        }
    }

    return bHit;
}

//=========================================================================

vector3 lighting::GetHitPoint( void )
{
    return m_CollisionPoint;
}

//=========================================================================

void lighting::Render( void )
{
    xbool  bColor     = TRUE;
    xbool  bWireAlso  = FALSE;
    xbool  bFullSolid = TRUE;
    xbool  bZones     = FALSE;

    if( bZones )
    {
        x_srand( 10101 );
        draw_Begin( DRAW_TRIANGLES );
        draw_ClearL2W();

        for( s32 z=0; z<m_nNodes; z++ )
        {
            node&   Node  = m_pNode[z];
            xcolor  ZoneC = xcolor( (u8)x_rand(), (u8)x_rand(), (u8)x_rand(), 255);

            for( s32 i=0; i<Node.nFacets; i++) 
            {
                s32    Index = Node.pFacet[i];
                facet& Facet = m_pFacet[ Index ];

                for( s32 j=0; j<3; j++ )
                {
                    vertex& V = m_pVertex[ Facet.iVertex[j] ];
                    draw_Color ( ZoneC );
                    draw_Vertex( V.Position );
                }
            }
        }

        draw_End();
    }

    if( bFullSolid )
    {
        draw_Begin( DRAW_TRIANGLES );
        draw_ClearL2W();

        for( s32 i=0; i<m_nFacets; i++) 
        {
            facet& F = m_pFacet[i];
            for( s32 j=0; j<3; j++ )
            {
                vertex& V = m_pVertex[ F.iVertex[j] ];
                if( bColor )
                {
                    draw_Color ( V.FinalColor );
                }
                else
                {
                    draw_Color ( xcolor( (u8)(V.Normal.X*128)+128,
                                         (u8)(V.Normal.Y*128)+128, 
                                         (u8)(V.Normal.Z*128)+128, 255) );
                }
                draw_Vertex( V.Position );
            }
        }

        draw_End();
    }

    if( bWireAlso ) 
    {
        draw_Begin( DRAW_TRIANGLES, DRAW_WIRE_FRAME  );
        draw_ClearL2W();

        for( s32 i=0; i<m_nFacets; i++) 
        {
            facet& F = m_pFacet[i];
            for( s32 j=0; j<3; j++ )
            {
                vertex& V = m_pVertex[ F.iVertex[j] ];
                draw_Color ( xcolor( 0, 0,255,255) );
                draw_Vertex( V.Position );
            }
        }

        draw_End();
    }
}

//=========================================================================
//=========================================================================
// Interesting article about faster tri-ray collision test.
// Must test this ones day.
//=========================================================================
//=========================================================================
/*
Fast ray tri (with precomputation)
November 9, 2002
by Charles Bloom, cbloom@cbloom.com

Moller/Haines and others have good ray-tri intersection code for
the case of "no precomputation", but sometimes you want to do
precomputation.  For example, if you have only a few tris and
you want to shoot lots of rays at them, you should precompute
some things on your triangle to speed up the test.  Here's how.
I actually want segment-tri intersection, that is, my ray has
two end points.

We want to write a routine like

bool RayTriIntersect(const Triangle & tri,const Vec3 & from,
	const Vec3 & to, );

Compute the plane of the triangle.  The plane is a normal and
offset (4 floats).  Now you can do your first tests on your
segment :

dFm = tri.plane.Distance(from);
dTo = tri.plane.Distance(to);

Where plane.Distance is just (plane.normal * vec - plane.offset)

Now you need to check the signs of dFm and dTo to see if the
segment passes through the plane.  You have some options here
depending on where you want to treat your triangle as one-sided
or two-sided.  I'll assume that you want a one-sided triangle,
so only rays going "at" the triangle should count as hits, 
so :

if ( dTo <= 0 )
{
	// segment doesn't reach back side of triangle
	return false;
}
if ( dFm > 0 )
{
	// segment starts already on the back side
	return false;
}
assert( dTo > 0 && dFm <= 0 )

Now we know that the plane is hit.  We can now make the
point on the plane (we don't actually want to do this, but
we'll write it out) :

float t = dFm / (dTo - dFm)
onPlane = lerp(from,to,t);

Now we want to check if this point is on the triangle.  We do
that using barycentric coordinates.  To compute the barycentric
coords, you can use scaled planes.  You do :

edge1 = tri.vert1 - tri.vert0;
edge2 = tri.vert2 - tri.vert1;
edgePerp1 = tri.normal CROSS edge1
edgePerp2 = tri.normal CROSS edge2
edgePlane1 = { edgePerp1, edgePerp1*tri.vert0 }
edgePlane2 = { edgePerp2, edgePerp2*tri.vert1 }

Now a plane is just a vector and a constant; we can scale this
normal to change the distances we want, so we make

baryPlane1 = edgePlane1 / (edgePlane1.Distance(tri.vert2))
baryPlane2 = edgePlane2 / (edgePlane1.Distance(tri.vert0))

The result are these two "baryPlanes".  These give you the
barycentric coords just by doing dots :

u = baryPlane1.Distance(onPlane);
v = baryPlane2.Distance(onPlane);

These baryPlanes are pretty neat; they are the planes that lie
along the triangle edges and normal; their vectors are scaled
so that the opposite point on the triangle returns a distance
of 1.  Once we have these barycentric coordinates, we can
check collision easily :

if ( u < 0 || v < 0 || (u+v) > 1 )
{
	// no collision
	return false;
}
// collision !

And we also have the time of hit and the bary-coords, which is
good info to have.  Now lets put it all together and delay the
divide :

struct Triangle
{
	Plane	plane;
	Plane	bary1;
	Plane	bary2;
};

bool RayTriIntersect(const Triangle & tri,const Vec3 & from,
	const Vec3 & to)
{
	float dTo = tri.plane.Distance(to);
	if ( dTo <= 0 )
	{
		// segment doesn't reach back side of triangle
		return false;
	}

	float dFm = tri.plane.Distance(from);
	if ( dFm > 0 )
	{
		// segment starts already on the back side
		return false;
	}
	assert( dTo > 0 && dFm <= 0 );

	float denom = dTo - dFm;
	assert( denom > 0 );
	Vec3 temp = dTo * from + dFm * to;

	float uTimesDenom = (tri.bary1.vector DOT temp) - tri.bary1.offset * denom;

	if ( uTimesDenom < 0 || uTimesDenom > denom )
	{
		// off triangle
		return false;
	}

	float vTimesDenom = (tri.bary2.vector DOT temp) - tri.bary2.offset * denom;

	if ( vTimesDenom < 0 || (uTimesDenom+vTimesDenom) > denom )
	{
		// off triangle
		return false;
	}

	// it's on the triangle, compute hit info :
	float inv = 1 / denom;
	*pT = dFm * inv;
	*pU = uTimesDenom * inv;
	*pV = vTimesDenom * inv;

	return true;
}

On my machine (1 GHz P3), I measure about 165 clocks for Moller's optimized ray-tri #1 
(late division).  See :

http://www.ce.chalmers.se/staff/tomasm/raytri/

The code here takes about 120 clocks.  The exact numbers depend on how much my extra
early-outs help.  The 120 clock number is with no help from the early-outs ; in
practice, the number is much lower, more like 50-60 clocks average in typical use.

Most of all, I like the mental construct of the "barycentric planes".

The size of the pre-computed "Triangle" structure is very small.  It's only 12 floats,
compared to 9 floats to store the triangle vertices.  Thus, the memory-fetch cost of
this routine is not significantly higher.  Note that I don't use the triangle's actual
vertices at all in the ray-intersection routine.

Finally, note that in the real world there should be higher-level acceleration 
structures used for ray intersection of all kinds.
*/




