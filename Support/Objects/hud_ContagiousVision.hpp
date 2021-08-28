//==============================================================================
//
//  hud_ContagiousVision.hpp
//
//  Copyright (c) 2002-2005 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef __HUD_CONTAGIOUSVISION_HPP__
#define __HUD_CONTAGIOUSVISION_HPP__

//==============================================================================
// INCLUDES
//==============================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"
#include "Objects\Player.hpp"
#include "Auxiliary\fx_RunTime\Fx_Mgr.hpp"

#include "hud_Renderable.hpp"

//==============================================================================
// CLASS
//==============================================================================

class hud_contagious_vision : public hud_renderable
{
public:
                    hud_contagious_vision   ( void );
    virtual        ~hud_contagious_vision   ( void );

    virtual void    OnRender        ( player* pPlayer );            
    static  void    UpdateEffects   ( f32 DeltaTime );

protected:
    static fx_handle       m_OverlayHandle;
};

#endif // __HUD_CONTAGIOUSVISION_HPP__