//==============================================================================
//  GeomDesc.hpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the base class for all geometry descriptions. It can handle
//  setting up LODs and UV animation data.
//==============================================================================

#ifndef __GEOMDESC_HPP__
#define __GEOMDESC_HPP__

//==============================================================================
// INCLUDES
//==============================================================================

#include "../EDRscDesc\RSCDesc.hpp"

//==============================================================================
// CLASS DEFINITION
//==============================================================================

class geom_rsc_desc : public rsc_desc
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // PUBLIC STRUCTURES
    ////////////////////////////////////////////////////////////////////////////

    enum { MAX_NAME_LENGTH = 256 };

    // uv animation structures
    struct uv_animation
    {
        uv_animation( void );

        char                MatName[MAX_NAME_LENGTH];
        s32                 AnimType;
        s32                 StartFrame;
        s32                 FPS;
        s32                 nFrames;
    };

    // virtual mesh structures
    struct lod_info
    {
        enum { MAX_LOD_MESHES = 16 };

        lod_info( void );

        s32         ScreenSize;
        s32         nMeshes;
        char        MeshName[MAX_LOD_MESHES][MAX_NAME_LENGTH];
    };

    struct virtual_mesh
    {
        virtual_mesh( void );

        char                Name[MAX_NAME_LENGTH];
        xarray<lod_info>    LODs;
    };

    // virtual texture structures
    struct texture_info
    {
        texture_info( void );

        char    FileName[MAX_NAME_LENGTH];
        char    Name[MAX_NAME_LENGTH];
    };

    struct virtual_texture
    {
        virtual_texture( void );

        void Reset( void );

        char                    Name[MAX_NAME_LENGTH];
        xarray<texture_info>    Textures;
        xbool                   OverrideDefault;
        char                    OverrideFileName[MAX_NAME_LENGTH];
    };

    ////////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // Normal resource description functions
                        geom_rsc_desc               ( rsc_desc_type& Desc    );
    virtual void        OnStartEdit                 ( void                   );
    virtual void        OnCheckIntegrity            ( void                   );
    virtual void        OnEnumProp                  ( prop_enum& List        );
    virtual xbool       OnProperty                  ( prop_query& I          );
    virtual void        OnGetCompilerDependencies   ( xarray<xstring>& List  );
    virtual void        OnGetCompilerRules          ( xstring& CompilerRules ) = 0;
    virtual void        OnGetFinalDependencies      ( xarray<xstring>& List, platform Platform, const char* pDirectory );

    // Specific geom description functions
            void        RefreshMatxFileInfo         ( void );
            void        SetMatxFileName             ( const char* pFileName );
    const   char*       GetMatxFileName             ( void ) const;
    static void         SetColoredMips              ( xbool OnOff );

    // query routines
    const uv_animation*     GetUVAnimation              ( const char* MatName ) const;
    s32                     GetVirtualMeshCount         ( void                ) const;
    const virtual_mesh&     GetVirtualMesh              ( s32 Index           ) const;
    s32                     GetVirtualTextureCount      ( void                ) const;
    const virtual_texture&  GetVirtualTexture           ( s32 Index           ) const;

    // functions for building vmeshes manually
    virtual_mesh&           AppendVirtualMesh           ( void );
    
