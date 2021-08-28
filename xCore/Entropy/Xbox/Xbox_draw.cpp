///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_XBOX
#   include "xbox_private.hpp"
#endif

#include "e_Engine.hpp"

#ifndef CONFIG_RETAIL
static xbool SwitchOffDraw = FALSE;
#endif

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define NUM_VERTEX_BUFFERS          4                               // Number of vertex buffers to cycle through
#define NUM_VERTICES                1000                            // Number of vertices in each buffer
#define NUM_QUAD_INDICES            ((NUM_VERTICES/4)*6)            // Number of Quad indices needed

#define TRIGGER_POINTS              ((NUM_VERTICES/1)*1)            // Vertex indicies to trigger buffer dispatch
#define TRIGGER_LINES               ((NUM_VERTICES/2)*2)            // ...
#define TRIGGER_LINE_STRIPS         ((NUM_VERTICES/2)*2)            // ...
#define TRIGGER_TRIANGLES           ((NUM_VERTICES/3)*3)            // ...
#define TRIGGER_TRIANGLE_STRIPS     ((NUM_VERTICES/3)*3)            // ...
#define TRIGGER_QUADS               ((NUM_VERTICES/4)*4)            // ...
#define TRIGGER_RECTS               ((NUM_VERTICES/4))              // ...

#define NUM_SPRITES                 (NUM_VERTICES/4)                // Number of Sprites in sprite buffer

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

typedef void (*fnptr_dispatch)( void );

extern void xbox_FixedFunction();

///////////////////////////////////////////////////////////////////////////
// DRAW VERTEX
///////////////////////////////////////////////////////////////////////////

#define D3DFVF_DRAWVERTEX_2D  ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define D3DFVF_DRAWVERTEX_3D  ( D3DFVF_XYZ    | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define D3DFVF_DRAWVERTEX_3DN ( D3DFVF_XYZ    | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_NORMAL )  //with normal

struct drawvertex3d
{
    vector3p    Position;
    D3DCOLOR    Color;
    vector2     UV;
};

struct drawvertex3dn
{
    vector3p    Position;
    vector3p    Normal;
    D3DCOLOR    Color;
    vector2     UV;
};

struct drawvertex2d
{
    vector3p    Position;
    f32         RHW;
    D3DCOLOR    Color;
    vector2     UV;
};

struct drawsprite
{
    vector3p    Position;
    vector2     WH;
    D3DCOLOR    Color;
    vector2     UV0;
    vector2     UV1;
    radian      Rotation;
    xbool       IsRotated;
};

///////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////

xbool                       m_Initialized = FALSE;                  // Becomes TRUE when Initialized
xbool                       m_bBegin = FALSE;                       // TRUE when between begin/end

draw_primitive              m_Primitive;                            // Primitive Type, see enum draw_primitive
u32                         m_Flags;                                // Flags, see defines
xbool                       m_Is2D;                                 // TRUE for 2D mode
xbool                       m_IsTextured;                           // TRUE for Textured mode

xbool                       m_SatCompensation = FALSE;              // TRUE means we halve vertex colors

xbool                       m_HasNormal;                            // TRUE for lighted

matrix4                     m_L2W;                                  // L2W matrix for draw

///////////////////////////////////////////////////////////////////////////

const vector2*              m_pUVs;                                 // Pointer to UV array
s32                         m_nUVs;                                 // Number of elements
s32                         m_sUVs;                                 // Stride of elements

const xcolor*               m_pColors;                              // Pointer to Color array
s32                         m_nColors;                              // Number of elements
s32                         m_sColors;                              // Stride of elements

const vector3p*             m_pVerts;                               // Pointer to vertex array
const vector3p*             m_pNormals;                             // Pointer to normal array
s32                         m_nVerts;                               // Number of elements
s32                         m_sVerts;                               // Stride of elements

///////////////////////////////////////////////////////////////////////////

xbool                       m_bD3DImmBegin;
xbool                       m_bEnabled = 0;                         // Enable/Disable draw subsystem

s32                         m_ZBias;

drawsprite*                 m_pSpriteBuffer;                        // Sprite Buffer
s32                         m_iSprite;                              // Next Sprite Index

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void draw_SetZBias( s32 Bias )
{
    ASSERT( (Bias>=0) && (Bias<=16) );
    m_ZBias = Bias;
}

///////////////////////////////////////////////////////////////////////////
void draw_Init( void )
{
    m_ZBias = 0;

    ASSERT( !m_Initialized );

    m_bD3DImmBegin = FALSE;

    // Clear local to world
    m_L2W.Identity();

    // Allocate Sprite Buffer
    m_pSpriteBuffer = (drawsprite*)x_malloc( sizeof(drawsprite) * NUM_SPRITES );
    m_iSprite = 0;

    // Tell system we are initialized
    m_Initialized = TRUE;
    m_bEnabled = TRUE;
}

