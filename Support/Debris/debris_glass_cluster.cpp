//=============================================================================
//  DEBRIS_GLASS_CLUSTER.CPP
//=============================================================================

//=============================================================================
// INCLUDES
//=============================================================================
#include "debris_glass_cluster.hpp"
#include "e_Draw.hpp"
#include "e_ScratchMem.hpp"
#include "..\Support\GameLib\StatsMgr.hpp"
#include "Objects\PropSurface.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "audiomgr\AudioMgr.hpp"

#define UV_TO_FIXED(V2)     (u32)((((u32)(((V2).X)*16))<<16) | ((u32)(((V2).Y)*16)))

//=============================================================================
// CONSTANTS
//=============================================================================
static const f32 k_TESSELATION_AREA_THRESHOLD           = (50*50);
static const f32 k_TESSELATION_MAX_EDGE_LENGTH_SQUARED  = (40*40);
static const s32 k_MAX_COLLIDERS                        = 6;

//=============================================================================
// OBJECT DESC.
//=============================================================================
static struct debris_glass_cluster_desc : public object_desc
{
    debris_glass_cluster_desc( void ) : object_desc( 
        object::TYPE_DEBRIS_GLASS_CLUSTER, 
        "DebrisGlassCluster", 
        "EFFECTS",
        object::ATTR_NEEDS_LOGIC_TIME   |
        object::ATTR_RENDERABLE         | 
        object::ATTR_TRANSPARENT        |
        object::ATTR_NO_RUNTIME_SAVE,
        FLAGS_IS_DYNAMIC  )
    {}

    virtual object* Create          ( void )
    {
        return new debris_glass_cluster;
    }

} s_debris_glass_cluster_desc;

//=============================================================================================

const object_desc&  debris_glass_cluster::GetTypeDesc ( void ) const
{
    return s_debris_glass_cluster_desc;
}

//=============================================================================================

const object_desc&  debris_glass_cluster::GetObjectType ( void )
{
    return s_debris_glass_cluster_desc;
}

debris_glass_cluster::debris_glass_cluster()
{
    m_nShards           = 0;
    m_TotalTime         = 0;

    m_BBox.Set( vector3(0,0,0), 5 );
}


debris_glass_cluster::~debris_glass_cluster()
{

}

//=============================================================================
//
//=============================================================================
void debris_glass_cluster::OnInit( void )
{
    object::OnInit();
}

//=============================================================================
//
//=============================================================================
bbox debris_glass_cluster::GetLocalBBox( void ) const
{
    return m_BBox;
}


