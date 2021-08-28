///////////////////////////////////////////////////////////////////////////////
//
//  condition_player_has_item.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_player_has_item_
#define _condition_player_has_item_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "..\TriggerEx_Conditionals.hpp"
#include "Inventory\Inventory2.hpp"

//=========================================================================
// Check Property
//=========================================================================

class condition_player_has_item : public conditional_ex_base
{
public:
                    condition_player_has_item                ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_PLAYER_HAS;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Check if the player has a particular item."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

protected:
    
    enum codes
    { 
        INVALID_ITEM_CODES = -1,
        ITEM_CODE_HAS_ITEM,
        ITEM_CODE_NOT_HAVE_ITEM,
        ITEM_CODES_END
    };

    s32                                 m_Code;         // Used to determine what type of conditional check to perform
    inven_item                          m_Item;         // ID of item to see if player has
//    s32                                 m_ItemNameID;
};

#endif
