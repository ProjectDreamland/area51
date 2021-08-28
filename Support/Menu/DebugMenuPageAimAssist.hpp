//==============================================================================
//  DebugMenuPageAimAssist.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for rendering debug options.
//  
//==============================================================================

#ifndef DEBUG_MENU_PAGE_AIM_ASSIST_HPP
#define DEBUG_MENU_PAGE_AIM_ASSIST_HPP

//==============================================================================

class debug_menu_page_aim_assist : public debug_menu_page
{
public:
                                debug_menu_page_aim_assist      ( );
    virtual                     ~debug_menu_page_aim_assist     ( ) { };

    virtual void                OnChangeItem        ( debug_menu_item* pItem );

protected:
    debug_menu_item*            m_pItemPolyCache;
};

//==============================================================================

#endif // DEBUG_MENU_PAGE_AIM_ASSIST_HPP