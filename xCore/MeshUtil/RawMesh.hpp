
#ifndef RAW_MESH_HPP
#define RAW_MESH_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_files.hpp"
#include "RawAnim.hpp"


//=========================================================================
// CLASS
//=========================================================================
struct rawmesh
{
//=========================================================================

     rawmesh( void );
    ~rawmesh( void );

    xbool   Load                ( const char* pFileName );
    void    Save                ( const char* pFileName );
    void    Kill                ( void );
    bbox    GetBBox             ( void );
    bbox    GetSubmeshBBox      ( s32 iSubmesh );
    void    SortFacetsByMaterial( void );
    void    SortFacetsByMaterialAndBone( void );
    void    CleanMesh           ( s32 iSubMesh = -1 );
    void    SanityCheck         ( void );
    void    CleanWeights        ( s32 MaxNumWeights, f32 MinWeightValue );
    void    CollapseMeshes      ( const char* pMeshName );
    void    CollapseNormals     ( radian ThresholdAngle = R_20 );
    void    ComputeMeshBBox     ( s32 iMesh, bbox& BBox );
    xbool   IsolateSubmesh      ( s32 iSubmesh, rawmesh& NewMesh ,
                                  xbool RemoveFromRawMesh = FALSE );
    xbool   IsolateSubmesh      ( const char* pMeshName, rawmesh& NewMesh );
    xbool   IsBoneUsed          ( s32 iBone );
    
    s32     GetBoneIDFromName   ( const char* pBoneName ) const;
    void    DeleteBone          ( s32 iBone );
    void    DeleteBone          ( const char* pBoneName );
    //void    DeleteDummyBones    ( void );           // Deletes all bones with "dummy" in the name
    
    void    ApplyNewSkeleton    ( rawanim& RawAnim );
    void    ApplyNewSkeleton    ( const rawmesh& Skel );

static void    SetRenameDuplicates ( xbool b );
//=========================================================================

    enum max
    {
        VERTEX_MAX_UV           = 8,
        VERTEX_MAX_NORMAL       = 8,
        VERTEX_MAX_COLOR        = 8,
        VERTEX_MAX_WEIGHT       = 16,

        MATERIAL_MAX_TEXTURES   = 8,
        MATERIAL_MAX_CONSTANTS  = 8,

        TEX_MATERIAL_MAX_REF    = 4,
    };

    enum address_mode
    {
        TEXTURE_NONE,
        TEXTURE_WRAP,
        TEXTURE_CLAMP,
        TEXTURE_MIRROR,
        TEXTURE_PAD = 0xffffffff
    };

    enum texture_type
    {
        TEX_TYPE_NONE,
        TEX_TYPE_BASE1,
        TEX_TYPE_BASE2,
        TEX_TYPE_BASE3,
        TEX_TYPE_BASE4,
        TEX_TYPE_BLEND_RGB,
        TEX_TYPE_BLEND_RG,
        TEX_TYPE_BLEND_R,
        TEX_TYPE_OPACITY_ALPHA,
        TEX_TYPE_OPACITY_PUNCH,
        TEX_TYPE_ENVIROMENT,
        TEX_TYPE_SPECULAR,
        TEX_TYPE_ANISOTROPIC,
        TEX_TYPE_BUMP,
        TEX_TYPE_INTENSITY,
        TEX_TYPE_NORMAL,
        TEX_TYPE_SELF_ILLUM,
        TEX_TYPE_LIGHTMAP,
        TEX_TYPE_PAD = 0xffffffff
    };  
    
    enum illumination_type
    {
        ILLUM_TYPE_NONE,
        ILLUM_TYPE_DYNAMIC_VERTEX,
        ILLUM_TYPE_DYNAMIC_VERTEX_MONOCROM,
        ILLUM_TYPE_LIGHTMAP,
        ILLUM_TYPE_PAD = 0xffffffff
    };

    enum composition_type
    {
        COMPOSITION_TYPE_NONE,
        COMPOSITION_TYPE_OVERWRITE,
        COMPOSITION_TYPE_ADD,
        COMPOSITION_TYPE_SUB,
        COMPOSITION_TYPE_MUL,
        COMPOSITION_TYPE_PAD = 0xffffffff
    };

    enum material_type
    {
        MATERIAL_TYPE_NONE,            
        MATERIAL_TYPE_DEFAULT,            
        MATERIAL_TYPE_UVEYES,
        MATERIAL_TYPE_WATER,
        MATERIAL_TYPE_PAD = 0xffffffff
    };

