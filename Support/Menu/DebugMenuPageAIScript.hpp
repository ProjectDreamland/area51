//==============================================================================
//  DebugMenuPageAIScript.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for AI and scripting options.
//  
//==============================================================================

#ifndef DEBUG_MENU_PAGE_AISCRIPT_HPP
#define DEBUG_MENU_PAGE_AISCRIPT_HPP

//==============================================================================

class debug_menu_page_aiscript : public debug_menu_page
{
public:
                                debug_menu_page_aiscript      ( );
    virtual                     ~debug_menu_page_aiscript     ( ) { };

    virtual void                OnChangeItem        ( debug_menu_item* pItem );
    virtual void                OnPreRender         ( void );

protected:
    debug_menu_item*            m_pItemTriggerDump;
    xbool                       m_bShowActiveAIs;
    xbool                       m_bToggleWallRender;
    xbool                       m_bToggleTriggerRender;
    xbool                       m_bToggleDamageRender;
};

//==============================================================================

#endif // DEBUG_MENU_PAGE_AISCRIPT_HPP
