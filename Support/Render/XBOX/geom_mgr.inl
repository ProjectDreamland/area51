//=============================================================================
//
//  XBOX Specific Geom Management Routines
//
//=============================================================================


#include "../lightmgr.hpp"
#include "../platform_render.hpp"
#include "entropy/xbox/xbox_private.hpp"
#include <xgraphics.h>

#if (!defined CONFIG_RETAIL) && (!defined _D3D8PERF_H_)
#include <D3d8perf.h>
#endif

#include "xbox_render.hpp"

static bbox s_CurrentBBox;
static xbool s_bForcedToGlow  = FALSE;
static s32  s_TotalDynLights  = 0;
static f32  s_GlowScale       = 1.0000f;
static f32  s_GlowBeg         = 0.6800f;
static f32  s_GlowEnd         = 0.7500f;
static f32  s_GlowInc         = 0.0050f; // speed of mutation pulsing
static f32  PPL_Max           = 4800.0f;// per pixel lighting max distance

extern void(*g_DrawBeginCB)(void);
extern void(*g_VSyncCB    )(void);

extern xbool g_bClearingViewport;
extern xbool g_bFlippable;
extern xbool g_b480P;
extern xbool g_b720P;

extern u32  g_PhysW;
extern u32  g_PhysH;

#ifndef CONFIG_RETAIL
    xbool g_bShowBackFaces = FALSE;
    xbool g_bFrameScaling  = TRUE ;
#else
#   define g_bShowBackFaces 0
#   define g_bFrameScaling  1
#endif

#ifdef X_DEBUG
static u32 TotalContiguous_General = 0;
static u32 TotalContiguous_Tiled   = 0;
#endif
static u32 TotalPhysicalAvail;
static u32 TotalPhysicalFree;
static u32 GetTotals( void )
{
    MEMORYSTATUS Status;
    GlobalMemoryStatus( &Status );
    TotalPhysicalAvail = Status.dwTotalPhys;
    TotalPhysicalFree  = Status.dwAvailPhys;
    return 0;
}
static u32 InitTotals = GetTotals();

//=============================================================================
//=============================================================================
// Local constants
//=============================================================================
//=============================================================================

#if MAX_VS_LIGHTS > 4
#   error Too many lights!
#endif

static const D3DTRANSFORMSTATETYPE TextureType[4] =
{
    D3DTS_TEXTURE0,
    D3DTS_TEXTURE1,
    D3DTS_TEXTURE2,
    D3DTS_TEXTURE3,
};

enum PipTarget
{
    kTARGET_MAIN,
    kTARGET_PIP,
    kTARGET_OFF
}
s_PipId = kTARGET_OFF;

// This fixes a crash whereby GetSurface() was failing in the pipeline constructor.

texture_factory::volume_handle pipeline_mgr::m_hAttenuationVolume = NULL;
texture_factory::handle        pipeline_mgr::m_pTexture[kTOTAL_SLOTS];
IDirect3DSurface8*             pipeline_mgr::m_pTarget [kTOTAL_SLOTS];
IDirect3DSurface8*             pipeline_mgr::m_pBkSurface = NULL;
xbool                          pipeline_mgr::m_NeedsAlloc = TRUE;




//=============================================================================
//=============================================================================
// Blitters
//=============================================================================
//=============================================================================



///////////////////////////////////////////////////////////////////////////////

void Blt( f32 dx,f32 dy,f32 dw,f32 dh,f32 sx,f32 sy,f32 sw,f32 sh )
{
    //  ----------------------------------------------------------------------
    //
    //  Calculate coords
    //
    const f32 dL = dx;
    const f32 dT = dy;
    const f32 dR = dL+dw;
    const f32 dB = dT+dh;
    const f32 sL = sx;
    const f32 sT = sy;
    const f32 sR = sL+sw;
    const f32 sB = sT+sh;

    struct vertex
    {
        f32 x,y,z,w;
        f32 u,v;
    }
    TriList[4] = 
    {
        { dL,dT,0,0,sL,sT },
        { dR,dT,0,0,sR,sT },
        { dR,dB,0,0,sR,sB },
        { dL,dB,0,0,sL,sB }
    };

    //  ----------------------------------------------------------------------
    //
    //  Draw quad
    //
    g_pShaderMgr->m_VSFlags.Mask = 0;
    g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
    if( SWITCH_USE_TEXTURES && g_pShaderMgr->m_PSFlags.Mask )
        g_pd3dDevice->SetPixelShaderConstant( 0,& vector4(0.0f,0.0f,0.0f,1.0f),1 );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST,1,TriList,sizeof( vertex ) );
}

///////////////////////////////////////////////////////////////////////////////

static void LightenShadows( f32* pFogColour )
{
    //  ----------------------------------------------------------------------
    //
    //  Calculate viewport
    //
    D3DVIEWPORT8 Vp;
    g_pPipeline->CalculateViewport( pipeline_mgr::kPRIMARY,Vp );

    //  ----------------------------------------------------------------------
    //
    //  Calculate coords
    //
    const f32 dL = f32(Vp.X);
    const f32 dT = f32(Vp.Y);
    const f32 dR = dL+f32(Vp.Width);
    const f32 dB = dT+f32(Vp.Height);
    const f32 sL = f32(Vp.X);
    const f32 sT = f32(Vp.Y);
    const f32 sR = sL+f32(Vp.Width);
    const f32 sB = sT+f32(Vp.Height);

    struct vertex
    {
        f32 x,y,z,w;
        f32 u,v;
    }
    TriList[4] = 
    {
        { dL,dT,0,0,sL,sT },
        { dR,dT,0,0,sR,sT },
        { dR,dB,0,0,sR,sB },
        { dL,dB,0,0,sL,sB }
    };

    //  ----------------------------------------------------------------------
    //
    //  Draw quad
    //
    g_pShaderMgr->m_VSFlags.Mask = 0;
    if( SWITCH_USE_TEXTURES && g_pShaderMgr->m_PSFlags.Mask )
        g_pd3dDevice->SetPixelShaderConstant( 0,pFogColour,1 );
    g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST,1,TriList,sizeof( vertex ) );
}

///////////////////////////////////////////////////////////////////////////////

void Blt( f32 dx,f32 dy,f32 dw,f32 dh,f32 sw,f32 sh )
{
    //  ----------------------------------------------------------------------
    //
    //  Calculate coords
    //
    const f32 dL = dx-0.5f;
    const f32 dT = dy-0.5f;
    const f32 dR = dx+dw-0.5f;
    const f32 dB = dy+dh-0.5f;
    const f32 sL = 0.0f;
    const f32 sT = 0.0f;
    const f32 sR = sL+sw;
    const f32 sB = sT+sh;

    struct vertex
    {
        f32 x,y,z,w;
        f32 u,v;
    }
    TriList[4] = 
    {
        { dL,dT,0,0,sL,sT },
        { dR,dT,0,0,sR,sT },
        { dR,dB,0,0,sR,sB },
        { dL,dB,0,0,sL,sB }
    };

    //  ----------------------------------------------------------------------
    //
    //  Draw quad
    //
    g_pShaderMgr->m_VSFlags.Mask = 0;
    g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
    if( SWITCH_USE_TEXTURES && g_pShaderMgr->m_PSFlags.Mask )
        g_pd3dDevice->SetPixelShaderConstant( 0,& vector4(0.0f,0.0f,0.0f,1.0f),1 );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST,1,TriList,sizeof( vertex ) );
}

///////////////////////////////////////////////////////////////////////////////

void Blt( f32 dx,f32 dy,f32 dw,f32 dh,f32 Fade,f32 sx,f32 sy,f32 sw,f32 sh )
{
    //  ----------------------------------------------------------------------
    //
    //  Calculate coords
    //
    const D3DCOLOR d0 = 0xFFFFFFFF;

    const f32 dL = dx-0.5f;
    const f32 dT = dy-0.5f;
    const f32 dR = dx+dw-0.5f;
    const f32 dB = dy+dh-0.5f;
    const f32 sL = sx;
    const f32 sT = sy;
    const f32 sR = sL+sw;
    const f32 sB = sT+sh;

    struct vertex
    {
        f32 x,y,z,w;
        D3DCOLOR d0;
        f32 u,v;
    }
    TriList[4] = 
    {
        { dL,dT,0,0,d0,sL,sT },
        { dR,dT,0,0,d0,sR,sT },
        { dR,dB,0,0,d0,sR,sB },
        { dL,dB,0,0,d0,sL,sB }
    };

    //  ----------------------------------------------------------------------
    //
    //  Draw quad
    //
    g_RenderState.Set( D3DRS_CULLMODE, D3DCULL_NONE );
    g_RenderState.Set( D3DRS_ZWRITEENABLE, FALSE );
    if( SWITCH_USE_TEXTURES && g_pShaderMgr->m_PSFlags.Mask )
        g_pd3dDevice->SetPixelShaderConstant( 0,& vector4(0.0f,0.0f,0.0f,Fade),1 );
    g_pShaderMgr->m_VSFlags.Mask = 0;
    g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_DIFFUSE |D3DFVF_TEX1 );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST,1,TriList,sizeof( vertex ) );
}

///////////////////////////////////////////////////////////////////////////////

void Blt( f32 dx,f32 dy,f32 dw,f32 dh,f32 Fade,f32 sw,f32 sh )
{
    Blt( dx,dy,dw,dh,Fade,0.0f,0.0f,sw,sh );
}




//=============================================================================
//=============================================================================
// External contributors
//=============================================================================
//=============================================================================



///////////////////////////////////////////////////////////////////////////////

f32 FastLog2( f32 x )
{
    // trick to get a really fast log2 approximation for floating-point numbers.

    // float = sign.exp.mantissa (1.8.23 bits)
    //       = sign * 2^(exp+127) * 1.mantissa
    // method 1 using integer instructions:
    //        a) mask out and shift the exponent
    //        b) subtract 127
    // method 2 using floating-point instructions:
    //        a) treat the float as an integer, and convert it to float again
    //           when the float is looked at as an integer the upper bits have the exponent,
    //           which are the important ones. Convert that integer to float, and we have one
    //           big-ass floating-point number
    //        b) divide by 2^23 to rescale the decimal point to its proper range
    //        c) subtract 127.0f
    // With both methods, the lower bits are essentially ignored, so this is only
    // a rough approximation, and you should use some other algorithm to get accurate results.
    //
    // As a side note...converting integer to float is approximately in the form y=A*log2(x)+B,
    // and converting float to integer is the inverse x=pow(2.0, (y-B)/A). With this knowledge,
    // you can convert a "float" to a logarithmic space by doing a float->float conversion,
    // multiply by any power, add an approriate offset, and convert it back to linear space by
    // doing a float->int conversion. With this idea you can approximate any power. Useful for
    // inverse sqrts, sqrts, specular powers, etc...

    s32 i = reinterpret_cast<s32 &>(x);
    f32 f = f32(i);
    f *= 0.00000011920928955078125f;
    f -= 127.0f;
    return f;
}

///////////////////////////////////////////////////////////////////////////////

f32 xbox_CalcDistance( const matrix4& L2W, const bbox& B )
{
    vector3 Center( B.GetCenter() );
    f32     Radius = B.GetRadius();

    const view* pView = eng_GetView();
    const matrix4& W2V = pView->GetW2V();

    Center = W2V * (L2W * Center);
    f32 MinZ = Center.GetZ() - Radius;
    
    // get the z range
    if( MinZ < 0.01f ) MinZ = 0.01f;

    return MinZ;
}

///////////////////////////////////////////////////////////////////////////////

s32 xbox_GetPipTexture(void)
{
    return g_pShaderMgr->m_VRAM_PipID;
}



///////////////////////////////////////////////////////////////////////////////

void xbox_FrameCopy( s32 VRAMID )
{
    const view* pView = eng_GetView();

    s32 L = 0, T = 0;
    s32 R = s32( g_pPipeline->m_OptW );
    s32 B = s32( g_pPipeline->m_OptH );

    f32 U0 = L +  (3.0f/8.0f)*(f32)(R-L);
    f32 U1 = L +  (5.0f/8.0f)*(f32)(R-L);
    f32 V0 = T + (5.0f/14.0f)*(f32)(B-T);
    f32 V1 = T + (9.0f/14.0f)*(f32)(B-T);

    g_pPipeline->StoreFrameCopyInfo( VRAMID,U0,V0,U1,V1 );
    g_pPipeline->CopyFrameTo();
}



///////////////////////////////////////////////////////////////////////////////

void xbox_GetRes( s32& XRes,s32& YRes )
{
    if( !g_pPipeline )
    {
        eng_GetRes( XRes,YRes );
        return;
    }

    XRes = (s32)g_pPipeline->m_OptW;
    YRes = (s32)g_pPipeline->m_OptH;
}


///////////////////////////////////////////////////////////////////////////////

xbool g_bPipelineIn3D = FALSE;

void xbox_DrawBeginCB( void )
{
    if( g_pShaderMgr )
        g_pShaderMgr->SetPixelShader( kREMOVE );
}

///////////////////////////////////////////////////////////////////////////////

void xbox_SyncBeginCB( void )
{
    g_pPipeline->BeginTiming();
}



///////////////////////////////////////////////////////////////////////////////

xbool xbox_IsPipTarget( void )
{
    return g_pPipeline->m_bPipActive;
}

///////////////////////////////////////////////////////////////////////////////

void xbox_SetBackWithZ( void )
{
    g_pPipeline->SetRenderTarget( pipeline_mgr::kLAST,pipeline_mgr::kPRIMARY_Z );
}

///////////////////////////////////////////////////////////////////////////////

void xbox_SetBackSurfaceWithZ( xbool Enable )
{
    if( Enable )
        xbox_SetBackWithZ();
    else
    {
        g_pPipeline->SwitchToBackBuffer();
        g_pPipeline->LockOut( FALSE );
    }
}

///////////////////////////////////////////////////////////////////////////////

void xbox_SetFogTarget( void )
{
    xbox_SetBackWithZ();
}

///////////////////////////////////////////////////////////////////////////////

void xbox_SetPrimaryTarget( void )
{
    g_pPipeline->SetRenderTarget( pipeline_mgr::kPRIMARY,pipeline_mgr::kPRIMARY_Z );
}

///////////////////////////////////////////////////////////////////////////////

