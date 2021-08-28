///////////////////////////////////////////////////////////////////////////////
//
//  action_portal_activate.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __action_portal_activate__
#define __action_portal_activate__

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================== ==============================================
// action_object_move
//=========================================================================

class action_portal_activate : public actions_ex_base
{
public:
                    action_portal_activate              ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_PORTAL_ACTIVATE;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Activate or deactivate a portal."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         guid*               GetGuidRef0     ( xstring& Desc ) { Desc = "Portal object error: "; return &m_PortalGuid; }
#endif

#ifndef X_RETAIL
    virtual         void                OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:

    enum activate_codes
    { 
        INVALID_ACTIVATE_CODES = -1,
        CODE_ACTIVATE,
        CODE_DEACTIVATE,
        ACTIVATE_CODES_END
    };

    s32         m_ActivateCode;      
    guid        m_PortalGuid;
};


#endif
