///////////////////////////////////////////////////////////////////////////////
//
//  condition_check_focus_object.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_check_focus_object_
#define _condition_check_focus_object_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Conditionals.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// Check Property
//=========================================================================

class condition_check_focus_object : public conditional_ex_base
{
public:
                    condition_check_focus_object                 ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_CHECK_FOCUS_OBJECT;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Check if a focus object is focal rendering."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifndef X_RETAIL
    virtual         void                    OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:

    guid  m_FocusObj;
};

#endif
