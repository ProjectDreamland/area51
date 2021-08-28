///////////////////////////////////////////////////////////////////////////////
//
//  set_actor_faction.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_SET_ACTOR_FACTION_
#define _TRIGGER_ACTIONS_SET_ACTOR_FACTION_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// SET_ACTOR_FACTION : sets an actor's friends
//=========================================================================

class set_actor_faction : public actions_base
{
public:
                    set_actor_faction                ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Set Faction"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Sets the actors faction."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
    
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_SET_ACTOR_FACTION;}

protected:

    guid            m_ActorGuid;
    factions        m_TriggeredFaction;
};

#endif
