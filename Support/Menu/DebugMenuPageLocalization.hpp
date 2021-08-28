//==============================================================================
//  DebugMenuPageLocalization.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for localization debug. 
//  
//==============================================================================

#ifndef DEBUG_MENU_PAGE_LOCALIZATION_HPP
#define DEBUG_MENU_PAGE_LOCALIZATION_HPP

//==============================================================================

class debug_menu_page_localization : public debug_menu_page
{
public:
                                debug_menu_page_localization      ( );
    virtual                     ~debug_menu_page_localization     ( ) { };

    virtual void                OnChangeItem        ( debug_menu_item* pItem );

    virtual void                OnFocus             ( void );
    virtual void                OnRenderActive      ( void );

protected:
    debug_menu_item*            m_pItemShowSmallFont;
    debug_menu_item*            m_pItemShowLargeFont;
    debug_menu_item*            m_pItemTerritory;

private:
    xbool                       m_bShowLargeFont;
    xbool                       m_bShowSmallFont;
};

//==============================================================================

#endif // DEBUG_MENU_PAGE_LOCALIZATION_HPP
