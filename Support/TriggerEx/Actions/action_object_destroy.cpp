///////////////////////////////////////////////////////////////////////////////
//
//  action_object_destroy.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_object_destroy.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "..\TriggerEx_Object.hpp"

static const xcolor s_DestroyColor          (255,255,255);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_object_destroy::action_object_destroy( guid ParentGuid ):  actions_ex_base(  ParentGuid )
{
}

//=============================================================================

xbool action_object_destroy::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        if (pObject->GetGuid() == m_TriggerGuid)
        {
            trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
            pTrigger->KillTrigger();
            //return false so execution stops
            return FALSE;
        }

        g_ObjMgr.DestroyObject( pObject->GetGuid() );
    }

    return TRUE;
}    

//=============================================================================

#ifndef X_RETAIL
void action_object_destroy::OnDebugRender ( s32 Index )
{
    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        draw_Line( GetPositionOwner(), pObject->GetPosition(), s_DestroyColor );
        draw_BBox( pObject->GetBBox(), s_DestroyColor );

        if (!GetElse())
        {
            draw_Label( pObject->GetPosition(), s_DestroyColor, xfs("[%d]Destroy Object", Index) );
        }
        else
        {
            draw_Label( pObject->GetPosition(), s_DestroyColor, xfs("[Else %d]Destroy Object", Index) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

void action_object_destroy::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    m_ObjectAffecter.OnEnumProp( rPropList, "Object" );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_object_destroy::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_ObjectAffecter.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_object_destroy::GetDescription( void )
{
    static big_string   Info;
    Info.Set(xfs("Destroy %s", m_ObjectAffecter.GetObjectInfo()));          
    return Info.Get();
}
