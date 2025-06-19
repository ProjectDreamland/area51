//=============================================================================
//
//  PS2 Specific Render Routines
//
//=============================================================================

#include "../LightMgr.hpp"
#include "../platform_Render.hpp"
#include "vu1/vu1.hpp"


//=============================================================================
//=============================================================================
// Types and constants specific to the ps2-implementation
//=============================================================================
//=============================================================================

struct tex1_packet
{
    dmatag  DMA;
    giftag  GIF;
    u64     Tex1Data;
    u64     Tex1Addr;
};

static const f32 kDetailMinMipBias = -6.0f;
static const f32 kDetailMaxMipBias = -4.0f;
static const f32 kDetailMipScale   = 1.0f;
static const s32 kDetailMipL       = 1;
static const s32 kMipMapMode       = 4;          // 4 = Normal  5 = Tri-Linear
static const s32 kDetailMipMapMode = 5;          // 4 = Normal  5 = Tri-Linear
static const s32 kShadowTexSize    = 128;
static const s32 kShadowAlpha      = 0x60;
static const s32 kEnvTexW          = 64;
static const s32 kEnvTexH          = 64;

//=============================================================================
//=============================================================================
// Static data specific to the ps2-implementation
//=============================================================================
//=============================================================================

static matrix4                  s_W2V           PS2_ALIGNMENT( 64 );
static f32                      s_ScreenDist;
static f32                      s_DetailScale;
static s32                      s_nDiffuseMips;
static s32                      s_nDetailMips;
static f32                      s_DetailMipK;
static xbool                    s_DetailMapPresent;
static xbool                    s_bMatCanReceiveShadow;
static bbox                     s_CurrentBBox;
static s32                      s_SkinMode;          // -1 not active, 0 is unclipped, 1 is clipped
static skin_geom::dlist_ps2*    s_pCurrSkinDList;
static u8                       s_ShadowPalette[1024]  PS2_ALIGNMENT( 16 );   // TODO: Try to make this is a 4-bit palette
static s32                      s_hShadowClut = -1;

//=============================================================================
// static function declarations so we can do X_SECTIONs
//=============================================================================

static f32   ps2_FastLog2           ( f32 x )                             X_SECTION( render_deferred );
static xbool ps2_DetailMapVisible   ( const matrix4& L2W, const bbox& B ) X_SECTION( render_deferred );
static void  ps2_SetMipK            ( f32 WorldPixelSize )                X_SECTION( render_deferred );
static void  ClearVRAM              ( s32 DstBase,
                                      s32 Width,
                                      s32 Height,
                                      xcolor Color,
                                      u32 Mask,
                                      xbool ClearZBuffer )                X_SECTION( render_infrequent );

//=============================================================================
//=============================================================================
// Functions specific to  the pc implementation
//=============================================================================
//=============================================================================

static
f32 ps2_FastLog2( f32 x )
{
    // trick to get a really fast log2 approximation for floating-point numbers.

    // float = sign.exp.mantissa (1.8.23 bits)
    //       = sign * 2^(exp+127) * 1.mantissa
    // method 1 using integer instructions:
    //        a) mask out and shift the exponent
    //        b) subtract 127
    // method 2 using floating-point instructions:
    //        a) treat the float as an integer, and convert it to float again
    //           when the float is looked at as an integer the upper bits have the exponent,
    //           which are the important ones. Convert that integer to float, and we have one
    //           big-ass floating-point number
    //        b) divide by 2^23 to rescale the decimal point to its proper range
    //        c) subtract 127.0f
    // With both methods, the lower bits are essentially ignored, so this is only
    // a rough approximation, and you should use some other algorithm to get accurate results.
    //
    // As a side note...converting integer to float is approximately in the form y=A*log2(x)+B,
    // and converting float to integer is the inverse x=pow(2.0, (y-B)/A). With this knowledge,
    // you can convert a "float" to a logarithmic space by doing a float->float conversion,
    // multiply by any power, add an approriate offset, and convert it back to linear space by
    // doing a float->int conversion. With this idea you can approximate any power. Useful for
    // inverse sqrts, sqrts, specular powers, etc...

    s32 i = reinterpret_cast<s32 &>(x);
    f32 f = f32(i);
    f *= 0.00000011920928955078125f;
    f -= 127.0f;
    return f;
}

//=============================================================================

static
xbool ps2_DetailMapVisible( const matrix4& L2W, const bbox& B )
{
    asm __volatile__ ( "pref 0, 0x00(%0)" : : "r" (&L2W) );

    vector3 Center( B.GetCenter() );
    f32     Radius = B.GetRadius();
    const matrix4& W2V = s_W2V;
    Center = W2V * (L2W * Center);
    f32 MinZ = Center.GetZ() - Radius;
    
    // get the z range
    if( MinZ < 0.01f ) MinZ = 0.01f;

    // Compute max lod
    f32 MinLOD = 2.0f * ps2_FastLog2(MinZ) + s_DetailMipK;     // "2.0f*" ==== "<<s_DetailMipL"

    return ( MinLOD < 1.0f );
}

//=============================================================================

static
void ps2_SetMipK( f32 WorldPixelSize )
{
    // calculate the diffuse MipK
    f32 DiffuseMipK = vram_ComputeMipK( WorldPixelSize, s_ScreenDist );

    // caclulate the DetailMipK--this is a bit of a fudged calculation,
    // but it seems to work well enough. (smoke & mirrors, baby!)
    f32 Scale = (s_DetailScale > 0.001f) ? 1.0f / s_DetailScale : Scale;
    f32 DetailMipK = vram_ComputeMipK( WorldPixelSize * s_DetailScale, s_ScreenDist );
    f32 t = (s_DetailScale - 1.0f) / 3.0f;
    f32 DetailMipBias = kDetailMinMipBias + t * (kDetailMaxMipBias - kDetailMinMipBias);

    u64 Diffuse = SCE_GS_SET_TEX1(
                  0,                       // LCM
                  s_nDiffuseMips,          // MXL
                  1,                       // MMAG
                  kMipMapMode,             // MMIN
                  0,                       // MTBA
                  0,                       // L
                  (s32)(DiffuseMipK * 16.0f) );   // K

    s_DetailMipK = DetailMipK * kDetailMipScale + DetailMipBias;
    u64 Detail  = SCE_GS_SET_TEX1(
                  0,                       // LCM
                  s_nDetailMips,           // MXL
                  1,                       // MMAG
                  kDetailMipMapMode,       // MMIN
                  0,                       // MTBA
                  kDetailMipL,             // L
                  (s32)(s_DetailMipK* 16.0f) );   // K

    // Set Diffuse pass TEX1 on Context 1
    DLPtrAlloc( pPacket, tex1_packet );
    pPacket->DMA.SetCont( sizeof(tex1_packet) - sizeof(dmatag) );
    pPacket->DMA.MakeDirect();
    pPacket->GIF.BuildRegLoad( 1, TRUE );
    pPacket->Tex1Data = Diffuse;
    pPacket->Tex1Addr = SCE_GS_TEX1_1;

    // Upload Detail map TEX1 to VU memory
    g_VU1.SetDetailTex1( Detail );
}

//=============================================================================
//=============================================================================
// Implementation
//=============================================================================
//=============================================================================

