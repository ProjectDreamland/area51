//==============================================================================
//
//  RigidGeomCollision.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "RigidGeomCollision.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "e_Draw.hpp"

#define INDEX_MASK  0x7FFF

//==============================================================================
//  FUNCTIONS
//==============================================================================

inline
void RigidGeom_SetPrimKey( s32& PrimKey, xbool bUseHighPoly, s32 iCluster, s32 iTriangle )
{
    PrimKey  = (iCluster << 16) | (iTriangle);
    PrimKey |= (bUseHighPoly) ? (0x80000000) : (0x00000000);
}

//==============================================================================

inline
void RigidGeom_GetPrimKey( s32 PrimKey, xbool& bUseHighPoly, s32& iCluster, s32& iTriangle )
{
    ASSERT( PrimKey != -1 );
    bUseHighPoly    = (PrimKey & 0x80000000) ? (TRUE):(FALSE);
    iTriangle       = (PrimKey & 0xFFFF);
    iCluster        = (PrimKey >> 16) & 0x7FFF;
}

//===========================================================================

xbool RigidGeom_GetTriangle( const rigid_geom*          pRigidGeom,
                             s32                   Key,
                             vector3&              P0,
                             vector3&              P1,
                             vector3&              P2)
{
    if( pRigidGeom==NULL )
        return FALSE;

    ASSERT( pRigidGeom->m_Collision.nHighClusters );
    
    xbool bUseHighPoly;
    s32 iCluster;
    s32 iTriangle;

    RigidGeom_GetPrimKey(Key,bUseHighPoly,iCluster,iTriangle);
        
    if( !IN_RANGE( 0, iCluster, pRigidGeom->m_Collision.nHighClusters-1 ) )
        return( FALSE );

    #ifdef TARGET_XBOX
    {
        collision_data::high_cluster& Cluster = pRigidGeom->m_Collision.pHighCluster [iCluster];
        rigid_geom::dlist_xbox      & DList   = pRigidGeom->m_System.pXbox[Cluster.iDList];
        ASSERT( DList.iColor <= pRigidGeom->m_nVertices );
        if( !IN_RANGE( 0, iTriangle, Cluster.nTris-1 ) )
            return( FALSE );

        s32 Index = pRigidGeom->m_Collision.pHighIndexToVert0[ Cluster.iOffset + iTriangle ];

        // We have arrived.  Fill out the information.

        P0 = DList.pVert[ DList.pIndices[Index+0] ].Pos;
        P1 = DList.pVert[ DList.pIndices[Index+1] ].Pos;
        P2 = DList.pVert[ DList.pIndices[Index+2] ].Pos;

        return( TRUE );
    }
    #endif

    #ifdef TARGET_PC
    {
        collision_data::high_cluster& Cluster = pRigidGeom->m_Collision.pHighCluster [iCluster];
        rigid_geom::dlist_pc&    DList   = pRigidGeom->m_System.pPC[Cluster.iDList];

        if( !IN_RANGE( 0, iTriangle, Cluster.nTris-1 ) )
            return( FALSE );

        s32 Index = pRigidGeom->m_Collision.pHighIndexToVert0[ Cluster.iOffset + iTriangle ];

        // We have arrived.  Fill out the information.

        P0 = DList.pVert[ DList.pIndices[Index+0] ].Pos;
        P1 = DList.pVert[ DList.pIndices[Index+1] ].Pos;
        P2 = DList.pVert[ DList.pIndices[Index+2] ].Pos;

        return( TRUE );
    }
    #endif

    #ifdef TARGET_PS2
    {
        collision_data::high_cluster& Cluster = pRigidGeom->m_Collision.pHighCluster[iCluster];
        rigid_geom::dlist_ps2&   DList   = pRigidGeom->m_System.pPS2[Cluster.iDList];

        if( !IN_RANGE( 0, iTriangle, Cluster.nTris-1 ) )
            return( FALSE );

        u16 Index =  pRigidGeom->m_Collision.pHighIndexToVert0[ Cluster.iOffset + iTriangle ];

        if( Index & 0x8000 )
        {
            Index &= 0x7FFF;
            P0 = *((vector3*)(DList.pPosition + (Index+0)));
            P1 = *((vector3*)(DList.pPosition + (Index+2)));
            P2 = *((vector3*)(DList.pPosition + (Index+1)));
        }
        else
        {
            P0 = *((vector3*)(DList.pPosition + (Index+0)));
            P1 = *((vector3*)(DList.pPosition + (Index+1)));
            P2 = *((vector3*)(DList.pPosition + (Index+2)));
        }

        return( TRUE );
    }
    #endif

    return( FALSE );

}

//===========================================================================
#ifdef TARGET_XBOX
static vector3 UnpackNormal( u32 PackedNormal )
{
    return vector3( f32((PackedNormal>> 0L) &~0x7ff)/1023.0f,
                    f32((PackedNormal>>11L) &~0x7ff)/1023.0f,
                    f32((PackedNormal>>22L) &~0x3ff) /511.0f );
}
#endif

//===========================================================================

xbool RigidGeom_GetColDetails(const rigid_geom*     pRigidGeom,
                              const matrix4*        pL2W,
                              const void*           pColorIn,
                              s32                   Key,
                              object::detail_tri&   Tri )
{
#ifdef TARGET_XBOX
    const u32* pColor=( u32* )pColorIn;
#else
    const u16* pColor=( u16* )pColorIn;
#endif
    ASSERT( pRigidGeom );
    ASSERT( pRigidGeom->m_Collision.nHighClusters );
    const collision_data& Coll = pRigidGeom->m_Collision;

    xbool   bUseHighPoly;
    s32     iCluster;
    s32     iTriangle;
    RigidGeom_GetPrimKey( Key, bUseHighPoly, iCluster, iTriangle );
    ASSERT( bUseHighPoly );
        
    if( !IN_RANGE( 0, iCluster, Coll.nHighClusters-1 ) )
        return( FALSE );

    #ifdef TARGET_XBOX
    {
        collision_data::high_cluster& Cluster = Coll.pHighCluster[iCluster];
        rigid_geom::dlist_xbox      & DList   = pRigidGeom->m_System.pXbox[Cluster.iDList];
        ASSERT( DList.iColor <= pRigidGeom->m_nVertices );

        if( !IN_RANGE( 0, iTriangle, Cluster.nTris-1 ) )
            return( FALSE );

        s32 Index  = Coll.pHighIndexToVert0[ Cluster.iOffset + iTriangle ] & INDEX_MASK;

        // We have arrived.  Fill out the information.
        const matrix4& L2W = *pL2W;

        Tri.Vertex[0] = DList.pVert[ DList.pIndices[Index+0] ].Pos;
        Tri.Vertex[1] = DList.pVert[ DList.pIndices[Index+1] ].Pos;
        Tri.Vertex[2] = DList.pVert[ DList.pIndices[Index+2] ].Pos;
        L2W.Transform( Tri.Vertex, Tri.Vertex, 3 );

        if( !pColor )
        {
            Tri.Color [0] = 0xFFFFFFFF;
            Tri.Color [1] = 0xFFFFFFFF;
            Tri.Color [2] = 0xFFFFFFFF;
        }
        else
        {
            Tri.Color [0] = pColor[ DList.pIndices[Index+0]+DList.iColor ];
            Tri.Color [1] = pColor[ DList.pIndices[Index+1]+DList.iColor ];
            Tri.Color [2] = pColor[ DList.pIndices[Index+2]+DList.iColor ];
        }

        Tri.Normal[0] = UnpackNormal( DList.pVert[ DList.pIndices[Index+0] ].PackedNormal );
        Tri.Normal[1] = UnpackNormal( DList.pVert[ DList.pIndices[Index+1] ].PackedNormal );
        Tri.Normal[2] = UnpackNormal( DList.pVert[ DList.pIndices[Index+2] ].PackedNormal );
        L2W.Transform( Tri.Normal, Tri.Normal, 3 );
        Tri.Normal[0] -= L2W.GetTranslation();
        Tri.Normal[1] -= L2W.GetTranslation();
        Tri.Normal[2] -= L2W.GetTranslation();

        Tri.UV    [0] = DList.pVert[ DList.pIndices[Index+0] ].UV;
        Tri.UV    [1] = DList.pVert[ DList.pIndices[Index+1] ].UV;
        Tri.UV    [2] = DList.pVert[ DList.pIndices[Index+2] ].UV;

        Tri.MaterialInfo = Cluster.MaterialInfo;

        return( TRUE );
    }
    #endif

