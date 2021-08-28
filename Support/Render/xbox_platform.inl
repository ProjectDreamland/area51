//=============================================================================
//
//  XBOX Specific Render Routines
//
//=============================================================================

#include "shader_mgr.inl"
#include "geom_mgr.inl"

#include "../Decals//DecalMgr.hpp"



//=============================================================================
//=============================================================================
// Static data specific to the xbox-implementation
//=============================================================================
//=============================================================================

static const f32 ItoFScale  = 1.0f/4096.0f;
static xbool s_bMatCanReceiveShadow;
static xbool s_DetailMapPresent;

//=============================================================================
//=============================================================================
// Functions specific to  the xbox implementation
//=============================================================================
//=============================================================================


//=============================================================================
//=============================================================================
// Platform implementation
//=============================================================================
//=============================================================================



//=============================================================================

static
void platform_Init( void )
{
    s_DetailMapPresent = FALSE;

    ASSERT( !g_pShaderMgr );
    g_pShaderMgr = new shader_mgr;
}

//=============================================================================

void xbox_InitShaders( void )
{
    // Build pixel shaders ................................................

    x_memset( ps::Details,0,sizeof( ps::Details ));
    x_memset( vs::Details,0,sizeof( vs::Details ));

    vs::Count = vs::OldSize = vs::Size = 0;
    ps::Count = ps::OldSize = ps::Size = 0;

    // Build pixel shaders ................................................

    const u64 KnownPSFlags[193]=
    {
    0x00000000000000A0,0x0000000000200000,0x0000000000800000,
    0x0000000080000000,0x0000000100000080,0x0000200000000000,
    0x0000800000000000,0x0001000000000000,0x0002000000000001,
    0x0002000000000002,0x0002000000000004,0x0002000000000040,
    0x0002000000010001,0x0002000000010002,0x0002000000010004,
    0x0002000000010040,0x0002000000200000,0x0002000000400000,
    0x0002000008000001,0x0002000008000002,0x0002000008000004,
    0x0002000008000040,0x0002000008010001,0x0002000008010002,
    0x0002000008010004,0x0002000008010040,0x0002000100000001,
    0x0002000100000002,0x0002000100000004,0x0002000100000040,
    0x0002000100010001,0x0002000100010002,0x0002000100010004,
    0x0002000100010040,0x0002000200000001,0x0002000200000040,
    0x0002000200010001,0x0002000200010040,0x0002000300000001,
    0x0002000300000040,0x0002000300010001,0x0002000300010040,
    0x0002000400000002,0x0002000400000004,0x0002000400010002,
    0x0002000400010004,0x0002000500000002,0x0002000500000004,
    0x0002000500010002,0x0002000500010004,0x0002000800010002,
    0x0002000800010004,0x0004000000000001,0x0004000000000002,
    0x0004000000000004,0x0004000000000008,0x0004000000000010,
    0x0004000000000020,0x0004000000000040,0x0004000000000080,
    0x0004000000010001,0x0004000000010002,0x0004000000010004,
    0x0004000000010008,0x0004000000010020,0x0004000000010040,
    0x0004000000010080,0x0004000000040000,0x0004000000040004,
    0x0004000000040080,0x0004000000200000,0x0004000000200080,
    0x0004000000400000,0x0004000000800000,0x0004000000800080,
    0x0004000001000000,0x0004000001200000,0x0004000001800000,
    0x0004000002000000,0x0004000004000000,0x0004000004000002,
    0x0004000004000004,0x0004000008000001,0x0004000008000002,
    0x0004000008000004,0x0004000008000020,0x0004000008000040,
    0x0004000008000080,0x0004000008010001,0x0004000008010002,
    0x0004000008010004,0x0004000008010040,0x0004000008010080,
    0x000400000A000040,0x000400000C000002,0x0004000010000000,
    0x0004000010000004,0x0004000010000080,0x0004000040000000,
    0x0004000080000000,0x0004000081000000,0x0004000100000001,
    0x0004000100000002,0x0004000100000004,0x0004000100000010,
    0x0004000100000020,0x0004000100000040,0x0004000100000080,
    0x0004000100010001,0x0004000100010002,0x0004000100010004,
    0x0004000100010020,0x0004000100010040,0x0004000100010080,
    0x0004000200000001,0x0004000200000040,0x0004000200010040,
    0x0004000300000040,0x0004000400000002,0x0004000400000004,
    0x0004000400000020,0x0004000400010002,0x0004000400010004,
    0x0004000500000020,0x0004000800010002,0x0004000800010004,
    0x0004001000000000,0x0004008000000000,0x0004010000000000,
    0x0006000000000001,0x0006000000000002,0x0006000000000004,
    0x0006000000000040,0x0006000000010001,0x0006000000010002,
    0x0006000000010004,0x0006000000010040,0x0006000008000002,
    0x0006000008000040,0x0006000008010002,0x0006000008010040,
    0x0006000100000001,0x0006000100000002,0x0006000100000004,
    0x0006000100000040,0x0006000100010001,0x0006000100010004,
    0x0006008000000000,0x0006010000000000,0x0010000000000080,
    0x0010000000040000,0x0010000010000000,0x0010000010000080,
    0x0044000080000000,0x0046000080000000,0x0080000000200000,
    0x0104000000000000,0x0200000000200000,0x0200000000800000,
    0x0202000000000001,0x0202000000000002,0x0202000000000004,
    0x0202000000000040,0x0202000000010001,0x0202000000010002,
    0x0202000000010004,0x0202000000010040,0x0204000000000001,
    0x0204000000000002,0x0204000000000004,0x0204000000000008,
    0x0204000000000010,0x0204000000000040,0x0204000000000080,
    0x0204000000010001,0x0204000000010002,0x0204000000010004,
    0x0204000000010040,0x0204000000010080,0x0204000100000001,
    0x0204000100000002,0x0204000100000004,0x0204000100000040,
    0x0204000100000080,0x0204000100010002,0x0204000100010004,
    0x0204000100010040,0x0280000000200000,0x0404000000000000,
    0x1004000000000000,0x1004000000000080,0x2004000000000000,
    0x4004000000000000,
    };
    u32 i;
    ps::desc PSFlags;
    for( i=0;i<sizeof(KnownPSFlags)/sizeof(KnownPSFlags[0]);i++ )
    {
        PSFlags.Mask = KnownPSFlags[ i ];
        GetShader( ps::path( PSFlags ) );
    }

    // Build vertex shaders ...............................................

    const u32 KnownVSFlags[233]=
    {
    0x00000001,0x00000002,0x00000021,0x00000022,
    0x00000081,0x00000082,0x00000086,0x000000A1,
    0x000000A2,0x000000A9,0x000000C1,0x000000C9,
    0x00000101,0x00000102,0x00000106,0x00000121,
    0x00000122,0x00000129,0x00000141,0x00000149,
    0x00000181,0x00000182,0x00000201,0x00000202,
    0x00000206,0x00000221,0x00000222,0x00000229,
    0x00000241,0x00000249,0x00000301,0x00000302,
    0x00000401,0x00000402,0x00000406,0x00000422,
    0x00000441,0x00000449,0x00000481,0x00000482,
    0x00000486,0x000004A2,0x00000501,0x00000502,
    0x00000522,0x00000581,0x00000582,0x00000601,
    0x00000602,0x00000606,0x00000622,0x00000641,
    0x00000649,0x00001081,0x00001086,0x000010A1,
    0x000010C1,0x000010C9,0x00001106,0x00001121,
    0x00001141,0x00001149,0x00001206,0x00001221,
    0x00001241,0x00001249,0x00004000,0x00008001,
    0x00008002,0x00008021,0x00008022,0x00008029,
    0x00008041,0x00008049,0x00009001,0x00009021,
    0x00009081,0x00080001,0x001000A1,0x001000C1,
    0x001010A1,0x00200121,0x00200141,0x00200221,
    0x00200241,0x00200249,0x00201241,0x00201249,
    0x00400121,0x00400221,0x00800001,0x00800002,
    0x01000001,0x01000002,0x04000001,0x04000002,
    0x04000081,0x04000082,0x04000086,0x040000A1,
    0x040000A2,0x040000A9,0x040000C1,0x040000C9,
    0x04000106,0x04000121,0x04000122,0x04000129,
    0x04000141,0x04000149,0x04000206,0x04000221,
    0x04000222,0x04000229,0x04000241,0x04000249,
    0x04001086,0x040010A1,0x040010A9,0x040010C1,
    0x040010C9,0x04001106,0x04001121,0x04001141,
    0x04001149,0x04001206,0x04001221,0x04001241,
    0x04001249,0x04008001,0x04008006,0x04008021,
    0x04008029,0x04008041,0x04008049,0x04009001,
    0x04009021,0x041000C1,0x041000C9,0x041010C1,
    0x041010C9,0x04200141,0x04200149,0x04200241,
    0x04200249,0x04201141,0x04201149,0x04201241,
    0x04400141,0x04400149,0x04400241,0x04400249,
    0x04800001,0x04800002,0x05000001,0x05000002,
    0x08000001,0x20008081,0x20008082,0x200080A1,
    0x200080A2,0x24008081,0x240080A1,0x24009081,
    0x40000086,0x400000A1,0x400000A2,0x400000A9,
    0x400000C1,0x400000C9,0x40000106,0x40000121,
    0x40000122,0x40000129,0x40000141,0x40000149,
    0x40000206,0x40000221,0x40000222,0x40000229,
    0x40000241,0x40000249,0x40001086,0x400010A1,
    0x400010C1,0x400010C9,0x40001106,0x40001121,
    0x40001141,0x40001149,0x40001206,0x40001221,
    0x40001241,0x401000A1,0x40400121,0x40400141,
    0x40400221,0x40400241,0x44000086,0x440000A1,
    0x440000A2,0x440000C1,0x440000C9,0x44000106,
    0x44000121,0x44000122,0x44000129,0x44000141,
    0x44000149,0x44000206,0x44000221,0x44000222,
    0x44000229,0x44000241,0x44000249,0x44001086,
    0x440010A1,0x440010C1,0x440010C9,0x44001106,
    0x44001121,0x44001129,0x44001141,0x44001149,
    0x44001206,0x44001221,0x44001241,0x44001249,
    0x441000C1,0x44400141,0x44400241,0x80008001,
    0x80008022,
    };
    vs::desc VSFlags;
    for( i=0;i<sizeof(KnownVSFlags)/sizeof(KnownVSFlags[0]);i++ )
    {
        VSFlags.Mask = KnownVSFlags[ i ];
        GetShader( vs::path( VSFlags ) );
    }
}

