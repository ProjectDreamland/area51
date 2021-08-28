//==============================================================================
//
//  PolyCache.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "PolyCache.hpp"
#include "Entropy.hpp"
#include "CollisionMgr/CollisionMgr.hpp"
#include "Objects\Object.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "PlaySurfaceMgr\PlaySurfaceMgr.hpp"
#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"

//==============================================================================

#define POLYCACHE_CELL_SIZE             (400.0f)
#define POLYCACHE_ONE_OVER_CELL_SIZE    (1.0f / POLYCACHE_CELL_SIZE)

s32 g_n8Clusters  = POLYCACHE_MAX_8_CLUSTERS;
s32 g_n16Clusters = POLYCACHE_MAX_16_CLUSTERS;
s32 g_n32Clusters = POLYCACHE_MAX_32_CLUSTERS;

//==============================================================================

poly_cache g_PolyCache;
vector3 s_InvalidateMarker[128];
s32 s_nInvalidateMarkers=0;

#define DO_LOGGING (0)

guid POLYCACHE_INVALIDATE_GUID[32]={0};
s32  POLYCACHE_N_INVALIDATE_GUIDS = 0;

//==============================================================================
//==============================================================================
//==============================================================================
// POLY_CACHE
//==============================================================================
//==============================================================================
//==============================================================================

poly_cache::poly_cache( void )
{
    ASSERT( POLYCACHE_MAX_CELLS > 2 );

    // Initialize the cluster numbers.
    m_n8Clusters       = g_n8Clusters;
    m_n16Clusters      = g_n16Clusters;
    m_n32Clusters      = g_n32Clusters;
    m_nSumClusters     = m_n8Clusters + m_n16Clusters + m_n32Clusters;
    m_nClusterHashSize = m_nSumClusters*2+1;

    // Initialize the cluster buffers.
    m_Cluster_8   = NULL;
    m_Cluster_16  = NULL;
    m_Cluster_32  = NULL;
    m_ClusterHash = NULL;

    Init( m_n8Clusters, m_n16Clusters, m_n32Clusters );

#if !defined(X_RETAIL) || defined(X_QA)
    x_memset(&m_Debug,0,sizeof(m_Debug));
#endif // X_RETAIL
}

//==============================================================================

poly_cache::~poly_cache( void )
{
    Clear();
}

//==============================================================================

void poly_cache::Clear( void )
{
    s32 i;

    // Clear hash table
    for( i=0; i<POLYCACHE_HASH_SIZE; i++ )
        m_Hash[i] = NULL;

    // Clear cells
    for( i=0; i<POLYCACHE_MAX_CELLS; i++ )
    {
        m_Cell[i].pHashNext = NULL;
        m_Cell[i].iHash     = -1;
        m_Cell[i].nClusters = 0;
        m_Cell[i].ppCluster = m_Cell[i].CLUSTERPTR;
    }

    // Setup MRU chain
    m_pMRU = &m_Cell[0];
    m_pLRU = &m_Cell[0];
    for( i=1; i<POLYCACHE_MAX_CELLS; i++ )
    {
        m_Cell[i].pMRUNext  = NULL;
        m_Cell[i].pMRUPrev  = m_pLRU;            
        m_pLRU->pMRUNext = &m_Cell[i];
        m_pLRU = &m_Cell[i];
    }

#if !defined(X_RETAIL) || defined(X_QA)
    ResetStats();
#endif // X_RETAIL

    m_pGatherCell = NULL;
    m_bInsideGather = FALSE;

    // Clear cluster hash
    x_memset(m_ClusterHash,0,m_nClusterHashSize*sizeof(cluster*));

    InitClusters();

    m_Sequence = 1;
}

//==============================================================================

#if defined(DEBUG_POLY_CACHE)
void poly_cache::ResetStats( void )
{
    m_nCacheMisses = 0;
    m_nCacheHits = 0;
    m_nInvalidationMisses = 0;
    m_nInvalidationHits = 0;
    m_CacheMissTime = 0;
    m_nCellsDestroyed = 0;
}

//==============================================================================
void poly_cache::DisplayStats( void )
{
#if !defined(X_EDITOR)
    irect Rect;
    s32 XRes,YRes;

    eng_GetRes(XRes,YRes);

    s32 x = 350;
    s32 y = 20;
    s32 font = g_UiMgr->FindFont("small");

    Rect.Set( x-5, y, x + 150, y + (g_UiMgr->GetLineHeight(font) * 4) );
    draw_Rect( Rect, xcolor(0,0,0,128), FALSE );

    xwstring Text1 = (const char*)xfs( "Cache Hit: %d", g_PolyCache.m_nCacheHits ); 
    g_UiMgr->TextSize( font, Rect, Text1, Text1.GetLength());
    Rect.Translate(x, y);
    g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text1 );
    y += g_UiMgr->GetLineHeight(font);
  
    xwstring Text2 = (const char*)xfs( "Cache Miss: %d", g_PolyCache.m_nCacheMisses ); 
    g_UiMgr->TextSize( font, Rect, Text2, Text2.GetLength());
    Rect.Translate(x, y);
    g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text2 );
    y += g_UiMgr->GetLineHeight(font);

    xwstring Text3 = (const char*)xfs( "Invalid Hit: %d", g_PolyCache.m_nInvalidationHits ); 
    g_UiMgr->TextSize( font, Rect, Text3, Text3.GetLength());
    Rect.Translate(x, y);
    g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text3 );
    y += g_UiMgr->GetLineHeight(font);

    xwstring Text4 = (const char*)xfs( "Invalid Miss: %d", g_PolyCache.m_nInvalidationMisses ); 
    g_UiMgr->TextSize( font, Rect, Text4, Text4.GetLength());
    Rect.Translate(x, y);
    g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text4 );
    y += g_UiMgr->GetLineHeight(font);

    xwstring Text5 = (const char*)xfs( "(Miss-Invalid): %d", g_PolyCache.m_nCacheMisses-g_PolyCache.m_nInvalidationHits ); 
    g_UiMgr->TextSize( font, Rect, Text5, Text5.GetLength());
    Rect.Translate(x, y);
    g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text5 );
    y += g_UiMgr->GetLineHeight(font);

    ResetStats();
#endif // editor
}
#endif // DEBUG_POLY_CACHE

//==============================================================================

void poly_cache::Update( void )
{
    xbool bInit = (( m_n8Clusters != g_n8Clusters ) || 
                   (m_n16Clusters != g_n16Clusters) || 
                   (m_n32Clusters != g_n32Clusters));
    if( bInit )
        Init( g_n8Clusters, g_n16Clusters, g_n32Clusters );
}

//==============================================================================

void poly_cache::Init( s32 n8Clusters, s32 n16Clusters, s32 n32Clusters )
{
    // Has something changed?
    xbool bClear = (( m_n8Clusters != n8Clusters ) || 
                    (m_n16Clusters != n16Clusters) || 
                    (m_n32Clusters != n32Clusters));

    // Only if there was a change...
    if( bClear )
    {
        // Change 8 cluster?
        if( m_Cluster_8 && ( m_n8Clusters != n8Clusters ) )
        {
            m_n8Clusters = n8Clusters;
            x_free( m_Cluster_8 );
            m_Cluster_8 = NULL;
        }

        // Change 16 cluster?
        if( m_Cluster_16 && ( m_n16Clusters != n16Clusters ) )
        {
            m_n16Clusters = n16Clusters;
            x_free( m_Cluster_16 );
            m_Cluster_16 = NULL;
        }

        // Change 32 cluster?
        if( m_Cluster_32 && (m_n32Clusters != n32Clusters) )
        {
            m_n32Clusters = n32Clusters;
            x_free( m_Cluster_32 );
            m_Cluster_32 = NULL;
        }

        // Always nuke the hash table.
        if( m_ClusterHash )
        {
            x_free( m_ClusterHash );
            m_ClusterHash = NULL;
        }

        // Always recalculate these if something changed...
        m_nSumClusters     = m_n8Clusters + m_n16Clusters + m_n32Clusters;
        m_nClusterHashSize = m_nSumClusters*2+1;
    }

    // Allocate memory.
    if( m_Cluster_8 == NULL )
        m_Cluster_8 = (cluster_8*)x_malloc( m_n8Clusters*sizeof(cluster_8 ) );
    if( m_Cluster_16 == NULL )
        m_Cluster_16 = (cluster_16*)x_malloc( m_n16Clusters*sizeof(cluster_16) );
    if( m_Cluster_32 == NULL )
        m_Cluster_32 = (cluster_32*)x_malloc( m_n32Clusters*sizeof(cluster_32) );
    if( m_ClusterHash == NULL )
        m_ClusterHash = (cluster**)x_malloc( m_nClusterHashSize*sizeof(cluster*) );

    Clear();
}

//==============================================================================

void poly_cache::Kill( void )
{
    Clear();
}


//==============================================================================

xbool poly_cache::VerifyInFreeList( cluster* pCL )
{
    s32 iB = pCL->iBank;
    cluster_bank& B = m_ClusterBank[iB];
    cluster* pC = B.pFirstFreeCluster;

    // Loop through free list looking for pCL
    while( pC != NULL ) 
    {
        if( pC == pCL ) 
            return TRUE;

        pC = pC->pNext;
    }

    return FALSE;
}

//==============================================================================

