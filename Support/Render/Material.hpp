#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "Texture.hpp"
#include "Material_Prefs.hpp"

//=============================================================================
// Shader library file

#ifdef TARGET_XBOX

struct shader
{
    shader( u32,XGBuffer&,u32* );
    shader( void ){}
~   shader( void );

    XGBuffer Microcode;
    u32 Handle;
    u32 Size;
    u32 Id;
};

namespace ps
{
    union desc
    {
        // first 32-bits of union are the
        // shader flags. the next are the
        // shader handle.

        struct
        {
            u64 Mask; // makes sure we cover all our flags!
        };

        ///////////////////////////////////////////////////////////////////////

        // assignment operators

        void operator = ( desc PSFlags )
        {
            Mask = PSFlags.Mask;
        }

        // clear. this routine blows out
        // all the flags in one go.

        void clear( void )
        {
            Mask = 0;
        }

        ///////////////////////////////////////////// must occupy first 32-bits

        // base pixel shaders. this
        // group represents each of
        // the game material types.

        struct
        {
            u64 bDiffusePerPixelIllum     :1; // 00 "PS0000.PSH"
            u64 bDiffusePerPixelEnvAdd    :1; // 01 "PS0001.PSH"
            u64 bDiffusePerPixelEnv       :1; // 02 "PS0002.PSH"
            u64 bAlphaPerPixelIllum       :1; // 03 "PS0003.PSH"
            u64 bAlphaPerPolyIllum        :1; // 04 "PS0004.PSH"
            u64 bAlphaPerPolyEnv          :1; // 05 "PS0005.PSH"
            u64 bDiffuse                  :1; // 06 "PS0007.PSH"
            u64 bAlpha                    :1; // 07 "PS0008.PSH"

            u64 bLit_DiffusePerPixelIllum :1; // 08 "PS0013.PSH"
            u64 bLit_DiffusePerPixelEnvAdd:1; // 09 "PS0014.PSH"
            u64 bLit_DiffusePerPixelEnv   :1; // 10 "PS0015.PSH"
            u64 bLit_AlphaPerPixelIllum   :1; // 11 "PS0016.PSH"
            u64 bLit_AlphaPerPolyIllum    :1; // 12 "PS0017.PSH"
            u64 bLit_AlphaPerPolyEnv      :1; // 13 "PS0018.PSH"
            u64 bLit_Diffuse              :1; // 14 "PS0021.PSH"
            u64 bLit_Alpha                :1; // 15 "PS0022.PSH"
        };

        // group flags ********************************************************

        struct
        {
            u64 MaterialID           :16;
            u64 bDetailMap           : 1; // 16 "PS0006.PSH" add detail map
            u64 bDepthBlur           : 1; // 17 "PS0028.PSH" depth blur
            u64 bRgbaByTex0          : 1; // 18 "PS0033.PSH" diffuse & a * tex0
            u64 bDecal0              : 1; // 19 "PS0034.PSH" intensity mapping
            u64 bDecal1              : 1; // 20 "PS0035.PSH" intensity mapping
            u64 bPerPixelLit         : 1; // 21 "PS0032.PSH" per pixel lighting
            u64 bPerPixelLitFog      : 1; // 22 "PS0044.PSH" ppl + fogging
            u64 bPerPixelLitProj     : 1; // 23 "PS0040.PSH" ppl + flashlight
            u64 bPerPixelLitPunchthru: 1; // 24 "PS0041.PSH" ppl + punchthru alpha
            u64 bDistortion          : 1; // 25 distortion fx
            u64 bDistortEnv          : 1; // 26 distortion  + env
            u64 bForcedGlow          : 1; // 27 forced self illum
            u64 bTexelModded         : 1; // 28 Shadow blurring
            u64 bTexelOnly           : 1; // 29 Load texel only
            u64 bFogPass             : 1; // 30 Fogging setup
            u64 bShadow              : 1; // 31 Shadow setup
            u64 bProj                : 1; // 32 Flashlight
        };

        // intensity flags ****************************************************

        struct
        {
            u64        Pad:19;
            u64      Decal: 2;
            u64   LightMap: 4;
            u64 Distortion :2;
        };

        ////////////////////////////////////////////////////////// must be last

        // shadow flags *******************************************************

