//==============================================================================
//
//  hud_TextRenderer.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "hud_Text.hpp"
#include "HudObject.hpp"

#include "Ui/ui_font.hpp"
#include "stringmgr\stringmgr.hpp"
#ifndef X_EDITOR
#include "Ui/ui_manager.hpp"
#endif

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

// These must be positive and in increasing order.  Shouldn't overlap,
// but there can be gaps if so desired.  Units are in seconds.
#define SCROLL_DOWN_START    1.9f
#define SCROLL_DOWN_END      2.0f

#define SLIDE_IN_START       0.0f
#define SLIDE_IN_END         0.0f

#define FLARE_START          2.5f
#define FLARE_END            3.0f


// These must be negative and in decreasing order.
#define SLIDE_OUT_START     -0.0f
#define SLIDE_OUT_END       -0.0f

#define SCROLL_UP_START     -2.0f
#define SCROLL_UP_END       -2.1f 

#define BASE_MSG_STAY       1.8f
#define MSG_STAY_PER_CHAR   0.04f
#define MAX_MSG_STAY        8.0f

#define MAX_CHARS_SOFT      240     // Point after which to set message time left to SLIDE_OUT_START because we're getting too much to display.
#define MAX_CHARS_HARD      300     // Point after which to set message inuse field to FALSE.
#define MAX_LINES_SOFT      5      // Number of lines after which we start immediately fading.
#define MAX_LINES_HARD      5      // Number of lines after which we make any disappear immediately.


//==============================================================================
//  FUNCTIONS
//==============================================================================



//==============================================================================

hud_text::hud_text( void )
{
    m_NumDisplay            =  5;
    m_CursorPos             =  0.0f;
    m_TopLine               =  0;
    m_NumGoals              =  0;

    m_MaxTextWidth          = 100;

    m_TopGoal               = 0;

    m_TextBoxRect.l         = 0;
    m_TextBoxRect.t         = 0;
    m_TextBoxRect.r         = 0;
    m_TextBoxRect.b         = 0;

    m_PosX                  = 50;
    m_PosY                  = 50;

    s32 i;
    for( i = 0; i < MAX_QUEUE; i++ ) 
    {
       m_Lines[ i ].Reset();

    }  

    for( i = 0; i < MAX_GOALS; i++ ) 
    {
        m_Goals[ i ].Reset();
    } 

    m_Bonus.Reset();
    m_WeaponInfo.Reset();

    m_TextBoxRectState = TEXT_BOX_STATE_CLOSED;
    m_PercentOpen = 0.0f;
    //m_IncomingBmp.SetName(PRELOAD_FILE("HUD_Campaign_incoming.xbmp"));
}

//==============================================================================

// I'm using this instead of a straight circular queue like the messages use
// because goals can come and go as they please, leaving gaps in any queue.
s32 hud_text::GetIthGoal( s32 Index )
{
    s32 Last = -1;

    s32 i;    
    for( i = 0; i <= Index; i++ )
    {
        s32 Current = -1;
        
        s32 j;
        for( j = 0; j < MAX_GOALS; j++ )
        {
            if( m_Goals[ j ].InUse )
            {
                
                if( (Last == -1) || (m_Goals[ j ].SeqNum > m_Goals[ Last ].SeqNum) )
                {
                    if( (Current == -1) ||  (m_Goals[ j ].SeqNum < m_Goals[ Current ].SeqNum) ) 
                    {
                        Current = j;
                    }
                }

            }
        }
        Last = Current;
    }
    
    return Last;
}

//==============================================================================

hud_text::~hud_text( void )
{
}

//==============================================================================
s32 s_wi_t = 340;
s32 s_wi_b = 360;
s32 s_wi_l = 400;
s32 s_wi_r = 500;

