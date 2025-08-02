//=============================================================================
//
//  Render Manager
//
//=============================================================================

#include "Entropy.hpp"
#include "Render.hpp"

#define RENDER_PRIVATE
#include "MaterialArray.hpp"
#undef RENDER_PRIVATE

//=============================================================================
//=============================================================================
// Structures and types
//=============================================================================
//=============================================================================

struct distortion_info
{
    radian3 NormalRot;  // rotation for perturbing the normals
    u32     MatIndex;   // index of the original material, or -1 if we are
                        // completely overriding the material settings
};

//=============================================================================

#define ENABLE_RENDER_XTIMERS   0

enum
{
    ORDER_OPAQUE        = 0,
    ORDER_GLOWING       = 1,
    ORDER_ALPHA_GLOWING = 2,
    ORDER_ALPHA         = 3,
    // everything aver this point will be in the custom render
    ORDER_CUSTOM_START  = 4,
    ORDER_FORCED_LAST   = 4,
    ORDER_ZPRIME        = 5,
    ORDER_FADING_ALPHA  = 6,
    ORDER_DISTORTION    = 7
};

union sortkey
{
    struct
    {
        u32     GeomSubMesh  : 8;
        u32     GeomHandle   : 9;
        u32     GeomType     : 1;
        u32     MatIndex     : 10;
        u32     RenderOrder  : 3;
    };
    u32         Bits;
};

union shad_sortkey
{
    struct
    {
        u32     ProjectorIndex  : 6;
        u32     GeomSubMesh     : 8;
        u32     GeomHandle      : 9;
        u32     GeomType        : 1;
        u32     ShadType        : 1;    // cast or receive
    };
    u32     Bits;
};

//=============================================================================

#ifdef TARGET_PC
    xbool g_bZPriming;
#endif

//=============================================================================

typedef enum geom_type
{
    TYPE_RIGID = 0,
    TYPE_SKIN,
    TYPE_UNKNOWN
};

//=============================================================================

struct rigid_data
{
    rigid_geom*     pGeom;
    const matrix4*  pL2W;
    const void*     pColInfo;
};

//=============================================================================

struct skin_data
{
    skin_geom*      pGeom;
    const matrix4*  pBones;
    u32             Pad;
};

//=============================================================================

union instance_data
{
    rigid_data  Rigid;
    skin_data   Skin;
};

//=============================================================================

struct render_instance
{
    union
    {
        shad_sortkey    ShadSortKey;
        sortkey         SortKey;
    };
    u32             Flags;
    void*           pLighting;
    instance_data   Data;

    u8              UOffset;
    u8              VOffset;
    u8              Alpha;
    u8              OverrideMat;    // such as distortion

    // information for the hash table
    s16             Brother;
    s16             Next;

#ifdef TARGET_PC
    xhandle         hDList;
#endif // TARGET_PC
};

//=============================================================================

struct sort_struct
{
    u32 SortKey;
    s32 iRenderInst;
};

//=============================================================================

struct private_instance
{
    geom*       pGeom;
    geom_type   Type;

#ifdef TARGET_PC
    xarray<xhandle> RigidDList;
    xbool           IsLit;
#endif // TARGET_PC
};

//=============================================================================

struct private_geom
{
    // simple struct for registered geoms...no information is really needed,
    // but we'll put a pointer back in for sanity checking later on
    geom*       pGeom;

#ifdef TARGET_PC
    xarray<xhandle> SkinDList;
#endif // TARGET_PC
};

//=============================================================================

struct texture_projection
{
    matrix4         L2W;
    radian          FOV;
    f32             Length;
    texture::handle Texture;
};

//=============================================================================
//=============================================================================
// Statics
//=============================================================================
//=============================================================================

// Stats for determining how much data has been loaded
#ifndef X_RETAIL
static s32 s_nGeomsLoaded         = 0;
static s32 s_nGeomBonesLoaded     = 0;
static s32 s_nGeomMeshesLoaded    = 0;
static s32 s_nGeomSubMeshesLoaded = 0;
static s32 s_nGeomMaterialsLoaded = 0;
static s32 s_nGeomTexturesLoaded  = 0;
static s32 s_nGeomUVKeysLoaded    = 0;
static s32 s_nGeomVMatsLoaded     = 0;
#endif

#if defined(TARGET_XBOX) || defined(TARGET_PC)
color_info::usage color_info::m_Usage = color_info::kUse32;
#elif defined(TARGET_PS2)
color_info::usage color_info::m_Usage = color_info::kUse16;
#else
#endif

//=============================================================================


// constants
// VERY IMPORTANT NOTE: README README README README!!!!!
//    Some of these max numbers will get used by the sort keys, so if they need
//    to increase, make sure the sort key still has enough bits to deal with it.

static const s32 kHashTableSize          = 769;  // 1543;   // keep this a prime number for best hashing results.
static const s32 kMaxRegisteredGeoms     = 512;
#if defined(X_EDITOR) || defined(CONFIG_VIEWER)
static const s32 kMaxRegisteredInstances = 12800;
#else
static const s32 kMaxRegisteredInstances = 10000;
#endif
static const s32 kMaxRegisteredMaterials = 640; // NOTE: Don't go over what the sort key can handle!
static const s32 kMaxTexAnims            = 2048;
static const s32 kMaxTexAnimInstances    = 1024;
static const s32 kMaxRegisteredTexAnims  = 1024;
static const s32 kMaxDistortedInstances  = 16;
#ifdef TARGET_PC
static const s32 kMaxRenderedInstances   = 32768;
#elif defined( TARGET_XBOX )
static const s32 kMaxRenderedInstances   = 4096;
#else
static const s32 kMaxRenderedInstances   = 3000;
#endif

// arrays for rendering everything
static s32                          s_LoHashMark;   // below this needs sorting
static s32                          s_HiHashMark;   // above this is a duplicate key (no need to sort)
static s16                          s_HashTable[kMaxRenderedInstances];
static xarray<sort_struct>          s_lSortData;
static xharray<private_geom>        s_lRegisteredGeoms;
static xharray<private_instance>    s_lRegisteredInst;
static material_array               s_lRegisteredMaterials;
static xarray<render_instance>      s_lRenderInst;
static xarray<distortion_info>      s_lDistortionInfo;

// sanity check data
static xbool s_InRawBegin    = FALSE;
static xbool s_InRenderBegin = FALSE;
static xbool s_InShadowBegin = FALSE;
static xbool s_InCustomBegin = FALSE;

// misc. data
static cubemap* s_pCurrCubeMap = NULL;
static f32      s_PulseTime;
static s32      s_CustomStart;

// stats
#if ENABLE_RENDER_STATS
static render::stats        s_RenderStats;
#endif

// debugging options
#ifndef X_RETAIL
render::debug_options       g_RenderDebug = { FALSE, FALSE, FALSE, FALSE };
#endif

// Filter light data
static s32    s_bFilterLight = FALSE;
static xcolor s_FilterLightColor(xcolor(30,0,0,255));

// texture and shadow projection
static texture_projection   s_TextureProjection;
static texture_projection   s_ShadowProjections[render::MAX_SHADOW_PROJECTORS];
static xbool                s_bDoTextureProjection;
static s32                  s_nShadowProjections;
static view                 s_TextureProjectionView;
static view                 s_ShadowProjectionViews[render::MAX_SHADOW_PROJECTORS];
static matrix4              s_TextureProjectionMatrix;
static matrix4              s_ShadowProjectionMatrices[render::MAX_SHADOW_PROJECTORS];

// projected shadows that are generated dynamically
static s32                  s_nDynamicShadows;

//=============================================================================

inline
xbool IsAlphaMaterial( material_type Type )
{
    switch ( Type )
    {
    default:
    case Material_Diff:
    case Material_Diff_PerPixelEnv:
    case Material_Diff_PerPixelIllum:
        return FALSE;

    case Material_Alpha:
    case Material_Alpha_PerPolyEnv:
    case Material_Alpha_PerPixelIllum:
    case Material_Alpha_PerPolyIllum:
    case Material_Distortion:
    case Material_Distortion_PerPolyEnv:
        return TRUE;
    }
}

//=============================================================================
//=============================================================================
// Platform-specific code
//=============================================================================
//=============================================================================

#ifdef TARGET_XBOX
#   include "LightMgr.hpp"
#   include "platform_Render.hpp"
#   include "Entropy/XBox/xbox_private.hpp"
#   include <XGraphics.h>
#   include "XBOX/xbox_render.hpp"
#   include "XBOX/xbox_post.inl"
#   include "XBOX/xbox_platform.inl"
shader_mgr  * g_pShaderMgr = NULL;
pipeline_mgr* g_pPipeline  = NULL;
#endif

#ifdef TARGET_PS2
#include "PS2/ps2_post.cpp"
#include "PS2/ps2_Render.cpp"
#endif

#ifdef TARGET_PC
#include "LightMgr.hpp"
#include "platform_Render.hpp"
#include "PC/pc_render.hpp"
#include "PC/pc_post.inl"
#include "PC/pc_platform.inl"
shader_mgr  * g_pShaderMgr = NULL;
pipeline_mgr* g_pPipeline  = NULL;
#endif

//=============================================================================
// Static declarations that we can put X_SECTION's on.
//=============================================================================

static u32                  HashFn                  ( u32 SortKey )                 X_SECTION( render_add );
static render_instance&     AddToHashHybrid         ( u32 SortKey )                 X_SECTION( render_add );
static xhandle              FindMaterial            ( material& Material )          X_SECTION( init );
static void                 RegisterMaterials       ( geom& Geom )                  X_SECTION( init );
static void                 UnregisterMaterials     ( geom& Geom )                  X_SECTION( init );
static void                 RegisterGeom            ( geom& Geom )                  X_SECTION( init );
static u32                  GetRenderOrder          ( material_type Type )          X_SECTION( render_add );
static void                 ComputeBaseSortKeys     ( geom& Geom,
                                                      geom_type Type )              X_SECTION( init );
static void                 UnregisterGeom          ( geom& Geom )                  X_SECTION( init );
static void                 RegisterRigidGeom       ( rigid_geom& Geom )            X_SECTION( init );
static void                 UnregisterRigidGeom     ( rigid_geom& Geom )            X_SECTION( init );
static void                 RegisterSkinGeom        ( skin_geom& Geom )             X_SECTION( init );
static void                 UnregisterSkinGeom      ( skin_geom& Geom )             X_SECTION( init );
static render::hgeom_inst   AddPrivateInstance      ( geom& Geom,
                                                      geom_type Type )              X_SECTION( init );
static void                 RemovePrivateInstance   ( render::hgeom_inst hInst )    X_SECTION( init );  
static s32                  InstanceCompareFn       ( const void* p1,
                                                      const void* p2 )              X_SECTION( render_infrequent );
static void                 GetUVOffset             ( u8& UOffset,
                                                      u8& VOffset,
                                                      geom* pGeom,
                                                      material& Mat )               X_SECTION( render_add );
static xbool                IntersectsView          ( const view& V,
                                                      const bbox& BBox )            X_SECTION( render_add );
static xbool                IntersectsProjTexture   ( const bbox& BBox )            X_SECTION( render_add );
static xbool                IntersectsShadowTexture ( s32 Index,
                                                      const bbox& BBox )            X_SECTION( render_add );
static u32                  CollectProjectionInfo   ( u32 RenderFlags,
                                                      const bbox& BBox )            X_SECTION( render_add );
static xbool                CanHaveProjTexture      ( const geom::material& Mat )   X_SECTION( render_add );

static void                 CalcVMatOffsets         ( s32* VMatOffsets,
                                                      const geom* pGeom,
                                                      u32 VTextureMask )            X_SECTION( render_infrequent );


//=============================================================================

color_info::color_info( fileio& File )
{
    (void)File;

#if defined(TARGET_XBOX)
    m_hColors = g_VertFactory.Create( "Vertex colours",m_nColors*sizeof(u32),m_pVoid );
    m_pColor32 = (u32*)m_hColors->m_Ptr;
#endif
}

//=============================================================================
//=============================================================================
// Internal functions
//=============================================================================
//=============================================================================

static
u32 HashFn( u32 SortKey )
{
    return SortKey % kHashTableSize;
}

//=============================================================================

static
render_instance& AddToHashHybrid( u32 SortKey )
{
    ASSERT( s_LoHashMark < s_HiHashMark );
    #if defined(X_EDITOR) || defined(CONFIG_VIEWER)
    if ( s_LoHashMark >= s_HiHashMark )
        x_throw( "Too many submeshes rendered." );
    #endif // X_EDITOR || VIEWER

    u32 HashIndex = HashFn( SortKey );
    if ( s_HashTable[HashIndex] == -1 )
    {
        // if there is no hash entry, just add it
        render_instance& AddInst = s_lRenderInst[s_LoHashMark];
        s_HashTable[HashIndex]   = s_LoHashMark;
        AddInst.SortKey.Bits     = SortKey;
        AddInst.Next             = -1;
        AddInst.Brother          = -1;
        AddInst.pLighting        = NULL;
        sort_struct& SortInst    = s_lSortData[s_LoHashMark];
        SortInst.iRenderInst     = s_LoHashMark;
        SortInst.SortKey         = SortKey;
        s_LoHashMark++;
        return AddInst;
    }
    else
    {
        // loop through the linked list of hash collisions
        s32 CurrLink = s_HashTable[HashIndex];
        #ifdef X_ASSERT
        s32 Count = 0;
        #endif
        while ( 1 )
        {
            ASSERT( ++Count < kMaxRenderedInstances && "Infinite loop detected" );
            render_instance& TestInst = s_lRenderInst[CurrLink];
            if ( TestInst.SortKey.Bits == SortKey )
            {
                // this sort key isn't unique, add it to the end of the array
                // so that it won't be considered for sorting, and link it as a
                // "brother" of the unique instance
                render_instance& AddInst = s_lRenderInst[s_HiHashMark];
                AddInst.SortKey.Bits     = SortKey;
                AddInst.Next             = -1;
                AddInst.Brother          = TestInst.Brother;
                AddInst.pLighting        = NULL;
                TestInst.Brother         = s_HiHashMark;
                s_HiHashMark--;
                return AddInst;
            }

            if ( TestInst.Next == -1 )
            {
                // the key wasn't found, so add it to the end of our linked list of
                // hash collisions
                render_instance& AddInst = s_lRenderInst[s_LoHashMark];
                AddInst.SortKey.Bits     = SortKey;
                AddInst.Next             = -1;
                AddInst.Brother          = -1;
                AddInst.pLighting        = NULL;
                TestInst.Next            = s_LoHashMark;
                sort_struct& SortInst    = s_lSortData[s_LoHashMark];
                SortInst.iRenderInst     = s_LoHashMark;
                SortInst.SortKey         = SortKey;
                s_LoHashMark++;
                return AddInst;
            }

            CurrLink = TestInst.Next;
        }
    }


/*
    ASSERT( s_LoHashMark < s_HiHashMark );
    s_lRenderInst[s_LoHashMark].SortKey   = SortKey;
    s_lRenderInst[s_LoHashMark].Next      = -1;
    s_lRenderInst[s_LoHashMark].Brother   = -1;
    s_lSortData[s_LoHashMark].iRenderInst = s_LoHashMark;
    s_lSortData[s_LoHashMark].SortKey     = SortKey;
    s_LoHashMark++;

    return s_lRenderInst[s_LoHashMark-1];
    */
}

//=============================================================================

static
xhandle FindMaterial( material& Material )
{
    for ( s32 i = 0; i < s_lRegisteredMaterials.GetCount(); i++ )
    {
        material& M = s_lRegisteredMaterials[i];
        
        if ( M == Material )
        {
            return s_lRegisteredMaterials.GetHandleByIndex(i);
        }
    }

    return HNULL;
}

//=============================================================================

