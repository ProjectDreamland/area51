///////////////////////////////////////////////////////////////////////////////
//
//  condition_is_censored.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_is_censored.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

extern xbool g_bCensoredBuild;

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

condition_is_censored::condition_is_censored( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_bBranchIfCensored(TRUE)
{
}

//=============================================================================

xbool condition_is_censored::Execute( guid TriggerGuid )
{    
    (void) TriggerGuid;

#ifdef X_EDITOR
    return ( g_bCensoredBuild == m_bBranchIfCensored );
#else
    return ( x_IsBuildCensored() == m_bBranchIfCensored ); 
#endif
}    

//=============================================================================

void condition_is_censored::OnEnumProp ( prop_enum& rPropList )
{ 
    rPropList.PropEnumBool ("Branch Condition" ,"Choose to branch on TRUE or FALSE", PROP_TYPE_MUST_ENUM );

    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_is_censored::OnProperty ( prop_query& rPropQuery )
{
    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.VarBool ( "Branch Condition" , m_bBranchIfCensored ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* condition_is_censored::GetDescription( void )
{
    static big_string   Info;

    if ( m_bBranchIfCensored )
        Info.Set("Content IS Censored");
    else
        Info.Set("Content IS NOT Censored");

    return Info.Get();
}