//=============================================================================

static
void platform_Kill( void )
{
    if( g_pShaderMgr )
    {
        delete g_pShaderMgr;
        g_pShaderMgr = NULL;
    }
}

//=============================================================================

static
void platform_BeginSession( u32 nPlayers )
{
    bool b60fps = false;
    if( nPlayers > 1 )
    {
        // hold down these buttons to kick the game into 60fps

        input_UpdateState();
        xbool L = input_IsPressed( INPUT_XBOX_L_TRIGGER,0 );
        xbool R = input_IsPressed( INPUT_XBOX_R_TRIGGER,0 );
        xbool W = input_IsPressed( INPUT_XBOX_BTN_BLACK,0 );
        b60fps = L && R && W;
    }
    if( ! g_pPipeline )
          g_pPipeline = new pipeline_mgr( nPlayers,b60fps );

extern void
    xbox_BeginFrameSmoothing();
    xbox_BeginFrameSmoothing();
}

//=============================================================================

static
void platform_EndSession( void )
{
    if( g_pPipeline )
    {
        delete g_pPipeline;
        g_pPipeline = NULL;
    }

    extern void
    xbox_EndFrameSmoothing();
    xbox_EndFrameSmoothing();
}

//=============================================================================

static
void platform_BeginRigidGeom( geom* pGeom, s32 iSubMesh )
{
    s_CurrentBBox = pGeom->m_BBox;

    g_pPipeline->BeginRigid( pGeom,iSubMesh );
}

//=============================================================================

static
void* platform_CalculateRigidLighting( const matrix4&   L2W,
                                       const bbox&      BBox )
{
    void* pResult = NULL;
    {
        s32 n = g_LightMgr.CollectLights( BBox,3 );
        if( n )
        {
            lights* pLights =( lights       * )smem_BufferAlloc(   sizeof( lights ));
            pLights->pPoint =( lights::point* )smem_BufferAlloc( n*sizeof( lights::point ));
            pLights->Count  =n;

            s_TotalDynLights += n;

            f32     Radius;
            xcolor  Color;
            vector3 Pos;

            for( s32 i=0;i<n;i++ )
            {
                g_LightMgr.GetCollectedLight( i,Pos,Radius,Color );
                pLights->pPoint[i].Pos.Set ( Pos.GetX(),
                                             Pos.GetY(),
                                             Pos.GetZ(),
                                             Radius );
                pLights->pPoint[i].Col.Set (
                    f32(Color.R)/255,
                    f32(Color.G)/255,
                    f32(Color.B)/255,
                    1.0f );
            }
            pResult = pLights;
        }
    }
    return pResult;
}