protected:
    ////////////////////////////////////////////////////////////////////////////
    // PROTECTED FUNCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // Internal helpers for dealing with the enumeration
            void        OnEnumUVAnim                ( prop_enum& List, s32 AnimId );
            void        OnEnumVirtualMesh           ( prop_enum& List, s32 VMeshId );
            void        OnEnumLODs                  ( prop_enum& List, s32 VMeshId, s32 LodId );
            void        OnEnumMesh                  ( prop_enum& List, s32 VMeshId, s32 LodId, s32 MeshId );
            void        OnEnumVirtualTexture        ( prop_enum& List, s32 VTexId );
            void        OnEnumDiffTexture           ( prop_enum& List, s32 VTexId, s32 DiffId );

    // Internal helpers for dealing with the properties
            xbool       OnPropertyUVAnim            ( prop_query& I );
            xbool       OnPropertyVirtualMesh       ( prop_query& I );
            xbool       OnPropertyLODs              ( prop_query& I, s32 VMeshId );
            xbool       OnPropertyMesh              ( prop_query& I, s32 VMeshId, s32 LodId );
            xbool       OnPropertyVirtualTexture    ( prop_query& I );
            xbool       OnPropertyDiffTexture       ( prop_query& I, s32 VTexId );

    // other internal helpers
            void        SortVMeshes                 ( void );

    ////////////////////////////////////////////////////////////////////////////
    // INTERNAL STRUCTURES
    ////////////////////////////////////////////////////////////////////////////

    struct matx_mesh_info
    {
        char    Name[MAX_NAME_LENGTH];
    };

    struct matx_tex_info
    {
        char    Name[MAX_NAME_LENGTH];
    };

    ////////////////////////////////////////////////////////////////////////////
    // DATA
    ////////////////////////////////////////////////////////////////////////////

    char                    m_FileName[MAX_NAME_LENGTH];            // the source matx file
    char                    m_PhysicsMatx[MAX_NAME_LENGTH];         // Physics matx file
    char                    m_SettingsFile[MAX_NAME_LENGTH];    // Settings file name
    xarray<uv_animation>    m_UVAnimList;                           // the list of uv animations in the matx file
    xarray<virtual_mesh>    m_VirtualMeshes;                        // the list of virtual meshes
    xarray<virtual_texture> m_VirtualTextures;                      // the list of virtual textures

    char                    m_SelectedMesh[MAX_NAME_LENGTH];        // the selected mesh for property editing
    xarray<matx_mesh_info>  m_MatxMeshList;                         // this list of meshes imported from the matx file
    xarray<matx_tex_info>   m_MatxDiffuseList;                      // the list of diffuse textures used in the matx file
    
    static xbool            s_ColoredMips;                          // colored mips for debugging
};

//==============================================================================
// INLINES
//==============================================================================

inline
void geom_rsc_desc::SetMatxFileName( const char* pFileName )
{
    x_strsavecpy( m_FileName, pFileName, MAX_NAME_LENGTH );
}

//==============================================================================

inline
const char* geom_rsc_desc::GetMatxFileName( void ) const
{
    return m_FileName;
}

//==============================================================================

inline
const geom_rsc_desc::uv_animation* geom_rsc_desc::GetUVAnimation( const char* MatName ) const
{
    s32 i;
    for( i = 0; i < m_UVAnimList.GetCount(); i++ )
    {
        if( !x_strcmp( MatName, m_UVAnimList[i].MatName ) )
        {
            return &m_UVAnimList[i];
        }
    }
    return NULL;
}

//==============================================================================

inline
s32 geom_rsc_desc::GetVirtualMeshCount( void ) const
{
    return m_VirtualMeshes.GetCount();
}

//==============================================================================

inline
const geom_rsc_desc::virtual_mesh& geom_rsc_desc::GetVirtualMesh( s32 Index ) const
{
    ASSERT( (Index >= 0) && (Index < GetVirtualMeshCount()) );
    return m_VirtualMeshes[Index];
}

//==============================================================================

inline
s32 geom_rsc_desc::GetVirtualTextureCount( void ) const
{
    return m_VirtualTextures.GetCount();
}

//==============================================================================

inline
const geom_rsc_desc::virtual_texture& geom_rsc_desc::GetVirtualTexture( s32 Index ) const
{
    ASSERT( (Index >= 0) && (Index < GetVirtualTextureCount()) );
    return m_VirtualTextures[Index];
}

//==============================================================================

inline
geom_rsc_desc::virtual_mesh& geom_rsc_desc::AppendVirtualMesh( void )
{
    return m_VirtualMeshes.Append();
}

//==============================================================================

#endif // __GEOMDESC_HPP__
