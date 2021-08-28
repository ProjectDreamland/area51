///////////////////////////////////////////////////////////////////////////////
//
//  action_player_hud.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_player_hud_
#define _action_player_hud_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// Check Property
//=========================================================================

class action_player_hud : public actions_ex_base
{
public:
                    action_player_hud                ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_PLAYER_HUD;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Activate or Deactivate an object."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

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

    s32              m_ElementID;      
    xbool            m_DoPulse;
};

#endif
