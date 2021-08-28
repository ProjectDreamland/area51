#ifndef RAYCAST_LIGHTING_HPP
#define RAYCAST_LIGHTING_HPP

//=========================================================================
// INCLUDE
//=========================================================================
#include "x_files.hpp"
#include "Auxiliary\MiscUtils\Property.hpp"

#define RAYCAST_GRID_SIZE       64
#define GRID_FACET_CACHE_SIZE   8
#define RAYCAST_MIN_CELL_SIZE   (25.0f)

#define MAX_SUBDIVISIONS        4
#define MAX_SUBDIVIDED_POINTS   28

//=========================================================================
// CLASS
//=========================================================================
class raycast_lighting : public prop_interface
{
public:

    enum facet_flags
    {
        FLAG_CAST_SHADOW    =   (1<<0),
    };

virtual void  OnEnumProp           ( prop_enum&  List );
virtual xbool OnProperty           ( prop_query& I    );
    
           ~raycast_lighting        ( void );
            raycast_lighting        ( void );

    void    AddFacet                ( vector3& P0, vector3& N0, vector3& P1, vector3& N1,
                                      vector3& P2, vector3& N2, s32 SlotID, u32 Flags,
                                      u32 UserData1, u32 UserData2 );

    void    AddLight                ( vector3& Pos, f32 AttenR, xcolor Color, xcolor AmbColor, f32 Intensity );
    void    CompileData             ( void );
    void    ComputeLighting         ( void );
    void    ComputePartialLighting  ( s32 iStart, s32 iFinish );
    void    Save                    ( const char* pFileName );
    void    Load                    ( const char* pFileName );
    void    Render                  ( void );
    xbool   CheckCollision          ( vector3& Start, vector3& Dir );
    vector3 GetHitPoint             ( void );
    void    Clear                   ( void );

    s32     GetFacetCount           ( void ) const;
    void    GetFacetData            ( s32 Index, s32* pI, s32& SlotID, u32* pUserData ) const;
    xcolor  GetVertexColor          ( s32 Index ) const;

protected:

    struct tempv
    {
        vector3 Position;
        vector3 Normal;
        s32     RemapIndex;     // Which vertex Index it shold now use. 
        s32     Index;          // Index to the original vertex
        s32     iNext;          // next node in the has 
    };

    struct subdiv_info
    {
        s32 nSubdivision;
        s32 nPoints;
        f32 U[MAX_SUBDIVIDED_POINTS];
        f32 V[MAX_SUBDIVIDED_POINTS];
        f32 W[MAX_SUBDIVIDED_POINTS];
    };

    struct gridcell
    {
        s32     nRefs;
        s32     iFirstRef;
        s32     GridUseSequence;
    };

    struct facetref
    {
        s32     iFacet;
        s32     iNext;
    };

    struct light
    {
        vector3     Position;
        f32         Radius;
        f32         RadiusSquared;
        xcolor      Color;
        bbox        BBox;
        f32         Intensity;
        xcolor      AmbLight;
    };

    struct vertex
    {
        vector3     Normal;
        vector3     Position;
        xcolor      FinalColor;
    };

    struct facet
    {
        bbox        BBox;
        vector3     SphereCenter;
        plane       Plane;
        s32         iVertex[3];
        u32         Flags;
        u32         UserData[2];
        f32         SphereRadius;
        s32         GridCollisionSequence;
        u32         nSubdivisions:8,
                    SlotID:16;
        vector3*    pC;
        byte        MinBBoxIndices[3];
        byte        MaxBBoxIndices[3];
    };

    struct facet_group
    {
        s32         iStart;
        s32         iEnd;
        bbox        BBox;
    };

protected:

