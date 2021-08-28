///////////////////////////////////////////////////////////////////////////////
//
//  condition_is_censored.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_is_censored_
#define _condition_is_censored_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "..\TriggerEx_Conditionals.hpp"

//=========================================================================
// Check Property
//=========================================================================

class condition_is_censored : public conditional_ex_base
{
public:
    condition_is_censored                   ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_IS_CENSORED;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Will return TRUE if build is censored version."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

protected:

    xbool             m_bBranchIfCensored;
};

#endif