//=============================================================================

static
void* platform_CalculateSkinLighting( u32            Flags,
                                      const matrix4& L2W,
                                      const bbox&    BBox,
                                      xcolor         Ambient )
{
    (void)Flags;
    void* pResult = NULL;
    {
        s32 n = g_LightMgr.CollectCharLights( L2W,BBox,4 );

        lights*pLights=(lights*)smem_BufferAlloc( sizeof( lights ));

        pLights->Ambient.GetX() = f32( Ambient.R )/255;
        pLights->Ambient.GetY() = f32( Ambient.G )/255;
        pLights->Ambient.GetZ() = f32( Ambient.B )/255;
        pLights->Ambient.GetW() = 1.0f;
        pLights->Count     = n;

        if( ! n )
            pLights->pDir = NULL;
        else
        {
            pLights->pDir=( lights::dir* )smem_BufferAlloc( n*sizeof( lights::dir ));

            vector3 Dir;
            xcolor Col;

            for( s32 i=0;i<n;i++ )
            {
                g_LightMgr.GetCollectedCharLight( i,Dir,Col );
                pLights->pDir[i].Dir.Set( Dir.GetX(),Dir.GetY(),Dir.GetZ(),1 );
                pLights->pDir[i].Col.Set(
                    f32(Col.R)/255,
                    f32(Col.G)/255,
                    f32(Col.B)/255,
                    1.0f );
            }
        }
        pResult = pLights;
    }
    return pResult;
}

//=============================================================================

static
void platform_RenderLitRigidInstance( render_instance& Inst )
{
    static f32 fDist = 1200.0f;
    if( s_DetailMapPresent && (xbox_CalcDistance( *Inst.Data.Rigid.pL2W,s_CurrentBBox ) < fDist) )
        Inst.Flags |= render::INSTFLAG_DETAIL;

    if ( Inst.Flags & render::CLIPPED )
        Inst.Flags |= render::INSTFLAG_CLIPPED;

    if ( (Inst.Flags & render::SHADOW_PASS) && s_bMatCanReceiveShadow )
        Inst.Flags |= render::INSTFLAG_SHADOW_PASS;

    if ( Inst.Flags & render::GLOWING )
        Inst.Flags |= render::INSTFLAG_GLOWING;

    if ( Inst.Flags & render::FADING_ALPHA )
    {
        Inst.Flags |= render::INSTFLAG_FADING_ALPHA;
        Inst.Flags &= ~(render::INSTFLAG_DETAIL | render::INSTFLAG_SPOTLIGHT | render::INSTFLAG_SHADOW_PASS);
    }
    g_pPipeline->RenderToLightMap( Inst );
}

//=============================================================================

static void platform_RenderSkinZInstance( render_instance& Inst )
{
    static f32 fDist = 1200.0f;
    if( s_DetailMapPresent && (xbox_CalcDistance( *Inst.Data.Rigid.pL2W,s_CurrentBBox ) < fDist) )
        Inst.Flags |= render::INSTFLAG_DETAIL;

    if( render::IsFilterLightEnabled() && ((Inst.Flags & render::DISABLE_FILTERLIGHT) == 0) )
        Inst.Flags |= render::INSTFLAG_FILTERLIGHT;

    if ( Inst.Flags & render::CLIPPED )
        Inst.Flags |= render::INSTFLAG_CLIPPED;

    if ( (Inst.Flags & render::SHADOW_PASS) && s_bMatCanReceiveShadow )
        Inst.Flags |= render::INSTFLAG_SHADOW_PASS;

    if ( Inst.Flags & render::GLOWING )
        Inst.Flags |= render::INSTFLAG_GLOWING;

    if ( Inst.Flags & render::FADING_ALPHA )
    {
        Inst.Flags |= render::INSTFLAG_FADING_ALPHA;
        Inst.Flags &= ~(render::INSTFLAG_DETAIL | render::INSTFLAG_SPOTLIGHT | render::INSTFLAG_SHADOW_PASS);
    }

    g_pPipeline->RenderToZBuffer( Inst );
}

//=============================================================================

static void platform_RenderRigidZInstance( render_instance& Inst )
{
    if( Inst.Flags & render::CLIPPED )
        Inst.Flags |= render::INSTFLAG_CLIPPED;

    if( (Inst.Flags & render::SHADOW_PASS) && s_bMatCanReceiveShadow )
        Inst.Flags |= render::INSTFLAG_SHADOW_PASS;

    if( Inst.Flags & render::GLOWING )
        Inst.Flags |= render::INSTFLAG_GLOWING;

    if( Inst.Flags & render::FADING_ALPHA )
    {
        Inst.Flags |= render::INSTFLAG_FADING_ALPHA;
        Inst.Flags &= ~(render::INSTFLAG_DETAIL | render::INSTFLAG_SPOTLIGHT | render::INSTFLAG_SHADOW_PASS);
    }

    g_pPipeline->RenderToZBuffer( Inst );
}

//=============================================================================

static
void platform_RenderRigidInstance( render_instance& Inst )
{
    static f32 fDist = 1200.0f;
    if( s_DetailMapPresent && (xbox_CalcDistance( *Inst.Data.Rigid.pL2W,s_CurrentBBox ) < fDist) )
        Inst.Flags |= render::INSTFLAG_DETAIL;

    if ( Inst.Flags & render::CLIPPED )
        Inst.Flags |= render::INSTFLAG_CLIPPED;

    if ( (Inst.Flags & render::SHADOW_PASS) && s_bMatCanReceiveShadow )
        Inst.Flags |= render::INSTFLAG_SHADOW_PASS;

    if ( Inst.Flags & render::GLOWING )
        Inst.Flags |= render::INSTFLAG_GLOWING;

    if ( Inst.Flags & render::FADING_ALPHA )
    {
        Inst.Flags |= render::INSTFLAG_FADING_ALPHA;
        Inst.Flags &= ~(render::INSTFLAG_DETAIL | render::INSTFLAG_SPOTLIGHT | render::INSTFLAG_SHADOW_PASS);
    }
    g_pPipeline->RenderToPrimary( Inst );
}

