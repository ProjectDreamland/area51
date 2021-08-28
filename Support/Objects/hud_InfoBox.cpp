//==============================================================================
//
//  hud_InfoBox.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "hud_InfoBox.hpp"
#include "HudObject.hpp"  

#include "NetworkMgr\GameMgr.hpp"

#include "GameLib\RenderContext.hpp"
#include "Ui\ui_manager.hpp"
#include "Ui\ui_font.hpp"
#include "Objects\flag.hpp"


static xcolor SCORE_RECT_COLOR_GREEN = xcolor( 0,31,0,127 );

//==============================================================================
// FUNCTIONS
//==============================================================================

hud_info_box::hud_info_box( void ) 
{
    m_ScoreString_Col1[0][0] = 0x0000;
    m_ScoreString_Col1[1][0] = 0x0000;
    m_ScoreString_Col2[0][0] = 0x0000;
    m_ScoreString_Col2[1][0] = 0x0000;
    m_ScoreString_Col3[0][0] = 0x0000;
    m_ScoreString_Col3[1][0] = 0x0000;

    m_RenderCTFFlag      = FALSE;
    m_CTFFlagAlpha       = 0.0f;
    m_CTFFlag.SetName    ( PRELOAD_FILE( "HUD_multiplayer_FlagIcon.xbmp") );
    m_CTFFlagRing.SetName( PRELOAD_FILE( "HUD_multiplayer_FlagIcon_backgnd.xbmp"));

    m_LastWidth = 0;
}

//==============================================================================

