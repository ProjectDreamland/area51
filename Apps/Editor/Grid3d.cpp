
#include "BaseStdAfx.h"
#include "Grid3d.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================


//=========================================================================
grid3d::grid3d( void )
{
    m_Color.Set( 0, 0, 128, 128);
    m_SeparationX = 100;        // One meter separation
    m_SeparationZ = 100;
    m_SizeX       = 10000;
    m_SizeZ       = 10000;
    m_OffSetX     = 0;
    m_OffSetZ     = 0;
    m_OffSetY     = 0;
}

//=========================================================================

void grid3d::SetColor( xcolor GridColor )
{
    m_Color = GridColor;
}

//=========================================================================

void grid3d::SetTranslations ( vector3& Pos )
{
    m_OffSetX = Pos.GetX();
    m_OffSetY = Pos.GetY();
    m_OffSetZ = Pos.GetZ();
}

//=========================================================================

void grid3d::SetSeparation( f32 X, f32 Z )
{
    ASSERT( X >= 0 );
    ASSERT( Z >= 0 );
    m_SeparationX = X;
    m_SeparationZ = Z;
}

//=========================================================================

void grid3d::SetSize( f32 X, f32 Z )
{
    ASSERT( X >= 0 );
    ASSERT( Z >= 0 );
    m_SizeX = X;
    m_SizeZ = Z;
}

//=========================================================================

void grid3d::Render( void )
{
    ASSERT( eng_InBeginEnd() );

    if( !g_pd3dDevice )
        return;

    static vertex   Buffer[1024];
    f32             x, z;
    matrix4         L2W;
    s32             nVertex=0;

    //
    // Set the world matrix
    //
    L2W.Identity();
    L2W.SetTranslation( vector3( m_OffSetX, m_OffSetY, m_OffSetZ ) ); 

    g_pd3dDevice->SetTransform( D3DTS_WORLDMATRIX(0), (D3DMATRIX*)&L2W );

    //
    // Set somce basic render modes
    //
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,           FALSE   );
    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR,      m_Color );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,           D3DBLEND_SRCALPHA );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,          D3DBLEND_INVSRCALPHA );

    g_pd3dDevice->SetFVF( D3DFVF_XYZ );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,      D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,    D3DTA_TFACTOR     );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,      D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1,    D3DTA_TFACTOR     );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,      D3DTOP_DISABLE    );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,      D3DTOP_DISABLE    );

    //
    // Okay start collecting lines to be draw
    //
    for( x = -m_SizeX; x <= m_SizeX; x += m_SeparationX )
    {
        Buffer[nVertex++].Pos.Set( x, 0, -m_SizeZ );
        Buffer[nVertex++].Pos.Set( x, 0,  m_SizeZ );
    }

    for( z = -m_SizeZ; z <= m_SizeZ; z += m_SeparationZ )
    {
        Buffer[nVertex++].Pos.Set( -m_SizeX, 0, z );
        Buffer[nVertex++].Pos.Set(  m_SizeX, 0, z );
    }

    for( x = -m_SizeX; x <= m_SizeX; x += m_SeparationX*5 )
    {
        Buffer[nVertex++].Pos.Set( x, 0, -m_SizeZ );
        Buffer[nVertex++].Pos.Set( x, 0,  m_SizeZ );
    }

    for( z = -m_SizeZ; z <= m_SizeZ; z += m_SeparationZ*5 )
    {
        Buffer[nVertex++].Pos.Set( -m_SizeX, 0, z );
        Buffer[nVertex++].Pos.Set(  m_SizeX, 0, z );
    }

    //
    // Okay now draw the grid
    //
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, nVertex/2, Buffer, sizeof(vertex) );

    //
    // Okay now draw the main axis
    //
    xcolor C( iMin( m_Color.R+50, 255 ), 
              iMin( m_Color.G+50, 255 ),
              iMin( m_Color.B+50, 255 ),
              255 );

    nVertex = 0;

    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, C );

    Buffer[nVertex++].Pos.Set( 0, 0, -m_SizeZ );
    Buffer[nVertex++].Pos.Set( 0, 0,  m_SizeZ );

    Buffer[nVertex++].Pos.Set( -m_SizeX, 0, 0 );
    Buffer[nVertex++].Pos.Set(  m_SizeX, 0, 0 );

/*
    // lavel the axis
    glRasterPos3f (  m_SizeX,  0,   0      ); glCallLists (1, GL_UNSIGNED_BYTE, "X");
    glRasterPos3f ( -m_SizeX,  0,   0      ); glCallLists (1, GL_UNSIGNED_BYTE, "x");
    glRasterPos3f (  0,        0,   m_SizeZ); glCallLists (1, GL_UNSIGNED_BYTE, "Z");
    glRasterPos3f (  0,        0,  -m_SizeZ); glCallLists (1, GL_UNSIGNED_BYTE, "z");

    glPopMatrix();  
*/

}
