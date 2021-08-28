///////////////////////////////////////////////////////////////////////////////
//
//  condition_random_chance.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_random_chance_
#define _condition_random_chance_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "..\TriggerEx_Conditionals.hpp"

//=========================================================================
// Check Property
//=========================================================================

class condition_random_chance : public conditional_ex_base
{
public:
                    condition_random_chance                ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_RANDOM_CHANCE;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Will return TRUE based upon a random chance."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

protected:

    f32             m_RandomPercent;        // Random Chance of a True event...
    
};

#endif
