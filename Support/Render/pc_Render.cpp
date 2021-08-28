//=============================================================================
//
//  PC Specific Render Routines
//
//=============================================================================

#include "Render\VertexMgr.hpp"
#include "Shaders\SkinShader.h"
#include "Render\SoftVertexMgr.hpp"
#include "Render\platform_Render.hpp"
#include "Render\LightMgr.hpp"

//=============================================================================
//=============================================================================
// Types and constants specific to the pc-implementation
//=============================================================================
//=============================================================================

#define D3DTA_NOTUSED   D3DTA_CURRENT

static const s32 kEnvTextureW = 256;
static const s32 kEnvTextureH = 256;

//=============================================================================
//=============================================================================
// Static data specific to the pc-implementation
//=============================================================================
//=============================================================================

static vertex_mgr           s_RigidVertMgr;
static soft_vertex_mgr      s_SkinVertMgr;
static const material*      s_pMaterial = NULL;
static s32                  s_NStages = 0;
static s32                  s_TextureAlphaFactor;       //#### HACK HACK

static rigid_geom*          s_pRigidGeom      = NULL;
static skin_geom*           s_pSkinGeom       = NULL;
static matrix4              s_CustomEnvMapMatrix;
static matrix4              s_EnvMapMatrix;
static IDirect3DTexture9*   s_pEnvMapTexture  = NULL; 
static IDirect3DSurface9*   s_pEnvMapZBuffer  = NULL;
static IDirect3DSurface9*   s_pEnvMapSurface  = NULL;
static s32                  s_DrawFlags       = 0;
static const xbitmap*       s_pDrawBitmap     = NULL;

static D3DTRANSFORMSTATETYPE s_TextureType[] =
{
    D3DTS_TEXTURE0, D3DTS_TEXTURE1, D3DTS_TEXTURE2, D3DTS_TEXTURE3,
    D3DTS_TEXTURE4, D3DTS_TEXTURE5, D3DTS_TEXTURE6, D3DTS_TEXTURE7,
};

enum pc_blend_mode
{
    PC_BLEND_NONE           = 0,
    PC_BLEND_NORMAL,
    PC_BLEND_ADDITIVE,
    PC_BLEND_SUBTRACTIVE
};

//=============================================================================
//=============================================================================
// Functions specific to  the pc implementation
//=============================================================================
//=============================================================================

static
void pc_ResetTextureStages( void )
{
    if( !g_pd3dDevice )
        return;

    matrix4 Identity;
    Identity.Identity();

    for( s32 i=0; i<8; i++ )
    {
        g_pd3dDevice->SetSamplerState( i, D3DSAMP_ADDRESSU,              D3DTADDRESS_WRAP     );
        g_pd3dDevice->SetSamplerState( i, D3DSAMP_ADDRESSV,              D3DTADDRESS_WRAP     );
        g_pd3dDevice->SetTextureStageState( i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE      );
        g_pd3dDevice->SetTextureStageState( i, D3DTSS_TEXCOORDINDEX,         D3DTSS_TCI_PASSTHRU  );
        g_pd3dDevice->SetTransform( s_TextureType[i], (D3DMATRIX*)&Identity );
    }
}

//=============================================================================

static
void pc_SetTexture( s32 Stage, const xbitmap* pBitmap )
{
    if( !g_pd3dDevice )
        return;

    if( pBitmap != NULL )
    {
        g_pd3dDevice->SetTexture( Stage, vram_GetSurface( *pBitmap ) );
    }
    else
    {
        g_pd3dDevice->SetTexture( Stage, NULL );
    }
}

//=============================================================================

static
void pc_SetTextureColorStage( s32 Stage, DWORD Arg1, D3DTEXTUREOP Op, DWORD Arg2 )
{
    if( !g_pd3dDevice )
        return;

    g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP,   Op   );
    g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG1, Arg1 ); 
    g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG2, Arg2 );
}

//=============================================================================

static
void pc_SetTextureAlphaStage( s32 Stage, DWORD Arg1, D3DTEXTUREOP Op, DWORD Arg2 )
{
    if( !g_pd3dDevice )
        return;

    g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAOP,   Op   );
    g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG1, Arg1 ); 
    g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG2, Arg2 );
}

//=============================================================================

static
void pc_SetAlphaBlending( pc_blend_mode BlendMode )
{
    if( !g_pd3dDevice )
        return;

    switch( BlendMode )
    {
    default:
        ASSERT( FALSE );
        break;

    case PC_BLEND_NONE:
        g_pd3dDevice->SetRenderState( D3DRS_BLENDOP,          D3DBLENDOP_ADD         );
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE                  );
        break;

    case PC_BLEND_NORMAL:
        g_pd3dDevice->SetRenderState( D3DRS_BLENDOP,          D3DBLENDOP_ADD         );
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE                   );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA      );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA   );
        break;

    case PC_BLEND_ADDITIVE:
        g_pd3dDevice->SetRenderState( D3DRS_BLENDOP,          D3DBLENDOP_ADD         );
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE                   );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA      );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ONE           );
        break;

    case PC_BLEND_SUBTRACTIVE:
        g_pd3dDevice->SetRenderState( D3DRS_BLENDOP,          D3DBLENDOP_REVSUBTRACT );
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE                   );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA      );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ONE           );
        break;
    }
}

//=============================================================================

static
void pc_SetZBufferWrite( xbool DoZBufferWrite )
{
    if( !g_pd3dDevice )
        return;

    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, DoZBufferWrite );
}

//=============================================================================

static
void pc_SetBackFaceCulling( xbool DoCulling )
{
    if( !g_pd3dDevice )
        return;

    if( DoCulling == TRUE )
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
    else
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
}

//=============================================================================

static
void pc_Wireframe( void )
{
    // Color tinting
    pc_SetTextureColorStage ( 0, D3DTA_TFACTOR, D3DTOP_SELECTARG1, D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 0, D3DTA_TFACTOR, D3DTOP_SELECTARG1, D3DTA_NOTUSED );
    
    // Disable remaining stages
    pc_SetTextureColorStage ( 1, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 1, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );

    s_NStages = 1;
    
    // Alpha Blending ON and Z-Buffer writes ON
    pc_SetAlphaBlending  ( PC_BLEND_NORMAL  );
    pc_SetZBufferWrite   ( TRUE  );
    pc_SetBackFaceCulling( FALSE );
}

//=============================================================================

static
void pc_SetWireframe( xbool DoWireframe )
{
    if( !g_pd3dDevice )
        return;

    if( DoWireframe == TRUE )
    {
        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
        pc_Wireframe();
    }
    else
    {
        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
        if( s_pMaterial )
            platform_ActivateMaterial( *s_pMaterial );
    }
}

//=============================================================================

static
void pc_Diff( const xbitmap* pDiffuse, const xbitmap* pDetail )
{
    (void)pDetail;
    
    // Diffuse + Vertex Coloring
    pc_SetTexture           ( 0, pDiffuse );
    pc_SetTextureColorStage ( 0, D3DTA_TEXTURE, D3DTOP_MODULATE,   D3DTA_DIFFUSE );
    pc_SetTextureAlphaStage ( 0, D3DTA_TEXTURE, D3DTOP_MODULATE,   D3DTA_DIFFUSE );

    // Color tinting and saturation
    pc_SetTextureColorStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_TFACTOR );
    pc_SetTextureAlphaStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_TFACTOR );

    // Done
    s_NStages = 2;

    // Alpha Blending OFF and Z-Buffer writes ON
    pc_SetAlphaBlending  ( PC_BLEND_NONE );
    pc_SetZBufferWrite   ( TRUE  );
    pc_SetBackFaceCulling( TRUE  );
}

//=============================================================================

static
void pc_Alpha( const xbitmap* pDiffuse, const xbitmap* pDetail, xbool ZWrite, pc_blend_mode BlendMode )
{
    (void)pDetail;
    
    // Diffuse + Vertex Coloring
    pc_SetTexture           ( 0, pDiffuse );
    pc_SetTextureColorStage ( 0, D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE );
    pc_SetTextureAlphaStage ( 0, D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE );

    // Color tinting and saturation
    pc_SetTextureColorStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_TFACTOR );
    pc_SetTextureAlphaStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_TFACTOR );

    // Done
    s_NStages = 2;

    // Alpha Blending ON and Z-Buffer writes OFF
    pc_SetAlphaBlending  ( BlendMode );
    pc_SetZBufferWrite   ( ZWrite );
    pc_SetBackFaceCulling( FALSE );
}

//=============================================================================

static
void pc_SetEnvironmentMatrix( s32 Stage, xbool UseCustomEnvMap )
{
    if( !g_pd3dDevice )
        return;

    if ( UseCustomEnvMap )
    {
        g_pd3dDevice->SetTransform( s_TextureType[ Stage ], (D3DXMATRIX*)&s_CustomEnvMapMatrix );
        g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
        g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 | D3DTTFF_PROJECTED );
    }
    else
    {
        g_pd3dDevice->SetTransform( s_TextureType[ Stage ], (D3DXMATRIX*)&s_EnvMapMatrix );
        g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
        g_pd3dDevice->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 );
    }
}

//=============================================================================

