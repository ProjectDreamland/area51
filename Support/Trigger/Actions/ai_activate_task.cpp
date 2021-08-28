///////////////////////////////////////////////////////////////////////////
//
//  ai_activate_task.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\ai_activate_task.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Characters\Character.hpp"
#include "..\Support\Characters\TaskSystem\character_task_set.hpp"

#include "Entropy.hpp"

//=========================================================================
// ai_activate_task
//=========================================================================

ai_activate_task::ai_activate_task ( guid ParentGuid ) : actions_base( ParentGuid )
{
}

//=========================================================================

void ai_activate_task::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * ai_activate_task::Execute" );
    
    (void) pParent;

    ASSERT( pParent );

    guid Activator;
    
    const guid* pGuidActor = pParent->GetTriggerActor();

    if (pGuidActor)
    {
        Activator = guid(pGuidActor->Guid);
    }

    object* pObject = g_ObjMgr.GetObjectByGuid(m_TaskGuid);
    if ( pObject && pObject->IsKindOf( character_task_set::GetRTTI() ) == TRUE )
    {
        //heres the task object
        character_task_set* pTask = (character_task_set*)pObject;
        pTask->ActivateTask( );
    }
}

//=========================================================================

void ai_activate_task::OnRender ( void )
{
    object_ptr<object> ObjectPtr(m_TaskGuid);
    
    if ( ObjectPtr.IsValid() )
    {
        vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
        draw_Line( MyPosition, ObjectPtr.m_pObject->GetBBox().GetCenter(), XCOLOR_PURPLE );
        draw_BBox( ObjectPtr.m_pObject->GetBBox(), XCOLOR_PURPLE );
        draw_Label( ObjectPtr.m_pObject->GetPosition(), XCOLOR_WHITE, GetTypeName() );
    }
}

//=========================================================================

void ai_activate_task::OnEnumProp ( prop_enum& List )
{   
    List.AddGuid ( "Task" , "Task to Activate (MUST point to a character task set." );

    actions_base::OnEnumProp( List );
}

//=========================================================================

xbool ai_activate_task::OnProperty ( prop_query& I )
{
    if (I.VarGUID("Task", m_TaskGuid))
    {
//REMOVING: THIS HAS CAUSED SOME ISSUES WITH THE MULTIPLE CHECKOUTS BETWEEN
//          ARTISTS AND DESIGNERS
/*               
        if (I.IsRead())
        {
            object* pObject = g_ObjMgr.GetObjectByGuid(m_TaskGuid);
            if ( !pObject || !pObject->IsKindOf( character_task_set::GetRTTI() ) == TRUE )
            {
                //needs to be a task set
                x_DebugMsg("This trigger can only be applied to task sets.\n");               
                m_TaskGuid = 0;
            }
        }
*/
        return TRUE;
    }
    
    if( actions_base::OnProperty( I ) )
        return TRUE;
    
    return FALSE;
}

