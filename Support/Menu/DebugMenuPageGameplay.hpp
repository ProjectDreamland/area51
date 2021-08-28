//==============================================================================
//  DebugMenuPageGamePlay.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for game-play debug options.
//  
//==============================================================================

#ifndef DEBUG_MENU_PAGE_GAMEPLAY_HPP
#define DEBUG_MENU_PAGE_GAMEPLAY_HPP

//==============================================================================

class debug_menu_page_gameplay : public debug_menu_page
{
public:
                                debug_menu_page_gameplay      ( );
    virtual                     ~debug_menu_page_gameplay     ( ) { };

    virtual void                OnFocus             ( void );
    virtual void                OnChangeItem        ( debug_menu_item* pItem );

protected:
    debug_menu_item*            m_pItemInvulnerability;
    debug_menu_item*            m_pItemResetPlayerToStart;
    debug_menu_item*            m_pItemResetPlayerToSafeSpot;
    debug_menu_item*            m_pItemDifficulty;
    debug_menu_item*            m_pAcquireLevelLoreItems;
    debug_menu_item*            m_pUnlockAll;
};

//==============================================================================

#endif // DEBUG_MENU_PAGE_GAMEPLAY_HPP