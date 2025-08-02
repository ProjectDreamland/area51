//=============================================================================
//
//  PC Specific Post-effect Routines
//
//=============================================================================

#include "Render\platform_Render.hpp"

//=============================================================================
//=============================================================================
// Types and constants specific to the PC-implementation
//=============================================================================
//=============================================================================

#define ZBUFFER_BITS    (19)
#define MAX_POST_MIPS   (3)
#define POST_MOTIONBLUR_FLAG        0x0001
#define POST_SELFILLUM_FLAG         0x0002
#define POST_MULTSCREEN_FLAG        0x0004
#define POST_RADIALBLUR_FLAG        0x0008
#define POST_ZFOGFILTERFN_FLAG      0x0010
#define POST_ZFOGFILTERCUSTOM_FLAG  0x0020
#define POST_MIPFILTERFN_FLAG       0x0040
#define POST_MIPFILTERCUSTOM_FLAG   0x0080

//=============================================================================
//=============================================================================
// Internal data
//=============================================================================
//=============================================================================

struct post_effect_params
{
    // flags for turning post-effects on and off
    xbool   Override        : 1;    // Set this to play around with values in the debugger
    xbool   DoMotionBlur    : 1;
    xbool   DoSelfIllumGlow : 1;
    xbool   DoMultScreen    : 1;
    xbool   DoRadialBlur    : 1;
    xbool   DoZFogFn        : 1;
    xbool   DoZFogCustom    : 1;
    xbool   DoMipFn         : 1;
    xbool   DoMipCustom     : 1;
    xbool   DoNoise         : 1;
    xbool   DoScreenFade    : 1;

    // post-effect parameters
    f32                         MotionBlurIntensity;
    f32                         GlowMotionBlurIntensity;
    s32                         GlowCutoff;
    xcolor                      MultScreenColor;
    render::post_screen_blend   MultScreenBlend;
    f32                         RadialZoom;
    radian                      RadialAngle;
    f32                         RadialAlphaSub;
    f32                         RadialAlphaScale;

    render::post_falloff_fn     FogFn       [5];
    xcolor                      FogColor    [5];
    f32                         FogParam1   [5];
    f32                         FogParam2   [5];
    s32                         FogPaletteIndex;

    render::post_falloff_fn     MipFn       [4];
    xcolor                      MipColor    [4];
    f32                         MipParam1   [4];
    f32                         MipParam2   [4];
    s32                         MipCount    [4];
    f32                         MipOffset   [4];
    s32                         MipPaletteIndex;

    xcolor                      NoiseColor;
    xcolor                      FadeColor;
};

// misc. data common to all post-effects
static post_effect_params           s_Post;
static xbool                        s_InPost = FALSE;
static s32                          s_PostViewL;
static s32                          s_PostViewT;
static s32                          s_PostViewR;
static s32                          s_PostViewB;
static f32                          s_PostNearZ = 1.0f;
static f32                          s_PostFarZ  = 2.0f;

// motion blur data
static f32                          s_MotionBlurIntensity;

// mult screen data
static xcolor                       s_MultScreenColor;
static render::post_screen_blend    s_MultScreenBlend;

// radial blur data
static f32                          s_RadialZoom;
static radian                       s_RadialAngle;
static f32                          s_RadialAlphaSub;
static f32                          s_RadialAlphaScale;

// fog data
static s32                          s_hFogClutHandle[3] = { -1, -1, -1 };
static xbool                        s_bFogValid[3]      = { FALSE, FALSE, FALSE };
static u8                           s_FogPalette[3][1024];

// mip filter data
static const xbitmap*               s_pMipTexture;
static s32                          s_hMipClutHandle[2] = { -1, -1 };
static u8                           s_MipPalette[2][1024];