///////////////////////////////////////////////////////////////////////////

void draw_Kill( void )
{
    ASSERT( m_Initialized );

    // Free Sprite buffer
    x_free( m_pSpriteBuffer );

    // No longer initialized
    m_Initialized = FALSE;
}

///////////////////////////////////////////////////////////////////////////

__forceinline void inline_BeginAttrib( u32 Primitive )
{
    if( !m_bD3DImmBegin )
    {
        switch( Primitive )
        {
            case DRAW_POINTS          : VERIFY( !g_pd3dDevice->Begin( D3DPT_POINTLIST     )); break;
            case DRAW_LINES           : VERIFY( !g_pd3dDevice->Begin( D3DPT_LINELIST      )); break;
            case DRAW_LINE_STRIPS     : VERIFY( !g_pd3dDevice->Begin( D3DPT_LINESTRIP     )); break;
            case DRAW_TRIANGLES       : VERIFY( !g_pd3dDevice->Begin( D3DPT_TRIANGLELIST  )); break;
            case DRAW_TRIANGLE_STRIPS : VERIFY( !g_pd3dDevice->Begin( D3DPT_TRIANGLESTRIP )); break;
            default                   : VERIFY( !g_pd3dDevice->Begin( D3DPT_QUADLIST      )); break;
        }
        m_bD3DImmBegin = TRUE;
    }
}

///////////////////////////////////////////////////////////////////////////

static
void draw_DispatchSprites( void )
{
    s32 j;

    ASSERT( m_Primitive == DRAW_SPRITES );

    // If there are any sprites to draw
    if( m_iSprite > 0 )
    {
        // Loop through active views
        {
            // Get View
            const view* pView = eng_GetView( );
            ASSERT( pView );

            // Get Sprite and Vertex buffer pointers
            drawsprite*   pSprite = m_pSpriteBuffer;

            // Get W2V matrix
            const matrix4& W2V = pView->GetW2V();

            // Loop through sprites
            for( j=0; j<m_iSprite; j++ )
            {
                xcolor  Color = pSprite->Color;
                f32     U0    = pSprite->UV0.X;
                f32     V0    = pSprite->UV0.Y;
                f32     U1    = pSprite->UV1.X;
                f32     V1    = pSprite->UV1.Y;
                f32     w     = pSprite->WH.X / 2.0f;
                f32     h     = pSprite->WH.Y / 2.0f;
                xbool   isrot = pSprite->IsRotated;
                radian  a     = pSprite->Rotation;
                f32     s, c;
                vector3p v0;
                vector3p v1;

                // Construct points v0 and v1
                x_sincos( -a, s, c );

                v0.X = c*w - s*h;
                v0.Y = s*w + c*h;
                v0.Z = 0.0f;
                v1.X = c*w + s*h;
                v1.Y = s*w - c*h;
                v1.Z = 0.0f;

                // 2D or 3D?
                if( m_Is2D )
                {
                    // If not rotated then the sprites position is actually upper left corner, so offset
                    if( !isrot )
                    {
                        // Contruct corner points of quad
                        draw_Color( Color );
                        {
                            draw_UV    ( vector2( U0,V0 ));
                            draw_Vertex( vector3(
                                pSprite->Position.X,
                                pSprite->Position.Y,
                                0.5f ));

                            draw_UV    ( vector2( U0,V1 ));
                            draw_Vertex( vector3(
                                pSprite->Position.X,
                                pSprite->Position.Y+pSprite->WH.Y,
                                0.5f ));

                            draw_UV    ( vector2( U1,V1 ));
                            draw_Vertex( vector3(
                                pSprite->Position.X+pSprite->WH.X,
                                pSprite->Position.Y+pSprite->WH.Y,
                                0.5f ));

                            draw_UV    ( vector2( U1,V0 ));
                            draw_Vertex( vector3(
                                pSprite->Position.X+pSprite->WH.X,
                                pSprite->Position.Y,
                                0.5f ));
                        }
                    }
                    else
                    {
                        // Get center point
                        vector3p Center = pSprite->Position;

                        draw_Color( Color );
                        {
                            draw_UV    ( vector2( U0,V0 ));
                            draw_Vertex( Center - v0 );

                            draw_UV    ( vector2( U0,V1 ));
                            draw_Vertex( Center - v1 );

                            draw_UV    ( vector2( U1,V1 ));
                            draw_Vertex( Center + v0 );

                            draw_UV    ( vector2( U1,V0 ));
                            draw_Vertex( Center + v1 );
                        }
                    }

                    // Advance to next sprite
                    pSprite++;
                }
                else
                {
                    // Transform center point into view space
                    vector3 Center;
                    vector3 Position( pSprite->Position.X,
                                      pSprite->Position.Y,
                                      pSprite->Position.Z );
                    W2V.Transform( &Center, &Position, 1 );
                    {
                        draw_Color( Color );
                        {
                            draw_UV    ( vector2( U0,V0 ));
                            draw_Vertex( Center + v0 );

                            draw_UV    ( vector2( U0,V1 ));
                            draw_Vertex( Center + v1 );

                            draw_UV    ( vector2( U1,V1 ));
                            draw_Vertex( Center - v0 );

                            draw_UV    ( vector2( U1,V0 ));
                            draw_Vertex( Center - v1 );
                        }
                    }
                    // Advance to next sprite
                    pSprite++;
                }
            }
        }
    }

    // Clear Sprite Buffer
    m_iSprite = 0;
}

