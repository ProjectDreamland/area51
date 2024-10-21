///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "e_Engine.hpp"

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

///////////////////////////////////////////////////////////////////////////
// DRAW VERTEX
///////////////////////////////////////////////////////////////////////////

#define D3DFVF_DRAWVERTEX_2D (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define D3DFVF_DRAWVERTEX_3D (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

struct drawvertex3d
{
    vector3p    Position;
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
    vector3     Position;
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

matrix4                     m_L2W;                                  // L2W matrix for draw

const vector2*              m_pUVs;                                 // Pointer to UV array
s32                         m_nUVs;                                 // Number of elements
s32                         m_sUVs;                                 // Stride of elements

const xcolor*               m_pColors;                              // Pointer to Color array
s32                         m_nColors;                              // Number of elements
s32                         m_sColors;                              // Stride of elements

const vector3*              m_pVerts;                               // Poitner to vertex array
s32                         m_nVerts;                               // Number of elements
s32                         m_sVerts;                               // Stride of elements

vector2                     m_UV;                                   // Current UV
xcolor                      m_Color;                                // Current Color

s32                         m_iActiveBuffer;                        // Active vertex buffer index
IDirect3DVertexBuffer9*     m_pVertexBuffer3d[NUM_VERTEX_BUFFERS];  // Array of vertex buffer pointers
drawvertex3d*               m_pActiveBuffer3dStart;                 // Active vertex buffer data pointer
drawvertex3d*               m_pActiveBuffer3d;                      // Active vertex buffer data pointer
IDirect3DVertexBuffer9*     m_pVertexBuffer2d[NUM_VERTEX_BUFFERS];  // Array of vertex buffer pointers
drawvertex2d*               m_pActiveBuffer2dStart;                 // Active vertex buffer data pointer
drawvertex2d*               m_pActiveBuffer2d;                      // Active vertex buffer data pointer

s32                         m_iVertex;                              // Index of vertex in buffer
s32                         m_iTrigger;                             // Index of vertex to trigger flush

IDirect3DIndexBuffer9*      m_pIndexQuads;                          // Index array for rendering Quads

drawsprite*                 m_pSpriteBuffer;                        // Sprite Buffer
s32                         m_iSprite;                              // Next Sprite Index

fnptr_dispatch              m_pfnDispatch;                          // Dispatch Function

s32                         m_ZBias;

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void draw_SetZBias( s32 Bias )
{
    ASSERT( (Bias>=0) && (Bias<=16) );
    m_ZBias = Bias;
}

void draw_Init( void )
{
    s32     i;
    s32     v;
    u16*    pIndex;
    dxerr   Error;

    m_ZBias = 0;

    ASSERT( !m_Initialized );

    if( g_pd3dDevice )
    {
        // Allocate vertex buffers
        for( i=0 ; i<NUM_VERTEX_BUFFERS ; i++ )
        {
            Error = g_pd3dDevice->CreateVertexBuffer( NUM_VERTICES*sizeof(drawvertex3d),
                                                      D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
                                                      D3DFVF_DRAWVERTEX_3D,
                                                      D3DPOOL_SYSTEMMEM, // Faster for small batches
                                                      //D3DPOOL_DEFAULT, 
                                                      &m_pVertexBuffer3d[i],
                                                      NULL );
            ASSERT( Error == 0 );

            Error = g_pd3dDevice->CreateVertexBuffer( NUM_VERTICES*sizeof(drawvertex2d),
                                                      D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
                                                      D3DFVF_DRAWVERTEX_2D,
                                                      D3DPOOL_SYSTEMMEM, // Faster for small batches
                                                      //D3DPOOL_DEFAULT, 
                                                      &m_pVertexBuffer2d[i],
                                                      NULL );
            ASSERT( Error == 0 );
        }

        // Set active vertex buffer
        m_iActiveBuffer = 0;
        Error = m_pVertexBuffer3d[m_iActiveBuffer]->Lock( 0, 0, (void**)&m_pActiveBuffer3dStart, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD );
        ASSERT( Error == 0 );
        Error = m_pVertexBuffer2d[m_iActiveBuffer]->Lock( 0, 0, (void**)&m_pActiveBuffer2dStart, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD );
        ASSERT( Error == 0 );
        m_pActiveBuffer3d = m_pActiveBuffer3dStart;
        m_pActiveBuffer2d = m_pActiveBuffer2dStart;
        m_iVertex = 0;

        // Allocate index buffer for Quads
        {
            Error = g_pd3dDevice->CreateIndexBuffer( NUM_QUAD_INDICES*sizeof(u16),
                                                     D3DUSAGE_WRITEONLY, // | D3DUSAGE_DYNAMIC,
                                                     D3DFMT_INDEX16,
                                                     D3DPOOL_MANAGED,
                                                     &m_pIndexQuads,
                                                     NULL );
            ASSERT( Error == 0 );
            // First lets lock the buffer
            Error = m_pIndexQuads->Lock( 0, 0, (void**)&pIndex, 0 );
            ASSERT( Error == 0 );

            for( v=i=0; i<NUM_QUAD_INDICES; i += 6, v += 4 )
            {
                pIndex[i+0] = v;    
                pIndex[i+1] = v+1;
                pIndex[i+2] = v+2;

                pIndex[i+3] = v+0;
                pIndex[i+4] = v+2;
                pIndex[i+5] = v+3;
            }

            // Unlock the buffer we are done!
            Error = m_pIndexQuads->Unlock();
            ASSERT( Error == 0 );
        }
    }

    // Allocate Sprite Buffer
    m_pSpriteBuffer = (drawsprite*)x_malloc( sizeof(drawsprite) * NUM_SPRITES );
    m_iSprite = 0;

    // Clear L2W matrix, UV, Color and Vertex
    m_L2W.Identity();
    m_UV    = vector2( 0.0f, 0.0f );
    m_Color = xcolor( 255, 255, 255, 255 );

    // Clear pointers
    m_pUVs    = NULL;
    m_pColors = NULL;
    m_pVerts  = NULL;

    // Tell system we are initialized
    m_Initialized = TRUE;
}

///////////////////////////////////////////////////////////////////////////

void draw_Kill( void )
{
    s32     i;
    dxerr   Error;

    ASSERT( m_Initialized );

    if( g_pd3dDevice )
    {
        // Release vertex buffers
        for( i=0 ; i<NUM_VERTEX_BUFFERS ; i++ )
        {
            Error = m_pVertexBuffer3d[i]->Release();
            ASSERT( Error == 0 );
            Error = m_pVertexBuffer2d[i]->Release();
            ASSERT( Error == 0 );
        }

        // Release index buffer
        Error = m_pIndexQuads->Release();
        //ASSERT( Error == 0 );
    }

    // Free Sprite buffer
    x_free( m_pSpriteBuffer );

    // No longer initialized
    m_Initialized = FALSE;
}

///////////////////////////////////////////////////////////////////////////

static
void draw_SetMatrices( const view* pView )
{
    if( !g_pd3dDevice )
        return;

    // Set Viewport for rendering
    eng_SetViewport( *pView );

/*
    // 2D or 3D
    if( m_Is2D )
    {
        matrix4 m, pm;
        s32     x0,y0,x1,y1;
        s32     w, h;
        f32     ZNear, ZFar;

        // Get Viewport info
        pView->GetViewport(x0,y0,x1,y1);
        pView->GetZLimits(ZNear,ZFar);
        w = x1-x0;
        h = y1-y0;

        // Set matrices to identity
        m.Identity();
        pm = m;

        // Clamp Z to ZNear for 2D work
        m(2,2) = 0.0f;
        m(3,2) = ZNear;

        // Make Screen to Clip matrix for D3D, this is used to counteract the
        // Clip to Screen matrix that is hard coded into D3D
        pm(0,0) = 2.0f/w;
        pm(1,1) = -2.0f/h;
        pm(2,2) = 1.0f/(ZFar-ZNear);        // 0.0f;
        pm(3,3) = 1.0f;
        pm(3,0) = -1.0f;
        pm(3,1) = +1.0f;
        pm(3,2) = 0.0f;

        g_pd3dDevice->SetTransform( D3DTS_WORLD,      (D3DMATRIX*)&m );
        g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&m );
        g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&pm );
    }
    else
*/
    {
        // 3D Sprites are transformed into View Space before drawing, so only Projection needs setting,
        // WORLD and VIEW matrices will be identity
        if( m_Primitive == DRAW_SPRITES )
        {
            matrix4 m;
            m.Identity();
            g_pd3dDevice->SetTransform( D3DTS_WORLD,      (D3DMATRIX*)&m );
            g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&m );
            g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&pView->GetV2C() );
        }
        else
        {
            g_pd3dDevice->SetTransform( D3DTS_WORLD,      (D3DMATRIX*)&m_L2W );
            g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&pView->GetW2V() );
            g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&pView->GetV2C() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////

static
void draw_Dispatch( void )
{
    if( !g_pd3dDevice )
        return;

    dxerr   Error;

    if( m_iVertex != 0 )
    {
        // Unlock buffer
        Error = m_pVertexBuffer3d[m_iActiveBuffer]->Unlock();
        ASSERT( Error == 0 );
        Error = m_pVertexBuffer2d[m_iActiveBuffer]->Unlock();
        ASSERT( Error == 0 );

        // Get View
        const view* pView = eng_GetView();
        ASSERT( pView );

        // Setup D3D Matrices
        draw_SetMatrices( pView );

        // Set stream source
        if( m_Is2D )
            Error = g_pd3dDevice->SetStreamSource( 0, m_pVertexBuffer2d[m_iActiveBuffer], 0, sizeof(drawvertex2d) );
        else
            Error = g_pd3dDevice->SetStreamSource( 0, m_pVertexBuffer3d[m_iActiveBuffer], 0, sizeof(drawvertex3d) );
        ASSERT( Error == 0 );

        // Render buffer
        switch( m_Primitive )
        {
        case DRAW_POINTS:
            g_pd3dDevice->DrawPrimitive( D3DPT_POINTLIST,     0, m_iVertex   );
            break;

        case DRAW_LINES:
            g_pd3dDevice->DrawPrimitive( D3DPT_LINELIST,      0, m_iVertex/2 );
            break;

        case DRAW_LINE_STRIPS:
            g_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP,     0, m_iVertex-1 );
            break;

        case DRAW_TRIANGLES:
            g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST,  0, m_iVertex/3 );
            break;

        case DRAW_TRIANGLE_STRIPS:
            g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, m_iVertex-2 );
            break;

