
#include "Entropy.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"
#include "RawMesh.hpp"
#include "RenderMesh.hpp"

//==============================================================================
// STRUCTURES
//==============================================================================

struct simple_vert
{
    vector3p P;
    DWORD    C;
    vector2  BaseUV;
};

struct mesh_data
{
    rawmesh*        pMesh;
    simple_vert*    pVertData;
};


//==============================================================================
// DATA
//==============================================================================

static xharray<mesh_data>   s_RegisteredMeshes;

//==============================================================================
// FUNCTIONS
//==============================================================================

xhandle InitializeMeshData( rawmesh& Mesh )
{
    xhandle    MeshHandle;
    mesh_data& MeshData = s_RegisteredMeshes.Add( MeshHandle );

    // fill in the mesh data
    s32 VertIndex      = 0;
    MeshData.pMesh     = &Mesh;
    MeshData.pVertData = new simple_vert[Mesh.m_nFacets*3];
    for ( s32 i = 0; i < Mesh.m_nFacets; i++ )
    {
        for ( s32 j = 0; j < 3; j++ )
        {
            simple_vert& V = MeshData.pVertData[VertIndex++];

            V.P        = Mesh.m_pVertex[Mesh.m_pFacet[i].iVertex[j]].Position;
            //V.C        = 0xFF808080;
            V.C        = 0;
            V.BaseUV.X = Mesh.m_pVertex[Mesh.m_pFacet[i].iVertex[j]].UV[0].X;
            V.BaseUV.Y = Mesh.m_pVertex[Mesh.m_pFacet[i].iVertex[j]].UV[0].Y;
        }
    }

    return MeshHandle;
};

//==============================================================================

void RenderMeshToShadowMap( xhandle Handle, xbitmap& BMP, matrix4& L2W, u32 Color )
{
    mesh_data& MeshData = s_RegisteredMeshes( Handle );

    // set the transform matrix
    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W );

    // enable alpha blending
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,      TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,              D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,             D3DBLEND_ZERO );
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,              TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT,               Color );

    // set the diffuse texture
    vram_Activate(BMP);
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    g_pd3dDevice->SetTexture          ( 0, vram_GetSurface( BMP ) );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );   //####
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    // set the vertex shader
    g_pd3dDevice->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, MeshData.pMesh->m_nFacets, MeshData.pVertData, sizeof(simple_vert) );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,              FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT,               0xffffffff );
}

//==============================================================================

void RenderMeshWithShadow( xhandle Handle, xbitmap& BMP, IDirect3DTexture8* pShadTexture, matrix4& L2W, matrix4& ShadowProj, xbool UseTexW, xbool Project )
{
    mesh_data& MeshData = s_RegisteredMeshes( Handle );

    // set the transform matrix
    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W );

    // enable alpha blending
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,         FALSE );

    // set the diffuse state
    s32 nStages = 0;
    vram_Activate(BMP);
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_TEXCOORDINDEX, 0 );
    g_pd3dDevice->SetTexture          ( nStages, vram_GetSurface( BMP ) );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    nStages++;

    // set the shadow state
    DWORD TexTransFlags = 0;
    if ( UseTexW )  TexTransFlags |= D3DTTFF_COUNT4;
    else            TexTransFlags |= D3DTTFF_COUNT3;
    if ( Project )  TexTransFlags |= D3DTTFF_PROJECTED;
    g_pd3dDevice->SetTransform( D3DTS_TEXTURE1, (D3DMATRIX*)&ShadowProj );
    g_pd3dDevice->SetTexture( nStages, pShadTexture );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_TEXCOORDINDEX,         D3DTSS_TCI_CAMERASPACEPOSITION );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_TEXTURETRANSFORMFLAGS, TexTransFlags );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_ADDRESSU,              D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_ADDRESSV,              D3DTADDRESS_CLAMP );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    //g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_COLORARG2, D3DTA_CURRENT );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
    nStages++;

    // no more states
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    // set the vertex shader
    g_pd3dDevice->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, MeshData.pMesh->m_nFacets, MeshData.pVertData, sizeof(simple_vert) );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,              FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT,               0xffffffff );
}

//==============================================================================

void RenderMesh( xhandle Handle, xbitmap& BMP, matrix4& L2W, u32 Color )
{
    mesh_data& MeshData = s_RegisteredMeshes( Handle );

    // set the transform matrix
    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W );

    // enable alpha blending
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,         TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT,          Color );

    // set the diffuse texture
    vram_Activate(BMP);
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    g_pd3dDevice->SetTexture          ( 0, vram_GetSurface( BMP ) );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    // set the vertex shader
    g_pd3dDevice->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, MeshData.pMesh->m_nFacets, MeshData.pVertData, sizeof(simple_vert) );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,              FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT,               0xffffffff );
}

//==============================================================================

void KillMeshData( xhandle Handle )
{
    mesh_data& MeshData = s_RegisteredMeshes(Handle);
    delete []MeshData.pVertData;

    s_RegisteredMeshes.DeleteByHandle(Handle);
}
