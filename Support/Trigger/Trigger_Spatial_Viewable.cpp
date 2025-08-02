//=========================================================================
//
//  trigger_spatial_viewable.cpp
//
//=========================================================================

//=========================================================================
//  INCLUDES
//=========================================================================

#include "Trigger\Trigger_Spatial_Viewable.hpp"
#include "Trigger\Trigger_Manager.hpp"
#include "Entropy.hpp"
#include "Render\Editor\editor_icons.hpp"

#include "Objects\Door.hpp"

//=========================================================================
// STATIC DEFINITIONS AND CONSTS
//=========================================================================

extern xbool g_game_running;
extern xbool g_first_person;

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct trigger_spatial_viewable_desc : public object_desc
{
    trigger_spatial_viewable_desc( void ) : object_desc( 
        object::TYPE_VIEWABLE_SPATIAL_TRIGGER, 
        "Viewable Spatial Trigger", 
        "SCRIPT",
        object::ATTR_SPACIAL_ENTRY		|
        object::ATTR_COLLISION_PERMEABLE,
        FLAGS_IS_DYNAMIC ) {}

    //-------------------------------------------------------------------------
    
    virtual object* Create          ( void )
    {
        return new trigger_spatial_viewable;
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        draw_Label( Object.GetPosition(), XCOLOR_RED, "<<OBSOLETE>>" );    
        return EDITOR_ICON_TRIGGER;
    }

#endif // X_EDITOR

} s_TriggerSpatialViewableObjectDesc;

//=========================================================================

const object_desc&  trigger_spatial_viewable::GetTypeDesc( void ) const
{
    return s_TriggerSpatialViewableObjectDesc;
}

//=========================================================================

const object_desc&  trigger_spatial_viewable::GetObjectType( void )
{
    return s_TriggerSpatialViewableObjectDesc;
}

//=========================================================================

//=========================================================================
// trigger_spatial_viewable
//=========================================================================

trigger_spatial_viewable::trigger_spatial_viewable(void) : trigger_spatial_object()
{
}

//=========================================================================

trigger_spatial_viewable::~trigger_spatial_viewable(void)
{}

//=============================================================================

void trigger_spatial_viewable::OnInit ( void )
{
    trigger_spatial_object::OnInit();

    m_ActivationType = ACTIVATE_ON_BY_OVERRIDE_TYPE;
}

//=============================================================================

void trigger_spatial_viewable::OnKill ( void )
{
    trigger_spatial_object::OnKill();
}

//=============================================================================

#ifndef X_RETAIL
void trigger_spatial_viewable::OnDebugRender( void )
{
    if (m_OnActivate)
    {   
        if (g_game_running && g_first_person)
            ActivateTrigger();
    }

    trigger_spatial_object::OnDebugRender();
}
#endif // X_RETAIL

//=============================================================================

void trigger_spatial_viewable::ExecuteLogic ( f32 DeltaTime )
{    
    TRIGGER_CONTEXT("trigger_spatial_viewable::ExecuteLogic");

    trigger_spatial_object::ExecuteLogic( DeltaTime );
}

//=============================================================================

void  trigger_spatial_viewable::OnEnumProp ( prop_enum& rPropList )
{   
    trigger_spatial_object::OnEnumProp( rPropList ); 
    
    rPropList.AddHeader  ( "Spatial Viewable Trigger", "A special trigger which can be activated on viewing.", PROP_TYPE_HEADER);
}

//=============================================================================

xbool trigger_spatial_viewable::OnProperty ( prop_query& rPropQuery )
{  
    if( trigger_spatial_object::OnProperty( rPropQuery ) )
        return TRUE;
   
    return FALSE;
}

//=============================================================================

void trigger_spatial_viewable::ActivateTrigger( void )
{
    trigger_spatial_object::ActivateTrigger( );
}


