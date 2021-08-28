///////////////////////////////////////////////////////////////////////////
//
//  trigger_meta_delay.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "trigger_meta_delay.hpp"
#include "..\TriggerEx_Object.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

trigger_meta_delay::trigger_meta_delay ( guid ParentGuid ) : trigger_meta_base( ParentGuid ),
m_DelayTime(1.0f),
m_TimeDelayingSoFar(0.0f)
{
}

//=============================================================================

void trigger_meta_delay::OnActivate ( xbool Flag )
{
    (void)Flag;

    m_TimeDelayingSoFar = 0.0f;
}

//=============================================================================

xbool trigger_meta_delay::Execute ( f32 DeltaTime )
{
    m_TimeDelayingSoFar += DeltaTime;

    if (m_TimeDelayingSoFar > m_DelayTime)
    {
        //done with delay
        m_TimeDelayingSoFar = 0.0f;
        return TRUE;
    }
    else
    {
        //delay some more
        return FALSE;
    }
}

//=============================================================================

void trigger_meta_delay::OnEnumProp	( prop_enum& rPropList )
{
    rPropList.PropEnumFloat	 ( "Delay", "In Seconds, how much time to delay.", PROP_TYPE_MUST_ENUM );

    trigger_meta_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool trigger_meta_delay::OnProperty	( prop_query& rPropQuery )
{
    if( trigger_meta_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    if ( rPropQuery.VarFloat( "Delay" , m_DelayTime ) )
    {
       return TRUE; 
    }

    return FALSE;
}

//=============================================================================

const char* trigger_meta_delay::GetDescription( void )
{
    static big_string   Info;
    Info.Set(xfs("* DELAY %g seconds",m_DelayTime));
    return Info.Get();
}