        case DRAW_QUADS:
        case DRAW_RECTS:
        case DRAW_SPRITES:
            Error = g_pd3dDevice->SetIndices( m_pIndexQuads );
            ASSERT( Error == 0 );
            Error = g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_iVertex, 0, m_iVertex/2 );
            ASSERT( Error == 0 );
            break;
        }

        // Set new active vertex buffer
        m_iActiveBuffer++;
        if( m_iActiveBuffer == NUM_VERTEX_BUFFERS ) m_iActiveBuffer = 0;
        
        Error = m_pVertexBuffer3d[m_iActiveBuffer]->Lock( 0, 0, (void**)&m_pActiveBuffer3dStart, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD );
        ASSERT( Error == 0 );
        m_pActiveBuffer3d = m_pActiveBuffer3dStart;

        Error = m_pVertexBuffer2d[m_iActiveBuffer]->Lock( 0, 0, (void**)&m_pActiveBuffer2dStart, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD );
        ASSERT( Error == 0 );
        m_pActiveBuffer2d = m_pActiveBuffer2dStart;

        m_iVertex = 0;
    }
}

///////////////////////////////////////////////////////////////////////////

static
void draw_DispatchRects( void )
{
    if( !g_pd3dDevice )
        return;

    s32             nQuads = m_iVertex/2;
    s32             i;

    // Only if there are quads to process
    if( nQuads > 0 )
    {
        if( m_Is2D )
        {
            drawvertex2d*   ps     = &m_pActiveBuffer2dStart[(nQuads-1)*2];
            drawvertex2d*   pd     = &m_pActiveBuffer2dStart[(nQuads-1)*4];

            // Rects are specified with top-left and bottom-right corners, expand the data in place
            // to include all points to make quads, then render
            for( i=nQuads ; i>0 ; i-- )
            {
                pd[3] = ps[0];
                pd[2] = ps[1];
                pd[1] = pd[2];
                pd[0] = pd[3];

                pd[3].Position.X = pd[2].Position.X;
                pd[1].Position.X = pd[0].Position.X;
                pd[3].UV.X       = pd[2].UV.X;
                pd[1].UV.X       = pd[0].UV.X;

                ps -= 2;
                pd -= 4;
            }

            m_iVertex = nQuads*4;
        }
        else
        {
            drawvertex3d*   ps     = &m_pActiveBuffer3dStart[(nQuads-1)*2];
            drawvertex3d*   pd     = &m_pActiveBuffer3dStart[(nQuads-1)*4];

            // Rects are specified with top-left and bottom-right corners, expand the data in place
            // to include all points to make quads, then render
            for( i=nQuads ; i>0 ; i-- )
            {
                pd[3] = ps[0];
                pd[2] = ps[1];
                pd[1] = pd[2];
                pd[0] = pd[3];

                pd[3].Position.X = pd[2].Position.X;
                pd[1].Position.X = pd[0].Position.X;
                pd[3].UV.X       = pd[2].UV.X;
                pd[1].UV.X       = pd[0].UV.X;

                ps -= 2;
                pd -= 4;
            }

            m_iVertex = nQuads*4;
        }

        // Call regular Dispatch function
        draw_Dispatch();
    }
}

