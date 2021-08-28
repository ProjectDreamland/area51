///////////////////////////////////////////////////////////////////////////////
//
//  action_object_activation.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_object_activation_
#define _action_object_activation_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// Check Property
//=========================================================================

class action_object_activation : public actions_ex_base
{
public:
                    action_object_activation                ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_OBJECT_ACTIVATION;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Activate or Deactivate an object."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*        GetObjectRef0   ( xstring& Desc ) { Desc = "Object error: "; return &m_ObjectAffecter; }
#endif

#ifndef X_RETAIL
    virtual         void                    OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:
    
    enum activate_codes
    { 
        INVALID_ACTIVATE_CODES = -1,
        CODE_ACTIVATE,
        CODE_DEACTIVATE,
        ACTIVATE_CODES_END
    };

    s32              m_ActivateCode;      
    object_affecter  m_ObjectAffecter;
};

#endif
