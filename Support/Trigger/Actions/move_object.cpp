///////////////////////////////////////////////////////////////////////////
//
//  move_object.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\move_object.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"

static const xcolor s_MoveColor             (255,0,255);

//=========================================================================
// MOVE_OBJECT
//=========================================================================

move_object::move_object ( guid ParentGuid ) : actions_base( ParentGuid ),
m_ObjectGuid(NULL),
m_Position(0.0f,0.0f,0.0f),
m_Orientation(0.0f,0.0f,0.0f)
{
}

//=============================================================================

void move_object::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * move_object::Execute" );

    (void) pParent;

    object_ptr<object>  ObjectPtr( m_ObjectGuid );
    
    if ( !ObjectPtr.IsValid() )
    {
        return;
    }

    matrix4 M;
    
    M.Setup( vector3(1.0f,1.0f,1.0f), m_Orientation, m_Position );

    //some objects require zones set first, some after, doing both
    ObjectPtr.m_pObject->SetZone1( pParent->GetZone1() );
    ObjectPtr.m_pObject->SetZone2( pParent->GetZone2() );   
    ObjectPtr.m_pObject->OnTriggerTransform( M );
    ObjectPtr.m_pObject->SetZone1( pParent->GetZone1() );
    ObjectPtr.m_pObject->SetZone2( pParent->GetZone2() );
}

//=============================================================================

void move_object::OnRender ( void )
{
    object_ptr<object> ObjectPtr(m_ObjectGuid);
    
    if ( !ObjectPtr.IsValid() )
        return;

#ifdef TARGET_PC
    vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ObjectPtr.m_pObject->GetPosition(), s_MoveColor );
    draw_BBox( ObjectPtr.m_pObject->GetBBox(), s_MoveColor );
    draw_Label( ObjectPtr.m_pObject->GetPosition(), s_MoveColor, GetTypeName() );
#endif
}

//=============================================================================

void move_object::OnEnumProp	( prop_enum& rPropList )
{
    //object info 
    rPropList.AddGuid ( "Object Guid" , "GUID of the object to be moved." );
    
    rPropList.AddVector3 ( "Position" , "Position of the object to move too." );
    
    rPropList.AddRotation ( "Orientation" , "Orientation of the object onced move." );
   
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool move_object::OnProperty	( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID ( "Object Guid"  , m_ObjectGuid ) )
        return TRUE;
    
    if ( rPropQuery.VarVector3 ( "Position"  , m_Position ) )
        return TRUE;
    
    if ( rPropQuery.VarRotation ( "Orientation"  , m_Orientation ) )
        return TRUE;
    
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}