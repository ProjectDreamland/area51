
#ifndef GEOM_COMPILER_HPP
#define GEOM_COMPILER_HPP

#include "x_files.hpp"
#include "RawMesh2.hpp"
#include "texinfo.hpp"
#include "..\..\Render\Geom.hpp"
#include "..\..\Render\CollisionVolume.hpp"
#include "Auxiliary\MiscUtils\dictionary.hpp"

struct rawmesh2;
struct skin_geom;
struct rigid_geom;
class geom_rsc_desc;

class geom_compiler
{
public:
        
    enum comp_type
    {
        TYPE_NONE,
        TYPE_RIGID,
        TYPE_SKIN
    };

            geom_compiler       ( void );
            ~geom_compiler      ( void );

    void    SetUserInfo         ( const char* pUserName, const char* pComputerName );
    void    SetExportDate       ( s32 Month, s32 Day, s32 Year );
    void    ReportWarning       ( const char* pWarning );
    void    ReportError         ( const char* pError );
    void    ThrowError          ( const char* pError );

    void    AddPlatform         ( platform Platform, const char* pFileName );
    s32     GetPlatformIndex    ( platform Platform );
    void    Export              ( const char* pFileName, comp_type Type, const char* pTexturePath );
    void    AddFastCollision    ( const char* pFileName );
    void    SetPhysicsMatx      ( const char* pFileName );
    void    SetSettingsFile     ( const char* pFileName );

    void    CompileLowCollision ( rigid_geom&   RigidGeom, 
                                  rawmesh2&     LowMesh, 
                                  rawmesh2&     HighMesh,
                                  const char*   pFileName);
   
    void    CompileLowCollisionFromBBox ( rigid_geom& RigidGeom, rawmesh2&   HighMesh );

    void    CompileHighCollisionPC      ( rigid_geom& RigidGeom, collision_data::mat_info* pMatList, const char* pFileName );
    void    CompileHighCollisionPS2     ( rigid_geom& RigidGeom, collision_data::mat_info* pMatList, const char* pFileName );
    void    CompileHighCollisionXBOX    ( rigid_geom& RigidGeom, collision_data::mat_info* pMatList, const char* pFileName );

public:

    //
    // Fixed layout of "Map" buttons in 3DS Max material editor
    //

    enum
    {
        Max_Diffuse1,           // 3DS Max Defaults
        Max_Diffuse2,           // 3DS Max Defaults
        Max_Blend,              // 3DS Max Defaults 
        Max_LightMap,           // 3DS Max Defaults 
        Max_Opacity,            // 3DS Max Defaults 
        Max_Intensity,          // Environment Intensity Map
        Max_Environment,        // Environment Map
        Max_SelfIllumination,   // Per-Pixel Self-Illumination
        Max_DetailMap,          // Detail Map
        Max_PunchThrough,       // Punch-Through Map
        NumMaps
    };

    struct high_tri
    {
        high_tri*                   pNext;
        s32                         I;
        vector3                     P[3];
        s32                         iBone;
        s32                         iMesh;
        s32                         iDList;
        collision_data::mat_info    MatInfo;
        xbool                       bFlipOrient;
        plane                       Plane;
        bbox                        BBox;
    };

protected:

    struct info
    {
        char                FileName[256];
        f32                 MinDistance;
        xbool               BuildCollision;
    };

    struct plat_info
    {
        platform            Platform;
        char                FileName[256];
    };

    struct dlist
    {
        s32                 iMaterial;
        s32                 iBone;
        xarray<s32>         lTri;
    };

    struct sub_mesh
    {
        char                        Name[32];
        const rawmesh2*             pRawMesh;
        const rawmesh2::sub_mesh*   pRawSubMesh;
        xharray<dlist>              lDList;
    };

    struct map_info
    {
        xbitmap Bitmap;
        xstring InputBitmapName;
        xstring OutputBitmapName;
        xstring CompiledBitmapName;
        xbool   bOutOfDate;
    };

    struct map_slot
    {
        xarray<map_info> MapList;
    };

    struct material
    {
        s32                 iRawMaterial;
        const rawmesh2*     pRawMesh;
        tex_info            TexInfo;
        map_slot            Maps[NumMaps];
    };

    struct mesh
    {
        xarray<material>    Material;
        xharray<sub_mesh>   SubMesh;
    };

protected:
    
    void    LoadResource        ( const char* pFileName, xbool bSkin );
    
