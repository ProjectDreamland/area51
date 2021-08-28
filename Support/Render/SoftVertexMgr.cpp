
#include "SoftVertexMgr.hpp"
#include "Shaders/SkinShader.h"
#include "Render\Shaders\SkinShader.vsa.h"

//=========================================================================
// VARIABLES
//=========================================================================
s32   soft_vertex_mgr::s_InitCount  = 0;
DWORD soft_vertex_mgr::s_hShader    = 0;

IDirect3DVertexDeclaration9*    s_pDecl = NULL;
IDirect3DVertexShader9*         s_pShader = NULL;

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

void soft_vertex_mgr::Init( void )
{
    if( s_InitCount == 0 )
    {
        // Create skin vertex shader
/*
        const DWORD Declaration[] = 
        {
	        D3DVSD_STREAM(0),
	        D3DVSD_REG(0, D3DVSDT_FLOAT4),  // Position, W=B1
	        D3DVSD_REG(1, D3DVSDT_FLOAT4),  // Normal,   W=B2
	        D3DVSD_REG(2, D3DVSDT_FLOAT4),  // XY=UV, ZW=W1,W2
	        D3DVSD_END(),
        };
*/
        const D3DVERTEXELEMENT9 Declaration[] =
        {
            {0,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            {0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
            {0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        dxerr Error;

        // Setup flags
        DWORD dwFlags = 0 ;
        //if (d3deng_GetMode() & ENG_ACT_SHADERS_IN_SOFTWARE)
            //dwFlags = D3DRS_SOFTWAREVERTEXPROCESSING ;
        //else
            //dwFlags = 0 ;

        // Create vertex shader
        if( g_pd3dDevice )
        {
            D3DXFVFFromDeclarator( Declaration, &s_hShader );
            g_pd3dDevice->CreateVertexDeclaration( Declaration, &s_pDecl );
            Error = g_pd3dDevice->CreateVertexShader( dwSkinShaderVertexShader, &s_pShader );
            if( Error )
                x_throw( "Error in D3D creating the soft skining" );
        }
    }

    // Create the vertex shader
//    vertex_mgr::Init( D3DFVF_XYZW | D3DFVF_NORMAL | D3DFVF_TEX0, sizeof( skin_geom::vertex_pc ) );
    vertex_mgr::Init( s_hShader, sizeof( skin_geom::vertex_pc ) );
    s_InitCount++;
}

//=========================================================================

void soft_vertex_mgr::Kill( void )
{
    ASSERT( s_InitCount > 0 );

    if( s_InitCount == 1 ) 
    {
        if( g_pd3dDevice && s_pShader )
            s_pShader->Release();
        if( g_pd3dDevice && s_pDecl )
            s_pDecl->Release();
    }

    s_InitCount--;

    //
    // Clear vertex stuff
    // 
    vertex_mgr::Kill();

    //
    // Clear all the soft dlists
    //
    for( s32 i=0; i<m_lSoftDList.GetCount(); i++ )
    {
        if( m_lSoftDList[i].pCmd ) 
        {
            delete[]m_lSoftDList[i].pCmd;
            m_lSoftDList[i].pCmd = NULL;
        }
    }

    m_lSoftDList.Clear();
}

//=========================================================================

xhandle soft_vertex_mgr::AddDList( 
    void*                   pVertex, 
    s32                     nVertices, 
    u16*                    pIndex, 
    s32                     nIndices, 
    s32                     nPrims, 
    s32                     nCmds, 
    skin_geom::command_pc*  pCmd )
{
    xhandle     hSoftDList;
    soft_dlist& SoftDList = m_lSoftDList.Add( hSoftDList );

    x_try;

    SoftDList.hDList    = vertex_mgr::AddDList( pVertex, nVertices, pIndex, nIndices, nPrims );
    SoftDList.nCommands = nCmds;
    SoftDList.pCmd      = new skin_geom::command_pc[ nCmds ];

    x_memcpy( SoftDList.pCmd, pCmd, sizeof(skin_geom::command_pc)*nCmds );

    x_catch_begin;

    m_lSoftDList.DeleteByHandle( hSoftDList );
    
    x_catch_end_ret;

    return hSoftDList;
}

//=========================================================================

void soft_vertex_mgr::DelDList( xhandle hDList )
{
    soft_dlist& SoftDList = m_lSoftDList( hDList );

    vertex_mgr::DelDList( SoftDList.hDList );

    if(SoftDList.pCmd) 
    {
        delete []SoftDList.pCmd;
        SoftDList.pCmd = NULL;
    }
 
    m_lSoftDList.DeleteByHandle( hDList );
}

//=========================================================================

void soft_vertex_mgr::InvalidateCache( void )
{
    vertex_mgr::InvalidateCache();
}

//=========================================================================

void soft_vertex_mgr::BeginRender( void )
{
    vertex_mgr::BeginRender();
}

//=========================================================================

void soft_vertex_mgr::DrawDList( xhandle hDList, const matrix4* pBone, const d3d_skin_lighting* pLighting )
{
    if( !g_pd3dDevice )
        return;

    soft_dlist& SoftDList = m_lSoftDList( hDList );
    dlist&      DList     = m_lDList    ( SoftDList.hDList  );
    node&       DLVert    = m_lNode     ( DList.hVertexNode );
    node&       DLIndex   = m_lNode     ( DList.hIndexNode  );

    // Must have lighting!
    ASSERT(pLighting) ;

    // Make sure that the vertex streams are activated
    ActivateStreams( SoftDList.hDList );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX,         0 );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX,         1 );
    g_pd3dDevice->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX,         2 );
    g_pd3dDevice->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX,         3 );
    g_pd3dDevice->SetTextureStageState( 4, D3DTSS_TEXCOORDINDEX,         4 );
    g_pd3dDevice->SetTextureStageState( 5, D3DTSS_TEXCOORDINDEX,         5 );
    g_pd3dDevice->SetTextureStageState( 6, D3DTSS_TEXCOORDINDEX,         6 );
    g_pd3dDevice->SetTextureStageState( 7, D3DTSS_TEXCOORDINDEX,         7 );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
    g_pd3dDevice->SetTextureStageState( 2, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
    g_pd3dDevice->SetTextureStageState( 3, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
    g_pd3dDevice->SetTextureStageState( 4, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
    g_pd3dDevice->SetTextureStageState( 5, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
    g_pd3dDevice->SetTextureStageState( 6, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
    g_pd3dDevice->SetTextureStageState( 7, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );

    // Get active view
    const view& View = *eng_GetView();

    // Setup shader
    g_pd3dDevice->SetVertexDeclaration( s_pDecl );
    g_pd3dDevice->SetVertexShader( s_pShader );

    // Download vertex shader constants
    d3d_vs_consts   VS ;
    VS.W2C          = View.GetW2C() ;
    VS.W2C.Transpose() ;
    VS.Zero         = 0.0f ;
    VS.One          = 1.0f ;
    VS.MinusOne     = -1.0f ;
    VS.Fog          = 0.0f ;
    VS.LightDirCol  = pLighting->DirCol * 0.5f ;
    VS.LightAmbCol  = pLighting->AmbCol * 0.5f ;
    g_pd3dDevice->SetVertexShaderConstantF( 0, (float*)&VS, sizeof(VS) / 16 ) ;

    // Loop over all commands
    for( s32 c = 0 ; c< SoftDList.nCommands ; c++)
    {
        // Which command?
        skin_geom::command_pc& Cmd = SoftDList.pCmd[c] ;
        switch(Cmd.Cmd)
        {
            // Load a matrix
            case skin_geom::PC_CMD_UPLOAD_MATRIX: // Arg1 = BoneID, Arg2 = CacheID
            {
                // Lookup bone id and cache id
                s16 BoneID  = Cmd.Arg1 ;
                s16 CacheID = Cmd.Arg2 ;
                ASSERT( BoneID  >= 0 );
                ASSERT( CacheID >= 0 );

                // Get transpose of bone L2W
                matrix4 L2W ;
                L2W = pBone[BoneID] ;
                L2W.Transpose() ;

                // Setup vertex shader bone
                d3d_vs_bone B ;
                B.L2W0 = *(vector4*)&L2W(0,0) ;
                B.L2W1 = *(vector4*)&L2W(1,0) ;
                B.L2W2 = *(vector4*)&L2W(2,0) ;
                B.LightDir = L2W.RotateVector(pLighting->Dir) ;   // Inverse rotate light 

                // Download to vertex shader
                ASSERT(VS_BONE_SIZE == (sizeof(B)/16)) ;
                s32 Register = VS_BONE_REG_OFFSET + (CacheID*VS_BONE_SIZE) ;
                ASSERT((Register+VS_BONE_SIZE) <= (96)) ;
                g_pd3dDevice->SetVertexShaderConstantF( Register, (float*)&B, VS_BONE_SIZE ) ;
            }
            break ;

            // Draw section
            case skin_geom::PC_CMD_DRAW_SECTION:  // Arg1 = Start,  Arg2 = End
            {
                s16 Start = Cmd.Arg1 ;
                s16 End   = Cmd.Arg2 ;

                g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,                 // Type
                                                    0,
                                                    DLVert.Offset,                      // Min vertex index
                                                    DLVert.User,                        // # of vertices indexed
                                                    DLIndex.Offset + (Start*3),         // Start into index array
                                                    (End - Start)  ) ;                      // # of primitives 
            }
            break ;
        }
    }

    g_pd3dDevice->SetVertexShader( NULL );
}