void xbox_SetPipTarget( s32 PipId,s32 W,s32 H )
{
    g_pPipeline->m_bPipActive = false;
    s_PipId = (PipTarget)PipId;

    switch( PipId )
    {
        case kTARGET_MAIN:
            g_pPipeline->SetRenderTarget( pipeline_mgr::kPRIMARY,pipeline_mgr::kPRIMARY_Z );
            g_pPipeline->LockOut( FALSE );
            break;

        case kTARGET_PIP:
            g_pPipeline->SetRenderTarget( pipeline_mgr::kPRIMARY,pipeline_mgr::kPRIMARY_Z );
            g_pd3dDevice->Clear( 0,0,D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,0,1.0f,0 );
            g_pPipeline->ResizePipTexture( W,H );
            g_pPipeline->m_bPipActive = true;
            g_pPipeline->LockOut( TRUE );
            break;

        case kTARGET_OFF:
            g_pPipeline->SwitchToBackBuffer();
            g_pPipeline->LockOut( FALSE );
            return;
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::ResizePipTexture( s32 W,s32 H )
{
    s32 Pitch = 0;

    if( W < 0 ) // -1 means use max W
    {
        W = PropW(700);
    }
    else if( W < PropW(700) )
    {
        Pitch = 0x700;
    }
    else
    {
        __asm int 3
        __asm int 3
    }

    if( H < 0 ) // -1 means use max H
    {
        H = s32(PropH(700));
    }
    else if( H > s32(PropH(700)) )
    {
        __asm int 3
        __asm int 3
    }

    *m_pTexture[kPIP] = *g_TextureFactory.Alias( Pitch,
                                                 W,
                                                 H,
                                                 D3DFMT_LIN_A8R8G8B8,
                                                 g_pShaderMgr->m_pPipData,
                                                 texture_factory::ALIAS_FROM_SCRATCH );

    *vram_GetSurface( g_pShaderMgr->m_VRAM_PipID ) = *m_pTexture[kPIP];

    IDirect3DSurface8* Surface = NULL;
    m_pTexture[kPIP]->GetSurfaceLevel( 0,&Surface );
    if( Surface )
    {
        *m_pTarget[kPIP] = *Surface;
        Surface->Release();
    }

    m_PipW = W;
    m_PipH = H;
}

///////////////////////////////////////////////////////////////////////////////

template< class T >vert_factory::handle RegisterVerts( const char* pResourceName,T& DList )
{
    ASSERT(  DList.pPushBuffer );
    ASSERT(  DList.pVert );

    vert_factory::handle hVert = g_VertFactory.Create(
        pResourceName,
        DList.nVerts*sizeof( DList.pVert[0] ),
        DList.pVert );

    DList .hVert = hVert;
    return hVert;
}



///////////////////////////////////////////////////////////////////////////////

void xbox_PreRegister( const char* pResourceName,skin_geom* pGeom )
{
#if defined( ENABLE_DEBUG_MENU )
    extern u32 g_SkinnedGeomSize,g_PushSize;
#endif
    for( s32 i=0;i<pGeom->m_nDList;i++ )
    {
        skin_geom::dlist_xbox& DList = pGeom->m_System.pXbox[i];

        // This is just to trick the rest of the codebase into thinking we're pointing to the
        // old data. It is the old data but in a region of memory both GPU and CPU can read.

        vert_factory::handle hVert=RegisterVerts( pResourceName,DList );
        DList.pVert=(skin_geom::vertex_xbox*)hVert->m_Ptr;
        {
            s32  Push,j,n = DList.nCommands;
            for( Push=j=0;j<n;j++ )
            {
                if( DList.pCmd[j].Cmd==skin_geom::XBOX_CMD_DRAW_SECTION )
                {
                    Push++;
                }
            }

            u32 BufferSize = sizeof( push_factory::buffer )*Push;
            push_factory::buffer* PushBuffer = (push_factory::buffer*)x_malloc( BufferSize );
            x_memset( PushBuffer,0,BufferSize );

            DList.pOpt = PushBuffer;

            for( Push=j=0;j<n;j++ )
            {
                skin_geom::command_xbox& Cmd = DList.pCmd[j];

                if( Cmd.Cmd==skin_geom::XBOX_CMD_DRAW_SECTION )
                {
                    push_factory::handle hPush = &PushBuffer[Push++];

                    u32 Len = Cmd.Arg1; // Push length
                    u32 Off = Cmd.Arg2; // Push offset
                #if defined( ENABLE_DEBUG_MENU )
                    g_SkinnedGeomSize += DList.nVerts*sizeof(skin_geom::vertex_xbox);
                    g_PushSize += Len;
                #endif
                    hPush->push_factory::buffer::buffer();
                    hPush->Set( pResourceName,DList.pPushBuffer+Off,Len );

                    ASSERT( hPush->Data );
                    Cmd.Arg1 = u32( hPush );
                    hPush->m_Length = Len;
                }
            }
        }
    }
}



///////////////////////////////////////////////////////////////////////////////

void xbox_Unregister( skin_geom* pGeom )
{
    for( s32 i=0;i<pGeom->m_nDList;i++ )
    {
        skin_geom::dlist_xbox& DList = pGeom->m_System.pXbox[i];
        if( DList.hVert )
        {
            vert_factory::handle Handle=( vert_factory::handle )DList.hVert;
            DList.hVert = NULL;
            delete Handle;
        }
        if( !DList.pOpt )
            continue;
        push_factory::buffer* PushBuffer = (push_factory::buffer*)DList.pOpt;
        s32  Push,j,n = DList.nCommands;
        for( Push=j=0;j<n;j++ )
        {
            skin_geom::command_xbox& Cmd = DList.pCmd[j];
            if( Cmd.Cmd==skin_geom::XBOX_CMD_DRAW_SECTION )
            {
                Cmd.Arg1 = PushBuffer[Push].m_Length;
                PushBuffer[Push].push_factory::buffer::~buffer();
                Push++;
            }
        }
        x_free( DList.pOpt );
        DList.pOpt = NULL;
    }
}



///////////////////////////////////////////////////////////////////////////////

void xbox_PreRegister( const char* pResourceName,rigid_geom* pGeom )
{
    #if defined( ENABLE_DEBUG_MENU )
    extern u32 g_RigidGeomSize,g_PushSize;
    #endif

    s32 i;
    for( i = 0; i < pGeom->m_nDList; i++ )
    {
        rigid_geom::dlist_xbox* pDList = &pGeom->m_System.pXbox[i];

        // This is just to trick the rest of the codebase into thinking we're pointing to the
        // old data. It is the old data but in a region of memory both GPU and CPU can read.

        vert_factory::handle hVert= RegisterVerts( pResourceName,*pDList );
        pDList->pVert = (rigid_geom::vertex_xbox*)hVert->m_Ptr;

        #if defined( ENABLE_DEBUG_MENU )
        g_RigidGeomSize += pDList->nVerts*sizeof(rigid_geom::vertex_xbox);
        g_PushSize      += pDList->nPushSize;
        #endif

        push_factory::handle hPushBuffer = g_PushFactory.Create( );
        pDList->hPushBuffer = hPushBuffer;
        ASSERT( hPushBuffer );
        hPushBuffer->Set(
            pResourceName,
            pDList->pPushBuffer,
            pDList->nPushSize
        );
    }
}



///////////////////////////////////////////////////////////////////////////////

void xbox_Unregister( rigid_geom* pGeom )
{
    s32 i,n = pGeom->m_nDList;
    for( i=0;i<n;i++ )
    {
        rigid_geom::dlist_xbox& DList = pGeom->m_System.pXbox[i];
        ASSERT( DList.iColor <= pGeom->m_nVertices );

        //  ------------------------------------------------------------------
        //
        //  Attach push buffer
        //
        if( DList.hPushBuffer )
        {
            push_factory::handle Handle=( push_factory::handle )DList.hPushBuffer;
            DList.hPushBuffer = NULL;
            delete Handle;
        }

        //  ------------------------------------------------------------------
        //
        //  Create vertex buffer
        //
        if( DList.pVert )
        {
            vert_factory::handle Handle=( vert_factory::handle )DList.hVert;
            DList.hVert = NULL;
            delete Handle;
        }
    }
}



//=============================================================================
//=============================================================================
// Public methods
//=============================================================================
//=============================================================================

render_target g_RenderTarget;

///////////////////////////////////////////////////////////////////////////////

bool render_target::SetTiled( s32 Target,s32 Depth )
{
    texture_factory::handle T = NULL;
    texture_factory::handle D = NULL;

    IDirect3DSurface8* t = NULL;
    IDirect3DSurface8* d = NULL;

    if( Target >= 0 )
    {
        T = g_pPipeline->m_pTexture[Target];
        t = g_pPipeline->m_pTarget [Target];
    }

    if( Depth >= 0 )
    {
        D = g_pPipeline->m_pTexture[Depth];
        d = g_pPipeline->m_pTarget [Depth];
    }

    bool A = SetTile( T,D );
    bool B = Set( Target,Depth );

    return( A && B );
}

///////////////////////////////////////////////////////////////////////////////

bool render_target::Set( s32 Target,s32 Depth )
{
    IDirect3DSurface8* t[2] = { NULL,NULL };
    const s32 Usage[2]={ Target,Depth };

    for( s32 i=0;i<2;i++ )
    {
        if( Usage[i] >= 0 )
        {
            t[i] = g_pPipeline->m_pTarget [Usage[i]];
            if( t[i] )
                t[i]->Data = g_pPipeline->m_pTexture[Usage[i]]->Data;

            #ifdef X_ASSERT
            {
                texture_factory::handle Handle = g_pPipeline->m_pTexture[Usage[i]];
                if( Handle )
                {
                    s32 Tid = Handle->GetTileId();
                    ASSERT( (Tid >= -1) && (Tid<D3DTILE_MAXTILES) );
                }
            }
            #endif
        }
        else
        {
            t[i] = NULL;
        }
    }

    return Set( t[0],t[1] );
}

///////////////////////////////////////////////////////////////////////////////

bool render_target::Set( IDirect3DSurface8* pTarget,IDirect3DSurface8* pDepth )
{
    if( pTarget != m_pTarget[m_Stack] || pDepth != m_pDepth[m_Stack] )
    {
        g_pd3dDevice->SetRenderTarget( pTarget,pDepth );
        m_pTarget[m_Stack] = pTarget;
        m_pDepth [m_Stack] = pDepth;
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool render_target::SetTile( texture_factory::handle pTarget,texture_factory::handle pDepth )
{
    s32 bResult = (pTarget || pDepth);
    if( bResult )
    {
        // set tiling for render target .......................................

        if( pTarget )
        {
            D3DTILE* pTile = pTarget->GetTilePointer();
            s32 iTile = pTarget->GetTileId();
            if( iTile > -1 )
            {
                ASSERT( iTile < D3DTILE_MAXTILES );

                g_pd3dDevice->SetTile( iTile,NULL  );
                g_pd3dDevice->SetTile( iTile,pTile );
            }
        }

        // set tiling for depth buffer ........................................

        if( pDepth )
        {
            D3DTILE* pTile = pDepth->GetTilePointer();
            s32 iTile = pDepth->GetTileId();
            if( iTile > -1 )
            {
                ASSERT( iTile < D3DTILE_MAXTILES );

                g_pd3dDevice->SetTile( iTile,NULL  );
                g_pd3dDevice->SetTile( iTile,pTile );
            }
        }
    }
    return !!bResult;
}

///////////////////////////////////////////////////////////////////////////////

void render_target::Push( IDirect3DSurface8* pTarget,IDirect3DSurface8* pDepth )
{
    ASSERT( m_Stack < 4 );
    m_Stack++;

    g_pd3dDevice->SetRenderTarget( pTarget,pDepth );

    m_pTarget[m_Stack]=pTarget;
    m_pDepth [m_Stack]=pDepth;
}

///////////////////////////////////////////////////////////////////////////////

void render_target::Pop( void )
{
    ASSERT( m_Stack > 0 );
    m_Stack--;

    g_pd3dDevice->SetRenderTarget( m_pTarget[m_Stack],m_pDepth[m_Stack] );
}

///////////////////////////////////////////////////////////////////////////////

void render_target::Reset( void )
{
    // it's safe: no __vfptr
    x_memset( this,0,sizeof(render_target) );
}



///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::BeginRigid( geom* pGeom,s32 iSubMesh )
{
#ifndef CONFIG_RETAIL
D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"Rigid mesh" );
#endif

    m_VSFlags.oPos       = 0;
    m_VSFlags.oPos_Rigid = true;
    m_pLightData         = NULL;
    m_iSubMesh           = iSubMesh;
    m_pActiveRigidGeom   = pGeom;
    m_bUseRigidGeom      = 1;
}



///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::BeginSkin( geom* pGeom,s32 iSubMesh )
{
#ifndef CONFIG_RETAIL
D3DPERF_BeginEvent( D3DCOLOR_RGBA( 23,145,2,255 ),"Skinned mesh" );
#endif

    m_VSFlags.oPos      = 0;
    m_VSFlags.oPos_Skin = true;
    m_pLightData        = NULL;
    m_iSubMesh          = iSubMesh;
    m_pActiveRigidGeom  = pGeom;
    m_bUseRigidGeom     = 0;
}



///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::End( void )
{
    g_PushFactory.Flush();
#ifndef CONFIG_RETAIL
D3DPERF_EndEvent();
#endif
}



///////////////////////////////////////////////////////////////////////////////

pipeline_mgr::~pipeline_mgr( void )
{
    g_pPipeline = NULL;

    // launch default.xbe *****************************************************

    if( SWITCH_XBOX_E3 )
    {
        LAUNCH_DATA LaunchData;
        x_memset( & LaunchData,0,sizeof( LaunchData ));
        XLaunchNewImage( "D:\\default.xbe",&LaunchData );
    }

    // exit gracefully ********************************************************

    g_DrawBeginCB = NULL;
    g_VSyncCB     = NULL;

    g_bClearingViewport = TRUE;
    g_bFlippable = TRUE;

    for( s32 i=2;i<D3DTILE_MAXTILES;i++ )
    {
        g_pd3dDevice->SetTile( i,NULL );
    }

#if defined( ENABLE_DEBUG_MENU )
    extern u32 g_SkinnedGeomSize,g_RigidGeomSize,g_PushSize;
    g_SkinnedGeomSize = 0;
    g_RigidGeomSize   = 0;
    g_PushSize        = 0;
#endif
}



///////////////////////////////////////////////////////////////////////////////

static void ClearRenderTarget( u32 Target )
{
    g_pPipeline->SetRenderTarget( Target,-1 );
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );
}



///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::InitialiseTiledMemory( void )
{
    // Setup W and H //////////////////////////////////////////////////////////

    u32 W,H,Pitch;
    {
        Pitch  = D3DTILE_PITCH_0A00;
        W = s32(m_MaxW);
        H = s32(m_MaxH);
    }

    ///////////////////////////////////////////////////////////////////////////
    //  -----------------------------------------------------------------------
    //
    //  Grab back buffer pointers
    //
    g_pd3dDevice->GetBackBuffer( 0,D3DBACKBUFFER_TYPE_MONO,&m_pBkSurface );

    ///////////////////////////////////////////////////////////////////////////
    //  -----------------------------------------------------------------------
    //
    //  Enable DXTC noise
    //
    g_RenderState.Set( D3DRS_DXT1NOISEENABLE,TRUE );



    ///////////////////////////////////////////////////////////////////////////

    #define kPRIMARY_W     g_pPipeline->m_OptW
    #define kPRIMARY_H     g_pPipeline->m_OptH
    #define PRIMARY_TILE   2
    #define PRIMARY_TILE_Z 3

    // create primary colour buffer (917504) ***********************************
    //
    // The allocation order is important. We allocate after the shadow creation
    // so the big assertion above can work. This buffer is also shared by the
    // light map.

    m_pTexture[kPRIMARY] = g_TextureFactory.CreateTiled
    (
        "kPRIMARY"          , // Resource name
        Pitch               , // Pitch
        W                   , // Width
        H                   , // Height
        m_pTarget[kPRIMARY] , // Surface
        D3DFMT_LIN_A8R8G8B8 , // Diffuse buffer format
        PRIMARY_TILE          // Stride = 0x800
    );

    // Create primary z-buffer (917504) ***************************************

    m_pTexture[kPRIMARY_Z] = g_TextureFactory.CreateTiledZ
    (
        "kPRIMARY_Z"         , // Resource name
        Pitch                , // Pitch
        W                    , // Width
        H                    , // Height
        m_pTarget[kPRIMARY_Z], // Surface
        D3DFMT_LIN_D24S8     , // Depth buffer format
        PRIMARY_TILE_Z       , // Stride = 0x800
        true                   // Compression
    );



    // Create pip buffers (229376) ********************************************
    //
    // PIPs and shadows are mutually exclusive.

    m_pTexture[kPIP] = g_pShaderMgr->m_PIP_Texture;
    m_pTarget [kPIP] = g_pShaderMgr->m_PIP_Target;



    ///////////////////////////////////////////////////////////////////////////

    // sampling buffers (360448) **********************************************

    D3DFORMAT Format = D3DFMT_LIN_A8R8G8B8;
    {
        m_pTexture[kSAMPLE0] = g_TextureFactory.CreateTiled( "kSAMPLE0",0,PropW(400),PropH(400),m_pTarget[kSAMPLE0],Format,4 );
        m_pTexture[kSAMPLE1] = g_TextureFactory.CreateTiled( "kSAMPLE1",0,PropW(200),PropH(200),m_pTarget[kSAMPLE1],Format,5 );
        m_pTexture[kSAMPLE2] = g_TextureFactory.CreateTiled( "kSAMPLE2",0,PropW(100),PropH(100),m_pTarget[kSAMPLE2],Format,6 );
        m_pTexture[kJITTER ] = g_TextureFactory.CreateTiled( "kJITTER" ,0,PropW(100),PropH(100),m_pTarget[kJITTER ],Format,7 );

        // create glow accumulator (14336) ****************************************

        m_pTexture[kGLOW_ACCUMULATOR] = g_TextureFactory.Create
        (
            xfs( "kGLOW_ACCUMULATOR" ),0,
            PropW(100),
            PropH(100),
            m_pTarget[ kGLOW_ACCUMULATOR ],
            Format,
            kPOOL_TILED
        );
    }
    ClearRenderTarget( kGLOW_ACCUMULATOR );



    ///////////////////////////////////////////////////////////////////////////

    // Create shadow buffers (1835008) ****************************************
    //
    // This code allocates all the shadow buffers OVER all others except the
    // primary buffers. PIP has no shadows, no detail mapping, etc. It's a
    // bare bones affair so we can get speed and memory reuse.

    if( SWITCH_USE_SHADOWS  )
    {
        for( s32 i=0;i<MAX_SHADOWBUFFER;i++ )
        {
            // Create texture .................................................

            texture_factory::handle Handle = g_TextureFactory.Create(
                xfs( "kSHADOW_BEGIN+%d",i ),
                Pitch/2,
                W/2,
                H/2,
                m_pTarget[SHADOW_BEGIN+i],
                D3DFMT_LIN_A8R8G8B8,
                kPOOL_TILED
            );

            // Save texture off ...............................................

            Handle->GetSurfaceLevel( 0,&m_pTarget[SHADOW_BEGIN+i] );
            m_pTexture[SHADOW_BEGIN+i] = Handle;

            // Clear it to white ..............................................

            SetRenderTarget( SHADOW_BEGIN+i,-1 );
            g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0xFFFFFFFF,0.0f,0 );
        }
    }

    // create 3D cotton ball (229376) *****************************************

    if( SWITCH_PER_PIXEL_LIGHTING )
    {
        // create attenuated volume (262144) ..................................

        #if VOL_DEBUG
        D3DFORMAT VolFormat = D3DFMT_A8R8G8B8;
        u32 VolBpp = 32;
        #else
        D3DFORMAT VolFormat = D3DFMT_A8;
        u32 VolBpp = 8;
        #endif
        {
            D3DLOCKED_BOX Lb;
            m_hAttenuationVolume = g_TextureFactory.CreateVolume
            (
                "PPL cotton ball",
                VolFormat,
                VolBpp,
                0,
                VOL_W,
                VOL_H,
                VOL_D,
                kPOOL_TILED
            );
            m_hAttenuationVolume->LockBox( 0,&Lb,NULL,0 );
            {
                u8* pAtt=(u8*)x_malloc( Lb.SlicePitch*VOL_D );
                s32 x,y,z,i;
                u8 l;
                {
                    for( z=0;z<VOL_D;z++ )
                    for( y=0;y<VOL_H;y++ )
                    for( x=0;x<VOL_W;x++ )
                    {
                        vector3 d(
                            f32(x)-f32(VOL_W/2),
                            f32(y)-f32(VOL_H/2),
                            f32(z)-f32(VOL_D/2) );
                        f32 Scale = 255.0f;
                        l = u8(Scale-x_min( 1.0f,x_max(
                            0.0f,d.Length()/f32(VOL_D/2)))*Scale );
                        #if VOL_DEBUG
                        {
                            i = (x*4)+(y*Lb.RowPitch)+(z*Lb.SlicePitch);
                            if( 1 )
                            {
                                pAtt[i+0]=l;
                                pAtt[i+1]=l;
                                pAtt[i+2]=l;
                                pAtt[i+3]=l;
                            }
                            else // debugging
                            {
                                pAtt[i+0]=x*8;
                                pAtt[i+1]=y*8;
                                pAtt[i+2]=z*8;
                                pAtt[i+3]=1;

                                if( !pAtt[i+0] )pAtt[i+0]=1;
                                if( !pAtt[i+1] )pAtt[i+1]=1;
                                if( !pAtt[i+2] )pAtt[i+2]=1;
                            }
                        }
                        #else
                        i = x+(y*Lb.RowPitch)+(z*Lb.SlicePitch);
                        pAtt[i]=l;
                        #endif
                    }
                }
                XGSwizzleBox( pAtt,0,0,NULL,Lb.pBits,VOL_W,VOL_H,VOL_D,NULL,VolBpp>>3 );
                x_free( pAtt );
            }
            m_hAttenuationVolume->UnlockBox(0);
        }
    #ifdef X_DEBUG
        g_Texture.Set( 0,m_hAttenuationVolume );
    #endif
    }

__asm wbinvd

    // Clear all targets for debugging ****************************************

#ifdef X_DEBUG
    g_RenderTarget.Set( kPRIMARY,kPRIMARY_Z );
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,1.0f,0 );

    g_RenderTarget.Set( kSAMPLE0,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );

    g_RenderTarget.Set( kSAMPLE1,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );

    g_RenderTarget.Set( kSAMPLE2,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );

    g_RenderTarget.Set( kJITTER,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );

    g_RenderTarget.Set( kGLOW_ACCUMULATOR,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );
#endif

    // Stop routine from ever being called again ******************************

    m_NeedsAlloc = FALSE;
}

///////////////////////////////////////////////////////////////////////////////