// noise filter data
static const s32 kNoiseMapW = 64;
static const s32 kNoiseMapH = 64;
static xbitmap   s_NoiseBitmap;
static u32       s_NoiseMap[kNoiseMapW*kNoiseMapH/8] =
{
    0x2dfbfeff,0xbfdefffc,0x86fdffff,0xdf5ff889,0x7f63fcff,0xf982f57f,0xa9fcff4f,0xffffff6f,
    0xf3ffbb5f,0xf8fffffc,0xffdfffff,0xa8fce3ff,0xdffffff3,0xf35ff6ff,0x5ffaf6f6,0xfff2fcff,
    0xf9dfffff,0xfffdf3ff,0xffd7ef5f,0x97b75f6a,0x27f43953,0x4fff2fff,0x8ff7f7f3,0x5bf9afff,
    0xf3f4ffdb,0x25f747af,0xff9f9dff,0x3feff9cf,0x5ffff9f2,0xfcf2ffbb,0xfff8ffff,0xffffdfff,
    0xff26443f,0xfa6ff8f5,0x47e853f3,0xfffffbf5,0x84ffde92,0xff79eff9,0x2f5dfbd9,0xf2ffdff7,
    0xffffd2ff,0xf497ffff,0xfef4ffff,0xffd9fd8f,0xffffffff,0x48bf5fff,0x3dff2f2f,0x2bf9fecf,
    0x3ff963e2,0x76ff6bbf,0x9fffffff,0x3ff24fff,0xdaff8fef,0xffff8f42,0xffffd3af,0x6ff923fa,
    0xabffffff,0x8a2f94ff,0xffda2f45,0xdfcc62da,0x6f3fff47,0x4f3f9fff,0xffbffcff,0xf2ffff78,
    0xdaefffbf,0x2fffff8f,0xffffef8f,0xffffff6e,0xdf3fff9d,0x7df5ffff,0x75f6affd,0x5395397b,
    0xfffeffff,0x85d8f7fc,0xc87ffe55,0xfffa1dff,0xaf9ffb9f,0xf26f6eff,0xfff87fff,0xbf5e9ffe,
    0xff79fffb,0xec7cffff,0xf5fe9fff,0xbfdf5fdc,0xff4ccff7,0xbf4cfbff,0xf6ffdfbd,0xb3ffdfd7,
    0x3f3fe5ff,0x7f5f5fef,0xfd2ff9fc,0xf7f8ff8f,0xf3fdff62,0x35fff7ff,0xff6df353,0x53dfcfa4,
    0xf7ffffff,0xf3ff7ff2,0xf65ff6f7,0xf62ffaef,0x895f4ff4,0xfbffffe9,0xffff3faf,0x4fafcff8,
    0xfffdf46f,0xff25fffc,0x4f8ffffb,0xdf9aff64,0xf8f27f3f,0x985fffff,0xfef2fcdc,0xd4fef4f4,
    0xaffd7ef5,0x297b75f6,0xf26f4395,0x34fff2ff,0xf8ff7f7f,0xf5bf99ff,0xfff8ff49,0xc58f35ff,
    0xfff3ffff,0xfff5af8c,0x9fffafaf,0x47f96aff,0x6ffd988f,0xef8caf8f,0xf7adf69b,0xf923ff47,
    0xfffffdfc,0xebbf3baf,0xc2fff9f9,0xf8cffd2f,0xf2597faf,0xfffdf46f,0xff25fffc,0x4f8ffffb,
    0xb36b24ff,0xfeffffff,0xf5fffffa,0xf6fffcc8,0x6489f893,0xff8ff9ff,0xc5ff3f96,0xdfffff2f,
    0xefff79bf,0xffeff77f,0xffc3deff,0xf3d385ff,0xf68ffd6f,0x4fff8edf,0xfc8ffff5,0xfffcfffc,
    0xe66f3ff4,0x2ff4ff8f,0xfd4fffff,0xfff5f9ff,0xfe78fccf,0x89ffffe6,0xfafffcc6,0xff6fcfaf,
    0xf4ffc248,0xff9ffefc,0xffabffff,0x448a2f94,0xdaffda2f,0x47dfcc63,0xff6f3fff,0xff4f3f8f,
    0xffecfee6,0xff3f78f9,0xf5ffffff,0xb8f4bfff,0xfefaf73f,0xbf5ebff6,0xfffcfff4,0xff8afff8,
    0xaf9ffb9f,0xf26f6eff,0xfff87fff,0xbf5e9ffe,0x8fff3ffb,0x248af6be,0xefcf4ffc,0xfffff9ff,
    0xcff5ffaf,0x34cfff2f,0x6ff62e35,0xfe3ffff8,0xff59f8cf,0xfffafaff,0x6f96aff9,0xffd988f4,
    0xfff43daf,0xf633d6e2,0xbff94eff,0xfffdfc7f,0xbf4bafff,0xfff9f9eb,0xcffd2fc2,0x597faff8,
    0xf3f7ff2e,0xfaff3ff9,0xff9f9f8f,0xfeffffbf,0xd8f7fcff,0x7ffe6585,0xfa2dffc8,0x9ffb9fff,
    0xf7ffffdf,0xbff9ffe4,0x2af4ffff,0xff58ffff,0xda7f4faf,0xf3bfffff,0xfff9fffe,0x69ff68f9,
    0x6fffaf37,0x9ffecfee,0xfff3f78f,0xff5fffff,0xfb7f4bff,0x6fffaf73,0x4bf4ebff,0x8fffcfff,
    0x8bdf9fdf,0x8f369f92,0xffedf9cd,0x24ff54ff,0xf2cfff7f,0xf8c5f2ff,0xfafcf4f3,0x3f52674f,
    0xffaf7fff,0xbff8ff8b,0xfff5fcdf,0x59ff29f8,0xf9fffeff,0xdfffff2f,0xfffcff6f,0x8feff7ff,
    0x876fffff,0xff8fffff,0xf7e877ff,0xfaff2fff,0xfff8afff,0xbffacfff,0x3af8ffcf,0x28f6f9fb,
    0xf25fffff,0xfebffeff,0x5ff26443,0x3fa6ff8f,0x548e853f,0x3fffffbf,0x984ffde9,0x9ff79eff,
    0xf6fef545,0x85f5f3fc,0xb9f5fff4,0xfdf6ff44,0x4bffffff,0xfff9f65f,0xf3f223fe,0x47f7f5af,
    0x8f98eeff,0xfff47f9f,0x3fffffaf,0xfdf365ff,0xfaafefff,0x8ffaff8e,0xfebff6f4,0xffef4dfe,
    0xf6fffcff,0xefbfff4f,0xf3f54bff,0xf3ff64ff,0x3ff4af6f,0xff8fe66f,0xffff2ff4,0xf9fffd4f,
    0xfcf4f3f8,0x52583ffa,0xaefffa3f,0xc6ff6d5f,0xfff55fff,0xff2f7fff,0x6f7f3ff8,0xaeff65ff,
    0x4cdef7bf,0x4cfbffff,0xffdfbcbf,0xffdfd7f6,0xa5fbfdb3,0xff9ffcef,0x9f96f7f9,0xeef4fff9,
    0xdf489ff8,0xf9f92fff,0x3ff2ffff,0xfffffaff,0xbf87ffff,0xfffff4a8,0xfeffafff,0xbffcaf7f,
    0xff8fdaef,0xef8f2fff,0xff6effff,0xff9dfeff,0xffffdf3f,0xaffc7ef5,0x297b75f6,0xf26f43a5,
    0xf9f78f4f,0x5ff2dd2f,0x74ff8f4d,0x8bffffff,0xbff2ffff,0xfff89fff,0xffc7cfff,0xcf4ff9ff,
    0xff8d7fff,0xffffff52,0x6b7f82ff,0xf5fef75f,0xfff894f8,0xffbff7ff,0xff963e2f,0x6ff6bbf3,
    0xff79bffc,0xeff77fef,0xc3deffff,0xd385ffff,0x8ffd7ff3,0xff8edff6,0x8ffff54f,0xfcfffdfc,
    0x7d33fd4f,0xf94a593f,0xfccff74f,0xf9dfff4f,0x6fffffff,0xff9a738f,0x3cffffed,0x8e283faf,
    0xe5ffffc8,0x6fb29f3f,0x28ffcdf6,0xffffffff,0x6fffff6f,0x8fffff87,0xe877ffff,0xff2ffff7,
    0xff44ffbf,0xfdfffff7,0xfffb44bf,0xfafffccf,0xcdd9528f,0xd5dff3ff,0xfd2f53ff,0xfaffff99,
    0xf8af5fff,0xf37f3e7f,0xfee6fffa,0x78f9ffec,0xffffff3f,0xbffff5ff,0xf82fb7f4,0xbff6fefa,
    0x6d5ffbfd,0x3f3fafbf,0xe82ffff2,0xdffef2f9,0xffdffaf5,0xffffffe4,0x2fff2f7f,0xfffffffd,
    0xf3fdf9af,0xffff8f27,0xcdc985ff,0x4f4fef2f,0x33fd4fef,0x4a593f7d,0xcff84ff9,0xdfff4ffc,
    0xf33ff7ff,0xffff9bff,0xef874337,0xffe9fff7,0xfe4ff8ff,0x64ffbfd4,0x6fabfb7f,0xcffcf4df,
    0x6f96aff9,0xffd988f4,0xf8caf7f6,0x7adf69be,0x923ff47f,0x8ff8ff9f,0xfcf9f2d3,0xaff8f4f4,
    0xfbffdff8,0x3fcff978,0xfff5f4af,0xf34ffff9,0xf6fff2ff,0xff3ff6ff,0x2fffffff,0xcffffff3,
    0x335fff7f,0x4ff6cf35,0xf53dfdfa,0xfb43ffcf,0xbfafffff,0x5f7f3ffb,0xeeefff5e,0xfffff9cf,
    0xffff8f42,0xffffd3af,0x6ff922fa,0x2fffcf2d,0xfff9fcff,0xb9fadf2f,0x5faf2ff4,0xffdfff69,
    0x62ffaeff,0x95f4ff4f,0xaffffe98,0xfff3faff,0xfafcff8f,0xf2f5ef74,0x2fef7252,0xf3fffbff,
    0xf6fe5fe2,0xffffffbf,0xf5ffbfbf,0xfffdf5bf,0xfff64fff,0xffff8fff,0xdfffffad,0xbdf9fdfa,
    0x3f6b9fca,0x9fcff4a2,0xf8c9ffff,0x6ffcfff9,0xff93f8ff,0x4faf9fba,0xff8265ff,0xffcf76ff,
    0xaf6ff3ff,0xe66f3ff4,0x2ff4ff8f,0xfd4fffff,0xfff5f9ff,0xff78fccf,0x89fffff6,0xfafffcc7,
    0xffff8f42,0xffffd3af,0x6ff932fa,0x2fffcf2d,0xfff9fcff,0xb9fadf2f,0x5faf2ff4,0xffdfff69,
    0xf6bafca8,0xfcff4a23,0x8c9ffff9,0xffcfff9f,0xf94f8ff6,0xfaf9fbaf,0xf8266ff4,0xfcf76fff,
    0x8bf4ffff,0xdff2f2f4,0xbf9fedf3,0xaecffef2,0xfa6fefcf,0x2fc8fff8,0xf9f55fff,0x6a66f58f,
    0xbff7ffff,0x972e2fff,0xf6bbf3ff,0xfffff76f,0x24fff9ff,0xf8fef3ff,0xf8f43caf,0xfd3affff,
    0xf4f256ef,0x3fdffbff,0xbffe9aff,0xfc3ffff8,0xfa7fef5c,0x8ff2bd4c,0xffbfffdf,0x6f3ecfff,
    0xfdfff7fe,0x6fff8a8f,0x3fffffd6,0x37ff8ff7,0xffc86faa,0x9f3fe5ff,0xcdf66fb2,0xffff28ff,
    0xf57fff3f,0xbfefca6c,0xfff2eff9,0x5f3ceafb,0x7f49ff69,0xffeff886,0xfafa7fff,0xfaff2f8f
};

