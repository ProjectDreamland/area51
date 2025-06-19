#ifndef PLATFORM_RENDER_HPP
#define PLATFORM_RENDER_HPP

#include "Render\Render.hpp"

//=============================================================================
//=============================================================================
// Struct functions
//=============================================================================
//=============================================================================

    struct render_instance;

//=============================================================================

// These functions are for internal rendering use, and you should never call
// them directly. If you need to, then there is a flaw in the system.

//=============================================================================
// init/kill functions
//=============================================================================
static void     platform_Init                       ( void )                        X_SECTION( init );
static void     platform_Kill                       ( void )                        X_SECTION( init );
static void     platform_BeginSession               ( u32 nPlayers )                X_SECTION( init );
static void     platform_EndSession                 ( void )                        X_SECTION( init );
static void     platform_RegisterMaterial           ( material&          Mat   )    X_SECTION( init );
static void     platform_RegisterSkinGeom           ( skin_geom&         Geom  )    X_SECTION( init );
static void     platform_RegisterRigidGeom          ( rigid_geom&        Geom  )    X_SECTION( init );
static void     platform_UnregisterSkinGeom         ( skin_geom&         Geom  )    X_SECTION( init );
static void     platform_UnregisterRigidGeom        ( rigid_geom&        Geom  )    X_SECTION( init );
static void     platform_RegisterRigidInstance      ( rigid_geom&        Geom,
                                                      render::hgeom_inst hInst )    X_SECTION( init );
static void     platform_RegisterSkinInstance       ( skin_geom&         Geom,
                                                      render::hgeom_inst hInst )    X_SECTION( init );
static void     platform_UnregisterRigidInstance    ( render::hgeom_inst hInst )    X_SECTION( init );
static void     platform_UnregisterSkinInstance     ( render::hgeom_inst hInst )    X_SECTION( init );

//=============================================================================
// functions for rendering raw data
//=============================================================================
static void     platform_StartRawDataMode           ( void )                        X_SECTION( render_raw );
static void     platform_EndRawDataMode             ( void )                        X_SECTION( render_raw );
static void     platform_RenderRawStrips            ( s32               nVerts,
                                                      const matrix4&    L2W,
                                                      const vector4*    pPos,
                                                      const s16*        pUV,
                                                      const u32*        pColor )    X_SECTION( render_raw );
static void     platform_Render3dSprites            ( s32               nSprites,
                                                      f32               UniScale,
                                                      const matrix4*    pL2W,
                                                      const vector4*    pPositions,
                                                      const vector2*    pRotScales,
                                                      const u32*        pColors )   X_SECTION( render_raw );
static void     platform_RenderVelocitySprites      ( s32               nSprites,
                                                      f32               UniScale,
                                                      const matrix4*    pL2W,
                                                      const matrix4*    pVelMatrix,
                                                      const vector4*    pPositions,
                                                      const vector4*    pVelocities,
                                                      const u32*        pColors )   X_SECTION( render_raw );
static void     platform_RenderHeatHazeSprites      ( s32               nSprites,
                                                      f32               UniScale,
                                                      const matrix4*    pL2W,
                                                      const vector4*    pPositions,
                                                      const vector2*    pRotScales,
                                                      const u32*        pColors )   X_SECTION( render_raw );
static void     platform_SetDiffuseMaterial         ( const xbitmap&    Bitmap,
                                                      s32               BlendMode,
                                                      xbool             ZTestEnabled )  X_SECTION( render_raw );
static void     platform_SetGlowMaterial            ( const xbitmap&    Bitmap,
                                                      s32               BlendMode,
                                                      xbool             ZTestEnabled )  X_SECTION( render_raw );
static void     platform_SetEnvMapMaterial          ( const xbitmap&    Bitmap,
                                                      s32               BlendMode,
                                                      xbool             ZTestEnabled )  X_SECTION( render_raw );
static void     platform_SetDistortionMaterial      ( s32               BlendMode,
                                                      xbool             ZTestEnabled )  X_SECTION( render_raw );

//=============================================================================
// material functions
//=============================================================================
static void     platform_ActivateMaterial           ( const material& Material  )   X_SECTION( render_deferred );
static void     platform_ActivateDistortionMaterial ( const material* pMaterial,
                                                      const radian3&  NormalRot )   X_SECTION( render_infrequent );
static void     platform_ActivateZPrimeMaterial     ( void                      )   X_SECTION( render_infrequent );

