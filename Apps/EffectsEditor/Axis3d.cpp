#include "StdAfx.h"
#include "Axis3d.hpp"

//=========================================================================
// DEFINES
//=========================================================================
#define ARROW_VERTEX_COUNT ( 50 )
#define ARROW_TRIANGLE_COUNT ( 50 )

//=========================================================================
// VARIABLES
//=========================================================================
static vector3 s_Arrow[3][ ARROW_VERTEX_COUNT + 3 ];
static xbool   s_InititalizeArrows = TRUE;

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

axis3d::axis3d( void )
{
    L2W.Identity();

    //
    // Generate the axis for drawing
    //
    if( s_InititalizeArrows )
    {
        s32      i,j;
        radian   r;
        f32      R,L,RL;

        s_InititalizeArrows = FALSE;
        R  = 10/2;   // Radious of the arrow
        L  = 100/2;   // Length of the line
        RL = 30/2;    // Length of the arrow     
        for( i=0; i<3; i++ )
        {
            s_Arrow[i][0].GetX() = 0;
            s_Arrow[i][0].GetY() = 0;
            s_Arrow[i][0].GetZ() = L;

            s_Arrow[i][1].GetX() = 0;
            s_Arrow[i][1].GetY() = 0;
            s_Arrow[i][1].GetZ() = L+RL;

            for( j=2, r=0; j<(ARROW_VERTEX_COUNT+3); r += R_1 *(360.0f/(ARROW_VERTEX_COUNT-1)), j++ )
            {
                ASSERT( j < (ARROW_VERTEX_COUNT+3) );
                s_Arrow[i][j].GetX() = x_sin( -r ) * R;
                s_Arrow[i][j].GetY() = x_cos( -r ) * R;
                s_Arrow[i][j].GetZ() = L;
            }

            // Close the arrow polygon
            s_Arrow[i][j-1] = s_Arrow[i][2];
        
            if( i == 0 ) 
                for( ; j>=0; j-- ) 
                {
                    s_Arrow[i][j].Rotate( radian3( 0,R_1*90, 0) );
                }

            if( i == 1 ) 
                for( ; j>=0; j-- ) 
                {
                    s_Arrow[i][j].Rotate( radian3(-R_1*90, 0, 0) );
                }
        }
    }
}

//=========================================================================

void axis3d::SetPosition( vector3& Pos )
{   
    L2W.SetTranslation( Pos );
}


//=========================================================================

void axis3d::Render( void )
{
    ASSERT( eng_InBeginEnd() );

    s32     i;
    vertex  LineBuff[ARROW_VERTEX_COUNT*2];

    g_pd3dDevice->SetTransform( D3DTS_WORLDMATRIX(0), (D3DMATRIX*)&L2W );

    //
    // Set somce basic render modes
    //
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,           FALSE   );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE   );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,           D3DBLEND_ONE  );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,          D3DBLEND_ZERO );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,           D3DCULL_NONE );

    g_pd3dDevice->SetFVF( D3DFVF_XYZ );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,      D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,    D3DTA_TFACTOR     );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,      D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1,    D3DTA_TFACTOR     );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,      D3DTOP_DISABLE    );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,      D3DTOP_DISABLE    );


    //
    // Render the axis line
    //
    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR,      xcolor(255,0,0,255) );
    LineBuff[0].Pos.Set( 0, 0, 0 );
    LineBuff[1].Pos.Set( s_Arrow[0][0].GetX(), s_Arrow[0][0].GetY(), s_Arrow[0][0].GetZ() );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, LineBuff, sizeof(vertex) );


    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR,      xcolor(0,255,0,255) );
    LineBuff[0].Pos.Set( 0, 0, 0 );
    LineBuff[1].Pos.Set( s_Arrow[1][0].GetX(), s_Arrow[1][0].GetY(), s_Arrow[1][0].GetZ() );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, LineBuff, sizeof(vertex) );


    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR,      xcolor(0,0,255,255) );
    LineBuff[0].Pos.Set( 0, 0, 0 );
    LineBuff[1].Pos.Set( s_Arrow[2][0].GetX(), s_Arrow[2][0].GetY(), s_Arrow[2][0].GetZ() );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, LineBuff, sizeof(vertex) );

    //
    // Render the axis arrows
    //
    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, xcolor(255,0,0,255) );
    for( i=1; i<(ARROW_VERTEX_COUNT+2); i++ ) 
		LineBuff[i-1].Pos.Set( s_Arrow[0][i].GetX(), s_Arrow[0][i].GetY(), s_Arrow[0][i].GetZ() );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, ARROW_TRIANGLE_COUNT-1, LineBuff, sizeof(vertex) );


    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, xcolor(0,255,0,255) );
    for( i=1; i<(ARROW_VERTEX_COUNT+2); i++ ) 
		LineBuff[i-1].Pos.Set( s_Arrow[1][i].GetX(), s_Arrow[1][i].GetY(), s_Arrow[1][i].GetZ() );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, ARROW_TRIANGLE_COUNT-1, LineBuff, sizeof(vertex) );


    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, xcolor(0,0,255,255) );
    for( i=1; i<(ARROW_VERTEX_COUNT+2); i++ ) 
		LineBuff[i-1].Pos.Set( s_Arrow[2][i].GetX(), s_Arrow[2][i].GetY(), s_Arrow[2][i].GetZ() );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, ARROW_TRIANGLE_COUNT-1, LineBuff, sizeof(vertex) );
    

    // Clean up
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
}