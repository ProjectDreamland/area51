
#ifndef RAW_MESH2_HPP
#define RAW_MESH2_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_files.hpp"
#include "RawAnim.hpp"
#include "..\xcore\parsing\TextIn.hpp"
struct rawmesh;

//=========================================================================
// CLASS
//=========================================================================
struct rawmesh2
{
//=========================================================================

     rawmesh2( void );
    ~rawmesh2( void );

    xbool   Load                ( const char* pFileName );
    void    Save                ( const char* pFileName );
    void    Kill                ( void );
    void    SanityCheck         ( void );

    void    CleanMesh           ( s32 iSubMesh = -1 );
    void    CleanWeights        ( s32 MaxNumWeights, f32 MinWeightValue );

    void    CollapseMeshes      ( const char* pMeshName );
    void    CollapseNormals     ( radian ThresholdAngle = R_20 );

    bbox    GetBBox             ( void );
    void    ComputeMeshBBox     ( s32 iMesh, bbox& BBox );
    void    ComputeBoneInfo     ( void ) ;

    xbool   IsolateSubmesh      ( s32 iSubmesh, rawmesh2& NewMesh ,xbool RemoveFromRawMesh = FALSE );
    xbool   IsolateSubmesh      ( const char* pMeshName, rawmesh2& NewMesh );

    xbool   IsBoneUsed          ( s32 iBone );
    s32     GetBoneIDFromName   ( const char* pBoneName, xbool bAnywhere = FALSE ) const;
    void    DeleteBone          ( s32 iBone );
    void    DeleteBone          ( const char* pBoneName );
    
    s32     GetRigidBodyIDFromName( const char* pRigidBodyName, xbool bAnywhere = FALSE ) const;
    
    void    ApplyNewSkeleton    ( rawanim& RawAnim );
    void    ApplyNewSkeleton    ( const rawmesh2& Skel );

    void    SortFacetsByMaterial( void );
    void    SortFacetsByMaterialAndBone( void );

    static void    SetRenameDuplicates ( xbool b );

    void    BuildFromRM( rawmesh& RM );

    void    PrintHierarchy   ( void ) const;
    void    PrintRigidBodies ( void ) const;

    //
    //  Internal use only:
    //
    xbool   LoadMatx2           ( text_in& TIn );

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

    struct bone
    {
        char                Name[256];
        s32                 nChildren;
        s32                 iParent;
        vector3             Scale;
        quaternion          Rotation;
        vector3             Position;
        bbox                BBox;
        s32                 iRigidBody;
        s32                 LODGroup;
    };

    struct rigid_body
    {
        struct dof
        {
            s32     bActive;
            s32     bLimited;
            f32     Min;
            f32     Max;   
        };
            
        char                Name[256];
        char                Type[256];
        f32                 Mass;
        s32                 iParent;
        
        vector3             PivotPosition;
        vector3             PivotScale;
        quaternion          PivotRotation;
        
        vector3             BodyScale;
        quaternion          BodyRotation;
        vector3             BodyPosition;
        
        f32                 Radius;
        f32                 Width;
        f32                 Height;
        f32                 Length;
        dof                 DOF[6];
    };
    
    struct sub_mesh
    {
        char                Name[256];
        s32                 nBones ;
    };

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
        s32                 iVertex[FACET_MAX_VERTICES];
        s32                 iMaterial;
        plane               Plane;
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
        s32             nKeys;
        s32             nParamsPerKey;
        s32             iFirstKey;
        s32             FPS;
        anim_type       ModeType;
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
        //texanim_type        TYPE_OF_TEXTURE_PLAYBACK;
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
        xbool               bHasVariableVertAlpha;  // True is any verts in facets using this
                                                    //  material have non-255 or non-0 alphas
        xbool               bExposeName;            // Material needs to be unique - by name

        param_pkg           TintColor;
        param_pkg           TintAlpha;
        param_pkg           Constants[ MATERIAL_MAX_CONSTANTS ];

        s32                 nMaps;
        map                 Map[ MATERIAL_MAX_MAPS ];

        s32                 iFirstKey;          // offset to first f32 used by this mat
        s32                 nKeys;              // total # of f32's used by this materials pkgs
    };

//=========================================================================

    char            m_SourceFile[X_MAX_PATH];
    char            m_UserName[256];
    char            m_ComputerName[256];
    s32             m_ExportDate[3];  // month, day, year

    s32             m_nBones;
    bone*           m_pBone;
    
    s32             m_nRigidBodies;
    rigid_body*     m_pRigidBodies;

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

    s32             m_nParamKeys;
    f32*            m_pParamKey;

};

//=========================================================================
#endif