///////////////////////////////////////////////////////////////////////////

void draw_SetMatrices( const view* pView )
{
    // 3D Sprites are transformed into View Space before drawing, so only Projection needs setting,
    // WORLD and VIEW matrices will be identity
    xbool bWasImmBegin = m_bD3DImmBegin;
    if( m_bBegin )
    {
        if( m_iSprite )
            draw_DispatchSprites();
        if( m_bD3DImmBegin )
        {
            // Flush any drawing we have queued up
            VERIFY( !g_pd3dDevice->End( ));
            m_bD3DImmBegin = FALSE;
        }
    }

    if( m_Primitive == DRAW_SPRITES )
    {
        matrix4 m;
        m.Identity();
        g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&pView->GetV2C() );
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE0,   (D3DMATRIX*)&m );
        g_pd3dDevice->SetTransform( D3DTS_WORLD,      (D3DMATRIX*)&m );
        g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&m );
    }
    else
    {
        g_pd3dDevice->SetTransform( D3DTS_WORLD,      (D3DMATRIX*)&m_L2W );
        g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&pView->GetW2V() );
        g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&pView->GetV2C() );
    }

    xbool bBeginAttrib = (( m_Flags & DRAW_XBOX_NO_BEGIN )!=DRAW_XBOX_NO_BEGIN );
    if( bBeginAttrib && bWasImmBegin )
    {
        inline_BeginAttrib( m_Primitive );
    }
}

///////////////////////////////////////////////////////////////////////////

void(*g_DrawBeginCB)(void)=NULL;

///////////////////////////////////////////////////////////////////////////