void hud_info_box::OnRender( player* pPlayer )
{
    (void)pPlayer;

#ifndef X_EDITOR
    xwchar  ClockStr      [ 8 ];          
    s32 NumBoxes = 0;
    vector3 Pos;

    //
    // Draw the data bars.
    //
    s32 BoxNum;

    f32 LineWidth = 64.0f;
    f32 LineWidthSeg1 = 12.0f;
    f32 LineWidthSeg2 = 12.0f;
    f32 LineWidthSeg3 = 12.0f;


    for( BoxNum = 0; BoxNum < 2; BoxNum++ )
    {
        irect TextRect;

        f32 Line1Width[3];
        Line1Width[0] = 8.0f;
        Line1Width[1] = 8.0f;
        Line1Width[2] = 8.0f;

        if( x_wstrlen(m_ScoreString_Col1[BoxNum]) )
        {                
            g_UiMgr->TextSize( 1, TextRect, m_ScoreString_Col1[ BoxNum ], -1);
            Line1Width[0] += (f32)TextRect.GetWidth();
        }

        if( x_wstrlen(m_ScoreString_Col2[BoxNum]) )
        {
            NumBoxes++;
            g_UiMgr->TextSize( 1, TextRect, m_ScoreString_Col2[ BoxNum ], -1);
            Line1Width[1] += (f32)TextRect.GetWidth();
        }

        if( x_wstrlen(m_ScoreString_Col3[BoxNum]) )
        {
            g_UiMgr->TextSize( 1, TextRect, m_ScoreString_Col3[ BoxNum ], -1);
            Line1Width[2] += (f32)TextRect.GetWidth();
        }


        LineWidthSeg1 = x_max( LineWidthSeg1, Line1Width[0] );
        LineWidthSeg2 = x_max( LineWidthSeg2, Line1Width[1] );
        LineWidthSeg3 = x_max( LineWidthSeg3, Line1Width[2] );
        LineWidth = (LineWidthSeg1+LineWidthSeg2+LineWidthSeg3);
    }

    Pos.GetX() = m_XPos; 
    Pos.GetY() = m_YPos;               

    Pos.GetX() -= LineWidth;

    m_LastWidth = (s32)LineWidth;

    // check for pulsing
    xcolor PulseColor( g_HudColor );

    if( m_bPulsing )
    {
        PulseColor.A = (u8)(((f32)PulseColor.A / 255) * hud_object::m_PulseAlpha);
    }

    // Now draw both the boxes.
    for( s32 i = 0; (i <= NumBoxes) && (NumBoxes != 0); i++ )
    {
        if( i >= NumBoxes )
        {
            break;
        }

        // Text
        {
            irect Rect;

            // Fade Out
            Rect.Set( (s32)(Pos.GetX() + 2)-8,(s32)(Pos.GetY() + 1), 
                      (s32)(Pos.GetX() + 2),  (s32)(Pos.GetY() + 17) );
            draw_GouraudRect(Rect,xcolor(0,31,0,0),xcolor(0,31,0,0),SCORE_RECT_COLOR_GREEN,SCORE_RECT_COLOR_GREEN,FALSE);

            // Back Drop
            Rect.Set( (s32)(Pos.GetX() + 2),                                                  (s32)(Pos.GetY() + 1), 
                      (s32)(Pos.GetX() + LineWidthSeg1 + LineWidthSeg2 + LineWidthSeg3 - 2),  (s32)(Pos.GetY() + 17) );
            draw_Rect(Rect, SCORE_RECT_COLOR_GREEN, FALSE);

            // Seg 1
            Rect.Set( (s32)(Pos.GetX() + 2),                 (s32)(Pos.GetY() + 1), 
                      (s32)(Pos.GetX() + LineWidthSeg1 - 2)-4, (s32)(Pos.GetY() + 17) );

            xcolor textColor( XCOLOR_WHITE );
            //draw_Rect(Rect, xcolor(0,255,0,80), TRUE);
            if( x_wstrlen(m_ScoreString_Col1[i]) )
            {                
                RenderLine( m_ScoreString_Col1[ i ],  Rect, 255, textColor, 1, ui_font::h_right |ui_font::v_bottom, FALSE );
            }

            // Seg 2
            Rect.Set( (s32)(Pos.GetX() + LineWidthSeg1 + 2),                 (s32)(Pos.GetY() + 1), 
                      (s32)(Pos.GetX() + LineWidthSeg1 + LineWidthSeg2 - 2), (s32)(Pos.GetY() + 17) );
            //draw_Rect(Rect, xcolor(255,0,0,180), TRUE);
            if( x_wstrlen(m_ScoreString_Col2[i]) )
            {                
                RenderLine( m_ScoreString_Col2[ i ],  Rect, 255, textColor, 1, ui_font::h_left|ui_font::v_bottom, FALSE );
            }

            // Seg 3
            Rect.Set( (s32)(Pos.GetX() + LineWidthSeg1 + LineWidthSeg2 + 2),                 (s32)(Pos.GetY() + 1), 
                      (s32)(Pos.GetX() + LineWidthSeg1 + LineWidthSeg2 + LineWidthSeg3 - 2), (s32)(Pos.GetY() + 17) );
            //draw_Rect(Rect, xcolor(255,255,0,180), TRUE);
            if( x_wstrlen(m_ScoreString_Col3[i]) )
            {               
                RenderLine( m_ScoreString_Col3[ i ],  Rect, 255, textColor, 1, ui_font::h_right|ui_font::v_bottom, FALSE );
            }

            // End Flare
            Rect.Set( (s32)(Pos.GetX() + LineWidthSeg1 + LineWidthSeg2 + LineWidthSeg3 - 2),   (s32)(Pos.GetY() + 1), 
                      (s32)(Pos.GetX() + LineWidthSeg1 + LineWidthSeg2 + LineWidthSeg3 - 2)+8, (s32)(Pos.GetY() + 17) );
            draw_GouraudRect(Rect,SCORE_RECT_COLOR_GREEN,SCORE_RECT_COLOR_GREEN,xcolor(0,180,0,80),xcolor(0,180,0,80),FALSE);

            // Bright side line.
            irect rLine;
            rLine.Set(
                        Rect.l+8, Rect.t,
                        Rect.l+7, Rect.b
                     );
            draw_Rect(rLine,g_HudColor,TRUE);

            Pos.GetY()+=16; // Line Feed
        }
    }

    //
    // Draw the Clock Box
    //
    if( GameMgr.GetClockMode() == -1 )
    {
        f32 GameTimeLeft = GameMgr.GetClock();
        GameTimeLeft += 1.0f;        
        GameTimeLeft = MINMAX( 0.0f, GameTimeLeft, (f32)GameMgr.GetClockLimit() );

        s32 Seconds1 = ((s32)(GameTimeLeft) % 60) / 10;
        s32 Seconds2 = ((s32)(GameTimeLeft) % 60) % 10;
        s32 Minutes1 = (((s32)(GameTimeLeft)) / 60) / 10;
        s32 Minutes2 = (((s32)(GameTimeLeft)) / 60) % 10;

        if( Minutes1 != 0 )
            x_wstrcpy( ClockStr, (const xwchar*)((xwstring)xfs( "%d%d:%d%d", Minutes1, Minutes2, Seconds1, Seconds2 )) );
        else
            x_wstrcpy( ClockStr, (const xwchar*)((xwstring)xfs( "%d:%d%d", Minutes2, Seconds1, Seconds2 )) );

        irect TimeStringRect;
        g_UiMgr->TextSize( 1, TimeStringRect, ClockStr, -1);
        s32 TimeStringWidth = (s32)TimeStringRect.GetWidth();       
        TimeStringWidth+=8;

        if( ClockStr[ 0 ] == 0 )
        {
            return;
        }
        
        vector3 ClockBarPos = Pos;
        ClockBarPos.GetX() = (m_XPos-2) - TimeStringWidth; 

        irect Rect;
        xcolor TextColor;

        TextColor = XCOLOR_WHITE;
        if( NumBoxes == 0 )
        {
            Rect.Set( (s32)ClockBarPos.GetX(), (s32)(ClockBarPos.GetY() + 2.0f), (s32)(ClockBarPos.GetX() + TimeStringWidth), (s32)(ClockBarPos.GetY() + 18 + 2.0f) );
        }
        else
        {
            Rect.Set( (s32)ClockBarPos.GetX(), (s32)(ClockBarPos.GetY() + 1.0f), (s32)(ClockBarPos.GetX() + TimeStringWidth), (s32)(ClockBarPos.GetY() + 18 + 1.0f) );
        }

        // Back fill
        draw_Rect(Rect, SCORE_RECT_COLOR_GREEN, FALSE);

        // Text
        RenderLine( ClockStr, Rect, 255, TextColor, 1, ui_font::h_right|ui_font::v_top, FALSE );

        // End Flare ( bright side )
        Rect.Set( (s32)(ClockBarPos.GetX() + TimeStringWidth),    (s32)(ClockBarPos.GetY() + 1.0f), 
                  (s32)(ClockBarPos.GetX() + TimeStringWidth)+8,  (s32)(ClockBarPos.GetY() + 18 + 1.0f) );
        draw_GouraudRect(Rect,SCORE_RECT_COLOR_GREEN,SCORE_RECT_COLOR_GREEN,xcolor(0,180,0,80),xcolor(0,180,0,80),FALSE);

        // Bright side line.
        irect rLine;
        rLine.Set(Rect.l+8,Rect.t,Rect.l+7,Rect.b);
        draw_Rect(rLine,g_HudColor,TRUE);

        // Fade Out
        Rect.Set( (s32)(ClockBarPos.GetX()-8),    (s32)(ClockBarPos.GetY() + 1.0f), 
                  (s32)(ClockBarPos.GetX()),      (s32)(ClockBarPos.GetY() + 18 + 1.0f) );
        draw_GouraudRect(Rect,xcolor(0,31,0,0),xcolor(0,31,0,0),SCORE_RECT_COLOR_GREEN,SCORE_RECT_COLOR_GREEN,FALSE);

        Pos.GetY()+=16; // Line Feed
    }

    // Render the CTF Flag icon
    OnRenderCTF_Flag(Pos);

#endif // X_EDITOR
}

