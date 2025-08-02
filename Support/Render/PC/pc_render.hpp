#ifndef _PC_RENDER_H_
#define _PC_RENDER_H_

//=============================================================================
//=============================================================================
// Forward references
//=============================================================================
//=============================================================================

#define COMPILE_SHADERS 1
#define PRELOAD_SHADERS 1

struct render_instance;

extern s32 XRes, YRes;

// Evil code: we shall not speak of it again.
#define PropH(a) u32( (f32(PropW(a)) * f32(XRes) / f32(YRes)) + 0.5f )
#define PropW(a) u32( 0x##a /4 )

extern s32                 vram_Register    ( IDirect3DTexture9* pTexture );
extern void                vram_Activate    ( s32 Stage, s32 VRAM_ID );
extern IDirect3DTexture9*  vram_GetSurface  ( s32 VRAM_ID ); //IDK

//=============================================================================
//=============================================================================
// Types and constants specific to the PC-implementation
//=============================================================================
//=============================================================================

#ifdef CONFIG_RETAIL
    #define SWITCH_PER_PIXEL_LIGHTING     1 // enables per pixel lighting
    #define SWITCH_DUMP_SHADER_LIBRARIES  0
    #define SWITCH_USE_INTENSITY_DECALS   1
    #define SWITCH_USE_NORMAL_MATERIALS   1 // enables normal material rendering
    #define SWITCH_USE_DEPTH_OF_FIELD     1
    #define SWITCH_USE_FRAME_SCALING      0 // enables frame scaling logic
    #define SWITCH_ENABLE_COPYFRAME       1 // enable the copy frame logic
    #define SWITCH_USE_PROJ_SHADOWS       1 // enable projective shadows
    #define SWITCH_USE_DETAIL_MAPS        1 // enables detail map effect
    #define SWITCH_USE_MUTANT_MODE        1 // enables mutant vision mode
    #define SWITCH_USE_DISTORTION         1 // enables the distortion effect
    #define SWITCH_USE_FLASHLIGHT         1
    #define SWITCH_USE_Z_PRIMING          1 // enables object alpha fading
    #define SWITCH_USE_TEXTURES           1
    #define SWITCH_USE_SHADOWS            1
    #define SWITCH_USE_FOG                1 // integral to all: cannot be zero here
#else
    #define SWITCH_USE_DIFFUSE_ONLY       1 // debug: uses only diffuse materials
    static SWITCH_DUMP_SHADER_LIBRARIES  =1;
    static SWITCH_USE_INTENSITY_DECALS   =1;
    static SWITCH_USE_NORMAL_MATERIALS   =1;
    static SWITCH_USE_DEPTH_OF_FIELD     =1;
    static SWITCH_PER_PIXEL_LIGHTING     =1; // enables per pixel lighting
    static SWITCH_USE_FRAME_SCALING      =0; // enables frame scaling logic
    static SWITCH_ENABLE_COPYFRAME       =1; // enable the copy frame logic
    static SWITCH_USE_PROJ_SHADOWS       =1; // enable projective shadows
    static SWITCH_USE_DETAIL_MAPS        =1; // enables detail map effect
    static SWITCH_USE_MUTANT_MODE        =1; // enables mutant vision mode
    static SWITCH_USE_DISTORTION         =1;
    static SWITCH_USE_FLASHLIGHT         =1;
    static SWITCH_USE_Z_PRIMING          =1; // enables object alpha fading
    static SWITCH_SHOW_LIGHTMAP          =0;
    static SWITCH_USE_TEXTURES           =1;
    static SWITCH_USE_SHADOWS            =1;
    static SWITCH_USE_FOG                =1; // integral to all: cannot be zero here
#endif

#define MAX_VS_WHITE_BUFF     262144
#define MAX_RESOURCES         16384
#define MAX_VS_LIGHTS         4
#define MAX_CAST              4
#define MAX_FRAGS             ( MAX_VS_STYLE_BITS + MAX_PS_STYLE_BITS )
#define MAX_SHADOWBUFFER      ( 32 / MAX_CAST )
#define PER_PIXEL_POINT_RANGE 10000.0f
#define OPTIMISE_SHADERS      1

