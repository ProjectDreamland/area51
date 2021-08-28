///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_AI_MODIFY_BEHAVIOR
#define _TRIGGER_ACTIONS_AI_MODIFY_BEHAVIOR

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"
#include "..\Support\Characters\Character.hpp"

//=========================================================================
// AI_MODIFY_BEHAVIOR : modifes an AI behavior upon entering the spatial trigger
//=========================================================================

class ai_modify_behavior : public actions_base
{
public:
                    ai_modify_behavior                 ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Modify AI Behavior"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Modifies an AI behavior upon entering a volume."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
 
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_AI_MODIFY_BEHAVIOR;}

protected:

    character_state::states m_DesiredBehavior;           //Desired behavior for the AI to be
};

#endif