    #ifdef TARGET_PC
    {
        collision_data::high_cluster& Cluster = Coll.pHighCluster[iCluster];
        rigid_geom::dlist_pc&    DList   = pRigidGeom->m_System.pPC[Cluster.iDList];

        if( !IN_RANGE( 0, iTriangle, Cluster.nTris-1 ) )
            return( FALSE );

        s32 Index  = Coll.pHighIndexToVert0[ Cluster.iOffset + iTriangle ] & INDEX_MASK;

        // We have arrived.  Fill out the information.
        const matrix4& L2W = *pL2W;

        Tri.Vertex[0] = DList.pVert[ DList.pIndices[Index+0] ].Pos;
        Tri.Vertex[1] = DList.pVert[ DList.pIndices[Index+1] ].Pos;
        Tri.Vertex[2] = DList.pVert[ DList.pIndices[Index+2] ].Pos;
        L2W.Transform( Tri.Vertex, Tri.Vertex, 3 );

        if( !pColor )
        {
            Tri.Color [0] = 0xFFFFFFFF;
            Tri.Color [1] = 0xFFFFFFFF;
            Tri.Color [2] = 0xFFFFFFFF;
        }
        else
        {
            Tri.Color [0] = DList.pVert[ DList.pIndices[Index+0] ].Color;
            Tri.Color [1] = DList.pVert[ DList.pIndices[Index+1] ].Color;
            Tri.Color [2] = DList.pVert[ DList.pIndices[Index+2] ].Color;
            //Tri.Color [0] = pColor[ DList.pIndices[Index+0]+DList.iColor ];
            //Tri.Color [1] = pColor[ DList.pIndices[Index+1]+DList.iColor ];
            //Tri.Color [2] = pColor[ DList.pIndices[Index+2]+DList.iColor ];
        }

        Tri.Normal[0] = DList.pVert[ DList.pIndices[Index+0] ].Normal;
        Tri.Normal[1] = DList.pVert[ DList.pIndices[Index+1] ].Normal;
        Tri.Normal[2] = DList.pVert[ DList.pIndices[Index+2] ].Normal;
        L2W.Transform( Tri.Normal, Tri.Normal, 3 );
        Tri.Normal[0] -= L2W.GetTranslation();
        Tri.Normal[1] -= L2W.GetTranslation();
        Tri.Normal[2] -= L2W.GetTranslation();

        Tri.UV    [0] = DList.pVert[ DList.pIndices[Index+0] ].UV;
        Tri.UV    [1] = DList.pVert[ DList.pIndices[Index+1] ].UV;
        Tri.UV    [2] = DList.pVert[ DList.pIndices[Index+2] ].UV;

        Tri.MaterialInfo = Cluster.MaterialInfo;

        return( TRUE );
    }
    #endif

    #ifdef TARGET_PS2
    {
        f32                      Factor;
        collision_data::high_cluster& Cluster = Coll.pHighCluster[iCluster];
        rigid_geom::dlist_ps2&   DList   = pRigidGeom->m_System.pPS2[Cluster.iDList];

        if( !IN_RANGE( 0, iTriangle, Cluster.nTris-1 ) )
            return( FALSE );

        u16 Index  = Coll.pHighIndexToVert0[ Cluster.iOffset + iTriangle ] & INDEX_MASK;
        
        // We have arrived.  Fill out the information.
        const matrix4& L2W = *pL2W;

        Tri.Vertex[0] = *((vector3*)(DList.pPosition + (Index+0)));
        Tri.Vertex[1] = *((vector3*)(DList.pPosition + (Index+1)));
        Tri.Vertex[2] = *((vector3*)(DList.pPosition + (Index+2)));
        L2W.Transform( Tri.Vertex, Tri.Vertex, 3 );

        if( pColor )
        {
            pColor += DList.iColor;
            pColor += Index;

            //
            // Assuming the color uses its 16 bits as follows:
            //
            //   1111:1111:1111:1111
            //   ABBB:BBGG:GGGR:RRRR
            //
            //   1000:0000:0000:0000 = 0x8000
            //   0111:1100:0000:0000 = 0x7C00
            //   0000:0011:1110:0000 = 0x03E0
            //   0000:0000:0001:1111 = 0x001F
            //
            //  For RGB, since there are only 5 bits present, the top 3 bits of
            //  the original 5 are used again for the lower 3 bits to give best 
            //  possible results.  
            //
            //  Thus, for the 8 bits of R...
            //  Take the original data:         abbb:bbgg:gggR:RRRR  
            //  Mask out all but R data:        0000:0000:000R:RRRR
            //  Shift left by 3:                0000:0000:RRRR:R000
            //  Also shift RIGHT by 2:          0000:0000:0000:0RRR.rr
            //  Combine last two:               0000:0000:RRRR:RRRR
            //
            //  Since the A channel has only 1 bit, we just result in 0 or 255.
            //

            Tri.Color[0].R = ((*pColor & 0x001F) <<  3) | ((*pColor & 0x001F) >>  2);
            Tri.Color[0].G = ((*pColor & 0x03E0) >>  2) | ((*pColor & 0x03E0) >>  7);
            Tri.Color[0].B = ((*pColor & 0x7C00) >>  7) | ((*pColor & 0x7C00) >> 12);
            Tri.Color[0].A =  (*pColor & 0x8000) ? 255 : 0;
            
            pColor += 1;

            Tri.Color[1].R = ((*pColor & 0x001F) <<  3) | ((*pColor & 0x001F) >>  2);
            Tri.Color[1].G = ((*pColor & 0x03E0) >>  2) | ((*pColor & 0x03E0) >>  7);
            Tri.Color[1].B = ((*pColor & 0x7C00) >>  7) | ((*pColor & 0x7C00) >> 12);
            Tri.Color[1].A =  (*pColor & 0x8000) ? 255 : 0;
            
            pColor += 1;

            Tri.Color[2].R = ((*pColor & 0x001F) <<  3) | ((*pColor & 0x001F) >>  2);
            Tri.Color[2].G = ((*pColor & 0x03E0) >>  2) | ((*pColor & 0x03E0) >>  7);
            Tri.Color[2].B = ((*pColor & 0x7C00) >>  7) | ((*pColor & 0x7C00) >> 12);
            Tri.Color[2].A =  (*pColor & 0x8000) ? 255 : 0;
        }
        else
        {
            Tri.Color [0] = XCOLOR_WHITE;
            Tri.Color [1] = XCOLOR_WHITE;
            Tri.Color [2] = XCOLOR_WHITE;
        } 

        // Normals are fixed point as S0.7
        Factor = 1.0f / 128.0f;

        Tri.Normal[0].Set( ((f32)(DList.pNormal[ ((Index+0)*3)+0 ])) * Factor,
                           ((f32)(DList.pNormal[ ((Index+0)*3)+1 ])) * Factor,
                           ((f32)(DList.pNormal[ ((Index+0)*3)+2 ])) * Factor );
        Tri.Normal[1].Set( ((f32)(DList.pNormal[ ((Index+1)*3)+0 ])) * Factor,
                           ((f32)(DList.pNormal[ ((Index+1)*3)+1 ])) * Factor,
                           ((f32)(DList.pNormal[ ((Index+1)*3)+2 ])) * Factor );
                                                          
        Tri.Normal[2].Set( ((f32)(DList.pNormal[ ((Index+2)*3)+0 ])) * Factor,
                           ((f32)(DList.pNormal[ ((Index+2)*3)+1 ])) * Factor,
                           ((f32)(DList.pNormal[ ((Index+2)*3)+2 ])) * Factor );
        
        Tri.Normal[0] = L2W.RotateVector( Tri.Normal[0] );
        Tri.Normal[1] = L2W.RotateVector( Tri.Normal[1] );
        Tri.Normal[2] = L2W.RotateVector( Tri.Normal[2] );

        // UVs are fixed point as S3.12
        Factor = 1.0f / 4096.0f;

        Tri.UV[0].X = ((f32)(DList.pUV[ ((Index+0)*2)+0 ])) * Factor;
        Tri.UV[0].Y = ((f32)(DList.pUV[ ((Index+0)*2)+1 ])) * Factor;

        Tri.UV[1].X = ((f32)(DList.pUV[ ((Index+1)*2)+0 ])) * Factor;
        Tri.UV[1].Y = ((f32)(DList.pUV[ ((Index+1)*2)+1 ])) * Factor;

        Tri.UV[2].X = ((f32)(DList.pUV[ ((Index+2)*2)+0 ])) * Factor;
        Tri.UV[2].Y = ((f32)(DList.pUV[ ((Index+2)*2)+1 ])) * Factor;

        // And let's not forget the material.
        Tri.MaterialInfo = Cluster.MaterialInfo;

        return( TRUE );
    }
    #endif

