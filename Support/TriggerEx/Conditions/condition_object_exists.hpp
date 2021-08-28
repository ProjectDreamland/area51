///////////////////////////////////////////////////////////////////////////////
//
//  condition_object_exists.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_object_exists_
#define _condition_object_exists_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Conditionals.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// Check Property
//=========================================================================

class condition_object_exists : public conditional_ex_base
{
public:
                    condition_object_exists                 ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_OBJECT_EXISTS;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Check whether or not a specified object exists."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

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
        CODE_DOES_OBJECT_EXIST,
        CODE_DOES_OBJECT_NOT_EXIST,
        CHECK_CODES_END
    };

    s32              m_CheckCode;        // Used to determine what type of conditional check to perform
    object_affecter  m_Affecter;
};

#endif
