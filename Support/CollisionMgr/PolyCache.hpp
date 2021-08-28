//==============================================================================
//
//  PolyCache.hpp
//
//==============================================================================

#ifndef POLYCACHE_HPP
#define POLYCACHE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_stdio.hpp"
#include "x_math.hpp"
#include "miscutils\Guid.hpp"
#include "render\collisionvolume.hpp"
#include "objects\object.hpp"
#include "GridWalker.hpp"

#if defined( X_EDITOR ) || (!defined( CONFIG_RETAIL ) && !defined(CONFIG_PROFILE))
#define DEBUG_POLY_CACHE
#endif

//==============================================================================
//  TYPES
//==============================================================================

class poly_cache;

//==============================================================================
//  DEFINES
//==============================================================================

#ifdef TARGET_PC

    #define POLYCACHE_MAX_CELLS                         800
    #define POLYCACHE_HASH_SIZE                        2053
    #define POLYCACHE_MAX_8_CLUSTERS                   1024
    #define POLYCACHE_MAX_16_CLUSTERS                   512
    #define POLYCACHE_MAX_32_CLUSTERS                   512
    #define POLYCACHE_NUM_CLUSTER_BANKS                   3  
    #define POLYCACHE_MAX_CLUSTER_PTRS_PER_CELL          32
    #define POLYCACHE_MAX_CLUSTERS_IN_LIST             1024

#else

    #define POLYCACHE_MAX_CELLS                         800
    #define POLYCACHE_HASH_SIZE                        2053
    #define POLYCACHE_MAX_8_CLUSTERS                    512
    #define POLYCACHE_MAX_16_CLUSTERS                    64
    #define POLYCACHE_MAX_32_CLUSTERS                   180
    #define POLYCACHE_NUM_CLUSTER_BANKS                   3  
    #define POLYCACHE_MAX_CLUSTER_PTRS_PER_CELL          35
    #define POLYCACHE_MAX_CLUSTERS_IN_LIST              512

#endif

#define POLYCACHE_MAX_CLUSTERS      (POLYCACHE_MAX_8_CLUSTERS+POLYCACHE_MAX_16_CLUSTERS+POLYCACHE_MAX_32_CLUSTERS)
#define POLYCACHE_CLUSTER_HASH_SIZE ((POLYCACHE_MAX_CLUSTERS*2)+1)
#define POLYCACHE_MAX_SEQUENCE      (65535)
 
//==============================================================================
//==============================================================================
//==============================================================================
// POLY_CACHE
//==============================================================================
//==============================================================================
//==============================================================================

class poly_cache
{

//------------------------------------------------------------------------------
//  Public structures
//------------------------------------------------------------------------------
public:

//==============================================================================
#ifdef DEBUG_POLY_CACHE
    struct debug
    {
        xbool RENDER;
        xbool FORCE_REBUILD;     
        xbool LOG_INVALIDATIONS; 
        xbool LOG_GATHER;        
        xbool LOG_DESTROY;       
        xbool RENDER_STATS;
        xbool RENDER_HIT_CLUSTERS;  
        xbool RENDER_HIT_CLUSTERS_NORMALS;
        xbool RENDER_CLUSTER_BOXES; 
        xbool RENDER_NEW_CELL_BOXES;
        xbool RENDER_HIT_CELL_BOXES;
        xbool RENDER_ALL_CELL_BOXES;
    };
#endif // X_RETAIL

    struct cluster
    {

        #define BOUNDS_X_POS    (1<<0)
        #define BOUNDS_X_NEG    (1<<1)
        #define BOUNDS_Y_POS    (1<<2)
        #define BOUNDS_Y_NEG    (1<<3)
        #define BOUNDS_Z_POS    (1<<4)
        #define BOUNDS_Z_NEG    (1<<5)
        #define BOUNDS_IS_QUAD  (1<<6)

        struct bounds
        {
            vector3 BBoxMin;
            vector3 BBoxMax;
            u32     Flags;
            f32     PlaneD;
        }  PS2_ALIGNMENT(16);

        struct quad
        {
            byte    iP[4];
            byte    iN;
        };
 
        bbox                        BBox;
        guid                        Guid;   
        u32                         AttrBits;
        u32                         PrimKey;

        //u32                         nPoints:6,
        //                            nNormals:6,
        //                           nQuads:6,
        //                            iBank:2,
        //                            ObjectType:12;

        object::type                ObjectType;
        byte                        nPoints;
        byte                        nNormals;
        byte                        nQuads;
        byte                        iBank;
        byte                        iBone;
        byte                        nReferences;
        byte                        nHits;
        u16                         iCollDataCluster;
        u16                         Sequence;
    
        cluster*                    pNext;
        vector3*                    pPoint;
        vector3*                    pNormal;
        quad*                       pQuad;
        bounds*                     pBounds;