//==========================================================================

static inline void draw_sincos( radian Angle, f32& Sine, f32& Cosine )
{
    #define I_360   (1<<16)
    #define I_90    (I_360/4)
    #define I_180   (I_360/2)
    #define I_270   (I_90*3)

    s32 IAngle = ((s32)(Angle*(I_360/R_360)))&(I_360-1);
    s32 si,ci;

    if( IAngle >= I_180 )
    {
        if( IAngle >= I_270 ) { si =  IAngle - I_360; ci =  IAngle - I_270; }
        else                  { si = -IAngle + I_180; ci =  IAngle - I_270; }
    }
    else
    {
        if( IAngle >= I_90 )  { si = -IAngle + I_180; ci = -IAngle + I_90;  }
        else                  { si = IAngle;          ci = -IAngle + I_90;  }
    }

    f32 sq  = si*(R_360/I_360);
    f32 cq  = ci*(R_360/I_360);
    f32 sq2 = sq*sq;
    f32 cq2 = cq*cq;
    Sine   = (((0.00813767f*sq2) - 0.1666666f)*sq2 + 1)*sq;
    Cosine = (((0.00813767f*cq2) - 0.1666666f)*cq2 + 1)*cq;

    #undef I_360
    #undef I_90
    #undef I_180
    #undef I_270
}