static
void pc_Diff_PerPixelEnv( const xbitmap* pDiffuse, const xbitmap* pEnvironment, const xbitmap* pDetail, xbool UseCustomEnvMap ) 
{
    (void)pDetail;

    // Diffuse
    pc_SetTexture           ( 0, pDiffuse );
    pc_SetTextureColorStage ( 0, D3DTA_TEXTURE, D3DTOP_SELECTARG1, D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 0, D3DTA_TEXTURE, D3DTOP_SELECTARG1, D3DTA_NOTUSED );

    // Environment Map
    if ( UseCustomEnvMap )
        g_pd3dDevice->SetTexture( 1, s_pEnvMapTexture );
    else
        pc_SetTexture       ( 1, pEnvironment );
    pc_SetTextureColorStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATEALPHA_ADDCOLOR, D3DTA_TEXTURE );
    // pc_SetTextureColorStage ( 1, D3DTA_CURRENT, D3DTOP_SELECTARG1,             D3DTA_NOTUSED ); // <<< TURN OFF ENVIRONMENT MAP!
    // pc_SetTextureColorStage ( 1, D3DTA_TEXTURE, D3DTOP_SELECTARG1,             D3DTA_NOTUSED ); // <<< TURN OFF ENVIRONMENT MAP!
    pc_SetTextureAlphaStage ( 1, D3DTA_TEXTURE, D3DTOP_SELECTARG1,             D3DTA_NOTUSED );
    pc_SetEnvironmentMatrix ( 1, UseCustomEnvMap );

    // Vertex coloring and saturation
    pc_SetTextureColorStage ( 2, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_DIFFUSE );
    //pc_SetTextureAlphaStage ( 2, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_DIFFUSE );
    pc_SetTextureAlphaStage ( 2, D3DTA_DIFFUSE, D3DTOP_ADD, D3DTA_DIFFUSE );

    // Color tinting
    // TODO: Do we actually use color tinting?
    // pc_SetTextureColorStage ( 3, D3DTA_CURRENT, D3DTOP_MODULATE,   D3DTA_TFACTOR );
    // pc_SetTextureAlphaStage ( 3, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    
    // Done
    s_NStages = 3;

    // Alpha Blending OFF and Z-Buffer writes ON
    pc_SetAlphaBlending  ( PC_BLEND_NONE );
    pc_SetZBufferWrite   ( TRUE  );
    pc_SetBackFaceCulling( TRUE  );
}

//=============================================================================

static
void pc_Diff_PerPixelIllum( const xbitmap* pDiffuse, const xbitmap* pDetail )
{
    (void)pDetail;

    // Diffuse + Vertex Coloring
    pc_SetTexture           ( 0, pDiffuse );
    pc_SetTextureColorStage ( 0, D3DTA_TEXTURE, D3DTOP_MODULATE,   D3DTA_DIFFUSE );
    pc_SetTextureAlphaStage ( 0, D3DTA_TEXTURE, D3DTOP_SELECTARG1, D3DTA_NOTUSED );

    // Per-Pixel Self-Illumination
    pc_SetTexture           ( 1, pDiffuse );
    pc_SetTextureColorStage ( 1, D3DTA_TEXTURE, D3DTOP_BLENDTEXTUREALPHA, D3DTA_CURRENT );
    pc_SetTextureAlphaStage ( 1, D3DTA_CURRENT, D3DTOP_SELECTARG1,        D3DTA_NOTUSED );

    // Color tinting and saturation
    pc_SetTextureColorStage ( 2, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_TFACTOR );
    pc_SetTextureAlphaStage ( 2, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_TFACTOR );

    // Disable remaining stages
    pc_SetTextureColorStage ( 3, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 3, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    s_NStages = 3;

    // Alpha Blending OFF and Z-Buffer writes ON
    pc_SetAlphaBlending  ( PC_BLEND_NONE );
    pc_SetZBufferWrite   ( TRUE  );
    pc_SetBackFaceCulling( TRUE  );
}

//=============================================================================

static
void pc_Alpha_PerPolyEnv( const xbitmap* pDiffuse, const xbitmap* pEnvironment, const xbitmap* pDetail, xbool UseCustomEnvMap, xbool ZWrite, pc_blend_mode BlendMode )
{
    (void)pDetail;

    if( !g_pd3dDevice )
        return;

    // Diffuse
    pc_SetTexture           ( 0, pDiffuse );
    pc_SetTextureColorStage ( 0, D3DTA_TEXTURE, D3DTOP_SELECTARG1, D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 0, D3DTA_TEXTURE, D3DTOP_SELECTARG1, D3DTA_NOTUSED );

    // Environment Map
    if ( UseCustomEnvMap )
        g_pd3dDevice->SetTexture( 1, s_pEnvMapTexture );
    else
        pc_SetTexture       ( 1, pEnvironment );
    pc_SetTextureColorStage ( 1, D3DTA_CURRENT, D3DTOP_BLENDFACTORALPHA, D3DTA_TEXTURE );
    pc_SetTextureAlphaStage ( 1, D3DTA_CURRENT, D3DTOP_SELECTARG1,       D3DTA_NOTUSED );
    pc_SetEnvironmentMatrix ( 1, UseCustomEnvMap );
    
    // Vertex coloring and saturation
    pc_SetTextureColorStage ( 2, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_DIFFUSE );
    pc_SetTextureAlphaStage ( 2, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_DIFFUSE );

    // Color tinting
    // TODO: Why tinting?
    // pc_SetTextureColorStage ( 3, D3DTA_CURRENT, D3DTOP_MODULATE,   D3DTA_TFACTOR );
    // pc_SetTextureAlphaStage ( 3, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );

    // Disable remaining stages
    pc_SetTextureColorStage ( 3, D3DTA_CURRENT, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 3, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    s_NStages = 3;

    // Alpha Blending ON and Z-Buffer writes OFF
    pc_SetAlphaBlending  ( BlendMode );
    pc_SetZBufferWrite   ( ZWrite    );
    pc_SetBackFaceCulling( FALSE     );
}

//=============================================================================

static
void pc_Alpha_PerPixelIllum( const xbitmap* pDiffuse, const xbitmap* pDetail, xbool ZWrite, pc_blend_mode BlendMode )
{
    (void)pDetail;

    // Diffuse
    pc_SetTexture           ( 0, pDiffuse );
    pc_SetTextureColorStage ( 0, D3DTA_TEXTURE, D3DTOP_SELECTARG1, D3DTA_CURRENT );
    pc_SetTextureAlphaStage ( 0, D3DTA_TEXTURE, D3DTOP_MODULATE,   D3DTA_DIFFUSE );

    // Color tinting and saturation
    pc_SetTextureColorStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATE2X,   D3DTA_TFACTOR );
    pc_SetTextureAlphaStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATE2X,   D3DTA_TFACTOR );

    // Disable remaining stages
    pc_SetTextureColorStage ( 2, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 2, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    s_NStages = 2;

    // Alpha Blending ON and Z-Buffer writes OFF
    pc_SetAlphaBlending  ( BlendMode );
    pc_SetZBufferWrite   ( ZWrite    );
    pc_SetBackFaceCulling( FALSE     );
}

//=============================================================================

static
void pc_Alpha_PerPolyIllum( const xbitmap* pDiffuse, const xbitmap* pDetail, xbool ZWrite, pc_blend_mode BlendMode )
{
    (void)pDetail;

    // Diffuse
    pc_SetTexture           ( 0, pDiffuse );
    pc_SetTextureColorStage ( 0, D3DTA_TEXTURE, D3DTOP_SELECTARG1, D3DTA_CURRENT );
    pc_SetTextureAlphaStage ( 0, D3DTA_TEXTURE, D3DTOP_MODULATE,   D3DTA_DIFFUSE );

    // Color tinting and saturation
    pc_SetTextureColorStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_TFACTOR );
    pc_SetTextureAlphaStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATE2X, D3DTA_TFACTOR );

    // Disable remaining stages
    pc_SetTextureColorStage ( 2, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 2, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    s_NStages = 2;

    // Alpha Blending ON and Z-Buffer writes OFF
    pc_SetAlphaBlending  ( BlendMode );
    pc_SetZBufferWrite   ( ZWrite    );
    pc_SetBackFaceCulling( FALSE     );
}

//=============================================================================

static
void pc_Distortion( void )
{
    // DS NOTE: We can't do a proper distortion material without overhauling
    // the pc rendering code a bit, so instead, we'll just throw away the
    // texture. That way we can at least differentiate between cloaked and not.
    g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

    // Diffuse + Vertex Coloring
    pc_SetTextureColorStage ( 0, D3DTA_DIFFUSE, D3DTOP_SELECTARG1, D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 0, D3DTA_DIFFUSE, D3DTOP_SELECTARG1, D3DTA_NOTUSED );

    // Color tinting
    //pc_SetTextureColorStage ( 1, D3DTA_CURRENT, D3DTOP_MODULATE,   D3DTA_TFACTOR );
    //pc_SetTextureAlphaStage ( 1, D3DTA_CURRENT, D3DTOP_SELECTARG1, D3DTA_NOTUSED );

    // Add the saturation
    pc_SetTextureColorStage ( 1, D3DTA_CURRENT, D3DTOP_ADD,        D3DTA_CURRENT );
    pc_SetTextureAlphaStage ( 1, D3DTA_CURRENT, D3DTOP_SELECTARG1, D3DTA_NOTUSED );

    // Done
    s_NStages = 2;

    // Alpha Blending OFF and Z-Buffer writes ON
    pc_SetAlphaBlending  ( PC_BLEND_NONE );
    pc_SetZBufferWrite   ( TRUE          );
    pc_SetBackFaceCulling( TRUE          );
}

//=============================================================================

