//==============================================================================
//
//  hud_Health.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_HEALTHBAR_HPP
#define HUD_HEALTHBAR_HPP

//==============================================================================
// INCLUDES
//==============================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"
#include "Objects\Player.hpp"

#include "hud_Renderable.hpp"

//==============================================================================
// CLASS
//==============================================================================

struct status_bar
{
    f32     m_CurrentVal;
    f32     m_MaxVal;
    f32     m_TargetVal; 
    f32     m_StartVal;  
    f32     m_StunTime;    

    f32     m_CurrentDisplayPercentage;

    f32     m_CurrentPhase;
    f32     m_PhaseSign;

    xbool   m_bFlip;

    xcolor  m_OuterColor;
    xcolor  m_InnerColor;

    xbool   m_FirstPass;
};

enum bar_type
{
    MUTAGEN,
    HEALTH,
    FLASHLIGHT,
    NUM_BARS
};

class hud_health : public hud_renderable
{
public:
                    hud_health      ( void );
    virtual        ~hud_health      ( void );

            void    Init            ( void );
            void    Kill            ( void );

    virtual void    OnRender        ( player* pPlayer );
    virtual void    OnAdvanceLogic  ( player* pPlayer, f32 DeltaTime );
    virtual xbool   OnProperty      ( prop_query& rPropQuery );
    virtual void    OnEnumProp      ( prop_enum&  List );

            void    RenderEKG       ( void );
            void    AdvanceEKG      ( player* pPlayer, f32 DeltaTime );

            void    RenderBar       ( bar_type BarNum  );
            void    AdvanceBar      ( player* pPlayer, f32 DeltaTime, bar_type BarNum );

            void    RenderMutationTakeOver ( void );
            void    UpdateMutationTakeOver ( player* pPlayer, f32 DeltaTime );

            void    RenderVolScanner( void );

            void    RenderEQBar     ( s32 EQBar, s32 AmountPercent );
            void    UpdateEQBar     ( player* pPlayer, f32 DeltaTime );



//------------------------------------------------------------------------------
// Public Storage
public:
    static rhandle<xbitmap>     m_OuterBmp;
    static rhandle<xbitmap>     m_MutationBmp;
    static rhandle<xbitmap>     m_InnerBmp;
    static s32                  m_Instances;

    rhandle<xbitmap>            m_FlashLightBmp;
    rhandle<xbitmap>            m_StaticBmp1;
    rhandle<xbitmap>            m_StaticBmp2;
    rhandle<xbitmap>            m_StaticBmp3;
    rhandle<xbitmap>            m_VolScanBmp;

    // MultiPlayer?
    xbool                       m_MultiPlayerHud;
    xbool                       m_HudInited;

    // EKG

    #define     NUM_DOTS        27           
    f32                         m_EKGSpeed;        
    f32                         m_BlipCount;
    f32                         m_CountAtSwitch;
    vector2                     m_EKGArray[ NUM_DOTS ];
    xcolor                      m_LineColor;
    s32                         m_BeatArray;
    s32                         m_OldBeatArray;

    xbool                       m_FlashLightActive;
    s32                         m_staticFrame;

    f32                         m_EQBarUpdate;
    s32                         m_EQBarPeaks[3];
    s32                         m_EQBar1_Value;
    s32                         m_EQBar2_Value;
    s32                         m_EQBar3_Value;
    xbool                       m_EQBarPeakUpdate;

    status_bar m_Bars[ NUM_BARS ];

    // MUTATION BOOT.

//  Warning sign flashing under reticle "Danger:  Suit Breached"
//  Warning sign flashing under reticle "Mutagen Detected: 5.25%"
//  Warning sign flashing under reticle "Mutagen Detected: 8.92%"
//  Warning sign flashing under reticle "Mutagen Tracking Protocol Initiated" 
//  Sides of the screen are pulsing red
//  Heartrate increases and background color of heartrate flashes orange / red
//  Graphic of mutagen bar outline fades in
//  The mutagen bar fills up
//  Screen settles and warning sign pops up "Lifesign stabilized" then fades


    enum MUTATION_STATES
    {
        MUTATION_START,
        MUTATION_SUIT_BREACH,
        MUTATION_MUTATION_DETECTED_1,
        MUTATION_MUTATION_DETECTED_2,
        MUTATION_MUTATION_PROTOCOL_INIT,
        MUTATION_MUTATION_SCREEN_FX,
        MUTATION_HEALTH_RAMP,
        MUTATION_FADE_BAR_IN,
        MUTATION_BAR_FILL,
        MUTATION_LIFESIGN_STABILIZED,
        MUTATION_END,
        NUM_MUTATION_STATES
    };

    xbool                       m_bPlayHudMutation;
    s32                         m_MutationState;
    s32                         m_MutationBarAlpha;
    s32                         m_MutationFadeInStep;
    s32                         m_MutationFadeResetChance;
    f32                         m_MutationDisplayTime;

    typedef struct _Cell
    {
        f32 timeslice;
        f32 ypos;
        xbool active;
    }Cell;

    Cell m_Cells[2][20];
};

#endif
