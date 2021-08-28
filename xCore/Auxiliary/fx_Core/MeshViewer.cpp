
#include "MeshViewer.hpp"
#include "Entropy/e_VRAM.hpp"
#include "Entropy/e_ScratchMem.hpp"
#include "Auxiliary/Bitmap/aux_Bitmap.hpp"

namespace fx_core
{

//=========================================================================
// FUNCTIONS
//=========================================================================

void mesh_viewer::CleanUp( void )
{
    for( s32 i=0; i<m_Mesh.m_nTextures; i++ )
    {
        if( m_Bitmap[i].GetVRAMID() != 0 )
        {
            vram_Unregister( m_Bitmap[i] );
            m_Bitmap[i].Kill();
        }
    }

    m_Mesh.~rawmesh();
    m_Anim.~rawanim();

    m_Mesh.rawmesh::rawmesh();
    m_Anim.rawanim::rawanim();


    m_AnimFrameRate = 30;
}

//=========================================================================

mesh_viewer::mesh_viewer()
{
    //set bbox so view doesn't corrupt
    m_BBox.Set(vector3(0,0,0),10);
    m_bBackFacets = FALSE;
}

//=========================================================================

mesh_viewer::mesh_viewer( const mesh_viewer& mViewer )
{
    //set bbox so view doesn't corrupt
    m_BBox.Set(vector3(0,0,0),10);
    m_bBackFacets = FALSE;
}

//=========================================================================

mesh_viewer::~mesh_viewer( void )
{
    for( s32 i=0; i<m_Mesh.m_nTextures; i++ )
    {
        if( m_Bitmap[i].GetVRAMID() != 0 )
        {
            vram_Unregister( m_Bitmap[i] );
            m_Bitmap[i].Kill();
        }
    }
}

//=========================================================================

void mesh_viewer::Load( const char* pFileName )
{
    CleanUp();

    m_Mesh.Load( pFileName );
    m_Anim.Load( pFileName );

    m_L2W.Identity();
    m_Frame         = 0;
    m_bPlayAnim     = FALSE;

    m_BBox = m_Mesh.GetBBox();
    m_LightDir.Set( -0.5f, 1, -0.5f );
    m_LightDir.Normalize();

    m_Mesh.SortFacetsByMaterialAndBone();
    for( s32 i=0; i<m_Mesh.m_nTextures; i++ )
    {
        if( auxbmp_LoadD3D( m_Bitmap[i], m_Mesh.m_pTexture[i].FileName ) )
        {
        }        
        vram_Register( m_Bitmap[i] );
    }

    m_Ambient.Set( 0.5f, 0.5f, 0.5f );
}

//=========================================================================

void mesh_viewer::Unload( void )
{
    CleanUp();
}

//=========================================================================

void mesh_viewer::Render( xcolor TintColor )
{
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,      D3DTOP_MODULATE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,    D3DTA_DIFFUSE   );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,    D3DTA_TEXTURE   );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,      D3DTOP_MODULATE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1,    D3DTA_DIFFUSE   );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2,    D3DTA_TEXTURE   );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,      D3DTOP_DISABLE  );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,      D3DTOP_DISABLE  );

    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE                 ); //FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA    );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );
    g_pd3dDevice->SetRenderState( D3DRS_BLENDOP,          D3DBLENDOP_ADD       );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,     FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_COLORVERTEX,  TRUE  );

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      D3DZB_TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_TRUE ); //D3DZB_FALSE );

    if( m_bBackFacets )
    {
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    }
    else
    {
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
    }

    if( m_Mesh.m_nBones == 1 )
    {
        RenderSolid( TintColor );
    }
    else
    {
        RenderSoftSkin();
    }
}

//=========================================================================

void mesh_viewer::SetBackFacets( xbool bFaceFacets )
{
    m_bBackFacets = bFaceFacets;
}

//=========================================================================

