//==============================================================================
//  vu1.hpp
//
//  Copyright (c) 2003-2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This class provides an interface for rendering geometry with vu1 microcode.
//==============================================================================

#ifndef VU1_HPP
#define VU1_HPP

#include "Entropy.hpp"
#include "../../Material_Prefs.hpp"
#include "../../Texture.hpp"
#include "mcode/include.vu"
#include "../../Render/Render.hpp"
#include "PS2/ps2_misc.hpp"

#include "EEStruct.h"

class vu1_interface
{
public:
    //=========================================================================
    // Types which are used to communicate with vu1
    //=========================================================================

    enum envmap_type
    {
        ENV_POSITIONS = 0,
        ENV_VIEWSPACE,
        ENV_WORLDSPACE
    };

    //=========================================================================
    // Init/kill routines
    //=========================================================================
    vu1_interface( void )   X_SECTION( init );

    //=========================================================================
    // Functions for entering into a rendering state with vu1. The general
    // idea is that you will begin a type of rendering and then end it.
    // Internally, the End call is what will trigger all of the added instances
    // to be rendered.
    //=========================================================================
    void Begin                  ( void ) X_SECTION( render_infrequent );
    void End                    ( void ) X_SECTION( render_infrequent );
    void BeginShadows           ( void ) X_SECTION( render_infrequent );
    void EndShadows             ( void ) X_SECTION( render_infrequent );

    //=========================================================================
    // Functions for rendering raw data. Normal geometry should not go through
    // this and will o through the instance-based stuff instead, but some
    //custom objects such as cloth, decals, or particles may go through these.
    //=========================================================================
    void RenderRawStrips        ( s32            nVerts,
                                  const matrix4& L2W,
                                  const vector4* pPos,
                                  const s16*     pUV,
                                  const u32*     pColor )   X_SECTION( render_raw );
    void Render3dSprites        ( s32            nSprites,
                                  f32            UniScale,
                                  const matrix4* pL2W,
                                  const vector4* pPositions,
                                  const vector2* pRotScales,
                                  const u32*     pColors )  X_SECTION( render_raw );
    void RenderVelocitySprites  ( s32            nSprites,
                                  f32            UniScale,
                                  const matrix4* pL2W,
                                  const matrix4* pVelMatrix,
                                  const vector4* pPositions,
                                  const vector4* pVelocities,
                                  const u32*     pColors )  X_SECTION( render_raw );
    void RenderHeatHazeSprites  ( s32            nSprites,
                                  f32            UniScale,
                                  const matrix4* pL2W,
                                  const vector4* pPositions,
                                  const vector2* pRotScales,
                                  const u32*     pColors )  X_SECTION( render_raw );

    //=========================================================================
    // Functions for rendering instanced rigid objects.
    //=========================================================================

    void BeginInstance          ( const vector4* pVert,
                                  const s16*     pUV,
                                  const s8*      pNormal,
                                  s32            nVerts )               X_SECTION( render_deferred );
    void EndInstance            ( void )                                X_SECTION( render_deferred );
    void AddInstance            ( const u16*     pRGB,
                                  const matrix4& pL2W,
                                  u8             UOffset,
                                  u8             VOffset,
                                  u32            Flags = 0,
                                  s32            Alpha = 255,
                                  void*          pLightInfo = NULL )    X_SECTION( render_deferred );

    //=========================================================================
    // Functions for rendering skinned objects.
    //=========================================================================
    void SetSkinFlags                   ( u32               Flags,
                                          u8                Alpha       ) X_SECTION( render_deferred );
    void SetSkinUVOffset                ( u8                UOffset,
                                          u8                VOffset     ) X_SECTION( render_deferred );
    void SetSkinLightInfo               ( void*             pLightInfo  ) X_SECTION( render_deferred );
    void UploadSkinMatrix               ( const matrix4&    Mat,
                                          s16               CacheIndex  ) X_SECTION( render_deferred );
    void BeginSkinRenderNormal          ( void                          ) X_SECTION( render_deferred );
    void RenderSkinVertsNormalTwoBones  ( s32               NVerts,
                                          const s16*        pUV,
                                          const s16*        pPos,
                                          const u8*         pBoneIndices,
                                          const u8*         pNormals    ) X_SECTION( render_deferred );
    void RenderSkinVertsNormalOneBone   ( s32               NVerts,
                                          const s16*        pUV,
                                          const s16*        pPos,
                                          const u8*         pBoneIndices,
                                          const u8*         pNormals    ) X_SECTION( render_deferred );
    void EndSkinRenderNormal            ( void                          ) X_SECTION( render_deferred );
    void BeginSkinRenderClipped         ( void                          ) X_SECTION( render_deferred );
    void RenderSkinVertsClipped         ( s32               NVerts,
                                          const s16*        pUV,
                                          const s16*        pPos,
                                          const u8*         pBoneIndices,
                                          const u8*         pNormals    ) const X_SECTION( render_deferred );
    void EndSkinRenderClipped           ( void                          ) X_SECTION( render_deferred );

