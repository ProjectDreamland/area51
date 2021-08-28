///////////////////////////////////////////////////////////////////////////
//
//  return_door_to_normal.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\return_door_to_normal.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Objects\Door.hpp "
#include "Entropy.hpp"

static const xcolor s_OpenReturnToNormal             (0,128,0);

//=========================================================================
// RETURN_DOOR_TO_NORMAL
//=========================================================================

return_door_to_normal::return_door_to_normal ( guid ParentGuid ) : 
    actions_base( ParentGuid ),
    m_DoorObjectGuid(NULL)
{
}

//=============================================================================

void return_door_to_normal::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * move_object::Execute" );

    (void) pParent;

    object_ptr<door>  DoorPtr( m_DoorObjectGuid );
    
    if ( !DoorPtr.IsValid() )
    {
        return;
    }

//    DoorPtr.m_pObject->TriggerResetDoor();
}

//=============================================================================

void return_door_to_normal::OnRender ( void )
{
    object_ptr<object> ObjectPtr(m_DoorObjectGuid);
    
    if ( !ObjectPtr.IsValid() )
        return;

#ifdef TARGET_PC
    vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ObjectPtr.m_pObject->GetPosition(), s_OpenReturnToNormal );
    draw_BBox( ObjectPtr.m_pObject->GetBBox(), s_OpenReturnToNormal );
    draw_Label( ObjectPtr.m_pObject->GetPosition(), s_OpenReturnToNormal, GetTypeName() );
#endif
}

//=============================================================================

void return_door_to_normal::OnEnumProp	( prop_enum& rPropList )
{
    //object info 
    rPropList.AddGuid ( "Door Guid" , "GUID of the door that you want to open and lock" );
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool return_door_to_normal::OnProperty	( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID ( "Door Guid"  , m_DoorObjectGuid ) )
        return TRUE;
    
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}