void draw_Begin( draw_primitive Primitive, u32 Flags )
{
    CONTEXT( "draw_Begin" );

    if (!m_bEnabled)
        return;

    /* DRAW_XBOX_NO_BEGIN requires you call draw_Begin a second time
       once you've set up all your render states. This is because
       the GPU will crash if you do D3D->Begin() first.
       */

    if( m_bBegin )
    {
        if( ! m_bD3DImmBegin )
        {
            if( m_Flags &  DRAW_XBOX_NO_BEGIN )
                m_Flags &=~DRAW_XBOX_NO_BEGIN;
             inline_BeginAttrib( m_Primitive );
        }
        return;
    }

    ASSERT( m_Initialized );

    // Confirm we are in a render context
    #if !defined( X_DEBUG ) && !defined( TARGET_XBOX ) && DRAW_SAFE_AREA
    ASSERT( eng_InBeginEnd() );
    #endif

    if( g_DrawBeginCB )
        g_DrawBeginCB();

    // Save primitive and flags
    m_Primitive  = Primitive;
    m_Flags      = Flags;
    m_Is2D       = Flags & DRAW_2D;
    m_IsTextured = Flags & DRAW_TEXTURED;
    m_HasNormal  = Flags & DRAW_HAS_NORMAL;

    // Set internal state from primitive type
    const view* pView = eng_GetView( );
    ASSERT( pView );

    draw_SetMatrices( pView );

    // Disable lighting.
    if( Flags & ~DRAW_KEEP_STATES )
    {
        g_RenderState.Set( D3DRS_LIGHTING, FALSE );
        g_TextureStageState.Set( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );

        if( Flags & DRAW_TEXTURED )
        {
            g_TextureStageState.Set( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
            g_TextureStageState.Set( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
            g_TextureStageState.Set( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
        }
        else
        {
            g_TextureStageState.Set( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
            g_TextureStageState.Set( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE   );
            g_Texture.Clear(0);
        }

        // Turn off unused texture stages
        g_Texture.Clear( 1 );
        g_Texture.Clear( 2 );
        g_Texture.Clear( 3 );

        // Set D3D render states for ALPHA
        if( Flags & DRAW_USE_ALPHA )
        {
            g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
            g_RenderState.Set( D3DRS_SRCBLEND ,D3DBLEND_SRCALPHA );
            g_RenderState.Set( D3DRS_CULLMODE, D3DCULL_NONE );
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE, TRUE );

            g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_ENABLE );
            if( m_IsTextured )
            {
                g_TextureStageState.Set( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
                g_TextureStageState.Set( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
                g_TextureStageState.Set( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );

                g_RenderState.Set( D3DRS_ALPHAFUNC,D3DCMP_GREATER );
                g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );
                g_RenderState.Set( D3DRS_ALPHAREF,16 );
            }
            else
            {
                g_TextureStageState.Set( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                g_TextureStageState.Set( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE    );
            }
        }
        else
        {
            g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE, FALSE );
            g_RenderState.Set( D3DRS_ALPHATESTENABLE,  FALSE );
        }

        // Terminate the texture stages
        g_TextureStageState.Set( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
        g_TextureStageState.Set( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

        // Turn off stream source
        g_pd3dDevice->SetStreamSource( 0,0,0 );

        // Set D3D render states for ZBUFFER
        g_RenderState.Set( D3DRS_ZBIAS, m_ZBias );
        if( Flags & DRAW_NO_ZBUFFER )
        {
            g_RenderState.Set( D3DRS_ZENABLE, D3DZB_FALSE );
        }
        else
        {
            g_RenderState.Set( D3DRS_ZENABLE, D3DZB_TRUE );
        }

        // No Z write?
        if( Flags & DRAW_NO_ZWRITE )
        {
            g_RenderState.Set( D3DRS_ZWRITEENABLE, D3DZB_FALSE );
        }
        else
        {
            g_RenderState.Set( D3DRS_ZWRITEENABLE, D3DZB_TRUE );
        }

        // Add?
        if( Flags & DRAW_BLEND_ADD )
        {
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE, TRUE );
            g_RenderState.Set( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
            g_RenderState.Set( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
            g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_ONE );
        }
        // Sub?
        else if( Flags & DRAW_BLEND_SUB )
        {
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE, TRUE );
            g_RenderState.Set( D3DRS_BLENDOP  , D3DBLENDOP_REVSUBTRACT );
            g_RenderState.Set( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
            g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_ONE );
        }
        // Alpha
        else
        {
            g_RenderState.Set( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
            g_RenderState.Set( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
            g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
        }

        // Set D3D render states for CULLING
        if( Flags & DRAW_CULL_NONE )
        {
            g_RenderState.Set( D3DRS_CULLMODE, D3DCULL_NONE );
        }
        else
        {
            g_RenderState.Set( D3DRS_CULLMODE, D3DCULL_CW );
        }

        // Override cullmode: 2d always double sided
        if( m_Is2D )
        {
            g_RenderState.Set( D3DRS_CULLMODE, D3DCULL_NONE );
            g_RenderState.Set( D3DRS_FOGENABLE,FALSE );
        }

        // Set D3D render states for UV tiling
        if( Flags & DRAW_U_CLAMP )
        {
            g_TextureStageState.Set( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
        }
        else
        {
            g_TextureStageState.Set( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
        }

        // Set D3D render states for UV tiling
        if( Flags & DRAW_V_CLAMP )
        {
            g_TextureStageState.Set( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
        }
        else
        {
            g_TextureStageState.Set( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
        }

        // Set D3D render states for WIREFRAME
        if( Flags & DRAW_WIRE_FRAME )
        {
            g_RenderState.Set( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
        }
        else
        {
            g_RenderState.Set( D3DRS_FILLMODE, D3DFILL_SOLID );
        }

        // Set Shader
        g_pd3dDevice->SetShaderConstantMode( D3DSCM_96CONSTANTS );
        if( m_Is2D )
            g_pd3dDevice->SetVertexShader ( D3DFVF_DRAWVERTEX_2D );
        else if (m_HasNormal)
            g_pd3dDevice->SetVertexShader ( D3DFVF_DRAWVERTEX_3DN );
        else
            g_pd3dDevice->SetVertexShader ( D3DFVF_DRAWVERTEX_3D );
    }

    // Clear list pointers
    m_pUVs     = NULL;
    m_pColors  = NULL;
    m_pVerts   = NULL;
    m_pNormals = NULL;

    // Set in begin state
    m_bBegin = TRUE;

    // write to the alpha channel?
    if( Flags & DRAW_XBOX_WRITE_A )
    {
        g_RenderState.Set(
            D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_RED
            |   D3DCOLORWRITEENABLE_GREEN
            |   D3DCOLORWRITEENABLE_BLUE
            |   D3DCOLORWRITEENABLE_ALPHA );
    }
    else
    {
        g_RenderState.Set(
            D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_RED
            |   D3DCOLORWRITEENABLE_GREEN
            |   D3DCOLORWRITEENABLE_BLUE );
    }

    // D3D begin
    xbool bBeginAttrib = (( Flags & DRAW_XBOX_NO_BEGIN )!=DRAW_XBOX_NO_BEGIN );
    if  ( bBeginAttrib )
    {
        inline_BeginAttrib( m_Primitive );
    }
}

///////////////////////////////////////////////////////////////////////////

void draw_End( void )
{
    CONTEXT( "draw_End" );
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    if( m_iSprite )
        draw_DispatchSprites();
    if( m_bD3DImmBegin )
    {
        // Flush any drawing we have queued up
        VERIFY( !g_pd3dDevice->End( ));
        m_bD3DImmBegin = FALSE;
    }

    // Set D3D render states to normal for ZBUFFER
    g_RenderState.Set( D3DRS_ZENABLE, D3DZB_TRUE );
    g_RenderState.Set( D3DRS_ZBIAS, 0 );

    // Clear in begin state
    m_bBegin = FALSE;
}

///////////////////////////////////////////////////////////////////////////

void draw_SetL2W( const matrix4& L2W )
{
    ASSERT( !m_bBegin );

    if (!m_bEnabled)
        return;

    m_L2W = L2W;
}

///////////////////////////////////////////////////////////////////////////

const matrix4 draw_GetL2W( void )
{
    return m_L2W;
}

///////////////////////////////////////////////////////////////////////////

void draw_ClearL2W( void )
{
    if (!m_bEnabled)
        return;

    m_L2W.Identity();
}

///////////////////////////////////////////////////////////////////////////

void draw_SetTexture( const xbitmap& Bitmap )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );

    xbool bWasImmBegin = m_bD3DImmBegin;
    {
        ASSERT( m_bBegin );
        if( m_iSprite )
            draw_DispatchSprites();
        if( m_bD3DImmBegin )
        {
            // Flush any drawing we have queued up
            VERIFY( !g_pd3dDevice->End( ));
            m_bD3DImmBegin = FALSE;
        }
    }

    // Activate the texture if Textured mode is set
    vram_Activate( Bitmap );
    {
        xbool bBeginAttrib = (( m_Flags & DRAW_XBOX_NO_BEGIN )!=DRAW_XBOX_NO_BEGIN );
        if  ( bBeginAttrib && bWasImmBegin )
        {
            inline_BeginAttrib( m_Primitive );
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void draw_SetTexture( void )
{
//    if (!m_bEnabled)
//        return;
//
//    vram_Activate();
}

///////////////////////////////////////////////////////////////////////////

void draw_UV( const vector2& UV )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );

    #ifndef CONFIG_RETAIL
    if( ! SwitchOffDraw )
    #endif
    VERIFY( !g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0,UV.X,UV.Y ));
}

///////////////////////////////////////////////////////////////////////////

void draw_UV( f32 U, f32 V )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );

    #ifndef CONFIG_RETAIL
    if( ! SwitchOffDraw )
    #endif
    VERIFY( !g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0,U,V ));
}

///////////////////////////////////////////////////////////////////////////

void draw_Color( const xcolor& Color )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    D3DCOLOR C;

    if( m_SatCompensation )
    {
        C = D3DCOLOR_RGBA(
                Color.R>>1,
                Color.G>>1,
                Color.B>>1,
                Color.A );
    }
    else
    {
        C = D3DCOLOR_RGBA(
            Color.R,
            Color.G,
            Color.B,
            Color.A );
    }


    #ifndef CONFIG_RETAIL
    if( ! SwitchOffDraw )
    #endif
    VERIFY( !g_pd3dDevice->SetVertexDataColor( D3DVSDE_DIFFUSE,C ));
}

///////////////////////////////////////////////////////////////////////////

void draw_Color( f32 R, f32 G, f32 B, f32 A )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );

    D3DCOLOR C = D3DCOLOR_RGBA
    (
        u32(R*255.0f),
        u32(G*255.0f),
        u32(B*255.0f),
        u32(A*255.0f)
    );

    #ifndef CONFIG_RETAIL
    if( ! SwitchOffDraw )
    #endif
    VERIFY( !g_pd3dDevice->SetVertexDataColor( D3DVSDE_DIFFUSE,C ));
}

///////////////////////////////////////////////////////////////////////////

void draw_Normal( const vector3& Normal )
{
    #ifndef CONFIG_RETAIL
    if( ! SwitchOffDraw )
    #endif
    VERIFY( !g_pd3dDevice->SetVertexData4f( D3DVSDE_NORMAL,Normal.GetX(),Normal.GetY(),Normal.GetZ(),1.0f ));
}

///////////////////////////////////////////////////////////////////////////

void draw_Normal( f32 X, f32 Y, f32 Z )
{
    #ifndef CONFIG_RETAIL
    if( ! SwitchOffDraw )
    #endif
    VERIFY( !g_pd3dDevice->SetVertexData4f( D3DVSDE_NORMAL,X,Y,Z,1.0f  ));
}

///////////////////////////////////////////////////////////////////////////

void draw_Vertex( const vector3& Vertex )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );

    // Must do it here thanks to SetTexture() before Begin() requirement.
    #ifndef CONFIG_RETAIL
    if( ! SwitchOffDraw )
    #endif
    g_pd3dDevice->SetVertexData4f(
        D3DVSDE_VERTEX,
        Vertex.GetX(),
        Vertex.GetY(),
        Vertex.GetZ(),
        1.0f
    );
}

///////////////////////////////////////////////////////////////////////////

void draw_Vertex( f32 X, f32 Y, f32 Z )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );

    #ifndef CONFIG_RETAIL
    if( ! SwitchOffDraw )
    #endif
    g_pd3dDevice->SetVertexData4f(
        D3DVSDE_VERTEX,
        X,
        Y,
        Z,
        1.0f
    );
}

