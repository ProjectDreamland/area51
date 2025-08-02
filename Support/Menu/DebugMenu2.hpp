//==============================================================================
//  DebugMenu2.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  These classes implement a page oriented debug menu. It uses the in-game
//  font to render itself on top of the display. 
//==============================================================================

#ifndef DEBUG_MENU2_HPP
#define DEBUG_MENU2_HPP

//==============================================================================

#include "Entropy.hpp"
#include "DebugMenuItem.hpp"
#include "DebugMenuPage.hpp"
#include "DebugMenuPageGeneral.hpp"
#include "DebugMenuPageMemory.hpp"
#include "DebugMenuPageRender.hpp"
#include "DebugMenuPageGameplay.hpp"
#include "DebugMenuPageAimAssist.hpp"
#include "DebugMenuPageAIScript.hpp"
#include "DebugMenuPageMonkey.hpp"
#include "DebugMenuPageLogging.hpp"
#include "DebugMenuAudio.hpp"
#include "DebugMenuPerception.hpp"
#include "DebugMenuPageAdvCheckpoints.hpp"
#include "DebugMenuPagePolyCache.hpp"
#include "DebugMenuPageFx.hpp"
#include "DebugMenuPageMultiplayer.hpp"
#include "DebugMenuPageLocalization.hpp"
#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif
#include "../StateMgr/StateMgr.hpp"

//==============================================================================
//  Determine if the debug menu should be enabled
//==============================================================================

#if defined( X_DEBUG ) || ((defined( CONFIG_QA ) || defined( CONFIG_VIEWER ) || defined( CONFIG_PROFILE )) && (!CONFIG_IS_DEMO) && !defined( LAN_PARTY_BUILD ) && !defined( OPM_REVIEW_BUILD ) )
#define ENABLE_DEBUG_MENU
#endif

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

struct stats
{
    struct ps2
    {
        xbool   MemoryUsage;
    };

    xbool   EngineStats;    
    xbool   RenderStats;
    xbool   PS2_MemStats;
    xbool   RenderVerbose;
    xbool   MemVertBars;
    ps2     PS2;
    s32     Interval;
};

//==============================================================================

class debug_menu2
{
public:

                                debug_menu2         ( );
                                ~debug_menu2        ( );

            xbool               Init                ( void );

            void                Enable              ( void );
            void                Disable             ( void );
            xbool               IsActive            ( void );

            xbool               Update              ( f32 DeltaTime );
            void                Render              ( void );

            debug_menu_page *   FindPage            ( const char* pTitle );

    static  void                RenderLine          ( s32           iFont,
                                                      const char*   Line, 
                                                      irect         iRect, 
                                                      xbool         bHighLight = FALSE, 
                                                      xbool         bFlash = FALSE );

public:
            f32                         m_FadeAlpha;

protected:

            xarray<debug_menu_page*>    m_Pages;
            s32                         m_iActivePage;
            xbool                       m_bMenuActive;
            s32                         m_ItemChangeDelay;
};

extern debug_menu2 g_DebugMenu;

//==============================================================================

#endif // defined( ENABLE_DEBUG_MENU )

//==============================================================================

#endif // DEBUG_MENU2_HPP

//==============================================================================
