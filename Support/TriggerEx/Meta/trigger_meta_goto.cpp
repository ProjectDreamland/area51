///////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_goto.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "trigger_meta_goto.hpp"
#include "..\TriggerEx_Object.hpp"
#include "Dictionary\global_dictionary.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

trigger_meta_goto::trigger_meta_goto ( guid ParentGuid ) : trigger_meta_base( ParentGuid ),
m_ConditionAffecter(FALSE, 2),
m_Label(-1)
{
}

//=============================================================================

xbool trigger_meta_goto::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    if (m_Label != -1)
    {
        if (m_ConditionAffecter.EvaluateConditions( m_TriggerGuid ) == conditional_affecter::EVAL_COMMIT_IF)
        {
            if (!SetTriggerActionIndexToLabel( g_StringMgr.GetString(m_Label) ))
            {
                m_bErrorInExecute = TRUE;
            }
            //return false so we don't update the action index ptr
            return FALSE;
        }
    }
    
    return TRUE;
}

//=============================================================================

void trigger_meta_goto::OnEnumProp	( prop_enum& rPropList )
{
#ifdef X_EDITOR
    m_ConditionAffecter.EnumPropSelector( rPropList );
#endif // X_EDITOR

    //order is important here
    m_ConditionAffecter.EnumPropConditionData( rPropList );
    m_ConditionAffecter.EnumPropConditions( rPropList );

    object* pObject = NULL;            
    if ( SMP_UTIL_IsGuidOfType ( &pObject, m_TriggerGuid , trigger_ex_object::GetRTTI() ) == TRUE )
    {
        trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
        char LabelList[256];
        pTrigger->GetValidLabels(GetElse(), LabelList);
        rPropList.PropEnumEnum( "Go To Label", LabelList, "Name of the label to jump to.", PROP_TYPE_MUST_ENUM );
    }


    trigger_meta_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool trigger_meta_goto::OnProperty	( prop_query& rPropQuery )
{
    if( trigger_meta_base::OnProperty( rPropQuery ) )
        return TRUE;

    //for backwards compatibility...
    if ( rPropQuery.IsVar  ( "Goto Label" ))
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_Label >= 0 )
                rPropQuery.SetVarString( g_StringMgr.GetString(m_Label), 256 );
            else
                rPropQuery.SetVarString("", 256);
            return TRUE;
        }
        else
        {
            if (x_strlen(rPropQuery.GetVarString()) > 0)
            {
                m_Label = g_StringMgr.Add( rPropQuery.GetVarString() );
                return TRUE;
            }
        }
    }
    
    if ( rPropQuery.IsVar  ( "Go To Label" ))
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_Label >= 0 )
                rPropQuery.SetVarEnum( g_StringMgr.GetString(m_Label) );
            else
                rPropQuery.SetVarEnum( "" );
            return TRUE;
        }
        else
        {
            if (x_strlen(rPropQuery.GetVarEnum()) > 0)
            {
                m_Label = g_StringMgr.Add( rPropQuery.GetVarEnum() );
                return TRUE;
            }
        }
    }

#ifdef X_EDITOR
    if( m_ConditionAffecter.OnPropertySelector( rPropQuery ) )
        return TRUE;
#endif // X_EDITOR

    if ( m_ConditionAffecter.OnPropertyConditionData( rPropQuery ) )
        return TRUE;

    if ( m_ConditionAffecter.OnPropertyConditions( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* trigger_meta_goto::GetDescription( void )
{
    static big_string   Info;

    if (m_Label == -1)
    {
        Info.Set("* GOTO Undefined");
    }
    else if (m_ConditionAffecter.GetConditionCount() > 0)
    {
        Info.Set(xfs("* GOTO %s on CONDITION",g_StringMgr.GetString(m_Label) ) );
    }
    else
    {
        Info.Set(xfs("* GOTO %s",g_StringMgr.GetString(m_Label) ) );
    }

    return Info.Get();
}

//=========================================================================

xbool trigger_meta_goto::SetTriggerActionIndexToLabel( const char* pName )
{
    ASSERT( m_TriggerGuid );    
    object* pObject = NULL;            
    if ( SMP_UTIL_IsGuidOfType ( &pObject, m_TriggerGuid , trigger_ex_object::GetRTTI() ) == TRUE )
    {
        trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
        return pTrigger->SetTriggerActionIndexToLabel(pName);
    }
    
    return FALSE;
}


