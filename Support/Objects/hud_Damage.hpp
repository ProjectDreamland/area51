//==============================================================================
//
//  hud_Damage.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_DAMAGE_HPP
#define HUD_DAMAGE_HPP

//==============================================================================
// INCLUDES
//==============================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"
#include "Objects\Player.hpp"

#include "hud_Renderable.hpp"

//==============================================================================
// DEFINES
//==============================================================================

#define MAX_PAIN_EVENTS 16

//==============================================================================
// CLASS
//==============================================================================

class hud_damage : public hud_renderable
{
public:
                    hud_damage      ( void );
    virtual        ~hud_damage      ( void ) {};

    virtual void    OnRender        ( player*       pPlayer );
    virtual void    OnAdvanceLogic  ( player*       pPlayer, f32 DeltaTime );
    virtual xbool   OnProperty      ( prop_query&   rPropQuery );
    virtual void    OnEnumProp      ( prop_enum&    List );

    void            RenderDamage    ( player*       pPlayer );


//------------------------------------------------------------------------------
// Public Storage
public:
    static rhandle<xbitmap>            m_DamageBitmap;
    static f32                         m_DamageTimeTillFade;


    static f32                         m_DamageBitmapOffset;
    static f32                         m_DamageFadeOutTime;
    static f32                         m_ScreenFlashDeltaTime;

    static xcolor                      m_FragGrenadeDamageColor;
    static xcolor                      m_GravGrenadeDamageColor;

    struct hud_pain
    {
        pain    Pain;
        radian  LastRot;
        f32     PainTime;
        u8      Overlay;
        s32     LocalSlot;
    };

    hud_pain                    m_pPain[ MAX_PAIN_EVENTS ];

};

#endif