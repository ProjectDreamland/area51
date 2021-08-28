//==============================================================================
//  DebugMenuPageLogging.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu Logging page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "EventMgr\EventMgr.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

debug_menu_page_logging::debug_menu_page_logging( ) : debug_menu_page()
{
    m_pTitle = "Logging Control";

#if !defined(X_RETAIL)
    AddItemBool     ( "Audio Event Logging" , g_EventMgr.m_bLogAudio );
    AddItemBool     ( "Particle Event Logging", g_EventMgr.m_bLogParticle );
    AddItemSeperator();
#endif


}

#endif // defined( ENABLE_DEBUG_MENU )