///////////////////////////////////////////////////////////////////////////

void draw_UVs( const vector2* pUVs, s32 Count, s32 Stride )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );

    m_pUVs = pUVs;
    m_nUVs = Count;
    m_sUVs = Stride;
}

///////////////////////////////////////////////////////////////////////////

void draw_Colors( const xcolor*  pColors, s32 Count, s32 Stride )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pColors = pColors;
    m_nColors = Count;
    m_sColors = Stride;
}

///////////////////////////////////////////////////////////////////////////

void draw_Verts( const vector3* pVerts, s32 Count, s32 Stride )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pVerts = (const vector3p*)pVerts;
    m_nVerts = Count;
    m_sVerts = Stride;
}

///////////////////////////////////////////////////////////////////////////

void draw_Verts( const vector3* pVerts, const vector3* pNormals,  s32 Count, s32 Stride )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pVerts = (const vector3p*)pVerts;
    m_nVerts = Count;
    m_sVerts = Stride;
    m_pNormals = (const vector3p*)pNormals;
}

///////////////////////////////////////////////////////////////////////////

static void s_LoadExtraVertData( s32 Index )
{
    /* load uv coords */

    ASSERT( m_pUVs );
    ASSERT( Index < m_nUVs );
    vector2& UV = *((vector2*)&(((byte*)m_pUVs)[m_sUVs*Index]));
    draw_UV( UV );

    /* load colour */

    ASSERT( m_pColors );
    ASSERT( Index < m_nColors );
    xcolor& Color = *((xcolor*)&(((byte*)m_pColors)[m_sColors*Index]));
    draw_Color( Color );
}