static
void RegisterMaterials( geom& Geom )
{
    // Register all the materials in the geom
    for ( s32 iMat = 0; iMat < Geom.m_nMaterials; iMat++ )
    {
        material    Mat;

        // get the next material used by the geom
        geom::material& GeomMat = Geom.m_pMaterial[iMat];

        // set the material type
        Mat.m_Type = GeomMat.Type;

        // set the flags
        Mat.m_Flags = GeomMat.Flags;

        // set the detail scale
        Mat.m_DetailScale = GeomMat.DetailScale;

        // set the fixed alpha
        Mat.m_FixedAlpha = GeomMat.FixedAlpha;

        // copy across the uvanim data
        Mat.m_UVAnim.CurrentFrame = 0.0f;
        Mat.m_UVAnim.iKey         = GeomMat.UVAnim.iKey;
        Mat.m_UVAnim.iFrame       = 0;
        Mat.m_UVAnim.Dir          = 1;
        Mat.m_UVAnim.Type         = GeomMat.UVAnim.Type;
        Mat.m_UVAnim.nFrames      = GeomMat.UVAnim.nKeys;
        Mat.m_UVAnim.FPS          = GeomMat.UVAnim.FPS;
        Mat.m_UVAnim.StartFrame   = GeomMat.UVAnim.StartFrame;

        // sanity checks
        if ( ((Mat.m_Type == Material_Diff_PerPixelEnv) ||
              (Mat.m_Type == Material_Alpha_PerPolyEnv)) &&
              !(Mat.m_Flags & geom::material::FLAG_ENV_CUBE_MAP) )
        {
            if( !(GeomMat.Flags & geom::material::FLAG_HAS_ENV_MAP) )
            {
                x_throw( "Environment mapped material without an env texture!" );
            }
        }

        // copy the texture info over
        s32 iDiffuse     = GeomMat.iTexture;
        s32 iEnvironment = iDiffuse     + GeomMat.nVirtualMats;
        s32 iDetail      = iEnvironment +
                           ((GeomMat.Flags&geom::material::FLAG_HAS_ENV_MAP) ? 1 : 0);

        // now for virtual textures, each different bitmap choice will become a
        // unique material
        for ( s32 iVMat = 0; iVMat < GeomMat.nVirtualMats; iVMat++ )
        {
            // set the diffuse map for this bitmap choice
            Mat.m_DiffuseMap.SetName( Geom.GetTextureName( iDiffuse + iVMat ) );

            // set the env map for this bitmap choice
            if ( GeomMat.Flags&geom::material::FLAG_HAS_ENV_MAP )
            {
                Mat.m_EnvironmentMap.SetName( Geom.GetTextureName( iEnvironment ) );
            }
            else
            {
                Mat.m_EnvironmentMap.SetName( "" );
            }

            // set the detail map for this bitmap choice
            if ( GeomMat.Flags&geom::material::FLAG_HAS_DETAIL_MAP )
            {
                Mat.m_DetailMap.SetName( Geom.GetTextureName( iDetail ) );
            }
            else
            {
                Mat.m_DetailMap.SetName( "" );
            }

            // check if we already have this material registered
            xhandle Handle = FindMaterial( Mat );

            // if this is a new material, then add it
            if ( Handle == HNULL )
            {
                // If you hit this assert, this means we are causing a realloc
                // which will fragment memory, and we are possibly going over
                // the max number of materials that will fit within the sort
                // key. This could cause material corruptions.
                ASSERT( s_lRegisteredMaterials.GetCount() < kMaxRegisteredMaterials );
                material& NewMat = s_lRegisteredMaterials.Add( Handle );
                NewMat           = Mat;
            }

            // finally, we can add a ref to this material, and let the geometry know its
            // material handle
            material& FinalMat = s_lRegisteredMaterials(Handle);
            FinalMat.AddRef();

            // let the geometry know where its registered material can be found
            Geom.m_pVirtualMaterials[GeomMat.iVirtualMat+iVMat].MatHandle = Handle;

            // and let the platform do any initialization that it needs to
            platform_RegisterMaterial( FinalMat );
        }
    }
}

//=============================================================================

static
void UnregisterMaterials( geom& Geom )
{
    // unregister any of the materials
    for ( s32 iMat = 0; iMat < Geom.m_nMaterials; iMat++ )
    {
        geom::material& GeomMat = Geom.m_pMaterial[iMat];

        for ( s32 iVMat = 0; iVMat < GeomMat.nVirtualMats; iVMat++ )
        {
            xhandle Handle = Geom.m_pVirtualMaterials[GeomMat.iVirtualMat+iVMat].MatHandle;
            material& Mat = s_lRegisteredMaterials(Handle);
            Mat.Release();
            if ( Mat.GetRefCount() == 0 )
            {
                s_lRegisteredMaterials.DeleteByHandle(Handle);
            }
        }
    }
}

//=============================================================================

static
void RegisterGeom( geom& Geom )
{
    // register the geometry
    ASSERT( Geom.m_hGeom == HNULL );
    ASSERT( Geom.GetRefCount() == 0 );
    ASSERT( s_lRegisteredGeoms.GetCount() < kMaxRegisteredGeoms );
    private_geom& RegGeom = s_lRegisteredGeoms.Add( Geom.m_hGeom );

    // this pointer isn't really needed, but will be nice for sanity checking
    // later on
    RegGeom.pGeom = &Geom;

    // register the materials this geometry uses
    RegisterMaterials( Geom );

#ifndef X_RETAIL
    s_nGeomsLoaded         += 1;
    s_nGeomBonesLoaded     += Geom.m_nBones;
    s_nGeomMeshesLoaded    += Geom.m_nMeshes;
    s_nGeomSubMeshesLoaded += Geom.m_nSubMeshes;
    s_nGeomMaterialsLoaded += Geom.m_nMaterials;
    s_nGeomTexturesLoaded  += Geom.m_nTextures;
    s_nGeomUVKeysLoaded    += Geom.m_nUVKeys;
    s_nGeomVMatsLoaded     += Geom.m_nVirtualMaterials;
#endif
}

//=============================================================================

static
u32 GetRenderOrder( material_type Type )
{
    switch( Type )
    {
    default:
        ASSERTS( FALSE, "Unknown material type." );
    case Material_Diff:
    case Material_Diff_PerPixelEnv:
        return ORDER_OPAQUE;
    case Material_Diff_PerPixelIllum:
        return ORDER_GLOWING;
    case Material_Alpha_PerPixelIllum:
    case Material_Alpha_PerPolyIllum:
        return ORDER_ALPHA_GLOWING;
    case Material_Alpha:
    case Material_Alpha_PerPolyEnv:
        return ORDER_ALPHA;
    case Material_Distortion:
    case Material_Distortion_PerPolyEnv:
        return ORDER_DISTORTION;
    }
}

//=============================================================================

static
void ComputeBaseSortKeys( geom& Geom, geom_type Type )
{
    s32 iSubMesh;
    ASSERT( Geom.m_hGeom.IsNonNull() );
    for ( iSubMesh = 0; iSubMesh < Geom.m_nSubMeshes; iSubMesh++ )
    {
        geom::submesh*  pSubMesh  = &Geom.m_pSubMesh[iSubMesh];
        geom::material* pMaterial = &Geom.m_pMaterial[pSubMesh->iMaterial];
        s32             TypeBit   = (Type==TYPE_RIGID) ? 0 : 1;

        // range safety check for the sort key
        ASSERT( (Geom.m_hGeom>=0) && (Geom.m_hGeom<kMaxRegisteredGeoms    ) );
        ASSERT( (iSubMesh    >=0) && (iSubMesh    <256                    ) );

        // build the sort key
        sortkey BaseSortKey;
        BaseSortKey.Bits = 0;
        BaseSortKey.GeomSubMesh = iSubMesh;
        BaseSortKey.GeomHandle  = Geom.m_hGeom;
        BaseSortKey.GeomType    = TypeBit;
        BaseSortKey.RenderOrder = GetRenderOrder( (material_type)pMaterial->Type );
        pSubMesh->BaseSortKey   = BaseSortKey.Bits;
    }
}

//=============================================================================

static
void UnregisterGeom( geom& Geom )
{
    ASSERT( Geom.GetRefCount() == 0 );

    // unregister the materials
    UnregisterMaterials( Geom );

    // unregister the geom
    ASSERT( s_lRegisteredGeoms( Geom.m_hGeom ).pGeom == &Geom );
    s_lRegisteredGeoms.DeleteByHandle( Geom.m_hGeom );
    Geom.m_hGeom = HNULL;

#ifndef X_RETAIL
    s_nGeomsLoaded         -= 1;
    s_nGeomBonesLoaded     -= Geom.m_nBones;
    s_nGeomMeshesLoaded    -= Geom.m_nMeshes;
    s_nGeomSubMeshesLoaded -= Geom.m_nSubMeshes;
    s_nGeomMaterialsLoaded -= Geom.m_nMaterials;
    s_nGeomTexturesLoaded  -= Geom.m_nTextures;
    s_nGeomUVKeysLoaded    -= Geom.m_nUVKeys;
    s_nGeomVMatsLoaded     -= Geom.m_nVirtualMaterials;
#endif
}

//=============================================================================

static
void RegisterRigidGeom( rigid_geom& Geom )
{
    RegisterGeom( Geom );
    ComputeBaseSortKeys( Geom, TYPE_RIGID );
    platform_RegisterRigidGeom( Geom );
}

//=============================================================================

static
void UnregisterRigidGeom( rigid_geom& Geom )
{
    platform_UnregisterRigidGeom( Geom );
    UnregisterGeom( Geom );
}

//=============================================================================

static
void RegisterSkinGeom( skin_geom& Geom )
{
    RegisterGeom( Geom );
    ComputeBaseSortKeys( Geom, TYPE_SKIN );
    platform_RegisterSkinGeom( Geom );
}

//=============================================================================

static
void UnregisterSkinGeom( skin_geom& Geom )
{
    platform_UnregisterSkinGeom( Geom );
    UnregisterGeom( Geom );
}

//=============================================================================

static
render::hgeom_inst AddPrivateInstance( geom& Geom, geom_type Type )
{
    // increment the geom's ref count
    Geom.AddRef();

    // add the instance
    render::hgeom_inst Handle;
    ASSERT( s_lRegisteredInst.GetCount() < kMaxRegisteredInstances );
    private_instance& Inst = s_lRegisteredInst.Add( Handle );
    Inst.pGeom     = &Geom;
    Inst.Type      = Type;

    // return the new handle
    return Handle;
}

//=============================================================================

static
void RemovePrivateInstance( render::hgeom_inst hInst )
{
    // decrement the geom's ref count
    private_instance& Inst = s_lRegisteredInst( hInst );
    Inst.pGeom->Release();

    // delete the instance
    s_lRegisteredInst.DeleteByHandle( hInst );
}

//=============================================================================

static
s32 InstanceCompareFn( const void* p1, const void* p2 )
{
    sort_struct* Inst1 = (sort_struct*)p1;
    sort_struct* Inst2 = (sort_struct*)p2;

    if ( Inst1->SortKey > Inst2->SortKey )  return  1;
    if ( Inst1->SortKey < Inst2->SortKey )  return -1;
    
    return 0;
}

//=============================================================================

static
void GetUVOffset( u8& UOffset, u8& VOffset, geom* pGeom, material& Mat )
{
    if( Mat.m_UVAnim.nFrames == 0 )
    {
        UOffset = VOffset = 0;
        return;
    }

    s32 iKey = Mat.m_UVAnim.iKey + Mat.m_UVAnim.iFrame;
    UOffset = pGeom->m_pUVKey[iKey].OffsetU;
    VOffset = pGeom->m_pUVKey[iKey].OffsetV;
}

//=============================================================================

static
xbool IntersectsView( const view& V, const bbox& BBox )
{
    return ( V.BBoxInView(BBox) != view::VISIBLE_NONE );
}

//=============================================================================

static
xbool IntersectsProjTexture( const bbox& BBox )
{
    return IntersectsView( s_TextureProjectionView, BBox );
}

//=============================================================================

static
xbool IntersectsShadowTexture( s32 Index, const bbox& BBox )
{
    return IntersectsView( s_ShadowProjectionViews[Index], BBox );
}

//=============================================================================

static
u32 CollectProjectionInfo( u32 RenderFlags, const bbox& BBox )
{
    u32 RetFlags = 0;

    // do we even have a texture projection we can use?
    if( !s_bDoTextureProjection && !s_nShadowProjections )
    {
        return RetFlags;
    }

    // check the spotlight
    if( s_bDoTextureProjection &&
        !(RenderFlags & render::DISABLE_SPOTLIGHT) &&
        IntersectsProjTexture( BBox ) )
    {
        RetFlags |= render::INSTFLAG_SPOTLIGHT;
    }

    // check any projected shadows
    if( s_nShadowProjections &&
        !(RenderFlags & render::DISABLE_PROJ_SHADOWS) &&
        IntersectsShadowTexture( 0, BBox ) )
    {
        RetFlags |= render::INSTFLAG_PROJ_SHADOW_1;
    }

    if( (s_nShadowProjections>1) &&
        !(RenderFlags & render::DISABLE_PROJ_SHADOWS) &&
        IntersectsShadowTexture( 1, BBox ) )
    {
        RetFlags |= render::INSTFLAG_PROJ_SHADOW_2;
    }

    return RetFlags;
}

//=============================================================================

static
xbool CanHaveProjTexture( const geom::material& Mat )
{
    // Opaque materials can always accept projected textures.
    if( !IsAlphaMaterial((material_type)Mat.Type) )
        return TRUE;

    // They must have the zfill flag on to accept a projected texture.
    if( !(Mat.Flags & geom::material::FLAG_FORCE_ZFILL) )
        return FALSE;

    // Now it just comes down to whether or not they are subtractive.
    // Subtractive materials don't like doing flashlights at all, because
    // the flashlight area is rendered with a regular diffuse blend.
    if( Mat.Flags & geom::material::FLAG_IS_SUBTRACTIVE )
        return FALSE;

    // Yes, this alpha material can receive a projected texture.
    return TRUE;
}

//=============================================================================

#ifdef X_DEBUG
static
void SortSanityCheck( void )
{
    #if 0
    {
        u32 LastSortKey = 0;
        s32 i;
        for ( i = 0; i < s_LoHashMark; i++ )
        {
            render_instance& Inst = s_lRenderInst[s_lSortData[i].iRenderInst];
            ASSERT( Inst.SortKey == s_lSortData[i].SortKey );
            ASSERT( Inst.SortKey >= LastSortKey );
            LastSortKey = Inst.SortKey;
        }
    }
    #endif
}
#endif

//=============================================================================

static
void CalcVMatOffsets( s32* VMatOffsets, const geom* pGeom, u32 VTextureMask )
{
    x_memset( VMatOffsets, 0, sizeof(s32)*32 );

    s32 i, j;
    for( i = 0; i < pGeom->m_nVirtualTextures; i++ )
    {
        s32 Offset = VTextureMask & 0xf;
        VTextureMask >>= 4;

        geom::virtual_texture& VTexture = pGeom->m_pVirtualTextures[i];
        for( j = 0; j < pGeom->m_nMaterials; j++ )
        {
            ASSERT( j < 32 );
            if( VTexture.MaterialMask & (1<<j) )
            {
                Offset = MINMAX( 0, Offset, pGeom->m_pMaterial[j].nVirtualMats-1 );
                VMatOffsets[j] = Offset;
            }
        }
    }
}

//=============================================================================
//=============================================================================
// Implementation of "public" functions
//=============================================================================
//=============================================================================

s32 render::GetHardwareBufferSize( void )
{
    return 80;
}

//=============================================================================

void render::Init( void )
{
    s_PulseTime = 0.0f;

    s_lRegisteredGeoms.Clear();
    s_lRegisteredGeoms.GrowListBy( kMaxRegisteredGeoms );
    s_lRegisteredInst.Clear();
    s_lRegisteredInst.GrowListBy( kMaxRegisteredInstances );
    s_lRegisteredMaterials.Clear();
    s_lRegisteredMaterials.GrowListBy( kMaxRegisteredMaterials );
    s_lRenderInst.Clear();
    s_lRenderInst.SetCapacity( kMaxRenderedInstances );
    s_lRenderInst.SetCount( kMaxRenderedInstances );
    s_lDistortionInfo.Clear();
    s_lDistortionInfo.SetCapacity( kMaxDistortedInstances );
    s_lDistortionInfo.SetLocked(TRUE);
    s_lSortData.Clear();
    s_lSortData.SetCapacity( kMaxRenderedInstances );
    s_lSortData.SetCount( kMaxRenderedInstances );

    platform_Init();
}

//=============================================================================

void render::Kill( void )
{
    platform_Kill();

    ASSERT( s_lRegisteredGeoms.GetCount() == 0 );
    ASSERT( s_lRegisteredInst.GetCount() == 0 );
    ASSERT( s_lRegisteredMaterials.GetCount() == 0 );
    s_lRegisteredGeoms.Clear();
    s_lRegisteredInst.Clear();
    s_lRegisteredMaterials.Clear();
    s_lRenderInst.Clear();
    s_lSortData.Clear();
}

//=============================================================================

void render::Update( f32 DeltaTime )
{
    s_PulseTime += DeltaTime;

    // update all uv animations
    s_lRegisteredMaterials.Update( DeltaTime );
}

//=============================================================================

void render::StartRawDataMode( void )
{
    ASSERT( !s_InRenderBegin && !s_InShadowBegin && !s_InRawBegin );
    s_InRawBegin = TRUE;

    platform_StartRawDataMode();
}

//=============================================================================

void render::EndRawDataMode( void )
{
    platform_EndRawDataMode();
    
    ASSERT( s_InRawBegin );
    s_InRawBegin = FALSE;
}

//=============================================================================

void render::RenderRawStrips( s32            nVerts,
                              const matrix4& L2W,
                              const vector4* pPos,
                              const s16*     pUV,
                              const u32*     pColor )
{
    ASSERT( s_InRawBegin );
    platform_RenderRawStrips( nVerts, L2W, pPos, pUV, pColor );
}

//=============================================================================

void render::Render3dSprites( s32            nSprites,
                              f32            UniScale,
                              const matrix4* pL2W,
                              const vector4* pPositions,
                              const vector2* pRotScales,
                              const u32*     pColors )
{
    ASSERT( s_InRawBegin );

    platform_Render3dSprites( nSprites, UniScale, pL2W, pPositions, pRotScales, pColors );
}

//=============================================================================

void render::RenderHeatHazeSprites( s32             nSprites,
                                    f32             UniScale,
                                    const matrix4*  pL2W,
                                    const vector4*  pPositions,
                                    const vector2*  pRotScales,
                                    const u32*      pColors )
{
    ASSERT( s_InRawBegin );

    platform_RenderHeatHazeSprites( nSprites, UniScale, pL2W, pPositions, pRotScales, pColors );
}

//=============================================================================

void render::RenderVelocitySprites( s32             nSprites,
                                    f32             UniScale,
                                    const matrix4*  pL2W,
                                    const matrix4*  pVelMatrix,
                                    const vector4*  pPositions,
                                    const vector4*  pVelocities,
                                    const u32*      pColors  )
{
    ASSERT( s_InRawBegin );

    platform_RenderVelocitySprites( nSprites, UniScale, pL2W, pVelMatrix, pPositions, pVelocities, pColors );
}