static
void platform_Init( void )
{
    // Ensure the render flags in render.hpp and vu1.hpp match up!
    ASSERT( render::INSTFLAG_CLIPPED        == VU1_CLIPPED            );
    ASSERT( render::INSTFLAG_GLOWING        == VU1_GLOWING            );
    ASSERT( render::INSTFLAG_SHADOW_PASS    == VU1_SHADOW_PASS        );
    ASSERT( render::INSTFLAG_FILTERLIGHT    == VU1_FILTERLIGHT        );
    ASSERT( render::INSTFLAG_SPOTLIGHT      == VU1_SPOTLIGHT          );
    ASSERT( render::INSTFLAG_FADING_ALPHA   == VU1_FADING_ALPHA       );
    ASSERT( render::INSTFLAG_DYNAMICLIGHT   == VU1_DYNAMICLIGHT       );
    ASSERT( render::INSTFLAG_DETAIL         == VU1_DETAIL             );
    ASSERT( render::INSTFLAG_PROJ_SHADOW_1  == (VU1_PROJ_SHADOW_1<<8) );
    ASSERT( render::INSTFLAG_PROJ_SHADOW_2  == (VU1_PROJ_SHADOW_2<<8) );
    ASSERT( render::GetHardwareBufferSize() == VU1_VERTS              );

    s_nDiffuseMips     = 0;
    s_nDetailMips      = 0;
    s_DetailScale      = 1.0f;
    s_DetailMapPresent = FALSE;
    s_DetailMipK       = 0.0f;
    s_pCurrSkinDList   = NULL;

    // we need some permanent memory for the cube map and accumlating the glow buffer
    s32 EnvPagesNeeded   = (kEnvTexW*kEnvTexH*4)/8192;
    s32 GlowPagesNeeded  = ((VRAM_FRAME_BUFFER_WIDTH>>3)*(VRAM_FRAME_BUFFER_WIDTH>>3))*4/8192;
    vram_AllocatePermanent( EnvPagesNeeded + GlowPagesNeeded );

    //TODO: Get the fading in here
    x_memset( s_ShadowPalette, kShadowAlpha, sizeof(s_ShadowPalette) );
    s_ShadowPalette[0] = 0x80;
    s_ShadowPalette[1] = 0x80;
    s_ShadowPalette[2] = 0x80;
    s_ShadowPalette[3] = 0x80;
    s_hShadowClut = vram_RegisterClut( s_ShadowPalette, 256 );  // TODO: Try to make this a 4-bit palette

    platform_InitPostEffects();
}

//=============================================================================

static
void platform_Kill( void )
{
    platform_KillPostEffects();

    vram_FreePermanent();
}

//=============================================================================

static
void platform_StartRawDataMode( void )
{
    // Initialize VU1
    g_VU1.Begin();

    // draw and other systems like to muck with our alpha tests, etc. so reset them
    gsreg_Begin( 3 );
    gsreg_SetFBMASK( 0x00000000 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PushGSContext( 1 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PopGSContext();
    gsreg_End();
}

//=============================================================================

static
void platform_EndRawDataMode( void )
{
    g_VU1.End();
}

//=============================================================================

static
void platform_RenderRawStrips( s32              nVerts,
                              const matrix4&    L2W,
                              const vector4*    pPos,
                              const s16*        pUV,
                              const u32*        pColor )
{
    g_VU1.RenderRawStrips( nVerts, L2W, pPos, pUV, pColor );
}

//=============================================================================

static
void platform_Render3dSprites( s32               nSprites,
                               f32               UniScale,
                               const matrix4*    pL2W,
                               const vector4*    pPositions,
                               const vector2*    pRotScales,
                               const u32*        pColors )
{
    g_VU1.Render3dSprites( nSprites, UniScale, pL2W, pPositions, pRotScales, pColors );
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
    g_VU1.RenderVelocitySprites( nSprites, UniScale, pL2W, pVelMatrix, pPositions, pVelocities, pColors );
}

//=============================================================================

static
void platform_RenderHeatHazeSprites( s32            nSprites,
                                     f32            UniScale,
                                     const matrix4* pL2W,
                                     const vector4* pPositions,
                                     const vector2* pRotScales,
                                     const u32*     pColors )
{
    g_VU1.RenderHeatHazeSprites( nSprites, UniScale, pL2W, pPositions, pRotScales, pColors );
}

//=============================================================================

static
void platform_SetDiffuseMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestEnabled )
{
    g_VU1.SetDiffuseMaterial( Bitmap, BlendMode, ZTestEnabled );
}

//=============================================================================

static
void platform_SetGlowMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestEnabled )
{
    g_VU1.SetGlowMaterial( Bitmap, BlendMode, ZTestEnabled );
}

//=============================================================================

static
void platform_SetEnvMapMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestEnabled )
{
    g_VU1.SetEnvMapMaterial( Bitmap, BlendMode, ZTestEnabled );
}

//=============================================================================

static
void platform_SetDistortionMaterial( s32 BlendMode, xbool ZTestEnabled )
{
    g_VU1.SetDistortionMaterial( BlendMode, ZTestEnabled );
}

//=============================================================================

static
void platform_ActivateMaterial( const material& Material )
{
    CONTEXT("platform_ActivateMaterial") ;

    s_nDiffuseMips     = Material.m_nDiffuseMips;
    s_nDetailMips      = Material.m_nDetailMips;
    s_DetailScale      = Material.m_DetailScale;
    s_DetailMapPresent = !!(Material.m_Flags & geom::material::FLAG_HAS_DETAIL_MAP);
    if ( IsAlphaMaterial( (material_type)Material.m_Type ) && !(Material.m_Flags & geom::material::FLAG_FORCE_ZFILL) )
        s_bMatCanReceiveShadow = FALSE;
    else    
        s_bMatCanReceiveShadow = TRUE;

    g_VU1.SetMaterial( Material );
}

//=============================================================================

static
void platform_ActivateDistortionMaterial( const material* pMaterial,
                                          const radian3&  NormalRot )
{
    CONTEXT("platform_ActivateMaterial") ;

    // set any statics needed for calculating mip k values (not totally clean, but
    // neither is the ps2!)
    s_nDiffuseMips         = 0;
    s_nDetailMips          = 0;
    s_DetailScale          = 1.0f;
    s_DetailMapPresent     = FALSE;
    s_bMatCanReceiveShadow = FALSE;

    // set up the distortion material
    g_VU1.SetDistortionMaterial( pMaterial, NormalRot );
}

//=============================================================================

static
void platform_ActivateZPrimeMaterial( void )
{
    CONTEXT("platform_ActivateMaterial") ;

    // set any statics needed for calculating mip k values (not totally clean, but
    // neither is the ps2!)
    s_nDiffuseMips         = 0;
    s_nDetailMips          = 0;
    s_DetailScale          = 1.0f;
    s_DetailMapPresent     = FALSE;
    s_bMatCanReceiveShadow = FALSE;

    // activate it!
    g_VU1.SetZPrimeMaterial();
}

//=============================================================================