static xcolor CHAT_RECT_COLOR_GREEN = xcolor( 0,31,0,127 );
static xbool  CHAT_DROP_SHADOW = FALSE;
static xcolor TEXT_BOX_BACK_COLOR = xcolor( 0,0,0,80);
#define CHAT_BOX_LINEFEED   17
void hud_text::OnRender( player* pPlayer )
{
#ifndef X_EDITOR 
    (void)pPlayer;

    // Get the hardware resolution for use later
    s32 XRes;
    s32 YRes;
    eng_GetRes( XRes, YRes );

    ui_font* pFont      = g_UiMgr->GetFont( "small" );    

    irect iRect;

    s32 TopFullLine     = (s32)(m_CursorPos);
    f32 Offset          = m_CursorPos - TopFullLine;
    
    u8 Alpha = 255;
    u8 RectAlpha = 180;

    s32 i;

    f32 CurrentOffset = m_YPos;

    //
    // Render the goals
    //
    for( i = 0; i < m_NumGoals; i++ )
    {
        s32 iGoal = GetIthGoal( i );

        if( m_Goals[ iGoal ].InUse )
        {
            f32 PercentOnScreen     = 0.0f;
            xwchar* pLine           = m_Goals[ iGoal ].Text;
            
            PercentOnScreen = 1.0f;

            //Find the ith goal
            iRect.t = (s32)CurrentOffset;
            iRect.b = (s32)CurrentOffset + pFont->TextHeight(pLine);//CHAT_BOX_LINEFEED;
            iRect.l = (s32)0;
            iRect.r = (s32)0;

            CurrentOffset  += pFont->TextHeight(pLine);//CHAT_BOX_LINEFEED;

            iRect.l = (s32)m_XPos;
            iRect.r = (s32)pFont->TextWidth(pLine)+(s32)m_XPos+8;

            if( m_Goals[iGoal].KeyingPos < 400 )
            {
                m_Goals[iGoal].KeyingPos+=10;

                // scissor region
#ifdef TARGET_PS2
                gsreg_Begin( 1 );
                gsreg_SetScissor(   
                    /*iRect.l*/0, 
                    iRect.t,
                    iRect.l+(s32)m_Goals[iGoal].KeyingPos, 
                    iRect.b
                    );
                gsreg_End();
#endif

#ifdef TARGET_XBOX
                D3DRECT Rects[1];
                Rects[0].x1 = /*iRect.l*/0;
                Rects[0].y1 = iRect.t;
                Rects[0].x2 = iRect.l+(s32)m_Goals[iGoal].KeyingPos;
                Rects[0].y2 = iRect.b;
                g_pd3dDevice->SetScissors( 1,FALSE,Rects );
#endif
            }

            xcolor TextColor = XCOLOR_WHITE;
            RectAlpha = CHAT_RECT_COLOR_GREEN.A;
            Alpha = (u8)(255 * (m_Goals[ iGoal ].Time > 0.25f ? 1.0f : (m_Goals[ iGoal ].Time * 4.0f)));
            RectAlpha = (u8)(CHAT_RECT_COLOR_GREEN.A * (m_Goals[ iGoal ].Time > 0.25f ? 1.0f : (m_Goals[ iGoal ].Time * 4.0f)));

            // full rect area

            xcolor greenColor;
            greenColor = CHAT_RECT_COLOR_GREEN;
            greenColor.A = RectAlpha;

            draw_Rect( iRect, greenColor, FALSE);

            // blended end
            irect iFadeOut;
            iFadeOut.l = iRect.r;
            iFadeOut.r = iFadeOut.l+8;
            iFadeOut.t = iRect.t;
            iFadeOut.b = iRect.b;
            draw_GouraudRect(iFadeOut,greenColor,greenColor,xcolor(0,31,0,0),xcolor(0,31,0,0),FALSE);

            // blended start
            irect iFadeIn;
            iFadeIn.l = iRect.l-8;
            iFadeIn.r = iRect.l;
            iFadeIn.t = iRect.t;
            iFadeIn.b = iRect.b;
            draw_GouraudRect(iFadeIn,xcolor(0,180,0,80),xcolor(0,180,0,80),greenColor,greenColor,FALSE);

            // Bright side line.
            irect rLine;
            rLine.Set(iRect.l-8,iRect.t,iRect.l-7,iRect.b);
            draw_Rect(rLine,g_HudColor,TRUE);

            // text
            iRect.t-=1;
            iRect.b-=1;
            RenderLine( pLine, iRect, Alpha, TextColor, 1, ui_font::h_left|ui_font::v_top, CHAT_DROP_SHADOW );

            // restore scissor region
#ifdef TARGET_PS2
            gsreg_Begin( 1 );
            gsreg_SetScissor( 0, 0, XRes, YRes );
            gsreg_End();
#endif

#ifdef TARGET_XBOX
            // ensure we're not using a shrunken viewport
            g_pd3dDevice->SetViewport( NULL );
#endif
        }
    }

    iRect.l = (s32)m_XPos+1;
    iRect.r = (s32)300;

    CurrentOffset += 0.0f;

    i = TopFullLine;
    if( Offset > 0.0f )
    {
        i               += 1;
        CurrentOffset   -= (1.0f - Offset) * pFont->TextHeight( m_Lines[ i % MAX_QUEUE ].Text );
    }


    //
    // Render the messages to the display
    //
    for( ; 
        (i >= 0) && (i > (TopFullLine - m_NumDisplay)); 
        i-- )
    {
        s32 QueueIndex = i % MAX_QUEUE;

        const xwchar* pLine = m_Lines[ QueueIndex ].Text;

        iRect.t = (s32)CurrentOffset;
        iRect.b = iRect.t + pFont->TextHeight(pLine);//CHAT_BOX_LINEFEED;    

        iRect.l = (s32)m_XPos;
        iRect.r = (s32)pFont->TextWidth(pLine)+(s32)m_XPos+8;

        // Blank line?
        if( iRect.r-8 <= iRect.l )
        {
            iRect.r = iRect.l;
        }
        else
        {       
            CurrentOffset += pFont->TextHeight(pLine);//CHAT_BOX_LINEFEED;

            RectAlpha = CHAT_RECT_COLOR_GREEN.A;

            // Alpha for top line as it scrolls in.
            if( i == (TopFullLine + 1) )
            {
                Alpha = (u8)(255 * Offset);
                RectAlpha = (u8)(CHAT_RECT_COLOR_GREEN.A * Offset);
            }

            // Alpha for bottom line that is potentially timing out or scrolling out.
            else if( i == (TopFullLine - m_NumDisplay + 1) ) 
            {
                u8 Option1 = (u8)(255 * (1.0f - Offset));
                u8 Option2 = (u8)(255 * (m_Lines[ QueueIndex ].Time > 0.25f ? 1.0f : (m_Lines[ QueueIndex ].Time * 4.0f)));

                Alpha = x_min(Option1, Option2);

                Option1 = (u8)(CHAT_RECT_COLOR_GREEN.A * (1.0f - Offset));
                Option2 = (u8)(CHAT_RECT_COLOR_GREEN.A * (m_Lines[ QueueIndex ].Time > 0.25f ? 1.0f : (m_Lines[ QueueIndex ].Time * 4.0f)));

                RectAlpha = x_min(Option1, Option2);
            }

            // Alpha for all other lines that could potentially be timing out.
            else
            {
                Alpha =     (u8)(255 * (m_Lines[ QueueIndex ].Time > 0.25f ? 1.0f : (m_Lines[ QueueIndex ].Time * 4.0f)));
                RectAlpha = (u8)(CHAT_RECT_COLOR_GREEN.A * (m_Lines[ QueueIndex ].Time > 0.25f ? 1.0f : (m_Lines[ QueueIndex ].Time * 4.0f)));
            }

            xcolor TextColor = XCOLOR_WHITE;
           
            xcolor greenColor = CHAT_RECT_COLOR_GREEN;
            greenColor.A = RectAlpha;

            // full rect area
            draw_Rect( iRect, greenColor, FALSE);

            // blended end
            irect iFadeOut;
            iFadeOut.l = iRect.r;
            iFadeOut.r = iFadeOut.l+8;
            iFadeOut.t = iRect.t;
            iFadeOut.b = iRect.b;
            draw_GouraudRect(iFadeOut,greenColor,greenColor,xcolor(0,31,0,0),xcolor(0,31,0,0),FALSE);

            // blended Start
            irect iFadeIn;
            iFadeIn.l = iRect.l-8;
            iFadeIn.r = iRect.l;
            iFadeIn.t = iRect.t;
            iFadeIn.b = iRect.b;
            draw_GouraudRect(iFadeIn,xcolor(0,180,0,80),xcolor(0,180,0,80),greenColor,greenColor,FALSE);

            // Bright side line.
            irect rLine;
            rLine.Set(iRect.l-8,iRect.t,iRect.l-7,iRect.b);
            draw_Rect(rLine,g_HudColor,TRUE);

            // text
            iRect.t-=1;
            iRect.b-=1;
            RenderLine( pLine, iRect, Alpha, TextColor, 1, ui_font::h_left|ui_font::v_top, CHAT_DROP_SHADOW );
        }
    }
    
    //
    // Render the bonus text
    //
    if( m_Bonus.InUse )
    {
        static irect BonusRect = irect(0,148,512,448);

        rect ViewDimensions;
        view& rView = pPlayer->GetView();
        rView.GetViewport( ViewDimensions );
        BonusRect.r = (s32)ViewDimensions.GetWidth();
        BonusRect.b = (s32)ViewDimensions.GetHeight();
        
        xcolor TextColor = XCOLOR_YELLOW;
        RenderLine( m_Bonus.Text, BonusRect, 255, TextColor, 1, ui_font::h_center|ui_font::v_top, TRUE );
    }

    // Render Weapon Info Text
    if( m_WeaponInfo.InUse )
    {
        iRect.t = s_wi_t; // 340
        iRect.b = s_wi_b; // 360
        iRect.l = s_wi_l; // 400
        iRect.r = s_wi_r; // 500

        xcolor TextColor = XCOLOR_BLUE;
        RenderLine( m_WeaponInfo.Text, iRect, 255, TextColor, 0, ui_font::h_left|ui_font::v_bottom, TRUE );
    }

#endif
}