    //=========================================================================
    // Functions for feeding shadow information into vu1. This information is
    // used for creating both the small shadow textures per character and for
    // the "screen shadow map" intermediate texture.
    //=========================================================================
    void ClearShadowCache               ( void )    X_SECTION( render_infrequent );
    void SetShadowCastMaterial          ( void )    X_SECTION( render_infrequent );
    void SetShadowReceiveMaterial       ( void )    X_SECTION( render_infrequent );
    void ClearShadowProjectors          ( void )    X_SECTION( render_infrequent );
    void SetShadowCastInfo              ( s32            ProjectorIndex,
                                          u64            Frame,
                                          const matrix4& ProjMat ) X_SECTION( render_add_shadow );
    void SetShadowReceiveInfo           ( s32            ProjectorIndex,
                                          u64            Tex0,
                                          const matrix4& ProjMat ) X_SECTION( render_add_shadow );

    //=========================================================================
    // Functions for creating shadow textures (you should also use the standard
    // SetSkinFlags from above).
    //=========================================================================
    void BeginSkinShadow                ( s32               iProj        ) X_SECTION( render_deferred_shadow );
    void UploadSkinShadowMatrix         ( const matrix4&    Mat,
                                          s16               CacheIndex   ) X_SECTION( render_deferred_shadow );
    void RenderSkinShadowTwoBones       ( s32               nVerts,
                                          const s16*        pPos,
                                          const u8*         pBoneIndices ) X_SECTION( render_deferred_shadow );
    void RenderSkinShadowOneBone        ( s32               nVerts,
                                          const s16*        pPos,
                                          const u8*         pBoneIndices ) X_SECTION( render_deferred_shadow );
    void EndSkinShadow                  ( void                           ) X_SECTION( render_deferred_shadow );

    //=========================================================================
    // Functions for receiving shadows into the intermediate screen shadow map
    //=========================================================================
    void BeginShadReceiveInstance       ( const vector4* pPos,
                                          const s16*     pUV,
                                          s32            nVerts ) X_SECTION( render_deferred_shadow );
    void AddShadReceiveInstance         ( const matrix4* pL2W,
                                          u8             UOffset,
                                          u8             VOffset,
                                          s32            Flags,
                                          s32            iProj  ) X_SECTION( render_deferred_shadow );
    void EndShadReceiveInstance         ( void                  ) X_SECTION( render_deferred_shadow );

    //=========================================================================
    // Functions for setting up a projected spotlight. Once set, the vu1 code
    // will use these values until told otherwise.
    //=========================================================================
    void SetSpotlightTexture    ( const texture::handle& Texture ) X_SECTION( render_infrequent );
    void SetSpotlightPos        ( const vector3&         Pos     ) X_SECTION( render_infrequent );
    void SetSpotlightMatrix     ( const matrix4&         Matrix  ) X_SECTION( render_infrequent );

    //=========================================================================
    // Functions for setting up a projected shadow. NOTE: Make sure the
    // number of shadow projectors is set each frame so that we don't try to
    // keep rendering the same shadow all the time!
    //=========================================================================
    void SetProjShadowTexture   ( s32             Index,
                                  texture::handle Texture     ) X_SECTION( render_infrequent );
    void SetProjShadowMatrix    ( s32             Index,
                                  const matrix4&  Matrix      ) X_SECTION( render_infrequent );
    void SetNShadowProjectors   ( s32             nProjectors ) X_SECTION( render_infrequent );

    //=========================================================================
    // Functions for activating a material. Generally, you will use
    // SetMaterial() during the normal course of geometry rendering, and you
    // will use the specific material type functions for custom renderers (such
    // as decals or particles).
    //=========================================================================
    void SetDetailTex1          ( u64 Tex1                          ) const     X_SECTION( render_deferred );
    void SetMaterial            ( const material&        Mat        ) const     X_SECTION( render_deferred );
    void SetDistortionMaterial  ( const material*        pMaterial,
                                  const radian3&         NormalRot  ) const     X_SECTION( render_infrequent );
    void SetZPrimeMaterial      ( void                              ) const     X_SECTION( render_infrequent );
    void SetDiffuseMaterial     ( const xbitmap&         Bitmap,
                                  s32                    BlendMode,
                                  xbool                  ZTestOn    ) const     X_SECTION( render_raw );
    void SetGlowMaterial        ( const xbitmap&         Bitmap,
                                  s32                    BlendMode,
                                  xbool                  ZTestOn    ) const     X_SECTION( render_raw );
    void SetEnvMapMaterial      ( const xbitmap&         Bitmap,
                                  s32                    BlendMode,
                                  xbool                  ZTestOn    ) const     X_SECTION( render_raw );
    void SetDistortionMaterial  ( s32                    BlendMode,
                                  xbool                  ZTestOn    ) const     X_SECTION( render_raw );

