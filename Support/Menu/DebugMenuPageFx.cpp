//==============================================================================
//  DebugMenuPageFx.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu Fx page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Gamelib\statsmgr.hpp"
#include "Render\Render.hpp"
#include "fx_RunTime\fx_Mgr.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU ) && defined( DEBUG_FX )


//==============================================================================

debug_menu_page_fx::debug_menu_page_fx( ) : debug_menu_page()
{
    m_pTitle = "FX";

    AddItemBool     ( "Effect Axis", FXDebug.EffectAxis );
    AddItemBool     ( "Effect Center", FXDebug.EffectCenter );
    AddItemBool     ( "Effect Volume", FXDebug.EffectVolume );
    AddItemBool     ( "Effect Bounds", FXDebug.EffectBounds );
    AddItemSeperator( );
    AddItemBool     ( "Element Axis", FXDebug.ElementAxis );
    AddItemBool     ( "Element Center", FXDebug.ElementCenter );
    AddItemBool     ( "Element Volume", FXDebug.ElementVolume );
    AddItemBool     ( "Element Bounds", FXDebug.ElementBounds );
    AddItemBool     ( "Element Wire", FXDebug.ElementWire );
    AddItemBool     ( "Element Sprite Center", FXDebug.ElementSpriteCenter );
    AddItemBool     ( "Element Sprite Count", FXDebug.ElementSpriteCount );
    AddItemBool     ( "Element Custom", FXDebug.ElementCustom );
}

//==============================================================================

#endif // defined( ENABLE_DEBUG_MENU ) && defined( DEBUG_FX )