#define VOL_DEBUG 0
#define VOL_W 64
#define VOL_H 64
#define VOL_D 64

#define SASMT_PIXELSHADER  0 //XBOX stuff, should be removed
#define SASMT_VERTEXSHADER 1 //XBOX stuff, should be removed

//////////////////////////////////////////////////////////////////////////////

struct lights
{
    struct point
    {
        vector4 Pos; // W=radius
        vector4 Col;
    };

    struct dir
    {
        vector4 Dir;
        vector4 Col;
    };

    vector4 Ambient;
    u32     Count;

    union
    {
        point* pPoint;
         void* pBoth;
          dir* pDir;
    };

    lights( void )
    {
        pBoth = NULL;
    }
};

//////////////////////////////////////////////////////////////////////////////

template< class T >struct node
{
    void Insert ( T* pNode )
    {
        pNode-> pNext = pNext;
        pNext-> pPrev = pNode;
        pNode-> pPrev = (T*)this;
        pNext = pNode;
    }

    bool IsEmpty( void )
    {
        return( this!=pPrev && this!=pNext );
    }

    T* Remove ( void )
    {
        pPrev-> pNext = pNext;
        pNext-> pPrev = pPrev;
        return(T*)this;
    }

    node( void )
    {
        pNext=( T* )this;
        pPrev=( T* )this;
    }

    T* pNext;
    T* pPrev;
};

//////////////////////////////////////////////////////////////////////////////

enum shader_style // public shaders
{
    kFULL_POST_EFFECT,
    kFINISH_FRAME,
    kDISTORT_BACK,
    kDIFFUSE_SAT,
    kCAST_SHADOW,
    kSHADOW_BLUR,
    kT0_MASK_T1,
    kDIFFUSE,
    kREMOVE
};

//////////////////////////////////////////////////////////////////////////////

#define kTEX0   1
#define kTEX1   2
#define kTEX2   4
#define kTEX3   8

///////////////////////////////////////////////////////////////////////////

extern struct shader_mgr
{
    // --------------------------------------------------------------------

    vector4   m_FogConst[5];
    u8        m_FogPalette[5][64*4];

    // --------------------------------------------------------------------

    IDirect3DTexture9*      m_PIP_Texture;
    IDirect3DSurface9*      m_PIP_Target;
    s32                     m_VRAM_PipID;
    void*                   m_pPipData;

    s32                     m_CanContinue;
    f32                     m_FixedAlpha; // used for fading objects
    f32                     m_WhiteConst;
    s32                     m_FogIndex;
    vs::desc                m_VSFlags;
    ps::desc                m_PSFlags;

    // --------------------------------------------------------------------

    union // a few internal states
    {
        struct
        {
            u32 m_bFullControl:1;
            u32 m_bVSLoaded   :1;
            u32 m_bPSLoaded   :1;
        };
        u32 m_State;
    };

    // --------------------------------------------------------------------

    void SetWhiteConst( f32 WhiteConst )
    {
        m_WhiteConst = WhiteConst;
    }

    // --------------------------------------------------------------------

    void SetFixedAlpha( f32 FixedAlpha )
    {
        m_FixedAlpha = FixedAlpha;
    }

    // --------------------------------------------------------------------

    void ConstSanityCheck   ( void );

    // --------------------------------------------------------------------

    void DumpShader   ( void );

    // --------------------------------------------------------------------

    void SetCustomFogPalette   ( const texture::handle&,xbool,s32 );
    s32  InsertShadow          ( ps::desc&,IDirect3DTexture9* );
    s32  SetPerPixelPointLights( const lights*,const matrix4& );
    s32  SetPointLights        ( const lights*,const matrix4& );
    void SetDirLights          ( const lights* );
    void SetPixelShader        ( shader_style );

    // --------------------------------------------------------------------

    //! Link shaders
    /** This routine takes shader descriptions
        and quickly links them together to
        form a useable vertex/pixel shader
        combo.
        */

    u32 Link( vs::desc&,
              ps::desc&,
              bool bAllocTop = false );
    u32 Link( u32 Count      ,
              const matrix4* ,
              vs::desc&      ,
              const vector4* ,
              ps::desc&      ,
              bool bAllocTop = false );

    // --------------------------------------------------------------------

    void Begin( void );
    void End  ( void );

    // --------------------------------------------------------------------

~   shader_mgr( void );
    shader_mgr( void );
}
* g_pShaderMgr;

