//==============================================================================
//
//  hud_TextRenderer.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_TEXTRENDERER_HPP
#define HUD_TEXTRENDERER_HPP

//==============================================================================
// INCLUDES
//==============================================================================

#include "x_types.hpp"

#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"
#include "Objects\Player.hpp"

#include "hud_Renderable.hpp"

//==============================================================================
// DEFINES
//==============================================================================

#define SLOWEST_SCROLL_SPEED 0.04f
#define MAX_DISPLAY_LENGTH 256
#define MAX_QUEUE 32
#define MAX_GOALS 5

//---------------------------------------------------------------------
    
struct text_display
{
    xwchar  Text[ MAX_DISPLAY_LENGTH ];
    f32     Time;
    xbool     InUse;

    f32     ScrollState;
    f32     KeyingPos;
    s32     SeqNum;
    s32     GoalID;

    inline void Reset( void )
    {
        InUse          = FALSE;
        Time           = -1.0;
        Text[0]        = 0;
        ScrollState    = 0.0f;
        SeqNum         = 0;
        KeyingPos      = 0;
    }
};

//==============================================================================
// CLASS
//==============================================================================

class hud_text : public hud_renderable
{
//------------------------------------------------------------------------------
// Public Functions
public:
                    hud_text        ( void );
    virtual        ~hud_text        ( void );

    virtual void    OnRender        ( player*       pPlayer );
    virtual void    OnAdvanceLogic  ( player*       pPlayer, f32 DeltaTime );

            void    AddLine         ( const xwchar* pLine );

            void    UpdateGoal      ( s32 GoalSeq, xbool enabled, const xwchar* pGoal, f32 Time = -1.0f );

            void    SetBonus        ( const xwchar* pBonus, f32 Time );
            void    SetWeaponInfo   ( const xwchar* pWeaponInfo, f32 Time );

            s32     GetIthGoal      ( s32 Index );

            void    SetMaxWidth     ( s32 MaxWidth ) { m_MaxTextWidth = MaxWidth; }

//------------------------------------------------------------------------------
// Private Functions
private:
            void    AddGoal         ( s32 GoalSeq, const xwchar* pGoal, f32 Time = -1.0f );
            void    ClearGoal       ( s32 GoalSeq );
    inline  void    AddLinesAndChars( s32& NumLines, s32& NumChars, xwchar* pLine );
    inline  void    ClearAllBelow   ( s32 MsgIndex );



//------------------------------------------------------------------------------
// Public Storage
public:   
    xcolor  m_TextColor;
    xcolor  m_TextShadowColor;
    f32     m_TextBoxTimeTillFade;
    f32     m_TextBoxFadeDownTime;

    s32     m_MaxTextWidth;

    s32     m_TopGoal;
    s32     m_NumGoals;             // How many goals should be displayed.
    f32     m_GoalSpace;            // Indicates how much space is alloted for the goals, always follows m_NumGoals.

//------------------------------------------------------------------------------
// Private Storage
private:   
    s32 m_PosX;
    s32 m_PosY;
    s32 m_NumDisplay;

    rhandle<xbitmap>            m_TextBoxBitmap;

    s32 m_TopLine;
    f32 m_CursorPos; // Position in the buffer of text messages.  Allows fractional lines.

    text_display m_Lines[ MAX_QUEUE ];
    text_display m_Goals[ MAX_GOALS ];
    text_display m_Bonus;
    text_display m_WeaponInfo;

    enum TEXT_BOX_RECT_STATES
    {
        TEXT_BOX_STATE_CLOSED,
        TEXT_BOX_STATE_OPENING,
        TEXT_BOX_STATE_OPEN,
        TEXT_BOX_STATE_CLOSEING,
        NUM_TEXT_BOX_STATES
    };
    xbool                       m_RenderTextBoxRect;
    irect                       m_TextBoxRect;
    s32                         m_TextBoxRectState;
    f32                         m_PercentOpen;
    rhandle<xbitmap>            m_IncomingBmp;

};

#endif