void debris_glass_cluster::TesselateIntoShards ( const play_surface*       pPlaySurface, 
                                                 object::detail_tri*       pTriStack,
                                                 s32                       iRead,
                                                 s32                       iWrite,
                                                 s32                       iMaxStackSize,
                                                 f32*                      pRawTriStartTime )
{
    if (NULL == pPlaySurface)
        return;
    if (NULL == pTriStack)
        return;
    if (iWrite == iRead)
        return;
    if (iMaxStackSize <= 0)
        return;

    // ASSUMPTION; iRead == 0, and iWrite > 0
    if (iRead != 0)
        return;

    s32 nShardsInStack = iWrite;

    vector3 PainPos     = m_SourcePain.GetImpactPoint();
    vector3 PainDir     = m_SourcePain.GetDirection();
    f32     PainForce   = m_SourcePain.GetForce();
    s32     HitType     = m_SourcePain.GetHitType();

    static f32 ErodeTime = 400.0f;  // Four meters per second
    
    static f32 ShardMaxVelocity = 1000;
    static f32 ShardMaxVelocitySquared = ShardMaxVelocity * ShardMaxVelocity;

    f32     PainDamageRadius = 50.0f;
    f32     PainDamageRadiusSquared;

    const pain_profile* pPainProfile = m_SourcePain.GetPainHandle().GetPainProfile();
    if( pPainProfile && pPainProfile->m_bSplash )
    {
        PainDamageRadius = pPainProfile->m_DamageFarDist;
    }

    xbool  bSplash = pPainProfile->m_bSplash;

    
    // Do some setup based on hit type
    switch(HitType)
    {
    case 1:
        // sniper round or other extremely fast direct projectile
        PainForce = 400.0f;
        PainDamageRadius = 5.0f;
        break;
    case 2:
        // standard bullet
        PainForce = 800.0f;
        PainDamageRadius = 50.0f;
        break;
    case 3:
        // shotgun style blast
        PainForce = 1200.0f;
        PainDamageRadius = 90.0f;
        break;
    case 4:
        // explosive impact ie: grenade, meson secondary, etc...
        PainForce = 2000.0f;
        PainDamageRadius = MAX(PainDamageRadius,400.0f);
        break;
    }

    PainDamageRadiusSquared = PainDamageRadius*PainDamageRadius;

    // While triangles remain, and we haven't filled the shard buffer...
    while ((iRead != iWrite) && (m_nShards < MAX_SHARDS))
    {
        // Pull tri from TOS
        object::detail_tri& DetailTri = pTriStack[ iRead ];

        iRead = (iRead+1)%iMaxStackSize;

        nShardsInStack--;

        vector3 Pos[3];

        Pos[0] = DetailTri.Vertex[0];
        Pos[1] = DetailTri.Vertex[1];
        Pos[2] = DetailTri.Vertex[2];

        vector3 Center = Pos[0] + Pos[1] + Pos[2];
        Center /= 3.0f;

        vector3 ArmA, ArmB;
        ArmA = Pos[1] - Pos[0];
        ArmB = Pos[2] - Pos[0];

        // Decide if it should be split
        xbool bSplit = FALSE;

        // If it is too big, we want to split it
        f32 Area = (ArmA.Cross( ArmB )).Length() / 2;        
        if (Area > k_TESSELATION_AREA_THRESHOLD)
            bSplit = TRUE;

        // If there is only enough room in the shard buffer for the
        // triangles remaining in the stack, then we must not split
        s32 nShardSlotsOpen = MAX_SHARDS - m_nShards - 1;
        if (nShardSlotsOpen <= nShardsInStack)
            bSplit = FALSE;

        // If there are not enough slots in the stack for 4 triangles
        // then we must not split, otherwise we would lose tris
        if (nShardsInStack >= iMaxStackSize - 4)
        {
            bSplit = FALSE;
        }

        if ( bSplit )
        {
            //             M[0]
            //    0--------*-------1
            //     \     /  \     /
            //      \   /    \   /
            //       \ /      \ /
            //   M[2] *________* M[1]
            //         \      /    
            //          \    /
            //           \  /
            //            2
            //
            //  0,1,2 are original verts
            //  M[0 thru 2] are the midpoint verts used for subdividing
            //
            vector3     EdgeMidpoint[3];
            vector2     EdgeUV[3];

            //TODO: Color, normal

            f32         T[3];

            T[0] = x_frand( 0.3f,0.7f );
            T[1] = x_frand( 0.3f,0.7f );
            T[2] = x_frand( 0.3f,0.7f );

            EdgeMidpoint[0] = (Pos[1] - Pos[0]) * T[0] + Pos[0];
            EdgeMidpoint[1] = (Pos[2] - Pos[1]) * T[1] + Pos[1];
            EdgeMidpoint[2] = (Pos[0] - Pos[2]) * T[2] + Pos[2];

            EdgeUV[0] = (DetailTri.UV[1] - DetailTri.UV[0]) * T[0] + DetailTri.UV[0];
            EdgeUV[1] = (DetailTri.UV[2] - DetailTri.UV[1]) * T[1] + DetailTri.UV[1];
            EdgeUV[2] = (DetailTri.UV[0] - DetailTri.UV[2]) * T[2] + DetailTri.UV[2];

            // Push on 4 new triangles (or until the stack gets full)
            if (((iWrite+1)%iMaxStackSize) != iRead)
            {
                object::detail_tri& T = pTriStack[ iWrite ];
                T.Vertex[0] = Pos[0];
                T.Vertex[1] = EdgeMidpoint[0];
                T.Vertex[2] = EdgeMidpoint[2];                
                T.UV[0] = DetailTri.UV[0];
                T.UV[1] = EdgeUV[0];
                T.UV[2] = EdgeUV[2];
                //T.Color[0] = T.Color[1] = T.Color[2] = 0xFFFFFFFF;
                //T.Normal[0] = DetailTri.Normal[0];
                //T.Normal[1] = DetailTri.Normal[1];
                //T.Normal[2] = DetailTri.Normal[2];                                

                iWrite = (iWrite+1)%iMaxStackSize;
                nShardsInStack++;
            }

            if (((iWrite+1)%iMaxStackSize) != iRead)
            {
                object::detail_tri& T = pTriStack[ iWrite ];
                T.Vertex[0] = EdgeMidpoint[0];
                T.Vertex[1] = Pos[1];
                T.Vertex[2] = EdgeMidpoint[1];                
                T.UV[0] = EdgeUV[0];
                T.UV[1] = DetailTri.UV[1];
                T.UV[2] = EdgeUV[1];
                //T.Color[0] = T.Color[1] = T.Color[2] = 0xFFFFFFFF;
                //T.Normal[0] = DetailTri.Normal[0];
                //T.Normal[1] = DetailTri.Normal[1];
                //T.Normal[2] = DetailTri.Normal[2]; 

                iWrite = (iWrite+1)%iMaxStackSize;
                nShardsInStack++;
            }

            if (((iWrite+1)%iMaxStackSize) != iRead)
            {
                object::detail_tri& T = pTriStack[ iWrite ];
                T.Vertex[0] = EdgeMidpoint[2];
                T.Vertex[1] = EdgeMidpoint[1];
                T.Vertex[2] = Pos[2];                
                T.UV[0] = EdgeUV[2];
                T.UV[1] = EdgeUV[1];
                T.UV[2] = DetailTri.UV[2];
                //T.Color[0] = T.Color[1] = T.Color[2] = 0xFFFFFFFF;
                //T.Normal[0] = DetailTri.Normal[0];
                //T.Normal[1] = DetailTri.Normal[1];
                //T.Normal[2] = DetailTri.Normal[2];  

                iWrite = (iWrite+1)%iMaxStackSize;
                nShardsInStack++;
            }

            if (((iWrite+1)%iMaxStackSize) != iRead)
            {
                object::detail_tri& T = pTriStack[ iWrite ];
                T.Vertex[0] = EdgeMidpoint[0];
                T.Vertex[1] = EdgeMidpoint[1];
                T.Vertex[2] = EdgeMidpoint[2];
                T.UV[0] = EdgeUV[0];
                T.UV[1] = EdgeUV[1];
                T.UV[2] = EdgeUV[2];
                //T.Color[0] = T.Color[1] = T.Color[2] = 0xFFFFFFFF;
                //T.Normal[0] = DetailTri.Normal[0];
                //T.Normal[1] = DetailTri.Normal[1];
                //T.Normal[2] = DetailTri.Normal[2];

                iWrite = (iWrite+1)%iMaxStackSize;
                nShardsInStack++;
            }
        }
        else
        {
            // This triangle is ok
            //AddTriangleToShardList
            shard& Shard = m_Shards[ m_nShards ];

            Shard.m_Variation = 0;
            Shard.m_Velocity.Set(0,0,0);
            Shard.m_SpinRate.Set(0,0,0);
            Shard.m_TotalSpin.Set(0,0,0);

            f32 Spin = R_180;

            vector3 Center = Pos[0] + Pos[1] + Pos[2];
            Center /= 3.0f;                

            vector3 DeltaToPain = Center - PainPos;
            f32     DistSquaredToPain = DeltaToPain.LengthSquared();

            pRawTriStartTime[ m_nShards ] = x_sqrt( DistSquaredToPain ) / ErodeTime;

            if (DistSquaredToPain < PainDamageRadiusSquared)
            {
                // Inside blast
                // T=0 at boundary
                // T=1 at center
                Shard.m_StartTime = 0;
                vector3 ShardPainDir;
                if (bSplash)
                    ShardPainDir = Center - PainPos;
                else
                    ShardPainDir = PainDir;

                f32 T = DistSquaredToPain / PainDamageRadiusSquared;
                T = 1.0f-(T*T);

                static f32 ForceScale = 5.0f;

                Shard.m_Velocity = ShardPainDir;
                Shard.m_Velocity.NormalizeAndScale( T );
                Shard.m_Velocity *= (PainForce * ForceScale );

                f32 Len = Shard.m_Velocity.LengthSquared();
                if (Len > ShardMaxVelocitySquared)
                    Shard.m_Velocity.NormalizeAndScale( ShardMaxVelocity );

                Spin *= 3;
            }
            else
            {
                // Outside blast
                
                static f32 VelocityScale = 0.1f;
                f32 Dist = DeltaToPain.Length();
                f32 T = (Dist - PainDamageRadius) / ErodeTime;

                Shard.m_Velocity = DeltaToPain;
                Shard.m_Velocity *= VelocityScale;
                f32 R = 100;
                {
                    f32 x = x_frand(-R,R);
                    f32 y = x_frand(-R,R);
                    f32 z = x_frand(-R,R);
                    Shard.m_Velocity += vector3( x, y, z );
                }
                Shard.m_StartTime = T;
            }
            {   f32 x = x_frand(-Spin,Spin);
                f32 y = x_frand(-Spin,Spin);
                Shard.m_SpinRate.Set( x, y, 0 );// x_frand(-Spin,Spin) );
            }
            Shard.m_Variation = 1.0f;

            m_MinStartTime = MIN(m_MinStartTime, Shard.m_StartTime);
            m_MaxStartTime = MAX(m_MaxStartTime, Shard.m_StartTime);

            s32 Idx = m_nShards*3;

            m_pPos     [ Idx+0 ] = Pos[0];
            //m_pColor   [ Idx+0 ] = 0xFFFFFFFF;
           // m_pUV      [ Idx+0 ] = UV_TO_FIXED(DetailTri.UV[0]);
            m_pUV      [ Idx+0 ] = DetailTri.UV[0];
            m_pLocalPos[ Idx+0 ] = Pos[0] - Center;
            // TODO: m_pLocalPos[ Idx+0 ].GetW() = 1;

            m_pPos     [ Idx+1 ] = Pos[1];
            //m_pColor   [ Idx+1 ] = 0xFFFFFFFF;
           // m_pUV      [ Idx+1 ] = UV_TO_FIXED(DetailTri.UV[1]);
            m_pUV      [ Idx+1 ] = DetailTri.UV[1];
            m_pLocalPos[ Idx+1 ] = Pos[1] - Center;
            // TODO: m_pLocalPos[ Idx+1 ].GetW() = 1;

            m_pPos     [ Idx+2 ] = Pos[2];
            //m_pColor   [ Idx+2 ] = 0xFFFFFFFF;
           // m_pUV      [ Idx+2 ] = UV_TO_FIXED(DetailTri.UV[2]);
            m_pUV      [ Idx+2 ] = DetailTri.UV[2];
            m_pLocalPos[ Idx+2 ] = Pos[2] - Center;
            // TODO: m_pLocalPos[ Idx+2 ].GetW() = 1;

            Shard.m_Pos = Center;

            m_nShards++;
        }
    }
}