///////////////////////////////////////////////////////////////////////////

extern struct pipeline_mgr
{
    // ---------------------------------------------------------- internals

    void     ApplyProjectedShadows( void );
    void     SetupShadowConsts    ( void );
    void     SetupTexConsts       ( ps::desc& );
    void     SetupCubeConsts      ( vs::desc& );
    void     SetupDistortion      ( vs::desc&,ps::desc& );
    void     AddDetailMap         ( vs::desc&,ps::desc& );
    vector4* SetupLighting        ( vs::desc&,ps::desc& );
    void     InsertShadow         ( vs::desc&,ps::desc& );
    bool     AddFlashlight        ( vs::desc&,ps::desc& );
    void     SetSkinConst         ( skin_geom::command_pc& );
    void     BloomFilter          ( s32,f32,f32 );
    void     CreateGlowEffect     ( void );
    void     EndProfiling         ( void );
    bool     BeginPass            ( render_instance& );
    void     SetupL2W             ( render_instance&,vs::desc&,ps::desc& );
    void     BeginProfiling       ( void );
    void     SetupDownSampling    ( void );
    void     JitterGlows          ( void );
    void     ApplyGlow            ( void );

    // --------------------------------------------------------------------

    f32              m_MotionBlurIntensity;
    s32              m_iDistortionCount;
    u32              m_bMutantGlowed;
    s8               m_bDirtyShadows;
    s32              m_MaterialFlags;
    s32              m_MaterialType;
    s8               m_bSplitScreen;
    f32              m_GlowCutoff;
    f32              m_FixedAlpha;
    u32              m_AlphaFade;
    radian3          m_NormalRot;
    s32              m_iShadow;
    u32              m_bGlowed;
    render_instance* m_pInst;

    // --------------------------------------------------------------------

public:

    // --------------------------------------------------------------------

    enum e_target
    {
        /* main buffers */

        MAIN_BEGIN                 , // 00
            kPRIMARY = MAIN_BEGIN  , // 00 Rigid/smooth geoms rendered here   (512x448)
            kPRIMARY_Z             , // 01 Primary Z
            kSAMPLE0               , // 02 Down-sample stage 1                (256x224)
            kSAMPLE1               , // 03 Down-sample stage 2                (128x112)
            kSAMPLE2               , // 04 Down-sample stage 2                ( 64x56 )
            kJITTER                , // 05 Down-sample stage 3                (256x224)
        MAIN_LAST                  , // 06 Cannot be more than 6

        /* pip buffers */

        GLOW_BEGIN = MAIN_LAST             , // 06
            kGLOW_ACCUMULATOR = GLOW_BEGIN , // 06 Special mutant vision glow accumulator
        GLOW_LAST                          , // 07

        /* shadow buffers */

        SHADOW_BEGIN = GLOW_LAST,
        SHADOW_LAST  = SHADOW_BEGIN + MAX_SHADOWBUFFER,

        /* last buffer */

        kLAST = SHADOW_LAST,
        kTEMP_PIP,
        kPIP,
        kFOG,
        kTOTAL_SLOTS,

        /* shared aliases */

        kINTERMEDIATE = kPRIMARY,
        kLIGHTMAP     = kPRIMARY,
    };

    // --------------------------------------------------------------------

    union
    {
        geom * m_pActiveRigidGeom;
        void * m_pVariant;
    };

    // --------------------------------------------------------------------

    IDirect3DTexture9* m_pDetailMap;
    IDirect3DTexture9* m_pProjMap;

    // --------------------------------------------------------------------

    static IDirect3DVolumeTexture9* m_hAttenuationVolume;
    static IDirect3DTexture9*       m_pTexture[kTOTAL_SLOTS];
    static IDirect3DSurface9*       m_pTarget [kTOTAL_SLOTS];
    static IDirect3DSurface9*       m_pBkSurface;
    static xbool                    m_NeedsAlloc;

    // --------------------------------------------------------------------

    IDirect3DTexture9* m_LitPunchthru;

    void*     m_pWhiteBuffer;
    f32       m_DetailScale;
    f32       m_OptW,m_MaxH;
    f32       m_OptH,m_MaxW;
    matrix4   m_ProjMatrix;
    void*     m_pLightData;
    view      m_NormalView;
    f32       m_IDistance;
    vector3   m_OldEye[2];
    f32       m_AveDuration;
    u32       m_AveDurSteps;
    f32       m_Duration;
    s32       m_iSubMesh;
    u32       m_VshRigid;
    f32       m_AveSteps;
    f32       m_AveTime;
    vs::desc  m_VSFlags;
    ps::desc  m_PSFlags;
    s32       m_nZones;
    s32       m_Zone;
    xtimer    m_Time;
    matrix4   m_L2W;

    s32 m_CopyVRAMID;
    f32 m_CopyU0;
    f32 m_CopyV0;
    f32 m_CopyU1;
    f32 m_CopyV1;

    s32 m_PipW;
    s32 m_PipH;

    // --------------------------------------------------------------------

    matrix4 m_ShadowW2C[render::MAX_SHADOW_CASTERS];
    matrix4 m_ShadowW2V[render::MAX_SHADOW_CASTERS];
    union
    {
        struct
        {
            u32 m_bMatCanReceiveShadow:1;
            u32 m_bSelfIllumUseDiffuse:1;
            u32 m_bColourStreamActive :1;
            u32 m_bNoVertexColours    :1;
            u32 m_bDirtySelfIllum     :1;
            u32 m_bDirtyLightMap      :1;
            u32 m_bInDistortion       :1;
            u32 m_bUseRigidGeom       :1;
            u32 m_bIsSelfIllum        :1;
            u32 m_bIsDiffIllum        :1;
            u32 m_bPerPixelLit        :1;
            u32 m_bIsPunchthru        :1;
            u32 m_bInLightMap         :1;
            u32 m_bAlphaBlend         :1;
            u32 m_bForceZFill         :1;
            u32 m_bPipActive          :1;
            u32 m_bUsingPIP           :1;
            u32 m_bZPriming           :1;
            u32 m_bLockOut            :1;
            u32 m_bZPrime             :1;
            u32 m_bEnvL2W             :1;
        };
        u32 m_State;
    };

    f32 m_TintColour[4];

    // --------------------------------------------------------------------

public:

    // --------------------------------------------------------------------

    pipeline_mgr( u32 NZones,bool bRunAt60 ); //So , do we need bRunAt60 on pc?
~   pipeline_mgr( void );

    // --------------------------------------------------------------------

    void SetMotionBlurIntensity( f32 MotionBlurIntensity ){ m_MotionBlurIntensity = MotionBlurIntensity; }
    void SetGlowCutOff         ( f32 GlowCutoff          ){ m_GlowCutoff          = GlowCutoff;          }
    void SetZPrime             ( xbool );

    // --------------------------------------------------------------------

    void  CalculateViewport      ( s32 TargetID,D3DVIEWPORT9& Vp                               );
    void  BeginShadowReceiveRigid( geom* pGeom,s32 iSubMesh                                    );
    void  AddDirShadowProjector  ( const matrix4& L2W,f32 Width,f32 Height,f32 NearZ ,f32 FarZ );
    void  SetupDistortionConsts  ( render_instance& Inst                                       );
    void  SetProjectiveTexture   ( const IDirect3DTexture9* Texture                            );
    void  SetDefaultDistortion   ( const radian3& NormalRot                                    );
    void  SetProjShadowConsts    ( s32 Index,s32 BaseRegister                                  );
    void  ClearShadowBuffers     ( u32 nShadowProjectors                                       );
    bool  FogEnable              ( vs::desc& VSFlags,ps::desc& PSFlags,bool bEnable            );
    void  SetupChannels          ( vs::desc& VSFlags,ps::desc& PSFlags                         );
    void  SetupEnvMap            ( u32 Flags,const xbitmap*,const xbitmap*                     );
    void  SetProjShadowStage     ( s32 ProjectiveIndex,s32 TextureStage                        );
    void  BeginShadowPass        (                                                             );
    void  EndShadowPass          (                                                             );
    void  StartShadowReceive     (                                                             );
    void  SetupDepthOfField      (                                                             );
    void  BeginNormalRender      (                                                             );
    void  EndShadowReceive       (                                                             );
    void  ResizePipTexture       ( s32 W,s32 H                                                 );
    void  SetEnvDistortion       ( u32 Flags,const radian3& NormalRot,const xbitmap* pEnvMap   );
    void  BlendEffects           ( shader_style                                                );
    void  RenderToShadows        ( render_instance &,s32 iProj                                 );
    void  BeginDistortion        (                                                             );
    bool  SetZMaterial           ( const material& Material                                    );
    void  SetLitMaterial         ( const material& Material                                    );
    void  EndNormalRender        (                                                             );
    void  PostEffect             (                                                             );
    void  SetRenderTarget        ( s32 TargetID, s32 DepthID                                   );
    void  SetRenderTarget        ( s32 TargetID, s32 DepthID,D3DVIEWPORT9& Vp                  );
    void  SwitchToBackBuffer     (                                                             );
    void  SkinCastShadow         ( render_instance &,s32 iProj                                 );
    void  EndDistortion          (                                                             );
    void  RenderToLightMap       ( render_instance &                                           );
    void  RenderToZBuffer        ( render_instance &                                           );
    void  SetPipTexture          (                                                             );
    void  BeginLightMap          (                                                             );
    void  EndLightMap            (                                                             );
    xbool IsPipActive            (                                                             ){   return m_bPipActive; }
    void  StoreFrameCopyInfo     ( s32 VRAMID,f32 U0,f32 V0,f32 U1,f32 V1                      );
    void  BeginZPrime            (                                                             );
    void  EndZPrime              (                                                             );
    void  CopyFrameTo            (                                                             );
    void  SetMaterial            ( const   material&                                           );
    void  RenderInstance         (                                                             );
    void  BeginRigid             ( geom *,s32                                                  );
    void  BeginSkin              ( geom *,s32                                                  );
    void  LockOut                ( xbool bLockOut                                              ){ m_bLockOut = bLockOut; }
    void  RenderToPrimary        ( render_instance &                                           );
    void  AddFilterLight         ( ps::desc& PSFlags                                           );
    void  CreateAliasedTarget    ( s32,s32,s32,s32                                             );
    void  SetAliasedTarget       ( s32,s32,s32,s32                                             );
    void  End                    (                                                             );
    void  ApplyFog               (                                                             );

    // --------------------------------------------------------------------

    void SetProjMatrix( const matrix4& Matrix )
    {
        m_ProjMatrix = Matrix;
    }

    // --------------------------------------------------------------------

    void BeginTiming( void )
    {
        m_Duration = m_Time.ReadMs();

        m_Time.Reset();
        m_Time.Start();
    }

    // --------------------------------------------------------------------

    f32 GetHeight( void )   { return m_OptH; }
    f32 GetWidth ( void )   { return m_OptW; }


    const xbitmap* m_pEnvironmentMap;
}
* g_pPipeline;

#endif