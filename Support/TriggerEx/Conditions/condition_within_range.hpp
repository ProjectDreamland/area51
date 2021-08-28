///////////////////////////////////////////////////////////////////////////////
//
//  condition_within_range.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_within_range_
#define _condition_within_range_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Conditionals.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// Check Property
//=========================================================================

class condition_within_range : public conditional_ex_base
{
public:
                    condition_within_range                 ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_WITHIN_RANGE;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Check whether or not an object is within range of another object."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*        GetObjectRef0   ( xstring& Desc ) { Desc = "Object1 error: "; return &m_Affecter1; }
    virtual         object_affecter*        GetObjectRef1   ( xstring& Desc ) { Desc = "Object2 error: "; return &m_Affecter2; }
#endif

#ifndef X_RETAIL
    virtual         void                    OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:
    
    enum check_codes
    { 
        INVALID_CHECK_CODES = -1,
        CODE_WITHIN_RANGE,
        CODE_OUT_OF_RANGE,
        CHECK_CODES_END
    };

    s32              m_CheckCode;        // Used to determine what type of conditional check to perform
    object_affecter  m_Affecter1;
    object_affecter  m_Affecter2;
    f32              m_fDistance;
};

#endif
