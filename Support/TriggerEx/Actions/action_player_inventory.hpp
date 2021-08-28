///////////////////////////////////////////////////////////////////////////////
//
//  action_player_inventory.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_player_inventory_
#define _action_player_inventory_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "Inventory/Inventory2.hpp"

//=========================== ==============================================
// action_player_inventory
//=========================================================================

class action_player_inventory : public actions_ex_base
{
public:
                    action_player_inventory             ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_PLAYER_INVENTORY;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Give item to player, or take item from player."; }
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

protected:
    
    enum codes
    { 
        INVALID_PLAYER_ITEM_CODES = -1,
        PLAYER_ITEM_CODE_GIVE,
        PLAYER_ITEM_CODE_TAKE,
        PLAYER_ITEM_CODES_END
    };

    s32                         m_Code;         // Used to determine what type of action to perform
    inven_item                  m_Item;         // ID of item to see if player has
    xbool                       m_RemoveAllItem;

};


#endif