    (void)Tri;
    return( FALSE );
}

//==============================================================================

xbool ClipLineToBBoxWithSafeSpace( const bbox& BBox, const vector3& P0, const vector3& P1, f32& T0, f32& T1 )
{
    vector3 Dir = P1 - P0;
    f32 tx_min, tx_max;
    f32 ty_min, ty_max;
    f32 tz_min, tz_max;
    f32 t_min, t_max;

    vector3 MinMinusP0 = BBox.Min - P0;
    vector3 MaxMinusP0 = BBox.Max - P0;

    if( Dir.GetX() >= 0.0f )
    {
        t_min = tx_min = MinMinusP0.GetX() / Dir.GetX();
        t_max = tx_max = MaxMinusP0.GetX() / Dir.GetX();
    }
    else
    {
        t_min = tx_min = MaxMinusP0.GetX() / Dir.GetX();
        t_max = tx_max = MinMinusP0.GetX() / Dir.GetX();
    }

    if( Dir.GetY() >= 0.0f )
    {
        ty_min = MinMinusP0.GetY() / Dir.GetY();
        ty_max = MaxMinusP0.GetY() / Dir.GetY();
    }
    else
    {
        ty_min = MaxMinusP0.GetY() / Dir.GetY();
        ty_max = MinMinusP0.GetY() / Dir.GetY();
    }

    if( t_min > ty_max || ty_min > t_max )
        return FALSE;

    if( t_min < ty_min )
        t_min = ty_min;
    if( t_max > ty_max )
        t_max = ty_max;

    if( Dir.GetZ() >= 0.0f )
    {
        tz_min = MinMinusP0.GetZ() / Dir.GetZ();
        tz_max = MaxMinusP0.GetZ() / Dir.GetZ();
    }
    else
    {
        tz_min = MaxMinusP0.GetZ() / Dir.GetZ();
        tz_max = MinMinusP0.GetZ() / Dir.GetZ();
    }

    if( t_min > tz_max || tz_min > t_max )
        return FALSE;

    if( t_min < tz_min )
        t_min = tz_min;
    if( t_max > tz_max )
        t_max = tz_max;

    if( !((t_min <= 1.0f) && (t_max >= 0.0f)) ) 
        return FALSE;

   // Set the initial entry and exit points of the ray
   T0 = t_min;
   T1 = t_max;

   // Backup and push forward by 1cm.
   f32 RayLen = Dir.Length();
   f32 SafeT = 1.0f / RayLen;
   T0 -= SafeT;
   T1 += SafeT;
   if( T0 < 0 ) T0 = 0;
   if( T1 > 1 ) T1 = 1;

   return TRUE;
}

//==============================================================================

#ifndef X_RETAIL
s32 TRI_STATS[20]={0};
xbool bDISPLAY_TIME = 0;
#endif // X_RETAIL

//==============================================================================

xbool IsPointInsideTri( const vector3& P, const vector3& TriP0, const vector3& TriP1, const vector3& TriP2, const vector3& TriNormal )
{
    vector3 EdgeNormal;

    EdgeNormal = TriNormal.Cross( TriP1 - TriP0 );
    f32 PDist = EdgeNormal.Dot( P - TriP0 );
    if( (PDist < 0.0f) )
        return FALSE;

    EdgeNormal = TriNormal.Cross( TriP2 - TriP1 );
    if( EdgeNormal.Dot( P - TriP1 ) < 0.0f )
        return FALSE;

    EdgeNormal = TriNormal.Cross( TriP0 - TriP2 );
    if( EdgeNormal.Dot( P - TriP2 ) < 0.0f )
        return FALSE;

    return TRUE;
}

//==============================================================================

