#ifndef LIGHTING_HPP
#define LIGHTING_HPP

//=========================================================================
// INCLUDE
//=========================================================================
#include "x_files.hpp"

class lighting
{
public:
    
           ~lighting                ( void );
            lighting                ( void );

    void    AddFacet                ( vector3& P0, vector3& N0, vector3& P1, vector3& N1,
                                      vector3& P2, vector3& N2, guid Object, u32 UserData );

    void    AddLight                ( vector3& Pos, f32 AttenR, xcolor Color, xcolor AmbColor );
    void    CompileData             ( void );
    void    ComputeLighting         ( void );
    void    ComputePartialLighting  ( s32 iStart, s32 iFinish );
    void    Save                    ( const char* pFileName );
    void    Load                    ( const char* pFileName );
    void    Render                  ( void );
    xbool   CheckCollision          ( vector3& Start, vector3& Dir );
    vector3 GetHitPoint             ( void );

protected:

    struct light
    {
        vector3     Position;
        f32         AttenR;
        xcolor      Color;
        bbox        BBox;
        xcolor      AmbLight;
    };

    struct vertex
    {
        vector3     Normal;
        vector3     Position;
        vector3     C1;
        vector3     C2;
        f32         W;
        xcolor      FinalColor;
    };

    struct facet
    {
        plane       Plane;
        plane       EdgePlane[3];
        s32         iVertex[3];
        guid        gObject;
        u32         UserData;
        bbox        BBox;
    };

    struct node
    {
        bbox        BBox;           // BBox of this bsp node
        f32         AxisOffset;     // 
        s16         Axis;           // [0=+X,1=+Y,2=+Z]
        s32         iLeft;
        s32         iRight;

        s32         nFacets;
        s32*        pFacet;
    };

    enum 
    {
        LEAF_SIZE       = 12,             // Maximun poly counts allow in a leaf
        MAX_DEPTH       = 32,             // Maximun depth for the bsp
        LIGHTING_STEPS  = 4               // How many times to sub-divide the triangle
    };

protected:

    void            CollapseVertexPositions ( void );
    static xbool    TempVCompare            ( const vertex& A, const vertex& B );
    void            BuildNodes              ( void );
    s32             BuildBranch             ( s32* piFacet, s32 Size, s32 Axis, const vector3& Min, const vector3& Max, s32 Depth );
    xbool           ComputeRayTriCollision  ( const facet& Facet, const vector3& Start, const vector3& Dir, f32& T, vector3& HitPoint );
    xbool           NodeRayIntersection     ( const node& Node, const vector3& Origin, const vector3& Dir, f32 tMin, f32 tMax );
    xbool           RayHit                  ( s32 iBaseNode, const vector3& Origin, const vector3& Dir, f32 TMin, f32 TMax );
    void            LightingTriangle        ( s32 Index );
    void            LightingEnd             ( void );
    xbool           CanSeeLight             ( light& Light, vector3& Point );

protected:

    s32         m_VBuffSize;
    s32         m_nVertices;
    vertex*     m_pVertex;

    s32         m_TBuffSize;
    s32         m_nFacets;
    facet*      m_pFacet;

    s32         m_LBuffSize;
    s32         m_nLights;
    light*      m_pLight;

    s32         m_BuffNodeSize;
    s32         m_nNodes;
    node*       m_pNode; 

    bbox        m_BBox;                             // BBox of the world
    s32         m_TotalVerticesRemoved;             // Tells how many vertices have been collapse when preparing the mesh
    s32         m_BSPNumberOfFacetsResolved;        // use to update the progress-bar while building the bsp-tree
    s32         m_BSPDuplicatedFacets;              // Number of duplicated facets that the bsp generated
    s32         m_BSPBuildNodeIterator;

    xbool       m_bFindClosestIntersection;         // Turn on/off to get acurate collisions
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
};

//=========================================================================
// END
//=========================================================================
#endif