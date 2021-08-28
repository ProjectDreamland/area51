///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_AI_MODIFY_BEHAVIOR_TARGETED
#define _TRIGGER_ACTIONS_AI_MODIFY_BEHAVIOR_TARGETED

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"
#include "..\Support\Characters\Character.hpp"

//=========================================================================
// AI_MODIFY_BEHAVIOR_TARGETED : modifies a specific AI behavior upon entering a spatial trigger
//=========================================================================

class ai_modify_behavior_targeted : public actions_base
{
public:
                    ai_modify_behavior_targeted        ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Modify AI Behavior Targeted"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Modifies a particualr AI behavior when it enters a volume."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_AI_MODIFY_BEHAVIOR_TARGETED;}

protected:
 
    guid        m_AIGuid;                   //AI GUID
    character_state::states      m_DesiredBehavior;          //Desired behavior for the AI to be
};

#endif
