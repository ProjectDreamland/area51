//==============================================================================
//  DebugMenuPageGameplay.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu gameplay page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "objects\player.hpp"
#include "Objects\Actor\Actor.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Gamelib/DebugCheats.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================
#ifndef CONFIG_VIEWER
extern f32      g_WorldTimeDilation;
extern xbool    g_FreeCamPause;
#endif
extern xbool    g_GameLogicDebug;
extern s32      g_Difficulty;
extern const char* DifficultyText[];

//==============================================================================

debug_menu_page_gameplay::debug_menu_page_gameplay( ) : debug_menu_page()
{
    m_pTitle = "Gameplay";
#ifndef CONFIG_VIEWER
    m_pItemInvulnerability          = AddItemBool     ( "Invulnerability"               , DEBUG_INVULNERABLE );
                                      AddItemBool     ( "Unlimited ammo"                , DEBUG_INFINITE_AMMO   );
                                      AddItemFloat    ( "World Time Dilation", 
                                                        g_WorldTimeDilation, 
                                                        0.01f, 3.0f, 0.01f );
    m_pItemDifficulty               = AddItemEnum     ( "Difficulty Level", g_Difficulty, DifficultyText, 3 );
                                      AddItemSeperator( );
                                      AddItemBool     ( "Pause in Fly mode"             , g_FreeCamPause );
                                      AddItemSeperator( );
#endif									  
    m_pItemResetPlayerToStart       = AddItemButton   ( "Reset player to start"                           );
    m_pItemResetPlayerToSafeSpot    = AddItemButton   ( "Reset player to safe spot"                       );
                                      AddItemSeperator( );
                                      AddItemBool     ( "Jumping Bean Expert Mode"    , DEBUG_EXPERT_JUMPINGBEAN );

    m_pAcquireLevelLoreItems        = AddItemButton   ( "Collect all level Lore Items" );

    m_pUnlockAll                    = AddItemButton   ( "Unlock everything" );
}

//==============================================================================

void debug_menu_page_gameplay::OnFocus( void )
{
}

//==============================================================================

void debug_menu_page_gameplay::OnChangeItem( debug_menu_item* pItem )
{
    // Set variables
    player* pPlayer = SMP_UTIL_GetActivePlayer();

    if( pItem == m_pItemDifficulty )
    {
        // KSS -- FIXME -- Do we need to do anything here?
    }

    if( pItem == m_pItemResetPlayerToStart )
    {
        if( pPlayer )
            pPlayer->OnReset();
    }

    if( pItem == m_pItemResetPlayerToSafeSpot )
    {
        if( pPlayer )
            pPlayer->ResetToLastSafeSpot();
    }

    if( pItem == m_pAcquireLevelLoreItems )
    {
        if( pPlayer )
            pPlayer->AcquireAllLoreObjects();
    }

    if( pItem == m_pUnlockAll )
    {
#ifndef CONFIG_RETAIL
        g_StateMgr.GetActiveProfile(0).UnlockAll();
#endif
    }
}

//==============================================================================

#endif // defined( ENABLE_DEBUG_MENU )
