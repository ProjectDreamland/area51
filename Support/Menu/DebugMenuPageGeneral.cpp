//==============================================================================
//  DebugMenuPageGeneral.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu general page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "objects\player.hpp"
#include "StateMgr/mapList.hpp"
#include "Ui\ui_manager.hpp"
#include "Ui\ui_font.hpp"
#include "Configuration/GameConfig.hpp"
#include "PhysicsMgr\PhysicsMgr.hpp"

#if defined( TARGET_PS2 )
#include "StatsMgr.hpp"
#endif


//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================
// Data

extern          xbool       g_ShowPlayerPos;
extern          xbool       g_DevWantsToSave;
extern          xbool       g_DevWantsToLoad;
extern          xbool       g_RenderFrameRateInfo;
extern          xbool       g_bFrameScaling;

extern          s32         g_Changelist;
extern  const   char*       g_pBuildDate;

extern          map_list    g_MapList;

                xbool       g_ShowQAInfo      = FALSE;
                xbool       g_ShowVertexUsage = FALSE;
                xbool       g_ShowGammaInfo   = FALSE;
                s32         g_ScreenShotSize  = 3;
                xbool       g_ScreenShotModeEnabled = FALSE;

                u32         g_SkinnedGeomSize = 0;
                u32         g_RigidGeomSize = 0;
                u32         g_PushSize = 0;

#if ENABLE_STATS_MGR
        static  s32         s_StatsLegend = 0;
static const char*  StatsLegendText[] = { "Off", "CPU time", "GPU time", "Small Bars" };
#endif

static const char*  ScreenShotSizeText[] = { "1x1", "2x2", "3x3", "4x4", "5x5", "6x6", "7x7", "8x8" };

//==============================================================================

debug_menu_page_general::debug_menu_page_general( ) : debug_menu_page()
{
    m_pTitle = "General";

    m_pItemShowQA =         AddItemBool     ( "Show QA info"           , g_ShowQAInfo );
#if ENABLE_STATS_MGR
                            AddItemSeperator( );
                            AddItemBool     ( "Display horizontal bars", stats_mgr::m_bShowHorizontalBars );
    m_pItemStatsLegend =    AddItemEnum     ( "Stats legend"           , s_StatsLegend, StatsLegendText, 4 );
#endif

#ifdef ENABLE_COLLISION_STATS
                            AddItemBool     ( "Display collision stats", g_CollisionMgr.m_DisplayStats );
#endif
                            AddItemBool     ( "Show player position"   , g_ShowPlayerPos );

#ifdef ENABLE_PHYSICS_DEBUG
                            AddItemSeperator( );
                            AddItemBool     ( "Display physics stats"      , g_PhysicsMgr.m_Settings.m_bShowStats );
                            AddItemBool     ( "Render physics debug"       , g_PhysicsMgr.m_Settings.m_bDebugRender );
#endif


#if !defined( TARGET_PC )
                            #ifdef TARGET_XBOX
                            AddItemSeperator( );
                            AddItemBool( "Show vertex usage", g_ShowVertexUsage );
                            AddItemBool( "Show gamma info"  , g_ShowGammaInfo   );
                            AddItemBool( "Frame scaling"    , g_bFrameScaling );
                            #endif

                            AddItemSeperator( );
                            m_pItemScreenShotSize = AddItemEnum     ( "Screen shot size"       , g_ScreenShotSize, ScreenShotSizeText, 8 );
                            m_pItemScreenShot =     AddItemButton   ( "Take screen shot" );
                            AddItemBool     ( "Enable screen shot mode", g_ScreenShotModeEnabled );
                            AddItemSeperator( "(L3+R3 controller 0, or X controller 1)" );
#endif
                            AddItemSeperator( );
                            AddItemFloat    ( "Debug menu background alpha", g_DebugMenu.m_FadeAlpha, 0.0f, 1.0f, 0.1f );
                            AddItemSeperator( );
    m_pItemQuickLoad =      AddItemButton   ( "Quick load" );
    m_pItemQuickSave =      AddItemButton   ( "Quick save" );
}

//==============================================================================

