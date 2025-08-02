#ifndef RENDER_HPP
#define RENDER_HPP

#include "x_types.hpp"
#include "x_time.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Render\Material.hpp"
#include "Render\RigidGeom.hpp"
#include "Render\SkinGeom.hpp"

//=============================================================================

#if defined(dstewart) && !defined(X_OPTIMIZED)
#define ENABLE_RENDER_STATS 1
#else
#define ENABLE_RENDER_STATS 0
#endif

//=============================================================================

namespace render
{
    // debugging information that can be exposed to the game
#ifndef X_RETAIL
    struct debug_options
    {
        xbool RenderRigidOnly;
        xbool RenderSkinOnly;
        xbool RenderClippedOnly;
        xbool RenderShadowedOnly;
    };
#endif

    // startup and shutdown routines
    void    Init    ( void )    X_SECTION( init );
    void    Kill    ( void )    X_SECTION( init );

    // update routines (needed for uv and texture animation)
    void    Update  ( f32 DeltaTime )   X_SECTION( update );

    // routines for rendering raw triangle data or sprite data
    enum
    {
        BLEND_MODE_ADDITIVE = 0,
        BLEND_MODE_SUBTRACTIVE,
        BLEND_MODE_NORMAL,
        BLEND_MODE_INTENSITY
    };
    s32     GetHardwareBufferSize   ( void ) X_SECTION( render_infrequent );
    void    StartRawDataMode        ( void ) X_SECTION( render_raw );
    void    EndRawDataMode          ( void ) X_SECTION( render_raw );
    void    RenderRawStrips         ( s32               nVerts,
                                      const matrix4&    L2W,
                                      const vector4*    pPos,
                                      const s16*        pUV,
                                      const u32*        pColor ) X_SECTION( render_raw );
    void    Render3dSprites         ( s32               nSprites,
                                      f32               UniScale,
                                      const matrix4*    pL2W,
                                      const vector4*    pPositions,     // w contains 0 if particle is active, otherwise 0x8000
                                      const vector2*    pRotScales,     // array of vector2(rotation,scale)
                                      const u32*        pColors ) X_SECTION( render_raw );
    void    RenderVelocitySprites   ( s32               nSprites,
                                      f32               UniScale,
                                      const matrix4*    pL2W,
                                      const matrix4*    pVelMatrix,     // a velocity matrix that will be combined with l2w
                                      const vector4*    pPositions,     // w contains 0 if particle is active, otherwise 0x8000
                                      const vector4*    pVelocities,    // w contains scale
                                      const u32*        pColors ) X_SECTION( render_raw );
    void    RenderHeatHazeSprites   ( s32               nSprites,
                                      f32               UniScale,
                                      const matrix4*    pL2W,
                                      const vector4*    pPositions,
                                      const vector2*    pRotScales,
                                      const u32*        pColors ) X_SECTION( render_raw );
    void    SetDiffuseMaterial      ( const xbitmap&    Bitmap,
                                      s32               BlendMode,
                                      xbool             ZTestEnabled = TRUE ) X_SECTION( render_raw );
    void    SetGlowMaterial         ( const xbitmap&    Bitmap,
                                      s32               BlendMode,
                                      xbool             ZTestEnabled = TRUE ) X_SECTION( render_raw );
    void    SetEnvMapMaterial       ( const xbitmap&    Bitmap,
                                      s32               BlendMode,
                                      xbool             ZTestEnabled = TRUE ) X_SECTION( render_raw );
    void    SetDistortionMaterial   ( s32               BlendMode,
                                      xbool             ZTestEnabled = TRUE ) X_SECTION( render_raw );

    // data registration routines
    typedef xhandle hgeom_inst;
    hgeom_inst  RegisterRigidInstance   ( rigid_geom&   Geom  ) X_SECTION( init );
    void        UnregisterRigidInstance ( hgeom_inst    hInst ) X_SECTION( init );
    hgeom_inst  RegisterSkinInstance    ( skin_geom&    Geom  ) X_SECTION( init );
    void        UnregisterSkinInstance  ( hgeom_inst    hInst ) X_SECTION( init );
    const geom* GetGeom                 ( hgeom_inst    hInst ) X_SECTION( init );