    //=========================================================================
    // Functions for setting up lighting parameters. You should create a new
    // lighting setup once per object. This will put it into smem, then you
    // can fill in that lighting setup with the other functions. Once it is
    // time to render that object in vu1, we'll dma the lighting information
    // out of smem.
    //=========================================================================
    void  SetFilterColor        ( xcolor         Filter     )                   X_SECTION( render_infrequent );
    void* NewLightingSetup      ( void                      ) const             X_SECTION( render_add );
    void  SetLight              ( void*          pLightInfo,
                                  s32            iLight,
                                  const matrix4& L2W,
                                  const vector3& Pos,
                                  f32            Radius,
                                  xcolor         Col        ) const             X_SECTION( render_add );
    void  SetSkinAmbient        ( void*          pLightInfo,
                                  xcolor         Ambient,
                                  u32            InstFlags  ) const             X_SECTION( render_add );
    void  SetSkinLight          ( void*          pLightInfo,
                                  s32            iLight,
                                  const vector3& Dir,
                                  xcolor         Col,
                                  u32            InstFlags  ) const             X_SECTION( render_add );

public:
    //=========================================================================
    // Internal data structures that will be passed to vu1. These structures
    // are considered PRIVATE. NOTE: THESE STRUCTURES ARE VERY SENSITIVE. ANY
    // CHANGES TO THESE STRUCTURES WILL REQUIRE MATCHING CHANGES TO THE
    // MICROCODE!!!!
    //=========================================================================
    enum { MAX_LIGHTS = 4 };
    enum { SHADOW_CACHE_SIZE = 5 };
    //-------------------------------------------------------------------------
    struct vu1_tex0
    {
        union
        {
            sceGsTex0   TEX0;
            u64         TEXData;
        };
        u64         TEX0Addr;
    };
    //-------------------------------------------------------------------------
    struct vu1_AD
    {
        u64 Data;
        u64 Addr;
    };
    //-------------------------------------------------------------------------
    struct vu1_tex1
    {
        dmatag      DMA;
        vu1_AD      TEX1;
    };
    //-------------------------------------------------------------------------
    struct vu1_clip_plane
    {
        u32 Mask;
        u32 JumpAddr;
        u32 Pad0;
        u32 Pad1;
    };
    //-------------------------------------------------------------------------
    struct vu1_init
    {
        dmatag    DMA;
        u32       VIF1[ 16 ];
        matrix4   W2S;
        u32       Kick[ 4 ];
    };
    //-------------------------------------------------------------------------
    struct vu1_clip_data
    {
        vu1_clip_plane  Planes[VU1_CLIP_NUM_PLANES];
        matrix4         W2C;
        matrix4         C2W;
        matrix4         C2S;
    };
    //-------------------------------------------------------------------------
    struct vu1_const_material
    {
        dmatag      DMA;
        vector4     LightMultiplier;
        vu1_AD      ShadowTex;
        vu1_tex0    SpotlightTex;
        vu1_AD      Clamp;
        vu1_AD      FrameBackAll;
        vu1_AD      FrameBackRGB;
        vu1_AD      FrameBackAlpha;
        vu1_AD      FrameFrontAlpha;
        vu1_AD      IntensityAlpha;
        vu1_AD      SpotlightAlpha;
        giftag      Context1Gif;
        giftag      Context2Gif;
        giftag      Context2GifNoTex;
        vector4     SpotlightPos;
        matrix4     SpotlightMatrix;
        vu1_tex0    ProjShadowTex[render::MAX_SHADOW_PROJECTORS];
        matrix4     ProjShadowMatrix[render::MAX_SHADOW_PROJECTORS];
        giftag      RegLoadGif;
        vector4     Storage;
    };
    //-------------------------------------------------------------------------
    struct vu1_material
    {
        dmatag      DMA;
        u32         Flags;
        f32         Unused1;
        f32         DetailScale;
        u32         PerPolyAlpha;
        vu1_tex0    DiffuseTex;
        vu1_tex0    DecalTex;
        vu1_tex0    EnvTex;
        vu1_tex0    DetailTex;
        vu1_AD      Mips;
        vu1_AD      DiffuseAlpha;
        vu1_AD      EnvIllumAlpha;
        vector4     EnvMatrix0;         // column 0 of texture matrix (col2 hidden in zw)
        vector4     EnvMatrix1;         // column 1 of texture matrix
    };
    //-------------------------------------------------------------------------
    struct vu1_material_init
    {
        dmatag      DMA;
        giftag      RegLoadGif;
        vu1_AD      Texflush;
        vu1_AD      DiffuseTex1;
        vu1_AD      DiffuseMip1;
        vu1_AD      DiffuseMip2;
        vu1_AD      DetailMip1;
        vu1_AD      Clamp1;
        vu1_AD      ZBuf1;
        vu1_AD      ZBuf2;
        vu1_AD      Test;
    };
    //-------------------------------------------------------------------------
    struct vu1_material_set
    {
        vu1_material        Mat;
        vu1_material_init   Init;
    };
    //-------------------------------------------------------------------------
    struct vu1_light
    {
        f32 Pad0, Pad1, X, Y;
        f32 Pad2, Pad3, Z, Radius;
        f32 Pad4, Pad5, R, G;
        f32 Pad6, Pad7, B, Pad8;
    };
    //-------------------------------------------------------------------------
    struct vu1_vector4
    {
        dmatag  DMA;
        u32     X, Y, Z, W;
    };
    //-------------------------------------------------------------------------
    struct vu1_lightinfo
    {
        u32             SkipWrite;
        u32             LightMask[2];
        u32             Unpack;
        vu1_light       Lights[MAX_LIGHTS];
        u32             RestoreMask[2];
        u32             Pad[2];
    };
    //-------------------------------------------------------------------------
    struct vu1_instance
    {
        const matrix4* pL2W;
        const u16*     pRGB;
        u8             UOffset;
        u8             VOffset;
        const byte*    pLightInfo;
        u32            Flags;
        u32            ShadFlags;
        u8             Alpha;
    };
    //-------------------------------------------------------------------------
    struct vu1_shadow_cast
    {
        vu1_AD      AlphaChannel;
        matrix4     ProjectionMatrix;
    };
    //-------------------------------------------------------------------------
    struct vu1_shadow_receive
    {
        vu1_interface::vu1_tex0 ShadowTex;
        matrix4                 ProjectionMatrix;
    };
    //-------------------------------------------------------------------------
    struct vu1_shadow_info
    {
        vu1_shadow_cast*    pShadCast;
        vu1_shadow_receive* pShadReceive;
        u8                  CacheIndex;
    };
    //-------------------------------------------------------------------------
    struct vu1_shadow_material
    {
        dmatag      DMA;
        vector4     Flags;
        giftag      PassOneGIF;
        giftag      PassTwoGIF;
        u32         VIF[4];
        giftag      RegLoadGIF;
    };
    //-------------------------------------------------------------------------
    struct mat_struct
    {
        dmatag  DMARef;
        dmatag  DMACont;
    };
    //-------------------------------------------------------------------------
    struct sprite_init
    {
        dmatag      DMACont;
        matrix4     W2V;
    };
    //-------------------------------------------------------------------------
    struct sprite_init_coeff
    {
        dmatag      DMACont;
        matrix4     W2V;
        dmatag      DMARef;
    };
    //-------------------------------------------------------------------------
    enum
    {
        // microcode to force the gs to an idle state
        MCODE_SYNC = 0,