static u32 s_NoisePalette[16] =
{
    0x62626262, 0x64646464, 0x66666666, 0x68686868,
    0x6a6a6a6a, 0x6c6c6c6c, 0x6e6e6e6e, 0x70707070,
    0x72727272, 0x74747474, 0x76767676, 0x78787878,
    0x7a7a7a7a, 0x7c7c7c7c, 0x7e7e7e7e, 0x80808080,
};

//=============================================================================
//=============================================================================
// Internal routines
//=============================================================================
//=============================================================================

static
void pc_MotionBlur( void )
{
}

//=============================================================================

static
void pc_BuildScreenMips( s32 nMips, xbool UseAlpha )
{
}

//=============================================================================

static
void pc_ApplySelfIllumGlows( void )
{
    g_pShaderMgr->DumpShader();
}

//=============================================================================

static
void pc_CopyBackBuffer( void )
{
}

//=============================================================================

static
void pc_MultScreen( void )
{
}

//=============================================================================

static
void pc_RadialBlur( void )
{
}

//=============================================================================

static
void pc_CopyRG2BA( void )
{
}

//=============================================================================

static
void pc_SetFogTexture( void )
{
}

//=============================================================================

static
void pc_SetFogTexture( const xbitmap* pBitmap )
{
}

//=============================================================================

