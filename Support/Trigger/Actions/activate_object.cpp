///////////////////////////////////////////////////////////////////////////
//
//  activate_object.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\Activate_Object.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"

static const xcolor s_ActivateColor         (0,255,0);

//=========================================================================
// ACTIVATE_OBJECT
//=========================================================================

activate_object::activate_object ( guid ParentGuid ) : actions_base( ParentGuid ),
m_ObjectGuid(NULL)
{
}

//=============================================================================

void activate_object::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * activate_object::Execute" );

    (void) pParent;

    object_ptr<object>  ObjectPtr( m_ObjectGuid );
    
    if ( !ObjectPtr.IsValid() )
    {
        return;
    }

    ObjectPtr.m_pObject->OnActivate(TRUE);
}

//=============================================================================

void activate_object::OnRender ( void )
{
    object_ptr<object> ObjectPtr( m_ObjectGuid );

    if ( !ObjectPtr.IsValid() )
        return;
#ifdef TARGET_PC
    vector3 MyPosition = GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ObjectPtr.m_pObject->GetPosition(), s_ActivateColor );
    draw_BBox( ObjectPtr.m_pObject->GetBBox(), s_ActivateColor );
    draw_Label( ObjectPtr.m_pObject->GetPosition(), s_ActivateColor, GetTypeName() );
#endif
}

//=============================================================================

void activate_object::OnEnumProp	( prop_enum& rPropList )
{
    //object info
    rPropList.AddGuid ( "Object Guid" , 
        "GUID of the object to activate." );
    
    rPropList.AddBool ( "Activate", "Activate if TRUE, Deactive if FALSE" );

    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool activate_object::OnProperty	( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID ( "Object Guid"  , m_ObjectGuid ) )
        return TRUE;

    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}


