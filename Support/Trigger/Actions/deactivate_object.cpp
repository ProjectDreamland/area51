///////////////////////////////////////////////////////////////////////////
//
//  deactivate_object.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\deactivate_object.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"

static const xcolor s_DeactivateColor       (255,0,0);

//=========================================================================
// DEACTIVATE_OBJECT
//=========================================================================

deactivate_object::deactivate_object ( guid ParentGuid ) : actions_base( ParentGuid ),
m_ObjectGuid(NULL)
{
}

//=============================================================================

void deactivate_object::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * deactivate_object::Execute" );

    (void) pParent;

    //call the OnActivate(FALSE)
      
    if ( m_ObjectGuid == NULL )
        return;

    object_ptr<object>  ObjectPtr( m_ObjectGuid );
    
    if ( !ObjectPtr.IsValid() )
    {
        return;
    }
    
    ObjectPtr.m_pObject->OnActivate(FALSE);
}

//=============================================================================

void deactivate_object::OnRender ( void )
{
    object_ptr<object> ObjectPtr(m_ObjectGuid);
    
    if ( !ObjectPtr.IsValid() )
        return;

#ifdef TARGET_PC
    vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ObjectPtr.m_pObject->GetPosition(), s_DeactivateColor );
    draw_BBox( ObjectPtr.m_pObject->GetBBox(), s_DeactivateColor );
    draw_Label( ObjectPtr.m_pObject->GetPosition(), s_DeactivateColor, GetTypeName() );
#endif
}

//=============================================================================

void deactivate_object::OnEnumProp ( prop_enum& rPropList )
{
    //object info
    rPropList.AddGuid ( "Object Guid" ,  "GUID of the object to deactivate." );
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool deactivate_object::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID ( "Object Guid"  , m_ObjectGuid ) )
        return TRUE;
    
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}