///////////////////////////////////////////////////////////////////////////

static
void draw_DispatchSprites( void )
{
    if( !g_pd3dDevice )
        return;

    s32     j;

    ASSERT( m_Primitive == DRAW_SPRITES );
    ASSERT( m_iVertex == 0 );

    // If there are any sprites to draw
    if( m_iSprite > 0 )
    {
        // Get View
        const view* pView = eng_GetView();
        ASSERT( pView );

        // Get Sprite and Vertex buffer pointers
        drawsprite*   pSprite = m_pSpriteBuffer;
        drawvertex3d* pVertex3d = m_pActiveBuffer3dStart;
        drawvertex2d* pVertex2d = m_pActiveBuffer2dStart;

        // Get W2V matrix
        const matrix4& W2V = pView->GetW2V();

        // Loop through sprites
        for( j=0 ; j<m_iSprite ; j++ )
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
            vector3 v0;
            vector3 v1;

            // Construct points v0 and v1
            //x_sincos( a, s, c );
            draw_sincos( -a, s, c );
            v0.Set( c*w - s*h,
                    s*w + c*h,
                    0.0f );
            v1.Set( c*w + s*h,
                    s*w - c*h,
                    0.0f );

            // 2D or 3D?
            if( m_Is2D )
            {
                // If not rotated then the sprites position is actually upper left corner, so offset
                if( !isrot )
                {
                    // Contruct corner points of quad
                    pVertex2d->Position.X = pSprite->Position.GetX();
                    pVertex2d->Position.Y = pSprite->Position.GetY();
                    pVertex2d->Position.Z = 0.5f;
                    pVertex2d->RHW        = 1.0f;
                    pVertex2d->Color      = Color;
                    pVertex2d->UV         = vector2(U0,V0);
                    pVertex2d++;
                    pVertex2d->Position.X = pSprite->Position.GetX();
                    pVertex2d->Position.Y = pSprite->Position.GetY()+pSprite->WH.Y;
                    pVertex2d->Position.Z = 0.5f;
                    pVertex2d->RHW        = 1.0f;
                    pVertex2d->Color      = Color;
                    pVertex2d->UV         = vector2(U0,V1);
                    pVertex2d++;
                    pVertex2d->Position.X = pSprite->Position.GetX()+pSprite->WH.X;
                    pVertex2d->Position.Y = pSprite->Position.GetY()+pSprite->WH.Y;
                    pVertex2d->Position.Z = 0.5f;
                    pVertex2d->RHW        = 1.0f;
                    pVertex2d->Color      = Color;
                    pVertex2d->UV         = vector2(U1,V1);
                    pVertex2d++;
                    pVertex2d->Position.X = pSprite->Position.GetX()+pSprite->WH.X;
                    pVertex2d->Position.Y = pSprite->Position.GetY();
                    pVertex2d->Position.Z = 0.5f;
                    pVertex2d->RHW        = 1.0f;
                    pVertex2d->Color      = Color;
                    pVertex2d->UV         = vector2(U1,V0);
                    pVertex2d++;
                }
                else
                {
                    // Get center point
                    vector3 Center = pSprite->Position;

                    // Contruct corner points of quad
                    pVertex2d->Position = Center - v0;
                    pVertex2d->RHW      = 1.0f;
                    pVertex2d->Color    = Color;
                    pVertex2d->UV       = vector2(U0,V0);
                    pVertex2d++;
                    pVertex2d->Position =  Center - v1;
                    pVertex2d->RHW      = 1.0f;
                    pVertex2d->Color    = Color;
                    pVertex2d->UV       = vector2(U0,V1);
                    pVertex2d++;
                    pVertex2d->Position = Center + v0;
                    pVertex2d->RHW      = 1.0f;
                    pVertex2d->Color    = Color;
                    pVertex2d->UV       = vector2(U1,V1);
                    pVertex2d++;
                    pVertex2d->Position = Center + v1;
                    pVertex2d->RHW      = 1.0f;
                    pVertex2d->Color    = Color;
                    pVertex2d->UV       = vector2(U1,V0);
                    pVertex2d++;
                }

                // Advance to next sprite
                pSprite++;
            }
            else
            {
                // Transform center point into view space
                vector3 Center;
                W2V.Transform( &Center, &pSprite->Position, 1 );

                // Contruct corner points of quad
                pVertex3d->Position = Center + v0;
                pVertex3d->Color    = Color;
                pVertex3d->UV       = vector2(U0,V0);
                pVertex3d++;
                pVertex3d->Position = Center + v1;
                pVertex3d->Color    = Color;
                pVertex3d->UV       = vector2(U0,V1);
                pVertex3d++;
                pVertex3d->Position = Center - v0;
                pVertex3d->Color    = Color;
                pVertex3d->UV       = vector2(U1,V1);
                pVertex3d++;
                pVertex3d->Position = Center - v1;
                pVertex3d->Color    = Color;
                pVertex3d->UV       = vector2(U1,V0);
                pVertex3d++;

                // Advance to next sprite
                pSprite++;
            }
        }

        // Set number of vertices
        m_iVertex = 4 * m_iSprite;

        // Call regular Dispatch function
        draw_Dispatch();
    }

    // Clear Sprite Buffer
    m_iSprite = 0;
}

