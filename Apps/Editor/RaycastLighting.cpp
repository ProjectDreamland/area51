//=========================================================================
// RaycastLighting.hpp
//=========================================================================

//
// I'm shutting off the asserts to get one last boost in performance here
//
#include "x_target.hpp"
#undef X_ASSERT
#undef X_ASSERTS


#include "RayCastLighting.hpp"
#include "Entropy.hpp"
#include <crtdbg.h>
#include "ManagerRegistration.hpp"
#include "..\WorldEditor\WorldEditor.hpp"
#include "CollisionMgr\GridWalker.hpp"

//=========================================================================
// DEFINES
//=========================================================================

#define VERTEX_MERGE_POSITION_EPSILON           (1.00f)
#define VERTEX_MERGE_NORMAL_EPSILON             (0.866) // cos(30)
#define SUBDIVISION_STEP_LENGTH                 (100.0f / 5.0f)
#define DO_LOS_CHECK                            1
#define DIST_TO_DISPLACE_POINT_BEFORE_LIGHTING  (0.25f)
//#define DO_DEBUGMSG

//=========================================================================
// FUNCTIONS
//=========================================================================

s32 SphereIntersectHits=0;
s32 SphereIntersectMisses=0;
s32 BACKFACED_TRIS=0;

s32 POINTS_BACKFACED        = 0;
s32 POINTS_WITH_LOS         = 0;
s32 POINTS_IN_SHADOW        = 0;
s32 POINTS_OUTSIDE_LIGHT    = 0;
s32 POINTS_INSIDE_LIGHT     = 0;
s32 POINTS_PROCESSED        = 0;
s32 TRIS_OUTSIDE_LIGHT      = 0;

static xbool DO_BACKWARDS_RAYCAST = FALSE;

//=========================================================================

raycast_lighting::~raycast_lighting( void )
{
    Clear();
}

//=========================================================================

void raycast_lighting::Clear( void )
{
    //
    // Clear all the facets/vert/lights
    //
    if( m_pVertex  )  x_free( m_pVertex  );
    if( m_pFacet   )  x_free( m_pFacet   );
    if( m_pLight   )  x_free( m_pLight   );
    if( m_pGridCell)  x_free( m_pGridCell);
    if( m_pFacetGroup)  x_free( m_pFacetGroup);
    if( m_pFacetColor) x_free( m_pFacetColor );
    if( m_pFacetRef) x_free( m_pFacetRef );

    m_pVertex               = NULL;
    m_pFacet                = NULL;
    m_pLight                = NULL;
    m_pGridCell             = NULL;
    m_pFacetGroup           = NULL;
    m_pFacetColor           = NULL;
    m_pFacetRef             = NULL;

    m_nFacetGroupsAllocated = 0;
    m_nFacetGroups          = 0;
    
    m_VBuffSize             = 0;
    m_nVertices             = 0;

    m_FBuffSize             = 0;
    m_nFacets               = 0;

    m_LBuffSize             = 0;
    m_nLights               = 0;

    m_BBox.Clear();
    m_TotalVerticesRemoved  = 0;
    //
    // Collision info
    //
    m_CollisionIndexFacet           = -1;
    m_CollisionT                    = -1;
    m_NumberOfTriRayIntersections   =  0;
    m_NumberOfRaysCheck             =  0;
    m_NumberOfNodeTravel            =  0;

    m_GridUseSequence = 0;

    m_pGridCell = NULL;
    m_pFacetRef = NULL;
    m_pFacetGroup = NULL;
    m_pFacetColor = NULL;
    m_GridCellSize = -1;
    m_nGridCellsTotal = 0;
    m_nGridCellsWide = 0;
    m_OneOverGridCellSize = -1;
    m_pGridLight = NULL;
}

//=========================================================================

raycast_lighting::raycast_lighting( void )
{
    //
    // Buffer and lights
    //
    m_pVertex               = NULL;
    m_pFacet                = NULL;
    m_pLight                = NULL;
    m_pGridCell             = NULL;
    m_pFacetGroup           = NULL;
    m_pFacetColor           = NULL;
    m_pFacetRef             = NULL;

    // Clear all and get it ready
    Clear();

}

//=========================================================================

void raycast_lighting::AddLight( vector3& Pos, f32 AttenR, xcolor Color, xcolor AmbColor, f32 Intensity )
{
    //
    // Make sure that we have space for the light
    //
    if( m_nLights >= (m_LBuffSize - 1) )
    {
        m_LBuffSize += 1000;
        m_pLight      = (light*)x_realloc( m_pLight, sizeof(light) * m_LBuffSize );
        if( m_pLight == NULL ) 
            x_throw( "Out of memory" );
    }

    light& Light = m_pLight[ m_nLights++ ];
    Light.Position  = Pos;
    Light.Radius    = AttenR;
    Light.RadiusSquared = Light.Radius*Light.Radius;
    Light.Color     = Color;
    Light.AmbLight  = AmbColor;
    Light.Intensity = Intensity;  // 50% increase to match it up with the normal distance light

    Light.BBox.Set( Light.Position, Light.Radius );
}

//=========================================================================