        //const vector4*  GetPointPtr ( void ) {return g_PolyCache.GetPointPtr(*this);}
        //const vector4*  GetNormalPtr( void ) {return g_PolyCache.GetNormalPtr(*this);}
        //const quad*     GetQuadPtr  ( void ) {return g_PolyCache.GetQuadPtr(*this);}
        //const bounds*   GetBoundsPtr( void ) {return g_PolyCache.GetBoundsPtr(*this);}
        // Hidden -------------------------------
    };

    struct cell
    {
        cell*       pHashNext;
        cell*       pMRUNext;
        cell*       pMRUPrev;
        s16         X,Y,Z;
        s16         iHash;
        s16         nClusters;
        cluster**   ppCluster;

        // Hidden ----------------------------------

        cluster* CLUSTERPTR[ POLYCACHE_MAX_CLUSTER_PTRS_PER_CELL ];

        #ifdef DEBUG_POLY_CACHE
        byte    nHits;
        byte    bBuiltThisFrame;
        #endif
    };

    struct cluster_32 : public cluster
    {
        vector3                     POINT   [32];
        vector3                     NORMAL  [32];
        quad                        QUAD    [32];
        bounds                      BOUNDS  [32];
    };

    struct cluster_16 : public cluster
    {
        vector3                     POINT   [16];
        vector3                     NORMAL  [16];
        quad                        QUAD    [16];
        bounds                      BOUNDS  [16];
    };

    struct cluster_8 : public cluster
    {
        vector3                     POINT   [8];
        vector3                     NORMAL  [8];
        quad                        QUAD    [8];
        bounds                      BOUNDS  [8];
    };

    struct cluster_bank
    {
        s32             nQuads;
        s32             nPoints;
        s32             nNormals;
        s32             nClusters;
        s32             nFree;
        s32             iBank;
        cluster*        pFirstFreeCluster;
        cluster*        pCluster;
        s32             ClusterSize;
        s32             PointOffset;
        s32             NormalOffset;
        s32             QuadOffset;
        s32             BoundsOffset;
    };


//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------
public:

                        poly_cache      ( void );
                       ~poly_cache      ( void );

        void            Init            ( s32 n8Clusters,
                                          s32 n16Clusters,
                                          s32 n32Clusters );
 
        void            Kill            ( void );
        void            Update          ( void );
        void            ClearStats      ( void );

        void            SanityCheck     ( void );
        void            Render          ( void );

#ifdef DEBUG_POLY_CACHE
        void            ResetStats      ( void );
        void            DisplayStats    ( void );
#endif // X_RETAIL

        cell*           AcquireCell     ( s32 X, s32 Y, s32 Z );
        void            InvalidateCell  ( s32 X, s32 Y, s32 Z );
        void            InvalidateCells ( const bbox& BBox, const guid& Guid );
        void            InvalidateAllCells( void );
        void            CacheCells      ( const bbox& BBox );

        void            GetCellBBox     ( s32 X, s32 Y, s32 Z, bbox& BBox );
        void            GetCellBBox     ( const cell& Cell, bbox& BBox );

        void            GetCellRegion   ( const bbox& BBox,
                                                s32&  MinX, s32 &MinY, s32& MinZ,
                                                s32&  MaxX, s32 &MaxY, s32& MaxZ );

        //
        // Used by objects when poly_cache is gathering triangles
        //
        const bbox&     GetGatherBBox       ( void );

        void            GatherCluster       ( const collision_data&   CollData,
                                              const matrix4*          pL2W,
                                              u64                     MeshMask,
                                              guid                    Guid );

//------------------------------------------------------------------------------

public:

        // This routine and members are for collecting all the clusters
        // intersected by a bbox
        void            BuildClusterList    ( const bbox&   BBox, 
                                              object::type  ThisType,
                                              u32           TheseAttributes,
                                              u32           NotTheseAttributes,
                                              const guid*   pIgnoreList = NULL,
                                              s32           nIgnores    = 0);


        cluster*        m_ClusterList[POLYCACHE_MAX_CLUSTERS_IN_LIST];
        s32             m_nClusters;

public:

        // These routines are for walking a ray through the polycache
        void            BeginRayClusterWalk ( const vector3& RayStart, 
                                              const vector3& RayEnd,
                                              object::type  ThisType,
                                              u32           TheseAttributes,
                                              u32           NotTheseAttributes,
                                              const guid*   pIgnoreList = NULL,
                                              s32           nIgnores    = 0);

        cluster*        GetNextClusterFromRayWalk( void );

private:
        xbool           StepRayClusterWalker( void );
        object::type    m_ThisType;
        u32             m_TheseAttributes;
        u32             m_NotTheseAttributes;
        const guid*     m_pIgnoreList;
        s32             m_nIgnores;
        grid_walker     m_GridWalker;
        s32             m_iNextCluster;
        vector3         m_RayStart;
        vector3         m_RayEnd;

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------
private:
        void            Clear           ( void );
        s32             ComputeHash     ( s32 X, s32 Y, s32 Z );

