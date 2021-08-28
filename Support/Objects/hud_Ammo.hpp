//==============================================================================
//
//  hud_Ammo.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_AMMO_HPP
#define HUD_AMMO_HPP

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

class hud_ammo : public hud_renderable
{
public:
                    hud_ammo                    ( void );
    virtual        ~hud_ammo                    ( void ) {};

            void    Init                        ( void );

    virtual void    OnRender                    ( player*       pPlayer );
    virtual void    OnAdvanceLogic              ( player*       pPlayer, f32 DeltaTime );
    virtual xbool   OnProperty                  ( prop_query&   rPropQuery );
    virtual void    OnEnumProp                  ( prop_enum&    List );

            void    RenderNades                 ( player*       pPlayer , inven_item NadType);
            void    RenderAmmoString            ( player*       pPlayer );
            void    RenderAmmoBar               ( player*       pPlayer );
            void    RenderFlashlight            ( player*       pPlayer );



            void    SetRenderColor              ( xcolor RenderColor ) { m_AmmoHudColor = RenderColor; }

//------------------------------------------------------------------------------
// Public Storage
public:
    //static rhandle<xbitmap>            m_AmmoBar;
    rhandle<xbitmap>            m_FragAmmoIcon;
    rhandle<xbitmap>            m_JBeanAmmoIcon;
    rhandle<xbitmap>            m_JBeanXAmmoIcon;
    rhandle<xbitmap>            m_AmmoBoxBMP;
    rhandle<xbitmap>            m_AmmoBoxBMP_Clip;
    rhandle<xbitmap>            m_AmmoBoxBMP_Resv;
    rhandle<xbitmap>            m_AmmoBoxBMP_Nads;

    xbool                       m_MultiPlayerHud;
    xbool                       m_AmmoHudInited;

    xwstring                    m_WeaponAmmoCount;

    irect                       m_AmmoRect;
    s32                         m_CurrentWeaponAmmoIndex;
    s32                         m_CurrentWeaponAmmo;

    #define                     NUM_SHADOW_AMMO_RENDERS     5
    s32                         m_WeaponAmmoQAmounts[NUM_SHADOW_AMMO_RENDERS];

    s32                         m_GrenadeAmmoJBean;
    s32                         m_GrenadeAmmoFrag;           

    vector3                     m_AmmoBarPos;
    vector3                     m_GrenadeStartPos;



    static xcolor                      m_FragGrenadeColor;
    static xcolor                      m_GravGrenadeColor;

    rhandle<xbitmap>            m_WeaponIcons[ INVEN_NUM_WEAPONS ];

    vector3                     m_WeaponIconPos;

    vector2                     m_Dimensions;

    // These are for aligning the ammo count numbers.
    s32                         m_LeftMag;
    s32                         m_TopMag;

    s32                         m_TopReserve;
    s32                         m_LeftReserve;

    s32                         m_TopNade;
    s32                         m_LeftNade;

    xbool                       m_WarningClip;
    xbool                       m_CriticalClip;
    xbool                       m_WarningResv;
    xbool                       m_CriticalResv;
    xbool                       m_WarningNads;
    xbool                       m_CriticalNads;

    s32                         m_WarningClipALPHA;
    s32                         m_WarningResvALPHA;
    s32                         m_WarningNadsALPHA;

    voice_id                    m_CriticalSoundID;

protected:
    xcolor                      m_AmmoHudColor;
};

#endif