//==============================================================================

void hud_info_box::OnAdvanceLogic( player* pPlayer, f32 DeltaTime )
{
    (void)pPlayer;
    (void)DeltaTime;

    // Update the CTF Flag Icon
    UpdateCFT_Flag( DeltaTime, pPlayer );
}

//==============================================================================

xbool hud_info_box::OnProperty( prop_query& rPropQuery )
{
    (void)rPropQuery;
    return FALSE;
}

//==============================================================================

void hud_info_box::OnEnumProp( prop_enum&  List )
{
    (void)List;
}

//==============================================================================

void hud_info_box::SetScoreInfo( const xwchar* Col1, const xwchar* Col2, const xwchar* Col3, s32 Slot )
{
    if( Slot<0 )
    {
        s32 s = ABS(Slot);
        s -= 1;    
        s = MAX(s,0);

        m_ScoreString_Col1[ABS(Slot)-1][0] = 0x0000;
        m_ScoreString_Col2[ABS(Slot)-1][0] = 0x0000;
        m_ScoreString_Col3[ABS(Slot)-1][0] = 0x0000;
    }
    else    
    {
        s32 s = ABS(Slot);
        s -= 1;
        s = MAX(s,0);

        if( x_wstrlen(Col1) )
            x_wstrcpy( m_ScoreString_Col1[s], Col1 );
        else
            m_ScoreString_Col1[s][0] = 0x0000;

        if( x_wstrlen(Col2) )
            x_wstrcpy( m_ScoreString_Col2[s], Col2 );
        else
            m_ScoreString_Col2[s][0] = 0x0000;

        if( x_wstrlen(Col3) )
            x_wstrcpy( m_ScoreString_Col3[s], Col3 );
        else
            m_ScoreString_Col3[s][0] = 0x0000;
    }
}

