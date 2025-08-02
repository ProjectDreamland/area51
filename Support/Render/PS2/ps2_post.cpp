//=============================================================================
//
//  PS2 Specific Post-effect Routines
//
//=============================================================================

#include "../platform_Render.hpp"
#include "PS2/PS2_Misc.hpp"

//=============================================================================
//=============================================================================
// Types and constants specific to the ps2-implementation
//=============================================================================
//=============================================================================

#define SCISSOR_LEFT        (2048-(VRAM_FRAME_BUFFER_WIDTH/2))
#define SCISSOR_TOP         (2048-(VRAM_FRAME_BUFFER_HEIGHT/2))
#define ZBUFFER_BITS        (19)
#define MAX_POST_MIPS       (3)
#define MAX_SCREEN_WARPS    (8)
#define MAX_PALETTES        (3)

struct screen_copy
{
    s32     OrigFBP;
    s32     NewFBP;
    u32     Mask;
    s16     OrigWidth;
    s16     OrigHeight;
    s16     NewWidth;
    s16     NewHeight;
    s16     OrigL, OrigT, OrigR, OrigB;
    s16     NewL,  NewT,  NewR,  NewB;
    xcolor  VertColor;
    u64     AlphaBlend;
    f32     XOffset;
    f32     YOffset;
    s32     ZOffset;
};

//=============================================================================
//=============================================================================
// Internal data
//=============================================================================
//=============================================================================

struct post_effect_params
{
    // flags for turning post-effects on and off
    u32 Override        : 1;    // Set this to play around with values in the debugger
    u32 DoMotionBlur    : 1;
    u32 DoSelfIllumGlow : 1;
    u32 DoMultScreen    : 1;
    u32 DoRadialBlur    : 1;
    u32 DoZFogFn        : 1;
    u32 DoZFogCustom    : 1;
    u32 DoMipFn         : 1;
    u32 DoMipCustom     : 1;
    u32 DoNoise         : 1;
    u32 DoScreenFade    : 1;
    u32 NScreenWarps    : 4;

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
    render::post_falloff_fn     FogFn[MAX_PALETTES];
    xcolor                      FogColor[MAX_PALETTES];
    f32                         FogParam1[MAX_PALETTES];
    f32                         FogParam2[MAX_PALETTES];
    s32                         FogPaletteIndex;
    render::post_falloff_fn     MipFn[MAX_PALETTES];
    xcolor                      MipColor[MAX_PALETTES];
    f32                         MipParam1[MAX_PALETTES];
    f32                         MipParam2[MAX_PALETTES];
    s32                         MipCount[MAX_PALETTES];
    f32                         MipOffset[MAX_PALETTES];
    s32                         MipPaletteIndex;
    xcolor                      NoiseColor;
    xcolor                      FadeColor;
    vector3                     ScreenWarpPos[MAX_SCREEN_WARPS];
    f32                         ScreenWarpRadius[MAX_SCREEN_WARPS];
    f32                         ScreenWarpAmount[MAX_SCREEN_WARPS];
};

// misc. data common to all post-effects
static post_effect_params           s_Post;
static xbool                        s_InPost = FALSE;
static s32                          s_PostBackBuffer;           // page address
static s32                          s_PostFrontBuffer;          // page address
static s32                          s_PostZBuffer;              // page address
static s32                          s_PostMip[MAX_POST_MIPS];   // page address
static s32                          s_PostViewL;
static s32                          s_PostViewT;
static s32                          s_PostViewR;
static s32                          s_PostViewB;
static f32                          s_PostNearZ = 1.0f;
static f32                          s_PostFarZ  = 2.0f;

// fog data
static s32                          s_hFogClutHandle[MAX_PALETTES] = { -1, -1, -1 };
static xbool                        s_bFogValid[MAX_PALETTES]      = { FALSE, FALSE, FALSE };
static u8                           s_FogPalette[MAX_PALETTES][1024] PS2_ALIGNMENT(16);

// mip filter data
static const xbitmap*               s_pMipTexture;
static s32                          s_hMipClutHandle[MAX_PALETTES] = { -1, -1, -1 };
static u8                           s_MipPalette[MAX_PALETTES][1024] PS2_ALIGNMENT(16);

// noise filter data
static const s32 kNoiseMapW = 64;
static const s32 kNoiseMapH = 64;
static xbitmap   s_NoiseBitmap;
static u32       s_NoiseMap[kNoiseMapW*kNoiseMapH/8] PS2_ALIGNMENT(16) =
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

static u32 s_NoisePalette[16] PS2_ALIGNMENT(16) =
{
    0x62626262, 0x64646464, 0x66666666, 0x68686868,
    0x6a6a6a6a, 0x6c6c6c6c, 0x6e6e6e6e, 0x70707070,
    0x72727272, 0x74747474, 0x76767676, 0x78787878,
    0x7a7a7a7a, 0x7c7c7c7c, 0x7e7e7e7e, 0x80808080,
};

//=============================================================================
//=============================================================================
// Static declarations so we can do X_SECTIONs
//=============================================================================
//=============================================================================

static  s32     ps2_TexLog              ( s32 Dimension )           X_SECTION( render_deferred_post );
static  void    ps2_ScreenCopy          ( const screen_copy& SC )   X_SECTION( render_deferred_post );
static  void    ps2_MotionBlur          ( void )                    X_SECTION( render_deferred_post );
static  void    ps2_BuildScreenMips     ( s32 nMips,
                                          xbool UseAlpha )          X_SECTION( render_deferred_post );
static  void    ps2_ApplySelfIllumGlows ( void )                    X_SECTION( render_deferred_post );
static  void    ps2_CopyBackBuffer      ( void )                    X_SECTION( render_deferred_post );
static  void    ps2_MultScreen          ( void )                    X_SECTION( render_deferred_post );
static  void    ps2_RadialBlur          ( void )                    X_SECTION( render_deferred_post );
static  void    ps2_ScreenWarps         ( void )                    X_SECTION( render_deferred_post );
static  void    ps2_CopyRG2BA           ( s32 XRes,
                                          s32 YRes,
                                          s32 ZBP )                 X_SECTION( render_deferred_post );
static void     ps2_SetFogTexture       ( void )                    X_SECTION( render_deferred_post );
static void     ps2_RenderFogSprite     ( void )                    X_SECTION( render_deferred_post );
static void     ps2_ZFogFilter          ( void )                    X_SECTION( render_deferred_post );
static void     ps2_SetMipTexture       ( void )                    X_SECTION( render_deferred_post );
static void     ps2_SetMipTexture       ( const xbitmap* pBitmap )  X_SECTION( render_deferred_post );
static void     ps2_RemapAlpha          ( void )                    X_SECTION( render_deferred_post );
static void     ps2_RenderMipSprites    ( void )                    X_SECTION( render_deferred_post );
static void     ps2_MipFilter           ( void )                    X_SECTION( render_deferred_post );
static void     ps2_CreateFalloffPalette( u8* pPalette,
                                          render::hgeom_inst Fn,
                                          xcolor Color,
                                          f32 Param1,
                                          f32 Param2 )              X_SECTION( render_post );
static void     ps2_CreateFogPalette    ( render::post_falloff_fn Fn,
                                          xcolor Color,
                                          f32 Param1,
                                          f32 Param2 )              X_SECTION( render_post );
static void     ps2_CreateMipPalette    ( render::post_falloff_fn Fn,
                                          xcolor Color,
                                          f32 Param1,
                                          f32 Param2,
                                          s32 PaletteIndex )        X_SECTION( render_post );
static void     ps2_NoiseFilter         ( void )                    X_SECTION( render_deferred_post );
static void     ps2_ScreenFade          ( void )                    X_SECTION( render_deferred_post );

//=============================================================================
//=============================================================================
// Internal routines
//=============================================================================
//=============================================================================

static
s32 ps2_TexLog( s32 Dimension )
{
    switch (Dimension)
    {
    case 8:     return 3;
    case 16:    return 4;
    case 32:    return 5;
    case 64:    return 6;
    case 128:   return 7;
    case 256:   return 8;
    case 512:   return 9;
    default:
        ASSERT( FALSE );
        return 1;
    }
}

//=============================================================================

static
void ps2_ScreenCopy( const screen_copy& SC )
{
    CONTEXT( "ps2_ScreenCopy" );

    // set up the frame buffer and texture
    gsreg_Begin( 6 );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( SC.NewFBP, SC.NewWidth/64, SCE_GS_PSMCT32, SC.Mask ) );
    gsreg_Set( SCE_GS_TEX0_1,  SCE_GS_SET_TEX0( SC.OrigFBP*32, SC.OrigWidth/64, SCE_GS_PSMCT32,
                                                ps2_TexLog(SC.OrigWidth), ps2_TexLog(SC.OrigHeight), 1, 0, 0, 0, 0, 0, 0 ) );
    gsreg_SetScissor( SC.NewL, SC.NewT, SC.NewR, SC.NewB );
    gsreg_Set( SCE_GS_ALPHA_1, SC.AlphaBlend );
    gsreg_Set( SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 2, 2, SC.OrigL, SC.OrigR-1, SC.OrigT, SC.OrigB-1 ) );
    gsreg_End();

    // Copy the screen in vertical strips (this will reduce texture/frame cache misses, and
    // give us a much nicer fill rate), using page-width columns is sufficient when bilinear is
    // off, but we need half page-width columns when bilinear is on.
    s32 nColumns = SC.OrigWidth / 32;
    nColumns = MAX( (SC.NewWidth / 64), nColumns );

    s32 SrcDW = SC.OrigWidth / nColumns;
    s32 DstDW = SC.NewWidth  / nColumns;
    ASSERT( (SrcDW % nColumns) == 0 );
    ASSERT( (DstDW % nColumns) == 0 );
    SrcDW = SrcDW<<4;
    DstDW = DstDW<<4;
    s32 XOffset = (s32)(SC.XOffset*16.0f);
    s32 YOffset = (s32)(SC.YOffset*16.0f);
    s32 LClip   = ((SCISSOR_LEFT+SC.NewL)<<4) - (1<<3);
    s32 RClip   = ((SCISSOR_LEFT+SC.NewR)<<4) - (1<<3);
    s32 X0      = (SCISSOR_LEFT<<4) - (1<<3) + XOffset;
    s32 X1      = X0 + DstDW;
    s32 Y0      = (SCISSOR_TOP<<4) - (1<<3) + YOffset;
    s32 Y1      = Y0 + (SC.NewHeight<<4);
    s32 U0      = 0;
    s32 U1      = SrcDW;
    s32 V0      = 0;
    s32 V1      = (SC.OrigHeight<<4);

    // Figure out how many columns will actually be rendered (i.e. how many registers do we need)
    // TODO: Do this mathematically rather than looping.
    s32 i;
    s32 NRegs = 2;
    for( i = 0; i < nColumns; i++ )
    {
        if ( X0 >= RClip )
            break;

        if ( X1 > LClip )
            NRegs += 4;

        X0 += DstDW;
        X1 += DstDW;
    }

    // Reset the loop params
    X0 = (SCISSOR_LEFT<<4) - (1<<3) + XOffset;
    X1 = X0 + DstDW;

    // render the strips
    gsreg_Begin( NRegs );
    gsreg_Set( SCE_GS_RGBAQ,  SCE_GS_SET_RGBAQ( SC.VertColor.R, SC.VertColor.G, SC.VertColor.B, SC.VertColor.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_PRIM,   SCE_GS_SET_PRIM( GIF_PRIM_SPRITE, 1, 1, 0, 1, 0, 1, 0, 0 ) );
    for( i = 0; i < nColumns; i++ )
    {
        if ( X0 >= RClip )
            break;

        if ( X1 > LClip )
        {
            gsreg_Set( SCE_GS_UV,     SCE_GS_SET_UV  ( U0, V0    ) );
            gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ2( X0, Y0, SC.ZOffset ) );
            gsreg_Set( SCE_GS_UV,     SCE_GS_SET_UV  ( U1, V1    ) );
            gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ2( X1, Y1, SC.ZOffset ) );
        }

        X0 += DstDW;
        X1 += DstDW;
        U0 += SrcDW;
        U1 += SrcDW;
    }
    gsreg_End();
}

