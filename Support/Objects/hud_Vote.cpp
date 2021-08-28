//==============================================================================
//
//  hud_Vote.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "hud_Vote.hpp"
#include "HudObject.hpp"
#include "Ui/ui_font.hpp"
#include "StringMgr/StringMgr.hpp"

#ifndef X_EDITOR
#include "Ui/ui_manager.hpp"
#endif

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

//==============================================================================
// STORAGE
//==============================================================================

//==============================================================================
// FUNCTIONS
//==============================================================================

hud_vote::hud_vote( void ) 
{
    m_XPos              = 0;
    m_YPos              = 0;
    m_bKeyOn            = FALSE;
    m_KeyOpacity        = 0.0f;
    m_bTallyOn          = FALSE;
    m_TallyOpacity      = 0.0f;

    m_PercentOpen       = 0.0f;
    m_PercentOnScreen   = 0.0f;

    m_YesVotes          = 0;
    m_NoVotes           = 0;

    m_NoPercentage      = 0.0f;
    m_YesPercentage     = 0.0f;
    m_TargetPercentage  = 0.6f;
}

//==============================================================================

void hud_vote::OnRender( player* pPlayer )
{
#ifndef X_EDITOR
    (void)pPlayer;

    if( !IsActive() )
    {
        return;
    }

    const view* pView = eng_GetView();
    s32 L, T, R, B;
    pView->GetViewport( L, T, R, B );
    //
    // Render the key, if applicable.
    //
    {
        m_ControllerBmp.SetName(PRELOAD_FILE("hud_multiplayer_arrow.xbmp"));
        xbitmap* pBitmap = m_ControllerBmp.GetPointer();
        if( pBitmap == NULL )
            return;

        s32 BitmapWidth  = pBitmap->GetWidth();
        s32 BitmapHeight = pBitmap->GetHeight();

        //
        // Render the labels first.
        //

        const xwchar* pCancel  = g_StringTableMgr("ui", "IDS_CANCEL");
        const xwchar* pAye     = g_StringTableMgr("ui", "IDS_YES");
        const xwchar* pNay     = g_StringTableMgr("ui", "IDS_NO");
        const xwchar* pAbstain = g_StringTableMgr("ui", "IDS_ABSTAIN");
        ui_font* pFont = g_UiMgr->GetFont( "small" );

        s32 LineWidth  = pFont->TextWidth ( pCancel );
        s32 LineHeight = pFont->TextHeight( pCancel );

        f32 XPos = (m_LeftMargin + m_RightMargin) / 2.0f;
        f32 YPos = m_TopMargin - (m_KeyOpacity * 2.0f * (LineHeight + BitmapHeight)) + LineHeight + BitmapHeight;

        XPos -= (LineWidth + BitmapWidth / 2.0f);
        YPos -= (BitmapHeight / 2.0f);

        xcolor Color; 

        s32 Opacity = (s32)(255 * m_KeyOpacity);

        // LEFT    
        Color = XCOLOR_RED;
#ifdef TARGET_XBOX
        static s32 LB_R = -352;
#else
        static s32 LB_R = -296;
#endif
        irect LeftBox( 
                        (s32)L,
                        (s32)T,
                        (s32)R+LB_R,
                        (s32)B
                     );
        RenderLine( pNay, LeftBox, Opacity, Color, 1, ui_font::h_right | ui_font::v_center );
        //draw_Rect(LeftBox,XCOLOR_RED,TRUE);

        // TOP
        Color = XCOLOR_GREY; 
        static s32 TB_B = -268;
        irect TopBox(   
                        (s32)L,
                        (s32)T,
                        (s32)R, 
                        (s32)B+TB_B
                    );
        RenderLine( pAbstain, TopBox, Opacity, Color, 1, ui_font::h_center | ui_font::v_bottom );


        // BOTTOM
        Color = XCOLOR_GREY;
        static s32 BB_T = 268;
        irect BottomBox( 
                        (s32)L,
                        (s32)T+BB_T,
                        (s32)R,
                        (s32)B
                        );
        RenderLine( pCancel,       BottomBox,     Opacity, Color, 1, ui_font::h_center | ui_font::v_top );


        // RIGHT
        Color = XCOLOR_GREEN;
#ifdef TARGET_XBOX
        static s32 RB_L = 356;
#else
        static s32 RB_L = 296;
#endif
        irect RightBox( 
                        (s32)L+RB_L,
                        (s32)T,
                        (s32)R,
                        (s32)B
                      );
        RenderLine( pAye,   RightBox, Opacity, Color, 1, ui_font::h_left | ui_font::v_center );

        //
        // Now render the graphic.
        //

        vector2 WH( (f32)(BitmapWidth), (f32)(BitmapHeight) );  
        Color = g_HudColor;
        Color.A = Opacity;

        draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_2D | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER );
        draw_SetTexture( *pBitmap );
        draw_DisableBilinear();

        vector3 Pos( (f32)R/2-BitmapWidth/2,(f32)B/2-BitmapHeight/2,0.0f );
        draw_Sprite( Pos, WH, Color );

        //draw_EnableBilinear();
        draw_End();
    }

    // 
    // Render the vote tally.
    // 
    {
        static xcolor YesColor      (   0, 100,   0, 150 );
        static xcolor NoColor       ( 100,   0,   0, 150 );
        static xcolor NeutralColor  ( 80,  150, 150, 150 ); // g_HudColor
        static xcolor TextColor     ( XCOLOR_WHITE   );

        //
        // Box.
        //
        {
            m_ControllerBmp.SetName(PRELOAD_FILE("HUD_multiplayer_voting.xbmp"));
            xbitmap* pBitmap = m_ControllerBmp.GetPointer();
            if( pBitmap == NULL )
            {
                return;
            }

            s32 BitmapWidth  = pBitmap->GetWidth();
            s32 BitmapHeight = pBitmap->GetHeight();

            xcolor Color = g_HudColor;

            draw_Begin( DRAW_SPRITES, DRAW_TEXTURED|DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER );
            draw_SetTexture( *pBitmap );

            draw_DisableBilinear();
            vector2 WH( (f32)(BitmapWidth), (f32)(BitmapHeight) ); 
            static f32 VOTE_BAR_X = -16;
#ifdef TARGET_XBOX
            static f32 VOTE_BAR_Y = -84;
#else
            static f32 VOTE_BAR_Y = -108;
#endif
            vector3 Pos( m_LeftMargin+VOTE_BAR_X, m_YPos+VOTE_BAR_Y, 0.0f );
            draw_Sprite( Pos, WH, Color );
            draw_End();

            {
                draw_Begin( DRAW_QUADS, DRAW_USE_ALPHA|DRAW_2D|DRAW_NO_ZBUFFER);

                f32 Near = 0.001f;
                draw_Color( YesColor );

                // Yes Votes.
                irect YesRect(  
                                (s32)m_LeftMargin,
                                (s32)(m_BottomMargin - (m_BottomMargin - m_TopMargin) * m_YesPercentage),
                                (s32)m_RightMargin,
                                (s32)m_BottomMargin
                             );
                draw_Vertex( (f32)YesRect.l, (f32)YesRect.t, Near );
                draw_Vertex( (f32)YesRect.l, (f32)YesRect.b, Near );
                draw_Vertex( (f32)YesRect.r, (f32)YesRect.b, Near );
                draw_Vertex( (f32)YesRect.r, (f32)YesRect.t, Near );

                // No Votes.
                draw_Color( NoColor );

                irect NoRect(  
                                (s32)m_LeftMargin,
                                (s32)m_TopMargin,
                                (s32)m_RightMargin,
                                (s32)(m_TopMargin + (m_BottomMargin - m_TopMargin) * m_NoPercentage)
                            );
                draw_Vertex( (f32)NoRect.l, (f32)NoRect.t, Near );
                draw_Vertex( (f32)NoRect.l, (f32)NoRect.b, Near );
                draw_Vertex( (f32)NoRect.r, (f32)NoRect.b, Near );
                draw_Vertex( (f32)NoRect.r, (f32)NoRect.t, Near );

                // Neutral gray in between.
                draw_Color( NeutralColor );
                irect NeutralRect(  
                                    (s32)m_LeftMargin,
                                    (s32)(m_BottomMargin - (m_BottomMargin - m_TopMargin) * m_YesPercentage),
                                    (s32)m_RightMargin,
                                    (s32)(m_TopMargin + (m_BottomMargin - m_TopMargin) * m_NoPercentage - 1.0f)
                                 );

                draw_Vertex( (f32)NeutralRect.l, (f32)NeutralRect.t, Near );
                draw_Vertex( (f32)NeutralRect.l, (f32)NeutralRect.b, Near );
                draw_Vertex( (f32)NeutralRect.r, (f32)NeutralRect.b, Near );
                draw_Vertex( (f32)NeutralRect.r, (f32)NeutralRect.t, Near );
                draw_End();
            }

            // Draw the triangle ticks where the passing vote will be.
            {
                f32 PassPoint = m_TopMargin + (m_BottomMargin - m_TopMargin) * (1.0f - m_PercentNeeded);
              
                draw_Begin( DRAW_LINES, DRAW_2D|DRAW_NO_ZBUFFER );
                draw_Color( XCOLOR_YELLOW );
                draw_Vertex(  m_LeftMargin , PassPoint, 0.0f );
                draw_Vertex(  m_RightMargin, PassPoint, 0.0f );
                draw_End();
            }

            // Left Votes.
            {
                irect ScoreRect(  
                                    (s32)(m_RightMargin  + 12.0f),
                                    (s32)(m_BottomMargin - 8.0f),
                                    (s32)(m_RightMargin  + 12.0f),
                                    (s32)(m_BottomMargin - 8.0f) 
                               );
                xcolor Color = XCOLOR_GREEN;

                xwstring YesVotes(g_StringTableMgr("ui", "IDS_YES"));
                YesVotes += (const char*)xfs(":%d", m_YesVotes);
                const xwchar* pYesVotes = (const xwchar *)YesVotes;
                RenderLine( pYesVotes, ScoreRect, (u8)(255.0f*m_PercentOpen), Color, 1, ui_font::v_center|ui_font::h_left, TRUE, 0.0f );
            }

            // Right Votes.
            {
                irect ScoreRect( 
                                    (s32)(m_RightMargin  + 12.0f),
                                    (s32)(m_TopMargin    + 4.0f),
                                    (s32)(m_RightMargin  + 12.0f),
                                    (s32)(m_TopMargin    + 4.0f) 
                               );
                xcolor Color = XCOLOR_RED;

                xwstring NoVotes(g_StringTableMgr("ui", "IDS_NO"));
                NoVotes += (const char*)xfs( ":%d", m_NoVotes );
                const xwchar* pNoVotes = (const xwchar *)NoVotes;
                RenderLine( pNoVotes, ScoreRect, (u8)(255.0f*m_PercentOpen), Color, 1, ui_font::v_center|ui_font::h_left, TRUE, 0.0f );
            }  

            // Text.
            if( m_PercentOpen >= 1.0f )
            {
                static f32 VLPS = 0.0f;
                static f32 VTPS = 350.0f;
                static f32 VRPS = (f32)R;
                static f32 VBPS = 350.0f+15.0f;

                irect TextRect( 
                                (s32)VLPS, 
                                (s32)VTPS, 
                                (s32)VRPS, 
                                (s32)VBPS
                              );
                RenderLine( m_pVoteType, TextRect, 255, TextColor, 1, ui_font::h_center|ui_font::v_center, TRUE );
                TextRect.b += 15;
                TextRect.t += 15;
                RenderLine( m_pVoteSub,  TextRect, 255, TextColor, 1, ui_font::h_center|ui_font::v_center, TRUE );
            }
        }  
    }


