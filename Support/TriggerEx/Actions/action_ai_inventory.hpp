///////////////////////////////////////////////////////////////////////////////
//
//  action_player_inventory.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_ai_inventory_
#define _action_ai_inventory_

//=========================================================================
// INCLUDES
//=========================================================================

#include "action_ai_base.hpp"
#include "Inventory\Inventory2.hpp"

//=========================== ==============================================
// action_player_inventory
//=========================================================================

class action_ai_inventory : public action_ai_base
{
public:
                    action_ai_inventory                 ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_AI_INVENTORY;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Give item to an NPC, or take item from an NPC."; }
    virtual         const char*         GetDescription  ( void );
    virtual         ai_action_types     GetAIActionType ( void ) { return AI_INVENTORY; }

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         s32*                GetTemplateRef  ( xstring& Desc );

    virtual         void                EditorPreGame   ( void );
#endif // X_EDITOR

protected:
    
    enum codes
    { 
        INVALID_AI_ITEM_CODES = -1,
        AI_ITEM_CODE_GIVE,
        AI_ITEM_CODE_TAKE,
        AI_ITEM_CODES_END
    };

    object_affecter             m_TargetAffecter;
    s32                         m_Code;         // Used to determine what type of action to perform
    s32                         m_TemplateIndex;
    inven_item                  m_Item;         // ID of item to see if player has
    xbool                       m_RemoveAllItem;
};

 //=========================================================================

#endif