//=============================================================================

static
void ps2_MotionBlur()
{
    CONTEXT( "ps2_MotionBlur" );

    // set up a bilinear fill with no zbuffer tests or writing at all
    gsreg_Begin( 3 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_SetZBufferUpdate(FALSE);
    gsreg_End();

    // copy the front buffer over the back buffer with some alpha
    screen_copy SC;
    s32 iIntensity = (s32)(s_Post.MotionBlurIntensity * 128.0f);
    SC.OrigFBP    = s_PostFrontBuffer;
    SC.NewFBP     = s_PostBackBuffer;
    SC.Mask       = 0xff000000;
    SC.OrigWidth  = VRAM_FRAME_BUFFER_WIDTH;
    SC.OrigHeight = VRAM_FRAME_BUFFER_HEIGHT;
    SC.NewWidth   = VRAM_FRAME_BUFFER_WIDTH;
    SC.NewHeight  = VRAM_FRAME_BUFFER_HEIGHT;
    SC.OrigL      = s_PostViewL;
    SC.OrigT      = s_PostViewT;
    SC.OrigR      = s_PostViewR;
    SC.OrigB      = s_PostViewB;
    SC.NewL       = s_PostViewL;
    SC.NewT       = s_PostViewT;
    SC.NewR       = s_PostViewR;
    SC.NewB       = s_PostViewB;
    SC.VertColor.Set(128,128,128,128);
    SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_DST,A_FIX,C_DST)) | (((u64)iIntensity)<<32);
    SC.XOffset    = 0.0f;
    SC.YOffset    = 0.0f;
    SC.ZOffset    = 0;
    ps2_ScreenCopy( SC );
}

//=============================================================================

static
void ps2_BuildScreenMips( s32 nMips, xbool UseAlpha )
{
    CONTEXT( "ps2_BuildScreenMips" );

    // filter the screen down a few times
    s32 CurrentFBP    = s_PostBackBuffer;
    s32 CurrentWidth  = VRAM_FRAME_BUFFER_WIDTH;
    s32 CurrentHeight = VRAM_FRAME_BUFFER_HEIGHT;
    s32 NextFBP       = s_PostMip[0];
    s32 NextWidth     = VRAM_FRAME_BUFFER_WIDTH>>1;
    s32 NextHeight    = VRAM_FRAME_BUFFER_HEIGHT>>1;
    s32 CurrL         = s_PostViewL;
    s32 CurrT         = s_PostViewT;
    s32 CurrR         = s_PostViewR;
    s32 CurrB         = s_PostViewB;
    s32 NextL         = s_PostViewL>>1;
    s32 NextT         = s_PostViewT>>1;
    s32 NextR         = s_PostViewR>>1;
    s32 NextB         = s_PostViewB>>1;
    for ( s32 i = 0; i < nMips; i++ )
    {
        screen_copy SC;
        SC.OrigFBP    = CurrentFBP;
        SC.NewFBP     = NextFBP;
        SC.Mask       = 0x00000000;
        SC.OrigWidth  = CurrentWidth;
        SC.OrigHeight = CurrentHeight;
        SC.NewWidth   = NextWidth;
        SC.NewHeight  = NextHeight;
        SC.OrigL      = CurrL;
        SC.OrigT      = CurrT;
        SC.OrigR      = CurrR;
        SC.OrigB      = CurrB;
        SC.NewL       = NextL;
        SC.NewT       = NextT;
        SC.NewR       = NextR;
        SC.NewB       = NextB;
        SC.VertColor.Set(128,128,128,128);
        if ( UseAlpha )
            SC.AlphaBlend = (i==0) ? (ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_SRC,C_ZERO)) : (ALPHA_BLEND_MODE(C_ZERO,C_ZERO,A_SRC,C_SRC));
        else
            SC.AlphaBlend = ALPHA_BLEND_MODE(C_ZERO,C_ZERO,A_SRC,C_SRC);
        SC.XOffset    = 0.0f;
        SC.YOffset    = 0.0f;
        SC.ZOffset    = 0;
        ps2_ScreenCopy( SC );

        CurrentFBP     = NextFBP;
        CurrentWidth   = NextWidth;
        CurrentHeight  = NextHeight;
        NextFBP        = s_PostMip[(i+1)%MAX_POST_MIPS];
        NextWidth      = NextWidth>>1;
        NextHeight     = NextHeight>>1;
        CurrL          = NextL;
        CurrT          = NextT;
        CurrR          = NextR;
        CurrB          = NextB;
        NextL          = NextL>>1;
        NextT          = NextT>>1;
        NextR          = NextR>>1;
        NextB          = NextB>>1;
    }
}

//=============================================================================

