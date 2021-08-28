//==============================================================================
//
//  fx_Plane.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Plane.hpp"
#include "x_context.hpp"
#include "e_Draw.hpp"

//==============================================================================
//  DAMN LINKER!
//==============================================================================

s32 fx_Plane;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void fx_plane::AdvanceLogic( const fx_effect_base* pEffect, f32 DeltaTime )
{
    CONTEXT( "fx_plane::AdvanceLogic" );

    (void)DeltaTime;

    // All we need to do here is update the bounding box.

    matrix4 L2W;

    L2W.Setup( m_Scale, m_Rotate, m_Translate );
    L2W = pEffect->GetL2W() * L2W;

    m_BBox.Set( vector3(0,0,0), 0.5f );
    m_BBox.Transform( L2W );
}

//==============================================================================

#ifdef TARGET_GCN
void SPEmitterGCNSetup( const xbitmap* pAlpha, xbool Sub );
#endif

//==============================================================================

void fx_plane::Render( const fx_effect_base* pEffect ) const
{
    CONTEXT( "fx_plane::Render" );

    if( pEffect->IsSuspended() )
        return;

    const fx_edef_plane& PlaneDef = (fx_edef_plane&)(*m_pElementDef);

    matrix4 L2W;

    L2W.Setup( m_Scale, m_Rotate, m_Translate );
    L2W = pEffect->GetL2W() * L2W;

    vector3 v3TL( -0.5f,  0.5f,  0.0f );  // Top left
    vector3 v3TR(  0.5f,  0.5f,  0.0f );  // Top right
    vector3 v3BR(  0.5f, -0.5f,  0.0f );  // Bottom right
    vector3 v3BL( -0.5f, -0.5f,  0.0f );  // Bottom left

    f32 TU = PlaneDef.TileU;
    f32 TV = PlaneDef.TileV;
    f32 SU = x_fmod( (PlaneDef.ScrollU * pEffect->GetAge()), TU );
    f32 SV = x_fmod( (PlaneDef.ScrollV * pEffect->GetAge()), TV );

    vector2 uvTL( 0.0f + SU, 0.0f + SV );  // Top left 
    vector2 uvTR(  TU  + SU, 0.0f + SV );  // Top right
    vector2 uvBR(  TU  + SU,  TV  + SV );  // Bottom right
    vector2 uvBL( 0.0f + SU,  TV  + SV );  // Bottom left    

    const xbitmap* pDiffuse  = NULL;
    const xbitmap* pAlpha    = NULL;

    pEffect->GetBitmaps( PlaneDef.BitmapIndex, pDiffuse, pAlpha );

    u32 DrawFlags = DRAW_TEXTURED | DRAW_NO_ZWRITE | DRAW_CULL_NONE | DRAW_USE_ALPHA;

    if( PlaneDef.CombineMode > 0 )
        DrawFlags |= DRAW_BLEND_ADD;

    if( PlaneDef.CombineMode < 0 )
        DrawFlags |= DRAW_BLEND_SUB;

    if( !PlaneDef.ReadZ )
        DrawFlags |= DRAW_NO_ZBUFFER;

    draw_SetL2W( L2W );

    draw_Begin( DRAW_TRIANGLES, DrawFlags );
    draw_SetTexture( *pDiffuse );

    #ifdef TARGET_GCN
    SPEmitterGCNSetup( pAlpha, (DrawFlags & DRAW_BLEND_SUB) ? TRUE : FALSE );
    #endif

    draw_Color( m_Color );

    draw_UV( uvTL );    draw_Vertex( v3TL );
    draw_UV( uvBL );    draw_Vertex( v3BL );
    draw_UV( uvBR );    draw_Vertex( v3BR );

    draw_UV( uvBR );    draw_Vertex( v3BR );
    draw_UV( uvTR );    draw_Vertex( v3TR );
    draw_UV( uvTL );    draw_Vertex( v3TL );

    draw_End();

#ifndef X_RETAIL
    fx_element::Render( pEffect );
#endif // X_RETAIL
}

//==============================================================================

#undef new
REGISTER_FX_ELEMENT_CLASS( fx_plane, "PLANE", DefaultElementMemoryFn );

//==============================================================================