#ifndef X_RETAIL
void poly_cache::SanityCheckFreeClusterList( void )
{
    return;

    // Loop through banks
    for( s32 iB=0; iB<POLYCACHE_NUM_CLUSTER_BANKS; iB++ )
    {
        cluster_bank& B = m_ClusterBank[iB];

        // Confirm all free clusters have no references
        {
            cluster* pPrevCL = NULL;
            cluster* pCL = B.pFirstFreeCluster;

            // Loop through free list looking for CL
            s32 nInFreeList=0;
            while( pCL != NULL ) 
            {
                nInFreeList++;
                ASSERT( pCL->nReferences == 0 );
                pPrevCL = pCL;
                pCL = pCL->pNext;
            }

            ASSERT( nInFreeList == B.nFree );
        }
    }

}
#endif // X_RETAIL

//==============================================================================

#ifndef X_RETAIL
void poly_cache::SanityCheck( void )
{

    //return;
//    s32 i,j;
    ASSERT( m_pMRU != m_pLRU );
/*
    // Check for duplicates of cells
    {
        i=j=0;
        cell* pA = m_pMRU;
        while( pA )
        {
            if( pA->iHash != -1 )
            {
                cell* pB = pA->pMRUNext;
                while( pB )
                {
                    if( pB->iHash != -1 )
                    {
                        ASSERT( (pB->X != pA->X) ||
                                (pB->Y != pA->Y) ||
                                (pB->Z != pA->Z) );
                    }
                    pB = pB->pMRUNext;
                }
            }

            pA = pA->pMRUNext;
        }
    }
*/    

    /*
    // Loop through banks
    for( s32 iB=0; iB<POLYCACHE_NUM_CLUSTER_BANKS; iB++ )
    {
        cluster_bank& B = m_ClusterBank[iB];

        // Confirm there are not duplicate clusters
        {
            // Loop through all the clusters
            for( i=0; i<B.nClusters; i++ )
            {
                cluster& CLA = *((cluster*)(((byte*)(B.pCluster)) + (i*B.ClusterSize)));
                if( CLA.nReferences==0 ) continue;

                for( j=i+1; j<B.nClusters; j++ )
                {
                    cluster& CLB = *((cluster*)(((byte*)(B.pCluster)) + (j*B.ClusterSize)));
                    if( CLB.nReferences==0 ) continue;
                    ASSERT( (CLA.Guid != CLB.Guid) || (CLA.iCollDataCluster != CLB.iCollDataCluster) );
                }
            }
        }

        // Confirm all free clusters have no references
        {
            cluster* pCL = B.pFirstFreeCluster;

            // Loop through free list looking for CL
            s32 nInFreeList=0;
            while( pCL != NULL ) 
            {
                nInFreeList++;
                ASSERT( pCL->nReferences == 0 );
                pCL = pCL->pNext;
            }

            ASSERT( nInFreeList == B.nFree );
        }

        // Loop through all the clusters
        for( i=0; i<B.nClusters; i++ )
        {
            // Remember the cluster structures are different sizes so we need to
            // some manual array math to get to the head of the cluster
            cluster& CL = *((cluster*)(((byte*)(B.pCluster)) + (i*B.ClusterSize)));

            // Is this cluster referenced?
            if( CL.nReferences > 0 )
            {

                // Yes!  Loop through cells and confirm there is a cell that references it.
                s32 nTrueReferences=0;
                cell* pCell = m_pMRU;
                while( pCell )
                {
                    // Is this cell in use?
                    {
                        for( j=0; j<pCell->nClusters; j++ )
                        if( pCell->ppCluster[j] == &CL )
                            nTrueReferences++;
                    }

                    pCell = pCell->pMRUNext;
                }

                // Be sure the cluster knows how many references it really has!
                ASSERT( nTrueReferences == CL.nReferences );
            }
            else
            {
                // No!  Be sure it is in the free list
                cluster* pCL = B.pFirstFreeCluster;

                // Loop through free list looking for CL
                while( (pCL != NULL) && (pCL != &CL) ) 
                {
                    pCL = pCL->pNext;
                }
                ASSERT( pCL == &CL );

                // Loop through the cells and be sure it is not referenced
                s32 nTrueReferences=0;
                cell* pCell = m_pMRU;
                while( pCell )
                {
                    // Is this cell in use?
                    //if( pCell->iHash!=-1 )
                    {
                        for( j=0; j<pCell->nClusters; j++ )
                        if( pCell->ppCluster[j] == &CL )
                            nTrueReferences++;
                    }

                    pCell = pCell->pMRUNext;
                }

                ASSERT( nTrueReferences == 0 );
            }
        }
    }
    */
}
#endif // X_RETAIL

//==============================================================================

#ifndef CONFIG_RETAIL
void poly_cache::DrawCluster( cluster* pCL, f32* Intensity )
{

    random R;
    R.srand( ((u32)(pCL)) & 0x0000FFFF );
    xcolor CC = xcolor( R.irand( 128, 255 ), R.irand( 128, 255 ), R.irand( 128, 255 ), 255 );
    s32 iC=0;

    for( s32 i=0; i<(s32)pCL->nQuads; i++ )
    {
        cluster::quad& QD = pCL->pQuad[i];

        // Set color
        {
            xcolor C;
            f32    I = Intensity[(iC++)%128];
            C.R = (u8)( CC.R * I );
            C.G = (u8)( CC.G * I );
            C.B = (u8)( CC.B * I );
            C.A = 255;
            draw_Color( C );
        }

        vector3* P0 = &pCL->pPoint[ QD.iP[0] ];
        vector3* P1 = &pCL->pPoint[ QD.iP[1] ];
        vector3* P2 = &pCL->pPoint[ QD.iP[2] ];
        vector3* P3 = &pCL->pPoint[ QD.iP[3] ];

        draw_Vertex( *P0 );
        draw_Vertex( *P1 );
        draw_Vertex( *P2 );

        if( pCL->pBounds[i].Flags & BOUNDS_IS_QUAD )
        {
/*
            // Set color
            {
                xcolor C;
                f32    I = Intensity[(iC++)%128];
                C.R = (u8)( CC.R * I );
                C.G = (u8)( CC.G * I );
                C.B = (u8)( CC.B * I );
                C.A = 255;
                draw_Color( C );
            }
*/
            draw_Vertex( *P0 );
            draw_Vertex( *P2 );
            draw_Vertex( *P3 );
        }
    }

}
#endif // X_RETAIL

//==============================================================================

#ifndef CONFIG_RETAIL
void poly_cache::DrawClusterNormals( cluster* pCL )
{

    draw_Color( XCOLOR_RED );

    for( s32 i=0; i<(s32)pCL->nQuads; i++ )
    {
        cluster::quad& QD = pCL->pQuad[i];

        vector3* P0 = (vector3*)(&pCL->pPoint[ QD.iP[0] ]);
        vector3* P1 = (vector3*)(&pCL->pPoint[ QD.iP[1] ]);
        vector3* P2 = (vector3*)(&pCL->pPoint[ QD.iP[2] ]);
        vector3* P3 = (vector3*)(&pCL->pPoint[ QD.iP[3] ]);
/*
        vector3 Center = (*P0+*P1+*P2+*P3)*0.25f;
        plane Plane(*P0,*P1,*P2);
        vector3 Tip = Center + (Plane.Normal * 10.0f); 
        draw_Vertex( Center );
        draw_Vertex( Tip );
*/
        {
            vector3 Center = (*P0+*P1+*P2)*0.333f;
            plane Plane(*P0,*P1,*P2);
            vector3 Tip = Center + (Plane.Normal * 10.0f); 
            draw_Color(XCOLOR_RED);
            draw_Vertex( Center );
            draw_Vertex( Tip );
        }
        {
            vector3 Center = (*P0+*P2+*P3)*0.333f;
            plane Plane(*P0,*P2,*P3);
            vector3 Tip = Center + (Plane.Normal * 10.0f); 
            draw_Color(XCOLOR_GREEN);
            draw_Vertex( Center );
            draw_Vertex( Tip );
        }
    }
}
#endif // X_RETAIL

//==============================================================================
#ifdef DEBUG_POLY_CACHE
xbool DUMP_CELLS = FALSE;

