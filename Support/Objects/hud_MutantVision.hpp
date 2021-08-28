//==============================================================================
//
//  hud_MutantVision.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#ifndef HUD_MUTANTVISION_HPP
#define HUD_MUTANTVISION_HPP

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

class hud_mutant_vision : public hud_renderable
{
public:
                    hud_mutant_vision   ( void );
    virtual        ~hud_mutant_vision   ( void );

    virtual void    OnRender        ( player* pPlayer );
    virtual xbool   OnProperty      ( prop_query& rPropQuery );
    virtual void    OnEnumProp      ( prop_enum&  List );
            
    static  void    UpdateEffects   ( f32 DeltaTime );

protected:
    static rhandle<char>   m_OverlayResource;
    static fx_handle       m_OverlayHandle;
};

#endif // HUD_MUTANTVISION_HPP