    // functions for setting a projected texture (only one allowed--use it for
    // a flashlight!)
    void        SetTextureProjection        ( const matrix4&         L2W,
                                              radian                 FOV,
                                              f32                    Length,
                                              const texture::handle& Texture ) X_SECTION( render_infrequent );

    // projected shadows that cast onto both players and the world
    enum { MAX_SHADOW_PROJECTORS = 2 };
    void        SetShadowProjection         ( const matrix4&         L2W,
                                              radian                 FOV,
                                              f32                    Length,
                                              const texture::handle& Texture ) X_SECTION( render_infrequent );

    // send these flags to describe how the object should be rendered.
    // most of the time you should just be using 0 and possibly CLIPPED,
    // as the other flags are not guaranteed to work on all platforms (they
    // are intended for use by the editor)
    enum
    {
        WIREFRAME            = 0x00000001,
        WIREFRAME2           = 0x00000002,
        PULSED               = 0x00000004,
        SHADOW_PASS          = 0x00000008,
        GLOWING              = 0x00000010,
        FADING_ALPHA         = 0x00000020,
        CLIPPED              = 0x00000040,
        FORCE_LAST           = 0x01000000,
        DISABLE_SPOTLIGHT    = 0x02000000,
        DISABLE_FILTERLIGHT  = 0x04000000,
        DISABLE_PROJ_SHADOWS = 0x08000000,
        DO_SIMPLE_LIGHTING   = 0x10000000,  // for optimization--enables the lighting for AddRigidInstanceSimple

        // Xbox per pixel lighting:
        PERPIXEL_POINTLIGHT  = 0x20000000,
        
        // TODO: Our flags have started clashing. For now, if we make INSTFLAG_CLIPPED
        // the same as CLIPPED we are fine, but these flags really need to be re-thought.
        // Preferably, the render flags should be completely separated from the instance
        // flags, and the render system can internally do whatever it needs to do to make
        // things work.
        // these instance flags are considered private. don't look!
        INSTFLAG_CLIPPED       = 0x00000080,    // Does the instance intersect with the frustum?
        INSTFLAG_GLOWING       = 0x00000100,    // we have forced something that doesn't normally glow to glow
        INSTFLAG_SHADOW_PASS   = 0x00000200,    // we are receiving dynamic shadows
        INSTFLAG_FILTERLIGHT   = 0x00000400,    // modulate vertex lighting (i.e. emergency red light situation)
        INSTFLAG_SPOTLIGHT     = 0x00000800,    // The instance receives a spotlight projection (flashlight)
        INSTFLAG_FADING_ALPHA  = 0x00001000,    // the geometry is fading out
        INSTFLAG_DYNAMICLIGHT  = 0x00002000,    // dynamic lighting is on (point or directional
        INSTFLAG_DETAIL        = 0x00004000,    // detail mapping is on (material still has it, but the object is distant)
        INSTFLAG_PROJ_SHADOW_1 = 0x00010000,    // we are receiving the first projected shadow
        INSTFLAG_PROJ_SHADOW_2 = 0x00020000,    // we are receiving the second projected shadow
    };