static
void ps2_ApplySelfIllumGlows( void )
{
    CONTEXT( "ps2_ApplySelfIllumGlows" );

    s32 i;
    s32 Src, Dst;

    // set up a bilinear fill with no zbuffer tests or writing at all
    gsreg_Begin( 3 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_SetZBufferUpdate(FALSE);
    gsreg_End();

    // copy the front buffer alpha to the back buffer
    screen_copy SC;
    SC.OrigFBP    = s_PostFrontBuffer;
    SC.NewFBP     = s_PostBackBuffer;
    SC.Mask       = 0x00FFFFFF;
    SC.OrigWidth  = VRAM_FRAME_BUFFER_WIDTH;
    SC.OrigHeight = VRAM_FRAME_BUFFER_HEIGHT;
    SC.NewWidth   = VRAM_FRAME_BUFFER_WIDTH;
    SC.NewHeight  = VRAM_FRAME_BUFFER_HEIGHT;
    SC.OrigL      = s_PostViewL;
    SC.OrigT      = s_PostViewT;
    SC.OrigR      = s_PostViewR;
    SC.OrigB      = s_PostViewB;
    SC.NewL       = s_PostViewL;
    SC.NewT       = s_PostViewT;
    SC.NewR       = s_PostViewR;
    SC.NewB       = s_PostViewB;
    SC.VertColor.Set(128,128,128,128);
    SC.AlphaBlend = ALPHA_BLEND_MODE(C_ZERO,C_ZERO,A_SRC,C_SRC);
    SC.XOffset    = 0.0f;
    SC.YOffset    = 0.0f;
    SC.ZOffset    = 0;
    ps2_ScreenCopy( SC );

    // filter the screen down a few times
    ps2_BuildScreenMips(MAX_POST_MIPS, TRUE);

    // do a motion blur on the glow portion
    Src = (vram_GetPermanentArea()/32) + 2;
    Dst = s_PostMip[2];
    SC.OrigFBP    = Src;
    SC.NewFBP     = Dst;
    SC.Mask       = 0xFF000000;
    SC.OrigWidth  = (VRAM_FRAME_BUFFER_WIDTH>>3);
    SC.OrigHeight = (VRAM_FRAME_BUFFER_HEIGHT>>3);
    SC.NewWidth   = (VRAM_FRAME_BUFFER_WIDTH>>3);
    SC.NewHeight  = (VRAM_FRAME_BUFFER_HEIGHT>>3);
    SC.OrigL      = (s_PostViewL>>3);
    SC.OrigT      = (s_PostViewT>>3);
    SC.OrigR      = (s_PostViewR>>3);
    SC.OrigB      = (s_PostViewB>>3);
    SC.NewL       = (s_PostViewL>>3);
    SC.NewT       = (s_PostViewT>>3);
    SC.NewR       = (s_PostViewR>>3);
    SC.NewB       = (s_PostViewB>>3);
    SC.VertColor.Set(128,128,128,128);
    s32 iIntensity = (s32)(s_Post.GlowMotionBlurIntensity*128.0f);
    SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_DST)) | (((u64)(iIntensity))<<32);
    SC.XOffset    = 0.0f;
    SC.YOffset    = 0.0f;
    SC.ZOffset    = 0;
    ps2_ScreenCopy( SC );

    // make a copy of glow buffer which can be used for accumulating it the next frame
    // (i.e. we can motion blur the glow separate from a normal motion blur)
    // we'll put this at the bottom of the zbuffer which has some unused space because
    // the zbuffer is only 512x448, but 512x512 has been reserved. The environment map
    // also goes there, and so we should skip past that as well. The environment map
    // currently uses 2 pages. We have plenty left over...
    Src = s_PostMip[2];
    Dst = (vram_GetPermanentArea()/32) + 2;
    SC.OrigFBP    = Src;
    SC.NewFBP     = Dst;
    SC.Mask       = 0x00000000;
    SC.OrigWidth  = (VRAM_FRAME_BUFFER_WIDTH>>3);
    SC.OrigHeight = (VRAM_FRAME_BUFFER_HEIGHT>>3);
    SC.NewWidth   = (VRAM_FRAME_BUFFER_WIDTH>>3);
    SC.NewHeight  = (VRAM_FRAME_BUFFER_HEIGHT>>3);
    SC.OrigL      = (s_PostViewL>>3);
    SC.OrigT      = (s_PostViewT>>3);
    SC.OrigR      = (s_PostViewR>>3);
    SC.OrigB      = (s_PostViewB>>3);
    SC.NewL       = (s_PostViewL>>3);
    SC.NewT       = (s_PostViewT>>3);
    SC.NewR       = (s_PostViewR>>3);
    SC.NewB       = (s_PostViewB>>3);
    SC.VertColor.Set(128,128,128,128);
    iIntensity = (s32)(s_Post.GlowMotionBlurIntensity*128.0f);
    SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_ZERO,C_ZERO,A_FIX,C_SRC)) | (((u64)(iIntensity))<<32);
    SC.XOffset    = 0.0f;
    SC.YOffset    = 0.0f;
    SC.ZOffset    = 0;
    ps2_ScreenCopy( SC );

    // do a fake gaussian filter onto "blur" image
    struct glow_pass
    {
        s32 XOffset;
        s32 YOffset;
        u8  Alpha;
    };


    static const s32 kHorzPasses = 9;
    static const s32 kVertPasses = 9;
    static glow_pass HorzPasses[kHorzPasses] =
    {
        // alphas from zero to one are 0.1875, 0.15625, 0.125, 0.078125, and 0.0625
        {  0,  0, 24 },
        { -1,  0, 20 }, { -2,  0, 16 }, { -3,  0, 10 }, { -4,  0,  8 },
        {  1,  0, 20 }, {  2,  0, 16 }, {  3,  0, 10 }, {  4,  0,  8 },
    };
    static glow_pass VertPasses[kVertPasses] =
    {
        // alphas from zero to one are 0.15625, 0.125, 0.078125, and 0.0625 divided by 0.1875
        {  0,  0, 96 },
        {  0, -1, 40 }, {  0, -2, 32 }, {  0, -3, 20 }, {  0, -4,  16 },
        {  0,  1, 40 }, {  0,  2, 32 }, {  0,  3, 20 }, {  0,  4,  16 },
    };

    Src = s_PostMip[2];
    Dst = s_PostMip[2] + (4*(VRAM_FRAME_BUFFER_WIDTH>>3)*(VRAM_FRAME_BUFFER_HEIGHT>>3))/8192;
    for ( i = 0; i < kHorzPasses; i++ )
    {
        s32 HorzAlpha = HorzPasses[i].Alpha;
        SC.OrigFBP    = Src;
        SC.NewFBP     = Dst;
        SC.Mask       = 0xFF000000;
        SC.OrigWidth  = (VRAM_FRAME_BUFFER_WIDTH>>3);
        SC.OrigHeight = (VRAM_FRAME_BUFFER_HEIGHT>>3);
        SC.NewWidth   = (VRAM_FRAME_BUFFER_WIDTH>>3);
        SC.NewHeight  = (VRAM_FRAME_BUFFER_HEIGHT>>3);
        SC.OrigL      = (s_PostViewL>>3);
        SC.OrigT      = (s_PostViewT>>3);
        SC.OrigR      = (s_PostViewR>>3);
        SC.OrigB      = (s_PostViewB>>3);
        SC.NewL       = (s_PostViewL>>3);
        SC.NewT       = (s_PostViewT>>3);
        SC.NewR       = (s_PostViewR>>3);
        SC.NewB       = (s_PostViewB>>3);
        SC.VertColor.Set(128,128,128,128);
        if ( i == 0 )
            SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_ZERO)) | (((u64)HorzAlpha)<<32);
        else
            SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_DST))  | (((u64)HorzAlpha)<<32);
        SC.XOffset    = (f32)HorzPasses[i].XOffset;
        SC.YOffset    = (f32)HorzPasses[i].YOffset;
        SC.ZOffset    = 0;
        ps2_ScreenCopy( SC );
    }

    s32 Tmp = Src;
    Src     = Dst;
    Dst     = Tmp;
    for ( i = 0; i < kVertPasses; i++ )
    {
        s32 VertAlpha = VertPasses[i].Alpha;
        SC.OrigFBP    = Src;
        SC.NewFBP     = Dst;
        SC.Mask       = 0xFF000000;
        SC.OrigWidth  = (VRAM_FRAME_BUFFER_WIDTH>>3);
        SC.OrigHeight = (VRAM_FRAME_BUFFER_HEIGHT>>3);
        SC.NewWidth   = (VRAM_FRAME_BUFFER_WIDTH>>3);
        SC.NewHeight  = (VRAM_FRAME_BUFFER_HEIGHT>>3);
        SC.OrigL      = (s_PostViewL>>3);
        SC.OrigT      = (s_PostViewT>>3);
        SC.OrigR      = (s_PostViewR>>3);
        SC.OrigB      = (s_PostViewB>>3);
        SC.NewL       = (s_PostViewL>>3);
        SC.NewT       = (s_PostViewT>>3);
        SC.NewR       = (s_PostViewR>>3);
        SC.NewB       = (s_PostViewB>>3);
        SC.VertColor.Set(128,128,128,128);
        if ( i == 0 )
            SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_ZERO)) | (((u64)VertAlpha)<<32);
        else
            SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_DST))  | (((u64)VertAlpha)<<32);
        SC.XOffset    = (f32)VertPasses[i].XOffset;
        SC.YOffset    = (f32)VertPasses[i].YOffset;
        SC.ZOffset    = 0;
        ps2_ScreenCopy( SC );
    }

    if ( s_Post.GlowCutoff != 255 )
    {
        // turn on an alpha kill to leave the center portion clear
        gsreg_Begin( 1 );
        gsreg_SetAlphaAndZBufferTests( TRUE, ALPHA_TEST_LEQUAL, s_Post.GlowCutoff, ALPHA_TEST_FAIL_KEEP,
                                       FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
        gsreg_End();
    }

    // render the half size image over the screen
    SC.OrigFBP    = s_PostMip[0];
    SC.NewFBP     = s_PostBackBuffer;
    SC.Mask       = 0xFF000000;
    SC.OrigWidth  = (VRAM_FRAME_BUFFER_WIDTH>>1);
    SC.OrigHeight = (VRAM_FRAME_BUFFER_HEIGHT>>1);
    SC.NewWidth   = (VRAM_FRAME_BUFFER_WIDTH>>0);
    SC.NewHeight  = (VRAM_FRAME_BUFFER_HEIGHT>>0);
    SC.OrigL      = (s_PostViewL>>1);
    SC.OrigT      = (s_PostViewT>>1);
    SC.OrigR      = (s_PostViewR>>1);
    SC.OrigB      = (s_PostViewB>>1);
    SC.NewL       = s_PostViewL;
    SC.NewT       = s_PostViewT;
    SC.NewR       = s_PostViewR;
    SC.NewB       = s_PostViewB;
    SC.VertColor.Set(128,128,128,128);
    SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_DST)) | (((u64)64)<<32);
    SC.XOffset    = 0.0f;
    SC.YOffset    = 0.0f;
    SC.ZOffset    = 0;
    ps2_ScreenCopy( SC );

    // render the quarter size image over the screen
    SC.OrigFBP    = s_PostMip[1];
    SC.NewFBP     = s_PostBackBuffer;
    SC.Mask       = 0xFF000000;
    SC.OrigWidth  = (VRAM_FRAME_BUFFER_WIDTH>>2);
    SC.OrigHeight = (VRAM_FRAME_BUFFER_HEIGHT>>2);
    SC.NewWidth   = (VRAM_FRAME_BUFFER_WIDTH>>0);
    SC.NewHeight  = (VRAM_FRAME_BUFFER_WIDTH>>0);
    SC.OrigL      = (s_PostViewL>>2);
    SC.OrigT      = (s_PostViewT>>2);
    SC.OrigR      = (s_PostViewR>>2);
    SC.OrigB      = (s_PostViewB>>2);
    SC.NewL       = s_PostViewL;
    SC.NewT       = s_PostViewT;
    SC.NewR       = s_PostViewR;
    SC.NewB       = s_PostViewB;
    SC.VertColor.Set(128,128,128,128);
    SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_DST)) | (((u64)64)<<32);
    SC.XOffset    = 0.0f;
    SC.YOffset    = 0.0f;
    SC.ZOffset    = 0;
    ps2_ScreenCopy( SC );

    // and then we just bilinear our "blur" image over the screen
    SC.OrigFBP    = Dst;
    SC.NewFBP     = s_PostBackBuffer;
    SC.Mask       = 0xFF000000;
    SC.OrigWidth  = (VRAM_FRAME_BUFFER_WIDTH>>3);
    SC.OrigHeight = (VRAM_FRAME_BUFFER_HEIGHT>>3);
    SC.NewWidth   = (VRAM_FRAME_BUFFER_WIDTH>>0);
    SC.NewHeight  = (VRAM_FRAME_BUFFER_HEIGHT>>0);
    SC.OrigL      = (s_PostViewL>>3);
    SC.OrigT      = (s_PostViewT>>3);
    SC.OrigR      = (s_PostViewR>>3);
    SC.OrigB      = (s_PostViewB>>3);
    SC.NewL       = s_PostViewL;
    SC.NewT       = s_PostViewT;
    SC.NewR       = s_PostViewR;
    SC.NewB       = s_PostViewB;
    SC.VertColor.Set(128,128,128,128);
    SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_DST)) | (((u64)128)<<32);
    SC.XOffset    = 0.0f;
    SC.YOffset    = 0.0f;
    SC.ZOffset    = 0;
    ps2_ScreenCopy( SC );

    // restore the standard alpha/z-buffer test
    if ( s_Post.GlowCutoff != 255 )
    {
        gsreg_Begin( 2 );
        gsreg_Set( SCE_GS_FBA_1, 0 );
        gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                       FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
        gsreg_End();
    }
}

//=============================================================================

static
void ps2_CopyBackBuffer( void )
{
    CONTEXT( "ps2_CopyBackBuffer" );

    // set up a bilinear fill with no zbuffer tests or writing at all
    gsreg_Begin( 3 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_SetZBufferUpdate(FALSE);
    gsreg_End();

    // make a copy of the backbuffer in the free memory
    screen_copy SC;
    SC.OrigFBP    = s_PostBackBuffer;
    SC.NewFBP     = VRAM_FREE_MEMORY_START/8192;
    SC.Mask       = 0x00000000;
    SC.OrigWidth  = VRAM_FRAME_BUFFER_WIDTH;
    SC.OrigHeight = VRAM_FRAME_BUFFER_HEIGHT;
    SC.NewWidth   = VRAM_FRAME_BUFFER_WIDTH>>1;
    SC.NewHeight  = VRAM_FRAME_BUFFER_HEIGHT>>1;
    SC.OrigL      = s_PostViewL;
    SC.OrigT      = s_PostViewT;
    SC.OrigR      = s_PostViewR;
    SC.OrigB      = s_PostViewB;
    SC.NewL       = s_PostViewL>>1;
    SC.NewT       = s_PostViewT>>1;
    SC.NewR       = s_PostViewR>>1;
    SC.NewB       = s_PostViewB>>1;
    SC.VertColor.Set(128,128,128,128);
    SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_FIX,C_ZERO)) | (((u64)128)<<32);
    SC.XOffset    = 0.0f;
    SC.YOffset    = 0.0f;
    SC.ZOffset    = 0;
    ps2_ScreenCopy( SC );
}

//=============================================================================

static
void ps2_MultScreen( void )
{
    CONTEXT( "ps2_MultScreen" );

    // copy the backup backbuffer over to the real backbuffer
    screen_copy SC;
    SC.OrigFBP     = VRAM_FREE_MEMORY_START/8192;
    SC.NewFBP      = s_PostBackBuffer;
    SC.Mask        = 0xff000000;
    SC.OrigWidth   = VRAM_FRAME_BUFFER_WIDTH>>1;
    SC.OrigHeight  = VRAM_FRAME_BUFFER_HEIGHT>>1;
    SC.NewWidth    = VRAM_FRAME_BUFFER_WIDTH;
    SC.NewHeight   = VRAM_FRAME_BUFFER_HEIGHT;
    SC.OrigL       = s_PostViewL>>1;
    SC.OrigT       = s_PostViewT>>1;
    SC.OrigR       = s_PostViewR>>1;
    SC.OrigB       = s_PostViewB>>1;
    SC.NewL        = s_PostViewL;
    SC.NewT        = s_PostViewT;
    SC.NewR        = s_PostViewR;
    SC.NewB        = s_PostViewB;
    SC.VertColor   = s_Post.MultScreenColor;
    SC.VertColor.A =128;
    SC.XOffset     = 0.0f;
    SC.YOffset     = 0.0f;
    SC.ZOffset     = 0;
    switch( s_Post.MultScreenBlend )
    {
    default:
    case render::SOURCE_MINUS_DEST:
        SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_ZERO,C_DST,A_FIX,C_SRC)) | (((u64)s_Post.MultScreenColor.A)<<32);
        break;
    }

    ps2_ScreenCopy( SC );
}

//=============================================================================

