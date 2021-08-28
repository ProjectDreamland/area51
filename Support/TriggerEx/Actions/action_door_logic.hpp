///////////////////////////////////////////////////////////////////////////////
//
//  action_door_logic.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_door_logic_
#define _action_door_logic_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"
#include "..\Support\Objects\Door.hpp "

//=========================================================================
// Check Property
//=========================================================================

class action_door_logic : public actions_ex_base
{
public:
                    action_door_logic                       ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_DOOR_LOGIC;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Affect a Door."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*        GetObjectRef0   ( xstring& Desc ) { Desc = "Door object error: "; return &m_ObjectAffecter; }
#endif

#ifndef X_RETAIL
    virtual         void                    OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:

    object_affecter  m_ObjectAffecter;
    door::state      m_State;
    xbool            m_Logic;
};

#endif
