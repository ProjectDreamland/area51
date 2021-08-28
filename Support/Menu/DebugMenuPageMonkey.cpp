//==============================================================================
//  DebugMenuPageMonkey.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu monkey page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "InputMgr\Monkey.hpp"
#include "Ui\ui_manager.hpp"
#include "Ui\ui_font.hpp"
#include "Gamelib/DebugCheats.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

debug_menu_page_monkey::debug_menu_page_monkey( ) : debug_menu_page()
{
    m_pTitle = "Monkey Business";

    m_pItemMonkeyEnable        = AddItemBool     ( "Enable Monkey" , g_MonkeyOptions.Enabled );

    AddItemSeperator();
    
    AddItemBool     ( "MonkeyMode:  Normal" ,       g_MonkeyOptions.ModeEnabled[MONKEY_NORMAL] );
    AddItemBool     ( "MonkeyMode:  Jumpman" ,      g_MonkeyOptions.ModeEnabled[MONKEY_JUMPMAN] );
    AddItemBool     ( "MonkeyMode:  Crouchman" ,    g_MonkeyOptions.ModeEnabled[MONKEY_CROUCHMAN] );
    AddItemBool     ( "MonkeyMode:  Gunman" ,       g_MonkeyOptions.ModeEnabled[MONKEY_GUNMAN] );
    AddItemBool     ( "MonkeyMode:  Grenadier" ,    g_MonkeyOptions.ModeEnabled[MONKEY_GRENADIER] );
    AddItemBool     ( "MonkeyMode:  Mutation" ,     g_MonkeyOptions.ModeEnabled[MONKEY_MUTATION] );
    AddItemBool     ( "MonkeyMode:  MenuMonkey" ,   g_MonkeyOptions.ModeEnabled[MONKEY_MENUMONKEY] );
    AddItemBool     ( "MonkeyMode:  Twitch",        g_MonkeyOptions.ModeEnabled[MONKEY_TWITCH] );
    AddItemBool     ( "MonkeyMode:  MemoryHog",     g_MonkeyOptions.ModeEnabled[MONKEY_MEMHOG] );

    AddItemSeperator();

    AddItemBool     ( "Test Out Of World?",         g_MonkeyOptions.bTestOutOfWorld );
}

//==============================================================================

void debug_menu_page_monkey::OnChangeItem( debug_menu_item* pItem )
{
    if( pItem == m_pItemMonkeyEnable )
    {
        DEBUG_INFINITE_AMMO = g_MonkeyOptions.Enabled;        
        input_SuppressFeedback( g_MonkeyOptions.Enabled );
    }

    g_MonkeyOptions.Dirty = true;
}

//==============================================================================

void debug_menu_page_monkey::OnRenderActive( void )
{
}
#endif // defined( ENABLE_DEBUG_MENU )