    // shadow creation routines--we can handle up to 64 shadow textures, that way
    // each caster should be able to have it's own texture to reduce aliasing
    // The projection mask says which receivers receive which textures,
    // and also which casters cast into which textures.
    enum
    {
        MAX_SHADOW_CASTERS = 64,
    };
    void    BeginShadowCreation     ( void ) X_SECTION( render_infrequent );
    void    EndShadowCreation       ( void ) X_SECTION( render_deferred_shadow );
    void    AddPointShadowProjection( const matrix4&         L2W,
                                      radian                 FOV,
                                      f32                    NearZ,
                                      f32                    FarZ       ) X_SECTION( render_add_shadow );
    void    AddDirShadowProjection  ( const matrix4&         L2W,
                                      f32                    Width,
                                      f32                    Height,
                                      f32                    NearZ,
                                      f32                    FarZ       ) X_SECTION( render_add_shadow );
    void    AddRigidCasterSimple    ( hgeom_inst             hInst,
                                      const matrix4*         pL2W,  // will be DMA ref'd to!
                                      u64                    ProjMask   ) X_SECTION( render_add_shadow );
    void    AddRigidCaster          ( hgeom_inst             hInst,
                                      const matrix4*         pL2W,
                                      u64                    Mask,
                                      u64                    ProjMask   ) X_SECTION( render_add_shadow );
    void    AddSkinCaster           ( hgeom_inst             hInst,
                                      const matrix4*         pBone,
                                      u64                    Mask,
                                      u64                    ProjMask   ) X_SECTION( render_add_shadow );
    void    AddRigidReceiverSimple  ( hgeom_inst             hInst,
                                      const matrix4*         pL2W,  // will be DMA ref'd to!
                                      u32                    Flags,
                                      u64                    ProjMask   ) X_SECTION( render_add_shadow );
    void    AddRigidReceiver        ( hgeom_inst             hInst,
                                      const matrix4*         pL2W,
                                      u64                    Mask,
                                      u32                    Flags,
                                      u64                    ProjMask   ) X_SECTION( render_add_shadow );
    void    AddSkinReceiver         ( hgeom_inst             hInst,
                                      const matrix4*         pBone,
                                      u64                    Mask,
                                      u32                    Flags,
                                      u64                    ProjMask   ) X_SECTION( render_add_shadow );

    // xbox specific light map rendering
    #ifdef TARGET_XBOX
    void ZPrimeRenderTarget( void );
    void RenderLightMap( void );
    #endif

    // basic instance-rendering routines
    // you should call them in this order:
    // BeginNormalRender()
    //   for all instances:
    //   AddRigidInstanceSimple  OR
    //   AddRigidInstance        OR
    //   AddSkinInstance         OR
    //   AddSkinInstanceDistorted
    // EndNormalRender()
    // BeginCustomRender()
    //   Render custom 3d stuff that will write to the z-buffer (such as decals)
    //   BeginMidPostEffects()
    //     Render any post-effects that you want to take place before distortion.
    //     For example, if you want distorted materials to distort the fogged objects
    //     behind them, render the fog.
    //   EndMidPostEffects()
    // EndCustomRender()
    // Render custom 3d stuff that will not write to the z-buffer (such as particles)
    // BeginPostEffects()
    //   Render any other post effects that you like
    // EndPostEffects()

    void    BeginNormalRender       ( void ) X_SECTION( render_infrequent );
    void    EndNormalRender         ( void ) X_SECTION( render_deferred );
    void    BeginCustomRender       ( void ) X_SECTION( render_infrequent );
    void    EndCustomRender         ( void ) X_SECTION( render_infrequent );    // was render_deferred...moved so render_deferred fits in i-cache
    void    ResetAfterException     ( void ) X_SECTION( render_infrequent );
    void    AddRigidInstanceSimple  ( hgeom_inst        hInst,
                                      const void*       pCol,
                                      const matrix4*    pL2W,   // will be DMA ref'd to!
                                      const bbox&       WorldBBox,
                                      u32               Flags ) X_SECTION( render_add );
    void    AddRigidInstance        ( hgeom_inst        hInst,
                                      const void*       pCol,
                                      const matrix4*    pL2W,
                                      u64               Mask,
                                      u32               Flags,
                                      s32               Alpha ) X_SECTION( render_add );
    void    AddRigidInstance        ( hgeom_inst        hInst,
                                      const void*       pCol,
                                      const matrix4*    pL2W,
                                      u64               Mask,
                                      u32               VTextureMask,
                                      u32               Flags,
                                      s32               Alpha ) X_SECTION( render_add );
    void    AddSkinInstance         ( hgeom_inst        hInst,
                                      const matrix4*    pBone,
                                      u64               Mask,
                                      u32               VTextureMask,
                                      u32               Flags,
                                      const xcolor&     Ambient ) X_SECTION( render_add );
    void    AddSkinInstanceDistorted( hgeom_inst        hInst,
                                      const matrix4*    pBone,
                                      u64               Mask,
                                      u32               Flags,
                                      const radian3&    NormalRot,
                                      xcolor            Ambient ) X_SECTION( render_add );