void poly_cache::Render( void )
{
    if( m_Debug.FORCE_REBUILD )
    {
        InvalidateAllCells();
    }

    if( DUMP_CELLS )
    {
        DUMP_CELLS = 0;
        X_FILE* fp = x_fopen("c:/temp/cells.txt","wt");
        if( fp )
        {
            s32 I=0;
            cell* pCell = m_pMRU;
            while( pCell )
            {
                x_fprintf(fp,"[%4d] %4d (%4d,%4d,%4d) %4d\n",
                    I,pCell->iHash,pCell->X,pCell->Y,pCell->Z,pCell->nClusters);
                I++;
                pCell = pCell->pMRUNext;
            }

            x_fclose(fp);   
        }
    }

    if( m_Debug.RENDER==FALSE )
        return;

    s32 nCellsHit=0;
    s32 nClustersHit=0;
    s32 n8ClustersHit=0;
    s32 n16ClustersHit=0;
    s32 n32ClustersHit=0;
    s32 n8ClustersRefd=0;
    s32 n16ClustersRefd=0;
    s32 n32ClustersRefd=0;

    draw_ClearL2W();

    // Setup some random intensities
    f32 Intensity[128];
    {
        random R;
        for( s32 i=0; i<128; i++ )
            Intensity[i] = R.frand(0.5f,1.0f);
    }

    // Draw clusters
    if( m_Debug.RENDER_HIT_CLUSTERS )
    {
        draw_Begin( DRAW_TRIANGLES );
        draw_ClearL2W();
        draw_SetZBias( 3 );

        s32 iCL;

        for( iCL=0; iCL<POLYCACHE_MAX_8_CLUSTERS; iCL++ )
        {
            if( m_Cluster_8[iCL].nHits )
                DrawCluster( (cluster*)(&m_Cluster_8[iCL]), Intensity );
        }

        for( iCL=0; iCL<POLYCACHE_MAX_16_CLUSTERS; iCL++ )
        {
            if( m_Cluster_16[iCL].nHits )
                DrawCluster( (cluster*)(&m_Cluster_16[iCL]), Intensity );
        }

        for( iCL=0; iCL<POLYCACHE_MAX_32_CLUSTERS; iCL++ )
        {
            if( m_Cluster_32[iCL].nHits )
                DrawCluster( (cluster*)(&m_Cluster_32[iCL]), Intensity );
        }

        draw_End();
        draw_SetZBias( 0 );
    }

    // Draw cluster normals
    if( m_Debug.RENDER_HIT_CLUSTERS_NORMALS )
    {
        draw_Begin( DRAW_LINES );
        draw_ClearL2W();
        draw_SetZBias( 3 );

        s32 iCL;

        for( iCL=0; iCL<POLYCACHE_MAX_8_CLUSTERS; iCL++ )
        {
            if( m_Cluster_8[iCL].nHits )
                DrawClusterNormals( (cluster*)(&m_Cluster_8[iCL]) );
        }

        for( iCL=0; iCL<POLYCACHE_MAX_16_CLUSTERS; iCL++ )
        {
            if( m_Cluster_16[iCL].nHits )
                DrawClusterNormals( (cluster*)(&m_Cluster_16[iCL]) );
        }

        for( iCL=0; iCL<POLYCACHE_MAX_32_CLUSTERS; iCL++ )
        {
            if( m_Cluster_32[iCL].nHits )
                DrawClusterNormals( (cluster*)(&m_Cluster_32[iCL]) );
        }

        draw_End();
        draw_SetZBias( 0 );
    }
    

    if( m_Debug.RENDER_CLUSTER_BOXES )
    {
        draw_ClearL2W();

        s32 iCL;

        for( iCL=0; iCL<POLYCACHE_MAX_8_CLUSTERS; iCL++ )
        {
            if( m_Cluster_8[iCL].nHits )
                draw_BBox( m_Cluster_8[iCL].BBox, XCOLOR_RED );
        }

        for( iCL=0; iCL<POLYCACHE_MAX_16_CLUSTERS; iCL++ )
        {
            if( m_Cluster_16[iCL].nHits )
                draw_BBox( m_Cluster_16[iCL].BBox, XCOLOR_RED );
        }

        for( iCL=0; iCL<POLYCACHE_MAX_32_CLUSTERS; iCL++ )
        {
            if( m_Cluster_32[iCL].nHits )
                draw_BBox( m_Cluster_32[iCL].BBox, XCOLOR_RED );
        }
    }

    // Count number hit and Clear nHits
    {
        s32 iCL;

        for( iCL=0; iCL<POLYCACHE_MAX_8_CLUSTERS; iCL++ )
        if( m_Cluster_8[iCL].nHits )
        {
            nClustersHit++;
            n8ClustersHit++;
            m_Cluster_8[iCL].nHits = 0;
        }

        for( iCL=0; iCL<POLYCACHE_MAX_16_CLUSTERS; iCL++ )
        if( m_Cluster_16[iCL].nHits )
        {
            nClustersHit++;
            n16ClustersHit++;
            m_Cluster_16[iCL].nHits = 0;
        }

        for( iCL=0; iCL<POLYCACHE_MAX_32_CLUSTERS; iCL++ )
        if( m_Cluster_32[iCL].nHits )
        {
            nClustersHit++;
            n32ClustersHit++;
            m_Cluster_32[iCL].nHits = 0;
        }

        for( iCL=0; iCL<POLYCACHE_MAX_8_CLUSTERS; iCL++ )
            n8ClustersRefd += (m_Cluster_8[iCL].nReferences>0) ? (1):(0);

        for( iCL=0; iCL<POLYCACHE_MAX_16_CLUSTERS; iCL++ )
            n16ClustersRefd += (m_Cluster_16[iCL].nReferences>0) ? (1):(0);

        for( iCL=0; iCL<POLYCACHE_MAX_32_CLUSTERS; iCL++ )
            n32ClustersRefd += (m_Cluster_32[iCL].nReferences>0) ? (1):(0);
    }


    // Draw Cell bboxes
    draw_ClearL2W();

    cell* pCell = m_pMRU;
    while( pCell )
    {
        random R;
        R.srand( ((u32)(pCell)) & 0x0000FFFF );

        if( pCell->iHash!=-1 )
        {
            if( pCell->nHits>0 )
                nCellsHit++;

            if( m_Debug.RENDER_NEW_CELL_BOXES && pCell->bBuiltThisFrame )
            {
                bbox BBox;
                GetCellBBox( *pCell, BBox );
                draw_BBox(BBox,XCOLOR_RED);
                draw_Marker(BBox.GetCenter(),R.color());
            }

            if( m_Debug.RENDER_HIT_CELL_BOXES && (pCell->nHits>0))
            {
                bbox BBox;
                GetCellBBox( *pCell, BBox );
                draw_BBox(BBox,XCOLOR_BLUE);
            }

            if( m_Debug.RENDER_ALL_CELL_BOXES )
            {
                bbox BBox;
                GetCellBBox( *pCell, BBox );
                draw_BBox(BBox,XCOLOR_BLUE);
            }
        }

        pCell = pCell->pMRUNext;
    }

    {
        for( s32 i=0; i<POLYCACHE_N_INVALIDATE_GUIDS; i++ )
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( POLYCACHE_INVALIDATE_GUID[i] );
            if( pObj )
            {
                xcolor C;
                C.R = x_irand(128,255);
                C.G = x_irand(128,255);
                C.B = x_irand(128,255);
                C.A = 255;
                draw_Marker( pObj->GetBBox().GetCenter(), C );
            }
        }
        POLYCACHE_N_INVALIDATE_GUIDS = 0;
    }

    // Get Cell stats
    s32 nCellsCached = 0;
    s32 nCellsEmpty  = 0;
    s32 nCellsFree   = 0;
    s32 nCellsFull   = 0;
    {
        cell* pCell = m_pMRU;
        while( pCell )
        {
            if( pCell->iHash != -1 )
            {
                nCellsCached++;
                if( pCell->nClusters == 0 )
                    nCellsEmpty++;
                else
                    nCellsFull++;
            }
            else
            {
                nCellsFree++;
            }
            pCell = pCell->pMRUNext;
        }
    }

    if( m_Debug.RENDER_STATS )
    {
        s32 X=0;
        s32 Y=13;
        x_printfxy(X,Y++,"nCELHit %2d",nCellsHit);
        x_printfxy(X,Y++,"nC08_HR %3d %3d",n8ClustersHit ,n8ClustersRefd );
        x_printfxy(X,Y++,"nC16_HR %3d %3d",n16ClustersHit,n16ClustersRefd);
        x_printfxy(X,Y++,"nC32_HR %3d %3d",n32ClustersHit,n32ClustersRefd);
        x_printfxy(X,Y++,"nCMiss  %2d %5.3f",m_nCacheMisses,m_CacheMissTime);
        x_printfxy(X,Y++,"nCHits  %2d",m_nCacheHits);
        x_printfxy(X,Y++,"nCDestr %2d",m_nCellsDestroyed);
        x_printfxy(X,Y++,"nIVMiss %2d",m_nInvalidationMisses);
        x_printfxy(X,Y++,"nIVHits %2d",m_nInvalidationHits);
        x_printfxy(X,Y++,"nCells  %3d %3d %3d",nCellsCached,nCellsFull,nCellsEmpty);
        x_printfxy(X,Y++,"Size    %2d",sizeof(poly_cache));
    }

    g_PolyCache.ResetStats();

    // Clear Used value
    for( s32 i=0; i<POLYCACHE_MAX_CELLS; i++ )
    {
        m_Cell[i].nHits = 0;
        m_Cell[i].bBuiltThisFrame = FALSE;
    }
}
#endif // X_RETAIL

//==============================================================================

poly_cache::cell* poly_cache::AcquireCell     ( s32 X, s32 Y, s32 Z )
{
    CONTEXT("poly_cache::AcquireCell");

    cell* pCell = LookupCell(X,Y,Z);
    
    if( pCell )
    {
#if !defined(X_RETAIL) || defined(X_QA)
        if( pCell->nHits < 255 )
            pCell->nHits++;

        m_nCacheHits++;
#endif // X_RETAIL

        // Move to front of MRU if not already
        if( m_pMRU != pCell )
        {
            // Move cell to front of MRU
            // Remove from MRU list
            if( pCell->pMRUNext ) pCell->pMRUNext->pMRUPrev = pCell->pMRUPrev;
            else                  m_pLRU = pCell->pMRUPrev;

            if( pCell->pMRUPrev ) pCell->pMRUPrev->pMRUNext = pCell->pMRUNext;
            else                  m_pMRU = pCell->pMRUNext;

            // Add to MRU end
            m_pMRU->pMRUPrev = pCell;
            pCell->pMRUPrev  = NULL;
            pCell->pMRUNext  = m_pMRU;
            m_pMRU           = pCell;
        }
    }
    else
    {
#if !defined(X_RETAIL) || defined(X_QA)
        m_nCacheMisses++;
#endif // X_RETAIL
        pCell = BuildNewCell( X, Y, Z );
    }

    // Mark clusters as hit
#if !defined(X_RETAIL) || defined(X_QA)
    {
        for( s32 i=0; i<pCell->nClusters; i++ )
        {
            if( pCell->ppCluster[i]->nHits < 255 )
                pCell->ppCluster[i]->nHits++;
        }
    }
#endif

    return pCell;
}