static
void platform_RegisterMaterial( material& Mat )
{
    // pre-compute as much of the microcode registers we can to make the
    // run-time as simple as possible

    // store out pointers to the textures so we don't have to go through the
    // more expensive GetPointer function at run-time
    if( (Mat.m_Type == Material_Distortion) || (Mat.m_Type == Material_Distortion_PerPolyEnv) )
        Mat.m_pDiffuseTex = NULL;
    else
        Mat.m_pDiffuseTex = Mat.m_DiffuseMap.GetPointer();

    if( Mat.m_Flags & geom::material::FLAG_HAS_DETAIL_MAP )
        Mat.m_pDetailTex = Mat.m_DetailMap.GetPointer();
    else
        Mat.m_pDetailTex = NULL;

    xbool bUseEnvMap  = FALSE;
    xbool bNotCubeMap = FALSE;
    if( (Mat.m_Type == Material_Diff_PerPixelEnv) || (Mat.m_Type == Material_Alpha_PerPolyEnv) )
    {
        bUseEnvMap  = TRUE;
        bNotCubeMap = !(Mat.m_Flags & geom::material::FLAG_ENV_CUBE_MAP);
    }
    else if( Mat.m_Type == Material_Distortion_PerPolyEnv )
    {
        bUseEnvMap  = TRUE;
        bNotCubeMap = TRUE;
    }
    
    if( bUseEnvMap && bNotCubeMap )
        Mat.m_pEnvTex = Mat.m_EnvironmentMap.GetPointer();
    else
        Mat.m_pEnvTex = NULL;

    // Env map data if necessary. Note that if we are in world space
    // we need identity. If we are in view space, then that will have
    // to be supplied at run-time, and everything else doesn't need the
    // vectors, but to be safe we'll zero them out.
    Mat.m_EnvMapVectors[0].Zero();
    Mat.m_EnvMapVectors[1].Zero();
    if( (Mat.m_Flags & geom::material::FLAG_HAS_ENV_MAP) &&
        (Mat.m_Flags & geom::material::FLAG_ENV_WORLD_SPACE) )
    {
        Mat.m_EnvMapVectors[0].Set( 1.0f, 0.0f, 0.0f, 0.0f );
        Mat.m_EnvMapVectors[1].Set( 0.0f, 1.0f, 0.0f, 0.0f );
    }

    // diffuse alpha register
    switch( Mat.m_Type )
    {
    default:
        ASSERT( FALSE );
    
    case Material_Diff:
    case Material_Diff_PerPixelEnv:
    case Material_Diff_PerPixelIllum:
        // diffuse pass is a straight copy into the frame buffer
        Mat.m_DiffuseAlphaReg = SCE_GS_SET_ALPHA( C_SRC, C_DST, A_FIX, C_DST, 0x80 );
        break;
    case Material_Alpha:
    case Material_Alpha_PerPolyEnv:
    case Material_Alpha_PerPixelIllum:
    case Material_Alpha_PerPolyIllum:
        {
            // diffuse pass is alpha'd into the frame buffer
            if( Mat.m_Flags & geom::material::FLAG_IS_ADDITIVE )
                Mat.m_DiffuseAlphaReg = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_SRC, C_DST, 0 );
            else if( Mat.m_Flags & geom::material::FLAG_IS_SUBTRACTIVE )
                Mat.m_DiffuseAlphaReg = SCE_GS_SET_ALPHA( C_ZERO, C_SRC, A_SRC, C_DST, 0 );
            else
                Mat.m_DiffuseAlphaReg = SCE_GS_SET_ALPHA( C_SRC, C_DST, A_SRC, C_DST, 0 );
        }
        break;
    case Material_Distortion:
    case Material_Distortion_PerPolyEnv:
        // diffuse pass is alpha with not quite one to make it slightly darker
        Mat.m_DiffuseAlphaReg = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_FIX, C_ZERO, 0x70 );
        break;
    }

    // Env/Illum alpha register
    switch( Mat.m_Type )
    {
    default:
        ASSERT( FALSE );
    case Material_Diff:
    case Material_Alpha:
    case Material_Diff_PerPixelEnv:
    case Material_Distortion:
        {
            // per-pixel environment is either a normal or additive alpha blend into
            // the frame buffer using dest alpha as the mask (context 2)
            if( Mat.m_Flags & geom::material::FLAG_IS_ADDITIVE )
                Mat.m_EnvIllumAlphaReg = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_DST, C_DST, 0 );
            else if( Mat.m_Flags & geom::material::FLAG_IS_SUBTRACTIVE )
                Mat.m_EnvIllumAlphaReg = SCE_GS_SET_ALPHA( C_ZERO, C_SRC, A_DST, C_DST, 0 );
            else
                Mat.m_EnvIllumAlphaReg = SCE_GS_SET_ALPHA( C_SRC, C_DST, A_DST, C_DST, 0 );
            Mat.m_EnvIllumAlphaAddr = SCE_GS_ALPHA_2;
        }
        break;
    case Material_Diff_PerPixelIllum:
        // per-pixel illumination is the diffuse pass in decal mode written to the
        // frame buffer using dest alpha as a mask (context 1)
        Mat.m_EnvIllumAlphaReg  = SCE_GS_SET_ALPHA( C_SRC, C_DST, A_DST, C_DST, 0 );
        Mat.m_EnvIllumAlphaAddr = SCE_GS_ALPHA_1;
        break;

    case Material_Alpha_PerPolyEnv:
    case Material_Distortion_PerPolyEnv:
        // per poly environment map is either a normal or additive alpha blend into
        // the frame buffer using a fixed alpha (context 2)
        {
            s32 FixedAlpha = (s32)(128.0f * Mat.m_FixedAlpha);

            if( Mat.m_Flags & geom::material::FLAG_IS_ADDITIVE )
                Mat.m_EnvIllumAlphaReg  = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_FIX, C_DST, FixedAlpha );
            else if( Mat.m_Flags & geom::material::FLAG_IS_SUBTRACTIVE )
                Mat.m_EnvIllumAlphaReg  = SCE_GS_SET_ALPHA( C_ZERO, C_SRC, A_FIX, C_DST, FixedAlpha );
            else
                Mat.m_EnvIllumAlphaReg  = SCE_GS_SET_ALPHA( C_SRC, C_DST, A_FIX, C_DST, FixedAlpha );
            Mat.m_EnvIllumAlphaAddr = SCE_GS_ALPHA_2;
        }
        break;

    case Material_Alpha_PerPixelIllum:
    case Material_Alpha_PerPolyIllum:
        // illum with alpha will just render the diffuse pass in decal mode
        // using source alpha as the mask (context 1)
        {
            if( Mat.m_Flags & geom::material::FLAG_IS_ADDITIVE )
                Mat.m_EnvIllumAlphaReg = SCE_GS_SET_ALPHA( C_SRC, C_ZERO, A_SRC, C_DST, 0 );
            else if( Mat.m_Flags & geom::material::FLAG_IS_SUBTRACTIVE )
                Mat.m_EnvIllumAlphaReg = SCE_GS_SET_ALPHA( C_ZERO, C_SRC, A_SRC, C_DST, 0 );
            else
                Mat.m_EnvIllumAlphaReg = SCE_GS_SET_ALPHA( C_SRC, C_DST, A_SRC, C_DST, 0 );
            Mat.m_EnvIllumAlphaAddr = SCE_GS_ALPHA_1;
        }
        break;
    }

    // mcode flags
    Mat.m_MCodeFlags = 0;
    if( Mat.m_Flags & geom::material::FLAG_HAS_DETAIL_MAP )
        Mat.m_MCodeFlags |= DETAIL_PASS;
    if( Mat.m_UVAnim.nFrames > 0 )
        Mat.m_MCodeFlags |= UVSCROLL_PASS;

    switch( Mat.m_Type )
    {
    default:
        ASSERT( FALSE );

    case Material_Diff:
        Mat.m_MCodeFlags |= DIFFUSE_PASS;
        break;

    case Material_Alpha:
        Mat.m_MCodeFlags |= DIFFUSE_PASS;
        break;

    case Material_Diff_PerPixelEnv:
    case Material_Alpha_PerPolyEnv:
        Mat.m_MCodeFlags |= DIFFUSE_PASS | ENVIRONMENT_PASS;
        if( Mat.m_Flags & geom::material::FLAG_ENV_CUBE_MAP )
            Mat.m_MCodeFlags |= USE_POSITIONS;
        else if( Mat.m_Flags & geom::material::FLAG_ENV_WORLD_SPACE )
            Mat.m_MCodeFlags |= USE_WORLDSPACE;
        break;
    
    case Material_Diff_PerPixelIllum:
        Mat.m_MCodeFlags |= DIFFUSE_PASS | SELFILLUM_PASS;
        if( Mat.m_Flags & geom::material::FLAG_ILLUM_USES_DIFFUSE )
            Mat.m_MCodeFlags |= SELFILLUM_DIFFUSELIT;
        break;
    
    case Material_Alpha_PerPixelIllum:
        Mat.m_MCodeFlags |= SELFILLUM_PASS;
        if( Mat.m_Flags & geom::material::FLAG_ILLUM_USES_DIFFUSE )
            Mat.m_MCodeFlags |= SELFILLUM_DIFFUSELIT;
        break;

    case Material_Alpha_PerPolyIllum:
        Mat.m_MCodeFlags |= SELFILLUM_PASS | SELFILLUM_PERPOLY;
        if( Mat.m_Flags & geom::material::FLAG_ILLUM_USES_DIFFUSE )
            Mat.m_MCodeFlags |= SELFILLUM_DIFFUSELIT;
        break;

    case Material_Distortion:
        Mat.m_MCodeFlags |= DIFFUSE_PASS | DISTORTION_PASS;
        break;
    
    case Material_Distortion_PerPolyEnv:
        Mat.m_MCodeFlags |= DIFFUSE_PASS | DISTORTION_PASS | ENVIRONMENT_PASS;
        break;
    }

    // per poly illum value
    if( Mat.m_Type == Material_Alpha_PerPolyIllum )
    {
        Mat.m_PerPolyAlpha = (u32)(128.0f * Mat.m_FixedAlpha);
    }
    else
    {
        Mat.m_PerPolyAlpha = 0;
    }

    // copy out the diffuse mips and detail mip counts
    if( (Mat.m_Type == Material_Distortion) || (Mat.m_Type == Material_Distortion_PerPolyEnv) )
        Mat.m_nDiffuseMips = 0;
    else
        Mat.m_nDiffuseMips = Mat.m_pDiffuseTex->m_Bitmap.GetNMips();

    if( Mat.m_Flags & geom::material::FLAG_HAS_DETAIL_MAP )
        Mat.m_nDetailMips = Mat.m_pDetailTex->m_Bitmap.GetNMips();
    else
        Mat.m_nDetailMips = 0;
}