///////////////////////////////////////////////////////////////////////////

void draw_Begin( draw_primitive Primitive, u32 Flags )
{
    ASSERT( m_Initialized );
    ASSERT( !m_bBegin );

    // Confirm we are in a render context
    ASSERT( eng_InBeginEnd() );

    // Save primitive and flags
    m_Primitive  = Primitive;
    m_Flags      = Flags;
    m_Is2D       = Flags & (DRAW_2D | DRAW_2D_KEEP_Z);
    m_IsTextured = Flags & DRAW_TEXTURED;

    // Set internal state from primitive type
    switch( m_Primitive )
    {
    case DRAW_POINTS:
        m_pfnDispatch = draw_Dispatch;
        m_iTrigger = TRIGGER_POINTS;
        break;
    case DRAW_LINES:
        m_pfnDispatch = draw_Dispatch;
        m_iTrigger = TRIGGER_LINES;
        break;
    case DRAW_LINE_STRIPS:
        m_pfnDispatch = draw_Dispatch;
        m_iTrigger = TRIGGER_LINE_STRIPS;
        break;
    case DRAW_TRIANGLES:
        m_pfnDispatch = draw_Dispatch;
        m_iTrigger = TRIGGER_TRIANGLES;
        break;
    case DRAW_TRIANGLE_STRIPS:
        m_pfnDispatch = draw_Dispatch;
        m_iTrigger = TRIGGER_TRIANGLE_STRIPS;
        break;
    case DRAW_QUADS:
        m_pfnDispatch = draw_Dispatch;
        m_iTrigger = TRIGGER_QUADS;
        break;
    case DRAW_RECTS:
        ASSERT( m_Is2D );
        m_pfnDispatch = draw_DispatchRects;
        m_iTrigger = TRIGGER_RECTS;
        break;
    case DRAW_SPRITES:
        m_pfnDispatch = draw_DispatchSprites;
        break;
    }

    // Clear list pointers
    m_pUVs    = NULL;
    m_pColors = NULL;
    m_pVerts  = NULL;

    // Set in begin state
    m_bBegin = TRUE;
    m_Color = XCOLOR_WHITE;
    m_UV.Zero();

    if( !g_pd3dDevice )
        return;

    // Set default texture stages
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE    );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE    );

    if( Flags & DRAW_TEXTURED )
	{
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE  );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE  );
	}
	else
	{
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1  );
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
	}

    // Set D3D render states for ALPHA
    if( Flags & DRAW_USE_ALPHA )
    {
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );

        if( m_IsTextured )
        {
            g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE   );
            g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE     );
            g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE     );
        }
        else
        {
            g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
            g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE     );
        }
    }
    else
    {
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    }
    
    // Set D3D render states for ZBUFFER
    g_pd3dDevice->SetRenderState( D3DRS_DEPTHBIAS, m_ZBias );
    if( Flags & DRAW_NO_ZBUFFER )
    {
        g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
    }
    else
    {
        g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    }

    // No Z write?
    if( Flags & DRAW_NO_ZWRITE )
    {
        g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_FALSE );
    }
    else
    {
        g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_TRUE );
    }

    // Add?
    if( Flags & DRAW_BLEND_ADD )
    {
        g_pd3dDevice->SetRenderState( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
    } 
    else if( Flags & DRAW_BLEND_SUB )
    {
        g_pd3dDevice->SetRenderState( D3DRS_BLENDOP  , D3DBLENDOP_REVSUBTRACT );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
    }
    
    // Alpha
    else if( Flags & DRAW_USE_ALPHA )
    {
        g_pd3dDevice->SetRenderState( D3DRS_BLENDOP  , D3DBLENDOP_ADD );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    }

    // Set D3D render states for CULLING
    if( Flags & DRAW_CULL_NONE )
    {
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    }
    else
    {
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
    }

    // Set D3D render states for UV tiling
    if( Flags & DRAW_U_CLAMP )
    {
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    }
    else
    {
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
    }

    // Set D3D render states for UV tiling
    if( Flags & DRAW_V_CLAMP )
    {
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    }
    else
    {
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
    }

    // Set D3D render states for WIREFRAME
    if( Flags & DRAW_WIRE_FRAME )
    {
        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
    }
    else
    {
        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
    }

    // Set Shader
    if( m_Is2D )
        g_pd3dDevice->SetFVF( D3DFVF_DRAWVERTEX_2D );
    else
        g_pd3dDevice->SetFVF( D3DFVF_DRAWVERTEX_3D );

    // Clear Texture if not in textured mode
    if( !m_IsTextured )
    {
        g_pd3dDevice->SetTexture( 0, 0 );
    }
}

