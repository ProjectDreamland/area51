//==============================================================================
//
//  blueprint_anchor.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "stdafx.h"

#include "blueprint_anchor.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Parsing\TextIn.hpp"
#include "x_color.hpp"
#include "Entropy.hpp"
#include "Render\Editor\editor_icons.hpp"

const f32 c_Sphere_Radius = 50.0f;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================
void BluePrintAnchor_Link( void ){}

//=========================================================================

static struct blueprint_anchor_desc : public object_desc
{
    blueprint_anchor_desc( void ) : object_desc( object::TYPE_EDITOR_BLUEPRINT_ANCHOR, 
                                        "Editor BPAnchor",
                                        "EDITOR",
                                        object::ATTR_SPACIAL_ENTRY          |
                                        object::ATTR_COLLISION_PERMEABLE    |
                                        object::ATTR_EDITOR_TEMP_OBJECT ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return  new blueprint_anchor; }

    //-------------------------------------------------------------------------

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_ANCHOR;
    }

} s_blueprint_anchor_Desc;

//=========================================================================

const object_desc& blueprint_anchor::GetTypeDesc( void ) const
{
    return s_blueprint_anchor_Desc;
}

//=========================================================================

const object_desc& blueprint_anchor::GetObjectType( void )
{
    return s_blueprint_anchor_Desc;
}

//==============================================================================
// blueprint_anchor
//==============================================================================

//==============================================================================
blueprint_anchor::blueprint_anchor(void) 
{
}

//==============================================================================
blueprint_anchor::~blueprint_anchor(void)
{
}

//==============================================================================

void blueprint_anchor::OnImport ( text_in& TextIn )
{
}

//==============================================================================
void blueprint_anchor::OnInit(void)
{
    object::OnInit();
}

//==============================================================================

#ifndef X_RETAIL
void blueprint_anchor::OnDebugRender ( void )
{
}
#endif // X_RETAIL

//==============================================================================

bbox blueprint_anchor::GetLocalBBox( void ) const 
{ 
    return bbox(vector3(0,0,0), c_Sphere_Radius);
}

//==============================================================================

void blueprint_anchor::OnEnumProp ( prop_enum& List )
{
    object::OnEnumProp(List);
}

//==============================================================================
xbool   blueprint_anchor::OnProperty( prop_query&        I    )
{
    return object::OnProperty(I);
}

//==============================================================================
void  blueprint_anchor::OnMove( const vector3& newPos )
{
    object::OnMove(newPos);
}