        // microcode to initialize the system
        MCODE_INIT,

        // microcode to perform generic clipping
        MCODE_CLIPPER,

        // microcode to handle rendering whatever it is we're rendering (shadows,
        // materials, etc.)
        MCODE_MATERIALS,
        MCODE_SHADRECEIVE_PASSES,

        // different types of transform microcode
        MCODE_SKIN_XFORM,
        MCODE_RIGID_XFORM,
        MCODE_SHADCAST_XFORM,
        MCODE_SHADRECEIVE_XFORM,
        MCODE_SPRITE_XFORM,
    };

    //=========================================================================
    // Internal routines for activating the different pieces of microcode.
    // If the microcode needs to be swapped out, it will only swap out that
    // piece and not all 16k. (xform will be the main one and weighs in at
    // approximately 4k, but can go up or down as things change)
    //=========================================================================
    void    LoadMCode           ( u32 MainMemAddr, u32 VUAddr, u32 Size ) X_SECTION( render_deferred );
    void    LoadSyncMCode       ( void )                                  X_SECTION( render_deferred );
    void    LoadInitMCode       ( void )                                  X_SECTION( render_deferred );
    void    LoadClipperMCode    ( void )                                  X_SECTION( render_deferred );
    void    LoadMaterialMCode   ( s32 MCodeType )                         X_SECTION( render_deferred );
    void    LoadXFormMCode      ( s32 MCodeType )                         X_SECTION( render_deferred );

    //=========================================================================
    // Internal helpers for setting up materials
    //=========================================================================
    void SetupRawMat            ( DLPtr(pMaterial, vu1_material),
                                  const xbitmap& Bitmap,
                                  s32            BlendMode       ) const    X_SECTION( render_raw );
    void SendConstMaterial      ( void                           ) const    X_SECTION( render_raw );
    void SetTextureRegisters    ( const xbitmap& Bitmap,
                                  vu1_tex0&      Texture,
                                  xbool          IsDecal = FALSE ) const    X_SECTION( render_raw );
    void DiffuseTexture         ( DLPtr(pMaterial, vu1_material),
                                  const xbitmap& Bitmap          ) const    X_SECTION( render_raw );
    void EnvironmentTexture     ( DLPtr(pMaterial, vu1_material),
                                  const xbitmap* pBitmap         ) const    X_SECTION( render_raw );
    void InitMaterial           ( DLPtr(pMaterial, vu1_material) ) const    X_SECTION( render_raw );

    //=========================================================================
    // Internal helper routines for creating shadows
    //=========================================================================
    s32     AddReceiverToShadowCache    ( s32 iProj )   X_SECTION( render_deferred_shadow );