    enum chanel_type
    {
        TEX_MATERIAL_NONE,
        TEX_MATERIAL_TEXTURE_AUTOGEN,
        TEX_MATERIAL_TEXTURE,
        TEX_MATERIAL_VERTEX,
        TEX_MATERIAL_CONSTANT,
        TEX_MATERIAL_PAD = 0xffffffff
    };

    enum uvwanim_type
    {
        UVWANIM_NONE,        
        UVWANIM_LOOP,
        UVWANIM_RELATIVE,
        UVWANIM_PAD = 0xffffffff
    };

    enum filter_type
    {
        FILTER_POINT,
        FILTER_BILINEAR,
        FILTER_TRILINEAR,
        FILTER_ANISOTROPIC,
        FILTER_PAD = 0xffffffff
    };

    enum texanim_type
    {
        TEXANIM_NONE,
        TEXANIM_FORWARD,
        TEXANIM_REVERSE,
        TEXANIM_PING_PONG,
        TEXANIM_RANDOM,
        TEXANIM_PAD = 0xffffffff
    };

    enum tinting_type
    {
        TINTING_NONE,
        TINTING_ADD,
        TINTING_MUL,
        TINTING_REPL,
        TINTING_PAD = 0xffffffff
    };


//=========================================================================

    struct weight
    {
        s32                 iBone;
        f32                 Weight;
    };

    struct vertex
    {
        vector3             Position;

        s32                 iFrame;
        s32                 nWeights;
        s32                 nNormals;
        s32                 nUVs;
        s32                 nColors;

        vector2             UV    [ VERTEX_MAX_UV     ];
        vector3             Normal[ VERTEX_MAX_NORMAL ];
        xcolor              Color [ VERTEX_MAX_COLOR  ];
        weight              Weight[ VERTEX_MAX_WEIGHT ];
    };

    struct facet
    {
        s32                 iMesh;
        s32                 nVertices;
        s32                 iVertex[16];
        s32                 iMaterial;
        plane               Plane;
    };

    struct texture
    {
        char                FileName[256];
    };

    struct uvanim_frame
    {
        s32                 ID;
        s32                 iFrame;

        vector3             Scale;
        vector3             Position;
        radian3             Rotation;
    };

    struct tex_material
    {
        chanel_type         ChanelType;         // Type of chanel
        s32                 iChanel;            // Index to: UVChanel, or VertexColor Chanel

        texture_type        TexType;            // If it is a texture what type is it.
        s32                 iTexture;           // Index to the texture

        address_mode        UAddress;           // Adress for U ( Mirror,Clamp, ..etc )
        address_mode        VAddress;
        address_mode        WAddress;

        filter_type         FilterType;         // The type of filter to use
        f32                 Bias;               // Changes the MIP LOD BIAS

        uvwanim_type        UVWAnimType;        // Type for the UVW animation
        s32                 nUVWAnimFrames;     // Number of frames of animation for the UVW
        s32                 iUVWAnimID;         // ID of the UVW animation

        texanim_type        TexAnimType;        // Type of texture animation
        f32                 TexAnimPlaySpeed;   // Speed for playing texture animations

        tinting_type        TintingType;        // Type for the tinting of the texture
        xcolor              Color;              // Color for the tinting or constant color
    };

    struct material
    {
        char                Name[256];

        material_type       Type;
        illumination_type   IllumType;
        composition_type    CompType;

        xbool               bDoubleSide;

        s32                 nConstans;
        f32                 K         [ MATERIAL_MAX_CONSTANTS ];

        s32                 nTexMaterials;
        tex_material        TexMaterial[ MATERIAL_MAX_TEXTURES ];

        // Functions
        tex_material*       GetTexMap       ( texture_type TexType );
        s32                 GetUVChanelCount( void );
    };

    struct bone
    {
        char                Name[256];
        s32                 nChildren;
        s32                 iParent;
        vector3             Scale;
        quaternion          Rotation;
        vector3             Position;
    };

    struct sub_mesh
    {
        char                Name[256];
    };


//=========================================================================

    s32             m_nBones;
    bone*           m_pBone;

    s32             m_nVFrames;
    s32             m_nVertices;
    vertex*         m_pVertex;

    s32             m_nFacets;
    facet*          m_pFacet;

    s32             m_nTextures;
    texture*        m_pTexture;

    s32             m_nMaterials;
    material*       m_pMaterial;

    s32             m_nSubMeshs;
    sub_mesh*       m_pSubMesh;

    s32             m_nUVWAnimFrames;
    uvanim_frame*   m_pUVWAnimFrame;
};

//=========================================================================
// END
//=========================================================================
#endif










