//==============================================================================

void poly_cache::InvalidateCell  ( s32 X, s32 Y, s32 Z )
{
    CONTEXT("poly_cache::InvalidateCell");

    cell* pCell = LookupCell(X,Y,Z);

    if( pCell )
    {
        DestroyCell(pCell);
#if !defined(X_RETAIL) || defined(X_QA)
        m_nInvalidationHits++;
#endif // X_RETAIL
    }
    else
    {
#if !defined(X_RETAIL) || defined(X_QA)
        m_nInvalidationMisses++;
#endif // X_RETAIL
    }
}

//==============================================================================

void poly_cache::InvalidateCells ( const bbox& BBox, const guid& Guid )
{
    (void)Guid;
    CONTEXT("poly_cache::InvalidateCells");

    bbox IBBox = BBox;
    IBBox.Inflate(1,1,1);

    s32 MinX,MinY,MinZ;
    s32 MaxX,MaxY,MaxZ;

    GetCellRegion( IBBox, MinX, MinY, MinZ, MaxX, MaxY, MaxZ );

    s32 nCells = (MaxX-MinX)*(MaxY-MinY)*(MaxZ-MinZ);


    if( POLYCACHE_N_INVALIDATE_GUIDS < 32 )
    {
        POLYCACHE_INVALIDATE_GUID[POLYCACHE_N_INVALIDATE_GUIDS++] = Guid;
    }

    if( nCells > 1000 )
    {
        CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","InvalidateCells (%d)",nCells);

        // Run through list looking for cells in range!
        cell* pCell = m_pLRU;
        while( pCell )
        {
            cell* pNextCell = pCell->pMRUPrev;
            if( (pCell->iHash != -1) &&
                (pCell->X >= MinX) && (pCell->X <= MaxX) &&
                (pCell->Y >= MinY) && (pCell->Y <= MaxY) &&
                (pCell->Z >= MinZ) && (pCell->Z <= MaxZ) )
            {
                DestroyCell( pCell );
            }
            pCell = pNextCell;
        }

        return;
    }
    else
    {
        for( s32 X=MinX; X<=MaxX; X++ )
        for( s32 Y=MinY; Y<=MaxY; Y++ )
        for( s32 Z=MinZ; Z<=MaxZ; Z++ )
        {
            cell* pCell = LookupCell(X,Y,Z);

            if( pCell )
            {
                DestroyCell(pCell);
#if !defined(X_RETAIL) || defined(X_QA)
                m_nInvalidationHits++;
#endif // X_RETAIL

                //if( s_nInvalidateMarkers<128 )
                //    s_InvalidateMarker[s_nInvalidateMarkers++] = BBox.GetCenter();
            }
            else
            {
#if !defined(X_RETAIL) || defined(X_QA)
                m_nInvalidationMisses++;
#endif // X_RETAIL
            }
        }
    }
}

//==============================================================================

void poly_cache::CacheCells( const bbox& BBox )
{
    CONTEXT("poly_cache::InvalidateCells");

    bbox IBBox = BBox;
    IBBox.Inflate(1,1,1);

    s32 MinX,MinY,MinZ;
    s32 MaxX,MaxY,MaxZ;

    GetCellRegion( IBBox, MinX, MinY, MinZ, MaxX, MaxY, MaxZ );

    for( s32 X=MinX; X<=MaxX; X++ )
    for( s32 Y=MinY; Y<=MaxY; Y++ )
    for( s32 Z=MinZ; Z<=MaxZ; Z++ )
    {
        AcquireCell(X,Y,Z);
    }
}

//==============================================================================

void poly_cache::GetCellBBox( s32 X, s32 Y, s32 Z, bbox& BBox )
{
    BBox.Min.Set( (f32)X, (f32)Y, (f32)Z );
    BBox.Min *= POLYCACHE_CELL_SIZE;
    BBox.Max = BBox.Min;
    BBox.Max += vector3(POLYCACHE_CELL_SIZE, POLYCACHE_CELL_SIZE, POLYCACHE_CELL_SIZE );
}

//==============================================================================

void poly_cache::GetCellBBox( const cell& Cell, bbox& BBox )
{
    GetCellBBox( Cell.X, Cell.Y, Cell.Z, BBox );
}

//==============================================================================

void poly_cache::DestroyCell( cell* pCell )
{
    CONTEXT("poly_cache::DestroyCell");

#ifndef X_RETAIL
#if DO_LOGGING
    {
        CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","--------------- DESTROY CELL ---------------");
        CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","DestroyCell %08X %d",(u32)pCell,pCell->nClusters);

        {
            for( s32 iC=0; iC<pCell->nClusters; iC++ )
            {
                cluster* pCL = pCell->ppCluster[iC];
                CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","cluster in cell %08X %d",(u32)pCL,pCL->nReferences);
            }
        }
    }
#endif
#endif

#if !defined(X_RETAIL) || defined(X_QA)
    m_nCellsDestroyed++;
#endif // X_RETAIL


    ASSERT(pCell);
    ASSERT((pCell>=m_Cell) && (pCell<&m_Cell[POLYCACHE_MAX_CELLS]));
    ASSERT(pCell->iHash != -1);
    ASSERT(pCell != m_pGatherCell);

    //
    // Free the clusters
    //
    {
        // Check clusters for stomp
        #ifdef TARGET_DEV
        {
            for( s32 i=0; i<pCell->nClusters; i++ )
            {
                ASSERT( ((u32)pCell->ppCluster[i]) > 1024 );
                if( ((u32)pCell->ppCluster[i]) < 1024 )
                {
                    BREAK;
                }
            }
        }
        #endif
    
        for( s32 iC=0; iC<pCell->nClusters; iC++ )
        {
            cluster* pCL = pCell->ppCluster[iC];
            ASSERT(pCL);

            // Be sure cluster is in the hash
            #ifdef X_ASSERT
            {
                s32 iHash = ComputeClusterHash( pCL->Guid, pCL->iCollDataCluster );
                cluster* pC = m_ClusterHash[iHash];
                while( pC )
                {
                    if( pC==pCL ) break;
                    pC = pC->pNext;
                }
                ASSERT( pC ); // Could not find cell in hash table!?!
            }
            #endif

            pCL->nReferences--;
            if( pCL->nReferences==0 )
                FreeCluster( pCell->ppCluster[iC] );

            pCell->ppCluster[iC] = NULL;
        }

        // Free cluster ptr array if not internal
        if( pCell->ppCluster != pCell->CLUSTERPTR )
        {
            x_free(pCell->ppCluster);
            pCell->ppCluster = pCell->CLUSTERPTR;
        }
    }

    // Find head of this chain and find previous cell in hash chain
    // for easy removal
    cell* pPrevC = NULL;
    cell* pC = m_Hash[pCell->iHash];
    while( pC )
    {
        if( pC==pCell ) break;
        pPrevC = pC;
        pC = pC->pHashNext;
    }
    ASSERT( pC ); // Could not find chain in hash table!?!

    // Remove pCell from hash table
    if( pPrevC )
        pPrevC->pHashNext = pCell->pHashNext;
    else
        m_Hash[pCell->iHash] = pCell->pHashNext;

    // Remove from MRU list
    if( pCell->pMRUNext ) pCell->pMRUNext->pMRUPrev = pCell->pMRUPrev;
    else                  m_pLRU = pCell->pMRUPrev;

    if( pCell->pMRUPrev ) pCell->pMRUPrev->pMRUNext = pCell->pMRUNext;
    else                  m_pMRU = pCell->pMRUNext;

    // Add to LRU end
    m_pLRU->pMRUNext = pCell;
    pCell->pMRUPrev  = m_pLRU;
    pCell->pMRUNext  = NULL;
    m_pLRU           = pCell;

    pCell->iHash = -1;
    pCell->pHashNext = NULL;
    pCell->X = 0;
    pCell->Y = 0;
    pCell->Z = 0;
    pCell->nClusters = 0;

    CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","--------------------------------------------");
}

//==============================================================================

void poly_cache::GetCellRegion( const bbox& aBBox,
                                        s32&  MinX, s32 &MinY, s32& MinZ,
                                        s32&  MaxX, s32 &MaxY, s32& MaxZ )
{

    // Clip bbox to objmgr limits
    bbox BBox  = aBBox;
    bbox LBBox = g_ObjMgr.GetSafeBBox();

    BBox.Min.Max( LBBox.Min );
    BBox.Max.Min( LBBox.Max );

    MinX = (s32)(( BBox.Min.GetX() * POLYCACHE_ONE_OVER_CELL_SIZE )+4096.0f)-4096;
    MinY = (s32)(( BBox.Min.GetY() * POLYCACHE_ONE_OVER_CELL_SIZE )+4096.0f)-4096;
    MinZ = (s32)(( BBox.Min.GetZ() * POLYCACHE_ONE_OVER_CELL_SIZE )+4096.0f)-4096;
    MaxX = (s32)(( BBox.Max.GetX() * POLYCACHE_ONE_OVER_CELL_SIZE )+4096.0f)-4096;
    MaxY = (s32)(( BBox.Max.GetY() * POLYCACHE_ONE_OVER_CELL_SIZE )+4096.0f)-4096;
    MaxZ = (s32)(( BBox.Max.GetZ() * POLYCACHE_ONE_OVER_CELL_SIZE )+4096.0f)-4096;
}