void debug_menu_page_general::OnChangeItem( debug_menu_item* pItem )
{
    if( pItem == m_pItemQuickLoad )
    {
#ifdef OLD_SAVE
        g_SaveMgr.RequestLoad();
#endif
    }

    if( pItem == m_pItemQuickSave )
    {
#ifdef OLD_SAVE
        g_SaveMgr.RequestSave();
#endif
    }

    if( pItem == m_pItemScreenShot )
    {
#if !defined( X_RETAIL ) && !defined( TARGET_PC )
        eng_ScreenShot( DEBUG_SCREEN_SHOT_DIR, g_ScreenShotSize + 1 );
#endif
    }

    if( pItem == m_pItemStatsLegend )
    {
#if ENABLE_STATS_MGR
        stats_mgr* pStatsMgr = stats_mgr::GetStatsMgr();
        if ( pStatsMgr )
        {
            pStatsMgr->m_bShowCPUTimeLegend  = FALSE;
            pStatsMgr->m_bShowGPUTimeLegend  = FALSE;
            pStatsMgr->m_bShowVerticalLegend = FALSE;

            switch( s_StatsLegend )
            {
            case 0:
                break;
            case 1:
                pStatsMgr->m_bShowCPUTimeLegend  = TRUE;
                break;
            case 2:
                pStatsMgr->m_bShowGPUTimeLegend  = TRUE;
                break;
            case 3:
                pStatsMgr->m_bShowVerticalLegend = TRUE;
                break;
            }
        }
#endif
    }

    if( pItem == m_pItemShowQA )
    {
        g_ShowPlayerPos = g_ShowQAInfo;
        g_RenderFrameRateInfo = g_ShowQAInfo;
    }
}

//==============================================================================