        cell*           LookupCell      ( s32 X, s32 Y, s32 Z );
        cell*           BuildNewCell    ( s32 X, s32 Y, s32 Z );
        void            DestroyCell     ( cell* pCell );

        void            GatherClustersIntoCell( cell& Cell, const bbox& CellBBox );

        void            InitClusters        ( void );
        cluster*        AllocCluster        ( s32 nPoints, s32 nNormals, s32 nQuads, guid Guid, s32 iCollDataCluster );
        cluster*        FindCluster         ( guid Guid, s32 iCollDataCluster );
        void            FreeCluster         ( cluster* pCluster );
        s32             ComputeClusterHash  ( guid Guid, s32 iCollDataCluster );
        void            DrawCluster         ( cluster* pCL, f32* Intensity );
        void            DrawClusterNormals  ( cluster* pCL );

        void            SanityCheckFreeClusterList( void );
        xbool           VerifyInFreeList( cluster* pCL );

        void            IncrementSequence   ( void );
        void            ClearSequence       ( void );

//------------------------------------------------------------------------------
//  Public Data
//------------------------------------------------------------------------------
public:

        cell            m_Cell[POLYCACHE_MAX_CELLS];

#ifdef DEBUG_POLY_CACHE
        debug           m_Debug;
#endif // X_RETAIL

//------------------------------------------------------------------------------
//  Private Data
//------------------------------------------------------------------------------
private:

        cluster_8*      m_Cluster_8;
        cluster_16*     m_Cluster_16;
        cluster_32*     m_Cluster_32;
        cluster**       m_ClusterHash;

        cluster_bank    m_ClusterBank[POLYCACHE_NUM_CLUSTER_BANKS];


        cell*           m_pMRU;
        cell*           m_pLRU;
        cell*           m_Hash[POLYCACHE_HASH_SIZE];

        bbox            m_GatherBBox;
        cell*           m_pGatherCell;
        xbool           m_bInsideGather;

        // Stats tracking
#ifdef DEBUG_POLY_CACHE
        s32             m_nCacheMisses;
        s32             m_nCacheHits;
        s32             m_nInvalidationMisses;
        s32             m_nInvalidationHits;
        f32             m_CacheMissTime;
        s32             m_nCellsDestroyed;
#endif // X_RETAIL

        cluster*        m_GatherClusterList[128];
        s32             m_nGatherClusters;
        s32             m_nGatherClustersAlreadyCached;
        u16             m_Sequence;

        s32             m_n8Clusters;
        s32             m_n16Clusters;
        s32             m_n32Clusters;
        s32             m_nSumClusters;
        s32             m_nClusterHashSize;
};

//==============================================================================

inline
s32 GetBoneIndexFromPolyCachePrimKey( u32 PrimKey ) { return PrimKey & 0xFF; }

//==============================================================================

inline
poly_cache::cluster* poly_cache::GetNextClusterFromRayWalk( void )
{
    if( m_iNextCluster < m_nClusters )
    {
        poly_cache::cluster* pCluster = m_ClusterList[ m_iNextCluster ];

        // Increment to next cluster
        m_iNextCluster++;

        // Return cluster address
        return pCluster;
    }
    else
    {
        // Be sure there are clusters in the cache
        while( m_iNextCluster == m_nClusters )
        {
            // Step ray through grid and collect clusters.  If we hit the end of the ray
            // then return NULL
            if( StepRayClusterWalker() == FALSE )
                return NULL;
        }

        m_iNextCluster = 1;
        return m_ClusterList[0];
    }
}

//==============================================================================

inline void poly_cache::IncrementSequence( void )
{
    if( m_Sequence < POLYCACHE_MAX_SEQUENCE ) 
    {
        m_Sequence++;
    }
    else
    {
        ClearSequence();
    }
}

//==============================================================================

inline s32 poly_cache::ComputeHash( s32 X, s32 Y, s32 Z )
{
    s32 H = ((u32)((((X<<10)+Y)<<10)+Z)) % POLYCACHE_HASH_SIZE;
    return H;
}

//==============================================================================

inline poly_cache::cell* poly_cache::LookupCell( s32 X, s32 Y, s32 Z )
{
    s32 H = ComputeHash(X,Y,Z);

    cell* pC = m_Hash[H];
    while( pC )
    {
        if( (pC->X == X) && (pC->Y == Y) && (pC->Z == Z) )
            break;

        pC = pC->pHashNext;
    }
    
    return pC;
}

//==============================================================================

extern poly_cache g_PolyCache;

//==============================================================================
#endif // POLYCACHE_HPP
//==============================================================================