//=============================================================================
//
//=============================================================================
void debris_glass_cluster::CreateFromRigidGeom ( play_surface* pPlaySurface, const pain* pPain )
{
    LOG_STAT( k_stats_Debris );

    //  make sure we have some real data first
    ASSERT( pPlaySurface );
    ASSERT( pPain );

    m_SourcePain = *pPain;

    rigid_inst& RigidInst  = pPlaySurface->GetRigidInst();
    rigid_geom* pRigidGeom = RigidInst.GetRigidGeom();

    ASSERT( pRigidGeom );    
    ASSERT( pRigidGeom->m_Collision.nHighClusters );

    collision_data& Coll = pRigidGeom->m_Collision;

    // Count the number of shards we need
    s32 iCluster, iTri;
    
    m_nShards = 0;

    m_MinStartTime     = 1e30f;
    m_MaxStartTime     = -1;
    
    smem_StackPushMarker();
    
    object::detail_tri* pTriStack = (object::detail_tri*)smem_StackAlloc( sizeof(object::detail_tri) * MAX_SHARDS );
    if (NULL == pTriStack)
    {
        smem_StackPopToMarker();
        return;    
    }

    f32* pRawTriStartTime = (f32*)smem_StackAlloc( sizeof(f32) * MAX_SHARDS );
    if (NULL == pRawTriStartTime)
    {
        smem_StackPopToMarker();
        return;    
    }

    s32 iTopOfStack = 0;
    s32 iMaxStackSize = MAX_SHARDS;
    
    // Consider each cluster in the collision.
    for( iCluster = 0; iCluster < Coll.nHighClusters; iCluster++ )
    {
        // If the cluster is not glass not glass, then skip it.
        if( Coll.pHighCluster[ iCluster ].MaterialInfo.SoundType != object::MAT_TYPE_GLASS )
            continue;

        // Consider all the triangles in the glass cluster.
        for( iTri = 0; iTri < Coll.pHighCluster[ iCluster ].nTris; iTri++ )
        {
            s32     Key = (iCluster << 16) | iTri;

            // indicate we want high cluster data
            Key |= 0x80000000;

            if (!pPlaySurface->GetColDetails( Key, pTriStack[ iTopOfStack ] ))
                continue;

            iTopOfStack++;            

            if (iTopOfStack >= (MAX_SHARDS-1))
                break;
        }
        if (iTopOfStack >= (MAX_SHARDS-1))
            break;
    }

    TesselateIntoShards( pPlaySurface, pTriStack, 0, iTopOfStack, iMaxStackSize, pRawTriStartTime );
   
    // Normalize the parameters
    static f32 MinCollapseTime = 0.5f;      // MINIMUM amount of time we want the glass to
                                            // hang in space

    if (m_MaxStartTime < MinCollapseTime)
    {
        // We need to fixup the times
        s32 i;
        f32 MaxRaw = 0;
        f32 MinRaw = 1e30f;

        for (i=0;i<m_nShards;i++)
        {
            MinRaw = MIN(MinRaw, pRawTriStartTime[i]);
            MaxRaw = MAX(MaxRaw, pRawTriStartTime[i]);
        }

        f32 Range = MaxRaw - MinRaw;
        f32 Scale = MinCollapseTime / Range;

        for (i=0;i<m_nShards;i++)
        {
            m_Shards[i].m_StartTime = (pRawTriStartTime[i]-MinRaw) * Scale;
        }

        m_MinStartTime = 0;
        m_MaxStartTime = MinCollapseTime;
    }
    
    m_MaxStartTime -= m_MinStartTime;
    if (m_MaxStartTime == 0)
        m_MaxStartTime = 0.001f;

    s32 i;
    f32 T;
    s32 nColliders = 0;    
    s32 AudioStep = m_nShards / k_MAX_COLLIDERS;

    for (i=0;i<m_nShards;i++)
    {
        shard& Shard = m_Shards[i];

        Shard.m_StartTime -= m_MinStartTime;
        
        T = 1.0f - (Shard.m_StartTime / m_MaxStartTime);

        //Shard.m_Variation = x_frand(0.8f,1.2f);
        Shard.m_Velocity *= T * Shard.m_Variation;   
        Shard.m_bCheckAudioCollision = FALSE;

        if (((i % AudioStep) == 0) && (nColliders < k_MAX_COLLIDERS))
        {
            Shard.m_bCheckAudioCollision = TRUE;
            nColliders++;
        }
    }

    smem_StackPopToMarker();

    m_BBox.Set( vector3(0,0,0), 800 );
    
    OnMove( pPlaySurface->GetL2W().GetTranslation() );    
}