static
void pc_RenderFogSprite( void )
{
}

//=============================================================================

static
void pc_ZFogFilter( void )
{
    CONTEXT( "pc_ZFogFilter" );

    // upload the fog palette, and turn the alpha channel of the front buffer
    // into a PSMT8H texture
    pc_SetFogTexture();

    // now just render the fog sprite with normal blending
    pc_RenderFogSprite();
}

//=============================================================================

static
void pc_SetMipTexture(void)
{
   s32 mipLevel = s_Post.MipCount[s_Post.MipPaletteIndex] - 1;
}

//=============================================================================
static
void pc_SetMipTexture(const xbitmap* pBitmap)
{
}

//=============================================================================

static
void pc_RemapAlpha(void)
{
}

//=============================================================================

extern void Blt( f32 dx,f32 dy,f32 dw,f32 dh,f32 sx,f32 sy,f32 sw,f32 sh );

//=============================================================================

static
void pc_RenderMipSprites(void)
{
return; //Skip    
    if (!g_pd3dDevice)
        return;

    const view* pView = eng_GetView();
    s32 vportL, vportT, vportR, vportB;
    pView->GetViewport(vportL, vportT, vportR, vportB);

    DWORD oldZEnable, oldAlphaBlendEnable, oldSrcBlend, oldDestBlend;
    g_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &oldZEnable );
    g_pd3dDevice->GetRenderState( D3DRS_ALPHABLENDENABLE, &oldAlphaBlendEnable );
    g_pd3dDevice->GetRenderState( D3DRS_SRCBLEND, &oldSrcBlend );
    g_pd3dDevice->GetRenderState( D3DRS_DESTBLEND, &oldDestBlend );
                                  
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    struct Vertex
    {
        f32 x,y,z,w;
        f32 u,v;
    };

    s32 mipLevel = s_Post.MipCount[s_Post.MipPaletteIndex] - 1;
    f32 mipWidth = ( f32 )( vportR - vportL ) / ( 1 << mipLevel );
    f32 mipHeight = ( f32 )( vportB - vportT ) / ( 1 << mipLevel );

    Vertex TriList[4] = {
        // Top-left
        {( float )vportL - 0.5f, ( float )vportT - 0.5f, 0.0f, 1.0f, 0.0f, 0.0f },
        // Top-right                   
        {( float )vportR - 0.5f, ( float )vportT - 0.5f, 0.0f, 1.0f, mipWidth, 0.0f },
        // Bottom-right                
        {( float )vportR - 0.5f, ( float )vportB - 0.5f, 0.0f, 1.0f, mipWidth, mipHeight },
        // Bottom-left                 
        {( float )vportL - 0.5f, ( float )vportB - 0.5f, 0.0f, 1.0f, 0.0f, mipHeight }
    };

    //  ----------------------------------------------------------------------
    //
    //  Draw quad
    //
    
    ASSERT( g_pShaderMgr );
    g_pShaderMgr->m_VSFlags.Mask = 0;
    g_pd3dDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, TriList, sizeof( Vertex ) );

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, oldZEnable );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, oldAlphaBlendEnable );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, oldSrcBlend );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, oldDestBlend );

    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
}