//=============================================================================

static
void platform_RegisterSkinGeom( skin_geom& Geom  )
{
    (void)Geom;
}

//=============================================================================

static
void platform_UnregisterSkinGeom( skin_geom& Geom  )
{
    (void)Geom;
}

//=============================================================================

static
void platform_RegisterRigidInstance( rigid_geom& Geom, render::hgeom_inst hInst )
{
    (void)Geom;
    (void)hInst;
}

//=============================================================================

static
void platform_RegisterSkinInstance( skin_geom& Geom, render::hgeom_inst hInst )
{
    (void)Geom;
    (void)hInst;
}

//=============================================================================

static
void platform_UnregisterRigidInstance( render::hgeom_inst hInst )
{
    (void)hInst;
}

//=============================================================================

static
void platform_UnregisterSkinInstance( render::hgeom_inst hInst )
{
    (void)hInst;
}

//=============================================================================

#ifdef X_EDITOR
static
void* platform_LockRigidDListVertex( render::hgeom_inst hInst, s32 iSubMesh )
{
    (void)hInst;
    (void)iSubMesh;
    return NULL;
}

//=============================================================================

static
void platform_UnlockRigidDListVertex( render::hgeom_inst hInst, s32 iSubMesh )
{
    (void)hInst;
    (void)iSubMesh;
}

//=============================================================================

static
void* platform_LockRigidDListIndex( render::hgeom_inst hInst, s32 iSubMesh,  s32& VertexOffset )
{
    (void)hInst;
    (void)iSubMesh;
    (void)VertexOffset;
    return NULL;
}

//=============================================================================

static
void platform_UnlockRigidDListIndex( render::hgeom_inst hInst, s32 iSubMesh )
{
    (void)hInst;
    (void)iSubMesh;
}
#endif

//=============================================================================

static
void* platform_CalculateRigidLighting( const matrix4&   L2W,
                                       const bbox&      WorldBBox )
{
    CONTEXT("platform_CalculateRigidLighting") ;

    void* pLighting = NULL;

    s32 NLights = g_LightMgr.CollectLights( WorldBBox );
    if ( NLights )
    {
        pLighting = g_VU1.NewLightingSetup();
        for ( s32 iLight = 0; iLight < NLights; iLight++ )
        {
            vector3 Pos;
            f32     Radius;
            xcolor  Color;
            g_LightMgr.GetCollectedLight( iLight, Pos, Radius, Color );
            g_VU1.SetLight( pLighting, iLight, L2W, Pos, Radius, Color );
        }
    }

    return pLighting;
}

//=============================================================================

//#define WEAPON_LIGHTING_TESTS

#if defined(dstewart) && defined(WEAPON_LIGHTING_TESTS)

inline xcolor ColorMax( xcolor Color, u8 Clamp )
{
    return xcolor( MAX( Color.R, Clamp ),
                   MAX( Color.G, Clamp ),
                   MAX( Color.B, Clamp ),
                   Color.A );
}

inline xcolor MultColors( xcolor LHS, xcolor RHS )
{
    s32 R = ((s32)LHS.R * (s32)RHS.R)/128;
    s32 G = ((s32)LHS.G * (s32)RHS.G)/128;
    s32 B = ((s32)LHS.B * (s32)RHS.B)/128;
    s32 A = ((s32)LHS.A * (s32)RHS.A)/128;
    return xcolor( (u8)R, (u8)G, (u8)B, (u8)A );
}

static vector3 g_ForcedWeaponLightDir0( 1.0f, -1.0f, 0.0f );
static vector3 g_ForcedWeaponLightDir1( -1.0f, 1.0f, 0.0f );
static xcolor  g_ForcedWeaponLightCol0( 128, 100, 100, 128 );
static xcolor  g_ForcedWeaponLightCol1( 100, 100, 108, 128 );
static u8      g_ForcedWeaponClamp0( 32 );
static u8      g_ForcedWeaponClamp1( 20 );
static xcolor  g_ForcedWeaponAmbient( 0, 0, 0, 128 );
static xcolor  g_ForcedWeaponCharInfluence( 64, 64, 64, 128 );

#endif

//=============================================================================

static void*   platform_CalculateSkinLighting( u32            Flags,
                                               const matrix4& L2W,
                                               const bbox&    BBox,
                                               xcolor         Ambient )
{
#if defined(dstewart) && defined(WEAPON_LIGHTING_TESTS)
    (void)BBox;
    (void)Ambient;

    void* pLighting = g_VU1.NewLightingSetup();
    g_VU1.SetSkinAmbient( pLighting, g_ForcedWeaponAmbient, Flags );

    // build a new l2w without pitch for lighting purposes
    radian3 Rot = L2W.GetRotation();
    Rot.Pitch = R_0;
    matrix4 TmpL2W( vector3(1.0f,1.0f,1.0f), Rot, vector3(0.0f, 0.0f, 0.0f) );

    // handle light 0
    xcolor LightCol = MultColors( g_ForcedWeaponLightCol0, Ambient );
    LightCol = ColorMax( LightCol, g_ForcedWeaponClamp0 );
    vector3 LightDir = g_ForcedWeaponLightDir0;
    LightDir.Normalize();
    LightDir = TmpL2W.RotateVector( LightDir );
    g_VU1.SetSkinLight( pLighting, 0, LightDir, LightCol, Flags );

    // handle light 1
    LightCol = MultColors( g_ForcedWeaponLightCol1, Ambient );
    LightCol = ColorMax( LightCol, g_ForcedWeaponClamp1 );
    LightDir = g_ForcedWeaponLightDir1;
    LightDir.Normalize();
    LightDir = TmpL2W.RotateVector( LightDir );
    g_VU1.SetSkinLight( pLighting, 1, LightDir, LightCol, Flags );

    // grab a 3rd light from the character lights
    s32 NLights = g_LightMgr.CollectCharLights( L2W, BBox, 1 );
    if( NLights )
    {
        vector3 Dir;
        xcolor  Col;
        g_LightMgr.GetCollectedCharLight( 0, Dir, Col );
        Col = MultColors( Col, g_ForcedWeaponCharInfluence );
        g_VU1.SetSkinLight( pLighting, 2, Dir, Col, Flags );
    }

    return pLighting;
#else
    CONTEXT("platform_CalculateSkinLighting") ;

    void* pLighting = g_VU1.NewLightingSetup();
    g_VU1.SetSkinAmbient( pLighting, Ambient, Flags );

    // if we're doing self-illum glows don't use directional light
    if ( Flags & render::GLOWING )
    {
        return pLighting;
    }

    // collect directional lights
    s32 NLights = g_LightMgr.CollectCharLights( L2W, BBox );
    if ( NLights )
    {
        vector3 Dir;
        xcolor  Col;
        for ( s32 iLight = 0; iLight < NLights; iLight++ )
        {
            g_LightMgr.GetCollectedCharLight( iLight, Dir, Col );
            g_VU1.SetSkinLight( pLighting, iLight, Dir, Col, Flags );
        }
    }
    return pLighting;
#endif
}

//=============================================================================

static
void platform_BeginRigidGeom( geom* pGeom, s32 iSubMesh )
{
    // Get the SubMesh and DList we want to render
    rigid_geom& RigidGeom        = (rigid_geom&)(*pGeom);
    geom::submesh& GeomSubMesh   = RigidGeom.m_pSubMesh[iSubMesh];
    rigid_geom::dlist_ps2& DList = RigidGeom.m_System.pPS2[GeomSubMesh.iDList];

    // Set the MipK
    ps2_SetMipK( GeomSubMesh.WorldPixelSize );

    // Set the static data to be uploaded
    g_VU1.BeginInstance( DList.pPosition, DList.pUV, DList.pNormal, DList.nVerts );

    // store a copy of the bbox, because it will be needed by the detail map
    // visibility calculation
    s_CurrentBBox = pGeom->m_BBox;
}

