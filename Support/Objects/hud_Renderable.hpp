//==============================================================================
//
//  hud_Renderable.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_RENDERABLE_HPP
#define HUD_RENDERABLE_HPP

//==============================================================================
// INCLUDES
//==============================================================================

#include "x_math.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class player;
class prop_query;
class prop_enum;

//==============================================================================
// CLASS
//==============================================================================

class hud_renderable 
{
public:
    hud_renderable( void )
    {
        m_Enabled    = FALSE;
        m_bPulsing   = FALSE;
        
        m_XPos       = 0.0f;
        m_YPos       = 0.0f;

        m_ViewDimensions.Clear();
    }

    virtual        ~hud_renderable  ( void ) {}
    
    virtual void    OnRender        ( player*       /*pPlayer*/ )                    {  };
    virtual void    OnAdvanceLogic  ( player*       /*pPlayer*/, f32 /*DeltaTime*/ ) {  };
    virtual xbool   OnProperty      ( prop_query&   /*rPropQuery*/ )                 { return FALSE; };
    virtual void    OnEnumProp      ( prop_enum&    /*List*/ )                       {  };

//------------------------------------------------------------------------------
// Public Storage
public:
    u32 m_Enabled  : 1;
    u32 m_bPulsing : 1;
    
    /*
    f32 m_PulseRate;
    s32 m_PulseAlpha;
    */
    
    f32 m_XPos;
    f32 m_YPos;

    rect m_ViewDimensions;
};

#endif