//=============================================================================
//
//=============================================================================
void debris_glass_cluster::OnAdvanceLogic      ( f32 DeltaTime )
{
    static f32 Mass = 30;

    m_TotalTime += DeltaTime;

    if (m_TotalTime > 5.0f)
    {
        g_ObjMgr.DestroyObject( GetGuid() );
        return;
    }

    s32 i;
    vector3 G(0,-981,0);

    G *= DeltaTime * DeltaTime * Mass;

    for (i=0;i<m_nShards;i++)
    {
        shard& Shard = m_Shards[i];

        if (m_TotalTime < Shard.m_StartTime)
            continue;

        vector3 DeltaPos = Shard.m_Velocity * DeltaTime;
        vector3 OldPos = Shard.m_Pos;
        
        Shard.m_Velocity += G * Shard.m_Variation;

        Shard.m_Pos       += DeltaPos;        
        Shard.m_TotalSpin += Shard.m_SpinRate * DeltaTime;        

        if (Shard.m_bCheckAudioCollision)
        {
            g_CollisionMgr.LineOfSightSetup( GetGuid(), OldPos, Shard.m_Pos );
            g_CollisionMgr.SetMaxCollisions(1);
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES,
                                            object::ATTR_BLOCKS_SMALL_DEBRIS,
                                            (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING));                                            
            
            if (g_CollisionMgr.m_nCollisions > 0)
            {
                Shard.m_bCheckAudioCollision = FALSE;
                
                // Play some sweetened audio here
                // "Glass_Impact" is from SFX_Pickups theme
                g_AudioMgr.Play( "Glass_Impact", Shard.m_Pos, GetZone1(), TRUE );
            }
        }
    }
}


