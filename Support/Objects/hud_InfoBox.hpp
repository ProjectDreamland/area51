//==============================================================================
//
//  hud_InfoBox.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_INFOBOX_HPP
#define HUD_INFOBOX_HPP

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

class hud_info_box : public hud_renderable
{
public:
    hud_info_box                            ( void );
    virtual        ~hud_info_box            ( void ) {};

    virtual void    OnRender                ( player*         pPlayer );
    virtual void    OnAdvanceLogic          ( player*         pPlayer, f32 DeltaTime );
    virtual xbool   OnProperty              ( prop_query&     rPropQuery );
    virtual void    OnEnumProp              ( prop_enum&      List );

            void    OnRenderCTF_Flag        ( vector3& Pos );
            void    UpdateCFT_Flag          ( f32 DeltaTime, player* pPlayer );
            void    SetScoreInfo            ( const xwchar* Col1, const xwchar* Col2, const xwchar* Col3, s32 Slot ); 
            s32     GetLastWidth            ( void ) { return m_LastWidth; }

    //------------------------------------------------------------------------------
    // Public Storage
public:

private:
    s32            m_LastWidth;
    xwchar         m_ScoreString_Col1[2][32];
    xwchar         m_ScoreString_Col2[2][32];
    xwchar         m_ScoreString_Col3[2][32];

    f32                 m_CTFFlagAlpha;
    rhandle<xbitmap>    m_CTFFlag;
    rhandle<xbitmap>    m_CTFFlagRing;
    xbool               m_RenderCTFFlag;

};

#endif