void raycast_lighting::AddFacet( 
    vector3& P0, 
    vector3& N0, 
    vector3& P1, 
    vector3& N1,
    vector3& P2, 
    vector3& N2, 
    s32      SlotID, 
    u32      Flags,
    u32      UserData1,
    u32      UserData2 )
{
    //
    // Make sure that we have all the memory that we need
    //
    ASSERT( m_FBuffSize > 0 );
    if( m_nFacets >= (m_FBuffSize - 1) )
    {
        s32 NewSize = MAX(500000,(m_FBuffSize + 500000));
        x_DebugMsg("Realloc Facet Array (%d,%d) (%d,%d)\n",m_FBuffSize,NewSize,sizeof(facet)*m_FBuffSize,sizeof(facet)*NewSize);

        m_FBuffSize = NewSize;
        m_pFacet      = (facet*)x_realloc( m_pFacet, sizeof(facet) * m_FBuffSize );
        if( m_pFacet == NULL ) 
            x_throw( "Out of memory" );
    }

    ASSERT( m_VBuffSize > 0 );
    if( m_nVertices >= (m_VBuffSize-3) )
    {
        s32 NewSize = MAX(1000000,(m_VBuffSize + 1000000));
        x_DebugMsg("Realloc Vertex Array (%d,%d) (%d,%d)\n",m_VBuffSize,NewSize,sizeof(vertex)*m_VBuffSize,sizeof(vertex)*NewSize);

        m_VBuffSize = NewSize;
        m_pVertex      = (vertex*)x_realloc( m_pVertex, sizeof(vertex) * m_VBuffSize );
        if( m_pVertex == NULL ) 
            x_throw( "Out of memory" );
    }

    //
    // Add the triangle in the list
    //
    m_pFacet[ m_nFacets ].SlotID     = SlotID;
    m_pFacet[ m_nFacets ].Flags      = Flags;
    m_pFacet[ m_nFacets ].UserData[0]= UserData1;
    m_pFacet[ m_nFacets ].UserData[1]= UserData2;

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

    // Compute BBoxIndices
    s32 MinIndices[3];
    s32 MaxIndices[3];
    m_pFacet[ m_nFacets ].Plane.GetBBoxIndices( MinIndices, MaxIndices );
    m_pFacet[ m_nFacets ].MinBBoxIndices[0] = (byte)MinIndices[0];
    m_pFacet[ m_nFacets ].MinBBoxIndices[1] = (byte)MinIndices[1];
    m_pFacet[ m_nFacets ].MinBBoxIndices[2] = (byte)MinIndices[2];
    m_pFacet[ m_nFacets ].MaxBBoxIndices[0] = (byte)MaxIndices[0];
    m_pFacet[ m_nFacets ].MaxBBoxIndices[1] = (byte)MaxIndices[1];
    m_pFacet[ m_nFacets ].MaxBBoxIndices[2] = (byte)MaxIndices[2];

    // now build the planes for the edges. (This could be factor out for all facets)
    //m_pFacet[ m_nFacets ].EdgePlane[0].Setup( P0, P1, P0 + m_pFacet[ m_nFacets ].Plane.Normal );
    //m_pFacet[ m_nFacets ].EdgePlane[1].Setup( P1, P2, P1 + m_pFacet[ m_nFacets ].Plane.Normal );
    //m_pFacet[ m_nFacets ].EdgePlane[2].Setup( P2, P0, P2 + m_pFacet[ m_nFacets ].Plane.Normal );

    m_pFacet[ m_nFacets ].SphereCenter = m_pFacet[ m_nFacets ].BBox.GetCenter();
    m_pFacet[ m_nFacets ].SphereRadius = m_pFacet[ m_nFacets ].BBox.GetRadius();

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

//=========================================================================

inline
xbool raycast_lighting::TempVCompare( const tempv& A, const tempv& B )
{
    if( (A.Position - B.Position).LengthSquared() > VERTEX_MERGE_POSITION_EPSILON*VERTEX_MERGE_POSITION_EPSILON )
        return FALSE;

    f32 Dot = A.Normal.Dot(B.Normal);
    if( A.Normal.Dot(B.Normal) < VERTEX_MERGE_NORMAL_EPSILON )
        return FALSE;

    return TRUE;
}

//=========================================================================
// Collapse vertices that are too close to each other and have 
// the same properties.
//=========================================================================
void raycast_lighting::CollapseVertexPositions( void )
{
    struct hash
    {
        s32 iNext;
        s32 nVerts;
        s32 iLastMerged;
    };

    if( m_nVertices <= 0 )
        x_throw( "There were no vertices submitted to the raycast system" );

    s32         i;
    hash*       pHash     = NULL;
    tempv*      pTempV    = NULL;
    vertex*     pVertex   = NULL;
    f32         MaxX, MinX, Shift;
    s32         MaxVertsInHash=0;

    x_DebugMsg("Starting nVerts: %d\n",m_nVertices);

    // begin the error block
    x_try;

    // Allocate memory
    pTempV  = (tempv*)x_malloc(sizeof(tempv)*m_nVertices);

    if( pTempV == NULL )
        x_throw( "Out of memory" );

    // Fill the nodes for each of the dimensions
    MaxX = m_pVertex[0].Position.GetX();
    MinX = MaxX;
    for( i=0; i<m_nVertices; i++) 
    {
        pTempV[i].Index         =  i;
        pTempV[i].iNext         = -1;
        pTempV[i].RemapIndex    =  i;
        pTempV[i].Position = m_pVertex[i].Position;
        pTempV[i].Normal = m_pVertex[i].Normal;
       
        MaxX = MAX( MaxX, m_pVertex[i].Position.GetX() );
        MinX = MIN( MinX, m_pVertex[i].Position.GetX() );
    }

    s32     HashSize  = (s32)((MaxX - MinX)/(VERTEX_MERGE_POSITION_EPSILON*4)); 
    if( HashSize > 100000 ) HashSize = 100000;
    if( HashSize < 1 ) HashSize = 1;

    // begin the error block
    pHash   = (hash*)x_malloc(sizeof(hash)*HashSize);
    if( pHash == NULL )
        x_throw( "Out of memory" );

    // Initialize the hash with terminators
    for( i=0; i<HashSize; i++) 
    {
        pHash[i].iNext = -1;
        pHash[i].nVerts = 0;
        pHash[i].iLastMerged = -1;
    }
    Shift = HashSize/(MaxX-MinX+1);

    #ifdef DO_DEBUGMSG
    x_DebugMsg("HashSize:        %d\n",HashSize);
    #endif

    s32 LastMergedHit=0;
    f32 MergeEpsilon = VERTEX_MERGE_POSITION_EPSILON;

    // Loop through all verts and start adding to the hash table
    for( s32 I=0; I<m_nVertices; I++ )
    {
        // Get the three hash entries we should check in for a match
        s32 iHash[3];
        iHash[0] = (s32)((( m_pVertex[I].Position.GetX() - MinX ) +            0) * Shift);
        iHash[1] = (s32)((( m_pVertex[I].Position.GetX() - MinX ) + MergeEpsilon) * Shift);
        iHash[2] = (s32)((( m_pVertex[I].Position.GetX() - MinX ) - MergeEpsilon) * Shift);
        ASSERT( (iHash[0] >= 0) && (iHash[0] < HashSize) );
        if( iHash[1] >= HashSize ) iHash[1] = HashSize-1;
        if( iHash[2] <  0 ) iHash[2] = 0;
        if( iHash[1] == iHash[0] ) iHash[1] = -1;
        if( iHash[2] == iHash[0] ) iHash[2] = -1;

        pTempV[I].RemapIndex = -1;

        // Loop through the hash entries
        for( i=0; i<3; i++ )
        if( iHash[i] != -1 )
        {
            hash& H = pHash[iHash[i]];
            s32 j;

            // Check lastmerged first
            if( H.iLastMerged != -1 )
            {
                if( TempVCompare( pTempV[H.iLastMerged], pTempV[I] ))
                {
                    // Remap pVertex and bail
                    pTempV[I].RemapIndex = H.iLastMerged;
                    LastMergedHit++;
                    
                    // Add this vert to the vert we mapped to
                    {
                        tempv& TV = pTempV[ pTempV[I].RemapIndex ];
                        TV.Position  = (TV.Position + pTempV[I].Position) * 0.5f;
                        TV.Normal   += pTempV[I].Normal;
                        TV.Normal.Normalize();
                    }
                }
            }

            if( pTempV[I].RemapIndex == -1 )
            {
                // Loop through current entries in hash
                for( j = pHash[iHash[i]].iNext; j != -1; j = pTempV[j].iNext )
                {
                    // If both vertices are close then remap vertex
                    if( TempVCompare( pTempV[j], pTempV[I] ))
                    {
                        // Remap pVertex and bail
                        pTempV[I].RemapIndex    = j;
                        H.iLastMerged = j;

                        // Add this vert to the vert we mapped to
                        {
                            tempv& TV = pTempV[ pTempV[I].RemapIndex ];
                            TV.Position  = (TV.Position + pTempV[I].Position) * 0.5f;
                            TV.Normal   += pTempV[I].Normal;
                            TV.Normal.Normalize();
                        }

                        break;
                    }
                }

                // Did we find a match?
                if( j != -1 )
                    break;
            }
        }

        // If we found no match then add to iHash[0]
        if( pTempV[I].RemapIndex == -1 )
        {
            pTempV[I].RemapIndex = I;
            hash& H = pHash[iHash[0]];

            pTempV[I].iNext = H.iNext;                
            H.iNext = I;
            H.nVerts++;
            H.iLastMerged = I;
            MaxVertsInHash = MAX( MaxVertsInHash, H.nVerts );
        }
    }
    
    #ifdef DO_DEBUGMSG
    x_DebugMsg("LastMergedHits %d\n",LastMergedHit);
    x_DebugMsg("Max verts in a single hash: %d\n",MaxVertsInHash);
    #endif

    // Okay now we must collapse all the unused vertices
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
    pVertex     = (vertex*)x_malloc( sizeof(vertex) * nVerts );
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

            // Copy merged position into vertex
            pVertex[ pTempV[ iRemap ].RemapIndex ].Position = pTempV[ iRemap ].Position;
            pVertex[ pTempV[ iRemap ].RemapIndex ].Normal = pTempV[ iRemap ].Normal;
        }
    }

    x_DebugMsg("Final nVerts: %d\n",nVerts);

    // Finally set the new count and 
    x_free( m_pVertex );
    m_pVertex = NULL;
    m_TotalVerticesRemoved += m_nVertices - nVerts;
    m_pVertex   = pVertex;
    m_nVertices = nVerts;
    m_VBuffSize = nVerts;

    // clean up
    x_free(pHash);
    pHash = NULL;
    x_free(pTempV);
    pTempV = NULL;
    return;
    
    // Handle the errors
    x_catch_begin;

    if( pHash   ) x_free(pHash);
    if( pTempV  ) x_free(pTempV);
    if( pVertex ) x_free( pVertex );

    x_catch_end_ret;
}