//==============================================================================

void poly_cache::InvalidateAllCells( void )
{
    CONTEXT("poly_cache::InvalidateAllCells");

    while( m_pMRU->iHash != -1 )
    {
        DestroyCell(m_pMRU);
    }
}

//==============================================================================

poly_cache::cell* poly_cache::BuildNewCell( s32 X, s32 Y, s32 Z )
{
    CONTEXT("poly_cache::BuildNewCell");

#ifndef X_RETAIL
    SanityCheck();

    xtimer Timer;
    Timer.Start();
#endif // X_RETAIL

    //
    // Allocate a new cell
    //
    if( m_pLRU->iHash != -1 )
    {
        // Look for a cell with the fewest number of clusters
        // on the LRU end of the list
        s32 I=16;
        cell* pCell = m_pLRU;
        cell* pBestCell = NULL;
        s32   BestScore = S32_MAX;
        while( pCell )
        {
            if( --I == 0 ) 
                break;

            if( pCell->nClusters < BestScore )
            {
                BestScore = pCell->nClusters;
                pBestCell = pCell;
                if( BestScore == 0 )
                    break;
            }
        }
        ASSERT( pBestCell );

        CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","Recycling Cell (nCL:%d)",pBestCell->nClusters);

        DestroyCell( pBestCell );
    }
    ASSERT( m_pLRU->iHash == -1 );

    //
    // Pull from the LRU end of the list
    //
    cell& C = *m_pLRU;
    m_pLRU  = C.pMRUPrev;
    ASSERT( m_pLRU );
    m_pLRU->pMRUNext = NULL;

    //
    // At this point the cell does not exist in the MRU/LRU list
    //


    //
    // Fill out basics
    //
    {
        C.iHash             = ComputeHash(X,Y,Z);;
        C.X                 = X;
        C.Y                 = Y;
        C.Z                 = Z;
        C.pHashNext         = m_Hash[C.iHash];
        m_Hash[C.iHash]     = &C;
        C.nClusters         = 0;
#ifdef DEBUG_POLY_CACHE
        C.bBuiltThisFrame   = TRUE;
#endif
    }

    //
    // Compute bbox containing cell and inflate for safety
    //
    bbox CellBBox;
    GetCellBBox(X,Y,Z,CellBBox);
    CellBBox.Inflate(1,1,1);

    //
    // Gather clusters into cell
    //
    GatherClustersIntoCell( C, CellBBox );

    //
    // Insert cell at head of MRU list
    //
    C.pMRUPrev = NULL;
    C.pMRUNext = m_pMRU;
    m_pMRU->pMRUPrev = &C;
    m_pMRU = &C;

    // Finish timing and return cell
#ifndef X_RETAIL
    Timer.Stop();
    m_CacheMissTime += Timer.ReadMs();

    SanityCheck();
#endif // X_RETAIL

    return &C;
}

//==============================================================================

void poly_cache::InitClusters( void )
{
    s32 i;



    //
    // Allocate the '8' bank
    //

    m_ClusterBank[0].nClusters  = POLYCACHE_MAX_8_CLUSTERS;
    m_ClusterBank[0].nPoints    = 8;
    m_ClusterBank[0].nNormals   = 8;
    m_ClusterBank[0].nQuads     = 8;
    m_ClusterBank[0].iBank      = 0;
    m_ClusterBank[0].nFree      = POLYCACHE_MAX_8_CLUSTERS;
    m_ClusterBank[0].pFirstFreeCluster = (cluster*)m_Cluster_8;
    m_ClusterBank[0].pCluster = (cluster*)m_Cluster_8;
    m_ClusterBank[0].ClusterSize = sizeof(cluster_8);
    for( i=0; i<POLYCACHE_MAX_8_CLUSTERS; i++ )
    {
        m_Cluster_8[i].pNext = (cluster*)(&m_Cluster_8[i+1]);
        m_Cluster_8[i].iBank = 0;
        m_Cluster_8[i].pPoint  = m_Cluster_8[i].POINT;
        m_Cluster_8[i].pNormal = m_Cluster_8[i].NORMAL;
        m_Cluster_8[i].pQuad   = m_Cluster_8[i].QUAD;
        m_Cluster_8[i].pBounds = m_Cluster_8[i].BOUNDS;
        m_Cluster_8[i].nReferences = 0;

        #ifdef TARGET_PS2
        ASSERT( (((u32)(m_Cluster_8[i].BOUNDS)) & 0xF)==0 );
        #endif
    }
    m_Cluster_8[POLYCACHE_MAX_8_CLUSTERS-1].pNext = NULL;

    //
    // Allocate the '16' bank
    //

    m_ClusterBank[1].nClusters  = POLYCACHE_MAX_16_CLUSTERS;
    m_ClusterBank[1].nPoints    = 16;
    m_ClusterBank[1].nNormals   = 16;
    m_ClusterBank[1].nQuads     = 16;
    m_ClusterBank[1].iBank      = 1;
    m_ClusterBank[1].nFree      = POLYCACHE_MAX_16_CLUSTERS;
    m_ClusterBank[1].pFirstFreeCluster = (cluster*)m_Cluster_16;
    m_ClusterBank[1].pCluster = (cluster*)m_Cluster_16;
    m_ClusterBank[1].ClusterSize = sizeof(cluster_16);
    for( i=0; i<POLYCACHE_MAX_16_CLUSTERS; i++ )
    {
        m_Cluster_16[i].pNext = (cluster*)(&m_Cluster_16[i+1]);
        m_Cluster_16[i].iBank = 1;
        m_Cluster_16[i].pPoint  = m_Cluster_16[i].POINT;
        m_Cluster_16[i].pNormal = m_Cluster_16[i].NORMAL;
        m_Cluster_16[i].pQuad   = m_Cluster_16[i].QUAD;
        m_Cluster_16[i].pBounds = m_Cluster_16[i].BOUNDS;
        m_Cluster_16[i].nReferences = 0;

        #ifdef TARGET_PS2
        ASSERT( (((u16)(m_Cluster_16[i].BOUNDS)) & 0xF)==0 );
        #endif
    }
    m_Cluster_16[POLYCACHE_MAX_16_CLUSTERS-1].pNext = NULL;

    //
    // Allocate the '32' bank
    //

    m_ClusterBank[2].nClusters  = POLYCACHE_MAX_32_CLUSTERS;
    m_ClusterBank[2].nPoints    = 32;
    m_ClusterBank[2].nNormals   = 32;
    m_ClusterBank[2].nQuads     = 32;
    m_ClusterBank[2].iBank      = 2;
    m_ClusterBank[2].nFree      = POLYCACHE_MAX_32_CLUSTERS;
    m_ClusterBank[2].pFirstFreeCluster = (cluster*)m_Cluster_32;
    m_ClusterBank[2].pCluster = (cluster*)m_Cluster_32;
    m_ClusterBank[2].ClusterSize = sizeof(cluster_32);
    for( i=0; i<POLYCACHE_MAX_32_CLUSTERS; i++ )
    {
        m_Cluster_32[i].pNext = (cluster*)(&m_Cluster_32[i+1]);
        m_Cluster_32[i].iBank = 2;
        m_Cluster_32[i].pPoint  = m_Cluster_32[i].POINT;
        m_Cluster_32[i].pNormal = m_Cluster_32[i].NORMAL;
        m_Cluster_32[i].pQuad   = m_Cluster_32[i].QUAD;
        m_Cluster_32[i].pBounds = m_Cluster_32[i].BOUNDS;
        m_Cluster_32[i].nReferences = 0;

        #ifdef TARGET_PS2
        ASSERT( (((u32)(m_Cluster_32[i].BOUNDS)) & 0xF)==0 );
        #endif
    }
    m_Cluster_32[POLYCACHE_MAX_32_CLUSTERS-1].pNext = NULL;
}

//==============================================================================

poly_cache::cluster* poly_cache::FindCluster( guid Guid, s32 iCollDataCluster )
{
    s32 iHash = ComputeClusterHash( Guid, iCollDataCluster );
    cluster* pCL = m_ClusterHash[iHash];
    while( pCL != NULL )
    {
        if( (pCL->Guid==Guid) && (pCL->iCollDataCluster==iCollDataCluster) )
        {
            ASSERT( pCL->iCollDataCluster < 1024 );
            return pCL;
        }

        pCL = pCL->pNext;
    }

    return NULL;
}

//==============================================================================