//=============================================================================

void render::SetDiffuseMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestEnabled )
{
    ASSERT( s_InRawBegin );
    platform_SetDiffuseMaterial( Bitmap, BlendMode, ZTestEnabled );
}

//=============================================================================

void render::SetGlowMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestEnabled )
{
    ASSERT( s_InRawBegin );
    platform_SetGlowMaterial( Bitmap, BlendMode, ZTestEnabled );
}

//=============================================================================

void render::SetEnvMapMaterial( const xbitmap& Bitmap, s32 BlendMode, xbool ZTestEnabled )
{
    ASSERT( s_InRawBegin );
    platform_SetEnvMapMaterial( Bitmap, BlendMode, ZTestEnabled );
}

//============================================================================= 

void render::SetDistortionMaterial( s32 BlendMode, xbool ZTestEnabled )
{
    ASSERT( s_InRawBegin );
    platform_SetDistortionMaterial( BlendMode, ZTestEnabled );
}

//============================================================================= 

render::hgeom_inst render::RegisterRigidInstance( rigid_geom& Geom )
{
    ASSERT( !s_InRenderBegin && !s_InShadowBegin && !s_InRawBegin );

    // register the geom if that hasn't been done yet
    if ( Geom.GetRefCount() == 0 )
        RegisterRigidGeom( Geom );

    // safety check
    ASSERT( s_lRegisteredGeoms(Geom.m_hGeom).pGeom == &Geom );

    // add the instance
    render::hgeom_inst Handle = AddPrivateInstance( Geom, TYPE_RIGID );    
    platform_RegisterRigidInstance( Geom, Handle );

    return Handle;
}

//=============================================================================

void render::UnregisterRigidInstance( hgeom_inst hInst )
{
    ASSERT( !s_InRenderBegin && !s_InShadowBegin && !s_InRawBegin );

    // do we need to unregister the geom?
    xbool bUnregisterGeom  = FALSE;;
    private_instance& Inst = s_lRegisteredInst(hInst);
    ASSERT( Inst.Type == TYPE_RIGID );
    if ( Inst.pGeom->GetRefCount() == 1 )
        bUnregisterGeom = TRUE;

    // unregister the instance
    platform_UnregisterRigidInstance( hInst );
    RemovePrivateInstance( hInst );

    // unregister the geom
    if ( bUnregisterGeom )
        UnregisterRigidGeom( *((rigid_geom*)Inst.pGeom) );
}

//=============================================================================

render::hgeom_inst render::RegisterSkinInstance( skin_geom& Geom )
{
    ASSERT( !s_InRenderBegin && !s_InShadowBegin && !s_InRawBegin );

    // register the geom if that hasn't been done yet
    if ( Geom.GetRefCount() == 0 )
        RegisterSkinGeom( Geom );

    // safety check
    ASSERT( s_lRegisteredGeoms(Geom.m_hGeom).pGeom == &Geom );

    // add the instance
    render::hgeom_inst Handle = AddPrivateInstance( Geom, TYPE_SKIN );
    platform_RegisterSkinInstance( Geom, Handle );

    return Handle;
}

//=============================================================================

void render::UnregisterSkinInstance( hgeom_inst hInst )
{
    ASSERT( !s_InRenderBegin && !s_InShadowBegin && !s_InRawBegin );

    // do we need to unregister the geom?
    xbool bUnregisterGeom  = FALSE;;
    private_instance& Inst = s_lRegisteredInst(hInst);
    ASSERT( Inst.Type == TYPE_SKIN );
    if ( Inst.pGeom->GetRefCount() == 1 )
        bUnregisterGeom = TRUE;

    // unregister the instance
    platform_UnregisterSkinInstance( hInst );
    RemovePrivateInstance( hInst );

    // unregister the geom
    if ( bUnregisterGeom )
        UnregisterSkinGeom( *((skin_geom*)Inst.pGeom) );
}

//=============================================================================

const geom* render::GetGeom( hgeom_inst hInst )
{
    if ( hInst.IsNull() )
        return NULL;

    private_instance& Inst = s_lRegisteredInst(hInst);
    return Inst.pGeom;
}

//=============================================================================

void render::SetTextureProjection( const matrix4&         L2W,
                                   radian                 FOV,
                                   f32                    Length,
                                   const texture::handle& Texture )
{
    if ( Texture.GetPointer() != NULL )
    {
        // Test if the projection is too perpendicular to the view, and turn it
        // off if it is. This can cause too many artifacts where the projection
        // would be clipped (we can't do clipping for performance reasons). We
        // may still see the occasional artifact, but hopefully this will keep it
        // to an absolute minimum.
        const view*    pView   = eng_GetView();
        const matrix4& V2W     = pView->GetV2W();
        vector3 V0, V1, ViewDir, ProjDir;
        V2W.GetColumns( V0, V1, ViewDir );
        L2W.GetColumns( V0, V1, ProjDir );
        if ( ViewDir.Dot( ProjDir ) > 0.8660f )
        {
            s_bDoTextureProjection      = TRUE;
            s_TextureProjection.L2W     = L2W;
            s_TextureProjection.FOV     = FOV;
            s_TextureProjection.Length  = Length;
            s_TextureProjection.Texture = Texture;

            platform_SetProjectedTexture     ( Texture );
            platform_ComputeProjTextureMatrix( s_TextureProjectionMatrix, s_TextureProjectionView, s_TextureProjection );
            platform_SetTextureProjection    ( s_TextureProjection );
        }
    }
}

//=============================================================================

void render::SetShadowProjection( const matrix4&         L2W,
                                  radian                 FOV,
                                  f32                    Length,
                                  const texture::handle& Texture )
{
    if( (Texture.GetPointer() != NULL) && (s_nShadowProjections < 2) )
    {
        s_ShadowProjections[s_nShadowProjections].L2W     = L2W;
        s_ShadowProjections[s_nShadowProjections].FOV     = FOV;
        s_ShadowProjections[s_nShadowProjections].Length  = Length;
        s_ShadowProjections[s_nShadowProjections].Texture = Texture;
        
        platform_SetProjectedShadowTexture( s_nShadowProjections, Texture );
        platform_ComputeProjShadowMatrix( s_ShadowProjectionMatrices[s_nShadowProjections],
                                          s_ShadowProjectionViews[s_nShadowProjections],
                                          s_ShadowProjections[s_nShadowProjections] );

        s_nShadowProjections++;
    }
}

//=============================================================================

void render::SetCustomFogPalette( const texture::handle& Texture, xbool ImmediateSwitch, s32 PaletteIndex )
{
    platform_SetCustomFogPalette( Texture, ImmediateSwitch, PaletteIndex );
}

//=============================================================================

xcolor render::GetFogValue( const vector3& WorldPos, s32 PaletteIndex )
{
    return platform_GetFogValue( WorldPos, PaletteIndex );
}

//=============================================================================

void render::BeginNormalRender( void )
{
    CONTEXT( "render::Begin" );

    // clear out the distorted instance list
    s_lDistortionInfo.Delete( 0, s_lDistortionInfo.GetCount() );

    // sort the materials
    s_lRegisteredMaterials.Sort();

    // safety check
    ASSERT( eng_InBeginEnd() );
    ASSERT( !s_InRenderBegin && !s_InShadowBegin && !s_InRawBegin );
    s_InRenderBegin = TRUE;

    // clear out the list of render instances
    s_LoHashMark = 0;
    s_HiHashMark = kMaxRenderedInstances - 1;
    x_memset( s_HashTable, 0xff, sizeof(s16)*kMaxRenderedInstances );

    // clear out texture and shadow projections
    s_bDoTextureProjection = FALSE;
    s_nShadowProjections   = 0;
    texture::handle Handle;
    platform_SetProjectedTexture( Handle );

    platform_BeginNormalRender();
}

//=============================================================================

#ifdef TARGET_PC
namespace render
{
    void PrimeZBuffer( void )
    {
        // First of all we prime the Z-buffer *********************************

        DWORD CullMode;
        g_pd3dDevice->GetRenderState( D3DRS_CULLMODE,&CullMode );
        g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,0 );
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,D3DCULL_NONE );
        g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,TRUE );

        g_bZPriming = TRUE;
        {
            // loop through all of the render instances and render those bad boys
            sortkey   CurrentSortData;
            CurrentSortData.Bits          = 0xffffffff;
            geom*     pCurrentGeom        = NULL;
            geom_type CurrentType         = TYPE_UNKNOWN;
            for ( s32 iUniqueInst = 0; iUniqueInst < s_LoHashMark; iUniqueInst++ )
            {
                render_instance& Inst = s_lRenderInst[s_lSortData[iUniqueInst].iRenderInst];
                
                // If this is the start of the custom distortion and alpha instances,
                // the bail out of this loop. They'll come later
                if ( Inst.SortKey.RenderOrder >= ORDER_CUSTOM_START )
                {
                    // distortion meshes have to be done separately
                    s_CustomStart = iUniqueInst;
                    break;
                }

                // activate the material if necessary
                if ( CurrentSortData.MatIndex  != Inst.SortKey.MatIndex )
                {
                    if ( pCurrentGeom != NULL )
                    {
                        ASSERT( CurrentType != TYPE_UNKNOWN );
                        if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                        else                                platform_EndSkinGeom ();
                        pCurrentGeom = NULL;
                    }

                    #if ENABLE_RENDER_XTIMERS
                    StatsTimer.Reset();
                    StatsTimer.Start();
                    #endif

                    material& Mat = s_lRegisteredMaterials[Inst.SortKey.MatIndex];
                    ASSERT( (Mat.m_Type != Material_Distortion) &&
                            (Mat.m_Type != Material_Distortion_PerPolyEnv) );

                    #if ENABLE_RENDER_XTIMERS
                    s_RenderStats.m_MaterialActivateTime += StatsTimer.Stop();
                    #endif

                    CurrentSortData.MatIndex  = Inst.SortKey.MatIndex;

                    // update the stats
                    #if ENABLE_RENDER_STATS
                    s_RenderStats.m_nMaterialsRendered++;
                    #endif
                }

                // start a new instance batch if necessary (geometry sorting should already
                // be built into the sort key)
                if ( (Inst.SortKey.GeomType == 0) &&
                    ((pCurrentGeom != Inst.Data.Rigid.pGeom) || (CurrentSortData.GeomSubMesh != Inst.SortKey.GeomSubMesh)) )
                {
                    if ( pCurrentGeom != NULL )
                    {
                        ASSERT( CurrentType != TYPE_UNKNOWN );
                        if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                        else                                platform_EndSkinGeom ();
                    }

                    pCurrentGeom                = Inst.Data.Rigid.pGeom;
                    CurrentType                 = TYPE_RIGID;
                    CurrentSortData.GeomSubMesh = Inst.SortKey.GeomSubMesh;
                    platform_BeginRigidGeom( pCurrentGeom, Inst.SortKey.GeomSubMesh );
                }
                else
                if ( Inst.SortKey.GeomType &&
                    ((pCurrentGeom != Inst.Data.Skin.pGeom) || (CurrentSortData.GeomSubMesh != Inst.SortKey.GeomSubMesh)) )
                {
                    if ( pCurrentGeom != NULL )
                    {
                        ASSERT( CurrentType != TYPE_UNKNOWN );
                        if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                        else                                platform_EndSkinGeom ();
                    }

                    pCurrentGeom                = Inst.Data.Skin.pGeom;
                    CurrentType                 = TYPE_SKIN;
                    CurrentSortData.GeomSubMesh = Inst.SortKey.GeomSubMesh;
                    platform_BeginSkinGeom( pCurrentGeom, Inst.SortKey.GeomSubMesh );
                }

                // let the platform run its render code on the instances
                s32 iInstToRender = s_lSortData[iUniqueInst].iRenderInst;
                if ( Inst.SortKey.GeomType == 0 )
                {
                    while ( iInstToRender != -1 )
                    {
                    #ifndef X_RETAIL
                        if( g_RenderDebug.RenderClippedOnly && !(s_lRenderInst[iInstToRender].Flags & render::CLIPPED) )
                        {
                            iInstToRender = s_lRenderInst[iInstToRender].Brother;
                            continue;
                        }

                        if( g_RenderDebug.RenderShadowedOnly &&
                            !(s_lRenderInst[iInstToRender].Flags & (render::INSTFLAG_PROJ_SHADOW_1 | render::INSTFLAG_PROJ_SHADOW_2 | render::SHADOW_PASS)))
                        {
                            iInstToRender = s_lRenderInst[iInstToRender].Brother;
                            continue;
                        }
                    #endif

                        ASSERT( s_lRenderInst[iInstToRender].SortKey.Bits == Inst.SortKey.Bits );
                        platform_RenderRigidInstance( s_lRenderInst[iInstToRender] );
                        iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    };
                }
                else
                {
                    while ( iInstToRender != -1 )
                    {
                    #ifndef X_RETAIL
                        if( g_RenderDebug.RenderClippedOnly && !(s_lRenderInst[iInstToRender].Flags & render::CLIPPED) )
                        {
                            iInstToRender = s_lRenderInst[iInstToRender].Brother;
                            continue;
                        }

                        if( g_RenderDebug.RenderShadowedOnly &&
                            !(s_lRenderInst[iInstToRender].Flags & (render::INSTFLAG_PROJ_SHADOW_1 | render::INSTFLAG_PROJ_SHADOW_2 | render::SHADOW_PASS)))
                        {
                            iInstToRender = s_lRenderInst[iInstToRender].Brother;
                            continue;
                        }
                    #endif

                        ASSERT( s_lRenderInst[iInstToRender].SortKey.Bits == Inst.SortKey.Bits );
                        platform_RenderSkinInstance ( s_lRenderInst[iInstToRender] );
                        iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    };
                }
            }

            // finish up any pending tasks
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                else                                platform_EndSkinGeom ();
            }
        }
        g_bZPriming = FALSE;

        g_pd3dDevice->SetRenderState(
            D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_ALPHA
                |   D3DCOLORWRITEENABLE_RED
                |   D3DCOLORWRITEENABLE_GREEN
                |   D3DCOLORWRITEENABLE_BLUE
        );
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,CullMode );
    }
}
#endif

//=============================================================================

#ifdef TARGET_XBOX
void render::ZPrimeRenderTarget( void )
{
    platform_BeginZPrime();
    {
        // loop through all of the render instances and render

        sortkey   CurrentSortData;
        CurrentSortData.Bits          = 0xffffffff;
        geom*     pCurrentGeom        = NULL;
        geom_type CurrentType         = TYPE_UNKNOWN;
        for ( s32 iUniqueInst = 0; iUniqueInst < s_LoHashMark; iUniqueInst++ )
        {
            render_instance& Inst = s_lRenderInst[s_lSortData[iUniqueInst].iRenderInst];
            
            // If this is the start of the custom distortion and alpha instances,
            // the bail out of this loop. They'll come later
            if ( Inst.SortKey.RenderOrder >= ORDER_CUSTOM_START )
            {
                // distortion meshes have to be done separately
                s_CustomStart = iUniqueInst;
                break;
            }

            // activate the material if necessary
            if ( CurrentSortData.MatIndex  != Inst.SortKey.MatIndex )
            {
                if ( pCurrentGeom != NULL )
                {
                    ASSERT( CurrentType != TYPE_UNKNOWN );
                    if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                    else                                platform_EndSkinGeom ();
                    pCurrentGeom = NULL;
                }

                #if ENABLE_RENDER_XTIMERS
                StatsTimer.Reset();
                StatsTimer.Start();
                #endif

                material& Mat = s_lRegisteredMaterials[Inst.SortKey.MatIndex];
                ASSERT( (Mat.m_Type != Material_Distortion) &&
                        (Mat.m_Type != Material_Distortion_PerPolyEnv) );

                if( !g_pPipeline->SetZMaterial( Mat ))
                    continue;

                #if ENABLE_RENDER_XTIMERS
                s_RenderStats.m_MaterialActivateTime += StatsTimer.Stop();
                #endif
                
                CurrentSortData.MatIndex  = Inst.SortKey.MatIndex;

                // update the stats
                #if ENABLE_RENDER_STATS
                s_RenderStats.m_nMaterialsRendered++;
                #endif
            }

            // start a new instance batch if necessary (geometry sorting should already
            // be built into the sort key)
            if ( (Inst.SortKey.GeomType == 0) &&
                ((pCurrentGeom != Inst.Data.Rigid.pGeom) || (CurrentSortData.GeomSubMesh != Inst.SortKey.GeomSubMesh)) )
            {
                if ( pCurrentGeom != NULL )
                {
                    ASSERT( CurrentType != TYPE_UNKNOWN );
                    if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                    else                                platform_EndSkinGeom ();
                }

                pCurrentGeom                = Inst.Data.Rigid.pGeom;
                CurrentType                 = TYPE_RIGID;
                CurrentSortData.GeomSubMesh = Inst.SortKey.GeomSubMesh;
                platform_BeginRigidGeom( pCurrentGeom, Inst.SortKey.GeomSubMesh );
            }
            else
            if ( Inst.SortKey.GeomType &&
                ((pCurrentGeom != Inst.Data.Skin.pGeom) || (CurrentSortData.GeomSubMesh != Inst.SortKey.GeomSubMesh)) )
            {
                if ( pCurrentGeom != NULL )
                {
                    ASSERT( CurrentType != TYPE_UNKNOWN );
                    if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                    else                                platform_EndSkinGeom ();
                }

                pCurrentGeom                = Inst.Data.Skin.pGeom;
                CurrentType                 = TYPE_SKIN;
                CurrentSortData.GeomSubMesh = Inst.SortKey.GeomSubMesh;
                platform_BeginSkinGeom( pCurrentGeom, Inst.SortKey.GeomSubMesh );
            }

            // let the platform run its render code on the instances
            s32 iInstToRender = s_lSortData[iUniqueInst].iRenderInst;
            if ( Inst.SortKey.GeomType == 0 )
            {
                while ( iInstToRender != -1 )
                {
                    ASSERT( s_lRenderInst[iInstToRender].SortKey.Bits == Inst.SortKey.Bits );
                    platform_RenderRigidZInstance( s_lRenderInst[iInstToRender] );
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                };
            }
            else
            {
                while ( iInstToRender != -1 )
                {
                    ASSERT( s_lRenderInst[iInstToRender].SortKey.Bits == Inst.SortKey.Bits );
                    platform_RenderSkinZInstance( s_lRenderInst[iInstToRender] );
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                };
            }
        }

        // finish up any pending tasks
        if ( pCurrentGeom != NULL )
        {
            ASSERT( CurrentType != TYPE_UNKNOWN );
            if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
            else                                platform_EndSkinGeom ();
        }
    }
    platform_EndZPrime();
}
#endif