///////////////////////////////////////////////////////////////////////////

void draw_End( void )
{
    ASSERT( m_bBegin );

    // Clear in begin state
    m_bBegin = FALSE;

    if( !g_pd3dDevice )
        return; 

    // Flush any drawing we have queued up
    m_pfnDispatch();

    // Set D3D render states to normal for ZBUFFER
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_DEPTHBIAS, 0 );

    const view* pView = eng_GetView();
    ASSERT( pView );
    g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&pView->GetW2V() );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&pView->GetV2C() );
}

///////////////////////////////////////////////////////////////////////////

void draw_ResetAfterException( void )
{
    m_bBegin = FALSE;

    // Exit if we lost the D3D device
    if( !g_pd3dDevice )
        return;

    // Clear device ZBuffer render mode
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_DEPTHBIAS, 0 );

    // Setup default view
    const view* pView = eng_GetView();
    ASSERT( pView );
    g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&pView->GetW2V() );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&pView->GetV2C() );
}

///////////////////////////////////////////////////////////////////////////

void draw_SetL2W( const matrix4& L2W )
{
    m_L2W = L2W;
}

///////////////////////////////////////////////////////////////////////////

void draw_ClearL2W( void )
{
    m_L2W.Identity();
}

///////////////////////////////////////////////////////////////////////////

void draw_SetTexture( const xbitmap& Bitmap )
{
    if( m_bBegin )
    {
        m_pfnDispatch();
    }

    // Activate the texture if Textured mode is set
    vram_Activate( Bitmap );

}

///////////////////////////////////////////////////////////////////////////

void draw_SetTexture( void )
{
    if( m_bBegin )
    {
        m_pfnDispatch();
    }

    vram_Activate();
}

///////////////////////////////////////////////////////////////////////////

void draw_UV( const vector2& UV )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pUVs = NULL;
    m_UV = UV;
}

void draw_UV( f32 U, f32 V )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pUVs = NULL;
    m_UV.X = U;
    m_UV.Y = V;
}

///////////////////////////////////////////////////////////////////////////

void draw_Color( const xcolor& Color )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pColors = NULL;
    m_Color = Color;
}

void draw_Color( f32 R, f32 G, f32 B, f32 A )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pColors = NULL;
    m_Color.R = (u8)(R*255.0f);
    m_Color.G = (u8)(G*255.0f);
    m_Color.B = (u8)(B*255.0f);
    m_Color.A = (u8)(A*255.0f);
}

///////////////////////////////////////////////////////////////////////////

void draw_Vertex( const vector3& Vertex )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    if( !g_pd3dDevice )
        return;

    if( m_Is2D )
    {
        // Setup vertex in buffer
        m_pActiveBuffer2d->UV       = m_UV;
        m_pActiveBuffer2d->Color    = m_Color;
        m_pActiveBuffer2d->Position = Vertex;
        m_pActiveBuffer2d->RHW      = 1.0f;

        // Advance buffer pointer
        m_pActiveBuffer2d++;
        m_iVertex++;

        // Check if it is time to dispatch this buffer
        if( m_iVertex == m_iTrigger )
            m_pfnDispatch();
    }
    else
    {
        // Setup vertex in buffer
        m_pActiveBuffer3d->UV       = m_UV;
        m_pActiveBuffer3d->Color    = m_Color;
        m_pActiveBuffer3d->Position = Vertex;

        // Advance buffer pointer
        m_pActiveBuffer3d++;
        m_iVertex++;

        // Check if it is time to dispatch this buffer
        if( m_iVertex == m_iTrigger )
            m_pfnDispatch();
    }
}