//=============================================================================

static
void platform_EndRigidGeom( void )
{
    g_pPipeline->End( );
}

//=============================================================================

static
void platform_ActivateLitMaterial( const material& Material )
{
    s_DetailMapPresent = FALSE;
    if ( IsAlphaMaterial( (material_type)Material.m_Type ) && !(Material.m_Flags & geom::material::FLAG_FORCE_ZFILL) )
        s_bMatCanReceiveShadow = FALSE;
    else    
        s_bMatCanReceiveShadow = TRUE;
    g_pPipeline->SetLitMaterial( Material );
}

//=============================================================================

static
void platform_ActivateMaterial( const material& Material )
{
    s_DetailMapPresent = !!(Material.m_Flags & geom::material::FLAG_HAS_DETAIL_MAP);
    if ( IsAlphaMaterial( (material_type)Material.m_Type ) && !(Material.m_Flags & geom::material::FLAG_FORCE_ZFILL) )
        s_bMatCanReceiveShadow = FALSE;
    else    
        s_bMatCanReceiveShadow = TRUE;
    g_pPipeline->SetMaterial( Material );
}

//=============================================================================

static
void platform_ActivateDistortionMaterial( const material* pMaterial, const radian3& NormalRot )
{
    if( ! pMaterial )
        g_pPipeline->SetDefaultDistortion( NormalRot );
    else
        g_pPipeline->SetMaterial( *pMaterial );
    s_bMatCanReceiveShadow = FALSE;
    s_DetailMapPresent = FALSE;
}

//=============================================================================

static
void platform_ActivateZPrimeMaterial( void )
{
    g_pPipeline->SetZPrime( true );
    s_bMatCanReceiveShadow = FALSE;
    s_DetailMapPresent = FALSE;
}

//=============================================================================

static
void platform_BeginSkinGeom( geom* pGeom, s32 iSubMesh )
{
    g_pPipeline->BeginSkin( pGeom,iSubMesh );
}

//=============================================================================

static
void platform_RenderSkinInstance( render_instance& Inst )
{
    static f32 fDist = 1200.0f;
    if( s_DetailMapPresent && (xbox_CalcDistance( *Inst.Data.Rigid.pL2W,s_CurrentBBox ) < fDist) )
        Inst.Flags |= render::INSTFLAG_DETAIL;

    if( render::IsFilterLightEnabled() && ((Inst.Flags & render::DISABLE_FILTERLIGHT) == 0) )
        Inst.Flags |= render::INSTFLAG_FILTERLIGHT;

    if ( Inst.Flags & render::CLIPPED )
        Inst.Flags |= render::INSTFLAG_CLIPPED;

    if ( (Inst.Flags & render::SHADOW_PASS) && s_bMatCanReceiveShadow )
        Inst.Flags |= render::INSTFLAG_SHADOW_PASS;

    if ( Inst.Flags & render::GLOWING )
        Inst.Flags |= render::INSTFLAG_GLOWING;

    if ( Inst.Flags & render::FADING_ALPHA )
    {
        Inst.Flags |= render::INSTFLAG_FADING_ALPHA;
        Inst.Flags &= ~(render::INSTFLAG_DETAIL | render::INSTFLAG_SPOTLIGHT | render::INSTFLAG_SHADOW_PASS);
    }
    g_pPipeline->RenderToPrimary( Inst );
}

//=============================================================================

static
void platform_EndSkinGeom( void )
{
    g_pPipeline->End( );
}

//=============================================================================

static
void platform_RegisterRigidGeom( rigid_geom& Geom  )
{
}

//=============================================================================

static
void platform_UnregisterRigidGeom( rigid_geom& Geom  )
{
}

//=============================================================================

static
void platform_RegisterSkinGeom( skin_geom& Geom  )
{
}

//=============================================================================

static
void platform_UnregisterSkinGeom( skin_geom& Geom  )
{
}



//=============================================================================
//=============================================================================
// Deprecated functions
//=============================================================================
//=============================================================================



//=============================================================================

static
void platform_RegisterRigidInstance( rigid_geom& Geom, render::hgeom_inst hInst )
{
}

//=============================================================================

static
void platform_UnregisterRigidInstance( render::hgeom_inst hInst )
{
}

//=============================================================================

static
void platform_RegisterSkinInstance( skin_geom& Geom, render::hgeom_inst hInst )
{
}

//=============================================================================

static
void platform_UnregisterSkinInstance( render::hgeom_inst hInst )
{
}





















































//=============================================================================

static
void* platform_LockRigidDListIndex( render::hgeom_inst hInst, s32 iSubMesh,  s32& VertexOffset )
{
    (void)hInst;
    (void)iSubMesh;
    (void)VertexOffset;
    ASSERT(0);
    return NULL;
}

//=============================================================================

static
void platform_UnlockRigidDListIndex( render::hgeom_inst hInst, s32 iSubMesh )
{
    (void)hInst;
    (void)iSubMesh;
    ASSERT(0);
}

//=============================================================================

static
void platform_BeginShaders( void )
{
    g_pShaderMgr->Begin( );
}

//=============================================================================

static
void platform_EndShaders( void )
{
    g_pShaderMgr->End( );
}

//=============================================================================

static
void platform_CreateEnvTexture( void )
{
}

//=============================================================================

static
void platform_SetProjectedTexture( texture::handle Texture )
{
    g_pPipeline->SetProjectiveTexture( Texture );
}

//=============================================================================

static
void CalculateProjMatrix( matrix4& Matrix, view& View, const texture_projection& Projection  )
{
    f32 ZNear = 1.0f;
    f32 ZFar  = Projection.Length;

    const xbitmap& Bitmap = Projection.Texture.GetPointer()->m_Bitmap;

    View.SetXFOV( Projection.FOV );
    View.SetZLimits( ZNear, ZFar );
    View.SetViewport( 0, 0, Bitmap.GetWidth(), Bitmap.GetHeight() );
    View.SetV2W( Projection.L2W );

    Matrix  = View.GetV2C();
    Matrix(2,2)  = -1.0f /       (ZFar-ZNear);
    Matrix(3,2)  =  1.0f + ZNear/(ZFar-ZNear);
    Matrix *= View.GetW2V();

    Matrix.Scale    (vector3( 0.5f, -0.5f, 1.0f) );
    Matrix.Translate(vector3( 0.5f,  0.5f, 0.0f) );
}