void RigidGeom_RayVsPS2HiPoly(      guid        Guid, 
                               const bbox&      WorldBBox,
                               const u64         MeshMask,
                               const matrix4*    pL2W, 
                               const rigid_geom& RigidGeom )
{
    s32 k;

#ifndef X_RETAIL
    xtimer Timer;
    if( bDISPLAY_TIME )
        Timer.Start();
    TRI_STATS[0]++;
#endif // X_RETAIL
    
    //
    // Dig out Ray and clip to world bbox
    //
    vector3 WorldRayStart;
    vector3 WorldRayEnd;
    f32     LocalRayT0;
    f32     LocalRayT1;
    {

        const collision_mgr::dynamic_ray& Ray = g_CollisionMgr.GetDynamicRay();
        if( !ClipLineToBBoxWithSafeSpace( WorldBBox, Ray.Start, Ray.End, LocalRayT0, LocalRayT1 ) )
            return;

        vector3 Direction = Ray.End - Ray.Start;
        WorldRayStart = Ray.Start + LocalRayT0*(Direction);
        WorldRayEnd   = Ray.Start + LocalRayT1*(Direction);
    }

    // Which collision resolution to use?
    ASSERT( g_CollisionMgr.IsUsingHighPoly() );
    ASSERT( RigidGeom.m_Collision.nHighClusters );

    const collision_data& Coll = RigidGeom.m_Collision;

    // For every bone...
    s32 iCurrentBone = -1;
    const matrix4* pCurrentL2W = NULL;
    vector3 LocalRayStart;
    vector3 LocalRayEnd;
    vector3 LocalRayDelta;
    bbox    LocalRayBBox;
    vector3 ClusterRayStart;
    vector3 ClusterRayEnd;
    vector3 ClusterRayDelta;
    bbox    ClusterRayBBox;
    f32     ClusterRayT0;
    f32     ClusterRayT1;

    // Loop thru the clusters and apply them if:
    //  - mesh is active
    //  - cluster uses the current bone
    for( s32 iC = 0; iC < Coll.nHighClusters; iC++ )
    {
        const collision_data::high_cluster& Cluster = Coll.pHighCluster[iC];

        //
        // Do quick checks to see if we should be skipping this mesh or material
        //
        {
            u64 Bit = 1 << Cluster.iMesh;
            if( !(MeshMask & Bit) )
                continue;

            if( (Cluster.MaterialInfo.SoundType == object::MAT_TYPE_GLASS) && 
                (g_CollisionMgr.IsIgnoringGlass()) )
                continue;
        }

        xbool bDoubleSided = !!(Cluster.MaterialInfo.Flags & collision_data::mat_info::FLAG_DOUBLESIDED);

        //
        // Update local ray info
        //
        if( Cluster.iBone != iCurrentBone )
        {
            iCurrentBone = Cluster.iBone;

            // Get ptr to the current matrix
            pCurrentL2W = &pL2W[iCurrentBone];

            // Transform ray into space of bone.  Rather than InvertRT the matrix
            // we're just going to use InvRotateVector
            vector3 Translation = pCurrentL2W->GetTranslation();
            LocalRayStart = pCurrentL2W->InvRotateVector( WorldRayStart - Translation );
            LocalRayEnd   = pCurrentL2W->InvRotateVector( WorldRayEnd   - Translation );

            LocalRayDelta = LocalRayEnd - LocalRayStart;
            LocalRayBBox.Set(LocalRayStart,LocalRayEnd);
            LocalRayBBox.Inflate(1,1,1);
        }

        //
        // Check if local ray bbox intersects cluster bbox and if ray intersects
        //
        {
            if( !LocalRayBBox.Intersect( Cluster.BBox ) )
                continue;
        
            if( !ClipLineToBBoxWithSafeSpace( Cluster.BBox, LocalRayStart, LocalRayEnd, ClusterRayT0, ClusterRayT1 ) )
                continue;

            ClusterRayStart = LocalRayStart + ClusterRayT0*LocalRayDelta;
            ClusterRayEnd   = LocalRayStart + ClusterRayT1*LocalRayDelta;
            ClusterRayDelta = ClusterRayEnd - ClusterRayStart;
            ClusterRayBBox.Set(ClusterRayStart,ClusterRayEnd);
        }

        //
        // Apply the cluster to the collision manager.
        //
        const rigid_geom::dlist_ps2& DList = RigidGeom.m_System.pPS2[Cluster.iDList];

        for( k = 0; k < Cluster.nTris; k++ )
        {
            u32 Offset  = Coll.pHighIndexToVert0[ Cluster.iOffset + k ];

            //
            // Setup ptrs to positions
            //
            const vector3* P0;
            const vector3* P1;
            const vector3* P2;
            if( Offset & 0x8000 )
            {
                Offset &= INDEX_MASK;
                P0 = (vector3*)(&DList.pPosition[Offset+2]);
                P1 = (vector3*)(&DList.pPosition[Offset+1]);
                P2 = (vector3*)(&DList.pPosition[Offset+0]);
            }
            else
            {
                P0 = (vector3*)(&DList.pPosition[Offset+0]);
                P1 = (vector3*)(&DList.pPosition[Offset+1]);
                P2 = (vector3*)(&DList.pPosition[Offset+2]);
            }

            if( ClusterRayBBox.IntersectTriBBox( *P0, *P1, *P2 ) )
            {
                vector3 PlaneNormal;
                f32     PlaneD;

                vector3 P1MinusP0 = (*P1)-(*P0);
                vector3 P2MinusP0 = (*P2)-(*P0);

                // Calculate non-normalized normal vector
                PlaneNormal = v3_Cross( P1MinusP0, P2MinusP0 );

                f32 PlaneNormalDotClusterRayStartPlusPlaneD;
                f32 PlaneNormalDotClusterRayDelta;
                f32 PlaneIntersectT;

                if (!bDoubleSided)
                {
                    // Check if we are moving away from the triangle
                    PlaneNormalDotClusterRayDelta = PlaneNormal.Dot( ClusterRayDelta );
                    if( PlaneNormalDotClusterRayDelta >= 0 )
                        continue;

                    // Compute PlaneD
                    PlaneD = -PlaneNormal.Dot( *P0 );

                    // Check if ray end is in front of triangle
                    if( PlaneNormal.Dot(ClusterRayEnd)+PlaneD > 0 )
                        continue;

                    // Check if ray start is behind triangle
                    PlaneNormalDotClusterRayStartPlusPlaneD = PlaneNormal.Dot(ClusterRayStart) + PlaneD;
                    if( PlaneNormalDotClusterRayStartPlusPlaneD < 0 )
                        continue;

                    // SB: 2/22/05
                    // NOTE: Due to float precision from almost degenerate tris (we are using the artists render 
                    //       tris afterall) this intersect value may still be less than 0 or greater than 1, so a 
                    //       final interval test must still be performed
                    PlaneIntersectT = -PlaneNormalDotClusterRayStartPlusPlaneD / PlaneNormalDotClusterRayDelta;
                }
                else
                {
                    PlaneNormalDotClusterRayDelta = PlaneNormal.Dot( ClusterRayDelta );
                    PlaneD = -PlaneNormal.Dot( *P0 );
                    PlaneNormalDotClusterRayStartPlusPlaneD = PlaneNormal.Dot(ClusterRayStart) + PlaneD;
                    PlaneIntersectT = -PlaneNormalDotClusterRayStartPlusPlaneD / PlaneNormalDotClusterRayDelta;
                }

                // Check if intersection is not in the interval of the test
                if( ( PlaneIntersectT < 0.0f ) || ( PlaneIntersectT > 1.0f ) )
                    continue;

                // Compute intersect point between ClusterRay and triangle plane               
                vector3 HP = ClusterRayStart + PlaneIntersectT*ClusterRayDelta;

                // Confirm that intersection point is inside triangle bounds.  The order 
                // of testing is a bit twisted to make use of data already available.
                // For the first two edgenormals we are also testing the intersection point
                // against the opposite point boundary.
                f32 HPDist;
                vector3 EdgeNormal;
                vector3 HPMinusP0 = HP - (*P0);

                // Test P0->P1 edge
                EdgeNormal = PlaneNormal.Cross( P1MinusP0 );
                HPDist = EdgeNormal.Dot( HPMinusP0 );
                if( (HPDist < 0.0f) || (HPDist > EdgeNormal.Dot(P2MinusP0)) )
                    continue;

                // Test P0->P2 edge
                EdgeNormal = PlaneNormal.Cross( P2MinusP0 );
                HPDist = EdgeNormal.Dot( HPMinusP0 );
                if( (HPDist > 0.0f) || (HPDist < EdgeNormal.Dot(P1MinusP0)) )
                    continue;

                // Test P1->P2 edge
                EdgeNormal = PlaneNormal.Cross( (*P2) - (*P1) );
                if( EdgeNormal.Dot( HP - (*P1) ) < 0.0f )
                    continue;

                {
                    // Compute the Primitive Key
                    s32 PrimKey;
                    RigidGeom_SetPrimKey( PrimKey, TRUE, iC, k );

                    // Transform back into worldspace
                    plane TrianglePlane( *P0, *P1, *P2 );
                    TrianglePlane.Transform( *pCurrentL2W );
                    HP = *pCurrentL2W * HP;

                    // KSS -- make sure there is some amount of error allowance
                    ASSERT( (PlaneIntersectT >= -0.001f) && (PlaneIntersectT <= 1.001f) );

                    f32 T = PlaneIntersectT;
                        T = ClusterRayT0 + T*(ClusterRayT1-ClusterRayT0);
                        T = LocalRayT0 + T*(LocalRayT1-LocalRayT0);

                    if( T<0 ) T = 0;
                    if( T>1 ) T = 1;

                    // Build collision entry
                    collision_mgr::collision TempCollision(
                        T,
                        HP,
                        TrianglePlane,
                        TrianglePlane,
                        Guid,
                        PrimKey,
                        PRIMITIVE_STATIC_TRIANGLE,
                        FALSE,
                        MAX( P0->GetY(), MAX( P1->GetY(), P2->GetY() ) ),
                        Cluster.MaterialInfo.SoundType );

                    // Handle it off to the collision mgr
                    g_CollisionMgr.RecordCollision( TempCollision );

                    // If we are doing LOS then we are done!
                    if( g_CollisionMgr.IsStopOnFirstCollision() )
                        return;
                }
            }
        }
    }
}

