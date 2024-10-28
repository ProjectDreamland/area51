///////////////////////////////////////////////////////////////////////////////
//
//  condition_check_focus_object.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_check_focus_object.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "..\Support\Objects\focusobject.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

condition_check_focus_object::condition_check_focus_object( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_FocusObj(0)
{
}

//=============================================================================

xbool condition_check_focus_object::Execute( guid TriggerGuid )
{    
    (void) TriggerGuid;

    object* pObject = g_ObjMgr.GetObjectByGuid(m_FocusObj);
    if (pObject && pObject->IsKindOf(focus_object::GetRTTI()) )
    {
        focus_object *pFocus = (focus_object*)pObject;
        if (pFocus->HasFocus())
        {
            return TRUE;
        }
    }
    
    return FALSE; 
}    

//=============================================================================

void condition_check_focus_object::OnEnumProp ( prop_enum& rPropList )
{ 
    rPropList.PropEnumGuid ( "Focus Object", "Focus Object on which we intend to check state" , 0 );

    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_check_focus_object::OnProperty ( prop_query& rPropQuery )
{
    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.VarGUID ( "Focus Object"  , m_FocusObj ) )
        return TRUE;
    
    return FALSE;
}

//=============================================================================

#ifndef X_RETAIL
void condition_check_focus_object::OnDebugRender ( s32 Index )
{
    object* pObject = g_ObjMgr.GetObjectByGuid(m_FocusObj);
    if (pObject)
    {
        draw_BBox( pObject->GetBBox(), XCOLOR_PURPLE );

        if (!GetElse())
        {
            draw_Label( pObject->GetPosition(), XCOLOR_PURPLE, xfs("[If %d]Check Focus", Index) );
        }
        else
        {
            draw_Label( pObject->GetPosition(), XCOLOR_PURPLE, xfs("[Else If %d]Check Focus", Index) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

const char* condition_check_focus_object::GetDescription( void )
{
    static big_string   Info;

    object* pObject = g_ObjMgr.GetObjectByGuid(m_FocusObj);
    if (pObject && pObject->IsKindOf(focus_object::GetRTTI()) )
    {
        Info.Set("Check Focus Object");
    }
    else
    {
        Info.Set("Check Invalid Focus Object");
    }

    return Info.Get();
}