//=============================================================================

static
void platform_ComputeProjTextureMatrix( matrix4& Matrix, view& View, const texture_projection& Projection  )
{
    CalculateProjMatrix( Matrix,View,Projection );
    g_pPipeline->SetProjMatrix( Matrix );
}

//=============================================================================

static
void platform_SetTextureProjection( const texture_projection& Projection )
{
    (void)Projection;
}

//=============================================================================

static
void platform_SetTextureProjectionMatrix( const matrix4& Matrix )
{
    (void)Matrix;
}

//=============================================================================

static
void platform_SetProjectedShadowTexture( s32 Index, texture::handle Texture )
{
    (void)Index;
    (void)Texture;
}

//=============================================================================

static
void platform_ComputeProjShadowMatrix( matrix4& Matrix, view& View, const texture_projection& Projection )
{
    // same as a projected texture
    CalculateProjMatrix( Matrix, View, Projection );
}

//=============================================================================

static
void platform_SetShadowProjectionMatrix( s32 Index, const matrix4& Matrix )
{
    (void)Index;
    (void)Matrix;
}

//=============================================================================

static
void platform_ClearShadowProjectorList( void )
{
}

//=============================================================================

static
void platform_AddPointShadowProjection( const matrix4&         L2W,
                                        radian                 FOV,
                                        f32                    NearZ,
                                        f32                    FarZ )
{
    // TODO:
    (void)L2W;
    (void)FOV;
    (void)NearZ;
    (void)FarZ;
}

//=============================================================================

static const f32 kDetailMinMipBias = -6.0f;
static const f32 kDetailMaxMipBias = -4.0f;
static const f32 kDetailMipScale   = 1.0f;
static const s32 kDetailMipL       = 1;
static const s32 kMipMapMode       = 4;          // 4 = Normal  5 = Tri-Linear
static const s32 kDetailMipMapMode = 5;          // 4 = Normal  5 = Tri-Linear
static const s32 kShadowTexSize    = 128;
static const s32 kShadowAlpha      = 0x60;

static
void platform_AddDirShadowProjection( const matrix4&         L2W,
                                      f32                    Width,
                                      f32                    Height,
                                      f32                    NearZ,
                                      f32                    FarZ )
{
    g_pPipeline->AddDirShadowProjector( L2W,Width,Height,NearZ,FarZ );
}

//=============================================================================

static
void platform_BeginShadowShaders( void )
{
    g_pShaderMgr->Begin( );
    g_pPipeline->BeginShadowPass();
    s_bMatCanReceiveShadow = TRUE;
}

//=============================================================================

static
void platform_EndShadowShaders( void )
{
    g_pShaderMgr->End( );
    g_pPipeline->EndShadowPass();
    s_bMatCanReceiveShadow = FALSE;
}

//=============================================================================

static
void platform_StartShadowCast( void )
{
    g_pPipeline->ClearShadowBuffers( s_nDynamicShadows );
}

//=============================================================================

static
void platform_EndShadowCast( void )
{
}

//=============================================================================

static
void platform_StartShadowReceive( void )
{
    g_pPipeline->StartShadowReceive();
}

//=============================================================================

static
void platform_EndShadowReceive( void )
{
    g_pPipeline->EndShadowReceive();
}

//=============================================================================

static
void platform_BeginShadowCastRigid( geom* pGeom, s32 iSubMesh )
{
    (void)pGeom;
    (void)iSubMesh;
    // TODO:
}

//=============================================================================

static
void platform_RenderShadowCastRigid( render_instance& Inst )
{
}

//=============================================================================

static
void platform_EndShadowCastRigid( void )
{
    // TODO:
}

//=============================================================================

static
void platform_BeginShadowCastSkin( geom* pGeom, s32 iSubMesh )
{
    g_pPipeline->BeginSkin( pGeom,iSubMesh );

    g_Texture.Clear( 0 );
    g_Texture.Clear( 1 );
    g_Texture.Clear( 2 );
    g_Texture.Clear( 3 );
}

//=============================================================================

static
void platform_RenderShadowCastSkin( render_instance& Inst, s32 iProj )
{
    g_pPipeline->SkinCastShadow( Inst,iProj );
}

//=============================================================================

static
void platform_EndShadowCastSkin( void )
{
    g_pd3dDevice->SetViewport( NULL );
    g_pPipeline->End( );
}

//=============================================================================

static
void platform_BeginShadowReceiveRigid( geom* pGeom, s32 iSubMesh )
{
    g_RenderState.Set( D3DRS_ALPHAFUNC,D3DCMP_LESS  );
    g_RenderState.Set( D3DRS_ALPHATESTENABLE,TRUE );
    g_RenderState.Set( D3DRS_ALPHAREF,128 );

    g_pPipeline->BeginShadowReceiveRigid( pGeom,iSubMesh );
}

//=============================================================================

static
void platform_RenderShadowReceiveRigid( render_instance& Inst, s32 iProj )
{
    g_pPipeline->RenderToShadows( Inst,iProj );
}

//=============================================================================

static
void platform_EndShadowReceiveRigid( void )
{
    g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );

    g_pShaderMgr->End( );
    g_pPipeline ->End( );
}

//=============================================================================

static
void platform_BeginNormalRender( void )
{
}

//=============================================================================

static
void platform_EndNormalRender( void )
{
    g_pPipeline->EndNormalRender();
}

//=============================================================================

static
void platform_BeginShadowReceiveSkin( geom* pGeom, s32 iSubMesh )
{
    g_pPipeline  ->BeginSkin( pGeom,iSubMesh );
    g_pShaderMgr->Begin( );
}

//=============================================================================

static
void platform_RenderShadowReceiveSkin( render_instance& Inst )
{
    (void)Inst;
    // TODO:
}

//=============================================================================

static
void platform_EndShadowReceiveSkin( void )
{
    g_pPipeline->End( );
}

//=============================================================================

static
void* platform_LockRigidDListVertex( render::hgeom_inst hInst, s32 iSubMesh )
{
    (void)iSubMesh;
    (void)hInst;
    ASSERT(0);
    return NULL;
}

//=============================================================================

static
void platform_UnlockRigidDListVertex( render::hgeom_inst hInst, s32 iSubMesh )
{
    (void)iSubMesh;
    (void)hInst;
    ASSERT(0);
}