static
void ps2_RadialBlur( void )
{
    CONTEXT( "ps2_RadialBlur" );
    
    // set up the proper blur registers
    gsreg_Begin( 3 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_SetZBufferUpdate(FALSE);
    gsreg_End();

    static const s32 kHorzVert    = 17;
    static const s32 kVertVert    = 17;

    // set up the blends and texture for doing the radial blur
    gsreg_Begin( 6 );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( s_PostBackBuffer, VRAM_FRAME_BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0xff000000 ) );
    gsreg_Set( SCE_GS_TEX0_1,  SCE_GS_SET_TEX0( (VRAM_FREE_MEMORY_START/8192)*32, (VRAM_FRAME_BUFFER_WIDTH>>1)/64, SCE_GS_PSMCT32,
                                                ps2_TexLog(VRAM_FRAME_BUFFER_WIDTH>>1), ps2_TexLog(VRAM_FRAME_BUFFER_HEIGHT>>1), 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_SetScissor( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );
    gsreg_Set( SCE_GS_ALPHA_1, (u64)(ALPHA_BLEND_MODE(C_SRC,C_DST,A_SRC,C_DST)) );
    gsreg_Set( SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 2, 2, s_PostViewL>>1, (s_PostViewR>>1)-1, s_PostViewT>>1, (s_PostViewB>>1)-1 ) );
    gsreg_End();

    f32 RadialWidth  = s_PostViewR-s_PostViewL;
    f32 RadialHeight = s_PostViewB-s_PostViewT;
    vector2 ViewCenter( (f32)(s_PostViewR+s_PostViewL)/2, (f32)(s_PostViewB+s_PostViewT)/2 );
    ViewCenter.X /= (f32)VRAM_FRAME_BUFFER_WIDTH;
    ViewCenter.Y /= (f32)VRAM_FRAME_BUFFER_HEIGHT;
    radian Angle = s_Post.RadialAngle;
    for ( s32 iPass = 0; iPass < 2; iPass++ )
    {
        if ( iPass == 1 )
        {
            Angle = -Angle;
        }
        else
        if ( iPass == 2 )
        {
            Angle /= 2;
        }
        else
        if ( iPass == 3 )
        {
            Angle = -Angle;
        }

        // set up the horizontal positions
        //ASSERT( ((s32)RadialWidth  % (kHorzVert-1)) == 0 );
        //ASSERT( ((s32)RadialHeight % (kVertVert-1)) == 0 );
        f32 DeltaTX  = 1.0f / (f32)(kHorzVert-1);
        f32 DeltaTY  = 1.0f / (f32)(kVertVert-1);
        DeltaTX     *= RadialWidth  / (f32)VRAM_FRAME_BUFFER_WIDTH;
        DeltaTY     *= RadialHeight / (f32)VRAM_FRAME_BUFFER_HEIGHT;
        f32 TX0      = s_PostViewL / (f32)VRAM_FRAME_BUFFER_WIDTH;
        f32 TX1      = TX0 + DeltaTX;
        s32 DeltaSX  = ((s32)RadialWidth/(kHorzVert-1))<<4;
        s32 DeltaSY  = ((s32)RadialHeight/(kVertVert-1))<<4;
        s32 SX0      = (SCISSOR_LEFT<<4) - (1<<3) + (s32)(s_PostViewL*16.0f);
        s32 SX1      = SX0 + DeltaSX;

        for ( s32 x = 0; x < kHorzVert-1; x++ )
        {
            // set up the vertical positions
            f32 TY = s_PostViewT / (f32)VRAM_FRAME_BUFFER_HEIGHT;
            s32 SY = (s_PostViewT<<4) + (SCISSOR_TOP<<4) - (1<<3);

            // start the strip
            gsreg_Begin( 1 + 6*kVertVert );
            gsreg_Set( SCE_GS_PRIM,   SCE_GS_SET_PRIM( GIF_PRIM_TRIANGLESTRIP, 1, 1, 0, 1, 0, 0, 0, 0 ) );
            for ( s32 y = 0; y < kVertVert; y++ )
            {
                // calculate the uv's
                vector2 LUV( TX0 - ViewCenter.X, TY - ViewCenter.Y );
                vector2 RUV( TX1 - ViewCenter.X, TY - ViewCenter.Y );
                LUV.Scale( s_Post.RadialZoom );
                RUV.Scale( s_Post.RadialZoom );
                LUV.Rotate( Angle );
                RUV.Rotate( Angle );
                LUV += ViewCenter;
                RUV += ViewCenter;

                // calculate the alphas
                f32 DistX0 = (TX0-ViewCenter.X);
                f32 DistX1 = (TX1-ViewCenter.X);
                f32 DistY  = (TY-ViewCenter.Y);
                f32 DistL = x_sqrt(4.0f*DistX0*DistX0 + 4.0f*DistY*DistY);
                f32 DistR = x_sqrt(4.0f*DistX1*DistX1 + 4.0f*DistY*DistY);
                s32 AL    = (s32)(DistL*s_Post.RadialAlphaScale - s_Post.RadialAlphaSub);
                s32 AR    = (s32)(DistR*s_Post.RadialAlphaScale - s_Post.RadialAlphaSub);
                AL = MAX(AL, 0);
                AR = MAX(AR, 0);

                // render the verts
                gsreg_Set( SCE_GS_ST,     SCE_GS_SET_ST  ( reinterpret_cast<u32&>(LUV.X), reinterpret_cast<u32&>(LUV.Y) ) );
                gsreg_Set( SCE_GS_RGBAQ,  SCE_GS_SET_RGBAQ( 128, 128, 128, (s32)(AL), 0x3f800000 ) );
                gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ2( SX0, SY, 0 ) );
                gsreg_Set( SCE_GS_ST,     SCE_GS_SET_ST  ( reinterpret_cast<u32&>(RUV.X), reinterpret_cast<u32&>(RUV.Y) ) );
                gsreg_Set( SCE_GS_RGBAQ,  SCE_GS_SET_RGBAQ( 128, 128, 128, (s32)(AR), 0x3f800000 ) );
                gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ2( SX1, SY, 0 ) );

                // inc. by our deltas
                TY += DeltaTY;
                SY += DeltaSY;
            }

            // inc. by our deltas
            TX0 += DeltaTX;
            TX1 += DeltaTX;
            SX0 += DeltaSX;
            SX1 += DeltaSX;

            // end the strip
            gsreg_End();
        }
    }
}

//=============================================================================

static
void ps2_ScreenWarps( void )
{
    static const s32 kNumWarpSegments = 16;
    static const s32 kNumWarpRings    = 4;

    const matrix4& V2W = eng_GetView()->GetV2W();
    const matrix4& W2V = eng_GetView()->GetW2V();
    f32 ZNear, ZFar;
    eng_GetView()->GetZLimits( ZNear, ZFar );

    // apply some transformation to polar coordinates that will remap uvs for us
    struct polar_coord_remap
    {
        f32     VertRadius;
        radian  VertPhi;
        f32     TexRadius;
        radian  TexPhi;
    };
    polar_coord_remap Remaps[kNumWarpSegments*kNumWarpRings];

    s32 i, j, k;
    for( i = 0; i < (s32)s_Post.NScreenWarps; i++ )
    {
        const vector3& WorldPos = s_Post.ScreenWarpPos[i];
        f32            Radius   = s_Post.ScreenWarpRadius[i];
        f32            Amount   = s_Post.ScreenWarpAmount[i];

        // figure out the orientation we'll use to get us at the right scale
        // and facing the camera for our warp geometry
        vector3 ViewPos = W2V * WorldPos;
        matrix4 L2W = V2W;
        matrix4 L2S = eng_GetView()->GetW2S() * L2W;
        
        // throw it away if we're not within the view boundary
        if( (ViewPos.GetZ() < ZNear) || (ViewPos.GetZ() > ZFar) )
            continue;

        // apply some transformation to polar coordinates that will remap uvs for us
        for( j = 0; j < kNumWarpRings; j++ )
        {
            for( k = 0; k < kNumWarpSegments; k++ )
            {
                f32    R   = (f32)j/(f32)(kNumWarpRings-1);
                radian Phi = 2.0f*PI*(f32)k/(f32)(kNumWarpSegments-1);
                
                Remaps[j*kNumWarpSegments+k].VertRadius = R;
                Remaps[j*kNumWarpSegments+k].VertPhi    = Phi;
                Remaps[j*kNumWarpSegments+k].TexRadius  = x_pow( R, Amount );
                (void)Amount;
                //Remaps[j*kNumWarpSegments+k].TexRadius  = 0.5f - 0.5f * x_cos( R * PI );
                Remaps[j*kNumWarpSegments+k].TexPhi     = Phi;
            }
        }

        // set up the proper blend and texture registers for drawing
        gsreg_Begin( 9 );
        gsreg_Set( SCE_GS_TEXFLUSH, 0 );
        gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP, FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
        gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
        gsreg_SetZBufferUpdate( FALSE );
        gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( s_PostBackBuffer, VRAM_FRAME_BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0xff000000 ) );
        gsreg_Set( SCE_GS_TEX0_1,  SCE_GS_SET_TEX0( (VRAM_FREE_MEMORY_START/8192)*32, (VRAM_FRAME_BUFFER_WIDTH>>1)/64, SCE_GS_PSMCT32,
                   ps2_TexLog(VRAM_FRAME_BUFFER_WIDTH>>1), ps2_TexLog(VRAM_FRAME_BUFFER_HEIGHT>>1), 0, 0, 0, 0, 0, 0, 0 ) );
        gsreg_SetScissor( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );
        gsreg_Set( SCE_GS_ALPHA_1, (u64)(ALPHA_BLEND_MODE(C_ZERO,C_ZERO,A_SRC,C_SRC)) );
        gsreg_Set( SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 2, 2, s_PostViewL>>1, (s_PostViewR>>1)-1, s_PostViewT>>1, (s_PostViewB>>1)-1 ) );
        gsreg_End();

        // draw it
        for( j = 0; j < kNumWarpRings - 1; j++ )
        {
            // start a strip
            gsreg_Begin( 2 + (kNumWarpSegments+1)*4 );
            gsreg_Set( SCE_GS_PRIM, SCE_GS_SET_PRIM( GIF_PRIM_TRIANGLESTRIP, 1, 1, 0, 0, 0, 1, 0, 0 ) );
            //gsreg_Set( SCE_GS_PRIM, SCE_GS_SET_PRIM( GIF_PRIM_LINESTRIP, 1, 0, 0, 0, 0, 1, 0, 0 ) );
            gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( 128, 128, 128, 128, 0x3f800000 ) );

            s32 Clip = 0;
            for( k = 0; k <= kNumWarpSegments; k++ )
            {

                s32 I0 = (j*kNumWarpSegments) + (k%kNumWarpSegments);
                s32 I1 = ((j+1)*kNumWarpSegments) + (k%kNumWarpSegments);

                // get the vert positions in screen space
                f32 S0, C0, S1, C1;
                x_sincos( Remaps[I0].VertPhi, S0, C0 );
                x_sincos( Remaps[I1].VertPhi, S1, C1 );
                vector4 ScreenPos0( ViewPos.GetX() + Radius*Remaps[I0].VertRadius*C0,
                                    ViewPos.GetY() + Radius*Remaps[I0].VertRadius*S0,
                                    ViewPos.GetZ(), 1.0f );
                vector4 ScreenPos1( ViewPos.GetX() + Radius*Remaps[I1].VertRadius*C1,
                                    ViewPos.GetY() + Radius*Remaps[I1].VertRadius*S1,
                                    ViewPos.GetZ(), 1.0f );
                ScreenPos0 = L2S * ScreenPos0;
                ScreenPos1 = L2S * ScreenPos1;
                ScreenPos0 *= 1.0f / ScreenPos0.GetW();
                ScreenPos1 *= 1.0f / ScreenPos1.GetW();

                // get the uv positions in screen space (using our warping remap)
                x_sincos( Remaps[I0].TexPhi, S0, C0 );
                x_sincos( Remaps[I1].TexPhi, S1, C1 );
                vector4 TexPos0( ViewPos.GetX() + Radius*Remaps[I0].TexRadius*C0,
                                 ViewPos.GetY() + Radius*Remaps[I0].TexRadius*S0,
                                 ViewPos.GetZ(), 1.0f );
                vector4 TexPos1( ViewPos.GetX() + Radius*Remaps[I1].TexRadius*C1,
                                 ViewPos.GetY() + Radius*Remaps[I1].TexRadius*S1,
                                 ViewPos.GetZ(), 1.0f );
                TexPos0 = L2S * TexPos0;
                TexPos1 = L2S * TexPos1;
                TexPos0 *= 1.0f / TexPos0.GetW();
                TexPos1 *= 1.0f / TexPos1.GetW();

                // convert the screen and texture coordinates into fixed-point
                s32 X0 = (s32)(ScreenPos0.GetX() * 16.0f);
                s32 Y0 = (s32)(ScreenPos0.GetY() * 16.0f); 
                s32 X1 = (s32)(ScreenPos1.GetX() * 16.0f);
                s32 Y1 = (s32)(ScreenPos1.GetY() * 16.0f); 
                s32 U0 = (s32)((TexPos0.GetX()-(f32)SCISSOR_LEFT) * 8.0f);
                s32 V0 = (s32)((TexPos0.GetY()-(f32)SCISSOR_TOP) * 8.0f); 
                s32 U1 = (s32)((TexPos1.GetX()-(f32)SCISSOR_LEFT) * 8.0f);
                s32 V1 = (s32)((TexPos1.GetY()-(f32)SCISSOR_TOP) * 8.0f); 

                gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV( U0, V0 ) );
                if( (X0 & 0xFFFF0000) || (Y0 & 0xFFFF0000) ||
                    (U0 & 0xFFFF8000) || (V0 & 0xFFFF8000) )
                    Clip = 3;
                if( Clip )
                {
                    gsreg_Set( SCE_GS_XYZ3, SCE_GS_SET_XYZ( X0, Y0, 0 ) );
                    Clip--;
                }
                else
                {
                    gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( X0, Y0, 0 ) );
                }

                gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV( U1, V1 ) );
                if( (X1 & 0xFFFF0000) || (Y1 & 0xFFFF0000) ||
                    (U1 & 0xFFFF8000) || (V1 & 0xFFFF8000) )
                    Clip = 3;
                if( Clip )
                {
                    gsreg_Set( SCE_GS_XYZ3, SCE_GS_SET_XYZ( X1, Y1, 0 ) );
                    Clip--;
                }
                else
                {
                    gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( X1, Y1, 0 ) );
                }
            }

            gsreg_End();
        }
    }
}

