//==============================================================================
//  DebugMenuPageLocalization.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu localization page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "UI\ui_manager.hpp"
#include "UI\ui_font.hpp"

extern xbool    g_bBloodEnabled;
extern xbool    g_bRagdollsEnabled;

extern xbool    g_bControllerCheck;

#if !defined(X_RETAIL) || defined(X_QA)
extern xbool    g_bStringTest;
extern xbool    g_bShowStringID;
#endif

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

static const char* s_TerritoryStrings[] = { "Americas", "Japan (unsupported)", "Europe (censored)" };
s32 s_Territory = XL_TERRITORY_AMERICA;

s32 Spacing = 3;

xbool   bCharOutline = FALSE;

debug_menu_page_localization::debug_menu_page_localization( ) : debug_menu_page()
{
    m_pTitle = "Localization";

    s_Territory =           x_GetTerritory();

                            AddItemBool     ( "Enable Blood" , g_bBloodEnabled );
                            AddItemBool     ( "Enable Ragdoll", g_bRagdollsEnabled );
                            AddItemSeperator();
#if !defined(X_RETAIL) || defined(X_QA)
                            AddItemBool     ( "Substitute Test Strings", g_bStringTest );
                            AddItemBool     ( "Show String ID", g_bShowStringID );
                            AddItemSeperator( "(Note: Substitute Test Strings is destructive.)" );
                            AddItemSeperator();
#endif
                            AddItemBool     ( "Outline Text", bCharOutline );
    m_pItemShowLargeFont =  AddItemButton   ( "Show Large Font" );
    m_pItemShowSmallFont =  AddItemButton   ( "Show Small Font" );
                            AddItemSeperator();
    m_pItemTerritory =      AddItemEnum     ( "Change Territory", s_Territory, s_TerritoryStrings, 3 );
                            AddItemSeperator();
                            AddItemBool     ( "Check for pulled Controllers", g_bControllerCheck );

    m_bShowLargeFont    = FALSE;
    m_bShowSmallFont    = FALSE;
}

void debug_menu_page_localization::OnChangeItem( debug_menu_item* pItem ) 
{ 
    if( pItem == m_pItemShowSmallFont )
    {
        if( m_bShowSmallFont )
        {
            if( Spacing == 3 )
            {
                Spacing = 1;
            }
            else
            {
                Spacing = 3;
            }
        }
        else
        {
            Spacing = 3;
            m_bShowLargeFont    = FALSE;
            m_bShowSmallFont    = TRUE;
        }
    }
    
    if( pItem == m_pItemShowLargeFont )
    {
        if( m_bShowLargeFont )
        {
            if( Spacing == 3 )
            {
                Spacing = 1;
            }
            else
            {
                Spacing = 3;
            }
        }
        else
        {
            Spacing = 3;
            m_bShowLargeFont    = TRUE;
            m_bShowSmallFont    = FALSE;
        }
    }

    if( pItem == m_pItemTerritory )
    {
        ASSERT( s_Territory < XL_NUM_TERRITORIES );
        x_SetTerritory((x_console_territory) s_Territory );
    }
}

void debug_menu_page_localization::OnFocus( void )
{
    m_bShowLargeFont    = FALSE;
    m_bShowSmallFont    = FALSE;
    Spacing = 3;
}

void debug_menu_page_localization::OnRenderActive( void ) 
{ 
    ui_font* pFont;
    irect Rect;

    s32 Xpos = 20;
    s32 Ypos = 250;

    xwchar wc[2];

    if( m_bShowSmallFont )
    {
        pFont = g_UiMgr->GetFont("small");

        for (s32 ch = 0x12; ch <= 0xff; ch++)
        {
            if ((ch < 0x81) || ((ch > 0x84) && (ch < 0x8d)) || ((ch > 0x9f) && (ch < 0xaf)) || (ch > 0xbe))
            {
                wc[0] = ch;
                wc[1] = 0;

                Rect.Set(0,0,pFont->GetCharacter(ch).W,pFont->GetLineHeight());

                if( Xpos + Rect.GetWidth() > 480 )
                {
                    Ypos += Rect.GetHeight() + Spacing;
                    Xpos = 20;
                }

                Rect.Translate(Xpos, Ypos);

                if( bCharOutline )
                    draw_Rect( Rect, xcolor(128,128,128) );
                pFont->RenderText( Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), wc );

                Xpos += Rect.GetWidth() + Spacing;
            }
        }
    }
    else if( m_bShowLargeFont )
    {
        pFont = g_UiMgr->GetFont("large");

        for (s32 ch = 0x12; ch <= 0xff; ch++)
        {
            if ((ch < 0x85) || ((ch > 0x85) && (ch < 0x8d)) || (ch > 0x9f))
            {
                wc[0] = ch;
                wc[1] = 0;

                Rect.Set(0,0,pFont->GetCharacter(ch).W,pFont->GetLineHeight());

                if( Xpos + Rect.GetWidth() > 480 )
                {
                    Ypos += Rect.GetHeight() + Spacing;
                    Xpos = 20;
                }

                Rect.Translate(Xpos, Ypos);

                if( bCharOutline )
                    draw_Rect( Rect, xcolor(128,128,128) );
                pFont->RenderText( Rect, ui_font::h_left|ui_font::v_top, xcolor(XCOLOR_WHITE), wc );

                Xpos += Rect.GetWidth() + Spacing;
            }
        }
    }
}

#endif // defined( ENABLE_DEBUG_MENU )