//=============================================================================

static
void platform_StartRawDataMode( void )
{
    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
    g_RenderState.Set( D3DRS_ZFUNC,D3DCMP_LESSEQUAL );
    g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_NONE );
    g_RenderState.Set( D3DRS_LIGHTING,D3DZB_FALSE  );
    g_RenderState.Set( D3DRS_ZENABLE ,D3DZB_FALSE  );

    const view   *pView = eng_GetView();
    eng_SetView( *pView );
}

//=============================================================================

static
void platform_EndRawDataMode( void )
{
}

//=============================================================================

static const xbitmap* s_pDrawBitmap=NULL;
static s32 s_bGlowing;
static s32 s_DrawFlags = 0;
static s32 s_BlendMode = 0;
static ps::desc s_PSFlags;
static vs::desc s_VSFlags;

static
void platform_SetDiffuseMaterial( const xbitmap& Bitmap,s32 BlendMode, xbool ZTestEnabled )
{
    // do some entropy stuff //////////////////////////////////////////////////

    vram_Activate( Bitmap );

    s_BlendMode = BlendMode;
    s_bGlowing = false;

    // setup shader engine ////////////////////////////////////////////////////

    s_VSFlags.clear();
    s_PSFlags.clear();

    // bRgbaByTex0 = "mul r0,t0,v0"
    s_PSFlags.bRgbaByTex0 = true;
    s_PSFlags.xfc_Std = true;

    // we can use draw to set up render states at which point the shader engine
    // will hijack what it needs and route the verts through its pixel pipeline

    s_DrawFlags = DRAW_TEXTURED | DRAW_NO_ZWRITE | DRAW_UV_CLAMP | DRAW_CULL_NONE | DRAW_USE_ALPHA;
    if( !ZTestEnabled )
        s_DrawFlags |= DRAW_NO_ZBUFFER;

    switch( BlendMode )
    {
        case render::BLEND_MODE_INTENSITY:
            break;

        case render::BLEND_MODE_NORMAL:
            break;

        case render::BLEND_MODE_ADDITIVE:
            s_DrawFlags |= DRAW_BLEND_ADD;
            break;

        // bAlpha = "mul r0.rgb,t0.rgb,v0.rgb + mov r0.a,t0.a"
        case render::BLEND_MODE_SUBTRACTIVE:
            s_DrawFlags |= DRAW_BLEND_SUB;
            s_PSFlags.bRgbaByTex0 = false;
            s_PSFlags.bAlpha = true;
            break;
    }

    g_pPipeline->m_bZPrime = false;

    s_pDrawBitmap = &Bitmap;


    // setup write mask ///////////////////////////////////////////////////////

    g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
}

//=============================================================================

static
void platform_SetGlowMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestEnabled )
{
    platform_SetDiffuseMaterial( Bitmap, BlendMode, ZTestEnabled );
    g_pPipeline->m_bDirtySelfIllum  = true ;
    s_PSFlags.bRgbaByTex0           = false;
    s_PSFlags.bAlpha                = true ;
    s_bGlowing                      = true ;
}

//=============================================================================

static
void platform_SetEnvMapMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestEnabled )
{
    // do some entropy stuff //////////////////////////////////////////////////

    platform_SetDiffuseMaterial( Bitmap, BlendMode, ZTestEnabled );

    g_pPipeline->SetupEnvMap( geom::material::FLAG_ENV_CUBE_MAP,NULL,NULL );

    // setup shader engine ////////////////////////////////////////////////////

    s_PSFlags.clear();
    s_PSFlags.bDiffusePerPixelEnv = true;
    s_PSFlags.xfc_Std = true;
}

//=============================================================================

static
void platform_SetDistortionMaterial( s32 BlendMode, xbool ZTestEnabled )
{
    //#ifdef bhapgood
    //#error DBS: Hey Bryon, Im putting this compile bomb in here for you, but we
    //#error will need to implement this for the xbox
    //#endif
    ASSERTS( FALSE, "Not implemented yet!" );
    (void)BlendMode;
    (void)ZTestEnabled;

    g_pPipeline->m_bZPrime = false;
}

//=============================================================================

void xbox_WriteVert( const vector4& Pos,f32 U,f32 V,xcolor Color )
{
    draw_Color( Color );
    draw_UV( U,V );

    vector3& P = (vector3&)Pos;
    draw_Vertex( P );
}

//=============================================================================

