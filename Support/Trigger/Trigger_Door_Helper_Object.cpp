//=========================================================================
//
//  Trigger_Door_Helper_Object.cpp
//
//=========================================================================

//=========================================================================
//  INCLUDES
//=========================================================================

#include "Trigger\Trigger_Door_Helper_Object.hpp"
#include "Trigger\Trigger_Manager.hpp"
#include "Entropy.hpp"

#include "Objects\Door.hpp"

//=========================================================================
// STATIC DEFINITIONS AND CONSTS
//=========================================================================

//=========================================================================

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct trigger_door_helper_object_desc : public object_desc
{
    trigger_door_helper_object_desc( void ) : object_desc( 
        object::TYPE_SPATIAL_TRIGGER, 
        "Door Helper Trigger", 
        "SCRIPT",
        object::ATTR_SPACIAL_ENTRY		|
        object::ATTR_COLLISION_PERMEABLE,
        0) {}
    
    //=========================================================================
    
    virtual object* Create          ( void )
    {
        return new trigger_door_helper_object;
    }

} s_TriggerDoorHelperlObjectDesc;

//=========================================================================

const object_desc&  trigger_door_helper_object::GetTypeDesc( void ) const
{
    return s_TriggerDoorHelperlObjectDesc;
}

//=========================================================================

const object_desc&  trigger_door_helper_object::GetObjectType( void )
{
    return s_TriggerDoorHelperlObjectDesc;
}

//=========================================================================

//=========================================================================
// trigger_door_helper_object
//=========================================================================

trigger_door_helper_object::trigger_door_helper_object(void) : trigger_spatial_object()
{
}

//=========================================================================

trigger_door_helper_object::~trigger_door_helper_object(void)
{}

//=============================================================================

void trigger_door_helper_object::OnInit ( void )
{
    trigger_spatial_object::OnInit();
     
    //starts in sleeping state...
    SetTriggerState(STATE_SLEEPING);

    //set the m_OnActivate flag to FALSE, this object is only activated one setup is called...
    m_OnActivate = FALSE;
}

//=============================================================================

void trigger_door_helper_object::OnKill ( void )
{
    trigger_spatial_object::OnKill();
}

//=============================================================================

void trigger_door_helper_object::Render ( u32 ParentAttribs )
{
    OnRender();
    if( ParentAttribs & ATTR_EDITOR_SELECTED )
    {
#ifdef TARGET_PC
        draw_BBox ( GetBBox(), XCOLOR_BLUE);
#endif
    }
}

//=============================================================================

void trigger_door_helper_object::Setup( const bbox& BBox, const guid& DoorGuid )
{
    vector3 Size = BBox.GetSize();

    //setup the base classes..
    f32 Dimension[MAX_DIMENSIONS_PARAMS] = { Size.X, Size.Y, Size.Z };
    
    trigger_spatial_object::Setup( ACTIVATE_ON_NPC_OR_PLAYER, 
                                   TYPE_AXIS_CUBE,
                                   RESPONSE_RETURN_TO_SLEEP,
                                   Dimension );

    m_OnActivate = TRUE;
    m_Type       = TRIGGER_REPEATING;
    m_DoorGuid   = DoorGuid;

    OnMove(BBox.GetCenter());
}

//=============================================================================

void trigger_door_helper_object::Sync( const bbox& BBox )
{
    vector3 Size = BBox.GetSize();
    
    f32 Dimension[MAX_DIMENSIONS_PARAMS] = { Size.X, Size.Y, Size.Z };
    
    trigger_spatial_object::Setup( Dimension );

    OnMove(BBox.GetCenter());
}

//=============================================================================

void trigger_door_helper_object::ExecuteLogic ( f32 DeltaTime )
{    
    TRIGGER_CONTEXT("trigger_door_helper_object::ExecuteLogic");

    trigger_spatial_object::ExecuteLogic( DeltaTime );
}

//=============================================================================

void  trigger_door_helper_object::OnEnumProp ( prop_enum& rPropList )
{   
    trigger_spatial_object::OnEnumProp( rPropList );
}

//=============================================================================

xbool trigger_door_helper_object::OnProperty ( prop_query& rPropQuery )
{  
    if( trigger_spatial_object::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

void trigger_door_helper_object::ActivateTrigger( void )
{
    trigger_spatial_object::ActivateTrigger( );

    object_ptr<door>    ObjectPtr( m_DoorGuid );

    if (!ObjectPtr.IsValid())
        return;

//    ObjectPtr.m_pObject->WakeUp();
}