static
void pc_Distortion_PerPolyEnv( void )
{
    // DS NOTE: We can't do a proper distortion material without overhauling
    // the pc rendering code a bit, so instead, we'll just throw away the
    // texture. That way we can at least differentiate between cloaked and not.
    pc_Distortion();
}

//=============================================================================

static
void pc_SetTextureFactor( xcolor Color )
{
    if( !g_pd3dDevice )
        return;

    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA( Color.R, Color.G, Color.B, Color.A ) );
}

//=============================================================================

void pc_SetZBias( s32 ZBias )
{
    if( !g_pd3dDevice )
        return;

// DX8 code    g_pd3dDevice->SetRenderState( D3DRS_ZBIAS, ZBias );

    g_pd3dDevice->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, 0 );
    g_pd3dDevice->SetRenderState( D3DRS_DEPTHBIAS, ZBias );
}

//=============================================================================

static
void pc_RenderRigid( xhandle hDList, const matrix4* pL2W )
{
    if( !g_pd3dDevice )
        return;

    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)pL2W );
    s_RigidVertMgr.DrawDList( hDList, D3DPT_TRIANGLELIST );
}

//=============================================================================

static
void pc_RenderSkin( xhandle hDList, const matrix4* pBone, d3d_skin_lighting* pLighting )
{
    s_SkinVertMgr.DrawDList( hDList, pBone, pLighting );
}

//=============================================================================

static
xhandle pc_GetRigidDList( render::hgeom_inst hInst, s32 iSubMesh )
{
    ASSERT( hInst.IsNonNull() );

    // get the approriate structures necessary for this
    private_instance& PrivateInst = s_lRegisteredInst(hInst);
    rigid_geom* pGeom             = (rigid_geom*)PrivateInst.pGeom;
    ASSERT( PrivateInst.Type == TYPE_RIGID );
    ASSERT( (iSubMesh>=0) && (iSubMesh<pGeom->m_nSubMeshes) );

    // return the dlist
    rigid_geom::submesh& SubMesh  = pGeom->m_pSubMesh[iSubMesh];
    
    return PrivateInst.RigidDList[(s32)SubMesh.iDList];
}

//==============================================================================

static
void pc_InitializeCubeMapSurface( void )
{
    if( !g_pd3dDevice )
        return;

    dxerr Error;

    // make a depth buffer to go with the first texture
    Error = g_pd3dDevice->CreateDepthStencilSurface( kEnvTextureW, kEnvTextureH,
                                                     D3DFMT_D24X8,
                                                     D3DMULTISAMPLE_NONE, 0,
                                                     FALSE,
                                                     &s_pEnvMapZBuffer,
                                                     NULL );
    ASSERT( Error == 0 );

    // We sould not need mip mapping for this.
    Error = g_pd3dDevice->CreateTexture( kEnvTextureW, kEnvTextureH, 1, 
                                         D3DUSAGE_RENDERTARGET,
                                         D3DFMT_A8R8G8B8, 
                                         D3DPOOL_DEFAULT,
                                         &s_pEnvMapTexture,
                                         NULL );
    ASSERT( Error == 0 );

    // Get the first and only mipmap 
    Error = s_pEnvMapTexture->GetSurfaceLevel(0, &s_pEnvMapSurface );
    ASSERT( Error == 0 );
}

void pc_PreResetCubeMap( void )
{
    if( s_pEnvMapSurface )
    {
        s_pEnvMapSurface->Release();
        s_pEnvMapSurface = NULL;
    }

    if( s_pEnvMapTexture )
    {
        s_pEnvMapTexture->Release();
        s_pEnvMapTexture = NULL;
    }

    if( s_pEnvMapZBuffer )
    {
        s_pEnvMapZBuffer->Release();
        s_pEnvMapZBuffer = NULL;
    }
}

void pc_PostResetCubeMap( void )
{
    pc_InitializeCubeMapSurface();
}

//=============================================================================
//=============================================================================
// Implementation
//=============================================================================
//=============================================================================

static
void platform_Init( void )
{
    s_RigidVertMgr.Init( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1, sizeof( rigid_geom::vertex_pc ) );
    s_SkinVertMgr.Init();
    s_EnvMapMatrix.Identity();
    s_CustomEnvMapMatrix.Identity();
    s_TextureAlphaFactor = 255;
    pc_InitializeCubeMapSurface();
}

//=============================================================================

static
void platform_Kill( void )
{
    s_RigidVertMgr.Kill();
    s_SkinVertMgr.Kill();
}

//=============================================================================

static
void platform_ActivateMaterial( const material& Material )
{
    if( !g_pd3dDevice )
        return;

    x_try;

    s_pMaterial = &Material;
    g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    // get pointers to the necessary textures    
    texture* pDiffuse     = Material.m_DiffuseMap.GetPointer();
    texture* pEnvironment = Material.m_EnvironmentMap.GetPointer();
    texture* pDetail      = Material.m_DetailMap.GetPointer();
    const xbitmap* pDiffuseMap     = pDiffuse     ? &pDiffuse->m_Bitmap     : NULL;
    const xbitmap* pEnvironmentMap = pEnvironment ? &pEnvironment->m_Bitmap : NULL;
    const xbitmap* pDetailMap      = pDetail      ? &pDetail->m_Bitmap      : NULL;

    pc_blend_mode BlendMode = PC_BLEND_NORMAL;
    if( Material.m_Flags & geom::material::FLAG_IS_ADDITIVE )
        BlendMode = PC_BLEND_ADDITIVE;
    else if( Material.m_Flags & geom::material::FLAG_IS_SUBTRACTIVE )
        BlendMode = PC_BLEND_SUBTRACTIVE;

    // Reset D3D to a known state
    pc_ResetTextureStages();

    // punchthru on or off?
    g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, (Material.m_Flags & geom::material::FLAG_IS_PUNCH_THRU) ? TRUE : FALSE );

    // Throw an exception if the diffuse isn't found
    if ( pDiffuseMap == NULL )
    {   
        x_throw( xfs("Diffuse texture (%s) missing", Material.m_DiffuseMap.GetName()) );
    }

    // Setup the D3D Texture States
    switch( Material.m_Type )
    {
        case Material_Diff :
            s_TextureAlphaFactor = 255;
            pc_Diff( pDiffuseMap, pDetailMap );
            break;
        
        case Material_Alpha :
            s_TextureAlphaFactor = 255;
            pc_Alpha( pDiffuseMap, pDetailMap, !!(Material.m_Flags & geom::material::FLAG_FORCE_ZFILL), BlendMode );
            break;

        case Material_Diff_PerPixelEnv :
            s_TextureAlphaFactor = 255;
            if( !(Material.m_Flags & geom::material::FLAG_ENV_CUBE_MAP) && (pEnvironmentMap == NULL) )
                x_throw( "Environment map is required for this Material type" );
            pc_Diff_PerPixelEnv( pDiffuseMap,
                                 pEnvironmentMap,
                                 pDetailMap,
                                 !!(Material.m_Flags & geom::material::FLAG_ENV_CUBE_MAP) );
            break;

        case Material_Diff_PerPixelIllum :
            s_TextureAlphaFactor = 255;
            pc_Diff_PerPixelIllum( pDiffuseMap, pDetailMap );
            break;

        case Material_Alpha_PerPolyEnv :
            if( !(Material.m_Flags & geom::material::FLAG_ENV_CUBE_MAP) && (pEnvironmentMap == NULL) )
                x_throw( "Environment map is required for this Material type" );
            
            // TODO: get this factor from the material
            s_TextureAlphaFactor = 128;
            pc_Alpha_PerPolyEnv( pDiffuseMap,
                                 pEnvironmentMap,
                                 pDetailMap,
                                 !!(Material.m_Flags & geom::material::FLAG_ENV_CUBE_MAP),
                                 !!(Material.m_Flags & geom::material::FLAG_FORCE_ZFILL),
                                 BlendMode );
            break;

        case Material_Alpha_PerPixelIllum :
            s_TextureAlphaFactor = 255;
            pc_Alpha_PerPixelIllum( pDiffuseMap, pDetailMap, !!(Material.m_Flags & geom::material::FLAG_FORCE_ZFILL), BlendMode );
            break;

        case Material_Alpha_PerPolyIllum :
            s_TextureAlphaFactor = 255;
            pc_Alpha_PerPixelIllum( pDiffuseMap, pDetailMap, !!(Material.m_Flags & geom::material::FLAG_FORCE_ZFILL), BlendMode );
            break;

        case Material_Distortion :
            s_TextureAlphaFactor = 255;
            pc_Distortion();
            break;
        case Material_Distortion_PerPolyEnv :
            s_TextureAlphaFactor = 255;
            pc_Distortion_PerPolyEnv();
            break;

        default :
            x_throw( "Unknown Material type" );
            break;
    }

    {
        // now we are done
        pc_SetTextureColorStage ( s_NStages, D3DTA_NOTUSED, D3DTOP_DISABLE, D3DTA_NOTUSED );
        pc_SetTextureAlphaStage ( s_NStages, D3DTA_NOTUSED, D3DTOP_DISABLE, D3DTA_NOTUSED );
    }

    x_catch_display;
}

//=============================================================================