//=============================================================================

static
void ps2_CopyRG2BA( s32 XRes, s32 YRes, s32 ZBP )
{
    CONTEXT( "ps2_CopyRG2BA" );

    // Need to copy double the height since we are copying 16-bit instead of 32-bit
    (void)XRes;
    s32 CopyHeight = YRes*2;

    // Copy 8 pixel wide columns at a time
    s32 ColumnWidth = 8;
    s32 nColumns    = VRAM_FRAME_BUFFER_WIDTH / ColumnWidth;

    // Setup GS registers for the copy
    gsreg_Begin( 12 + 4*(nColumns/2) );
    gsreg_Set( SCE_GS_FRAME_1,   SCE_GS_SET_FRAME_1  ( ZBP, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMCT16, 0x00003FFF ) );
    gsreg_Set( SCE_GS_ZBUF_1,    SCE_GS_SET_ZBUF     ( ZBP, SCE_GS_PSMZ24, 1 ) );
    gsreg_Set( SCE_GS_TEST_1,    SCE_GS_SET_TEST_1   ( 0, 0, 0, 0, 0, 0, 1, SCE_GS_ZALWAYS ) );
    gsreg_SetClamping(FALSE, FALSE);
    gsreg_Set( SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR_1( 0, VRAM_FRAME_BUFFER_WIDTH-1, 0, CopyHeight-1 ) );
    gsreg_Set( SCE_GS_TEXFLUSH,  0 );
    gsreg_Set( SCE_GS_TEXA,      SCE_GS_SET_TEXA     ( 0x00, 0, 0x80 ) );
    gsreg_Set( SCE_GS_TEX1_1,    SCE_GS_SET_TEX1_1   ( 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_TEX0_1,    SCE_GS_SET_TEX0_1   ( ZBP*32, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMZ16, 10, 10, 1, 1, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_PRIM,      SCE_GS_SET_PRIM     ( SCE_GS_PRIM_SPRITE, 0, 1, 0, 0, 0, 1, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,     SCE_GS_SET_RGBAQ    ( 128, 128, 128, 128, 0x3F800000 ) );

    s32 X  = 0;
    s32 Z  = 0;
    s32 V0 = 0;
    s32 Y0 = SCISSOR_TOP;
    s32 V1 = V0 + CopyHeight;
    s32 Y1 = Y0 + CopyHeight;

    // Copy every column to the right
    for( s32 i=0; i<nColumns; i+=2 )
    {
        s32 U0 = X;
        s32 X0 = SCISSOR_LEFT + X + ColumnWidth;
        
        s32 U1 = U0 + ColumnWidth;
        s32 X1 = X0 + ColumnWidth;
    
        gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV ( (U0 << 4) + 8, (V0 << 4) + 8 ) );
        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X0 << 4),     (Y0 << 4),  Z ) );
        gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV ( (U1 << 4) + 8, (V1 << 4) + 8 ) );
        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X1 << 4),     (Y1 << 4),  Z ) );

        X += ColumnWidth * 2;
    }

    // Reset scissoring region back to the screen dimensions
    gsreg_Set( SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR_1( s_PostViewL, s_PostViewR-1, s_PostViewT, s_PostViewB-1 ) );
    gsreg_End();
}

//=============================================================================

static
void ps2_SetFogTexture( void )
{
    CONTEXT( "ps2_SetFogTexture" );

    // load the clut
    ASSERT( s_hFogClutHandle[s_Post.FogPaletteIndex] != -1 );
    vram_LoadClut( s_hFogClutHandle[s_Post.FogPaletteIndex] );

    // set up the frame buffer as a texture in psmt8h mode
    s32 ClutAddr = vram_GetClutBaseAddr( s_hFogClutHandle[s_Post.FogPaletteIndex] );

    gsreg_Begin( 3 );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_TEX0_1, SCE_GS_SET_TEX0_1( 32*s_PostZBuffer, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMT8H, 10, 10, 1, 1, ClutAddr, SCE_GS_PSMCT32, 0, 0, 1 ) );
    gsreg_End();
}

//=============================================================================

static
void ps2_RenderFogSprite( void )
{
    CONTEXT( "ps2_RenderFogSprite" );

    s32 ColumnWidth = 32;
    s32 nColumns    = VRAM_FRAME_BUFFER_WIDTH / ColumnWidth;
    
    gsreg_Begin( 5 );
    gsreg_Set( SCE_GS_FRAME_1,   SCE_GS_SET_FRAME_1  ( s_PostBackBuffer, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMCT32, 0xFF000000 ) );
    gsreg_Set( SCE_GS_ZBUF_1,    SCE_GS_SET_ZBUF     ( s_PostZBuffer, SCE_GS_PSMZ24, 1 ) );
    gsreg_Set( SCE_GS_TEST_1,    SCE_GS_SET_TEST_1   ( 0, 0, 0, 0, 0, 0, 1, SCE_GS_ZGEQUAL ) );
    gsreg_Set( SCE_GS_ALPHA_1,   SCE_GS_SET_ALPHA    ( 0, 1, 0, 1, 0 ) );
    gsreg_Set( SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR_1( s_PostViewL, s_PostViewR-1, s_PostViewT, s_PostViewB-1 ) );
    gsreg_End();
    
    s32 Z;
    if ( s_Post.FogFn[s_Post.FogPaletteIndex] == render::FALLOFF_CONSTANT )
    {
        const view* pView = eng_GetView();

        // clamp fog to the near z plane
        f32 FogZ = MAX( s_Post.FogParam1[s_Post.FogPaletteIndex], s_PostNearZ+1.0f );
        FogZ     = MIN( s_Post.FogParam1[s_Post.FogPaletteIndex], s_PostFarZ -1.0f );

        // transform the fog z into screen space
        vector4 FogVec = pView->GetV2S() * vector4(0.0f,0.0f,FogZ,1.0f);
        FogVec.GetZ() /= FogVec.GetW();
        Z = (s32)(FogVec.GetZ()*16.0f);
    }
    else
    {
        Z = 0x0000ffff;
    }
    s32 X  = SCISSOR_LEFT;
    s32 Y0 = SCISSOR_TOP;
    s32 Y1 = Y0 + VRAM_FRAME_BUFFER_HEIGHT;
    s32 U  = 0;
    s32 V0 = 0;
    s32 V1 = VRAM_FRAME_BUFFER_HEIGHT;
    s32 LClip = SCISSOR_LEFT + s_PostViewL;
    s32 RClip = SCISSOR_LEFT + s_PostViewR;

    // figure out how many registers we'll need
    s32 i;
    s32 NRegs = 3;
    for( i = 0; i < nColumns; i++ )
    {
        if( X >= RClip )
            break;
        if( (X+ColumnWidth) > LClip )
            NRegs += 4;
        X += ColumnWidth;
    }
    X = SCISSOR_LEFT;

    gsreg_Begin( NRegs );
    gsreg_Set( SCE_GS_PRIM,      SCE_GS_SET_PRIM     ( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,     SCE_GS_SET_RGBAQ    ( 0x80, 0x80, 0x80, 0x80, 0x3F800000 ) );
    for( i=0; i<nColumns; i++ )
    {
        s32 X0 = X;
        s32 X1 = X0 + ColumnWidth;
        s32 U0 = U;
        s32 U1 = U0 + ColumnWidth;
    
        if ( X0 >= RClip )
            break;

        if ( X1 > LClip )
        {
            gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV ( (U0 << 4) + 8,  (V0 << 4) + 8 ) );
            gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X0 << 4),      (Y0 << 4), Z  ) );
            gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV ( (U1 << 4) + 8,  (V1 << 4) + 8 ) );
            gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X1 << 4),      (Y1 << 4), Z  ) );
        }
        
        X += ColumnWidth;
        U += ColumnWidth;
    }

    // reset the frame register
    gsreg_Set( SCE_GS_FRAME_1,   SCE_GS_SET_FRAME_1  ( s_PostBackBuffer, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMCT32, 0x00000000 ) );
    gsreg_End();
}

//=============================================================================

static
void ps2_ZFogFilter( void )
{
    CONTEXT( "ps2_ZFogFilter" );

    // upload the fog palette, and turn the alpha channel of the front buffer
    // into a PSMT8H texture
    ps2_SetFogTexture();

    // now just render the fog sprite with normal blending
    ps2_RenderFogSprite();
}

//=============================================================================

static
void ps2_SetMipTexture( void )
{
    CONTEXT( "ps2_SetMipTexture" );

    // load the clut
    ASSERT( s_Post.MipPaletteIndex != -1 );
    ASSERT( s_hMipClutHandle[s_Post.MipPaletteIndex] != -1 );
    vram_LoadClut( s_hMipClutHandle[s_Post.MipPaletteIndex] );

    // set up the frame buffer as a texture in psmt8h mode
    s32 ClutAddr = vram_GetClutBaseAddr( s_hMipClutHandle[s_Post.MipPaletteIndex] );
    gsreg_Begin( 3 );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_TEX0_1, SCE_GS_SET_TEX0_1( 32*s_PostZBuffer, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMT8H, 10, 10, 1, 1, ClutAddr, SCE_GS_PSMCT32, 0, 0, 1 ) );
    gsreg_End();
}

