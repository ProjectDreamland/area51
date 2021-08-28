//==============================================================================
//
//  hud_Icon.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_ICON_HPP
#define HUD_ICON_HPP

//==============================================================================
// INCLUDES
//==============================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"
#include "Objects\Player.hpp"

#include "hud_Renderable.hpp"

#define NUM_ICONS 64

//==============================================================================
// CLASS
//==============================================================================

enum icon_type
{
    ICON_ALLY,
    ICON_ENEMY,
    ICON_BLANK1,
    ICON_WAYPOINT,

    ICON_CNH_OUTER,
    ICON_CNH_INNER,
    ICON_SPEAKING,
    ICON_BASE,

    ICON_FLAG_INNER,
    ICON_FLAG_OUTER,
};

enum gutter_type
{
    GUTTER_NONE,
    GUTTER_ELLIPSE,
    GUTTER_RECTANGLE,
};

struct icon_inf 
{
    icon_type       IconType; 
    vector3         FocusPosition;
    vector3         RenderPosition;
    xbool           bOccludes;
    xbool           bAlignToBottom;
    gutter_type     GutterType; 
    xcolor          Color; 
    xbool           Pulsing; 
    xbool           Distance; 
    xwchar          Label[32];
    s32             PlayerNum;
    f32             Opacity;
    f32             IconFadeDist;
    f32             TextFadeDist;
};

class hud_icon : public hud_renderable
{
public:
    hud_icon      ( void );
    virtual        ~hud_icon      ( void ) {};
    void    Init            ( void );
    void    Kill            ( void );

    virtual void    OnRender        ( player*   pPlayer );
    virtual void    OnAdvanceLogic  ( player* pPlayer, f32 DeltaTime );
    virtual xbool   OnProperty      ( prop_query& rPropQuery );
    virtual void    OnEnumProp      ( prop_enum&  List );

    void    AddIcon         (       icon_type     IconType, 
        const vector3&      FocusPosition,
        const vector3&      RenderPosition,
        xbool         bOccludes,
        xbool         bAlignToBottom,
        gutter_type   GutterType, 
        xcolor        Color,
        const xwchar*       pCharName,
        xbool         Pulsing,
        xbool         Distance, 
        f32           Opacity,
        f32           IconFadeDist = -1.0f,
        f32           TextFadeDist = -1.0f
        );

    void    RenderIcon      ( player* pPlayer, icon_inf& Icon );
    void    Reset           ( void );
    f32     GetOpacity      ( f32 DistFromCenter, f32 Opacity, f32 FadeDist );

    //------------------------------------------------------------------------------
    // Public Storage
public:    
    s32                     m_NumActiveIcons;
    icon_inf                m_Icons[ NUM_ICONS ];

    xbool                   m_Active;
    rect                    m_ViewDimensions;
    rhandle<xbitmap>        m_ScreenEdgeBmp;
    rhandle<xbitmap>        m_ScreenCenterBmp;
    xcolor                  m_ScreenEdgeColor;
    xcolor                  m_ScreenCenterColor;
};

#endif
