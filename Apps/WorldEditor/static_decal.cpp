//==============================================================================
//  static_decal.cpp
//
//  Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class describes where a static decal will be placed in the level. It is
//  only meant to be used in the editor, and these static decals will be
//  collected and exported in a more compressed format for the game-side.
//==============================================================================

//==============================================================================
// Includes
//==============================================================================

#include "stdafx.h"
#include "static_decal.hpp"

//==============================================================================
// Object description
//==============================================================================

void StaticDecal_Link( void ){}

//==============================================================================

static struct static_decal_desc : public object_desc
{
    static_decal_desc( void ) : object_desc( object::TYPE_EDITOR_STATIC_DECAL,
                                             "Static Decal",
                                             "EDITOR",
                                             object::ATTR_SPACIAL_ENTRY          |
                                             object::ATTR_COLLISION_PERMEABLE    |
                                             object::ATTR_RENDERABLE             |
                                             object::ATTR_EDITOR_TEMP_OBJECT,
                                             FLAGS_NO_ICON ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return  new static_decal; }

    //-------------------------------------------------------------------------

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

} s_static_decal_Desc;

//=========================================================================

const object_desc& static_decal::GetTypeDesc( void ) const
{
    return s_static_decal_Desc;
}

//=========================================================================

const object_desc& static_decal::GetObjectType( void )
{
    return s_static_decal_Desc;
}

//=========================================================================
// Implementation
//=========================================================================

static_decal::static_decal( void ) :
    object(),
    m_Group         (0),
    m_Decal         (0),
    m_IsValidDecal  (FALSE),
    m_Size          (vector2(10.0f,10.0f)),
    m_Roll          (R_0),
    m_nVerts        (0)
{
    m_DecalPackage.SetName("");
}

//=========================================================================

static_decal::~static_decal( void )
{
}

//=========================================================================

bbox static_decal::GetLocalBBox( void ) const
{
    bbox LocalBBox( vector3(0.0f,0.0f,0.0f), 20.0f );

    if ( IsValid() )
    {
        for ( s32 i = 0; i < m_nVerts; i++ )
        {
            LocalBBox += m_Verts[i].Pos;
        }
    }

    return LocalBBox;
}

//=========================================================================

void static_decal::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "StaticDecal", "Static Decal Properties", 0 );
    List.PropEnumExternal( "StaticDecal\\DecalPackage",  "Resource\0decalpkg\0", "Decal package", PROP_TYPE_MUST_ENUM );
    List.PropEnumInt     ( "StaticDecal\\GroupIndex",    "The group index inside the package.", 0 );
    List.PropEnumInt     ( "StaticDecal\\DecalIndex",    "The decal definition index in the group.", 0 );
    List.PropEnumVector2 ( "StaticDecal\\DecalSize",     "Dimensions of decal.", 0 );
    List.PropEnumAngle   ( "StaticDecal\\DecalRoll",     "Amount to rotate decal.", 0 );
    List.PropEnumBool    ( "StaticDecal\\IsValid",       "Does this decal have valid verts, etc?", 0 );

    // add the verts
    List.PropEnumInt     ( "StaticDecal\\VertCount",     "Number of verts in this decal.", PROP_TYPE_DONT_SHOW );
    for ( s32 i = 0; i < m_nVerts; i++ )
    {
        List.PropEnumVector3 ( xfs("StaticDecal\\Vert[%d]\\Pos",   i), "Vertex Position", PROP_TYPE_DONT_SHOW );
        List.PropEnumInt     ( xfs("StaticDecal\\Vert[%d]\\Flags", i), "Vertex Flags",    PROP_TYPE_DONT_SHOW );
        List.PropEnumVector2 ( xfs("StaticDecal\\Vert[%d]\\UV",    i), "Vertex UV",       PROP_TYPE_DONT_SHOW );
    }
}

//=========================================================================

xbool static_decal::OnProperty( prop_query& I )
{
    char PackageName[256];
    x_strcpy( PackageName, m_DecalPackage.GetName() );

    if ( object::OnProperty( I ) )
    {
    }
    else
    if ( I.VarExternal( "StaticDecal\\DecalPackage", PackageName, 256 ) )
    {
        if ( !I.IsRead() )
        {
            m_DecalPackage.SetName( PackageName );
            m_DecalPackage.GetPointer();
        }
    }
    else
    if ( I.VarInt( "StaticDecal\\GroupIndex", m_Group ) )
    {
    }
    else
    if ( I.VarInt( "StaticDecal\\DecalIndex", m_Decal ) )
    {
    }
    else
    if ( I.VarVector2( "StaticDecal\\DecalSize", m_Size ) )
    {
    }
    else
    if ( I.VarAngle( "StaticDecal\\DecalRoll", m_Roll, R_0, R_360 ) )
    {
    }
    else
    if ( I.VarBool( "StaticDecal\\IsValid", m_IsValidDecal ) )
    {
    }
    else
    if ( I.VarInt( "StaticDecal\\VertCount", m_nVerts ) )
    {
    }
    else
    if ( I.IsBasePath( "StaticDecal\\Vert[]" ) )
    {
        s32 Index        = I.GetIndex(0);
        if ( I.VarVector3( "StaticDecal\\Vert[]\\Pos", m_Verts[Index].Pos ) )
        {
        }
        else
        if ( I.VarInt( "StaticDecal\\Vert[]\\Flags", m_Verts[Index].Flags ) )
        {
        }
        else
        if ( I.VarVector2( "StaticDecal\\Vert[]\\UV", m_Verts[Index].UV ) )
        {
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

void static_decal::OnRender( void )
{
    xbool bDoWireframe = FALSE;
    u32 AttrBits = GetAttrBits();
    if ( IsValid() && 
         ((AttrBits&object::ATTR_EDITOR_PLACEMENT_OBJECT) ||
          (AttrBits&object::ATTR_EDITOR_SELECTED)) )
    {
        bDoWireframe = TRUE;
    }

    const decal_definition* pDecalDef = GetDecalDefinition();
    if ( IsValid() && pDecalDef )
    {
        OnTransform( GetL2W() );
        g_DecalMgr.RenderStaticDecal( *pDecalDef,
                                      m_Verts,
                                      m_nVerts,
                                      GetL2W(),
                                      bDoWireframe );
    }
}

//=========================================================================

void static_decal::OnColCheck( void )
{
    if ( g_CollisionMgr.IsUsingHighPoly() && IsValid() )
    {
        const matrix4& L2W = GetL2W();
        const matrix4  W2L = m4_InvertRT( L2W );
        g_CollisionMgr.StartApply( GetGuid(), L2W, W2L );
        
        s32   i;
        xbool WindingCW = TRUE;        
        for ( i = 0; i < m_nVerts; i++ )
        {
            if ( m_Verts[i].Flags & decal_mgr::decal_vert::FLAG_SKIP_TRIANGLE )
            {
                WindingCW = TRUE;
                continue;
            }

            ASSERT( i>=2 );
            
            if ( WindingCW )
                g_CollisionMgr.ApplyTriangle( m_Verts[i-2].Pos, m_Verts[i-1].Pos, m_Verts[i].Pos );
            else
                g_CollisionMgr.ApplyTriangle( m_Verts[i].Pos, m_Verts[i-1].Pos, m_Verts[i-2].Pos );
            WindingCW = !WindingCW;
        }

        g_CollisionMgr.EndApply();
    }
}