//=========================================================================

xbool raycast_lighting::ComputeRayTriCollision( 
    const facet&    Facet,
    const vector3&  aStart, 
    const vector3&  aDir )
{
    vector3 Start;
    vector3 Dir;

    if( DO_BACKWARDS_RAYCAST )
    {
        Start = aStart;
        Dir   = aDir;
    }
    else
    {
        Start = aStart + aDir;
        Dir   = -aDir;
    }


    m_NumberOfTriRayIntersections++;

    // does the tri cast shadows
    if( !(Facet.Flags & FLAG_CAST_SHADOW) )
    {
        return FALSE;
    }

    // Compute T intersect value.  
    f32 T;
    {
        T = Facet.Plane.Normal.Dot( Dir );

        // check whether we have a parallel line (also makes our division zero)
        if( x_abs(T) <= 0.00001f )
            return FALSE;

        T = -Facet.Plane.Distance( Start ) / T;
        if( T>1 || T<0 )
            return FALSE;
    }

    // Compute the actual contact point which lies on the plane
    vector3 HitPoint = Start + ( Dir * T);

    // See if hit point is inside tri.  This could be sped up quite a bit.
    //if( Facet.EdgePlane[0].InFront( HitPoint ) ) return FALSE;
    //if( Facet.EdgePlane[1].InFront( HitPoint ) ) return FALSE;
    //if( Facet.EdgePlane[2].InFront( HitPoint ) ) return FALSE;

    vector3 EdgeNormal;
    for( s32 i=0; i<3; i++ )
    {
        EdgeNormal = Facet.Plane.Normal.Cross( m_pVertex[ Facet.iVertex[(i+1)%3] ].Position - m_pVertex[ Facet.iVertex[i] ].Position );
        if( EdgeNormal.Dot( HitPoint - m_pVertex[ Facet.iVertex[i] ].Position ) < 0.0f )
        {
            return FALSE;
        }
    }

    // Collision
    return TRUE;
}

//=========================================================================

void raycast_lighting::LightingEnd( void )
{
    s32 i,j;

    struct vert_color
    {
        vector3 C;
        f32     W;
    };

    vert_color* pVertColor = (vert_color*)x_malloc(sizeof(vert_color) * m_nVertices );
    if( !pVertColor )
    {
        x_throw( "Out of memory" );
    }

    // Clear vertex accumulations
    for( i=0; i<m_nVertices; i++ )
    {
        pVertColor[i].C.Zero();
        pVertColor[i].W = 0;
    }

    // Run through facets and apply subdivided points
    for( i=0; i<m_nFacets; i++ )
    {
        facet& F = m_pFacet[i];
        subdiv_info& SD = m_SubDivInfo[F.nSubdivisions];

        for( j=0; j<SD.nPoints; j++ )
        {
            vector3 C = F.pC[j];

            // Peg color in valid color range
            if( C.GetX() <   0 ) C.GetX() = 0;
            if( C.GetX() > 255 ) C.GetX() = 255;
            if( C.GetY() <   0 ) C.GetY() = 0;
            if( C.GetY() > 255 ) C.GetY() = 255;
            if( C.GetZ() <   0 ) C.GetZ() = 0;
            if( C.GetZ() > 255 ) C.GetZ() = 255;

            // Distribute color to verts
            pVertColor[F.iVertex[0]].C += C * SD.U[j];
            pVertColor[F.iVertex[1]].C += C * SD.V[j];
            pVertColor[F.iVertex[2]].C += C * SD.W[j];
            pVertColor[F.iVertex[0]].W +=     SD.U[j];
            pVertColor[F.iVertex[1]].W +=     SD.V[j];
            pVertColor[F.iVertex[2]].W +=     SD.W[j];
        }
    }

    // Process each vertex
    for( i=0; i<m_nVertices; i++ )
    {
        vector3 L = pVertColor[i].C;

        if( pVertColor[i].W == 0 )
            L.Set( 0,0,0 );
        else
            L /= pVertColor[i].W;

        if( L.GetX() <   0 ) L.GetX() = 0;
        if( L.GetX() > 255 ) L.GetX() = 255;
        if( L.GetY() <   0 ) L.GetY() = 0;
        if( L.GetY() > 255 ) L.GetY() = 255;
        if( L.GetZ() <   0 ) L.GetZ() = 0;
        if( L.GetZ() > 255 ) L.GetZ() = 255;

        m_pVertex[i].FinalColor = xcolor( (byte)L.GetX(), (byte)L.GetY(), (byte)L.GetZ() );
    }

    x_free(pVertColor);
}

//=========================================================================