//=============================================================================

static
void pc_MipFilter(void)
{
    CONTEXT( "pc_MipFilter" );

    ASSERT( s_Post.MipCount[s_Post.MipPaletteIndex] <= MAX_POST_MIPS );

    // upload the mip palette, which is really an alpha re-map palette
    if ( s_Post.MipFn[s_Post.MipPaletteIndex] == render::FALLOFF_CUSTOM )
        pc_SetMipTexture( s_pMipTexture );
    else
        pc_SetMipTexture();

    // now remap the alpha's into the back buffer
    pc_RemapAlpha();

    // now we render the mip multiple times with a dest alpha blend
    pc_RenderMipSprites();
}

//=============================================================================

static
void pc_CreateFalloffPalette( u8* pPalette, render::hgeom_inst Fn, xcolor Color, f32 Param1, f32 Param2 )
{
    f32 FarMinusNear = s_PostFarZ-s_PostNearZ;
    for ( s32 i = 0; i < 256; i++ )
    {
        s32 SwizzledIndex = ((i&0x08)<<1) | ((i&0x10)>>1) | ((i&0xE7));
        ASSERT( (SwizzledIndex>=0) && (SwizzledIndex<256) );

        // The z-buffer is 24-bit, but we can only access bits 8..15 of that.
        // Because the z-buffer is 0 at the far plane, this is actually okay, we
        // just make sure that we're far enough in that bits 16..23 are always zero.
        // So to figure out what the fog color should be, we need to convert from the
        // playstation's z-buffer to our units.
        f32 ZDoublePrime = (f32)((i<<8) + 0x80);
        
        // knowing what we do about the C2S matrix, we can work our way back
        // into the [-1,1] clipping cube
        // Z" = -Z' * (scale/2) + (scale/2)
        // ==> Z' = (-2 * Z" / scale) + 1
        f32 ZScale = (f32)((s32)1<<ZBUFFER_BITS);
        f32 ZPrime = ((-2.0f * ZDoublePrime) / ZScale) + 1;
        
        // knowing what we do about the V2C matrix, we can work our way back
        // into the original Z in view space
        // Z' = (Z(F+N)/(F-N) - 2*N*F/(F-N))/Z
        // ==> Z = -2*N*F / (Z'(F-N) - F - N)
        f32 Denom = (ZPrime*FarMinusNear) - s_PostFarZ - s_PostNearZ;
        ASSERT( (Denom >= -2.0f*s_PostFarZ) && (Denom <= -2.0f*s_PostNearZ) );
        f32 Z = -2.0f * s_PostNearZ * s_PostFarZ / Denom;
        ASSERT( (Z>=s_PostNearZ) && (Z<=s_PostFarZ) );
        
        // now apply the appropriate function to figure out the fog intensity
        f32 F;
        switch ( Fn )
        {
        default:
            ASSERT( FALSE );
            F = 1.0f;
            break;            
        case render::FALLOFF_CONSTANT:
            F = 1.0f - Color.A/128.0f;
            break;            
        case render::FALLOFF_LINEAR:
            ASSERT( Param2 > Param1 );
            if ( Z > Param2 )
                Z = Param2;
            F = (Param2-Z)/(Param2-Param1);
            break;        
        case render::FALLOFF_EXP:
            {
                f32 D = (Z-s_PostNearZ)/(FarMinusNear);
                // DS - don't use x-files because that is calling the system's exp which
                // uses doubles. I'd like to write a fast approximation using robin green's
                // methods, but don't have the time to mess with it right now.
                f32 Denom = expf(D);
                ASSERT( Denom > 0.001f );
                F = 1.0f/Denom;
            }
            break;            
        case render::FALLOFF_EXP2:
            {
                f32 D = (Z-s_PostNearZ)/(FarMinusNear);
                D *= Param1;
                D = D*D;
                // DS - don't use x-files because that is calling the system's exp which
                // uses doubles. I'd like to write a fast approximation using robin green's
                // methods, but don't have the time to mess with it right now.
                f32 Denom = expf(D);
                ASSERT( Denom > 0.001f );
                F = 1.0f/Denom;
            }
            break;
        }
        
        F = MIN(F, 1.0f);
        F = MAX(F, 0.0f);
        
        pPalette[SwizzledIndex*4+0] = Color.R;
        pPalette[SwizzledIndex*4+1] = Color.G;
        pPalette[SwizzledIndex*4+2] = Color.B;
        
        if ( (i == 0) && (Fn!=render::FALLOFF_CONSTANT) )
            pPalette[SwizzledIndex*4+3] = 0;
        else
            pPalette[SwizzledIndex*4+3] = 0x80-(u8)(F*128.0f);
    }
}

//=============================================================================

