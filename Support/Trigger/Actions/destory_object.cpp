///////////////////////////////////////////////////////////////////////////
//
//  destory_object.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\destory_object.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"

static const xcolor s_DestroyColor          (255,255,255);

//=========================================================================
// DESTORY_OBJECT
//=========================================================================

destory_object::destory_object ( guid ParentGuid ) : actions_base( ParentGuid ),
m_ObjectGuid(NULL)
{
}

//=============================================================================

void destory_object::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * destory_object::Execute" );

    (void) pParent;

    object* pObject = g_ObjMgr.GetObjectByGuid( m_ObjectGuid );
    if( pObject == NULL )
        return;

    if (m_ObjectGuid == m_ParentGuid)
    {
        ASSERT( 0 );
        x_DebugMsg( "Cannot use Destroy object action upon the trigger which contains this action. Use Kill This Trigger action instead.");
        return;
    }

    ASSERT( m_ObjectGuid == pObject->GetGuid() );

    g_ObjMgr.DestroyObject( pObject->GetGuid() );
    m_ObjectGuid = NULL;
}

//=============================================================================

void destory_object::OnRender ( void )
{
    object_ptr<object> ObjectPtr(m_ObjectGuid);
    
    if ( !ObjectPtr.IsValid() )
        return;

#ifdef TARGET_PC
    vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ObjectPtr.m_pObject->GetPosition(), s_DestroyColor );
    draw_BBox( ObjectPtr.m_pObject->GetBBox(), s_DestroyColor );
    draw_Label( ObjectPtr.m_pObject->GetPosition(), s_DestroyColor, GetTypeName() );
#endif
}

//=============================================================================

void destory_object::OnEnumProp ( prop_enum& rPropList )
{
    //object info
    rPropList.AddGuid ( "Object Guid" , "GUID of the object to be destroyed." );
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool destory_object::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID ( "Object Guid"  , m_ObjectGuid ) )
        return TRUE;
    
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
    
    return FALSE;
}