//=============================================================================

#ifdef TARGET_XBOX
void render::RenderLightMap( void )
{
    if( g_pPipeline->m_bPipActive )
        return;
    //if( !s_TotalDynLights ) // should be && "no shadows"
    //    return;

    if( SWITCH_PER_PIXEL_LIGHTING )
    {
        platform_BeginLightMap();
        {
            // loop through all of the render instances and render

            sortkey   CurrentSortData;
            CurrentSortData.Bits          = 0xffffffff;
            geom*     pCurrentGeom        = NULL;
            for ( s32 iUniqueInst = 0; iUniqueInst < s_LoHashMark; iUniqueInst++ )
            {
                render_instance& Inst = s_lRenderInst[s_lSortData[iUniqueInst].iRenderInst];
                
                // If this is the start of the custom distortion and alpha instances,
                // the bail out of this loop. They'll come later
                if ( Inst.SortKey.RenderOrder >= ORDER_CUSTOM_START )
                {
                    // distortion meshes have to be done separately
                    s_CustomStart = iUniqueInst;
                    break;
                }

                // Only rigid geoms go to the light map
                if( Inst.SortKey.GeomType )
                    continue;

                // activate the material if necessary
                if ( CurrentSortData.MatIndex  != Inst.SortKey.MatIndex )
                {
                    if ( pCurrentGeom != NULL )
                    {
                        platform_EndRigidGeom();
                        pCurrentGeom = NULL;
                    }

                    #if ENABLE_RENDER_XTIMERS
                    StatsTimer.Reset();
                    StatsTimer.Start();
                    #endif

                    material& Mat = s_lRegisteredMaterials[Inst.SortKey.MatIndex];
                    ASSERT( (Mat.m_Type != Material_Distortion) &&
                            (Mat.m_Type != Material_Distortion_PerPolyEnv) );

                    if( IsAlphaMaterial( (material_type)Mat.m_Type ))
                        continue;

                    // Xbox only platform call
                    platform_ActivateLitMaterial( Mat );

                    #if ENABLE_RENDER_XTIMERS
                    s_RenderStats.m_MaterialActivateTime += StatsTimer.Stop();
                    #endif
                    
                    CurrentSortData.MatIndex  = Inst.SortKey.MatIndex;

                    // update the stats
                    #if ENABLE_RENDER_STATS
                    s_RenderStats.m_nMaterialsRendered++;
                    #endif
                }

                // start a new instance batch if necessary (geometry sorting should already
                // be built into the sort key)
                if ( ((pCurrentGeom != Inst.Data.Rigid.pGeom) ||
                     (CurrentSortData.GeomSubMesh != Inst.SortKey.GeomSubMesh)) )
                {
                    if ( pCurrentGeom != NULL )
                    {
                        platform_EndRigidGeom();
                    }

                    pCurrentGeom                = Inst.Data.Rigid.pGeom;
                    CurrentSortData.GeomSubMesh = Inst.SortKey.GeomSubMesh;
                    platform_BeginRigidGeom( pCurrentGeom, Inst.SortKey.GeomSubMesh );
                }

                // let the platform run its render code on the instances
                s32 iInstToRender = s_lSortData[iUniqueInst].iRenderInst;
                while ( iInstToRender != -1 )
                {
                    ASSERT( s_lRenderInst[iInstToRender].SortKey.Bits == Inst.SortKey.Bits );
                    platform_RenderLitRigidInstance( s_lRenderInst[iInstToRender] );
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                }
            }

            // finish up any pending tasks
            if ( pCurrentGeom != NULL )
            {
                platform_EndRigidGeom();
            }
        }
        platform_EndLightMap();
    }
}
#endif

//=============================================================================

void render::EndNormalRender( void )
{
    CONTEXT( "render::End" );

    #if ENABLE_RENDER_XTIMERS
    xtimer TotalEndTime;
    TotalEndTime.Start();
    #endif

    // mark that we have no distortion or alpha meshes to render during the custom phase
    s_CustomStart = s_LoHashMark;

    // safety check
    ASSERT( eng_InBeginEnd() );
    ASSERT( s_InRenderBegin );
    s_InRenderBegin = FALSE;

    // bail out early if there are no instances to render
    if ( s_LoHashMark == 0 )
    {
        #if ENABLE_RENDER_XTIMERS
        s_RenderStats.m_TotalEndRenderTime += TotalEndTime.Stop();
        #endif
        return;
    }

    // update the stats
    #if ENABLE_RENDER_STATS
    s_RenderStats.m_nSubMeshesRendered += s_LoHashMark + (kMaxRenderedInstances-1-s_HiHashMark);
    #endif

    // set up the "cube" environment texture
    platform_CreateEnvTexture();

    // set the projected texture matrices
    platform_SetTextureProjectionMatrix( s_TextureProjectionMatrix );
    platform_SetShadowProjectionMatrix( 0, s_ShadowProjectionMatrices[0] );
    platform_SetShadowProjectionMatrix( 1, s_ShadowProjectionMatrices[1] );
    // render the light map from the same

    // let the platform-specific shaders get initialized (whether its d3d vert/pixel
    // shaders, vu0/vu1 microcode, or gamecube passes)
    platform_BeginShaders();

    // sort the render instances (by material and sort key)
    #if ENABLE_RENDER_XTIMERS
    xtimer StatsTimer;
    StatsTimer.Start();
    #endif
    x_qsort( s_lSortData.GetPtr(), s_LoHashMark, sizeof(sort_struct), InstanceCompareFn );
    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_InstanceSortTime += StatsTimer.Stop();
    #endif

#ifdef X_DEBUG
    // sanity check
    SortSanityCheck();
#endif

#ifdef TARGET_PC
    //BeginNormalRender();
    //render::PrimeZBuffer();
#endif

#ifdef TARGET_XBOX
    // Clear all buffers; nestle tightly against priming and lightmap
    g_pPipeline->BeginNormalRender();
    render::ZPrimeRenderTarget();
    render::RenderLightMap();

    bool bLightmapApplied = false;
#endif
{
    // loop through all of the render instances and render those bad boys
    sortkey   CurrentSortData;
    CurrentSortData.Bits          = 0xffffffff;
    geom*     pCurrentGeom        = NULL;
    geom_type CurrentType         = TYPE_UNKNOWN;
    for ( s32 iUniqueInst = 0; iUniqueInst < s_LoHashMark; iUniqueInst++ )
    {
        render_instance& Inst = s_lRenderInst[s_lSortData[iUniqueInst].iRenderInst];
        
        // If this is the start of the custom distortion and alpha instances,
        // the bail out of this loop. They'll come later
        if ( Inst.SortKey.RenderOrder >= ORDER_CUSTOM_START )
        {
            // distortion meshes have to be done separately
            s_CustomStart = iUniqueInst;
            break;
        }

        // activate the material if necessary
        if ( CurrentSortData.MatIndex  != Inst.SortKey.MatIndex )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                else                                platform_EndSkinGeom ();
                pCurrentGeom = NULL;
            }

            #if ENABLE_RENDER_XTIMERS
            StatsTimer.Reset();
            StatsTimer.Start();
            #endif

            material& Mat = s_lRegisteredMaterials[Inst.SortKey.MatIndex];
            ASSERT( (Mat.m_Type != Material_Distortion) &&
                    (Mat.m_Type != Material_Distortion_PerPolyEnv) );

            platform_ActivateMaterial( Mat );
            
            #if ENABLE_RENDER_XTIMERS
            s_RenderStats.m_MaterialActivateTime += StatsTimer.Stop();
            #endif
            
            CurrentSortData.MatIndex  = Inst.SortKey.MatIndex;

            // update the stats
            #if ENABLE_RENDER_STATS
            s_RenderStats.m_nMaterialsRendered++;
            #endif
        }

        // start a new instance batch if necessary (geometry sorting should already
        // be built into the sort key)
        if ( (Inst.SortKey.GeomType == 0) &&
             ((pCurrentGeom != Inst.Data.Rigid.pGeom) || (CurrentSortData.GeomSubMesh != Inst.SortKey.GeomSubMesh)) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                else                                platform_EndSkinGeom ();
            }

            pCurrentGeom                = Inst.Data.Rigid.pGeom;
            CurrentType                 = TYPE_RIGID;
            CurrentSortData.GeomSubMesh = Inst.SortKey.GeomSubMesh;
            platform_BeginRigidGeom( pCurrentGeom, Inst.SortKey.GeomSubMesh );
        }
        else
        if ( Inst.SortKey.GeomType &&
             ((pCurrentGeom != Inst.Data.Skin.pGeom) || (CurrentSortData.GeomSubMesh != Inst.SortKey.GeomSubMesh)) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                else                                platform_EndSkinGeom ();
            }

            pCurrentGeom                = Inst.Data.Skin.pGeom;
            CurrentType                 = TYPE_SKIN;
            CurrentSortData.GeomSubMesh = Inst.SortKey.GeomSubMesh;
            platform_BeginSkinGeom( pCurrentGeom, Inst.SortKey.GeomSubMesh );
        }

        // let the platform run its render code on the instances
        s32 iInstToRender = s_lSortData[iUniqueInst].iRenderInst;
        if ( Inst.SortKey.GeomType == 0 )
        {
            while ( iInstToRender != -1 )
            {
#ifndef X_RETAIL
                if( g_RenderDebug.RenderClippedOnly && !(s_lRenderInst[iInstToRender].Flags & render::CLIPPED) )
                {
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    continue;
                }

                if( g_RenderDebug.RenderShadowedOnly &&
                    !(s_lRenderInst[iInstToRender].Flags & (render::INSTFLAG_PROJ_SHADOW_1 | render::INSTFLAG_PROJ_SHADOW_2 | render::SHADOW_PASS)))
                {
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    continue;
                }
#endif

                ASSERT( s_lRenderInst[iInstToRender].SortKey.Bits == Inst.SortKey.Bits );
                platform_RenderRigidInstance( s_lRenderInst[iInstToRender] );
                iInstToRender = s_lRenderInst[iInstToRender].Brother;
            };
        }
        else
        {
            while ( iInstToRender != -1 )
            {
#ifndef X_RETAIL
                if( g_RenderDebug.RenderClippedOnly && !(s_lRenderInst[iInstToRender].Flags & render::CLIPPED) )
                {
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    continue;
                }

                if( g_RenderDebug.RenderShadowedOnly &&
                    !(s_lRenderInst[iInstToRender].Flags & (render::INSTFLAG_PROJ_SHADOW_1 | render::INSTFLAG_PROJ_SHADOW_2 | render::SHADOW_PASS)))
                {
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    continue;
                }
#endif

                ASSERT( s_lRenderInst[iInstToRender].SortKey.Bits == Inst.SortKey.Bits );
                platform_RenderSkinInstance ( s_lRenderInst[iInstToRender] );
                iInstToRender = s_lRenderInst[iInstToRender].Brother;
            };
        }
    }

    // finish up any pending tasks
    if ( pCurrentGeom != NULL )
    {
        ASSERT( CurrentType != TYPE_UNKNOWN );
        if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
        else                                platform_EndSkinGeom ();
    }
}

    // let the microcode or whatever finish up
    platform_EndShaders();
    platform_EndNormalRender();

    // stats update
    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_TotalEndRenderTime += TotalEndTime.Stop();
    #endif
}

//=============================================================================

void render::BeginCustomRender( void )
{
    // safety check
    ASSERT( eng_InBeginEnd() );
    ASSERT( !s_InRenderBegin && !s_InShadowBegin && !s_InRawBegin && !s_InCustomBegin );
    s_InCustomBegin = TRUE;
}

//=============================================================================

void render::EndCustomRender( void )
{
    #if ENABLE_RENDER_XTIMERS
    xtimer TotalEndTime;
    TotalEndTime.Start();
    #endif

    // safety check
    ASSERT( eng_InBeginEnd() );
    ASSERT( s_InCustomBegin );
    s_InCustomBegin = FALSE;

    // bail out early if there are no distorted meshes to render
    if ( s_CustomStart == s_LoHashMark )
        return;

    // let the platform-specific shaders get initialized (whether its d3d vert/pixel
    // shaders, vu0/vu1 microcode, or gamecube passes)
    platform_BeginShaders();

    // loop through all of the render instances and render those bad boys
    // now handle the distorted meshes
    sortkey   CurrentSortData;
    CurrentSortData.Bits        = 0xffffffff;
    CurrentSortData.RenderOrder = ORDER_OPAQUE;
    geom*     pCurrentGeom      = NULL;
    geom_type CurrentType       = TYPE_UNKNOWN;

    for ( s32 iUniqueInst = s_CustomStart; iUniqueInst < s_LoHashMark; iUniqueInst++ )
    {
        render_instance& Inst = s_lRenderInst[s_lSortData[iUniqueInst].iRenderInst];

        ASSERT( Inst.SortKey.RenderOrder >= ORDER_CUSTOM_START );

        // set up a z-prime material or distortion map for the first time if necessary
        if ( CurrentSortData.RenderOrder != Inst.SortKey.RenderOrder )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                else                                platform_EndSkinGeom ();
                pCurrentGeom = NULL;
            }

            CurrentSortData.MatIndex    = 0x3ff;
            CurrentSortData.RenderOrder = Inst.SortKey.RenderOrder;

            if ( CurrentSortData.RenderOrder == ORDER_ZPRIME )
            {
                platform_ActivateZPrimeMaterial();
            }
            else
            if ( CurrentSortData.RenderOrder == ORDER_DISTORTION )
                platform_BeginDistortion();
        }

        // the distortion and fading alpha materials need to get set properly
        if ( ((CurrentSortData.RenderOrder == ORDER_FORCED_LAST) ||
              (CurrentSortData.RenderOrder == ORDER_FADING_ALPHA)) &&
             (CurrentSortData.MatIndex != Inst.SortKey.MatIndex) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                else                                platform_EndSkinGeom ();
                pCurrentGeom = NULL;
            }

            #if ENABLE_RENDER_XTIMERS
            StatsTimer.Reset();
            StatsTimer.Start();
            #endif
            material& Mat = s_lRegisteredMaterials[Inst.SortKey.MatIndex];
            platform_ActivateMaterial( Mat );
            CurrentSortData.MatIndex  = Inst.SortKey.MatIndex;
            #if ENABLE_RENDER_XTIMERS
            s_RenderStats.m_MaterialActivateTime += StatsTimer.Stop();
            #endif
            #if ENABLE_RENDER_STATS
            s_RenderStats.m_nMaterialsRendered++;
            #endif
        }
        else
        if ( (CurrentSortData.RenderOrder == ORDER_DISTORTION) &&
             (CurrentSortData.MatIndex != Inst.SortKey.MatIndex) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                else                                platform_EndSkinGeom ();
                pCurrentGeom = NULL;
            }

            #if ENABLE_RENDER_XTIMERS
            StatsTimer.Reset();
            StatsTimer.Start();
            #endif
            if ( Inst.OverrideMat )
            {
                const distortion_info& DistortInfo = s_lDistortionInfo[(s32)Inst.SortKey.MatIndex];
                if ( DistortInfo.MatIndex != 0xffffffff )
                {
                    material& Mat = s_lRegisteredMaterials[DistortInfo.MatIndex];
                    platform_ActivateDistortionMaterial( &Mat, DistortInfo.NormalRot );
                }
                else
                {
                    platform_ActivateDistortionMaterial( NULL, DistortInfo.NormalRot );
                }
            }
            else
            {
                material& Mat = s_lRegisteredMaterials[Inst.SortKey.MatIndex];
                platform_ActivateMaterial( Mat );
            }
            
            CurrentSortData.MatIndex  = Inst.SortKey.MatIndex;
            #if ENABLE_RENDER_XTIMERS
            s_RenderStats.m_MaterialActivateTime += StatsTimer.Stop();
            #endif
            #if ENABLE_RENDER_STATS
            s_RenderStats.m_nMaterialsRendered++;
            #endif
        }

        // start a new instance batch if necessary (geometry sorting should already
        // be built into the sort key)
        if ( (Inst.SortKey.GeomType == 0) &&
            ((pCurrentGeom != Inst.Data.Rigid.pGeom) || (CurrentSortData.GeomSubMesh != Inst.SortKey.GeomSubMesh)) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                else                                platform_EndSkinGeom ();
            }

            pCurrentGeom                = Inst.Data.Rigid.pGeom;
            CurrentType                 = TYPE_RIGID;
            CurrentSortData.GeomSubMesh = Inst.SortKey.GeomSubMesh;
            platform_BeginRigidGeom( pCurrentGeom, Inst.SortKey.GeomSubMesh );
        }
        else
        if ( Inst.SortKey.GeomType &&
            ((pCurrentGeom != Inst.Data.Skin.pGeom) || (CurrentSortData.GeomSubMesh != Inst.SortKey.GeomSubMesh)) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
                else                                platform_EndSkinGeom ();
            }

            pCurrentGeom                = Inst.Data.Skin.pGeom;
            CurrentType                 = TYPE_SKIN;
            CurrentSortData.GeomSubMesh = Inst.SortKey.GeomSubMesh;
            platform_BeginSkinGeom( pCurrentGeom, Inst.SortKey.GeomSubMesh );
        }

        // let the platform run its render code on the instances
        s32 iInstToRender = s_lSortData[iUniqueInst].iRenderInst;
        if ( Inst.SortKey.GeomType == 0 )
        {
            while ( iInstToRender != -1 )
            {
#ifndef X_RETAIL
                if( g_RenderDebug.RenderClippedOnly && !(s_lRenderInst[iInstToRender].Flags & render::CLIPPED) )
                {
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    continue;
                }

                if( g_RenderDebug.RenderShadowedOnly &&
                    !(s_lRenderInst[iInstToRender].Flags & (render::INSTFLAG_PROJ_SHADOW_1 | render::INSTFLAG_PROJ_SHADOW_2 | render::SHADOW_PASS)))
                {
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    continue;
                }
#endif

                ASSERT( s_lRenderInst[iInstToRender].SortKey.Bits == Inst.SortKey.Bits );
                platform_RenderRigidInstance( s_lRenderInst[iInstToRender] );
                iInstToRender = s_lRenderInst[iInstToRender].Brother;
            };
        }
        else
        {
            while ( iInstToRender != -1 )
            {
#ifndef X_RETAIL
                if( g_RenderDebug.RenderClippedOnly && !(s_lRenderInst[iInstToRender].Flags & render::CLIPPED) )
                {
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    continue;
                }

                if( g_RenderDebug.RenderShadowedOnly &&
                    !(s_lRenderInst[iInstToRender].Flags & (render::INSTFLAG_PROJ_SHADOW_1 | render::INSTFLAG_PROJ_SHADOW_2 | render::SHADOW_PASS)))
                {
                    iInstToRender = s_lRenderInst[iInstToRender].Brother;
                    continue;
                }
#endif

                ASSERT( s_lRenderInst[iInstToRender].SortKey.Bits == Inst.SortKey.Bits );
                platform_RenderSkinInstance ( s_lRenderInst[iInstToRender] );
                iInstToRender = s_lRenderInst[iInstToRender].Brother;
            };
        }
    }

    // finish up any pending tasks
    if ( pCurrentGeom != NULL )
    {
        ASSERT( CurrentType != TYPE_UNKNOWN );
        if ( CurrentType == TYPE_RIGID )    platform_EndRigidGeom();
        else                                platform_EndSkinGeom ();
    }

    // end distortion
    if( CurrentSortData.RenderOrder == ORDER_DISTORTION )
        platform_EndDistortion();

    // let the microcode or whatever finish up
    platform_EndShaders();

    // stats update
    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_TotalEndRenderTime += TotalEndTime.Stop();
    #endif
}

