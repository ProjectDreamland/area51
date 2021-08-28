//==============================================================================
//
//  hud_Player.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_PLAYER_HPP
#define HUD_PLAYER_HPP

#include "hud_Renderable.hpp"
#include "hud_Text.hpp"
#include "hud_Health.hpp"
#include "hud_Damage.hpp"
#include "hud_Sniper.hpp"
#include "hud_Ammo.hpp"
#include "hud_Reticle.hpp"
#include "hud_MutantVision.hpp"
#include "hud_ContagiousVision.hpp"
#include "hud_Icon.hpp"
#include "hud_InfoBox.hpp"
#include "hud_Vote.hpp"
#include "hud_Scanner.hpp"

enum hud_element
{
    HUD_ELEMENT_MUTANT_VISION=0,
    HUD_ELEMENT_CONTAGIOUS_VISION,
    HUD_ELEMENT_SNIPER,
    HUD_ELEMENT_AMMO_BAR,
    HUD_ELEMENT_TEXT_BOX,
    HUD_ELEMENT_RETICLE,
    HUD_ELEMENT_DAMAGE,
    HUD_ELEMENT_ICON,
    HUD_ELEMENT_INFO_BOX,
    HUD_ELEMENT_HEALTH_BAR,
    HUD_ELEMENT_VOTE,
    HUD_ELEMENT_SCANNER,    

    HUD_ELEMENT_NUM_ELEMENTS,       // must be last!
};

class player_hud 
{
    enum icon
    {
        HUD_NOTIFICATION_NONE,
        HUD_NOTIFICATION_FRIEND_REQ,
        HUD_NOTIFICATION_GAME_INVITE,
    };

public:
                        player_hud              ( void );
    void                OnRender                ( void );
    void                OnAdvanceLogic          ( f32 DeltaTime );
    void                TriggerAutoSaveON       ( void ) {};
    void                TriggerAutoSaveOFF      ( void ) {};
    void                UpdateMPIcons           ( f32 DeltaTime, player* pPlayer );

    xbool               m_Active;
    s32                 m_PlayerSlot;
    s32                 m_LocalSlot;
    s32                 m_NetSlot;

    f32                 m_XPos;
    f32                 m_YPos;
    f32                 m_Width;
    f32                 m_Height;
    f32                 m_CenterX;
    f32                 m_CenterY;

    f32                 m_HowTextTimer;
    s32                 m_HowLastMode;
    u8                  m_HowTextAlpha;

    s32                 m_Type;

    rhandle<xbitmap>    m_HowTeamBMP;
    rhandle<xbitmap>    m_HowLocalBMP;
    rhandle<xbitmap>    m_HowGlobalBMP;
    rhandle<xbitmap>    m_WhoBMP;
    rhandle<xbitmap>    m_FriendReqBMP;
    rhandle<xbitmap>    m_GameInviteBMP;
    rhandle<xbitmap>    m_AutoSave;

    icon                m_NotificationIcon;
    xbool               m_ReceivedFriendReq;
    xbool               m_ReceivedGameInvite;
    xbool               m_AutoSaveing;

    s32                 m_InvitationAlpha;
    s32                 m_AutoSaveingAlpha;
    s32                 m_InvitationState;
    s32                 m_AutoSaveState;
    f32                 m_InvitationHoldTime;
    f32                 m_AutoSaveHoldTime;


    hud_text                m_Text; 
    hud_health              m_Health;
    hud_sniper              m_Sniper;
    hud_damage              m_Damage;
    hud_reticle             m_Reticle;
    hud_ammo                m_Ammo;
    hud_mutant_vision       m_MutantVision;
    hud_icon                m_Icon;
    hud_info_box            m_InfoBox;
    hud_vote                m_Vote;
    hud_scanner             m_Scanner;
    hud_contagious_vision   m_ContagiousVision;

    hud_renderable*     m_HudComponents[ HUD_ELEMENT_NUM_ELEMENTS ];

    // QUESTION: Is hud_player the best place for this?!?
    // We want the hud to add icons for enemies an allies in a multi-player game.
    // Before this was done in player, actor, and hud_reticle. This gets it all
    // in one place, but is it the right place?!?
    // Also, this brings a question of whether or not this is really part of
    // logic or render? The AddIcon calls were previously in both logic and
    // render. So what should it be?
    f32                 m_EnemySightDelay[32];
    f32                 m_MPIconOpacities[32];
};

#endif