//==============================================================================

inline void hud_text::AddLinesAndChars( s32& NumLines, s32& NumChars, xwchar* pLine )
{
    if( *pLine == 0 )
        return;
    
    {    
        NumLines++;
        while( *pLine != 0 )
        {
            // Linebreak?
            if( *pLine == '\n' )
            {
                NumLines++;
            }

            // Normal character?
            else if( (*pLine & 0xFF00) != 0xFF00 ) 
            {
                NumChars++;
            }

            // Skip color codes.
            else
            {
                pLine++;
            }

            pLine++;
        }
    }
}

//==============================================================================
                                       
inline void hud_text::ClearAllBelow( s32 MsgIndex )
{
    while( (MsgIndex >= 0) && ( (MsgIndex % MAX_QUEUE != m_TopLine % MAX_QUEUE) || (MsgIndex == m_TopLine)) )
    {
        m_Lines[ MsgIndex % MAX_QUEUE ].Time  = 0.0f;
        m_Lines[ MsgIndex % MAX_QUEUE ].InUse = FALSE;

        m_Lines[ MsgIndex % MAX_QUEUE ].Reset();

        MsgIndex--;
    }
}

//==============================================================================

static s32  TEXT_BOX_POS_L = 16;
static s32  TEXT_BOX_POS_T = 16;
static s32  TEXT_BOX_POS_R = 350;
static s32  TEXT_BOX_LINE_HEIGHT = CHAT_BOX_LINEFEED;
static f32  TEXT_BOX_OPEN_SPEED = 6.0f;