void debug_menu_page_general::OnPreRender ( void )
{
    // Set variables
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    s32 font = g_UiMgr->FindFont("small");
    s32 XRes,YRes;
    eng_GetRes(XRes,YRes);

    if ( g_ShowPlayerPos && pPlayer )
    {
        vector3 Pos = pPlayer->GetPosition();
        irect Rect;

        xwstring posText = (const char *)xfs( "Player( %7.1f, %7.1f, %7.1f )", Pos.GetX(), Pos.GetY(), Pos.GetZ() );
        g_UiMgr->TextSize( font, Rect, posText, posText.GetLength());
        #ifdef TARGET_XBOX
        Rect.Translate(XRes/2-50, 25);
        #else
        Rect.Translate(XRes - 270, 25);
        #endif

        draw_Rect( Rect, xcolor(0,0,0,128), FALSE );
        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), posText );
    }

    #ifdef TARGET_XBOX
    /*if( g_bFrameScaling )
    {
        extern f32  g_Brightness;
        extern f32  g_Contrast;

        irect Rect;
        s32 x = XRes/2-50;
        s32 y = 50;
        Rect.Set(x - 4, y, x + 230, y + g_UiMgr->GetLineHeight(font) * 4);
        draw_Rect( Rect, xcolor(0,0,0,128), FALSE );

        // Show resolutions
        xwstring Text;
    extern void
        xbox_GetResolutionText( xwstring& );
        xbox_GetResolutionText( Text );

        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Text );
        y += g_UiMgr->GetLineHeight(font);
    }
    else if( g_ShowGammaInfo )
    {
        extern f32  g_Brightness;
        extern f32  g_Contrast;

        irect Rect;
        s32 x = XRes/2-50;
        s32 y = 50;
        Rect.Set(x - 4, y, x + 230, y + g_UiMgr->GetLineHeight(font) * 4);
        draw_Rect( Rect, xcolor(0,0,0,128), FALSE );

        // heading
        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), xwstring( "BACK + DPad_R/L or White/Black" ) );
        y += g_UiMgr->GetLineHeight(font);

        // brightness
        xwstring Line0 = (const char *)xfs( "Brightness: %fMB", g_Brightness );
        g_UiMgr->TextSize( font, Rect, Line0, Line0.GetLength());
        Rect.Translate(x, y);
        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Line0 );
        y += g_UiMgr->GetLineHeight(font);

        // contrast
        xwstring Line1 = (const char *)xfs( "Contrast: %fMB", g_Contrast );
        g_UiMgr->TextSize( font, Rect, Line1, Line1.GetLength());
        Rect.Translate(x, y);
        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Line1 );
        y += g_UiMgr->GetLineHeight(font);
    }*/
    else if( g_ShowVertexUsage )
    {
        irect Rect;
        s32 x = XRes/2-50;
        s32 y = 50;
        Rect.Set(x - 4, y, x + 230, y + g_UiMgr->GetLineHeight(font) * 4);
        draw_Rect( Rect, xcolor(0,0,0,128), FALSE );

        // vertex memory pool
        u32 VertexRAM = g_VertFactory.GetGeneralPool().GetFree();
        u32 TotalVRAM = g_VertFactory.GetGeneralPool().GetSize();
        {
            xwstring RAM = (const char *)xfs( "Vertices: %fMB of %fMB", f32(VertexRAM)/1024.0f/1024.0f,f32(TotalVRAM)/1024.0f/1024.0f );
            g_UiMgr->TextSize( font, Rect, RAM, RAM.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), RAM );
            y += g_UiMgr->GetLineHeight(font);
        }

        // texture memory pool
        u32 TextureRAM = g_TextureFactory.GetGeneralPool().GetFree();
        u32 TotalTRAM  = g_TextureFactory.GetGeneralPool().GetSize();
        {
            xwstring RAM = (const char *)xfs( "Textures: %fMB of %fMB", f32(TextureRAM)/1024.0f/1024.0f,f32(TotalTRAM)/1024.0f/1024.0f );
            g_UiMgr->TextSize( font, Rect, RAM, RAM.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), RAM );
            y += g_UiMgr->GetLineHeight(font);
        }

        // skinned geoms
        {
            xwstring RAM = (const char *)xfs( "Skinned: %fMB", f32(g_SkinnedGeomSize)/1024.0f/1024.0f );
            g_UiMgr->TextSize( font, Rect, RAM, RAM.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), RAM );
            y += g_UiMgr->GetLineHeight(font);
        }

        // regid geoms
        {
            xwstring RAM = (const char *)xfs( "Rigid: %fMB", f32(g_RigidGeomSize)/1024.0f/1024.0f );
            g_UiMgr->TextSize( font, Rect, RAM, RAM.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), RAM );
            y += g_UiMgr->GetLineHeight(font);
        }

        // push buffers
        {
            xwstring RAM = (const char *)xfs( "Push: %fMB", f32(g_PushSize)/1024.0f/1024.0f );
            g_UiMgr->TextSize( font, Rect, RAM, RAM.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), RAM );
            y += g_UiMgr->GetLineHeight(font);
        }

        // push buffers
        {
            xwstring RAM = (const char *)xfs( "General: %fMB", f32(x_MemGetFree())/1024.0f/1024.0f );
            g_UiMgr->TextSize( font, Rect, RAM, RAM.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), RAM );
            y += g_UiMgr->GetLineHeight(font);
        }
    }
    else
    // nb: fall through to if( g_ShowQAInfo ) because they both draw
    // nb: to the same location onscreen.
    #endif

    if( g_ShowQAInfo )
    {
        irect Rect;
        #ifdef TARGET_XBOX
        s32 x = XRes/2-50;
        #else
        s32 x = XRes - 235;
        #endif
        s32 y = 50;
        Rect.Set(x - 4, y, x + 230, y + g_UiMgr->GetLineHeight(font) * 4);
        draw_Rect( Rect, xcolor(0,0,0,128), FALSE );

        //zone #
        if( pPlayer )
        {
            s32 PlayerZone = pPlayer->GetZone1();
            xwstring Zone = (const char *)xfs( "Zone: %d", PlayerZone);
            g_UiMgr->TextSize( font, Rect, Zone, Zone.GetLength());
            Rect.Translate(x, y);
            g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Zone );
            y += g_UiMgr->GetLineHeight(font);
        }

        // level name
        s32 LevelID = g_ActiveConfig.GetLevelID();
        xwstring Level  = (const char *)xfs( "Lvl:  %s", g_MapList.GetDisplayName(LevelID));
        g_UiMgr->TextSize( font, Rect, Level, Level.GetLength());
        Rect.Translate(x, y);
        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Level );
        y += g_UiMgr->GetLineHeight(font);

        // change list 
        xwstring Change  = (const char *)xfs( "Chg: %d", g_Changelist );
        g_UiMgr->TextSize( font, Rect, Level, Change.GetLength());
        Rect.Translate(x, y);
        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Change );
        y += g_UiMgr->GetLineHeight(font);

        // build date
        xwstring Build  = (const char *)xfs( "Bld: %s", g_pBuildDate );
        g_UiMgr->TextSize( font, Rect, Level, Build.GetLength());
        Rect.Translate(x, y);
        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Build );
        y += g_UiMgr->GetLineHeight(font);

        // build low water mark
        extern s32 g_MemoryLowWater;
        xwstring Mem  = (const char *)xfs( "Mem: %d", g_MemoryLowWater );
        g_UiMgr->TextSize( font, Rect, Level, Mem.GetLength());
        Rect.Translate(x, y);
        g_UiMgr->RenderText( font, Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), Mem );
        y += g_UiMgr->GetLineHeight(font);
    }

}

#endif // defined( ENABLE_DEBUG_MENU )