///////////////////////////////////////////////////////////////////////////

void draw_Index( s32 Index )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    if( Index > -1 )
    {
        if( m_Is2D ) ///////////////////////////////////////////////////////////
        {
            ASSERT( Index < m_nVerts );

            s_LoadExtraVertData( Index );

            vector3p& Pos = *((vector3p*)&(((byte*)m_pVerts)[m_sVerts*Index]));

            #ifndef CONFIG_RETAIL
            if( ! SwitchOffDraw )
            #endif
            g_pd3dDevice->SetVertexData4f(
                D3DVSDE_VERTEX,
                Pos.X,
                Pos.Y,
                Pos.Z,
                1.0f
            );
        }
        else if( m_HasNormal ) /////////////////////////////////////////////////
        {
            ASSERT( Index < m_nVerts );

            s_LoadExtraVertData( Index );

            vector3p& Norm = *((vector3p*)&(((byte*)m_pNormals)[m_sVerts*Index]));
            vector3p& Pos  = *((vector3p*)&(((byte*)m_pVerts  )[m_sVerts*Index]));

            #ifndef CONFIG_RETAIL
            if( ! SwitchOffDraw )
            #endif
            g_pd3dDevice->SetVertexData4f(
                D3DVSDE_NORMAL,
                Norm.X,
                Norm.Y,
                Norm.Z,
                1.0f
            );
            #ifndef CONFIG_RETAIL
            if( ! SwitchOffDraw )
            #endif
            g_pd3dDevice->SetVertexData4f(
                D3DVSDE_VERTEX,
                Pos.X,
                Pos.Y,
                Pos.Z,
                1.0f
            );
        }
        else
        {
            ASSERT( Index < m_nVerts );

            s_LoadExtraVertData( Index );

            vector3p& Pos  = *((vector3p*)&(((byte*)m_pVerts)[m_sVerts*Index]));

            #ifndef CONFIG_RETAIL
            if( ! SwitchOffDraw )
            #endif
            g_pd3dDevice->SetVertexData4f(
                D3DVSDE_VERTEX,
                Pos.X,
                Pos.Y,
                Pos.Z,
                1.0f
            );
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void draw_Execute( const s16* pIndices, s32 NIndices )
{
    if (!m_bEnabled)
        return;

    s32     i;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    // Loop through indices supplied
    for( i=0; i<NIndices; i++ )
    {
        // Read Index
        s32 Index = pIndices[i];

        // Kick on -1, otherwise add to buffer
        //if( Index == -1 )
        //{
        //    m_pfnDispatch();
        //}
        //else
            draw_Index( pIndices[i] );
    }
}

///////////////////////////////////////////////////////////////////////////

void draw_Sprite( const vector3& Position,  // Hot spot (2D Left-Top), (3D Center)
                  const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                  const xcolor & Color )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_SPRITES );

    drawsprite* p = &m_pSpriteBuffer[m_iSprite];

    p->IsRotated = FALSE;
    p->Position  = Position;
    p->WH        = WH;
    p->Color     = Color;
    p->Rotation  = 0.0f;
    p->UV0.Set( 0.0f, 0.0f );
    p->UV1.Set( 1.0f, 1.0f );

    m_iSprite++;
    if( m_iSprite == NUM_SPRITES )
    {
        draw_DispatchSprites();
    }
}