void hud_text::OnAdvanceLogic( player* pPlayer, f32 DeltaTime )
{
    (void)pPlayer;

    s32 LinesDisplayed  = 0; // How many lines we're potentially displaying on the screen.
    s32 CharsDisplayed  = 0; // How many characters we're displaying on the screen.

    s32 textWidth = 0;

#ifndef X_EDITOR
    ui_font* pFont      = g_UiMgr->GetFont( "small" ); 
#endif

    //
    // Do all goal sliding here
    //
    for( s32 i = 0; i < MAX_GOALS; i++ )
    {
        // Is this goal currently active?
        if( m_Goals[ i ].InUse )
        {
            AddLinesAndChars( LinesDisplayed, CharsDisplayed, m_Goals[ i ].Text );
#ifndef X_EDITOR   
            textWidth = MAX(textWidth, pFont->TextWidth(m_Goals[i].Text));
#endif
            m_Goals[ i ].Time -= DeltaTime;

            if( m_Goals[ i ].Time <= 0.0f )
            {
                m_Goals[ i ].Reset();
                m_NumGoals--;
            }
        }
    }

    //
    // Do normal message logic here.
    //
    {
        // Scroll the messages some if they need scrolling.
        const f32 ScrollSpeed = 0.8f;
        if( m_CursorPos < m_TopLine )
        {
            f32 Scroll = (5.0f + (m_TopLine - m_CursorPos)) * DeltaTime * ScrollSpeed;

            m_CursorPos += Scroll;

            if( m_CursorPos > m_TopLine )
            {
                m_CursorPos = (f32)m_TopLine;
            }
        }

        // This makes sure that only m_NumDisplay messages are ever visible.
        // I'm using m_CursorPos because that allows an extra message to be on the screen
        // a new one is in the process of scrolling in.
        ClearAllBelow( (s32)m_CursorPos - m_NumDisplay );

        // Loop through all the messages and make sure we never have too much text 
        // on the screen or take up too much space, and time out the last visible message.
        for( s32 i = m_TopLine; i >= 0; i-- )
        {
            // Add this line to the current running tally of messages.
            AddLinesAndChars( LinesDisplayed, CharsDisplayed, m_Lines[ i  % MAX_QUEUE ].Text );

#ifndef X_EDITOR   
            textWidth = MAX(textWidth, pFont->TextWidth(m_Lines[ i  % MAX_QUEUE ].Text));
#endif

            // Have we gone over the hard limit?  Then immediately remove this line.
            if( (LinesDisplayed > MAX_LINES_HARD) ||
                (CharsDisplayed > MAX_CHARS_HARD ) )
            {
                m_Lines[ i % MAX_QUEUE ].Time  = 0.0f;
                ClearAllBelow( i );
                break;
            }
            // If we've gone over the soft limit, tell this line to hurry up and die.
            else if(    (LinesDisplayed > MAX_LINES_SOFT) ||
                (CharsDisplayed > MAX_CHARS_SOFT) )
            {
                m_Lines[ i % MAX_QUEUE ].Time = x_min( 0.25f, m_Lines[ i % MAX_QUEUE ].Time );
                ClearAllBelow( i - 1 );
            }

            // If the next one is dead, this one should be dying.
            if( (!m_Lines[ (i - 1) % MAX_QUEUE ].InUse) )
            {
                m_Lines[ i % MAX_QUEUE ].Time -= DeltaTime;

                if( m_Lines[ i % MAX_QUEUE ].Time <= 0.0f )
                {
                    ClearAllBelow( i );
                }
                break;
            } 
        }
    }

    //
    // Bonus logic
    //
    if( m_Bonus.InUse )
    {
        m_Bonus.Time -= DeltaTime;
        if( m_Bonus.Time < 0 )
        {
            m_Bonus.InUse = FALSE;
        }
    }

    if( m_WeaponInfo.InUse )
    {
        m_WeaponInfo.Time -= DeltaTime;
        if( m_WeaponInfo.Time < 0 )
        {
            m_WeaponInfo.InUse = FALSE;
        }
    }

    // setup the text box rect that draws around the text lines.
    m_TextBoxRect.l = TEXT_BOX_POS_L;
    m_TextBoxRect.t = TEXT_BOX_POS_T;

    if( LinesDisplayed )
    {
        m_TextBoxRect.b = (TEXT_BOX_LINE_HEIGHT * (LinesDisplayed+1));
        m_TextBoxRect.r = TEXT_BOX_POS_R;
        m_RenderTextBoxRect = TRUE;

        switch( m_TextBoxRectState )
        {
            case TEXT_BOX_STATE_CLOSED:
                // Text box is closed so start to open it.
                m_TextBoxRectState = TEXT_BOX_STATE_OPENING;
                m_PercentOpen = 0.0f;
            break;

            case TEXT_BOX_STATE_OPENING:
                // Calc the irect here for opening.
                m_PercentOpen += (DeltaTime * TEXT_BOX_OPEN_SPEED);

                if( m_PercentOpen >= 1.0f )
                    m_TextBoxRectState = TEXT_BOX_STATE_OPEN;
            break;

            case TEXT_BOX_STATE_OPEN:
                // do nothing but render the box open.
                m_PercentOpen = 1.0f;
            break;

            case TEXT_BOX_STATE_CLOSEING:
                // If closeing then set to open again.
                m_TextBoxRectState = TEXT_BOX_STATE_OPENING;
            break;

        }
    }
    else
    {
        m_RenderTextBoxRect = FALSE;
        
        switch( m_TextBoxRectState )
        {
            case TEXT_BOX_STATE_OPEN:
            case TEXT_BOX_STATE_OPENING:
                m_TextBoxRectState = TEXT_BOX_STATE_CLOSEING;
            break;

            case TEXT_BOX_STATE_CLOSEING:
                // Start Closing the box
                m_PercentOpen -= (DeltaTime * TEXT_BOX_OPEN_SPEED);
                if( m_PercentOpen <= 0 )
                    m_TextBoxRectState = TEXT_BOX_STATE_CLOSED;
            break;

            case TEXT_BOX_STATE_CLOSED:
                // Do nothing we are closed.
                m_PercentOpen = 0.0f;
            break;
        }
    }

    m_TextBoxRect.b = MAX( (s32)(m_TextBoxRect.b * m_PercentOpen), TEXT_BOX_POS_T)+8;
    m_TextBoxRect.r = (s32)((textWidth+8+16) * m_PercentOpen)+TEXT_BOX_POS_L;

    if( m_TextBoxRect.l > m_TextBoxRect.r )
        m_TextBoxRect.l = m_TextBoxRect.r;
    if( m_TextBoxRect.r < m_TextBoxRect.l )
        m_TextBoxRect.r = m_TextBoxRect.l;
    if( m_TextBoxRect.t > m_TextBoxRect.b )
        m_TextBoxRect.t = m_TextBoxRect.b;
    if( m_TextBoxRect.b < m_TextBoxRect.t )
        m_TextBoxRect.b = m_TextBoxRect.t;
}