///////////////////////////////////////////////////////////////////////////

void draw_Vertex( f32 X, f32 Y, f32 Z )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    if( !g_pd3dDevice )
        return;

    if( m_Is2D )
    {
        // Setup vertex in buffer
        m_pActiveBuffer2d->UV    = m_UV;
        m_pActiveBuffer2d->Color = m_Color;
        m_pActiveBuffer2d->Position.Set( X, Y, Z );
        m_pActiveBuffer2d->RHW   = 1.0f;

        // Advance buffer pointer
        m_pActiveBuffer2d++;
        m_iVertex++;

        // Check if it is time to dispatch this buffer
        if( m_iVertex == m_iTrigger )
            m_pfnDispatch();
    }
    else
    {
        // Setup vertex in buffer
        m_pActiveBuffer3d->UV    = m_UV;
        m_pActiveBuffer3d->Color = m_Color;
        m_pActiveBuffer3d->Position.Set( X, Y, Z );

        // Advance buffer pointer
        m_pActiveBuffer3d++;
        m_iVertex++;

        // Check if it is time to dispatch this buffer
        if( m_iVertex == m_iTrigger )
            m_pfnDispatch();
    }
}

///////////////////////////////////////////////////////////////////////////

void draw_UVs( const vector2* pUVs, s32 Count, s32 Stride )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pUVs = pUVs;
    m_nUVs = Count;
    m_sUVs = Stride;
}

void draw_Colors( const xcolor*  pColors, s32 Count, s32 Stride )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pColors = pColors;
    m_nColors = Count;
    m_sColors = Stride;
}

void draw_Verts( const vector3* pVerts,  s32 Count, s32 Stride )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    m_pVerts = pVerts;
    m_nVerts = Count;
    m_sVerts = Stride;
}

///////////////////////////////////////////////////////////////////////////

void draw_Index( s32 Index )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    if( !g_pd3dDevice )
        return;

    if( Index == -1 )
    {
        m_pfnDispatch();
    }
    else
    {
        if( m_Is2D )
        {
            // Setup vertex in buffer
            if( m_pUVs )
            {
                ASSERT( Index < m_nUVs );
                m_pActiveBuffer2d->UV = m_pUVs[Index];
            }
            else
            {
                m_pActiveBuffer2d->UV = m_UV;
            }
            if( m_pColors )
            {
                ASSERT( Index < m_nColors );
                m_pActiveBuffer2d->Color = m_pColors[Index];
            }
            else
            {
                m_pActiveBuffer2d->Color = m_Color;
            }

            ASSERT( Index < m_nVerts );
            m_pActiveBuffer2d->Position = m_pVerts[Index];
            m_pActiveBuffer2d->RHW      = 1.0f;

            // Advance buffer pointer
            m_pActiveBuffer2d++;
            m_iVertex++;

            // Check if it is time to dispatch this buffer
            if( m_iVertex == m_iTrigger )
                m_pfnDispatch();
        }
        else
        {
            // Setup vertex in buffer
            if( m_pUVs )
            {
                ASSERT( Index < m_nUVs );
                m_pActiveBuffer3d->UV = m_pUVs[Index];
            }
            else
            {
                m_pActiveBuffer3d->UV = m_UV;
            }
            if( m_pColors )
            {
                ASSERT( Index < m_nColors );
                m_pActiveBuffer3d->Color = m_pColors[Index];
            }
            else
            {
                m_pActiveBuffer3d->Color = m_Color;
            }

            ASSERT( Index < m_nVerts );
            m_pActiveBuffer3d->Position = m_pVerts[Index];

            // Advance buffer pointer
            m_pActiveBuffer3d++;
            m_iVertex++;

            // Check if it is time to dispatch this buffer
            if( m_iVertex == m_iTrigger )
                m_pfnDispatch();
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void draw_Execute( const s16* pIndices, s32 NIndices )
{
    s32     i;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive != DRAW_SPRITES );

    if( !g_pd3dDevice )
        return;

    // Loop through indices supplied
    for( i=0 ; i<NIndices ; i++ )
    {
        // Read Index
        s32 Index = pIndices[i];

        // Kick on -1, otherwise add to buffer
        if( Index == -1 )
            m_pfnDispatch();
        else
            draw_Index( pIndices[i] );
    }
}

///////////////////////////////////////////////////////////////////////////

void    draw_Sprite     ( const vector3& Position,  // Hot spot (2D Left-Top), (3D Center)
                          const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                          const xcolor&  Color )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_SPRITES );

    if( !g_pd3dDevice )
        return;

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
        m_pfnDispatch();
}