    void    BuildBone           ( geom::bone& Bone, const rawmesh2::bone& RawBone );

    void    BuildBones          (       geom&     Geom, 
                                  const rawmesh2& GeomRawMesh,
                                  const rawmesh2& PhysicsRawMesh );
    
    void    BuildRigidBody      ( geom::rigid_body& RigidBody, const rawmesh2::rigid_body& RawRigidBody );
    
    void    BuildRigidBodies    (       geom&     Geom, 
                                  const rawmesh2& GeomRawMesh,
                                  const rawmesh2& PhysicsRawMesh );
    
    
    void    BuildSettings       ( geom& Geom, const char* pSettingsFile, const rawmesh2& GeomRawMesh );

    xbool   IsSameMaterial      ( const rawmesh2::material& RawMatA,
                                  const rawmesh2::material& RawMatB,
                                  const rawmesh2&           RawMesh );
    void    BuildBasicStruct    ( geom& Geom, const rawmesh2& RawMesh, mesh& Mesh, xbool IsRigid );

    void    RemoveUnusedVMeshes ( rawmesh2& RawMesh );

    void    ExportRigidGeom     ( const char* pFileName );
    void    ExportRigidGeomXbox ( mesh& Mesh, rigid_geom& RigidGeom, const char* pFileName );
    void    ExportRigidGeomPS2  ( mesh& Mesh, rigid_geom& RigidGeom, const char* pFileName );
    void    ExportRigidGeomPC   ( mesh& Mesh, rigid_geom& RigidGeom, const char* pFileName );

    void    ExportSkinGeom      ( const char* pFileName );
    void    ExportSkinGeomXbox  ( mesh& Mesh, skin_geom&  SkinGeom,  const char* pFileName );
    void    ExportSkinGeomPS2   ( mesh& Mesh, skin_geom&  SkinGeom,  const char* pFileName );
    void    ExportSkinGeomPC    ( mesh& Mesh, skin_geom&  SkinGeom,  const char* pFileName );

    void    RecenterUVs         ( s16*                      pUVs,
                                  s32                       nVerts );
    void    ExportUVAnimation   ( const rawmesh2::material& RawMat,
                                  const f32*                pParamKey,
                                  geom::material&           Material,
                                  geom&                     Geom );
    void    ReadBitmap          ( map_info&                 MapInfo );
    void    ComputeMapNames     ( map_slot&                 Map,
                                  s32                       MapType,
                                  u32                       CheckSum,
                                  pref_bpp                  PreferredBPP,
                                  const char*               pInputFile,
                                  platform                  PlatformId );
    void    CheckTimeStamps     ( map_slot&                 Map,
                                  const char*               pMixMap );
    void    FillMapSlots        ( mesh&                     Mesh,
                                  platform                  PlatformId );
    void    ExportMaterial      ( mesh&                     Mesh,
                                  geom&                     Geom,
                                  s32                       PlatformID );
    void    ExportVirtualMeshes ( mesh&                     Mesh,
                                  geom&                     Geom,
                                  s32                       PlatformID );
    void    BuildTexturePath    ( char* pPath, const char* pName, s32 PlatformID );
    void    ExportDiffuse       ( const xbitmap& Bitmap, const char* pName, pref_bpp BPP, s32 nMips, s32 PlatformID );
    void    ExportEnvironment   ( const xbitmap& Bitmap, const char* pName, s32 PlatformID );
    void    ExportDetail        ( const xbitmap& Bitmap, const char* pName, s32 PlatformID );

    void    ProcessPunchThruMap( map_info& DiffuseMap, const char* pPunchThruMap );

    void    CompileHighCollision        ( rigid_geom& RigidGeom, high_tri* pTri, s32 nTris, const char* pFileName, platform PlatformID );

    void    CompileDictionary   ( geom& Geom );

    void    PrintSummary        ( geom& Geom );

protected:

    xarray<info>        m_InfoList;
    xarray<plat_info>   m_PlatInfo;
    char                m_TexturePath[X_MAX_PATH] ;
    char                m_FastCollision[X_MAX_PATH];
    char                m_PhysicsMatx[X_MAX_PATH];
    char                m_SettingsFile[X_MAX_PATH];
    geom_rsc_desc*      m_pGeomRscDesc;
    dictionary          m_Dictionary;
    char                m_UserName[256];
    char                m_ComputerName[256];
    s32                 m_ExportDate[3];
};

#endif