void raycast_lighting::CompileData( void )
{
    xtimer Timer;
    
    x_DebugMsg( "Collapsing %d vertices...\n",m_nVertices);
    Timer.Start();
    CollapseVertexPositions();
    Timer.Stop();
    x_DebugMsg( "Done in %f Seconds. %d vertices removed.\n", Timer.ReadSec(), m_TotalVerticesRemoved );

    x_DebugMsg( "Initializing triangle subdivision data...\n");
    if( 1 )
    {
        // Build facet groups
        {
            m_nFacetGroups = 0;
            s32 LastSlot = -1;
            s32  Count=0;
            s32 iStart=0;
            bbox BBox;

            while( iStart < m_nFacets )
            {
                LastSlot = m_pFacet[iStart].SlotID;
                Count    = 0;
                BBox.Clear();
                s32 iEnd = iStart;

                while( 1 )
                {
                    if( iEnd == m_nFacets )
                        break;

                    if( m_pFacet[iEnd].SlotID != LastSlot )
                        break;

                    if( Count == 512 ) 
                        break;

                    BBox += m_pFacet[iEnd].BBox;
                    Count++;
                    iEnd++;
                }

                if( m_nFacetGroups == m_nFacetGroupsAllocated )
                {
                    m_nFacetGroupsAllocated += MAX(16384,m_nFacetGroupsAllocated/2);
                    m_pFacetGroup = (facet_group*)x_realloc(m_pFacetGroup,sizeof(facet_group)*m_nFacetGroupsAllocated);
                    ASSERT(m_pFacetGroup);
                }

                m_pFacetGroup[m_nFacetGroups].iStart = iStart;
                m_pFacetGroup[m_nFacetGroups].iEnd   = iEnd-1;
                m_pFacetGroup[m_nFacetGroups].BBox   = BBox;
                m_nFacetGroups++;

                iStart = iEnd;
            }
        }

        // Build subdivision groups
        {
            s32 i;
            x_memset(m_SubDivInfo,0,sizeof(m_SubDivInfo));
            for( i=1; i<=MAX_SUBDIVISIONS; i++ )
            {
                m_SubDivInfo[i].nSubdivision = i;
                m_SubDivInfo[i].nPoints = 0;

                f32 UVWStep = 1.0f/((f32)m_SubDivInfo[i].nSubdivision);
                for( f32 u=0; u<=1.0f;  u+=UVWStep )
                for( f32 v=0; v<=(1-u); v+=UVWStep )
                {
                    ASSERT( m_SubDivInfo[i].nPoints < MAX_SUBDIVIDED_POINTS );
                    m_SubDivInfo[i].U[ m_SubDivInfo[i].nPoints ] = u;
                    m_SubDivInfo[i].V[ m_SubDivInfo[i].nPoints ] = v;
                    m_SubDivInfo[i].W[ m_SubDivInfo[i].nPoints ] = 1 - u - v;
                    m_SubDivInfo[i].nPoints++;
                }
            }
        }


        // Decide subdivisions for Facets
        {
            s32 i,j;
            s32 nPointsToLight = 0;
            s32 Hist[MAX_SUBDIVISIONS+1]={0};
            for( i=0; i<m_nFacets; i++ )
            {
                facet& F = m_pFacet[i];

                f32 LongestEdgeLenSquared = 0;
                for( j=0; j<3; j++ )
                {
                    vector3 Diff = m_pVertex[F.iVertex[(j+1)%3]].Position - m_pVertex[F.iVertex[j]].Position;
                    f32 LS = Diff.LengthSquared();
                    if( LS > LongestEdgeLenSquared )
                        LongestEdgeLenSquared = LS;
                }

                f32 Len = x_sqrt(LongestEdgeLenSquared);
                s32 nSubdivisions = (s32)((Len / SUBDIVISION_STEP_LENGTH) + 0.5f);
                if( nSubdivisions < 1 ) nSubdivisions = 1;
                if( nSubdivisions > MAX_SUBDIVISIONS ) nSubdivisions = MAX_SUBDIVISIONS;
                F.nSubdivisions = nSubdivisions;

                nPointsToLight += m_SubDivInfo[nSubdivisions].nPoints;
                Hist[nSubdivisions]++;

            }
            
            #ifdef DO_DEBUGMSG
            for( i=0; i<=MAX_SUBDIVISIONS; i++ )
                x_DebugMsg("[%2d] Subdivisions %6d\n",i,Hist[i]);
            #endif

            x_DebugMsg("TotalPointsToLight:          %10d\n",nPointsToLight);
            x_DebugMsg("BruteForce RayChecks Needed: %10d\n",nPointsToLight*m_nLights);

        }

        // Build facet color array
        {
            m_pFacetColor   = NULL;
            m_nFacetColors  = 0;

            s32 i;
            s32 C = 0;
            for( i=0; i<m_nFacets; i++ )
            {
                facet& F = m_pFacet[i];
                C += m_SubDivInfo[ F.nSubdivisions ].nPoints;
            }

            m_nFacetColors = C;
            x_DebugMsg("Alloc facet color data: %d %d\n",m_nFacetColors,sizeof(vector3)*m_nFacetColors);
            m_pFacetColor = (vector3*)x_malloc(sizeof(vector3)*m_nFacetColors);
            x_memset( m_pFacetColor, 0, sizeof(vector3)*m_nFacetColors);

            C = 0;
            for( i=0; i<m_nFacets; i++ )
            {
                facet& F = m_pFacet[i];
                F.pC = &m_pFacetColor[C];
                C += m_SubDivInfo[ F.nSubdivisions ].nPoints;
            }
        }
    }


    //
    // Allocate gridcells and initial facetrefs
    //
    x_DebugMsg("Initializing gridcells and facetrefs...\n");
    {
        m_pGridCell = (gridcell*)x_malloc(sizeof(gridcell)*RAYCAST_GRID_SIZE * RAYCAST_GRID_SIZE * RAYCAST_GRID_SIZE);
        ASSERT(m_pGridCell);

        m_nFacetRefsAllocated = 65536;
        m_nFacetRefs = 0;
        m_pFacetRef = (facetref*)x_malloc(sizeof(facetref)*m_nFacetRefsAllocated);
        ASSERT(m_pFacetRef);
    }
}

//=========================================================================
xbool raycast_lighting::DoesFacetIntersectBBox( const facet& Facet, const vector3* P, const bbox& CellBBox )
{
/*
    // This was already being done by caller.  We wouldn't be here
    // if this weren't true.

    bbox TriBBox;
    TriBBox.Clear();
    TriBBox.AddVerts(P,3);
    TriBBox.Inflate(1,1,1);

    // Do quick check to see if bboxes intersect
    if( CellBBox.Intersect( TriBBox ) == FALSE )
        return FALSE;
*/
    // Do quick check to see if verts are in CellBBox
    if( CellBBox.Intersect(P[0]) ) return TRUE;

    // Quick BBox vs. Plane check
    {
        f32* pF = (f32*)&CellBBox;

        // Compute max dist along normal
        f32 MaxDist = Facet.Plane.Normal.GetX() * pF[Facet.MaxBBoxIndices[0]] +
                      Facet.Plane.Normal.GetY() * pF[Facet.MaxBBoxIndices[1]] +
                      Facet.Plane.Normal.GetZ() * pF[Facet.MaxBBoxIndices[2]] +
                      Facet.Plane.D;

        // If outside plane, we are culled.
        if( MaxDist < 0 )
            return FALSE;

        // Compute min dist along normal
        f32 MinDist = Facet.Plane.Normal.GetX() * pF[Facet.MinBBoxIndices[0]] +
                      Facet.Plane.Normal.GetY() * pF[Facet.MinBBoxIndices[1]] +
                      Facet.Plane.Normal.GetZ() * pF[Facet.MinBBoxIndices[2]] +
                      Facet.Plane.D;

        // If outside plane, we are culled.
        if( MinDist > 0 )
            return FALSE;
    }

    // Quick check to see if CellBBox intersects triangle plane
    //if( CellBBox.Intersect( Facet.Plane ) == FALSE )
    //    return FALSE;

    // Do quick check to see if verts are in CellBBox
    if( CellBBox.Intersect(P[1]) ) return TRUE;
    if( CellBBox.Intersect(P[2]) ) return TRUE;
    if( CellBBox.Intersect((P[0]+P[1]+P[2]) * 0.33333f)) return TRUE;

    // Clip triangle to cell bounds for accurate intersection
    vector3 SrcVert[24];
    vector3 DstVert[24];
    SrcVert[0] = P[0];
    SrcVert[1] = P[1];
    SrcVert[2] = P[2];
    s32 nSrcVerts = 3;
    s32 nDstVerts = 0;

    // Build planes
    plane   BoxPlane[6];
    BoxPlane[0].Setup( CellBBox.Min, vector3(1,0,0) );
    BoxPlane[1].Setup( CellBBox.Min, vector3(0,1,0) );
    BoxPlane[2].Setup( CellBBox.Min, vector3(0,0,1) );
    BoxPlane[3].Setup( CellBBox.Max, vector3(-1,0,0) );
    BoxPlane[4].Setup( CellBBox.Max, vector3(0,-1,0) );
    BoxPlane[5].Setup( CellBBox.Max, vector3(0,0,-1) );

    // Clip ngon to bbox
    BoxPlane[0].ClipNGon( DstVert, nDstVerts, SrcVert, nSrcVerts ); if( nDstVerts==0 ) return FALSE;
    BoxPlane[1].ClipNGon( SrcVert, nSrcVerts, DstVert, nDstVerts ); if( nSrcVerts==0 ) return FALSE;
    BoxPlane[2].ClipNGon( DstVert, nDstVerts, SrcVert, nSrcVerts ); if( nDstVerts==0 ) return FALSE;
    BoxPlane[3].ClipNGon( SrcVert, nSrcVerts, DstVert, nDstVerts ); if( nSrcVerts==0 ) return FALSE;
    BoxPlane[4].ClipNGon( DstVert, nDstVerts, SrcVert, nSrcVerts ); if( nDstVerts==0 ) return FALSE;
    BoxPlane[5].ClipNGon( SrcVert, nSrcVerts, DstVert, nDstVerts ); if( nSrcVerts==0 ) return FALSE;

    // Decided it is in
    return TRUE;
}

