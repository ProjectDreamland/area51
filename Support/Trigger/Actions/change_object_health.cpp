///////////////////////////////////////////////////////////////////////////
//
//  change_object_health.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\change_object_health.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Objects\Pain.hpp"

#include "Entropy.hpp"


static const xcolor s_ActivateColor         (255,0,0);

//=========================================================================
// change_object_health
//=========================================================================

change_object_health::change_object_health ( guid ParentGuid ) : actions_base( ParentGuid ),
m_Health(0.0f),
m_ObjectGuid(NULL)
{}

//=============================================================================

void change_object_health::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * change_object_health::Execute" );

    (void) pParent;
    
    ASSERT(pParent);

    object_ptr<object>  ObjectPtr( m_ObjectGuid );
    
    if ( !ObjectPtr.IsValid() )
    {
        return;
    }
    
    pain PainEvent;

    PainEvent.Type      = pain::PAIN_ON_TRIGGER;
    PainEvent.Center    = ObjectPtr.m_pObject->GetPosition();
    PainEvent.Origin    = pParent->GetGuid();
    PainEvent.PtOfImpact= ObjectPtr.m_pObject->GetPosition();

    PainEvent.DamageR0  = -m_Health; 
    PainEvent.DamageR1  = -m_Health; 

    PainEvent.RadiusR0  = 100.0f ;
    PainEvent.RadiusR1  = 100.0f ;

    ObjectPtr.m_pObject->OnPain( PainEvent );
}

//=============================================================================

void change_object_health::OnRender ( void )
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

void change_object_health::OnEnumProp	( prop_enum& rPropList )
{    
    rPropList.AddFloat ( "Health Amount" ,  "Amount of health to change by." );
   
    rPropList.AddGuid ( "Object Guid"    ,  "GUID of the object to activate." );

    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool change_object_health::OnProperty	( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID ( "Object Guid"  , m_ObjectGuid ) )
        return TRUE;
     
    if ( rPropQuery.VarFloat ( "Health Amount"  , m_Health ) )
        return TRUE;
    
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}