//=============================================================================

void render::ResetAfterException( void )
{
    s_InRenderBegin = FALSE;
    s_InShadowBegin = FALSE;
    s_InRawBegin = FALSE;
}

//=============================================================================

void render::AddRigidInstanceSimple( hgeom_inst     hInst,
                                     const void*    pCol,
                                     const matrix4* pL2W,
                                     const bbox&    WorldBBox,
                                     u32            Flags )
{
#ifndef X_RETAIL
    if( g_RenderDebug.RenderSkinOnly )
        return;
#endif

    #if ENABLE_RENDER_XTIMERS
    xtimer AddTime;
    AddTime.Start();
    #endif

    // safety check
    ASSERT( s_InRenderBegin );
    ASSERT( pL2W->IsValid() );
#ifdef TARGET_PS2
    ASSERT( ((u32)pCol & 0xf) == 0 );
#endif
    // grab the useful pointers out
    private_instance& RegisteredInst = s_lRegisteredInst( hInst );
    ASSERT( RegisteredInst.Type == TYPE_RIGID );
    rigid_geom* pGeom = (rigid_geom*)RegisteredInst.pGeom;

    // calculate lighting
    void* pLighting = NULL;
    if( Flags & DO_SIMPLE_LIGHTING )
    {
        pLighting = platform_CalculateRigidLighting( *pL2W, WorldBBox );
        if ( pLighting )
            Flags |= INSTFLAG_DYNAMICLIGHT;
    }

    // collect texture projections
    u32 ProjFlags = CollectProjectionInfo( Flags, WorldBBox );

    // Use filter light?
    if ( ( s_bFilterLight ) && ( (Flags & DISABLE_FILTERLIGHT) == 0 ) )
        Flags |= INSTFLAG_FILTERLIGHT;

    // add each of the submeshes to the render list
    for ( s32 iMesh = 0; iMesh < pGeom->m_nMeshes; iMesh++ )
    {
        geom::mesh& Mesh = pGeom->m_pMesh[iMesh];
        for ( s32 iSubMesh = Mesh.iSubMesh; iSubMesh < Mesh.iSubMesh+Mesh.nSubMeshes; iSubMesh++ )
        {
            geom::submesh&  SubMesh  = pGeom->m_pSubMesh[iSubMesh];
        
            // get the material handle info
            geom::material& Material      = pGeom->m_pMaterial[SubMesh.iMaterial];
            xhandle         hMat          = pGeom->m_pVirtualMaterials[Material.iVirtualMat].MatHandle;
            ASSERT( (hMat>=0) && (hMat<kMaxRegisteredMaterials) );

            // set the color pointer
			#if defined(TARGET_XBOX) || defined(TARGET_PC)
                const u32* pInstCol=( u32* )pCol;
            #elif defined(TARGET_PS2)
                const u16* pInstCol=( u16* )pCol;
            #else
            #endif

            #ifdef TARGET_XBOX
            rigid_geom::dlist_xbox& DList = pGeom->m_System.pXbox[SubMesh.iDList];
            ASSERT( DList.iColor <= pGeom->m_nVertices );
            if( pInstCol )
                pInstCol += DList.iColor;
            #endif

            #ifdef TARGET_PS2
            rigid_geom::dlist_ps2& DList = pGeom->m_System.pPS2[SubMesh.iDList];
            if( pInstCol )
                pInstCol += DList.iColor;
            #endif

            // make sure we're aligned for dma purposes
            #ifdef TARGET_PS2
            ASSERT( ALIGN_16(pL2W) == (s32)pL2W );
            #endif

            // figure out the sort key
            sortkey SortKey;
            SortKey.Bits     = SubMesh.BaseSortKey;
            SortKey.MatIndex = s_lRegisteredMaterials.GetIndexByHandle(hMat);

            // fill in the basic render instance info
            render_instance& Inst = AddToHashHybrid( SortKey.Bits );
            Inst.Flags            = Flags;
            Inst.OverrideMat      = FALSE;
            Inst.Alpha            = 255;

            // get scrolling uv information
            GetUVOffset( Inst.UOffset, Inst.VOffset, pGeom, s_lRegisteredMaterials(hMat) );

            // fill in the rigid geom instance info
            Inst.Data.Rigid.pGeom    = pGeom;
            Inst.Data.Rigid.pL2W     = pL2W;
            Inst.Data.Rigid.pColInfo = pInstCol;

            // fill in the lighting
            Inst.pLighting = pLighting;

            // do texture projections
            if( (Inst.SortKey.RenderOrder < ORDER_ZPRIME) &&
                CanHaveProjTexture( Material ) )
            {
                Inst.Flags |= ProjFlags;
            }

            #ifdef TARGET_PC
            Inst.hDList = RegisteredInst.RigidDList[(s32)SubMesh.iDList];
            #endif // TARGET_PC
        }
    }

    // stats update
    #if ENABLE_RENDER_STATS
    s_RenderStats.m_nInstancesRendered++;
    s_RenderStats.m_nTrisRendered     += pGeom->m_nFaces;
    s_RenderStats.m_nVerticesRendered += pGeom->m_nVertices;
    #endif

    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_InstanceAddTime   += AddTime.Stop();
    #endif
}

//=============================================================================

void render::AddRigidInstance( hgeom_inst     hInst,
                               const void*    pCol,
                               const matrix4* pL2W,
                               u64            Mask,
                               u32            Flags,
                               s32            Alpha )
{
#ifndef X_RETAIL
    if( g_RenderDebug.RenderSkinOnly )
        return;
#endif

    CONTEXT( "render::AddRigidInstance" );

    #if ENABLE_RENDER_XTIMERS
    xtimer AddTime;
    AddTime.Start();
    #endif

    // safety check
    ASSERT( s_InRenderBegin );
    ASSERT( pL2W->IsValid() );
#ifdef TARGET_PS2
    ASSERT( ((u32)pCol & 0xf) == 0 );
#endif

    // stats update
    #if ENABLE_RENDER_STATS
    s_RenderStats.m_nInstancesRendered++;
    #endif

    // grab the useful pointers out
    private_instance& RegisteredInst = s_lRegisteredInst( hInst );
    ASSERT( RegisteredInst.Type == TYPE_RIGID );
    rigid_geom* pGeom = (rigid_geom*)RegisteredInst.pGeom;

    // calculate lighting
    bbox WorldBBox( pGeom->m_BBox );
    WorldBBox.Transform( *pL2W );
    void* pLighting = platform_CalculateRigidLighting( *pL2W, WorldBBox );
    if ( pLighting )
        Flags |= INSTFLAG_DYNAMICLIGHT;

    // collect texture projections
    u32 ProjFlags = CollectProjectionInfo( Flags, WorldBBox );

    // Use filter light?
    if ( ( s_bFilterLight ) && ( (Flags & DISABLE_FILTERLIGHT) == 0 ) )
        Flags |= INSTFLAG_FILTERLIGHT;

    // fading alpha?
    if ( Alpha != 255 )
        Flags |= INSTFLAG_FADING_ALPHA;

    // add the meshes and submeshes to the render list
    s32         iMesh    = 0;
    geom::mesh* pMesh    = pGeom->m_pMesh;
    geom::mesh* pEndMesh = pMesh + pGeom->m_nMeshes;
    while ( pMesh < pEndMesh )
    {
        // skip this mesh?
        if( (Mask & 1) == 0 )
        {
            pMesh++;
            iMesh++;
            Mask >>= 1;
            continue;
        }

        // add each of the submeshes to the render list
        for ( s32 iSubMesh = pMesh->iSubMesh;
              iSubMesh < pMesh->iSubMesh+pMesh->nSubMeshes;
              iSubMesh++ )
        {
            geom::submesh&  SubMesh  = pGeom->m_pSubMesh[iSubMesh];
            geom::material& Material = pGeom->m_pMaterial[SubMesh.iMaterial];

            // get the material handle info
            xhandle hMat = pGeom->m_pVirtualMaterials[Material.iVirtualMat].MatHandle;

            // range safety check for the sort key
            ASSERT( (pGeom->m_hGeom>=0) && (pGeom->m_hGeom<kMaxRegisteredGeoms    ) );
            ASSERT( (hMat          >=0) && (hMat          <kMaxRegisteredMaterials) );
            ASSERT( (iSubMesh      >=0) && (iSubMesh      <256                    ) );

            // figure out the bone we should render with
            #ifdef TARGET_PC
            s32 iBone = pGeom->m_System.pPC  [SubMesh.iDList].iBone;
            #elif defined(TARGET_XBOX)
            s32 iBone = pGeom->m_System.pXbox[SubMesh.iDList].iBone;
            #elif defined(TARGET_PS2)
            s32 iBone = pGeom->m_System.pPS2 [SubMesh.iDList].iBone;
            #else
            s32 iBone = 0;
            #error unknown target
            #endif

            // set the color pointer
			#if defined(TARGET_XBOX) || defined(TARGET_PC)
                const u32* pInstCol=( u32* )pCol;
            #elif defined(TARGET_PS2)
                const u16* pInstCol=( u16* )pCol;
            #else
            #endif

            #ifdef TARGET_XBOX
            rigid_geom::dlist_xbox& DList = pGeom->m_System.pXbox[SubMesh.iDList];
            ASSERT( DList.iColor <= pGeom->m_nVertices );
            if( pInstCol )
                pInstCol += DList.iColor;
            #endif

            #ifdef TARGET_PS2
            rigid_geom::dlist_ps2& DList = pGeom->m_System.pPS2[SubMesh.iDList];
            if( pInstCol )
                pInstCol += DList.iColor;
            #endif

            // build the sort key
            sortkey SortKey;
            SortKey.Bits        = 0;
            SortKey.GeomSubMesh = iSubMesh;
            SortKey.GeomHandle  = pGeom->m_hGeom;
            SortKey.GeomType    = 0;
            SortKey.MatIndex    = s_lRegisteredMaterials.GetIndexByHandle(hMat);
            SortKey.RenderOrder = GetRenderOrder( (material_type)Material.Type );
            if ( (Flags & render::FADING_ALPHA) && (SortKey.RenderOrder < ORDER_FADING_ALPHA) )
                SortKey.RenderOrder = ORDER_FADING_ALPHA;

            // make a copy of the l2w in smem that we can ref to
            matrix4* pMat = (matrix4*)smem_BufferAlloc(sizeof(matrix4));
            #ifdef TARGET_PS2
            if ( ((u32)pL2W & 0xf) == 0 )
            {
                ASSERT( ((u32)pMat & 0xf) == 0 );
                ASSERT( ALIGN_16(pMat) == (s32)pMat );
                ASSERT( ALIGN_16(pL2W) == (s32)pL2W );

                u_long128* pSrc = (u_long128*)(pL2W + iBone);
                u_long128* pDst = (u_long128*)pMat;
                pDst[0] = pSrc[0];
                pDst[1] = pSrc[1];
                pDst[2] = pSrc[2];
                pDst[3] = pSrc[3];
            }
            else
            #endif
            {
                *pMat = *(pL2W + iBone);
                ASSERT( pMat->IsValid() );
            }

            // fill in the basic render instance info
            render_instance& Inst    = AddToHashHybrid( SortKey.Bits );
            Inst.SortKey             = SortKey;
            Inst.Flags               = Flags;
            Inst.OverrideMat         = FALSE;
            Inst.Alpha               = Alpha;

            // get scrolling uv information
            GetUVOffset( Inst.UOffset, Inst.VOffset, pGeom, s_lRegisteredMaterials(hMat) );

            // fill in the rigid geom instance info
            Inst.Data.Rigid.pGeom    = pGeom;
            Inst.Data.Rigid.pL2W     = pMat;
            Inst.Data.Rigid.pColInfo = pInstCol;

            // fill in the lighting
            Inst.pLighting = pLighting;

            // do texture projections
            if( (Inst.SortKey.RenderOrder < ORDER_ZPRIME) &&
                CanHaveProjTexture( Material ) )
            {
                Inst.Flags |= ProjFlags;
            }

            #ifdef TARGET_PC
            Inst.hDList = RegisteredInst.RigidDList[(s32)SubMesh.iDList];
            #endif // TARGET_PC

            // handle fading geometry
            if ( Flags & render::FADING_ALPHA )
            {
                // we need to render twice to prime the z-buffer for alpha geometry
                SortKey.GeomSubMesh         = iSubMesh;
                SortKey.GeomHandle          = pGeom->m_hGeom;
                SortKey.GeomType            = 0;
                SortKey.MatIndex            = 0x3ff;
                SortKey.RenderOrder         = ORDER_ZPRIME;
                render_instance& ZPrimeInst = AddToHashHybrid( SortKey.Bits );
                ZPrimeInst.Flags            = Inst.Flags & render::CLIPPED;
                ZPrimeInst.pLighting        = pLighting;
                ZPrimeInst.Data             = Inst.Data;
                ZPrimeInst.UOffset          = Inst.UOffset;
                ZPrimeInst.VOffset          = Inst.VOffset;
                ZPrimeInst.Alpha            = 0x80;
                ZPrimeInst.OverrideMat      = 1;
                #ifdef TARGET_PC
                ZPrimeInst.hDList           = Inst.hDList;
                #endif
            }
        }

        // update the stats
        #if ENABLE_RENDER_STATS
        s_RenderStats.m_nVerticesRendered += pMesh->nVertices;
        s_RenderStats.m_nTrisRendered     += pMesh->nFaces;
        #endif

        // next mesh
        iMesh++;
        pMesh++;
        Mask >>= 1;
    }

    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_InstanceAddTime += AddTime.Stop();
    #endif
}

//=============================================================================