static
void platform_ActivateDistortionMaterial( const material* pMaterial, const radian3& NormalRot )
{
    (void)NormalRot;

    // Reset D3D to a known state
    pc_ResetTextureStages();
    pc_Distortion();

    // now we are done
    pc_SetTextureColorStage ( s_NStages, D3DTA_NOTUSED, D3DTOP_DISABLE, D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( s_NStages, D3DTA_NOTUSED, D3DTOP_DISABLE, D3DTA_NOTUSED );
}

//=============================================================================

static
void platform_ActivateZPrimeMaterial( void )
{
    if( !g_pd3dDevice )
        return;

    // Reset D3D to a known state
    pc_ResetTextureStages();

    // Prime Z buffer pass
    pc_SetZBufferWrite   ( TRUE );
    pc_SetBackFaceCulling( FALSE );
    pc_SetZBias( 0 );
    g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    // Setup alpha blend so frame buffer is not touched
    s_NStages = 1;
    pc_SetTextureColorStage ( 0, D3DTA_DIFFUSE, D3DTOP_SELECTARG1, D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 0, D3DTA_DIFFUSE, D3DTOP_SELECTARG1, D3DTA_NOTUSED );
    pc_SetTextureColorStage ( 1, D3DTA_NOTUSED, D3DTOP_DISABLE, D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( 1, D3DTA_NOTUSED, D3DTOP_DISABLE, D3DTA_NOTUSED );

    //g_pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0) ;
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE          );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ZERO );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ONE  );
}

//=============================================================================

static
void platform_BeginRigidGeom( geom* pGeom, s32 iSubMesh )
{
    (void)iSubMesh;
    ASSERT( s_pRigidGeom == NULL );
    s_pRigidGeom = (rigid_geom*)pGeom;
    s_RigidVertMgr.BeginRender();
}

//=============================================================================

static
void platform_EndRigidGeom( void )
{
    ASSERT( s_pRigidGeom );
    s_pRigidGeom = NULL;
}

//=============================================================================

static
void platform_BeginSkinGeom( geom* pGeom, s32 iSubMesh )
{
    (void)iSubMesh;
    ASSERT( s_pSkinGeom == NULL );
    s_pSkinGeom = (skin_geom*)pGeom;
    s_SkinVertMgr.BeginRender();
}

//=============================================================================

static
void platform_EndSkinGeom( void )
{
    ASSERT( s_pSkinGeom );
    s_pSkinGeom = NULL;
}

//=============================================================================

static
void platform_RenderRigidInstance( render_instance& Inst )
{
    if( !g_pd3dDevice )
        return;

    // handle projected shadows
    s32 nStages = s_NStages;
    if( (Inst.Flags & render::INSTFLAG_PROJ_SHADOW_1) &&
        s_ShadowProjections[0].Texture.GetPointer() )
    {
        // set up the texture
        pc_SetTexture( nStages, &s_ShadowProjections[0].Texture.GetPointer()->m_Bitmap );
        pc_SetTextureColorStage( nStages, D3DTA_TEXTURE, D3DTOP_MODULATE2X, D3DTA_CURRENT );
        pc_SetTextureAlphaStage( nStages, D3DTA_CURRENT, D3DTOP_SELECTARG1, D3DTA_NOTUSED );

        // set up the uv transform
        g_pd3dDevice->SetTransform( s_TextureType[nStages], (D3DXMATRIX*)&s_ShadowProjectionMatrices[0] );
        g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
        g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3| D3DTTFF_PROJECTED );

        // set up the wrapping
        g_pd3dDevice->SetSamplerState( nStages, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP     );
        g_pd3dDevice->SetSamplerState( nStages, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP     );

        // we have one more stage now
        nStages++;
    }

    if( (Inst.Flags & render::INSTFLAG_PROJ_SHADOW_2) &&
        s_ShadowProjections[1].Texture.GetPointer() )
    {
        // set up the texture
        pc_SetTexture( nStages, &s_ShadowProjections[1].Texture.GetPointer()->m_Bitmap );
        pc_SetTextureColorStage( nStages, D3DTA_TEXTURE, D3DTOP_MODULATE2X, D3DTA_CURRENT );
        pc_SetTextureAlphaStage( nStages, D3DTA_CURRENT, D3DTOP_SELECTARG1, D3DTA_NOTUSED );

        // set up the uv transform
        g_pd3dDevice->SetTransform( s_TextureType[nStages], (D3DXMATRIX*)&s_ShadowProjectionMatrices[1] );
        g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
        g_pd3dDevice->SetTextureStageState( nStages, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3| D3DTTFF_PROJECTED );

        // set up the wrapping
        g_pd3dDevice->SetSamplerState( nStages, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP     );
        g_pd3dDevice->SetSamplerState( nStages, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP     );

        // we have one more stage now
        nStages++;
    }

    // terminate the stages
    pc_SetTexture( nStages, NULL );
    pc_SetTextureColorStage ( nStages, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );
    pc_SetTextureAlphaStage ( nStages, D3DTA_NOTUSED, D3DTOP_DISABLE,    D3DTA_NOTUSED );

    // render the instance
    if( !(Inst.Flags & render::PULSED) )
    {
        pc_SetTextureFactor( xcolor( 255, 255, 255, s_TextureAlphaFactor ) );
        pc_SetZBias( 0 );
        pc_RenderRigid( Inst.hDList, Inst.Data.Rigid.pL2W );
    }

    if( Inst.Flags & render::PULSED )
    {
        s32 I = (s32)( 128.0f + (80.0f * x_sin( x_fmod( s_PulseTime * 4, PI*2.0f ) )) );
    
        pc_SetTextureFactor( xcolor( 255, 0, 0, I ) );
        pc_SetZBias( 1 );
        pc_SetWireframe( TRUE );
        pc_RenderRigid( Inst.hDList, Inst.Data.Rigid.pL2W );
        pc_SetWireframe( FALSE );
    }
    
    if( Inst.Flags & render::WIREFRAME )
    {
        pc_SetTextureFactor( xcolor( 255, 0, 0 ) );
        pc_SetZBias( 1 );
        pc_SetWireframe( TRUE );
        pc_RenderRigid( Inst.hDList, Inst.Data.Rigid.pL2W );
        pc_SetWireframe( FALSE );
    }

    if( Inst.Flags & render::WIREFRAME2 )
    {
        pc_SetTextureFactor( xcolor( 0, 255, 0 ) );
        pc_SetZBias( 1 );
        pc_SetWireframe( TRUE );
        pc_RenderRigid( Inst.hDList, Inst.Data.Rigid.pL2W );
        pc_SetWireframe( FALSE );
    }
}

//=============================================================================

static
void platform_RenderSkinInstance( render_instance& Inst )
{
    if( !g_pd3dDevice )
        return;

    if( Inst.Flags & render::FADING_ALPHA )
    {
        // Z buffer has already been primed.
        // Just need to turn Alpha Blending ON and Z-Buffer writes OFF
        pc_SetAlphaBlending  ( PC_BLEND_NORMAL );
        pc_SetZBufferWrite   ( FALSE );
        pc_SetBackFaceCulling( FALSE );

        // Render transparent geometry        
        pc_RenderSkin( Inst.hDList, Inst.Data.Skin.pBones, (d3d_skin_lighting*)Inst.pLighting );
    }
    else
    {
        // Normal render
        pc_SetTextureFactor( XCOLOR_WHITE );
        pc_SetZBias( 0 );
        pc_RenderSkin( Inst.hDList, Inst.Data.Skin.pBones, (d3d_skin_lighting*)Inst.pLighting );
    }
}

//=============================================================================

static
void platform_RegisterRigidInstance( rigid_geom& Geom, render::hgeom_inst hInst )
{
    private_instance& PrivateInst = s_lRegisteredInst(hInst);
    PrivateInst.RigidDList.Clear();
    PrivateInst.IsLit = FALSE;

    // Copy the instance over (the pc renderer isn't really instance based, so
    // we need to make a copy of the verts)
    rigid_geom::dlist_pc* pPCDList = Geom.m_System.pPC;

    // Allocate work memory and setup pointers for the copy loop
    s32 nVerts = Geom.GetNVerts();
    rigid_geom::vertex_pc* pBuffer = new rigid_geom::vertex_pc[nVerts];
    rigid_geom::vertex_pc* pVertex = pBuffer;
    rigid_geom::vertex_pc* pEnd    = (pVertex + nVerts);

    // Loop through all SubMeshs
    for ( s32 iSubMesh = 0; iSubMesh < Geom.m_nSubMeshes; iSubMesh++ )
    {
        // Get the DList for this submesh
        geom::submesh&        GeomSubMesh = Geom.m_pSubMesh[iSubMesh];
        rigid_geom::dlist_pc& DList       = pPCDList[GeomSubMesh.iDList];

        // Copy vertex data
        for ( s32 iVert = 0; iVert < DList.nVerts; iVert++ )
        {
            pVertex[iVert].Pos    = DList.pVert[iVert].Pos;
            pVertex[iVert].UV     = DList.pVert[iVert].UV;
            pVertex[iVert].Normal = DList.pVert[iVert].Normal;
            pVertex[iVert].Color  = xcolor( 128, 128, 128, 255 );
        }

        // create a new handle in the private instance
        xhandle& hDList = PrivateInst.RigidDList.Append();

        // create the dlist and store the handle out
        hDList = s_RigidVertMgr.AddDList( pVertex,
                                          DList.nVerts,
                                          DList.pIndices,
                                          DList.nIndices,
                                          DList.nIndices/3 );

        // advance the ptr
        pVertex += DList.nVerts;
    }

    // Free the work memory
    delete []pBuffer;
    ASSERT( pVertex == pEnd );
}

//=============================================================================

static
void platform_RegisterSkinInstance( skin_geom& Geom, render::hgeom_inst hInst )
{
    (void)Geom;
    (void)hInst;
}

//=============================================================================

