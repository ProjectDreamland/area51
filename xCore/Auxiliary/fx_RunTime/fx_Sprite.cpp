//==============================================================================
//
//  fx_Sprite.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Sprite.hpp"
#include "x_context.hpp"
#include "e_Draw.hpp"
#include "Entropy.hpp"

//==============================================================================
//  DAMN LINKER!
//==============================================================================

s32 fx_Sprite;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void fx_sprite::AdvanceLogic( const fx_effect_base* pEffect, f32 DeltaTime )
{
    CONTEXT( "fx_sprite::AdvanceLogic" );

    (void)DeltaTime;

    // All we need to do here is update the bounding box.

    f32     Scale    = pEffect->GetUniformScale() * MAX( m_Scale.X, m_Scale.Y );
    vector3 Position = pEffect->GetL2W()          * m_Translate;

    m_BBox.Set( Position, Scale );
}

//==============================================================================

#ifdef TARGET_GCN
void SPEmitterGCNSetup( const xbitmap* pAlpha, xbool Sub );
#endif

//==============================================================================

void fx_sprite::Render( const fx_effect_base* pEffect ) const
{
    CONTEXT( "fx_sprite::Render" );

    if( pEffect->IsSuspended() )
        return;

    const fx_edef_sprite& SpriteDef = (fx_edef_sprite&)(*m_pElementDef);

    vector3  Position = pEffect->GetL2W() * m_Translate;
    f32      UniScale = pEffect->GetUniformScale();
    vector2  WH( UniScale * m_Scale.X, UniScale * m_Scale.Y );
    vector2  UV0( 0.0f, 0.0f );
    vector2  UV1( 1.0f, 1.0f );

    if( SpriteDef.ZBias != 0.0f )
    {
        const view* pView = eng_GetView();
        Position += pView->GetViewZ() * SpriteDef.ZBias;
    }

    u32 DrawFlags = DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_UV_CLAMP | DRAW_CULL_NONE;

    const xbitmap* pDiffuse  = NULL;
    const xbitmap* pAlpha    = NULL;

    pEffect->GetBitmaps( SpriteDef.BitmapIndex, pDiffuse, pAlpha );

    if( SpriteDef.CombineMode > 0 )
        DrawFlags |= DRAW_BLEND_ADD;

    if( SpriteDef.CombineMode < 0 )
        DrawFlags |= DRAW_BLEND_SUB;

    if( !SpriteDef.ReadZ )
        DrawFlags |= DRAW_NO_ZBUFFER;

    draw_ClearL2W();

    draw_Begin( DRAW_SPRITES, DrawFlags );

    if( pDiffuse )
        draw_SetTexture( *pDiffuse );

    #ifdef TARGET_GCN
    SPEmitterGCNSetup( pAlpha, (DrawFlags & DRAW_BLEND_SUB) ? TRUE : FALSE );
    #endif

    draw_SpriteUV( Position, WH, UV0, UV1, m_Color, m_Rotate.Roll );

    draw_End();

#ifndef X_RETAIL
    fx_element::Render( pEffect );
#endif // X_RETAIL
}

//==============================================================================

#undef new
REGISTER_FX_ELEMENT_CLASS( fx_sprite, "SPRITE", DefaultElementMemoryFn );

//==============================================================================