    // material access
    material& GetMaterial           ( hgeom_inst        hInst,
                                      s32               iSubMesh ) X_SECTION( render_infrequent );
    texture*  GetVTexture           ( const geom*       pGeom,
                                      s32               iMaterial,
                                      s32               VTextureMask ) X_SECTION( render_infrequent );

    // env. map specification routines
    void    SetAreaCubeMap      ( const cubemap::handle&  CubeMap ) X_SECTION( render_infrequent );

    // post-effect rendering routines--the begin and end pair are required and
    // allow the platforms to do some optimization by sharing major screen
    // buffer work between the post-effects.
    enum post_screen_blend
    {
        SOURCE_MINUS_DEST = 0,
    };

    enum post_falloff_fn
    {
        FALLOFF_CONSTANT = 0,
        FALLOFF_LINEAR,
        FALLOFF_EXP,
        FALLOFF_EXP2,
        FALLOFF_CUSTOM,
        
        ///////////////////////////////////////////////////////////////////////
        // For all types of fogging, the final color is:
        // C = f*OrigColor + (1-f)*FogColor
        //
        // CONSTANT:
        //  f=(z>start)?1.0f-color.a:0
        //  Param1 = start
        //  
        // LINEAR:
        //  f=(end-z)/(end-start)
        //  Param1 = start
        //  Param2 = end
        //
        // EXP
        //  (d=(z-n)/(f-n), f=1/e^(d*density)
        //  Param1 = density
        //  Param2 = unused
        //
        // EXP2
        //  (d=(z-n)/(f-n), f=1/e^((d*density)*(d*density))
        //  Param1 = density
        //  Param2 = unused
        //
        // CUSTOM
        //  A bitmap palette is used to describe the fog pattern.
        ///////////////////////////////////////////////////////////////////////
    };

    // For the post-effects, colors are considered to be 128==1, and 255==2 so that we
    // can get oversaturate. The post effects are listed here in the order they will
    // occur in. This is done to maximize performance, since some operations can be
    // shared between post effects (such as filtering a screen down). If you need
    // to do the post-effects in a specific order, you'll need to put them in
    // their own Begin/End blocks, but be warned that performance won't be as good.

    // If you plan on using the zfog filter, but want to manually fog draw items
    // that do not write to the z-buffer or happen outside of the post-effects, use
    // these functions.
    void    SetCustomFogPalette ( const texture::handle& Texture,
                                  xbool                  ImmediateSwitch,
                                  s32                    PaletteIndex ) X_SECTION( render_post );
    xcolor  GetFogValue         ( const vector3&         WorldPos,
                                  s32                    PaletteIndex ) X_SECTION( render_infrequent );