static
void platform_UnregisterRigidInstance( render::hgeom_inst hInst )
{
    private_instance& PrivateInst = s_lRegisteredInst(hInst);
    for ( s32 i = 0; i < PrivateInst.RigidDList.GetCount(); i++ )
    {
        s_RigidVertMgr.DelDList( PrivateInst.RigidDList[i] );
    }
    PrivateInst.RigidDList.Clear();
}

//=============================================================================

static
void platform_UnregisterSkinInstance( render::hgeom_inst hInst )
{
    (void)hInst;
}

//=============================================================================

static
void platform_BeginSession( u32 nPlayers )
{
}

//=============================================================================

static
void platform_EndSession( void )
{
}

//=============================================================================

static
void platform_RegisterMaterial( material& Mat )
{
    (void)Mat;
}

//=============================================================================

static
void platform_RegisterRigidGeom( rigid_geom& Geom  )
{
}

//=============================================================================

static
void platform_UnregisterRigidGeom( rigid_geom& Geom  )
{
}

//=============================================================================

static
void platform_RegisterSkinGeom( skin_geom& Geom )
{
    private_geom& PrivateGeom = s_lRegisteredGeoms(Geom.m_hGeom);
    PrivateGeom.SkinDList.Clear();

    // make a private copy of the display lists and register them with the
    // skin vert manager
    skin_geom::dlist_pc* pPCDList = Geom.m_System.pPC;

    // Allocate work memory and setup pointers for the copy loop
    s32 nVerts = Geom.GetNVerts();
    skin_geom::vertex_pc* pBuffer = new skin_geom::vertex_pc[nVerts];
    skin_geom::vertex_pc* pVertex = pBuffer;
    skin_geom::vertex_pc* pEnd    = (pVertex + nVerts);

    // Loop through all SubMeshs
    for ( s32 iSubMesh = 0; iSubMesh < Geom.m_nSubMeshes; iSubMesh++ )
    {
        // Get the DList for this submesh
        geom::submesh&       GeomSubMesh = Geom.m_pSubMesh[iSubMesh];
        skin_geom::dlist_pc& DList       = pPCDList[GeomSubMesh.iDList];

        // Copy vertex data
        for ( s32 iVert = 0; iVert < DList.nVertices; iVert++ )
        {
            // Copy vert
            pVertex[iVert].Position  = DList.pVertex[iVert].Position;
            pVertex[iVert].Normal    = DList.pVertex[iVert].Normal;
            pVertex[iVert].UVWeights = DList.pVertex[iVert].UVWeights;

            // Convert matrix indices into matrix offsets for the vertex shader
            pVertex[iVert].Position.GetW() *= VS_BONE_SIZE;
            pVertex[iVert].Normal.GetW()   *= VS_BONE_SIZE;
        }

        // Create a new handle
        xhandle& hDList = PrivateGeom.SkinDList.Append();

        // Create the dlist and store out the handle
        hDList = s_SkinVertMgr.AddDList( pVertex,
                                         DList.nVertices,
                                         (u16*)DList.pIndex,
                                         DList.nIndices,
                                         DList.nIndices / 3,
                                         DList.nCommands,
                                         DList.pCmd );

        // Advance ptrs
        pVertex += DList.nVertices;
    }

    // Free the work memory
    delete []pBuffer;
    ASSERT( pVertex == pEnd );
}

//=============================================================================

static
void platform_UnregisterSkinGeom( skin_geom& Geom )
{
    private_geom& PrivateGeom = s_lRegisteredGeoms(Geom.m_hGeom);
    for ( s32 i = 0; i < PrivateGeom.SkinDList.GetCount(); i++ )
    {
        s_SkinVertMgr.DelDList( PrivateGeom.SkinDList[i] );
    }
    PrivateGeom.SkinDList.Clear();
}

//=============================================================================

static
void platform_StartRawDataMode( void )
{
}

//=============================================================================

static
void platform_EndRawDataMode( void )
{
    s_pDrawBitmap = NULL;
}

//=============================================================================

static
void platform_RenderRawStrips( s32               nVerts,
                               const matrix4&    L2W,
                               const vector4*    pPos,
                               const s16*        pUV,
                               const u32*        pColor )
{
    static const f32 ItoFScale = 1.0f/4096.0f;

    // sanity check
    ASSERTS( s_pDrawBitmap, "You must set a material first!" );
    if( nVerts < 3 )
        return;

    // fill in the l2w...note we have to reset draw to do this
    draw_Begin( DRAW_TRIANGLES, s_DrawFlags );
    draw_SetTexture( *s_pDrawBitmap );
    draw_SetL2W( L2W );

    // prime the loop by grabbing the data for the first two verts
    xcolor  C0( pColor[0]&0xff, (pColor[0]&0xff00)>>8, (pColor[0]&0xff0000)>>16, (pColor[0]&0xff000000)>>24 );
    xcolor  C1( pColor[1]&0xff, (pColor[1]&0xff00)>>8, (pColor[1]&0xff0000)>>16, (pColor[1]&0xff000000)>>24 );
    vector2 UV0( pUV[0] * ItoFScale, pUV[1] * ItoFScale );
    vector2 UV1( pUV[2] * ItoFScale, pUV[3] * ItoFScale );
    vector3 Pos0( pPos[0].GetX(), pPos[0].GetX(), pPos[0].GetZ() );
    vector3 Pos1( pPos[1].GetX(), pPos[1].GetX(), pPos[1].GetZ() );
    C0.R = (C0.R==0x80) ? 255 : (C0.R<<1);
    C0.G = (C0.G==0x80) ? 255 : (C0.G<<1);
    C0.B = (C0.B==0x80) ? 255 : (C0.B<<1);
    C0.A = (C0.A==0x80) ? 255 : (C0.A<<1);
    C1.R = (C1.R==0x80) ? 255 : (C1.R<<1);
    C1.G = (C1.G==0x80) ? 255 : (C1.G<<1);
    C1.B = (C1.B==0x80) ? 255 : (C1.B<<1);
    C1.A = (C1.A==0x80) ? 255 : (C1.A<<1);

    // Now render the raw strips. Having 0x8000 in the w component means don't
    // kick this triangle. (Just like the ADC bit on the PS2.)
    s32 i;
    for( i = 2; i < nVerts; i++ )
    {
        // grab the next vert
        xcolor  C2( pColor[i]&0xff, (pColor[i]&0xff00)>>8, (pColor[i]&0xff0000)>>16, (pColor[i]&0xff000000)>>24 );
        vector2 UV2( pUV[i*2+0] * ItoFScale, pUV[i*2+1] * ItoFScale );
        vector3 Pos2( pPos[i].GetX(), pPos[i].GetX(), pPos[i].GetZ() );
        C2.R = (C2.R==0x80) ? 255 : (C2.R<<1);
        C2.G = (C2.G==0x80) ? 255 : (C2.G<<1);
        C2.B = (C2.B==0x80) ? 255 : (C2.B<<1);
        C2.A = (C2.A==0x80) ? 255 : (C2.A<<1);

        // kick the triangle
        if( (pPos[i].GetIW() & 0x8000) != 0x8000 )
        {
            draw_Color( C0 );   draw_UV( UV0 ); draw_Vertex( Pos0 );
            draw_Color( C1 );   draw_UV( UV1 ); draw_Vertex( Pos1 );
            draw_Color( C2 );   draw_UV( UV2 ); draw_Vertex( Pos2 );
        }

        // move to the next triangle
        C0 = C1;    UV0 = UV1;  Pos0 = Pos1;
        C1 = C2;    UV1 = UV2;  Pos1 = Pos2;
    }

    // finished
    draw_End();
}

//=============================================================================

static
void platform_Render3dSprites( s32               nSprites,
                               f32               UniScale,
                               const matrix4*    pL2W,
                               const vector4*    pPositions,
                               const vector2*    pRotScales,
                               const u32*        pColors )
{
    // sanity check
    ASSERTS( s_pDrawBitmap, "You must set a material first!" );
    if( nSprites == 0 )
        return;

    // start up draw
    const matrix4& V2W = eng_GetView()->GetV2W();
    const matrix4& W2V = eng_GetView()->GetW2V();
    matrix4 S2V;
    if( pL2W )
        S2V = W2V * (*pL2W);
    else
        S2V = W2V;
    draw_ClearL2W();
    draw_Begin( DRAW_TRIANGLES, s_DrawFlags );
    draw_SetTexture( *s_pDrawBitmap );
    draw_SetL2W( V2W );

    // loop through the sprites and render them
    s32 i, j;
    for( i = 0; i < nSprites; i++ )
    {
        // 0x8000 is an active flag, meaning to skip this sprite, similar
        // to the ADC bit on the ps2.
        if( (pPositions[i].GetIW() & 0x8000) != 0x8000 )
        {
            vector3 Center( pPositions[i].GetX(), pPositions[i].GetY(), pPositions[i].GetZ() );
            Center = S2V * Center;

            // calc the four sprite corners
            vector3 Corners[4];
            f32 Sine, Cosine;
            x_sincos( -pRotScales[i].X, Sine, Cosine );

            vector3 v0( Cosine - Sine, Sine + Cosine, 0.0f );
            vector3 v1( Cosine + Sine, Sine - Cosine, 0.0f );
            Corners[0] = v0;
            Corners[1] = v1;
            Corners[2] = -v0;
            Corners[3] = -v1;
            for( j = 0; j < 4; j++ )
            {
                Corners[j].Scale( pRotScales[i].Y * UniScale );
                Corners[j] += Center;
            }

            // now render it through draw
            xcolor C( pColors[i]&0xff, (pColors[i]&0xff00)>>8, (pColors[i]&0xff0000)>>16, (pColors[i]&0xff000000)>>24 );
//            C.R = (C.R==0x80) ? 255 : (C.R<<1);
//            C.G = (C.G==0x80) ? 255 : (C.G<<1);
//            C.B = (C.B==0x80) ? 255 : (C.B<<1);
//            C.A = (C.A==0x80) ? 255 : (C.A<<1);
            draw_Color( C );
            draw_UV( 0.0f, 0.0f );  draw_Vertex( Corners[0] );
            draw_UV( 1.0f, 0.0f );  draw_Vertex( Corners[3] );
            draw_UV( 0.0f, 1.0f );  draw_Vertex( Corners[1] );
            draw_UV( 1.0f, 0.0f );  draw_Vertex( Corners[3] );
            draw_UV( 0.0f, 1.0f );  draw_Vertex( Corners[1] );
            draw_UV( 1.0f, 1.0f );  draw_Vertex( Corners[2] );
        }
    }

    // finished
    draw_End();
}