//=============================================================================

static
void ps2_SetMipTexture( const xbitmap* pBitmap )
{
    CONTEXT( "ps2_SetMipTexture" );

    ASSERT( pBitmap );
    vram_Activate( *pBitmap );
    s32 ClutAddr = vram_GetClutBaseAddr( *pBitmap );

    // set up the frame buffer as a texture in psmt8h mode
    gsreg_Begin( 3 );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_TEX0_1, SCE_GS_SET_TEX0_1( 32*s_PostZBuffer, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMT8H, 10, 10, 1, 0, ClutAddr, SCE_GS_PSMCT32, 0, 0, 1 ) );
    gsreg_End();
}

//=============================================================================

static
void ps2_RemapAlpha( void )
{
    CONTEXT( "ps2_RemapAlpha" );

    s32 ColumnWidth = 32;
    s32 nColumns    = VRAM_FRAME_BUFFER_WIDTH / ColumnWidth;
    
    gsreg_Begin( 6 );
    gsreg_Set( SCE_GS_FRAME_1,   SCE_GS_SET_FRAME_1  ( s_PostBackBuffer, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMCT32, 0x00FFFFFF ) );
    gsreg_Set( SCE_GS_ZBUF_1,    SCE_GS_SET_ZBUF     ( s_PostZBuffer, SCE_GS_PSMZ24, 1 ) );
    gsreg_Set( SCE_GS_TEST_1,    SCE_GS_SET_TEST_1   ( 0, 0, 0, 0, 0, 0, 1, SCE_GS_ZALWAYS ) );
    gsreg_Set( SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR_1( s_PostViewL, s_PostViewR-1, s_PostViewT, s_PostViewB-1 ) );
    gsreg_SetClamping( TRUE, TRUE );
    gsreg_Set( SCE_GS_ALPHA_1,   SCE_GS_SET_ALPHA    ( 2, 2, 0, 0, 0 ) );
    gsreg_End();

    s32 Z  = 0;
    s32 X  = SCISSOR_LEFT;
    s32 Y0 = SCISSOR_TOP;
    s32 Y1 = Y0 + VRAM_FRAME_BUFFER_HEIGHT;
    s32 U  = 0;
    s32 V0 = 0;
    s32 V1 = VRAM_FRAME_BUFFER_HEIGHT;
    s32 LClip = SCISSOR_LEFT + s_PostViewL;
    s32 RClip = SCISSOR_LEFT + s_PostViewR;

    // how many registers will we need?
    s32 i;
    s32 NRegs = 3;
    for( i = 0; i < nColumns; i++ )
    {
        if ( X >= RClip )
            break;
        if ( (X+ColumnWidth) > LClip )
            NRegs += 4;
        X += ColumnWidth;
    }
    X = SCISSOR_LEFT;

    // render the strips
    gsreg_Begin( NRegs );
    gsreg_Set( SCE_GS_PRIM,      SCE_GS_SET_PRIM     ( SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,     SCE_GS_SET_RGBAQ    ( 0x80, 0x80, 0x80, 0x80, 0x3F800000 ) );
    for( i = 0; i < nColumns; i++ )
    {
        s32 X0 = X;
        s32 X1 = X0 + ColumnWidth;
        s32 U0 = U;
        s32 U1 = U0 + ColumnWidth;

        if ( X0 >= RClip )
            break;

        if ( X1 > LClip )
        {
            gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV ( (U0 << 4) + 8,  (V0 << 4) + 8 ) );
            gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X0 << 4),      (Y0 << 4), Z  ) );
            gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV ( (U1 << 4) + 8,  (V1 << 4) + 8 ) );
            gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X1 << 4),      (Y1 << 4), Z  ) );
        }
        
        X += ColumnWidth;
        U += ColumnWidth;
    }

    // reset the frame register
    gsreg_Set( SCE_GS_FRAME_1,   SCE_GS_SET_FRAME_1  ( s_PostBackBuffer, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMCT32, 0x00000000 ) );
    gsreg_End();
}

//=============================================================================

static
void ps2_RenderMipSprites( void )
{
    CONTEXT( "ps2_RenderMipSprites" );
    
    // we want to z-buffer the filter in for a nice depth-of-field type thing (plus
    // it's really the distance sparkles that need to be anti-aliased!)
    gsreg_Begin( 1 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    gsreg_End();

    // where should we set the z at?
    s32 Z;
    if ( s_Post.MipFn[s_Post.MipPaletteIndex] == render::FALLOFF_CONSTANT )
    {
        const view* pView = eng_GetView();

        // clamp mip to the near z plane
        f32 MipZ = MAX( s_Post.MipParam1[s_Post.MipPaletteIndex], s_PostNearZ+1.0f );
        MipZ     = MIN( s_Post.MipParam1[s_Post.MipPaletteIndex], s_PostFarZ -1.0f );

        // transform the mip z into screen space
        vector4 MipVec = pView->GetV2S() * vector4(0.0f,0.0f,MipZ,1.0f);
        MipVec.GetZ() /= MipVec.GetW();
        Z = (s32)(MipVec.GetZ()*16.0f);
    }
    else
    {
        Z = 0x0000ffff;
    }

    // now render that 4 times with the specified offset and color
    screen_copy SC;
    SC.OrigFBP    = s_PostMip[s_Post.MipCount[s_Post.MipPaletteIndex]-1];
    SC.NewFBP     = s_PostBackBuffer;
    SC.Mask       = 0xff000000;
    SC.OrigWidth  = (VRAM_FRAME_BUFFER_WIDTH>>s_Post.MipCount[s_Post.MipPaletteIndex]);
    SC.OrigHeight = (VRAM_FRAME_BUFFER_HEIGHT>>s_Post.MipCount[s_Post.MipPaletteIndex]);
    SC.NewWidth   = VRAM_FRAME_BUFFER_WIDTH;
    SC.NewHeight  = VRAM_FRAME_BUFFER_HEIGHT;
    SC.OrigL      = (s_PostViewL>>s_Post.MipCount[s_Post.MipPaletteIndex]);
    SC.OrigR      = (s_PostViewR>>s_Post.MipCount[s_Post.MipPaletteIndex]);
    SC.OrigT      = (s_PostViewT>>s_Post.MipCount[s_Post.MipPaletteIndex]);
    SC.OrigB      = (s_PostViewB>>s_Post.MipCount[s_Post.MipPaletteIndex]);
    SC.NewL       = s_PostViewL;
    SC.NewR       = s_PostViewR;
    SC.NewT       = s_PostViewT;
    SC.NewB       = s_PostViewB;
    SC.VertColor  = s_Post.MipColor[s_Post.MipPaletteIndex];
    SC.AlphaBlend = (u64)(ALPHA_BLEND_MODE(C_SRC,C_DST,A_DST,C_DST));

    if ( s_Post.MipOffset[s_Post.MipPaletteIndex] == 0.0f )
    {
        SC.XOffset = 0.0f;
        SC.YOffset = 0.0f;
        SC.ZOffset = Z;
        SC.VertColor.A <<= 2;
        ps2_ScreenCopy( SC );
    }
    else
    {
        for ( s32 i = 0; i < 4; i++ )
        {
            static s32 OffsetX[4] = { -1,  1, 1, -1 };
            static s32 OffsetY[4] = { -1, -1, 1,  1 };

            SC.XOffset = OffsetX[i] * s_Post.MipOffset[s_Post.MipPaletteIndex];
            SC.YOffset = OffsetY[i] * s_Post.MipOffset[s_Post.MipPaletteIndex];
            SC.ZOffset = Z;
        
            ps2_ScreenCopy( SC );
        }
    }
}

//=============================================================================

static
void ps2_MipFilter( void )
{
    CONTEXT( "ps2_MipFilter" );

    ASSERT( s_Post.MipCount[s_Post.MipPaletteIndex] <= MAX_POST_MIPS );

    // set the registers that are common to all of our filter passes
    gsreg_Begin( 3 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_SetZBufferUpdate(FALSE);
    gsreg_End();

    // filter the screen down a few times
    ps2_BuildScreenMips(MAX_POST_MIPS, FALSE);

    gsreg_Begin( 1 );
    gsreg_SetScissor( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );
    gsreg_End();

    // upload the mip palette, which is really an alpha re-map palette
    if ( s_Post.MipFn[s_Post.MipPaletteIndex] == render::FALLOFF_CUSTOM )
        ps2_SetMipTexture( s_pMipTexture );
    else
        ps2_SetMipTexture();

    // now remap the alpha's into the back buffer
    ps2_RemapAlpha();

    // now we render the mip multiple times with a dest alpha blend
    ps2_RenderMipSprites();
}

//=============================================================================

static
void ps2_CreateFalloffPalette( u8* pPalette, render::hgeom_inst Fn, xcolor Color, f32 Param1, f32 Param2 )
{
    CONTEXT( "ps2_CreateFalloffPalette" );

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
                f32 D     = (Z-s_PostNearZ)/(FarMinusNear);
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
                f32 D     = (Z-s_PostNearZ)/(FarMinusNear);
                D        *= Param1;
                D         = D*D;
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
void ps2_CreateFogPalette( render::post_falloff_fn Fn, xcolor Color, f32 Param1, f32 Param2 )
{
    CONTEXT( "ps2_CreateFogPalette" );

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

    ps2_CreateFalloffPalette( s_FogPalette[s_Post.FogPaletteIndex], Fn, Color, Param1, Param2 );
}

//=============================================================================

static
void ps2_CreateMipPalette( render::post_falloff_fn Fn, xcolor Color, f32 Param1, f32 Param2, s32 PaletteIndex )
{
    CONTEXT( "ps2_CreateMipPalette" );

    f32 NearZ, FarZ;
    const view* pView = eng_GetView();
    pView->GetZLimits( NearZ, FarZ );

    // is it necessary to rebuild the palette?
    if ( (s_Post.MipFn[PaletteIndex]     == Fn    ) &&
         (s_Post.MipParam1[PaletteIndex] == Param1) &&
         (s_Post.MipParam2[PaletteIndex] == Param2) &&
         (s_Post.MipColor[PaletteIndex]  == Color ) &&
         (s_PostNearZ      == NearZ ) &&
         (s_PostFarZ       == FarZ  ) )
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

    ps2_CreateFalloffPalette( s_MipPalette[PaletteIndex], Fn, Color, Param1, Param2 );
}

//=============================================================================

static
void ps2_NoiseFilter( void )
{
    // activate the bitmap
    vram_Activate( s_NoiseBitmap );

    // set up the frame buffer and texture
    gsreg_Begin( 7 );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( s_PostBackBuffer, VRAM_FRAME_BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0x00000000 ) );
    gsreg_SetScissor( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );
    //gsreg_Set( SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( C_SRC, C_DST, A_SRC, C_DST, 128 ) );
    gsreg_Set( SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( C_DST, C_ZERO, A_SRC, C_ZERO, 128 ) );
    gsreg_SetClamping( FALSE );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_End();

    s32 XOffset = x_rand() % kNoiseMapW;
    s32 YOffset = x_rand() % kNoiseMapH;

    // Copy the screen in vertical strips (this will reduce texture/frame cache misses, and
    // give us a much nicer fill rate), using page-width columns is sufficient when bilinear is
    // off, but we need half page-width columns when bilinear is on.
    s32 nColumns = VRAM_FRAME_BUFFER_WIDTH / 64;
    s32 DW       = (VRAM_FRAME_BUFFER_WIDTH / nColumns) << 4;
    s32 LClip    = ((SCISSOR_LEFT+s_PostViewL)<<4) - (1<<3);
    s32 RClip    = ((SCISSOR_LEFT+s_PostViewR)<<4) - (1<<3);
    s32 X0       = (SCISSOR_LEFT<<4) - (1<<3);
    s32 Y0       = (SCISSOR_TOP<<4)  - (1<<3);
    s32 X1       = X0 + DW;
    s32 Y1       = Y0 + (VRAM_FRAME_BUFFER_HEIGHT<<4);
    s32 U0       = XOffset<<4;
    s32 U1       = U0 + (64<<4);
    s32 V0       = YOffset<<4;
    s32 V1       = V0 + (VRAM_FRAME_BUFFER_HEIGHT<<4);

    // how many registers will we need?
    s32 i;
    s32 NRegs = 2;
    for ( i = 0; i < nColumns; i++ )
    {
        if ( X0 >= RClip )
            break;
        if ( X1 > LClip )
            NRegs += 4;
        X0 += DW;
        X1 += DW;
    }
    X0 = (SCISSOR_LEFT<<4) - (1<<3);
    X1 = X0 + DW;

    // render the strips
    gsreg_Begin( NRegs );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( s_Post.NoiseColor.R,
                                               s_Post.NoiseColor.G,
                                               s_Post.NoiseColor.B,
                                               s_Post.NoiseColor.A,
                                               0x3f800000 ) );
    gsreg_Set( SCE_GS_PRIM,  SCE_GS_SET_PRIM( GIF_PRIM_SPRITE, 1, 1, 0, 1, 0, 1, 0, 0 ) );
    for ( i = 0; i < nColumns; i++ )
    {
        if ( X0 >= RClip )
            break;

        if ( X1 > LClip )
        {
            gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV  ( U0, V0    ) );
            gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ2( X0, Y0, 0 ) );
            gsreg_Set( SCE_GS_UV,   SCE_GS_SET_UV  ( U1, V1    ) );
            gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ2( X1, Y1, 0 ) );
        }

        X0 += DW;
        X1 += DW;
    }

    gsreg_End();
}

