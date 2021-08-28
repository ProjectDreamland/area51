///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_attack_guid.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_AI_NAV_ACTIVATION__
#define __ACTION_AI_NAV_ACTIVATION__

//=========================================================================
// INCLUDES
//=========================================================================

#include "action_ai_base.hpp"

//=========================== ==============================================
// action_ai_attack_guid
//=========================================================================

class action_ai_nav_activation : public actions_ex_base
{
public:
                    action_ai_nav_activation           ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_AI_NAV_ACTIVATION;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Activate or Deactivate a nav connection."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         guid*                   GetGuidRef0     ( xstring& Desc ) { Desc = "NavConnection object error: "; return &m_NavConnectionGuid; }
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
    guid             m_NavConnectionGuid;
};

#endif