//=============================================================================

static
void platform_RenderHeatHazeSprites( s32            nSprites,
                                     f32            UniScale,
                                     const matrix4* pL2W,
                                     const vector4* pPositions,
                                     const vector2* pRotScales,
                                     const u32*     pColors )
{
    ASSERTS( FALSE, "Not implemented yet" );
    (void)nSprites;
    (void)UniScale;
    (void)pL2W;
    (void)pPositions;
    (void)pRotScales;
    (void)pColors;
}

//=============================================================================

static
void platform_RenderVelocitySprites( s32            nSprites,
                                     f32            UniScale,
                                     const matrix4* pL2W,
                                     const matrix4* pVelMatrix,
                                     const vector4* pPositions,
                                     const vector4* pVelocities,
                                     const u32*     pColors )
{
    // sanity check
    ASSERTS( s_pDrawBitmap, "You must set a material first!" );
    if( nSprites == 0 )
        return;

    // start up draw
    draw_ClearL2W();
    draw_Begin( DRAW_TRIANGLES, s_DrawFlags );
    draw_SetTexture( *s_pDrawBitmap );

    // Grab out a l2w matrix to use. If one is not specified, then
    // we will use the identity matrix.
    matrix4 L2W;
    if( pL2W )
        L2W = *pL2W;
    else
        L2W.Identity();

    // calculate the velocity l2w matrix
    matrix4 L2WNoTranslate = L2W;
    L2WNoTranslate.ClearTranslation();
    matrix4 VL2W = L2WNoTranslate * (*pVelMatrix);

    // grab out the view direction
    vector3 ViewDir = eng_GetView()->GetViewZ();

    // render the sprites
    s32 i;
    for( i = 0; i < nSprites; i++ )
    {
        // 0x8000 is an active flag, meaning to skip this sprite, similar
        // to the ADC bit on the ps2.
        if( (pPositions[i].GetIW() & 0x8000) != 0x8000 )
        {
            // calculate the sprite points
            vector3 P = L2W * vector3( pPositions[i].GetX(), pPositions[i].GetY(), pPositions[i].GetZ() );

            vector3 Right( pVelocities[i].GetX(), pVelocities[i].GetY(), pVelocities[i].GetZ() );
            Right = VL2W * Right;
            Right.Normalize();
            vector3 Up   = ViewDir.Cross( Right );
            Right *= pVelocities[i].GetW()*UniScale;
            Up    *= pVelocities[i].GetW()*UniScale;
            vector3 Fore = P + Right;
            vector3 Aft  = P - Right;
            vector3 V0   = Fore - Up;
            vector3 V1   = Aft  - Up;
            vector3 V2   = Aft  + Up;
            vector3 V3   = Fore + Up;

            // now render it through draw
            xcolor C( pColors[i]&0xff, (pColors[i]&0xff00)>>8, (pColors[i]&0xff0000)>>16, (pColors[i]&0xff000000)>>24 );
//            C.R = (C.R==0x80) ? 255 : (C.R<<1);
//            C.G = (C.G==0x80) ? 255 : (C.G<<1);
//            C.B = (C.B==0x80) ? 255 : (C.B<<1);
//            C.A = (C.A==0x80) ? 255 : (C.A<<1);
            draw_Color( C );
            draw_UV( 1.0f, 0.0f );  draw_Vertex( V0 );
            draw_UV( 0.0f, 0.0f );  draw_Vertex( V1 );
            draw_UV( 1.0f, 1.0f );  draw_Vertex( V3 );
            draw_UV( 0.0f, 0.0f );  draw_Vertex( V1 );
            draw_UV( 1.0f, 1.0f );  draw_Vertex( V3 );
            draw_UV( 0.0f, 1.0f );  draw_Vertex( V2 );
        }
    }

    // finished
    draw_End();
}

//=============================================================================

static
void platform_SetDiffuseMaterial( const xbitmap&    Bitmap,
                                  s32               BlendMode,
                                  xbool             ZTestEnabled )
{
    // what draw flags do we need?
    s_DrawFlags = DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_UV_CLAMP | DRAW_CULL_NONE;
    if( !ZTestEnabled )
        s_DrawFlags |= DRAW_NO_ZBUFFER;
    switch( BlendMode )
    {
    default:
    case render::BLEND_MODE_NORMAL:
    case render::BLEND_MODE_INTENSITY:
        break;
    case render::BLEND_MODE_ADDITIVE:
        s_DrawFlags |= DRAW_BLEND_ADD;
        break;
    case render::BLEND_MODE_SUBTRACTIVE:
        s_DrawFlags |= DRAW_BLEND_SUB;
        break;
    }

    // store a pointer to the bitmap...draw will be activated once we actually
    // start rendering stuff.
    s_pDrawBitmap = &Bitmap;
}

//=============================================================================

static
void platform_SetGlowMaterial( const xbitmap& Bitmap,
                               s32            BlendMode,
                               xbool          ZTestEnabled )
{
    // TODO: The glow material hasn't been implemented on the PC yet.
    platform_SetDiffuseMaterial( Bitmap, BlendMode, ZTestEnabled );
}

//=============================================================================

static
void platform_SetEnvMapMaterial( const xbitmap& Bitmap,
                                 s32            BlendMode,
                                 xbool          ZTestEnabled )
{
    // TODO: The EnvMap material hasn't been implemented on the PC yet.
    platform_SetDiffuseMaterial( Bitmap, BlendMode, ZTestEnabled );
}

//=============================================================================

static
void platform_SetDistortionMaterial( s32 BlendMode, xbool ZTestEnabled )
{
    // TODO: The distortion material hasn't been implemented on the PC yet.
    (void)BlendMode;
    (void)ZTestEnabled;
}

//=============================================================================

static
void* platform_CalculateRigidLighting( const matrix4&   L2W,
                                       const bbox&      WorldBBox )
{
    (void)L2W;
    (void)WorldBBox;

    return NULL;
}

//=============================================================================

static
void* platform_CalculateSkinLighting( u32            Flags,
                                      const matrix4& L2W,
                                      const bbox&    BBox,
                                      xcolor         Ambient )
{
    (void)Flags;

    CONTEXT("platform_CalculateSkinLighting") ;

    // Try allocate
    d3d_skin_lighting* pLighting = (d3d_skin_lighting*)smem_BufferAlloc( sizeof(d3d_skin_lighting) );
    if (!pLighting)
    {
        // Setup default lighting
        static d3d_skin_lighting Default ;
        Default.Dir.Set(0,1,0) ;
        Default.AmbCol.Set(0.3f, 0.3f, 0.3f, 1.0f) ;
        Default.DirCol.Set(1.0f, 1.0f, 1.0f, 1.0f) ;

        // Use it
        pLighting = &Default ;
    }
    else
    {
        // Setup ambient
        pLighting->AmbCol.Set((f32)Ambient.R * (1.0f / 255.0f),
                              (f32)Ambient.G * (1.0f / 255.0f),
                              (f32)Ambient.B * (1.0f / 255.0f),
                              (f32)Ambient.A * (1.0f / 255.0f) ) ;

        // Grab lights
        s32 NLights = g_LightMgr.CollectCharLights( L2W, BBox, 1 );
        if ( NLights )
        {
            // Just 1 light supported right now...
            ASSERT(NLights == 1) ;
            vector3 Dir;
            xcolor  Col;
            g_LightMgr.GetCollectedCharLight( 0, Dir, Col );

            // Setup skin light
            pLighting->Dir = Dir ;
            pLighting->DirCol.Set((f32)Col.R * (1.0f / 255.0f),
                                  (f32)Col.G * (1.0f / 255.0f),
                                  (f32)Col.B * (1.0f / 255.0f),
                                  (f32)Col.A * (1.0f / 255.0f) ) ;
        }
        else
        {
            // Turn off directional lighting
            pLighting->Dir.Zero() ;
            pLighting->DirCol.Zero() ;
        }
    }

    // Store in render instance
    ASSERT(pLighting) ;
    return pLighting;
}

//=============================================================================

static
void* platform_LockRigidDListVertex( render::hgeom_inst hInst, s32 iSubMesh )
{
    xhandle Handle = pc_GetRigidDList( hInst, iSubMesh );
    return s_RigidVertMgr.LockDListVerts( Handle );
}

//=============================================================================

