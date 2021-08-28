//==============================================================================
//  DebugMenuPageMonkey.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for enabling the monkey. 
//  (or perhaps beating him, or feeding him...)
//  
//==============================================================================

#ifndef DEBUG_MENU_PAGE_MONKEY_HPP
#define DEBUG_MENU_PAGE_MONKEY_HPP

//==============================================================================

class debug_menu_page_monkey : public debug_menu_page
{
public:
                                debug_menu_page_monkey      ( );
    virtual                     ~debug_menu_page_monkey     ( ) { };

    virtual void                OnChangeItem        ( debug_menu_item* pItem );
    virtual void                OnRenderActive      ( void );

protected:
    debug_menu_item*            m_pItemMonkeyEnable;
};

//==============================================================================

#endif // DEBUG_MENU_PAGE_MONKEY_HPP
