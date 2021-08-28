//==============================================================================
//
//  hud_Object.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_OBJECT_HPP
#define HUD_OBJECT_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"
#include "Objects\Player.hpp"

#include "hud_Player.hpp"
#include "NetworkMgr\NetLimits.hpp"
#ifndef X_EDITOR
#include "Ui\ui_manager.hpp"
#include "Ui\ui_font.hpp"
#endif

#define MAX_BIN_TXT_RSC   3

extern xcolor g_HudColor;

//=========================================================================
// CLASS
//=========================================================================

class hud_object : public object
{
public:
    CREATE_RTTI( hud_object, object, object )

                            hud_object              ( void );
    virtual                ~hud_object              ( void );
    virtual s32             GetMaterial             ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnRender                ( void );
    virtual void            OnAdvanceLogic	        ( f32 DeltaTime );
    virtual bbox            GetLocalBBox            ( void ) const;      

	virtual	void	        OnEnumProp		        ( prop_enum&  list );
	virtual	xbool	        OnProperty		        ( prop_query& rPropQuery );
            
    virtual const object_desc&  GetTypeDesc         ( void ) const;
    static  const object_desc&  GetObjectType       ( void );

            void            ResetFrameRateInfo      ( void );
            void            RenderFrameRateInfo     ( void );
            void            RenderTimer             ( void );
            
            void            GetBinaryResourceName   ( xstring& String );
            void            SetElementPulseState    ( s32 ElementID, xbool DoPulse );
            void            InitHud                 ( void ); 
            player_hud&     GetPlayerHud            ( s32 LocalSlot );

            void            SetupLetterBox          ( xbool On, f32 ScrollTime = 0.0f );
            xbool           IsLetterBoxOn           ( void ) const;
            f32             GetLetterBoxAmount      ( void ) const;
    static  void            RenderLetterBox         ( const rect& VP, f32 Percent );

            void            SetObjectiveText        ( s32 TableNameIndex, s32 TitleStringIndex );
            void            RenderObjectiveText     ( void );

            s32             GetNumPlayers           ( void ) { return m_NumHuds ; };

    player_hud m_PlayerHuds[ NET_MAX_PER_CLIENT ];

    static  s32                         m_PulseAlpha;
    static  f32                         m_PulseRate;
    xbool                               m_Initialized;

protected:
            rhandle<char>               m_hBinaryTextRsc[ MAX_BIN_TXT_RSC ];

            rect                        m_ViewDimensions;

            xbool                       m_LogicRunning;

            s32                         m_NumHuds;

            s32                         m_FPSCount15;
            s32                         m_FPSCount20;
            s32                         m_FPSCount30;
            s32                         m_Below30ImageCount;

            xbool                       m_bLetterBoxOn;
            f32                         m_LetterBoxTotalTime;
            f32                         m_LetterBoxCurrTime; 

            guid                        m_TimerTriggerGuid;
            xbool                       m_RenderTimer;
            xbool                       m_TimerActive;
            f32                         m_TimerTime;
            f32                         m_TimerAdd;
            f32                         m_TimerSub;
            xbool                       m_TimerWarning;
            xbool                       m_TimerCritical;
            xbool                       m_TimerCriticalStarted;
            xbool                       m_TimerWarningStarted;



            s32                        m_ObjectiveTableNameIndex;
            s32                        m_ObjectiveTitleStringIndex;
            f32                         m_ObjectiveTime;
};

//=========================================================================

inline void RenderLine( const xwchar* pLine, irect& iRect, u8 Alpha, xcolor& TextColor, s32 FontNum, s32 Flags, xbool bShadow = FALSE, f32 FlareAmount = 0.0f )
{
    // So that the editor doesn't choke on compile.
    (void)FlareAmount;
    (void)pLine;
    (void)iRect;
    (void)Alpha;
    (void)FontNum;
    (void)bShadow;

#ifndef X_EDITOR
    //aharp TODO Make use of flaring.

    if( pLine[ 0 ] == 0 )
    {
        return;
    }

    xcolor TextShadowColor  ( XCOLOR_BLACK );  
    xcolor LocalTextColor   ( TextColor );

    LocalTextColor.A  = (s32)(Alpha * 0.8f);    
    TextShadowColor.A = (s32)(Alpha * 0.8f);

    if( bShadow )
    {
        irect Shadow;

        Shadow = iRect;
        Shadow.Translate( 0, 2 );
        g_UiMgr->RenderText( FontNum, Shadow, Flags, TextShadowColor, pLine, TRUE,  FALSE );

        Shadow = iRect;
        Shadow.Translate( -2, 0 );
        g_UiMgr->RenderText( FontNum, Shadow, Flags, TextShadowColor, pLine, TRUE,  FALSE );

        Shadow = iRect;
        Shadow.Translate( 2, 0 );
        g_UiMgr->RenderText( FontNum, Shadow, Flags, TextShadowColor, pLine, TRUE,  FALSE );

        Shadow = iRect;
        Shadow.Translate( 0, -2 );
        g_UiMgr->RenderText( FontNum, Shadow, Flags, TextShadowColor, pLine, TRUE,  FALSE );
    } 

    g_UiMgr->RenderText( FontNum, iRect,  Flags, LocalTextColor,       pLine,     FALSE, TRUE );

#endif
}

class flare_fader 
{ 
public:
    void     AdvanceLogic    ( f32 DeltaTime ) 
    {
        m_TimeLeft -= DeltaTime;
    };

    f32     GetFlare        ( void )
    {
        if( m_TimeLeft > (m_HoldTime + m_RampDownTime) )
        {
            return (m_TimeLeft - (m_HoldTime + m_RampDownTime)) / m_RampUpTime;
        }
        else if( m_TimeLeft > m_RampDownTime )
        {
            return (m_TimeLeft - m_RampDownTime) / m_HoldTime;
        }
        else if( m_TimeLeft > 0.0f ) 
        {
            return (m_TimeLeft / m_RampDownTime);
        }

        return 0.0f;
    }

    void    SetFlare        ( f32 Time )
    {
        (void)Time;
    }

protected:
    f32 m_RampUpTime;
    f32 m_HoldTime;
    f32 m_RampDownTime;

    f32 m_TimeLeft;
};

//=========================================================================
// END
//=========================================================================


#endif