    void            CollapseVertexPositions     ( void );
    static xbool    TempVCompare                ( const tempv& A, const tempv& B );
    xbool           ComputeRayTriCollision      ( const facet& Facet, const vector3& Start, const vector3& Dir );
    void            LightingEnd                 ( void );
    void            SolveLightGrid              ( s32 iLight );
    void            ProcessTrianglesInLightGrid ( void );
    xbool           HasLOSUsingGrid             ( const vector3& StartPos, const vector3& EndPos );
    xbool           DoesLightIntersectTriangle  ( const light& Light, const facet& Facet );
    xbool           DoesFacetIntersectBBox      ( const facet& Facet, const vector3* pPoint, const bbox& BBox );

protected:

    s32         m_VBuffSize;
    s32         m_nVertices;
    vertex*     m_pVertex;

    s32         m_FBuffSize;
    s32         m_nFacets;
    facet*      m_pFacet;

    facet_group* m_pFacetGroup;
    s32         m_nFacetGroups;
    s32         m_nFacetGroupsAllocated;

    s32         m_LBuffSize;
    s32         m_nLights;
    light*      m_pLight;

    gridcell*   m_pGridCell;
    s32         m_nGridCellsTotal;
    s32         m_nGridCellsWide;
    f32         m_GridCellSize;
    f32         m_OneOverGridCellSize;
    bbox        m_GridBBox;
    s32         m_GridCollisionSequence;
    light*      m_pGridLight;
    vector3     m_GridLightPos;
    f32         m_GridLightRadiusSq;
    s32         m_GridFacetCache[GRID_FACET_CACHE_SIZE];
    s32         m_GridFacetCacheI;
    s32         m_GridFacetCacheLastHitI;
    s32         m_GridCacheHit;
    s32         m_GridCacheMiss;
    s32         m_GridIFirstRef;
    s32         m_GridNFacets;
    s32         m_GridUseSequence;

    facetref*   m_pFacetRef;
    s32         m_nFacetRefsAllocated;
    s32         m_nFacetRefs;

    bbox        m_BBox;                             // BBox of the world
    s32         m_TotalVerticesRemoved;             // Tells how many vertices have been collapse when preparing the mesh

    s32         m_CollisionIndexFacet;              // Index to the facet which we have collided
    f32         m_CollisionT;                       // Tell what T we have collided
    vector3     m_CollisionPoint;                   // The actual position of the collision
    s64         m_NumberOfTriRayIntersections;      // Number of tri/ray intersection checks
    s64         m_NumberOfRaysCheck;                // Number of rays checks done
    s64         m_NumberOfNodeTravel;               // Number of nodes travel in the BSP
    s32         m_iLastNodeCollision;
    s32         m_iLastFacetCollision;
    s32         m_NumberNodeChaceSuccess;
    s32         m_NumberFacetCacheSuccess;

    subdiv_info m_SubDivInfo[MAX_SUBDIVISIONS+1];
    vector3*    m_pFacetColor;
    s32         m_nFacetColors;
};

//=========================================================================
// INLINE
//=========================================================================

//=========================================================================
inline
s32 raycast_lighting::GetFacetCount( void ) const
{
    return m_nFacets;
}

//=========================================================================
inline
void raycast_lighting::GetFacetData( s32 Index, s32* pI, s32& SlotID, u32* pUserData ) const
{
    ASSERT( Index >=0 );
    ASSERT( Index < m_nFacets );

    pI[0] = m_pFacet[ Index ].iVertex[0];
    pI[1] = m_pFacet[ Index ].iVertex[1];
    pI[2] = m_pFacet[ Index ].iVertex[2];

    pUserData[0] = m_pFacet[ Index ].UserData[0];
    pUserData[1] = m_pFacet[ Index ].UserData[1];

    SlotID = m_pFacet[ Index ].SlotID;
}

//=========================================================================
inline 
xcolor raycast_lighting::GetVertexColor( s32 Index ) const
{
    ASSERT( Index >=0 );
    ASSERT( Index < m_nVertices );

    return m_pVertex[ Index ].FinalColor;
}

//=========================================================================
// END
//=========================================================================
#endif