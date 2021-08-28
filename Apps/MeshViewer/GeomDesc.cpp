//==============================================================================
//  GeomDesc.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//==============================================================================

#include "x_files.hpp"

#if defined(X_EDITOR)
#include "StdAfx.h"
#endif

#include "SkinDesc.hpp"
#include "MeshUtil\RawMesh2.hpp"
#include "..\Support\Render\geom.hpp"
#include "..\Editor\Project.hpp"

//=========================================================================
// STATICS
//=========================================================================

xbool geom_rsc_desc::s_ColoredMips = FALSE;

//=========================================================================
// UV ANIMATION
//=========================================================================

geom_rsc_desc::uv_animation::uv_animation( void ) :
    AnimType    ( 1 ),
    StartFrame  ( 0 ),
    FPS         ( 30 ),
    nFrames     ( 0 )
{
    MatName[0] = '\0';
}

//=========================================================================
// LOD INFO
//=========================================================================

geom_rsc_desc::lod_info::lod_info( void ) :
    ScreenSize      ( 10000 ),
    nMeshes         ( 0 )
{
    x_memset( MeshName, 0, sizeof(MeshName) );
}

//=========================================================================
// VIRTUAL MESH
//=========================================================================

geom_rsc_desc::virtual_mesh::virtual_mesh( void )
{
    LODs.Clear();
    x_memset( Name, 0, sizeof(Name) );
}

//=========================================================================
// TEXTURE INFO
//=========================================================================

geom_rsc_desc::texture_info::texture_info( void )
{
    FileName[0] = 0;
    Name[0]     = 0;
}

//=========================================================================
// VIRTUAL TEXTURE
//=========================================================================

geom_rsc_desc::virtual_texture::virtual_texture( void )
{
    Reset();
}

//=========================================================================