void mesh_viewer::RenderSolid( xcolor TintColor )
{
    g_pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

    struct lovert
    {
        vector3 P;
        xcolor  C;
        vector2 UV;
    };

    lovert*             pVertexBuffer   = NULL;
    rawmesh::vertex*    pVertex         = NULL;

    s32     i, j;

    s32     iLastTex    = -1;
    s32     iMaterial;
    s32     iVertex;
    s32     iTexture;
    s32     nFacets;
    s32     iVB;
    s32     iFace;

    vector3 N;
    vector3 L = m_LightDir;
    f32     I;

    for( iMaterial = 0; iMaterial < m_Mesh.m_nMaterials; iMaterial++ )
    {
        // Activate the diffuse texture for this material
        iTexture    = m_Mesh.m_pMaterial[ iMaterial ].TexMaterial[0].iTexture;

        //if( m_Bitmap[iMaterial].GetVRAMID() )       { vram_Activate( m_Bitmap[iMaterial] ); }
        if( m_Bitmap[iTexture].GetVRAMID() )        { vram_Activate( m_Bitmap[iTexture] ); }
        else                                        { vram_Activate(); }

        // Figure out how many facets use this material
        nFacets = 0;

        for( i = 0; i < m_Mesh.m_nFacets; i++ )
        {
            if( m_Mesh.m_pFacet[i].iMaterial == iMaterial )
            {
                nFacets++;
            }
        }

        // Allocate the vertex buffer for the facets that use this material
        pVertexBuffer   = new lovert[ nFacets * 3 ];
        iFace           = -1;

        // Gather the data for the vertex buffer
        for( i = 0; i < m_Mesh.m_nFacets; i++ )
        {
            if( m_Mesh.m_pFacet[i].iMaterial == iMaterial )
            {
                iFace++;

                for( j = 0; j < 3; j++ )
                {
                    iVertex                     = m_Mesh.m_pFacet[i].iVertex[j];
                    pVertex                     = &( m_Mesh.m_pVertex[ iVertex ] );
                    iVB                         = j + (iFace * 3); // j + (i * 3);

                    pVertexBuffer[iVB].UV.X     = pVertex->UV[0].X; 
                    pVertexBuffer[iVB].UV.Y     = pVertex->UV[0].Y;  
                    pVertexBuffer[iVB].P        = pVertex->Position;

                    N                           = pVertex->Normal[0];
                    I                           = fMax( 0, L.Dot( N ) );

                    ASSERT( I >= 0 );

                    pVertexBuffer[iVB].C.SetfRGBA( ( m_Ambient.GetX() + (TintColor.R / 255.0f * I) ),
                                                   ( m_Ambient.GetY() + (TintColor.G / 255.0f * I) ),
                                                   ( m_Ambient.GetZ() + (TintColor.B / 255.0f * I) ),
                                                   (f32)TintColor.A / 255.0f );
                }
            }
        }

        // Render the vertex buffer for this material
        g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, nFacets, pVertexBuffer, sizeof(lovert) );

        // Cleanup the vertex buffer data
        if( pVertexBuffer )
        {
            delete[] pVertexBuffer;
            pVertexBuffer = NULL;
        }
    }
}

//=========================================================================

void mesh_viewer::RenderSoftSkin( void )
{
    struct lovert
    {
        vector3 P;
        xcolor  C;
        vector2 UV;
    };

    s32 i;
    vector3 LightDir(1,0,0);

    // Check whether we have something to do
    if( m_Anim.m_nBones == 0 )
        return;

    g_pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

    // Allocate all the matrices
    smem_StackPushMarker();
    matrix4* pMatrix = (matrix4*)smem_StackAlloc( m_Anim.m_nBones * sizeof(matrix4) );
    if( pMatrix == NULL )
        return;

    // Compute the frame
    if( m_bPlayAnim )
    {
        f32 Time = m_Timer.TripSec();
        m_Frame += Time*m_AnimFrameRate;
    }
    m_Anim.ComputeBonesL2W( pMatrix, m_Frame );

    // Compute final matrices
    for( i=0; i<m_Anim.m_nBones; i++ )
    {
        pMatrix[i] = m_L2W * pMatrix[i];
    }

    // Render triangles
    s32 iLastTex = -1;
    for( i=0; i<m_Mesh.m_nFacets; i++ )
    {
        lovert      V[3];
        vector3     N[3];
        const rawmesh::facet& Facet = m_Mesh.m_pFacet[i];
        f32         MaxWeight = 0;
        s32         iBone;

        // This needs to update
        if( m_Mesh.m_pMaterial[ Facet.iMaterial ].TexMaterial[0].iTexture != iLastTex )
        {
            iLastTex = m_Mesh.m_pMaterial[ Facet.iMaterial ].TexMaterial[0].iTexture;
            if( m_Bitmap[iLastTex].GetVRAMID() ) vram_Activate( m_Bitmap[iLastTex] );
            else vram_Activate();
        }

        for( s32 j=0; j<3; j++ )
        {
            const rawmesh::vertex& Vert = m_Mesh.m_pVertex[ Facet.iVertex[j] ];
            V[j].P.Zero();
            N[j].Zero();
            
            for( s32 w=0; w<Vert.nWeights; w++ )
            {
                const rawmesh::weight& W = Vert.Weight[w];

                if( W.Weight > MaxWeight )
                {
                    MaxWeight = W.Weight;
                    iBone     = W.iBone;
                }

                V[j].P += (pMatrix[ W.iBone ] * Vert.Position) * W.Weight;
                N[j]   += pMatrix[ W.iBone ].RotateVector( Vert.Normal[0] ) * W.Weight;
            }

            N[j].Normalize();
            V[j].UV.X = Vert.UV[0].X; 
            V[j].UV.Y = Vert.UV[0].Y;  
            f32 I     = fMax( 0, LightDir.Dot( N[j] ) );
                I     = fMin( 1, I );

            //ASSERT( I >= 0 );

            V[j].C.SetfRGBA( fMin( 1, m_Ambient.GetX() + I), 
                             fMin( 1, m_Ambient.GetY() + I), 
                             fMin( 1, m_Ambient.GetZ() + I), 1 );

        }

        // Render the triangle
        g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, V, sizeof(lovert) );
    }

    // Free alloced memory
    smem_StackPopToMarker();
}

//=========================================================================

void mesh_viewer::PlayAnimation( void )
{
    // Nothing to do
    if( m_Mesh.m_nBones == 0 )
        return;

    if( m_Anim.m_nFrames == 0 )
        x_throw( "Mesh doesn't have animation" );


    m_bPlayAnim = TRUE;
    m_Timer.Start();
}

//=========================================================================

void mesh_viewer::PauseAnimation  ( void )
{
    m_bPlayAnim = FALSE;
    m_Timer.Stop();
}

} // namespace fx_core