//==============================================================================

void hud_text::AddLine( const xwchar* pLine )
{
(void)pLine;
#ifndef X_EDITOR
    m_TopLine++;

    irect iRect(0, 0, m_MaxTextWidth, 400);

    ui_font* pFont      = g_UiMgr->GetFont( "small" );
    xwstring WrappedLine;
    pFont->TextWrap( pLine, iRect, WrappedLine );
    const xwchar* pWrappedLine = (const xwchar*)WrappedLine;

    s32 i = -1;
    do {
        i++;
        m_Lines[ m_TopLine % MAX_QUEUE ].Text[ i ] = pWrappedLine[ i ];
    } while( pWrappedLine[ i ] != 0 );
    
    m_Lines[ m_TopLine % MAX_QUEUE ].InUse = FALSE;

    f32 StayTime = BASE_MSG_STAY + (i * MSG_STAY_PER_CHAR);

    if( StayTime > MAX_MSG_STAY )
    {
        StayTime = MAX_MSG_STAY;
    }

    m_Lines[ m_TopLine % MAX_QUEUE ].InUse = TRUE;
    m_Lines[ m_TopLine % MAX_QUEUE ].Time  = StayTime;
#endif
}

//==============================================================================

void hud_text::AddGoal( s32 GoalID, const xwchar* pGoal, f32 Time )
{
    (void)GoalID;
    (void)pGoal;
    (void)Time;
#ifndef X_EDITOR
    s32 i;

    irect iRect(0, 0, m_MaxTextWidth, 400);

    ui_font* pFont      = g_UiMgr->GetFont( "small" );
    xwstring WrappedLine;
    pFont->TextWrap( pGoal, iRect, WrappedLine );
    const xwchar* pWrappedLine = (const xwchar*)WrappedLine;

    for( i = 0; i < MAX_GOALS; i++ )
    {
        if( !m_Goals[ i ].InUse )
        {
            s32 j = -1;
            do {
                j++;
                m_Goals[ i ].Text[ j ] = pWrappedLine[ j ];
            } while( pWrappedLine[ j ] != 0 );

            // Play Sound here?
            g_AudioMgr.Play("HUD_Text_Alert", TRUE );

            m_Goals[ i ].ScrollState    = 1;
            m_Goals[ i ].SeqNum         = ++m_TopGoal;
            m_Goals[ i ].InUse          = TRUE;
            m_Goals[ i ].Time           = Time;
            m_Goals[ i ].GoalID         = GoalID;
            m_Goals[ i ].KeyingPos      = 0;

            m_NumGoals++;

            return;
        }
    }
#endif
}