//=========================================================================

xbool raycast_lighting::DoesLightIntersectTriangle( const light& Light, const facet& Facet )
{
    vector3 P[3];
    P[0] = m_pVertex[Facet.iVertex[0]].Position;
    P[1] = m_pVertex[Facet.iVertex[1]].Position;
    P[2] = m_pVertex[Facet.iVertex[2]].Position;

    // Quickly check if any facet vert is within light's radius
    if( (P[0] - Light.Position).LengthSquared() < Light.RadiusSquared ) return TRUE;
    if( (P[1] - Light.Position).LengthSquared() < Light.RadiusSquared ) return TRUE;
    if( (P[2] - Light.Position).LengthSquared() < Light.RadiusSquared ) return TRUE;

    // Compute closest point on triangle plane
    f32 DistFromPlaneToLight = Facet.Plane.Distance(Light.Position);
    if( DistFromPlaneToLight > Light.Radius )
        return FALSE;

    vector3 PlanePt = Light.Position - (Facet.Plane.Normal * DistFromPlaneToLight);

    // See if PlanePt is inside the triangle
    //if( Facet.EdgePlane[0].InFront( PlanePt ) ) return FALSE;
    //if( Facet.EdgePlane[1].InFront( PlanePt ) ) return FALSE;
    //if( Facet.EdgePlane[2].InFront( PlanePt ) ) return FALSE;

    vector3 EdgeNormal;
    for( s32 i=0; i<3; i++ )
    {
        EdgeNormal = Facet.Plane.Normal.Cross( m_pVertex[ Facet.iVertex[(i+1)%3] ].Position - m_pVertex[ Facet.iVertex[i] ].Position );
        if( EdgeNormal.Dot( PlanePt - m_pVertex[ Facet.iVertex[i] ].Position ) < 0.0f )
        {
            return FALSE;
        }
    }

    return TRUE;
}

//=========================================================================

f32 SOLVE_GRID[4] = {0};

void raycast_lighting::SolveLightGrid( s32 iLight )
{
    s32 i;

    #ifdef DO_DEBUGMSG
    xtimer Timer;
    Timer.Start();
    #endif

    // Clear grid sequence
    m_GridCollisionSequence = 0;
    m_GridIFirstRef = -1;
    m_GridNFacets=0;

    // Invalidate grid cells
    m_GridUseSequence++;

    // Clear GridFacetCache
    m_GridFacetCacheLastHitI = 0;
    m_GridFacetCacheI = 0;
    for( i=0; i<GRID_FACET_CACHE_SIZE; i++ )
        m_GridFacetCache[i] = -1;

    light&  Light = m_pLight[iLight];
    m_pGridLight = &m_pLight[iLight];
    m_GridLightPos = m_pGridLight->Position;
    m_GridLightRadiusSq = m_pGridLight->RadiusSquared;

    // Compute size of grid cell
    f32 LightDiameter = (Light.Radius*2.0f) + 2.0f;
    m_GridCellSize = LightDiameter / (f32)RAYCAST_GRID_SIZE;
    if( m_GridCellSize < RAYCAST_MIN_CELL_SIZE )
        m_GridCellSize = RAYCAST_MIN_CELL_SIZE;

    // Compute how many cells wide the grid will be
    m_nGridCellsWide = (s32)(LightDiameter / m_GridCellSize);
    if( (m_nGridCellsWide * m_GridCellSize) < LightDiameter )
        m_nGridCellsWide++;

    ASSERT( (m_nGridCellsWide>0) && (m_nGridCellsWide<=RAYCAST_GRID_SIZE) );
    ASSERT( (m_nGridCellsWide * m_GridCellSize) >= LightDiameter );

    // Compute how many gridcells total there will be
    m_nGridCellsTotal = m_nGridCellsWide*m_nGridCellsWide*m_nGridCellsWide;
    m_OneOverGridCellSize = 1.0f / m_GridCellSize;
    m_GridBBox.Min = Light.Position - vector3(1,1,1)*(LightDiameter/2);
    m_GridBBox.Max = m_GridBBox.Min + vector3(m_GridCellSize,m_GridCellSize,m_GridCellSize)*((f32)m_nGridCellsWide);

    #ifdef DO_DEBUGMSG
    Timer.Stop();
    SOLVE_GRID[0] += Timer.ReadMs();
    Timer.Reset();
    Timer.Start();
    #endif

    // Reset the facet references
    m_nFacetRefs = 0;

    // Loop through the facets and setup the refs
    s32 nUniqueFacets=0;
    bbox SafeGridBBox = m_GridBBox;
    SafeGridBBox.Inflate(1,1,1);
    for( s32 FGI=0; FGI<m_nFacetGroups; FGI++ )
    if( SafeGridBBox.Intersect( m_pFacetGroup[FGI].BBox ) )
    {
        for( s32 FI=m_pFacetGroup[FGI].iStart; FI<=m_pFacetGroup[FGI].iEnd; FI++ )
        {
            vector3 P[3];
            facet& F = m_pFacet[FI];
            P[0] = m_pVertex[F.iVertex[0]].Position;
            P[1] = m_pVertex[F.iVertex[1]].Position;
            P[2] = m_pVertex[F.iVertex[2]].Position;

            // Check if facet is completely out of the range of the light
            {
                // Check if facet bbox touches grid bbox
                if( SafeGridBBox.Intersect(F.BBox) == FALSE )
                    continue;

                // Check if facet is affected by light sphere
                if( DoesLightIntersectTriangle( Light, F )==FALSE )
                {
                    TRIS_OUTSIDE_LIGHT++;
                    continue;
                }
            }

            // Check if all points in triangle will be facing away from the light
            xbool bAllPointsFaceAway=FALSE;
            {
                s32 i;
                for( i=0; i<3; i++ )
                {
                    const vector3& N = m_pVertex[ F.iVertex[i] ].Normal;
                    if( N.Dot( Light.Position - P[i] ) < 0 )
                        break;
                }
                if( i!=3 )
                {
                    bAllPointsFaceAway = TRUE;
                }
            }

            // Add facet to list of facets affected by light
            if( !bAllPointsFaceAway )
            {
                // Get a facet ref
                if( m_nFacetRefs == m_nFacetRefsAllocated )
                {
                    s32 NewSize = m_nFacetRefsAllocated + 25000;
                    
                    x_DebugMsg("Realloc facet refs (%d,%d) (%d,%d)\n",
                        m_nFacetRefsAllocated,NewSize,
                        sizeof(facetref)*m_nFacetRefsAllocated,
                        sizeof(facetref)*NewSize);

                    m_nFacetRefsAllocated = NewSize;
                    m_pFacetRef = (facetref*)x_realloc(m_pFacetRef,sizeof(facetref)*m_nFacetRefsAllocated);
                    ASSERT(m_pFacetRef);
                }
                s32 RefI = m_nFacetRefs;
                m_nFacetRefs++;

                // Attach facetref to list of facets affected by light
                m_pFacetRef[RefI].iFacet = FI;
                m_pFacetRef[RefI].iNext  = m_GridIFirstRef;
                m_GridIFirstRef = RefI;
                m_GridNFacets++;
            }

            //
            // If the facet is facing AWAY from the light then we need
            // to add it to the list of grid cells to check for LOS.
            //

            if( DO_BACKWARDS_RAYCAST )
            {
                // Check if facet is facing the light
                if( F.Plane.Distance( Light.Position ) > 0.0f )
                    continue;
            }
            else
            {
                // Check if facet is facing the light
                if( F.Plane.Distance( Light.Position ) < 0.0f )
                    continue;
            }

            // Clear the facets sequence
            nUniqueFacets++;

            // Compute the subset of grid cells the facet intersects
            vector3 Min = (F.BBox.Min - vector3(4,4,4) - m_GridBBox.Min) * m_OneOverGridCellSize;
            vector3 Max = (F.BBox.Max + vector3(4,4,4) - m_GridBBox.Min) * m_OneOverGridCellSize;
            s32 MinX = (s32)(Min.GetX()+4096.0f)-4096;
            s32 MinY = (s32)(Min.GetY()+4096.0f)-4096;
            s32 MinZ = (s32)(Min.GetZ()+4096.0f)-4096;
            s32 MaxX = (s32)(Max.GetX()+4096.0f)-4096;
            s32 MaxY = (s32)(Max.GetY()+4096.0f)-4096;
            s32 MaxZ = (s32)(Max.GetZ()+4096.0f)-4096;

            // Clip Min&Max to grid bounds
            if( MinX < 0 ) MinX = 0;
            if( MinY < 0 ) MinY = 0;
            if( MinZ < 0 ) MinZ = 0;
            if( MaxX >= m_nGridCellsWide ) MaxX = m_nGridCellsWide-1;
            if( MaxY >= m_nGridCellsWide ) MaxY = m_nGridCellsWide-1;
            if( MaxZ >= m_nGridCellsWide ) MaxZ = m_nGridCellsWide-1;

            // Loop through grid range and add facetrefs to cells
            bbox CellSafeBBox;
            for( s32 Z=MinZ; Z<=MaxZ; Z++ )
            {
                s32 ZCellOffset = Z*(m_nGridCellsWide*m_nGridCellsWide);
                CellSafeBBox.Min.GetZ() = m_GridBBox.Min.GetZ() + Z * m_GridCellSize - 1.0f;
                CellSafeBBox.Max.GetZ() = CellSafeBBox.Min.GetZ() + m_GridCellSize   + 1.0f;

                for( s32 Y=MinY; Y<=MaxY; Y++ )
                {
                    s32 YCellOffset = Y*m_nGridCellsWide;
                    CellSafeBBox.Min.GetY() = m_GridBBox.Min.GetY() + Y * m_GridCellSize - 1.0f;
                    CellSafeBBox.Max.GetY() = CellSafeBBox.Min.GetY() + m_GridCellSize   + 1.0f;

                    for( s32 X=MinX; X<=MaxX; X++ )
                    {
                        s32 GridI = X + YCellOffset + ZCellOffset;
                        ASSERT( (GridI>=0) && (GridI<m_nGridCellsTotal) );
                        gridcell& GC = m_pGridCell[GridI];

                        CellSafeBBox.Min.GetX() = m_GridBBox.Min.GetX() + X * m_GridCellSize - 1.0f;
                        CellSafeBBox.Max.GetX() = CellSafeBBox.Min.GetX() + m_GridCellSize   + 1.0f;

                        // Check if facet is really intersecting cell
                        if( !DoesFacetIntersectBBox( F, P, CellSafeBBox ) )
                            continue;

                        // If this cell has not been initialized then do so
                        if( GC.GridUseSequence != m_GridUseSequence )
                        {
                            GC.GridUseSequence       = m_GridUseSequence;
                            GC.iFirstRef             = -1;
                            GC.nRefs                 = 0;
                        }

                        // Get a facet ref
                        if( m_nFacetRefs == m_nFacetRefsAllocated )
                        {
                            m_nFacetRefsAllocated += 25000;
                            m_pFacetRef = (facetref*)x_realloc(m_pFacetRef,sizeof(facetref)*m_nFacetRefsAllocated);
                            ASSERT(m_pFacetRef);
                        }
                        s32 RefI = m_nFacetRefs;
                        m_nFacetRefs++;

                        // Attach facetref to the grid cell
                        m_pFacetRef[RefI].iFacet = FI;
                        m_pFacetRef[RefI].iNext  = GC.iFirstRef;
                        GC.iFirstRef = RefI;
                        GC.nRefs++;
                    }
                }
            }
        }
    }

#ifdef DO_DEBUGMSG
    Timer.Stop();
    SOLVE_GRID[1] += Timer.ReadMs();

    //Timer.Stop();
    x_DebugMsg("---------------------------\n");
    //x_DebugMsg("SolveLightGrid: %8.4f Sec\n",Timer.ReadSec() );
    x_DebugMsg("LightRadius:        %6d\n",(s32)Light.Radius);
    x_DebugMsg("CellSize:           %6d\n",(s32)m_GridCellSize);
    x_DebugMsg("CellsWide:          %6d\n",(s32)m_nGridCellsWide);
    x_DebugMsg("nUniqueFacets:      %6d\n",nUniqueFacets);
    x_DebugMsg("FacetRefs:          %6d\n",m_nFacetRefs);
    x_DebugMsg("%10f %10f\n",SOLVE_GRID[0],SOLVE_GRID[1]);
#endif

}

