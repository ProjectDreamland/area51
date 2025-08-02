#include "Portal.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "ZoneMgr\ZoneMgr.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

#define MAX_PORTAL_DIMENSION    250000

//=========================================================================

static struct zone_portal_desc : public object_desc
{
    zone_portal_desc( void ) : object_desc( 
            object::TYPE_ZONE_PORTAL, 
            "Portal", 
            "SYSTEM",

            object::ATTR_EDITOR_TEMP_OBJECT     |
            object::ATTR_COLLIDABLE             |
            object::ATTR_COLLISION_PERMEABLE    |
            object::ATTR_BLOCKS_ALL_PROJECTILES |
            object::ATTR_BLOCKS_ALL_ACTORS      |
            object::ATTR_BLOCKS_RAGDOLL         |
            object::ATTR_BLOCKS_SMALL_DEBRIS    |
            object::ATTR_TRANSPARENT            |
            object::ATTR_SPACIAL_ENTRY          |
            object::ATTR_RENDERABLE,

            FLAGS_NO_ALLOW_COPY | 
            FLAGS_IS_DYNAMIC ) {}         

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new zone_portal; }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR
    
    virtual s32  OnEditorRender( object& Object ) const
    {
#ifndef X_RETAIL
        object_desc::OnEditorRender( Object );
        if( Object.IsKindOf( zone_portal::GetRTTI() ) )
        {
            zone_portal& Portal = zone_portal::GetSafeType( Object );   
            Portal.RenderVolume();
        }
#endif //X_RETAIL
        return EDITOR_ICON_PORTAL;
    }

#endif // X_EDITOR

} s_PortalObj_Desc;

//=========================================================================

const object_desc& zone_portal::GetTypeDesc( void ) const
{
    return s_PortalObj_Desc;
}

//=========================================================================

const object_desc& zone_portal::GetObjectType( void )
{
    return s_PortalObj_Desc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

zone_portal::zone_portal( void ) 
{
    m_Zone1             = 0;
    m_Zone2             = 0;
    m_Width             = 400.0f;
    m_Height            = 400.0f;
    m_SoundOcclusion    = 1.0f;
}

//=========================================================================

bbox zone_portal::GetLocalBBox( void ) const
{
    return bbox(vector3(m_Width/2,m_Height/2,1.0f),vector3(-m_Width/2,-m_Height/2,-1.0f));
}

//=========================================================================

xbool zone_portal::IsPortalValid( void )
{
    if (m_Zone1 == 0)
        return FALSE;

    if (m_Zone2 == 0)
        return FALSE;

    return TRUE;
}

//=========================================================================

#if !defined( CONFIG_RETAIL )

void zone_portal::OnRender( void )
{
    CONTEXT( "zone_portal::OnRender" );
#ifndef X_EDITOR
    draw_BBox    ( GetBBox(), xcolor(255,255,255,255) );
#endif //X_EDITOR
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================

#ifndef X_RETAIL
void zone_portal::RenderVolume( void )
{
    draw_BBox    ( GetBBox(), xcolor(255,255,255,255) );  

    xcolor Clr;

    if (g_ZoneMgr.IsPortalOn( GetGuid() ))
        Clr.Set(125,255,125,55);
    else
        Clr.Set(255,255,125,55);

    if ( !IsPortalValid() )
        Clr.Set(255,255,0,128);

    draw_SetL2W( GetL2W() );
    draw_Volume( bbox( vector3(-m_Width/2,-m_Height/2,5), vector3(m_Width/2,m_Height/2,-5) ), Clr );
    draw_ClearL2W();
}
#endif // X_RETAIL

//=========================================================================

#ifndef X_RETAIL
void zone_portal::OnDebugRender( void )
{
#ifndef X_EDITOR
    CONTEXT( "zone_portal::OnDebugRender" );
    RenderVolume();
#endif //X_EDITOR
}
#endif // X_RETAIL

//=========================================================================

#if !defined( CONFIG_RETAIL )

void zone_portal::OnRenderTransparent( void )
{
#ifndef X_RETAIL
#ifndef X_EDITOR
    CONTEXT( "zone_portal::OnRenderTransparent" );
    RenderVolume();
#endif //X_EDITOR
#endif //X_RETAIL
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================

void zone_portal::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "Portal",           "Portal Properties", 0 );
    List.PropEnumInt     ( "Portal\\Zone1",    "Connection to 1st Zone", PROP_TYPE_DONT_SHOW );
    List.PropEnumInt     ( "Portal\\Zone2",    "Connection to 2nd Zone", PROP_TYPE_DONT_SHOW );
    List.PropEnumExternal( "Portal\\1st Zone", "Zoning\0Zoning", "Which Zone to connect to...", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Portal\\2nd Zone", "Zoning\0Zoning", "Which Zone to connect to...", PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat   ( "Portal\\Width",    "Portal Width.", 0 );
    List.PropEnumFloat   ( "Portal\\Height",   "Portal Height.", 0 );
}

//=============================================================================

xbool zone_portal::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
    {
    }
    else if( I.IsVar( "Portal\\1st Zone" ) )
    {
    }
    else if( I.IsVar( "Portal\\2nd Zone" ) )
    {
    }
    else if( I.IsVar( "Portal\\Zone1" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_Zone1 );
        }
        else
        {
            m_Zone1 = I.GetVarInt();
        }
    }
    else if( I.IsVar( "Portal\\Zone2" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_Zone2 );
        }
        else
        {
            m_Zone2 = I.GetVarInt();
        }
    }
    else if( I.VarFloat("Portal\\Width", m_Width ) )
    {
        if (m_Width > MAX_PORTAL_DIMENSION)
        {
            m_Width = MAX_PORTAL_DIMENSION;
            return FALSE;
        }

        // This is to force recompute the bbox
        OnMove( GetPosition() );
    }
    else if( I.VarFloat("Portal\\Height", m_Height ) )
    {
        if (m_Height > MAX_PORTAL_DIMENSION)
        {
            m_Height = MAX_PORTAL_DIMENSION;
            return FALSE;
        }

        // This is to force recompute the bbox
        OnMove( GetPosition() );
    }
    else
    {   
        return FALSE ;        
    }

    return TRUE;
}