//=============================================================================

static
void platform_RenderRigidInstance( render_instance& Inst )
{
    // accumulate flags
    if ( s_DetailMapPresent && ps2_DetailMapVisible( *Inst.Data.Rigid.pL2W, s_CurrentBBox ) )
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

    // add it to the vu1 instance list
    g_VU1.AddInstance( (u16*)Inst.Data.Rigid.pColInfo, *Inst.Data.Rigid.pL2W, Inst.UOffset, Inst.VOffset, Inst.Flags, Inst.Alpha, Inst.pLighting );
}

//=============================================================================

static
void platform_EndRigidGeom( void )
{
    g_VU1.EndInstance();
}

//=============================================================================

static
void platform_BeginSkinGeom( geom* pGeom, s32 iSubMesh )
{
    s_SkinMode = -1;

    // store out a pointer to the skin's dlist
    skin_geom&          SkinGeom    = (skin_geom&)(*pGeom);
    skin_geom::submesh& GeomSubMesh = SkinGeom.m_pSubMesh[iSubMesh];
    s_pCurrSkinDList                = &SkinGeom.m_System.pPS2[GeomSubMesh.iDList];

    // Set the MipK
    ps2_SetMipK( GeomSubMesh.WorldPixelSize );

    // store a copy of the bbox, because it will be needed by the lighting and
    // detail map visibility calculation
    s_CurrentBBox = pGeom->m_BBox;
}

//=============================================================================

static
void platform_RenderSkinInstance( render_instance& Inst )
{
    CONTEXT("platform_RenderSkinInstance") ;

    // sanity check
    ASSERT( s_pCurrSkinDList );

    // switch from clipped to non-clipped if necessary (or start if we're in
    // neither mode yet)
    if ( Inst.Flags & render::CLIPPED )
    {
        if ( s_SkinMode == 0 )  g_VU1.EndSkinRenderNormal();
        if ( s_SkinMode != 1 )  g_VU1.BeginSkinRenderClipped();
        s_SkinMode = 1;
    }
    else
    {
        if ( s_SkinMode == 1 )  g_VU1.EndSkinRenderClipped();
        if ( s_SkinMode != 0 )  g_VU1.BeginSkinRenderNormal();
        s_SkinMode = 0;
    }

    // copy the clipping flag over
    if ( Inst.Flags & render::CLIPPED )
        Inst.Flags |= render::INSTFLAG_CLIPPED;

    // copy the shadow pass flag over
    if ( (Inst.Flags & render::SHADOW_PASS) && s_bMatCanReceiveShadow )
        Inst.Flags |= render::INSTFLAG_SHADOW_PASS;

    // do we need the detail map?
    if ( s_DetailMapPresent &&
         ps2_DetailMapVisible( Inst.Data.Skin.pBones[0], s_CurrentBBox ) )
    {
        Inst.Flags |= render::INSTFLAG_DETAIL;
    }

    // copy the glowing flag over
    if ( Inst.Flags & render::GLOWING )
    {
        Inst.Flags |= render::INSTFLAG_GLOWING;
    }

    // copy the fading alpha flag over
    if ( Inst.Flags & render::FADING_ALPHA )
    {
        Inst.Flags |= render::INSTFLAG_FADING_ALPHA;
        Inst.Flags &= ~(render::INSTFLAG_DETAIL | render::INSTFLAG_SPOTLIGHT | render::INSTFLAG_SHADOW_PASS);
    }

    // render it
    g_VU1.SetSkinFlags    ( Inst.Flags, Inst.Alpha );
    g_VU1.SetSkinUVOffset ( Inst.UOffset, Inst.VOffset );
    g_VU1.SetSkinLightInfo( Inst.pLighting );
    
    s32 iCmd;
    for ( iCmd = 0; iCmd < s_pCurrSkinDList->nCmds; iCmd++ )
    {
        skin_geom::command_ps2 Cmd = s_pCurrSkinDList->pCmd[iCmd];

        switch( Cmd.Cmd )
        {
        default:
            break;

        case skin_geom::PS2_CMD_UPLOAD_MATRIX:
            {
                s16 BoneID  = Cmd.Arg1;
                s16 CacheID = Cmd.Arg2;
                g_VU1.UploadSkinMatrix( Inst.Data.Skin.pBones[BoneID], CacheID );
            }
            break;

        case skin_geom::PS2_CMD_RENDER_VERTS_RIGID:
            {
                s32  NVerts       = Cmd.Arg2;
                s16* pUV          = (s16*)&s_pCurrSkinDList->pUV[Cmd.Arg1];
                s16* pPos         = (s16*)&s_pCurrSkinDList->pPos[Cmd.Arg1];
                u8*  pBoneIndices = (u8*) &s_pCurrSkinDList->pBoneIndex[Cmd.Arg1];
                u8*  pNormals     = (u8*) &s_pCurrSkinDList->pNormal[Cmd.Arg1];

                if ( s_SkinMode )
                    g_VU1.RenderSkinVertsClipped( NVerts, pUV, pPos, pBoneIndices, pNormals );
                else
                    g_VU1.RenderSkinVertsNormalOneBone( NVerts, pUV, pPos, pBoneIndices, pNormals );
            }
            break;

        case skin_geom::PS2_CMD_RENDER_VERTS_SOFT:
            {
                s32  NVerts       = Cmd.Arg2;
                s16* pUV          = (s16*)&s_pCurrSkinDList->pUV[Cmd.Arg1];
                s16* pPos         = (s16*)&s_pCurrSkinDList->pPos[Cmd.Arg1];
                u8*  pBoneIndices = (u8*) &s_pCurrSkinDList->pBoneIndex[Cmd.Arg1];
                u8*  pNormals     = (u8*) &s_pCurrSkinDList->pNormal[Cmd.Arg1];
            
                if ( s_SkinMode )
                    g_VU1.RenderSkinVertsClipped( NVerts, pUV, pPos, pBoneIndices, pNormals );
                else
                    g_VU1.RenderSkinVertsNormalTwoBones( NVerts, pUV, pPos, pBoneIndices, pNormals );
            }
            break;

        case skin_geom::PS2_CMD_END:
            break;
        }
    }
}

//=============================================================================

static
void platform_EndSkinGeom( void )
{
    // clean up and finish off any microcode
    s_pCurrSkinDList = NULL;
    switch ( s_SkinMode )
    {
    default:
    case -1:    break;
    case 0:     g_VU1.EndSkinRenderNormal();  break;
    case 1:     g_VU1.EndSkinRenderClipped(); break;
    }
}

//=============================================================================

