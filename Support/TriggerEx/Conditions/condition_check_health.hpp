///////////////////////////////////////////////////////////////////////////////
//
//  condition_check_health.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_check_health_
#define _condition_check_health_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Conditionals.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// Check Health
//=========================================================================

class condition_check_health : public conditional_ex_base
{
public:
                    condition_check_health                 ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_CHECK_HEALTH;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Check whether or not a specified object is alive or dead."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& List );
    virtual			xbool	                OnProperty	    ( prop_query& I );

#ifdef X_EDITOR
    virtual         object_affecter*        GetObjectRef0   ( xstring& Desc ) { Desc = "Object error: "; return &m_Affecter; }
#endif

#ifndef X_RETAIL
    virtual         void                    OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:
    
    enum check_codes
    { 
        INVALID_CHECK_CODES = -1,
        CODE_IS_OBJECT_DEAD,
        CODE_IS_OBJECT_ALIVE,
        CHECK_CODES_END
    };

    s32              m_CheckCode;        // Used to determine what type of conditional check to perform
    object_affecter  m_Affecter;
};

#endif
