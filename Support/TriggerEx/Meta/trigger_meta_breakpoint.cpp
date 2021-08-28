///////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_breakpoint.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "trigger_meta_breakpoint.hpp"
#include "..\TriggerEx_Object.hpp"

#ifdef X_EDITOR
extern g_EditorBreakpoint;
#endif // X_EDITOR

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

trigger_meta_breakpoint::trigger_meta_breakpoint ( guid ParentGuid ) : trigger_meta_base( ParentGuid ),
m_ConditionAffecter(FALSE, 2)
{
}

//=============================================================================

xbool trigger_meta_breakpoint::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

#ifdef X_EDITOR
    if (m_ConditionAffecter.EvaluateConditions( m_TriggerGuid ) == conditional_affecter::EVAL_COMMIT_IF)
    {
        g_EditorBreakpoint = TRUE;
    }
#endif // X_EDITOR
    
    return TRUE;
}

//=============================================================================

void trigger_meta_breakpoint::OnEnumProp	( prop_enum& rPropList )
{
#ifdef X_EDITOR
    m_ConditionAffecter.EnumPropSelector( rPropList );
#endif // X_EDITOR

    //order is important here
    m_ConditionAffecter.EnumPropConditionData( rPropList );
    m_ConditionAffecter.EnumPropConditions( rPropList );

    trigger_meta_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool trigger_meta_breakpoint::OnProperty	( prop_query& rPropQuery )
{
    if( trigger_meta_base::OnProperty( rPropQuery ) )
        return TRUE;
  
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

const char* trigger_meta_breakpoint::GetDescription( void )
{
    static big_string   Info;
    if (m_ConditionAffecter.GetConditionCount() > 0)
    {
        Info.Set("* BREAK if CONDITION");
    }
    else
    {
        Info.Set("* BREAK ALWAYS");
    }
    return Info.Get();
}