//=========================================================================

inline
xbool DoesSphereIntersectRay( const vector3& SphereCenter, f32 SphereRadius, const vector3& RayStart, const vector3& RayDir, f32 RayLen )
{
    vector3 SphereCenterInRaySpace = SphereCenter - RayStart;
    f32 DistToSphereCenterSq = SphereCenterInRaySpace.LengthSquared();
    f32 DistSq;
    f32 DistAlongRay;
    f32 SphereRadiusSq = SphereRadius*SphereRadius;

    if( DistToSphereCenterSq < SphereRadiusSq )
        goto SPHERE_HIT;

    DistAlongRay = RayDir.Dot(SphereCenterInRaySpace);
    if( (DistAlongRay < 0) || (DistAlongRay>(RayLen+SphereRadius)) )
        goto SPHERE_MISS;

    DistSq = (SphereCenterInRaySpace-RayDir*DistAlongRay).LengthSquared();
    if( DistSq > SphereRadiusSq )
        goto SPHERE_MISS;

SPHERE_HIT:
    SphereIntersectHits++;
    return TRUE;

SPHERE_MISS:
    SphereIntersectMisses++;
    return FALSE;
}

//=========================================================================

xbool raycast_lighting::HasLOSUsingGrid( const vector3& LightPos, const vector3& VertexPos )
{
    s32 i;
    //f32 CollT;
    vector3 CollPoint;
    vector3 Delta = LightPos - VertexPos;
    f32     RayLen = Delta.Length();
    vector3 RayDir = Delta * (1.0f/RayLen);
    if( RayLen < 1.0f )
        return TRUE;

    //
    // Increase the collision sequence
    //
    m_GridCollisionSequence++;
    if( m_GridCollisionSequence == S32_MAX )
    {
        for( i=0; i<m_nFacets; i++ )
            m_pFacet[i].GridCollisionSequence = 0;
        m_GridCollisionSequence = 1;
    }

    //
    // Check if there is a collision in the cache
    //
    if( 1 )
    {
        s32 i = m_GridFacetCacheLastHitI;
        while( 1 )
        {
            if( m_GridFacetCache[i] != -1 )
            {
                facet& F = m_pFacet[ m_GridFacetCache[i] ];
                if( F.GridCollisionSequence != m_GridCollisionSequence )
                {
                    F.GridCollisionSequence = m_GridCollisionSequence;

                    if( DoesSphereIntersectRay( F.SphereCenter, F.SphereRadius, VertexPos, RayDir, RayLen) ) 
                    {
                        if( ComputeRayTriCollision( F, VertexPos, Delta ) )
                        {
                            m_GridFacetCacheLastHitI = i;
                            m_GridCacheHit++;
                            return FALSE;
                        }
                    }
                }
            }

            i++;
            if( i == GRID_FACET_CACHE_SIZE ) i = 0;
            if( i == m_GridFacetCacheLastHitI )
                break;
        }
    }

    //
    // Run the cells
    //
    xbool bClearLOS=TRUE;
    {
        grid_walker GridWalker;
        ASSERT( m_GridCellSize >= 0 );

        if( DO_BACKWARDS_RAYCAST )
        {
            GridWalker.Setup( VertexPos, LightPos, m_GridBBox.Min, m_GridCellSize );
        }
        else
        {
            GridWalker.Setup( LightPos, VertexPos, m_GridBBox.Min, m_GridCellSize );
        }
        
        while( 1 )
        {
            // Pull out cell we are currently in
            s32 X,Y,Z;
            GridWalker.GetCell(X,Y,Z);

            // Be sure these are in bounds
            ASSERT( (X>=0) && (X<m_nGridCellsWide) );
            ASSERT( (Y>=0) && (Y<m_nGridCellsWide) );
            ASSERT( (Z>=0) && (Z<m_nGridCellsWide) );

            // Lookup the cell
            s32 GridI = X + Y*m_nGridCellsWide + Z*(m_nGridCellsWide*m_nGridCellsWide);
            ASSERT( (GridI>=0) && (GridI<m_nGridCellsTotal) );
            gridcell& GC = m_pGridCell[GridI];

            // Did the cell get setup with any facets?
            if( GC.GridUseSequence == m_GridUseSequence )
            {
                // Loop through facets listed in cell
                s32 FRI = GC.iFirstRef;
                while( FRI != -1 )
                {
                    // Get reference to facetref and move loop to next ref
                    facetref& FR = m_pFacetRef[FRI];
                    FRI = FR.iNext;

                    // Get reference to facet
                    facet& F = m_pFacet[FR.iFacet];
                    if( F.GridCollisionSequence == m_GridCollisionSequence )
                        continue;

                    F.GridCollisionSequence = m_GridCollisionSequence;

                    // Skip facet if it's not facing the light or sphere doesn't 
                    // intersect ray
                    if( DoesSphereIntersectRay( F.SphereCenter, F.SphereRadius, VertexPos, RayDir, RayLen) ) 
                    {
                        if( ComputeRayTriCollision( F, VertexPos, Delta ) )
                        {
                            m_GridCacheMiss++;
                            m_GridFacetCacheLastHitI = m_GridFacetCacheI;
                            m_GridFacetCache[ m_GridFacetCacheI ] = FR.iFacet;
                            m_GridFacetCacheI = (m_GridFacetCacheI+1)%GRID_FACET_CACHE_SIZE;
                            return FALSE;
                        }
                    }
                }
            }

            // Step forward to the next cell and quite if it would take us
            // past the end of the ray segment
            if( !GridWalker.Step() )
                break;
        }
    }
/*
SKIP_OUT:

    //
    // Run the cells
    //
    {
        f32 CollT;
        bbox RayBBox;
        RayBBox.Clear();
        RayBBox += LightPos;
        RayBBox += VertexPos;

        // Compute the subset of grid cells the ray intersects
        vector3 Min = (RayBBox.Min - vector3(1,1,1) - m_GridBBox.Min) * m_OneOverGridCellSize;
        vector3 Max = (RayBBox.Max + vector3(1,1,1) - m_GridBBox.Min) * m_OneOverGridCellSize;
        s32 MinX = (s32)(Min.GetX()+4096.0f)-4096;
        s32 MinY = (s32)(Min.GetY()+4096.0f)-4096;
        s32 MinZ = (s32)(Min.GetZ()+4096.0f)-4096;
        s32 MaxX = (s32)(Max.GetX()+4096.0f)-4096;
        s32 MaxY = (s32)(Max.GetY()+4096.0f)-4096;
        s32 MaxZ = (s32)(Max.GetZ()+4096.0f)-4096;

        // Clip Min&Max to grid bounds
        if( MinX < 0 ) MinX = 0;
        if( MinY < 0 ) MinY = 0;
        if( MinZ < 0 ) MinZ = 0;
        if( MaxX >= m_nGridCellsWide ) MaxX = m_nGridCellsWide-1;
        if( MaxY >= m_nGridCellsWide ) MaxY = m_nGridCellsWide-1;
        if( MaxZ >= m_nGridCellsWide ) MaxZ = m_nGridCellsWide-1;

        if( 0 )
        {
            MinX = MinY = MinZ = 0;
            MaxX = MaxY = MaxZ = m_nGridCellsWide-1;
        }

        // Loop through grid range and cells
        for( s32 Z=MinZ; Z<=MaxZ; Z++ )
        for( s32 Y=MinY; Y<=MaxY; Y++ )
        for( s32 X=MinX; X<=MaxX; X++ )
        {
            s32 GridI = X + Y*m_nGridCellsWide + Z*(m_nGridCellsWide*m_nGridCellsWide);
            ASSERT( (GridI>=0) && (GridI<m_nGridCellsTotal) );

            gridcell& GC = m_pGridCell[GridI];
            ASSERT( (GC.X==X) && (GC.Y==Y) && (GC.Z==Z) );

            if( GC.GridCollisionSequence == m_GridCollisionSequence )
                continue;

            GC.GridCollisionSequence = m_GridCollisionSequence;

            if( GC.BBox.Intersect( CollT, VertexPos, LightPos ) == FALSE )
                continue;

            // Loop through facets listed in cell
            s32 FRI = GC.iFirstRef;
            while( FRI != -1 )
            {
                // Get reference to facetref and move loop to next ref
                facetref& FR = m_pFacetRef[FRI];
                FRI = FR.iNext;

                // Get reference to facet
                facet& F = m_pFacet[FR.iFacet];
                if( F.GridCollisionSequence == m_GridCollisionSequence )
                    continue;

                F.GridCollisionSequence = m_GridCollisionSequence;

                // Skip facet if it's not facing the light or sphere doesn't 
                // intersect ray
                if( DoesSphereIntersectRay( F.SphereCenter, F.SphereRadius, VertexPos, RayDir, RayLen) ) 
                {
                    if( ComputeRayTriCollision( F, VertexPos, Delta ) )
                    {
                        m_GridCacheMiss++;
                        m_GridFacetCacheLastHitI = m_GridFacetCacheI;
                        m_GridFacetCache[ m_GridFacetCacheI ] = FR.iFacet;
                        m_GridFacetCacheI = (m_GridFacetCacheI+1)%GRID_FACET_CACHE_SIZE;
                        ASSERT( bClearLOS == FALSE );
                        return FALSE;
                    }
                }
            }
        }
    }

    ASSERT( bClearLOS==TRUE );
    */
    return TRUE;
}