//==============================================================================

void hud_text::UpdateGoal( s32 GoalID, xbool Enabled, const xwchar* pGoal, f32 Time )
{
    s32 i;
    for( i = 0; i < MAX_GOALS; i++ )
    {
        // This means that whoever created this goal does not want to manipulate it
        // in the future and are planning on it fading on its own.
        if( GoalID == -1 )
        {
            if( Enabled )
            {
                break;
            }
            else
            {
                return; // You can't clear a goal created with -1
            }
        }

        // Updating or deleting:
        if( GoalID == m_Goals[ i ].GoalID && m_Goals[ i ].InUse )
        {
            // If the time has changed, set that
            if( Time >= 0 )
            {
                m_Goals[ i ].Time = Time;
            }
            
            // Copy the new text over
            s32 j = -1;
            do {
                j++;
                m_Goals[ i ].Text[ j ] = pGoal[ j ];
            } while( pGoal[ j ] != 0 );

            //if( m_Goals[ i ].ScrollState > FLARE_START )
            //{
            //    m_Goals[ i ].ScrollState    = FLARE_START;
            //}
            //else
            //{
            //    m_Goals[ i ].ScrollState    = SCROLL_DOWN_START;
            //}

            //if( !Enabled )  
            //{
            //    m_Goals[ i ].ScrollState    = SLIDE_OUT_START;
            //}
            return;
        }
    }

    // Creating:
    if( Enabled )
    {
        AddGoal( GoalID, pGoal, Time );
    }
}

//==============================================================================

void hud_text::SetBonus( const xwchar* pBonus, f32 Time )
{
    m_Bonus.Time  = Time;
    m_Bonus.InUse = TRUE;
    
    s32 j = -1;
    do {
        j++;
        m_Bonus.Text[ j ] = pBonus[ j ];
    } while( pBonus[ j ] != 0 );
}

//==============================================================================

void hud_text::SetWeaponInfo( const xwchar* pWeaponInfo, f32 Time )
{
    m_WeaponInfo.Time  = Time;
    m_WeaponInfo.InUse = TRUE;

    s32 j = -1;
    do {
        j++;
        m_WeaponInfo.Text[ j ] = pWeaponInfo[ j ];
    } while( pWeaponInfo[ j ] != 0 );

}



//==============================================================================