//=============================================================================

static
void ps2_ScreenFade( void )
{

    // set up the frame buffer and blend settings
    gsreg_Begin( 5 );
    gsreg_Set( SCE_GS_TEXFLUSH, 0 );
    gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( s_PostBackBuffer, VRAM_FRAME_BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0x00000000 ) );
    gsreg_SetScissor( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );
    gsreg_Set( SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( C_SRC, C_DST, A_SRC, C_DST, 0x80 ) );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_End();

    // render it in vertical strips
    s32 nColumns = VRAM_FRAME_BUFFER_WIDTH / 64;
    s32 DW       = (VRAM_FRAME_BUFFER_WIDTH / nColumns) << 4;
    s32 LClip    = ((SCISSOR_LEFT+s_PostViewL)<<4) - (1<<3);
    s32 RClip    = ((SCISSOR_LEFT+s_PostViewR)<<4) - (1<<3);
    s32 X0       = (SCISSOR_LEFT<<4) - (1<<3);
    s32 Y0       = (SCISSOR_TOP<<4)  - (1<<3);
    s32 X1       = X0 + DW;
    s32 Y1       = Y0 + (VRAM_FRAME_BUFFER_HEIGHT<<4);

    // how many registers will we need?
    s32 i;
    s32 NRegs = 2;
    for ( i = 0; i < nColumns; i++ )
    {
        if ( X0 >= RClip )
            break;
        if ( X1 > LClip )
            NRegs += 2;
        X0 += DW;
        X1 += DW;
    }
    X0 = (SCISSOR_LEFT<<4) - (1<<3);
    X1 = X0 + DW;

    // render the strips
    gsreg_Begin( NRegs );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( s_Post.FadeColor.R,
        s_Post.FadeColor.G,
        s_Post.FadeColor.B,
        (s_Post.FadeColor.A==255) ? 128 : (s_Post.FadeColor.A>>1),
        0x3f800000 ) );
    gsreg_Set( SCE_GS_PRIM,  SCE_GS_SET_PRIM( GIF_PRIM_SPRITE, 1, 0, 0, 1, 0, 0, 0, 0 ) );
    for ( i = 0; i < nColumns; i++ )
    {
        if ( X0 >= RClip )
            break;

        if ( X1 > LClip )
        {
            gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ2( X0, Y0, 0 ) );
            gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ2( X1, Y1, 0 ) );
        }

        X0 += DW;
        X1 += DW;
    }
    gsreg_End();
}

//=============================================================================
//=============================================================================
// Functions that are exposed to the main render file
//=============================================================================
//=============================================================================

static
void platform_InitPostEffects( void )
{
    // register the fog and mip cluts
    s32 i;
    for( i = 0; i < MAX_PALETTES; i++ )
    {
        ASSERT( s_hFogClutHandle[i] == -1 );
        ASSERT( s_hMipClutHandle[i] == -1 );
        s_hFogClutHandle[i] = vram_RegisterClut( s_FogPalette[i], 256 );
        s_hMipClutHandle[i] = vram_RegisterClut( s_MipPalette[i], 256 );
        ASSERT( s_hFogClutHandle[i] != -1 );
        ASSERT( s_hMipClutHandle[i] != -1 );
    }

    // set up a noise bitmap and register it
    s_NoiseBitmap.Setup( xbitmap::FMT_P4_ABGR_8888, kNoiseMapW, kNoiseMapH, FALSE, (byte*)s_NoiseMap, FALSE, (byte*)s_NoisePalette );
    vram_Register( s_NoiseBitmap );

    // clear all current effects
    x_memset( &s_Post, 0, sizeof(post_effect_params) );
}

//=============================================================================

static
void platform_KillPostEffects( void )
{
    // unregister the fog and mip cluts
    s32 i;
    for( i = 0; i < MAX_PALETTES; i++ )
    {
        ASSERT( s_hFogClutHandle[i] != -1 );
        ASSERT( s_hMipClutHandle[i] != -1 );
        vram_UnRegisterClut( s_hFogClutHandle[i] );
        vram_UnRegisterClut( s_hMipClutHandle[i] );
        s_hFogClutHandle[i] = -1;
        s_hMipClutHandle[i] = -1;
    }

    // unregister the noise bitmap
    vram_Unregister( s_NoiseBitmap );
}

//=============================================================================

static
void platform_SetCustomFogPalette( const texture::handle& Texture, xbool ImmediateSwitch, s32 PaletteIndex )
{
    texture* pTexture = Texture.GetPointer();
    if ( !pTexture )
        return;

    ASSERT( (PaletteIndex >= 0) && (PaletteIndex < 2) );
    if ( ImmediateSwitch || !s_bFogValid[PaletteIndex] )
    {
        // Just do a memcpy. We know we're 16-byte aligned, so use 128s to speed things up
        // QUESTION: If this is slow, would it be better to work in scratchpad?!? It's only
        // 1k of data, so maybe not a big deal...
        u128* pDst = (u128*)s_FogPalette[PaletteIndex];
        u128* pSrc = (u128*)pTexture->m_Bitmap.GetClutData();
        for ( s32 i = 0; i < 1024/16; i++ )
        {
            *pDst++ = *pSrc++;
        }
    }
    else
    {
        ///////////////////////////////////////////////////////////////////////
        // Do a blend between palettes
        //
        // Since the following assembly can be a bit confusing, let me explain.
        // The motivation is to use the ps2's fancy multimedia instructions
        // to speed up blend process and operate on multiple colors at one
        // time. Additionally, we'd like to avoid branches, as they just tend
        // to stall things. What we're going for is this:
        //      if ( src > dst )
        //          dst++;
        //      else if ( src < dst )
        //          dst--;
        //
        // Of course, we're dealing with a lot of colors, so if you don't mind,
        // could you operate on sixteen bytes (or four colors) at a time?
        // Sounds easy, right? Well, maybe if we had a more powerful
        // instruction set. And those conditionals look nasty. Luckily we have
        // greater than and equal comparison instructions we can combine with
        // some bit tricks to get a delta of +1, -1, or 0. But then of course
        // the instructions are designed to work with signed bytes and we have
        // unsigned bytes. Never fear...with a few more instructions we can
        // handle that case. After all is said and done, follow along with this
        // table, and you'll see what the assembly is doing. Note that with
        // the comparison instructions, you get -1 if the test passes (all bits
        // set) and 0 if the test fails (see the EE documentation).
        //
        // EQU  = (ISRC ==  IDST)
        // GT   = (ISRC >   IDST)
        // SLTZ = (0    >   ISRC)
        // DLTZ = (0    >   IDST)
        // SDIF = (SLTZ XOR DLTZ)
        // GXOR = (SDIF XOR GT  )
        // DIFF = (EQU  -   GXOR)
        // NOTG = (GXOR NOR GXOR)
        // DELT = (DIFF XOR NOTG)
        // ODST = (IDST +   DELT)
        //
        // ISRC IDST |  EQU   GT SLTZ DLTZ SDIF GXOR DIFF NOTG DELT | ODST
        //-----------+----------------------------------------------+-----
        // 0x00 0x00 |   -1                            -1   -1    0 | 0x00
        // 0x00 0x40 |                                      -1   -1 | 0x3F
        // 0x00 0x60 |                                      -1   -1 | 0x5F
        // 0x00 0x80 |        -1        -1   -1             -1   -1 | 0x7F
        // 0x00 0xa0 |        -1        -1   -1             -1   -1 | 0x9F
        //-----------+----------------------------------------------+-----
        // 0x40 0x00 |        -1                  -1    1         1 | 0x01
        // 0x40 0x40 |   -1                            -1   -1    0 | 0x40
        // 0x40 0x60 |                                      -1   -1 | 0x5F
        // 0x40 0x80 |        -1        -1   -1             -1   -1 | 0x7F
        // 0x40 0xa0 |        -1        -1   -1             -1   -1 | 0x9F
        //-----------+----------------------------------------------+-----
        // 0x60 0x00 |        -1                  -1    1         1 | 0x01
        // 0x60 0x40 |        -1                  -1    1         1 | 0x41
        // 0x60 0x60 |   -1                            -1   -1    0 | 0x60
        // 0x60 0x80 |        -1        -1   -1             -1   -1 | 0x7F
        // 0x60 0xa0 |        -1        -1   -1             -1   -1 | 0x9F
        //-----------+----------------------------------------------+-----
        // 0x80 0x00 |             -1        -1   -1    1         1 | 0x01
        // 0x80 0x40 |             -1        -1   -1    1         1 | 0x41
        // 0x80 0x60 |             -1        -1   -1    1         1 | 0x61
        // 0x80 0x80 |   -1        -1   -1             -1   -1    0 | 0x80
        // 0x80 0xa0 |             -1   -1                  -1   -1 | 0x9F
        //-----------+----------------------------------------------+-----
        // 0xa0 0x00 |             -1        -1   -1    1         1 | 0x01
        // 0xa0 0x40 |             -1        -1   -1    1         1 | 0x41
        // 0xa0 0x60 |             -1        -1   -1    1         1 | 0x61
        // 0xa0 0x80 |        -1   -1   -1        -1    1         1 | 0x81
        // 0xa0 0xa0 |   -1        -1   -1             -1   -1    0 | 0xa0
        ///////////////////////////////////////////////////////////////////////
              u128* pDst = (u128*)s_FogPalette[PaletteIndex];
        const u128* pSrc = (u128*)pTexture->m_Bitmap.GetClutData();
        for ( s32 i = 0; i < 1024/16; i++ )
        {
            u128 ISRC, IDST, EQU, GT, SLTZ, DLTZ, SDIF, GXOR, DIFF, NOTG, DELT, ODST;

            ISRC = *pSrc;
            IDST = *pDst;
            asm( "pceqb EQU,  ISRC, IDST" : "=r EQU"  (EQU)  : "r ISRC" (ISRC), "r IDST" (IDST) );
            asm( "pcgtb GT,   ISRC, IDST" : "=r GT"   (GT)   : "r ISRC" (ISRC), "r IDST" (IDST) );
            asm( "pcgtb SLTZ, $0,   ISRC" : "=r SLTZ" (SLTZ) : "r ISRC" (ISRC)                  );
            asm( "pcgtb DLTZ, $0,   IDST" : "=r DLTZ" (DLTZ) : "r IDST" (IDST)                  );
            asm( "pxor  SDIF, SLTZ, DLTZ" : "=r SDIF" (SDIF) : "r SLTZ" (SLTZ), "r DLTZ" (DLTZ) );
            asm( "pxor  GXOR, SDIF, GT"   : "=r GXOR" (GXOR) : "r SDIF" (SDIF), "r GT"   (GT)   );
            asm( "psubb DIFF, EQU,  GXOR" : "=r DIFF" (DIFF) : "r EQU"  (EQU),  "r GXOR" (GXOR) );
            asm( "pnor  NOTG, GXOR, GXOR" : "=r NOTG" (NOTG) : "r GXOR" (GXOR)                  );
            asm( "pxor  DELT, DIFF, NOTG" : "=r DELT" (DELT) : "r DIFF" (DIFF), "r NOTG" (NOTG) );
            asm( "paddb ODST, IDST, DELT" : "=r ODST" (ODST) : "r IDST" (IDST), "r DELT" (DELT) );
            *pDst = ODST;

            pSrc++;
            pDst++;
        }
    }

    // now we have a valid fog
    s_bFogValid[PaletteIndex] = TRUE;
}