        struct
        {
            u64       Pad :33;
            u64 oT1Shadow : 1; // 33 "PS0024.PSH" Stage 1 shadow
            u64 oT2Shadow : 1; // 34 "PS0025.PSH" Stage 2 shadow
            u64 oT3Shadow : 1; // 35 "PS0026.PSH" Stage 3 shadow
            u64 oT0Shadow : 1; // 36 "PS0027.PSH" Stage 0 shadow
            u64 oT1Project: 1; // 37 "PS0037.PSH" Projective shadow on stage 1
            u64 oT2Project: 1; // 38 "PS0038.PSH" Projective shadow on stage 1 & 2
            u64 oT1LitProj: 1; // 39 "PS0042.PSH" Projective shadow on stage 1     to lightmap
            u64 oT2LitProj: 1; // 40 "PS0043.PSH" Projective shadow on stage 1 & 2 to lightmap
        };
        struct
        {
            u64     Pad:33;
            u64 iShadow: 8; // 33 Shadow index
        };

        // light map flags ****************************************************

        struct
        {
            u64         Pad:41;
            u64 oT1LightMap: 1; // 41 "PS0033.PSH" Stage 1 light map
            u64 oT2LightMap: 1; // 42 "PS0034.PSH" Stage 2 light map
            u64 oT3LightMap: 1; // 43 "PS0035.PSH" Stage 3 light map
            u64 oT0LightMap: 1; // 44 "PS0036.PSH" Stage 0 light map
        };
        struct
        {
            u64       Pad:41;
            u64 iLightMap: 4; // 41 Lightmap index
        };

        // Bltter flags *******************************************************

        struct
        {
            u64   Pad:45;
            u64 BltId :4;
        };
        struct
        {
            u64        Pad:45;
            u64 bBltPostFx: 1; // 45 final post effect
            u64 bBltFinal : 1; // 46 final colourisation
            u64 bBltx2    : 1; // 47 two stage blitter
            u64 bBlt      : 1; // 48 standard blitter
        };

        // Final combiner *****************************************************

        struct
        {
            u64   Pad:49;
            u64 XfcId: 2;
        };
        struct
        {
            u64     Pad:49;
            u64 xfc_Fog: 1; // 49 Fogged final combiner
            u64 xfc_Std: 1; // 50 Standard fc
            u64 xfc_Sat: 1; // 51 Saturate
            u64 xfc_Half:1; // 52 Halve
        };

        // Late flags (saves rebuilding link tables) **************************

        struct
        {
            u64 Pad:53;
            u64 bLitProjPunchthru    :1; // 53 Second lit punchthru pass
            u64 bFogNoPpl            :1; // 54 Second fog pass [class]
            u64 oT3_Mask             :1; // 55 Mask out T3
            u64 oT3_Proj             :1; // 56 Flashlight
            u64 oV0_Tint             :1; // 57 Tint
            u64 bDiffusePerPolyEnvAdd:1; // 58
            u64 bDiffusePerPolyEnv   :1; // 59
            u64 bApplyZFog           :1; // 60
            u64 bShadowZFog          :1; // 61
            u64 bFogPunchthru        :1; // 62
            u64 bCloth               :1; // 63 Cloth fogging
        };
    };

    struct path
    {
        void compile( const char*,shader& Result );

        path( void ){}
        path( desc& );

        desc Flags;
        s32  iLink[13];
        u32  ts,id;
    };
}

namespace vs
{
    union desc
    {
        // first 32-bits of union are the
        // vertex flags. the next are the
        // shader handle.

        struct
        {
            u32 Mask;
        };

        // each member is a group to which a vertex shader
        // belongs. these are stitched together at runtime
        // to the member it indexes into. much faster than
        // before and gets rid of mismatching

        struct
        {
            u32 oPos       :2;
            u32 oD0        :5;
            u32 oT0        :5;
            u32 oT3        :1;
            u32 oSt        :2;
            u32 iLightMap  :4;
            u32 iShadow    :7;
            u32 oD1        :1;
            u32 oBlt       :1;
        };

        // individual vertex shaders. this map
        // tells us which features this shader
        // fragment actually supports.