poly_cache::cluster* poly_cache::AllocCluster( s32 nPoints, 
                                               s32 nNormals, 
                                               s32 nQuads,
                                               guid Guid,
                                               s32  iCollDataCluster)
{
    // Find bank best suited to inputs
    cluster_bank* pCB = NULL;
    s32 i;
    for( i=0; i<POLYCACHE_NUM_CLUSTER_BANKS; i++ )
    {
        if( (m_ClusterBank[i].nPoints  >= nPoints  ) &&
            (m_ClusterBank[i].nNormals >= nNormals ) &&
            (m_ClusterBank[i].nQuads   >= nQuads   ) )
        {
            pCB = &m_ClusterBank[i];
            break;
        }
    }
    // There was no bank that could fit the request!!!
    ASSERT( pCB );

    // Check if there are any clusters available?
    cluster* pCL = NULL;
    while( 1 )
    {
        // Is there a cluster available?
        if( pCB->pFirstFreeCluster )
        {
            pCL = pCB->pFirstFreeCluster;
            ASSERT( pCL->nReferences==0 );
            pCB->pFirstFreeCluster = pCL->pNext;
            pCL->pNext = NULL;
            pCB->nFree--;
            break;
        }

        CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","Recycling Clusters. Needed Bank %d (%d,%d,%d)",i,nPoints,nNormals,nQuads);

        //
        // There was no cluster available so run up LRU and nuke
        // the first cell that has one!
        //
        s32     I=0;
        s32     FirstVictimI = -1;
        s32     BestScore = S32_MIN;
        cell*   pBestVictim = NULL;
        cell*   pVictim = m_pLRU;
        while( pVictim )
        {
            // If we've checked a number of cells past first victim then kill first victim
            if( pBestVictim && ((I-FirstVictimI) > 8) )
                break;

            ASSERT( pVictim != m_pGatherCell );

            s32 nCorrectComplete=0;
            s32 nIncorrectComplete=0;
            s32 nCorrectIncomplete=0;
            s32 nIncorrectIncomplete=0;

            //
            // We'll nuke the cell only if it has a cluster of
            // the bank type we are interested in.
            //
            s32 i;
            for( i=0; i<pVictim->nClusters; i++ )
            {
                cluster& CL = *pVictim->ppCluster[i];

                if( CL.nReferences==1 )
                {
                    if( (s32)CL.iBank == pCB->iBank )   nCorrectComplete++;
                    else                                nIncorrectComplete++;
                }
                else
                {
                    if( (s32)CL.iBank == pCB->iBank )   nCorrectIncomplete++;
                    else                                nIncorrectIncomplete++;
                }

            }

            // Compute score for this cell
            if( nCorrectComplete || nCorrectIncomplete )
            {
                if( pBestVictim==NULL )
                    FirstVictimI = I;

                s32 Score = nCorrectComplete        *(+1000)    +
                            nCorrectIncomplete      *(+100 )    +
                            nIncorrectComplete      *(-10  )    +
                            nIncorrectIncomplete    *(-1   );

                if( Score > BestScore )
                {
                    BestScore = Score;
                    pBestVictim = pVictim;
                }
            }

            // Remember best score

            // Go to next possible victim
            pVictim = pVictim->pMRUPrev;
            I++;
        }

        ASSERT( pBestVictim && (pBestVictim->iHash != -1) );

        //LOG_MESSAGE("POLYCACHE","BestVictim: %d\n",BestScore);
        DestroyCell( pBestVictim );

    }
    ASSERT( pCL );
    ASSERT( pCL->nReferences==0 );


    // Fill out basic info
    pCL->Guid               = Guid;
    pCL->iCollDataCluster   = iCollDataCluster;
    pCL->nNormals           = nNormals;
    pCL->nPoints            = nPoints;
    pCL->nQuads             = nQuads;
    pCL->nReferences        = 1;
    pCL->nHits              = 0;
    pCL->Sequence           = 0;


    //
    // Add cluster to hash
    //
    {
        s32 iHash = ComputeClusterHash( Guid, iCollDataCluster );
        pCL->pNext = m_ClusterHash[iHash];
        m_ClusterHash[iHash] = pCL;
    }

    CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","AllocCluster %08X %d (%08X:%08X,%d)",
        (u32)pCL,pCL->nReferences,
        (u32)(Guid>>32), (u32)Guid, iCollDataCluster);
    ASSERT( pCL->iCollDataCluster < 1024 );

    return pCL;
}

//==============================================================================

void poly_cache::FreeCluster( cluster* pCluster )
{

    CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","FreeCluster %08X %d",(u32)pCluster,pCluster->nReferences);

    //
    // Decrement references
    //
    ASSERT( pCluster->nReferences == 0 );

    //
    // Remove cluster from hash
    //
    {
        s32 iHash = ComputeClusterHash( pCluster->Guid, pCluster->iCollDataCluster );
        cluster* pPrevC = NULL;
        cluster* pC = m_ClusterHash[iHash];
        while( pC )
        {
            if( pC==pCluster ) break;
            pPrevC = pC;
            pC = pC->pNext;
        }
        ASSERT( pC ); // Could not find chain in hash table!?!

        // Remove pCell from hash table
        if( pPrevC )
            pPrevC->pNext = pC->pNext;
        else
        {
            ASSERT( m_ClusterHash[iHash] == pC );
            m_ClusterHash[iHash] = pC->pNext;
        }

        pC->pNext = NULL;

    }

    // Get ref to cluster's bank
    ASSERT( pCluster->iBank<POLYCACHE_NUM_CLUSTER_BANKS );
    cluster_bank& CB = m_ClusterBank[pCluster->iBank];

    // Put cluster bank into free list
    pCluster->pNext = CB.pFirstFreeCluster;
    CB.pFirstFreeCluster = pCluster;
    CB.nFree++;
    pCluster->nReferences = 0;
    pCluster->nHits = 0;
    pCluster->Guid = 0;
    pCluster->iCollDataCluster = 1024;
}

//==============================================================================

s32 poly_cache::ComputeClusterHash( guid Guid, s32 iCollDataCluster )
{
    //return ((u32)((/*(Guid>>32) ^*/ (Guid)) ^ iCollDataCluster)) % POLYCACHE_CLUSTER_HASH_SIZE;
    return ((u32)(((u32)(Guid>>16)) ^ iCollDataCluster)) % POLYCACHE_CLUSTER_HASH_SIZE;
}

//==============================================================================

void poly_cache::GatherClustersIntoCell( cell& Cell, const bbox& CellBBox )
{
    CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","**************** GATHER CLUSTERS ******************");

    // Be sure there isn't a nested gather
    ASSERT( m_pGatherCell == NULL );


    //
    // Setup necessary structures to start accepting Apply's
    //
    m_GatherBBox = CellBBox;
    m_pGatherCell = &Cell;
    m_nGatherClusters = 0;
    m_nGatherClustersAlreadyCached = 0;

    //
    // Run through playsurfaces                                                                                        
    //
    if( 1 )
    {
        g_PlaySurfaceMgr.CollectSurfaces( m_GatherBBox, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );
        playsurface_mgr::surface* pSurface = g_PlaySurfaceMgr.GetNextSurface();
        while ( pSurface != NULL )
        {
            playsurface_mgr::surface* pS = pSurface;
            guid SurfaceGuid = g_PlaySurfaceMgr.GetPlaySurfaceGuid();
            pSurface = g_PlaySurfaceMgr.GetNextSurface();

            rigid_geom* pGeom = (rigid_geom*)render::GetGeom( pS->RenderInst );

            RigidGeom_GatherToPolyCache( SurfaceGuid, pS->WorldBBox, (u64)-1, &pS->L2W, pGeom );
        }
    }

    //
    // Run through other types of objects derived from play_surface
    //
    if( 1 )
    {
        g_ObjMgr.SelectBBox(object::ATTR_COLLIDABLE, m_GatherBBox,object::TYPE_ALL_TYPES,object::ATTR_COLLISION_PERMEABLE );

        for( slot_id aID = g_ObjMgr.StartLoop(); aID != SLOT_NULL; aID = g_ObjMgr.GetNextResult(aID) )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot(aID);
            if( pObject )
                pObject->OnPolyCacheGather();
        }

        g_ObjMgr.EndLoop();
    }


    //
    // Copy list of clusters into cell
    //
    m_pGatherCell->nClusters = m_nGatherClusters;
    if( m_nGatherClusters > POLYCACHE_MAX_CLUSTER_PTRS_PER_CELL )
    {
        //LOG_MESSAGE("POLYCACHE","Allocating cell cluster array %d %d",POLYCACHE_MAX_CLUSTER_PTRS_PER_CELL,m_nGatherClusters);
        m_pGatherCell->ppCluster = (cluster**)x_malloc(sizeof(cluster*)*m_nGatherClusters);
        ASSERT( m_pGatherCell->ppCluster );
    }
    else
    {
        m_pGatherCell->ppCluster = m_pGatherCell->CLUSTERPTR;
    }

    // Copy cluster ptrs into cell and increase references to clusters
    for( s32 i=0; i<m_nGatherClusters; i++ )
    {
        m_pGatherCell->ppCluster[i] = m_GatherClusterList[i];
    }

    //
    // Shut off possibility of applying additional primitives
    //
    m_pGatherCell = NULL;

    CLOG_MESSAGE(DO_LOGGING,"POLYCACHE","***************************************************");

}

//==============================================================================

const bbox& poly_cache::GetGatherBBox( void ) { return m_GatherBBox; }

//==============================================================================

