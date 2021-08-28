#ifndef DEBUG_MENU_PAGE_POLYCACACHE_HPP
#define DEBUG_MENU_PAGE_POLYCACACHE_HPP

//==============================================================================
//  DebugMenuPagePolyCache.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page forpoly cache options.
//  
//==============================================================================

class debug_menu_page_polycache : public debug_menu_page
{
public:
    debug_menu_page_polycache( );
    virtual                    ~debug_menu_page_polycache( ) { };

    virtual void                OnChangeItem        ( debug_menu_item* pItem );
    virtual void                OnPreRenderActive   ( void );

protected:
    debug_menu_item*            m_p8Clusters;
    debug_menu_item*            m_p16Clusters;
    debug_menu_item*            m_p32Clusters;

};

//==============================================================================

#endif // DEBUG_MENU_PAGE_POLYCACACHE_HPP
