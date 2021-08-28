
#ifndef RAW_MATERIAL2_HPP
#define RAW_MATERIAL2_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_files.hpp"
#include "RawAnim.hpp"

struct rawmaterial;

//=========================================================================
// CLASS
//=========================================================================
struct rawmaterial2
{
//=========================================================================

    enum max
    {
        VERTEX_MAX_UV               = 8,
        VERTEX_MAX_NORMAL           = 4,
        VERTEX_MAX_COLOR            = 4,
        VERTEX_MAX_WEIGHT           = 16,
        FACET_MAX_VERTICES          = 8,
        MATERIAL_MAX_MAPS           = 16,
        MATERIAL_MAX_CONSTANTS      = 8,
        PARAM_PKG_MAX_ITEMS         = 4,
    };

//=========================================================================

    struct sub_mesh
    {
        char                Name[256];
        xbool               UsesMat[256];
    };

//=========================================================================

    enum blend_type
    {
        BLEND_TYPE_OVERWRITE,
        BLEND_TYPE_ADD,
        BLEND_TYPE_SUB,
        BLEND_TYPE_MUL,
        BLEND_TYPE_PAD = 0xffffffff
    };

    enum lighting_type
    {
        LIGHTING_TYPE_STATIC,
        LIGHTING_TYPE_DYNAMIC,
        LIGHTING_TYPE_STATIC_AND_DYNAMIC,
        LIGHTING_TYPE_SELF_ILLUM,
        LIGHTING_TYPE_PAD = 0xffffffff
    };

    enum tint_type
    {   
        TINT_TYPE_NONE,
        TINT_TYPE_FIRST_DIFFUSE,
        TINT_TYPE_FINAL_DIFFUSE_OUTPUT,
        TINT_TYPE_FINAL_OUTPUT,
        TINT_TYPE_CUSTOM1,
        TINT_TYPE_CUSTOM2,
        TINT_TYPE_PAD = 0xffffffff
    };

    enum address_type
    {
        ADDRESS_TYPE_WRAP,
        ADDRESS_TYPE_CLAMP,
        ADDRESS_TYPE_MIRROR,
        ADDRESS_TYPE_PAD = 0xffffffff
    };

    enum filter_type
    {
        FILTER_TYPE_BILINEAR,
        FILTER_TYPE_TRILINEAR,
        FILTER_TYPE_POINT,
        FILTER_TYPE_ANISOTROPIC,
        FILTER_TYPE_PAD = 0xffffffff
    };

    enum anim_type
    {
        ANIM_TYPE_CLAMP,
        ANIM_TYPE_CYCLE,
        ANIM_TYPE_PING_PONG,
        ANIM_TYPE_LINEAR,
        ANIM_TYPE_RELATIVE_REPEAT,
        ANIM_TYPE_PAD = 0xffffffff
    };

    enum rgba_source_type
    {
        RGBA_SOURCE_TYPE_RGB,
        RGBA_SOURCE_TYPE_R,
        RGBA_SOURCE_TYPE_G,
        RGBA_SOURCE_TYPE_B,
        RGBA_SOURCE_TYPE_A,
        RGBA_SOURCE_TYPE_PAD = 0xffffffff
    };

//=========================================================================

    struct texture
    {
        char                FileName[256];
    };

//=========================================================================

    struct param_pkg
    {
        // ???????? Start and end frames?

        s32             nKeys;
        s32             nParamsPerKey;
        s32             iStartFrame;
        s32             iFirstKey;
        s32             FPS;
        anim_type       ModeType[2];
        f32             Current[PARAM_PKG_MAX_ITEMS];
    };

//=========================================================================

    struct map
    {
        rgba_source_type    RGBASource;

        s32                 iUVChannel;         // Index to UVChanel
        address_type        UAddress;           // Adress for U ( Mirror,Clamp, ..etc )
        address_type        VAddress;

        filter_type         FilterType;         // The type of filter to use

        param_pkg           UVTranslation;      // UV animation data
        param_pkg           UVRotation;
        param_pkg           UVScale;

        s32                 iTexture;           // Index to the first texture
        s32                 nTextures;          // Number of frames in texture animation
        s32                 TextureFPS;         // Rate of texture anim playback
    };

//=========================================================================

    struct material
    {
        char                Name[256];
        s32                 Type;
        s32                 Sort;

        lighting_type       LightingType;
        blend_type          BlendType;
        tint_type           TintType;

        xbool               bTwoSided;
        xbool               bPunchthrough;
        xbool               bRandomizeAnim;

        param_pkg           TintColor;
        param_pkg           TintAlpha;
        param_pkg           Constants[ MATERIAL_MAX_CONSTANTS ];

        s32                 nMaps;
        map                 Map[ MATERIAL_MAX_MAPS ];

        s32                 iFirstKey;          // offset to first f32 used by this mat
        s32                 nKeys;              // total # of f32's used by this materials pkgs
    };

//=========================================================================

     rawmaterial2( void );
    ~rawmaterial2( void );

    xbool   Load                ( const char* pFileName );
    void    Kill                ( void );
    void    BuildFromRM         ( rawmaterial&  RM );
    
    void    GetMangledName      ( rgba_source_type  iSource,        // Map that controls mangling
                                  const char*       pSourceName,    // Source filename
                                        char*       pDestBuffer,    // Dest buffer for mangled name
                                  s32               nMaxChars );    // Max # chars in dest buffer                                 

//=========================================================================



    s32             m_nTextures;
    texture*        m_pTexture;

    s32             m_nMaterials;
    material*       m_pMaterial;

    s32             m_nSubMeshs;
    sub_mesh*       m_pSubMesh;

    s32             m_nParamKeys;
    f32*            m_pParamKey;

};

//=========================================================================
#endif

































