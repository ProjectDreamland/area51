#include "stdafx.h"
#include "BitmapViewer.hpp"

//=========================================================================
// FUNCTION
//=========================================================================

//=========================================================================

void bitmap_viewer::Clear( void )
{
    m_Bitmap.Clear();
}

//=========================================================================

void bitmap_viewer::AppendPicture( const char* pFileName )
{
    /*
    bitmap_info& Info.Bitmap = m_Bitmap.Appened();

    x_strcpy( Info.FileName, pFileName );
    auxbmp_LoadD3Dy( Info.Bitmap, Info.FileName );
    */
}

//=========================================================================

void bitmap_viewer::Render( void )
{
    /*
    const view& View = *eng_GetActiveView(0);

    //
    // Compute the view port for the bitmap
    //

    // Get the view port ready to render the bitmap
    rect Rect;
    View.GetViewport( Rect );

    //
    // Draw the bitmap in the window
    //
    draw_Begin( DRAW_SPRITES, DRAW_2D );

    g_pd3dDevice->SetTexture          ( 0, s_pTempTexture );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

    draw_Sprite     ( vector3(0,0,0),
                      vector2(100,100),
                      xcolor(0xffffffff) );

    draw_End();
    */
}