pipeline_mgr::pipeline_mgr( u32 nZones,bool bRunAt60 )
{
#ifdef X_DEBUG
    x_DebugMsg( "    Allocating pipeline manager\n" );
#endif
    x_memset( this,0,sizeof( pipeline_mgr ));
    g_pPipeline = this;

    // Ensure all resources are free
    g_pd3dDevice->BlockUntilIdle();

    // Ensure that render target is nicely disassociated
    g_pd3dDevice->SetRenderTarget( 0,0 );
    g_RenderTarget.Reset();

    // setup callback /////////////////////////////////////////////////////////

    g_DrawBeginCB = xbox_DrawBeginCB;
    g_VSyncCB     = xbox_SyncBeginCB;

    g_bClearingViewport = FALSE;

    // Setup object (no virtual ~) ////////////////////////////////////////////

    m_iDistortionCount =  1; // force full clear first time
    m_bDirtySelfIllum  = TRUE;
    m_MaxW = m_OptW    = 640.0f;
    m_MaxH = m_OptH    = 480.0f;
    m_bSplitScreen     = nZones > 1;
    m_AveDuration      =  0.0f;
    m_iSubMesh         = -1;
    m_nZones           = nZones;

    // INITIALISE THE PIPELINE ////////////////////////////////////////////////

    if( m_NeedsAlloc )
    {
        InitialiseTiledMemory();
    }

    g_RenderTarget.Set( kPRIMARY,kPRIMARY_Z );
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,1.0f,0 );

    g_RenderTarget.Set( kSAMPLE0,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );

    g_RenderTarget.Set( kSAMPLE1,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );

    g_RenderTarget.Set( kSAMPLE2,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );

    g_RenderTarget.Set( kJITTER,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );

    g_RenderTarget.Set( kGLOW_ACCUMULATOR,-1);
    g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );

    // Restore render target after monkeying above ****************************

    SwitchToBackBuffer();
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupEnvMap( u32 Flags,const xbitmap* pEnvironmentMap,const xbitmap* pDiffuseMap )
{
    /* load stages */

    if( pDiffuseMap )
        g_Texture.Set( 0,vram_GetSurface( *pDiffuseMap ));

    g_TextureStageState.Set( 1, D3DTSS_ADDRESSU , D3DTADDRESS_WRAP );
    g_TextureStageState.Set( 1, D3DTSS_ADDRESSV , D3DTADDRESS_WRAP );
    g_TextureStageState.Set( 1, D3DTSS_ADDRESSW , D3DTADDRESS_WRAP );

    /* select mapping: cube or env */

    if( Flags & geom::material::FLAG_ENV_CUBE_MAP )
    {
        // ENV_POSITIONS
        g_Texture.Set( 1,s_pCurrCubeMap->m_hTexture );
        m_VSFlags.oT0_Cube = true;
    }
    else
    {
        if( pEnvironmentMap )
            g_Texture.Set( 1,vram_GetSurface( *pEnvironmentMap ));

        if( Flags & geom::material::FLAG_ENV_VIEW_SPACE )
        {
            // ENV_VIEWSPACE

            const view* pView = eng_GetView();
            matrix4 W2V( pView->GetW2V( ));

            g_pd3dDevice->SetVertexShaderConstant( -5,&W2V,4 );
        }
        else
        {
            // ENV_WORLDSPACE

            matrix4 L2W;
            L2W.Identity();

            g_pd3dDevice->SetVertexShaderConstant( -5,&L2W,4 );
        }
        m_VSFlags.oT0_Env = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

static bool EyeInMotion( vector3 LastEye[2] )
{
    // get current position ...................................................

    const view  * pView= eng_GetView();
    vector3 Pos = pView->GetPosition();
    vector3 Old = LastEye[1];
    LastEye[1] = Pos;

    // reviewer detector: eliminate small amounts of error ....................

    radian Pitch,Yaw;
    pView->GetPitchYaw( Pitch,Yaw );
    vector3 Eye( Pitch,Yaw );

    f32 Theta    = x_abs( Eye.Dot( LastEye[0] ));
    s32 Rotating = !!(Theta < 0.9999f);
    LastEye[0]   = Eye;

    // Now determine motion ...................................................

    if( !Rotating && (Pos == Old) )
        // physically stopped
        return false;

    // If there's no gamepad action we're stopped .............................

    if( x_abs( input_GetValue( INPUT_XBOX_STICK_RIGHT_X )) > 0.5f ) return true;
    if( x_abs( input_GetValue( INPUT_XBOX_STICK_RIGHT_Y )) > 0.5f ) return true;
    if( x_abs( input_GetValue( INPUT_XBOX_STICK_LEFT_X  )) > 0.5f ) return true;
    if( x_abs( input_GetValue( INPUT_XBOX_STICK_LEFT_Y  )) > 0.5f ) return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

extern f32 eng_GetTimings( void );

void pipeline_mgr::BeginNormalRender( void )
{
    g_RenderState.Set( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
    g_RenderState.Set( D3DRS_STENCILENABLE,FALSE );

    static f32 fBias = -1.5f;
    s_bForcedToGlow  = FALSE;
    m_pLightData     = NULL;

    for( s32 i=0;i<4;i++ )
    {
        g_TextureStageState.Set( i, D3DTSS_MIPMAPLODBIAS, *((LPDWORD)(&fBias)));
        g_TextureStageState.Set( i, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
        g_TextureStageState.Set( i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
        g_TextureStageState.Set( i, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        g_TextureStageState.Set( i, D3DTSS_ADDRESSU , D3DTADDRESS_WRAP );
        g_TextureStageState.Set( i, D3DTSS_ADDRESSV , D3DTADDRESS_WRAP );
        g_TextureStageState.Set( i, D3DTSS_ADDRESSW , D3DTADDRESS_WRAP );
        g_TextureStageState.Set( i, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
    }
    g_PushFactory.Flush();
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::EndNormalRender( void )
{
    if( m_bPipActive )
    {
        // 2d states **********************************************************

        g_TextureStageState.Set( 0, D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSW,D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

        g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
        g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );
        g_RenderState.Set( D3DRS_SPECULARENABLE,FALSE );
        g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CCW );
        g_RenderState.Set( D3DRS_LIGHTING,FALSE );
        g_RenderState.Set( D3DRS_ZENABLE,FALSE );

        // 2d transform *******************************************************
        //
        // The PIP buffer had to be rendered to the primary render target so
        // to save on memory. This way we only need one Z-buffer though this
        // nasty blit it at the end consumes some framerate.

        matrix4 I; I.Identity();
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE0,( const D3DMATRIX* )&I );
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE1,( const D3DMATRIX* )&I );
        g_pd3dDevice->SetTransform( D3DTS_WORLD   ,( const D3DMATRIX* )&I );
        g_pd3dDevice->SetTransform( D3DTS_VIEW    ,( const D3DMATRIX* )&I );

        D3DXMATRIX Proj;
        D3DXMatrixPerspectiveFovLH( &Proj, D3DX_PI/3, 4.0f/3.0f, 1.0f, 100.0f );
        g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &Proj );
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_RED   |
            D3DCOLORWRITEENABLE_GREEN |
            D3DCOLORWRITEENABLE_BLUE  );

        g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET_A,0x7F7F7F7F,0.0f,0 );
        g_pd3dDevice->CopyRects( m_pTarget[kTEMP_PIP],NULL,0,m_pTarget[kPIP],NULL );
    }
    g_PushFactory.Flush();
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::BeginZPrime( void )
{
#ifndef CONFIG_RETAIL
D3DPERF_BeginEvent( D3DCOLOR_RGBA( 23,65,2,255 ),"Priming Z-buffer" );
#endif

    // Setup priming render states ............................................

    g_RenderState.Set( D3DRS_MULTISAMPLERENDERTARGETMODE,D3DMULTISAMPLEMODE_1X );
    g_RenderState.Set( D3DRS_MULTISAMPLEMODE,D3DMULTISAMPLEMODE_1X );
    g_RenderState.Set( D3DRS_MULTISAMPLEANTIALIAS,FALSE );
    g_RenderState.Set( D3DRS_OCCLUSIONCULLENABLE,TRUE );
    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
    g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );
    g_RenderState.Set( D3DRS_STENCILENABLE,FALSE );
    g_RenderState.Set( D3DRS_COLORWRITEENABLE,0 );
    g_RenderState.Set( D3DRS_ZWRITEENABLE,TRUE );
    g_RenderState.Set( D3DRS_ZENABLE,TRUE );
#ifndef CONFIG_RETAIL
    if( g_bShowBackFaces )
        g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_NONE );
#endif
    // Flush the pipeline .....................................................

    g_pShaderMgr->SetPixelShader( kREMOVE );
    g_PushFactory.Flush();
    m_bZPriming = true;

    if( m_bPipActive )
    {
        const view& View = *eng_GetView();

        rect Rect;
        View.GetViewport( Rect );
        Rect.Max.X = min( Rect.Max.X,PropW(700) );
        Rect.Max.Y = min( Rect.Max.Y,PropH(700) );

        SetAliasedTarget( kTEMP_PIP,kPRIMARY,s32(Rect.Max.X+0.5f),s32(Rect.Max.Y+0.5f) );
        ASSERT( m_pTarget[kTEMP_PIP] );

        g_pd3dDevice->Clear( 0,0,
            D3DCLEAR_TARGET
        |   D3DCLEAR_ZBUFFER
        |   D3DCLEAR_STENCIL,
            0,
            1.0f,
            0
        );
    }
}

///////////////////////////////////////////////////////////////////////////////

static u32 GetFogColour( void )
{
    return D3DCOLOR_ARGB(
        u8(s_FogColour[g_pShaderMgr->m_FogIndex].GetX()*255.0f),
        u8(s_FogColour[g_pShaderMgr->m_FogIndex].GetY()*255.0f),
        u8(s_FogColour[g_pShaderMgr->m_FogIndex].GetZ()*255.0f),
        255
    );
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::EndZPrime( void )
{
    if( !m_bPipActive )
    {
        D3DVIEWPORT8 Vp;
        CalculateViewport( kPRIMARY,Vp );

        POINT Point={ Vp.X,Vp.Y };
        RECT Rect={ Vp.X,Vp.Y,Vp.X+Vp.Width,Vp.Y+Vp.Height };
        g_pd3dDevice->CopyRects( m_pTarget[kPRIMARY],&Rect,0,m_pBkSurface,&Point );

        g_PushFactory.Flush();
        m_bZPriming = false;

        if( m_pTarget[kTEMP_PIP] )
        {
            m_pTarget[kTEMP_PIP]->Release();
            m_pTarget[kTEMP_PIP] = NULL;
        }

        SwitchToBackBuffer();
        g_pd3dDevice->SetViewport( &Vp );
        // Clear out all but the blue channel. Blue becomes alpha when we
        // the surface gets remapped to RGBA from ARGB.
        g_pd3dDevice->Clear( 0,0,
            D3DCLEAR_TARGET_A | // R
            D3DCLEAR_TARGET_R | // G
            D3DCLEAR_TARGET_G,  // B
            GetFogColour(),
            0.0f,
            0 );

        g_pShaderMgr->End();
            void RenderClothObject( void );
            RenderClothObject();
        g_pShaderMgr->Begin();

    #ifndef CONFIG_RETAIL
    D3DPERF_EndEvent();
    #endif
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::BeginLightMap( void )
{
    if( SWITCH_PER_PIXEL_LIGHTING )
    {
    #ifndef CONFIG_RETAIL
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 43,85,22,255 ),"Rendering light map" );
    #endif

        // reset the frame ....................................................

        SetRenderTarget( kLIGHTMAP,kPRIMARY_Z );
        m_bDirtyLightMap = false;
        m_bInLightMap    = true;

        // setup render states ................................................

        g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );

        g_RenderState.Set( D3DRS_MULTISAMPLERENDERTARGETMODE,D3DMULTISAMPLEMODE_1X );
        g_RenderState.Set( D3DRS_MULTISAMPLEMODE,D3DMULTISAMPLEMODE_1X );
        g_RenderState.Set( D3DRS_MULTISAMPLEANTIALIAS,FALSE );

        g_RenderState.Set( D3DRS_OCCLUSIONCULLENABLE,TRUE );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
        g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );
        g_RenderState.Set( D3DRS_SPECULARENABLE,FALSE );
        g_RenderState.Set( D3DRS_STENCILENABLE,FALSE );
        g_RenderState.Set( D3DRS_ZWRITEENABLE,FALSE );
        g_RenderState.Set( D3DRS_ZENABLE,TRUE );
        g_RenderState.Set(
            D3DRS_COLORWRITEENABLE,
                D3DCOLORWRITEENABLE_RED   |
                D3DCOLORWRITEENABLE_GREEN |
                D3DCOLORWRITEENABLE_BLUE  );

        for( s32 i=0;i<3;i++ )
        {
            g_TextureStageState.Set( i, D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU );
            g_TextureStageState.Set( i, D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP );
            g_TextureStageState.Set( i, D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP );
            g_TextureStageState.Set( i, D3DTSS_ADDRESSW,D3DTADDRESS_CLAMP );
            g_TextureStageState.Set( i, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
            g_TextureStageState.Set( i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
            g_Texture.Set( i,m_hAttenuationVolume );
        }
        g_Texture.Clear( 3 );

        // setup light map ....................................................

        m_VSFlags.clear();
        m_VSFlags.oT0_LightMapCreate = true;

        m_PSFlags.clear();
        m_PSFlags.bPerPixelLit = true;
    }
    g_PushFactory.Flush();
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::EndLightMap( void )
{
    if( SWITCH_PER_PIXEL_LIGHTING )
    {
        g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET_A,0,0.0f,0 );
        g_pd3dDevice->KickPushBuffer();
        m_bInLightMap = false;
        g_PushFactory.Flush();

    #ifndef CONFIG_RETAIL
    D3DPERF_EndEvent();
    #endif
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetDefaultDistortion( const radian3& NormalRot )
{
#ifndef CONFIG_RETAIL
D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"DEFAULT DISTORTION" );
D3DPERF_EndEvent();
#endif

    g_TextureStageState.Set( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSW,  D3DTADDRESS_CLAMP );

    // Setup render states ***************************************************

    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ZERO );
    g_RenderState.Set( D3DRS_SRCBLEND, D3DBLEND_ONE  );

    m_NormalRot = NormalRot;
    m_bZPrime   = false;

    m_VSFlags.oT0_Distortion = true;
    m_VSFlags.oT0_Env = false;

    m_PSFlags.clear();
    m_PSFlags.bDistortion = true;
    m_PSFlags.bDistortEnv = false;
}



///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetEnvDistortion( u32 Flags,const radian3& NormalRot,const xbitmap* pEnvMap )
{
    /* setup distortion *****************************************************/

    g_TextureStageState.Set( 0,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 0,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 0,D3DTSS_ADDRESSW,D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 1,D3DTSS_ADDRESSU,D3DTADDRESS_WRAP );
    g_TextureStageState.Set( 1,D3DTSS_ADDRESSV,D3DTADDRESS_WRAP );
    g_TextureStageState.Set( 1,D3DTSS_ADDRESSW,D3DTADDRESS_WRAP );

    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
    g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );

    m_VSFlags.oT0_Distortion = true;
    m_VSFlags.oT0_Env = true;

    m_PSFlags.clear();
    m_PSFlags.bDiffusePerPolyEnvAdd = true;
    m_PSFlags.bDistortion = false;
    m_PSFlags.bDistortEnv = false;

    m_NormalRot = NormalRot;



    /* select mapping: cube or env ******************************************/

    ASSERT( pEnvMap );

    g_Texture.Set( 1,vram_GetSurface( *pEnvMap ));

    // ENV_VIEWSPACE

    const view* pView = eng_GetView();
    matrix4 W2V( pView->GetW2V( ));

    g_pd3dDevice->SetVertexShaderConstant( -5,&W2V,4 );
}



///////////////////////////////////////////////////////////////////////////////

bool pipeline_mgr::SetZMaterial( const material& Material )
{
    // Setup the shader flags .............................................

    m_VSFlags.clear();
    m_PSFlags.clear();

    // Setup render states ................................................

    m_bIsPunchthru = !!( Material.m_Flags & geom::material::FLAG_IS_PUNCH_THRU );
    m_bForceZFill  = !!( Material.m_Flags & geom::material::FLAG_FORCE_ZFILL );
    m_bIsDiffIllum = !!( Material.m_Type == Material_Diff_PerPixelIllum );
    m_bAlphaBlend  = IsAlphaMaterial( (material_type)Material.m_Type );

    // Bail condition (takes care of crispy's head in fog)
    if( !m_bForceZFill && m_bAlphaBlend )
        return FALSE;

    if( m_bAlphaBlend )
        g_RenderState.Set( D3DRS_ZWRITEENABLE,FALSE );
    else
        g_RenderState.Set( D3DRS_ZWRITEENABLE,TRUE );

    #ifndef CONFIG_RETAIL
    if( !g_bShowBackFaces )
    #endif
    {
        if( (Material.m_Flags & geom::material::FLAG_DOUBLE_SIDED) || m_bAlphaBlend )
            g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_NONE );
        else
            g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CW );
    }

    // setup punchthrough .................................................

    if( m_bIsPunchthru )
    {
        texture* pDiffuse = Material.m_DiffuseMap.GetPointer( );
        ASSERT( pDiffuse );
        {
            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
            xbitmap& DiffuseMap = pDiffuse->m_Bitmap;
            vram_Activate( DiffuseMap );
            return TRUE;
        }
    }
    else
    {
        g_Texture.Clear( 1 );
        g_Texture.Clear( 2 );
        g_Texture.Clear( 3 );
    }

    // or not, it's up to you .................................................

    g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );
    g_Texture.Clear( 0 );
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
//  This routine is called to setup light map rendering: black for alpha
//  and self illuminating geometry.
//
void pipeline_mgr::SetLitMaterial( const material& Material )
{
    if( SWITCH_PER_PIXEL_LIGHTING )
    {
        // Setup render states ................................................

        m_bIsPunchthru = !!( Material.m_Flags & geom::material::FLAG_IS_PUNCH_THRU );
        m_bForceZFill  = !!( Material.m_Flags & geom::material::FLAG_FORCE_ZFILL );
        m_bIsDiffIllum = !!( Material.m_Type == Material_Diff_PerPixelIllum );
        m_bAlphaBlend  =   IsAlphaMaterial( (material_type)Material.m_Type );

        // setup back facing .. .................................................

        if( (Material.m_Flags & geom::material::FLAG_DOUBLE_SIDED) || m_bAlphaBlend )
            g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_NONE );
        else
            g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CW );

        // setup punchthrough .................................................

        if( m_bIsPunchthru )
        {
            texture* pDiffuse = Material.m_DiffuseMap.GetPointer( );
            ASSERT( pDiffuse );
            {
                g_TextureStageState.Set( 3,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );

                xbitmap& DiffuseMap = pDiffuse->m_Bitmap;
                m_LitPunchthru = vram_GetSurface( DiffuseMap );
                return;
            }
        }

        // or not, it's up to you .............................................

        g_TextureStageState.Set( 3,D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );
    }
}



///////////////////////////////////////////////////////////////////////////////
//
//  This routine sets render states, textures, and linker flags
//
void pipeline_mgr::SetMaterial( const material& Material )
{
    // Get maps ///////////////////////////////////////////////////////////////

    texture      * pDiffuse    = Material.m_DiffuseMap.GetPointer( );
    const xbitmap* pDiffuseMap = pDiffuse ? &pDiffuse->m_Bitmap:NULL;

    #ifndef X_RETAIL
    if( ! pDiffuseMap )__asm int 3
    #endif



    // Get environment map ////////////////////////////////////////////////////

    texture* pEnvironment = Material.m_EnvironmentMap.GetPointer( );
    m_pEnvironmentMap = pEnvironment ? &pEnvironment->m_Bitmap:NULL;



    // Get detail map /////////////////////////////////////////////////////////

    texture      * pDetail    = Material.m_DetailMap.GetPointer();
    const xbitmap* pDetailMap = pDetail ? &pDetail->m_Bitmap:NULL;



    // Set min/mag filters ////////////////////////////////////////////////////

    for( s32 i=0;i<4;i++ )
    {
        g_TextureStageState.Set( i,D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        g_TextureStageState.Set( i,D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    }



    // Set texture flags //////////////////////////////////////////////////////
    xbool bIsAdditive = !!(Material.m_Flags & geom::material::FLAG_IS_ADDITIVE);

    m_bSelfIllumUseDiffuse = !!(Material.m_Flags & geom::material::FLAG_ILLUM_USES_DIFFUSE);
    m_bIsPunchthru         = !!(Material.m_Flags & geom::material::FLAG_IS_PUNCH_THRU);
    m_bForceZFill          = !!(Material.m_Flags & geom::material::FLAG_FORCE_ZFILL);
    m_bIsDiffIllum         = !!( Material.m_Type == Material_Diff_PerPixelIllum );
    m_DetailScale          = Material.m_DetailScale;
    m_FixedAlpha           = Material.m_FixedAlpha;
    m_MaterialFlags        = Material.m_Flags;    
    m_MaterialType         = Material.m_Type;
    m_bZPrime              = false;

    g_pShaderMgr->SetFixedAlpha( m_FixedAlpha );



    // Setup z blending ///////////////////////////////////////////////////////

    m_bAlphaBlend = IsAlphaMaterial( (material_type)Material.m_Type );
    m_bMatCanReceiveShadow = !m_bAlphaBlend && m_bForceZFill;
    if( m_bForceZFill )
        g_RenderState.Set( D3DRS_ZWRITEENABLE,TRUE );
    if( !m_bInDistortion )
        g_RenderState.Set( D3DRS_ALPHATESTENABLE,m_bAlphaBlend );
    if( m_bAlphaBlend )
    {
        g_RenderState.Set( D3DRS_ALPHAFUNC,D3DCMP_GREATER );
        g_RenderState.Set( D3DRS_ALPHAREF,4 );
    }

    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,m_bAlphaBlend );
    g_RenderState.Set( D3DRS_OCCLUSIONCULLENABLE,TRUE );
    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
    g_RenderState.Set( D3DRS_ZENABLE,TRUE );

    g_RenderState.Set( D3DRS_MULTISAMPLERENDERTARGETMODE,D3DMULTISAMPLEMODE_1X );
    g_RenderState.Set( D3DRS_MULTISAMPLEMODE,D3DMULTISAMPLEMODE_1X );
    g_RenderState.Set( D3DRS_MULTISAMPLEANTIALIAS,FALSE );

    m_bZPrime = false;



    // Turn on bilinear filtering /////////////////////////////////////////////

    if( pDiffuseMap->GetVRAMID()==g_pShaderMgr->m_VRAM_PipID )
    {
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSW , D3DTADDRESS_CLAMP );
        m_bUsingPIP = true;
    }
    else
    {
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , D3DTADDRESS_WRAP );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , D3DTADDRESS_WRAP );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSW , D3DTADDRESS_WRAP );
        m_bUsingPIP = false;
    }

    g_Texture.Clear( 1 );
    g_Texture.Clear( 2 );
    g_Texture.Clear( 3 );




    // material switches //////////////////////////////////////////////////////

    #if SWITCH_USE_DIFFUSE_ONLY
    static xbool s_Material_Alpha_PerPixelIllum = 1;
    static xbool s_Material_Diff_PerPixelIllum  = 1;
    static xbool s_Material_Alpha_PerPolyIllum  = 1;
    static xbool s_Material_Diff_PerPixelEnv    = 1;
    static xbool s_Material_Alpha_PerPolyEnv    = 1;
    static xbool s_Material_Alpha               = 1;
    #endif



    // select pixel shader ////////////////////////////////////////////////////

    m_VSFlags.clear();
    m_PSFlags.clear();

    switch( Material.m_Type )
    {
        // Env distortion type ************************************************

        case Material_Distortion_PerPolyEnv:
        {
            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
            SetEnvDistortion( Material.m_Flags,m_NormalRot,m_pEnvironmentMap );
        }
        break;

        // Distortion type
        //
        case Material_Distortion:
            SetDefaultDistortion( m_NormalRot );
            break;

        // Diffuse per pixel illumination *************************************

        case Material_Diff_PerPixelIllum:
        {
        #if SWITCH_USE_DIFFUSE_ONLY
        if( !s_Material_Diff_PerPixelIllum )goto diff;
        #endif
            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
            g_Texture.Set( 0,vram_GetSurface( *pDiffuseMap ));
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
            m_PSFlags.bDiffusePerPixelIllum = true;
            m_VSFlags.oT0_Normal = true;
        }
        break;

        // Alpha per pixel illumination ***************************************

        case Material_Alpha_PerPixelIllum:
        {
        #if SWITCH_USE_DIFFUSE_ONLY
        if( !s_Material_Alpha_PerPixelIllum )goto diff;
        #endif
            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
            g_Texture.Set( 0,vram_GetSurface( *pDiffuseMap ));
            m_PSFlags.bAlphaPerPixelIllum = true;
            m_VSFlags.oT0_Normal = true;
        }
        break;

        // Alpha per poly illumination ****************************************

        case Material_Alpha_PerPolyIllum:
        {
        #if SWITCH_USE_DIFFUSE_ONLY
        if( !s_Material_Alpha_PerPolyIllum )goto diff;
        #endif
            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
            g_Texture.Set( 0,vram_GetSurface( *pDiffuseMap ));
            m_PSFlags.bAlphaPerPolyIllum = true;
            m_VSFlags.oT0_Normal = true;
        }
        break;

        // Diffuse per pixel environment mapping ******************************

        case Material_Diff_PerPixelEnv:
        {
        #if SWITCH_USE_DIFFUSE_ONLY
        if( !s_Material_Diff_PerPixelEnv )goto diff;
        #endif
            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
            {
                SetupEnvMap( Material.m_Flags,m_pEnvironmentMap,pDiffuseMap );
                if( bIsAdditive )
                    m_PSFlags.bDiffusePerPixelEnvAdd = true;
                else
                    m_PSFlags.bDiffusePerPixelEnv = true;
            }
        }
        break;

        // Alpha per poly environment mapping *********************************
        //
        // First pass: Diffuse texture blended with frame just like Material_Alpha
        // Second pass: Env. blend is done with a fixed alpha for the entire
        // poly using the same blend method as the first pass.
        //
        case Material_Alpha_PerPolyEnv:
        {
        #if SWITCH_USE_DIFFUSE_ONLY
        if( !s_Material_Alpha_PerPolyEnv )goto diff;
        #endif
            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
            g_Texture.Set( 0,vram_GetSurface( *pDiffuseMap ));
            m_VSFlags.oT0_Normal = true;
            m_PSFlags.bAlpha = true;
        }
        break;

        // Diffuse only *******************************************************

        case Material_Diff:
        {
        diff:
        #if SWITCH_USE_DIFFUSE_ONLY
        #endif
            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );
            if( pDiffuseMap->GetVRAMID()==g_pShaderMgr->m_VRAM_PipID )
                g_Texture.Set( 0,m_pTexture[kPIP] );
            else
                g_Texture.Set( 0,vram_GetSurface(*pDiffuseMap) );
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
            m_VSFlags.oT0_Normal = true;
            m_PSFlags.bDiffuse = true;
        }
        break;

        // Diffuse plus alpha *************************************************

        case Material_Alpha:
        {
        #if SWITCH_USE_DIFFUSE_ONLY
        if( !s_Material_Alpha )goto diff;
        #endif
            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
            g_Texture.Set( 0,vram_GetSurface( *pDiffuseMap ));
            m_VSFlags.oT0_Normal = true;
            m_PSFlags.bAlpha = true;
        }
        break;

        // Die if we get confused *********************************************

        default:
            ASSERTS( 0,"Unknown Material type" );
            break;
    }



    // Setup alpha material blends ////////////////////////////////////////////

    if( m_MaterialFlags & geom::material::FLAG_IS_ADDITIVE )
    {
        g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ONE );
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
    }
    else if( m_MaterialFlags & geom::material::FLAG_IS_SUBTRACTIVE )
    {
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_REVSUBTRACT );
        g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ONE );
    }
    else
    {
        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
        g_RenderState.Set( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
    }



    // sidedness override /////////////////////////////////////////////////////

    if( (Material.m_Flags & geom::material::FLAG_DOUBLE_SIDED) || m_bAlphaBlend )
        g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_NONE );
    else
        g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CW );



    // punch through //////////////////////////////////////////////////////////

    if( m_bIsPunchthru )
        g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
    else
        g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );



    // detail map /////////////////////////////////////////////////////////////

    if( pDetailMap )
    {
        m_pDetailMap = vram_GetSurface( *pDetailMap );
    #ifdef X_DEBUG
        g_Texture.Set( 2,m_pDetailMap );
    #endif
    }
    else
    {
        m_pDetailMap = NULL;
    }
}



///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetSkinConst( skin_geom::command_xbox& Cmd )
{
    u32 CacheID = Cmd.Arg2;
    u32 BoneID  = Cmd.Arg1;

    ASSERT( CacheID >= 0 );
    ASSERT( BoneID  >= 0 );

    const matrix4& L2W = m_pInst->Data.Skin.pBones[BoneID];

    g_pd3dDevice->SetVertexShaderConstant( CacheID*4,&L2W,4 );
}