///////////////////////////////////////////////////////////////////////////

void    draw_SpriteUV   ( const vector3& Position,  // Hot spot (2D Left-Top), (3D Center)
                          const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                          const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                          const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                          const xcolor & Color )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_SPRITES );

    drawsprite* p = &m_pSpriteBuffer[m_iSprite];

    p->IsRotated = FALSE;
    p->Position  = Position;
    p->WH        = WH;
    p->UV0       = UV0;
    p->UV1       = UV1;
    p->Color     = Color;
    p->Rotation  = 0.0f;

    m_iSprite++;
    if( m_iSprite == NUM_SPRITES )
    {
        draw_DispatchSprites();
    }
}

///////////////////////////////////////////////////////////////////////////

void    draw_SpriteUV   ( const vector3& Position,  // Hot spot (3D Center)
                          const vector2& WH,        // (3D World W&H)
                          const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                          const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                          const xcolor&  Color,     //
                                radian   Rotate )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_SPRITES );

    drawsprite* p = &m_pSpriteBuffer[m_iSprite];

    p->IsRotated = TRUE;
    p->Position  = Position;
    p->WH        = WH;
    p->UV0       = UV0;
    p->UV1       = UV1;
    p->Color     = Color;
    p->Rotation  = Rotate;

    m_iSprite++;
    if( m_iSprite == NUM_SPRITES )
    {
        draw_DispatchSprites();
    }
}


//==========================================================================

void    draw_OrientedQuad( const vector3& Pos0,
                           const vector3& Pos1,
                           const vector2& UV0,
                           const vector2& UV1,
                           const xcolor & Color0,
                           const xcolor & Color1,
                                 f32      Radius )
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_TRIANGLES );

    vector3 Dir = Pos1 - Pos0;
    Dir.Normalize();

    vector3 CrossDir = Dir.Cross( eng_GetView()->GetPosition() - Pos0 );
    CrossDir.Normalize();
    CrossDir *= Radius;

    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( Pos1 + CrossDir );
    draw_UV( UV1.X, UV0.Y );    draw_Vertex( Pos1 - CrossDir );
    draw_Color( Color0 );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( Pos0 - CrossDir );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( Pos0 - CrossDir );
    draw_UV( UV0.X, UV1.Y );    draw_Vertex( Pos0 + CrossDir );
    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( Pos1 + CrossDir );
}

//==========================================================================

