//==============================================================================
//  DebugMenuPageRender.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for rendering debug options.
//  
//==============================================================================

#ifndef DEBUG_MENU_PAGE_RENDER_HPP
#define DEBUG_MENU_PAGE_RENDER_HPP

//==============================================================================

class debug_menu_page_render : public debug_menu_page
{
public:
                                debug_menu_page_render      ( );
    virtual                     ~debug_menu_page_render     ( ) { };

    virtual void                OnChangeItem        ( debug_menu_item* pItem );

protected:
    debug_menu_item*            m_pItemPolyCache;
};

//==============================================================================

#endif // DEBUG_MENU_PAGE_RENDER_HPP