static
void platform_RenderRawStrips( s32 nVerts,
                               const matrix4&    L2W,
                               const vector4*    pPos,
                               const s16*        pUV,
                               const u32*        pColor )
{
    if( !SWITCH_USE_TEXTURES )
    {
        return;
    }

    draw_SetL2W( L2W );

    u32 DrawFlags = s_DrawFlags | DRAW_NO_ZWRITE | DRAW_XBOX_NO_BEGIN;

    draw_Begin( DRAW_TRIANGLES,DrawFlags );
    if( s_bGlowing )
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL );

    vs::desc VSFlags = s_VSFlags;
    ps::desc PSFlags = s_PSFlags;

    switch( s_BlendMode )
    {
        case render::BLEND_MODE_ADDITIVE:
        {
            g_RenderState.Set( D3DRS_BLENDOP  , D3DBLENDOP_ADD    );
            g_RenderState.Set( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
            g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_ONE      );
            PSFlags.bTexelModded = false;
            PSFlags.bRgbaByTex0 = true;
        }
        break;

        case render::BLEND_MODE_SUBTRACTIVE:
        {
            g_RenderState.Set( D3DRS_BLENDOP  , D3DBLENDOP_REVSUBTRACT );
            g_RenderState.Set( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
            g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_ONE );
            PSFlags.bTexelModded = false;
            PSFlags.bRgbaByTex0 = true;
        }
        break;

        case render::BLEND_MODE_NORMAL:
        {
            g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
            g_RenderState.Set( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
            g_RenderState.Set( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
            PSFlags.bTexelModded = false;
            PSFlags.bRgbaByTex0 = true;
        }
        break;

        case render::BLEND_MODE_INTENSITY:
        if( SWITCH_USE_INTENSITY_DECALS )
        {
            // fragment = pshader output * dest pixel * 2
            g_RenderState.Set( D3DRS_SRCBLEND , D3DBLEND_DESTCOLOR );
            g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
            g_RenderState.Set( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
            PSFlags.xfc_Std = true;
            break;
        }
        else
        {
            draw_End();
            return;
        }
    }

    // Now link the shaders to apply our predetermined effect *****************
    {
        g_pShaderMgr->Link( 0,NULL,VSFlags,NULL,PSFlags,0 );

        struct UV { s16 U,V; } *pUv =(UV*) pUV;

        if( VSFlags.oT0_Intensity )
        {
            const view* pView = eng_GetView();

            matrix4 L2C = pView->GetW2C()*L2W;
            g_pd3dDevice->SetVertexShaderConstant( 92,&L2C,4 );

            g_pPipeline->SetupTexConsts( PSFlags );
        }

        if( PSFlags.bPerPixelLitPunchthru )
        {
            const f32 FullAlpha[4]={ 0.0f,0.0f,0.0f,1.0f };
            g_pd3dDevice->SetPixelShaderConstant( 0,FullAlpha,1 ); 
        }

        // 2nd draw_Begin is necessary due to the GPU crash you'll get
        // if D3D->Begin() is called before setting all the above
        // render states. DRAW_KEEP_STATES is mandatory.

        draw_Begin( DRAW_TRIANGLES,DRAW_KEEP_STATES );
        {
            if( s_BlendMode==render::BLEND_MODE_INTENSITY )
            {
                // setup white for intensity blends ...........................

                draw_Color( 1.0f,1.0f,1.0f,1.0f );

                // send polygons to gpu .......................................

                for( s32 iVert=0; iVert<nVerts; iVert++ )
                {
                    f32 W = pPos[iVert].GetW( );
                    if( *(( u32* )&W ) & decal_mgr::decal_vert::FLAG_SKIP_TRIANGLE )
                        continue;

                    vector3 & p0 = (vector3&)pPos[iVert-2];
                    vector3 & p1 = (vector3&)pPos[iVert-1];
                    vector3 & p2 = (vector3&)pPos[iVert-0];

                    draw_UV(
                        ( pUv[iVert-2].U )*ItoFScale,
                        ( pUv[iVert-2].V )*ItoFScale );
                    draw_Vertex(
                        p0.GetX(),
                        p0.GetY(),
                        p0.GetZ() );
                    draw_UV(
                        ( pUv[iVert-1].U )*ItoFScale,
                        ( pUv[iVert-1].V )*ItoFScale );
                    draw_Vertex(
                        p1.GetX(),
                        p1.GetY(),
                        p1.GetZ()
                    );
                    draw_UV(
                        ( pUv[iVert].U )*ItoFScale,
                        ( pUv[iVert].V )*ItoFScale );
                    draw_Vertex(
                        p2.GetX(),
                        p2.GetY(),
                        p2.GetZ()
                    );
                }
            }
            else
            {
                for( s32 iVert=0; iVert<nVerts; iVert++ )
                {
                    f32 W = pPos[iVert].GetW( );
                    if( *(( u32* )&W ) & decal_mgr::decal_vert::FLAG_SKIP_TRIANGLE )
                        continue;

                    xcolor C0((pColor[iVert-2] & 0xff00ff00)      +
                             ((pColor[iVert-2] & 0x00ff0000)>>16) +
                             ((pColor[iVert-2] & 0x000000ff)<<16));
                    xcolor C1((pColor[iVert-1] & 0xff00ff00)      +
                             ((pColor[iVert-1] & 0x00ff0000)>>16) +
                             ((pColor[iVert-1] & 0x000000ff)<<16));
                    xcolor C2((pColor[iVert-0] & 0xff00ff00)      +
                             ((pColor[iVert-0] & 0x00ff0000)>>16) +
                             ((pColor[iVert-0] & 0x000000ff)<<16));

                    xbox_WriteVert(
                        pPos    [iVert-2],
                        f32( pUv[iVert-2].U )*ItoFScale,
                        f32( pUv[iVert-2].V )*ItoFScale,
                        C0 );
                    xbox_WriteVert(
                        pPos    [iVert-1],
                        f32( pUv[iVert-1].U )*ItoFScale,
                        f32( pUv[iVert-1].V )*ItoFScale,
                        C1 );
                    xbox_WriteVert(
                        pPos    [iVert],
                        f32( pUv[iVert].U )*ItoFScale,
                        f32( pUv[iVert].V )*ItoFScale,
                        C2 );
                }
            }
        }
        draw_End();

        if( VSFlags.oT0_Intensity )
            g_pd3dDevice->SetShaderConstantMode( D3DSCM_96CONSTANTS  );
    }
}

//=============================================================================

static
void platform_Render3dSprites( s32            nSprites,
                               f32            UniScale,
                               const matrix4* pL2W,
                               const vector4* pPositions,
                               const vector2* pRotScales,
                               const u32*     pColors )
{
    if( !SWITCH_USE_TEXTURES )
    {
        return;
    }

    // sanity check
    ASSERTS( s_pDrawBitmap, "You must set a material first!" );
    if( nSprites == 0 )
        return;

    // start up draw
    const matrix4& V2W = eng_GetView()->GetV2W();
    const matrix4& W2V = eng_GetView()->GetW2V();
    matrix4 S2V;
    if( pL2W )
        S2V = W2V * (*pL2W);
    else
        S2V = W2V;
    draw_ClearL2W();
    draw_Begin( DRAW_TRIANGLES, s_DrawFlags|DRAW_XBOX_NO_BEGIN|DRAW_CULL_NONE );
    if( s_bGlowing )
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL );

    g_pd3dDevice->SetTransform( D3DTS_WORLD,      (D3DMATRIX*)&V2W );
    g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&W2V );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&eng_GetView()->GetV2C() );

    // Now link the shaders to apply our predetermined effect
    // Over saturation unnecessary because it will be applied
    // during the post effect.

    vs::desc VSFlags = s_VSFlags;
    ps::desc PSFlags = s_PSFlags;

    g_pShaderMgr->Link( VSFlags,PSFlags );

    if( s_PSFlags.bPerPixelLitPunchthru )
    {
        const f32 FullAlpha[4]={ 0.0f,0.0f,0.0f,1.0f };
        g_pd3dDevice->SetPixelShaderConstant( 0,FullAlpha,1 ); 
    }

    draw_Begin( DRAW_TRIANGLES,DRAW_KEEP_STATES );

    // loop through the sprites and render them
    s32 i, j;
    for( i = 0; i < nSprites; i++ )
    {
        // 0x8000 is an active flag, meaning to skip this sprite, similar
        // to the ADC bit on the ps2.
        if( (pPositions[i].GetIW() & 0x8000) != 0x8000 )
        {
            vector3 Center( pPositions[i].GetX(), pPositions[i].GetY(), pPositions[i].GetZ() );
            Center = S2V * Center;

            // calc the four sprite corners
            vector3 Corners[4];
            f32 Sine, Cosine;
            x_sincos( -pRotScales[i].X, Sine, Cosine );

            vector3 v0( Cosine - Sine, Sine + Cosine, 0.0f );
            vector3 v1( Cosine + Sine, Sine - Cosine, 0.0f );
            Corners[0] = v0;
            Corners[1] = v1;
            Corners[2] = -v0;
            Corners[3] = -v1;
            for( j = 0; j < 4; j++ )
            {
                Corners[j].Scale( pRotScales[i].Y * UniScale );
                Corners[j] += Center;
            }

            // now render it through draw
            xcolor C( pColors[i]&0xff, (pColors[i]&0xff00)>>8, (pColors[i]&0xff0000)>>16, (pColors[i]&0xff000000)>>24 );

            draw_Color( C );
            draw_UV( 0.0f, 0.0f );  draw_Vertex( Corners[0] );
            draw_UV( 1.0f, 0.0f );  draw_Vertex( Corners[3] );
            draw_UV( 0.0f, 1.0f );  draw_Vertex( Corners[1] );
            draw_UV( 1.0f, 0.0f );  draw_Vertex( Corners[3] );
            draw_UV( 0.0f, 1.0f );  draw_Vertex( Corners[1] );
            draw_UV( 1.0f, 1.0f );  draw_Vertex( Corners[2] );
        }
    }

    // finished
    draw_End();
}