void render::AddRigidInstance( hgeom_inst        hInst,
                               const void*       pCol,
                               const matrix4*    pL2W,
                               u64               Mask,
                               u32               VTextureMask,
                               u32               Flags,
                               s32               Alpha )
{
#ifndef X_RETAIL
    if( g_RenderDebug.RenderSkinOnly )
        return;
#endif

    CONTEXT( "render::AddRigidInstance" );

#if ENABLE_RENDER_XTIMERS
    xtimer AddTime;
    AddTime.Start();
#endif

    // safety check
    ASSERT( s_InRenderBegin );
    ASSERT( pL2W->IsValid() );
#ifdef TARGET_PS2
    ASSERT( ((u32)pCol & 0xf) == 0 );
#endif

    // stats update
#if ENABLE_RENDER_STATS
    s_RenderStats.m_nInstancesRendered++;
#endif

    // grab the useful pointers out
    private_instance& RegisteredInst = s_lRegisteredInst( hInst );
    ASSERT( RegisteredInst.Type == TYPE_RIGID );
    rigid_geom* pGeom = (rigid_geom*)RegisteredInst.pGeom;

    // calculate lighting
    bbox WorldBBox( pGeom->m_BBox );
    WorldBBox.Transform( *pL2W );
    void* pLighting = platform_CalculateRigidLighting( *pL2W, WorldBBox );
    if ( pLighting )
        Flags |= INSTFLAG_DYNAMICLIGHT;

    // collect texture projections
    u32 ProjFlags = CollectProjectionInfo( Flags, WorldBBox );

    // Use filter light?
    if ( ( s_bFilterLight ) && ( (Flags & DISABLE_FILTERLIGHT) == 0 ) )
        Flags |= INSTFLAG_FILTERLIGHT;

    // fading alpha?
    if ( Alpha != 255 )
        Flags |= INSTFLAG_FADING_ALPHA;

    // calculate the virtual mesh offsets
    s32 VMatOffsets[32];
    CalcVMatOffsets( VMatOffsets, pGeom, VTextureMask );

    // add the meshes and submeshes to the render list
    s32         iMesh    = 0;
    geom::mesh* pMesh    = pGeom->m_pMesh;
    geom::mesh* pEndMesh = pMesh + pGeom->m_nMeshes;
    while ( pMesh < pEndMesh )
    {
        // skip this mesh?
        if( (Mask & 1) == 0 )
        {
            pMesh++;
            iMesh++;
            Mask >>= 1;
            continue;
        }

        // add each of the submeshes to the render list
        for ( s32 iSubMesh = pMesh->iSubMesh;
            iSubMesh < pMesh->iSubMesh+pMesh->nSubMeshes;
            iSubMesh++ )
        {
            geom::submesh&  SubMesh  = pGeom->m_pSubMesh[iSubMesh];
            geom::material& Material = pGeom->m_pMaterial[SubMesh.iMaterial];

            // get the material handle info
            xhandle hMat = pGeom->m_pVirtualMaterials[Material.iVirtualMat + VMatOffsets[SubMesh.iMaterial]].MatHandle;

            // range safety check for the sort key
            ASSERT( (pGeom->m_hGeom>=0) && (pGeom->m_hGeom<kMaxRegisteredGeoms    ) );
            ASSERT( (hMat          >=0) && (hMat          <kMaxRegisteredMaterials) );
            ASSERT( (iSubMesh      >=0) && (iSubMesh      <256                    ) );

            // figure out the bone we should render with
            #ifdef TARGET_PC
            s32 iBone = pGeom->m_System.pPC  [SubMesh.iDList].iBone;
            #elif defined(TARGET_XBOX)
            s32 iBone = pGeom->m_System.pXbox[SubMesh.iDList].iBone;
            #elif defined(TARGET_PS2)
            s32 iBone = pGeom->m_System.pPS2 [SubMesh.iDList].iBone;
            #else
            s32 iBone = 0;
            #error unknown target
            #endif

            // set the color pointer
            #if defined(TARGET_XBOX) || defined(TARGET_PC)
            const u32* pInstCol=( u32* )pCol;
            #elif defined(TARGET_PS2)
            const u16* pInstCol=( u16* )pCol;
            #else
            #endif

            #ifdef TARGET_XBOX
            rigid_geom::dlist_xbox& DList = pGeom->m_System.pXbox[SubMesh.iDList];
            ASSERT( DList.iColor <= pGeom->m_nVertices );
            if( pInstCol )
                pInstCol += DList.iColor;
            #endif
		    
            #ifdef TARGET_PS2
            rigid_geom::dlist_ps2& DList = pGeom->m_System.pPS2[SubMesh.iDList];
            if( pInstCol )
                pInstCol += DList.iColor;
            #endif

            // build the sort key
            sortkey SortKey;
            SortKey.Bits        = 0;
            SortKey.GeomSubMesh = iSubMesh;
            SortKey.GeomHandle  = pGeom->m_hGeom;
            SortKey.GeomType    = 0;
            SortKey.MatIndex    = s_lRegisteredMaterials.GetIndexByHandle(hMat);
            SortKey.RenderOrder = GetRenderOrder( (material_type)Material.Type );
            if ( (Flags & render::FADING_ALPHA) && (SortKey.RenderOrder < ORDER_FADING_ALPHA) )
                SortKey.RenderOrder = ORDER_FADING_ALPHA;

            // make a copy of the l2w in smem that we can ref to
            matrix4* pMat = (matrix4*)smem_BufferAlloc(sizeof(matrix4));
            #ifdef TARGET_PS2
            if ( ((u32)pL2W & 0xf) == 0 )
            {
                ASSERT( ((u32)pMat & 0xf) == 0 );
                ASSERT( ALIGN_16(pMat) == (s32)pMat );
                ASSERT( ALIGN_16(pL2W) == (s32)pL2W );

                u_long128* pSrc = (u_long128*)(pL2W + iBone);
                u_long128* pDst = (u_long128*)pMat;
                pDst[0] = pSrc[0];
                pDst[1] = pSrc[1];
                pDst[2] = pSrc[2];
                pDst[3] = pSrc[3];
            }
            else
            #endif
            {
                *pMat = *(pL2W + iBone);
                ASSERT( pMat->IsValid() );
            }

            // fill in the basic render instance info
            render_instance& Inst    = AddToHashHybrid( SortKey.Bits );
            Inst.SortKey             = SortKey;
            Inst.Flags               = Flags;
            Inst.OverrideMat         = FALSE;
            Inst.Alpha               = Alpha;

            // get scrolling uv information
            GetUVOffset( Inst.UOffset, Inst.VOffset, pGeom, s_lRegisteredMaterials(hMat) );

            // fill in the rigid geom instance info
            Inst.Data.Rigid.pGeom    = pGeom;
            Inst.Data.Rigid.pL2W     = pMat;
            Inst.Data.Rigid.pColInfo = pInstCol;

            // fill in the lighting
            Inst.pLighting = pLighting;

            // do texture projections
            if( (Inst.SortKey.RenderOrder < ORDER_ZPRIME) &&
                CanHaveProjTexture( Material ) )
            {
                Inst.Flags |= ProjFlags;
            }

            #ifdef TARGET_PC
            Inst.hDList = RegisteredInst.RigidDList[(s32)SubMesh.iDList];
            #endif // TARGET_PC

            // handle fading geometry
            if ( Flags & render::FADING_ALPHA )
            {
                // we need to render twice to prime the z-buffer for alpha geometry
                SortKey.GeomSubMesh         = iSubMesh;
                SortKey.GeomHandle          = pGeom->m_hGeom;
                SortKey.GeomType            = 0;
                SortKey.MatIndex            = 0x3ff;
                SortKey.RenderOrder         = ORDER_ZPRIME;
                render_instance& ZPrimeInst = AddToHashHybrid( SortKey.Bits );
                ZPrimeInst.Flags            = Inst.Flags & render::CLIPPED;
                ZPrimeInst.pLighting        = pLighting;
                ZPrimeInst.Data             = Inst.Data;
                ZPrimeInst.UOffset          = Inst.UOffset;
                ZPrimeInst.VOffset          = Inst.VOffset;
                ZPrimeInst.Alpha            = 0x80;
                ZPrimeInst.OverrideMat      = 1;
                #ifdef TARGET_PC
                ZPrimeInst.hDList           = Inst.hDList;
                #endif
            }
        }

        // update the stats
        #if ENABLE_RENDER_STATS
        s_RenderStats.m_nVerticesRendered += pMesh->nVertices;
        s_RenderStats.m_nTrisRendered     += pMesh->nFaces;
        #endif

        // next mesh
        iMesh++;
        pMesh++;
        Mask >>= 1;
    }

    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_InstanceAddTime += AddTime.Stop();
    #endif
}

//=============================================================================

void render::AddSkinInstance( hgeom_inst     hInst,
                              const matrix4* pBone,
                              u64            Mask,
                              u32            VTextureMask,
                              u32            Flags,
                              const xcolor&  Ambient )
{
#ifndef X_RETAIL
    if( g_RenderDebug.RenderRigidOnly )
        return;
#endif

    CONTEXT( "render::AddSkinInstance" );

    #if ENABLE_RENDER_XTIMERS
    xtimer AddTime;
    AddTime.Start();
    #endif

    // safety check
    ASSERT( s_InRenderBegin );

    // stats update
    #if ENABLE_RENDER_STATS
    s_RenderStats.m_nInstancesRendered++;
    #endif

    // grab the useful pointers out
    private_instance& RegisteredInst = s_lRegisteredInst( hInst );
    ASSERT( RegisteredInst.Type == TYPE_SKIN );
    skin_geom* pGeom = (skin_geom*)RegisteredInst.pGeom;

    // calculate lighting
    void* pLighting = platform_CalculateSkinLighting( Flags, pBone[0], pGeom->m_BBox, Ambient );
    Flags |= INSTFLAG_DYNAMICLIGHT;

    // collect texture projections
    bbox WorldBBox( pGeom->m_BBox );
    WorldBBox.Transform( pBone[0] );
    u32 ProjFlags = CollectProjectionInfo( Flags, WorldBBox );

    // calculate the virtual mesh offsets
    s32 VMatOffsets[32];
    CalcVMatOffsets( VMatOffsets, pGeom, VTextureMask );

    // add the meshes and submeshes to the render list
    for ( s32 iMesh = 0; iMesh < pGeom->m_nMeshes; iMesh++ )
    {
        // skip this mesh?
        if( (Mask & ((u64)1 << iMesh)) == 0 )
            continue;

        // add each of the submeshes to the render list
        geom::mesh& Mesh = pGeom->m_pMesh[iMesh];
        for ( s32 iSubMesh = Mesh.iSubMesh;
              iSubMesh < Mesh.iSubMesh+Mesh.nSubMeshes;
              iSubMesh++ )
        {
            geom::submesh&  SubMesh  = pGeom->m_pSubMesh[iSubMesh];
            geom::material& Material = pGeom->m_pMaterial[SubMesh.iMaterial];

            // get the material handle info
            xhandle hMat = pGeom->m_pVirtualMaterials[Material.iVirtualMat + VMatOffsets[SubMesh.iMaterial]].MatHandle;

            // range safety check for the sort key
            ASSERT( (pGeom->m_hGeom>=0) && (pGeom->m_hGeom<kMaxRegisteredGeoms    ) );
            ASSERT( (hMat          >=0) && (hMat          <kMaxRegisteredMaterials) );
            ASSERT( (iSubMesh      >=0) && (iSubMesh      <256                    ) );

            // build the sort key
            sortkey SortKey;
            SortKey.Bits        = 0;
            SortKey.GeomSubMesh = iSubMesh;
            SortKey.GeomHandle  = pGeom->m_hGeom;
            SortKey.GeomType    = 1;
            SortKey.MatIndex    = s_lRegisteredMaterials.GetIndexByHandle(hMat);
            SortKey.RenderOrder = GetRenderOrder( (material_type)Material.Type );
            if ( (Flags & render::FADING_ALPHA) && (SortKey.RenderOrder < ORDER_FADING_ALPHA) )
                SortKey.RenderOrder = ORDER_FADING_ALPHA;
            if ( (Flags & render::GLOWING) && (SortKey.RenderOrder < ORDER_GLOWING) )
                SortKey.RenderOrder = ORDER_GLOWING;
            if ( (Flags & render::FORCE_LAST) && (SortKey.RenderOrder < ORDER_FORCED_LAST) )
                SortKey.RenderOrder = ORDER_FORCED_LAST;

            // fill in the basic render instance info
            render_instance& Inst    = AddToHashHybrid( SortKey.Bits );
            Inst.SortKey             = SortKey;
            Inst.Flags               = Flags;
            Inst.OverrideMat         = FALSE;

            // get scrolling uv information
            GetUVOffset( Inst.UOffset, Inst.VOffset, pGeom, s_lRegisteredMaterials(hMat) );

            // fill in the alpha
            Inst.Alpha = Ambient.A;

            // fill in the skin geom instance info
            Inst.Data.Skin.pGeom     = pGeom;
            Inst.Data.Skin.pBones    = pBone;
            Inst.Data.Skin.Pad       = 0;

            // fill in the lighting
            Inst.pLighting = pLighting;

            // do texture projections
            if( (Inst.SortKey.RenderOrder < ORDER_ZPRIME) &&
                CanHaveProjTexture( Material ) )
            {
                Inst.Flags |= ProjFlags;
            }

            #ifdef TARGET_PC
            private_geom& PrivateGeom = s_lRegisteredGeoms(pGeom->m_hGeom);
            Inst.hDList               = PrivateGeom.SkinDList[(s32)SubMesh.iDList];
            #endif

            // handle fading geometry
            if ( Flags & render::FADING_ALPHA )
            {
                // we need to render twice to prime the z-buffer for alpha geometry
                SortKey.GeomSubMesh         = iSubMesh;
                SortKey.GeomHandle          = pGeom->m_hGeom;
                SortKey.GeomType            = 1;
                SortKey.MatIndex            = 0x3ff;
                SortKey.RenderOrder         = ORDER_ZPRIME;
                render_instance& ZPrimeInst = AddToHashHybrid( SortKey.Bits );
                ZPrimeInst.Flags            = Inst.Flags & render::CLIPPED;
                ZPrimeInst.pLighting        = pLighting;
                ZPrimeInst.Data             = Inst.Data;
                ZPrimeInst.UOffset          = Inst.UOffset;
                ZPrimeInst.VOffset          = Inst.VOffset;
                ZPrimeInst.Alpha            = 0x80;
                ZPrimeInst.OverrideMat      = 1;
                #ifdef TARGET_PC
                ZPrimeInst.hDList           = Inst.hDList;
                #endif
            }
        }

        #if ENABLE_RENDER_STATS
        s_RenderStats.m_nTrisRendered     += Mesh.nFaces;
        s_RenderStats.m_nVerticesRendered += Mesh.nVertices;
        #endif
    }

    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_InstanceAddTime += AddTime.Stop();
    #endif
}

//=============================================================================

void render::AddSkinInstanceDistorted( hgeom_inst        hInst,
                                       const matrix4*    pBone,
                                       u64               Mask,
                                       u32               Flags,
                                       const radian3&    NormalRot,
                                       xcolor            Ambient )
{
#ifndef X_RETAIL
    if( g_RenderDebug.RenderRigidOnly )
        return;
#endif

    CONTEXT( "render::AddSkinInstance" );

    #if ENABLE_RENDER_XTIMERS
    xtimer AddTime;
    AddTime.Start();
    #endif

    // safety check
    ASSERT( s_InRenderBegin );

    // stats update
    #if ENABLE_RENDER_STATS
    s_RenderStats.m_nInstancesRendered++;
    #endif

    // grab the useful pointers out
    private_instance& RegisteredInst = s_lRegisteredInst( hInst );
    ASSERT( RegisteredInst.Type == TYPE_SKIN );
    skin_geom* pGeom = (skin_geom*)RegisteredInst.pGeom;

    // calculate lighting
    // TODO: Ignore dynamic lights for "cloaked" objects
    void* pLighting = platform_CalculateSkinLighting( Flags, pBone[0], pGeom->m_BBox, Ambient );
//    void* pLighting = platform_CalculateDistortionLighting( pBone[0], pGeom->m_BBox, Ambient );
    Flags |= INSTFLAG_DYNAMICLIGHT;

    // generate a default distortion info struct for handling this guy
    s32 DefaultInfoIndex = s_lDistortionInfo.GetCount();
    distortion_info& DefaultInfo = s_lDistortionInfo.Append();
    DefaultInfo.MatIndex  = 0xffffffff;
    DefaultInfo.NormalRot = NormalRot;

    // add the meshes and submeshes to the render list
    for ( s32 iMesh = 0; iMesh < pGeom->m_nMeshes; iMesh++ )
    {
        // skip this mesh?
        if( (Mask & ((u64)1 << iMesh)) == 0 )
            continue;

        // add each of the submeshes to the render list
        geom::mesh& Mesh = pGeom->m_pMesh[iMesh];
        for ( s32 iSubMesh = Mesh.iSubMesh;
              iSubMesh < Mesh.iSubMesh+Mesh.nSubMeshes;
              iSubMesh++ )
        {
            geom::submesh&  SubMesh  = pGeom->m_pSubMesh[iSubMesh];
            geom::material& Material = pGeom->m_pMaterial[SubMesh.iMaterial];

            // get the material handle info
            xhandle hMat = pGeom->m_pVirtualMaterials[Material.iVirtualMat].MatHandle;

            // range safety check for the sort key
            ASSERT( (pGeom->m_hGeom>=0) && (pGeom->m_hGeom<kMaxRegisteredGeoms    ) );
            ASSERT( (hMat          >=0) && (hMat          <kMaxRegisteredMaterials) );
            ASSERT( (iSubMesh      >=0) && (iSubMesh      <256                    ) );

            // are we overriding a non-distortion material, or using a distortion material
            // already?!?
            if( (Material.Type == Material_Distortion) ||
                (Material.Type == Material_Distortion_PerPolyEnv) )
            {
                // NOTE: Since the default info is shared by the entire mesh, this
                // will break any cases where we've mixed distortion materials within
                // a single piece of geometry. This shouldn't ever happen, but if
                // it does, then we need a DefaultInfo per material.
                DefaultInfo.MatIndex = s_lRegisteredMaterials.GetIndexByHandle( hMat );
            }

            // build the sort key
            sortkey SortKey;
            SortKey.Bits        = 0;
            SortKey.GeomSubMesh = iSubMesh;
            SortKey.GeomHandle  = pGeom->m_hGeom;
            SortKey.GeomType    = 1;
            SortKey.MatIndex    = DefaultInfoIndex;
            SortKey.RenderOrder = ORDER_DISTORTION;

            // fill in the basic render instance info
            render_instance& Inst    = AddToHashHybrid( SortKey.Bits );
            Inst.SortKey             = SortKey;
            Inst.Flags               = Flags;
            Inst.OverrideMat         = TRUE;

            // get scrolling uv information
            GetUVOffset( Inst.UOffset, Inst.VOffset, pGeom, s_lRegisteredMaterials(hMat) );

            // fill in the alpha
            Inst.Alpha = Ambient.A;

            // fill in the skin geom instance info
            Inst.Data.Skin.pGeom     = pGeom;
            Inst.Data.Skin.pBones    = pBone;
            Inst.Data.Skin.Pad       = 0;

            // fill in the lighting
            Inst.pLighting = pLighting;

            #ifdef TARGET_PC
            private_geom& PrivateGeom = s_lRegisteredGeoms(pGeom->m_hGeom);
            Inst.hDList               = PrivateGeom.SkinDList[(s32)SubMesh.iDList];
            #endif
        }

        #if ENABLE_RENDER_STATS
        s_RenderStats.m_nTrisRendered     += Mesh.nFaces;
        s_RenderStats.m_nVerticesRendered += Mesh.nVertices;
        #endif
    }

    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_InstanceAddTime += AddTime.Stop();
    #endif
}