static
void platform_BeginShaders( void )
{
    // as a safety precaution for any REF'd data used by MFIFO, flush the
    // data cache
    FlushCache( WRITEBACK_DCACHE );

    // cache the active view so we don't get I$ misses going to entropy for it
    const view* pCurrView = eng_GetView();
    s_ScreenDist  = pCurrView->GetScreenDist();
    s_W2V         = pCurrView->GetW2V();

    // Update the shadow maps (must be before the Begin call!)
    g_VU1.SetNShadowProjectors( s_nShadowProjections );

    // Initialize VU1
    g_VU1.Begin();

    // draw and other systems like to muck with our alpha tests, etc. so reset them
    gsreg_Begin( 2 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PushGSContext( 1 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PopGSContext();
    gsreg_End();

    // Update filter light color
    g_VU1.SetFilterColor( render::GetFilterLightColor() );
}

//=============================================================================

static
void platform_EndShaders( void )
{
    g_VU1.End();
}

//=============================================================================

static
void platform_CreateEnvTexture( void )
{
    if ( s_pCurrCubeMap == NULL )
        return;

    // reset the viewport to the env. map size
    view OrigView = *eng_GetView();
    view EnvView  = OrigView;
    s32 OrigX0, OrigY0, OrigX1, OrigY1;
    EnvView.GetViewport( OrigX0, OrigY0, OrigX1, OrigY1 );
    EnvView.SetViewport( 0, 0, kEnvTexW, kEnvTexH );
    EnvView.SetPosition( vector3(0.0f,0.0f,0.0f) );
    eng_SetView( EnvView );

    // set the frame buffer to 32-bit and hide it in the permanently allocated
    // portion of vram
    gsreg_Begin( 2 );
    gsreg_Set( (SCE_GS_FRAME_1+eng_GetGSContext()), SCE_GS_SET_FRAME(vram_GetPermanentArea()/32, 1, SCE_GS_PSMCT32, 0) );
    gsreg_SetScissor( 0, 0, kEnvTexW, kEnvTexH );
    gsreg_End();

    // draw the env. map cube
    // render each side
    struct vertex{ f32 x, y, z, u, v; };
    static vertex Vertex[]={
    // top side                                       // Top/bottom, front/back, left/right
    { -1.0f,  1.0f, -1.0f, 0.0078125f, 0.9921875f },  // T-F-L
    { -1.0f,  1.0f,  1.0f, 0.0078125f, 0.0078125f },  // T-B-L
    {  1.0f,  1.0f, -1.0f, 0.9921875f, 0.9921875f },  // T-F-R
    {  1.0f,  1.0f,  1.0f, 0.9921875f, 0.0078125f },  // T-B-R
    // bottom side                                    // Top/bottom, front/back, left/right
    { -1.0f, -1.0f,  1.0f, 0.9921875f, 0.0078125f },  // B-B-L
    { -1.0f, -1.0f, -1.0f, 0.9921875f, 0.9921875f },  // B-F-L
    {  1.0f, -1.0f,  1.0f, 0.0078125f, 0.0078125f },  // B-B-R
    {  1.0f, -1.0f, -1.0f, 0.0078125f, 0.9921875f },  // B-F-R
    // front side                                     // Top/bottom, front/back, left/right
    { -1.0f, -1.0f, -1.0f, 0.0078125f, 0.9921875f },  // B-F-L
    { -1.0f,  1.0f, -1.0f, 0.0078125f, 0.0078125f },  // T-F-L
    {  1.0f, -1.0f, -1.0f, 0.9921875f, 0.9921875f },  // B-F-R
    {  1.0f,  1.0f, -1.0f, 0.9921875f, 0.0078125f },  // T-F-R
    // back side                                      // Top/bottom, front/back, left/right
    {  1.0f, -1.0f,  1.0f, 0.9921875f, 0.0078125f },  // B-B-R
    {  1.0f,  1.0f,  1.0f, 0.9921875f, 0.9921875f },  // T-B-R
    { -1.0f, -1.0f,  1.0f, 0.0078125f, 0.0078125f },  // B-B-L
    { -1.0f,  1.0f,  1.0f, 0.0078125f, 0.9921875f },  // T-B-L
    // left side                                      // Top/bottom, front/back, left/right
    { -1.0f, -1.0f,  1.0f, 0.0078125f, 0.0078125f },  // B-B-L
    { -1.0f,  1.0f,  1.0f, 0.9921875f, 0.0078125f },  // T-B-L
    { -1.0f, -1.0f, -1.0f, 0.0078125f, 0.9921875f },  // B-F-L
    { -1.0f,  1.0f, -1.0f, 0.9921875f, 0.9921875f },  // T-F-L
    // right side                                     // Top/bottom, front/back, left/right
    {  1.0f, -1.0f, -1.0f, 0.9921875f, 0.9921875f },  // B-F-R
    {  1.0f,  1.0f, -1.0f, 0.0078125f, 0.9921875f },  // T-F-R
    {  1.0f, -1.0f,  1.0f, 0.9921875f, 0.0078125f },  // B-B-R
    {  1.0f,  1.0f,  1.0f, 0.0078125f, 0.0078125f },  // T-B-R
    };

    matrix4 L2W;
    L2W.Identity();
    L2W.Scale(1000.0f);
    draw_SetL2W(L2W);
    for ( s32 i = 0; i < 6; i++ )
    {
        // set up for this face
        draw_SetTexture( s_pCurrCubeMap->m_Bitmap[i] );
        draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_NO_ZBUFFER | DRAW_NO_ZWRITE | DRAW_CULL_NONE | DRAW_USE_ALPHA );
        draw_Color( 1.0f, 1.0f, 1.0f, 1.0f );

        // first tri
        draw_UV     ( Vertex[i*4+0].u, Vertex[i*4+0].v );
        draw_Vertex ( Vertex[i*4+0].x, Vertex[i*4+0].y, Vertex[i*4+0].z );
        draw_UV     ( Vertex[i*4+1].u, Vertex[i*4+1].v );
        draw_Vertex ( Vertex[i*4+1].x, Vertex[i*4+1].y, Vertex[i*4+1].z );
        draw_UV     ( Vertex[i*4+2].u, Vertex[i*4+2].v );
        draw_Vertex ( Vertex[i*4+2].x, Vertex[i*4+2].y, Vertex[i*4+2].z );

        // second tri
        draw_UV     ( Vertex[i*4+1].u, Vertex[i*4+1].v );
        draw_Vertex ( Vertex[i*4+1].x, Vertex[i*4+1].y, Vertex[i*4+1].z );
        draw_UV     ( Vertex[i*4+2].u, Vertex[i*4+2].v );
        draw_Vertex ( Vertex[i*4+2].x, Vertex[i*4+2].y, Vertex[i*4+2].z );
        draw_UV     ( Vertex[i*4+3].u, Vertex[i*4+3].v );
        draw_Vertex ( Vertex[i*4+3].x, Vertex[i*4+3].y, Vertex[i*4+3].z );

        // done with this face
        draw_End();
    }
    draw_ClearL2W();

    // restore the destination address
    gsreg_Begin( 2 );
    gsreg_Set((SCE_GS_FRAME_1+eng_GetGSContext()), eng_GetFRAMEReg());
    gsreg_SetScissor( OrigX0, OrigY0, OrigX1, OrigY1 );
    gsreg_End();

    // restore the original viewport
    EnvView.SetViewport( OrigX0, OrigY0, OrigX1, OrigY1 );
    eng_SetViewport( OrigView );
    eng_SetView( OrigView ); 
}

//=============================================================================

static
void platform_SetProjectedTexture( texture::handle Texture )
{
    g_VU1.SetSpotlightTexture( Texture );
}

//=============================================================================

static
void platform_ComputeProjTextureMatrix( matrix4& Matrix, view& View, const texture_projection& Projection  )
{
    f32 ZNear = 1.0f;
    f32 ZFar  = Projection.Length;

    const xbitmap& Bitmap = Projection.Texture.GetPointer()->m_Bitmap;

    View.SetXFOV        ( Projection.FOV );
    View.SetPixelScale  ( 1.0f );
    View.SetZLimits     ( ZNear, ZFar );
    View.SetViewport    ( 0, 0, Bitmap.GetWidth(), Bitmap.GetHeight() );
    View.SetV2W         ( Projection.L2W );

    // compute the texture matrix using the "clip" matrix.
    // the z-component will be set up special to make the microcode happy
    Matrix       = View.GetV2C();
    Matrix(2,2)  = -1.0f / (ZFar-ZNear);
    Matrix(3,2)  = 1.0f+ZNear/(ZFar-ZNear);
    Matrix      *= View.GetW2V();
    
    // scale and translate to get uv coordinates
    Matrix.Scale    (vector3( 0.5f, -0.5f, 1.0f));
    Matrix.Translate(vector3( 0.5f,  0.5f, 0.0f));
}

//=============================================================================

static
void platform_SetTextureProjection( const texture_projection& Projection )
{
    g_VU1.SetSpotlightPos( Projection.L2W.GetTranslation() );
}

//=============================================================================

static
void platform_SetTextureProjectionMatrix( const matrix4& Matrix )
{
    g_VU1.SetSpotlightMatrix( Matrix );
}

//=============================================================================

static
void platform_SetProjectedShadowTexture( s32 Index, texture::handle Texture )
{
    g_VU1.SetProjShadowTexture( Index, Texture );
}

//=============================================================================

static
void platform_ComputeProjShadowMatrix( matrix4& Matrix, view& View, const texture_projection& Projection )
{
    // same as a projected texture
    platform_ComputeProjTextureMatrix( Matrix, View, Projection );
}

//=============================================================================

static
void platform_SetShadowProjectionMatrix( s32 Index, const matrix4& Matrix )
{
    g_VU1.SetProjShadowMatrix( Index, Matrix );
}

//=============================================================================

static
void platform_ClearShadowProjectorList( void )
{
    g_VU1.ClearShadowProjectors();
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

static
void platform_AddDirShadowProjection( const matrix4&         L2W,
                                      f32                    Width,
                                      f32                    Height,
                                      f32                    NearZ,
                                      f32                    FarZ )
{
    s32 ProjIndex = s_nDynamicShadows;

    // buildthe matrix that will map world coordinates into our projected
    // texture
    matrix4 W2Proj = L2W;
    W2Proj.InvertRT();

    // Note that this is following the d3d standard for Z, which is [0,1] after
    // transform, but since we never clip, it shouldn't really matter for the ps2.
    matrix4 Proj2Clip;
    Proj2Clip.Identity();
    Proj2Clip(0,0) = 2.0f / Width;
    Proj2Clip(1,1) = 2.0f / Height;
    Proj2Clip(2,2) = 1.0f / (FarZ-NearZ);
    Proj2Clip(3,2) = NearZ / (FarZ-NearZ);

    // figure out the final matrices for creating the shadow texture
    matrix4 W2TextureCast    = Proj2Clip * W2Proj;
    matrix4 W2TextureReceive = W2TextureCast;

    // move it into ps2 coordinate system
    W2TextureCast.Scale(vector3(kShadowTexSize*0.5f,kShadowTexSize*0.5f,1.0f));
    W2TextureCast.Translate(vector3(kShadowTexSize*0.5f,kShadowTexSize*0.5f,0.0f));
    W2TextureCast.Translate(vector3(2048.0f-256.0f, 2048.0f-256.0f,0.0f));
    W2TextureReceive.Scale(vector3(0.5f,0.5f,1.0f));
    W2TextureReceive.Translate(vector3(0.5f,0.5f,0.0f));

    // figure out where it should go in memory, note that we are basically using
    // 64 128x128 4-bit textures that are in the start of vram where the
    // front and back buffer alpha is
    s32 nPages        = (kShadowTexSize/64) * (kShadowTexSize/32); // nHorzPages * nVertPages
    s32 FramePageAddr = ProjIndex * nPages;
    u32 FrameMask     = 0x0f000000;
    u32 PSM           = SCE_GS_PSMT8H;   // TODO: Try to make this 4-bit instead
    if ( ProjIndex > 32 )
    {
        FramePageAddr -= 32 * nPages;
        FrameMask      = 0xf0000000;
        //PSM            = SCE_GS_PSMT4HL;
    }
    s32 FrameBlockAddr = FramePageAddr * 32;

    // upload the custom palettes
    vram_LoadClut( s_hShadowClut, 0 );
    s32 CBP = vram_GetClutBaseAddr( s_hShadowClut );

    // fill in the registers
    u64 FRAME = SCE_GS_SET_FRAME( FramePageAddr, kShadowTexSize/64, SCE_GS_PSMCT32, 0x0fffffff );
    u64 TEX0  = SCE_GS_SET_TEX0( FrameBlockAddr, kShadowTexSize/64, PSM, vram_GetLog2(kShadowTexSize), vram_GetLog2(kShadowTexSize), 1, 0, CBP, SCE_GS_PSMCT32, 0, 0, 1 );

    g_VU1.SetShadowCastInfo( ProjIndex, FRAME, W2TextureCast );
    g_VU1.SetShadowReceiveInfo( ProjIndex, TEX0, W2TextureReceive );
}

//=============================================================================

// TODO: This function shouldn't be here, and we should have a nice generic function
// somewhere else in the code. Maybe entropy?
static
void ClearVRAM( s32 DstBase, s32 Width, s32 Height, xcolor Color, u32 Mask, xbool ClearZBuffer )
{
    ASSERT( DstBase < 512 );

    // Draw 64 pixel wide columns at a time
    s32 ColumnWidth  = 64;
    s32 nColumns     = Width / ColumnWidth;
    
    gsreg_Begin( 6 + 2*nColumns );
    gsreg_Set( SCE_GS_FRAME_1,   SCE_GS_SET_FRAME_1  ( DstBase, Width / 64, SCE_GS_PSMCT32, Mask ) );
    gsreg_Set( SCE_GS_TEST_1,    SCE_GS_SET_TEST_1   ( 0, 0, 0, 0, 0, 0, 1, 1 ) );
    gsreg_SetZBufferUpdate(ClearZBuffer);
    gsreg_SetScissor( 0, 0, Width, Height );
    gsreg_Set( SCE_GS_PRIM,      SCE_GS_SET_PRIM     ( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,     SCE_GS_SET_RGBAQ    ( Color.R, Color.G, Color.B, Color.A, 0x3F800000 ) );

    s32 X  = (2048-(VRAM_FRAME_BUFFER_WIDTH/2));
    s32 Y0 = (2048-(VRAM_FRAME_BUFFER_HEIGHT/2));
    s32 Y1 = Y0 + Height;
    s32 i;
    for( i = 0; i < nColumns; i++ )
    {
        s32 X0 = X;
        s32 X1 = X0 + ColumnWidth;
    
        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X0 << 4), (Y0 << 4), 0 ) );
        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X1 << 4), (Y1 << 4), 0 ) );
        
        X += ColumnWidth;
    }
    gsreg_End();
}