void poly_cache::GatherCluster( const collision_data&   CollData,
                                const matrix4*          pL2W,
                                      u64               MeshMask,
                                guid                    Guid )
{
    ASSERT( pL2W );
    if( !pL2W )
        return;

    object* pObj = g_ObjMgr.GetObjectByGuid( Guid );
    if( !pObj )
        return;

    // Get object type and attrbits
    object::type ObjectType = pObj->GetType();
    u32          AttrBits   = pObj->GetAttrBits();

    // Loop through the low_clusters
    for( s32 iCL=0; iCL<CollData.nLowClusters; iCL++ )
    {
        collision_data::low_cluster& CL = CollData.pLowCluster[iCL];

#if defined( USE_OBJECT_DEBUGINFO )
        if( CL.iBone == -1 )
        {
            const char* pName = pObj->GetName();
            const char* pTypeName = pObj->m_DebugInfo.m_pDesc->GetTypeName();
            (void)pName;
            (void)pTypeName;
            ASSERTS( (CL.iBone != -1), xfs("The collision data for '%s':'%s' %08x:%08x has a -1 bone assigned", pName, pTypeName, Guid.GetHigh(), Guid.GetLow()) );
            return ;
        }
#endif

        u64 Bit = 1 << CL.iMesh;
        if( !(MeshMask & Bit) )
            continue;

        // Transform local bbox into world
        bbox WorldBBox = CL.BBox;
        WorldBBox.Transform( pL2W[ CL.iBone ] );

        // Check if we want this cluster
        if( WorldBBox.Intersect( m_GatherBBox ) )
        {
            cluster* pCL = NULL;

            // Check if cluster has already been cached
            if( !(pCL = FindCluster( Guid, iCL )) )
            {
                // Allocate a cluster to cache into
                pCL = AllocCluster( CL.nPoints, CL.nNormals, CL.nQuads, Guid, iCL );
                ASSERT(pCL);

                // Most of the basic info was filled out in the Alloc()
                pCL->ObjectType = ObjectType;
                pCL->AttrBits   = AttrBits;
                pCL->iBone      = (byte)CL.iBone;
                pCL->PrimKey    = (((u32)iCL)<<16) | ((u32)CL.iBone);
            
                // Transform points
                {
                    //-------------------------------------------------------
                    const matrix4& M = pL2W[CL.iBone];
                    vector3* pS = CollData.pLowVector + CL.iVectorOffset;
                    vector3* pD = pCL->pPoint;
                    s32       N = pCL->nPoints;

                    /*#ifdef TARGET_PS2
                    asm __volatile__
                    ("
                        .set noreorder
                        lqc2    vf04,   0x30(%4)            # load col3
                        lqc2    vf03,   0x20(%4)            # load col2
                        lqc2    vf02,   0x10(%4)            # load col1
                        lqc2    vf01,   0x00(%4)            # load col0
                        ctc2        %0,     vi21            # I = F32_MAX
                        vsub.w      vf05,   vf05,   vf00    # bbox.min.w   = 0
                        vaddi.xyz   vf05,   vf00,   I       # bbox.min.xyz = ( F32_MAX, F32_MAX, F32_MAX)
                        vsub.w      vf06,   vf06,   vf00    # bbox.max.w   = 0
                        vsubi.xyz   vf06,   vf00,   I       # bbox.max.xyz = (-F32_MAX,-F32_MAX,-F32_MAX)
                        vsub.w      vf08, vf08, vf08        # zero out vf08w
                    vert_trans_loop:
                        lqc2        vf07, 0x00(%1)          # load *pS
                        vmulaw.xyz  acc,  vf04, vf00w       # xform col3
                        vmaddaz.xyz acc,  vf03, vf07z       # xform col2
                        vmadday.xyz acc,  vf02, vf07y       # xform col1
                        vmaddx.xyz  vf08, vf01, vf07x       # xform col0
                        addi        %1, %1, 16              # pS++
                        addi        %2, %2, 16              # pD++
                        subi        %0, %0, 1               # N--
                        vmini.xyz   vf05, vf05, vf08        # BBox.Min = MIN(BBox.Min,Point)
                        vmax.xyz    vf06, vf06, vf08        # BBox.Max = MAX(BBox.Max,Point)
                        bgtz        %0, vert_trans_loop     # loop
                        sqc2        vf08, -16(%2)           # (BDS) store *(pD-1)
                        vsubw.xyz   vf05,   vf05,   vf00w   # bbox.min -= 1.0f
                        vaddw.xyz   vf06,   vf06,   vf00w   # bbox.max += 1.0f
                        sqc2        vf05, 0x00(%3)          # store bbox
                        sqc2        vf06, 0x10(%3)          # store bbox
                        .set reorder
                    " :
                    "+r" (N), "+r" (pS), "+r" (pD) :
                    "r" (&pCL->BBox), "r" (&M) :
                    "memory", "vf1", "vf2", "vf3", "vf4", "vf5", "vf6", "vf7", "vf8" );
                    #else*/
                    //-------------------------------------------------------
                    bbox BBox;
                    BBox.Clear();
                    while( N-- )
                    {
                        *pD = M.Transform( *pS );
                        BBox += *pD;
                        pD++;
                        pS++;
                    }
                    BBox.Inflate(1.0f,1.0f,1.0f);
                    pCL->BBox = BBox;
                    /*#endif*/

#if defined( X_ASSERT ) && defined( X_EDITOR )
                    if( CL.iBone != -1 )
                    {
                        ASSERT( (pCL->BBox.Min.GetX() > -1E20f) && (pCL->BBox.Max.GetX() < +1E20f) );
                        ASSERT( (pCL->BBox.Min.GetY() > -1E20f) && (pCL->BBox.Max.GetY() < +1E20f) );
                        ASSERT( (pCL->BBox.Min.GetZ() > -1E20f) && (pCL->BBox.Max.GetZ() < +1E20f) );
                    }
#endif
                }

                // Transform normals
                {
                    vector3* pS = CollData.pLowVector + CL.iVectorOffset + CL.nPoints;
                    vector3* pD = pCL->pNormal;
                    s32       N = pCL->nNormals;

/*
                    //#### Disabled until the vector3 optimizations are complete
                    #ifdef TARGET_PS2
                    //-------------------------------------------------------
                    ASSERT( (((u32)pS) & 0x0F) == 0 );
                    ASSERT( (((u32)pD) & 0x0F) == 0 );
                    asm __volatile__
                    ("
                        .set noreorder
                        vsub.w      vf08, vf08, vf08        # zero out vf08w
                    normal_trans_loop:
                        lqc2        vf07, 0x00(%0)          # load *pS
                        subi        %2, %2, 1               # N--
                        vmulaz.xyz  acc,  vf03, vf07z       # xform col2
                        vmadday.xyz acc,  vf02, vf07y       # xform col1
                        vmaddx.xyz  vf08, vf01, vf07x       # xform col0
                        addi        %0, %0, 16              # pS++
                        addi        %1, %1, 16              # pD++
                        bgtz        %2, normal_trans_loop   # loop
                        sqc2        vf08, -16(%1)           # (BDS) store *(pD-1)
                        .set reorder
                    " : "+r" (pS), "+r" (pD), "+r" (N) : : "memory" );
                    #else
                    */
                    //-------------------------------------------------------
                    const matrix4& M = pL2W[CL.iBone];
                    while( N-- )
                    {
                        *pD = M.RotateVector( *pS );
                        pD++;
                        pS++;
                    }
                    //#endif
                }

                // Copy over quad info and generate other information
                for( s32 i=0; i<CL.nQuads; i++ )
                {
                    collision_data::low_quad& QD = CollData.pLowQuad[ CL.iQuadOffset + i ];
                    cluster::quad& CQD = pCL->pQuad[i];
                
                    CQD.iP[0] = QD.iP[0];
                    CQD.iP[1] = QD.iP[1];
                    CQD.iP[2] = QD.iP[2];
                    CQD.iP[3] = QD.iP[3];
                    CQD.iN    = QD.iN;

                    // Clear the bbox and build it
                    bbox& BBox = *((bbox*)&(pCL->pBounds[i]));
                    /*
                    //#### Disabled until the vector3 optimizations are complete
                    #ifdef TARGET_PS2
                    //-------------------------------------------------------
                    f32 fmax = F32_MAX;
                    u32 tmp  = reinterpret_cast<u32&>(fmax);
                    asm __volatile__
                    ("
                        ctc2        %0,     vi21            # I = F32_MAX
                        vsub.w      vf01,   vf01,   vf00    # bbox.min.w   = 0
                        vaddi.xyz   vf01,   vf00,   I       # bbox.min.xyz = ( F32_MAX, F32_MAX, F32_MAX)
                        vsub.w      vf02,   vf02,   vf00    # bbox.max.w   = 0
                        vsubi.xyz   vf02,   vf00,   I       # bbox.max.xyz = (-F32_MAX,-F32_MAX,-F32_MAX)
                        lqc2        vf03,   0x00(%2)        # point0
                        lqc2        vf04,   0x00(%3)        # point1
                        lqc2        vf05,   0x00(%4)        # point2
                        lqc2        vf06,   0x00(%5)        # point3
                        vmini.xyz   vf01,   vf01,   vf03    # bbox.min = MIN(bbox.min,point0)
                        vmax.xyz    vf02,   vf02,   vf03    # bbox.max = MAX(bbox.max,point0)
                        vmini.xyz   vf01,   vf01,   vf04    # bbox.min = MIN(bbox.min,point1)
                        vmax.xyz    vf02,   vf02,   vf04    # bbox.max = MAX(bbox.max,point1)
                        vmini.xyz   vf01,   vf01,   vf05    # bbox.min = MIN(bbox.min,point2)
                        vmax.xyz    vf02,   vf02,   vf05    # bbox.max = MAX(bbox.max,point2)
                        vmini.xyz   vf01,   vf01,   vf06    # bbox.min = MIN(bbox.min,point3)
                        vmax.xyz    vf02,   vf02,   vf06    # bbox.max = MAX(bbox.max,point3)
                        vsubw.xyz   vf01,   vf01,   vf00w   # bbox.min -= 1.0f
                        vaddw.xyz   vf02,   vf02,   vf00w   # bbox.max += 1.0f
                        sqc2        vf01,   0x00(%1)        # store bbox.min
                        sqc2        vf02,   0x10(%1)        # store bbox.max
                    " :
                    "+r" (tmp) :
                    "r" (&BBox),
                    "r" (&pCL->pPoint[CQD.iP[0]]),
                    "r" (&pCL->pPoint[CQD.iP[1]]),
                    "r" (&pCL->pPoint[CQD.iP[2]]),
                    "r" (&pCL->pPoint[CQD.iP[3]]) :
                    "memory" );
                    #else
                    */
                    //-------------------------------------------------------
                    BBox.Clear();
                    BBox += pCL->pPoint[ CQD.iP[0] ];
                    BBox += pCL->pPoint[ CQD.iP[1] ];
                    BBox += pCL->pPoint[ CQD.iP[2] ];
                    BBox += pCL->pPoint[ CQD.iP[3] ];
                    BBox.Inflate(1,1,1);
                    //#endif

                    // Clear flags
                    pCL->pBounds[i].Flags = 0;

                    // Remember if this is a quad
                    if( pCL->pPoint[ CQD.iP[0] ]!=pCL->pPoint[ CQD.iP[3] ] )
                        pCL->pBounds[i].Flags |= BOUNDS_IS_QUAD;

                    // Compute D value of plane
                    vector4 N = pCL->pNormal[CQD.iN];
                    N.GetW() = 0;
                    pCL->pBounds[i].PlaneD = -N.Dot( pCL->pPoint[ CQD.iP[0] ] );

                    // Set other normal flags
                    if( N.GetX() > +0.999f ) pCL->pBounds[i].Flags |= BOUNDS_X_POS;
                    if( N.GetX() < -0.999f ) pCL->pBounds[i].Flags |= BOUNDS_X_NEG;
                    if( N.GetY() > +0.999f ) pCL->pBounds[i].Flags |= BOUNDS_Y_POS;
                    if( N.GetY() < -0.999f ) pCL->pBounds[i].Flags |= BOUNDS_Y_NEG;
                    if( N.GetZ() > +0.999f ) pCL->pBounds[i].Flags |= BOUNDS_Z_POS;
                    if( N.GetZ() < -0.999f ) pCL->pBounds[i].Flags |= BOUNDS_Z_NEG;
                }
            }
            else
            {
                m_nGatherClustersAlreadyCached++;
                pCL->nReferences++;
            }

            //
            // Add cluster to list for this cell!
            //
            ASSERT( m_nGatherClusters<128 );
            ASSERT( pCL->iCollDataCluster < 1024 );
            m_GatherClusterList[m_nGatherClusters] = pCL;
            m_nGatherClusters++;
        }
    }
}


