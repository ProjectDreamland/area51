//==============================================================================
//  static_decal.hpp
//
//  Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class describes where a static decal will be placed in the level. It is
//  only meant to be used in the editor, and these static decals will be
//  collected and exported in a more compressed format for the game-side.
//==============================================================================

#ifndef STATIC_DECAL_HPP
#define STATIC_DECAL_HPP

//==============================================================================
// Includes
//==============================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "..\Support\Decals\DecalPackage.hpp"
#include "..\Support\Decals\DecalMgr.hpp"

//==============================================================================
// Class
//==============================================================================

class static_decal : public object
{
public:

    CREATE_RTTI( static_decal, object, object )

                            static_decal        ( void );
    virtual                 ~static_decal       ( void );
    virtual s32             GetMaterial         ( void ) const { return MAT_TYPE_NULL; }
    virtual bbox            GetLocalBBox        ( void ) const;

    virtual void            OnEnumProp          ( prop_enum& List );
    virtual xbool           OnProperty          ( prop_query& I );
    virtual void            OnRender            ( void );
    virtual void            OnColCheck          ( void );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    
    xbool                   IsValid             ( void ) const;
    const char*             GetPackageName      ( void ) const;
    s32                     GetGroup            ( void ) const;
    s32                     GetDecalDef         ( void ) const;
    const decal_definition* GetDecalDefinition  ( void );
    vector2                 GetSize             ( void ) const;
    radian                  GetRoll             ( void ) const;
    s32                     GetNVerts           ( void ) const;
    decal_mgr::decal_vert*  GetVertPtr          ( void );

    void                    SetValid            ( xbool Valid );
    void                    SetSize             ( const vector2& Size );
    void                    SetRoll             ( radian Roll );
    void                    SetNVerts           ( s32 nVerts );

protected:
    rhandle<decal_package>      m_DecalPackage;
    s32                         m_Group;
    s32                         m_Decal;
    xbool                       m_IsValidDecal;
    vector2                     m_Size;
    radian                      m_Roll;
    s32                         m_nVerts;
    decal_mgr::decal_vert       m_Verts[decal_mgr::MAX_VERTS_PER_DECAL];
};

//==============================================================================
// Inlines
//==============================================================================

inline xbool static_decal::IsValid( void ) const
{
    return m_IsValidDecal;
}

//==============================================================================

inline const char* static_decal::GetPackageName( void ) const
{
    return m_DecalPackage.GetName();
}

//==============================================================================

inline s32 static_decal::GetGroup( void ) const
{
    return m_Group;
}

//==============================================================================

inline s32 static_decal::GetDecalDef( void ) const
{
    return m_Decal;
}

//==============================================================================

inline const decal_definition* static_decal::GetDecalDefinition( void )
{
    decal_package*  pDecalPkg = m_DecalPackage.GetPointer();
    if ( !pDecalPkg )
        return NULL;

    if ( (m_Group < 0) || (m_Group > pDecalPkg->GetNGroups()) )
        return NULL;

    if ( (m_Decal < 0) || (m_Decal > pDecalPkg->GetNDecalDefs(m_Group)) )
        return NULL;

    return &pDecalPkg->GetDecalDef( m_Group, m_Decal );
}

//==============================================================================

inline vector2 static_decal::GetSize( void ) const
{
    return m_Size;
}

//==============================================================================

inline radian static_decal::GetRoll( void ) const
{
    return m_Roll;
}

//==============================================================================

inline s32 static_decal::GetNVerts( void ) const
{
    return m_nVerts;
}

//==============================================================================

inline decal_mgr::decal_vert* static_decal::GetVertPtr( void )
{
    return m_Verts;
}

//==============================================================================

inline void static_decal::SetValid( xbool Valid )
{
    m_IsValidDecal = Valid;
}

//==============================================================================

inline void static_decal::SetSize( const vector2& Size )
{
    m_Size           = Size;
}

//==============================================================================

inline void static_decal::SetRoll( radian Roll )
{
    m_Roll           = Roll;
}

//==============================================================================

inline void static_decal::SetNVerts( s32 nVerts )
{
    if ( nVerts == 0 )
        SetValid( FALSE );
    m_nVerts         = nVerts;
}

//==============================================================================

#endif // STATIC_DECAL_HPP