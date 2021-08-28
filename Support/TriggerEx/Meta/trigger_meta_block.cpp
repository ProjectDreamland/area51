///////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_block.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "trigger_meta_block.hpp"
#include "..\TriggerEx_Object.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

trigger_meta_block::trigger_meta_block ( guid ParentGuid ) : trigger_meta_base( ParentGuid ),
m_ConditionAffecter(FALSE, 2)
{
}

//=============================================================================

xbool trigger_meta_block::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    if (m_ConditionAffecter.EvaluateConditions( m_TriggerGuid ) == conditional_affecter::EVAL_COMMIT_IF)
    {
        return TRUE;
    }
    return FALSE;
}

//=============================================================================

void trigger_meta_block::OnEnumProp	( prop_enum& rPropList )
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

xbool trigger_meta_block::OnProperty	( prop_query& rPropQuery )
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

const char* trigger_meta_block::GetDescription( void )
{
    static big_string   Info;
    if (m_ConditionAffecter.GetConditionCount() > 0)
    {
        Info.Set("* BLOCK until CONDITION");
    }
    else
    {
        Info.Set("* BLOCK for 1 frame");
    }
    return Info.Get();
}



