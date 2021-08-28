//==============================================================================
//
//  hud_scanner.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_SCANNER_HPP
#define HUD_SCANNER_HPP

//==============================================================================
// INCLUDES
//==============================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"
#include "Objects\Player.hpp"

#include "hud_Renderable.hpp"

enum eScanSectionID
{
    SSID_DNA, 
    SSID_TITLE, 
    SSID_JOB,
    SSID_SPECIES,
    SSID_BLOOD, 
    SSID_HEART,
    SSID_HEART_FIGHT,
    SSID_EQUIP,
};

#define MAX_SCAN_LINE 7

//==============================================================================
// CLASS
//==============================================================================

struct scanner_bar
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
};

class hud_scanner : public hud_renderable
{
public:
                    hud_scanner      ( void );
    virtual        ~hud_scanner      ( void ) {};

    virtual void    OnRender        ( player* pPlayer );
    virtual void    OnAdvanceLogic  ( player* pPlayer, f32 DeltaTime );
    virtual xbool   OnProperty      ( prop_query& rPropQuery );
    virtual void    OnEnumProp      ( prop_enum&  List );
                        
            void    RenderGeiger    ( player *pPlayer );
            void    RenderScanInfo  ( player *pPlayer );
            void    RenderBar       ( void  );
            void    AdvanceBar      ( player* pPlayer, f32 DeltaTime );

            void    RenderLoreBar   ( s32 AmountPercent );
            void    NotifyScanComplete(player* pPlayer, guid TheGuid );

            xbool   GetLoreDetected ( void ) { return m_LoreDetected; };

protected:
            xbool   BuildScanIdentifier ( char *pIdentifier, player* pPlayer, eScanSectionID Ident );
            void    BuildDataLines      ( player *pPlayer, guid TheGuid );
            void    BuildAirSample      ( player *pPlayer );
            void    SetAirSampleRandSeed( player *pPlayer );
            void    SetCharacterRandSeed( object *pObject );
            void    PrependHeartbeat    ( player *pPlayer, s32 TheLine );
            xbool   IsGenericCharacter  ( object *pObject );

//------------------------------------------------------------------------------
// Public Storage
public:
    static rhandle<xbitmap>     m_OuterBmp;
    static rhandle<xbitmap>     m_OuterBmpFlipped;
    static rhandle<xbitmap>     m_InnerBmp;

    rhandle<xbitmap>            m_ScannerBarBmp;
    s32                         m_ScannerBmpAlpha;
    s32                         m_ScanPeak;
    xbool                       m_ScanPeakUpdate;
    f32                         m_ScanPeakUpdateTime;

    xbool                       m_LoreDetected;

    scanner_bar                 m_Bar;

    xwstring                    m_DataLine[MAX_SCAN_LINE];
    xbool                       m_bRenderScanData;
    f32                         m_ScanDataTimeout;
    xbool                       m_bJustAirSample;
};

#endif