//=============================================================================

void render::BeginMidPostEffects( void )
{
    // DS: Do we really need separate functions for the "mid" post-effects?
    // Perhaps for the x-box...need to check with Bryon...
    platform_BeginPostEffects();
}

//=============================================================================

void render::EndMidPostEffects( void )
{
    // DS: Do we really need separate functions for the "mid" post-effects?
    // Perhaps for the x-box...need to check with Bryon...
    platform_EndPostEffects();
}

//=============================================================================

void render::BeginPostEffects( void )
{
    CONTEXT( "render::BeginPostEffects" );
    platform_BeginPostEffects();
}

//=============================================================================

void render::ApplySelfIllumGlows( f32 MotionBlurIntensity, s32 GlowCutoff )
{
    platform_ApplySelfIllumGlows( MotionBlurIntensity, GlowCutoff );
}

//=============================================================================

void render::ZFogFilter( render::post_falloff_fn Fn, xcolor Color, f32 Param1, f32 Param2 )
{
    platform_ZFogFilter( Fn, Color, Param1, Param2 );
}

//=============================================================================

void render::ZFogFilter( render::post_falloff_fn Fn, s32 PaletteIndex )
{
    platform_ZFogFilter( Fn, PaletteIndex );
}

//=============================================================================

void render::AddScreenWarp( const vector3& WorldPos, f32 Radius, f32 WarpAmount )
{
    platform_AddScreenWarp( WorldPos, Radius, WarpAmount );
}

//=============================================================================

void render::MotionBlur( f32 Intensity )
{
    platform_MotionBlur( Intensity );
}

//=============================================================================

void render::MipFilter( s32                     nFilters,
                        f32                     Offset,
                        render::post_falloff_fn Fn,
                        xcolor                  Color,
                        f32                     Param1,
                        f32                     Param2,
                        s32                     PaletteIndex )
{
    platform_MipFilter( nFilters, Offset, Fn, Color, Param1, Param2, PaletteIndex );
}

//=============================================================================

void render::MipFilter( s32                     nFilters,
                        f32                     Offset,
                        render::post_falloff_fn Fn,
                        const texture::handle&  Texture,
                        s32                     PaletteIndex )
{
    platform_MipFilter( nFilters, Offset, Fn, Texture, PaletteIndex );
}

//=============================================================================

void render::MultScreen( xcolor MultColor, post_screen_blend FinalBlend )
{
    platform_MultScreen( MultColor, FinalBlend );
}

//=============================================================================

void render::RadialBlur( f32 Zoom, radian Angle, f32 AlphaSub, f32 AlphaScale )
{
    platform_RadialBlur( Zoom, Angle, AlphaSub, AlphaScale );
}

//=============================================================================

void render::NoiseFilter( xcolor Color )
{
    platform_NoiseFilter( Color );
}

//=============================================================================

void render::ScreenFade( xcolor Color )
{
    platform_ScreenFade( Color );
}

//=============================================================================

void render::EndPostEffects( void )
{
    CONTEXT( "render::EndPostEffects" );
    platform_EndPostEffects();
}

//=============================================================================

#ifdef TARGET_PC
void* render::LockRigidDListVertex( render::hgeom_inst hInst, s32 iSubMesh )
{
    return platform_LockRigidDListVertex( hInst, iSubMesh );
}

//=============================================================================

void render::UnlockRigidDListVertex( render::hgeom_inst hInst, s32 iSubMesh )
{
    platform_UnlockRigidDListVertex( hInst, iSubMesh );
}

//=============================================================================

void* render::LockRigidDListIndex( render::hgeom_inst hInst, s32 iSubMesh,  s32& VertexOffset )
{
    return platform_LockRigidDListIndex( hInst, iSubMesh, VertexOffset );
}

//=============================================================================

void render::UnlockRigidDListIndex( render::hgeom_inst hInst, s32 iSubMesh )
{
    platform_UnlockRigidDListIndex( hInst, iSubMesh );
}
#endif

//=============================================================================

material& render::GetMaterial( hgeom_inst hInst, s32 iSubMesh )
{
    // grab the useful pointers out
    private_instance& RegisteredInst = s_lRegisteredInst( hInst );

    geom* pGeom = RegisteredInst.pGeom;
    ASSERT(pGeom) ;

    // get the internal registered material from the geometry material
    geom::submesh&  SubMesh  = pGeom->m_pSubMesh[iSubMesh];
    geom::material& Material = pGeom->m_pMaterial[SubMesh.iMaterial];
    xhandle         hMat     = pGeom->m_pVirtualMaterials[Material.iVirtualMat].MatHandle;
    ASSERT( (hMat>=0) && (hMat<kMaxRegisteredMaterials) );

    return s_lRegisteredMaterials(hMat);
}

//=============================================================================

texture* render::GetVTexture( const geom* pGeom,
                              s32         iMaterial,
                              s32         VTextureMask )
{
    // assume no offset to start with
    s32 VMatOffset = 0;

    // find any virtual textures that might affect this material, and if so
    // the material offset will come directly from the mask
    s32 i;
    for( i = 0; i < pGeom->m_nVirtualTextures; i++ )
    {
        // grab the associated bits for this vtexture from the texture mask
        s32 Offset = VTextureMask & 0xf;
        VTextureMask >>= 4;
        
        // does this virtual texture affect this material?
        const geom::virtual_texture& VTexture = pGeom->m_pVirtualTextures[i];
        if( VTexture.MaterialMask & (1<<iMaterial) )
        {
            VMatOffset = Offset;
            break;
        }
    }

    // now, using the offset, get the texture (requires a good bit of
    // redirection, but all comes out in the end)
    const geom::material&         GeomMat  = pGeom->m_pMaterial[iMaterial];
    const geom::virtual_material& GeomVMat = pGeom->m_pVirtualMaterials[GeomMat.iVirtualMat + VMatOffset];
    xhandle                       hMat     = GeomVMat.MatHandle;
    material&                     Mat      = s_lRegisteredMaterials(hMat);
    
    return Mat.m_DiffuseMap.GetPointer();
}

//=============================================================================

void render::SetAreaCubeMap( const cubemap::handle&  CubeMap )
{
    s_pCurrCubeMap = CubeMap.GetPointer();
}

//=============================================================================
// Filter lighting functions
//=============================================================================

void render::EnableFilterLight( xbool  bEnable )
{
    s_bFilterLight = bEnable ;
}

//=============================================================================

xbool render::IsFilterLightEnabled( void )
{
    return s_bFilterLight ;
}

//=============================================================================

void render::SetFilterLightColor( xcolor Color   )
{
    s_FilterLightColor = Color ;
}

//=============================================================================

xcolor render::GetFilterLightColor( void )
{
    return s_FilterLightColor ;
}

//=============================================================================
//=============================================================================

#if ENABLE_RENDER_STATS

render::stats::stats( void )
{
    ClearAllStats();
}

//=============================================================================

render::stats::~stats( void )
{
}

//=============================================================================

void render::stats::Begin( void )
{
    m_nMaterialsRendered   = 0;
    m_nInstancesRendered   = 0;
    m_nSubMeshesRendered   = 0;
    m_nVerticesRendered    = 0;
    m_nTrisRendered        = 0;
    m_InstanceSortTime     = 0;
    m_MaterialActivateTime = 0;
    m_InstanceAddTime      = 0;
    m_TotalEndRenderTime   = 0;
}

//=============================================================================

void render::stats::End( void )
{
    m_MaxMaterialsRendered    = MAX( m_MaxMaterialsRendered,    m_nMaterialsRendered   );
    m_MaxInstancesRendered    = MAX( m_MaxInstancesRendered,    m_nInstancesRendered   );
    m_MaxSubMeshesRendered    = MAX( m_MaxSubMeshesRendered,    m_nSubMeshesRendered   );
    m_MaxVerticesRendered     = MAX( m_MaxVerticesRendered,     m_nVerticesRendered    );
    m_MaxTrisRendered         = MAX( m_MaxTrisRendered,         m_nTrisRendered        );
    m_MaxInstanceSortTime     = MAX( m_MaxInstanceSortTime,     m_InstanceSortTime     );
    m_MaxMaterialActivateTime = MAX( m_MaxMaterialActivateTime, m_MaterialActivateTime );
    m_MaxInstanceAddTime      = MAX( m_MaxInstanceAddTime,      m_InstanceAddTime      );
    m_MaxTotalEndRenderTime   = MAX( m_MaxTotalEndRenderTime,   m_TotalEndRenderTime   );
}

//=============================================================================

void render::stats::Print( s32 Mode, s32 Flags )
{
    // this is kinda dangerous! make sure this array is big enough to hold all
    // of the stats and all of the text associated with each one!
    char Text[20][64];
    s32  Line = 0;

    #ifdef TARGET_PS2
    x_sprintf( Text[Line++], "Estimated FPS = %.1f\n", eng_GetFPS() );
    #endif

    x_sprintf( Text[Line++], "NMaterialsRendered = %d\n",     m_nMaterialsRendered                );
    x_sprintf( Text[Line++], "NInstancesRendered = %d\n",     m_nInstancesRendered                );
    x_sprintf( Text[Line++], "NSubMeshesRendered = %d\n",     m_nSubMeshesRendered                );
    x_sprintf( Text[Line++], "NVerticesRendered  = %d\n",     m_nVerticesRendered                 );
    x_sprintf( Text[Line++], "NTrisRendered      = %d\n",     m_nTrisRendered                     );
    x_sprintf( Text[Line++], "InstanceSortTime   = %.3fms\n", x_TicksToMs(m_InstanceSortTime)     );
    x_sprintf( Text[Line++], "MatActivateTime    = %.3fms\n", x_TicksToMs(m_MaterialActivateTime) );
    x_sprintf( Text[Line++], "InstanceAddTime    = %.3fms\n", x_TicksToMs(m_InstanceAddTime)      );
    x_sprintf( Text[Line++], "TotalEndRenderTime = %.3fms\n", x_TicksToMs(m_TotalEndRenderTime)   );

    if ( Flags & FLAG_VERBOSE )
    {
        x_sprintf( Text[Line++], "MaxMaterialsRendered  = %d\n",     m_MaxMaterialsRendered                 );
        x_sprintf( Text[Line++], "MaxInstancesRendered  = %d\n",     m_MaxInstancesRendered                 );
        x_sprintf( Text[Line++], "MaxSubMeshesRendered  = %d\n",     m_MaxSubMeshesRendered                 );
        x_sprintf( Text[Line++], "MaxVerticesRendered   = %d\n",     m_MaxVerticesRendered                  );
        x_sprintf( Text[Line++], "MaxTrisRendered       = %d\n",     m_MaxTrisRendered                      );
        x_sprintf( Text[Line++], "MaxInstanceSortTime   = %.3fms\n", x_TicksToMs(m_MaxInstanceSortTime)     );
        x_sprintf( Text[Line++], "MaxMatActivateTime    = %.3fms\n", x_TicksToMs(m_MaxMaterialActivateTime) );
        x_sprintf( Text[Line++], "MaxInstanceAddTime    = %.3fms\n", x_TicksToMs(m_MaxInstanceAddTime)      );
        x_sprintf( Text[Line++], "MaxTotalEndRenderTime = %.3fms\n", x_TicksToMs(m_MaxTotalEndRenderTime)   );
    }

    switch ( Mode )
    {
    case OUTPUT_TO_DEBUG:
        {
            x_DebugMsg( "======= Render Stats =======\n" );
            for ( s32 i = 0; i < Line; i++ )
            {
                x_DebugMsg( Text[Line] );
            }
        }
        break;

    case OUTPUT_TO_SCREEN:
        {
            for ( s32 i = 0; i < Line; i++ )
            {
                x_printfxy( 1, 3+i, Text[i] );
            }
        }
        break;

    case OUTPUT_TO_FILE:
        ASSERT( "Not implemented" );
        break;
    }
}

//=============================================================================

void render::stats::ClearAllStats( void )
{
    m_nMaterialsRendered      = 0;
    m_nInstancesRendered      = 0;
    m_nSubMeshesRendered      = 0;
    m_nVerticesRendered       = 0;
    m_nTrisRendered           = 0;
    m_InstanceSortTime        = 0;
    m_MaterialActivateTime    = 0;
    m_InstanceAddTime         = 0;
    m_TotalEndRenderTime      = 0;
    m_MaxMaterialsRendered    = 0;
    m_MaxInstancesRendered    = 0;
    m_MaxSubMeshesRendered    = 0;
    m_MaxVerticesRendered     = 0;
    m_MaxTrisRendered         = 0;
    m_MaxInstanceSortTime     = 0;
    m_MaxMaterialActivateTime = 0;
    m_MaxInstanceAddTime      = 0;
    m_MaxTotalEndRenderTime   = 0;
}

//=============================================================================

render::stats& render::GetStats( void )
{
    return s_RenderStats;
}

#endif //ENABLE_RENDER_STATS

//=============================================================================

void render::BeginShadowCreation( void )
{
    ASSERT( !s_InRenderBegin && !s_InShadowBegin && !s_InRawBegin );

    s_InShadowBegin = TRUE;

    // clear out the list of render instances
    s_LoHashMark = 0;
    s_HiHashMark = kMaxRenderedInstances - 1;
    x_memset( s_HashTable, 0xff, sizeof(s16)*kMaxRenderedInstances );

    // clear out any current projectors
    platform_ClearShadowProjectorList();
    s_nDynamicShadows = 0;
}

//=============================================================================