static
void pc_CreateFogPalette( render::post_falloff_fn Fn, xcolor Color, f32 Param1, f32 Param2 )
{
    CONTEXT( "pc_CreateFogPalette" );
   
    f32 NearZ, FarZ;
    const view* pView = eng_GetView();
    pView->GetZLimits( NearZ, FarZ );
   
    // force this to be fog palette 2
    s_Post.FogPaletteIndex = 2;
    
    // is it necessary to rebuild the palette?
    if ( (s_Post.FogFn[s_Post.FogPaletteIndex]     == Fn    ) &&
         (s_Post.FogParam1[s_Post.FogPaletteIndex] == Param1) &&
         (s_Post.FogParam2[s_Post.FogPaletteIndex] == Param2) &&
         (s_Post.FogColor[s_Post.FogPaletteIndex]  == Color ) &&
         (s_PostNearZ == NearZ ) &&
         (s_PostFarZ  == FarZ  ) )
    {
        return;
    }
    
    s_Post.FogFn[s_Post.FogPaletteIndex]     = Fn;
    s_Post.FogParam1[s_Post.FogPaletteIndex] = Param1;
    s_Post.FogParam2[s_Post.FogPaletteIndex] = Param2;
    s_Post.FogColor[s_Post.FogPaletteIndex]  = Color;
    s_PostNearZ = NearZ;
    s_PostFarZ  = FarZ;
    
    pc_CreateFalloffPalette( s_FogPalette[s_Post.FogPaletteIndex], Fn, Color, Param1, Param2 );
    s_bFogValid[s_Post.FogPaletteIndex] = TRUE;
}

//=============================================================================

static
void pc_CreateMipPalette( render::post_falloff_fn Fn, xcolor Color, f32 Param1, f32 Param2, s32 PaletteIndex )
{
    CONTEXT( "pc_CreateMipPalette" );
    
    f32 NearZ, FarZ;
    const view* pView = eng_GetView();
    pView->GetZLimits( NearZ, FarZ );

    // is it necessary to rebuild the palette?
    if ( ( s_Post.MipFn[PaletteIndex]     == Fn     ) &&
         ( s_Post.MipParam1[PaletteIndex] == Param1 ) &&
         ( s_Post.MipParam2[PaletteIndex] == Param2 ) &&
         ( s_Post.MipColor[PaletteIndex]  == Color  ) &&
         ( s_PostNearZ      == NearZ ) &&
         ( s_PostFarZ       == FarZ  ) )
    {
        return;
    }

    s_Post.MipFn[PaletteIndex]     = Fn;
    s_Post.MipParam1[PaletteIndex] = Param1;
    s_Post.MipParam2[PaletteIndex] = Param2;
    s_Post.MipColor[PaletteIndex]  = Color;
    s_Post.MipPaletteIndex         = PaletteIndex;
    s_PostNearZ = NearZ;
    s_PostFarZ  = FarZ;

    pc_CreateFalloffPalette( s_MipPalette[PaletteIndex], Fn, Color, Param1, Param2 );
}

//=============================================================================

static
void pc_NoiseFilter(void)
{
}

//=============================================================================

static
void pc_ScreenFade( void )
{
   if ( !g_pd3dDevice )
       return;
       
   // render a big transparent quad over the screen
   irect Rect;
   Rect.l = s_PostViewL;
   Rect.t = s_PostViewT;
   Rect.r = s_PostViewR;
   Rect.b = s_PostViewB;
   
   // Save current render states
   DWORD alphaBlendEnable;
   DWORD srcBlend;
   DWORD destBlend;
   DWORD zEnable;
   DWORD zWriteEnable;
   
   g_pd3dDevice->GetRenderState( D3DRS_ALPHABLENDENABLE, &alphaBlendEnable );
   g_pd3dDevice->GetRenderState( D3DRS_SRCBLEND, &srcBlend );
   g_pd3dDevice->GetRenderState( D3DRS_DESTBLEND, &destBlend );
   g_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &zEnable );
   g_pd3dDevice->GetRenderState( D3DRS_ZWRITEENABLE, &zWriteEnable );
   
   // Set render states for fade effect
   g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
   g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
   g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
   g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
   g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
   
   // Draw the fade rect
   draw_Rect( Rect, s_Post.FadeColor, FALSE );
   
   // Restore render states
   g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, alphaBlendEnable );
   g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, srcBlend );
   g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, destBlend );
   g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, zEnable );
   g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, zWriteEnable );
}

//=============================================================================
//=============================================================================
// Functions that are exposed to the main render file
//=============================================================================
//=============================================================================

//=============================================================================

static
void platform_SetCustomFogPalette( const texture::handle& Texture, xbool ImmediateSwitch, s32 PaletteIndex ) //SHADER SYSTEM!!!
{
    //ASSERT( g_pShaderMgr );
    //g_pShaderMgr->SetCustomFogPalette( Texture,ImmediateSwitch,PaletteIndex );
}

//=============================================================================

static
xcolor platform_GetFogValue( const vector3& WorldPos, s32 PaletteIndex )
{
    (void)WorldPos;
    (void)PaletteIndex;
    return xcolor(255,255,255,0);
}

//=============================================================================

