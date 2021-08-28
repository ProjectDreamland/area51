///////////////////////////////////////////////////////////////////////////////
//
//  set_actor_friends.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_SET_ACTOR_FRIENDS_
#define _TRIGGER_ACTIONS_SET_ACTOR_FRIENDS_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// SET_ACTOR_FRIENDS : sets an actor's friends
//=========================================================================

class set_actor_friends : public actions_base
{
public:
                    set_actor_friends                ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Set Friends"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Sets the actors friends"; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
    
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_SET_ACTOR_FRIENDS;}

protected:

    guid            m_ActorGuid;
    u32             m_TriggeredFriends;
};

#endif
