///////////////////////////////////////////////////////////////////////////
//
//  open_and_lock_door.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\open_and_lock_door.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "Entropy.hpp"

static const xcolor s_OpenAndLockColor             (0,0,255);

//=========================================================================
// OPEN_AND_LOCK_DOOR
//=========================================================================

open_and_lock_door::open_and_lock_door ( guid ParentGuid ) : 
    actions_base( ParentGuid ),
    m_DoorObjectGuid(NULL)
{
}

//=============================================================================

void open_and_lock_door::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * move_object::Execute" );

    (void) pParent;

    object_ptr<door>  DoorPtr( m_DoorObjectGuid );
    
    if ( !DoorPtr.IsValid() )
    {
        return;
    }

//    DoorPtr.m_pObject->TriggerSetState( door::DOOR_OPENING_LOCKED );
}

//=============================================================================

void open_and_lock_door::OnRender ( void )
{
    object_ptr<object> ObjectPtr(m_DoorObjectGuid);
    
    if ( !ObjectPtr.IsValid() )
        return;

#ifdef TARGET_PC
    vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ObjectPtr.m_pObject->GetPosition(), s_OpenAndLockColor );
    draw_BBox( ObjectPtr.m_pObject->GetBBox(), s_OpenAndLockColor );
    draw_Label( ObjectPtr.m_pObject->GetPosition(), s_OpenAndLockColor, GetTypeName() );
#endif
}

//=============================================================================

void open_and_lock_door::OnEnumProp	( prop_enum& rPropList )
{
    //object info 
    rPropList.AddGuid ( "Door Guid" , "GUID of the door that you want to open and lock" );
    rPropList.AddEnum ( "Door\\Target State", 
                          "CLOSED\0LOCKED\0OPEN\0", 
                          "The target state for the door ");
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool open_and_lock_door::OnProperty	( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID ( "Door Guid"  , m_DoorObjectGuid ) )
        return TRUE;
    
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}