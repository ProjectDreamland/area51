//==============================================================================
//  DebugMenuAudio.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu gameplay page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "CollisionMgr\PolyCache.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU ) && defined( DEBUG_POLY_CACHE )


extern s32 g_n8Clusters;
extern s32 g_n16Clusters;
extern s32 g_n32Clusters;

//==============================================================================

debug_menu_page_polycache::debug_menu_page_polycache( ) : debug_menu_page()
{
    m_pTitle = "Poly Cache";

    m_p8Clusters  = AddItemInt( "Number of 8 clusters", g_n8Clusters, 1, 1024 );
    m_p16Clusters = AddItemInt( "Number of 16 clusters", g_n16Clusters, 1, 1024 );
    m_p32Clusters = AddItemInt( "Number of 32 clusters", g_n32Clusters, 1, 1024 );
}

//==============================================================================

void debug_menu_page_polycache::OnChangeItem( debug_menu_item* pItem )
{
    if( (pItem == m_p8Clusters) || (pItem == m_p16Clusters) || (pItem == m_p32Clusters) )
    {
        g_PolyCache.ResetStats();
    }
}

//==============================================================================

void debug_menu_page_polycache::OnPreRenderActive( void )
{
    g_PolyCache.DisplayStats();
}

#endif // defined( ENABLE_DEBUG_MENU ) && defined( DEBUG_POLY_CACHE )