static
void platform_UnlockRigidDListVertex( render::hgeom_inst hInst, s32 iSubMesh )
{
    xhandle Handle = pc_GetRigidDList( hInst, iSubMesh );
    s_RigidVertMgr.UnlockDListVerts( Handle );
}

//=============================================================================

static
void* platform_LockRigidDListIndex( render::hgeom_inst hInst, s32 iSubMesh,  s32& VertexOffset )
{
    xhandle Handle = pc_GetRigidDList( hInst, iSubMesh );
    return s_RigidVertMgr.LockDListIndices( Handle, VertexOffset );
}

//=============================================================================

static
void platform_UnlockRigidDListIndex( render::hgeom_inst hInst, s32 iSubMesh )
{
    xhandle Handle = pc_GetRigidDList( hInst, iSubMesh );
    s_RigidVertMgr.UnlockDListIndices( Handle );
}

//=============================================================================

static
void platform_BeginShaders( void )
{
    if( !g_pd3dDevice )
        return;

    // calculate the environment map matrix
    matrix4 V2C, C2T;        
    V2C = eng_GetView()->GetV2C();
    C2T.Identity();
    C2T.SetScale( vector3( 0.5f, -0.5f, 0.5f ) );
    C2T.SetTranslation( vector3( 0.5f, 0.5f, 1.0f ) );
    s_CustomEnvMapMatrix = C2T * V2C;

    // clear out draw, just in case...
    draw_ClearL2W();

    // Enable Punch-Thru
    g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,        0x00 );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC,       D3DCMP_NOTEQUAL );

    g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

}

//=============================================================================

static
void platform_EndShaders( void )
{
    if( !g_pd3dDevice )
        return;

    g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
    g_pd3dDevice->SetRenderState( D3DRS_BLENDOP,  D3DBLENDOP_ADD );

    // enable this to draw the cube map on screen for debugging
    if( 0 )
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTexture          ( 0, s_pEnvMapTexture );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_Sprite     ( vector3(0,0,0),
                          vector2(300,300),
                          xcolor(0xffffffff) );

        draw_End();
    }
}

//=============================================================================