void geom_rsc_desc::virtual_texture::Reset( void )
{
    Name[0] = 0;
    Textures.Clear();
    OverrideDefault = FALSE;
    OverrideFileName[0] = 0;

    // now fill in one texture (this will be the default one that gets
    // swapped out)
    texture_info& TexInfo = Textures.Append();
    TexInfo.FileName[0]   = 0;
    TexInfo.Name[0]       = 0;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

geom_rsc_desc::geom_rsc_desc( rsc_desc_type& Desc ) : rsc_desc( Desc )
{
    x_memset( m_FileName, 0, sizeof( m_FileName ) );
    x_memset( m_PhysicsMatx, 0, sizeof( m_PhysicsMatx ) );
    x_memset( m_SettingsFile, 0 , sizeof( m_SettingsFile ) );
    x_memset( m_SelectedMesh, 0, sizeof(m_SelectedMesh) );
    m_MatxMeshList.Clear();
    m_MatxDiffuseList.Clear();
    m_UVAnimList.Clear();
}

//=========================================================================

void geom_rsc_desc::OnEnumProp( prop_enum& List )
{
    s32 i;

    // basic file info
    rsc_desc::OnEnumProp( List );
    List.PropEnumFileName( "ResDesc\\FileName",
                           "MATX Files (*.matx)|*.matx||",
                           "Select an Exported 3DMax File", 0 );

    // Physics file
    List.PropEnumFileName( "ResDesc\\PhysicsMatx",
                           "MATX Files (*.matx)|*.matx||",
                           "Select an Exported 3DMax File that contains physics info", 0 );

    // Bone masks file
    List.PropEnumFileName( "ResDesc\\SettingsFile",
                           "Settings file (*.txt)|*.txt||",
                           "Select a .txt file that contains settings such as bone mask definitions to be used by loco etc.", 0 );

    // a method for refreshging mesh names, uv animations, and diffuse texture names
    List.PropEnumButton( "ResDesc\\ForceMeshInfoUpdate",
                         "Re-load the matx file to force an update of mesh list and UV/Texture animations",
                         PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    // uv animation data
    List.PropEnumHeader( "ResDesc\\UVAnims",
                         "The UV animation properties for this mesh", 0 );
    List.PropEnumInt   ( "ResDesc\\UVAnims\\NumUVAnims",
                         "The total number of UV anims",
                         PROP_TYPE_DONT_SHOW | PROP_TYPE_MUST_ENUM );
    for( i = 0; i < m_UVAnimList.GetCount(); i++ )
    {
        OnEnumUVAnim( List, i );
    }

    // virtual mesh data
    List.PropEnumHeader( "ResDesc\\VirtualMeshes",
                         "Virtual meshes used for toggling geometry on and off and setting LODs", 0 );
    List.PropEnumInt   ( "ResDesc\\VirtualMeshes\\NumVirtualMeshes",
                         "The total number of virtual meshes",
                         PROP_TYPE_DONT_SHOW | PROP_TYPE_MUST_ENUM );
    List.PropEnumButton( "ResDesc\\VirtualMeshes\\SortVirtualMeshes",
                         "Sort the virtual meshes by alphabetical order",
                         PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    List.PropEnumButton( "ResDesc\\VirtualMeshes\\AddVirtualMesh",
                         "Add a new virtual mesh used for toggling geometry on and off and setting LODs",
                         PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    for( i = 0; i < m_VirtualMeshes.GetCount(); i++ )
    {
        OnEnumVirtualMesh( List, i );
    }

    // virtual texture data
    List.PropEnumHeader( "ResDesc\\VirtualTextures",
                         "Named texture sets used for setting up skins", 0 );
    List.PropEnumInt   ( "ResDesc\\VirtualTextures\\NumVirtualTextures",
                         "The total number of virtual textures",
                         PROP_TYPE_DONT_SHOW | PROP_TYPE_MUST_ENUM );
    List.PropEnumButton( "ResDesc\\VirtualTextures\\AddVirtualTexture",
                         "Add a new virtual texture used for setting up skins",
                         PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    for( i = 0; i < m_VirtualTextures.GetCount(); i++ )
    {
        OnEnumVirtualTexture( List, i );
    }
}

//=========================================================================

void geom_rsc_desc::OnEnumUVAnim( prop_enum& List, s32 AnimId )
{
    ASSERT( (AnimId >= 0) && (AnimId < m_UVAnimList.GetCount()) );

    // add the header
    List.PropEnumString( xfs( "ResDesc\\UVAnims\\UVAnim[%d]", AnimId ),
                            "Settings for a UV animation",
                            PROP_TYPE_HEADER );

    // push the path for the properties yet to come
    s32 HeaderId = List.PushPath( xfs( "ResDesc\\UVAnims\\UVAnim[%d]\\", AnimId ) );

    // enumerate the uv anim properties
    List.PropEnumString( "MaterialName",
                         "The name of the material with an animated uv", 0 );
    List.PropEnumEnum  ( "AnimType",
                         "Fixed\0Looped\0PingPong\0OneShot\0",
                         "The type of animation to apply to this geometry", 0 );
    List.PropEnumInt   ( "Start",
                         "The starting frame of this animation", 0 );
    List.PropEnumInt   ( "FPS",
                         "The frames per second this animation will run at", 0 );
    List.PropEnumInt   ( "NumFrames",
                         "The number of frames in this animation",
                         PROP_TYPE_READ_ONLY );

    // pop the path
    List.PopPath( HeaderId );
}

//=========================================================================

void geom_rsc_desc::OnEnumVirtualMesh( prop_enum& List, s32 VMeshId )
{
    ASSERT( (VMeshId >= 0) && (VMeshId < m_VirtualMeshes.GetCount()) );

    // add the header
    List.PropEnumString( xfs( "ResDesc\\VirtualMeshes\\VirtualMesh[%d]", VMeshId ),
                            "A named mesh that can be exposed to other systems",
                            PROP_TYPE_HEADER );

    // push the path for the properties yet to come
    s32 HeaderId = List.PushPath( xfs( "ResDesc\\VirtualMeshes\\VirtualMesh[%d]\\", VMeshId ) );
    
    // enumerate the mesh properties
    List.PropEnumButton( "RemoveVirtualMesh",
                            "Removes the virtual mesh from this list",
                            PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    List.PropEnumString( "Name",
                            "The name of this virtual mesh that is exposed to other systems", 0 );
    List.PropEnumInt   ( "NumLODs",
                            "The total number of LODs in this virtual mesh",
                            PROP_TYPE_DONT_SHOW | PROP_TYPE_MUST_ENUM );
    List.PropEnumButton( "AddLOD",
                            "Adds an LOD to this virtual mesh (by default, all meshes are visible)",
                            PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    // add the LODs
    s32 i;
    for( i = 0; i < m_VirtualMeshes[VMeshId].LODs.GetCount(); i++ )
    {
        OnEnumLODs( List, VMeshId, i );
    }

    // pop the path
    List.PopPath( HeaderId );
}

//=========================================================================

void geom_rsc_desc::OnEnumLODs( prop_enum& List, s32 VMeshId, s32 LodId )
{
    ASSERT( (VMeshId >= 0) && (VMeshId < m_VirtualMeshes.GetCount()) );
    ASSERT( (LodId >= 0) && (LodId < m_VirtualMeshes[VMeshId].LODs.GetCount()) );

    // add the header
    List.PropEnumString( xfs( "LOD[%d]", LodId ),
                            "A group of visible meshes based on screen size",
                            PROP_TYPE_HEADER );

    // push the path for the properties yet to come
    s32 HeaderId = List.PushPath( xfs( "LOD[%d]\\", LodId ) );

    // build a string list of meshes
    static xstring MeshEnumList;
    MeshEnumList.Clear();
    
    s32 i;
    if( m_MatxMeshList.GetCount() )
    {
        for( i = 0; i < m_MatxMeshList.GetCount(); i++ )
        {
            MeshEnumList += m_MatxMeshList[i].Name;
            MeshEnumList += "~";
        }
    }
    else
    {
        MeshEnumList += "~~";
    }
    
    for( i = 0; MeshEnumList[i]; i++ )
    {
        if( MeshEnumList[i] == '~' )
            MeshEnumList[i] = 0;
    }

    // enumerate the lod properties
    List.PropEnumButton( "RemoveLOD",
                            "Remove this LOD",
                            PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    List.PropEnumInt   ( "NumMeshs",
                            "Number of meshes",
                            PROP_TYPE_DONT_SHOW | PROP_TYPE_MUST_ENUM );
    List.PropEnumInt   ( "ScreenSize",
                            "The screen percentage where this LOD kicks in", 0 );
    List.PropEnumEnum  ( "MeshList",
                            MeshEnumList,
                            "The mesh to add as visible",
                            PROP_TYPE_DONT_SAVE );
    List.PropEnumButton( "AddMesh",
                            "Add a visible mesh to this LOD",
                            PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    // add the lod meshes
    for( i = 0; i < m_VirtualMeshes[VMeshId].LODs[LodId].nMeshes; i++ )
    {
        OnEnumMesh( List, VMeshId, LodId, i );
    }

    // pop the path
    List.PopPath( HeaderId );
}

//=========================================================================

void geom_rsc_desc::OnEnumMesh( prop_enum& List, s32 VMeshId, s32 LodId, s32 MeshId )
{
    ASSERT( (VMeshId >= 0) && (VMeshId < m_VirtualMeshes.GetCount()) );
    ASSERT( (LodId >= 0) && (LodId < m_VirtualMeshes[VMeshId].LODs.GetCount()) );
    ASSERT( (MeshId >= 0) && (MeshId < m_VirtualMeshes[VMeshId].LODs[LodId].nMeshes ) );

    // add the header
    List.PropEnumString( xfs( "Mesh[%d]", MeshId ),
                         "A visible mesh inside an LOD",
                         PROP_TYPE_HEADER );

    // push the path for the properties yet to come
    s32 HeaderId = List.PushPath( xfs( "Mesh[%d]\\", MeshId ) );

    // enumerate the properties
    List.PropEnumString( "Name",
                         "Name of the visible mesh", 0 );
    List.PropEnumButton( "RemoveMesh",
                         "Remove this visible mesh",
                         PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    // pop the path
    List.PopPath( HeaderId );
}

//=========================================================================

void geom_rsc_desc::OnEnumVirtualTexture( prop_enum& List, s32 VTexId )
{
    ASSERT( (VTexId >= 0) && (VTexId < m_VirtualTextures.GetCount()) );

    // add the header
    List.PropEnumString( xfs( "ResDesc\\VirtualTextures\\VirtualTexture[%d]", VTexId ),
                         "A named virtual texture that can be exposed to other systems",
                         PROP_TYPE_HEADER );

    // push the path for the properties yet to come
    s32 HeaderId = List.PushPath( xfs( "ResDesc\\VirtualTextures\\VirtualTexture[%d]\\", VTexId ) );

    // enumerate the virtual texture properties
    List.PropEnumButton  ( "RemoveVirtualTexture",
                           "Remove this virtual texture from the list",
                           PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    List.PropEnumString  ( "Name",
                           "Name of this virtual texture that will be exposed to other systems", 0 );
    List.PropEnumButton  ( "AddDiffuseTexture",
                           "Add a diffuse texture that will be selectable for other systems",
                           PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    List.PropEnumInt     ( "NumDiffTextures",
                           "Number of selectable diffuse textures for this virtual texture",
                           PROP_TYPE_DONT_SHOW | PROP_TYPE_MUST_ENUM );
    List.PropEnumBool    ( "OverrideDefault",
                           "Override the default texture (for saving memory when the max version is not used)", 0 );
    List.PropEnumFileName( "OverrideFileName",
                           "TGA Files (*.tga)|*.tga||",
                           "File to replace the default with", 0 );
    s32 i;
    for( i = 0; i < m_VirtualTextures[VTexId].Textures.GetCount(); i++ )
    {
        OnEnumDiffTexture( List, VTexId, i );
    }

    // pop the path
    List.PopPath( HeaderId );
}

//=========================================================================

void geom_rsc_desc::OnEnumDiffTexture( prop_enum& List, s32 VTexId, s32 DiffId )
{
    ASSERT( (VTexId >= 0) && (VTexId < m_VirtualTextures.GetCount()) );
    ASSERT( (DiffId >= 0) && (DiffId < m_VirtualTextures[VTexId].Textures.GetCount()) );

    // add the header
    List.PropEnumString( xfs( "DiffTexture[%d]", DiffId ),
                         "A switchable texture (0 is the texture to switch, 1..n are the switch options",
                         PROP_TYPE_HEADER );
    
    // push the path for the properties yet to come
    s32 HeaderId = List.PushPath( xfs( "DiffTexture[%d]\\", DiffId ) );

    // enumerate the properties

    // add the actual file to be swapped
    if( DiffId == 0 )
    {
        // build a string list of meshes
        static xstring TexEnumList;
        TexEnumList.Clear();

        s32 i;
        if( m_MatxDiffuseList.GetCount() )
        {
            for( i = 0; i < m_MatxDiffuseList.GetCount(); i++ )
            {
                TexEnumList += m_MatxDiffuseList[i].Name;
                TexEnumList += "~";
            }
        }
        else
        {
            TexEnumList += "~~";
        }

        for( i = 0; TexEnumList[i]; i++ )
        {
            if( TexEnumList[i] == '~' )
                TexEnumList[i] = 0;
        }

        // and make an enumerated type
        List.PropEnumEnum( "FileName",
                           TexEnumList,
                           "The file to be swapped", 0 );
    }
    else
    {
        // make a filename type
        List.PropEnumFileName( "FileName",
                               "TGA Files (*.tga)|*.tga||",
                               "The file to be swapped", 0 );
    }
    
    // add a more descriptive name to be swapped
    List.PropEnumString( "Name",
                         "A descriptive name for this texture that will be exposed to other systems", 0 );

    // and if it's not the first texture, it can be removed
    if( DiffId > 0 )
    {
        List.PropEnumButton( "RemoveDiffTexture",
                             "Remove this texture as a possible swap",
                             PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    }

    // pop the path
    List.PopPath( HeaderId );
}

//=========================================================================

xbool geom_rsc_desc::OnProperty( prop_query& I )
{
    if( rsc_desc::OnProperty( I ) )
    {   
        // nothing to do
    }
    else if( I.IsVar( "ResDesc\\FileName" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFileName( m_FileName, MAX_NAME_LENGTH );
        }
        else
        {
            x_strsavecpy( m_FileName, I.GetVarFileName(), MAX_NAME_LENGTH );
        }
    }
    else if( I.IsVar( "ResDesc\\PhysicsMatx" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFileName( m_PhysicsMatx, MAX_NAME_LENGTH );
        }
        else
        {
            x_strsavecpy( m_PhysicsMatx, I.GetVarFileName(), MAX_NAME_LENGTH );
        }
    }
    else if( I.IsVar( "ResDesc\\SettingsFile" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFileName( m_SettingsFile, MAX_NAME_LENGTH );
        }
        else
        {
            x_strsavecpy( m_SettingsFile, I.GetVarFileName(), MAX_NAME_LENGTH );
        }
    }
    else if( I.IsVar( "ResDesc\\ForceMeshInfoUpdate" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Refresh Mesh Info" );
        }
        else
        {
            RefreshMatxFileInfo();
        }
    }
    else if( I.IsVar( "ResDesc\\UVAnims\\NumUVAnims" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_UVAnimList.GetCount() );
        }
        else
        {
            m_UVAnimList.SetCount( I.GetVarInt( 0, 32 ) );
        }
    }
    else if( OnPropertyUVAnim( I ) )
    {
        // nothing to do
    }
    else if( I.IsVar( "ResDesc\\VirtualMeshes\\NumVirtualMeshes" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_VirtualMeshes.GetCount() );
        }
        else
        {
            m_VirtualMeshes.SetCount( I.GetVarInt( 0, 32 ) );
        }
    }
    else if( I.IsVar( "ResDesc\\VirtualMeshes\\SortVirtualMeshes" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Sort virtual meshes" );
        }
        else
        {
            SortVMeshes();
        }
    }
    else if( I.IsVar( "ResDesc\\VirtualMeshes\\AddVirtualMesh" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add Virtual Mesh" );
        }
        else
        {
            m_VirtualMeshes.Append();
        }
    }
    else if( OnPropertyVirtualMesh( I ) )
    {
        // nothing to do
    }
    else if( I.IsVar( "ResDesc\\VirtualTextures\\NumVirtualTextures" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_VirtualTextures.GetCount() );
        }
        else
        {
            m_VirtualTextures.SetCount( I.GetVarInt( 0, 32 ) );
        }
    }
    else if( I.IsVar( "ResDesc\\VirtualTextures\\AddVirtualTexture" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add Virtual Texture" );
        }
        else
        {
            virtual_texture& VTex = m_VirtualTextures.Append();
            VTex.Reset();
        }
    }
    else if( OnPropertyVirtualTexture( I ) )
    {
        // nothing to do
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

xbool geom_rsc_desc::OnPropertyUVAnim( prop_query& I )
{
    // early out
    if( !I.IsBasePath( "ResDesc\\UVAnims\\UVAnim[]" ) )
    {
        return FALSE;
    }

    // range check
    s32 UVAnimId = I.GetIndex( 0 );
    if( (UVAnimId < 0) || (UVAnimId >= m_UVAnimList.GetCount()) )
    {
        return FALSE;
    }

    // grab a handy reference to the uv animation
    uv_animation& UVAnim = m_UVAnimList[UVAnimId];

    // header?
    if( I.IsVar( "ResDesc\\UVAnims\\UVAnim[]" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( UVAnim.MatName, MAX_NAME_LENGTH );
        }

        return TRUE;
    }

    // push the header to simplify the rest of this process
    s32 HeaderId = I.PushPath( "ResDesc\\UVAnims\\UVAnim[]\\" );

    // handle the properties
    if( I.VarString( "MaterialName", UVAnim.MatName, MAX_NAME_LENGTH ) )
    {
    }
    else if( I.IsVar( "AnimType" ) )
    {
        if( I.IsRead() )
        {
            switch( UVAnim.AnimType )
            {
            case 0: I.SetVarEnum( "Fixed" );    break;
            case 1: I.SetVarEnum( "Looped" );   break;
            case 2: I.SetVarEnum( "PingPong" ); break;
            case 3: I.SetVarEnum( "OneShot" );  break;
            }
        }
        else
        {
            const char* AnimEnum = I.GetVarEnum();
            if( !x_strcmp( AnimEnum, "Fixed" ) )
                UVAnim.AnimType = 0;
            else if( !x_strcmp( AnimEnum, "Looped" ) )
                UVAnim.AnimType = 1;
            else if( !x_strcmp( AnimEnum, "PingPong" ) )
                UVAnim.AnimType = 2;
            else if( !x_strcmp( AnimEnum, "OneShot" ) )
                UVAnim.AnimType = 3;
        }
    }
    else if( I.VarInt( "Start", UVAnim.StartFrame, 0, S32_MAX ) )
    {
    }
    else if( I.VarInt( "FPS", UVAnim.FPS, 0, S32_MAX) )
    {
    }
    else if( I.VarInt( "NumFrames", UVAnim.nFrames, 0, S32_MAX ) )
    {
    }
    else
    {
        I.PopPath( HeaderId );
        return FALSE;
    }

    // pop the path and return out
    I.PopPath( HeaderId );
    return TRUE;
}

//=========================================================================

xbool geom_rsc_desc::OnPropertyVirtualMesh( prop_query& I )
{
    if( !I.IsBasePath( "ResDesc\\VirtualMeshes\\VirtualMesh[]" ) )
    {
        return FALSE;
    }

    // range check
    s32 VMeshId = I.GetIndex( 0 );
    if( (VMeshId < 0) || (VMeshId >= m_VirtualMeshes.GetCount()) )
    {
        return FALSE;
    }
    
    // grab a handy reference to the virtual mesh
    virtual_mesh& VMesh = m_VirtualMeshes[VMeshId];

    // header?
    if( I.IsVar( "ResDesc\\VirtualMeshes\\VirtualMesh[]" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( VMesh.Name, MAX_NAME_LENGTH );
        }
        
        return TRUE;
    }

    // push the header to simplify the rest of this process
    s32 HeaderId = I.PushPath( "ResDesc\\VirtualMeshes\\VirtualMesh[]\\" );

    // handle the properties
    if( I.IsVar( "RemoveVirtualMesh" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Remove Virtual Mesh" );
        }
        else
        {
            m_VirtualMeshes.Delete( VMeshId );
        }
    }
    else if( I.IsVar( "Name" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( VMesh.Name, MAX_NAME_LENGTH );
        }
        else
        {
            x_strsavecpy( VMesh.Name, I.GetVarString(), MAX_NAME_LENGTH );
        }
    }
    else if( I.IsVar( "NumLODs" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( VMesh.LODs.GetCount() );
        }
        else
        {
            VMesh.LODs.SetCount( I.GetVarInt(0, 32) );
        }
    }
    else if( I.IsVar( "AddLOD" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add LOD" );
        }
        else
        {
            VMesh.LODs.Append();
        }
    }
    else if( OnPropertyLODs( I, VMeshId ) )
    {
        // nothing to do
    }
    else
    {
        I.PopPath( HeaderId );
        return FALSE;
    }

    // pop the path and return out
    I.PopPath( HeaderId );
    return TRUE;
}

//=========================================================================

xbool geom_rsc_desc::OnPropertyLODs( prop_query& I, s32 VMeshId )
{
    ASSERT( (VMeshId >= 0) && (VMeshId < m_VirtualMeshes.GetCount()) );
    
    // range check
    virtual_mesh& VMesh = m_VirtualMeshes[VMeshId];
    s32           LodId = I.GetIndex( 1 );
    if( (LodId < 0) || (LodId >= VMesh.LODs.GetCount()) )
    {
        return FALSE;
    }

    // header?
    if( I.IsVar( "LOD[]" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( xfs( "ScreenSize(%d)", VMesh.LODs[LodId].ScreenSize ), 256 );
        }

        return TRUE;
    }

    // push the header to simplify the rest of this process
    s32 HeaderId = I.PushPath( "LOD[]\\" );

    // handle the properties
    if( I.IsVar( "RemoveLOD" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Remove LOD" );
        }
        else
        {
            VMesh.LODs.Delete( LodId );
        }
    }
    else if( I.VarInt( "NumMeshs", VMesh.LODs[LodId].nMeshes, 0, lod_info::MAX_LOD_MESHES ) )
    {
    }
    else if( I.VarInt( "ScreenSize", VMesh.LODs[LodId].ScreenSize, 0, S16_MAX ) )
    {
    }
    else if( I.VarEnum( "MeshList", m_SelectedMesh ) )
    {
    }
    else if( I.IsVar( "AddMesh" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add Mesh" );
        }
        else
        {
            VMesh.LODs[LodId].nMeshes = MIN( lod_info::MAX_LOD_MESHES, VMesh.LODs[LodId].nMeshes + 1 );
            x_strsavecpy( VMesh.LODs[LodId].MeshName[VMesh.LODs[LodId].nMeshes-1], m_SelectedMesh, MAX_NAME_LENGTH );
        }
    }
    else if( OnPropertyMesh( I, VMeshId, LodId ) )
    {
    }
    else
    {
        I.PopPath( HeaderId );
        return FALSE;
    }

    // pop the path and return out
    I.PopPath( HeaderId );
    return TRUE;
}

//=========================================================================

xbool geom_rsc_desc::OnPropertyMesh( prop_query& I, s32 VMeshId, s32 LodId )
{
    ASSERT( (VMeshId >= 0) && (VMeshId < m_VirtualMeshes.GetCount()) );
    ASSERT( (LodId >= 0) && (LodId < m_VirtualMeshes[VMeshId].LODs.GetCount()) );

    // range check
    virtual_mesh& VMesh  = m_VirtualMeshes[VMeshId];
    lod_info&     LOD    = VMesh.LODs[LodId];
    s32           MeshId = I.GetIndex( 2 );
    if( (MeshId < 0) || (MeshId >= LOD.nMeshes) )
    {
        return FALSE;
    }

    // handle the properties
    if( I.IsVar( "Mesh[]" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( LOD.MeshName[MeshId], MAX_NAME_LENGTH );
        }
    }
    else if( I.VarString( "Mesh[]\\Name", LOD.MeshName[MeshId], MAX_NAME_LENGTH ) )
    {
    }
    else if( I.IsVar( "Mesh[]\\RemoveMesh" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Remove Mesh" );
        }
        else
        {
            x_memmove( LOD.MeshName[MeshId],
                       LOD.MeshName[MeshId+1],
                       MAX_NAME_LENGTH*(lod_info::MAX_LOD_MESHES-MeshId-1) );
            LOD.nMeshes--;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

xbool geom_rsc_desc::OnPropertyVirtualTexture( prop_query& I )
{
    if( !I.IsBasePath( "ResDesc\\VirtualTextures\\VirtualTexture[]" ) )
    {
        return FALSE;
    }

    // range check
    s32 VTexId = I.GetIndex( 0 );
    if( (VTexId < 0) || (VTexId >= m_VirtualTextures.GetCount()) )
    {
        return FALSE;
    }

    // grab a handy reference to the virtual texture
    virtual_texture& VTex = m_VirtualTextures[VTexId];

    // header?
    if( I.IsVar( "ResDesc\\VirtualTextures\\VirtualTexture[]" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( VTex.Name, MAX_NAME_LENGTH );
        }

        return TRUE;
    }

    // push the header to simplify the rest of this process
    s32 HeaderId = I.PushPath( "ResDesc\\VirtualTextures\\VirtualTexture[]\\" );

    // handle the properties
    if( I.IsVar( "RemoveVirtualTexture" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Remove Virtual Texture" );
        }
        else
        {
            m_VirtualTextures.Delete( VTexId );
        }
    }
    else if( I.VarString( "Name", VTex.Name, MAX_NAME_LENGTH ) )
    {
    }
    else if( I.IsVar( "AddDiffuseTexture" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add Diffuse Texture" );
        }
        else
        {
            VTex.Textures.Append();
        }
    }
    else if( I.IsVar( "NumDiffTextures" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( VTex.Textures.GetCount() );
        }
        else
        {
            VTex.Textures.SetCount( I.GetVarInt( 0, 32 ) );
        }
    }
    else if( I.VarBool( "OverrideDefault", VTex.OverrideDefault ) )
    {
    }
    else if( I.VarFileName( "OverrideFileName", VTex.OverrideFileName, MAX_NAME_LENGTH ) )
    {
    }
    else if( OnPropertyDiffTexture( I, VTexId ) )
    {
    }
    else
    {
        return FALSE;
    }

    // pop the path and return out
    I.PopPath( HeaderId );
    return TRUE;
}

//=========================================================================

xbool geom_rsc_desc::OnPropertyDiffTexture( prop_query& I, s32 VTexId )
{
    ASSERT( (VTexId >= 0) && (VTexId < m_VirtualTextures.GetCount()) );

    // range check
    virtual_texture& VTex   = m_VirtualTextures[VTexId];
    s32              DiffId = I.GetIndex( 1 );
    if( (DiffId < 0) || (DiffId >= VTex.Textures.GetCount()) )
    {
        return FALSE;
    }

    // handle the properties
    if( I.IsVar( "DiffTexture[]" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( VTex.Textures[DiffId].Name, MAX_NAME_LENGTH );
        }
    }
    else if( I.IsVar( "DiffTexture[]\\FileName" ) )
    {
        if( DiffId == 0 )
        {
            I.VarFileName( "DiffTexture[]\\FileName", VTex.Textures[DiffId].FileName, MAX_NAME_LENGTH );
        }
        else
        {
            I.VarEnum( "DiffTexture[]\\FileName", VTex.Textures[DiffId].FileName );
        }
    }
    else if( I.VarString( "DiffTexture[]\\Name", VTex.Textures[DiffId].Name, MAX_NAME_LENGTH ) )
    {
    }
    else if( I.IsVar( "DiffTexture[]\\RemoveDiffTexture" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Remove texture" );
        }
        else
        {
            VTex.Textures.Delete( DiffId );
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

void geom_rsc_desc::SortVMeshes( void )
{
    s32 i, j;
    
    // do a simple bubble sort of the vmeshes...this is really inefficient,
    // but there are so few items, it's probably not a big deal.
    xbool bSorted = FALSE;
    for( i = 0; i < (m_VirtualMeshes.GetCount() - 1) && !bSorted; i++ )
    {
        bSorted = TRUE;
        for( j = 0; j < m_VirtualMeshes.GetCount() - 1 - i; j++ )
        {
            if( x_strcmp( m_VirtualMeshes[j].Name, m_VirtualMeshes[j+1].Name ) > 0 )
            {
                virtual_mesh Temp    = m_VirtualMeshes[j];
                m_VirtualMeshes[j]   = m_VirtualMeshes[j+1];
                m_VirtualMeshes[j+1] = Temp;
                bSorted              = FALSE;
            }
        }
    }
}

//=========================================================================

void geom_rsc_desc::OnGetCompilerDependencies( xarray<xstring>& List )
{
    // Add main matx file
    List.Append() = m_FileName;
    
    // Add physics matx if specified
    if( m_PhysicsMatx[0] )
        List.Append() = m_PhysicsMatx;
    
    // Add settings file if specified
    if( m_SettingsFile[0] )
        List.Append() = m_SettingsFile;
}

//=========================================================================

void geom_rsc_desc::OnGetFinalDependencies( xarray<xstring>& List, platform Platform, const char* pDirectory )
{
    (void)List;
    (void)Platform;
    (void)pDirectory;
}

//=========================================================================

void geom_rsc_desc::OnCheckIntegrity( void )
{
    rsc_desc::OnCheckIntegrity();

    if( m_FileName[0] == 0 )
        x_throw( "You must enter a matx file name" );
}

//=========================================================================

void geom_rsc_desc::OnStartEdit( void )
{
    rsc_desc::OnStartEdit();
    RefreshMatxFileInfo();
}

//=========================================================================

void geom_rsc_desc::RefreshMatxFileInfo( void )
{
    // make a copy of the uv animation list, so we can remap them after
    // doing a refresh from the geometry
    xarray<uv_animation> OldAnimList = m_UVAnimList;

    // load the matx file
    rawmesh2 RawMesh;
    xbool bLoaded = RawMesh.Load( m_FileName );
    if( bLoaded )
    {
        // clear out our compiled lists
        m_MatxMeshList.Clear();
        m_MatxDiffuseList.Clear();
        m_UVAnimList.Clear();

        // clean the mesh so we have the same data the compiler would use
        RawMesh.CleanMesh();

        // copy out the mesh names for use in our drop-down list
        s32 i, j;
        for( i = 0; i < RawMesh.m_nSubMeshs; i++ )
        {
            // NOTE: The mesh/submesh naming convention is inconsistent.
            // A rawmesh2 submesh is the same thing as a compiled piece
            // of geometry's mesh
            matx_mesh_info& MeshInfo = m_MatxMeshList.Append();
            x_strsavecpy( MeshInfo.Name, RawMesh.m_pSubMesh[i].Name, MAX_NAME_LENGTH );
        }

        // copy out any uv animations for use in our dialogs
        for( i = 0; i < RawMesh.m_nMaterials; i++ )
        {
            rawmesh2::material&  RawMat = RawMesh.m_pMaterial[i];
            rawmesh2::param_pkg& Param  = RawMat.Map[0].UVTranslation;
            if( Param.nKeys > 0 )
            {
                uv_animation& AnimInfo = m_UVAnimList.Append();
                x_strsavecpy( AnimInfo.MatName, RawMat.Name, MAX_NAME_LENGTH );
                AnimInfo.nFrames = Param.nKeys;
            }
        }

        // try to match up the old uv animations to the new ones so that
        // we don't lose settings
        for( i = 0; i < OldAnimList.GetCount(); i++ )
        {
            for( j = 0; j < m_UVAnimList.GetCount(); j++ )
            {
                if( !x_strcmp( OldAnimList[i].MatName, m_UVAnimList[j].MatName ) )
                {
                    m_UVAnimList[j].AnimType   = OldAnimList[i].AnimType;
                    m_UVAnimList[j].StartFrame = OldAnimList[i].StartFrame;
                    m_UVAnimList[j].FPS        = OldAnimList[i].FPS;
                    break;
                }
            }
        }

        // copy out all the diffuse texture names
        for( i = 0; i < RawMesh.m_nMaterials; i++ )
        {
            rawmesh2::material& RawMat  = RawMesh.m_pMaterial[i];
            s32                 iRawTex = RawMat.Map[0].iTexture;
            if( iRawTex >= 0 )
            {
                rawmesh2::texture& RawTex = RawMesh.m_pTexture[iRawTex];
                
                // strip out the drive and path, we're only interested in
                // the filename
                char FName[X_MAX_FNAME];
                char Ext[X_MAX_EXT];
                x_splitpath( RawTex.FileName, NULL, NULL, FName, Ext );
                x_makepath( FName, NULL, NULL, FName, Ext );

                // has this filename already been added to our list?
                xbool bAlreadyAdded = FALSE;
                for( j = 0; j < m_MatxDiffuseList.GetCount(); j++ )
                {
                    if( !x_stricmp( m_MatxDiffuseList[j].Name, FName ) )
                    {
                        bAlreadyAdded = TRUE;
                        break;
                    }
                }

                // no? then add it
                if( !bAlreadyAdded )
                {
                    matx_tex_info& TexInfo = m_MatxDiffuseList.Append();
                    x_strsavecpy( TexInfo.Name, FName, MAX_NAME_LENGTH );
                }
            }
        }
    }
}

//=========================================================================

void geom_rsc_desc::SetColoredMips( xbool OnOff )
{
    s_ColoredMips = OnOff;
}

//=========================================================================