        struct
        {
            u32 oPos_Rigid         :1; // 00 "VS0000.VSH"
            u32 oPos_Skin          :1; // 01 "VS0001.VSH"
            u32 oD0_DirectLight    :1; // 02 "VS0002.VSH"
            u32 oD0_PointLight     :1; // 03 "VS0003.VSH"
            u32 oD0_BlackLight     :1; // 04 "VS0004.VSH"
            u32 oD0_WhiteLight     :1; // special blender
            u32 oD0_Diffuse        :1; // 06 "VS0005.VSH"
            u32 oT0_Normal         :1; // 07 "VS0007.VSH"
            u32 oT0_Cube           :1; // 08 "VS0008.VSH"
            u32 oT0_Env            :1; // 09 "VS0009.VSH"
            u32 oT0_Distortion     :1; // 10 "VS0017.VSH"
            u32 oT0_Intensity      :1; // 11 "VS0025.VSH"
            u32 oT3_Projection     :1; // 12 "VS0010.VSH"
            u32 oSt_CastShadowRigid:1; // 13 "VS0011.VSH"
            u32 oSt_CastShadowSkin :1; // 14 "VS0012.VSH"
            u32 oT0_LightMapCreate :1; // 15 "VS0020.VSH"
            u32 oT1_LightMapInsert :1; // 16 "VS0022.VSH"
            u32 oT2_LightMapInsert :1; // 17 "VS0023.VSH"
            u32 oT3_LightMapInsert :1; // 18 "VS0024.VSH"
            u32 oT0_ShadowCreate   :1; // 19 "VS0016.VSH"
            u32 oT1_ShadowInsert   :1; // 20 "VS0013.VSH"
            u32 oT2_ShadowInsert   :1; // 21 "VS0014.VSH"
            u32 oT3_ShadowInsert   :1; // 22 "VS0015.VSH"
            u32 oT1_ShadowProject  :1; // 23 "VS0026.VSH"
            u32 oT2_ShadowProject  :1; // 24 "VS0027.VSH"
            u32 oT0_ProjectBack    :1; // 25 "VS0028.VSH" for projective shadows
            u32 oD1_Fog            :1; // 26 "VS0018.VSH"
            u32 Blitter            :1; // 27 "VS0019.VSH"
            u32 bIntensityMap      :1; // 28 special blender
            u32 bPunchthru         :1; // 29 special pplflag
            u32 bDetailMap         :1; // 30 special detail
            u32 oT3_Mask           :1; // 31 Special masking
        };

        // assignment operators

        void operator = ( desc VSFlags )
        {
            Mask = VSFlags.Mask;
        }

        // clear. this routine blows out
        // all the flags in one go.

        void clear( void )
        {
            Mask = 0;
        }
    };

    struct path
    {
        void compile( const char*,shader& Result );

        path( void ){}
        path( desc& );

        desc Flags;
        s32  iLink[9];
    };
}

#endif

//=============================================================================

class material
{
public:

    struct uvanim
    {
        f32     CurrentFrame;   // current frame as float
        s16     iKey;           // offset into geometry
        s8      iFrame;         // current frame as int
        s8      Dir;            // direction of animation {-1,0,1}
        s8      Type;           // type of animation
        s8      nFrames;        // total number of frames in animation
        s8      FPS;            // frames per second to run this animation
        s8      StartFrame;     // starting frame for this animation
    };

    material            ( void );
   ~material            ( void );
    xbool   operator==  ( material& RHS ) const;

    xbool               HasUVAnimation  ( void ) const;
    s32                 AddRef          ( void );
    s32                 Release         ( void );
    s32                 GetRefCount     ( void ) const;

    s8                  m_Type;
    f32                 m_DetailScale;
    f32                 m_FixedAlpha;
    u16                 m_Flags;                          // flags

    texture::handle     m_DiffuseMap;
    texture::handle     m_EnvironmentMap;
    texture::handle     m_DetailMap;
    uvanim              m_UVAnim;
    s32                 m_RefCount;

#ifdef TARGET_PS2
    vector4             m_EnvMapVectors[2];
    u64                 m_DiffuseAlphaReg;
    u64                 m_EnvIllumAlphaAddr;
    u64                 m_EnvIllumAlphaReg;
    u32                 m_MCodeFlags;
    u32                 m_PerPolyAlpha;

    s32                 m_nDiffuseMips;
    s32                 m_nDetailMips;

    texture*            m_pDiffuseTex;
    texture*            m_pEnvTex;
    texture*            m_pDetailTex;
#endif
};

//=============================================================================

inline xbool material::HasUVAnimation( void ) const
{
    return( m_UVAnim.nFrames > 0 );
}

//=============================================================================

inline s32 material::AddRef( void )
{
    return (++m_RefCount);
}

//=============================================================================

inline s32 material::Release( void )
{
    return (--m_RefCount);
}

//=============================================================================

inline s32 material::GetRefCount( void ) const
{
    return m_RefCount;
}

//=============================================================================

#endif