    //=========================================================================
    // Misc. other internal helpers
    //=========================================================================
    void    Sync                        ( void ) const                              X_SECTION( render_deferred );
    void    SyncDList                   ( void ) const                              X_SECTION( render_deferred );
    void    SyncAll                     ( void ) const                              X_SECTION( render_deferred );
    void    KickCont                    ( u32                   Addr ) const        X_SECTION( render_deferred );
    void    Kick                        ( u32                   Addr ) const        X_SECTION( render_deferred );
    void    Kick                        ( void ) const                              X_SECTION( render_deferred );
    void    NextBuffer                  ( void )                                    X_SECTION( render_deferred );
    void    SetBuffer                   ( s8                    Buffer )            X_SECTION( render_deferred );
    void    NextSkinBuffer              ( void )                                    X_SECTION( render_deferred );
    void    NextSkinShadowCastBuffer    ( void )                                    X_SECTION( render_deferred_shadow );
    void    SetSkinBuffer               ( s8                    Buffer )            X_SECTION( render_deferred );
    void    UploadVertCount             ( u32                   Count,
                                          u32                   Flags,
                                          u8                    UOffset,
                                          u8                    VOffset,
                                          u8                    Alpha ) const       X_SECTION( render_deferred );
    void    UploadShadowVertCount       ( u32                   Count,
                                          u32                   Flags,
                                          u32                   ShadFlags,
                                          u8                    UOffset,
                                          u8                    VOffset ) const     X_SECTION( render_deferred_shadow );
    void    UploadSpriteVertCount       ( u32                   SkipVertCount,
                                          u32                   VertCount,
                                          f32                   UniScale ) const    X_SECTION( render_raw );
    void    UploadVertChunk             ( const vector4*        pVert,
                                          const s16*            pUV,
                                          const s8*             pNormal,
                                          s32                   nVerts ) const      X_SECTION( render_deferred );
    void    UploadRawVertChunk          ( const vector4*        pPos,
                                          const s16*            pUV,
                                          const u32*            pColor,
                                          s32                   nSprites ) const    X_SECTION( render_raw );
    void    UploadSpriteVertChunk       ( const vector4*        pPos,
                                          const vector2*        pRotScale,
                                          const u32*            pColor,
                                          s32                   nVerts ) const      X_SECTION( render_raw );
    void    UploadSpriteVertChunk       ( const vector4*        pPos,
                                          const vector4*        pVelocity,
                                          const u32*            pColor,
                                          s32                   nVerts ) const      X_SECTION( render_raw );
    void    UploadShadReceiveVertChunk  ( const vector4*        pVert,
                                          const s16*            pUV,
                                          s32                   nVerts ) const      X_SECTION( render_deferred_shadow );
    void    UploadRGBChunk              ( const u16*            pRGB,
                                          s32                   nVerts ) const      X_SECTION( render_deferred );
    void    UploadSkinVertChunk         ( const s16*            pUV,
                                          const s16*            pPos,
                                          const u8*             pBoneIndices,
                                          const u8*             pNormals,
                                          s32                   nVerts ) const      X_SECTION( render_deferred );
    void    UploadSkinVertChunk         ( const s16*            pPos,
                                          const u8*             pBoneIndices,
                                          s32                   nVerts ) const      X_SECTION( render_deferred_shadow );
    void    UploadLights                ( const vu1_lightinfo*  pLightInfo ) const  X_SECTION( render_deferred );
    void    UploadSkinLights            ( const vu1_lightinfo*  pLightInfo ) const  X_SECTION( render_deferred );
    void    InitClipper                 ( void ) const                              X_SECTION( render_deferred );
    void    InitSkinClipper             ( void ) const                              X_SECTION( render_deferred );
    void    SetL2W                      ( const matrix4& L2W ) const                X_SECTION( render_deferred );
    void    SetVelMatrix                ( const matrix4& VelMatrix ) const          X_SECTION( render_raw );
    void    SetShadowL2W                ( const matrix4& L2W ) const                X_SECTION( render_deferred_shadow );
    void    BuildInitPackets            ( void )                                    X_SECTION( render_infrequent );
    void    RenderClippedInstances      ( void )                                    X_SECTION( render_deferred );
    void    RenderSkinVertsNormal       ( s32               NVerts,
                                          const s16*        pUV,
                                          const s16*        pPos,
                                          const u8*         pBoneIndices,
                                          const u8*         pNormals,
                                          u32               MCodeAddress )          X_SECTION( render_deferred );
    void    RenderNonClipInstances      ( void )                                    X_SECTION( render_deferred );
    void    RenderShadNonClipInstances  ( void )                                    X_SECTION( render_deferred_shadow );
    void    RenderShadClippedInstances  ( void )                                    X_SECTION( render_deferred_shadow );
    void    ApplyFilterLighting         ( vu1_light& VULight, u32 InstFlags ) const X_SECTION( render_add );

    //=========================================================================
    // Internal data
    //=========================================================================

    // identity matrix which is handy to dma
    matrix4             m_IdentityL2W;

    // cached spotlight information
    vector4             m_SpotlightPos;
    matrix4             m_SpotlightMatrix;
    texture::handle     m_SpotlightTexture;

