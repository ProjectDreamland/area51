//==============================================================================
//
//  Lightm_ShaftEffect.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "LightShaftEffect.hpp"
#include "Entropy.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"


//==============================================================================
//  FUNCTIONS
//==============================================================================

// Constructor/destructor
light_shaft_effect::light_shaft_effect()
{
    // Init properties
    //m_Position         = 0.5f;
    //m_HorizSpeed       = 0.0f;
    m_Position         = -0.3f;
    m_HorizSpeed       = 0.15f;
    m_PixelWidth       = 64.0f;
    m_HorizScale       = 0.5f;
    m_VertScale        = 8.0f;
    m_Alpha            = 0.15f;
    m_nPasses          = 20;
    
    m_FogAlpha         = 0.5f;
    m_FogSize          = 1024.0f*2.0f;
    m_FogAngle         = 0.0f;
    m_FogRotSpeed      = R_5;
    m_FogZoom          = 0.0f;
    m_FogZoomSpeed     = 0.2f;
    m_nFogPasses       = 20;
}

//==============================================================================

light_shaft_effect::~light_shaft_effect()
{
}

//==============================================================================

// Functions
void light_shaft_effect::Init( const char* pTextBMP, const char* pFogBMP )
{
    s32 x;
    s32 y;

    // Load text and set alpha to zero
    VERIFY( auxbmp_Load( m_TextBMP, pTextBMP ) );
    m_TextBMP.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
    for( y = 0 ; y < m_TextBMP.GetHeight(); y++ )
    {
        for( x = 0; x < m_TextBMP.GetWidth(); x++ )
        {
            xcolor C = m_TextBMP.GetPixelColor( x,y );
            C.A = 0;
            m_TextBMP.SetPixelColor( C, x,y );
        }
    }
    auxbmp_ConvertToNative( m_TextBMP );
    vram_Register( m_TextBMP );
    
    // Load fog and copy red to alpha
    VERIFY( auxbmp_Load( m_FogBMP, pFogBMP ) );
    m_FogBMP.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
    for( y = 0 ; y < m_FogBMP.GetHeight(); y++ )
    {
        for( x = 0; x < m_FogBMP.GetWidth(); x++ )
        {
            xcolor C = m_FogBMP.GetPixelColor( x,y );
            C.A = C.R;
            m_FogBMP.SetPixelColor( C, x,y );
        }
    }
    auxbmp_ConvertToNative( m_FogBMP );
    vram_Register( m_FogBMP );
}

//==============================================================================

void light_shaft_effect::Kill( void )
{
    // Text texture
    vram_Unregister( m_TextBMP );
    m_TextBMP.Kill();
    
    // Kill fog texture
    vram_Unregister( m_FogBMP );
    m_FogBMP.Kill();
}

//==============================================================================

void light_shaft_effect::Update( f32 DeltaTime )
{
    // Update rotation
    m_FogAngle += DeltaTime * m_FogRotSpeed;
    
    // Update pos
    m_Position += m_HorizSpeed * DeltaTime;
    if( m_Position > 1.3f )
    {
        m_Position = 1.3f;
        m_HorizSpeed = -m_HorizSpeed;
    }
    else if( m_Position < -0.3f )
    {
        m_Position = -0.3f;
        m_HorizSpeed = -m_HorizSpeed;
    }

    // Update zoom
    m_FogZoom -= m_FogZoomSpeed * DeltaTime;
    if( m_FogZoom < 0.0f )
        m_FogZoom += 1.0f;
}

//==============================================================================