static
void platform_InitPostEffects( void )
{
    // register the fog and mip cluts
    s32 i;
    for ( i = 0; i < 3; i++ )
    {
        s_bFogValid[i] = FALSE;
    }
    
    // set up a noise bitmap and register it
    s_NoiseBitmap.Setup( xbitmap::FMT_32_ARGB_8888, kNoiseMapW, kNoiseMapH, FALSE, (byte*)s_NoiseMap, FALSE, (byte*)s_NoisePalette );
    vram_Register( s_NoiseBitmap );
    
    // clear all current effects
    x_memset( &s_Post, 0, sizeof( post_effect_params ) );
}

//=============================================================================

static
void platform_KillPostEffects( void )
{
    vram_Unregister( s_NoiseBitmap );
}

//=============================================================================

xcolor pc_GetFogValue( const vector3& Position,s32 PaletteIndex )
{
    const matrix4& W2C = eng_GetView()->GetW2C();

    vector4 ScreenPos( Position );
    ScreenPos.GetW() = 1.0f;
    ScreenPos = W2C * ScreenPos;
    if( x_abs( ScreenPos.GetW() ) < 0.001f )
        return xcolor(255,255,255,0);

    f32 Z  = ScreenPos.GetZ();
    f32 Z2 = Z*Z;
    f32 Z3 = Z2*Z;
    f32 FogIntensity = g_pShaderMgr->m_FogConst[PaletteIndex].GetX()      +
                       g_pShaderMgr->m_FogConst[PaletteIndex].GetY() * Z  +
                       g_pShaderMgr->m_FogConst[PaletteIndex].GetZ() * Z2 +
                       g_pShaderMgr->m_FogConst[PaletteIndex].GetW() * Z3;
    
    FogIntensity = MINMAX( 0.0f, FogIntensity, 1.0f );
    
    return xcolor( 255, 255, 255, u8(FogIntensity*255.0f) );
}

//=============================================================================

static
void platform_BeginPostEffects( void )
{
    ASSERT( !s_InPost );
    s_InPost               = TRUE;
    
    if ( s_Post.Override )
        return;

    s_Post.DoMotionBlur    = FALSE;
    s_Post.DoSelfIllumGlow = FALSE;
    s_Post.DoMultScreen    = FALSE;
    s_Post.DoRadialBlur    = FALSE;
    s_Post.DoZFogFn        = FALSE;
    s_Post.DoZFogCustom    = FALSE;
    s_Post.DoMipFn         = FALSE;
    s_Post.DoMipCustom     = FALSE;
    s_Post.DoNoise         = FALSE;
    s_Post.DoScreenFade    = FALSE;

    s_pMipTexture          = NULL;
}

//=============================================================================

static
void platform_AddScreenWarp( const vector3& WorldPos, f32 Radius, f32 WarpAmount )
{
    (void)WorldPos;
    (void)Radius;
    (void)WarpAmount;
}

//=============================================================================

static
void platform_MotionBlur( f32 Intensity )
{
    if ( s_Post.Override )
        return;

    ASSERT( s_InPost );
    s_Post.DoMotionBlur         = TRUE;
    s_Post.MotionBlurIntensity  = Intensity;
}

//=============================================================================

static
void platform_ApplySelfIllumGlows( f32 MotionBlurIntensity, s32 GlowCutoff )
{
    if ( s_Post.Override )
        return;

    ASSERT( s_InPost );
    s_Post.DoSelfIllumGlow         = TRUE;
    s_Post.GlowMotionBlurIntensity = MotionBlurIntensity;
    s_Post.GlowCutoff              = GlowCutoff;
    
    //g_pPipeline->SetMotionBlurIntensity( MotionBlurIntensity );
    //g_pPipeline->SetGlowCutOff( (f32)GlowCutoff );
}

//=============================================================================

static
void platform_MultScreen( xcolor MultColor, render::post_screen_blend FinalBlend )
{
    if ( s_Post.Override )
        return;

    ASSERT( s_InPost );
    s_Post.DoMultScreen    = TRUE;
    s_Post.MultScreenColor = MultColor;
    s_Post.MultScreenBlend = FinalBlend;
}

//=============================================================================

static
void platform_RadialBlur( f32 Zoom, radian Angle, f32 AlphaSub, f32 AlphaScale  )
{
    if ( s_Post.Override )
        return;

    ASSERT( s_InPost );
    s_Post.DoRadialBlur     = TRUE;
    s_Post.RadialZoom       = Zoom;
    s_Post.RadialAngle      = Angle;
    s_Post.RadialAlphaSub   = AlphaSub;
    s_Post.RadialAlphaScale = AlphaScale;
}

//=============================================================================

static
void platform_ZFogFilter( render::post_falloff_fn Fn, xcolor Color, f32 Param1, f32 Param2 )
{
    if ( s_Post.Override )
        return;
   
    ASSERT( s_InPost );
    ASSERT( Fn != render::FALLOFF_CUSTOM );
   
    // adjust our fog palette (will also save the fog parameters)
    pc_CreateFogPalette( Fn, Color, Param1, Param2 );
    s_Post.DoZFogFn        = TRUE;
    s_Post.FogPaletteIndex = 2;
}

//=============================================================================

