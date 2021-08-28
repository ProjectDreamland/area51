///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_GIVE_PLAYER_ITEM
#define _TRIGGER_ACTIONS_GIVE_PLAYER_ITEM

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// GIVE_PLAYER_ITEM : gives the player an item.
//=========================================================================

class give_player_item : public actions_base
{
public:
                    give_player_item                ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Give the Player an Item"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Gives the player an item."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_GIVE_PLAYER_ITEM;}

protected:
    
    s32            m_TemplateIndex;
};

#endif