//=============================================================================
// runtime lighting functions
//=============================================================================
static void*    platform_CalculateRigidLighting     ( const matrix4&   L2W,
                                                      const bbox&      WorldBBox )  X_SECTION( render_add );
static void*    platform_CalculateSkinLighting      ( u32              Flags,
                                                      const matrix4&   L2W,
                                                      const bbox&      BBox,
                                                      xcolor           Ambient )    X_SECTION( render_add );

//=============================================================================
// rendering functions
//=============================================================================
static void     platform_BeginRigidGeom             ( geom*            pGeom,
                                                      s32              iSubMesh )   X_SECTION( render_deferred );
static void     platform_RenderRigidInstance        ( render_instance& Inst     )   X_SECTION( render_deferred );
static void     platform_EndRigidGeom               ( void                      )   X_SECTION( render_deferred );
static void     platform_BeginSkinGeom              ( geom*            pGeom,
                                                      s32              iSubMesh )   X_SECTION( render_deferred );
static void     platform_RenderSkinInstance         ( render_instance& Inst )       X_SECTION( render_deferred );
static void     platform_EndSkinGeom                ( void )                        X_SECTION( render_deferred );



//=============================================================================
// shader/render setup functions
//=============================================================================
static void     platform_BeginShaders               ( void )    X_SECTION( render_infrequent );
static void     platform_EndShaders                 ( void )    X_SECTION( render_infrequent );
static void     platform_CreateEnvTexture           ( void )    X_SECTION( render_infrequent );
static void     platform_BeginDistortion            ( void )    X_SECTION( render_infrequent );
static void     platform_EndDistortion              ( void )    X_SECTION( render_infrequent );

//=============================================================================
// projected textures and shadow setup
//=============================================================================
static void     platform_SetProjectedTexture        ( texture::handle           Texture )       X_SECTION( render_infrequent );
static void     platform_ComputeProjTextureMatrix   ( matrix4&                  Matrix,
                                                      view&                     View,
                                                      const texture_projection& Projection  )   X_SECTION( render_infrequent );
static void     platform_SetTextureProjection       ( const texture_projection& Projection  )   X_SECTION( render_infrequent );
static void     platform_SetTextureProjectionMatrix ( const matrix4&            Matrix      )   X_SECTION( render_infrequent );
static void     platform_SetProjectedShadowTexture  ( s32                       Index,
                                                      texture::handle           Texture     )   X_SECTION( render_infrequent );
static void     platform_ComputeProjShadowMatrix    ( matrix4&                  Matrix,
                                                      view&                     View,
                                                      const texture_projection& Projection  )   X_SECTION( render_infrequent );
static void     platform_SetShadowProjectionMatrix  ( s32                       Index,
                                                      const matrix4&            Matrix      )   X_SECTION( render_infrequent );

//=============================================================================
// projected shadows
//=============================================================================

static void     platform_ClearShadowProjectorList   ( void )                            X_SECTION( render_infrequent );
static void     platform_AddPointShadowProjection   ( const matrix4&         L2W,
                                                      radian                 FOV,
                                                      f32                    NearZ,
                                                      f32                    FarZ )     X_SECTION( render_add_shadow );
static void     platform_AddDirShadowProjection     ( const matrix4&         L2W,
                                                      f32                    Width,
                                                      f32                    Height,
                                                      f32                    NearZ,
                                                      f32                    FarZ )     X_SECTION( render_add_shadow );
static void     platform_BeginShadowShaders         ( void )                            X_SECTION( render_infrequent );
static void     platform_EndShadowShaders           ( void )                            X_SECTION( render_infrequent );
static void     platform_StartShadowCast            ( void )                            X_SECTION( render_infrequent );
static void     platform_EndShadowCast              ( void )                            X_SECTION( render_infrequent );
static void     platform_StartShadowReceive         ( void )                            X_SECTION( render_infrequent );
static void     platform_EndShadowReceive           ( void )                            X_SECTION( render_infrequent );
static void     platform_BeginShadowCastRigid       ( geom*            pGeom,
                                                      s32              iSubMesh )       X_SECTION( render_deferred_shadow );
static void     platform_RenderShadowCastRigid      ( render_instance& Inst     )       X_SECTION( render_deferred_shadow );
static void     platform_EndShadowCastRigid         ( void )                            X_SECTION( render_deferred_shadow );
static void     platform_BeginShadowCastSkin        ( geom*            pGeom,
                                                      s32              iSubMesh )       X_SECTION( render_deferred_shadow );