    // cached shadow information
    s32                 m_nShadowProjectors;
    const matrix4*      m_pProjShadowMatrices[render::MAX_SHADOW_PROJECTORS];
    texture::handle     m_ProjShadowTextures[render::MAX_SHADOW_PROJECTORS];

    // cached clipping data
    vu1_clip_data*      m_pClipData;    // stays in smem

    // cached data for skins
    u32                 m_SkinFlags;
    void*               m_pSkinLightInfo;
    u8                  m_SkinUOffset;
    u8                  m_SkinVOffset;

    // which type of microcode is active at the moment?
    s8                  m_ActiveSyncMCode;
    s8                  m_ActiveInitMCode;
    s8                  m_ActiveClipperMCode;
    s8                  m_ActiveMaterialMCode;
    s8                  m_ActiveXFormMCode;

    // data to speed up dma packet building
    dmatag              m_NormalBuffers[3] PS2_ALIGNMENT(16);
    vector4             m_ViewEnvMapMatrix0;
    vector4             m_ViewEnvMapMatrix1;
    u64                 m_DistortionTex0;
    u64                 m_DistortionClamp;

    // buffer management data
    s8                  m_VUBuffer;
    s8                  m_VUSkinBuffer;
    u16                 m_VUBuffers[3];
    u16                 m_VUSkinBuffers[3];

    // data for rendering instances
    s32                 m_nNonClipInstances;
    s32                 m_nClippedInstances;
    s32                 m_nVerts;
    s32                 m_Offset;
    const vector4*      m_pVert;
    const s16*          m_pUV;
    const s8*           m_pNormal;
    u8                  m_Alpha;
    
    // shadow information
    u8                  m_NextShadowEntry;
    u8                  m_ShadowProjectorCache[SHADOW_CACHE_SIZE];
    vu1_shadow_info     m_ShadowInfo[render::MAX_SHADOW_CASTERS];

    // filter lighting
    xcolor              m_FilterColor;

    //=========================================================================
    // Debugging flags to make sure things are being called in the proper order
    //=========================================================================
    #ifdef X_ASSERT
    u32     m_InBegin       : 1;
    u32     m_InShadowBegin : 1;
    u32     m_InBeginInst   : 1;
    u32     m_InBeginSkin   : 1;
    #endif
};

//=============================================================================

extern vu1_interface g_VU1;

// microcode entry points
extern u8 VU1_ENTRY_SHAD_CAST_SETUP_MATRIX[]    __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_SHAD_CAST_2BONES[]          __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_SHAD_CAST_1BONE[]           __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_SHAD_RECEIVE_FAST[]         __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_SHAD_RECEIVE_SLOW[]         __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_SKIN_SETUP_MATRIX[]         __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_SKIN_XFORM_1BONE[]          __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_SKIN_XFORM_2BONES[]         __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_SKIN_XFORM_CLIPPED[]        __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_RIGID_XFORM_FAST[]          __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_RIGID_XFORM_CLIPPED[]       __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_SPRITE_XFORM[]              __attribute__((section(".vudata")));
extern u8 VU1_ENTRY_VEL_SPRITE_XFORM[]          __attribute__((section(".vudata")));

//=============================================================================
//=============================================================================
//=============================================================================
// inlined functions
//=============================================================================
//=============================================================================
//=============================================================================

inline
void vu1_interface::SetFilterColor( xcolor Filter )
{
    m_FilterColor = Filter;
}

//=============================================================================

inline
void vu1_interface::SetSkinFlags( u32 Flags, u8 Alpha )
{
    m_SkinFlags = Flags;
    if ( Flags & (VU1_FADING_ALPHA | VU1_GLOWING) )
        m_Alpha = (Alpha==255) ? 128 : (Alpha>>1);
    else
        m_Alpha = 0x80;
}

//=============================================================================

inline
void vu1_interface::SetSkinUVOffset( u8 UOffset, u8 VOffset )
{
    m_SkinUOffset = UOffset;
    m_SkinVOffset = VOffset;
}

//=============================================================================

inline
void vu1_interface::SetSkinLightInfo( void* pLightInfo )
{
    m_pSkinLightInfo = pLightInfo;
}

//=============================================================================

inline
void vu1_interface::UploadSkinShadowMatrix( const matrix4& Mat, s16 CacheIndex )
{
    CONFIRM_ALIGNMENT(&Mat);

    DLPtrAlloc( pMat, mat_struct );
    pMat->DMARef.SetRef( sizeof(matrix4), (u32)&Mat );
    pMat->DMARef.PAD[0] = VIF_SkipWrite( 1, 0 );
    pMat->DMARef.PAD[1] = VIF_Unpack( VU1_SKIN_BONE_CACHE + (CacheIndex*4), 4, VIF_V4_32, TRUE, FALSE, TRUE );
    pMat->DMACont.SetCont( 0 );
    pMat->DMACont.PAD[0] = 0;
    pMat->DMACont.PAD[1] = SCE_VIF1_SET_MSCAL( (u32)(VU1_ENTRY_SHAD_CAST_SETUP_MATRIX + 2*CacheIndex), 0 );
}