//=============================================================================

static
xcolor platform_GetFogValue( const vector3& WorldPos, s32 PaletteIndex )
{
    const matrix4& W2S = eng_GetView()->GetW2S();
    
    // The post-effect works by mapping bits 8..16 of the screen z value into
    // our custom palette. We'll do the same thing to look up a fog color.
    vector4 ScreenPos(WorldPos);
    ScreenPos = W2S * ScreenPos;
    if ( x_abs( ScreenPos.GetW() ) < 0.001f )
        return xcolor(255,255,255,0);
    
    ScreenPos.GetZ() = 16.0f*(ScreenPos.GetZ() / ScreenPos.GetW());
    if ( !s_bFogValid[PaletteIndex]    ||
         (ScreenPos.GetZ() > 65535.0f) ||
         (ScreenPos.GetZ() < 0.0f) )
    {
        return xcolor(255,255,255,0);
    }

    s32 ClutIndex    = (s32)ScreenPos.GetZ();
    ClutIndex        = ((ClutIndex & 0x0000ff00)>>8);
    ClutIndex        = ((ClutIndex&0x08)<<1) | ((ClutIndex&0x10)>>1) | ((ClutIndex&0xE7));
    xcolor ClutColor = xcolor( s_FogPalette[PaletteIndex][ClutIndex*4+0],
                               s_FogPalette[PaletteIndex][ClutIndex*4+1],
                               s_FogPalette[PaletteIndex][ClutIndex*4+2],
                               s_FogPalette[PaletteIndex][ClutIndex*4+3] );
    xcolor RetColor( (ClutColor.R==0x80) ? 255 : (ClutColor.R<<1),
                     (ClutColor.G==0x80) ? 255 : (ClutColor.G<<1),
                     (ClutColor.B==0x80) ? 255 : (ClutColor.B<<1),
                     (ClutColor.A==0x80) ? 255 : (ClutColor.A<<1) );
    return RetColor;
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
    s_Post.NScreenWarps    = 0;

    s_pMipTexture          = NULL;
}

//=============================================================================

static
void platform_AddScreenWarp( const vector3& WorldPos, f32 Radius, f32 WarpAmount )
{
    if( s_Post.NScreenWarps < MAX_SCREEN_WARPS )
    {
        s_Post.ScreenWarpPos[s_Post.NScreenWarps]    = WorldPos;
        s_Post.ScreenWarpRadius[s_Post.NScreenWarps] = Radius;
        s_Post.ScreenWarpAmount[s_Post.NScreenWarps] = WarpAmount;
        s_Post.NScreenWarps++;
    }
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
    ps2_CreateFogPalette( Fn, Color, Param1, Param2 );
    s_Post.DoZFogFn        = TRUE;
    s_Post.FogPaletteIndex = 2;
}

//=============================================================================

static
void platform_ZFogFilter( render::post_falloff_fn Fn, s32 PaletteIndex )
{
    if ( s_Post.Override )
        return;

    ASSERT( (PaletteIndex >= 0) && (PaletteIndex < 2) );
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
    ASSERT( (PaletteIndex >= 0) && (PaletteIndex < MAX_PALETTES) );

    // adjust our mip palette (will also save the mip parameters)
    ps2_CreateMipPalette( Fn, Color, Param1, Param2, PaletteIndex );
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
    ASSERT( (PaletteIndex >= 0) && (PaletteIndex < MAX_PALETTES) );

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
    s_Post.DoScreenFade = TRUE;
    s_Post.FadeColor    = Color;
}

//=============================================================================

static
void platform_EndPostEffects( void )
{
    CONTEXT( "platform_EndPostEffects" );

    ASSERT( s_InPost );
    s_InPost = FALSE;

    // any post-effect will likely hammer vram
    vram_FlushBank(0);
    vram_FlushBank(1);
    vram_FlushBank(2);
    vram_FlushBank(3);

    // get common engine/screen information
    const view* pView = eng_GetView();
    pView->GetViewport( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );
    s_PostBackBuffer  = eng_GetFrameBufferAddr(0) / 2048;
    s_PostFrontBuffer = eng_GetFrameBufferAddr(1) / 2048;
    s_PostZBuffer     = VRAM_ZBUFFER_START / 8192;
    s_PostMip[0]      = VRAM_FREE_MEMORY_START / 8192;
    s_PostMip[1]      = s_PostMip[0] + ((VRAM_FRAME_BUFFER_WIDTH>>1) *
                                        (VRAM_FRAME_BUFFER_HEIGHT>>1) *
                                        (VRAM_FRAME_BUFFER_BPP)) / 8192;
    s_PostMip[2]      = s_PostMip[1] + ((VRAM_FRAME_BUFFER_WIDTH>>2) *
                                        (VRAM_FRAME_BUFFER_HEIGHT>>2) *
                                        (VRAM_FRAME_BUFFER_BPP)) / 8192;

    // both fog and screen mipping need a valid "depth" alpha in the
    // upper 8-bits, so copy bits 8..15 of the zbuffer into the front buffer
    if ( s_Post.DoMipFn     ||
         s_Post.DoMipCustom ||
         s_Post.DoZFogFn    ||
         s_Post.DoZFogCustom )
    {
        s32   XRes, YRes;
        xbool PALMode;

        eng_GetRes( XRes, YRes );
        eng_GetPALMode( PALMode );
        if( PALMode )
        {
            // The texture range is too limited to do this function in
            // one go at the higher resolution, so in PAL we need to do
            // this in two steps
            ps2_CopyRG2BA( XRes, YRes/2, s_PostZBuffer );
            ps2_CopyRG2BA( XRes, YRes/2, s_PostZBuffer + (VRAM_FRAME_BUFFER_WIDTH*VRAM_FRAME_BUFFER_HEIGHT*2/8192) );
        }
        else
        {
            ps2_CopyRG2BA( XRes, YRes, s_PostZBuffer );
        }
    }

    // handle fog
    if ( s_Post.DoZFogCustom || s_Post.DoZFogFn )
    {
        ps2_ZFogFilter();
    }
    
    // handle the mip filter
    if ( s_Post.DoMipCustom || s_Post.DoMipFn )
        ps2_MipFilter();

    // handle motion-blur. this is a stand-alone blur that will not effect vram
    // or the contents of any alpha or z-buffers.
    if ( s_Post.DoMotionBlur )
        ps2_MotionBlur();

    // handle self-illum
    if ( s_Post.DoSelfIllumGlow )
        ps2_ApplySelfIllumGlows();

    // radial blurs and post-mults will want a copy of the back-buffer to work with
    if ( s_Post.DoMultScreen || s_Post.DoRadialBlur || s_Post.NScreenWarps )
        ps2_CopyBackBuffer();

    // handle the multscreen filter
    if ( s_Post.DoMultScreen )
        ps2_MultScreen();

    // handle the radial blur filter
    if ( s_Post.DoRadialBlur )
        ps2_RadialBlur();

    // handle the screen warps
    if( s_Post.NScreenWarps )
        ps2_ScreenWarps();

    // handle the noise filter
    if ( s_Post.DoNoise )
        ps2_NoiseFilter();

    // handle the fade filter
    if ( s_Post.DoScreenFade )
        ps2_ScreenFade();

    // reset the things we've most likely screwed up along the way
    gsreg_Begin( 3 );
    gsreg_SetClamping( FALSE, FALSE );
    gsreg_SetFBMASK(0); // will also reset the frame register
    gsreg_SetScissor( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );
    gsreg_End();
}

//=============================================================================

static
void platform_BeginDistortion( void )
{
    // pretend we are in a post-effect so we can use some common functions
    // get common engine/screen information
    const view* pView = eng_GetView();
    pView->GetViewport( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );
    s_PostBackBuffer  = eng_GetFrameBufferAddr(0) / 2048;
    s_PostFrontBuffer = eng_GetFrameBufferAddr(1) / 2048;
    s_PostZBuffer     = VRAM_ZBUFFER_START / 8192;
    s_PostMip[0]      = vram_GetBankAddr(0) / 32;
    s_PostMip[1]      = s_PostMip[0] + ((VRAM_FRAME_BUFFER_WIDTH>>1) *
                                        (VRAM_FRAME_BUFFER_HEIGHT>>1) *
                                        (VRAM_FRAME_BUFFER_BPP)) / 8192;
    s_PostMip[2]      = s_PostMip[1] + ((VRAM_FRAME_BUFFER_WIDTH>>2) *
                                        (VRAM_FRAME_BUFFER_HEIGHT>>2) *
                                        (VRAM_FRAME_BUFFER_BPP)) / 8192;

    // the diffuse bank will be hosed after this
    vram_FlushBank(0);

    // set the registers that are common to all of our filter passes
    gsreg_Begin( 3 );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
    gsreg_Set( SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
    gsreg_SetZBufferUpdate(FALSE);
    gsreg_End();

    // make a down-sized copy of the screen
    ps2_BuildScreenMips( 1, FALSE );

    // reset the things we've most likely screwed up along the way
    gsreg_Begin( 3 );
    gsreg_SetClamping( FALSE, FALSE );
    gsreg_SetFBMASK(0); // will also reset the frame register
    gsreg_SetScissor( s_PostViewL, s_PostViewT, s_PostViewR, s_PostViewB );
    gsreg_End();
}

//=============================================================================

static
void platform_EndDistortion( void )
{
}

//=============================================================================

