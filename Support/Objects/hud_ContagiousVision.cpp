//==============================================================================
//
//  hud_MutantVision.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
 
//==============================================================================
// INCLUDES
//==============================================================================

#include "hud_ContagiousVision.hpp"
#include "GameLib\RenderContext.hpp"

#ifdef TARGET_PS2
#include "ps2/ps2_misc.hpp"
#endif

//==============================================================================
// STATICS
//==============================================================================

static const f32 s_MutantOverlayHeight = 425.0f;
static const f32 s_MutantOverlayFOV    = R_60;

fx_handle       hud_contagious_vision::m_OverlayHandle;
 
//==============================================================================
// FUNCTIONS
//==============================================================================
 
hud_contagious_vision::hud_contagious_vision( void )
{
    if (m_OverlayHandle.Validate())
        m_OverlayHandle.KillInstance();

    SMP_UTIL_InitFXFromString( "HUD_ContagionBorder.fxo", m_OverlayHandle );

    // initialize the effect
    if ( m_OverlayHandle.Validate() )
    {
        m_OverlayHandle.SetRotation     ( radian3( R_0, R_0, R_0 ) );
        m_OverlayHandle.SetTranslation  ( vector3( 0.0f, 0.0f, 0.0f ) );
        m_OverlayHandle.SetScale        ( vector3( 1.0f, 1.0f, 1.0f ) );
        m_OverlayHandle.SetColor        ( XCOLOR_WHITE );
    }
}

//==============================================================================

hud_contagious_vision::~hud_contagious_vision( void )
{
    if (m_OverlayHandle.Validate())
        m_OverlayHandle.KillInstance();
}

//==============================================================================

void hud_contagious_vision::OnRender( player* pPlayer )
{
    // If the fx isn't loaded for some horrible reason, bail out.
    if (!m_OverlayHandle.Validate())
        return;

    // Is the player contagious?
    if (!pPlayer->IsContagious())
        return;
   
    // back up the current view so we can set it back
    view OrigView = *eng_GetView();

    s32 L, T, R, B;
    OrigView.GetViewport( L, T, R, B );
    
    // set up a camera that looks directly at our particle effect
    view OverlayView;
    OverlayView.SetPosition( vector3( 0.0f, s_MutantOverlayHeight, 0.0f ) );
    OverlayView.SetRotation( radian3( R_90, R_0, R_0 ) );
    OverlayView.SetXFOV    ( s_MutantOverlayFOV );
    OverlayView.SetViewport( L, T, R, B );
    OverlayView.SetZLimits ( 10.0f, 5000.0f );
    eng_SetView( OverlayView );

    // Set up the scissoring region for the uber-nasty ps2
    #ifdef TARGET_PS2
    gsreg_Begin(1);
    gsreg_SetScissor( L, T, R, B );
    gsreg_End();
    #endif
 
    // render it!
    render::StartRawDataMode(); // required for particle render
    m_OverlayHandle.Render( );
    render::EndRawDataMode();

    // restore the original view
    eng_SetView( OrigView );
}

//==============================================================================

void hud_contagious_vision::UpdateEffects( f32 DeltaTime )
{
    m_OverlayHandle.AdvanceLogic( DeltaTime );
}