//==============================================================================

#ifdef TARGET_PC
s32 RPC_COUNT=0;
void RigidGeom_RayVsPCHiPoly(      guid        Guid, 
                               const bbox&      WorldBBox,
                               const u64         MeshMask,
                               const matrix4*    pL2W, 
                               const rigid_geom& RigidGeom )
{
    s32 k;

#ifndef X_RETAIL
    TRI_STATS[0]++;
#endif
                    extern xbool COLL_DISPLAY_OBJECTS;
                    if( COLL_DISPLAY_OBJECTS)
                    {
                        RPC_COUNT++;
                        if( RPC_COUNT == 800*5*2 )
                            BREAK;
                    }

    //
    // Dig out Ray and clip to world bbox
    //
    vector3 WorldRayStart;
    vector3 WorldRayEnd;
    f32     LocalRayT0;
    f32     LocalRayT1;
    {

        const collision_mgr::dynamic_ray& Ray = g_CollisionMgr.GetDynamicRay();
        if( !ClipLineToBBoxWithSafeSpace( WorldBBox, Ray.Start, Ray.End, LocalRayT0, LocalRayT1 ) )
            return;

        vector3 Direction = Ray.End - Ray.Start;
        WorldRayStart = Ray.Start + LocalRayT0*(Direction);
        WorldRayEnd   = Ray.Start + LocalRayT1*(Direction);
    }

    // Which collision resolution to use?
    ASSERT( g_CollisionMgr.IsUsingHighPoly() );
    ASSERT( RigidGeom.m_Collision.nHighClusters );

    const collision_data& Coll = RigidGeom.m_Collision;

    // For every bone...
    s32 iCurrentBone = -1;
    const matrix4* pCurrentL2W = NULL;
    vector3 LocalRayStart;
    vector3 LocalRayEnd;
    vector3 LocalRayDelta;
    bbox    LocalRayBBox;
    vector3 ClusterRayStart;
    vector3 ClusterRayEnd;
    vector3 ClusterRayDelta;
    bbox    ClusterRayBBox;
    f32     ClusterRayT0;
    f32     ClusterRayT1;

    // Loop thru the clusters and apply them if:
    //  - mesh is active
    //  - cluster uses the current bone
    for( s32 iC = 0; iC < Coll.nHighClusters; iC++ )
    {
        const collision_data::high_cluster& Cluster = Coll.pHighCluster[iC];
#ifndef X_RETAIL
        TRI_STATS[1]++;
#endif
        //
        // Do quick checks to see if we should be skipping this mesh or material
        //
        {
            u64 Bit = 1 << Cluster.iMesh;
            if( !(MeshMask & Bit) )
                continue;

            if( (Cluster.MaterialInfo.SoundType == object::MAT_TYPE_GLASS) && 
                (g_CollisionMgr.IsIgnoringGlass()) )
                continue;
        }
#ifndef X_RETAIL
        TRI_STATS[2]++;
#endif
        //
        // Update local ray info
        //
        if( Cluster.iBone != iCurrentBone )
        {
#ifndef X_RETAIL
            TRI_STATS[3]++;
#endif
            iCurrentBone = Cluster.iBone;

            // Get ptr to the current matrix
            pCurrentL2W = &pL2W[iCurrentBone];

            // Transform ray into space of bone.  Rather than InvertRT the matrix
            // we're just going to rework the transform.
            vector3 Translation = pCurrentL2W->GetTranslation();
            vector3 T,C1,C2,C3;
            pCurrentL2W->GetColumns(C1,C2,C3);

            T  = WorldRayStart - Translation;
            LocalRayStart.GetX() = T.Dot(C1);
            LocalRayStart.GetY() = T.Dot(C2);
            LocalRayStart.GetZ() = T.Dot(C3);

            T  = WorldRayEnd - Translation;
            LocalRayEnd.GetX() = T.Dot(C1);
            LocalRayEnd.GetY() = T.Dot(C2);
            LocalRayEnd.GetZ() = T.Dot(C3);

            LocalRayDelta = LocalRayEnd - LocalRayStart;
            LocalRayBBox.Set(LocalRayStart,LocalRayEnd);
            LocalRayBBox.Inflate(1,1,1);
        }

        //
        // Check if local ray bbox intersects cluster bbox and if ray intersects
        //
        {
            if( !LocalRayBBox.Intersect( Cluster.BBox ) )
            {
#ifndef X_RETAIL
                TRI_STATS[4]++;
#endif
                continue;
            }
        
            if( !ClipLineToBBoxWithSafeSpace( Cluster.BBox, LocalRayStart, LocalRayEnd, ClusterRayT0, ClusterRayT1 ) )
            {
#ifndef X_RETAIL
                TRI_STATS[5]++;
#endif
                continue;
            }

            ClusterRayStart = LocalRayStart + ClusterRayT0*LocalRayDelta;
            ClusterRayEnd   = LocalRayStart + ClusterRayT1*LocalRayDelta;
            ClusterRayDelta = ClusterRayEnd - ClusterRayStart;
            ClusterRayBBox.Set(ClusterRayStart,ClusterRayEnd);
        }


#ifndef X_RETAIL
        TRI_STATS[6]++;
#endif

        // Apply the cluster to the collision manager.
        const rigid_geom::dlist_pc& DList = RigidGeom.m_System.pPC[Cluster.iDList];
//        extern xbool COLL_DISPLAY_OBJECTS;
//        if( COLL_DISPLAY_OBJECTS )
//            x_DebugMsg("CLUSTER %4d\n",Cluster.nTris);

        // 0.133
        for( k = 0; k < Cluster.nTris; k++ )
        {
#ifndef X_RETAIL
            TRI_STATS[7]++;
#endif

            const u32 Offset  = Coll.pHighIndexToVert0[ Cluster.iOffset + k ] & INDEX_MASK;
            //
            // Setup ptrs to positions
            //
            const vector3* P0;
            const vector3* P1;
            const vector3* P2;
            P0 = (vector3*)(&DList.pVert[ DList.pIndices[Offset+0] ].Pos);
            P1 = (vector3*)(&DList.pVert[ DList.pIndices[Offset+1] ].Pos);
            P2 = (vector3*)(&DList.pVert[ DList.pIndices[Offset+2] ].Pos);

            if( ClusterRayBBox.IntersectTriBBox( *P0, *P1, *P2 ) )
            {
#ifndef X_RETAIL
                TRI_STATS[8]++;
#endif

                vector3 PlaneNormal;
                f32     PlaneD;

                vector3 P1MinusP0 = (*P1)-(*P0);
                vector3 P2MinusP0 = (*P2)-(*P0);

                // Calculate non-normalized normal vector
                PlaneNormal = v3_Cross( P1MinusP0, P2MinusP0 );

                f32 PlaneNormalDotClusterRayStartPlusPlaneD;
                f32 PlaneNormalDotClusterRayDelta;
                f32 PlaneIntersectT;

                xbool bDoubleSided = !!(Cluster.MaterialInfo.Flags & collision_data::mat_info::FLAG_DOUBLESIDED);
                
#ifdef X_EDITOR
                // SB: Allow user to select transparent triangles from either side in editor select mode!
                if( g_CollisionMgr.IsEditorSelectRay() )
                    bDoubleSided |= !!(Cluster.MaterialInfo.Flags & collision_data::mat_info::FLAG_TRANSPARENT);
#endif                

                if (!bDoubleSided)
                {
                    // Check if we are moving away from the triangle
                    PlaneNormalDotClusterRayDelta = PlaneNormal.Dot( ClusterRayDelta );
                    if( PlaneNormalDotClusterRayDelta >= 0 )
                        continue;

                    // Compute PlaneD
                    PlaneD = -PlaneNormal.Dot( *P0 );

                    // Check if ray end is in front of triangle
                    if( PlaneNormal.Dot(ClusterRayEnd)+PlaneD > 0 )
                        continue;

                    // Check if ray start is behind triangle
                    PlaneNormalDotClusterRayStartPlusPlaneD = PlaneNormal.Dot(ClusterRayStart) + PlaneD;
                    if( PlaneNormalDotClusterRayStartPlusPlaneD < 0 )
                        continue;

                    PlaneIntersectT = -PlaneNormalDotClusterRayStartPlusPlaneD / PlaneNormalDotClusterRayDelta;
                }
                else
                {
                    PlaneNormalDotClusterRayDelta = PlaneNormal.Dot( ClusterRayDelta );
                    PlaneD = -PlaneNormal.Dot( *P0 );
                    PlaneNormalDotClusterRayStartPlusPlaneD = PlaneNormal.Dot(ClusterRayStart) + PlaneD;
                    PlaneIntersectT = -PlaneNormalDotClusterRayStartPlusPlaneD / PlaneNormalDotClusterRayDelta;

                    // Check if intersection is not in the interval of the test
                    if ((PlaneIntersectT < 0) || (PlaneIntersectT > 1))
                        continue;
                }

                vector3 HP = ClusterRayStart + PlaneIntersectT*ClusterRayDelta;

                // Confirm that intersection point is inside triangle bounds.  The order 
                // of testing is a bit twisted to make use of data already available.
                // For the first two edgenormals we are also testing the intersection point
                // against the opposite point boundary.
                f32 HPDist;
                vector3 EdgeNormal;
                vector3 HPMinusP0 = HP - (*P0);

                // Test P0->P1 edge
                EdgeNormal = PlaneNormal.Cross( P1MinusP0 );
                HPDist = EdgeNormal.Dot( HPMinusP0 );
                if( (HPDist < 0.0f) || (HPDist > EdgeNormal.Dot(P2MinusP0)) )
                {
#ifndef X_RETAIL
                    TRI_STATS[13]++;
#endif
                    continue;
                }

                // Test P0->P2 edge
                EdgeNormal = PlaneNormal.Cross( P2MinusP0 );
                HPDist = EdgeNormal.Dot( HPMinusP0 );
                if( (HPDist > 0.0f) || (HPDist < EdgeNormal.Dot(P1MinusP0)) )
                {
#ifndef X_RETAIL
                    TRI_STATS[14]++;
#endif
                    continue;
                }

                // Test P1->P2 edge
                EdgeNormal = PlaneNormal.Cross( (*P2) - (*P1) );
                if( EdgeNormal.Dot( HP - (*P1) ) < 0.0f )
                {
#ifndef X_RETAIL
                    TRI_STATS[15]++;    
#endif
                    continue;
                }

                {
#ifndef X_RETAIL
                    TRI_STATS[16]++;
#endif
                    // Compute the Primitive Key
                    s32 PrimKey;
                    RigidGeom_SetPrimKey( PrimKey, TRUE, iC, k );

                    // Transform back into worldspace
                    plane TrianglePlane( *P0, *P1, *P2 );
                    TrianglePlane.Transform( *pCurrentL2W );
                    HP = *pCurrentL2W * HP;

                    // Work back and solve what the T value should be for the original ray
                    f32 T = PlaneIntersectT;
                        T = ClusterRayT0 + T*(ClusterRayT1-ClusterRayT0);
                        T = LocalRayT0 + T*(LocalRayT1-LocalRayT0);

                    // Build collision entry
                    collision_mgr::collision TempCollision(
                        T,
                        HP,
                        TrianglePlane,
                        TrianglePlane,
                        Guid,
                        PrimKey,
                        PRIMITIVE_STATIC_TRIANGLE,
                        FALSE,
                        MAX( P0->GetY(), MAX( P1->GetY(), P2->GetY() ) ),
                        Cluster.MaterialInfo.SoundType );

                    // Handle it off to the collision mgr
                    g_CollisionMgr.RecordCollision( TempCollision );

                    // If we are doing LOS then we are done!
                    if( g_CollisionMgr.IsStopOnFirstCollision() )
                        return;
                }
            }
        }
    }
}
#endif
//==============================================================================

