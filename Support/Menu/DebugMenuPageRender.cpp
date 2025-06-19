//==============================================================================
//  DebugMenuPageRender.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu render page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Gamelib\statsmgr.hpp"
#include "Render\Render.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

#ifndef CONFIG_VIEWER
extern xbool        g_MagentaColor;
extern stats        g_Stats;
#endif
extern xbool        g_RenderFrameRateInfo;
extern xbool        g_bShowBackFaces;
extern xbool        g_RenderBoneBBoxes;
extern xbool        g_RenderHUD;


//==============================================================================

static const char*  ColorText[] = { "BLACK", "MAGENTA" };

//==============================================================================

debug_menu_page_render::debug_menu_page_render( ) : debug_menu_page()
{
    m_pTitle = "Render";

                            AddItemBool     ( "Display frame rate info"    , g_RenderFrameRateInfo );
#ifndef CONFIG_VIEWER							
                            AddItemSeperator( );
                            AddItemEnum     ( "Background color"           , g_MagentaColor, ColorText, 2 );
#endif

#if !defined(X_RETAIL) || defined(X_QA)
    m_pItemPolyCache     =  AddItemBool     ( "Display polycache"          , g_PolyCache.m_Debug.RENDER );
#endif

#ifndef X_RETAIL
                            AddItemSeperator( );
                            
                            AddItemBool     ( "Display full screen stats"  , stats_mgr::m_bShowNumbers );
#endif
#ifndef CONFIG_VIEWER
                            AddItemBool     ( "Display PS2 render stats"   , g_Stats.RenderStats );
#endif							

#if defined TARGET_XBOX && (!defined CONFIG_RETAIL)

                            AddItemSeperator( );
                            AddItemBool     ( "Show back faces", g_bShowBackFaces );
#endif

#ifndef X_RETAIL
                            AddItemSeperator( );
                            AddItemBool     ( "Render bone BBoxes"         , g_RenderBoneBBoxes            );
                            AddItemBool     ( "Render rigids only"         , g_RenderDebug.RenderRigidOnly );
                            AddItemBool     ( "Render skins  only"         , g_RenderDebug.RenderSkinOnly );
                            AddItemBool     ( "Render clipped only"        , g_RenderDebug.RenderClippedOnly );
                            AddItemBool     ( "Render shadowed only"       , g_RenderDebug.RenderShadowedOnly );
#endif

#if !defined(X_RETAIL) || defined(X_QA)
                            AddItemBool     ( "Render HUD"                 , g_RenderHUD );
#endif
}

//==============================================================================

void debug_menu_page_render::OnChangeItem( debug_menu_item* pItem )
{
    if( pItem == m_pItemPolyCache )
    {
#if !defined(X_RETAIL) || defined(X_QA)
        g_PolyCache.m_Debug.RENDER_HIT_CLUSTERS = g_PolyCache.m_Debug.RENDER;
#endif
    }
}

//==============================================================================

#endif // defined( ENABLE_DEBUG_MENU )