static
void platform_ZFogFilter( render::post_falloff_fn Fn, s32 PaletteIndex )
{
    if ( s_Post.Override )
        return;
    
    ASSERT( (PaletteIndex >= 0) && (PaletteIndex <= 4) );
    if ( !s_bFogValid[PaletteIndex] )
        return;
   
    ASSERT( s_InPost );
    ASSERT( Fn == render::FALLOFF_CUSTOM );
    s_Post.DoZFogCustom        = TRUE;
    s_Post.FogFn[PaletteIndex] = Fn;
    s_Post.FogPaletteIndex     = PaletteIndex;
}

//=============================================================================

static
void platform_MipFilter( s32                        nFilters,
                         f32                        Offset,
                         render::post_falloff_fn    Fn,
                         xcolor                     Color,
                         f32                        Param1,
                         f32                        Param2, 
                         s32                        PaletteIndex )
{
    if ( s_Post.Override )
        return;

    ASSERT( s_InPost );
    ASSERT( Fn != render::FALLOFF_CUSTOM );
    ASSERT( (PaletteIndex >= 0) && (PaletteIndex < 4) );

    pc_CreateMipPalette( Fn, Color, Param1, Param2, PaletteIndex );
    s_Post.DoMipFn                 = TRUE;
    s_Post.MipFn[PaletteIndex]     = Fn;
    s_Post.MipCount[PaletteIndex]  = nFilters;
    s_Post.MipOffset[PaletteIndex] = Offset;
    s_Post.MipPaletteIndex         = PaletteIndex;
}

//=============================================================================

static
void platform_MipFilter( s32                        nFilters,
                         f32                        Offset,
                         render::post_falloff_fn    Fn,
                         const texture::handle&     Texture,
                         s32                        PaletteIndex )
{
    if ( s_Post.Override )
        return;

    ASSERT( s_InPost );
    ASSERT( Fn == render::FALLOFF_CUSTOM );
    ASSERT( (PaletteIndex >= 0) && (PaletteIndex < 4) );

    // safety check
    if ( Texture.GetPointer() == NULL )
        return;

    s_Post.DoMipCustom             = TRUE;
    s_pMipTexture                  = &Texture.GetPointer()->m_Bitmap;
    s_Post.MipFn[PaletteIndex]     = Fn;
    s_Post.MipCount[PaletteIndex]  = nFilters;
    s_Post.MipOffset[PaletteIndex] = Offset;
    s_Post.MipPaletteIndex         = PaletteIndex;
    ASSERT( s_pMipTexture );
}

//=============================================================================

static
void platform_NoiseFilter( xcolor Color )
{
    if ( s_Post.Override )
        return;

    ASSERT( s_InPost );

    s_Post.DoNoise    = TRUE;
    s_Post.NoiseColor = Color;
}

//=============================================================================

static
void platform_ScreenFade( xcolor Color )
{
    if ( s_Post.Override )
        return;

    ASSERT( s_InPost );
   
    s_Post.FadeColor    = Color;
    s_Post.DoScreenFade = TRUE;
}

//=============================================================================

static
void platform_EndPostEffects( void )
{
    CONTEXT( "platform_EndPostEffects" );

    ASSERT( s_InPost );
    s_InPost = FALSE;

    // get common engine/screen information
    const view* pView = eng_GetView();
    pView->GetViewport( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );

    // handle fog
    if ( s_Post.DoZFogCustom || s_Post.DoZFogFn )
    {
        pc_ZFogFilter();
    }
    
    // handle the mip filter
    if ( s_Post.DoMipCustom || s_Post.DoMipFn )
        pc_MipFilter();

    // handle motion-blur. this is a stand-alone blur that will not effect vram
    // or the contents of any alpha or z-buffers.
    if ( s_Post.DoMotionBlur )
        pc_MotionBlur();

    // handle self-illum
    if ( s_Post.DoSelfIllumGlow )
        pc_ApplySelfIllumGlows();

    // radial blurs and post-mults will want a copy of the back-buffer to work with
    if ( s_Post.DoMultScreen || s_Post.DoRadialBlur )
        pc_CopyBackBuffer();

    // handle the multscreen filter
    if ( s_Post.DoMultScreen )
        pc_MultScreen();

    // handle the radial blur filter
    if ( s_Post.DoRadialBlur )
        pc_RadialBlur();

    // handle the noise filter
    if ( s_Post.DoNoise )
        pc_NoiseFilter();

    // handle the fade filter
    if ( s_Post.DoScreenFade )
        pc_ScreenFade();

    // reset the things we've most likely screwed up along the way
    if (g_pd3dDevice)
    {
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
        
        g_pd3dDevice->SetTexture(0, NULL);
        g_pd3dDevice->SetTexture(1, NULL);
        g_pd3dDevice->SetTexture(2, NULL);
        g_pd3dDevice->SetTexture(3, NULL);
    }
}


//=============================================================================

static
void platform_BeginDistortion( void )
{
    // TODO:
    // g_pPipeline->BeginDistortion();
}

//=============================================================================

static
void platform_EndDistortion( void )
{
    // g_pPipeline->EndDistortion();
}