static
void platform_CreateEnvTexture( void )
{
    if( s_pCurrCubeMap == NULL )
        return;

    if( g_pd3dDevice == NULL )
        return;

    if( s_pEnvMapSurface == NULL )
        return;

    dxerr               Error;
    D3DVIEWPORT9        OldVP;
    IDirect3DSurface9*  pBackBuffer = NULL;
    IDirect3DSurface9*  pZBuffer    = NULL;

    // save out the old viewport
    g_pd3dDevice->GetViewport( &OldVP ); 

    // Get the main ZBuffer and Back buffer
    g_pd3dDevice->GetRenderTarget       ( 0, &pBackBuffer );
    g_pd3dDevice->GetDepthStencilSurface( &pZBuffer    );

    // Tell the system to render to our texture
    Error = g_pd3dDevice->SetRenderTarget( 0, s_pEnvMapSurface );
    ASSERT( Error == 0 );

    // A standard form of clearing the screen and Zbuffer. Here to see
    // what each of the parameters means.
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         0xffffffff, 1.0f, 0L ); 

    // Set the view port 
    D3DVIEWPORT9 vp = { 0, 0, kEnvTextureW, kEnvTextureH, 0.0f, 1.0f };
    g_pd3dDevice->SetViewport( &vp );

    // move the cube to our position
    matrix4 L2W;
    L2W.Identity();
    L2W.SetScale( vector3( 1000.0f, 1000.0f, 1000.0f ) );
    L2W.SetTranslation( eng_GetView()->GetPosition() );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W );

    // set the render states
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    // render each side
    struct vertex{ float x, y, z, nx, ny, nz, u, v; };
    static vertex Vertex[]={
    // top side                                                               // Top/bottom, front/back, left/right
    { -1.0f,  1.0f, -1.0f, -0.577350f,  0.577350f, -0.577350f, 0.0f, 1.0f },  // T-F-L
    { -1.0f,  1.0f,  1.0f, -0.577350f,  0.577350f,  0.577350f, 0.0f, 0.0f },  // T-B-L
    {  1.0f,  1.0f, -1.0f,  0.577350f,  0.577350f, -0.577350f, 1.0f, 1.0f },  // T-F-R
    {  1.0f,  1.0f,  1.0f,  0.577350f,  0.577350f,  0.577350f, 1.0f, 0.0f },  // T-B-R
    // bottom side                                                            // Top/bottom, front/back, left/right
    { -1.0f, -1.0f,  1.0f, -0.577350f, -0.577350f,  0.577350f, 1.0f, 0.0f },  // B-B-L
    { -1.0f, -1.0f, -1.0f, -0.577350f, -0.577350f, -0.577350f, 1.0f, 1.0f },  // B-F-L
    {  1.0f, -1.0f,  1.0f,  0.577350f, -0.577350f,  0.577350f, 0.0f, 0.0f },  // B-B-R
    {  1.0f, -1.0f, -1.0f,  0.577350f, -0.577350f, -0.577350f, 0.0f, 1.0f },  // B-F-R
    // front side                                                             // Top/bottom, front/back, left/right
    { -1.0f, -1.0f, -1.0f, -0.577350f, -0.577350f, -0.577350f, 0.0f, 1.0f },  // B-F-L
    { -1.0f,  1.0f, -1.0f, -0.577350f,  0.577350f, -0.577350f, 0.0f, 0.0f },  // T-F-L
    {  1.0f, -1.0f, -1.0f,  0.577350f, -0.577350f, -0.577350f, 1.0f, 1.0f },  // B-F-R
    {  1.0f,  1.0f, -1.0f,  0.577350f,  0.577350f, -0.577350f, 1.0f, 0.0f },  // T-F-R
    // back side                                                              // Top/bottom, front/back, left/right
    {  1.0f, -1.0f,  1.0f,  0.577350f, -0.577350f,  0.577350f, 1.0f, 0.0f },  // B-B-R
    {  1.0f,  1.0f,  1.0f,  0.577350f,  0.577350f,  0.577350f, 1.0f, 1.0f },  // T-B-R
    { -1.0f, -1.0f,  1.0f, -0.577350f, -0.577350f,  0.577350f, 0.0f, 0.0f },  // B-B-L
    { -1.0f,  1.0f,  1.0f, -0.577350f,  0.577350f,  0.577350f, 0.0f, 1.0f },  // T-B-L
    // left side                                                              // Top/bottom, front/back, left/right
    { -1.0f, -1.0f,  1.0f, -0.577350f, -0.577350f,  0.577350f, 0.0f, 0.0f },  // B-B-L
    { -1.0f,  1.0f,  1.0f, -0.577350f,  0.577350f,  0.577350f, 1.0f, 0.0f },  // T-B-L
    { -1.0f, -1.0f, -1.0f, -0.577350f, -0.577350f, -0.577350f, 0.0f, 1.0f },  // B-F-L
    { -1.0f,  1.0f, -1.0f, -0.577350f,  0.577350f, -0.577350f, 1.0f, 1.0f },  // T-F-L
    // right side                                                             // Top/bottom, front/back, left/right
    {  1.0f, -1.0f, -1.0f,  0.577350f, -0.577350f, -0.577350f, 1.0f, 1.0f },  // B-F-R
    {  1.0f,  1.0f, -1.0f,  0.577350f,  0.577350f, -0.577350f, 0.0f, 1.0f },  // T-F-R
    {  1.0f, -1.0f,  1.0f,  0.577350f, -0.577350f,  0.577350f, 1.0f, 0.0f },  // B-B-R
    {  1.0f,  1.0f,  1.0f,  0.577350f,  0.577350f,  0.577350f, 0.0f, 0.0f },  // T-B-R
    };

    // top
    g_pd3dDevice->SetTexture( 0, vram_GetSurface(s_pCurrCubeMap->m_Bitmap[cubemap::TOP]) );
    g_pd3dDevice->SetFVF( (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &Vertex[0], sizeof(vertex) );

    // bottom
    g_pd3dDevice->SetTexture( 0, vram_GetSurface(s_pCurrCubeMap->m_Bitmap[cubemap::BOTTOM]) );
    g_pd3dDevice->SetFVF( (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &Vertex[4], sizeof(vertex) );

    // front
    g_pd3dDevice->SetTexture( 0, vram_GetSurface(s_pCurrCubeMap->m_Bitmap[cubemap::FRONT]) );
    g_pd3dDevice->SetFVF( (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &Vertex[8], sizeof(vertex) );

    // back
    g_pd3dDevice->SetTexture( 0, vram_GetSurface(s_pCurrCubeMap->m_Bitmap[cubemap::BACK]) );
    g_pd3dDevice->SetFVF( (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &Vertex[12], sizeof(vertex) );

    // left
    g_pd3dDevice->SetTexture( 0, vram_GetSurface(s_pCurrCubeMap->m_Bitmap[cubemap::LEFT]) );
    g_pd3dDevice->SetFVF( (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &Vertex[16], sizeof(vertex) );

    // right
    g_pd3dDevice->SetTexture( 0, vram_GetSurface(s_pCurrCubeMap->m_Bitmap[cubemap::RIGHT]) );
    g_pd3dDevice->SetFVF( (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &Vertex[20], sizeof(vertex) );

    // Set the main frame back
    Error = g_pd3dDevice->SetRenderTarget( 0, pBackBuffer );
    ASSERT( Error == 0 );

    // Restore all modes
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
    g_pd3dDevice->SetViewport( &OldVP );

    // Clear the L2W just in case
    L2W.Identity();
    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W );

    pBackBuffer->Release();
    pZBuffer->Release();
}

//=============================================================================

static
void platform_SetProjectedTexture( texture::handle Texture )
{
    (void)Texture;
}

//=============================================================================

static
void platform_ComputeProjTextureMatrix( matrix4& Matrix, view& View, const texture_projection& Projection )
{
    f32 ZNear = 1.0f;
    f32 ZFar  = Projection.Length;

    const xbitmap& Bitmap = Projection.Texture.GetPointer()->m_Bitmap;
    View.SetXFOV( Projection.FOV );
    View.SetZLimits( ZNear, ZFar );
    View.SetViewport( 0, 0, Bitmap.GetWidth(), Bitmap.GetHeight() );
    View.SetV2W( Projection.L2W );

    Matrix  = View.GetV2C();
    Matrix *= View.GetW2V();
    Matrix *= eng_GetView()->GetV2W();
    Matrix.Scale(vector3( 0.5f, -0.5f, 1.0f) );
    Matrix.Translate(vector3( 0.5f, 0.5f, 0.0f) );
}

//=============================================================================

static
void platform_SetTextureProjection( const texture_projection& Projection )
{
    (void)Projection;
}

//=============================================================================

static
void platform_SetTextureProjectionMatrix( const matrix4& Matrix )
{
    (void)Matrix;
}

//=============================================================================

static
void platform_SetProjectedShadowTexture( s32 Index, texture::handle Texture )
{
    // I don't believe this function is needed for the PC since it can reference
    // the statics in render.cpp directly.
    ASSERT( (Index>=0) && (Index<render::MAX_SHADOW_PROJECTORS) );
    (void)Texture;
}

//=============================================================================

static
void platform_ComputeProjShadowMatrix( matrix4& Matrix, view& View, const texture_projection& Projection  )
{
    // set up the view based on the projection parameters
    f32 ZNear = 1.0f;
    f32 ZFar  = Projection.Length ;
    const xbitmap& Bitmap = Projection.Texture.GetPointer()->m_Bitmap;
    View.SetPixelScale( 1.0f );
    View.SetXFOV( Projection.FOV );
    View.SetZLimits( ZNear, ZFar );
    View.SetViewport( 0, 0, Bitmap.GetWidth(), Bitmap.GetHeight() );
    View.SetV2W( Projection.L2W );

    // Now the texture matrix will take a point from camera space to world space
    // then to projector space, then to projector clip space, then scaled and
    // translated into projector UV space.
    Matrix  = View.GetV2C();
    Matrix *= View.GetW2V();
    Matrix *= eng_GetView()->GetV2W();
    Matrix.Scale( vector3( 0.5f, -0.5f, 1.0f ) );
    Matrix.Translate( vector3( 0.5f, 0.5f, 0.0f ) );
}

//=============================================================================

static
void platform_SetShadowProjectionMatrix( s32 Index, const matrix4& Matrix )
{
    // This function isn't necessary for the pc implemation. It will reference
    // the matrices directly out of render.cpp.
    ASSERT( (Index>=0) && (Index<render::MAX_SHADOW_PROJECTORS) );
    (void)Index;
    (void)Matrix;
}

//=============================================================================

static
void platform_SetCustomFogPalette( const texture::handle& Texture, xbool ImmediateSwitch, s32 PaletteIndex )
{
    (void)Texture;
    (void)ImmediateSwitch;
    (void)PaletteIndex;
}

//=============================================================================

static
xcolor platform_GetFogValue( const vector3& WorldPos, s32 PaletteIndex )
{
    (void)WorldPos;
    (void)PaletteIndex;
    return xcolor(255,255,255,0);
}

//=============================================================================

static
void platform_BeginPostEffects( void )
{
}

//=============================================================================

static
void platform_AddScreenWarp( const vector3& WorldPos, f32 Radius, f32 WarpAmount )
{
    (void)WorldPos;
    (void)Radius;
    (void)WarpAmount;
}

//=============================================================================

static
void platform_ApplySelfIllumGlows( f32 MotionBlurIntensity, s32 GlowCutoff )
{
    (void)MotionBlurIntensity;
    (void)GlowCutoff;
}

//=============================================================================

static
void platform_MotionBlur( f32 Intensity )
{
    (void)Intensity;
}

//=============================================================================

static
void platform_ZFogFilter( render::post_falloff_fn Fn, xcolor Color, f32 Param1, f32 Param2 )
{
    (void)Fn;
    (void)Color;
    (void)Param1;
    (void)Param2;
}

//=============================================================================

static
void platform_ZFogFilter( render::post_falloff_fn Fn, s32 PaletteIndex )
{
    (void)Fn;
    (void)PaletteIndex;
}

//=============================================================================

static
void platform_MipFilter( s32                        nFilters,
                         f32                        Offset,
                         render::post_falloff_fn    Fn,
                         xcolor                     Color,
                         f32                        Param1,
                         f32                        Param2,
                         s32                        PaletteIndex )
{
    (void)nFilters;
    (void)Offset;
    (void)Fn;
    (void)Color;
    (void)Param1;
    (void)Param2;
    (void)PaletteIndex;
}

//=============================================================================

static
void platform_MipFilter( s32                        nFilters,
                         f32                        Offset,
                         render::post_falloff_fn    Fn,
                         const texture::handle&     Texture,
                         s32                        PaletteIndex )
{
    (void)nFilters;
    (void)Offset;
    (void)Fn;
    (void)Texture;
    (void)PaletteIndex;
}

//=============================================================================

static
void platform_NoiseFilter( xcolor Color )
{
    (void)Color;
}

//=============================================================================

static
void platform_ScreenFade( xcolor Color )
{
    (void)Color;
}

//=============================================================================

static
void platform_MultScreen( xcolor MultColor, render::post_screen_blend FinalBlend )
{
    (void)MultColor;
    (void)FinalBlend;
}

//=============================================================================

void platform_RadialBlur( f32 Zoom, radian Angle, f32 AlphaSub, f32 AlphaScale  )
{
    (void)Zoom;
    (void)Angle;
    (void)AlphaSub;
    (void)AlphaScale;
}

//=============================================================================

static
void platform_EndPostEffects( void )
{
}

//=============================================================================

static
void platform_BeginShadowShaders( void )
{
    // TODO:
}

//=============================================================================

static
void platform_EndShadowShaders( void )
{
    // TODO:
}

//=============================================================================

static
void platform_StartShadowCast( void )
{
    // TODO:
}

//=============================================================================

static
void platform_EndShadowCast( void )
{
    // TODO:
}

//=============================================================================

static
void platform_StartShadowReceive( void )
{
    // TODO:
}

//=============================================================================

static
void platform_EndShadowReceive( void )
{
    // TODO:
}

//=============================================================================

static
void platform_ClearShadowProjectorList( void )
{
}

//=============================================================================

static
void platform_AddPointShadowProjection( const matrix4&         L2W,
                                        radian                 FOV,
                                        f32                    NearZ,
                                        f32                    FarZ )
{
    (void)L2W;
    (void)FOV;
    (void)NearZ;
    (void)FarZ;
}

//=============================================================================

static
void platform_AddDirShadowProjection( const matrix4&         L2W,
                                      f32                    Width,
                                      f32                    Height,
                                      f32                    NearZ,
                                      f32                    FarZ )
{
    (void)L2W;
    (void)Width;
    (void)Height;
    (void)NearZ;
    (void)FarZ;
}

//=============================================================================

static
void platform_BeginShadowCastRigid( geom* pGeom, s32 iSubMesh )
{
    (void)pGeom;
    (void)iSubMesh;
    // TODO:
}

//=============================================================================

static
void platform_RenderShadowCastRigid( render_instance& Inst )
{
    (void)Inst;
    // TODO:
}

//=============================================================================

static
void platform_EndShadowCastRigid( void )
{
    // TODO:
}

//=============================================================================

static
void platform_BeginShadowCastSkin( geom* pGeom, s32 iSubMesh )
{
    (void)pGeom;
    (void)iSubMesh;
    // TODO:
}

//=============================================================================

static
void platform_RenderShadowCastSkin( render_instance& Inst, s32 iProj )
{
    (void)Inst;
    // TODO:
}

//=============================================================================

static
void platform_EndShadowCastSkin( void )
{
    // TODO:
}

//=============================================================================

static
void platform_BeginShadowReceiveRigid( geom* pGeom, s32 iSubMesh )
{
    (void)pGeom;
    (void)iSubMesh;
    // TODO:
}

//=============================================================================

static
void platform_RenderShadowReceiveRigid( render_instance& Inst, s32 iProj )
{
    (void)Inst;
    (void)iProj;
    // TODO:
}

//=============================================================================

static
void platform_EndShadowReceiveRigid( void )
{
    // TODO:
}

//=============================================================================

static
void platform_BeginShadowReceiveSkin( geom* pGeom, s32 iSubMesh )
{
    (void)pGeom;
    (void)iSubMesh;
    // TODO:
}

//=============================================================================

static
void platform_RenderShadowReceiveSkin( render_instance& Inst )
{
    (void)Inst;
    // TODO:
}

//=============================================================================

static
void platform_EndShadowReceiveSkin( void )
{
    // TODO:
}

//=============================================================================

static
void platform_BeginDistortion( void )
{
    // TODO:
}

//=============================================================================

static
void platform_EndDistortion( void )
{
}

//=============================================================================

static
void platform_BeginNormalRender( void )
{
}

//=============================================================================

static
void platform_EndNormalRender( void )
{
}