//=============================================================================

static
void platform_BeginShadowShaders( void )
{
    // as a safety precaution for any REF'd data used by MFIFO, flush the
    // data cache
    FlushCache( WRITEBACK_DCACHE );

    // clear the front and back buffer alpha. that's where all 64 shadows will
    // go. each shadow is 128x128, and by masking out 4-bits we can fit
    // 64 in there.

    // TODO: Make sure we're not doing too many screen clears which can get
    // costly. Just make sure we haven't duplicated any. Also make sure we only
    // clear the area that really needs it
    //const view* pView = eng_GetView();
    //irect       RC;
    //pView->GetViewport( RC.l, RC.t, RC.r, RC.b );
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );

    gsreg_Begin( 1 );
    gsreg_SetScissor( 0, 0, XRes, YRes );
    gsreg_End();

    ClearVRAM( VRAM_FRAME_BUFFER0_START/8192, VRAM_FRAME_BUFFER_WIDTH, VRAM_FRAME_BUFFER_HEIGHT, xcolor(0,0,0,0), 0x00ffffff, FALSE );
    ClearVRAM( VRAM_FRAME_BUFFER1_START/8192, VRAM_FRAME_BUFFER_WIDTH, VRAM_FRAME_BUFFER_HEIGHT, xcolor(0,0,0,0), 0x00ffffff, FALSE );
    ClearVRAM( VRAM_ZBUFFER_START/8192, XRes, YRes, xcolor(0,0,0,0x80), 0x00ffffff, TRUE );

    // cache the active view so we don't get I$ misses going to entropy for it
    const view* pCurrView = eng_GetView();
    s_ScreenDist  = pCurrView->GetScreenDist();
    s_W2V         = pCurrView->GetW2V();
    
    // activate the microcode
    g_VU1.BeginShadows();

    // set the scissoring one pixel in from the texture size
    gsreg_Begin( 1 );
    gsreg_SetScissor( 1, 1, kShadowTexSize-1, kShadowTexSize-1 );
    gsreg_End();
}

//=============================================================================

static
void platform_EndShadowShaders( void )
{
    g_VU1.EndShadows();

    // TODO: Organize these clears so that we are minimizing them all
    // clear out the frame alpha since that's used for the glow bitmap
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    ClearVRAM( VRAM_FRAME_BUFFER0_START/8192, XRes, YRes, xcolor(0,0,0,0), 0x00ffffff, TRUE );
    ClearVRAM( VRAM_FRAME_BUFFER1_START/8192, XRes, YRes, xcolor(0,0,0,0), 0x00ffffff, FALSE );

    // restore the scissoring and frame register (must be done AFTER the
    // vram clears)
    const view* pView = eng_GetView();
    s32 OrigX0, OrigY0, OrigX1, OrigY1;
    pView->GetViewport( OrigX0, OrigY0, OrigX1, OrigY1 );
    gsreg_Begin( 2 );
    gsreg_SetScissor( OrigX0, OrigY0, OrigX1, OrigY1 );
    gsreg_SetFBMASK( 0x00000000 );
    gsreg_End();
}