///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::RenderInstance( void )
{
    s32 i,n;

    ///////////////////////////////////////////////////////////////////////////

    #ifndef CONFIG_RETAIL
    // This causes the back faces to show up pink
    g_RenderState.Set( D3DRS_BACKFILLMODE,D3DFILL_POINT );
    #endif

    ///////////////////////////////////////////////////////////////////////////
    //
    //  Render rigid geoms
    //
    if( m_bUseRigidGeom )
    {
        // Render dlist *******************************************************

        rigid_data& Data = m_pInst->Data.Rigid;
        rigid_geom::dlist_xbox& DList = Data.pGeom->m_System.pXbox[m_iSubMesh];
        {
            g_pd3dDevice->SetVertexShaderConstant( 0,&m_L2W,4 );

            // stream 0 .......................................................

            vert_factory::handle VStream = vert_factory::handle( DList.hVert );
            u32 Stride = sizeof( rigid_geom::vertex_xbox );
            VStream->Set( 0,Stride );

            // stream 1 .......................................................

            vert_factory::handle CStream = NULL;
            Stride = sizeof( D3DCOLOR );
            if( m_pLightData )
            {
                CStream = g_VertFactory.Alias(
                    DList.nVerts*Stride,
                    m_pLightData,
                    vert_factory::ALIAS_FROM_SCRATCH
                );
                m_bColourStreamActive = TRUE;
                CStream->Set( 1,Stride );
            }
            else
            {
                if( m_bColourStreamActive )
                {
                    g_pd3dDevice->SetStreamSource( 1,NULL,Stride );
                    m_bColourStreamActive = 0;
                }
            }

            // Draw all poly soups ............................................

            push_factory::handle( DList.hPushBuffer )->Run();
        }
        return;
    }
 

    //////////////////////////////////////////////////////////////////////////
    //-----------------------------------------------------------------------
    //
    //  Set vertex stream
    //
    skin_data& Data = m_pInst->Data.Skin;
    skin_geom::dlist_xbox& DList = Data.pGeom->m_System.pXbox[m_iSubMesh];

    /* set vertex buffer */

    vert_factory::handle hVert=( vert_factory::handle )DList.hVert;
    u32 Stride = sizeof( skin_geom::vertex_xbox );
    hVert->Set( 0,Stride );

    /* slow run */

    n = DList.nCommands;

    /* fast run */

    for( i=0;i<n;i++ )
    {
        skin_geom::command_xbox& Cmd = DList.pCmd[i];

        if( Cmd.Cmd==skin_geom::XBOX_CMD_DRAW_SECTION )
        {
            push_factory::handle( Cmd.Arg1 )->Run();
        }
        else
        {
            SetSkinConst( Cmd );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SwitchToBackBuffer( void )
{
    SetRenderTarget( kLAST,-1 );
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::CalculateViewport( s32 TargetID,D3DVIEWPORT8& Vp )
{
    const view& View = *eng_GetView();

    rect Rect;
    View.GetViewport( Rect );

    IDirect3DSurface8* Surface;
    D3DSURFACE_DESC Desc;
    if( TargetID < 0 )
        Surface = m_pTarget[kPRIMARY];
    else
        Surface = m_pTarget[TargetID];
    if( Surface )
    {
        Surface->GetDesc( &Desc );

        Rect.Max.X /= f32(g_PhysW);
        Rect.Max.Y /= f32(g_PhysH);
        Rect.Min.X /= f32(g_PhysW);
        Rect.Min.Y /= f32(g_PhysH);

        Vp.Height = DWORD( (Rect.Max.Y-Rect.Min.Y)*f32(Desc.Height) + 0.5f );
        Vp.Width  = DWORD( (Rect.Max.X-Rect.Min.X)*f32(Desc.Width ) + 0.5f );
        Vp.Y      = DWORD( (Rect.Min.Y * f32(Desc.Height)) + 0.5f );
        Vp.X      = DWORD( (Rect.Min.X * f32(Desc.Width )) + 0.5f );
        Vp.MinZ   = 0.0f;
        Vp.MaxZ   = 1.0f;

        if( (TargetID >= SHADOW_BEGIN) && (TargetID < SHADOW_LAST) )
        {
            Vp.Width  -= 2;
            Vp.Height -= 2;
            Vp.X ++;
            Vp.Y ++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetRenderTarget( s32 TargetID, s32 DepthID,D3DVIEWPORT8& Vp )
{
    switch( TargetID )
    {
        case kLAST:
        {
            g_pShaderMgr->SetPixelShader( kREMOVE );
            IDirect3DSurface8* pDepth = NULL;
            if( DepthID >-1 )
                pDepth = m_pTarget[DepthID];
            g_RenderTarget.Set( m_pBkSurface,pDepth );

            extern view g_View;
            eng_SetViewport( g_View );
            draw_DisableSatCompensation();
            break;
        }
        case kPIP:
        case kTEMP_PIP:
            g_RenderTarget.Set( TargetID, DepthID );
            draw_EnableSatCompensation();
            break;

        default:
            g_RenderTarget.SetTiled( TargetID, DepthID );
            CalculateViewport( TargetID,Vp );
            g_pd3dDevice->SetViewport( &Vp );
            draw_EnableSatCompensation();
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetRenderTarget( s32 TargetID, s32 DepthID )
{
    D3DVIEWPORT8 Vp;
    SetRenderTarget( TargetID,DepthID,Vp );
}

///////////////////////////////////////////////////////////////////////////////
//
//  This is where we initialise the frame. It's the first thing to be called.
//
void pipeline_mgr::BeginShadowPass( void )
{
#ifndef CONFIG_RETAIL
D3DPERF_BeginEvent( D3DCOLOR_RGBA( 3,45,0,255 ),"Creating shadows" );
#endif



      ///////////////
     // PAGE FLIP //
    ///////////////



    // If first zone (single player) or split screen **************************

    if( !m_Zone )
    {
        extern void PresentFrame( void );
        g_bFlippable = FALSE;
        PresentFrame();
    }

    // Setup the max resolution for the frame *********************************

    rect Rect;
    const view* pView = eng_GetView();
    pView->GetViewport( Rect );
    {
        m_OptW = m_MaxW = Rect.Max.X-Rect.Min.X;
        m_OptH = m_MaxH = Rect.Max.Y-Rect.Min.Y;
    }



      /////////////////////
     // ADJUST FILLRATE //
    /////////////////////



    // scale resolution based on framerate ************************************

    if( SWITCH_USE_FRAME_SCALING && g_bFrameScaling )
    {
        // increase effect frequency if motionless ............................

        bool bScaled = true;
        f32 MipOffset = s_Post.MipOffset[s_Post.MipPaletteIndex];
        if( MipOffset )
            bScaled = false;
        else
        {
            bool bInMotion = EyeInMotion( m_OldEye );
            if( !bInMotion )
                bScaled = false;
        }

        // calculate average duration .........................................
        //
        // use average duration over multiple frames instead
        // of the last frame's problem. this should only be
        // for uprezzing.

        if( (++m_AveDurSteps) < 4 )
            m_AveDuration += m_Duration;
        else
            m_AveDurSteps = 1;
        m_AveDuration /= f32(m_AveDurSteps);

        // Scale between PS2 and Xbox resolutions .............................

        #define lerp(a,d,s) x_clamp( (d*(1.0f-a)) + (s*a),d,s)

        if( bScaled && ((m_AveDuration > 24.0f) || (m_Duration > 33.33333f)))
        {
            f32 Time = x_clamp( m_Duration / 33.33333f,0.0f,1.0f );

            f32 MinW = f32(PropW(800));
            f32 MinH = f32(PropH(800));

            m_OptW = f32((u32(lerp(1.0f-Time,MinW,m_MaxW))+15)&~15);
            m_OptH = f32((u32(lerp(1.0f-Time,MinH,m_MaxH))+15)&~15);
        }
    }



      ///////////////////
     // CLEAR TARGETS //
    ///////////////////



    // Clear it for rendering *************************************************

    SetRenderTarget( kPRIMARY,-1 );
    #ifndef CONFIG_RETAIL
    {
        u32 Flags =
            D3DCLEAR_TARGET_R |
            D3DCLEAR_TARGET_G |
            D3DCLEAR_TARGET_B ;
        D3DCOLOR Colour;
        if( g_bShowBackFaces )
            Colour = D3DCOLOR_RGBA( 255,0,255,255 );
        else
            Colour = 0;
        g_pd3dDevice->Clear( 0,0,Flags,Colour,1.0f,0 );
    }
    #else
    {
        g_pd3dDevice->Clear(
            0,
            0,
            D3DCLEAR_TARGET_R |
            D3DCLEAR_TARGET_G |
            D3DCLEAR_TARGET_B ,
            0,
            1.0f,
            0
        );
    }
    #endif
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::EndShadowPass( void )
{
#ifndef CONFIG_RETAIL
D3DPERF_EndEvent();
#endif

    if( m_bPipActive )
        return;

    if( SWITCH_USE_SHADOWS )
    {
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALPHA );
        g_RenderState.Set( D3DRS_BLENDCOLOR,D3DCOLOR_RGBA( 0,0,0,192 ));
        g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_CONSTANTALPHA );
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ONE );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
        g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );
        g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CCW );
        g_RenderState.Set( D3DRS_ZWRITEENABLE,FALSE );
        g_RenderState.Set( D3DRS_ZENABLE,FALSE );

        g_Texture.Clear( 0 );
        g_Texture.Clear( 1 );
        g_Texture.Clear( 2 );
        g_Texture.Clear( 3 );

        g_pShaderMgr->SetPixelShader( kCAST_SHADOW );
        f32 Colour[4]={ 0.0f,0.0f,0.0f,0.85f };
        LightenShadows( Colour );
    }
    g_pd3dDevice->KickPushBuffer();
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::BeginShadowReceiveRigid( geom* pGeom,s32 iSubMesh )
{
    if( m_bPipActive )
        return;

    if( SWITCH_USE_SHADOWS )
    {
        g_pPipeline ->BeginRigid( pGeom,iSubMesh );
        g_pShaderMgr->Begin();

        m_bDirtyShadows = true;
        m_pLightData    = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::RenderToShadows( render_instance& Inst,s32 iProj )
{
    if( m_bPipActive )
        return;

    m_pInst = &Inst;

    ASSERT( iProj < render::MAX_SHADOW_CASTERS );

    if( SWITCH_USE_SHADOWS )
    {
        if( iProj/4 >= MAX_SHADOWBUFFER )
            return;



        // Calculate the L2C transform ////////////////////////////////////////

        vs::desc VSFlags;
        ps::desc PSFlags;

        const view* pView = eng_GetView();
        matrix4 L2C( pView->GetW2C( ));
        L2C *= Inst.Data.Rigid.pL2W[0];



        // PASS ONE: Z-PRIME SHADOWS //////////////////////////////////////////

        #ifndef CONFIG_RETAIL
        D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"PASS ONE: Z-PRIME SHADOWS" );
        D3DPERF_EndEvent();
        #endif
        {
            g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
            g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALPHA );
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
            g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );
            g_RenderState.Set( D3DRS_SPECULARENABLE,FALSE );
            g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CW );
            g_RenderState.Set( D3DRS_ZFUNC,D3DCMP_LESS );
            g_RenderState.Set( D3DRS_ZWRITEENABLE,TRUE );
            g_RenderState.Set( D3DRS_ZENABLE,TRUE );

            g_Texture.Clear( 0 );

            VSFlags.clear();
            VSFlags.oPos_Rigid = true;

            PSFlags.clear();
            PSFlags.bShadow = true;

            f32 Shade[4]={ 0.0f,0.0f,0.0f,1.0f };
            g_pShaderMgr->Link( 1,&L2C,VSFlags,NULL,PSFlags,true );

            g_pd3dDevice->SetPixelShaderConstant( 0,Shade,1 );

            RenderInstance();
        }




        // PASS TWO: DRAW SHADOWS /////////////////////////////////////////////

        #ifndef CONFIG_RETAIL
        D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"PASS TWO: DRAW SHADOWS" );
        D3DPERF_EndEvent();
        #endif
        {
            g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_SRCALPHA );
            g_RenderState.Set( D3DRS_ZFUNC,D3DCMP_LESSEQUAL );
            g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_ZERO );
            g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
            g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CW );
            g_RenderState.Set( D3DRS_SPECULARENABLE,TRUE );
            g_RenderState.Set( D3DRS_ZWRITEENABLE,TRUE );
            g_RenderState.Set( D3DRS_ZENABLE,TRUE );

            g_TextureStageState.Set( 0, D3DTSS_MAGFILTER,D3DTEXF_GAUSSIANCUBIC );
            g_TextureStageState.Set( 0, D3DTSS_MAGFILTER,D3DTEXF_GAUSSIANCUBIC );
            g_TextureStageState.Set( 0, D3DTSS_MINFILTER,D3DTEXF_GAUSSIANCUBIC );
            g_TextureStageState.Set( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
            g_TextureStageState.Set( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
            g_TextureStageState.Set( 0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP );

            // Calculate matrices *********************************************

            matrix4 Mat_L2W( Inst.Data.Rigid.pL2W[ 0 ] );
            matrix4 Mat_L2C( m_ShadowW2C[iProj]*Mat_L2W );

            Mat_L2C.Scale    ( vector3(0.5f,-0.5f,1.0f ));
            Mat_L2C.Translate( vector3(0.5f, 0.5f,0.0f ));

            Mat_L2W.ClearTranslation();
            matrix4 Mat_L2V( m_ShadowW2V[iProj]*Mat_L2W );

            // Setup initial shader states ************************************

            VSFlags.clear();
            VSFlags.oPos_Rigid = true;
            VSFlags.oT0_ShadowCreate = true;

            PSFlags.clear();
            PSFlags.oT0Shadow = true;
            PSFlags.xfc_Std = true;

            // Shadow source stage ********************************************

            D3DVIEWPORT8 Vp;
            CalculateViewport( SHADOW_BEGIN+iProj/4,Vp );
            g_Texture.Set( 0, m_pTexture[SHADOW_BEGIN+iProj/4] );

            // link shaders and draw ******************************************

            g_pShaderMgr->Link( 1,&L2C,VSFlags,NULL,PSFlags,true );
            f32 Channel[4]=
            {
                f32((iProj%4)==0),
                f32((iProj%4)==1),
                f32((iProj%4)==2),
                f32((iProj%4)==3)
            };
            f32 Cst[8]=
            {
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                f32(Vp.X),
                f32(Vp.Y),
                f32(Vp.Width),
                f32(Vp.Height),
            };
            g_pd3dDevice->SetVertexShaderConstant( -27,&Cst,2 );
            g_pd3dDevice->SetVertexShaderConstant( -31,&Mat_L2C,4 );
            g_pd3dDevice->SetVertexShaderConstant(  80,&Mat_L2V,4 );
            g_pd3dDevice->SetPixelShaderConstant( 0,Channel,1 );
            m_bNoVertexColours = TRUE;

            RenderInstance();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SkinCastShadow( render_instance& Inst,s32 iProj )
{
    if( m_bPipActive )
        return;

    m_pInst = &Inst;

    ASSERT( iProj < render::MAX_SHADOW_CASTERS );

    if( SWITCH_USE_SHADOWS )
    {
        ASSERT( iProj/4 < MAX_SHADOWBUFFER );

        // Shadow colour
        f32 Shade[4]=
        {
            0.0f,0.0f,0.0f,0.0f,
        };

        // set render target
        static const u32 TargetChannel[4]=
        {
            D3DCOLORWRITEENABLE_RED,
            D3DCOLORWRITEENABLE_GREEN,
            D3DCOLORWRITEENABLE_BLUE,
            D3DCOLORWRITEENABLE_ALPHA
        };

        g_pd3dDevice->SetVertexShaderConstant( -31,&m_ShadowW2C[iProj],4 );
        SetRenderTarget( iProj/4+SHADOW_BEGIN,-1 );

        // setup blend equation
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
        g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );

        u32 SrcBlend;
        u32 DstBlend;

        if( iProj%4<3 )
        {
            DstBlend = D3DBLEND_SRCCOLOR;
            SrcBlend = D3DBLEND_ZERO;
        }
        else
        {
            DstBlend = D3DBLEND_SRCALPHA;
            SrcBlend = D3DBLEND_ZERO;
        }
        g_RenderState.Set( D3DRS_DESTBLEND,DstBlend );
        g_RenderState.Set( D3DRS_SRCBLEND ,SrcBlend );
        g_RenderState.Set(
            D3DRS_COLORWRITEENABLE,
            TargetChannel[ iProj%4 ]);

        // link second set of shaders
        vs::desc VSFlags;
        {
            VSFlags.clear();
            VSFlags.oSt_CastShadowSkin = true;
        }

        ps::desc PSFlags;
        {
            PSFlags.clear();
            PSFlags.bShadow = true;
        }

        g_pShaderMgr->Link( VSFlags,PSFlags,true );
        g_pd3dDevice->SetPixelShaderConstant( 0,Shade,1 );
        m_bNoVertexColours = TRUE;

        RenderInstance();
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::StartShadowReceive( void )
{
    if( m_bPipActive )
        return;

    SetRenderTarget( kPRIMARY,kPRIMARY_Z );
    g_pd3dDevice->Clear(
        0,
        0,
        D3DCLEAR_TARGET_A|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,
        0xFFFFFFFF,
        1.0f,
        0
    );
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::EndShadowReceive( void )
{
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::BeginProfiling( void )
{
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::EndProfiling( void )
{
}

////////////////////////////////////////////////////////////////////////////////

vector4* pipeline_mgr::SetupLighting( vs::desc& VSFlags,ps::desc& PSFlags )
{
    // RIGID LIGHTING /////////////////////////////////////////////////////////

    if( m_bUseRigidGeom )
    {
        // Add filter lighting ************************************************

        bool bIsPerPixel = !!(m_pInst->Flags & render::PERPIXEL_POINTLIGHT);
        if( !bIsPerPixel )
            AddFilterLight( PSFlags );

        // Calculate per vertex contribution **********************************

        VSFlags.oD0 = 0;
        if( !bIsPerPixel ) // do regular vertex lighting
        {
            // Setup vertex lighting ..........................................

            lights * pLights = ( lights* )m_pInst->pLighting;
            if( pLights && !s_bForcedToGlow )
            {
                g_pShaderMgr->SetPointLights( pLights,m_L2W );
                VSFlags.oD0_PointLight = true;
            }

            // Add baked lighting .............................................

            rigid_data& Data = m_pInst->Data.Rigid;
            if( Data.pColInfo )
            {
                // only use vertex colours when building the light map
                m_pLightData = (void*)Data.pColInfo;
                VSFlags.oD0_Diffuse = true;
            }
            else
            {
                // white = 25% luminance with this flag
                g_pShaderMgr->SetWhiteConst( 0.25f );
                VSFlags.oD0_WhiteLight = true;
                m_pLightData = NULL;
            }

            // Handle shadowing blend .........................................

            if( !(m_pInst->Flags & render::INSTFLAG_SHADOW_PASS) || m_bAlphaBlend )
            {
                g_RenderState.Set( D3DRS_ALPHABLENDENABLE,m_bAlphaBlend );

                if( m_MaterialFlags & geom::material::FLAG_IS_ADDITIVE )
                {
                    g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
                    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ONE );
                    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
                }
                else if( m_MaterialFlags & geom::material::FLAG_IS_SUBTRACTIVE )
                {
                    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_REVSUBTRACT );
                    g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
                    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ONE );
                }
                else
                {
                    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
                    g_RenderState.Set( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
                    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
                    
                }
                g_RenderState.Set( D3DRS_ALPHABLENDENABLE,m_bAlphaBlend );
            }
            else
            {
            #ifndef CONFIG_RETAIL
            D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"IN SHADOW (VERTEX LIT)" );
            D3DPERF_EndEvent();
            #endif
                g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR );
                g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ONE );
                g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
                g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
            }
        }

        // Calculate per pixel contribution ***********************************

        else
        {
        #ifndef CONFIG_RETAIL
        D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"PER PIXEL LIGHTING" );
        D3DPERF_EndEvent();
        #endif

            ASSERT( !m_bAlphaBlend );
            if( m_bIsSelfIllum )
            {
                // Self illum textures need to blend with glow channel
                g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR );
                g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
                g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
                g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
            }
            else
            {
            #ifndef CONFIG_RETAIL
            D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"PPL SHADOWS" );
            D3DPERF_EndEvent();
            #endif

                // All other light mapped materials multiply against LM
                g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR );
                g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_ZERO );
                g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
                g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
            }

            // white used for multiplying against lightmap
            g_pShaderMgr->SetWhiteConst(1.0f);
            VSFlags.oD0_WhiteLight = true;
            m_pLightData = NULL;
        }
        PSFlags.xfc_Std = true;
        return NULL;
    }

    // SKINNED LIGHTING ///////////////////////////////////////////////////////

    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,m_bAlphaBlend );
    PSFlags.xfc_Std = true;

    // We don't have to worry about per pixel lighting with skinned geoms
    // because they are purely point lit. (Skins use directional)

    lights* pLights = ( lights* )m_pInst->pLighting;
    if( pLights )
    {
        static vector4 Ambient;
        Ambient = pLights->Ambient;
        Ambient.GetW()=1.0f;

        if( m_pInst->Flags & render::GLOWING )
        {
            g_pShaderMgr->SetWhiteConst(1.0f);
            VSFlags.oD0_WhiteLight = true;
        }
        else
        {
            g_pShaderMgr->SetDirLights( pLights );
            VSFlags.oD0_DirectLight = true;
        }
        AddFilterLight( PSFlags );
        return &Ambient;
    }

    // UNLIT SKINNED //////////////////////////////////////////////////////////

    VSFlags.oD0_BlackLight = true;
    AddFilterLight( PSFlags );
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::AddDetailMap( vs::desc& VSFlags,ps::desc& PSFlags )
{
    if( m_bPipActive )
        return;
    if( SWITCH_USE_DETAIL_MAPS )
    {
        if( m_pDetailMap && ( m_pInst->Flags & render::INSTFLAG_DETAIL ))
        {
        #ifndef CONFIG_RETAIL
        D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"DETAIL MAP" );
        D3DPERF_EndEvent();
        #endif

            static f32 fBias = -0.5f;

            g_TextureStageState.Set( 2,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU );
            g_TextureStageState.Set( 2,D3DTSS_MIPMAPLODBIAS,*((LPDWORD)(&fBias)));
            g_TextureStageState.Set( 2,D3DTSS_MAGFILTER,D3DTEXF_ANISOTROPIC );
            g_TextureStageState.Set( 2,D3DTSS_MINFILTER,D3DTEXF_ANISOTROPIC );
            g_TextureStageState.Set( 2,D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
            g_TextureStageState.Set( 2,D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
            g_TextureStageState.Set( 2,D3DTSS_ADDRESSW, D3DTADDRESS_WRAP );

            g_Texture.Set( 2,m_pDetailMap );

            VSFlags.bDetailMap = true;
            PSFlags.bDetailMap = true;
        }
        else
        {
            PSFlags.bDetailMap = false;
            VSFlags.bDetailMap = false;
            g_Texture.Clear( 2 );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool pipeline_mgr::AddFlashlight( vs::desc& VSFlags,ps::desc& PSFlags )
{
    if( m_bPipActive )
        return false;

    if( SWITCH_USE_FLASHLIGHT )
    {
        if( m_pProjMap && ( m_pInst->Flags & render::INSTFLAG_SPOTLIGHT ) )
        {
        #ifndef CONFIG_RETAIL
        D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"FLASHLIGHT" );
        D3DPERF_EndEvent();
        #endif

            g_Texture.Set( 3,m_pProjMap );
            matrix4   Mtx( m_ProjMatrix );

            g_TextureStageState.Set( 3, D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );
            g_TextureStageState.Set( 3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
            g_TextureStageState.Set( 3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
            g_pd3dDevice->SetVertexShaderConstant( -36,&  Mtx,4 );

            vector3 Pos;
            Pos = s_TextureProjectionView.GetV2W().GetTranslation();

            vector4* pVec=(vector4*)&Pos;
            pVec->GetW()=0.50f;

            g_pd3dDevice->SetVertexShaderConstant( -32,&Pos,1 );

            // Demote pixel shader to projector set
            VSFlags.oT3_Projection = true;
            if( PSFlags.LightMap )
            {
                PSFlags.LightMap = 0;
                PSFlags.bPerPixelLitProj = true;
            }
            else
            {
                PSFlags.bProj = true;
            }
            return true;
        }
    }

    g_Texture.Clear( 3 );
    VSFlags.oT3_Projection = false;
    PSFlags.bDetailMap = false;
    PSFlags.bProj = false;
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::InsertShadow( vs::desc& VSFlags,ps::desc& PSFlags )
{
    if( m_bPipActive )
        return;
    if( SWITCH_USE_SHADOWS )
    {
        if( m_pInst->Flags & render::INSTFLAG_SHADOW_PASS && m_bMatCanReceiveShadow )
        {
            m_iShadow = g_pShaderMgr->InsertShadow( PSFlags,m_pTexture[kINTERMEDIATE] );
            if( m_iShadow > 0 )
            {
                m_pInst->Flags &= ~render::INSTFLAG_SHADOW_PASS;
                VSFlags.iShadow = 1<<(m_iShadow-0); // TEX1 - TEX3( diffuse = TEX0 )
                PSFlags.iShadow = 1<<(m_iShadow-1); // TEX1 - TEX3
                return;
            }            
        }
        PSFlags.iShadow = 0;
        VSFlags.iShadow = 0;
        m_iShadow = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupShadowConsts( void )
{
    if( m_bPipActive )
        return;
    if( SWITCH_USE_SHADOWS )
    {
        if( m_iShadow )
        {
            f32 Cnst[12]=
            {
                0.5f,-0.5f,1.0f,1.0f,
                0.5f, 0.5f,1.0f,1.0f,
                m_OptW,
                m_OptH,
                0.0f,
                0.0f,
            };
            g_pd3dDevice->SetVertexShaderConstant( -26,Cnst,3 );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupCubeConsts( vs::desc& VSFlags )
{
    if( VSFlags.oT0_Cube )
    {
        const view* pView = eng_GetView();
        vector3 Pos=pView-> GetPosition();

        matrix4 L2R;
        L2R = m_L2W;
        L2R.Translate( -Pos );

        g_pd3dDevice->SetVertexShaderConstant( -5,&L2R,4 );
    }
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupTexConsts( ps::desc& PSFlags )
{
    vector4 Pos;

    Pos.GetX() = (f32)m_pInst->UOffset / 255.0f;
    Pos.GetY() = (f32)m_pInst->VOffset / 255.0f;

    if( SWITCH_USE_DETAIL_MAPS )
    {
        if( PSFlags.bDetailMap )
        {
            Pos.GetZ() = m_DetailScale;
            Pos.GetW() = m_DetailScale;
            goto sk;
        }
    }

    Pos.GetZ() = 1.0f;
    Pos.GetW() = 1.0f;

sk: g_pd3dDevice->SetVertexShaderConstant( -9,&Pos,1 );

    if( m_bUsingPIP )
    {
        Pos.GetX()=f32(m_PipW);
        Pos.GetY()=f32(m_PipH);
    }
    else
    {
        Pos.GetX()=1.0f;
        Pos.GetY()=1.0f;
    }

    g_pd3dDevice->SetVertexShaderConstant( -8,&Pos,1 );
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupDistortion( vs::desc& VSFlags,ps::desc& PSFlags )
{
    if( SWITCH_USE_DISTORTION )
    {
        VSFlags.clear();
        VSFlags.oPos = m_VSFlags.oPos;
        VSFlags.oT0_Distortion = true;
        PSFlags.MaterialID = false;
        PSFlags.xfc_Fog = false;
        PSFlags.xfc_Std = true;
    }
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupDistortionConsts( render_instance& Inst )
{
    if( SWITCH_USE_DISTORTION || SWITCH_USE_PROJ_SHADOWS )
    {
        f32 Alpha = Inst.Alpha;
        f32 Ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        if( m_pInst->Flags & render::GLOWING )
        {
            // Make sure that glowing objects glow.
            g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL );
            s_bForcedToGlow = TRUE;
        }
        else
        {
            g_RenderState.Set( D3DRS_COLORWRITEENABLE,
                D3DCOLORWRITEENABLE_GREEN |
                D3DCOLORWRITEENABLE_BLUE  |
                D3DCOLORWRITEENABLE_RED
            );
        }

        const f32 C7[4]= { 0.0f,0.0f,0.0f,1.0f };
              f32 C0[4]= { 0.0f,0.0f,Ambient[3],Alpha };
              f32 C2[8]= { 0.0f,
                           0.0f,
                           f32(m_PipW),
                           f32(m_PipH),
                           0.5f,
                           0.0f,
                           1.0f,
                           8.0f };

        view View( *eng_GetView());
        View.SetViewport( 0,0,m_PipW,m_PipH );

        matrix4 Rot( m_NormalRot );
        matrix4 W2V( View.GetW2V()*Rot );

        g_pd3dDevice->SetVertexShaderConstant( -5,&W2V,3 );
        g_pd3dDevice->SetVertexShaderConstant( -2,C2,2 );
        g_pd3dDevice->SetPixelShaderConstant( 6,Ambient,1 );
        g_pd3dDevice->SetPixelShaderConstant( 7,C7,1 );
        g_pd3dDevice->SetPixelShaderConstant( 0,C0,1 );
    }
}

////////////////////////////////////////////////////////////////////////////////

bool pipeline_mgr::FogEnable( vs::desc& VSFlags,ps::desc& PSFlags,bool bEnable )
{
    if( SWITCH_USE_FOG )
    {
        // If something is fading fog cannot be applied
        if( m_pInst->Flags & render::FADING_ALPHA )
            bEnable = false;
        g_RenderState.Set( D3DRS_SPECULARENABLE,bEnable );
        if( bEnable )
        {
        #ifndef CONFIG_RETAIL
        D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"FOGGING" );
        D3DPERF_EndEvent();
        #endif

            PSFlags.xfc_Std = (m_bAlphaBlend && !m_bForceZFill);
            VSFlags.oD1_Fog = !PSFlags.xfc_Std;

            if( m_bIsPunchthru )
                PSFlags.bFogPunchthru = true;
            else
                PSFlags.bApplyZFog = true;
            PSFlags.xfc_Std = true;
            return true;
        }
    }
    PSFlags.xfc_Std = true;
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupChannels( vs::desc& VSFlags,ps::desc& PSFlags )
{
    u32 Flags = D3DCOLORWRITEENABLE_ALL;
    m_bIsSelfIllum =
        VSFlags.oT0_Distortion
        ||  PSFlags.bDiffusePerPixelIllum
        ||  PSFlags.bAlphaPerPixelIllum
        ||  PSFlags.bAlphaPerPolyIllum
        ||  PSFlags.bForcedGlow;
    if( m_bIsSelfIllum )
    {
        if( m_bSelfIllumUseDiffuse )
        {
            PSFlags.bDiffusePerPixelIllum = false;
            PSFlags.bAlphaPerPixelIllum = false;
            PSFlags.bForcedGlow = false;
            PSFlags.bAlpha = true;
        }
        m_bDirtySelfIllum = true;
    }
    else // includes alpha_per_poly_illum *********************************
    {
        Flags &= ~D3DCOLORWRITEENABLE_ALPHA;
    }
    g_RenderState.Set(
        D3DRS_COLORWRITEENABLE,
        Flags
    );
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::AddFilterLight( ps::desc& PSFlags )
{
    if( m_pInst->Flags & render::INSTFLAG_FILTERLIGHT )
    {
        PSFlags.oV0_Tint = true;
        m_TintColour[0]  = f32(s_FilterLightColor.R)/128.0f;
        m_TintColour[1]  = f32(s_FilterLightColor.G)/128.0f;
        m_TintColour[2]  = f32(s_FilterLightColor.B)/128.0f;
        m_TintColour[3]  = 1.0f;
        return;
    }
    PSFlags.oV0_Tint = false;
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::RenderToZBuffer( render_instance& Inst )
{
    ps::desc PSFlags;
    vs::desc VSFlags;

    SetupL2W ( Inst,VSFlags,PSFlags );

    // Prime for PIP target ***************************************************
    //
    // When rendering to PIP there will be no fogging or special priming.

    if( m_bPipActive )
    {
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,0 );
        g_pShaderMgr->Link( VSFlags, PSFlags );
        RenderInstance();
        return;
    }

    // Calculate active channels **********************************************
    //
    // This code turns fogging for everything but punchthru.

    u32 ColorWriteEnable = 0;
    if( SWITCH_USE_FOG && FogEnable( VSFlags,PSFlags,TRUE ) )
        ColorWriteEnable |= D3DCOLORWRITEENABLE_BLUE;
    if( m_bIsPunchthru )
    {
        SetupTexConsts( PSFlags );
        VSFlags.oT0_Normal = true;
    }
    else
    {
        s32 NotReceivingShadow = !(Inst.Flags & render::INSTFLAG_SHADOW_PASS);
        if( NotReceivingShadow )
        {
            ColorWriteEnable |= D3DCOLORWRITEENABLE_ALPHA;
            if( SWITCH_USE_FOG )
            {
                PSFlags.bApplyZFog = false;
                PSFlags.bShadowZFog = true;
            }
            else
            {
                PSFlags.bShadow = true;
            }
        }
    }
    g_RenderState.Set( D3DRS_COLORWRITEENABLE,ColorWriteEnable );

    // Link shaders ***********************************************************

    g_pShaderMgr->Link( VSFlags, PSFlags );

    // Setup constants for shadows and fog ************************************

    if( PSFlags.bShadowZFog || PSFlags.bShadow )
    {
        f32 Shade[4]={ 0.0f,0.0f,0.0f,1.0f };
        g_pd3dDevice->SetPixelShaderConstant( 0,Shade,1 );
    }

    // Setup fog coefficients *************************************************

    if( SWITCH_USE_FOG )
    {
        f32 FogStartConst[4]={ s_FogStart,0.0f,0.0f,0.0f };
        g_pd3dDevice->SetVertexShaderConstant( -47,FogStartConst,1 );
        g_pd3dDevice->SetVertexShaderConstant(
            -42-1,
            g_pShaderMgr->m_FogConst[
            g_pShaderMgr->m_FogIndex],
            1
        );
    }

    // Render instanced geometry **********************************************

    RenderInstance();
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::RenderToLightMap( render_instance& Inst )
{
    BeginProfiling();

    if( SWITCH_PER_PIXEL_LIGHTING )
    {
        ps::desc PSFlags;
        vs::desc VSFlags;

        // Light mapping( already linked )*************************************

        SetupL2W( Inst,VSFlags,PSFlags );

        // Render lightmap geometry *******************************************

        ASSERT( m_bInLightMap );
        {
            bool bReceivesShadow = !!(m_pInst->Flags & ( render::INSTFLAG_SHADOW_PASS    |
                                                         render::INSTFLAG_PROJ_SHADOW_1  |
                                                         render::INSTFLAG_PROJ_SHADOW_2  ));

            lights* pLights = (lights*)m_pInst->pLighting;
            s32     nLights = 0;

            // Setup coloured lighting pass ///////////////////////////////////
            //
            // Now we know whether to ppl or not. If so setup all shader, pixel
            // and vertex, and render states. Just the basic ones.

            #ifndef CONFIG_RETAIL
            if( !g_bShowBackFaces )
            {
            #endif
                m_bPerPixelLit = (m_bUseRigidGeom && (pLights || bReceivesShadow) );
                // We can only ppl if lights are available
                if( pLights )
                    nLights = pLights->Count;
                if( !nLights )
                    // If there are no lights and shadows are being cast
                    // on this object we have to be per pixel lit.
                    m_bPerPixelLit = bReceivesShadow;
            #ifndef CONFIG_RETAIL
            }
            #endif

            // Setup coloured lighting pass ///////////////////////////////////

            if( m_bPerPixelLit )
            {
                // setup distance attenuation .................................

                s32 i = 0;
                for( ;i<nLights;i++ )
                {
                    g_TextureStageState.Set( i,D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );
                    g_TextureStageState.Set( i,D3DTSS_ADDRESSU ,D3DTADDRESS_CLAMP );
                    g_TextureStageState.Set( i,D3DTSS_ADDRESSV ,D3DTADDRESS_CLAMP );
                    g_TextureStageState.Set( i,D3DTSS_ADDRESSW ,D3DTADDRESS_CLAMP );
                    g_Texture          .Set( i,m_hAttenuationVolume );
                }
                for( ;i<4;i++ )
                    g_Texture.Clear( i );

                // Setup channels for writing
                g_RenderState.Set( D3DRS_SPECULARENABLE,TRUE );
                g_RenderState.Set( D3DRS_COLORWRITEENABLE,
                    D3DCOLORWRITEENABLE_GREEN |
                    D3DCOLORWRITEENABLE_BLUE  |
                    D3DCOLORWRITEENABLE_RED
                );

                // setup flashlight; mark as dirty ............................

                m_pInst->Flags |= render::PERPIXEL_POINTLIGHT;
                if( !m_bIsPunchthru )
                    AddFlashlight( VSFlags,PSFlags );
                m_bDirtyLightMap = true;

                // setup vertex colour stream .................................

                rigid_data& Data = m_pInst->Data.Rigid;
                if( Data.pColInfo )
                {
                    m_pLightData = (void*)Data.pColInfo;
                    VSFlags.oD0_WhiteLight = false;
                }
                else
                {
                    g_pShaderMgr->SetWhiteConst(0.25f);
                    VSFlags.oD0_WhiteLight = true;
                    m_pLightData = NULL;
                }
            }
            else
            {
                EndProfiling();
                return;
            }

            // fill in punchthrough test //////////////////////////////////////
            //
            // Punchthrough is a special case because we have to punch holes
            // wherever the alpha of the diffuse texture is zero.

            if( m_bIsPunchthru )
            {
                // just a flag, tex t3 does all the work
                g_TextureStageState.Set( 3, D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
                g_TextureStageState.Set( 3, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
                g_TextureStageState.Set( 3, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
                g_TextureStageState.Set( 3, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP );
                g_Texture          .Set( 3,m_LitPunchthru );

                PSFlags.oT3_Mask = true;
                VSFlags.oT3_Mask = true;

                SetupTexConsts( PSFlags );
            }
            else
            {
                if( bReceivesShadow )
                {
                    g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_DESTALPHA );
                    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ZERO );
                    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
                }
                else
                {
                    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
                }
            }

            // link shaders together //////////////////////////////////////////

            static const f32 Extra[4] = { 0.5f,0.0f,0.0f,1.0f };

            PSFlags.bForcedGlow = false;
            PSFlags.xfc_Std     = false;

            AddFilterLight( PSFlags );
            g_pShaderMgr->Link( VSFlags,PSFlags );
            g_pd3dDevice->SetVertexShaderConstant( -1,Extra,1 );
            if( m_pInst->Flags & render::INSTFLAG_FILTERLIGHT )
                g_pd3dDevice->SetPixelShaderConstant( 11,m_TintColour,1 );

            // setup constant registers ///////////////////////////////////////

            if( VSFlags.oT0_Normal )
                SetupTexConsts( PSFlags );
            if( m_bPerPixelLit )
                g_pShaderMgr->SetPerPixelPointLights( pLights,m_L2W );
            else
            if( g_pShaderMgr->m_PSFlags.Mask )
            {
                static const vector4 s_C0 ( 0.0f,0.0f,0.0f,0.0f );
                g_pd3dDevice->SetPixelShaderConstant( 0,&s_C0,1 );
            }

            // restore some states ////////////////////////////////////////////

            g_RenderState.Set( D3DRS_ALPHATESTENABLE ,FALSE );
            g_RenderState.Set( D3DRS_SPECULARENABLE, FALSE );
            g_RenderState.Set( D3DRS_ZWRITEENABLE,FALSE );

            RenderInstance();

            // Apply projected shadows ////////////////////////////////////////

            ApplyProjectedShadows();

            // Apply flashlight as second pass ////////////////////////////////

            if( m_bIsPunchthru )
            {
                SetupL2W( Inst,VSFlags,PSFlags );

                VSFlags.oT0_Normal = 1;
                PSFlags.LightMap   = 0;
                PSFlags.bAlpha     = 1;

                if( AddFlashlight( VSFlags,PSFlags ))
                {
                    g_TextureStageState.Set( 3, D3DTSS_ALPHAKILL,D3DTALPHAKILL_DISABLE );
                    g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
                    g_TextureStageState.Set( 3, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
                    g_TextureStageState.Set( 3, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
                    g_TextureStageState.Set( 3, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP );
                    g_TextureStageState.Set( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
                    g_TextureStageState.Set( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
                    g_TextureStageState.Set( 0, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP );

                    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ONE );
                    g_RenderState.Set( D3DRS_SRCBLEND, D3DBLEND_ONE );
                    g_RenderState.Set( D3DRS_ALPHABLENDENABLE, TRUE );

                    g_Texture.Set  ( 0,m_LitPunchthru );
                    g_Texture.Clear( 1 );
                    g_Texture.Clear( 2 );

                    g_pShaderMgr->Link( VSFlags,PSFlags );
                    SetupTexConsts( PSFlags );

                    RenderInstance();
                }
            }
        }
    }
    EndProfiling();
}

////////////////////////////////////////////////////////////////////////////////
//
// This routine sets per instance render states, constants, and links the
// shader descriptions into vertex and pixel shaders. It also limits what
// colour channels are unnecessary.
//
//  Re: distortion effect
//
//      Position after W2C = [XYZ = -1.0f to 1.0]
//      Scale up to make UVs [0 to 639, 0 to 479]
//      Add normals in viewspace
//      Multiply by scale (4?)
//
//  Fogging only occurs if you call FogEnable() at the end
//  Z Priming takes precedence over everything else
//
////////////////////////////////////////////////////////////////////////////////

bool pipeline_mgr::BeginPass( render_instance& Inst )
{
    ps::desc PSFlags = m_PSFlags;
    vs::desc VSFlags = m_VSFlags;

    SetupL2W( Inst,VSFlags,PSFlags );



    ///////////////////////////////////////////////////////////////////////////



    // Handle alpha fading ****************************************************

    if( SWITCH_USE_Z_PRIMING )
    {
        if( m_bZPrime )
        {
            if( m_bIsPunchthru )
                return false;

            g_pShaderMgr->Link( VSFlags,PSFlags );
            RenderInstance();
            return false;
        }
    }



    ///////////////////////////////////////////////////////////////////////////



    // Distortion effect ******************************************************

    if( SWITCH_USE_DISTORTION )
    {
        if( VSFlags.oT0_Distortion )
        {
            SetupDistortion   ( VSFlags,PSFlags );
            g_pShaderMgr->Link( VSFlags,PSFlags );
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
            SetupDistortionConsts( Inst );
            m_iDistortionCount++;
            RenderInstance();
            return false;
        }
    }



    ///////////////////////////////////////////////////////////////////////////



    // Mutant vision mode *****************************************************

    s32 bMutantGlows = false;
    if( SWITCH_USE_MUTANT_MODE )
    {
        bMutantGlows=!!( m_pInst->Flags & render::GLOWING );
        if( bMutantGlows && !m_bPipActive )
        {
            if( PSFlags.bAlphaPerPixelIllum )
            {   PSFlags.bAlphaPerPixelIllum = false;
                PSFlags.bAlpha = true;
            }
            PSFlags.bDetailMap  = false;
            PSFlags.bForcedGlow = true;
            s_bForcedToGlow     = TRUE;
        }
    }



    ///////////////////////////////////////////////////////////////////////////



    // normal material rendering **********************************************

    if( SWITCH_USE_NORMAL_MATERIALS )
    {
        vector4* pAmbient = NULL;

        // setup adaptive sampling ............................................

        const view* pView= eng_GetView();
        vector3 Pos=pView->GetPosition()-m_L2W.GetTranslation();

        // render special case lighting .......................................

        s32 bIsPerPixel = m_pInst->Flags & render::PERPIXEL_POINTLIGHT;
        s32 bSpecialCase = (m_bUseRigidGeom && bIsPerPixel && m_bIsSelfIllum);
        if( bSpecialCase )
        {
        #ifndef CONFIG_RETAIL
        D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"SELF ILLUM PPL" );
        D3DPERF_EndEvent();
        #endif

            g_RenderState.Push();
            {
                // Setup first pass: Dst.rgb * (1-Src.a)+(Src.rgb*zero)
                g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
                g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_ZERO );
                g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
                g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );

                // The lighting sets up for seconds pass
                g_pShaderMgr->SetWhiteConst(1.0f);

                vs::desc VS = VSFlags;
                ps::desc PS = PSFlags;

                VS.oD0_WhiteLight = true;
                PS.xfc_Std = true;

                // Now link the shaders together
                g_pShaderMgr->Link( 0,NULL,VS,NULL,PS );
                SetupTexConsts( PSFlags );

                // Render the first pass
                RenderInstance();
            }
            g_RenderState.Pop();
        }

        // Render general case lighting .......................................

        pAmbient = SetupLighting( VSFlags,PSFlags );

        // insert effects .....................................................

        s32 bOnFlashlight = false;
        if( m_pInst->Flags & render::PERPIXEL_POINTLIGHT )
            bOnFlashlight = m_bIsPunchthru;
        else
            bOnFlashlight = true;
        if( bOnFlashlight )
            AddFlashlight( VSFlags,PSFlags );
        AddDetailMap( VSFlags,PSFlags );

        // stitch shaders together ............................................

        g_pShaderMgr->Link( 0,NULL,VSFlags,pAmbient,PSFlags );

        // setup mutant glows .................................................

        if( bMutantGlows )
            g_pd3dDevice->SetPixelShaderConstant( 6,pAmbient,1 );
        if( m_pInst->Flags & render::INSTFLAG_FILTERLIGHT )
            g_pd3dDevice->SetPixelShaderConstant( 11,m_TintColour,1 );

        // setup constant registers ...........................................

        SetupCubeConsts( VSFlags );
        SetupTexConsts ( PSFlags );
        SetupChannels  ( VSFlags,PSFlags );

        // Ensure proper alpha blending .......................................

        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,!!(m_bAlphaBlend || bIsPerPixel) );
        if( m_bIsSelfIllum )
            g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
    }

    // Fade objects by m_AlphaFade ********************************************

    if( m_pInst->Flags & render::FADING_ALPHA )
    {
        if( m_bIsPunchthru || m_bAlphaBlend )
        {
            f32 FadeReg[4] = {0.0f,0.0f,0.0f,f32(m_AlphaFade)/255.0f };
            g_pd3dDevice->SetPixelShaderConstant( 5,FadeReg,1 );

            g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,D3DTALPHAKILL_ENABLE );
            g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
            g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
        }
        else
        {
            const f32 FadeReg[4] = {0.0f,0.0f,0.0f,1.0f };
            g_pd3dDevice->SetPixelShaderConstant( 5,FadeReg,1 );

            g_RenderState.Set( D3DRS_BLENDCOLOR, D3DCOLOR_RGBA( 0,0,0,m_AlphaFade ));
            g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVCONSTANTALPHA );
            g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_CONSTANTALPHA );
        }
        g_RenderState.Set( D3DRS_ZFUNC,D3DCMP_LESSEQUAL );
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
        g_RenderState.Set( D3DRS_SPECULARENABLE,FALSE );
        g_RenderState.Set( D3DRS_ZWRITEENABLE,FALSE );
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_RED   |
            D3DCOLORWRITEENABLE_GREEN |
            D3DCOLORWRITEENABLE_BLUE
        );
    }
    else
    {
        g_RenderState.Set( D3DRS_ZWRITEENABLE,m_bForceZFill );
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetProjectiveTexture( const texture::handle& Texture )
{
    texture* pTexture = Texture.GetPointer();
    if( pTexture )
    {
        m_pProjMap = vram_GetSurface( pTexture->m_Bitmap );
    }
    else
    {
        m_pProjMap = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::BeginDistortion( void )
{
    m_iDistortionCount = 0;
    m_bInDistortion    = true;
    m_pLightData       = NULL;
    m_iShadow          = 0;

    if( !m_bPipActive )
    {
        // 2d transform *******************************************************

        matrix4 I; I.Identity();
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE0,( const D3DMATRIX* )&I );
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE1,( const D3DMATRIX* )&I );
        g_pd3dDevice->SetTransform( D3DTS_WORLD   ,( const D3DMATRIX* )&I );
        g_pd3dDevice->SetTransform( D3DTS_VIEW    ,( const D3DMATRIX* )&I );

        D3DXMATRIX Proj;
        D3DXMatrixPerspectiveFovLH( &Proj, D3DX_PI/3, 4.0f/3.0f, 1.0f, 100.0f );
        g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &Proj );

        // Setup texture stages ***********************************************

        g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
        g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

        // Distortion render states setup *************************************

        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
        g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );
        g_RenderState.Set( D3DRS_SPECULARENABLE,FALSE );
        g_RenderState.Set( D3DRS_ZWRITEENABLE,TRUE );
        g_RenderState.Set( D3DRS_LIGHTING,FALSE );
        g_RenderState.Set( D3DRS_ZENABLE,TRUE );
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_RED   |
            D3DCOLORWRITEENABLE_GREEN |
            D3DCOLORWRITEENABLE_BLUE  );

        // Copy primary to scaled buffer **************************************

        g_RenderState.Push();
        {
            // Setup render states ............................................

            g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL );
            g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
            g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CCW );

            // Set render target and blit .....................................

            ResizePipTexture( -1,-1 );
            g_RenderTarget.Set( kPIP,-1 );

            D3DVIEWPORT8 Vs;
            CalculateViewport( kPRIMARY,Vs );
            g_Texture.Set( 0,m_pTexture[kPRIMARY] );
            Blt( 0.0f,
                 0.0f,
                 f32(m_PipW),
                 f32(m_PipH),
                 f32(Vs.X),
                 f32(Vs.Y),
                 f32(Vs.Width),
                 f32(Vs.Height)
            );
        }
        g_RenderState.Pop();

        // Set source and destination buffers *********************************

        SetRenderTarget ( kPRIMARY, kPRIMARY_Z );
        g_Texture.Set( 0,m_pTexture[kPIP] );
    }
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::EndDistortion( void )
{
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetProjShadowStage( s32 Index,s32 Stage )
{
    if( SWITCH_USE_PROJ_SHADOWS )
    {
        texture* pTexture = s_ShadowProjections[Index].Texture.GetPointer();
        ASSERT ( pTexture );

        g_TextureStageState.Set( Stage,D3DTSS_ADDRESSU,D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( Stage,D3DTSS_ADDRESSV,D3DTADDRESS_CLAMP );

        xbitmap& Texture = pTexture->m_Bitmap;
        vram_Activate(
            Stage,
            Texture.GetVRAMID()
        );
    }
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetProjShadowConsts( s32 Index,s32 BaseRegister )
{
    if( SWITCH_USE_PROJ_SHADOWS )
    {
        matrix4 Mtx( s_ShadowProjectionMatrices[Index] );
        if( ! BaseRegister )
              BaseRegister = -32;
        else
              BaseRegister = -36;

        g_TextureStageState.Set( 3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( 3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
        g_pd3dDevice->SetVertexShaderConstant( BaseRegister,&Mtx,4 );
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//  Return false if building light map, true otherwise.

void pipeline_mgr::ApplyProjectedShadows( void )
{
    if( SWITCH_USE_PROJ_SHADOWS && !m_bIsPunchthru )
    {
    IDirect3DBaseTexture8* pTex0;
    IDirect3DBaseTexture8* pTex1;

    u32 AlphaBlendEnable;
    u32 SpecularEnable;
    u32 ZWriteEnable;
    u32 DestBlend;
    u32 SrcBlend;
    u32 BlendOp;
    u32 AddrU0;
    u32 AddrU1;
    u32 AddrV0;
    u32 AddrV1;

        // determine shadow viability *****************************************

        u32 bInShadow = m_pInst->Flags & (render::INSTFLAG_PROJ_SHADOW_1|render::INSTFLAG_PROJ_SHADOW_2);

        // multiply projective texture with lightmap **************************

        if( bInShadow )
        {
            // Gather up render states ........................................

            AlphaBlendEnable = g_RenderState.Get( D3DRS_ALPHABLENDENABLE );
            SpecularEnable   = g_RenderState.Get( D3DRS_SPECULARENABLE   );
            ZWriteEnable     = g_RenderState.Get( D3DRS_ZWRITEENABLE     );
            DestBlend        = g_RenderState.Get( D3DRS_DESTBLEND        );
            SrcBlend         = g_RenderState.Get( D3DRS_SRCBLEND         );
            BlendOp          = g_RenderState.Get( D3DRS_BLENDOP          );

            AddrU0           = g_TextureStageState.Get( 0,D3DTSS_ADDRESSU );
            AddrU1           = g_TextureStageState.Get( 1,D3DTSS_ADDRESSU );
            AddrV0           = g_TextureStageState.Get( 0,D3DTSS_ADDRESSV );
            AddrV1           = g_TextureStageState.Get( 1,D3DTSS_ADDRESSV );

            pTex0            = g_Texture.Get(0);
            pTex1            = g_Texture.Get(1);

            // Set up render states ...........................................

            g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR );
            g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR );
            g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
            g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
            g_RenderState.Set( D3DRS_SPECULARENABLE,FALSE );
            g_RenderState.Set( D3DRS_ZWRITEENABLE,FALSE );
            g_RenderState.Set( D3DRS_COLORWRITEENABLE,
                D3DCOLORWRITEENABLE_RED   |
                D3DCOLORWRITEENABLE_GREEN |
                D3DCOLORWRITEENABLE_BLUE  );

            // Set up projected shadow stuff ..................................
            {
                ps::desc PSFlags;
                vs::desc VSFlags;

                VSFlags.clear();
                PSFlags.clear(),PSFlags.xfc_Std = true;

                if( m_bUseRigidGeom )
                    VSFlags.oPos_Rigid = true;
                else
                    VSFlags.oPos_Skin = true;

                g_Texture.Clear( 0 );
                g_Texture.Clear( 1 );

                if(( m_pInst->Flags & render::INSTFLAG_PROJ_SHADOW_1 ) &&
                   ( m_pInst->Flags & render::INSTFLAG_PROJ_SHADOW_2 ) )
                {
                    SetProjShadowStage(0,0);
                    SetProjShadowStage(1,1);
                    {
                        VSFlags.oT2_ShadowProject = true;
                        PSFlags.oT2LitProj        = true;

                        g_pShaderMgr->Link( VSFlags,PSFlags );
                    }
                    SetProjShadowConsts(0,0);
                    SetProjShadowConsts(1,1);
                }
                else if( m_pInst->Flags & render::INSTFLAG_PROJ_SHADOW_1 )
                {
                    SetProjShadowStage(0,0);
                    {
                        VSFlags.oT1_ShadowProject = true;
                        PSFlags.oT1LitProj        = true;

                        g_pShaderMgr->Link( VSFlags,PSFlags );
                    }
                    SetProjShadowConsts(0,0);
                }
                else if( m_pInst->Flags & render::INSTFLAG_PROJ_SHADOW_2 )
                {
                    SetProjShadowStage(1,0);
                    {
                        VSFlags.oT1_ShadowProject = true;
                        PSFlags.oT1LitProj        = true;

                        g_pShaderMgr->Link( VSFlags,PSFlags );
                    }
                    SetProjShadowConsts(1,0);
                }
                SetupDistortionConsts( *m_pInst );
            }
            RenderInstance();

            // restore previous render states .................................

            g_RenderState.Set( D3DRS_ALPHABLENDENABLE,AlphaBlendEnable );
            g_RenderState.Set( D3DRS_SPECULARENABLE,SpecularEnable );
            g_RenderState.Set( D3DRS_ZWRITEENABLE,ZWriteEnable );
            g_RenderState.Set( D3DRS_DESTBLEND,DestBlend );
            g_RenderState.Set( D3DRS_SRCBLEND,SrcBlend );
            g_RenderState.Set( D3DRS_BLENDOP,BlendOp );

            g_TextureStageState.Set( 0,D3DTSS_ADDRESSU,AddrU0 );
            g_TextureStageState.Set( 1,D3DTSS_ADDRESSU,AddrU1 );
            g_TextureStageState.Set( 0,D3DTSS_ADDRESSV,AddrV0 );
            g_TextureStageState.Set( 1,D3DTSS_ADDRESSV,AddrV1 );

            g_Texture.Set( 0,pTex0 );
            g_Texture.Set( 1,pTex1 );

            // We could potentially draw twice ................................

            m_pInst->Flags &= ~(
                render::INSTFLAG_PROJ_SHADOW_1
            |   render::INSTFLAG_PROJ_SHADOW_2 );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupL2W( render_instance& Inst,vs::desc& VSFlags,ps::desc& PSFlags )
{
    m_bNoVertexColours = FALSE;
    m_AlphaFade        = Inst.Alpha;
    m_pInst            = &Inst;

    // Save off instance distance *********************************************

    m_IDistance = xbox_CalcDistance( *m_pInst->Data.Rigid.pL2W, s_CurrentBBox );

    // Upload world to clip transform *****************************************

    bool bResult = true;
    {
        const view* pView = eng_GetView();
        matrix4 L2C( pView->GetW2C());

        VSFlags = m_VSFlags;
        PSFlags = m_PSFlags;

        if( !m_bUseRigidGeom )
            VSFlags.oPos_Skin  = true;
        else
        {
            m_L2W = m_pInst->Data.Rigid.pL2W[0];
            VSFlags.oPos_Rigid = true;
            L2C *= m_L2W;
        }
        g_pd3dDevice->SetVertexShaderConstant( 92,&L2C,4 );
    }
}

////////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::RenderToPrimary( render_instance& Inst )
{
    BeginProfiling();
    {
        if( BeginPass( Inst ))
        {
            // Render normally to offscreen buffer ............................

            RenderInstance();

            // Next stage: render skinned projective shadows ..................

            if( !m_bUseRigidGeom )ApplyProjectedShadows();

            // Next stage: alpha per poly illum ...............................

            switch( m_MaterialType )
            {
                case Material_Alpha_PerPolyIllum:
                {
                #ifndef CONFIG_RETAIL
                D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"SECOND PASS: MATERIAL_ALPHA_PERPOLYILLUM" );
                D3DPERF_EndEvent();
                #endif

                    g_RenderState.Set( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA );
                    g_RenderState.Set( D3DRS_ALPHABLENDENABLE, FALSE );
                    {
                        g_pShaderMgr->SetPixelShader( kDIFFUSE );
                        f32 Const[4]=
                        {
                            0.0f,
                            0.0f,
                            0.0f,
                            m_FixedAlpha
                        };
                        g_pd3dDevice->SetPixelShaderConstant( 0,Const,1 );
                        RenderInstance();
                    }
                    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
                    g_RenderState.Set( D3DRS_ALPHABLENDENABLE, TRUE );
                    break;
                }

                // Second pass: Env. blend is done with a fixed alpha for the entire
                // poly using the same blend method as the first pass.

                case Material_Alpha_PerPolyEnv:
                {
                #ifndef CONFIG_RETAIL
                D3DPERF_BeginEvent( D3DCOLOR_RGBA( 83,65,2,255 ),"SECOND PASS: MATERIAL_ALPHA_PERPOLYENV" );
                D3DPERF_EndEvent();
                #endif

                #if 0 // Fix when I get time
                    g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );

                    vs::desc OldVS = m_VSFlags;
                    ps::desc OldPS = m_PSFlags;

                    SetupEnvMap( m_MaterialFlags,m_pEnvironmentMap,NULL );
                    m_PSFlags.bAlphaPerPolyEnv = true;
                    m_VSFlags.oT0_Normal = false;

                    g_pShaderMgr->Link( 0,0,m_VSFlags,0,m_PSFlags );
                    RenderInstance();

                    m_VSFlags = OldVS;
                    m_PSFlags = OldPS;
                #endif
                    break;
                }
            }
        }
    }
    EndProfiling();



    #ifdef X_DEBUG
    g_pShaderMgr->ConstSanityCheck();
    #endif
}










///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetPipTexture( void )
{
    g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , D3DTADDRESS_CLAMP );
    g_Texture.Set( 0,m_pTexture[kPIP] );
}



///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::CreateAliasedTarget( s32 TargetID,s32 SourceID,s32 W,s32 H )
{
    IDirect3DSurface8* Surface;
    if( SourceID > -1 )
        Surface = m_pTarget[SourceID];
    else
        Surface = m_pBkSurface;

    D3DFORMAT Format;
    if( TargetID==kFOG )
        Format = D3DFMT_LIN_R8G8B8A8;
    else
        Format = D3DFMT_LIN_A8R8G8B8;

    D3DLOCKED_RECT Lr;
    Surface->LockRect( &Lr,NULL,D3DLOCK_TILED );
    m_pTexture[TargetID] = g_TextureFactory.Alias(
        Lr.Pitch,
        W,
        H,
        Format,
        Lr.pBits,
        texture_factory::ALIAS_FROM_SCRATCH );
    Surface->UnlockRect();

    m_pTexture[TargetID]->GetSurfaceLevel( 0,&m_pTarget[ TargetID ] );
    g_pd3dDevice->GetTile( 0,m_pTexture[TargetID]->GetTilePointer() );
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetAliasedTarget( s32 TargetID,s32 SourceID,s32 W,s32 H )
{
    CreateAliasedTarget( TargetID,SourceID,W,H );
    SetRenderTarget( TargetID,kPRIMARY_Z );
}



///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::ClearShadowBuffers( u32 nShadowProjectors )
{
    if( SWITCH_USE_SHADOWS )
    {
        if( !nShadowProjectors )
            return;
        u32 i,n = nShadowProjectors/4 ;
        if( nShadowProjectors%4 )
            n++;
        for( i=0;i<n;i++ )
        {
            g_RenderTarget.Set( i+SHADOW_BEGIN,-1 );
            g_pd3dDevice->Clear(
                0,
                0,
                D3DCLEAR_TARGET,
                D3DCOLOR_RGBA( 255,255,255,255 ),
                0,
                0
            );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::AddDirShadowProjector( const matrix4& L2W,f32 W,f32 H,f32 NearZ,f32 FarZ )
{
    if( SWITCH_USE_SHADOWS )
    {
        if( s_nDynamicShadows >= MAX_SHADOWBUFFER )
            return;

        // set the light direction

        // build the matrix that will map world coordinates into our projected texture
        matrix4 W2Proj = L2W;
        W2Proj.InvertRT();

        // Note that this is following the d3d standard for Z, which is [0,1] after
        // transform, but since we never clip, it shouldn't really matter for the ps2.
        matrix4 Proj2Clip;
        Proj2Clip.Identity();
        Proj2Clip(0,0) =  2.0f / W;
        Proj2Clip(1,1) = -2.0f / H;
        Proj2Clip(2,2) =  1.0f / (FarZ-NearZ);
        Proj2Clip(3,2) = NearZ / (FarZ-NearZ);

        // get final matrix for the shadow texture
        m_ShadowW2C[ s_nDynamicShadows ] = Proj2Clip * W2Proj;
        W2Proj.ClearTranslation();
        m_ShadowW2V[ s_nDynamicShadows ] = W2Proj;
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetZPrime( xbool bZPrime )
{
    if( bZPrime )
    {
        // optimise fill rate .................................................

        g_RenderState.Set( D3DRS_MULTISAMPLERENDERTARGETMODE,D3DMULTISAMPLEMODE_1X );
        g_RenderState.Set( D3DRS_MULTISAMPLEMODE,D3DMULTISAMPLEMODE_1X );
        g_RenderState.Set( D3DRS_MULTISAMPLEANTIALIAS,FALSE );

        // setup pure z priming states ........................................

        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ZERO );
        g_RenderState.Set( D3DRS_ZFUNC,D3DCMP_LESSEQUAL  );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE  );
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,FALSE  );
        g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_ONE   );
        g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CW     );
        g_RenderState.Set( D3DRS_ZWRITEENABLE,TRUE       );

        m_VSFlags.clear();
        m_PSFlags.clear();

        g_pShaderMgr->SetWhiteConst(1.0f);
        m_VSFlags.oD0_WhiteLight = true;
    }
    m_bZPrime = bZPrime;
}








///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::BlendEffects( shader_style Style )
{
    SwitchToBackBuffer();

    // calculate destination rect *********************************************

    rect Rect;
    const view& View = *eng_GetView();
    View.GetViewport( Rect );

    f32 Gx = f32(g_PhysW-640)*0.5f;
    f32 Ld = Rect.Min.X+Gx;
    f32 Td = Rect.Min.Y;
    f32 Rd = Rect.Max.X+Gx;
    f32 Bd = Rect.Max.Y;

    // calculate depth of field rect ******************************************

    s32 TargetID;
    f32 MipOffset = s_Post.MipOffset[s_Post.MipPaletteIndex];
    if( MipOffset )
        TargetID = kPIP;
    else
        TargetID = kSAMPLE1;

    D3DVIEWPORT8 Vp;
    CalculateViewport( TargetID,Vp );

    f32 L0 = f32(Vp.X);
    f32 T0 = f32(Vp.Y);
    f32 R0 = f32(Vp.X + Vp.Width);
    f32 B0 = f32(Vp.Y + Vp.Height);

    // calculate offscreen rect ***********************************************

    CalculateViewport( kPRIMARY,Vp );

    f32 L1 = f32(Vp.X);
    f32 T1 = f32(Vp.Y);
    f32 R1 = f32(Vp.X + Vp.Width);
    f32 B1 = f32(Vp.Y + Vp.Height);

    // calculate glow rect ****************************************************

    CalculateViewport( kGLOW_ACCUMULATOR,Vp );

    f32 L2 = f32(Vp.X);
    f32 T2 = f32(Vp.Y);
    f32 R2 = f32(Vp.X + Vp.Width);
    f32 B2 = f32(Vp.Y + Vp.Height);

    // calculate z-buffer rect ************************************************

    if( MipOffset )
        CalculateViewport( kSAMPLE2,Vp );
    else
        CalculateViewport( kPRIMARY_Z,Vp );

    f32 L3 = f32(Vp.X);
    f32 T3 = f32(Vp.Y);
    f32 R3 = f32(Vp.X + Vp.Width);
    f32 B3 = f32(Vp.Y + Vp.Height);

    // setup vertices *********************************************************

    const struct vertex
    {
        f32 x,y,z,w;
        f32 u0,v0;
        f32 u1,v1;
        f32 u2,v2;
        f32 u3,v3;
    }
    Quads[4] = 
    {
        { Ld,Td,0,0, L0,T0,L1,T1,L2,T2,L3,T3 },
        { Rd,Td,0,0, R0,T0,R1,T1,R2,T2,R3,T3 },
        { Rd,Bd,0,0, R0,B0,R1,B1,R2,B2,R3,B3 },
        { Ld,Bd,0,0, L0,B0,L1,B1,L2,B2,L3,B3 }
    };

    // setup final blend shader ***********************************************

    g_pShaderMgr->SetPixelShader( Style );
    {
        const f32 ZBlur = 0.50f; // far blur intensity
        const f32 Const[]=
        {
            /* C0 */

            0.5f,0.0f,0.0f,0.0f,

            /* C1 */

            0.0f,0.0f,(f32(s_Post.GlowCutoff)/255.0f)-0.5f,0.0f,

            /* C2 */

            0.2989f,
            0.5866f,
            0.1144f,
            19.25f / 255.0f,

            /* C3 */

            0.0f,
            0.0f,
            0.925f,
            1.115f,

            /* C4 */

            0.0f,0.0f,ZBlur,0.0f,

            /* C5 */

            0.0f,0.0f,0.0f,0.0f
        };
        g_pd3dDevice->SetPixelShaderConstant( 0,Const,6 );
    }

    // setup final render states **********************************************

    for( s32 i=0;i<4;i++ )
    {
        g_TextureStageState.Set( i,D3DTSS_MINFILTER,D3DTEXF_LINEAR );
        g_TextureStageState.Set( i,D3DTSS_MAGFILTER,D3DTEXF_LINEAR );
    }

    // The tint alpha is also used to darken the scene ........................

    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
    g_RenderState.Set(
        D3DRS_MULTISAMPLEMODE,
        D3DMULTISAMPLEMODE_1X
    );

    // blit quad to screen ****************************************************

    g_pShaderMgr->m_VSFlags.Mask = 0;

    g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX4 );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST,1,Quads,sizeof(vertex));

    // Clear edge strips ******************************************************

    if( g_PhysW == 720 )
    {
        D3DRECT Rect[2]=
        {
            { 0               ,0,LONG(Gx),g_PhysH },
            { g_PhysW-LONG(Gx),0,g_PhysW ,g_PhysH },
        };
        for( s32 i=0;i<2;i++ )
        {
            g_pd3dDevice->Clear(
                1,
                Rect+i,
                D3DCLEAR_TARGET_R |
                D3DCLEAR_TARGET_G |
                D3DCLEAR_TARGET_B ,
                0,
                0.0f,
                0
            );
        }
    }
}









///////////////////////////////////////////////////////////////////////////////

static void SetVert( rigid_geom::vertex_xbox& Vert,f32 dx,f32 dy,f32 sx,f32 sy )
{
    Vert.Pos.X = dx;
    Vert.Pos.Y = dy;
    Vert.Pos.Z = 0.0f;
    Vert.PackedNormal = 0;
    Vert.UV.X = sx;
    Vert.UV.Y = sy;
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::StoreFrameCopyInfo( s32 VRAMID,f32 U0,f32 V0,f32 U1,f32 V1 )
{
    m_CopyVRAMID = VRAMID;
    m_CopyU0     = U0;
    m_CopyV0     = V0;
    m_CopyU1     = U1;
    m_CopyV1     = V1;
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::CopyFrameTo( void )
{
    if( SWITCH_ENABLE_COPYFRAME )
    {
        s32 VRAMID = m_CopyVRAMID;
        f32 U0     = m_CopyU0;
        f32 V0     = m_CopyV0;
        f32 U1     = m_CopyU1;
        f32 V1     = m_CopyV1;

        // setup render states ***********************************************

        u32 RS_COLORWRITEENABLE = g_RenderState.Set( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL );
        u32 RS_CULLMODE         = g_RenderState.Set( D3DRS_CULLMODE        , D3DCULL_NONE );
        u32 RS_ALPHABLENDENABLE = g_RenderState.Set( D3DRS_ALPHABLENDENABLE, FALSE );
        u32 RS_SPECULARENABLE   = g_RenderState.Set( D3DRS_SPECULARENABLE  , FALSE );
        u32 RS_LIGHTING         = g_RenderState.Set( D3DRS_LIGHTING        , FALSE );
        u32 RS_ZENABLE          = g_RenderState.Set( D3DRS_ZENABLE         , FALSE );

        u32 TSS_ALPHAKILL = g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
        u32 TSS_ADDRESSU  = g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , D3DTADDRESS_CLAMP );
        u32 TSS_ADDRESSV  = g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , D3DTADDRESS_CLAMP );

        // 2d world to clip **************************************************

        f32 W2C[16]=
        {
            2.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 2.0f, 0.0f,
           -1.0f,-1.0f, 0.0f, 1.0f,
        };

        // setup blit ********************************************************

        g_pShaderMgr->Begin();
        {
            texture_factory::handle Texture = vram_GetSurface( VRAMID );
            ASSERT( Texture );

            IDirect3DSurface8* pSurface = NULL;
            Texture->GetSurfaceLevel( 0, &pSurface );
            if( pSurface )
            {
                // link shaders ...............................................

                vs::desc VSFlags;
                {
                    VSFlags.clear();
                    VSFlags.oPos_Rigid = true;
                    VSFlags.Blitter    = true;
                }
                ps::desc PSFlags;
                {
                    PSFlags.clear();
                    PSFlags.bBlt = true;
                }
                g_pShaderMgr->Link( 1,(matrix4*)W2C,VSFlags,0,PSFlags );

                // load constants .............................................

                f32 Const[4]={ 1.0f,m_OptH,1.0f,1.0f };
                g_pd3dDevice->SetPixelShaderConstant ( 0,Const,1 );
                g_pd3dDevice->SetVertexShaderConstant( 0,Const,1 );

                g_Texture.Set( 0, m_pTexture[kPRIMARY] );
                g_RenderTarget.Set( pSurface,NULL );

                // draw quad ..................................................

                D3DSURFACE_DESC Desc;
                pSurface->GetDesc( &Desc );
                {
                    const f32 dL = 0.0f;
                    const f32 dT = 0.0f;
                    const f32 dR = 1.0f;
                    const f32 dB = 1.0f;
                    const f32 sL = U0;
                    const f32 sT = V0;
                    const f32 sR = U1;
                    const f32 sB = V1;

                    rigid_geom::vertex_xbox Vert[4];
                    SetVert( Vert[0],dL,dT,sL,sT );
                    SetVert( Vert[1],dR,dT,sR,sT );
                    SetVert( Vert[2],dR,dB,sR,sB );
                    SetVert( Vert[3],dL,dB,sL,sB );

                    g_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST,1,Vert,sizeof( Vert[0] ) );
                }
                pSurface->Release();
            }            
        }
        g_pShaderMgr->End();

        // restore render states *********************************************

        g_RenderState.Set( D3DRS_COLORWRITEENABLE, RS_COLORWRITEENABLE );
        g_RenderState.Set( D3DRS_SPECULARENABLE  , RS_SPECULARENABLE   );
        g_RenderState.Set( D3DRS_LIGHTING        , RS_LIGHTING         );
        g_RenderState.Set( D3DRS_CULLMODE        , RS_CULLMODE         );
        g_RenderState.Set( D3DRS_ZENABLE         , RS_ZENABLE          );

        g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, TSS_ALPHAKILL );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , TSS_ADDRESSU  );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , TSS_ADDRESSV  );

        // Set the render target *********************************************

        SetRenderTarget( kPRIMARY,kPRIMARY_Z );
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupDepthOfField( void )
{
    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );

    for( s32 i=0;i<4;i++ )
    {
        g_TextureStageState.Set( i, D3DTSS_TEXCOORDINDEX, i                     );
        g_TextureStageState.Set( i, D3DTSS_ALPHAKILL    , D3DTALPHAKILL_DISABLE );
        g_TextureStageState.Set( i, D3DTSS_ADDRESSU     , D3DTADDRESS_CLAMP     );
        g_TextureStageState.Set( i, D3DTSS_ADDRESSV     , D3DTADDRESS_CLAMP     );
        g_TextureStageState.Set( i, D3DTSS_MINFILTER    , D3DTEXF_LINEAR        );
        g_TextureStageState.Set( i, D3DTSS_MAGFILTER    , D3DTEXF_LINEAR        );
    }

    // otherwise build depth of field *****************************************

    g_pShaderMgr->SetPixelShader( kDIFFUSE );

    // downsample frame *******************************************************

    D3DVIEWPORT8 Vs;
    CalculateViewport( kPRIMARY,Vs );

    g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_GAUSSIANCUBIC );
    g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_GAUSSIANCUBIC );

    D3DVIEWPORT8 Vd;
    g_Texture.Set( 0,m_pTexture[kPRIMARY] );
    SetRenderTarget( kSAMPLE0,-1,Vd );
    Blt( f32(Vd.X),
         f32(Vd.Y),
         f32(Vd.Width),
         f32(Vd.Height),
         f32(Vs.X),
         f32(Vs.Y),
         f32(Vs.Width),
         f32(Vs.Height) );

    // build pain blur ********************************************************

    f32 MipOffset = s_Post.MipOffset[s_Post.MipPaletteIndex];
    if( MipOffset )
    {
        // Blit image and blend with red ......................................

        g_RenderState.Set(
            D3DRS_BLENDCOLOR,
            D3DCOLOR_RGBA(
                s_Post.MipColor[s_Post.MipPaletteIndex].R,
                s_Post.MipColor[s_Post.MipPaletteIndex].G,
                s_Post.MipColor[s_Post.MipPaletteIndex].B,
                // Devide by two so the following blits show more colour
                s_Post.MipColor[s_Post.MipPaletteIndex].A/2 ) );

        g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_CONSTANTCOLOR );
        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ZERO );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,
                           D3DCOLORWRITEENABLE_GREEN
                        |  D3DCOLORWRITEENABLE_BLUE
                        |  D3DCOLORWRITEENABLE_RED );

        g_Texture.Set( 0,m_pTexture[kSAMPLE0] );
        CalculateViewport( kSAMPLE0,Vs );

        ResizePipTexture( Vs.Width,Vs.Height );
        g_RenderTarget.Set( kPIP,-1 );
        CalculateViewport ( kPIP,Vd );

        Blt( f32(Vd.X),
             f32(Vd.Y),
             f32(Vd.Width),
             f32(Vd.Height),
             f32(0.5f),
             f32(Vs.X),
             f32(Vs.Y),
             f32(Vs.Width),
             f32(Vs.Height) );

        // Create pain effect -- ouch that hurt! ..............................

        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVCONSTANTALPHA );
        g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_CONSTANTALPHA );
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_RED   |
            D3DCOLORWRITEENABLE_GREEN |
            D3DCOLORWRITEENABLE_BLUE  );

        f32 Shift = MipOffset;

        Blt( f32(Vd.X)-Shift,f32(Vd.Y)-Shift,f32(Vd.Width),f32(Vd.Height),f32(Vs.X),f32(Vs.Y),f32(Vs.Width),f32(Vs.Height) );
        Blt( f32(Vd.X)+Shift,f32(Vd.Y)-Shift,f32(Vd.Width),f32(Vd.Height),f32(Vs.X),f32(Vs.Y),f32(Vs.Width),f32(Vs.Height) );
        Blt( f32(Vd.X)+Shift,f32(Vd.Y)+Shift,f32(Vd.Width),f32(Vd.Height),f32(Vs.X),f32(Vs.Y),f32(Vs.Width),f32(Vs.Height) );
        Blt( f32(Vd.X)-Shift,f32(Vd.Y)+Shift,f32(Vd.Width),f32(Vd.Height),f32(Vs.X),f32(Vs.Y),f32(Vs.Width),f32(Vs.Height) );

        g_Texture.Set( 0, m_pTexture[kPIP] );

        // Setup a fake Z buffer target
        SetRenderTarget( kSAMPLE2,-1 );
        g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0xFFFFFFFF,0.0f,0 );

        // Force depth of field to use pain image
        SetRenderTarget( kPRIMARY,-1 );
        g_Texture.Set( 3,m_pTexture[kSAMPLE2] );
    }
    else
    {
        g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
        g_Texture.Set( 0, m_pTexture[kSAMPLE0] );

        SetRenderTarget( kSAMPLE1,-1,Vd );
        CalculateViewport( kSAMPLE0,Vs );

        Blt( f32(Vd.X),
             f32(Vd.Y),
             f32(Vd.Width),
             f32(Vd.Height),
             0.5f,
             f32(Vs.X),
             f32(Vs.Y),
             f32(Vs.Width),
             f32(Vs.Height) );

        g_Texture.Set( 0,m_pTexture[kSAMPLE1] );

        /* setup z-buffer as texture */

        D3DSURFACE_DESC Desc;
        m_pTarget[kPRIMARY_Z]->GetDesc( &Desc );
        texture_factory::handle Handle = g_TextureFactory.Alias(
            Desc.Size/Desc.Height,
            Desc.Width,
            Desc.Height,
            D3DFMT_LIN_A8B8G8R8,
            NULL,
            texture_factory::ALIAS_FROM_SCRATCH
        );
        ( (IDirect3DTexture8*)Handle )->Data = m_pTexture[kPRIMARY_Z]->Data;
        g_Texture.Set( 3,Handle );
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::SetupDownSampling( void )
{
    g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_GAUSSIANCUBIC );
    g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

//  g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_ENABLE );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , D3DTADDRESS_CLAMP );

    g_TextureStageState.Set( 0, D3DTSS_COLOROP  , D3DTOP_SELECTARG1 );
    g_TextureStageState.Set( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE  );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAOP  , D3DTOP_DISABLE );
    g_TextureStageState.Set( 1, D3DTSS_COLOROP  , D3DTOP_DISABLE );
    g_TextureStageState.Set( 1, D3DTSS_ALPHAOP  , D3DTOP_DISABLE );

    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
    g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );

    g_RenderState.Set( D3DRS_MULTISAMPLEANTIALIAS,TRUE );
    g_RenderState.Set(
        D3DRS_MULTISAMPLERENDERTARGETMODE,
        D3DMULTISAMPLEMODE_1X
    );
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::CreateGlowEffect( void )
{
    BloomFilter( kPRIMARY,m_OptW,m_OptH );
    ApplyGlow();
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::BloomFilter( s32 Source,f32 Width,f32 Height )
{
    g_pShaderMgr->SetPixelShader( kDIFFUSE_SAT );
    SetupDownSampling();

    const s32 T[4]={ kPRIMARY,kSAMPLE0,kSAMPLE1,kSAMPLE2 };
    const s32 E[4]=
    {
        D3DTALPHAKILL_ENABLE,
        D3DTALPHAKILL_DISABLE,
        D3DTALPHAKILL_DISABLE,
        D3DTALPHAKILL_DISABLE
    };

    g_RenderTarget.Set( kSAMPLE0,-1 );
    g_pd3dDevice->Clear(
        0,
        NULL,
        D3DCLEAR_TARGET_G |
        D3DCLEAR_TARGET_B |
        D3DCLEAR_TARGET_R ,
        0,
        0.0f,
        0
    );

    D3DVIEWPORT8 Vd;
    D3DVIEWPORT8 Vs;
    for( s32 i=1;i<4;i++ )
    {
        g_TextureStageState.Set( 0,D3DTSS_ALPHAKILL,E[i-1] );
        g_Texture.Set( 0,m_pTexture[ T[i-1] ] );
        CalculateViewport( T[i-1],Vs );
        SetRenderTarget( T[i],-1,Vd );
        Blt( f32(Vd.X     ),
             f32(Vd.Y     ),
             f32(Vd.Width ),
             f32(Vd.Height),
             f32(Vs.X     )+0.5f,
             f32(Vs.Y     )+0.5f,
             f32(Vs.Width ),
             f32(Vs.Height)
        );
    }

    JitterGlows();
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::ApplyGlow( void )
{
    // setup blender **********************************************************

    g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );

    D3DVIEWPORT8 Vp;
    SetRenderTarget( kGLOW_ACCUMULATOR,-1,Vp );
    g_Texture.Set( 0, m_pTexture[ kSAMPLE2 ] );

    // Make the glows pulsate *************************************************

    if( !s_bForcedToGlow )
    {
        g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_GAUSSIANCUBIC );
        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_SRCALPHA );
        g_RenderState.Set( D3DRS_SRCBLEND, D3DBLEND_ONE );
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );

        // streak the glows slightly ......................................

        Blt( f32(Vp.X),
             f32(Vp.Y),
             f32(Vp.Width),
             f32(Vp.Height),
             f32(Vp.X),
             f32(Vp.Y),
             f32(Vp.Width),
             f32(Vp.Height) );

        // wipe away gaussian artifacts ...................................

        static f32 SatConst = 0.0125f;
        Blt( f32(Vp.X),
             f32(Vp.Y),
             f32(Vp.Width),
             f32(Vp.Height),
             SatConst,
             f32(Vp.X),
             f32(Vp.Y),
             f32(Vp.Width ),
             f32(Vp.Height)
        );
    }
    else
    {
        s_GlowScale += s_GlowInc;

        if( s_GlowScale >= s_GlowEnd ){ s_GlowScale = s_GlowEnd; s_GlowInc *= -1; }
        if( s_GlowScale <= s_GlowBeg ){ s_GlowScale = s_GlowBeg; s_GlowInc *= -1; }

        g_RenderState.Set( D3DRS_BLENDCOLOR,D3DCOLOR_RGBA( 0,0,0,u8(s_GlowScale*255.0f) ));
        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_CONSTANTALPHA );
        g_RenderState.Set( D3DRS_SRCBLEND, D3DBLEND_ONE );
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );

        Blt( f32(Vp.X),
             f32(Vp.Y),
             f32(Vp.Width),
             f32(Vp.Height),
             m_MotionBlurIntensity,
             f32(Vp.X)+0.5f,
             f32(Vp.Y)+0.5f,
             f32(Vp.Width),
             f32(Vp.Height)
        );
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::JitterGlows( void )
{
    //////////////////////////////////////////////////////////////////////////
    //  ----------------------------------------------------------------------
    //
    //  Jitter horizontally
    //
    g_TextureStageState.Set( 0, D3DTSS_COLOROP  , D3DTOP_SELECTARG1 );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAOP  , D3DTOP_SELECTARG1 );
    g_TextureStageState.Set( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

    g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ONE );
    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );

    g_Texture.Set( 0,m_pTexture[kSAMPLE2] );

    D3DVIEWPORT8 Vp;
    g_RenderTarget.Set( kJITTER,-1 );
    CalculateViewport( kJITTER,Vp );
    g_pd3dDevice->Clear(
        0,
        NULL,
        D3DCLEAR_TARGET_G |
        D3DCLEAR_TARGET_B |
        D3DCLEAR_TARGET_R ,
        0,
        0.0f,
        0
    );

    g_pd3dDevice->SetViewport( &Vp );

    //  ----------------------------------------------------------------------
    //
    //  Jitter horizontally
    //
    struct glow_pass
    {
        f32 XOffset;
        f32 YOffset;
        f32 Alpha;
    };

    static f32 Tweak = 255.0f;

    f32 x;
    {
    #if 0
        static const glow_pass HorzPasses[] =
        {
            {  0.0f,0.0f,32.0f },
            { -1.0f,0.0f,28.0f },
            { -2.0f,0.0f,24.0f },
            { -3.0f,0.0f,20.0f },
            { -4.0f,0.0f,16.0f },
            { -5.0f,0.0f,10.0f },
            { -6.0f,0.0f, 8.0f },
            {  1.0f,0.0f,28.0f },
            {  2.0f,0.0f,24.0f },
            {  3.0f,0.0f,20.0f },
            {  4.0f,0.0f,16.0f },
            {  5.0f,0.0f,10.0f },
            {  6.0f,0.0f, 8.0f },
        };
    #else
        static const glow_pass HorzPasses[] =
        {
            // alphas from zero to one are 0.1875, 0.15625, 0.125, 0.078125, and 0.0625
            {  0,  0, 24 },
            { -1,  0, 20 }, { -2,  0, 16 }, { -3,  0, 10 }, { -4,  0,  8 },
            {  1,  0, 20 }, {  2,  0, 16 }, {  3,  0, 10 }, {  4,  0,  8 },
        };
    #endif
        const f32 Passes = f32( sizeof(HorzPasses)/sizeof(HorzPasses[0]) );

        for( x=0;x<Passes;x+=1.0f )
        {
            s32 i = s32(x);
            Blt( f32(Vp.X+HorzPasses[i].XOffset),
                 f32(Vp.Y+HorzPasses[i].YOffset),
                 f32(Vp.Width),
                 f32(Vp.Height),
                 // This value is not arbitrary; it's the amount of brightness
                 // to submit to the accumulation buffer.
                 HorzPasses[i].Alpha/Tweak,
                 f32(Vp.X),
                 f32(Vp.Y),
                 f32(Vp.Width),
                 f32(Vp.Height)
            );
        }
    }

    //  ----------------------------------------------------------------------
    //
    //  Jitter vertically and store final frame
    //
    g_Texture.Set( 0,m_pTexture[kJITTER] );
    g_RenderTarget.Set( kSAMPLE2,-1 );
    CalculateViewport( kSAMPLE2,Vp );
    g_pd3dDevice->Clear(
        0,
        NULL,
        D3DCLEAR_TARGET_G |
        D3DCLEAR_TARGET_B |
        D3DCLEAR_TARGET_R ,
        0,
        0.0f,
        0
    );

    g_pd3dDevice->SetViewport( &Vp );

    f32 y;
    {
    #if 0
        static const glow_pass VertPasses[] =
        {
            { 0.0f, 0.0f,32.0f },
            { 0.0f,-1.0f,28.0f },
            { 0.0f,-2.0f,24.0f },
            { 0.0f,-3.0f,20.0f },
            { 0.0f,-4.0f,16.0f },
            { 0.0f,-5.0f,10.0f },
            { 0.0f,-6.0f, 8.0f },
            { 0.0f, 1.0f,28.0f },
            { 0.0f, 2.0f,24.0f },
            { 0.0f, 3.0f,20.0f },
            { 0.0f, 4.0f,16.0f },
            { 0.0f, 5.0f,10.0f },
            { 0.0f, 6.0f, 8.0f },
        };
    #else
        static const glow_pass VertPasses[] =
        {
            // alphas from zero to one are 0.15625, 0.125, 0.078125, and 0.0625 divided by 0.1875
            {  0,  0, 96 },
            {  0, -1, 40 }, {  0, -2, 32 }, {  0, -3, 20 }, {  0, -4,  16 },
            {  0,  1, 40 }, {  0,  2, 32 }, {  0,  3, 20 }, {  0,  4,  16 },
        };
    #endif
        const f32 Passes = f32( sizeof(VertPasses)/sizeof(VertPasses[0]) );

        for( y=0;y<Passes;y+=1.0f )
        {
            s32 i = s32(y);
            Blt( f32(Vp.X)+VertPasses[i].XOffset,
                 f32(Vp.Y)+VertPasses[i].YOffset,
                 f32(Vp.Width),
                 f32(Vp.Height),
                 // This value is not arbitrary; it's the amount of brightness
                 // to submit to the accumulation buffer.
                 VertPasses[i].Alpha/Tweak,
                 f32(Vp.X),
                 f32(Vp.Y),
                 f32(Vp.Width),
                 f32(Vp.Height)
            );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

template< class t >void OutputShaderMask( t* pDetails,u32 Count )
{
#ifndef X_RETAIL

    if( SWITCH_DUMP_SHADER_LIBRARIES )
    {
        u32 i,n;
        OutputDebugString( "-----------------------------------------------\n" );
        if( sizeof( pDetails->Flags.Mask )==4 )
        {
            OutputDebugString( "        " );
            OutputDebugString( xfs("const u32 KnownVSFlags[%d]=\n",Count ));
            OutputDebugString( "        {\n" );
            n = 4;
        }
        else
        {
            OutputDebugString( "        " );
            OutputDebugString( xfs("const u64 KnownPSFlags[%d]=\n",Count ));
            OutputDebugString( "        {\n" );
            n = 3;
        }
        OutputDebugString( "        " );
        for( i=0;i<Count;i++ )
        {
            if( sizeof( pDetails->Flags.Mask )==4 )
                OutputDebugString( xfs("0x%08X,",pDetails[i].Flags.Mask ));
            else
            {
                u64 Mask = pDetails[i].Flags.Mask;
                OutputDebugString( xfs("0x%08X%08X,",u32(Mask>>32),u32(Mask) ));
            }
            if( !((i+1)%n) )
                OutputDebugString( "\n        " );
        }
        if( i%n )
            OutputDebugString( "\n        " );
        OutputDebugString( "};\n" );
    }
#else
    (void)pDetails;
    (void)Count;
#endif
}

///////////////////////////////////////////////////////////////////////////////

template< class t >bool ExportShaderLibrary( t* pDetails,u32 Count,u32 Size,u32& OldSize,const char* pExt )
{
    #if 0//ndef CONFIG_RETAIL
    {
        if( Size > OldSize )
        {
            u8* pSpace = (u8*)x_malloc( Size ),*p=pSpace;
            u8* pEnd = pSpace+Size;
            u32 i,o;
            {
                // pixel shaders ..............................................

                for( Size=i=0;i<Count;i++ )
                {
                    XGBuffer& Buffer = pDetails[i].Shader.Microcode;
                    ASSERT( p < pEnd );
                    x_memcpy(
                        p,
                        Buffer.pData,
                        Buffer.size
                    );
                    Size += Buffer.size;
                    p    += Buffer.size;
                }

                // save library ...............................................

                HANDLE hFile = CreateFile(
                    xfs("Z:\\shaders.%s",pExt ),
                    GENERIC_WRITE,
                    0,
                    0,
                    OPEN_ALWAYS,
                    0,
                    0
                );
                if( hFile != INVALID_HANDLE_VALUE )
                {
                    // write count ............................................

                    DWORD Written;
                    WriteFile( hFile,&Count,sizeof(u32),&Written,NULL );
                    WriteFile( hFile,&Size ,sizeof(u32),&Written,NULL );
                    WriteFile( hFile,pDetails,Count*sizeof(t),&Written,NULL );

                    // write inventory of details .............................

                    for( o=i=0;i<Count;i++ )
                    {
                        t& Detail = pDetails[i];
                        WriteFile( hFile,&o,sizeof(u32),&Written,NULL );
                        WriteFile( hFile,&Detail.Shader.Size,sizeof(u32),&Written,NULL );
                        o += Detail.Shader.Size;
                    }

                    // write shader body ......................................

                    WriteFile( hFile,pSpace,Size,&Written,NULL );
                    CloseHandle( hFile );
                }
            }
            x_free( pSpace );

            OldSize = Size;
            return true;
        }
    }
    #else
    if( Size > OldSize )
    {
        OldSize = Size;
        return true;
    }
    #endif

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void GpuDone( DWORD Context )
{
    (void)Context;

    g_pPipeline->m_Time.Stop();
}

///////////////////////////////////////////////////////////////////////////////

static void BlurScopeCentre( f32 W0,f32 H0,D3DVIEWPORT8& Vd,D3DVIEWPORT8& Vs )
{
    g_Texture.Set( 1,g_pPipeline->m_pTexture[pipeline_mgr::kSAMPLE1] );

    //  ----------------------------------------------------------------------

    g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_ENABLE );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , D3DTADDRESS_WRAP  );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , D3DTADDRESS_WRAP  );

    g_TextureStageState.Set( 0, D3DTSS_COLOROP  , D3DTOP_SELECTARG1 );
    g_TextureStageState.Set( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE     );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAOP  , D3DTOP_SELECTARG1 );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE     );

    g_TextureStageState.Set( 1, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
    g_TextureStageState.Set( 1, D3DTSS_ADDRESSU , D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 1, D3DTSS_ADDRESSV , D3DTADDRESS_CLAMP );

    g_TextureStageState.Set( 1, D3DTSS_COLOROP  , D3DTOP_SELECTARG1 );
    g_TextureStageState.Set( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE     );
    g_TextureStageState.Set( 1, D3DTSS_ALPHAOP  , D3DTOP_DISABLE    );

    g_TextureStageState.Set( 2, D3DTSS_COLOROP  , D3DTOP_DISABLE    );
    g_TextureStageState.Set( 2, D3DTSS_ALPHAOP  , D3DTOP_DISABLE    );

    g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_ZERO );
    g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
    g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
    g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_ONE );

    //  ----------------------------------------------------------------------
    //
    //  Calculate ratios between source and destination viewports
    //
    f32 Dx = f32(Vd.X);
    f32 Dy = f32(Vd.Y);
    f32 Wd = f32(Vd.Width );
    f32 Hd = f32(Vd.Height);
    f32 W1 = f32(Vs.Width );
    f32 H1 = f32(Vs.Height);
    f32 Ox = Wd*0.5f;
    f32 Oy = Hd*0.5f;

    f32 Gx = f32(g_PhysW-640)*0.5f;
    f32 ScaleX  = ((W0*2.0f) < Wd ) ? 1.0f : (Wd/(W0*2.0f));
    f32 ScaleY  = ((H0*2.0f) < Hd ) ? 1.0f : (Hd/(H0*2.0f));
    f32 Scale   = MIN( ScaleX, ScaleY );

    W0 *= Scale;
    H0 *= Scale;

    //  ----------------------------------------------------------------------
    //
    //  Setup four quads to construct sniper scope
    //
    struct vertex
    {
        f32 x,y,z,w;
        f32 u0,v0;
        f32 u1,v1;
    }
    TriList[16]=
    {
        {/*XYZRHZ*/Gx+Dx+(Ox-W0),Dy+(Oy-H0),0.0f,0.0f,/*UV0*/ 0.0f, 0.0f,/*UV1*/(Dx+(Ox-W0))*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {          Gx+Dx+(Ox   ),Dy+(Oy-H0),0.0f,0.0f,        1.0f, 0.0f,       (Dx+(Ox   ))*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {          Gx+Dx+(Ox   ),Dy+(Oy   ),0.0f,0.0f,        1.0f, 1.0f,       (Dx+(Ox   ))*(W1/Wd),(Dy+(Oy   ))*(H1/Hd) },
        {          Gx+Dx+(Ox-W0),Dy+(Oy   ),0.0f,0.0f,        0.0f, 1.0f,       (Dx+(Ox-W0))*(W1/Wd),(Dy+(Oy   ))*(H1/Hd) },
        {/*XYZRHZ*/Gx+Dx+(Ox   ),Dy+(Oy-H0),0.0f,0.0f,/*UV0*/ 0.0f, 0.0f,/*UV1*/(Dx+(Ox   ))*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {          Gx+Dx+(Ox+W0),Dy+(Oy-H0),0.0f,0.0f,       -1.0f, 0.0f,       (Dx+(Ox+W0))*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {          Gx+Dx+(Ox+W0),Dy+(Oy   ),0.0f,0.0f,       -1.0f, 1.0f,       (Dx+(Ox+W0))*(W1/Wd),(Dy+(Oy   ))*(H1/Hd) },
        {          Gx+Dx+(Ox   ),Dy+(Oy   ),0.0f,0.0f,        0.0f, 1.0f,       (Dx+(Ox   ))*(W1/Wd),(Dy+(Oy   ))*(H1/Hd) },
        {/*XYZRHZ*/Gx+Dx+(Ox-W0),Dy+(Oy   ),0.0f,0.0f,/*UV0*/ 0.0f, 0.0f,/*UV1*/(Dx+(Ox-W0))*(W1/Wd),(Dy+(Oy   ))*(H1/Hd) },
        {          Gx+Dx+(Ox   ),Dy+(Oy   ),0.0f,0.0f,        1.0f, 0.0f,       (Dx+(Ox   ))*(W1/Wd),(Dy+(Oy   ))*(H1/Hd) },
        {          Gx+Dx+(Ox   ),Dy+(Oy+H0),0.0f,0.0f,        1.0f,-1.0f,       (Dx+(Ox   ))*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
        {          Gx+Dx+(Ox-W0),Dy+(Oy+H0),0.0f,0.0f,        0.0f,-1.0f,       (Dx+(Ox-W0))*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
        {/*XYZRHZ*/Gx+Dx+(Ox   ),Dy+(Oy   ),0.0f,0.0f,/*UV0*/ 0.0f, 0.0f,/*UV1*/(Dx+(Ox   ))*(W1/Wd),(Dy+(Oy   ))*(H1/Hd) },
        {          Gx+Dx+(Ox+W0),Dy+(Oy   ),0.0f,0.0f,       -1.0f, 0.0f,       (Dx+(Ox+W0))*(W1/Wd),(Dy+(Oy   ))*(H1/Hd) },
        {          Gx+Dx+(Ox+W0),Dy+(Oy+H0),0.0f,0.0f,       -1.0f,-1.0f,       (Dx+(Ox+W0))*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
        {          Gx+Dx+(Ox   ),Dy+(Oy+H0),0.0f,0.0f,        0.0f,-1.0f,       (Dx+(Ox   ))*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
    };

    //  ----------------------------------------------------------------------
    //
    //  Draw quad
    //
    //eng_SetViewport( View );
    g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX2 );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST,4,TriList,sizeof(vertex) );
}

///////////////////////////////////////////////////////////////////////////////

static void BlurScopeRemains(  f32 W0,f32 H0,D3DVIEWPORT8& Vd,D3DVIEWPORT8& Vs )
{
    g_Texture.Set( 0,g_pPipeline->m_pTexture[pipeline_mgr::kSAMPLE1] );

    //  ----------------------------------------------------------------------

    g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSU , D3DTADDRESS_CLAMP );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSV , D3DTADDRESS_CLAMP );

    g_TextureStageState.Set( 0, D3DTSS_COLOROP  , D3DTOP_SELECTARG1 );
    g_TextureStageState.Set( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE     );
    g_TextureStageState.Set( 0, D3DTSS_ALPHAOP  , D3DTOP_DISABLE    );

    g_TextureStageState.Set( 1, D3DTSS_COLOROP  , D3DTOP_DISABLE    );
    g_TextureStageState.Set( 1, D3DTSS_ALPHAOP  , D3DTOP_DISABLE    );

    //  ----------------------------------------------------------------------
    //
    //  Calculate ratios between source and destination viewports
    //
    f32 Dx = f32(Vd.X);
    f32 Dy = f32(Vd.Y);
    f32 Wd = f32(Vd.Width );
    f32 Hd = f32(Vd.Height);
    f32 W1 = f32(Vs.Width );
    f32 H1 = f32(Vs.Height);
    f32 Ox = Wd*0.5f;
    f32 Oy = Hd*0.5f;

    f32 Gx = f32(g_PhysW-640)*0.5f;
    f32 ScaleX  = ((W0*2.0f) < Wd ) ? 1.0f : (Wd/(W0*2.0f));
    f32 ScaleY  = ((H0*2.0f) < Hd ) ? 1.0f : (Hd/(H0*2.0f));
    f32 Scale   = MIN( ScaleX, ScaleY );

    W0 *= Scale;
    H0 *= Scale;

    //  ----------------------------------------------------------------------
    //
    //  Setup quads around sniper scope
    //
    struct vertex
    {
        f32 x,y,z,w;
        f32 u,v;
    }
    TriList[16] = 
    {
        {/*XYZRHZ*/Gx+Dx        ,Dy        ,0.0f,0.0f,/*UV*/(Dx        )*(W1/Wd),(Dy        )*(H1/Hd) },
        {          Gx+Dx+Wd     ,Dy        ,0.0f,0.0f,      (Dx+Wd     )*(W1/Wd),(Dy        )*(H1/Hd) },
        {          Gx+Dx+Wd     ,Dy+(Oy-H0),0.0f,0.0f,      (Dx+Wd     )*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {          Gx+Dx        ,Dy+(Oy-H0),0.0f,0.0f,      (Dx        )*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {/*XYZRHZ*/Gx+Dx        ,Dy+(Oy-H0),0.0f,0.0f,/*UV*/(Dx        )*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {          Gx+Dx+(Ox-W0),Dy+(Oy-H0),0.0f,0.0f,      (Dx+(Ox-W0))*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {          Gx+Dx+(Ox-W0),Dy+(Oy+H0),0.0f,0.0f,      (Dx+(Ox-W0))*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
        {          Gx+Dx        ,Dy+(Oy+H0),0.0f,0.0f,      (Dx        )*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
        {/*XYZRHZ*/Gx+Dx+(Ox+W0),Dy+(Oy-H0),0.0f,0.0f,/*UV*/(Dx+(Ox+W0))*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {          Gx+Dx+Wd     ,Dy+(Oy-H0),0.0f,0.0f,      (Dx+Wd     )*(W1/Wd),(Dy+(Oy-H0))*(H1/Hd) },
        {          Gx+Dx+Wd     ,Dy+(Oy+H0),0.0f,0.0f,      (Dx+Wd     )*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
        {          Gx+Dx+(Ox+W0),Dy+(Oy+H0),0.0f,0.0f,      (Dx+(Ox+W0))*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
        {/*XYZRHZ*/Gx+Dx        ,Dy+(Oy+H0),0.0f,0.0f,/*UV*/(Dx        )*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
        {          Gx+Dx+Wd     ,Dy+(Oy+H0),0.0f,0.0f,      (Dx+Wd     )*(W1/Wd),(Dy+(Oy+H0))*(H1/Hd) },
        {          Gx+Dx+Wd     ,Dy+Hd     ,0.0f,0.0f,      (Dx+Wd     )*(W1/Wd),(Dy+Hd     )*(H1/Hd) },
        {          Gx+Dx        ,Dy+Hd     ,0.0f,0.0f,      (Dx        )*(W1/Wd),(Dy+Hd     )*(H1/Hd) },
    };

    //  ----------------------------------------------------------------------
    //
    //  Draw quad
    //
    //eng_SetViewport( View );
    g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_QUADLIST,4,TriList,sizeof(vertex) );
}

///////////////////////////////////////////////////////////////////////////////

void xbox_ApplyScopeBlur( xbitmap& Bitmap,D3DVIEWPORT8& Vd,D3DVIEWPORT8& Vs )
{
    g_pShaderMgr->SetPixelShader( kREMOVE );

    //  ----------------------------------------------------------------------

    matrix4 I; I.Identity();
    g_pd3dDevice->SetTransform( D3DTS_TEXTURE0,( const D3DMATRIX* )&I );
    g_pd3dDevice->SetTransform( D3DTS_TEXTURE1,( const D3DMATRIX* )&I );
    g_pd3dDevice->SetTransform( D3DTS_WORLD   ,( const D3DMATRIX* )&I );
    g_pd3dDevice->SetTransform( D3DTS_VIEW    ,( const D3DMATRIX* )&I );

    D3DXMATRIX Proj;
    D3DXMatrixPerspectiveFovLH( &Proj, D3DX_PI/3, 4.0f/3.0f, 1.0f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &Proj );

    g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CCW );
    g_RenderState.Set( D3DRS_COLORWRITEENABLE,
        D3DCOLORWRITEENABLE_RED   |
        D3DCOLORWRITEENABLE_GREEN |
        D3DCOLORWRITEENABLE_BLUE  );

    //  ----------------------------------------------------------------------

    f32 sh = (f32)Bitmap.GetHeight();
    f32 sw = (f32)Bitmap.GetWidth ();
    vram_Activate( 0,Bitmap.GetVRAMID() );

    g_pPipeline->CalculateViewport( pipeline_mgr::kPRIMARY,Vd );
    g_pPipeline->CalculateViewport( pipeline_mgr::kSAMPLE1,Vs );

    BlurScopeCentre ( sw,sh,Vd,Vs );
    BlurScopeRemains( sw,sh,Vd,Vs );
}

///////////////////////////////////////////////////////////////////////////////

void xbox_GetResolutionText( xwstring& String )
{
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::ApplyFog( void )
{
    if( SWITCH_USE_FOG )
    {
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
        g_TextureStageState.Set( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );

        g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
        g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA );
        g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,TRUE );
        g_RenderState.Set( D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_RED   |
            D3DCOLORWRITEENABLE_GREEN |
            D3DCOLORWRITEENABLE_BLUE  );

        // Create texture out of the back buffer ******************************

        CreateAliasedTarget( kFOG,-1,640,480 );

        ASSERT( m_pTarget[kFOG] );
        g_Texture.Set( 0,m_pTexture[kFOG] );

        // Apply fog to buffer ************************************************

        ps::desc PSFlags; PSFlags.clear();
        vs::desc VSFlags; VSFlags.clear();

        PSFlags.xfc_Std = true;
        PSFlags.bAlpha  = true;

        g_pShaderMgr->Link( 0,0,VSFlags,0,PSFlags );

        D3DVIEWPORT8 Vd;
        SetRenderTarget( kPRIMARY,-1,Vd );

        D3DVIEWPORT8 Vs;
        CalculateViewport( kFOG,Vs );

        Blt( f32(Vd.X),
             f32(Vd.Y),
             f32(Vd.Width),
             f32(Vd.Height),
             1.0f,
             f32(Vs.X),
             f32(Vs.Y),
             f32(Vs.Width),
             f32(Vs.Height)
        );
        m_pTarget[kFOG]->Release();
        m_pTarget[kFOG] = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

void pipeline_mgr::PostEffect( void )
{
    if( m_bPipActive )
        return;



      ///////////////////////
     // APPLY POST EFFECT //
    ///////////////////////



    #ifndef CONFIG_RETAIL
    D3DPERF_BeginEvent( D3DCOLOR_RGBA( 108,108,172,255 ),"Rendering post effect" );
    #endif
    {
        // 2d states **********************************************************

        g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL );
        g_RenderState.Set( D3DRS_ALPHABLENDENABLE,FALSE );
        g_RenderState.Set( D3DRS_ALPHATESTENABLE,FALSE );
        g_RenderState.Set( D3DRS_SPECULARENABLE,FALSE );
        g_RenderState.Set( D3DRS_CULLMODE,D3DCULL_CCW );
        g_RenderState.Set( D3DRS_LIGHTING,FALSE );
        g_RenderState.Set( D3DRS_ZENABLE,FALSE );

        // 2d transform *******************************************************

        matrix4 I; I.Identity();
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE0,( const D3DMATRIX* )&I );
        g_pd3dDevice->SetTransform( D3DTS_TEXTURE1,( const D3DMATRIX* )&I );
        g_pd3dDevice->SetTransform( D3DTS_WORLD   ,( const D3DMATRIX* )&I );
        g_pd3dDevice->SetTransform( D3DTS_VIEW    ,( const D3DMATRIX* )&I );

        D3DXMATRIX Proj;
        D3DXMatrixPerspectiveFovLH( &Proj, D3DX_PI/3, 4.0f/3.0f, 1.0f, 100.0f );
        g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &Proj );

        g_RenderState.Set( D3DRS_COLORWRITEENABLE,
            D3DCOLORWRITEENABLE_RED   |
            D3DCOLORWRITEENABLE_GREEN |
            D3DCOLORWRITEENABLE_BLUE  );

        // compositer *********************************************************

        u32 iStg = 0;
        {
            // Stage 0: Depth of field blur
            // Stage 1: Offscreen renderered frame
            // Stage 2: Glow buffer
            // Stage 3: Z-buffer
            {
                ApplyFog();
                CreateGlowEffect ();
                SetupDepthOfField();

                // choose DOF or pain blur ....................................

                g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
                g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
                g_TextureStageState.Set( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
                g_TextureStageState.Set( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
                g_TextureStageState.Set( 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
                g_TextureStageState.Set( 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
                g_TextureStageState.Set( 3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
                g_TextureStageState.Set( 3, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

                g_Texture.Set( 1, m_pTexture[kPRIMARY] );

                // setup glow texture .........................................

                g_Texture.Set( 2,m_pTexture[kGLOW_ACCUMULATOR] );

                // apply blend effect .........................................

                BlendEffects( kFULL_POST_EFFECT );
            }
        }

        // track player views *************************************************

        m_Zone = (m_Zone+1) % m_nZones;

        // save shader libraries **********************************************

        #if (!defined CONFIG_RETAIL) && (defined bhapgood)
        {
            // this should never happen--but just in case .....................

            if( ExportShaderLibrary( ps::Details,ps::Count,ps::Size,ps::OldSize,"psl" ) ||
                ExportShaderLibrary( vs::Details,vs::Count,vs::Size,vs::OldSize,"vsl" ) )
            {
                // flash screen red and pause to show shaders were written ....

                SwitchToBackBuffer();
                g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,D3DCOLOR_RGBA( 255,0,0,0 ),0,0 );

                // write shader flags to log ..................................

                OutputShaderMask( ps::Details,ps::Count );
                OutputShaderMask( vs::Details,vs::Count );

                // debug memory stats .........................................

                static s32   TiledRAM;
                static s32 TextureRAM;
                static s32  VertexRAM;
                static s32    PushRAM;
                static s32 GeneralRAM;

                  TiledRAM = g_TextureFactory.  GetTiledPool().GetSize()-g_TextureFactory.  GetTiledPool().GetFree();
                TextureRAM = g_TextureFactory.GetGeneralPool().GetSize()-g_TextureFactory.GetGeneralPool().GetFree();
                 VertexRAM =    g_VertFactory.GetGeneralPool().GetSize()-   g_VertFactory.GetGeneralPool().GetFree();
                   PushRAM =    g_PushFactory.GetGeneralPool().GetSize()-   g_PushFactory.GetGeneralPool().GetFree();
                GeneralRAM = x_MemGetUsed();

                static MEMORYSTATUS Status;
                GlobalMemoryStatus( &Status );

                extern u32 DXT1_Allocs;
                extern u32 DXT3_Allocs;
                extern u32 DXT5_Allocs;

                OutputDebugString( xfs("Total Physical RAM :%8d\n",TotalPhysicalAvail) );
                OutputDebugString( xfs(" Free Physical RAM :%8d\n",TotalPhysicalFree)  );
                OutputDebugString( xfs(" Used Physical RAM :%8d\n",TotalPhysicalFree-Status.dwAvailPhys) );
                OutputDebugString( "----------------------------\n" );
                OutputDebugString( xfs("    Tiled memory : %d\n",TiledRAM) );
                OutputDebugString( xfs("       OS memory : %d\n",Status.dwTotalPhys-TiledRAM-TextureRAM-VertexRAM-PushRAM-GeneralRAM) );
                OutputDebugString( "----------------------------\n" );
                OutputDebugString( xfs("General memory  : %d\n",GeneralRAM) );
                OutputDebugString( xfs("Texture memory  : %d\n(DXT1%8d, DXT3%8d, DXT5%8d)\n",TextureRAM,DXT1_Allocs,DXT3_Allocs,DXT5_Allocs) );
                OutputDebugString( xfs(" Vertex memory  : %d\n", VertexRAM) );
                OutputDebugString( xfs("   Push memory  : %d\n",   PushRAM) );
            }
        }
        #endif
    }
    #ifndef CONFIG_RETAIL
    D3DPERF_EndEvent();
    #endif

    if( m_bColourStreamActive )
    {
        g_pd3dDevice->SetStreamSource( 1,NULL,sizeof( D3DCOLOR ));
        m_bColourStreamActive = 0;
    }

    g_PushFactory.Flush();
    s_TotalDynLights = 0;
    m_State = 0;
}