//------------------------------------------------------------------------------

void hud_info_box::UpdateCFT_Flag( f32 DeltaTime, player* pPlayer )
{
    (void)DeltaTime;
    (void)pPlayer;

#ifndef X_EDITOR

    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
    {
        return;
    }

    xbool bAttachedToPlayer = FALSE;

    g_ObjMgr.SelectByAttribute( object::ATTR_COLLIDABLE, object::TYPE_FLAG );
    slot_id aID = g_ObjMgr.StartLoop();
    while( (aID != SLOT_NULL) )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot(aID);

        flag& Flag = flag::GetSafeType( *pObject );

        if( Flag.IsAttached() && (Flag.GetAttachedTo() == pPlayer->net_GetSlot()) )
        {
            bAttachedToPlayer = TRUE;
            break;
        }

        aID = g_ObjMgr.GetNextResult( aID );
    }
    g_ObjMgr.EndLoop();

    // Check to see if we need to show that the player has the flag.
    if( bAttachedToPlayer )
    {
        m_RenderCTFFlag = TRUE;
        m_CTFFlagAlpha = 1.0f;
    }
    else
    {
        m_CTFFlagAlpha -= 4.0f;
        if( m_CTFFlagAlpha <= 0.0f)
        {
            m_RenderCTFFlag = FALSE;
            m_CTFFlagAlpha = 0.0f;
        }
    }
#endif
}

//------------------------------------------------------------------------------

void hud_info_box::OnRenderCTF_Flag( vector3& Pos )
{
    (void)Pos;
#ifndef X_EDITOR
    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
    {
        return;
    }

    if( m_RenderCTFFlag || m_CTFFlagAlpha > 0 )
    {
        s32 FLAG_Y = (s32)(Pos.GetY());
        xbitmap*  pCTFBitmap = NULL;
        pCTFBitmap = m_CTFFlag.GetPointer();
        if( pCTFBitmap )
        {
            xcolor FlagColor = xcolor(200,0,0,255);
            FlagColor.A = (u8)(255.0f*m_CTFFlagAlpha);
            FlagColor.A = MIN( FlagColor.A, hud_object::m_PulseAlpha );
            draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );
            draw_SetTexture(*pCTFBitmap);
            draw_DisableBilinear();                            
            draw_Sprite( 
                vector3(m_XPos-(f32)pCTFBitmap->GetWidth(),f32(FLAG_Y+4),0.0f ), 
                vector2((f32)pCTFBitmap->GetWidth(), (f32)pCTFBitmap->GetHeight()), 
                FlagColor );
            draw_End();
        }

        xbitmap* pCTFRingBitmap = NULL;
        pCTFRingBitmap = m_CTFFlagRing.GetPointer();
        if( pCTFRingBitmap )
        {
            xcolor FlagColor = g_HudColor;
            FlagColor.A = (u8)(255.0f*m_CTFFlagAlpha);
            draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );
            draw_SetTexture(*pCTFRingBitmap);
            draw_DisableBilinear();                            
            draw_Sprite( 
                vector3(m_XPos-(f32)pCTFRingBitmap->GetWidth(),f32(FLAG_Y+4),0.0f ), 
                vector2((f32)pCTFRingBitmap->GetWidth(), (f32)pCTFRingBitmap->GetHeight()), 
                FlagColor );
            draw_End();
        }

    }
#endif
}

//------------------------------------------------------------------------------