#endif
}

//==============================================================================

void hud_vote::OnAdvanceLogic( player* pPlayer, f32 DeltaTime )
{
    (void)pPlayer;

    s32 Percent = 0;

    xbool bVoteInProgress = FALSE;
#ifndef X_EDITOR
    bVoteInProgress = GameMgr.GetVoteData( m_pVoteType, m_pVoteSub, m_YesVotes, m_NoVotes, m_MissingVotes, Percent );
#endif
    m_PercentNeeded = (f32)Percent / 100.0f;

    s32 TotalVotes = m_YesVotes + m_NoVotes;
    m_TargetPercentage = -1.0;

    if( TotalVotes )
    {
        m_TargetPercentage = ((f32)m_YesVotes / (f32)TotalVotes);
    }

    //
    // Vote Key.
    // 
    {
        // If we are on, quickly ramp up the opacity.
        if( m_bKeyOn )
        {
            // Have we hit a condition that means we should be turning off?
            if( !pPlayer->CanVote() )
            {
                m_KeyOpacity -= 4.0f * DeltaTime;
                if( m_KeyOpacity <= 0.0f )
                {
                    m_bKeyOn = FALSE;
                }
            }

            // If not, ramp up the opacity to 1.0 really quickly, 
            // assuming we aren't there already.
            else
            {
                m_KeyOpacity += 4.0f * DeltaTime;
            }

            m_KeyOpacity = MINMAX( 0.0f, m_KeyOpacity, 1.0f );
        }

        // Check for a condition to turn on the vote graphic.
        else if( pPlayer->CanVote() )
        {
            m_bKeyOn = TRUE;    
        }  
    }

    //
    // Vote Display.
    //
    {
        // If we are on, quickly ramp up the opacity.
        if( m_bTallyOn )
        {
            // Have we hit a condition that means we should be turning off?
            if( !bVoteInProgress )
            {
                if( m_PercentOpen > 0.0f ) 
                {
                    m_PercentOpen -= 4.0f * DeltaTime;
                }
                else if( m_PercentOnScreen > 0.0f )
                {
                    m_PercentOnScreen -= 8.0f * DeltaTime;
                }
                else
                {
                    m_bTallyOn = FALSE;
                }
            }

            // If not, ramp up the opacity to 1.0 really quickly, 
            // assuming we aren't there already.
            else
            {
                // If it's offscreen, slide it in.
                if( m_PercentOnScreen < 1.0f )
                {
                    m_YesPercentage = 0.0f;
                    m_NoPercentage  = 0.0f;
                    m_PercentOnScreen += 8.0f * DeltaTime;
                }

                // If it's still closed, open it up.
                else if( m_PercentOpen < 1.0f )
                {
                    m_YesPercentage = 0.0f;
                    m_NoPercentage  = 0.0f;
                    m_PercentOpen += 4.0f * DeltaTime;
                }

                // Have the bars chase their target percentages.
                else if( m_TargetPercentage >= 0.0f ) 
                {
                    f32 Foozle = (0.7f * DeltaTime);
                    f32 Constant = 0.0f;

                    m_YesPercentage = ((1.0f - Foozle) * m_YesPercentage) + (Foozle * m_TargetPercentage);
                    if( ABS( m_TargetPercentage - m_YesPercentage ) > 0.01f )
                    {
                        (m_YesPercentage > m_TargetPercentage) ? Constant = -0.01f : Constant = 0.01f;
                    }
                    m_YesPercentage += Constant;

                    //
                    // Independant bars.
                    //
                    if( m_NoPercentage + m_YesPercentage < 0.99f )
                    {
                        Constant = 0.0f;

                        m_NoPercentage  = ((1.0f - Foozle) * m_NoPercentage ) + (Foozle * (1.0f - m_TargetPercentage));
                        if( ABS( (1.0f - m_TargetPercentage) - m_NoPercentage) > 0.01f )
                        {
                            (m_NoPercentage > 1.0f - m_TargetPercentage) ? Constant = -0.01f : Constant = 0.01f;
                        }
                        m_NoPercentage += Constant;
                    }

                    // Cojoined bars.
                    else
                    {
                        m_NoPercentage = 1.0f - m_YesPercentage;
                    }
                    m_YesPercentage = MINMAX( 0.0f, m_YesPercentage, 1.0f );
                    m_NoPercentage  = MINMAX( 0.0f, m_NoPercentage,  1.0f );
                }

                m_TallyOpacity += 4.0f * DeltaTime;
            }

            m_TallyOpacity    = MINMAX( 0.0f, m_TallyOpacity,    1.0f );

            m_PercentOnScreen = MINMAX( 0.0f, m_PercentOnScreen, 1.0f );
            m_PercentOpen     = MINMAX( 0.0f, m_PercentOpen,     1.0f );
        }

        // Check for a condition to turn on the vote graphic.
        else if( bVoteInProgress )
        {
            m_bTallyOn = TRUE;    
        }
    }

    // Setup the edges for everything.
    {
#ifdef TARGET_XBOX
        static  f32 XPosS = 56.0f;
        static  f32 YPosS = 313.0f;
#else
        static  f32 XPosS = 36.0f;
        static  f32 YPosS = 289.0f;
#endif
        static  f32 WidthS = 85.0f;
        static  f32 HeightS = 10.0f;

        f32 XPos     = XPosS - (1.0f - m_PercentOnScreen) * 100.0f;
        f32 YPos     = YPosS;

        f32 Width    = WidthS * m_PercentOpen;
        f32 Height   = HeightS;

        m_LeftMargin   = XPos - (0.5f * Height);
        m_RightMargin  = XPos + (0.5f * Height);
        m_TopMargin    = YPos - (0.5f * Width);
        m_BottomMargin = YPos + (0.5f * Width);
    }
}

//==============================================================================

xbool hud_vote::IsActive( void )
{
    return (m_PercentOnScreen > 0.0f);
}

//==============================================================================