//=========================================================================

void raycast_lighting::ProcessTrianglesInLightGrid( void)
{
    vector3 LightPos            = m_pGridLight->Position;
    f32     LightRadius         = m_pGridLight->Radius;
    f32     LightRadiusSquared  = m_pGridLight->RadiusSquared;

    vector3 LightAmbient    = vector3(m_pGridLight->AmbLight.R,
                                      m_pGridLight->AmbLight.G,
                                      m_pGridLight->AmbLight.B);

    vector3 LightColor      = vector3(m_pGridLight->Color.R,
                                      m_pGridLight->Color.G,
                                      m_pGridLight->Color.B);
    // Increase light by its intensity
    LightAmbient *= m_pGridLight->Intensity;
    LightColor   *= m_pGridLight->Intensity;

    // Loop through facets
    s32 FRI = m_GridIFirstRef;
    while( FRI != -1 )
    {
        // Get reference to facetref and move loop to next ref
        facetref& FR = m_pFacetRef[FRI];
        FRI = FR.iNext;

        // Get reference to facet
        facet& F = m_pFacet[FR.iFacet];
        subdiv_info& SD = m_SubDivInfo[F.nSubdivisions];

        s32 i;

        // Iterate through subdivision points
        POINTS_PROCESSED += SD.nPoints;
        for( i=0; i<SD.nPoints; i++ )
        {
            f32 u = SD.U[i];
            f32 v = SD.V[i];
            f32 w = SD.W[i];

            vector3 P = u*m_pVertex[ F.iVertex[0] ].Position + 
                        v*m_pVertex[ F.iVertex[1] ].Position + 
                        w*m_pVertex[ F.iVertex[2] ].Position;

            vector3 N = u*m_pVertex[ F.iVertex[0] ].Normal + 
                        v*m_pVertex[ F.iVertex[1] ].Normal + 
                        w*m_pVertex[ F.iVertex[2] ].Normal;

            N.Normalize();

            //-------------------------------------------------


            // Do lighting computation
            {
                vector3 LightContribution;

                // Compute delta and distance from point to light center
                vector3 PointToLight = LightPos - P;
                f32     DistFromLightSquared = PointToLight.LengthSquared();

                // Be sure point is actually within light's radius
                if( DistFromLightSquared <= LightRadiusSquared )
                {
                    POINTS_INSIDE_LIGHT++;

                    // Compute falloff contribution
                    f32 FalloffI = 1.0f - ( DistFromLightSquared / LightRadiusSquared );
                    if( FalloffI < 0 ) FalloffI = 0;
                    if( FalloffI > 1 ) FalloffI = 1;

                    // Add ambient color into contribution
                    LightContribution = LightAmbient * FalloffI;

                    // Check if normal is facing the light
                    f32 NormalI = N.Dot( PointToLight );
                    if( NormalI >= 0 )
                    {
                        // Check if point is visible by the light's center
                        vector3 LP = P + (N*DIST_TO_DISPLACE_POINT_BEFORE_LIGHTING);
                        if( (DO_LOS_CHECK==FALSE) || HasLOSUsingGrid( LightPos, LP ) )
                        {
                            POINTS_WITH_LOS++;

                            // Finish computing normal intensity
                            f32 DistFromLight = x_sqrt(DistFromLightSquared);
                            NormalI /= DistFromLight;
                            if( NormalI < 0 ) NormalI = 0;
                            if( NormalI > 1 ) NormalI = 1;

                            LightContribution += LightColor * (FalloffI * NormalI);
                        }
                        else
                        {
                            POINTS_IN_SHADOW++;
                        }
                    }
                    else
                    {
                        POINTS_BACKFACED++;
                    }

                    // Finally, add contribution to the facet's colors for this point
                    F.pC[i] += LightContribution;
                }
                else
                {
                    POINTS_OUTSIDE_LIGHT++;
                }
            }
        }
    }
}
    