void light_shaft_effect::Render( f32 Scale ) const
{
    s32 i;

    // Compute section to send light through
    s32 XRes,YRes;
    eng_GetRes( XRes, YRes );
    f32 w = (f32)m_TextBMP.GetWidth();
    f32 h = (f32)m_TextBMP.GetHeight();
    f32 x = ( XRes / 2 ) - ( ( (f32)w * Scale )* 0.5f );
    f32 y = ( YRes / 2 ) - ( ( (f32)h * Scale )* 0.5f );
    f32 c = ( m_Position * ( w - m_PixelWidth ) ) * Scale;
    vector3 FogCenter( x + c + ( ( Scale * m_PixelWidth ) / 2 ), 
                       y + ( ( Scale * h ) / 2 ), 0 );

    // Compute section of light shaft to draw
    f32 X0 = m_Position * ( w - m_PixelWidth );
    f32 X1 = X0 + m_PixelWidth;
    f32 U0 = X0 / w;
    f32 U1 = X1 / w;

    // Draw main text - just add to back ground without touching the alpha!
    draw_ClearL2W();
    draw_EnableBilinear();
    draw_SetTexture( m_TextBMP );
    draw_Begin( DRAW_RECTS, DRAW_2D | DRAW_NO_ZBUFFER | DRAW_TEXTURED );
	g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | 
                                                          D3DCOLORWRITEENABLE_GREEN | 
                                                          D3DCOLORWRITEENABLE_BLUE );//| D3DCOLORWRITEENABLE_ALPHA );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
    draw_Color( 1.0f, 1.0f, 1.0f, 1.0f );
    draw_UV( 0.0f, 0.0f );
    draw_Vertex( x, y, 0.0f );
    draw_UV( 1.0f, 1.0f );
    draw_Vertex( ( x + ( w * Scale ) ), ( y + ( h * Scale ) ), 0.0f );
    draw_End();

    // Render rotating, zooming fog into alpha channel. Each layer gets bigger and more transparent
    draw_SetTexture( m_FogBMP );
    draw_EnableBilinear();
    draw_Begin( DRAW_SPRITES, DRAW_2D | DRAW_NO_ZBUFFER | DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_BLEND_ADD | DRAW_U_CLAMP | DRAW_V_CLAMP );
	g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE  );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE  );
    for( i = 0; i < m_nFogPasses; i++ )
    {
        // Lookup shaft # with zoom
        f32 nShaft = i + ( m_FogZoom * (f32)m_nFogPasses );
        while( nShaft >= m_nFogPasses )
            nShaft -= m_nFogPasses;

        // Compute color
        f32 T    = (f32)nShaft / (f32)m_nFogPasses;
        f32 InvT = 1.0f - T;
        u8  Col = (u8)( InvT * m_FogAlpha * 255.0f );

        // Compute rotation speed and radius
        f32 Speed = T;
        f32 R = m_FogSize * Speed;
        f32 U = 0.0f;
        
        // Flip direction?
        if( i & 1 )
        {
            Speed = -Speed;
            U     = 1.0f;
        }

        // Draw fog layer
        draw_SpriteUV( vector3( FogCenter ),                            // Hot spot (3D Center)
                       vector2( R, R ),                                 // 3D World W&H
                       vector2( U,        0.0f ),                       // Upper Left   UV  [0.0 - 1.0]
                       vector2( 1.0f - U, 1.0f ),                       // Bottom Right UV  [0.0 - 1.0]
                       xcolor( Col, Col, Col, Col ),                    // Color
                       ( nShaft * R_5 ) + ( m_FogAngle * Speed ) );     // Rotation
    }
    draw_End();

    // Render light shafts by rendering layers of scaled up text (each layer more transparent)
    // To remove abrupt horizontal lines, the layers are rendered with 3 quads, with the left/right
    // qauds having a completely transparent edge.
    draw_EnableBilinear();
    draw_Begin( DRAW_QUADS, DRAW_CULL_NONE | DRAW_2D | DRAW_NO_ZBUFFER | DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_BLEND_ADD | DRAW_U_CLAMP | DRAW_V_CLAMP );
    draw_SetTexture( m_TextBMP );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE  );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
    g_pd3dDevice->SetRenderState( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_DESTALPHA );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED   | 
                                                          D3DCOLORWRITEENABLE_GREEN | 
                                                          D3DCOLORWRITEENABLE_BLUE  | D3DCOLORWRITEENABLE_ALPHA );
    for( i = 0; i < m_nPasses; i++ )
    {
        // Render single light shaft ( multiply by alpha in frame buffer)
        f32 T    = (f32)i / (f32)m_nPasses;
        f32 InvT = 1.0f - T;
        u8  A    = (u8)( 255.0f * m_Alpha * InvT );
        f32 WO   = w * ( T * m_HorizScale );
        f32 HO   = h * ( T * m_VertScale );
        xcolor   MidCol ( A, A, A, A );
        xcolor   EdgeCol( 0, 0, 0, 0 );

        // Compute rect values
        f32 u0 = U0;
        f32 u1 = U1;
        f32 v0 = 0.0f;
        f32 v1 = 1.0f;
        f32 x0 = x + ( ( X0 - WO ) * Scale );
        f32 y0 = y - ( HO * Scale );
        f32 x1 = x + ( ( X1 + WO ) * Scale );
        f32 y1 = y + ( ( h + HO ) * Scale );
        f32 z  = 0.0f;
        
        // Compute interior edge values
        f32 F  = 0.2f;
        f32 xi0 = x0 + ( F * ( x1 - x0 ) );
        f32 ui0 = u0 + ( F * ( u1 - u0 ) );
        f32 xi1 = x0 + ( ( 1.0f - F ) * ( x1 - x0 ) );
        f32 ui1 = u0 + ( ( 1.0f - F ) * ( u1 - u0 ) );

        // Draw left quad (fades from transparent -> opaque)
        draw_Color( EdgeCol );  draw_UV( u0,  v0 );  draw_Vertex( x0,  y0, z );
        draw_Color( MidCol  );  draw_UV( ui0, v0 );  draw_Vertex( xi0, y0, z );
        draw_Color( MidCol  );  draw_UV( ui0, v1 );  draw_Vertex( xi0, y1, z );
        draw_Color( EdgeCol );  draw_UV( u0,  v1 );  draw_Vertex( x0,  y1, z );

        // Draw mid (all opaque)
        draw_Color( MidCol  );  draw_UV( ui0, v0 );  draw_Vertex( xi0, y0, z );
        draw_Color( MidCol  );  draw_UV( ui1, v0 );  draw_Vertex( xi1, y0, z );
        draw_Color( MidCol  );  draw_UV( ui1, v1 );  draw_Vertex( xi1, y1, z );
        draw_Color( MidCol  );  draw_UV( ui0, v1 );  draw_Vertex( xi0, y1, z );

        // Draw right quad (fades from opaque -> transparent)
        draw_Color( MidCol  );  draw_UV( ui1, v0 );  draw_Vertex( xi1, y0, z );
        draw_Color( EdgeCol );  draw_UV( u1,  v0 );  draw_Vertex( x1,  y0, z );
        draw_Color( EdgeCol );  draw_UV( u1,  v1 );  draw_Vertex( x1,  y1, z );
        draw_Color( MidCol  );  draw_UV( ui1, v1 );  draw_Vertex( xi1, y1, z );
    }
    draw_End();
}

//==============================================================================