//==============================================================================

void poly_cache::BuildClusterList( const bbox&   aBBox,
                                   object::type  ThisType,
                                   u32           TheseAttributes,
                                   u32           NotTheseAttributes,
                                   const guid*   pIgnoreList,
                                   s32           nIgnores)
{
    bbox BBox = aBBox;
    BBox.Inflate(1,1,1);

    m_nClusters = 0;

    IncrementSequence();

    s32 MinX,MinY,MinZ;
    s32 MaxX,MaxY,MaxZ;
    GetCellRegion( BBox, MinX, MinY, MinZ, MaxX, MaxY, MaxZ );

    for( s32 X=MinX; X<=MaxX; X++ )
    for( s32 Y=MinY; Y<=MaxY; Y++ )
    for( s32 Z=MinZ; Z<=MaxZ; Z++ )
    {
        cell* pCell = AcquireCell(X,Y,Z);

        // Loop through clusters in cell and add to list
        for( s32 i=0; i<pCell->nClusters; i++ )
        {
            cluster* pCL = pCell->ppCluster[i];

            if( pCL->Sequence == m_Sequence )
                continue;
            pCL->Sequence = m_Sequence;

            if( (pCL->AttrBits & TheseAttributes) == 0 )
                continue;
            
            if( (pCL->AttrBits & NotTheseAttributes) != 0 )
                continue;

            // Make sure that it matches the type
            if( (ThisType != object::TYPE_ALL_TYPES) && (pCL->ObjectType != ThisType) )
                continue;

            if( BBox.Intersect( pCL->BBox ) )
            {
                // Check if cluster is already listed
/*                
                s32 j=0;
                for( j=0; j<m_nClusters; j++ )
                if( m_ClusterList[j] == pCL )
                    break;
                if( j==m_nClusters )
*/                    
                {
                    // Check if it's in the ignore list
                    s32 k=0;
                    for( k=0; k<nIgnores; k++ )
                    if( pCL->Guid == pIgnoreList[k] )
                        break;

                    if( k==nIgnores )
                    {
                        ASSERT( m_nClusters < POLYCACHE_MAX_CLUSTERS_IN_LIST );
                        if( m_nClusters < POLYCACHE_MAX_CLUSTERS_IN_LIST )
                        {
                            m_ClusterList[m_nClusters] = pCL;
                            m_nClusters++;
                        }
                    }
                }
            }
        }
    }
}

//==============================================================================

void poly_cache::BeginRayClusterWalk(   const vector3& RayStart, 
                                        const vector3& RayEnd,
                                        object::type  ThisType,
                                        u32           TheseAttributes,
                                        u32           NotTheseAttributes,
                                        const guid*   pIgnoreList,
                                        s32           nIgnores)
{
    // Clear cluster array.
    m_nClusters = 0;
    m_iNextCluster = 0;

    // Setup initial ray grid walking information
    m_GridWalker.Setup( RayStart, RayEnd, vector3(0,0,0), POLYCACHE_CELL_SIZE );    

    // Copy 
    m_RayStart = RayStart;
    m_RayEnd   = RayEnd;
    m_ThisType = ThisType;
    m_TheseAttributes = TheseAttributes;
    m_NotTheseAttributes = NotTheseAttributes;
    m_pIgnoreList = pIgnoreList;
    m_nIgnores = nIgnores;

    IncrementSequence();
}

//==============================================================================

xbool poly_cache::StepRayClusterWalker( void )
{
    m_nClusters = 0;
    m_iNextCluster = 0;

    while( m_nClusters==0 )
    {
        // Pull out cell we are currently in
        s32 X,Y,Z;
        m_GridWalker.GetCell( X, Y, Z );

        // Get segment positions
        //vector3 SegmentStart;
        //vector3 SegmentEnd;
        //m_GridWalker.GetSegment( SegmentStart, SegmentEnd );

        // Get bbox around segment
        //fbbox SegmentBBox( SegmentStart, SegmentEnd );
        //SegmentBBox.Inflate(1,1,1);

        // Check for any clusters in this cell
        {
            cell* pCell = AcquireCell(X,Y,Z);

            // Loop through clusters in cell and add to list
            for( s32 i=0; i<pCell->nClusters; i++ )	
            {
                cluster* pCL = pCell->ppCluster[i];

                // Use sequence number
                if( pCL->Sequence == m_Sequence )
                    continue;
                pCL->Sequence = m_Sequence;

                if( (pCL->AttrBits & m_TheseAttributes) == 0 )
                    continue;
                
                if( (pCL->AttrBits & m_NotTheseAttributes) != 0 )
                    continue;

                // Make sure that it matches the type
                if( (m_ThisType != object::TYPE_ALL_TYPES) && (pCL->ObjectType != m_ThisType) )
                    continue;

                //if( SegmentBBox.Intersect( pCL->BBox ) )
                //f32 T;
                //if( pCL->BBox.Intersect( T, m_RayStart, m_RayEnd ) )
                {
                    // Check if it's in the ignore list
                    
                    s32 k=0;
                    for( k=0; k<m_nIgnores; k++ )
                    if( pCL->Guid == m_pIgnoreList[k] )
                        break;

                    if( k==m_nIgnores )
                    {
                        ASSERT( m_nClusters < POLYCACHE_MAX_CLUSTERS_IN_LIST );
                        if( m_nClusters < POLYCACHE_MAX_CLUSTERS_IN_LIST )
                        {
                            m_ClusterList[m_nClusters] = pCL;
                            m_nClusters++;
                        }
                    }
                }
            }
        }

        
        // Step the grid walker
        if( !m_GridWalker.Step() )
            return FALSE;
    }

    return TRUE;
}


//==============================================================================

void poly_cache::ClearSequence( void )
{
    // Clear sequence for all clusters
    m_Sequence = 1;

    // Loop through banks
    for( s32 iB=0; iB<POLYCACHE_NUM_CLUSTER_BANKS; iB++ )
    {
        cluster_bank& B = m_ClusterBank[iB];

        // Loop through all the clusters
        for( s32 i=0; i<B.nClusters; i++ )
        {
            cluster& CL = *((cluster*)(((byte*)(B.pCluster)) + (i*B.ClusterSize)));
            CL.Sequence = 0;
        }
    }
}

//==============================================================================