    // Use these functions for doing the actual post-effect
    void    BeginMidPostEffects ( void ) X_SECTION( render_post );              // see comments above render::BeginNormalRender
    void    EndMidPostEffects   ( void ) X_SECTION( render_deferred_post );     // see comments above render::BeginNormalRender
    void    BeginPostEffects    ( void ) X_SECTION( render_post );
    void    EndPostEffects      ( void ) X_SECTION( render_deferred_post );
    void    AddScreenWarp       ( const vector3&            WorldPos,
                                  f32                       Radius,
                                  f32                       WarpAmount )                X_SECTION( render_post );
    void    MotionBlur          ( f32                       Intensity )                 X_SECTION( render_post );
    void    ApplySelfIllumGlows ( f32                       MotionBlurIntensity = 0.0f,
                                  s32                       GlowCutoff = 255 )          X_SECTION( render_post );
    void    MultScreen          ( xcolor                    MultColor,
                                  post_screen_blend         FinalBlend )                X_SECTION( render_post );
    void    RadialBlur          ( f32                       Zoom,
                                  radian                    Angle,
                                  f32                       AlphaSub, 
                                  f32                       AlphaScale )                X_SECTION( render_post );
    void    ZFogFilter          ( post_falloff_fn           Fn,
                                  xcolor                    Color,
                                  f32                       Param1,
                                  f32                       Param2 )                    X_SECTION( render_post );
    void    ZFogFilter          ( post_falloff_fn           Fn,
                                  s32                       PaletteIndex )              X_SECTION( render_post );
    void    MipFilter           ( s32                       nFilters,
                                  f32                       Offset,
                                  post_falloff_fn           Fn,
                                  xcolor                    Color,
                                  f32                       Param1, 
                                  f32                       Param2, 
                                  s32                       PaletteIndex )              X_SECTION( render_post );
    void    MipFilter           ( s32                       nFilters,
                                  f32                       Offset,
                                  post_falloff_fn           Fn,
                                  const texture::handle&    Texture,
                                  s32                       PaletteIndex )              X_SECTION( render_post );
    void    NoiseFilter         ( xcolor                    Color )                     X_SECTION( render_post );
    void    ScreenFade          ( xcolor                    Color )                     X_SECTION( render_post );

    // Functions for locking the display list. These functions are here specifically for
    // lighting the level in the editor, and will only work on the PC. If you feel like
    // you need to lock display lists for general purpose vertex mucking around, then
    // the system needs to be rethought.
#ifdef TARGET_PC
    void*   LockRigidDListVertex    ( hgeom_inst hInst, s32 iSubMesh )                      X_SECTION( render_infrequent );
    void    UnlockRigidDListVertex  ( hgeom_inst hInst, s32 iSubMesh )                      X_SECTION( render_infrequent );
    void*   LockRigidDListIndex     ( hgeom_inst hInst, s32 iSubMesh,  s32& VertexOffset )  X_SECTION( render_infrequent );
    void    UnlockRigidDListIndex   ( hgeom_inst hInst, s32 iSubMesh )                      X_SECTION( render_infrequent );
#endif

    // Filter lighting functions
    void    EnableFilterLight       ( xbool  bEnable )      X_SECTION( render_infrequent );
    xbool   IsFilterLightEnabled    ( void )                X_SECTION( render_infrequent );
    void    SetFilterLightColor     ( xcolor Color   )      X_SECTION( render_infrequent );
    xcolor  GetFilterLightColor     ( void )                X_SECTION( render_infrequent );

    // Stats logging for the renderer...some of these stats should be collected per frame.
    // let the the stats class know when to begin and end collecting by using the Begin() and
    // End() call. The other functions should be self-explanatory.
    #if ENABLE_RENDER_STATS
    class stats
    {
    public:
        enum
        {
            OUTPUT_TO_DEBUG = 0,
            OUTPUT_TO_SCREEN,
            OUTPUT_TO_FILE,
        };

        enum
        {
            FLAG_VERBOSE = 0x0001,
        };

         stats                  ( void )                        X_SECTION( render_stats );
        ~stats                  ( void )                        X_SECTION( render_stats );

        void    Begin           ( void )                        X_SECTION( render_stats );
        void    End             ( void )                        X_SECTION( render_stats );
        void    Print           ( s32 Mode  = OUTPUT_TO_SCREEN,
                                  s32 Flags = 0 )               X_SECTION( render_stats );
        void    ClearAllStats   ( void )                        X_SECTION( render_stats );