//=============================================================================

static
void platform_StartShadowCast( void )
{
    // clear VU1's internal shadow matrix cache
    g_VU1.ClearShadowCache();
    g_VU1.SetShadowCastMaterial();

    gsreg_Begin( 2 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_1, TRUE, ZBUFFER_TEST_GEQUAL );
    gsreg_SetZBufferUpdate(FALSE);
    gsreg_End();

}

//=============================================================================

static
void platform_EndShadowCast( void )
{
    // TODO:
    // Is this function really needed?!?!?
}

//=============================================================================

static
void platform_StartShadowReceive( void )
{
    // clear VU1's internal shadow matrix cache
    g_VU1.ClearShadowCache();
    g_VU1.SetShadowReceiveMaterial();

    // set the frame register to the z-buffer, and the scissor register back to
    // normal
    s32 L, T, R, B;
    const view* pEngView = eng_GetView();
    pEngView->GetViewport( L, T, R, B );
    
    // the shadow receive will take two passes
    // the first pass will clear the shadow where the z-buffer is greater
    gsreg_Begin( 15 );
    gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( VRAM_ZBUFFER_START / 8192, VRAM_FRAME_BUFFER_WIDTH/64, 0x0, 0x00ffffff ) );
    gsreg_SetScissor( L, T, R, B );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 0, 0, 0, 0, 0 ) );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_ZB_ONLY,
                                   FALSE, DEST_ALPHA_TEST_1, TRUE, ZBUFFER_TEST_GREATER );
    gsreg_SetZBufferUpdate(TRUE);
    gsreg_SetAlphaBlend( ALPHA_BLEND_OFF ); // TODO: What should this really be?!?!?
    gsreg_Set( SCE_GS_FBA_1, 1 );
    gsreg_SetClamping(TRUE,TRUE);

    // the second pass will build in the alpha
    eng_PushGSContext(1);
    gsreg_Set( SCE_GS_FRAME_2, SCE_GS_SET_FRAME( VRAM_ZBUFFER_START / 8192, VRAM_FRAME_BUFFER_WIDTH/64, 0x0, 0x00ffffff ) );
    gsreg_SetScissor( L, T, R, B );
    gsreg_Set( SCE_GS_TEX1_2, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_SetAlphaAndZBufferTests( TRUE, ALPHA_TEST_LEQUAL, kShadowAlpha+2, ALPHA_TEST_FAIL_ZB_ONLY,
                                   TRUE, DEST_ALPHA_TEST_1, TRUE, ZBUFFER_TEST_GEQUAL );
    gsreg_SetZBufferUpdate(TRUE);
    gsreg_SetAlphaBlend( ALPHA_BLEND_OFF ); // TODO: What should this really be?!?!?
    gsreg_SetClamping(TRUE,TRUE);
    eng_PopGSContext();
    gsreg_End();
}

//=============================================================================

static
void platform_EndShadowReceive( void )
{
    // restore the things we have probably broken
    gsreg_Begin( 7 );
    
    gsreg_Set( SCE_GS_FBA_1, 0 );
    gsreg_Set(SCE_GS_FRAME_1, eng_GetFRAMEReg());
    gsreg_SetClamping(FALSE, FALSE);
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    
    eng_PushGSContext( 1 );
    gsreg_Set(SCE_GS_FRAME_2, eng_GetFRAMEReg());
    gsreg_SetClamping(FALSE, FALSE);
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PopGSContext();
    
    gsreg_End();

    // TODO:
    // Is this function really needed?!?!?
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
    (void)Inst;
    // TODO:
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
    s_SkinMode = -1;

    // store out a pointer to the skin's dlist
    skin_geom&          SkinGeom    = (skin_geom&)(*pGeom);
    skin_geom::submesh& GeomSubMesh = SkinGeom.m_pSubMesh[iSubMesh];
    s_pCurrSkinDList                = &SkinGeom.m_System.pPS2[GeomSubMesh.iDList];
}

//=============================================================================

static
void platform_RenderShadowCastSkin( render_instance& Inst, s32 iProj )
{
    CONTEXT("platform_RenderShadowCastSkin") ;

    // sanity check
    ASSERT( s_pCurrSkinDList );

    // begin rendering it
    g_VU1.BeginSkinShadow( iProj );

    // render it
    g_VU1.SetSkinFlags( Inst.Flags, Inst.Alpha );
    for ( s32 iCmd = 0; iCmd < s_pCurrSkinDList->nCmds; iCmd++ )
    {
        skin_geom::command_ps2 Cmd = s_pCurrSkinDList->pCmd[iCmd];
        
        switch( Cmd.Cmd )
        {
        default:
            break;

        case skin_geom::PS2_CMD_UPLOAD_MATRIX:
            {
                s16 BoneID  = Cmd.Arg1;
                s16 CacheID = Cmd.Arg2;
                g_VU1.UploadSkinShadowMatrix( Inst.Data.Skin.pBones[BoneID], CacheID );
            }
            break;

        case skin_geom::PS2_CMD_RENDER_VERTS_RIGID:
            {
                s32  NVerts       = Cmd.Arg2;
                s16* pPos         = (s16*)&s_pCurrSkinDList->pPos[Cmd.Arg1];
                u8*  pBoneIndices = (u8*) &s_pCurrSkinDList->pBoneIndex[Cmd.Arg1];
            
                g_VU1.RenderSkinShadowOneBone( NVerts, pPos, pBoneIndices );
            }
            break;

        case skin_geom::PS2_CMD_RENDER_VERTS_SOFT:
            {
                s32  NVerts       = Cmd.Arg2;
                s16* pPos         = (s16*)&s_pCurrSkinDList->pPos[Cmd.Arg1];
                u8*  pBoneIndices = (u8*) &s_pCurrSkinDList->pBoneIndex[Cmd.Arg1];
            
                g_VU1.RenderSkinShadowTwoBones( NVerts, pPos, pBoneIndices );
            }
            break;

        case skin_geom::PS2_CMD_END:
            break;
        }
    }

    // finished rendering this skin
    g_VU1.EndSkinShadow();
}

//=============================================================================

static
void platform_EndShadowCastSkin( void )
{
    // clean up and finish off any microcode
    s_pCurrSkinDList = NULL;
}

//=============================================================================

static
void platform_BeginShadowReceiveRigid( geom* pGeom, s32 iSubMesh )
{
    // Get the SubMesh and DList we want to render
    rigid_geom&            RigidGeom   = (rigid_geom&)(*pGeom);
    geom::submesh&         GeomSubMesh = RigidGeom.m_pSubMesh[iSubMesh];
    rigid_geom::dlist_ps2& DList       = RigidGeom.m_System.pPS2[GeomSubMesh.iDList];

    // set the static data to be uploaded
    g_VU1.BeginShadReceiveInstance( DList.pPosition, DList.pUV, DList.nVerts );
}

//=============================================================================

static
void platform_RenderShadowReceiveRigid( render_instance& Inst, s32 iProj )
{
    // TODO:
    // Instead of re-transforming for every single projector, figure out a way
    // to handle a bunch at one time.

    // accumlate flags
    if ( Inst.Flags & render::CLIPPED )
        Inst.Flags |= render::INSTFLAG_CLIPPED;

    // add it to the vu1 instance list
    g_VU1.AddShadReceiveInstance( Inst.Data.Rigid.pL2W, Inst.UOffset, Inst.VOffset, Inst.Flags, iProj );
}

//=============================================================================

static
void platform_EndShadowReceiveRigid( void )
{
    // TODO:
    g_VU1.EndShadReceiveInstance();
}

//=============================================================================

static
void platform_BeginShadowReceiveSkin( geom* pGeom, s32 iSubMesh )
{
    (void)pGeom;
    (void)iSubMesh;
    // TODO:
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
    // TODO:
}

//=============================================================================

static
void platform_BeginSession( u32 nPlayers )
{
    (void)nPlayers;
}

//=============================================================================

static
void platform_EndSession( void )
{
}

//=============================================================================

static
void platform_RegisterRigidGeom( rigid_geom& Geom )
{
    (void)Geom;
}

//=============================================================================

static
void platform_UnregisterRigidGeom( rigid_geom& Geom )
{
    (void)Geom;
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
}