///////////////////////////////////////////////////////////////////////////

void    draw_SpriteUV   ( const vector3& Position,  // Hot spot (2D Left-Top), (3D Center)
                          const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                          const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                          const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                          const xcolor&  Color )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_SPRITES );

    if( !g_pd3dDevice )
        return;

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
        m_pfnDispatch();
}

void    draw_SpriteUV   ( const vector3& Position,  // Hot spot (3D Center)
                          const vector2& WH,        // (3D World W&H)
                          const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                          const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                          const xcolor&  Color,     //
                                radian   Rotate )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_SPRITES );

    if( !g_pd3dDevice )
        return;

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
        m_pfnDispatch();
}


//==========================================================================

void    draw_OrientedQuad(const vector3& Pos0,
                          const vector3& Pos1,
                          const vector2& UV0,
                          const vector2& UV1,
                          const xcolor&  Color0,
                          const xcolor&  Color1,
                                f32      Radius )
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_TRIANGLES );

    if( !g_pd3dDevice )
        return;

    vector3 Dir = Pos1 - Pos0;
    if( !Dir.SafeNormalize() )
        return;

    vector3 CrossDir = Dir.Cross( eng_GetView()->GetPosition() - Pos0 );
    if( !CrossDir.SafeNormalize() )
        return;

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

void    draw_OrientedQuad(const vector3& Pos0,
                          const vector3& Pos1,
                          const vector2& UV0,
                          const vector2& UV1,
                          const xcolor&  Color0,
                          const xcolor&  Color1,
                                f32      Radius0,
                                f32      Radius1)
{
    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_TRIANGLES );

    if( !g_pd3dDevice )
        return;

    vector3 Dir = Pos1 - Pos0;
    if( !Dir.SafeNormalize() )
        return;

    vector3 CrossDir = Dir.Cross( eng_GetView()->GetPosition() - Pos0 );
    if( !CrossDir.SafeNormalize() )
        return;

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

void    draw_OrientedStrand (const vector3* pPosData,
                                   s32      NumPts,
                             const vector2& UV0,
                             const vector2& UV1,
                             const xcolor&  Color0,
                             const xcolor&  Color1,
                                   f32      Radius )
{
    s32 i;
    vector3 quad[6];        //  storage for a quad, plus an extra edge for averaging
    vector2 uv0, uv1;

    if( !g_pd3dDevice )
        return;

    uv0 = UV0;
    uv1 = UV1;

    ASSERT( m_bBegin );
    ASSERT( m_Primitive == DRAW_TRIANGLES );

    for ( i = 0; i < NumPts - 1; i++ )
    {
        vector3 Dir = pPosData[i+1] - pPosData[i];
        if( !Dir.SafeNormalize() )
            Dir(0,1,0);

        vector3 CrossDir = Dir.Cross( eng_GetView()->GetPosition() - pPosData[i] );
        if( !CrossDir.SafeNormalize() )
            CrossDir(1,0,0);

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
            vector3 tq1, tq2;

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

void draw_SetZBuffer( const irect& Rect, f32 Z )
{
    // Make sure Z is valid
    ASSERT(Z >= 0.0f) ;
    ASSERT(Z <= 1.0f) ;

    // Begin
    draw_Begin( DRAW_QUADS, DRAW_2D ) ;

    // Always write to z buffer
    g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE,      D3DZB_TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_ZFUNC,        D3DCMP_ALWAYS);

    // Trick card into not changing the frame buffer
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ; 
    g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ZERO);
    g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

    // Do not need these features
    g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE) ;

    // Draw rect
    draw_Color( XCOLOR_WHITE );
    draw_Vertex( (f32)Rect.l, (f32)Rect.t, Z );
    draw_Vertex( (f32)Rect.l, (f32)Rect.b, Z );
    draw_Vertex( (f32)Rect.r, (f32)Rect.b, Z );
    draw_Vertex( (f32)Rect.r, (f32)Rect.t, Z );
    draw_End() ;

    // Restore settings
    g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE) ; 
}

//==============================================================================

void draw_ClearZBuffer( const irect& Rect )
{
    draw_SetZBuffer(Rect, 1.0f) ;
}

//==============================================================================

void draw_FillZBuffer( const irect& Rect )
{
    draw_SetZBuffer(Rect, 0.0f) ;
}

//==============================================================================

void draw_DisableBilinear( void )
{
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
}

//==============================================================================

void draw_EnableBilinear( void )
{
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
}

//==============================================================================