//=========================================================================

void raycast_lighting::ComputeLighting( void )
{
    //
    // test light cache
    //
    {
        m_GridUseSequence = 0;
        s32 LightList[] = {43,-1};//22,30,43,-1};

        if (g_WorldEditor.m_pHandler) g_WorldEditor.m_pHandler->SetProgressRange(0,m_nLights);

        x_DebugMsg("**************************************************\n");
        x_DebugMsg("nLights: %d\n",m_nLights);
        xtimer Timer;
        Timer.Start();
        for( s32 i=0; i<m_nLights; i++ )
        {
            s32 I = i;

            //I = LightList[i]; if( I==-1 ) break;
            if (g_WorldEditor.m_pHandler) g_WorldEditor.m_pHandler->SetProgress(i);

            m_GridCacheHit = 0;
            m_GridCacheMiss = 0;
            m_NumberOfTriRayIntersections = 0;
            BACKFACED_TRIS = 0;

            POINTS_BACKFACED        = 0;
            POINTS_WITH_LOS         = 0;
            POINTS_IN_SHADOW        = 0;
            POINTS_OUTSIDE_LIGHT    = 0;
            POINTS_INSIDE_LIGHT     = 0;
            POINTS_PROCESSED        = 0;
            TRIS_OUTSIDE_LIGHT      = 0;

            xtimer TimerA;
            TimerA.Start();
            SolveLightGrid(I);
            TimerA.Stop();

            xtimer TimerB;
            TimerB.Start();
            ProcessTrianglesInLightGrid();
            TimerB.Stop();

            f32 LPS = ((f32)(i+1) / Timer.ReadSec());
            f32 ETT = m_nLights / LPS;
            f32 ETR = (m_nLights - i - 1) / LPS;

#ifdef DO_DEBUGMSG
            x_DebugMsg("Processed light %3d of %3d (%6.2f,%6.2f) (%8d,%8d,%8d) (%5.1f,%5.1f,%5.1f)\n",
                I,m_nLights,
                TimerA.ReadSec(),TimerB.ReadSec(),
                m_GridCacheHit,m_GridCacheMiss,(u32)m_NumberOfTriRayIntersections,
                LPS,ETT,ETR);
            x_DebugMsg("SphereIntersect HT:%8d  MS:%8d\n",SphereIntersectHits,SphereIntersectMisses);
            SphereIntersectHits = 0;
            SphereIntersectMisses =0;
            x_DebugMsg("BACKFACED_TRIS          %6d\n",BACKFACED_TRIS       );
            x_DebugMsg("POINTS_BACKFACED        %6d\n",POINTS_BACKFACED     );
            x_DebugMsg("POINTS_WITH_LOS         %6d\n",POINTS_WITH_LOS      );
            x_DebugMsg("POINTS_IN_SHADOW        %6d\n",POINTS_IN_SHADOW     );
            x_DebugMsg("POINTS_OUTSIDE_LIGHT    %6d\n",POINTS_OUTSIDE_LIGHT );
            x_DebugMsg("POINTS_INSIDE_LIGHT     %6d\n",POINTS_INSIDE_LIGHT  );
            x_DebugMsg("POINTS_PROCESSED        %6d\n",POINTS_PROCESSED     );
            x_DebugMsg("TRIS_OUTSIDE_LIGHT      %6d\n",TRIS_OUTSIDE_LIGHT     );
#else
            x_DebugMsg("Processed light %4d of %4d.  (est) %3d min %3d sec remaining\n",
                I+1,m_nLights,
                ((s32)ETR)/60,
                ((s32)ETR)%60
                );
#endif
        }
        Timer.Stop();
        x_DebugMsg("Total raycast time: %f\n",Timer.ReadSec());
        x_DebugMsg("**************************************************\n");
    }

    LightingEnd();

    if (g_WorldEditor.m_pHandler) g_WorldEditor.m_pHandler->SetProgress(0);
}

//=========================================================================

void raycast_lighting::OnEnumProp( prop_enum&  List )
{
    List.PropEnumHeader( "RayCast", "This is to set the properties for the ray casting system", 0 );
    List.PropEnumBool( "RayCast\\DoReverseRaycast","",0);
    //List.AddInt  ( "RayCast\\Subdivisions", "How many times to subdivide the triangles. The more time the smoother the light will be but the time will became much larger" );
    //List.AddBool ( "RayCast\\LockGeometry", "Tells the system that even when the geometry changes in the editor the geometry in the system will not change." );

}

//=========================================================================

xbool raycast_lighting::OnProperty( prop_query& I )
{
    if( I.VarBool( "RayCast\\DoReverseRaycast", DO_BACKWARDS_RAYCAST ) )
        return TRUE;
/*
    if( I.VarInt( "RayCast\\Subdivisions", m_nSubdivisions, 1, 8 ) )
        return TRUE;

    if( I.VarBool( "RayCast\\LockGeometry", m_bLockGeometry ) )
    {
        // Can't lock anything untill we have some data
        if( m_bLockGeometry )
        {
            if( m_nVertices == 0 )
                m_bLockGeometry = FALSE;
        }
        return TRUE;
    }
*/

    return FALSE;
}

//=========================================================================




