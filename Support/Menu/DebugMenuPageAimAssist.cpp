//==============================================================================
//  DebugMenuPageAimAssist.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu aim assist page.
//  
//==============================================================================

#include "DebugMenu2.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

//==============================================================================

// aim assist stuff
extern xbool        g_AimAssist_Render_Reticle;
extern xbool        g_AimAssist_Render_Bullet;
extern xbool        g_AimAssist_Render_Turn;
extern xbool        g_AimAssist_Render_Bullet_Angle;
extern xbool        g_AimAssist_Render_Player_Pills;

//==============================================================================

debug_menu_page_aim_assist::debug_menu_page_aim_assist( ) : debug_menu_page()
{
    m_pTitle = "Aim Assist";

#ifndef X_RETAIL
                            AddItemBool     ( "AimAssist reticle pill"    , g_AimAssist_Render_Reticle );
                            AddItemBool     ( "AimAssist bullet markers"  , g_AimAssist_Render_Bullet );
                            AddItemBool     ( "AimAssist turn damp pill"  , g_AimAssist_Render_Turn );
                            AddItemBool     ( "AimAssist bullet angle"    , g_AimAssist_Render_Bullet_Angle );
                            AddItemBool     ( "AimAssist player pills"    , g_AimAssist_Render_Player_Pills );
#endif
}

//==============================================================================

void debug_menu_page_aim_assist::OnChangeItem( debug_menu_item* pItem )
{
    (void) pItem;

}

//==============================================================================

#endif // defined( ENABLE_DEBUG_MENU )
