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

#include "hud_MutantVision.hpp"
#include "GameLib\RenderContext.hpp"

#ifdef TARGET_PS2
#include "ps2/ps2_misc.hpp"
#endif

//==============================================================================
// STATICS
//==============================================================================

static const f32 s_MutantOverlayHeight = 425.0f;
static const f32 s_MutantOverlayFOV    = R_60;

rhandle<char>   hud_mutant_vision::m_OverlayResource;
fx_handle       hud_mutant_vision::m_OverlayHandle;

//==============================================================================
// FUNCTIONS
//==============================================================================

hud_mutant_vision::hud_mutant_vision( void )
{
}

//==============================================================================

hud_mutant_vision::~hud_mutant_vision( void )
{
    m_OverlayHandle.KillInstance();
}

//==============================================================================

void hud_mutant_vision::OnRender( player* pPlayer )
{
    (void)pPlayer;

    // mutant vision on or off?
    if( !g_RenderContext.m_bIsMutated )
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
#ifdef TARGET_XBOX
draw_DisableSatCompensation();
#endif    
    m_OverlayHandle.Render( );
    render::EndRawDataMode();
#ifdef TARGET_XBOX
draw_EnableSatCompensation();
#endif    

    // restore the original view
    eng_SetView( OrigView );
}

//==============================================================================

xbool hud_mutant_vision::OnProperty( prop_query& rPropQuery )
{
    if ( rPropQuery.IsVar( "MutantVision\\OverlayFX" ) )
    {
        if ( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_OverlayResource.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarExternal();
            if ( pString[0] )
            {
                m_OverlayResource.SetName( pString );

                // initialize the effect
                if ( m_OverlayResource.GetPointer() )
                {
                    m_OverlayHandle.KillInstance();
                    m_OverlayHandle.InitInstance    ( m_OverlayResource.GetPointer() );
                    m_OverlayHandle.SetRotation     ( radian3( R_0, R_0, R_0 ) );
                    m_OverlayHandle.SetTranslation  ( vector3( 0.0f, 0.0f, 0.0f ) );
                    m_OverlayHandle.SetScale        ( vector3( 1.0f, 1.0f, 1.0f ) );
                    m_OverlayHandle.SetColor        ( XCOLOR_WHITE );
                }
            }
        }

        return TRUE;
    }

    return FALSE;
}

//==============================================================================

void hud_mutant_vision::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader  ( "MutantVision", "Properties for the mutant vision HUD elements", 0 );
    List.PropEnumExternal( "MutantVision\\OverlayFX" , "Resource\0fxo", "Particle for the overlay effect" , PROP_TYPE_MUST_ENUM );
}

//==============================================================================

void hud_mutant_vision::UpdateEffects( f32 DeltaTime )
{
    m_OverlayHandle.AdvanceLogic( DeltaTime );
}