static void     platform_RenderShadowCastSkin       ( render_instance& Inst,
                                                      s32              iProj    )       X_SECTION( render_deferred_shadow );
static void     platform_EndShadowCastSkin          ( void )                            X_SECTION( render_deferred_shadow );
static void     platform_BeginShadowReceiveRigid    ( geom*            pGeom,
                                                      s32              iSubMesh )       X_SECTION( render_deferred_shadow );
static void     platform_RenderShadowReceiveRigid   ( render_instance& Inst,
                                                      s32              iProj    )       X_SECTION( render_deferred_shadow );
static void     platform_EndShadowReceiveRigid      ( void )                            X_SECTION( render_deferred_shadow );
static void     platform_BeginShadowReceiveSkin     ( geom*            pGeom,
                                                      s32              iSubMesh )       X_SECTION( render_deferred_shadow );
static void     platform_RenderShadowReceiveSkin    ( render_instance& Inst     )       X_SECTION( render_deferred_shadow );
static void     platform_EndShadowReceiveSkin       ( void )                            X_SECTION( render_deferred_shadow );

//=============================================================================
// post effects
//=============================================================================
static void     platform_SetCustomFogPalette        ( const texture::handle&    Texture,
                                                      xbool                     ImmediateSwitch,
                                                      s32                       PaletteIndex )  X_SECTION( render_post );
static xcolor   platform_GetFogValue                ( const vector3&            WorldPos,
                                                      s32                       PaletteIndex )  X_SECTION( render_infrequent );
static void     platform_InitPostEffects            ( void )                                    X_SECTION( init );
static void     platform_KillPostEffects            ( void )                                    X_SECTION( init );
static void     platform_BeginPostEffects           ( void )                                    X_SECTION( render_post );
static void     platform_AddScreenWarp              ( const vector3&            WorldPos,
                                                      f32                       Radius,
                                                      f32                       WarpAmount )    X_SECTION( render_post );
static void     platform_MotionBlur                 ( f32                       Intensity  )    X_SECTION( render_post );
static void     platform_ApplySelfIllumGlows        ( f32                       MotionBlurIntensity,
                                                      s32                       GlowCutoff )    X_SECTION( render_post );
static void     platform_MultScreen                 ( xcolor                    MultColor,
                                                      render::post_screen_blend FinalBlend )    X_SECTION( render_post );
static void     platform_RadialBlur                 ( f32                       Zoom,
                                                      radian                    Angle,
                                                      f32                       AlphaSub,
                                                      f32                       AlphaScale )    X_SECTION( render_post );
static void     platform_ZFogFilter                 ( render::post_falloff_fn   Fn,
                                                      xcolor                    Color,
                                                      f32                       Param1,
                                                      f32                       Param2     )    X_SECTION( render_post );
static void     platform_ZFogFilter                 ( render::post_falloff_fn   Fn,
                                                      s32                       PaletteIndex )  X_SECTION( render_post );
static void     platform_MipFilter                  ( s32                       nFilters,
                                                      f32                       Offset,
                                                      render::post_falloff_fn   Fn,
                                                      xcolor                    Color,
                                                      f32                       Param1,
                                                      f32                       Param2,
                                                      s32                       PaletteIndex )  X_SECTION( render_post );
static void     platform_MipFilter                  ( s32                       nFilters,
                                                      f32                       Offset,
                                                      render::post_falloff_fn   Fn,
                                                      const texture::handle&    Texture,
                                                      s32                       PaletteIndex )  X_SECTION( render_post );
static void     platform_NoiseFilter                ( xcolor                    Color      )    X_SECTION( render_post );
static void     platform_EndPostEffects             ( void )                                    X_SECTION( render_deferred_post );

//=============================================================================
// compilation/export functions
//=============================================================================
#if defined(X_EDITOR) || defined(CONFIG_VIEWER)
static void*    platform_LockRigidDListVertex       ( render::hgeom_inst hInst,
                                                      s32                iSubMesh     );
static void     platform_UnlockRigidDListVertex     ( render::hgeom_inst hInst,
                                                      s32                iSubMesh     );
static void*    platform_LockRigidDListIndex        ( render::hgeom_inst hInst,
                                                      s32                iSubMesh,
                                                      s32&               VertexOffset );
static void     platform_UnlockRigidDListIndex      ( render::hgeom_inst hInst,
                                                      s32                iSubMesh     );
#endif

//=============================================================================

#endif