//=============================================================================

static
void platform_RenderVelocitySprites( s32            nSprites,
                                     f32            UniScale,
                                     const matrix4* pL2W,
                                     const matrix4* pVelMatrix,
                                     const vector4* pPositions,
                                     const vector4* pVelocities,
                                     const u32*     pColors )
{
    if( !SWITCH_USE_TEXTURES )
    {
        return;
    }

    // sanity check
    ASSERTS( s_pDrawBitmap, "You must set a material first!" );
    if( nSprites == 0 )
        return;

    // start up draw
    draw_ClearL2W();
    draw_Begin( DRAW_TRIANGLES, s_DrawFlags|DRAW_XBOX_NO_BEGIN|DRAW_CULL_NONE );
    if( s_bGlowing )
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL );

    // Now link the shaders to apply our predetermined effect
    // Over saturation unnecessary because it will be applied
    // during the post effect.

    g_pShaderMgr->Link( 0,NULL,s_VSFlags,NULL,s_PSFlags,0 );

    draw_Begin( DRAW_TRIANGLES,DRAW_KEEP_STATES );
    draw_SetTexture( *s_pDrawBitmap );

    // Grab out a l2w matrix to use. If one is not specified, then
    // we will use the identity matrix.
    matrix4 L2W;
    if( pL2W )
        L2W = *pL2W;
    else
        L2W.Identity();

    // calculate the velocity l2w matrix
    matrix4 L2WNoTranslate = L2W;
    L2WNoTranslate.ClearTranslation();
    matrix4 VL2W = L2WNoTranslate * (*pVelMatrix);

    // grab out the view direction
    vector3 ViewDir = eng_GetView()->GetViewZ();

    // render the sprites
    s32 i;
    for( i = 0; i < nSprites; i++ )
    {
        // 0x8000 is an active flag, meaning to skip this sprite, similar
        // to the ADC bit on the ps2.
        if( (pPositions[i].GetIW() & 0x8000) != 0x8000 )
        {
            // calculate the sprite points
            vector3 P = L2W * vector3( pPositions[i].GetX(), pPositions[i].GetY(), pPositions[i].GetZ() );

            vector3 Right( pVelocities[i].GetX(), pVelocities[i].GetY(), pVelocities[i].GetZ() );
            Right = VL2W * Right;
            Right.Normalize();
            vector3 Up   = ViewDir.Cross( Right );
            Right *= pVelocities[i].GetW()*UniScale;
            Up    *= pVelocities[i].GetW()*UniScale;
            vector3 Fore = P + Right;
            vector3 Aft  = P - Right;
            vector3 V0   = Fore - Up;
            vector3 V1   = Aft  - Up;
            vector3 V2   = Aft  + Up;
            vector3 V3   = Fore + Up;

            // now render it through draw
            xcolor C( pColors[i]&0xff, (pColors[i]&0xff00)>>8, (pColors[i]&0xff0000)>>16, (pColors[i]&0xff000000)>>24 );

            draw_Color( C );
            draw_UV( 1.0f, 0.0f );  draw_Vertex( V0 );
            draw_UV( 0.0f, 0.0f );  draw_Vertex( V1 );
            draw_UV( 1.0f, 1.0f );  draw_Vertex( V3 );
            draw_UV( 0.0f, 0.0f );  draw_Vertex( V1 );
            draw_UV( 1.0f, 1.0f );  draw_Vertex( V3 );
            draw_UV( 0.0f, 1.0f );  draw_Vertex( V2 );
        }
    }

    // finished
    draw_End();
}

//=============================================================================

static
void platform_RenderHeatHazeSprites( s32               nSprites,
                                     f32               UniScale,
                                     const matrix4*    pL2W,
                                     const vector4*    pPositions,
                                     const vector2*    pRotScales,
                                     const u32*        pColors )
{
    //#ifdef bhapgood
    //#error DBS: Hey Bryon, Im putting this compile bomb in here for you, but we
    //#error will need to implement this for the xbox
    //#endif
    (void)nSprites;
    (void)UniScale;
    (void)pL2W;
    (void)pPositions;
    (void)pRotScales;
    (void)pColors;
}

//=============================================================================

static
void platform_BeginZPrime( void )
{
    s_bMatCanReceiveShadow = TRUE;
    g_pPipeline->BeginZPrime();
}

//=============================================================================

static
void platform_EndZPrime( void )
{
    g_pPipeline->EndZPrime();
}

//=============================================================================

static
void platform_BeginLightMap( void )
{
    g_pPipeline->BeginLightMap();
}

//=============================================================================

static
void platform_EndLightMap( void )
{
    g_pPipeline->EndLightMap();
}

//=============================================================================

static
void platform_RegisterMaterial( material& Material )
{
    (void)Material;
}