void render::EndShadowCreation( void )
{
    CONTEXT( "render::EndShadowCreation" );

    #if ENABLE_RENDER_XTIMERS
    xtimer TotalEndTime;
    TotalEndTime.Start();
    #endif

    // safety check
    ASSERT( eng_InBeginEnd() );
    ASSERT( s_InShadowBegin );
    s_InShadowBegin = FALSE;

    // update the stats
    #if ENABLE_RENDER_STATS
    s_RenderStats.m_nSubMeshesRendered += s_LoHashMark + (kMaxRenderedInstances-1-s_HiHashMark);
    #endif

    // let the platform-specific shaders get initialized (whether its d3d vert/pixel
    // shaders, vu0/vu1 microcode, or gamecube passes)
    platform_BeginShadowShaders();

    // sort the render instances (by material and sort key)
    #if ENABLE_RENDER_XTIMERS
    xtimer StatsTimer;
    StatsTimer.Start();
    #endif
    x_qsort( s_lSortData.GetPtr(), s_LoHashMark, sizeof(sort_struct), InstanceCompareFn );
    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_InstanceSortTime += StatsTimer.Stop();
    #endif

#ifdef X_DEBUG
    // sanity check
    SortSanityCheck();
#endif

    // we start by casting shadows
    platform_StartShadowCast();

    // loop through all of the render instances and render those bad boys
    geom*           pCurrentGeom = NULL;
    geom_type       CurrentType  = TYPE_UNKNOWN;
    shad_sortkey    CurrentSortData;
    CurrentSortData.Bits = 0xffffffff;
    s32       iUniqueInst;
    for ( iUniqueInst = 0; iUniqueInst < s_LoHashMark; iUniqueInst++ )
    {
        render_instance& Inst = s_lRenderInst[s_lSortData[iUniqueInst].iRenderInst];

        // break out of this loop if it's time to receive
        if ( Inst.ShadSortKey.ShadType )
        {
            break;
        }

        // TODO: Handle material swaps...shadows will eventually need to work
        // with punch-through

        // start a new instance batch if necessary (geometry sorting should already
        // be built into the sort key)
        if ( (Inst.ShadSortKey.GeomType == 0) &&
             ((pCurrentGeom != Inst.Data.Rigid.pGeom) || (CurrentSortData.GeomSubMesh != Inst.ShadSortKey.GeomSubMesh)) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndShadowCastRigid();
                else                                platform_EndShadowCastSkin();
            }

            pCurrentGeom                = Inst.Data.Rigid.pGeom;
            CurrentType                 = TYPE_RIGID;
            CurrentSortData.GeomSubMesh = Inst.ShadSortKey.GeomSubMesh;
            platform_BeginShadowCastRigid( pCurrentGeom, Inst.ShadSortKey.GeomSubMesh );
        }
        else
        if ( Inst.ShadSortKey.GeomType &&
             ((pCurrentGeom != Inst.Data.Skin.pGeom) || (CurrentSortData.GeomSubMesh != Inst.ShadSortKey.GeomSubMesh)) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndShadowCastRigid();
                else                                platform_EndShadowCastSkin();
            }

            pCurrentGeom                = Inst.Data.Skin.pGeom;
            CurrentType                 = TYPE_SKIN;
            CurrentSortData.GeomSubMesh = Inst.ShadSortKey.GeomSubMesh;
            platform_BeginShadowCastSkin( pCurrentGeom, Inst.ShadSortKey.GeomSubMesh );
        }

        // let the platform run its render code on the instances
        s32 iInstToRender = s_lSortData[iUniqueInst].iRenderInst;
        if ( Inst.ShadSortKey.GeomType == 0 )
        {
            while ( iInstToRender != -1 )
            {
                ASSERT( s_lRenderInst[iInstToRender].ShadSortKey.Bits == Inst.ShadSortKey.Bits );
                platform_RenderShadowCastRigid( s_lRenderInst[iInstToRender] );
                iInstToRender = s_lRenderInst[iInstToRender].Brother;
            };
        }
        else
        {
            while ( iInstToRender != -1 )
            {
                ASSERT( s_lRenderInst[iInstToRender].ShadSortKey.Bits == Inst.ShadSortKey.Bits );
                platform_RenderShadowCastSkin( s_lRenderInst[iInstToRender], Inst.ShadSortKey.ProjectorIndex );
                iInstToRender = s_lRenderInst[iInstToRender].Brother;
            };
        }
    }

    // finish any pending tasks
    if ( pCurrentGeom != NULL )
    {
        ASSERT( CurrentType != TYPE_UNKNOWN );
        if ( CurrentType == TYPE_RIGID )    platform_EndShadowCastRigid();
        else                                platform_EndShadowCastSkin();
    }

    // done casting, time to receive
    platform_EndShadowCast();

    platform_StartShadowReceive();

    pCurrentGeom         = NULL;
    CurrentType          = TYPE_UNKNOWN;
    CurrentSortData.Bits = 0xffffffff;
    for ( ; iUniqueInst < s_LoHashMark; iUniqueInst++ )
    {
        render_instance& Inst = s_lRenderInst[s_lSortData[iUniqueInst].iRenderInst];

        // sanity check
        ASSERT( Inst.ShadSortKey.ShadType );

        // TODO: Handle material swaps...shadows will eventually need to work
        // with punch-through

        // start a new instance batch if necessary (geometry sorting should already
        // be built into the sort key)
        if ( (Inst.ShadSortKey.GeomType == 0) &&
             ((pCurrentGeom != Inst.Data.Rigid.pGeom) || (CurrentSortData.GeomSubMesh != Inst.ShadSortKey.GeomSubMesh)) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndShadowReceiveRigid();
                else                                platform_EndShadowReceiveSkin();
            }

            pCurrentGeom                = Inst.Data.Rigid.pGeom;
            CurrentType                 = TYPE_RIGID;
            CurrentSortData.GeomSubMesh = Inst.ShadSortKey.GeomSubMesh;
            platform_BeginShadowReceiveRigid( pCurrentGeom, Inst.ShadSortKey.GeomSubMesh );
        }
        else
        if ( Inst.ShadSortKey.GeomType &&
             ((pCurrentGeom != Inst.Data.Skin.pGeom) || (CurrentSortData.GeomSubMesh != Inst.ShadSortKey.GeomSubMesh)) )
        {
            if ( pCurrentGeom != NULL )
            {
                ASSERT( CurrentType != TYPE_UNKNOWN );
                if ( CurrentType == TYPE_RIGID )    platform_EndShadowReceiveRigid();
                else                                platform_EndShadowReceiveSkin();
            }

            pCurrentGeom                = Inst.Data.Skin.pGeom;
            CurrentType                 = TYPE_SKIN;
            CurrentSortData.GeomSubMesh = Inst.ShadSortKey.GeomSubMesh;
            platform_BeginShadowReceiveSkin( pCurrentGeom, Inst.ShadSortKey.GeomSubMesh );
        }

        // let the platform run its render code on the instances
        s32 iInstToRender = s_lSortData[iUniqueInst].iRenderInst;
        if ( Inst.ShadSortKey.GeomType == 0 )
        {
            while ( iInstToRender != -1 )
            {
                ASSERT( s_lRenderInst[iInstToRender].ShadSortKey.Bits == Inst.ShadSortKey.Bits );
                platform_RenderShadowReceiveRigid( s_lRenderInst[iInstToRender], Inst.ShadSortKey.ProjectorIndex );
                iInstToRender = s_lRenderInst[iInstToRender].Brother;
            };
        }
        else
        {
            while ( iInstToRender != -1 )
            {
                ASSERT( s_lRenderInst[iInstToRender].ShadSortKey.Bits == Inst.ShadSortKey.Bits );
                platform_RenderShadowReceiveSkin( s_lRenderInst[iInstToRender] );
                iInstToRender = s_lRenderInst[iInstToRender].Brother;
            };
        }
    }

    // finish up any pending tasks
    if ( pCurrentGeom != NULL )
    {
        ASSERT( CurrentType != TYPE_UNKNOWN );
        if ( CurrentType == TYPE_RIGID )    platform_EndShadowReceiveRigid();
        else                                platform_EndShadowReceiveSkin();
    }

    // completely done
    platform_EndShadowReceive();
    platform_EndShadowShaders();

    // stats update
    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_TotalEndRenderTime += TotalEndTime.Stop();
    #endif
}

//=============================================================================

void render::AddPointShadowProjection( const matrix4&         L2W,
                                       radian                 FOV,
                                       f32                    NearZ,
                                       f32                    FarZ )
{
    ASSERT( s_InShadowBegin );
    platform_AddPointShadowProjection( L2W, FOV, NearZ, FarZ );
    s_nDynamicShadows++;

#if 0
    ASSERT( s_InShadowBegin );
    // TODO:
    //ASSERT( FALSE );
    (void)L2W;
    (void)FOV;
    (void)NearZ;
    (void)FarZ;
    (void)Texture;
#endif
}

//=============================================================================

void render::AddDirShadowProjection( const matrix4&         L2W,
                                     f32                    Width,
                                     f32                    Height,
                                     f32                    NearZ,
                                     f32                    FarZ )
{
    ASSERT( s_InShadowBegin );
    platform_AddDirShadowProjection( L2W, Width, Height, NearZ, FarZ );
    s_nDynamicShadows++;

#if 0
    shadow_projection& Proj = s_ShadowProjectors.Append();

    // build the matrix that will map world coordinates into our projected
    // texture
    matrix4 W2Proj = L2W;
    W2Proj.InvertRT();

    // Note that this is following the d3d standard for Z, which is [0,1] after
    // transform, but since we never clip, it shouldn't really matter for the ps2.
    matrix4 Proj2Clip;
    Proj2Clip.Identity();
    Proj2Clip(0,0) = 2.0f / Width;
    Proj2Clip(1,1) = 2.0f / Height;
    Proj2Clip(2,2) = 1.0f / (FarZ-NearZ);
    Proj2Clip(3,2) = NearZ / (FarZ-NearZ);

    // figure out the final matrices for creating the shadow texture
    Proj.W2TextureCast    = Proj2Clip * W2Proj;
    Proj.W2TextureReceive = Proj.W2TextureCast;

    #ifdef TARGET_PS2
    Proj.W2TextureCast.Scale(vector3(kShadowTexSize*0.5f,kShadowTexSize*0.5f,1.0f));
    Proj.W2TextureCast.Translate(vector3(kShadowTexSize*0.5f,kShadowTexSize*0.5f,0.0f));
    Proj.W2TextureCast.Translate(vector3(2048.0f-256.0f, 2048.0f-224.0f,0.0f));
    Proj.W2TextureReceive.Scale(vector3(0.5f,0.5f,1.0f));
    Proj.W2TextureReceive.Translate(vector3(0.5f,0.5f,0.0f));
    #endif

    #if defined(TARGET_XBOX) || defined(TARGET_PC)
    //#warning "the scales need to be set up correctly for these platforms"
    #endif

    Proj.Type = shadow_projection::SHAD_TYPE_DIRECTIONAL;
#endif
}

//=============================================================================

void render::AddRigidCasterSimple( render::hgeom_inst hInst,
                                   const matrix4*     pL2W,  // will be DMA ref'd to!
                                   u64                ProjMask )
{
    ASSERT( s_InShadowBegin );
    // TODO:
    ASSERT( FALSE );
    (void)hInst;
    (void)pL2W;
    (void)ProjMask;
}

//=============================================================================

void render::AddRigidCaster( render::hgeom_inst hInst,
                             const matrix4*     pL2W,
                             u64                Mask,
                             u64                ProjMask )
{
    ASSERT( s_InShadowBegin );
    // TODO:
    //ASSERT( FALSE );
    (void)hInst;
    (void)pL2W;
    (void)Mask;
    (void)ProjMask;
}

//=============================================================================

void render::AddSkinCaster( render::hgeom_inst hInst,
                            const matrix4*     pBone,
                            u64                Mask,
                            u64                ProjMask )
{
#ifndef X_RETAIL
    if( g_RenderDebug.RenderRigidOnly )
        return;
#endif

    CONTEXT( "render::AddSkinCaster" );

    #if ENABLE_RENDER_XTIMERS
    xtimer AddTime;
    AddTime.Start();
    #endif

    // safety check
    ASSERT( s_InShadowBegin );

    // stats update
    #if ENABLE_RENDER_STATS
    s_RenderStats.m_nInstancesRendered++;
    #endif

    // grab the useful pointers out
    private_instance& RegisteredInst = s_lRegisteredInst( hInst );
    ASSERT( RegisteredInst.Type == TYPE_SKIN );
    skin_geom* pGeom = (skin_geom*)RegisteredInst.pGeom;

    // for each of the projectors, add the meshes and submeshes to the render list
    for ( s32 iProj = 0; iProj < s_nDynamicShadows; iProj++ )
    {
        if ( (ProjMask & (1 << iProj)) == 0 )
            continue;
        
        for ( s32 iMesh = 0; iMesh < pGeom->m_nMeshes; iMesh++ )
        {
            // skip this mesh?
            if( (Mask & ((u64)1 << iMesh)) == 0 )
                continue;

            // add each of the submeshes to the render list
            geom::mesh& Mesh = pGeom->m_pMesh[iMesh];
            for ( s32 iSubMesh = Mesh.iSubMesh;
                  iSubMesh < Mesh.iSubMesh+Mesh.nSubMeshes;
                  iSubMesh++ )
            {
                // range safety check for the sort key
                ASSERT( (pGeom->m_hGeom>=0) && (pGeom->m_hGeom<kMaxRegisteredGeoms) );
                ASSERT( (iSubMesh      >=0) && (iSubMesh      <256                ) );

                // don't let alpha cast shadows
                geom::submesh&  SubMesh  = pGeom->m_pSubMesh[iSubMesh];
                geom::material& Material = pGeom->m_pMaterial[SubMesh.iMaterial];
                if ( IsAlphaMaterial( (material_type)Material.Type ) )
                    continue;

                // build the sort key
                shad_sortkey SortKey;
                SortKey.Bits           = 0;
                SortKey.ProjectorIndex = iProj;
                SortKey.GeomSubMesh    = iSubMesh;
                SortKey.GeomHandle     = pGeom->m_hGeom;
                SortKey.GeomType       = 1;
                SortKey.ShadType       = 0;

                // fill in the basic render instance info
                render_instance& Inst = AddToHashHybrid( SortKey.Bits );
                Inst.ShadSortKey      = SortKey;
                Inst.Flags            = 0;
                Inst.UOffset          = 0;
                Inst.VOffset          = 0;
                Inst.OverrideMat      = FALSE;

                // fill in the skin geom instance info
                Inst.Data.Skin.pGeom  = pGeom;
                Inst.Data.Skin.pBones = pBone;
                Inst.Data.Skin.Pad    = 0;

                #ifdef TARGET_PC
                private_geom&  PrivateGeom = s_lRegisteredGeoms(pGeom->m_hGeom);
                Inst.hDList                = PrivateGeom.SkinDList[(s32)SubMesh.iDList];
                #endif
            }

            #if ENABLE_RENDER_STATS
            s_RenderStats.m_nTrisRendered     += Mesh.nFaces;
            s_RenderStats.m_nVerticesRendered += Mesh.nVertices;
            #endif
        }
    }

    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_InstanceAddTime += AddTime.Stop();
    #endif
}

//=============================================================================

void render::AddRigidReceiverSimple( render::hgeom_inst hInst,
                                     const matrix4*     pL2W,  // will be DMA ref'd to!
                                     u32                Flags,
                                     u64                ProjMask )
{
#ifndef X_RETAIL
    if( g_RenderDebug.RenderSkinOnly )
        return;
#endif

#if ENABLE_RENDER_XTIMERS
    xtimer AddTime;
    AddTime.Start();
    #endif

    // safety check
    ASSERT( s_InShadowBegin );
    ASSERT( pL2W->IsValid() );

    // grab the useful pointers out
    private_instance& RegisteredInst = s_lRegisteredInst( hInst );
    ASSERT( RegisteredInst.Type == TYPE_RIGID );
    rigid_geom* pGeom = (rigid_geom*)RegisteredInst.pGeom;

    // for each shadow cast, add each of the submeshes to the render list
    for ( s32 iProj = 0; iProj < s_nDynamicShadows; iProj++ )
    {
        if ( (ProjMask & (1 << iProj)) == 0 )
            continue;

        for ( s32 iSubMesh = 0; iSubMesh < pGeom->m_nSubMeshes; iSubMesh ++ )
        {
            // don't let alpha receive shadows
            geom::submesh&  SubMesh  = pGeom->m_pSubMesh[iSubMesh];
            geom::material& Material = pGeom->m_pMaterial[SubMesh.iMaterial];
            if( IsAlphaMaterial( (material_type)Material.Type ) && !(Material.Flags&geom::material::FLAG_FORCE_ZFILL) )
                continue;

            // don't let punch-through receive shadows
            if( Material.Flags & geom::material::FLAG_IS_PUNCH_THRU )
                continue;

            // TODO: Later, these should use material info for scrolling uv's
            // and punch-through. For now, we can render solid polys.

            // make sure we're aligned for dma purposes
            ASSERT( ALIGN_16(pL2W) == (s32)pL2W );

            // build the sort key
            shad_sortkey SortKey;
            SortKey.Bits = 0;
            SortKey.ProjectorIndex = iProj;
            SortKey.GeomSubMesh    = iSubMesh;
            SortKey.GeomHandle     = pGeom->m_hGeom;
            SortKey.GeomType       = 0;
            SortKey.ShadType       = 1;

            // fill in the basic render instance info
            render_instance& Inst = AddToHashHybrid( SortKey.Bits );
            Inst.Flags            = Flags;
            Inst.UOffset          = 0;
            Inst.VOffset          = 0;
            Inst.OverrideMat      = FALSE;

            // fill in the rigid geom info
            Inst.Data.Rigid.pGeom = pGeom;
            Inst.Data.Rigid.pL2W  = pL2W;

            #ifdef TARGET_PC
            Inst.hDList = RegisteredInst.RigidDList[(s32)SubMesh.iDList];
            #endif
        }

        // stats update
        #if ENABLE_RENDER_STATS
        s_RenderStats.m_nInstancesRendered++;
        s_RenderStats.m_nTrisRendered     += pGeom->m_nFaces;
        s_RenderStats.m_nVerticesRendered += pGeom->m_nVertices;
        #endif
    }

    #if ENABLE_RENDER_XTIMERS
    s_RenderStats.m_InstanceAddTiem += AddTime.Stop();
    #endif
}

//=============================================================================

void render::AddRigidReceiver( render::hgeom_inst hInst,
                               const matrix4*     pL2W,
                               u64                Mask,
                               u32                Flags,
                               u64                ProjMask )
{
    ASSERT( s_InShadowBegin );
    // TODO:
    //ASSERT( FALSE );
    (void)hInst;
    (void)pL2W;
    (void)Flags;
    (void)Mask;
    (void)ProjMask;
}

//=============================================================================

void render::AddSkinReceiver( render::hgeom_inst hInst,
                              const matrix4*     pBone,
                              u64                Mask,
                              u32                Flags,
                              u64                ProjMask )
{
    ASSERT( s_InShadowBegin );
    // TODO:
    //ASSERT( FALSE );
    (void)hInst;
    (void)pBone;
    (void)Flags;
    (void)Mask;
    (void)ProjMask;
}

//=============================================================================

void render::BeginSession( u32 nPlayers )
{
    platform_BeginSession( nPlayers );
}

void render::EndSession( void )
{
    platform_EndSession( );
}

// EOF
