//==============================================================================
//  DebugMenuPageMemory.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for memory debug optoins.
//  
//==============================================================================

#ifndef DEBUG_MENU_PAGE_MEMORY_HPP
#define DEBUG_MENU_PAGE_MEMORY_HPP

//==============================================================================

class debug_menu_page_memory : public debug_menu_page
{
public:
                                debug_menu_page_memory      ( );
    virtual                     ~debug_menu_page_memory     ( ) { };

    virtual void                OnChangeItem        ( debug_menu_item* pItem );

protected:
    debug_menu_item*            m_pItemMemoryDump;
    debug_menu_item*            m_pItemMemorySummaryDump;
    debug_menu_item*            m_pItemResourceSummaryDump;
    debug_menu_item*            m_pItemObjMgrSummaryDump;
};

//==============================================================================

#endif // DEBUG_MENU_PAGE_MEMORY_HPP