void    draw_OrientedQuad( const vector3& Pos0,
                           const vector3& Pos1,
                           const vector2& UV0,
                           const vector2& UV1,
                           const xcolor & Color0,
                           const xcolor & Color1,
                                 f32      Radius0,
                                 f32      Radius1)
{
    if (!m_bEnabled)
        return;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_TRIANGLES );

    vector3 Dir = Pos1 - Pos0;
    Dir.Normalize();

    vector3 CrossDir = Dir.Cross( eng_GetView()->GetPosition() - Pos0 );
    CrossDir.Normalize();
    vector3 Cross0 = CrossDir * Radius0;
    vector3 Cross1 = CrossDir * Radius1;

    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( Pos1 + Cross1 );
    draw_UV( UV1.X, UV0.Y );    draw_Vertex( Pos1 - Cross1 );
    draw_Color( Color0 );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( Pos0 - Cross0 );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( Pos0 - Cross0 );
    draw_UV( UV0.X, UV1.Y );    draw_Vertex( Pos0 + Cross0 );
    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( Pos1 + Cross1 );
}

//==========================================================================

void    draw_OrientedStrand( const vector3* pPosData,
                                   s32      NumPts,
                             const vector2& UV0,
                             const vector2& UV1,
                             const xcolor & Color0,
                             const xcolor & Color1,
                                   f32      Radius )
{
    if (!m_bEnabled)
        return;

    s32 i;
    vector3p quad[6];        //  storage for a quad, plus an extra edge for averaging
    vector2 uv0, uv1;

    uv0 = UV0;
    uv1 = UV1;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_TRIANGLES );

    for ( i = 0; i < NumPts - 1; i++ )
    {
        vector3 Dir = pPosData[i+1] - pPosData[i];
        Dir.Normalize();

        vector3 CrossDir = Dir.Cross( eng_GetView()->GetPosition() - pPosData[i] );
        CrossDir.Normalize();
        CrossDir *= Radius;

        if ( i == 0 )
        {
            // first set, no point averaging necessary
            quad[ 0 ] =     pPosData[ i ] + CrossDir;
            quad[ 1 ] =     pPosData[ i ] - CrossDir;
            quad[ 2 ] =     pPosData[ i + 1 ] + CrossDir;
            quad[ 3 ] =     pPosData[ i + 1 ] - CrossDir;
        }
        else
        {
            vector3p tq1, tq2;

            tq1 = pPosData[ i ] + CrossDir;
            tq2 = pPosData[ i ] - CrossDir;

            // second set...average verts
            quad[ 2 ] =     ( quad[2] + tq1 ) / 2.0f;
            quad[ 3 ] =     ( quad[3] + tq2 ) / 2.0f;
            quad[ 4 ] =     pPosData[ i + 1 ] + CrossDir;
            quad[ 5 ] =     pPosData[ i + 1 ] - CrossDir;            
        }

        // render q0, q1, q2, and q3 then shift all of them down
        if ( i > 0 )
        {
            draw_Color( Color1 );
            draw_UV( uv1.X, uv1.Y );    draw_Vertex( quad[2] );
            draw_UV( uv1.X, uv0.Y );    draw_Vertex( quad[3] );
            draw_Color( Color0 );
            draw_UV( uv0.X, uv0.Y );    draw_Vertex( quad[1] );
            draw_UV( uv0.X, uv0.Y );    draw_Vertex( quad[1] );
            draw_UV( uv0.X, uv1.Y );    draw_Vertex( quad[0] );
            draw_Color( Color1 );
            draw_UV( uv1.X, uv1.Y );    draw_Vertex( quad[2] );

            // cycle the UV's
            uv0.X = uv1.X;
            uv1.X += ( UV1.X - UV0.X );
            quad[0] = quad[2];
            quad[1] = quad[3];
            quad[2] = quad[4];
            quad[3] = quad[5];
        }
        
    }

    // last edge...
    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( quad[2] );
    draw_UV( UV1.X, UV0.Y );    draw_Vertex( quad[3] );
    draw_Color( Color0 );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( quad[1] );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( quad[1] );
    draw_UV( UV0.X, UV1.Y );    draw_Vertex( quad[0] );
    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( quad[2] );

}

//==============================================================================

void draw_ClearZBuffer( const irect& Rect )
{
    if (!m_bEnabled)
        return;

}


void draw_Enable( xbool Enable )
{
    ASSERT(!m_bBegin);
    m_bEnabled = Enable;
}


void draw_DisableBilinear(void)
{
    g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
    g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
}

void draw_EnableBilinear(void)
{
    g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
}

void draw_EnableSatCompensation( void )
{
    m_SatCompensation = TRUE;
}

void draw_DisableSatCompensation( void )
{
    m_SatCompensation = FALSE;
}