void RigidGeom_ApplyCollision(       guid        Guid, 
                               const bbox&       BBox,
                               const u64         MeshMask,
                               const matrix4*    pL2W, 
                               const rigid_geom* pRigidGeom )
{
    s32 i, j, k;

    //
    // If we made it to here we should only be checking high poly!
    //
    ASSERT( g_CollisionMgr.IsUsingHighPoly() );

    //
    // Is the geometry available?  If not, apply bbox
    //
    if( pRigidGeom == NULL )
    {
        g_CollisionMgr.StartApply( Guid );
        g_CollisionMgr.ApplyAABBox( BBox );    
        g_CollisionMgr.EndApply();
        return;    
    }
   
    const rigid_geom& RigidGeom = *pRigidGeom;

    //
    // Did we compile high poly collision?
    //
    if( RigidGeom.m_Collision.nHighClusters == 0 )
    {
        return;
    }

    #ifdef TARGET_PS2
    //
    // Check for Ray vs. PS2 High poly
    //
    if( g_CollisionMgr.IsUsingHighPoly() &&
        ((g_CollisionMgr.GetDynamicPrimitive()==PRIMITIVE_DYNAMIC_RAY) ||
         (g_CollisionMgr.GetDynamicPrimitive()==PRIMITIVE_DYNAMIC_LOS))
      )
    {
        RigidGeom_RayVsPS2HiPoly( Guid, BBox, MeshMask, pL2W, *pRigidGeom );
        return;
    }
    #endif

    #ifdef TARGET_PC
    //
    // Check for Ray vs. PC High poly
    //
    if( g_CollisionMgr.IsUsingHighPoly() &&
        ((g_CollisionMgr.GetDynamicPrimitive()==PRIMITIVE_DYNAMIC_RAY) ||
         (g_CollisionMgr.GetDynamicPrimitive()==PRIMITIVE_DYNAMIC_LOS))
      )
    {
        RigidGeom_RayVsPCHiPoly( Guid, BBox, MeshMask, pL2W, *pRigidGeom );
        return;
    }
    #endif

    // Which collision resolution to use?
    if( g_CollisionMgr.IsUsingHighPoly() )
    {
        ASSERT( RigidGeom.m_Collision.nHighClusters );
        const collision_data& Coll = RigidGeom.m_Collision;

        // For every bone...
        for( i = 0; i < RigidGeom.m_nBones; i++ )
        {
            // Compute the inverse matrix.
            const matrix4& L2W = pL2W[i];
            const matrix4  W2L = m4_InvertRT( L2W );

            g_CollisionMgr.StartApply( Guid, L2W, W2L );

            // Loop thru the clusters and apply them if:
            //  - mesh is active
            //  - cluster uses the current bone

            for( j = 0; j < Coll.nHighClusters; j++ )
            {
                const collision_data::high_cluster& Cluster = Coll.pHighCluster[j];

                if( (Cluster.MaterialInfo.SoundType == object::MAT_TYPE_GLASS) && 
                    (g_CollisionMgr.IsIgnoringGlass()) )
                    continue;

                xbool bDoubleSided = !!(Cluster.MaterialInfo.Flags & collision_data::mat_info::FLAG_DOUBLESIDED);

                if( Cluster.iBone != i )
                    continue;

                u64 Bit = 1 << Cluster.iMesh;
                if( !(MeshMask & Bit) )
                    continue;

                #ifdef TARGET_XBOX
                {
                    // Apply the cluster to the collision manager.
                    const rigid_geom::dlist_xbox& DList = RigidGeom.m_System.pXbox[Cluster.iDList];
                    ASSERT( DList.iColor <= RigidGeom.m_nVertices );

                    for( k = 0; k < Cluster.nTris; k++ )
                    {
                        const u32 Offset  = Coll.pHighIndexToVert0[ Cluster.iOffset + k ] & INDEX_MASK;

                        s32 PrimKey;
                        RigidGeom_SetPrimKey( PrimKey, TRUE, j, k );

                        g_CollisionMgr.ApplyTriangle(
                            DList.pVert[ DList.pIndices[Offset+0] ].Pos, 
                            DList.pVert[ DList.pIndices[Offset+1] ].Pos, 
                            DList.pVert[ DList.pIndices[Offset+2] ].Pos, 
                            Cluster.MaterialInfo.SoundType,
                            PrimKey );

                        // Double Sided single plane so apply collision for both sides!
                        if ( bDoubleSided )
                        {
                            g_CollisionMgr.ApplyTriangle(
                                DList.pVert[ DList.pIndices[Offset+2] ].Pos, 
                                DList.pVert[ DList.pIndices[Offset+1] ].Pos, 
                                DList.pVert[ DList.pIndices[Offset+0] ].Pos, 
                                Cluster.MaterialInfo.SoundType,
                                PrimKey );
                        }
                    }
                }
                #endif

                #ifdef TARGET_PC
                {
                    // Apply the cluster to the collision manager.
                    const rigid_geom::dlist_pc& DList = RigidGeom.m_System.pPC[Cluster.iDList];

                    for( k = 0; k < Cluster.nTris; k++ )
                    {
                        const u32 Offset  = Coll.pHighIndexToVert0[ Cluster.iOffset + k ] & INDEX_MASK;

                        s32 PrimKey;
                        RigidGeom_SetPrimKey( PrimKey, TRUE, j, k );

                        g_CollisionMgr.ApplyTriangle(
                            DList.pVert[ DList.pIndices[Offset+0] ].Pos, 
                            DList.pVert[ DList.pIndices[Offset+1] ].Pos, 
                            DList.pVert[ DList.pIndices[Offset+2] ].Pos, 
                            Cluster.MaterialInfo.SoundType,
                            PrimKey );

                        // Double Sided single plane so apply collision for both sides!
                        if ( bDoubleSided )
                        {
                            g_CollisionMgr.ApplyTriangle(
                                DList.pVert[ DList.pIndices[Offset+2] ].Pos, 
                                DList.pVert[ DList.pIndices[Offset+1] ].Pos, 
                                DList.pVert[ DList.pIndices[Offset+0] ].Pos, 
                                Cluster.MaterialInfo.SoundType,
                                PrimKey );
                        }
                    }
                }
                #endif

                #ifdef TARGET_PS2
                {
                    // Apply the cluster to the collision manager.
                    const rigid_geom::dlist_ps2& DList = RigidGeom.m_System.pPS2[Cluster.iDList];

                    for( k = 0; k < Cluster.nTris; k++ )
                    {
                        u32 Offset  = Coll.pHighIndexToVert0[ Cluster.iOffset + k ];

                        s32 PrimKey;
                        RigidGeom_SetPrimKey( PrimKey, TRUE, j, k );

                        if( Offset & 0x8000 )
                        {
                            Offset &= INDEX_MASK;

                            g_CollisionMgr.ApplyTriangle(
                                *((vector3*)(&DList.pPosition[Offset+2])),
                                *((vector3*)(&DList.pPosition[Offset+1])),
                                *((vector3*)(&DList.pPosition[Offset+0])),
                                Cluster.MaterialInfo.SoundType,
                                 PrimKey );
                            if (bDoubleSided)
                            {
                                g_CollisionMgr.ApplyTriangle(
                                    *((vector3*)(&DList.pPosition[Offset+0])),
                                    *((vector3*)(&DList.pPosition[Offset+1])),
                                    *((vector3*)(&DList.pPosition[Offset+2])),
                                    Cluster.MaterialInfo.SoundType,
                                    PrimKey );
                            }
                        }
                        else
                        {
                            g_CollisionMgr.ApplyTriangle(
                                *((vector3*)(&DList.pPosition[Offset+0])),
                                *((vector3*)(&DList.pPosition[Offset+1])),
                                *((vector3*)(&DList.pPosition[Offset+2])),
                                Cluster.MaterialInfo.SoundType,
                                 PrimKey );

                            if (bDoubleSided)
                            {
                                g_CollisionMgr.ApplyTriangle(
                                    *((vector3*)(&DList.pPosition[Offset+2])),
                                    *((vector3*)(&DList.pPosition[Offset+1])),
                                    *((vector3*)(&DList.pPosition[Offset+0])),
                                    Cluster.MaterialInfo.SoundType,
                                    PrimKey );
                            }
                        }
                    }
                }
                #endif

            }

            g_CollisionMgr.EndApply();    
        }
    }
}