//=============================================================================

inline
void vu1_interface::SetSpotlightTexture( const texture::handle& Texture )
{
    m_SpotlightTexture = Texture;
}

//=============================================================================

inline
void vu1_interface::SetSpotlightPos( const vector3& Pos )
{
    m_SpotlightPos        = Pos;
    m_SpotlightPos.GetW() = 0;
}

//=============================================================================

inline
void vu1_interface::SetSpotlightMatrix( const matrix4& Matrix )
{
    m_SpotlightMatrix = Matrix;
}

//=============================================================================

inline
void vu1_interface::SetProjShadowTexture( s32 Index, texture::handle Texture )
{
    ASSERT( (Index>=0) && (Index<render::MAX_SHADOW_PROJECTORS) );
    m_ProjShadowTextures[Index] = Texture;
}

//=============================================================================

inline
void vu1_interface::SetProjShadowMatrix( s32 Index, const matrix4&  Matrix )
{
    ASSERT( (Index>=0) && (Index<render::MAX_SHADOW_PROJECTORS) );
    m_pProjShadowMatrices[Index] = &Matrix;
}

//=============================================================================

inline
void vu1_interface::SetNShadowProjectors( s32 nProjectors )
{
    m_nShadowProjectors = nProjectors;
}

//=============================================================================

inline
void vu1_interface::Sync( void ) const
{
    // Stall until the VU1, PATH1 and PATH2 are idle
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
}

//=============================================================================

inline
void vu1_interface::SyncDList( void ) const
{
    // Stall until the VU1, PATH1 and PATH2 are idle
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_FLUSH( 0 );
}

//=============================================================================

inline
void vu1_interface::SyncAll( void ) const
{
    // Stall until the VU1, PATH1, PATH2, PATH3 and GS are idle
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_MSCAL( MCODE_START_SYNC/8, 0 );
    pPack->PAD[1] = SCE_VIF1_SET_FLUSHA( 0 );
}

//=============================================================================

inline
void vu1_interface::KickCont( u32 Addr ) const
{
    // Set TOPS register and Kick VU1 from Addr
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_MSCAL( Addr, 0 );
    pPack->PAD[1] = SCE_VIF1_SET_MSCNT( 0 );
}

//=============================================================================

inline
void vu1_interface::Kick( u32 Addr ) const
{
    // Set TOPS register and Kick VU1 from Addr
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_MSCAL( Addr, 0 );
}

//=============================================================================

inline
void vu1_interface::Kick( void ) const
{
    // Set TOPS register and Kick VU1 from current PC position
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_MSCNT( 0 );
}

//=============================================================================

inline
void vu1_interface::NextBuffer( void )
{
    // Set destination buffer for uploading data
    DLPtrAlloc( pDst, u_long128 );
    *pDst = *((u_long128*)&m_NormalBuffers[m_VUBuffer]);

    // Advance the VU memory triple buffer
    m_VUBuffer++;
    if( m_VUBuffer == 3 )
        m_VUBuffer  = 0;
}

//=============================================================================

inline
void vu1_interface::SetBuffer( s8 Buffer )
{
    // Set destination buffer for uploading data
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_BASE( m_VUBuffers[ Buffer ], 0 );
    pPack->PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );
    
    m_VUBuffer = Buffer;
}

//=============================================================================

inline
void vu1_interface::NextSkinBuffer( void )
{
    // Set destination buffer for uploading data
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_BASE( m_VUSkinBuffers[ m_VUSkinBuffer ], 0 );
    pPack->PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );

    // Toggle the VU Skin double-buffer
    m_VUSkinBuffer = m_VUSkinBuffer ? 0 : 1;
}

//=============================================================================

inline
void vu1_interface::NextSkinShadowCastBuffer( void )
{
    // Set destination buffer for uploading data
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_BASE( m_VUSkinBuffers[ m_VUSkinBuffer ], 0 );
    pPack->PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );

    // Toggle the VU Skin double-buffer
    m_VUSkinBuffer = (m_VUSkinBuffer + 1) %3;
}

//=============================================================================

inline
void vu1_interface::SetSkinBuffer( s8 Buffer )
{
    // Set destination buffer for uploading data
    DLPtrAlloc( pPack, dmatag );
    pPack->SetCont( 0 );
    pPack->PAD[0] = SCE_VIF1_SET_BASE( m_VUSkinBuffers[ Buffer ], 0 );
    pPack->PAD[1] = SCE_VIF1_SET_OFFSET( 0, 0 );
    
    m_VUSkinBuffer = Buffer;
}

//=============================================================================

