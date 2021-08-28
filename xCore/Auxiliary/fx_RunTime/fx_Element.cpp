//==============================================================================
//
//  fx_Element.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Mgr.hpp"
#include "e_Draw.hpp"

//==============================================================================

s32 DefaultElementMemoryFn( const fx_element_def& ElementDef )
{
    (void)ElementDef;
    return( sizeof( fx_element ) );
}

//==============================================================================

void fx_element::Initialize( const fx_element_def* pElementDef, 
                                   f32*            pInput )
{
    // Initialize the fields from arguments.
    m_pElementDef = pElementDef;
    m_pInput      = pInput;

    // Install controller values OR default values.

    f32* pWrite  = (f32*)(&m_Scale);
    f32* pRead   = (f32*)(&m_pElementDef->Scale);
    s32* pOffset = (s32*)( m_pElementDef->CtrlOffsets);

    for( s32 i = 0; i < 13; i++ )
    {
        if( pOffset[i] == -1 )  pWrite[i] = pRead[i];
        else                    pWrite[i] = m_pInput[ pOffset[i] ];
    }

    // Color my world.
    m_BaseColor.Set( 255, 255, 255, (u8)255 );
    m_ColorDirty = TRUE;

    // The bounds are local by default.  Particular elements can override. 
    m_BBox.Clear();
    m_LocalDirty = TRUE;
}

//==============================================================================

void fx_element::BaseLogic( void )
{
    s32  i;
    f32* pWrite = (f32*)(&m_Scale);

    // Install controller values.
    for( i = 0; i < 13; i++ )
    {
        if( m_pElementDef->CtrlOffsets[i] != -1 )
        {
            if( i < 9 )  m_LocalDirty = TRUE;
            else         m_ColorDirty = TRUE;

            pWrite[i] = m_pInput[ m_pElementDef->CtrlOffsets[i] ];
        }
    }
}

//==============================================================================

void fx_element::AdvanceLogic( const fx_effect_base* pEffect, f32 DeltaTime )
{
    (void)DeltaTime;
    (void)pEffect;
}

//==============================================================================

void fx_element::ApplyColor( void )
{
    f32 FloatColor[4];

    // Enforce the float color range.  
    // (Smooth controlled color can get out of range.)
    FloatColor[0] = MINMAX( 0.0f, m_FloatColor[0], 1.0f );
    FloatColor[1] = MINMAX( 0.0f, m_FloatColor[1], 1.0f );
    FloatColor[2] = MINMAX( 0.0f, m_FloatColor[2], 1.0f );
    FloatColor[3] = MINMAX( 0.0f, m_FloatColor[3], 1.0f );

    // Installed combination of local element color and parent effect color.
    m_Color.R = (u8)(m_BaseColor.R * FloatColor[0]);
    m_Color.G = (u8)(m_BaseColor.G * FloatColor[1]);
    m_Color.B = (u8)(m_BaseColor.B * FloatColor[2]);
    m_Color.A = (u8)(m_BaseColor.A * FloatColor[3]);
}

//==============================================================================

void fx_element::Render( const fx_effect_base* pEffect ) const
{
    (void)pEffect;

#ifdef DEBUG_FX
    //
    // Debug rendering.
    //

    if( !FXDebug.ElementReserved )
        return;

    draw_ClearL2W();

    if( FXDebug.ElementBounds )
        draw_BBox( m_BBox, XCOLOR_GREEN );

    if( FXDebug.ElementCenter )
        draw_Marker( pEffect->GetL2W() * m_Translate, XCOLOR_GREEN );

    if( FXDebug.ElementAxis || FXDebug.ElementVolume )
    {
        draw_SetL2W( GetL2W( pEffect ) );

        if( FXDebug.ElementAxis )
            draw_Axis( 3.0f );
        if( FXDebug.ElementVolume )
            draw_BBox( bbox( vector3(-0.5f,-0.5f,-0.5f),
                             vector3( 0.5f, 0.5f, 0.5f) ), XCOLOR_BLUE );

        draw_ClearL2W();
    }
#endif // DEBUG_FX
}

//==============================================================================

xbool fx_element::IsFinished( const fx_effect_base* pEffect ) const
{
    return( (pEffect->IsSuspended()) || 
            (pEffect->GetAge() >= m_pElementDef->TimeStop) );
}

//==============================================================================

void fx_element::Reset( void )
{
    m_LocalDirty = TRUE;
}

//==============================================================================
