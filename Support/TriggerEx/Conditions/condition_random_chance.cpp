///////////////////////////////////////////////////////////////////////////////
//
//  condition_random_chance.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_random_chance.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

condition_random_chance::condition_random_chance( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_RandomPercent(0.0f)
{
}

//=============================================================================

xbool condition_random_chance::Execute( guid TriggerGuid )
{    
    (void) TriggerGuid;

    return ( x_frand(0.0f,100.0f) < m_RandomPercent );
}    

//=============================================================================

void condition_random_chance::OnEnumProp ( prop_enum& rPropList )
{ 
    rPropList.PropEnumFloat ("Random Percent" ,"The random chance of this condtion being true ( 0.0 - 100.0 ).", PROP_TYPE_MUST_ENUM );

    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_random_chance::OnProperty ( prop_query& rPropQuery )
{
    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.VarFloat ( "Random Percent" , m_RandomPercent, 0.0f, 100.0f ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* condition_random_chance::GetDescription( void )
{
    static big_string   Info;
    
    Info.Set(xfs("Random %g%% of Success", m_RandomPercent));

    return Info.Get();
}