//=============================================================================
//
//=============================================================================
void debris_glass_cluster::UpdatePhysics( f32 DeltaTime )
{
    CONTEXT("debris_glass_cluster::UpdatePhysics");
    (void)DeltaTime;
}


//=============================================================================
//
//=============================================================================
void debris_glass_cluster::OnMove( const vector3& rNewPos )
{
    object::OnMove( rNewPos );
}


//=============================================================================
//
//=============================================================================
void debris_glass_cluster::OnRender( void )
{
    /*
    draw_BBox( GetBBox(), XCOLOR_BLUE );
    draw_Sphere( m_SourcePain.Center, 5, XCOLOR_RED );
    draw_Sphere( m_SourcePain.Center, m_SourcePain.RadiusR0, XCOLOR_YELLOW );
    draw_Sphere( m_SourcePain.Center, m_SourcePain.RadiusR1, XCOLOR_GREEN );    
    */
}


//=============================================================================
//
//=============================================================================

void debris_glass_cluster::OnRenderTransparent ( void )
{
    //draw_BBox( GetBBox(), XCOLOR_YELLOW );
    CONTEXT("debris_glass_cluster::OnRenderTransparent");

// TODO:   u32 ADCOn  = 1<<15;
// TODO:   u32 ADCOff = 0;

    rhandle<xbitmap> BrokenGlass;
    
    BrokenGlass.SetName(PRELOAD_FILE("SHBrokenGlass.xbmp"));
    xbitmap* pBrokenGlass = BrokenGlass.GetPointer();

    if (!pBrokenGlass)
        return;

    //static xcolor Clr(255,255,255,255);
    xcolor Fog = render::GetFogValue( GetBBox().GetCenter(), g_RenderContext.LocalPlayerIndex );
    xcolor Clr(128,128,128,128);
    static const f32 FogAScalar = 0.75f;
    Clr.A = (u8)((255 - Fog.A) * FogAScalar);

    draw_ClearL2W();
    draw_Begin( DRAW_TRIANGLES,  DRAW_TEXTURED | DRAW_CULL_NONE | DRAW_USE_ALPHA   );
    //draw_Begin( DRAW_TRIANGLES,  DRAW_TEXTURED | DRAW_CULL_NONE );
    {        
        draw_Color( Clr );

        draw_SetTexture( *pBrokenGlass );

        matrix4     L2W;
        L2W.Identity();

        s32 i;
        for (i=0;i<m_nShards;i++)
        {
            shard&  Shard   = m_Shards[ i ];
            s32     idx     = i*3;

            L2W.SetRotation(Shard.m_TotalSpin);
            L2W.SetTranslation( Shard.m_Pos );
            L2W.Transform( &(m_pPos[ idx ]), &(m_pLocalPos[ idx ]), 3 );

// TODO: This code is only needed for the VU1 render code which is not yet written
//
//            m_pPos[idx+0].GetW() = *((f32*)(&ADCOn));
//            m_pPos[idx+1].GetW() = *((f32*)(&ADCOn));
//            m_pPos[idx+2].GetW() = *((f32*)(&ADCOff));

            draw_UV    ( m_pUV [idx+0] );
            draw_Vertex( m_pPos[idx+0] );
            draw_UV    ( m_pUV [idx+1] );
            draw_Vertex( m_pPos[idx+1] );
            draw_UV    ( m_pUV [idx+2] );
            draw_Vertex( m_pPos[idx+2] );
        }
    }
    draw_End( );
}