        // simple counting stats (kept per frame)
        s32             m_nMaterialsRendered;
        s32             m_nInstancesRendered;
        s32             m_nSubMeshesRendered;
        s32             m_nVerticesRendered;
        s32             m_nTrisRendered;

        // times in xtimer ticks
        xtick           m_InstanceSortTime;
        xtick           m_MaterialActivateTime;
        xtick           m_InstanceAddTime;
        xtick           m_TotalEndRenderTime;
    
        // accumulated worst-case statistics
        s32             m_MaxMaterialsRendered;
        s32             m_MaxInstancesRendered;
        s32             m_MaxSubMeshesRendered;
        s32             m_MaxVerticesRendered;
        s32             m_MaxTrisRendered;
        xtick           m_MaxInstanceSortTime;
        xtick           m_MaxMaterialActivateTime;
        xtick           m_MaxInstanceAddTime;
        xtick           m_MaxTotalEndRenderTime;
    };

    // and I suppose you want a way to access this stat class?
    stats&  GetStats        ( void )    X_SECTION( render_stats );
    #endif


    // New session methods
    void BeginSession   ( u32 nPlayers )    X_SECTION( init );
    void EndSession     ( void )            X_SECTION( init );
}

//=============================================================================

struct color_info
{
    color_info( fileio& File );

    color_info( void )
    {
        Init( );
    }

    //  -----------------------------------------------------------------------

    enum usage{ kUse32,kUse16,kUnknown };

    static usage Usage( void )
    {
        return m_Usage;
    }

    //  -----------------------------------------------------------------------

    void SetCount( u32 Count )
    {
        m_nColors = Count;
    }

    u32 GetCount( void )const
    {
        return m_nColors;
    }

    //  -----------------------------------------------------------------------

    void Set( u32* pColor32 )
    {
        m_pColor32 = pColor32;
        m_Usage    = kUse32;
    }

    void Set( u16* pColor16 )
    {
        m_pColor16 = pColor16;
        m_Usage    = kUse16;
    }

    void Set( void* pVoid )
    {
        m_Usage = kUnknown;
        m_pVoid = pVoid;
    }

    //  -----------------------------------------------------------------------

    operator void*( void )
    {
        return m_pVoid;
    }

    operator u32*( void )
    {
        ASSERT( m_Usage==kUse32 );
        return m_pColor32;
    }

    operator u16*( void )
    {
        ASSERT( m_Usage==kUse16 );
        return m_pColor16;
    }

    //  -----------------------------------------------------------------------

    void FileIO     ( fileio& );
    void Init       ( void    );

    //  -----------------------------------------------------------------------

private:

    s32             m_nColors;
    static usage    m_Usage;
    union
    {
        u32 *       m_pColor32;   // 32-bit color
        u16 *       m_pColor16;   // 16-bit color
        void*       m_pVoid;
    };

#ifdef TARGET_XBOX
public:
    vert_factory::handle m_hColors;
#endif
};

//=========================================================================

inline void color_info::FileIO( fileio& File )
{
    File.Static( m_nColors );

    switch( m_Usage )
    {
        case kUse32:
		    #if defined(TARGET_XBOX)
                File.Dynamic( m_pColor32,m_nColors );
            #elif defined(TARGET_PC)
                File.Static( m_pColor32,m_nColors );
            #endif
            break;

        case kUse16:
            File.Static( m_pColor16,m_nColors );
            break;

        default:
            ASSERT(0);
            break;
    }
}

//=============================================================================

inline void color_info::Init( void )
{
#ifdef TARGET_XBOX
    m_hColors = 0;
    m_Usage   = kUse32;
#elif defined(TARGET_PC)
    m_Usage   = kUse32;
#else
    m_Usage   = kUnknown;
#endif
    m_nColors = 0;
    m_pVoid   = 0;
}

//=============================================================================
// EXTERNS FOR DEBUGGING
//=============================================================================

#ifndef X_RETAIL
extern render::debug_options  g_RenderDebug;
#endif

//=============================================================================

#endif // RENDER_HPP