inline
void vu1_interface::UploadVertCount( u32 Count, u32 Flags, u8 UOffset, u8 VOffset, u8 Alpha ) const
{
    ASSERT( Count >= 3 );

    DLPtrAlloc( pPack, vu1_vector4 );
    pPack->DMA.SetCont( sizeof( vu1_vector4 ) - sizeof( dmatag ) );
    pPack->DMA.PAD[1] = VIF_Unpack( VU1_COUNT, 1, VIF_V4_32, FALSE, FALSE, FALSE );
    pPack->X = (u32)UOffset;
    pPack->Y = (u32)VOffset;
    pPack->Z = Count | (Flags & 0x0000ff80);
    pPack->W = (u32)Alpha | ((Flags & 0x00ff0000)>>8);
}

//=============================================================================

inline
void vu1_interface::UploadShadowVertCount( u32 Count,
                                           u32 Flags,
                                           u32 ShadFlags,
                                           u8  UOffset,
                                           u8  VOffset ) const
{
    ASSERT( Count >= 3 );

    DLPtrAlloc( pPack, vu1_vector4 );
    pPack->DMA.SetCont( sizeof( vu1_vector4 ) - sizeof( dmatag ) );
    pPack->DMA.PAD[1] = VIF_Unpack( VU1_COUNT, 1, VIF_V4_32, FALSE, FALSE, FALSE );
    pPack->X = (u32)UOffset;
    pPack->Y = (u32)VOffset;
    pPack->Z = Count | (Flags & 0x0000ff80);
    pPack->W = ShadFlags;
}

//=============================================================================

inline
void vu1_interface::UploadSpriteVertCount( u32 SkipVertCount, u32 VertCount, f32 UniScale ) const
{
    ASSERT( VertCount );

    DLPtrAlloc( pPack, vu1_vector4 );
    pPack->DMA.SetCont( sizeof( vu1_vector4 ) - sizeof( dmatag ) );
    pPack->DMA.PAD[1] = VIF_Unpack( VU1_COUNT, 1, VIF_V4_32, FALSE, FALSE, FALSE );
    pPack->X = (SkipVertCount*4);
    pPack->Y = reinterpret_cast<u32&>(UniScale);
    pPack->Z = (VertCount*4) | VU1_CLIPPED;
    pPack->W = 0;
}

//=============================================================================

inline
void vu1_interface::UploadLights( const vu1_lightinfo* pLightInfo ) const
{
    ASSERT( pLightInfo );
    DLPtrAlloc( pLight, dmatag );
    pLight->SetRef( sizeof(vu1_lightinfo), (u32)pLightInfo );
    pLight->PAD[0] = 0;
}

//=============================================================================

inline
void vu1_interface::UploadSkinLights( const vu1_lightinfo* pLightInfo ) const
{
    UploadLights( pLightInfo );
}

//=============================================================================

inline
void vu1_interface::SetVelMatrix( const matrix4& VelMatrix ) const
{
    ASSERT( m_InBegin == TRUE );
    DLPtrAlloc( pDMA, dmatag );
    pDMA->SetRef( sizeof(vector4)*3, (u32)&VelMatrix );
    pDMA->PAD[0] = VIF_SkipWrite( 1, 0 );
    pDMA->PAD[1] = VIF_Unpack( VU1_L2W+4, 3, VIF_V4_32, TRUE, FALSE, FALSE );
}

//=============================================================================

inline
void vu1_interface::SetShadowL2W( const matrix4& L2W ) const
{
    ASSERT( m_InShadowBegin );
    DLPtrAlloc( pDMA, dmatag );
    pDMA->SetRef( sizeof(matrix4), (u32)&L2W );
    pDMA->PAD[0] = VIF_SkipWrite( 1, 0 );
    pDMA->PAD[1] = VIF_Unpack( VU1_L2W, 4, VIF_V4_32, TRUE, FALSE, FALSE );
}

//=============================================================================

inline
void vu1_interface::ApplyFilterLighting( vu1_light& VULight, u32 InstFlags ) const
{
    if( render::IsFilterLightEnabled() &&
        ((InstFlags & render::DISABLE_FILTERLIGHT) == 0) )
    {
        xcolor Filter = render::GetFilterLightColor();
        VULight.R *= (f32)Filter.R * (1.0f / 128.0f);
        VULight.G *= (f32)Filter.G * (1.0f / 128.0f);
        VULight.B *= (f32)Filter.B * (1.0f / 128.0f);
    }
}

//=============================================================================

inline
void vu1_interface::RenderSkinVertsNormalOneBone( s32               NVerts,
                                                  const s16*        pUV,
                                                  const s16*        pPos,
                                                  const u8*         pBoneIndices,
                                                  const u8*         pNormals )
{
    RenderSkinVertsNormal( NVerts, pUV, pPos, pBoneIndices, pNormals, (u32)VU1_ENTRY_SKIN_XFORM_1BONE );
}

//=============================================================================

inline
void vu1_interface::RenderSkinVertsNormalTwoBones( s32               NVerts,
                                                   const s16*        pUV,
                                                   const s16*        pPos,
                                                   const u8*         pBoneIndices,
                                                   const u8*         pNormals )
{
    RenderSkinVertsNormal( NVerts, pUV, pPos, pBoneIndices, pNormals, (u32)VU1_ENTRY_SKIN_XFORM_2BONES );
}

//=============================================================================

#endif