//==============================================================================

#ifndef X_RETAIL
void RigidGeom_RenderCollision( const matrix4*  pBone, 
                                const rigid_geom*     pRigidGeom, 
                                xbool           bRenderHigh,
                                u64             LODMask)
{
    static const s32 LO_COLOR = 128;
    static const s32 HI_COLOR = 255;

    if( pRigidGeom == NULL )
        return;

    s32         ZBIAS = 5;
    s32         i, j;
    random      R;
    R.srand( ((u32)(pRigidGeom)) & 0x0000FFFF );

    if( bRenderHigh )
    {
        if( pRigidGeom->m_Collision.nHighClusters )
        {
            const collision_data& Coll = pRigidGeom->m_Collision;

            for( i = 0; i < Coll.nHighClusters; i++ )
            {      
                collision_data::high_cluster& Cluster = Coll.pHighCluster[i];

                u64 Bit = 1 << Cluster.iMesh;
                if( !(LODMask & Bit) )
                    continue;

                if( 1 )
                {
                    draw_SetL2W( pBone[ Cluster.iBone ] );

                    xcolor C = xcolor( R.irand( LO_COLOR, HI_COLOR ),
                                       R.irand( LO_COLOR, HI_COLOR ),
                                       R.irand( LO_COLOR, HI_COLOR ) );

                    draw_BBox( Cluster.BBox, C );
                }

                if( 0 )
                {
                    draw_SetL2W( pBone[ Cluster.iBone ] );

                    // Draw all faces in a solid color based on cluster / display list.
                    draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA );
                    draw_SetZBias( ZBIAS );

                    #ifdef TARGET_XBOX
                    {
                        rigid_geom::dlist_xbox& DList = pRigidGeom->m_System.pXbox[Cluster.iDList];
                        ASSERT( DList.iColor <= pRigidGeom->m_nVertices );

                        for( j = 0; j < Cluster.nTris; j++ )
                        {
                            draw_Color( xcolor( R.irand( LO_COLOR, HI_COLOR ), 
                                                R.irand( LO_COLOR, HI_COLOR ), 
                                                R.irand( LO_COLOR, HI_COLOR ), 255 ) );

                            s32 Offset = Coll.pHighIndexToVert0[ Cluster.iOffset + j ] & INDEX_MASK;
                            draw_Vertex( DList.pVert[ DList.pIndices[Offset+0] ].Pos );
                            draw_Vertex( DList.pVert[ DList.pIndices[Offset+1] ].Pos );
                            draw_Vertex( DList.pVert[ DList.pIndices[Offset+2] ].Pos );
                        }
                    }
                    #endif

                    #ifdef TARGET_PC
                    {
                        rigid_geom::dlist_pc& DList = pRigidGeom->m_System.pPC[Cluster.iDList];

                        for( j = 0; j < Cluster.nTris; j++ )
                        {
                            draw_Color( xcolor( R.irand( LO_COLOR, HI_COLOR ), 
                                                R.irand( LO_COLOR, HI_COLOR ), 
                                                R.irand( LO_COLOR, HI_COLOR ), 255 ) );

                            s32 Offset = Coll.pHighIndexToVert0[ Cluster.iOffset + j ] & INDEX_MASK;
                            draw_Vertex( DList.pVert[ DList.pIndices[Offset+0] ].Pos );
                            draw_Vertex( DList.pVert[ DList.pIndices[Offset+1] ].Pos );
                            draw_Vertex( DList.pVert[ DList.pIndices[Offset+2] ].Pos );
                        }
                    }
                    #endif

                    #ifdef TARGET_PS2
                    {
                        rigid_geom::dlist_ps2& DList = pRigidGeom->m_System.pPS2[Cluster.iDList];

                        for( j = 0; j < Cluster.nTris; j++ )
                        {
                            draw_Color( xcolor( R.irand( LO_COLOR, HI_COLOR ), 
                                                R.irand( LO_COLOR, HI_COLOR ), 
                                                R.irand( LO_COLOR, HI_COLOR ), 255 ) );

                            s32 Offset = Coll.pHighIndexToVert0[ Cluster.iOffset + j ] & INDEX_MASK;
                            draw_Vertex( *((vector3*)(DList.pPosition + (Offset+0))) );
                            draw_Vertex( *((vector3*)(DList.pPosition + (Offset+1))) );
                            draw_Vertex( *((vector3*)(DList.pPosition + (Offset+2))) );
                        }
                    }
                    #endif

                    draw_End();
                    draw_ClearL2W();
                    draw_SetZBias( 0 );
                }
            }  
        }
    }
    else
    {
        if( pRigidGeom->m_Collision.nLowClusters )
        {
            const collision_data& Coll = pRigidGeom->m_Collision;

            random R;
            R.srand( ((u32)(pRigidGeom)) & 0x0000FFFF );
            
            for( i=0; i<Coll.nLowClusters; i++ )
            {
                collision_data::low_cluster& CL = Coll.pLowCluster[i];

                draw_SetL2W( pBone[ CL.iBone ] );

                // Draw all faces in a solid color based on cluster / display list.
                draw_Begin( DRAW_TRIANGLES );
                draw_SetZBias( ZBIAS );

                xcolor CC = xcolor( R.irand( LO_COLOR, HI_COLOR ),
                                    R.irand( LO_COLOR, HI_COLOR ),
                                    R.irand( LO_COLOR, HI_COLOR ),
                                    255 );

                for( j=0; j<CL.nQuads; j++ )
                {
                    collision_data::low_quad& QD = Coll.pLowQuad[ CL.iQuadOffset + j ];

                    vector3* P0 = &Coll.pLowVector[ CL.iVectorOffset + QD.iP[0] ];
                    vector3* P1 = &Coll.pLowVector[ CL.iVectorOffset + QD.iP[1] ];
                    vector3* P2 = &Coll.pLowVector[ CL.iVectorOffset + QD.iP[2] ];
                    vector3* P3 = &Coll.pLowVector[ CL.iVectorOffset + QD.iP[3] ];

                    f32 I = R.frand(0.5f,1.0f);
                    xcolor C = CC;
                    C.R = (u8)( CC.R * I );
                    C.G = (u8)( CC.G * I );
                    C.B = (u8)( CC.B * I );
                    C.A = 255;

                    draw_Color( C );

                    draw_Vertex( *P0 );
                    draw_Vertex( *P1 );
                    draw_Vertex( *P2 );

                    if( QD.Flags )
                    {
                        draw_Vertex( *P0 );
                        draw_Vertex( *P2 );
                        draw_Vertex( *P3 );
                    }
                }

                draw_End();
                draw_ClearL2W();
                draw_SetZBias( 0 );
            }
        }
        else
        {
            bbox BBox = pRigidGeom->m_Collision.BBox;
            BBox.Transform( *pBone );
            draw_ClearL2W();
            draw_BBox( BBox, XCOLOR_YELLOW );
        }
    }
}
#endif // X_RETAIL

//==============================================================================

void RigidGeom_GatherToPolyCache(       guid        Guid, 
                               const bbox&       BBox,
                                     u64         MeshMask,
                               const matrix4*    pL2W, 
                               const rigid_geom* pRigidGeom )
{
    (void)BBox;
    if( (pRigidGeom==NULL) || (pRigidGeom->m_Collision.nLowClusters==0) )
        return;

    g_PolyCache.GatherCluster( pRigidGeom->m_Collision, pL2W, MeshMask, Guid );
}

//==============================================================================
